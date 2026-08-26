#ifndef __IIC_H__
#define __IIC_H__

#include "stm32f4xx.h"
#include "DELAY.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define SCL_CLK	RCC_AHB1Periph_GPIOB
#define SCL_PIN	GPIO_Pin_8
#define SCL_GROUP	GPIOB

#define SDA_CLK	RCC_AHB1Periph_GPIOB
#define SDA_PIN	GPIO_Pin_9
#define SDA_GROUP	GPIOB

#define IIC_SCL_SET(x) (x) ? (GPIO_SetBits(SCL_GROUP, SCL_PIN)) : (GPIO_ResetBits(SCL_GROUP, SCL_PIN))
#define IIC_SDA_SET(x) (x) ? (GPIO_SetBits(SDA_GROUP, SDA_PIN)) : (GPIO_ResetBits(SDA_GROUP, SDA_PIN))
#define IIC_SDA_READ	GPIO_ReadInputDataBit(SDA_GROUP, SDA_PIN)

extern SemaphoreHandle_t xI2C_Mutex;

void IIC_SDA_Out(void);
void IIC_SDA_In(void);
void IIC_config(void);
void IIC_Start(void);
void IIC_Stop(void);
void IIC_SendByte(uint8_t data);
uint8_t IIC_ReadByte(void);
uint8_t IIC_SlaveAck(void);
void IIC_MasterAck(uint8_t ack);

#endif
