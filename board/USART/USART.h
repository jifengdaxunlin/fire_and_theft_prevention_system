#ifndef __USART_H__
#define	__USART_H__

#include "stm32f4xx.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

extern bool stranger_detected;
extern uint16_t stranger_timeout_cnt;

void USART1_config(uint32_t BaudRate);
void USART2_config(uint32_t BaudRate);
void USART3_config(uint32_t BaudRate);
void Usart_SendHalfWord( USART_TypeDef * pUSARTx, uint16_t ch);

void Usart_SendString(USART_TypeDef *pUSARTx, char *str);
void Usart_SendArray( USART_TypeDef * pUSARTx, uint8_t *array, uint16_t num);
void Usart_SendBytes(USART_TypeDef * pUSARTx, uint8_t *buf,uint32_t len);
void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch);

#endif
