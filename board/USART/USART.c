#include "USART.h"


//串口printf打印端口的选择
#define USARTX USART1
//#define USARTX USART2
//#define USARTX USART3


void USART1_config(uint32_t BaudRate)
{
	GPIO_InitTypeDef GPIO_InitStruct;	
	USART_InitTypeDef USART_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
													   
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;	//UART1_TXD
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_High_Speed; 
	GPIO_Init(GPIOA, &GPIO_InitStruct);	

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;	//UART1_RXD
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);	//设置复用功能
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
	
	USART_InitStruct.USART_BaudRate = BaudRate;	//波特率
	USART_InitStruct.USART_Parity	= USART_Parity_No;	//无检验
	USART_InitStruct.USART_WordLength	= USART_WordLength_8b;	//8位字长
	USART_InitStruct.USART_StopBits	= USART_StopBits_1;	//1位停止位
	USART_InitStruct.USART_HardwareFlowControl	= USART_HardwareFlowControl_None;	//无硬件流控
	USART_InitStruct.USART_Mode	= USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART1, &USART_InitStruct);
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);	//开启串口接收中断
	
	NVIC_InitStruct.NVIC_IRQChannel	= USART1_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd	= ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority	= 6;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority	= 0;
	NVIC_Init(&NVIC_InitStruct);	//配置NVIC
	
	USART_Cmd(USART1, ENABLE);	//开启串口
}

/***串口二****/
void USART2_config(uint32_t BaudRate)
{
	/* 定义相关类型的结构体 */
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;

	/* 开启端口引脚时钟 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	/* 开启串口端口时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	/* 配置通信引脚工作方式 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;		  // UART1_TXD
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;	  // 复用功能
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;	  // 推完模式
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;	  // 上拉模式
	GPIO_InitStruct.GPIO_Speed = GPIO_High_Speed; // 输出速度为高
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3; // UART1_RXD
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* 配置引脚的复用功能 */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

	/* 配置串口接口的工作方式 */
	USART_InitStruct.USART_BaudRate = BaudRate;									 // 波特率
	USART_InitStruct.USART_Parity = USART_Parity_No;							 // 无检验
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;					 // 8位字长
	USART_InitStruct.USART_StopBits = USART_StopBits_1;							 // 1位停止位
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件流控
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;				 // 收发模式
	USART_Init(USART2, &USART_InitStruct);

	/* 开启串口接收中断 */
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

	/* 配置串口的中断优先级 */
	NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 6;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStruct);

	/* 开启串口工作 */
	USART_Cmd(USART2, ENABLE);
}

//串口3初始化
void USART3_config(uint32_t BaudRate)
{
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable GPIO clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	/* Enable USART clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	/* Connect USART pins to PB10 \ PB11*/
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);

	/* Configure USART Tx and Rx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOB, &GPIO_InitStructure);


	USART_InitStructure.USART_BaudRate = BaudRate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);


	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable USART */
	USART_Cmd(USART3, ENABLE);
	
	/* Enable the USARTz Receive Interrupt */
	USART_ITConfig(USART3, USART_IT_RXNE , ENABLE);
}



//重定向c库函数printf到串口，重定向后可使用printf函数
int fputc(int ch, FILE *f)
{
	/* 发送一个字节数据到串口 */
	USART_SendData(USARTX, (uint8_t) ch);

	/* 等待发送完毕 */
	while (USART_GetFlagStatus(USARTX, USART_FLAG_TXE) == RESET);		

	return (ch);
}

//重定向c库函数scanf到串口，重写向后可使用scanf、getchar等函数
int fgetc(FILE *f)
{
	/* 等待串口输入数据 */
	while (USART_GetFlagStatus(USARTX, USART_FLAG_RXNE) == RESET);

	return (int)USART_ReceiveData(USARTX);
}


