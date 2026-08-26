#ifndef __ADC_H__
#define __ADC_H__

#include "DELAY.h"
#include <stdio.h>
#include "stm32f4xx.h"
#include "light_adc.h"

#define ADC_GROUP	GPIOF
#define ADC_PIN		GPIO_Pin_7

void ADC_config(void);
uint16_t ADC_Get_Data(void);
float ADC_Get_Ave(uint8_t times);
uint16_t ADC_Get_Light(uint8_t times);

#endif
