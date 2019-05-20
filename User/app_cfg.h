#ifndef __APP_CFG_H
#define __APP_CFG_H
#include "SEGGER_RTT.h"
#include "stdio.h"

#define DBUG_Printf 	printf	// MY_RTT_printf

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



////////////////////////////////////////////////////////
#define DBUG_BSP_SPI



#ifdef DBUG_BSP_SPI
#define spi_printf		DBUG_Printf
#else
#define spi_printf(...)
#endif










#endif


