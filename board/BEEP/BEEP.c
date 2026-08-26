#include "BEEP.h"

// 《小星星》音调谱
const uint16_t Star_Music[] = {
    H1, H1, H5, H5, H6, H6, H5,
    H4, H4, H3, H3, H2, H2, H1,
    H5, H5, H4, H4, H3, H3, H2,
    H5, H5, H4, H4, H3, H3, H2,
    H1, H1, H5, H5, H6, H6, H5,
    H4, H4, H3, H3, H2, H2, H1
};

// 《小星星》节拍谱 (单位: 毫秒，400ms为一拍，800ms为长拍)
const uint16_t Star_Time[] = {
    400, 400, 400, 400, 400, 400, 800,
    400, 400, 400, 400, 400, 400, 800,
    400, 400, 400, 400, 400, 400, 800,
    400, 400, 400, 400, 400, 400, 800,
    400, 400, 400, 400, 400, 400, 800,
    400, 400, 400, 400, 400, 400, 800
};

// 乐谱定义：音调
const uint16_t TwoTigers_Music[] = {
    H1, H2, H3, H1,  H1, H2, H3, H1, 
    H3, H4, H5,      H3, H4, H5,
    H5, H6, H5, H4, H3, H1,   H5, H6, H5, H4, H3, H1,
    H2, H5, H1,      H2, H5, H1
};

// 节拍定义：数字表示相对时长 (4 = 1拍, 2 = 0.5拍)
const uint16_t TwoTigers_Time[] = {
    400, 400, 400, 400,  400, 400, 400, 400,
    400, 400, 800,     400, 400, 800,
    300, 100, 300, 100, 400, 400,   300, 100, 300, 100, 400, 400,
    400, 400, 800,     400, 400, 800
};

//通用定时器 TIM2 ~ 5
void BEEP_config(void){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);	//使能GPIO时钟
	GPIO_InitTypeDef mygpio = {0};
	mygpio.GPIO_Mode = GPIO_Mode_AF;	//工作模式: 复用
	mygpio.GPIO_OType = GPIO_OType_PP;
	mygpio.GPIO_Pin = BEEP_PIN;
	mygpio.GPIO_PuPd = GPIO_PuPd_UP;
	mygpio.GPIO_Speed = GPIO_High_Speed;
	GPIO_Init(BEEP_GROUP,&mygpio);
	
	//把 PA5 复用成 TIM2
	GPIO_PinAFConfig(BEEP_GROUP, GPIO_PinSource5, GPIO_AF_TIM2);
	
	//T = (ARR + 1)(PSC + 1)/84 000 000
	TIM_TimeBaseInitTypeDef mytim = {0};
	mytim.TIM_ClockDivision = TIM_CKD_DIV1;	//预分频,滤波(这里选不分频)
	mytim.TIM_CounterMode = TIM_CounterMode_Up;	//向上计数 0++
	mytim.TIM_Period = 10000 - 1;	//0 ~ 65535  ARR 数多少个数 (自动重装载寄存器)
	mytim.TIM_Prescaler = 42 - 1;//0 ~ 65535  PSC 计数的快慢(分频值)
	TIM_TimeBaseInit(TIM2, &mytim);
	
	TIM_OCInitTypeDef mybeep = {0};
	mybeep.TIM_OCMode = TIM_OCMode_PWM1;	//选择PWM模式1
	mybeep.TIM_OutputState = TIM_OutputState_Enable;	//输出使能
	mybeep.TIM_Pulse = 0;	//CCR 默认为 0 ,不响
	mybeep.TIM_OCPolarity = TIM_OCPolarity_High;	//高电平有效
	TIM_OC1Init(TIM2, &mybeep);	//OC1:通道一
	
	TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);	//开启预装载配置 CCR
	TIM_ARRPreloadConfig(TIM2, ENABLE);	//开启预装载配置 CCR
	
	TIM_Cmd(TIM2, ENABLE);
}

#include "BEEP.h"

// 1. 修改 BEEP_PlayNote 函数：直接传入频率 freq_hz 和持续时间 duration_ms
void BEEP_PlayNote(uint32_t freq_hz, uint16_t duration_ms) {
    if (freq_hz == SOUND_OFF || freq_hz == 0) {
        // 休止符：关闭输出
        BEEP_SetFrequency(0, 0);
    } else {
        // 计算 ARR 值：ARR = (2000000 / freq_hz) - 1
        uint32_t arr = (2000000 / freq_hz) - 1;
        
        // 调用真实的频率设置函数，传入目标频率 freq_hz，并将占空比设为 50% (即 sound = (arr + 1) / 2)
        // 50% 占空比下蜂鸣器发出的声音最响亮、最清脆
        BEEP_SetFrequency(freq_hz, (arr + 1) / 2);
    }

    // 播放持续时间
    delay_ms(duration_ms);

    // 【关键】音符间断音（20ms），防止连续相同音符黏连
    BEEP_SetFrequency(0, 0);
    delay_ms(20); 
}

// 2. 真实频率设置函数（保持并优化）
void BEEP_SetFrequency(uint32_t freq_hz, uint32_t sound) {
    if (freq_hz == 0) {
        TIM_SetCompare1(TIM2, 0); // 关闭输出
        return;
    }

    // 定时器输入时钟为 84MHz，PSC = 42-1 (计数频率为 2MHz)
    // 周期 ARR = (2000000 / freq_hz) - 1
    uint32_t arr = (2000000 / freq_hz) - 1;
    
    TIM_SetAutoreload(TIM2, arr);     // 改变 PWM 频率（音调）
    TIM_SetCompare1(TIM2, sound);     // 设置 PWM 占空比（音量/发声）
}

// 乐谱播放逻辑保持不变
void Play_Star(void) {
    uint16_t length = sizeof(Star_Music) / sizeof(Star_Music[0]);
    for (uint16_t i = 0; i < length; i++) {
        BEEP_PlayNote(Star_Music[i], Star_Time[i]);
    }
}

void Play_TwoTigers(void) {
    uint16_t length = sizeof(TwoTigers_Music) / sizeof(TwoTigers_Music[0]);
    for (uint16_t i = 0; i < length; i++) {
        BEEP_PlayNote(TwoTigers_Music[i], TwoTigers_Time[i]);
    }
}

//TIM_SetCompare1(TIM2, 5); //动态修改CCR
/*
	1s 定时
	T = PSC 和 ARR 有关系
	STM32F407  84MHz -> 1s 能数 84 000 000 个数,一个数 1 / 84 000 000
	1.	新的时钟的频率 1s 84 000 000Hz / (PSC + 1)  个数
	2.  1个数古(PSC + 1)(PSC + 1)/84 000 000
	3.  T = (ARR + 1)(PSC + 1)/84 000 000

	PSC 较大 适合长时间计数
	ARR 较大 适合短时间高精度计数

*/
