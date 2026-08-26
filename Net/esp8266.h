#ifndef __ESP8266_H
#define __ESP8266_H

#include "DELAY.h"
#include "stm32f4xx.h"
#include "DELAY.h"
#include "mqtt.h"
#include "RTC.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define REV_OK      0   // 接收完成标志
#define REV_WAIT    1   // 接收未完成标志

// 转义字符 \  注意 -> 无转义  无转义 -> 注意
// "AT+CWJAP="iPhonexyz","xiaoyuzai124"\r\n"

// WiFi的账号和密码
#define ESP8266_WIFI_INFO     "AT+CWJAP=\"JIFENGDAXUNLIN 0922\",\"84T09z*7\"\r\n"

// (TCP/UDP)模式，华为云 Broker Address，Broker Port
#define ESP8266_SERVER "AT+CIPSTART=\"TCP\",\"ec2e895127.st1.iotda-device.cn-north-4.myhuaweicloud.com\",1883\r\n"

extern volatile bool g_rtc_synced; // 记录时间是否已成功通过网络同步

_Bool ESP8266_WaitRecive(void);
void ESP8266_Init(void);
void ESP8266_memset_RecvBuff(void);
_Bool ESP8266_SendCmd(char *str, char* rev);
void ESP8266_ConnectServer(void);
void recv_data_control(char * data);
void MQTT_RX_DATE_DEAL(char *recv_buf);
bool ESP8266_Sync_RTC_Time(void);

#endif
