#include "bsp_spi.h"
#include "soft_spi.h"
#include "stm32f4xx_ll.h"

#define SPI_TIME_OUT_CNT	1000



static void spi_err_callback(u16 line)
{
	bsp_printf("BSP SPI ERR !!!!\n");
	bsp_printf("ERR LINE: %d\n",line);
	while(1);
}




static void spi_gpio_init(SPI_TypeDef *hspi)
{
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hspi == SPI1)
  {
    /**SPI1 GPIO Configuration    
    PB5     ------> SPI1_MOSI
    PB4     ------> SPI1_MISO
    PB3     ------> SPI1_SCK 
    */
	  GPIO_InitStruct.Pin = LL_GPIO_PIN_3 |LL_GPIO_PIN_4|LL_GPIO_PIN_5;
	  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	  GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
	  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  }
  if(hspi == SPI2)
  {

    /**SPI2 GPIO Configuration    
    PB15     ------> SPI2_MOSI
    PB14     ------> SPI2_MISO
    PB13     ------> SPI2_SCK 
    */
	  GPIO_InitStruct.Pin = LL_GPIO_PIN_13 |LL_GPIO_PIN_14|LL_GPIO_PIN_15;
	  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
	  GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
	  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  }

}

static void spi1_init(void(*cs_fun)(u8))
{
  ASSERT(cs_fun);
  spi1_obj.cs_str = cs_fun;
	
  LL_SPI_InitTypeDef SPI_InitStruct = {0};
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
  
  spi_gpio_init(SPI1);

  SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
  SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
  SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
  SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
  SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;
  SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;
  SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV2;
  SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
  SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
  SPI_InitStruct.CRCPoly = 10;
  LL_SPI_Init(SPI1, &SPI_InitStruct);
  LL_SPI_SetStandard(SPI1, LL_SPI_PROTOCOL_MOTOROLA);

  LL_SPI_Enable(SPI1);


}



static void spi2_init(void(*cs_fun)(u8))
{
  ASSERT(cs_fun);
  spi2_obj.cs_str = cs_fun;

  LL_SPI_InitTypeDef SPI_InitStruct = {0};
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);
  
  spi_gpio_init(SPI2);  
  
  SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
  SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
  SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
  SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
  SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;
  SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;
  SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV2;
  SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
  SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
  SPI_InitStruct.CRCPoly = 10;
  LL_SPI_Init(SPI2, &SPI_InitStruct);
  LL_SPI_SetStandard(SPI2, LL_SPI_PROTOCOL_MOTOROLA);

  LL_SPI_Enable(SPI2);

}





static bool spi1_ioctr(u8 cmd, void *buf)
{
	bsp_printf("FUN:%s\n",__func__);
	u8 res = false;
	u16 dat8  = 0;
	switch(cmd){
		case SPI_SET_SPEED_CMD:
			 dat8 = *((u16 *)(buf));
			 dat8 %= (SPI_CLK_PRE256+1);
	         SPI1->CR1 = (u16)((SPI1->CR1 & 0xFFC7)|(dat8<<3));
		     bsp_printf("[SPI1 CLK]:%d KHz\n",84000>>(dat8+1));
			 res = true;
			 return res;
		case SPI_SET_SPEED_LOW:
	         SPI1->CR1 = (u16)((SPI1->CR1 & 0xFFC7)|SPI_CLK_PRE128<<3);
		     res = true;
		     return res;
		case SPI_SET_SPEED_HIGH:
	         SPI1->CR1 = (u16)((SPI1->CR1 & 0xFFC7)|SPI_CLK_PRE2<<3);
		     res = true;
		     return res;
		default:
			break;
	}
	
	bsp_printf("[SPI ERR]: spi1_ioctr\n");
	return res;
}

static bool spi2_ioctr(u8 cmd, void *buf)
{
	u8 res = false;
	u16 dat8  = 0;
	switch(cmd){
		case SPI_SET_SPEED_CMD:
			 dat8 = *((u16 *)(buf));
			 dat8 %= (SPI_CLK_PRE256+1);
	         SPI2->CR1 = (u16)((SPI2->CR1 & 0xFFC7)|(dat8<<3));
		     bsp_printf("[SPI2 CLK]:%d KHz\n",42000>>(dat8+1));
			 res = true;
			 return res;
		case SPI_SET_SPEED_LOW:
	         SPI2->CR1 = (u16)((SPI2->CR1 & 0xFFC7)|SPI_CLK_PRE128);
			 res = true;
			 return res;
		case SPI_SET_SPEED_HIGH:
	         SPI2->CR1 = (u16)((SPI2->CR1 & 0xFFC7)|SPI_CLK_PRE2);
			 res = true;
			 return res;
		default:
			break;
	}
	return res;
}



static u8 spi1_send_read_byte(u8 dat)
{
	u32 t_cnt = SPI_TIME_OUT_CNT;
	while (((SPI1->SR & SPI_SR_TXE) == RESET)&&(--t_cnt));
	if(t_cnt == 0){
		spi_err_callback(__LINE__);
	}
	t_cnt = SPI_TIME_OUT_CNT;
	SPI1->DR = dat;
	while (((SPI1->SR & SPI_SR_RXNE) == RESET)&&(--t_cnt));
	if(t_cnt == 0){
		spi_err_callback(__LINE__);
	}
	return (u8)(SPI1->DR);	
}



static void spi1_send_buf(const u8 *pData, uint32_t Size)
{
	const u8 *ptr = pData;
	if(Size==0)return;
	while(Size--){
		spi1_send_read_byte(*ptr++);
	}
}

