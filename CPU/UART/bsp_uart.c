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

#define UART1_TX_BUF_SIZE			(10)
uint8_t uart1_tx_buf[UART1_TX_BUF_SIZE];


#define UART1_DMA_NUM				DMA2
#define UART1_DMA_TX_CHL			LL_DMA_CHANNEL_4
#define UART1_DMA_TX_STREAM			LL_DMA_STREAM_7
#define UART1_DMA_TX_INT		    DMA2_Stream7_IRQn
#define UART1_DMA_CLK				LL_AHB1_GRP1_PERIPH_DMA2
#define UART1_DMA_TX_DAT_BASE		(u32)(&(USART1->DR))

#define UART1_DMA_USE_FIFO			0


#define TEST_FLG()  	DBUG_Printf("Stream7 flg:%d; line:%d\n",LL_DMA_IsActiveFlag_TC7(DMA2), __LINE__)

void uart1_dma_init(void)
{
    /* UART  TX   DMA  configuration   */
	LL_AHB1_GRP1_EnableClock(UART1_DMA_CLK);
	// Select	Stream
	LL_DMA_DeInit(UART1_DMA_NUM, UART1_DMA_TX_STREAM);									
	while(LL_DMA_IsEnabledStream(UART1_DMA_NUM, UART1_DMA_TX_STREAM) != DISABLE){;}
	// Select	Chanel
	LL_DMA_SetChannelSelection(UART1_DMA_NUM, UART1_DMA_TX_STREAM, UART1_DMA_TX_CHL);	
	// Set		Direction		
	LL_DMA_SetDataTransferDirection(UART1_DMA_NUM, UART1_DMA_TX_STREAM, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
	// Set 		Priority
	LL_DMA_SetStreamPriorityLevel(UART1_DMA_NUM, UART1_DMA_TX_STREAM, LL_DMA_PRIORITY_LOW);
	// Set 		Mode
	LL_DMA_SetMode(UART1_DMA_NUM, UART1_DMA_TX_STREAM, LL_DMA_MODE_NORMAL);
    // Set 		Inc Mode
	LL_DMA_SetPeriphIncMode(UART1_DMA_NUM, UART1_DMA_TX_STREAM, LL_DMA_PERIPH_NOINCREMENT);
	LL_DMA_SetMemoryIncMode(UART1_DMA_NUM, UART1_DMA_TX_STREAM, LL_DMA_MEMORY_INCREMENT);
	// Set 		Dat	Size 
	LL_DMA_SetPeriphSize(UART1_DMA_NUM, UART1_DMA_TX_STREAM, LL_DMA_PDATAALIGN_BYTE);
	LL_DMA_SetMemorySize(UART1_DMA_NUM, UART1_DMA_TX_STREAM, LL_DMA_MDATAALIGN_BYTE);
	// Set 		Address 
	LL_DMA_SetPeriphAddress(UART1_DMA_NUM, UART1_DMA_TX_STREAM, UART1_DMA_TX_DAT_BASE);
	//LL_DMA_SetMemoryAddress(UART1_DMA_NUM, UART1_DMA_TX_STREAM, );
		
#if(UART1_DMA_USE_FIFO)
	LL_DMA_EnableFifoMode(UART1_DMA_NUM, UART1_DMA_TX_STREAM);	
    LL_DMA_ConfigFifo(UART1_DMA_NUM, UART1_DMA_TX_STREAM, LL_DMA_FIFOMODE_ENABLE, LL_DMA_FIFOTHRESHOLD_FULL); 
    LL_DMA_SetMemoryBurstxfer(UART1_DMA_NUM,UART1_DMA_TX_STREAM,LL_DMA_MBURST_INC16); 
    LL_DMA_SetPeriphBurstxfer(UART1_DMA_NUM,UART1_DMA_TX_STREAM,LL_DMA_MBURST_INC16);		
#else
	LL_DMA_DisableFifoMode(UART1_DMA_NUM, UART1_DMA_TX_STREAM);	
#endif

	LL_DMA_EnableStream(UART1_DMA_NUM, UART1_DMA_TX_STREAM);
	while(LL_DMA_IsEnabledStream(UART1_DMA_NUM, UART1_DMA_TX_STREAM) != ENABLE){;}
    /*************          END        **************/

	/* DMA  INT	configuration   */
	LL_DMA_ClearFlag_TCx(UART1_DMA_NUM, UART1_DMA_TX_STREAM);
	LL_DMA_EnableIT_TC(UART1_DMA_NUM, UART1_DMA_TX_STREAM);
		
	NVIC_SetPriority(UART1_DMA_TX_INT, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
	NVIC_EnableIRQ(UART1_DMA_TX_INT);
		
}



void uartx_dma_send_buf(USART_TypeDef *uartx, u8 *buf, u16 len)
{
	LL_DMA_DisableIT_TC(UART1_DMA_NUM, UART1_DMA_TX_STREAM);
	
	LL_DMA_DisableStream(UART1_DMA_NUM, UART1_DMA_TX_STREAM);
	while(LL_DMA_IsEnabledStream(UART1_DMA_NUM, UART1_DMA_TX_STREAM) != DISABLE);
	
	LL_DMA_SetDataLength(UART1_DMA_NUM, UART1_DMA_TX_STREAM, len+1);
	LL_DMA_SetMemoryAddress(UART1_DMA_NUM, UART1_DMA_TX_STREAM, (u32)buf);
	
	LL_DMA_ClearFlag_TCx(UART1_DMA_NUM, UART1_DMA_TX_STREAM);
	LL_DMA_EnableIT_TC(UART1_DMA_NUM, UART1_DMA_TX_STREAM);
	
	LL_DMA_EnableStream(UART1_DMA_NUM, UART1_DMA_TX_STREAM);
	LL_USART_EnableDMAReq_TX(uartx);	
}

void uart1_init(void)
{
	u16 i = 0;
	uart_gpio_init(USART1);
	uart_reg_init(USART1);
	

//	uart1_dma_init();
//	for(i = 0; i< UART1_TX_BUF_SIZE; i++){
//		uart1_tx_buf[i] = '0'+i;
//	}

//	uartx_dma_send_buf(USART1, uart1_tx_buf,UART1_TX_BUF_SIZE);
}



int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
	USART1->DR = (u8) ch;      
	return ch;
}
