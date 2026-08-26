#ifndef __MQTT_H
#define __MQTT_H

#include "esp8266.h"
#include "DELAY.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "USART.h"
#include "TIM6.h"

#define BYTE0(dwTemp)       (*( char *)(&dwTemp))
#define BYTE1(dwTemp)       (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp)       (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp)       (*((char *)(&dwTemp) + 3))
    
/************************  ONEET 服务器配置 ***************************************/
//#define MQTT_Client_ID  "DHT11" // 设置用户ID
//#define MQTT_User_Name "G7NqVv6le8" // 设置产品ID
//#define MQTT_Password  "version=2018-10-31&res=products%2FG7NqVv6le8%2Fdevices%2FDHT11&et=1729607740&method=md5&sign=VRwAjJjquF6I%2FyCFP%2FnnFQ%3D%3D" // MQTT连接OneNET的密码
//#define SubscribeTopic_Attribute "$sys/G7NqVv6le8/DHT11/thing/property/post/reply" // 订阅的主题
//#define SubscribeTopic_Server "$sys/G7NqVv6le8/DHT11/thing/service/LED_CONTROL/invoke" // 订阅的服务器地址
//#define PublishTopic   "$sys/G7NqVv6le8/DHT11/thing/property/post" // 发布的主题
//#define MQTTPUBLISH(temp, humi) "{\"id\":\"123\",\"params\":{\"temp\":{\"value\":%.2lf},\"humi\":{\"value\":%.2lf}}}",temp, humi // 发送OneJSON数据

/************************  华为云服务器配置 ***************************************/
#define MQTT_Client_ID  "6a83c964cbb0cf6bb97a573b_stm_32F407ZET6_0_0_2026081803" 
#define MQTT_User_Name  "6a83c964cbb0cf6bb97a573b_stm_32F407ZET6" // 设置产品ID
#define MQTT_Password  "53176d7f88608036c0518422dc55c2f8b4b9429a8d7ff44654ae4a6cd31f4725" // MQTT连接华为云的密码

#define SubscribeTopic_Server_Reply "$oc/devices/6a83c964cbb0cf6bb97a573b_stm_32F407ZET6/sys/commands/request_id={request_id}}" // 订阅的主题
#define SubscribeTopic_Server "$oc/devices/6a83c964cbb0cf6bb97a573b_stm_32F407ZET6/sys/messages/down"
// USART_ReceiveData()
//#define SubscribeTopic_Server_web "/k2877qwTkuz/shebei1/user/get" 

#define PublishTopic   "$oc/devices/6a83c964cbb0cf6bb97a573b_stm_32F407ZET6/sys/properties/report" // 发布的主题
//#define PublishTopic_web   "/k2877qwTkuz/shebei32/user/update" // 发布的主题

// #define MQTTPUBLISH(temp, humi) "{\"method\":\"thing.service.property.set\",\"id\":\"1229117953\",\"params\":{\"CurrentHumidity\":%2lf,\"CurrentTemperature\":%2lf,\"LED\":1,\"GeoLocation\":{\"CoordinateSystem\":10,\"Latitude\":10,\"Longitude\":10,\"Altitude\":10}},\"version\":\"1.0.0\"}",humi,temp

//#define MQTTPUBLISH(temp, humi, LED) "{\"method\":\"thing.service.property.set\",\"id\":\"1774038050\",\"params\":{\"CurrentHumidity\":%2lf,\"CurrentTemperature\":%2lf,\"LED\":%d},\"version\":\"1.0.0\"}",humi,temp,LED

#define MQTTPUBLISH(humi, temp, fire, gas, fan, gate, water, led, stranger) \
    "{\"services\":[{\"service_id\":\"jifengdaxunlin\",\"properties\":{" \
    "\"humi\":%.2f,\"temp\":%.2f,\"fire\":%.2f,\"gas\":%.2f," \
    "\"fan\":%d,\"gate\":%d,\"water\":%d,\"led\":%d,\"stranger\":%d}," \
    "\"event_time\":\"20151212T121212Z\"}]}", \
    humi, temp, fire, gas, fan, gate, water, led, stranger

/*
"{
   \"services\":
     [
      {\"service_id\":\"myenv\",
            \"properties\":{\"humi\":%f,\"temp\":%f,\"LED2\":%d,\"LUX\":%d},
            \"event_time\":\"20151212T121212Z\"}
        ]
}"
,humi,temp,LED2,LUX
*/

void MQTT_Clear(void);
void MQTT_SendData(uint8_t* buf, uint16_t len);
void MQTT_SendHeart(void);
void MQTT_Init(void);
void MQTT_Connect(void);
void MQTT_Disconnect(void);
// void MQTT_PublicTopic(float humi, float temp);

void MQTT_PublicTopic(float humi, float temp, float fire, float gas, bool fan, bool gate, bool water, AlarmType led, bool stranger);
void MQTT_PublicTopic_web(float temp, float humi, bool LED2);
void MQTT_SubscribeTopic(void);
void MQTT_SubscribeTopic_web(void);
void MQTT_UNSubscribeTopic(void);

#endif

/*
JSON 是一种字符串格式 
s = "{\"name\":\"小明\",\"age\":\"18\",\"hobby\":\"学习\"}";
{"LED":"%d","LED2":"%d","BEEP":"0"}
*/
