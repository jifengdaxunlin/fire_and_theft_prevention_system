#ifndef __PIR_H__
#define __PIR_H__

#include "stm32f4xx.h"

void PIR_config(void);

#define PIR_GPIO	GPIOE
#define PIR_PIN		GPIO_Pin_6
#define PIR_READ GPIO_ReadInputDataBit(PIR_GPIO,PIR_PIN)

#endif
