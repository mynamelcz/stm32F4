/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/


#include "diskio.h"		/* Declarations of disk functions */
#include "spi_flash.h"







/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = STA_NOINIT;
	switch (pdrv) {
	case DEV_FLASH:
		if(FLASH_JEDEC_ID == flash_read_jedec()){
			stat &= ~STA_NOINIT;
		}
		return stat;
	case DEV_RAM :
		return stat;
	case DEV_MMC :
		return stat;
	case DEV_USB :
		return stat;
	}
	return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/
DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = STA_NOINIT;
	switch (pdrv) {
	case DEV_FLASH:
	    spi_flash_init();
		if(flash_get_sizeKB()){
			stat &= ~STA_NOINIT;
		}
		return stat;
	case DEV_RAM :
		return stat;
	case DEV_MMC :
		return stat;
	case DEV_USB :
		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res = RES_ERROR;
	switch (pdrv) {
	case DEV_FLASH:
		 sector += FLASH_REV_SEC_NUM;
		 flash_read_buf(buff, sector << FLASH_SCT_POWER, count << FLASH_SCT_POWER);
	     res = RES_OK;
		return res;
	case DEV_RAM :
		return res;
	case DEV_MMC :
		return res;
	case DEV_USB :
		return res;
	}
	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;

	switch (pdrv) {
	case DEV_FLASH:
		 sector += FLASH_REV_SEC_NUM;  
         flash_erase_sectors(sector, count); 
	     flash_write_buf(buff, sector << FLASH_SCT_POWER, count << FLASH_SCT_POWER);
	     res = RES_OK;
		return res;		
	case DEV_RAM :
		return res;
	case DEV_MMC :
		return res;
	case DEV_USB :
		return res;
	}
	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	switch (pdrv) {
    case DEV_FLASH:
        flash_io_control(cmd, buff);
        res = RES_OK;
        return res;
	case DEV_RAM :
		return res;
	case DEV_MMC :
		return res;
	case DEV_USB :
		return res;
	}

	return RES_PARERR;
}

__weak DWORD get_fattime (void)
{
   DWORD time = 0;
    
   time = ((DWORD)((2019 - 1980)<< 25)) |      //year
          ((DWORD)( 1           << 21)) |      //month
          ((DWORD)( 1           << 16)) |      //day
          ((DWORD)( 1           << 11)) |      //hour
          ((DWORD)( 1           <<  5)) |      //min
          ((DWORD)( 1           >>  1));       //0.5 second
  return time;
}






