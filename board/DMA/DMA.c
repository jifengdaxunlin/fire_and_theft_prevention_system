#include "DMA.h"

void DMA_config(uint32_t date_val, uint32_t memory_add){
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);	//使能时钟(手册有问题)
	
	DMA_InitTypeDef mydma = {0};
	//DMA_StructInit()  未配置则设为默认值
	mydma.DMA_Channel = DMA_Channel_4;	//选择通道
	mydma.DMA_DIR = DMA_DIR_MemoryToPeripheral;	//数据传送方向,内存->外设
	mydma.DMA_Mode = DMA_Mode_Normal;	//传输模式  传一次就停止(一直传输)
	mydma.DMA_BufferSize = date_val;	//数据传输量的大小(函数传进来的值)(单次传输的数据)
	mydma.DMA_FIFOMode = DMA_FIFOMode_Disable;	//是否开启 FIFO 模式(FOFO里的数据到达 1/4,1/2,3/4,4/4 才发送)
	//mydma.DMA_FIFOThreshold = 
	//内存配置
	mydma.DMA_Memory0BaseAddr = memory_add;	//要搬运的数据在内存中的地址
	mydma.DMA_MemoryBurst = DMA_MemoryBurst_Single;	//一次中断发送几个 8bit 数据
	mydma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;	//单次发送多少数据 8bit/16/32
	mydma.DMA_MemoryInc = DMA_MemoryInc_Enable;		//是否开启增量模式(内存需要下移读取连续的数据)
	//外设配置
	mydma.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;	//搬运的数据到达的地址
	mydma.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;	//一次中断发送几个 8bit 数据
	mydma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;	//单次接受多少数据
	mydma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	//串口发送完数据后自动清空(无需下移)
	mydma.DMA_Priority = DMA_Priority_VeryHigh;	//DMA 传输数据的优先级
	DMA_Init(DMA2_Stream7, &mydma);
	
	NVIC_InitTypeDef mynvic = {0};
	mynvic.NVIC_IRQChannel = DMA2_Stream7_IRQn; //中断通道
	mynvic.NVIC_IRQChannelCmd = ENABLE; //使能
	mynvic.NVIC_IRQChannelPreemptionPriority = 1;  //抢占优先级 0~15
	mynvic.NVIC_IRQChannelSubPriority = 4;		//响应优先级 0~15
	NVIC_Init(&mynvic);
	
	DMA_ITConfig(DMA2_Stream7, DMA_IT_TC | DMA_IT_HT | DMA_IT_TE, ENABLE);
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
}

//  修改一些DMA参数, 开启DMA
void USART1_DMA_ENABLE(uint32_t date_val){
	DMA_Cmd(DMA2_Stream7,DISABLE);	//关闭DMA
	while(DMA_GetCmdStatus(DMA2_Stream7) != DISABLE);	//确认关闭完成
	DMA_SetCurrDataCounter(DMA2_Stream7, date_val);	//修改 重置 本次发送的数据量
	DMA_ClearFlag(DMA2_Stream7, DMA_FLAG_TCIF7);
	/*
	 @arg DMA_FLAG_TCIFx:  传输完成
     @arg DMA_FLAG_HTIFx:  传输一半
     @arg DMA_FLAG_TEIFx:  传输错误
	*/
	DMA_Cmd(DMA2_Stream7,ENABLE);	//开启DMA
}

void DMA2_Stream7_IRQHandler(void){
	if(DMA_GetITStatus(DMA2_Stream7, DMA_IT_TCIF7) == SET){
		USART1_DMA_ENABLE(sizeof("I love you!\r\n")-1);	//重新调用发送的使能，传入要发送的数据大小
		DMA_ClearITPendingBit(DMA2_Stream7, DMA_IT_TCIF7);
	}
}
