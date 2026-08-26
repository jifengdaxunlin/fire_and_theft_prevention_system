#include "TIM6.h"

void TIM6_config(void){
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
    
    TIM_TimeBaseInitTypeDef mytim = {0};
    mytim.TIM_ClockDivision = TIM_CKD_DIV1; 
    mytim.TIM_CounterMode = TIM_CounterMode_Up; 
    mytim.TIM_Period = 1000 - 1;    // 100ms 周期
    mytim.TIM_Prescaler = 8400 - 1;
    TIM_TimeBaseInit(TIM6, &mytim);
    
    NVIC_InitTypeDef mynvic = {0};
    mynvic.NVIC_IRQChannel = TIM6_DAC_IRQn; 
    mynvic.NVIC_IRQChannelCmd = ENABLE; 
    mynvic.NVIC_IRQChannelPreemptionPriority = 6;  // 【修改】必须 >= 5
    mynvic.NVIC_IRQChannelSubPriority = 0;         // PriorityGroup_4 下固定为 0
    NVIC_Init(&mynvic);
    
    TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE); 
    TIM_Cmd(TIM6, ENABLE);
}

AlarmType current_alarm = ALARM_NONE;

void TIM6_DAC_IRQHandler(void){
    static uint8_t toggle = 0;   
    static uint8_t ticks = 0;    
    
    if(TIM_GetITStatus(TIM6, TIM_IT_Update) == SET){
        toggle = !toggle; 
        ticks++;
        
        switch ((uint8_t)current_alarm){
            case ALARM_FIRE:
                if (toggle) {
                    RGB_SetColor(255, 0, 0);
                } else {
                    RGB_SetColor(0, 0, 0);
                    BEEP_SetFrequency(2000, 5);
                }
                break;

            case ALARM_GAS:
                if (toggle) {
                    RGB_SetColor(255, 0, 255);
                    BEEP_SetFrequency(1500, 5);
                } else {
                    RGB_SetColor(0, 0, 0);
                    BEEP_SetFrequency(1500, 0);
                }
                break;

            case ALARM_STRANGER:
                if (ticks % 2 == 0) {
                    if (toggle) {
                        RGB_SetColor(255, 150, 255);
                        BEEP_SetFrequency(1000, 5);
                    } else {
                        RGB_SetColor(0, 0, 0);
                        BEEP_SetFrequency(1000, 0);
                    }
                }
                break;

            case ALARM_TEMP:
                if (ticks % 5 == 0) {
                    if (toggle) {
                        RGB_SetColor(255, 50, 0);
                        BEEP_SetFrequency(600, 5);
                    } else {
                        RGB_SetColor(0, 0, 0);
                        BEEP_SetFrequency(600, 0);
                    }
                }
                break;

            case ALARM_HUMI:
                if (ticks % 5 == 0) {
                    if (toggle) {
                        RGB_SetColor(0, 0, 255);
                        BEEP_SetFrequency(400, 5);
                    } else {
                        RGB_SetColor(0, 0, 0);
                        BEEP_SetFrequency(400, 0);
                    }
                }
                break;
                
            case ALARM_MANUAL:
                if (ticks % 3 == 0) { 
                    if (toggle) {
                        RGB_SetColor(0, 255, 255); 
                        BEEP_SetFrequency(700, 5);  
                    } else {
                        RGB_SetColor(0, 0, 0);     
                        BEEP_SetFrequency(700, 0);
                    }
                }
                break;

            case ALARM_NONE:
                RGB_SetColor(0, 255, 0);           
                BEEP_SetFrequency(0, 0);           
                ticks = 0;
                break;
        }
        TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
    }
}

const char* Get_RGB_Color_Name(AlarmType alarm) {
    switch (alarm) {
        case ALARM_FIRE:     return "Red   "; 
        case ALARM_GAS:      return "Purp  "; 
        case ALARM_STRANGER: return "Whit  "; 
        case ALARM_TEMP:     return "Oran  "; 
        case ALARM_HUMI:     return "Blue  "; 
        case ALARM_MANUAL:   return "Cyan  "; 
        default:             return "Green "; 
    }
}
