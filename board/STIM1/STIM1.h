#ifndef __STIM1_H__
#define __STIM1_H__

#include "stm32f4xx.h"
#include <stdbool.h>

#define TIM1_GROUP GPIOA
#define TIM1N_GROUP GPIOB
#define TIM1_PIN GPIO_Pin_8
#define TIM1N_PIN GPIO_Pin_13
void TIM1_config(void);

#endif
