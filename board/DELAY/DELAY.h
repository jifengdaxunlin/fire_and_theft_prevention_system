#ifndef __DELAY_H__
#define __DELAY_H__

#include "stm32f4xx.h"

void delay(volatile uint32_t cnt);
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);
void delay_s(uint32_t s);

#endif
