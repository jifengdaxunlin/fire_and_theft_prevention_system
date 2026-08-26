#include "mqtt.h"

uint8_t MQTT_RX_BUF[512]; // MQTT数据缓冲区，用于发送和接收
uint16_t DataLen;

volatile uint16_t MQTT_Len;

/**
 * @brief 清空接收缓冲区数据
 *
 * @return 无
 */
void MQTT_Clear(void)
{
    DataLen = 0;
    MQTT_Len = 0;
    memset(MQTT_RX_BUF, 0, sizeof(MQTT_RX_BUF)); // 清空接收缓冲区
}

/**
 * @brief 通过MQTT发送服务器数据
 *
 * @return 无
 */
void MQTT_SendData(uint8_t* buf, uint16_t len)
{
    Usart_SendArray(USART3, buf, len);
}

/**
 * @brief MQTT发送心跳包指令
 *
 * @return 无
 */
void MQTT_SendHeart(void)
{
    MQTT_Clear();
    /*
        心跳包是检测 MQTT 连接是否正常的固定头部报文（PINGREQ）。
        心跳包报文的固定头部由两个字节组成：0xc0，
        表示报文类型为 PINGREQ（心跳包），且 DUP 标志为 0，QoS 等级为 0，保留位为 0。
    */
    MQTT_RX_BUF[0] = 0xc0; 
    /*
        心跳包报文的剩余长度：
        对于这种心跳包报文，没有可变头部和负载部分，因此剩余长度为 0。
    */
    MQTT_RX_BUF[1] = 0x00; 
    MQTT_SendData(MQTT_RX_BUF, 2);
}

/**
 * @brief MQTT指令登录服务器
 *
 * @return 无
 * 与华为云建立连接
 */
void MQTT_Connect(void)
{
    MQTT_Clear();
    
    /*******************先要计算的长度计算开始***************/
    //  10 + MQTT_ClientID_Len + 2 + MQTT_UserName_Len + 2 + MQTT_Password_Len + 2 = 
    uint8_t MQTT_ClientID_Len = strlen(MQTT_Client_ID);
    uint8_t MQTT_UserName_Len = strlen(MQTT_User_Name);
    uint8_t MQTT_Password_Len = strlen(MQTT_Password);

    // 可变报头(长度固定10) + Payload，每个字段都包含两个字节的长度标识
    DataLen = 10 + (MQTT_ClientID_Len + 2) + (MQTT_UserName_Len + 2) + (MQTT_Password_Len + 2);
    // 123     512    964     79321
    // 1111111 11111111  111111111
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    
    /*固定报头*/
    MQTT_RX_BUF[MQTT_Len++] = 0x10; // MQTT_RX_BUF[0] = 0x10;
    // 计算剩余长度~~~~~~~~~~~~~~~~~
    do
    {
        uint8_t encodedByte = DataLen % 128;
        DataLen = DataLen / 128;
        if (DataLen > 0)
            encodedByte = encodedByte | 128;
        MQTT_RX_BUF[MQTT_Len++] = encodedByte;   // 剩余长度 可变报头 + 有效载荷
    } while (DataLen > 0);
    
    /*************可变报头 长度10 ****************/
    MQTT_RX_BUF[MQTT_Len++] = 0x00; // MSB(0)
    MQTT_RX_BUF[MQTT_Len++] = 0x04; // LSB(4)
    MQTT_RX_BUF[MQTT_Len++] = 0x4d; // M
    MQTT_RX_BUF[MQTT_Len++] = 0x51; // Q
    MQTT_RX_BUF[MQTT_Len++] = 0x54; // T
    MQTT_RX_BUF[MQTT_Len++] = 0x54; // T
    MQTT_RX_BUF[MQTT_Len++] = 0x04; // Level(4)
    MQTT_RX_BUF[MQTT_Len++] = 0xc2; // 开启 User Name Flag, Password Flag, Clean Session
    MQTT_RX_BUF[MQTT_Len++] = 0x00; // 保活时间 MSB (0)
    MQTT_RX_BUF[MQTT_Len++] = 0x64; // 保活时间 LSB (100)  100s
    
    /*有效载荷*/
    MQTT_RX_BUF[MQTT_Len++] = 0x00;                      // Client ID 的 MSB    
    MQTT_RX_BUF[MQTT_Len++] = BYTE0(MQTT_ClientID_Len);  // Client ID 的 LSB      
    memcpy(&MQTT_RX_BUF[MQTT_Len], MQTT_Client_ID, MQTT_ClientID_Len); // Client ID 的数据    
    MQTT_Len += MQTT_ClientID_Len;
    
    /********************用户的用户名数据********************/
    MQTT_RX_BUF[MQTT_Len++] = 0x00;                                // 用户名的 MSB   
    MQTT_RX_BUF[MQTT_Len++] = BYTE0(MQTT_UserName_Len);            // 用户名的 LSB    
    memcpy(&MQTT_RX_BUF[MQTT_Len], MQTT_User_Name, MQTT_UserName_Len); // 用户名的数据
    MQTT_Len += MQTT_UserName_Len;
    
    /********************用户的密码数据********************/
    MQTT_RX_BUF[MQTT_Len++] = 0x00;                                // 密码的 MSB    
    MQTT_RX_BUF[MQTT_Len++] = BYTE0(MQTT_Password_Len);            // 密码的 LSB  
    memcpy(&MQTT_RX_BUF[MQTT_Len], MQTT_Password, MQTT_Password_Len); // 密码的数据
    MQTT_Len += MQTT_Password_Len;   
    
    MQTT_SendData(MQTT_RX_BUF, MQTT_Len); // 发送对应的数据到服务器进行登录
}

