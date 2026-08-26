#ifndef __FAN_H__
#define __FAN_H__

#include "stm32f4xx.h"

void KEY_config(void); 
uint16_t key_secret(void);

#define FAN_A_PIN GPIO_Pin_6	//D0
#define FAN_B_PIN GPIO_Pin_7	//D1

#define FAN_GROUP GPIOC
#define FAN_CLK RCC_AHB1Periph_GPIOC

void FAN_Config(void);
void FAN_turn_right(uint8_t speed);
void FAN_turn_left(uint8_t speed);
void FAN_stop(void);
void FAN_brake(void);

#endif
