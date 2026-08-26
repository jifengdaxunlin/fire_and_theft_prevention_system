#ifndef __EXTI_H__
#define __EXTI_H__

#include "stm32f4xx.h"
#include "KEY.h"

extern uint8_t key_flag;
void EXTI_config(void); 

#endif
