#ifndef __BEEP_H__
#define __BEEP_H__

#include "DELAY.h"
#include "stm32f4xx.h"

// 假设定时器时钟 84MHz，PSC = 84 - 1 (计数频率为 1MHz)
// 频率计算公式: ARR = 1,000,000 / 目标频率 - 1

// 0 表示休止符（不发声）
#define SOUND_OFF 0

// 低音 (Low)
#define L1 262
#define L2 294
#define L3 330
#define L4 349
#define L5 392
#define L6 440
#define L7 494

// 中音 (Middle)
#define M1 523
#define M2 587
#define M3 659
#define M4 698
#define M5 784
#define M6 880
#define M7 988

// 高音 (High)
#define H1 1047
#define H2 1175
#define H3 1319
#define H4 1397
#define H5 1568
#define H6 1760
#define H7 1976

#define BEEP_GROUP GPIOA
#define BEEP_PIN GPIO_Pin_5

void BEEP_config(void);
void Play_Star(void);
void Play_TwoTigers(void);
void BEEP_SetFrequency(uint32_t freq_hz, uint32_t sound);
void BEEP_PlayNote(uint32_t freq_hz, uint16_t duration_ms);

#endif
