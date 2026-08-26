#ifndef __DHT11_H__
#define __DHT11_H__

#include "stm32f4xx.h"
#include "DELAY.h"
#include <stdio.h>
#include <stdbool.h>

#define DHT11_CLK	RCC_AHB1Periph_GPIOB
#define DHT11_PIN	GPIO_Pin_13
#define DHT11_GROUP	GPIOB

typedef struct{
    float humi; // ʪ����ֵ (%)
    float temp; // �¶���ֵ (��)
}humi_temp;

uint8_t DHT11_Get(void);
uint8_t DHT11_Read(void);
uint8_t DHT11_Back(void);
void DHT11_Set(uint8_t n);
void DHT11_Output_Mode(void);
void DHT11_Input_Mode(void);
humi_temp DHT11_Read_Data(void);

#endif
