#ifndef __RGB_H
#define __RGB_H

#include "stm32f4xx.h"

// 引脚宏定义
#define RGB_R_PIN       GPIO_Pin_9		//D6
#define RGB_G_PIN       GPIO_Pin_11		//D8
#define RGB_B_PIN       GPIO_Pin_13		//D10
#define RGB_PORT        GPIOE
#define RGB_CLK         RCC_AHB1Periph_GPIOE

// 函数声明
void RGB_config(void);
void RGB_SetColor(uint8_t r, uint8_t g, uint8_t b);

#endif
