#include "RTC.h"

void RTC_config(void){
	//读取RTC的备份寄存器的内容,只会初始化一次
	if(RTC_ReadBackupRegister(RTC_BKP_DR0) != RTC_FLAG){
		RTC_Mode_config();	//
		RTC_Time_Date();
		RTC_Alarm();
		RTC_WriteBackupRegister(RTC_BKP_DR0, RTC_FLAG);	//向RTC的备份寄存器写入内容,数据不会因为断电消失
		printf("RTC First Time set OK!\n");
	}
	else{
		//其他
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);	//开启PWR的时钟信号
		PWR_BackupAccessCmd(ENABLE);	//RCC断电数据不丢失,PWR备份寄存器实现,开启寄存器访问权限
		RTC_WaitForSynchro();	//等待时钟同步
		RTC_ClearITPendingBit(RTC_IT_ALRA);
		RTC_ClearFlag(RTC_FLAG_ALRAF);	//清除标志位
		EXTI_ClearITPendingBit(EXTI_Line17);	//清除17号线的标志位
		printf("Don't need Congfigure RTC again\n");
	}
	RTC_Wake_Up_Config();	//配置唤醒功能
}

//选择时钟和分频
void RTC_Mode_config(void){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);	//开启PWR的时钟信号
	PWR_BackupAccessCmd(ENABLE);	//RCC断电数据不丢失,PWR备份寄存器实现,开启寄存器访问权限
	
	uint16_t AsyncPSC = 0;
	uint16_t SyncPSC = 0;
	
	if(LSE_OR_LSI == 0){
		//LSE
		RCC_LSEConfig(RCC_LSE_ON);	//开启LSE
		while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) != SET);	//等待LSE校准完毕
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);	//设置RTC时钟源
		AsyncPSC = 0x7F;	//127
		SyncPSC = 0xFF;		//255
	}
	else{
		//LSI
		RCC_LSICmd(ENABLE);	//开启LSI
		while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) != SET);	//等待LSI校准完毕
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);	//设置RTC时钟源
		AsyncPSC = 0x7F;	//127
		SyncPSC = 0xF9;		//249
	}
	
	RCC_RTCCLKCmd(ENABLE);	//开启RTC时钟
	RTC_WaitForSynchro();	//等待时钟同步
	RTC_InitTypeDef myrtc = {0};
	myrtc.RTC_AsynchPrediv = AsyncPSC;
	myrtc.RTC_SynchPrediv = SyncPSC;
	myrtc.RTC_HourFormat = RTC_HourFormat_24;
	RTC_Init(&myrtc);	//配置RTC
}


//时间和日期的配置
void RTC_Time_Date(void){
	RTC_TimeTypeDef mytime = {0};
	mytime.RTC_H12 = RTC_H12_AM;	//选择12小时制才生效
	mytime.RTC_Hours = HOUR;	//时
	mytime.RTC_Minutes = MIN;	//分
	mytime.RTC_Seconds = SEC;	//秒
	RTC_SetTime(RTC_Format_BIN, &mytime);	//配置时间函数
	
	RTC_DateTypeDef mydate = {0};
	mydate.RTC_Year = YEAR;		//年
	mydate.RTC_Month = MONTH;	//月
	mydate.RTC_Date = DATE;		//日
	mydate.RTC_WeekDay = WEEK;	//星期
	RTC_SetDate(RTC_Format_BIN, &mydate);	//配置日期函数
}

