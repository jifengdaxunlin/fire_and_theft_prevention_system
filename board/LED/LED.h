#ifndef __LED_H__
#define __LED_H__

#include "stm32f4xx.h"
#include <stdbool.h>
#include "DELAY.h"

void LED_config(void);
void led_all_control(bool station);
void led_on(GPIO_TypeDef* GPIOx, uint16_t led_flag);
void led_down(GPIO_TypeDef* GPIOx, uint16_t led_flag);
void led_running(GPIO_TypeDef* GPIOx,uint16_t* led_pin_list, uint32_t time);
void led_chasing(GPIO_TypeDef* GPIOx,uint16_t* led_pin, uint32_t time);
void led_change(GPIO_TypeDef* GPIOx, uint16_t led_flag);
void led_flash(GPIO_TypeDef* GPIOx, uint16_t* led_pin,uint32_t time_ms);
void led_chasing_double(GPIO_TypeDef* GPIOx, uint16_t* led_pin, uint32_t time);
void led_breath(GPIO_TypeDef* GPIOx, uint16_t led_flag);

extern uint16_t leds[];

#define LED_NUM 4
#define LED_PIN_LIST {GPIO_Pin_14,GPIO_Pin_13,GPIO_Pin_6,GPIO_Pin_11}

#endif
