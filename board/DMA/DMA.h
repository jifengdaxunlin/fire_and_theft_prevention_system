#ifndef __DMA_H__
#define __DMA_H__

#include "stm32f4xx.h"

void USART1_DMA_ENABLE(uint32_t date_val);
void DMA_config(uint32_t date_val, uint32_t memory_add);

#endif
