#include "RGB.h"

/**
  * @brief  初始化 TIM1 CH1(PE9)、CH2(PE11)、CH3(PE13) 为 PWM 输出
  */
void RGB_config(void) {
    // 1. 使能 GPIOE 和 TIM1 时钟
    RCC_AHB1PeriphClockCmd(RGB_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

    // 2. 配置 PE9, PE11, PE13 为复用功能
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = RGB_R_PIN | RGB_G_PIN | RGB_B_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        // 复用模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     // 推挽输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(RGB_PORT, &GPIO_InitStructure);

    // 3. 引脚复用映射至 TIM1
    GPIO_PinAFConfig(RGB_PORT, GPIO_PinSource9, GPIO_AF_TIM1);  // CH1 - Red
    GPIO_PinAFConfig(RGB_PORT, GPIO_PinSource11, GPIO_AF_TIM1); // CH2 - Green
    GPIO_PinAFConfig(RGB_PORT, GPIO_PinSource13, GPIO_AF_TIM1); // CH3 - Blue

    // 4. TIM1 基础时钟配置
    // TIM1 在 APB2 总线上 (84MHz / 168MHz)，这里配置 PWM 频率为 1kHz，最大占空比分辨率为 255 (对应 0~255 RGB 级)
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period = 255 - 1;          // ARR 重装载值 (0~255)
    TIM_TimeBaseStructure.TIM_Prescaler = 660 - 1;       // PSC 预分频
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    // 5. 配置 TIM1 PWM 输出通道
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;             // PWM 模式 1
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // 使能主输出
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;     // 高电平有效 (共阴极)
                                                                  // 若为共阳极RGB灯，改为 TIM_OCPolarity_Low
    TIM_OCInitStructure.TIM_Pulse = 0; // 默认占空比 0

    // 初始化 CH1, CH2, CH3
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);
    TIM_OC2Init(TIM1, &TIM_OCInitStructure);
    TIM_OC3Init(TIM1, &TIM_OCInitStructure);

    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);

    // 6. TIM1 为高级定时器，必须开启 MOE 主输出使能！
    TIM_CtrlPWMOutputs(TIM1, ENABLE);

    // 7. 启动 TIM1 定时器
    TIM_Cmd(TIM1, ENABLE);

    // 默认关闭灯光
    RGB_SetColor(0, 0, 0);
}

/**
  * @brief  设置 RGB 三色灯颜色
  * @param  r: 红色亮度 (0 ~ 255)
  * @param  g: 绿色亮度 (0 ~ 255)
  * @param  b: 蓝色亮度 (0 ~ 255)
  */
void RGB_SetColor(uint8_t r, uint8_t g, uint8_t b) {
    TIM_SetCompare1(TIM1, r); // CH1 控制 R
    TIM_SetCompare2(TIM1, g); // CH2 控制 G
    TIM_SetCompare3(TIM1, b); // CH3 控制 B
}
