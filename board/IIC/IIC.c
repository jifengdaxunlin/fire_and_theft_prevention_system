#include "IIC.h"

SemaphoreHandle_t xI2C_Mutex = NULL;

// DWT 微秒级延时：精度高、主频无关、多任务安全
static void IIC_DelayUs(uint32_t us) {
    // 1. 检查 DWT 周期计数器（CYCCNT）是否已经使能
    // DWT->CTRL 是控制寄存器，DWT_CTRL_CYCCNTENA_Msk 是使能 CYCCNT 的比特位掩码
    if ((DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) == 0U) {
        
        // 2. 开启 TRCENA（Trace Component Enable，跟踪组件使能）
        // CoreDebug->DEMCR 是核心调试寄存器，必须先置位 TRCENA，DWT 相关的寄存器才能正常工作
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        
        // 3. 清空 DWT 的 32 位循环计数器（CYCCNT）
        DWT->CYCCNT = 0U;
        
        // 4. 正式使能 DWT 周期计数器，让它随 CPU 主频自动每个时钟周期 +1
        DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
    }

    // 5. 记录当前时刻的时钟周期数（起始基准点）
    uint32_t start = DWT->CYCCNT;
    
    // 6. 计算目标微秒数（us）需要经历多少个硬件时钟周期
    // SystemCoreClock 是系统主频（例如 168MHz），SystemCoreClock / 1000000U 代表 1 微秒包含多少个周期
    uint32_t ticks = us * (SystemCoreClock / 1000000U); 
    
    // 7. 循环等待：只要“当前计数器值 - 起始值”小于目标周期数，就一直忙等
    while ((DWT->CYCCNT - start) < ticks) {
        // 忙等（空循环，占用 CPU 时间片）
    }
}
void IIC_config(void)
{
    RCC_AHB1PeriphClockCmd(SCL_CLK | SDA_CLK, ENABLE);

    GPIO_InitTypeDef mygpio;
    mygpio.GPIO_Mode  = GPIO_Mode_OUT;
    mygpio.GPIO_OType = GPIO_OType_OD;  // 开漏输出
    mygpio.GPIO_PuPd  = GPIO_PuPd_UP;    // 内部上拉
    mygpio.GPIO_Speed = GPIO_High_Speed;

    mygpio.GPIO_Pin = SCL_PIN;
    GPIO_Init(SCL_GROUP, &mygpio);

    mygpio.GPIO_Pin = SDA_PIN;
    GPIO_Init(SDA_GROUP, &mygpio);

    IIC_SCL_SET(1);
    IIC_SDA_SET(1);

    if (xI2C_Mutex == NULL) {
        xI2C_Mutex = xSemaphoreCreateMutex();
    }
}

void IIC_Start(void)
{
    IIC_SDA_SET(1);
    IIC_SCL_SET(1);
    IIC_DelayUs(2);
    IIC_SDA_SET(0);
    IIC_DelayUs(4);
    IIC_SCL_SET(0);
    IIC_DelayUs(4);
}

void IIC_Stop(void)
{
    IIC_SDA_SET(0);
    IIC_SCL_SET(0);
    IIC_DelayUs(4);
    IIC_SCL_SET(1);
    IIC_DelayUs(4);
    IIC_SDA_SET(1);
    IIC_DelayUs(4);
}

/* 移除 taskENTER_CRITICAL()，由互斥锁保障并发安全 */
void IIC_SendByte(uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++) {
        IIC_SCL_SET(0);
        IIC_DelayUs(2);
        IIC_SDA_SET((data & 0x80) ? 1 : 0);
        data <<= 1;
        IIC_DelayUs(2);
        IIC_SCL_SET(1);
        IIC_DelayUs(4);
    }
    IIC_SCL_SET(0);
    IIC_DelayUs(2);
}

uint8_t IIC_ReadByte(void)
{
    uint8_t data = 0;
    IIC_SDA_SET(1); // 释放 SDA

    for (uint8_t i = 0; i < 8; i++) {
        IIC_SCL_SET(0);
        IIC_DelayUs(4);
        IIC_SCL_SET(1);
        IIC_DelayUs(2);
        data <<= 1;
        if (IIC_SDA_READ) {
            data |= 0x01;
        }
        IIC_DelayUs(2);
    }
    IIC_SCL_SET(0);
    IIC_DelayUs(2);
    return data;
}

uint8_t IIC_SlaveAck(void)
{
    uint8_t ack = 0;
    IIC_SDA_SET(1);
    IIC_SCL_SET(0);
    IIC_DelayUs(4);
    IIC_SCL_SET(1);
    IIC_DelayUs(4);
    ack = IIC_SDA_READ ? 1 : 0;
    IIC_SCL_SET(0);
    IIC_DelayUs(4);
    return ack;
}

void IIC_MasterAck(uint8_t ack)
{
    IIC_SCL_SET(0);
    IIC_DelayUs(2);
    IIC_SDA_SET(ack ? 1 : 0);
    IIC_DelayUs(2);
    IIC_SCL_SET(1);
    IIC_DelayUs(4);
    IIC_SCL_SET(0);
    IIC_DelayUs(2);
}
