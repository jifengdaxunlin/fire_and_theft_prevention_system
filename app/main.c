#include "stm32f4xx.h"
#include <stdio.h>
#include "LED.h"
#include "DELAY.h"
#include "KEY.h"
#include "EXTI.h"
#include "USART.h"
#include "PIR.h"
#include "TIM6.h"
#include "TIM7.h"
#include "BEEP.h"
#include "Steering_Gear.h"
#include "RTC.h"
#include "DMA.h"
#include "DHT11.h"
#include "oled.h"
#include "Water.h"
#include "IR.h"
#include "RGB.h"
#include "EEPROM.h"
#include "MQ2.h"
#include "mqtt.h"
#include "esp8266.h"
#include "FAN.h"

/* FreeRTOS 头文件 */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// 结构体定义
typedef struct{
    bool Water;
    bool Fan;
    bool Gate;
} Status;

// 全局变量定义
Status s = {0};
Threshold T = {0}, T_temp = {0};
humi_temp g_ht = {0};
float g_fire_r = 1000000;
float g_mq2_per = 0;

// 互斥锁句柄
SemaphoreHandle_t xDataMutex = NULL;

// 任务句柄
TaskHandle_t xTaskSensorHandle  = NULL;
TaskHandle_t xTaskControlHandle = NULL;
TaskHandle_t xTaskDisplayHandle = NULL;
TaskHandle_t xTaskNetHandle     = NULL;

// 函数声明
void Task_SensorRead(void *pvParameters);
void Task_ControlLogic(void *pvParameters);
void Task_Display(void *pvParameters);
void Task_Network(void *pvParameters);

int main(void)
{
    // 1. NVIC中断分组 (FreeRTOS 必须使用组4)
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    // 2. 硬件外设初始化
    USART1_config(115200);
    USART3_config(115200);

    BEEP_config();
    KEY_config();
    EXTI_config();
    TIM6_config();
    TIM7_config();
    TIM_SetCompare1(TIM2, 0);
    SG_config();
	FAN_Config();
    RTC_config();
    OLED_Init();
    
    Water_config();
    IR_config();
    MQ2_config();
    RGB_config();

    // 从 EEPROM 加载初始阈值 (内部会执行 IIC_config)
    EEPROM_Load_Threshold(&T);
	T_temp = T;
    printf("[SYS] Thresholds loaded from EEPROM: Humi=%d, Temp=%d, Gas=%d, Fire=%.1fk\r\n", 
           T.humi, T.temp, T.gas, T.fire);

    // 3. 创建互斥锁
    xDataMutex = xSemaphoreCreateMutex();

    if (xDataMutex != NULL)
    {
        //printf("[SYS] Mutex created successfully.\r\n");

        xTaskCreate(Task_ControlLogic, "Control", 256, NULL, 4, &xTaskControlHandle);  
        xTaskCreate(Task_SensorRead,  "Sensor",  256, NULL, 3, &xTaskSensorHandle);   
        xTaskCreate(Task_Network,     "Network", 512, NULL, 2, &xTaskNetHandle);      
        xTaskCreate(Task_Display,     "Display", 512, NULL, 1, &xTaskDisplayHandle);  

        //printf("[SYS] All tasks created. Starting Scheduler...\r\n");
        //printf("========================================\r\n");

        // 5. 启动任务调度器
        vTaskStartScheduler();
    }
    else
    {
        //printf("[SYS_ERR] Mutex creation failed!\r\n");
    }

    while (1);
}

