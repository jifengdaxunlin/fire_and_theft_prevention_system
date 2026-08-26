#include "IWDG.h"

void IWDG_config(void){
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);	//开启写使能,允许修改看门狗里的东西
	IWDG_SetPrescaler(IWDG_Prescaler_64);	//预分频
	IWDG_SetReload(600);	//设置重装载值  1.2s里"喂狗"
	IWDG_Enable();
	IWDG_ReloadCounter();	//立即"喂狗",重装载
}
