#include "DELAY.h"

void delay(volatile uint32_t cnt){
	while (cnt > 0) {
		cnt--;
	}
}
// us数值不能超过 798 * 1000 * 1000
void delay_us(uint32_t us){
	SysTick->CTRL = 0;
	//设置重装载值
	SysTick->LOAD = us * 21;
	//清空当前计数寄存器
	SysTick->VAL = 0;
	//使能
	SysTick->CTRL = 1;
	//卡住
	while((SysTick->CTRL & 0x00010000) == 0);
	//关闭定时器
	SysTick->CTRL = 0;
}

void delay_ms(uint32_t ms){
	while(ms--){
		delay_us(1000);
	}
}

void delay_s(uint32_t s){
	while(s--){
		delay_ms(500);
		delay_ms(500);
	}
}