//闹钟的配置
void RTC_Alarm(void){
	RTC_AlarmTypeDef myalarm = {0};
	myalarm.RTC_AlarmTime.RTC_H12 = RTC_H12_AM;
	myalarm.RTC_AlarmTime.RTC_Hours = A_HOUR;
	myalarm.RTC_AlarmTime.RTC_Minutes = A_MIN;
	myalarm.RTC_AlarmTime.RTC_Seconds = A_SEC;
	
	myalarm.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_WeekDay;	//匹配是每月几号,还是每周星期几
	myalarm.RTC_AlarmDateWeekDay = A_WEEK_DAY;	//上面选每月几号选日期,每周星期几选星期
	myalarm.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;
	/*
	RTC_AlarmMask_None			全部生效
	RTC_AlarmMask_DateWeekDay	日期/星期无效
	RTC_AlarmMask_Hours			小时无效
	RTC_AlarmMask_Minutes		分钟无效
	RTC_AlarmMask_Seconds		秒无效
	RTC_AlarmMask_All			全部无效
	*/
	RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &myalarm);
	
	//配置 17 号中短线,上升沿
	EXTI_InitTypeDef myexti = {0};
	myexti.EXTI_Line = EXTI_Line17; //哪个中断线
	myexti.EXTI_LineCmd = ENABLE;  //使能
	myexti.EXTI_Mode = EXTI_Mode_Interrupt;  //工作模式  中断/事件
	myexti.EXTI_Trigger = EXTI_Trigger_Rising; //触发条件: 上升沿
	EXTI_Init(&myexti);
	
	NVIC_InitTypeDef mynvic = {0};
	mynvic.NVIC_IRQChannel = RTC_Alarm_IRQn; //中断通道
	mynvic.NVIC_IRQChannelCmd = ENABLE; //使能
	mynvic.NVIC_IRQChannelPreemptionPriority = 0;  //抢占优先级 0~15
	mynvic.NVIC_IRQChannelSubPriority = 1;		//响应优先级 0~15
	NVIC_Init(&mynvic);
	
	RTC_ITConfig(RTC_IT_ALRA, ENABLE);	//开启闹钟A中断
	RTC_ClearFlag(RTC_FLAG_ALRAF);	//清除标志位
	RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
}


//唤醒中断
void RTC_Wake_Up_Config(void){
	RTC_WakeUpCmd(DISABLE);
	
	//配置 22 号中短线,上升沿
	EXTI_InitTypeDef myexti = {0};
	myexti.EXTI_Line = EXTI_Line22; //哪个中断线
	myexti.EXTI_LineCmd = ENABLE;  //使能
	myexti.EXTI_Mode = EXTI_Mode_Interrupt;  //工作模式  中断/事件
	myexti.EXTI_Trigger = EXTI_Trigger_Rising; //触发条件: 上升沿
	EXTI_Init(&myexti);
	
	NVIC_InitTypeDef mynvic = {0};
	mynvic.NVIC_IRQChannel = RTC_WKUP_IRQn; //中断通道
	mynvic.NVIC_IRQChannelCmd = ENABLE; //使能
	mynvic.NVIC_IRQChannelPreemptionPriority = 0;  //抢占优先级 0~15
	mynvic.NVIC_IRQChannelSubPriority = 2;		//响应优先级 0~15
	NVIC_Init(&mynvic);
	
	RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);	//配置唤醒中断的时钟源(相当于RSC分频)
	RTC_SetWakeUpCounter(0x0);	//重装载值 相当于(arr)
	
	RTC_ClearITPendingBit(RTC_IT_WUT);	//清除唤醒中断标志位
	RTC_ITConfig(RTC_IT_WUT, ENABLE);	//开启唤醒中断
	RTC_WakeUpCmd(ENABLE);		//使能唤醒功能
}

//显示年月日时分秒
char show_ST[20] = {0};
RTC_TimeTypeDef mytime = {0};
RTC_DateTypeDef mydate = {0};
void RTC_Show_Time(void){	
	RTC_GetTime(RTC_Format_BIN, &mytime);
	RTC_GetDate(RTC_Format_BIN, &mydate);
	sprintf(show_ST, "%d-%d-%d %d:%d:%d  ", mydate.RTC_Year, mydate.RTC_Month, mydate.RTC_Date, mytime.RTC_Hours, mytime.RTC_Minutes, mytime.RTC_Seconds);
}

//闹钟
void RTC_Alarm_IRQHandler(void){
	if(RTC_GetITStatus(RTC_IT_ALRA) == SET){
		printf("Alarm_A is work now...");
		EXTI_ClearITPendingBit(EXTI_Line17);	//清除17号线的标志位
		RTC_ClearITPendingBit(RTC_IT_ALRA);
	}
}

volatile uint32_t g_sntp_sync_timer = 0;  // 1小时计时计数器 (秒)
volatile bool g_need_sntp_sync = true;     // 校时标志位
//唤醒
void RTC_WKUP_IRQHandler(void){
    if(RTC_GetITStatus(RTC_IT_WUT) == SET){
        RTC_Show_Time();
        
        // 每秒累加，满 3600 秒（1小时）设置标志位
        g_sntp_sync_timer++;
        if (g_sntp_sync_timer >= 3600) {
            g_sntp_sync_timer = 0;
            g_need_sntp_sync = true; // 标记需要校时
        }

        EXTI_ClearITPendingBit(EXTI_Line22);
        RTC_ClearITPendingBit(RTC_IT_WUT);
    }
}
