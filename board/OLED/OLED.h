#ifndef __OLED_H__
#define __OLED_H__

#include "stm32f4xx.h"
#include "oled_font.h"
#include "DELAY.h"
#include "IIC.h"

// 引脚宏定义 (请根据实际情况确认 GPIOB 和 Pin8/Pin9)
#define OLED_SCL_High() GPIO_SetBits(GPIOB, GPIO_Pin_8)
#define OLED_SCL_Low()  GPIO_ResetBits(GPIOB, GPIO_Pin_8)
#define OLED_SDA_High() GPIO_SetBits(GPIOB, GPIO_Pin_9)
#define OLED_SDA_Low()  GPIO_ResetBits(GPIOB, GPIO_Pin_9)

void OLED_Init(void);
void OLED_Clear_Gram(void);
void OLED_Refresh_Gram(void);
void OLED_SetPos(uint8_t x, uint8_t y);
void OLED_ShowStr_8x8(uint8_t x, uint8_t page, const char *str);
void OLED_ShowStr_5x8(uint8_t x, uint8_t page, const char *str);
void OLED_ClearArea_Gram(uint8_t start_x, uint8_t end_x, uint8_t start_page, uint8_t end_page);
void OLED_Display_Off(void);
void OLED_Display_On(void);

#endif
