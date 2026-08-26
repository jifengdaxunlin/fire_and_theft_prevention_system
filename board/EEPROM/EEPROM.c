#include "EEPROM.h"

#define EEPROM_DEV_ADDR     0xA0
#define EEPROM_PAGE_SIZE    64    // AT24C256 每页 64 字节
#define EEPROM_MAGIC_ADD    0x0000
#define EEPROM_DATA_ADD     0x0001
#define MAGIC_NUMBER        0x5A

static uint8_t EEPROM_Wait_StandbyState(void) 
{
    uint8_t ack = 1;
    uint16_t timeout = 1000;

    do {
        IIC_Start();
        IIC_SendByte(EEPROM_DEV_ADDR);
        ack = IIC_SlaveAck();
        IIC_Stop();
        
        if (ack == 0) break; // 收到 ACK 说明内部写入完毕
        
        if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
            vTaskDelay(pdMS_TO_TICKS(1));
        } else {
            delay_us(500);
        }
    } while (timeout--);

    return ack;
}

// 页内连续写入（16位地址）
static uint8_t EEPROM_Write_Page_Direct(uint16_t memAdd, uint8_t datasize, const uint8_t* dataptr) 
{
    IIC_Start();
    IIC_SendByte(EEPROM_DEV_ADDR);
    if (IIC_SlaveAck() == 1) { IIC_Stop(); return 1; }

    IIC_SendByte((uint8_t)(memAdd >> 8));     // 地址高字节
    if (IIC_SlaveAck() == 1) { IIC_Stop(); return 2; }

    IIC_SendByte((uint8_t)(memAdd & 0xFF));   // 地址低字节
    if (IIC_SlaveAck() == 1) { IIC_Stop(); return 3; }

    while (datasize--) {
        IIC_SendByte(*dataptr++);
        if (IIC_SlaveAck() == 1) { IIC_Stop(); return 4; }
    }

    IIC_Stop();
    return EEPROM_Wait_StandbyState();
}

// 跨页安全写入（16位地址）
uint8_t EEPROM_Write_Buffer(uint16_t add, uint16_t len, const uint8_t* pBuffer) 
{
    uint8_t err = 0;

    if (xSemaphoreTake(xI2C_Mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return 0xFF; 
    }

    while (len > 0) {
        uint8_t page_offset = add % EEPROM_PAGE_SIZE;
        uint8_t bytes_to_write = EEPROM_PAGE_SIZE - page_offset;
        
        if (bytes_to_write > len) {
            bytes_to_write = len;
        }

        err = EEPROM_Write_Page_Direct(add, bytes_to_write, pBuffer);
        if (err != 0) {
            xSemaphoreGive(xI2C_Mutex);
            return err;
        }

        len -= bytes_to_write;
        add += bytes_to_write;
        pBuffer += bytes_to_write;
    }

    xSemaphoreGive(xI2C_Mutex);
    return 0;
}

// 连续读取（16位地址）
uint8_t EEPROM_Read_Buffer(uint16_t add, uint16_t len, uint8_t* pBuffer) 
{
    if (xSemaphoreTake(xI2C_Mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return 0xFF;
    }

    IIC_Start();
    IIC_SendByte(EEPROM_DEV_ADDR);
    if (IIC_SlaveAck() == 1) { IIC_Stop(); xSemaphoreGive(xI2C_Mutex); return 1; }

    IIC_SendByte((uint8_t)(add >> 8));       // 地址高字节
    if (IIC_SlaveAck() == 1) { IIC_Stop(); xSemaphoreGive(xI2C_Mutex); return 2; }

    IIC_SendByte((uint8_t)(add & 0xFF));     // 地址低字节
    if (IIC_SlaveAck() == 1) { IIC_Stop(); xSemaphoreGive(xI2C_Mutex); return 3; }

    IIC_Start();
    IIC_SendByte(EEPROM_DEV_ADDR | 0x01);
    if (IIC_SlaveAck() == 1) { IIC_Stop(); xSemaphoreGive(xI2C_Mutex); return 4; }

    for (uint16_t i = 0; i < len; i++) {
        pBuffer[i] = IIC_ReadByte();
        IIC_MasterAck((i == (len - 1)) ? 1 : 0);
    }

    IIC_Stop();
    xSemaphoreGive(xI2C_Mutex);
    return 0;
}

/**
  * @brief  保存 Threshold 结构体
  */
void EEPROM_Save_Threshold(Threshold *t) 
{
    uint8_t magic = MAGIC_NUMBER;
    
    EEPROM_Write_Buffer(EEPROM_MAGIC_ADD, 1, &magic);
    EEPROM_Write_Buffer(EEPROM_DATA_ADD, sizeof(Threshold), (uint8_t *)t);
    
    printf("EEPROM: Threshold Saved Successfully!\r\n");
}

/**
  * @brief  加载 Threshold 结构体
  */
void EEPROM_Load_Threshold(Threshold *t) 
{
    uint8_t magic = 0;
    EEPROM_Read_Buffer(EEPROM_MAGIC_ADD, 1, &magic);

    if (magic != MAGIC_NUMBER) {
        printf("EEPROM: First boot, loading default thresholds...\r\n");
        t->humi = 80;
        t->temp = 35;
        t->gas  = 50;
        t->fire = 10.0f;

        EEPROM_Save_Threshold(t);
    } else {
        EEPROM_Read_Buffer(EEPROM_DATA_ADD, sizeof(Threshold), (uint8_t *)t);
    }
}
