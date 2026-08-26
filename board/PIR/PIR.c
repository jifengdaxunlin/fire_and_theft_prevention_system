#include "PIR.h"

void PIR_config(void){
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	GPIO_InitTypeDef mygpio;
	mygpio.GPIO_Mode = GPIO_Mode_IN;
	mygpio.GPIO_OType = GPIO_OType_PP;
	mygpio.GPIO_Pin = PIR_PIN;
	mygpio.GPIO_PuPd = GPIO_PuPd_DOWN;
	mygpio.GPIO_Speed = GPIO_High_Speed;
	GPIO_Init(PIR_GPIO,&mygpio);
}
