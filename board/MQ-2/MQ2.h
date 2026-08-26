#ifndef __MQ2_H__
#define __MQ2_H__

#include "DELAY.h"
#include <stdio.h>
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdlib.h>
#include <stdbool.h>

#define MQ2_GROUP	GPIOA
#define MQ2_PIN		GPIO_Pin_3

void MQ2_config(void);
uint16_t MQ2_Get_Data(void);
float MQ2_Get_Ave(uint8_t times);
float MQ2_Get_Per(uint8_t times);

#endif
