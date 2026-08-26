#ifndef __TIM6_H__
#define __TIM6_H__

#include "stm32f4xx.h"
#include "RGB.h"
#include "BEEP.h"
#include "FAN.h"

// 定义报警类型枚举（数值越小优先级越高）
typedef enum {
    ALARM_NONE = 0,
    ALARM_FIRE,     // 火焰
    ALARM_GAS,      // 烟雾/气体
    ALARM_STRANGER, // 陌生人
    ALARM_TEMP,     // 高温
    ALARM_HUMI,      // 高湿
	ALARM_MANUAL	// 手动警告
} AlarmType;

extern AlarmType current_alarm;

void TIM6_config(void);
const char* Get_RGB_Color_Name(AlarmType alarm);

#endif