char rxd_data;	//定义变量用于存放接收数据
static char u1_rx_buf[512];	//存放字符串
static uint16_t u1_rx_cnt = 0;	//计数值
//USART1 的中断函数
bool stranger_detected = false;
uint16_t stranger_timeout_cnt = 0;
void USART1_IRQHandler(void){
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET){
		rxd_data = USART_ReceiveData(USART1);
        
        // 防止缓冲区溢出
        if (u1_rx_cnt < sizeof(u1_rx_buf) - 1) {
            u1_rx_buf[u1_rx_cnt++] = rxd_data;
        }

        // 遇到换行符 '\n'，说明上位机的一帧指令发送完毕
        if (rxd_data == '\n' || rxd_data == '\r') {
            u1_rx_buf[u1_rx_cnt] = '\0'; // 补全字符串结束符
            
            // 匹配上位机发送的 "STRANGER" 字符
            if (strstr(u1_rx_buf, "STRANGER") != NULL) stranger_detected = true, stranger_timeout_cnt = 60; // 置位陌生人标志
            
            // 重置缓冲区指针，准备接收下一帧
            u1_rx_cnt = 0;
        }
		
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
}

void USART2_IRQHandler(void)
{
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
		//清楚接收中断标志位
		
		//
			USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		/* Read one byte from the receive data register */
		//Rx3Data = USART_ReceiveData(USART3);	
	}

	/* 回显，把接收到的单个字节数据发送到 发送方*/
	//USART_SendData(USART2, qqData);
}

//volatile uint32_t	Rx3Counter	= 0;
//volatile uint8_t	Rx3Data 	= 0;
//volatile uint8_t	Rx3End 		= 0;
//volatile uint8_t	Tx3Buffer[512]={0};
//volatile uint8_t	Rx3Buffer[512]={0};

//void USART3_IRQHandler(void)
//{
//	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
//	{
//		//清楚接收中断标志位
//			USART_ClearITPendingBit(USART3, USART_IT_RXNE);
//		/* Read one byte from the receive data register */
//		Rx3Data = USART_ReceiveData(USART3);	
//	}

//	/* 回显，把接收到的单个字节数据发送到 发送方*/
//	USART_SendData(USART1, Rx3Data);
//}


/*****************  发送一个字节 **********************/
void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch)
{
	/* 发送一个字节数据到USART */
	USART_SendData(pUSARTx,ch);
		
	/* 等待发送数据寄存器为空 */
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);	
	//USART_ClearFlag(pUSARTx,USART_FLAG_TXE);
}

/*****************  发送指定长度的字节 **********************/
void Usart_SendBytes(USART_TypeDef * pUSARTx, uint8_t *buf,uint32_t len)
{
	uint8_t *p = buf;
	
	while(len--)
	{
		USART_SendData(pUSARTx,*p);
		
		p++;
		
		//等待数据发送成功
		while(USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE)==RESET);
		USART_ClearFlag(pUSARTx, USART_FLAG_TXE);
	}
}

/****************** 发送8位的数组  ************************/
//arr[1] =  aa  arr[2] = 1
void Usart_SendArray( USART_TypeDef * pUSARTx, uint8_t *array, uint16_t num)
{
	uint16_t i;

	for(i=0; i<num; i++)
	{
		/* 发送一个字节数据到USART */
		Usart_SendByte(pUSARTx,array[i]);	
	}
	/* 等待发送完成 */
	while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET);
	//USART_ClearFlag(pUSARTx,USART_FLAG_TC);
}


/*****************  发送字符串 **********************/
void Usart_SendString(USART_TypeDef *pUSARTx, char *str)
{
	unsigned int k = 0;
	do
	{
		Usart_SendByte(pUSARTx, *(str + k));
		k++;
	} while (*(str + k) != '\0');

	/* 等待发送完成 */
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TC) == RESET);
}


/*****************  发送一个16位数 **********************/
void Usart_SendHalfWord( USART_TypeDef * pUSARTx, uint16_t ch)
{
	uint8_t temp_h, temp_l;

	/* 取出高八位 */
	temp_h = (ch&0XFF00)>>8;
	/* 取出低八位 */
	temp_l = ch&0XFF;

	/* 发送高八位 */
	USART_SendData(pUSARTx,temp_h);	
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);

	/* 发送低八位 */
	USART_SendData(pUSARTx,temp_l);	
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);	
}



