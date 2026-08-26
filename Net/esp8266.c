#include "esp8266.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>

// 缓冲区定义
uint8_t recv_buf[512] = {0};
volatile uint16_t cnt = 0;
static uint16_t esp8266_cntPre = 0;
volatile bool g_rtc_synced = false; // 记录时间是否已成功通过网络同步

/* RTOS 兼容的毫秒延时 */
static void ESP8266_DelayMs(uint32_t ms)
{
#if defined(FreeRTOS_H) || defined(INC_FREERTOS_H)
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        vTaskDelay(pdMS_TO_TICKS(ms));
    } else {
        delay_ms(ms);
    }
#else
    delay_ms(ms);
#endif
}

/**
 * @brief 清空接收缓冲区数据
 */
void ESP8266_memset_RecvBuff(void)
{
    __disable_irq();
    memset(recv_buf, 0, sizeof(recv_buf));
    cnt = 0;
    esp8266_cntPre = 0;
    __enable_irq();
}

/**
 * @brief 等待数据接收完成 (空闲检测)
 */
_Bool ESP8266_WaitRecive(void)
{
    if (cnt == 0) return REV_WAIT;
    
    if (cnt == esp8266_cntPre) {
        return REV_OK; // 数据已完成一帧的接收
    }
    esp8266_cntPre = cnt;
    return REV_WAIT;
}

/**
 * @brief 发送 AT 指令并等待预期响应 (支持自定义超时)
 * @param timeout_ms 超时时间(单位ms)
 */
_Bool ESP8266_SendCmd_Timeout(char *str, char *rev, uint32_t timeout_ms)
{
    uint32_t timeOut = timeout_ms / 10;
    ESP8266_memset_RecvBuff();
    
    if (str != NULL && strlen(str) > 0) {
        Usart_SendString(USART3, str);
    }

    while (timeOut--) 
    {
        if (ESP8266_WaitRecive() == REV_OK) 
        {
            if (rev == NULL || strlen(rev) == 0 || strstr((const char *)recv_buf, rev) != NULL) 
            {
                return 0; // 成功
            }
        }
        ESP8266_DelayMs(10);
    }
    return 1; // 超时失败
}

_Bool ESP8266_SendCmd(char *str, char *rev)
{
    return ESP8266_SendCmd_Timeout(str, rev, 2000); // 默认2秒超时
}

/**
 * @brief 退出透传模式
 */
void ESP8266_ExitUnvarnished(void)
{
    ESP8266_DelayMs(1000); // 1. 前置静默1秒
    ESP8266_memset_RecvBuff();
    
    // 2. 发送 +++ (绝对不要带 \r\n)
    Usart_SendString(USART3, "+++"); 
    ESP8266_DelayMs(1000); // 3. 后置静默1秒
}

/**
 * @brief 连接 WiFi 专用函数 (带长超时和抗 busy 机制)
 */
_Bool ESP8266_JoinAP(void)
{
    printf("Connecting WiFi...\r\n");
    // 发送连接指令，并等待最长 10 秒（防止 busy p...）
    if (ESP8266_SendCmd_Timeout(ESP8266_WIFI_INFO, "OK", 10000) == 0) {
        return 0; // 成功连接并获取 IP
    }
    return 1; // 失败
}

/**
 * @brief 同步 RTC 时间
 */
bool ESP8266_Sync_RTC_Time(void)
{
    char week_str[4] = {0}, month_str[4] = {0};
    int date = 0, hour = 0, min = 0, sec = 0, year = 0, month = 0;

    printf("Starting SNTP Sync...\r\n");

    // 1. 设置 SNTP 服务器 (8为东八区北京时间)
    if (ESP8266_SendCmd("AT+CIPSNTPCFG=1,8,\"ntp.aliyun.com\",\"ntp1.aliyun.com\"\r\n", "OK")) {
        printf("SNTP Config Failed!\r\n");
        return false;
    }
    ESP8266_DelayMs(500);

    // 2. 查询时间 (给予最多 3 秒等待网络返回)
    ESP8266_memset_RecvBuff();
    Usart_SendString(USART3, "AT+CIPSNTPTIME?\r\n");
    
    // 轮询等待 3 秒直到拿到 +CIPSNTPTIME 响应
    uint16_t retry = 300;
    while (retry--) {
        if (strstr((const char *)recv_buf, "+CIPSNTPTIME:")) break;
        ESP8266_DelayMs(10);
    }

    // 3. 解析响应
    char *ptr = strstr((const char *)recv_buf, "+CIPSNTPTIME:");
    if (ptr != NULL) 
    {
        if (sscanf(ptr, "+CIPSNTPTIME:%3s %3s %d %d:%d:%d %d", 
                   week_str, month_str, &date, &hour, &min, &sec, &year) == 7) 
        {
            if (year < 2024) return false;

            if      (strcmp(month_str, "Jan") == 0) month = 1;
            else if (strcmp(month_str, "Feb") == 0) month = 2;
            else if (strcmp(month_str, "Mar") == 0) month = 3;
            else if (strcmp(month_str, "Apr") == 0) month = 4;
            else if (strcmp(month_str, "May") == 0) month = 5;
            else if (strcmp(month_str, "Jun") == 0) month = 6;
            else if (strcmp(month_str, "Jul") == 0) month = 7;
            else if (strcmp(month_str, "Aug") == 0) month = 8;
            else if (strcmp(month_str, "Sep") == 0) month = 9;
            else if (strcmp(month_str, "Oct") == 0) month = 10;
            else if (strcmp(month_str, "Nov") == 0) month = 11;
            else if (strcmp(month_str, "Dec") == 0) month = 12;

            RTC_DateTypeDef mydate_s = {.RTC_Year = (uint8_t)(year % 100), .RTC_Month = (uint8_t)month, .RTC_Date = (uint8_t)date};
            RTC_TimeTypeDef mytime_s = {.RTC_Hours = (uint8_t)hour, .RTC_Minutes = (uint8_t)min, .RTC_Seconds = (uint8_t)sec};

            RTC_SetDate(RTC_Format_BIN, &mydate_s);
            RTC_SetTime(RTC_Format_BIN, &mytime_s);

            g_rtc_synced = true; // 标志位设为 true，代表同步成功！

            printf("RTC Sync Success: 20%02d-%02d-%02d %02d:%02d:%02d\r\n", 
                   mydate_s.RTC_Year, mydate_s.RTC_Month, mydate_s.RTC_Date, 
                   mytime_s.RTC_Hours, mytime_s.RTC_Minutes, mytime_s.RTC_Seconds);
            return true;
        }
    }
    printf("SNTP Parse Failed!\r\n");
    return false;
}

