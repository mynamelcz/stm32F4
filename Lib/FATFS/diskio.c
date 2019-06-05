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
#include "spi_sd.h"






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
		if(spi_flash_obj.status()){
			stat &= ~STA_NOINIT;
		}else{
			fs_printf("[%d][%s EER]\n",__LINE__,__FUNCTION__);
		}
		return stat;
	case DEV_RAM :
		return stat;
	case DEV_MMC :
		if(spi_sd_obj.status()){
			stat &= ~STA_NOINIT;
		}else{
			fs_printf("[%d][%s EER]\n",__LINE__,__FUNCTION__);
		}
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
	//  spi_flash_obj.init();
		if(spi_flash_obj.status()){
			stat &= ~STA_NOINIT;
		}else{
			fs_printf("[%d][%s EER]\n",__LINE__,__FUNCTION__);
		}
		return stat;
	case DEV_RAM :
		return stat;
	case DEV_MMC :
		if(spi_sd_obj.status()){
			stat &= ~STA_NOINIT;
		}else{
			fs_printf("[%d][%s EER]\n",__LINE__,__FUNCTION__);
		}
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
		 spi_flash_obj.read(buff, sector << FLASH_SCT_POWER, count << FLASH_SCT_POWER);
	     res = RES_OK;  //default  ok
		return res;
	case DEV_RAM :
		return res;
	case DEV_MMC :
		sector += FLASH_REV_SEC_NUM; 
		if(spi_sd_obj.read(buff, sector, count)){
			fs_printf("[%d][%s EER]\n",__LINE__,__FUNCTION__);
			res = RES_ERROR;
		}else{
			res = RES_OK;
		}
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
	DRESULT res = RES_ERROR;
	switch (pdrv) {
	case DEV_FLASH:
		 sector += FLASH_REV_SEC_NUM;  
         spi_flash_obj.erase(sector, count); 
	     spi_flash_obj.write(buff, sector << FLASH_SCT_POWER, count << FLASH_SCT_POWER);
	     res = RES_OK;  //default ok
		return res;		
	case DEV_RAM :
		return res;
	case DEV_MMC :
		sector += FLASH_REV_SEC_NUM;  
		if(spi_sd_obj.write(buff, sector, count)){
			fs_printf("[%d][%s EER]\n",__LINE__,__FUNCTION__);
			res = RES_ERROR;
		}else{
			res = RES_OK;
		}
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
	DRESULT res = RES_PARERR;
	switch (pdrv) {
    case DEV_FLASH:
        if(spi_flash_obj.io_ctr(cmd, buff)){
            res = RES_OK;
        }else{
			fs_printf("[%d][%s EER]\n",__LINE__,__FUNCTION__);
		}
        return res;
	case DEV_RAM :
		return res;
	case DEV_MMC :
        if(spi_sd_obj.io_ctr(cmd, buff)){
            res = RES_OK;
        }else{
			fs_printf("[%d][%s EER]\n",__LINE__,__FUNCTION__);
		}
		return res;
	case DEV_USB :
		return res;
	}

	return RES_PARERR;
}

DWORD get_fattime (void)
{
   DWORD *time;
	__time_tag t_tag;
	t_tag.year = 2019 - 1980;
	t_tag.mount = 1;
	t_tag.day = 21;
	t_tag.hour = 4;
	t_tag.min = 30;
	t_tag.sec = 10;
	time = (u32 *)(&t_tag);
    fs_printf("FUN:get_fattime :0x%x\n",*time);
	return (*time);
}






