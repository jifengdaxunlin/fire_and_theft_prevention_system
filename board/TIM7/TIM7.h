#ifndef __TIM_H__
#define __TIM_H__

#include "KEY.h"
#include "EXTI.h"
#include "stm32f4xx.h"
#include "OLED.h"

extern bool Manual_Auto, Set_Threshold,
	 key2_short,
	 key3_led_beep, key3_fan,
	 key4_water, key4_gate,
	 oled_clear, subscribe, read_data;

extern bool *Back, add, dec, save;

extern uint8_t threshold_item;

void TIM7_config(void);

#endif