/* ========================================================================= */
/* 任务 1: 高优先级 - 核心控制与逻辑判定 (50ms周期)                          */
/* ========================================================================= */
void Task_ControlLogic(void *pvParameters)
{
    static bool last_manual_state = false;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    //printf("[T_CTRL] Control Task Started.\r\n");

    for (;;)
    {
        bool need_water = false;
		bool fan_enable = false;
		uint8_t fan_speed = 0;
        uint16_t sg_angle = 0;
        if (xSemaphoreTake(xDataMutex, pdMS_TO_TICKS(20)) == pdTRUE)
        {
            if (!Set_Threshold)
            {
                if (!Manual_Auto) // 自动模式
                {
                    if (g_fire_r < (T.fire * 1000.0f))          current_alarm = ALARM_FIRE;
                    else if (g_mq2_per > T.gas)                  current_alarm = ALARM_GAS;
                    else if (stranger_detected)                  current_alarm = ALARM_STRANGER;
                    else if (g_ht.temp > T.temp)                 current_alarm = ALARM_TEMP;
                    else if (g_ht.humi >= T.humi)                current_alarm = ALARM_HUMI;
                    else                                       current_alarm = ALARM_NONE;
                    stranger_timeout_cnt -= stranger_timeout_cnt == 0 ? 0 : 1;
					if(stranger_timeout_cnt == 0) stranger_detected = false;
					
                    s.Fan = (current_alarm == ALARM_TEMP || current_alarm == ALARM_GAS || current_alarm == ALARM_FIRE);

					if (current_alarm == ALARM_FIRE) {
						s.Water = true;
						need_water = true;
						fan_enable = true;
						fan_speed = 50; // 火警全速
					} 
					else if (current_alarm == ALARM_TEMP || current_alarm == ALARM_GAS) {
						fan_enable = true;
						fan_speed = 40;  // 温度或气体超标，开 80% 速度
						s.Water = false;
						need_water = false;
					}
					else {
						s.Water = false;
						need_water = false;
						fan_enable = false;
						fan_speed = 0;
					}

                    last_manual_state = false;
                }
                else // 手动模式
                {
                    if (!last_manual_state) {
                        key4_gate  = s.Gate;
                        key3_fan   = s.Fan;
                        key4_water = s.Water;
						key3_led_beep = current_alarm == 0 ? false : true;
                        last_manual_state = true;
                    }

                    current_alarm = key3_led_beep ? ALARM_MANUAL : ALARM_NONE;
                    s.Fan = key3_fan;
                    s.Gate = key4_gate;
                    s.Water = key4_water;
					
					
                    sg_angle = key4_gate ? 180 : 0;
                    need_water = key4_water;
					fan_enable = key3_fan;
					fan_speed = key3_fan ? 50 : 0;
                }
            }
            else // 设置阈值模式
            {
                if (add) {
                    add = false;

                    if (threshold_item == 0 && T_temp.humi < 100) T_temp.humi++;
                    if (threshold_item == 1 && T_temp.temp < 80)  T_temp.temp++;
                    if (threshold_item == 2 && T_temp.gas < 100)  T_temp.gas++;
                    if (threshold_item == 3 && T_temp.fire < 100.0f) T_temp.fire += 0.5f;
                    printf("[T_CTRL] Threshold ADD. Item:%d\r\n", threshold_item);
                }
                if (dec) {
                    dec = false;
                    if (threshold_item == 0 && T_temp.humi > 0) T_temp.humi--;
                    if (threshold_item == 1 && T_temp.temp > 0) T_temp.temp--;
                    if (threshold_item == 2 && T_temp.gas > 0)  T_temp.gas--;
                    if (threshold_item == 3 && T_temp.fire > 0.5f) T_temp.fire -= 0.5f;
                    printf("[T_CTRL] Threshold DEC. Item:%d\r\n", threshold_item);
                }
                if (save) {
                    save = false;
                    if (T_temp.humi != T.humi || T_temp.temp != T.temp || 
						T_temp.gas != T.gas   || T_temp.fire != T.fire) {
						EEPROM_Save_Threshold(&T_temp);
						EEPROM_Load_Threshold(&T);
						printf("[T_CTRL] Threshold SAVED & Reloaded.\r\n");
					} 
					else {
						printf("[T_CTRL] Threshold unchanged, skip EEPROM write.\r\n");
					}
                    EEPROM_Load_Threshold(&T);
                    Set_Threshold = false;
                    oled_clear = true;
                    printf("[T_CTRL] Threshold SAVED & Reloaded.\r\n");
                }
				if(key2_short){
					EEPROM_Load_Threshold(&T);
					T_temp = T;
					key2_short = false;
				}
            }

            xSemaphoreGive(xDataMutex);
        }
        else
        {
            printf("[T_CTRL_ERR] Mutex Take Timeout!\r\n");
        }

        //锁体外部：安全执行耗时的物理硬件驱动
		static uint8_t exit_protect_cnt = 0;
        if (!Set_Threshold) {
            if (exit_protect_cnt > 0) {
                exit_protect_cnt--;
            } else {
                SG_turn(sg_angle);
                Water_ration(need_water ? 100 : 0);
				if (fan_enable) {
					FAN_turn_left(fan_speed);
				} 
				else {
					FAN_brake();
				}
            }
        } else {
            // 只要处于设置模式，就重置保护计数器
            exit_protect_cnt = 5; // 比如退出后前 5 个周期（250ms）不动作
        }

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(50)); 
    }
}

