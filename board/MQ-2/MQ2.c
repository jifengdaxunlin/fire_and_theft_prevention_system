#include "MQ2.h"

// 1. 定义 ADC1 硬件互斥锁
SemaphoreHandle_t xADC1_Mutex = NULL;

// 静态全局变量：用于保存上一次滤波后的稳定值（实现一阶惯性滤波）
static float s_filtered_percentage = 0.0f;
static bool  s_is_first_sample     = true;

/**
 * @brief  MQ-2 气体传感器 GPIO 与 ADC1 初始化
 */
void MQ2_config(void) 
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    // GPIOA Pin3 初始化为模拟输入
    GPIO_InitTypeDef mygpio;
    mygpio.GPIO_Mode  = GPIO_Mode_AN;
    mygpio.GPIO_Pin   = MQ2_PIN;
    mygpio.GPIO_PuPd  = GPIO_PuPd_NOPULL; // 注：若引脚常年悬空测试，可临时改为 GPIO_PuPd_DOWN 压低到地
    mygpio.GPIO_Speed = GPIO_High_Speed;
    GPIO_Init(MQ2_GROUP, &mygpio);

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
    
    s_is_first_sample = true;
}

/**
 * @brief  获取单次 MQ-2 ADC 采样值（带互斥锁保护）
 */
uint16_t MQ2_Get_RawData(void) 
{
    uint16_t adc_val = 0;
    uint16_t timeout = 5000;

    if (xSemaphoreTake(xADC1_Mutex, pdMS_TO_TICKS(20)) == pdTRUE) 
    {
        ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 1, ADC_SampleTime_480Cycles);
        ADC_SoftwareStartConv(ADC1);

        while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) != SET) {
            timeout--;
            if (timeout == 0) {
                xSemaphoreGive(xADC1_Mutex);
                return 0; 
            }
        }
        
        adc_val = ADC_GetConversionValue(ADC1);
        xSemaphoreGive(xADC1_Mutex);
    }

    return adc_val;
}

static int Compare_Uint16(const void *a, const void *b) 
{
    return (*(uint16_t *)a - *(uint16_t *)b);
}

/**
 * @brief  中值平均滤波采样
 */
uint16_t MQ2_Get_Filtered_Value(uint8_t samples) 
{
    if (samples < 3) samples = 3; 
    uint16_t buffer[20];
    if (samples > 20) samples = 20;

    for (uint8_t i = 0; i < samples; i++) {
        buffer[i] = MQ2_Get_RawData();
        
        if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
            vTaskDelay(pdMS_TO_TICKS(2));
        } else {
            delay_us(500);
        }
    }

    qsort(buffer, samples, sizeof(uint16_t), Compare_Uint16);

    uint32_t sum = 0;
    for (uint8_t i = 1; i < (samples - 1); i++) {
        sum += buffer[i];
    }

    return (uint16_t)(sum / (samples - 2));
}

/**
 * @brief  计算气体相对百分比 (加入一阶低通滤波与突变抑制)
 */
float MQ2_Get_Per(uint8_t times)
{
    uint16_t filtered_adc = MQ2_Get_Filtered_Value(times);
    if (filtered_adc > 4095) filtered_adc = 4095;

    // 当前样值的原始百分比
    float current_percentage = ((float)filtered_adc / 4095.0f) * 100.0f;

    // 如果是第一次采样，直接赋予初值
    if (s_is_first_sample) {
        s_filtered_percentage = current_percentage;
        s_is_first_sample = false;
        return s_filtered_percentage;
    }

    // --- 核心优化：突变抑制与一阶惯性滤波 ---
    // 1. 突变限幅：如果单次跳变幅度超过 15.0%，认为是悬空干扰毛刺，进行大幅度衰减或直接过滤
    float delta = current_percentage - s_filtered_percentage;
    if (delta > 15.0f || delta < -15.0f) {
        // 遇到剧烈跳变时，限制每周期最大变化量不超过 3.0%，过滤悬空毛刺
        if (delta > 0) current_percentage = s_filtered_percentage + 3.0f;
        else           current_percentage = s_filtered_percentage - 3.0f;
    }

    // 2. 一阶低通滤波公式: Y(n) = a * X(n) + (1 - a) * Y(n-1)
    // alpha 取 0.3，数值越小越平滑，抗干扰能力越强
    float alpha = 0.3f; 
    s_filtered_percentage = alpha * current_percentage + (1.0f - alpha) * s_filtered_percentage;

    return s_filtered_percentage;
}
