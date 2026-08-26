#include "TIM7.h"

void TIM7_config(void){
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
    
    TIM_TimeBaseInitTypeDef mytim = {0};
    mytim.TIM_ClockDivision = TIM_CKD_DIV1; 
    mytim.TIM_CounterMode = TIM_CounterMode_Up; 
    mytim.TIM_Period = 1000 - 1;    // 1ms 周期
    mytim.TIM_Prescaler = (840 - 1);
    TIM_TimeBaseInit(TIM7, &mytim);
    
    NVIC_InitTypeDef mynvic = {0};
    mynvic.NVIC_IRQChannel = TIM7_IRQn; 
    mynvic.NVIC_IRQChannelCmd = ENABLE; 
    mynvic.NVIC_IRQChannelPreemptionPriority = 6;  // 【修改】必须 >= 5，防止引发 HardFault
    mynvic.NVIC_IRQChannelSubPriority = 0;         // PriorityGroup_4 下固定为 0
    NVIC_Init(&mynvic);
    
    TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE); 
    TIM_Cmd(TIM7, ENABLE);
}

// 全局变量定义
bool Manual_Auto = false, Set_Threshold = false,
     key2_short = false,
     key3_led_beep = false, key3_fan = false,
     key4_water = false, key4_gate = false,
     oled_clear = false, subscribe = false, read_data = true;

bool *Back = &key2_short, 
     add = false, dec = false, save = false;

uint8_t key1_time = 0, key2_time = 0, key3_time = 0, key4_time = 0;
uint8_t threshold_item = 0; 
uint16_t oled_time = 0, subscribe_time = 0, data_time = 0;

void TIM7_IRQHandler(void){
    if(TIM_GetITStatus(TIM7, TIM_IT_Update) == SET){
        oled_time += oled_time > 1000 ? 0 : 1;  
        data_time += data_time > 50 ? 0 : 1;    
        subscribe_time += subscribe_time > 1000 ? 0 : 1;   
        
        if(oled_time == 1000) { oled_clear = true; oled_time = 0; }
        if(data_time == 50) { read_data = true; data_time = 0; }
        if(subscribe_time == 1000) { subscribe = true; subscribe_time = 0; }
        
        // Key 1 逻辑
        if(KEY0_READ){
            key1_time += key1_time < 150 ? 1 : 0;
        } else {
            if(key1_time == 150) {
                if(Set_Threshold) save = true; 
                else { Set_Threshold = true; threshold_item = 0; }
            } else if(key1_time < 150 && key1_time > 0) {
                if(!Set_Threshold) Manual_Auto = !Manual_Auto;
                else threshold_item = (threshold_item + 1) % 4; 
            }
            key1_time = 0;
        }
        
        // Key 2 逻辑
        if(KEY1_READ){
            key2_time += key2_time < 150 ? 1 : 0;
        } else {
            if(key2_time > 0){
                if(Set_Threshold) Set_Threshold = false, key2_short = true;
                else key2_short = !key2_short;
                key2_time = 0;
                oled_clear = true;
            }            
        }
        
        // Key 3 逻辑
        if(KEY2_READ){
            key3_time += key3_time < 150 ? 1 : 0;
            if(Set_Threshold && key3_time == 150) add = true;
        } else {
            if(!Set_Threshold){
                if(key3_time == 150) key3_led_beep = !key3_led_beep;
                else if(key3_time < 150 && key3_time > 0) key3_fan = !key3_fan;
            }
			else if(key3_time > 0)add = true;
            key3_time = 0;
        }
        
        // Key 4 逻辑
        if(KEY3_READ){
            key4_time += key4_time < 150 ? 1 : 0;
            if(Set_Threshold && key4_time == 150) dec = true;
        } else {
            if(!Set_Threshold){
                if(key4_time == 150) key4_water = !key4_water;
                else if(key4_time < 150 && key4_time > 0) key4_gate = !key4_gate;
            }
			else if(key4_time > 0) dec = true;
            key4_time = 0;
        }
        
        TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
    }
}
