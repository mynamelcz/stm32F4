#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H
#include "includes.h"
#include "bsp_spi.h"

#define FLASH_REV_SEC_NUM		0


#define DUMMY_DATA	0xFF


/**** SW Interface ****/
#define spi_read_buf		spi1_read_buf
#define spi_write_buf		spi1_send_buf
#define spi_delay_us(x)		HAL_Delay(1)

/**** HW Interface ****/
#define SPI_CS_PORT			GPIOB
#define SPI_CS_PIN			LL_GPIO_PIN_0
#define spi_cs_ctr(x)		(x)?LL_GPIO_SetOutputPin(SPI_CS_PORT, SPI_CS_PIN):\
								LL_GPIO_ResetOutputPin(SPI_CS_PORT, SPI_CS_PIN)




/*** FLASH ATTRIBUTE	****/
#define FLASH_JEDEC_ID			0xEF4015

#define	FLASH_PAGE_SIZE			256
#define FLASH_SECTOR_SIZE		(4<<10)
#define FLASH_SCT_POWER		    12
#define FLASH_BLOCK_SIZE		(64<<10)

/***	 FLASH CMD		****/
#define FLASH_WIRT_ENABLE		0x06 
#define FLASH_WIRT_DISABLE		0x04
#define FLASH_READ_DATA			0x03
#define FLASH_FAST_READ_DATA	0x0B
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
#define FLASH_READ_JEDEC_ID		0x9F 
#define FLASH_READ_DEVICE_ID	0x90
#define FLASH_READ_UNIQUE_ID	0x4B


/***	 REG BITS	****/
#define STATUS1_REG_BUSY_BIT		BIT(0)		// RO	write busy
#define STATUS1_REG_WEL_BIT			BIT(1)		// RO	write enable


/***	 API    ****/
void spi_flash_init(void);
u32 flash_get_sizeKB(void);
u32 flash_read_jedec(void);
void flash_read_buf(u8 *buf, u32 addr, u32 len);
void flash_write_buf(const u8 *buf, u32 addr, u32 len);
void flash_erase_sectors(u32 start_sec, u32 sct_num);
bool flash_io_control(u8 cmd, void *buff);


void spi_flash_test(void);
#endif