/**
 * @brief MQTT指令断开服务器连接
 *
 * @return 无
 */
void MQTT_Disconnect(void)
{
    MQTT_Clear();
    MQTT_RX_BUF[0] = 0xe0; 
    MQTT_RX_BUF[1] = 0x00; 
    // USART_SendArray(USART3, ARR)
    MQTT_SendData(MQTT_RX_BUF, 2); // 发送对应的数据到服务器进行断开连接
}

/**
 * @brief 通过MQTT发布对应数据到服务器
 * @param humi 湿度数据
 * @param temp 温度数据
 * @return 无
 */
// void MQTT_PublicTopic(float temp, float humi) 
void MQTT_PublicTopic(float humi, float temp, float fire, float gas, bool fan, bool gate, bool water, AlarmType led, bool stranger)
{
    char SendData[512];
    char time_buf[20];
    RTC_TimeTypeDef mytime = {0};
    RTC_DateTypeDef mydate = {0};

    MQTT_Clear();

    RTC_GetTime(RTC_Format_BIN, &mytime);
    RTC_GetDate(RTC_Format_BIN, &mydate);

    snprintf(time_buf, sizeof(time_buf), "20%02dT%02d%02d%02d%02d%02dZ",
             mydate.RTC_Year, mydate.RTC_Month, mydate.RTC_Date,
             mytime.RTC_Hours, mytime.RTC_Minutes, mytime.RTC_Seconds);

    snprintf(SendData, sizeof(SendData), 
        "{\"services\":[{\"service_id\":\"jifengdaxunlin\",\"properties\":{"
        "\"humi\":%.2f,\"temp\":%.2f,\"fire\":%.2f,\"gas\":%.2f,"
        "\"fan\":%d,\"gate\":%d,\"water\":%d,\"led\":%d,\"stranger\":%d},"
        "\"event_time\":\"%s\"}]}",
        humi, temp, fire, gas, fan, gate, water, led, stranger, time_buf);

    uint8_t MQTT_PublishTopic_Len = strlen(PublishTopic); 
    uint8_t MQTT_MQTTPUBLISH_Len  = strlen(SendData);  

    // 打印当前的 Payload Payload 长度与内容
    printf("[MQTT] Payload Len: %d | Time: %s\r\n", MQTT_MQTTPUBLISH_Len, time_buf);

    MQTT_RX_BUF[MQTT_Len++] = 0x30;
    DataLen = (MQTT_PublishTopic_Len + 2) + MQTT_MQTTPUBLISH_Len;
    do {
        uint8_t encodedByte = DataLen % 128;
        DataLen = DataLen / 128;
        if (DataLen > 0) encodedByte |= 128;
        MQTT_RX_BUF[MQTT_Len++] = encodedByte;
    } while (DataLen > 0);

    MQTT_RX_BUF[MQTT_Len++] = 0x00;
    MQTT_RX_BUF[MQTT_Len++] = BYTE0(MQTT_PublishTopic_Len);
    memcpy(&MQTT_RX_BUF[MQTT_Len], PublishTopic, MQTT_PublishTopic_Len);
    MQTT_Len += MQTT_PublishTopic_Len;

    memcpy(&MQTT_RX_BUF[MQTT_Len], SendData, MQTT_MQTTPUBLISH_Len);
    MQTT_Len += MQTT_MQTTPUBLISH_Len;

    // 检查是否超出缓冲区
    if (MQTT_Len > sizeof(MQTT_RX_BUF)) {
        printf("[MQTT_ERR] Buffer Overflow! MQTT_Len(%d) > BUF_SIZE(512)\r\n", MQTT_Len);
        return;
    }

    MQTT_SendData(MQTT_RX_BUF, MQTT_Len);
    printf("[MQTT] %d Bytes Sent to USART3.\r\n", MQTT_Len);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/**
 * @brief MQTT指令订阅服务器主题
 *
 * @return 无
 */
void MQTT_SubscribeTopic(void)
{
    MQTT_Clear();
    uint8_t MQTT_SubscribeTopic_Len = strlen(SubscribeTopic_Server); // 计算订阅字符串的长度
    
    DataLen = 2 + (MQTT_SubscribeTopic_Len + 2) + 1;
    
    /*固定报头*/
    MQTT_RX_BUF[MQTT_Len++] = 0x82;
    /*剩余长度*/
    do
    {
        uint8_t encodedByte = DataLen % 128;
        DataLen = DataLen / 128;
        if (DataLen > 0)
            encodedByte = encodedByte | 128;
        MQTT_RX_BUF[MQTT_Len++] = encodedByte;
    } while (DataLen > 0);
    
    /*可变报头*/
    MQTT_RX_BUF[MQTT_Len++] = 0x00;  // 用户自定义标识 
    MQTT_RX_BUF[MQTT_Len++] = 0x01;  // 用户自定义标识
    
    MQTT_RX_BUF[MQTT_Len++] = 0x00;  // 订阅的 MSB
    MQTT_RX_BUF[MQTT_Len++] = MQTT_SubscribeTopic_Len;  // 订阅的 LSB
    memcpy(&MQTT_RX_BUF[MQTT_Len], SubscribeTopic_Server, MQTT_SubscribeTopic_Len);  // 写入订阅字符串的路径
    MQTT_Len += MQTT_SubscribeTopic_Len;
    MQTT_RX_BUF[MQTT_Len++] = 0x00; // 订阅字符串的等级为 Q0
    
    MQTT_SendData(MQTT_RX_BUF, MQTT_Len);  // 发送MQTT订阅指令到服务器
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
void MQTT_SubscribeTopic_web(void)
{
    MQTT_Clear();
    uint8_t MQTT_SubscribeTopic_Len = strlen(SubscribeTopic_Server_web); // 计算订阅字符串的长度
    
    DataLen = 2 + (MQTT_SubscribeTopic_Len + 2) + 1;
    
    MQTT_RX_BUF[MQTT_Len++] = 0x82;
    
    do
    {
        uint8_t encodedByte = DataLen % 128;
        DataLen = DataLen / 128;
        if (DataLen > 0)
            encodedByte = encodedByte | 128;
        MQTT_RX_BUF[MQTT_Len++] = encodedByte;
    } while (DataLen > 0);
    
    MQTT_RX_BUF[MQTT_Len++] = 0x00;  // 用户自定义标识 
    MQTT_RX_BUF[MQTT_Len++] = 0x01;  // 用户自定义标识
    MQTT_RX_BUF[MQTT_Len++] = 0x00;  // 订阅的 MSB

    MQTT_RX_BUF[MQTT_Len++] = MQTT_SubscribeTopic_Len;  // 订阅的 LSB
    memcpy(&MQTT_RX_BUF[MQTT_Len], SubscribeTopic_Server_web, MQTT_SubscribeTopic_Len);  // 写入订阅字符串的路径
    MQTT_Len += MQTT_SubscribeTopic_Len;
    MQTT_RX_BUF[MQTT_Len++] = 0x00; // 订阅字符串的等级为 Q0
    
    MQTT_SendData(MQTT_RX_BUF, MQTT_Len);  // 发送MQTT订阅指令到服务器
}
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/**
 * @brief MQTT指令取消订阅对应的主题
 *
 * @return 无
 */
void MQTT_UNSubscribeTopic(void)
{
    MQTT_Clear();
    uint8_t MQTT_SubscribeTopic_Len = strlen(SubscribeTopic_Server_Reply); // 计算取消订阅字符串的长度

    DataLen = 2 + (MQTT_SubscribeTopic_Len + 2);
    
    /*固定报头*/
    MQTT_RX_BUF[MQTT_Len++] = 0xA2;
    /*剩余长度*/
    do
    {
        uint8_t encodedByte = DataLen % 128;
        DataLen = DataLen / 128;
        if (DataLen > 0)
            encodedByte = encodedByte | 128;
        MQTT_RX_BUF[MQTT_Len++] = encodedByte;
    } while (DataLen > 0);
    
    /*可变报头*/
    MQTT_RX_BUF[MQTT_Len++] = 0x00;  // 用户自定义标识
    MQTT_RX_BUF[MQTT_Len++] = 0x01;  // 用户自定义标识
    
    MQTT_RX_BUF[MQTT_Len++] = 0x00;  // 订阅的 MSB
    
    MQTT_RX_BUF[MQTT_Len++] = MQTT_SubscribeTopic_Len;  // 取消订阅的 LSB
    memcpy(&MQTT_RX_BUF[MQTT_Len], SubscribeTopic_Server_Reply, MQTT_SubscribeTopic_Len);  // 写入取消订阅字符串的路径
    MQTT_Len += MQTT_SubscribeTopic_Len;
    
    MQTT_SendData(MQTT_RX_BUF, MQTT_Len);  // 发送MQTT订阅指令到服务器
}
