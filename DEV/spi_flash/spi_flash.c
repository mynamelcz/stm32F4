#include "spi_flash.h"
#include "bsp_spi.h"




#define SPI_CS_PORT		GPIOB
#define SPI_CS_PIN		GPIO_PIN_0

#define SPI_CS_H()	HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS_PIN, GPIO_PIN_SET)
#define SPI_CS_L()	HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS_PIN, GPIO_PIN_RESET)


static void spi_cs_gpio_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = SPI_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(SPI_CS_PORT, &GPIO_InitStruct);
}

void spi_flash_init(void)
{ 
	spi1_init();
    spi_cs_gpio_init();
	SPI_CS_H();
}  

u16 flash_read_id(void)
{
	u16 Temp = 0;	  
	SPI_CS_L();			    
	spi1_send_byte(0x90);//∑¢ÀÕ∂¡»°ID√¸¡Ó	    
	spi1_send_byte(0x00); 	    
	spi1_send_byte(0x00); 	    
	spi1_send_byte(0x00); 	 			   
	Temp|=spi1_read_byte();  
	Temp|=spi1_read_byte();	 
	SPI_CS_H();	

	spi_printf("FLSH ID: 0x%x\n",Temp);
	return Temp;
}  