static void spi1_read_buf(uint8_t *pData, uint32_t Size)
{
	u8 *ptr = pData;
	if(Size==0)return;
	while(Size--){
		*ptr++ = spi1_send_read_byte(0xff);
	}
}




static u8 spi2_send_read_byte(u8 dat)
{
	u32 t_cnt = SPI_TIME_OUT_CNT;
	while (((SPI2->SR & SPI_SR_TXE) == RESET)&&(--t_cnt));
	if(t_cnt == 0){
		spi_err_callback(__LINE__);
	}
	t_cnt = SPI_TIME_OUT_CNT;
	SPI2->DR = dat;
	while (((SPI2->SR & SPI_SR_RXNE) == RESET)&&(--t_cnt));
	if(t_cnt == 0){
		spi_err_callback(__LINE__);
	}
	return (u8)(SPI2->DR);	
}



static void spi2_send_buf(const u8 *pData, uint32_t Size)
{
	const u8 *ptr = pData;
	if(Size==0)return;
	while(Size--){
		spi2_send_read_byte(*ptr++);
	}
}

static void spi2_read_buf(uint8_t *pData, uint32_t Size)
{
	u8 *ptr = pData;
	if(Size==0)return;
	while(Size--){
		*ptr++ = spi2_send_read_byte(0xff);
	}
}

////////////////////////////////////////////////
//Soft_SPI_hd soft_spi_hd;
//#define SOFT_SPI_PORT		GPIOB
//#define SOFT_SPI_PIN_CS		LL_GPIO_PIN_12
//#define SOFT_SPI_PIN_CLK	LL_GPIO_PIN_13
//#define SOFT_SPI_PIN_MISO	LL_GPIO_PIN_14
//#define SOFT_SPI_PIN_MOSI	LL_GPIO_PIN_15



//#define SOFT_SPI_PORT		GPIOB
//#define SOFT_SPI_PIN_CS		LL_GPIO_PIN_0
//#define SOFT_SPI_PIN_CLK	LL_GPIO_PIN_3
//#define SOFT_SPI_PIN_MISO	LL_GPIO_PIN_4
//#define SOFT_SPI_PIN_MOSI	LL_GPIO_PIN_5
//static void soft_spi_cs(u8 en)
//{
//	en?LL_GPIO_SetOutputPin(SOFT_SPI_PORT, SOFT_SPI_PIN_CS):\
//	   LL_GPIO_ResetOutputPin(SOFT_SPI_PORT, SOFT_SPI_PIN_CS);	
//}
//static u8 soft_spi_miso(void)
//{
//	return LL_GPIO_IsInputPinSet(SOFT_SPI_PORT, SOFT_SPI_PIN_MISO);
//}
//static void soft_spi_mosi(u8 en)
//{
//	en?LL_GPIO_SetOutputPin(SOFT_SPI_PORT, SOFT_SPI_PIN_MOSI):\
//	   LL_GPIO_ResetOutputPin(SOFT_SPI_PORT, SOFT_SPI_PIN_MOSI);
//}
//static void soft_spi_clk(u8 en)
//{
//	en?LL_GPIO_SetOutputPin(SOFT_SPI_PORT, SOFT_SPI_PIN_CLK):\
//	   LL_GPIO_ResetOutputPin(SOFT_SPI_PORT, SOFT_SPI_PIN_CLK);
//}
//static bool soft_spi_ioctr(u8 cmd, void *buf)
//{
//	u8 res = false;
//	return true;
//}
//static void soft_spi_init(void(*cs_fun)(u8))
//{
//	(void)(cs_fun);
//	soft_spi_hd.type = CPOL0_CPHA0;
//	
//	soft_spi_hd.CLK = soft_spi_clk;
//	soft_spi_hd.MISO= soft_spi_miso;
//	soft_spi_hd.MOSI= soft_spi_mosi;
//	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
//	GPIO_InitStruct.Pin = 	SOFT_SPI_PIN_CS | 
//							SOFT_SPI_PIN_CLK |
//							SOFT_SPI_PIN_MOSI;
//	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
//	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
//	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
//    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
//	LL_GPIO_Init(SOFT_SPI_PORT, &GPIO_InitStruct);	

//	GPIO_InitStruct.Pin = 	SOFT_SPI_PIN_MISO;
//	GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
//	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
//	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
//    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
//	LL_GPIO_Init(SOFT_SPI_PORT, &GPIO_InitStruct);		

//}


//static void soft_spi_send(const u8 *pData, uint32_t Size)
//{
//	//spi_send_buf(&soft_spi_hd, pData, Size);
//}
//static void soft_spi_read(u8 *pData, uint32_t Size)
//{
//	//spi_read_buf(&soft_spi_hd, pData, Size);
//}	
//static u8 soft_spi_send_read_byte(u8 dat)
//{
//	//return spi_tx_rx_byte(&soft_spi_hd, dat);
//}

__spi_ctr_obj spi1_obj = {
	.cs_str = NULL,
	.init 	= spi1_init,
	.read   = spi1_read_buf,
	.write  = spi1_send_buf,
	.w_r_byte = spi1_send_read_byte,
	.io_ctr = spi1_ioctr,
};

__spi_ctr_obj spi2_obj = {
	.cs_str = NULL,
	.init 	= spi2_init,
	.read   = spi2_read_buf,
	.write  = spi2_send_buf,
	.w_r_byte = spi2_send_read_byte,
	.io_ctr = spi2_ioctr,
};
  
//__spi_ctr_obj soft_spi_obj = {
//	.cs_str = soft_spi_cs,
//	.init 	= soft_spi_init,
//	.read   = soft_spi_read,
//	.write  = soft_spi_send,
//	.w_r_byte = soft_spi_send_read_byte,
//	.io_ctr = soft_spi_ioctr,
//};
