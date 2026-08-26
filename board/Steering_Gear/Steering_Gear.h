#ifndef __STEERING_GEAR_H__
#define __STEERING_GEAR_H__

#include "stm32f4xx.h"
#include "DELAY.h"

#define SG_GROUP GPIOC
#define SG_PIN GPIO_Pin_9	//D3

void SG_config(void);
void SG_turn(uint8_t angle);

#endif
