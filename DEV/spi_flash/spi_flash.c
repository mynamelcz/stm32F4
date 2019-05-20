#include "spi_flash.h"
#include "bsp_spi.h"


#define flash_read_buf		spi1_read_buf
#define flash_write_buf		spi1_send_buf

#define SPI_CS_PORT			GPIOB
#define SPI_CS_PIN			GPIO_PIN_0

#define spi_cs_ctr(x)	(x)?HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS_PIN, GPIO_PIN_SET):HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS_PIN, GPIO_PIN_RESET)


#define FLASH_WIRT_ENABLE		0x06 
#define FLASH_WIRT_DISABLE		0x04 
#define FLASH_READ_STATUS		0x05 
#define FLASH_WRITE_STATUS		0x01 
//#define W25X_ReadData			0x03 
//#define W25X_FastReadData		0x0B 
//#define W25X_FastReadDual		0x3B 
//#define W25X_PageProgram		0x02 
//#define W25X_BlockErase			0xD8 
//#define W25X_SectorErase		0x20 
//#define W25X_ChipErase			0xC7 
//#define W25X_PowerDown			0xB9 
//#define W25X_ReleasePowerDown	0xAB 
//#define W25X_DeviceID			0xAB 
//#define W25X_ManufactDeviceID	0x90 
//#define W25X_JedecDeviceID		0x9F 

#define FLASH_READ_JEDEC_ID		0x9F 
#define FLASH_READ_DEVICE_ID	0x90
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
	spi_cs_ctr(0);
}  

u16 flash_read_id(void)
{
	u8 cmd_buf[4]={FLASH_READ_DEVICE_ID, 0, 0, 0};
	u16 dev_id = 0;	  
	spi_cs_ctr(0);	
	flash_write_buf(cmd_buf, 4);
	flash_read_buf((u8 *)(&dev_id),2);
	spi_cs_ctr(1);	
 	spi_printf("FLSH ID: 0x%x\n",dev_id);
	return dev_id;
}  

u32 flash_read_jedec(void)
{
	u32 dev_id = 0;
	u8 cmd_buf[1]={FLASH_READ_JEDEC_ID};
	u8 buf[3];	  
	spi_cs_ctr(0);	 
	flash_write_buf(cmd_buf, 1);
	flash_read_buf(buf,3);
	spi_cs_ctr(1);
    dev_id = (buf[0]<<16) + (buf[1]<<8) + buf[2];	
 	spi_printf("FLSH ID: 0x%x\n",dev_id);
	return dev_id;
}








