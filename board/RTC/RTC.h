#ifndef __RTC_H__
#define __RTC_H__

#include "stm32f4xx.h"
#include "DELAY.h"
#include <stdio.h>
#include <stdbool.h>

#define RTC_FLAG 9
#define LSE_OR_LSI 0 //为 0 = LSE  为 1 = LSI
#define YEAR 26
#define MONTH 8
#define DATE 12
#define WEEK 2
#define HOUR 9
#define MIN 0
#define SEC 0

#define A_HOUR 9
#define A_MIN 0
#define A_SEC 10
#define A_WEEK_DAY 4

extern char show_ST[20];
extern volatile bool g_need_sntp_sync;
extern 	RTC_TimeTypeDef mytime;
extern RTC_DateTypeDef mydate;

void RTC_Show_Time(void);
void RTC_Alarm(void);
void RTC_config(void);
void RTC_Time_Date(void);
void RTC_Mode_config(void);
void RTC_Wake_Up_Config(void);
	
#endif