/**
 * @brief ESP8266 初始化与 MQTT 建连流程
 */
void ESP8266_Init(void)
{
    ESP8266_memset_RecvBuff();

    // 1. 正确退出透传模式
    ESP8266_ExitUnvarnished();

    // 测试 AT
    while (ESP8266_SendCmd("AT\r\n", "OK")) { ESP8266_DelayMs(500); }

    // 2. 设置 STA 模式
    while (ESP8266_SendCmd("AT+CWMODE_CUR=1\r\n", "OK")) { ESP8266_DelayMs(200); }

    // 3. 连接 Wi-Fi
    while (ESP8266_JoinAP() != 0) { 
        printf("WiFi Connect Fail, retrying...\r\n");
        ESP8266_DelayMs(2000); 
    }

    // 4. 连接网络后立即同步 RTC 时间
    ESP8266_Sync_RTC_Time();

    // 5. 连接华为云 TCP 服务器
    while (ESP8266_SendCmd(ESP8266_SERVER, "CONNECT")) { ESP8266_DelayMs(1000); }

    // 6. 开启透传模式
    while (ESP8266_SendCmd("AT+CIPMODE=1\r\n", "OK")) { ESP8266_DelayMs(200); }
    
    // 发送开启透传数据指令 (只要匹配到 OK 或 > 即可)
    ESP8266_memset_RecvBuff();
    Usart_SendString(USART3, "AT+CIPSEND\r\n");
    ESP8266_DelayMs(500); // 必须给予足够时间进入透传状态

    // 7. MQTT 登录 (发送 CONNECT 并等待回应，增加容错，避免死循环重发)
    uint8_t mqtt_retry = 0;
    while (1) 
    {
        ESP8266_memset_RecvBuff();
        MQTT_Connect();
        
        // 给予最长 2 秒等待服务器返回 CONNACK (0x20 0x02)
        uint16_t wait_time = 200; 
        bool conn_ok = false;
        while (wait_time--) 
        {
            if (cnt >= 2 && recv_buf[0] == 0x20 && recv_buf[1] == 0x02) {
                conn_ok = true;
                break;
            }
            ESP8266_DelayMs(10);
        }

        if (conn_ok) {
            printf("MQTT Connect Success!\r\n");
            break;
        }

        mqtt_retry++;
        printf("MQTT Connect Fail (%d), Retrying...\r\n", mqtt_retry);
        ESP8266_DelayMs(1000);
    }

    // 8. MQTT 订阅 Topic
    while (1) 
    {
        ESP8266_memset_RecvBuff();
        MQTT_SubscribeTopic();

        uint16_t wait_time = 200; 
        bool sub_ok = false;
        while (wait_time--) 
        {
            if (cnt >= 2 && recv_buf[0] == 0x90 && recv_buf[1] == 0x03) {
                sub_ok = true;
                break;
            }
            ESP8266_DelayMs(10);
        }

        if (sub_ok) {
            printf("MQTT Subscribe Success!\r\n");
            break;
        }

        ESP8266_DelayMs(1000);
    }
    
    printf("ESP8266 & MQTT Init Complete!\r\n");
}

/**
 * @brief 串口3中断服务函数
 */
void USART3_IRQHandler(void)
{
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        uint8_t res = USART_ReceiveData(USART3);
        
        if (cnt < sizeof(recv_buf) - 1) {
            recv_buf[cnt++] = res;
        }

        // 回显调试
        USART_SendData(USART1, res);

        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    }
}
