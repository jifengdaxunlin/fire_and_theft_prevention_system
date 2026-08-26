#include "EXTI.h"

void EXTI_config(void){
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
	GPIO_InitTypeDef mygpio;
	mygpio.GPIO_Mode = GPIO_Mode_IN;
	mygpio.GPIO_OType = GPIO_OType_PP;
	mygpio.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
	mygpio.GPIO_PuPd = GPIO_PuPd_UP;
	mygpio.GPIO_Speed = GPIO_High_Speed;
	GPIO_Init(GPIOG,&mygpio);
	
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG,EXTI_PinSource2);	//把引脚和中断线相连
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG,EXTI_PinSource3);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG,EXTI_PinSource4);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG,EXTI_PinSource5);
	
	EXTI_InitTypeDef myexti = {0};
	myexti.EXTI_Line = EXTI_Line2 | EXTI_Line3 | EXTI_Line4 | EXTI_Line5; //哪个中断线
	myexti.EXTI_LineCmd = ENABLE;  //使能
	myexti.EXTI_Mode = EXTI_Mode_Interrupt;  //工作模式  中断/事件
	myexti.EXTI_Trigger = EXTI_Trigger_Falling; //触发条件: 下降沿
	EXTI_Init(&myexti);
	
	NVIC_InitTypeDef mynvic = {0};
	mynvic.NVIC_IRQChannel = EXTI2_IRQn; //中断通道
	mynvic.NVIC_IRQChannelCmd = ENABLE; //使能
	mynvic.NVIC_IRQChannelPreemptionPriority = 6;  //抢占优先级 0~15
	mynvic.NVIC_IRQChannelSubPriority = 0;		//响应优先级 0~15
	NVIC_Init(&mynvic);
	
	mynvic.NVIC_IRQChannel = EXTI3_IRQn; //中断通道
	mynvic.NVIC_IRQChannelCmd = ENABLE; //使能
	mynvic.NVIC_IRQChannelPreemptionPriority = 6;  //抢占优先级 0~15
	mynvic.NVIC_IRQChannelSubPriority = 0;		//响应优先级 0~15
	NVIC_Init(&mynvic);
	
	mynvic.NVIC_IRQChannel = EXTI4_IRQn; //中断通道
	mynvic.NVIC_IRQChannelCmd = ENABLE; //使能
	mynvic.NVIC_IRQChannelPreemptionPriority = 6;  //抢占优先级 0~15
	mynvic.NVIC_IRQChannelSubPriority = 0;		//响应优先级 0~15
	NVIC_Init(&mynvic);
	
	mynvic.NVIC_IRQChannel = EXTI9_5_IRQn; //中断通道
	mynvic.NVIC_IRQChannelCmd = ENABLE; //使能
	mynvic.NVIC_IRQChannelPreemptionPriority = 6;  //抢占优先级 0~15
	mynvic.NVIC_IRQChannelSubPriority = 0;		//响应优先级 0~15
	NVIC_Init(&mynvic);
}

uint8_t key_flag = 0;

//中断函数  触发中断的时候自动调用  不手动清除,会一直执行
void EXTI2_IRQHandler(void){
	// 1. 先检查并清除中断标志位
	if(EXTI_GetITStatus(EXTI_Line2) == SET) { //判断进来的是不是 2 号的中断
        // 2. 执行快速操作（如直接翻转）
		key_flag = key_flag == 1 ? 0 : 1;// 3. 清除挂起位
        EXTI_ClearITPendingBit(EXTI_Line2);
		//}
    }
}

void EXTI3_IRQHandler(void){
	if(EXTI_GetITStatus(EXTI_Line3) == SET) {;
		key_flag = key_flag == 2 ? 0 : 2;
        EXTI_ClearITPendingBit(EXTI_Line3);
    }
}

void EXTI4_IRQHandler(void){
	if(EXTI_GetITStatus(EXTI_Line4) == SET) {
		key_flag = key_flag == 3 ? 0 : 3;
        EXTI_ClearITPendingBit(EXTI_Line4);
    }
}

void EXTI9_5_IRQHandler(void){
	if(EXTI_GetITStatus(EXTI_Line5) == SET) {
		if(KEY0_READ);
        EXTI_ClearITPendingBit(EXTI_Line5);
    }
}
