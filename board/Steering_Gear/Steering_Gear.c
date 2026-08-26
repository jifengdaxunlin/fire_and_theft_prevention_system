#include "Steering_Gear.h"
//#include "FreeRTOS.h"
//#include "task.h"

void SG_config(void){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);	//使能GPIO时钟
	GPIO_InitTypeDef mygpio = {0};
	mygpio.GPIO_Mode = GPIO_Mode_AF;	//工作模式: 复用
	mygpio.GPIO_OType = GPIO_OType_PP;
	mygpio.GPIO_Pin = SG_PIN;
	mygpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
	mygpio.GPIO_Speed = GPIO_High_Speed;
	GPIO_Init(SG_GROUP,&mygpio);
	
	//把 PA9 复用成 TIM3
	GPIO_PinAFConfig(SG_GROUP, GPIO_PinSource9, GPIO_AF_TIM3);
	
	//T(0.02) = (ARR + 1)(PSC + 1)/84 000 000
	TIM_TimeBaseInitTypeDef mytim = {0};
	mytim.TIM_ClockDivision = TIM_CKD_DIV1;	//预分频,滤波(这里选不分频)
	mytim.TIM_CounterMode = TIM_CounterMode_Up;	//向上计数 0++
	mytim.TIM_Period = 20000 - 1;	//0 ~ 65535  ARR 数多少个数 (自动重装载寄存器)
	mytim.TIM_Prescaler = 84 - 1;//0 ~ 65535  PSC 计数的快慢(分频值)
	TIM_TimeBaseInit(TIM3, &mytim);
	
	
	TIM_OCInitTypeDef mysg = {0};
	mysg.TIM_OCMode = TIM_OCMode_PWM1;	//选择PWM模式1
	mysg.TIM_OutputState = TIM_OutputState_Enable;	//输出使能
	mysg.TIM_Pulse = 0;	//CCR 默认为 0 ,不响
	mysg.TIM_OCPolarity = TIM_OCPolarity_High;	//高电平有效
	TIM_OC4Init(TIM3, &mysg);	//OC1:通道一
	
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);	//开启预装载配置 CCR
	TIM_ARRPreloadConfig(TIM3, ENABLE);	//开启预装载配置 CCR
	
	TIM_Cmd(TIM3, ENABLE);
}

static uint16_t last_sg_angle = 0xFFFF; // 记录上一次的角度

void SG_turn(uint8_t angle)
{
    if (angle > 180) angle = 180;
    
    // 只有当角度发生变化时，才重新设置定时器比较值，避免 PWM 频繁受到干扰
    if (last_sg_angle != angle) {
        last_sg_angle = angle;
        uint32_t pwm_val = 500 + ((uint32_t)2000 * angle) / 180;
        TIM_SetCompare4(TIM3, pwm_val);
    }
}
