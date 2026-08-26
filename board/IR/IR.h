#ifndef __IR_H__
#define __IR_H__

#include "DELAY.h"
#include <stdio.h>
#include "stm32f4xx.h"

#define IR_GROUP	GPIOA
#define IR_PIN		GPIO_Pin_2

void IR_config(void);
uint16_t IR_Get_Data(void);
float IR_Get_Ave(uint8_t times);
float IR_Get_Fire(uint8_t times);

#endif
