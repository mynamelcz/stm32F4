#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H
#include "includes.h"
#include "diskio.h"
#include "bsp_spi.h"

#define FLASH_REV_SEC_NUM		0


#define DUMMY_DATA	0xFF


/**** SW Interface ****/
#define spi_delay_us(x)			sys_delay_us(x)



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

/***   IO CTROL CMD ***/
typedef enum{
	/* file sys cmd */
	FLASH_CTRL_SYNC 		= CTRL_SYNC,
	FLASH_GET_SECTOR_COUNT	= GET_SECTOR_COUNT,
	FLASH_GET_SECTOR_SIZE   = GET_SECTOR_SIZE,
	FLASH_GET_BLOCK_SIZE	= GET_BLOCK_SIZE,
	FLASH_CTRL_TRIM			= CTRL_TRIM,
	FLASH_CTRL_POWER		= CTRL_POWER,
	FLASH_CTRL_LOCK			= CTRL_LOCK,
	FLASH_CTRL_EJECT		= CTRL_EJECT,
	FLASH_CTRL_FORMAT		= CTRL_FORMAT,
	/* user cmd */
	FLASH_GET_SIZE, 	   //unit KB 
}IO_CTR_CMD;




typedef struct{
	void (*init)(__spi_ctr_obj *spi_obj);
    u32  (*read_id)(void);
    u32  (*status)(void);
	void (*read)(u8 *buf, u32 addr, u32 len);
	void (*write)(const u8 *buf, u32 addr, u32 len);
	void (*erase)(u32 start_sec, u32 sct_num);
	bool (*io_ctr)(u8 cmd, void *buff);
}__spi_flash_obj;


extern const __spi_flash_obj spi_flash_obj;


void spi_flash_test(void);
#endif


