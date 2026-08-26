#include "FAN.h"

void FAN_Config(void){
	RCC_AHB1PeriphClockCmd(FAN_CLK, ENABLE);
	GPIO_InitTypeDef mygpio;
	mygpio.GPIO_Mode = GPIO_Mode_AF;
	mygpio.GPIO_OType = GPIO_OType_PP;
	mygpio.GPIO_Pin = FAN_A_PIN | FAN_B_PIN;
	mygpio.GPIO_PuPd = GPIO_PuPd_DOWN;
	mygpio.GPIO_Speed = GPIO_High_Speed;
	GPIO_Init(FAN_GROUP,&mygpio);
	
	GPIO_PinAFConfig(FAN_GROUP, GPIO_PinSource6, GPIO_AF_TIM3);
	GPIO_PinAFConfig(FAN_GROUP, GPIO_PinSource7, GPIO_AF_TIM3);
	
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
	TIM_OC1Init(TIM3, &mysg);	//OC1:通道一
	TIM_OC2Init(TIM3, &mysg);	//OC2:通道二
	
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);	//开启预装载配置 CCR
	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
	TIM_ARRPreloadConfig(TIM3, ENABLE);	//开启预装载配置 CCR
	
	TIM_Cmd(TIM3, ENABLE);
}

void FAN_turn_right(uint8_t speed){
	if(speed > 100) speed = 100;
	TIM_SetCompare1(TIM3, 20000 * speed / 100);
	TIM_SetCompare2(TIM3, 0);
}

void FAN_turn_left(uint8_t speed){
	if(speed > 100) speed = 100;
	TIM_SetCompare1(TIM3, 0);
	TIM_SetCompare2(TIM3, 20000 * speed / 100);
}

void FAN_stop(void){
	TIM_SetCompare1(TIM3, 0);
	TIM_SetCompare2(TIM3, 0);
}

void FAN_brake(void){
	TIM_SetCompare1(TIM3, 20000);
	TIM_SetCompare2(TIM3, 20000);
}
