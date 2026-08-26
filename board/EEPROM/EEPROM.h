#ifndef __EEPROM_H__
#define __EEPROM_H__

#include "stm32f4xx.h"
#include "IIC.h"
#include <stdio.h>

typedef struct{
    uint8_t humi; // 湿度阈值 (%)
    uint8_t temp; // 温度阈值 (℃)
    uint8_t gas;  // 气体阈值 (%)
    float fire;   // 火焰电阻阈值 (kΩ)
}Threshold;

void EEPROM_config(void);
uint8_t EEPROM_Write_Byte(uint8_t add, uint8_t data);
uint8_t EEPROM_PageWrite(uint8_t pageAdd, uint8_t datasize, uint8_t* dataptr);
uint8_t EEPROM_ReadCurrAdd(void);
uint8_t EEPROM_ReadByte(uint8_t Add);
void EEPROM_Test(void);

// 新增：结构体批量存取接口
void EEPROM_Save_Threshold(Threshold *t);
void EEPROM_Load_Threshold(Threshold *t);

#endif
