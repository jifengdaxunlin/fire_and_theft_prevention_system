#include "STIM1.h"

void TIM1_config(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);	//使能GPIO时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	GPIO_InitTypeDef mygpio = {0};
	mygpio.GPIO_Mode = GPIO_Mode_AF;	//工作模式: 复用
	mygpio.GPIO_OType = GPIO_OType_PP;
	mygpio.GPIO_Pin = TIM1_PIN;
	mygpio.GPIO_PuPd = GPIO_PuPd_UP;
	mygpio.GPIO_Speed = GPIO_High_Speed;
	GPIO_Init(TIM1_GROUP,&mygpio);
	
	mygpio.GPIO_Pin = TIM1N_PIN;
	GPIO_Init(TIM1N_GROUP,&mygpio);
	
	//把 PA8 和 PB13 复用成 TIM1
	GPIO_PinAFConfig(TIM1_GROUP, GPIO_PinSource8, GPIO_AF_TIM1);
	GPIO_PinAFConfig(TIM1N_GROUP, GPIO_PinSource13, GPIO_AF_TIM1);
	
	//T = (ARR + 1)(PSC + 1)/84 000 000
	TIM_TimeBaseInitTypeDef mytim = {0};
	mytim.TIM_ClockDivision = TIM_CKD_DIV1;	//预分频,滤波(这里选不分频)
	mytim.TIM_CounterMode = TIM_CounterMode_Up;	//向上计数 0++
	mytim.TIM_Period = 1000 - 1;	//0 ~ 65535  ARR 数多少个数 (自动重装载寄存器)
	mytim.TIM_Prescaler = 840 - 1;//0 ~ 65535  PSC 计数的快慢(分频值)
	TIM_TimeBaseInit(TIM2, &mytim);
	
	TIM_OCInitTypeDef myoc = {0};
	myoc.TIM_OCIdleState = TIM_OCNIdleState_Set;	//空闲时间低电平
	myoc.TIM_OCMode = TIM_OCMode_PWM1;	//选择PWM模式1
	myoc.TIM_OCNIdleState = TIM_OCNIdleState_Reset;	//空闲时间高电平
	myoc.TIM_OCNPolarity = TIM_OCPolarity_High;
	myoc.TIM_OCPolarity = TIM_OCPolarity_High;	//高电平有效
	myoc.TIM_OutputNState = TIM_OutputState_Enable;
	myoc.TIM_OutputState = TIM_OutputState_Enable;	//输出使能
	myoc.TIM_Pulse = 0;	//CCR 默认为 0 ,不响
	TIM_OC1Init(TIM2, &myoc);	//OC1:通道一
	
	TIM_BDTRInitTypeDef mybdt = {0};
	mybdt.TIM_AutomaticOutput = TIM_AutomaticOutput_Disable;	//自动输出：刹车结束后是否自动输出pwm
	mybdt.TIM_Break = TIM_Break_Enable;	//是否开启刹车(BKIN引脚控制)
	mybdt.TIM_BreakPolarity = TIM_BreakPolarity_Low;	//低电平刹车
	mybdt.TIM_DeadTime = 0x03;	//死区时间
	mybdt.TIM_LOCKLevel = TIM_LOCKLevel_OFF;
	/*
	定时器运行后
	#define TIM_LOCKLevel_OFF	不开启
	#define TIM_LOCKLevel_1		不能修改OSSR OSSI BRK刹车极性 只能修改死区时间
	#define TIM_LOCKLevel_2		不能修改OSSR OSSI BRK刹车极性 死区时间
	#define TIM_LOCKLevel_3		所有寄存器都锁住
	*/
	mybdt.TIM_OSSIState = TIM_OSSIState_Disable;	//空闲时 pwm 要不要取 64 和 65 行的状态
	mybdt.TIM_OSSRState = TIM_OSSRState_Enable;	//特殊原因关闭pwm后，依旧和另一条线波形互补，Disable则引脚状态随机
	TIM_BDTRConfig(TIM1, &mybdt);
	
	TIM_Cmd(TIM1, ENABLE);	//使能
	TIM_CtrlPWMOutputs(TIM1, ENABLE);	//开始输出pwm
}
