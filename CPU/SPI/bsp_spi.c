#include "bsp_spi.h"
#include "stm32f4xx_hal.h"
//#include "stm32f4xx_ll.h"


SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;
DMA_HandleTypeDef hdma_spi1_rx;
DMA_HandleTypeDef hdma_spi1_tx;

void spi_err_callback(u16 line)
{
	spi_printf("BSP SPI ERR !!!!\n");
	spi_printf("ERR LINE: %d\n",line);
	while(1);
}




static void spi_gpio_init(SPI_HandleTypeDef *hspi)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hspi->Instance==SPI1)
  {
    __HAL_RCC_SPI1_CLK_ENABLE();
    /**SPI1 GPIO Configuration    
    PB5     ------> SPI1_MOSI
    PB4     ------> SPI1_MISO
    PB3     ------> SPI1_SCK 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_4|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  }
  if(hspi->Instance==SPI2)
  {
    __HAL_RCC_SPI2_CLK_ENABLE();
    /**SPI1 GPIO Configuration    
    PB15     ------> SPI2_MOSI
    PB14     ------> SPI2_MISO
    PB13     ------> SPI2_SCK 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_15|GPIO_PIN_14|GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  }

}

void spi1_init(void)
{
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;    			//clk idle state 
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;       				 //
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;   //clk prescaler
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;			        //time out close
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  spi_gpio_init(&hspi1);
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
	spi_err_callback(__LINE__);
  }
  __HAL_SPI_ENABLE(&hspi1);
}



void spi2_init(void)
{
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
	
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;    			//clk idle state 
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;       				 //
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;   //clk prescaler
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;			        //time out close
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  spi_gpio_init(&hspi2);
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
	spi_err_callback(__LINE__);
  }
  __HAL_SPI_ENABLE(&hspi2);
}





void spi1_send_buf(const uint8_t *pData, uint16_t Size)
{
	HAL_SPI_Transmit(&hspi1, (uint8_t *)pData, Size, HAL_MAX_DELAY);


}

void spi1_read_buf(uint8_t *pData, uint16_t Size)
{

	if(HAL_SPI_Receive(&hspi1, pData, Size, HAL_MAX_DELAY) != HAL_OK){
		spi_printf("BSP SPI ERR spi1_read_buf!!!!\n");
	}
}



void spi2_send_buf(const uint8_t *pData, uint16_t Size)
{
	HAL_SPI_Transmit(&hspi2, (uint8_t *)pData, Size, HAL_MAX_DELAY);


}

void spi2_read_buf(uint8_t *pData, uint16_t Size)
{

	if(HAL_SPI_Receive(&hspi2, pData, Size, HAL_MAX_DELAY) != HAL_OK){
		spi_printf("BSP SPI ERR spi1_read_buf!!!!\n");
	}
}