/* ========================================================================= */
/* 任务 2: 中优先级 - 传感器定时采集 (500ms周期)                             */
/* ========================================================================= */
void Task_SensorRead(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    //printf("[T_SENSOR] Sensor Read Task Started.\r\n");

    for (;;)
    {
        humi_temp ht_temp = DHT11_Read_Data();
        float fire_temp   = IR_Get_Fire(5);
        float mq2_temp    = MQ2_Get_Per(5);

        if (xSemaphoreTake(xDataMutex, pdMS_TO_TICKS(50)) == pdTRUE)
        {
            if (ht_temp.temp > 0.0f || ht_temp.humi > 0.0f) {
                g_ht = ht_temp;
            }
            
            g_fire_r  = fire_temp;
            g_mq2_per = mq2_temp;
            
            xSemaphoreGive(xDataMutex);
        }
        else
        {
            //printf("[T_SENSOR_ERR] Mutex Take Timeout!\r\n");
        }

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));  
    }
}

/* ========================================================================= */
/* 任务 3: 低优先级 - 网络同步与云端交互                                     */
/* ========================================================================= */
void Task_Network(void *pvParameters)
{
    vTaskDelay(pdMS_TO_TICKS(500));

    //printf("\r\n[NET] Starting ESP8266 Init...\r\n");
    ESP8266_Init(); 
    //printf("[NET] ESP8266 Init Finished!\r\n");

    g_need_sntp_sync = false; 
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        if (g_need_sntp_sync) {
            //printf("[NET] Starting SNTP Time Sync...\r\n");
            g_need_sntp_sync = false;
            
            Usart_SendString(USART3, "+++");
            vTaskDelay(pdMS_TO_TICKS(1000));
            
            ESP8266_Sync_RTC_Time(); 
            ESP8266_SendCmd("AT+CIPMODE=1", "OK");
            ESP8266_SendCmd("AT+CIPSEND", ">");
            //printf("[NET] SNTP Sync Done, Re-entered Passthrough.\r\n");
        }

        float temp_humi = 0, temp_temp = 0, temp_fire = 0, temp_mq2 = 0;
        bool temp_fan = false, temp_gate = false, temp_water = false;
        uint8_t temp_alarm = 0, temp_stranger = 0;

        if (xSemaphoreTake(xDataMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            temp_humi     = g_ht.humi;
            temp_temp     = g_ht.temp;
            temp_fire     = g_fire_r;
            temp_mq2      = g_mq2_per;
            temp_fan      = s.Fan;
            temp_gate     = s.Gate;
            temp_water    = s.Water;
            temp_alarm    = (uint8_t)current_alarm;
            temp_stranger = stranger_detected ? 1 : 0;
            
            xSemaphoreGive(xDataMutex); 

            //printf("[NET] Lock Acquired. Publishing MQTT Data...\r\n");
            MQTT_PublicTopic(temp_humi, temp_temp, temp_fire, temp_mq2, 
                             temp_fan, temp_gate, temp_water, (AlarmType)temp_alarm, temp_stranger);
            printf("[NET] MQTT Publish Completed.\r\n");
        } else {
            printf("[NET_ERR] Lock Take Timeout (xDataMutex occupied too long!)\r\n");
        }

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(2000));  
    }
}

