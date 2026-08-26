#include "ADC.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdlib.h>

// 定义 ADC3 独立的硬件互斥锁
SemaphoreHandle_t xADC3_Mutex = NULL;

/**
  * @brief  ADC3 初始化 (使用 GPIOF - ADC3_IN5)
  */
void ADC_config(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    GPIO_InitTypeDef mygpio;
    mygpio.GPIO_Mode  = GPIO_Mode_AN;
    mygpio.GPIO_Pin   = ADC_PIN;
    mygpio.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    mygpio.GPIO_Speed = GPIO_High_Speed;
    GPIO_Init(ADC_GROUP, &mygpio);

    ADC_InitTypeDef myadc = {0};
    myadc.ADC_ContinuousConvMode = DISABLE;
    myadc.ADC_DataAlign          = ADC_DataAlign_Right;
    myadc.ADC_NbrOfConversion    = 1;
    myadc.ADC_Resolution         = ADC_Resolution_12b;
    myadc.ADC_ScanConvMode       = DISABLE;
    ADC_Init(ADC3, &myadc);

    ADC_Cmd(ADC3, ENABLE);

    // 创建 ADC3 互斥锁
    if (xADC3_Mutex == NULL) {
        xADC3_Mutex = xSemaphoreCreateMutex();
    }
}

/**
  * @brief  获取单次 ADC3 通道 5 采样值 (带 Timeout 与互斥锁)
  */
uint16_t ADC_Get_Data(void)
{
    uint16_t adc_val = 0;
    uint16_t timeout = 5000;

    // 获取 ADC3 互斥锁 (超时 20ms)
    if (xSemaphoreTake(xADC3_Mutex, pdMS_TO_TICKS(20)) == pdTRUE)
    {
        ADC_RegularChannelConfig(ADC3, ADC_Channel_5, 1, ADC_SampleTime_480Cycles);
        ADC_SoftwareStartConv(ADC3);

        while (ADC_GetFlagStatus(ADC3, ADC_FLAG_EOC) != SET) {
            timeout--;
            if (timeout == 0) {
                xSemaphoreGive(xADC3_Mutex);
                return 4095; // 超时返回暗态值
            }
        }

        adc_val = ADC_GetConversionValue(ADC3);
        xSemaphoreGive(xADC3_Mutex);
    }

    return adc_val;
}

/**
  * @brief  qsort 快速排序辅助比较函数
  */
static int Compare_Uint16(const void *a, const void *b)
{
    return (*(uint16_t *)a - *(uint16_t *)b);
}

/**
  * @brief  中值平均滤波 (剔除极值后取均值)
  */
float ADC_Get_Ave(uint8_t times)
{
    if (times < 3) times = 3;
    uint16_t buffer[20];
    if (times > 20) times = 20;

    for (uint8_t i = 0; i < times; i++) {
        buffer[i] = ADC_Get_Data();

        if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
            vTaskDelay(pdMS_TO_TICKS(2)); // RTOS 阻塞让出 CPU
        } else {
            delay_us(500);
        }
    }

    // 快速排序
    qsort(buffer, times, sizeof(uint16_t), Compare_Uint16);

    // 去头去尾取平均
    float sum = 0;
    for (uint8_t i = 1; i < (times - 1); i++) {
        sum += buffer[i];
    }

    return (sum / (times - 2));
}

/**
  * @brief  获取光照强度 Lux 值
  */
uint16_t ADC_Get_Light(uint8_t times)
{
    uint16_t adc_ave = ADC_Get_Ave(times);
    float adc_r = 0.0f;
    uint16_t light = 0;

    // 1. 边界下限保护 (强光饱和)
    if (adc_ave <= 10) {
        adc_r = 0.0f;
    }
    // 2. 边界上限保护 (暗态高阻，防止分母 (4095 - adc_ave) 为 0 导致崩溃)
    else if (adc_ave >= 4070) {
        adc_r = 999999.0f; // 代表极其微弱的光强或无光
    }
    else {
        // 3. 基于 ADC 采样全量程直接换算光敏电阻阻值:
        //    R_sensor = R_pull * ADC / (4095 - ADC)
        //    (假设上拉电阻为 10kΩ = 10000.0f)
        adc_r = 10000.0f * ((float)adc_ave / (4095.0f - (float)adc_ave));
    }

    // 查表/换算光照强度
    light = GetLux(adc_r);
    
    return light;
}
