#include "bsp_spi.h"




SPI_HandleTypeDef hspi1;
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

}




static uint8_t spix_read_byte(SPI_HandleTypeDef *hspi)
{
    uint16_t t_out = 1000;
	uint8_t rd_dat = 0;

    while(__HAL_SPI_GET_FLAG(hspi, SPI_FLAG_TXE) == RESET){
		if(--t_out == 0){
			spi_err_callback(__LINE__);
			return 0;
		}
	}
    hspi->Instance->DR = 0XFF;	
	t_out = 1000;
    while(__HAL_SPI_GET_FLAG(hspi, SPI_FLAG_RXNE) == RESET){
		if(--t_out == 0){
			spi_err_callback(__LINE__);
			return 0;
		}
	}
    rd_dat = hspi->Instance->DR;
	return rd_dat;
}
static uint8_t spix_send_byte(SPI_HandleTypeDef *hspi, uint8_t dat)
{
	uint8_t rd_dat = 0;
	uint16_t t_out = 1000;
    while(__HAL_SPI_GET_FLAG(hspi, SPI_FLAG_TXE) == RESET){
		if(--t_out == 0){
			spi_err_callback(__LINE__);
			return 0;
		}
	}
    hspi->Instance->DR = dat;	
	
	t_out = 1000;
    while(__HAL_SPI_GET_FLAG(hspi, SPI_FLAG_RXNE) == RESET){
		if(--t_out == 0){
			spi_err_callback(__LINE__);
			return 0;
		}
	}
    rd_dat = hspi->Instance->DR;
	
	return 1;
}




void spi1_init(void)
{
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	
  hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;    			//clk idle state 
  hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;       				 //
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

uint8_t spi1_read_byte(void)
{
	uint8_t rd_dat = 0;
	rd_dat	= spix_read_byte(&hspi1);
	return rd_dat;
}
void spi1_send_byte(uint8_t dat)
{
	spix_send_byte(&hspi1, dat);
}