/* ========================================================================= */
/* 任务 4: 低优先级 - OLED 界面刷新 (200ms周期)（带详尽printf测试）           */
/* ========================================================================= */
void Task_Display(void *pvParameters)
{
    char humi[20], temp[20], fire_str[20], led_str[20], MQ2_str[20], buf[20];
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    //printf("[T_DISP] Display Task Started Successfully.\r\n");

    for (;;)
    {
        // 尝试获取互斥锁（限时 50ms）
        if (xSemaphoreTake(xDataMutex, pdMS_TO_TICKS(50)) == pdTRUE)
        {
            // printf("[T_DISP] Mutex Acquired. Drawing to Gram Buffer...\r\n");

            OLED_ShowStr_8x8(0, 0, "Warehouse System");

            if (!g_rtc_synced) {
                OLED_ShowStr_8x8(24, 1, "waiting...");
            } else {
                RTC_Show_Time(); 
                OLED_ShowStr_5x8(27, 1, show_ST);
            }

            if (!Set_Threshold)
            {
                if (!key2_short) // 第一页
                {
                    if (oled_clear) { OLED_ClearArea_Gram(0, 127, 1, 7); oled_clear = false; }

                    if (g_ht.humi > 0.01f || g_ht.temp > 0.01f) {
                        snprintf(humi, sizeof(humi), "-Humi:%.1f%%  ", g_ht.humi);
                        snprintf(temp, sizeof(temp), "-Temp:%.1fC  ", g_ht.temp);
                    } else {
                        snprintf(humi, sizeof(humi), "-Humi:Init... ");
                        snprintf(temp, sizeof(temp), "-Temp:Init... ");
                    }
                    OLED_ShowStr_8x8(8, 2, humi);
                    OLED_ShowStr_8x8(8, 3, temp);

                    snprintf(MQ2_str, sizeof(MQ2_str), "-Gas:%.1f%%  ", g_mq2_per);
                    OLED_ShowStr_8x8(8, 4, MQ2_str);

                    if (g_fire_r >= 100000.0f)        snprintf(fire_str, sizeof(fire_str), "-Fire: Safe   ");
                    else if (g_fire_r >= 1000.0f)    snprintf(fire_str, sizeof(fire_str), "-Fire:%.1fk   ", g_fire_r / 1000.0f);
                    else                             snprintf(fire_str, sizeof(fire_str), "-Fire:%.0f Ohm ", g_fire_r);
                    
                    OLED_ShowStr_8x8(8, 5, fire_str);
                    OLED_ShowStr_8x8(8, 6, stranger_detected ? "-Stranger:Ill" : "-Stranger:Nor");
                    OLED_ShowStr_8x8(72, 7, "Page1/2");
                }
                else // 第二页
                {
                    if (oled_clear) { OLED_ClearArea_Gram(0, 127, 1, 7); oled_clear = false; }

                    OLED_ShowStr_8x8(8, 2, s.Water ? "-Water:on  " : "-Water:down");
                    OLED_ShowStr_8x8(8, 3, s.Fan   ? "-Fan:  on  " : "-Fan:  down");

                    snprintf(led_str, sizeof(led_str), "-LED: %s", Get_RGB_Color_Name(current_alarm));
                    OLED_ShowStr_8x8(8, 4, led_str);
                    OLED_ShowStr_8x8(8, 5, (current_alarm == 0) ? "-Beep: down" : "-Beep: on  ");
                    OLED_ShowStr_8x8(8, 6, s.Gate  ? "-Gate: on      " : "-Gate: down  ");
                    OLED_ShowStr_8x8(72, 7, "Page2/2");
                }

                OLED_ShowStr_8x8(0, 7, Manual_Auto ? "Manual" : "Auto  ");
            }
            else // 设置阈值界面
            {
                if (oled_clear) { OLED_ClearArea_Gram(0, 127, 1, 7); oled_clear = false; }

                OLED_ShowStr_8x8(0, 2, "=== Set Limit ==");
                snprintf(buf, sizeof(buf), "%c1.Humi : %d%%", (threshold_item == 0) ? '>' : ' ', T_temp.humi);
                OLED_ShowStr_8x8(0, 3, buf);
                snprintf(buf, sizeof(buf), "%c2.Temp : %d C", (threshold_item == 1) ? '>' : ' ', T_temp.temp);
                OLED_ShowStr_8x8(0, 4, buf);
                snprintf(buf, sizeof(buf), "%c3.Gas  : %d",   (threshold_item == 2) ? '>' : ' ', T_temp.gas);
                OLED_ShowStr_8x8(0, 5, buf);
                snprintf(buf, sizeof(buf), "%c4.Fire : %.1fk", (threshold_item == 3) ? '<' : ' ', T_temp.fire);
                OLED_ShowStr_8x8(0, 6, buf);
                OLED_ShowStr_8x8(0, 7, "K1:Item K3/4:+-");
            }

            // 释放互斥锁
            xSemaphoreGive(xDataMutex);
            OLED_Refresh_Gram(); 
        } 


        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(200));
    }
}
