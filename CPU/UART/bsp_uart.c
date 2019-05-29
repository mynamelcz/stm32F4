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

#define UART1_TX_BUF_SIZE			(0x3+1)
uint8_t uart1_tx_buf[UART1_TX_BUF_SIZE];


#define UART1_DMA_NUM				DMA2
#define UART1_DMA_TX_CHL			LL_DMA_CHANNEL_4
#define UART1_DMA_TX_STREAM			LL_DMA_STREAM_7
#define UART1_DMA_CLK				LL_AHB1_GRP1_PERIPH_DMA2

#define UART1_DMA_TX_DAT_BASE		(u32)(&(USART1->DR))



void uart1_dma_init(void)
{

	LL_DMA_InitTypeDef DMA_InitStruct = {0};

    /* UART  TX   DMA  configuration   */
	LL_AHB1_GRP1_EnableClock(UART1_DMA_CLK);

	LL_DMA_DeInit(UART1_DMA_NUM, UART1_DMA_TX_STREAM);
	
	while(LL_DMA_IsEnabledStream(UART1_DMA_NUM, UART1_DMA_TX_STREAM) != DISABLE);
	
	
	DMA_InitStruct.Channel = UART1_DMA_TX_CHL;
	DMA_InitStruct.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
	
	DMA_InitStruct.MemoryOrM2MDstAddress = (u32)uart1_tx_buf;
	DMA_InitStruct.NbData = UART1_TX_BUF_SIZE;
	DMA_InitStruct.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
	DMA_InitStruct.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
	
	DMA_InitStruct.PeriphOrM2MSrcAddress = UART1_DMA_TX_DAT_BASE;
	DMA_InitStruct.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
	DMA_InitStruct.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
	
	DMA_InitStruct.Mode = LL_DMA_MODE_NORMAL;
	
	DMA_InitStruct.MemBurst = LL_DMA_MBURST_SINGLE;
	DMA_InitStruct.PeriphBurst = LL_DMA_PBURST_SINGLE;
	
	DMA_InitStruct.FIFOMode = LL_DMA_FIFOMODE_DISABLE;
	DMA_InitStruct.FIFOThreshold = LL_DMA_FIFOTHRESHOLD_FULL;
	
	DMA_InitStruct.Priority = LL_DMA_PRIORITY_MEDIUM;
	
    LL_DMA_Init(UART1_DMA_NUM, UART1_DMA_TX_STREAM, &DMA_InitStruct);
	LL_DMA_EnableStream(UART1_DMA_NUM, UART1_DMA_TX_STREAM);
	
	while(LL_DMA_IsEnabledStream(UART1_DMA_NUM, UART1_DMA_TX_STREAM) != ENABLE);
    /*************          END        **************/

}



void uartx_dma_send_buf(USART_TypeDef *uartx, u8 *buf, u16 len)
{
	
	LL_DMA_DisableStream(UART1_DMA_NUM, UART1_DMA_TX_STREAM);
	LL_DMA_ClearFlag_TCx(UART1_DMA_NUM, UART1_DMA_TX_STREAM);
	LL_DMA_SetDataLength(UART1_DMA_NUM, UART1_DMA_TX_STREAM, len);
	LL_DMA_SetMemoryAddress(UART1_DMA_NUM, UART1_DMA_TX_STREAM, (u32)buf);
	LL_DMA_EnableStream(UART1_DMA_NUM, UART1_DMA_TX_STREAM);
	LL_USART_EnableDMAReq_TX(uartx);
	while(LL_DMA_IsActiveFlag_TCx(UART1_DMA_NUM, UART1_DMA_TX_STREAM)==0);
}
	
void uart1_init(void)
{
	u16 i = 0;
	uart_gpio_init(USART1);
	uart_reg_init(USART1);
	

	uart1_dma_init();
	for(i = 0; i< UART1_TX_BUF_SIZE; i++){
		uart1_tx_buf[i] = '#';
	}

//	LL_USART_EnableDMAReq_TX(USART1);
//    while(LL_DMA_IsActiveFlag_TCx(UART1_DMA_NUM, UART1_DMA_TX_STREAM)==0);
//	u8 buf[10]={"123456789"};
	uartx_dma_send_buf(USART1, uart1_tx_buf,UART1_TX_BUF_SIZE);
	
//	for(i = 0; i< UART1_TX_BUF_SIZE; i++){
//		uart1_tx_buf[i] = '^';
//	}
	
//	LL_DMA_DisableStream(UART1_DMA_NUM, UART1_DMA_TX_STREAM);
////	LL_DMA_ClearFlag_TC7(UART1_DMA_NUM);
//	LL_DMA_ClearFlag_TCx(UART1_DMA_NUM, UART1_DMA_TX_STREAM);
//	LL_DMA_SetDataLength(UART1_DMA_NUM, UART1_DMA_TX_STREAM, UART1_TX_BUF_SIZE);
//	LL_DMA_EnableStream(UART1_DMA_NUM, UART1_DMA_TX_STREAM);
//	LL_USART_EnableDMAReq_TX(USART1);
	
}



int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
	USART1->DR = (u8) ch;      
	return ch;
}
