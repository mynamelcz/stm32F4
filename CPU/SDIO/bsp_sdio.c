#include "bsp_sdio.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_ll.h"

SD_HandleTypeDef hsd;
DMA_HandleTypeDef hdma_sdio_rx;
DMA_HandleTypeDef hdma_sdio_tx;

static void sdio_err_callback(u16 line)
{
	bsp_printf("BSP SDIO ERR !!!!\n");
	bsp_printf("ERR LINE: %d\n",line);
	while(1);
}


static void sdio_gpio_init(void)
{
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
//    PC8     ------>  SDIO_D0
//    PC9     ------>  SDIO_D1
//    PC10     ------> SDIO_D2
//    PC11     ------> SDIO_D3
//    PC12     ------> SDIO_CK
//    PD2     ------>  SDIO_CMD 
//    */	
	GPIO_InitStruct.Pin = LL_GPIO_PIN_8|LL_GPIO_PIN_9|LL_GPIO_PIN_10|LL_GPIO_PIN_11|LL_GPIO_PIN_12;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
	LL_GPIO_Init(GPIOC, &GPIO_InitStruct);	
	
	GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
	LL_GPIO_Init(GPIOD, &GPIO_InitStruct);	

}

static void sdio_reg_init(void)
{

	SDIO_InitTypeDef SDIO_InitStruct={0}; 
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SDIO);
	
    SDIO_InitStruct.ClockEdge = SDIO_CLOCK_EDGE_RISING;
	SDIO_InitStruct.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;		// 1:��������Ƶ  0:ʹ�÷�Ƶ
	SDIO_InitStruct.ClockDiv = SDIO_INIT_CLK_DIV;	
	SDIO_InitStruct.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;	// ����ģʽ
	SDIO_InitStruct.BusWide = SDIO_BUS_WIDE_1B;						// SD����ʼ��ʱʹ�� 1bit
	SDIO_InitStruct.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_ENABLE; //Ӳ��������
	
    SDIO_Init(SDIO, SDIO_InitStruct);
}

#define SDIO_DMA_NUM			DMA2
#define SDIO_DMA_CHL			LL_DMA_CHANNEL_4
#define SDIO_DMA_DMA_CLK		LL_AHB1_GRP1_PERIPH_DMA2
#define SDIO_FIFO_PBASE			(u32)(&(SDIO->FIFO))

#define SDIO_DMA_TX_INT			DMA2_Stream3_IRQn
#define SDIO_DMA_RX_INT			DMA2_Stream6_IRQn


#define SDIO_DMA_USE_FIFO		1

#define SDIO_DMA_TX_STREAM		LL_DMA_STREAM_3
#define SDIO_DMA_RX_STREAM		LL_DMA_STREAM_6

static void sdio_tx_dma_init(u32 *src_addr, u32 bufferSize)
{
    /* UART  TX   DMA  configuration   */
	LL_AHB1_GRP1_EnableClock(SDIO_DMA_DMA_CLK);
	LL_DMA_DeInit(SDIO_DMA_NUM, SDIO_DMA_TX_STREAM);									
	while(LL_DMA_IsEnabledStream(SDIO_DMA_NUM, SDIO_DMA_TX_STREAM) != DISABLE){;}
	// Select	Chanel
	LL_DMA_SetChannelSelection(SDIO_DMA_NUM, SDIO_DMA_TX_STREAM, SDIO_DMA_CHL);	
	// Set		Direction		
	LL_DMA_SetDataTransferDirection(SDIO_DMA_NUM, SDIO_DMA_TX_STREAM, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
	// Set 		Priority
	LL_DMA_SetStreamPriorityLevel(SDIO_DMA_NUM, SDIO_DMA_TX_STREAM, LL_DMA_PRIORITY_LOW);
	// Set 		Mode
	LL_DMA_SetMode(SDIO_DMA_NUM, SDIO_DMA_TX_STREAM, LL_DMA_MODE_PFCTRL);
    // Set 		Inc Mode
	LL_DMA_SetPeriphIncMode(SDIO_DMA_NUM, SDIO_DMA_TX_STREAM, LL_DMA_PERIPH_NOINCREMENT);
	LL_DMA_SetMemoryIncMode(SDIO_DMA_NUM, SDIO_DMA_TX_STREAM, LL_DMA_MEMORY_INCREMENT);
	// Set 		Dat	Size 
	LL_DMA_SetPeriphSize(SDIO_DMA_NUM, SDIO_DMA_TX_STREAM, LL_DMA_PDATAALIGN_WORD);
	LL_DMA_SetMemorySize(SDIO_DMA_NUM, SDIO_DMA_TX_STREAM, LL_DMA_MDATAALIGN_WORD);
	// Set 		Address 
	LL_DMA_SetPeriphAddress(SDIO_DMA_NUM, SDIO_DMA_TX_STREAM, SDIO_FIFO_PBASE);
	LL_DMA_SetMemoryAddress(SDIO_DMA_NUM, SDIO_DMA_TX_STREAM, (u32)src_addr);
		
#if(SDIO_DMA_USE_FIFO)
	LL_DMA_EnableFifoMode(SDIO_DMA_NUM, SDIO_DMA_TX_STREAM);	
    LL_DMA_ConfigFifo(SDIO_DMA_NUM, SDIO_DMA_TX_STREAM, LL_DMA_FIFOMODE_ENABLE, LL_DMA_FIFOTHRESHOLD_FULL); 
    LL_DMA_SetMemoryBurstxfer(SDIO_DMA_NUM,SDIO_DMA_TX_STREAM,LL_DMA_MBURST_INC4); 
    LL_DMA_SetPeriphBurstxfer(SDIO_DMA_NUM,SDIO_DMA_TX_STREAM,LL_DMA_MBURST_INC4);		
#else
	LL_DMA_DisableFifoMode(SDIO_DMA_NUM, UART1_DMA_TX_STREAM);	
#endif	
	LL_DMA_EnableStream(SDIO_DMA_NUM, SDIO_DMA_TX_STREAM);
	while(LL_DMA_IsEnabledStream(SDIO_DMA_NUM, SDIO_DMA_TX_STREAM) != ENABLE){;}
    /*************          END        **************/



}	
static void sdio_rx_dma_init(void)
{

}

static void sdio_intterrupt_init(void)
{
	/* SDIO  INT	configuration   */	
	NVIC_SetPriority(SDIO_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
	NVIC_EnableIRQ(SDIO_IRQn);
	
	/* DMA  INT	configuration   */
	LL_DMA_ClearFlag_TCx(SDIO_DMA_NUM, SDIO_DMA_TX_STREAM);
	LL_DMA_EnableIT_TC(SDIO_DMA_NUM, SDIO_DMA_TX_STREAM);	
	NVIC_SetPriority(SDIO_DMA_TX_INT, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 1));
	NVIC_EnableIRQ(SDIO_DMA_TX_INT);	

	LL_DMA_ClearFlag_TCx(SDIO_DMA_NUM, SDIO_DMA_RX_STREAM);
	LL_DMA_EnableIT_TC(SDIO_DMA_NUM, SDIO_DMA_RX_STREAM);
	NVIC_SetPriority(SDIO_DMA_RX_INT, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 1));
	NVIC_EnableIRQ(SDIO_DMA_RX_INT);	
}

static void sdio_init(void)
{
	sdio_gpio_init();
	sdio_reg_init();

}

