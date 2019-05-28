#include "bsp_spi.h"
#include "stm32f4xx_hal.h"
//#include "stm32f4xx_ll.h"


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
static uint8_t spix_send_read_byte(SPI_HandleTypeDef *hspi, uint8_t dat)
{
    uint8_t s_dat = dat; 
	uint8_t r_dat = 0; 
    HAL_SPI_Transmit(hspi,&s_dat, 1, HAL_MAX_DELAY);
	HAL_SPI_Receive(hspi, &r_dat, 1, HAL_MAX_DELAY);
    return r_dat;
}
static uint8_t spix_read_byte(SPI_HandleTypeDef *hspi)
{
	uint8_t rd_dat = 0;
	rd_dat = spix_send_read_byte(hspi, 0xff);
	return rd_dat;
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



