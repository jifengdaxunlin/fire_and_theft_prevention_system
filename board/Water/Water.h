#ifndef __WATER_H__
#define __WATER_H__

#include "stm32f4xx.h"
#include "DELAY.h"

#define WATER_GROUP GPIOC
#define WATER_PIN GPIO_Pin_8	//D2

void Water_config(void);
void Water_ration(int16_t speed);

#endif
