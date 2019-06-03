#ifndef __APP_CFG_H
#define __APP_CFG_H
#include "SEGGER_RTT.h"
#include "stdio.h"

#define __ASSERT_PARAM

#define DBUG_Printf 	printf		// MY_RTT_printf
#define DBUG_Put_hex 	my_printhex	

//======= USER =======//
#define DBUG_MAIN	
#define DBUG_TASK_MANAGE

#ifdef DBUG_MAIN
#define main_printf		DBUG_Printf
#else
#define main_printf(...)
#endif

#ifdef DBUG_TASK_MANAGE
#define task_printf		DBUG_Printf
#else
#define main_printf(...)
#endif



//======= BSP =======//

#define __DBUG_BSP

#ifdef __DBUG_BSP
#define bsp_printf		DBUG_Printf
#define bsp_puthex		DBUG_Put_hex
#else
#define bsp_printf(...)
#define bsp_puthex(...)	
#endif


//======= DEV =======//
#define __DBUG_DEV_SPI_SD

#ifdef __DBUG_DEV_SPI_SD
#define sd_printf		DBUG_Printf
#define sd_puthex		DBUG_Put_hex
#else
#define sd_printf(...)
#define sd_puthex(...)	
#endif

#define __DBUG_SPI_FLASH

#ifdef __DBUG_SPI_FLASH
#define flash_printf		DBUG_Printf
#define flash_puthex		DBUG_Put_hex
#else
#define flash_printf(...)
#define flash_puthex(...)	
#endif

//======= LIB =======//
#define __DBUG_FATFS
#define __DBUG_VFS

#ifdef __DBUG_FATFS
#define fs_printf		DBUG_Printf
#define fs_puthex		DBUG_Put_hex
#else
#define fs_printf(...)
#define fs_puthex(...)	
#endif

#ifdef __DBUG_VFS
#define vfs_printf		DBUG_Printf
#define vfs_puthex		DBUG_Put_hex
#else
#define vfs_printf(...)
#define vfs_puthex(...)	
#endif




#endif


