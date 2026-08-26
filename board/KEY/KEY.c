#include "KEY.h"
//1. 按键 PG2~PG5 3.按下按键为低电平

bool children_lock = true;

void KEY_config(void){
	RCC_AHB1PeriphClockCmd(KEY_CLK, ENABLE);
	GPIO_InitTypeDef mygpio;
	mygpio.GPIO_Mode = GPIO_Mode_IN;
	mygpio.GPIO_OType = GPIO_OType_PP;
	mygpio.GPIO_Pin = KEY0_pin | KEY1_pin | KEY2_pin | KEY3_pin;
	mygpio.GPIO_PuPd = GPIO_PuPd_UP;
	mygpio.GPIO_Speed = GPIO_High_Speed;
	GPIO_Init(KEY_PORT,&mygpio);
}

uint16_t key_secret(void){
	uint16_t result = 0, count = 0;
	while(count < 4){
		if(KEY0_READ){
			delay_ms(300);
			led_on(GPIOG, 0);
			result = (result << 4) + 0x0;
			while(KEY0_READ);
			delay_ms(300);
			count++;
			led_down(GPIOG, 0);
		}
		
		if(KEY1_READ){
			delay_ms(300);
			led_on(GPIOG, 1);
			result = (result << 4) + 0x1;
			while(KEY1_READ);
			delay_ms(300);
			count++;
			led_down(GPIOG, 1);
		}
		
		if(KEY2_READ){
			delay_ms(300);
			led_on(GPIOG, 2);
			while(KEY2_READ);
			delay_ms(300);
			if(count > 0){
				count--;
				result = result >> 4;
			}
			led_down(GPIOG, 2);
		}
		
		if(KEY3_READ){
			delay_ms(300);
			led_on(GPIOG, 3);
			while(KEY3_READ);
			delay_ms(300);
			count = 0;
			result = 0;
			led_down(GPIOG, 3);
		}
	}
	return result;
}
