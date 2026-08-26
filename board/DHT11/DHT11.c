#include "DHT11.h"
#include "FreeRTOS.h"
#include "task.h"

// 精确微秒延时：使用内核周期计数器 DWT->CYCCNT，精度到周期级。
// 优点：任务内/临界区内均安全，不依赖 SysTick（不与 FreeRTOS tick 冲突）。
// 要求 SystemCoreClock 正确（本项目 = 168MHz，1us = 168 个时钟周期）。
static void DHT11_DelayUs(uint32_t us) {
    // 首次调用时使能 DWT 周期计数器（TRCENA 必须先置位）
    if ((DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) == 0U) {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0U;
        DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
    }

    uint32_t start  = DWT->CYCCNT;
    uint32_t ticks  = us * (SystemCoreClock / 1000000U); // 每 us 的周期数
    while ((DWT->CYCCNT - start) < ticks) {
        // 忙等
    }
}


// PG9 两个函数：输入 / 输出
void DHT11_Output_Mode(void){
    RCC_AHB1PeriphClockCmd(DHT11_CLK, ENABLE);
    GPIO_InitTypeDef mygpio;
    mygpio.GPIO_Mode = GPIO_Mode_OUT;
    mygpio.GPIO_OType = GPIO_OType_PP;
    mygpio.GPIO_Pin = DHT11_PIN;
    mygpio.GPIO_PuPd = GPIO_PuPd_UP;
    mygpio.GPIO_Speed = GPIO_High_Speed;
    GPIO_Init(DHT11_GROUP, &mygpio);
}

void DHT11_Input_Mode(void){
    GPIO_InitTypeDef mygpio;
    mygpio.GPIO_Mode = GPIO_Mode_IN;
    mygpio.GPIO_Pin = DHT11_PIN;
    mygpio.GPIO_PuPd = GPIO_PuPd_NOPULL; // 完全让 DHT11 总线控制电平
    GPIO_Init(DHT11_GROUP, &mygpio);
}

void DHT11_Set(uint8_t n){
    if(n == 1) GPIO_SetBits(DHT11_GROUP, DHT11_PIN);
    else GPIO_ResetBits(DHT11_GROUP, DHT11_PIN);
}

uint8_t DHT11_Read(void){
    return GPIO_ReadInputDataBit(DHT11_GROUP, DHT11_PIN);
}

// DHT11 的开始工作信号
// DHT11_Start 保持原样，使用修正后的 DelayUs
void DHT11_Start(void){
    DHT11_Output_Mode();
    DHT11_Set(0);
    DHT11_DelayUs(18000);  // 严谨拉低 18ms
    DHT11_Set(1);
    DHT11_DelayUs(30);     // 等待 30us
}

// DHT11 应答主机的信号
uint8_t DHT11_Back(void){
    uint16_t count = 0;
    DHT11_Input_Mode(); // 切成输入模式
    
    // 等待低电平响应
    while(DHT11_Read() == 1) {
        count++;
        DHT11_DelayUs(1);
        if(count > 100) return 3; // 【修改】超时直接返回，去掉 printf
    }

    count = 0;
    // 等待高电平，计算低电平时间
    while(DHT11_Read() == 0){
        count++;
        DHT11_DelayUs(1);
        if(count > 100) return 1; // 超时退出
    }

    count = 0;
    while(DHT11_Read() == 1){
        count++;
        DHT11_DelayUs(1);
        if(count > 100) return 2; // 超时退出
    }
    return 0; // 成功响应
}

uint8_t DHT11_Get(void){
    uint8_t temp = 0;
    uint8_t timeout = 0;

    for(uint8_t i = 0; i < 8; i++){
        timeout = 0;
        while (DHT11_Read() == 0) {
            timeout++;
            DHT11_DelayUs(1);
            if (timeout > 100) return 0; // 超时退出
        }

        DHT11_DelayUs(30);

        if(DHT11_Read() == 1){
            timeout = 0;
            while (DHT11_Read() == 1) {
                timeout++;
                DHT11_DelayUs(1);
                if (timeout > 100) break;
            }
            temp |= (0x01 << (7 - i));
        } else {
            temp &= ~(0x01 << (7 - i));
        }
    }
    return temp;
}

uint8_t humi_int = 0;
uint8_t humi_deci = 0;
uint8_t temp_int = 0;
uint8_t temp_deci = 0;
uint8_t check_sum = 0;

// DHT11 读取主函数
humi_temp DHT11_Read_Data(void){
    humi_temp ht = {0};
    uint8_t h_i = 0, h_d = 0, t_i = 0, t_d = 0, chk = 0;

    // DHT11 两次读取间隔必须 >= 1s，否则读到旧数据甚至响应失败
    static TickType_t last_read_tick = 0;
    static bool first_read = true;
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        if (!first_read && (xTaskGetTickCount() - last_read_tick) < pdMS_TO_TICKS(1000)) {
            return ht; // 间隔不足，跳过本次读取
        }
        first_read = false;
        last_read_tick = xTaskGetTickCount();
    }

    // 起始信号（拉低 18ms + 拉高 30us）时序不敏感，放在临界区外，
    // 避免长时间关闭中断导致 USART 丢字节 / FreeRTOS tick 丢失
    DHT11_Start();

    // 进入临界区：只保护对微秒级时序敏感的应答 + 40bit 读取（约 4ms）
    taskENTER_CRITICAL();

    if(DHT11_Back() == 0){
        h_i = DHT11_Get();
        h_d = DHT11_Get();
        t_i = DHT11_Get();
        t_d = DHT11_Get();
        chk = DHT11_Get();

        // DHT11 校验和 = 前 4 字节之和的低 8 位
        if(chk == (uint8_t)(h_i + h_d + t_i + t_d)){
            ht.humi = h_i + h_d / 100.0f;
            ht.temp = t_i + t_d / 100.0f;
        }
    }

    // 读完把总线拉高，交给上拉电阻维持空闲电平
    DHT11_Output_Mode();
    DHT11_Set(1);

    taskEXIT_CRITICAL();

    return ht;
}
