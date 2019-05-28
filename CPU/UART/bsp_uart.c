#include "bsp_uart.h"
#include "stm32f4xx_ll.h"

void uart_err_callback(u16 line)
{
	while(1);
}

void uart_gpio_init(USART_TypeDef *Instance)
{
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	if(Instance == USART1){
	    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOH);
		LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
		
		GPIO_InitStruct.Pin = LL_GPIO_PIN_9|LL_GPIO_PIN_10;
		GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
		GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
		GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
		GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
		LL_GPIO_Init(GPIOA, &GPIO_InitStruct);	
	}
	if(Instance == USART2){
		;
	}
}

void uart_reg_init(USART_TypeDef *Instance)
{
	LL_USART_InitTypeDef USART_InitStruct = {0};
	if(Instance == USART1){
		
		LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
		
		USART_InitStruct.BaudRate = 115200;
		USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
		USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
		USART_InitStruct.Parity = LL_USART_PARITY_NONE;
		USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
		USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
		USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
		LL_USART_Init(USART1, &USART_InitStruct);
		LL_USART_ConfigAsyncMode(USART1);
		LL_USART_Enable(USART1);	
	}
}

u8 tx_buf[10]={"sadasd"};
void uart_dma_init(void)
{

//	LL_DMA_InitTypeDef DMA_InitStruct = {0};
//	
//	DMA_InitStruct.Channel = LL_DMA_CHANNEL_4;
//	DMA_InitStruct.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
//	
//	DMA_InitStruct.MemoryOrM2MDstAddress = (u32)tx_buf;
//	
//	DMA_InitStruct.MemoryOrM2MDstDataSize = 10;
//	DMA_InitStruct.PeriphOrM2MSrcAddress = 
	
	
	
	
//  NVIC_SetPriority(DMA2_Stream7_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
//  NVIC_EnableIRQ(DMA2_Stream7_IRQn);
// /* USART1_TX Init */
//  LL_DMA_SetChannelSelection(DMA2, LL_DMA_STREAM_7, LL_DMA_CHANNEL_4);

//  LL_DMA_SetDataTransferDirection(DMA2, LL_DMA_STREAM_7, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

//  LL_DMA_SetStreamPriorityLevel(DMA2, LL_DMA_STREAM_7, LL_DMA_PRIORITY_LOW);

//  LL_DMA_SetMode(DMA2, LL_DMA_STREAM_7, LL_DMA_MODE_NORMAL);

//  LL_DMA_SetPeriphIncMode(DMA2, LL_DMA_STREAM_7, LL_DMA_PERIPH_NOINCREMENT);

//  LL_DMA_SetMemoryIncMode(DMA2, LL_DMA_STREAM_7, LL_DMA_MEMORY_INCREMENT);

//  LL_DMA_SetPeriphSize(DMA2, LL_DMA_STREAM_7, LL_DMA_PDATAALIGN_BYTE);

//  LL_DMA_SetMemorySize(DMA2, LL_DMA_STREAM_7, LL_DMA_MDATAALIGN_BYTE);

//  LL_DMA_DisableFifoMode(DMA2, LL_DMA_STREAM_7);

}




	
void uart1_init(void)
{
	uart_gpio_init(USART1);
	uart_reg_init(USART1);
}



int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
	USART1->DR = (u8) ch;      
	return ch;
}
