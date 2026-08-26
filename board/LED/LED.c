#include "LED.h"

uint16_t leds[] = LED_PIN_LIST;

void LED_config(void){
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
	GPIO_InitTypeDef mygpio;
	mygpio.GPIO_Mode = GPIO_Mode_OUT;
	mygpio.GPIO_OType = GPIO_OType_PP;
	mygpio.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_14;
	mygpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
	mygpio.GPIO_Speed = GPIO_High_Speed;
	GPIO_Init(GPIOG,&mygpio);
	GPIO_SetBits(GPIOG,GPIO_Pin_6 | GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_14);
}

void led_all_control(bool station){
	if (station){
		for(int i = 0; i < LED_NUM; i++){
			GPIO_ResetBits(GPIOG,leds[i]);
		}
	}
	else {
		for(int i = 0; i < LED_NUM; i++){
			GPIO_SetBits(GPIOG,leds[i]);
		}
	}
}

void led_on(GPIO_TypeDef* GPIOx, uint16_t led_flag){
	GPIO_ResetBits(GPIOx,leds[led_flag]);
}

void led_down(GPIO_TypeDef* GPIOx, uint16_t led_flag){
	GPIO_SetBits(GPIOx,leds[led_flag]);
}

void led_running(GPIO_TypeDef* GPIOx,uint16_t* led_pin_list, uint32_t time){
	uint16_t num = 0;
	while(num < LED_NUM){
		GPIO_ResetBits(GPIOx,led_pin_list[num]);delay_ms(time);
		num++;
	}
	num = 0;
	while(num < LED_NUM){
		GPIO_SetBits(GPIOx,led_pin_list[num]);delay_ms(time);
		num++;
	}
}

void led_chasing(GPIO_TypeDef* GPIOx,uint16_t* led_pin, uint32_t time){
	int16_t num = 0;
	while(num < LED_NUM){
		GPIO_ResetBits(GPIOx,led_pin[num]);
		delay_ms(time);
		GPIO_SetBits(GPIOx,led_pin[num]);
		num++;
	}
}

void led_chasing_double(GPIO_TypeDef* GPIOx, uint16_t* led_pin, uint32_t time){
	int16_t num = 0;
	led_all_control(0);
	while(num < LED_NUM){
		GPIO_ResetBits(GPIOx,led_pin[num]);
		delay_ms(time);
		GPIO_SetBits(GPIOx,led_pin[num]);
		num++;
	}
	num -= 1;
	while(num >= 0){
		GPIO_ResetBits(GPIOx,led_pin[num]);
		delay_ms(time);
		GPIO_SetBits(GPIOx,led_pin[num]);
		num--;
	}
}

void led_flash(GPIO_TypeDef* GPIOx, uint16_t* led_pin,uint32_t time_ms){
	uint16_t count = 0;
	led_all_control(0);
	while(count < LED_NUM){
		GPIO_ResetBits(GPIOx,led_pin[count]);
		count++;
	}
	delay_ms(time_ms);
	count = 0;
	while(count < LED_NUM){
		GPIO_SetBits(GPIOx,led_pin[count]);
		count++;
	}
	delay_ms(time_ms);
}

void led_change(GPIO_TypeDef* GPIOx, uint16_t led_flag){
	GPIO_ToggleBits(GPIOx, leds[led_flag]);
}

void led_breath(GPIO_TypeDef* GPIOx, uint16_t led_flag){
	int16_t step = 200, count = 1;
	for(count = 1;count < step; count++){
		for(uint8_t i = 0; i < 20 ;i++){
			led_on(GPIOx, led_flag);
			delay_us(count);
			led_down(GPIOx, led_flag);
			delay_us(step - count);
		}
	}
	
	for(count = step - 1;count > 0; count--){
		for(uint8_t i = 0; i < 20 ;i++){
			led_on(GPIOx, led_flag);
			delay_us(count);
			led_down(GPIOx, led_flag);
			delay_us(step - count);
		}
	}
	led_down(GPIOx, led_flag);
	delay_ms(500);
}
