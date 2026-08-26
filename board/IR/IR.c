#include "IR.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdlib.h>

// 复用共用的 ADC1 互斥锁 (在全局或主初始化中创建)
extern SemaphoreHandle_t xADC1_Mutex;

/**
  * @brief  红外火焰传感器 GPIO 与 ADC1 初始化 (PA2 - ADC1_IN2)
  */
void IR_config(void) 
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    // GPIOA Pin2 初始化为模拟输入
    GPIO_InitTypeDef mygpio;
    mygpio.GPIO_Mode  = GPIO_Mode_AN;
    mygpio.GPIO_Pin   = IR_PIN;
    mygpio.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    mygpio.GPIO_Speed = GPIO_High_Speed;
    GPIO_Init(IR_GROUP, &mygpio);

    // ADC1 参数初始化
    ADC_InitTypeDef myadc = {0};
    myadc.ADC_ContinuousConvMode = DISABLE;
    myadc.ADC_DataAlign          = ADC_DataAlign_Right;
    myadc.ADC_NbrOfConversion    = 1;
    myadc.ADC_Resolution         = ADC_Resolution_12b;
    myadc.ADC_ScanConvMode       = DISABLE;
    ADC_Init(ADC1, &myadc);

    ADC_Cmd(ADC1, ENABLE);

    if (xADC1_Mutex == NULL) {
        xADC1_Mutex = xSemaphoreCreateMutex();
    }
}

/**
  * @brief  获取单次红外传感器 ADC 采样值 (带互斥锁保护)
  */
uint16_t IR_Get_RawData(void) 
{
    uint16_t adc_val = 0;
    uint16_t timeout = 5000;

    // 获取 ADC 互斥锁
    if (xSemaphoreTake(xADC1_Mutex, pdMS_TO_TICKS(20)) == pdTRUE) 
    {
        // 切换至通道 2 (PA2)
        ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 1, ADC_SampleTime_480Cycles);

        ADC_SoftwareStartConv(ADC1);

        while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) != SET) {
            timeout--;
            if (timeout == 0) {
                xSemaphoreGive(xADC1_Mutex);
                return 4095; // 超时默认返回暗态高阻值
            }
        }
        
        adc_val = ADC_GetConversionValue(ADC1);

        xSemaphoreGive(xADC1_Mutex);
    }

    return adc_val;
}

/**
  * @brief  qsort 比较辅助函数
  */
static int Compare_Uint16(const void *a, const void *b) 
{
    return (*(uint16_t *)a - *(uint16_t *)b);
}

/**
  * @brief  中值平均滤波获取稳定 ADC 值
  */
uint16_t IR_Get_Filtered_Value(uint8_t samples) 
{
    if (samples < 3) samples = 3;
    uint16_t buffer[20];
    if (samples > 20) samples = 20;

    for (uint8_t i = 0; i < samples; i++) {
        buffer[i] = IR_Get_RawData();

        if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
            vTaskDelay(pdMS_TO_TICKS(2)); // RTOS 阻塞，不卡死 CPU
        } else {
            delay_us(500);
        }
    }

    // 排序并剔除最大最小值
    qsort(buffer, samples, sizeof(uint16_t), Compare_Uint16);

    uint32_t sum = 0;
    for (uint8_t i = 1; i < (samples - 1); i++) {
        sum += buffer[i];
    }

    return (uint16_t)(sum / (samples - 2));
}

/**
  * @brief  计算火焰传感器当前的等效电阻值 (单位: Ω)
  * @param  samples: 滤波采样次数
  * @retval 传感器阻值 (欧姆)。暗处无火返回 999999.0f，明火强光返回 0.0f
  */
float IR_Get_Fire(uint8_t times)
{
    uint16_t adc_ave = IR_Get_Filtered_Value(times);

    // 1. 下限饱和保护 (强光/火源极近，传感器导通阻值趋近 0)
    if (adc_ave <= 10) {
        return 0.0f;
    }

    // 2. 上限防除零保护 (暗态无火，ADC 采样值接近 4095)
    if (adc_ave >= 4070) {
        return 999999.0f; // 代表阻值极大 (未检测到火焰)
    }

    // 3. 基于全量程直接计算分压电阻 (避免中间浮点电压换算)
    // Formula: R_sensor = R_pull * ADC_val / (4095 - ADC_val)
    float R_sensor = 10000.0f * ((float)adc_ave / (4095.0f - (float)adc_ave));

    return R_sensor;
}
