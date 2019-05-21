#include "spi_flash.h"
#include "bsp_spi.h"

#define DUMMY_DATA	0xFF

#define flash_read_buf		spi1_read_buf
#define flash_write_buf		spi1_send_buf



#define SPI_CS_PORT			GPIOB
#define SPI_CS_PIN			GPIO_PIN_0
#define spi_cs_ctr(x)		(x)?HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS_PIN, GPIO_PIN_SET):HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS_PIN, GPIO_PIN_RESET)
/*** FLASH ATTRIBUTE	****/
#define	FLASH_PAGE_SIZE			256
#define FLASH_SECTOR_SIZE		(4  * 1024)
#define FLASH_BLOCK_SIZE		(64 * 1024)

/***	 FLASH CMD		****/
#define FLASH_WIRT_ENABLE		0x06 
#define FLASH_WIRT_DISABLE		0x04 

#define FLASH_PAGE_PROGRAM		0x02
#define FLASH_SECTOR_ERASE		0x20
#define FLASH_BLOCK32_ERASE		0x52
#define FLASH_BLOCK64_ERASE		0xD8
#define FLASH_CHIP_ERASE		0x60	// 0x60 or 0xC7

#define FLASH_POWER_DOWN		0xB9
#define FLASH_POWER_ON			0xAB

#define FLASH_READ_STATUS1		0x05 
#define FLASH_READ_STATUS2		0x35 
#define FLASH_READ_STATUS3		0x15 
#define FLASH_WRITE_STATUS1		0x01 
#define FLASH_WRITE_STATUS2		0x31
#define FLASH_WRITE_STATUS3		0x11

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
#define FLASH_READ_UNIQUE_ID	0x4B



#define STATUS1_REG_BUSY_BIT		BIT(0)		// RO	write busy
#define STATUS1_REG_WEL_BIT			BIT(1)		// RO	write enable


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


/******	 读取 ID	******/
u16 flash_read_id(void)
{
	u8 cmd_buf[4]={FLASH_READ_DEVICE_ID, DUMMY_DATA, DUMMY_DATA, DUMMY_DATA};
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
 	spi_printf("FLSH JEDEC: 0x%x\n",dev_id);
	return dev_id;
}
void flash_read_unique_id(u8 *id_buf)
{
	u8 cmd_buf[5]={FLASH_READ_UNIQUE_ID,DUMMY_DATA, DUMMY_DATA,DUMMY_DATA,DUMMY_DATA};
	spi_cs_ctr(0);
	flash_write_buf(cmd_buf, 5);
	flash_read_buf(id_buf,8);	
	spi_cs_ctr(1);
}

/******	 写使能 	******/
void flash_write_enable(void)
{
	u8 cmd_buf[1]={FLASH_WIRT_ENABLE};
	spi_cs_ctr(0);
	flash_write_buf(cmd_buf, 1);
	spi_cs_ctr(1);
}


/******	 读状态REG 	******/
u8 flash_read_status_register(void)
{
	u8 status = 0;
	u8 cmd_buf[1]={FLASH_READ_STATUS1};
	spi_cs_ctr(0);
	flash_write_buf(cmd_buf, 1);
	flash_read_buf(&status,1);
	spi_cs_ctr(1);	
	return status;
}
/******	 忙碌等待 	******/
void flash_wait_write_end(void)
{
	u8 status = 0;
	u8 cmd_buf[1]={FLASH_READ_STATUS1};
	spi_cs_ctr(0);
	flash_write_buf(cmd_buf, 1);
    do{
		flash_read_buf(&status,1);
	}while(status & STATUS1_REG_BUSY_BIT);
}
/******	 扇区 擦除 	******/
void flash_sector_erase(u32 sector_addr)
{
	if(sector_addr& (FLASH_SECTOR_SIZE-1)){		
		// not align by FLASH_SECTOR_SIZE
		spi_printf("FLASH WARNING: flash_sector_erase addr not align\n");
	}
	u8 cmd_buf[4];
    cmd_buf[0]=  FLASH_SECTOR_ERASE;
	cmd_buf[1] = (sector_addr>>16) & 0xff;
	cmd_buf[2] = (sector_addr>> 8) & 0xff;
	cmd_buf[3] = (sector_addr>> 0) & 0xff;
	flash_write_enable();
	spi_cs_ctr(0);
	flash_write_buf(cmd_buf, 4);
	spi_cs_ctr(1);
	flash_wait_write_end();
}
/******	 页 写入 	******/
/*1.应确保写入的地方已经被擦除*/
/*2.应确保写入的地方在同一个页*/
void flash_page_program(u8 *buf, u32 addr, u32 len)
{
	if((len + addr&(FLASH_SECTOR_SIZE-1)) > FLASH_PAGE_SIZE){		
		spi_printf("FLASH WARNING: write too long or not in one page\n");
	}
	u8 cmd_buf[4];
    cmd_buf[0]=  FLASH_PAGE_PROGRAM;
	cmd_buf[1] = (addr>>16) & 0xff;
	cmd_buf[2] = (addr>> 8) & 0xff;
	cmd_buf[3] = (addr>> 0) & 0xff;	
	flash_write_enable();
	spi_cs_ctr(0);
	flash_write_buf(cmd_buf, 4);
	flash_write_buf(buf, len);
	spi_cs_ctr(1);	
	flash_wait_write_end();
}

//void flash_wirte_status_register(void)
//{
//FLASH_WRITE_STATUS
//}



void spi_flash_test(void)
{
	u8 unique_id[8];
	flash_read_id();
	flash_read_jedec();
    flash_read_unique_id(unique_id);
	
	u8 i = 0;
	for(;i<8;i++){
		spi_printf("unique_id[%d]: 0x%x\n",i,unique_id[i]);
	}
}



//u32 flash_write_buf(u8 *buf, u32 addr, u32 size)
//{

//}












