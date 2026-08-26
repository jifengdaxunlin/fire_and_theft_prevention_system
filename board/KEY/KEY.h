#ifndef __KEY_H__
#define __KEY_H__

#include "stm32f4xx.h"
#include "LED.h"
#include "DELAY.h"
#include <stdbool.h>

extern bool children_lock;

void KEY_config(void); 
uint16_t key_secret(void);

#define KEY0_pin GPIO_Pin_7
#define KEY1_pin GPIO_Pin_8
#define KEY2_pin GPIO_Pin_10
#define KEY3_pin GPIO_Pin_12

#define KEY_PORT GPIOE
#define KEY_CLK RCC_AHB1Periph_GPIOE

#define KEY0_READ GPIO_ReadInputDataBit(KEY_PORT,KEY0_pin) == 0
#define KEY1_READ GPIO_ReadInputDataBit(KEY_PORT,KEY1_pin) == 0
#define KEY2_READ GPIO_ReadInputDataBit(KEY_PORT,KEY2_pin) == 0
#define KEY3_READ GPIO_ReadInputDataBit(KEY_PORT,KEY3_pin) == 0

#endif
