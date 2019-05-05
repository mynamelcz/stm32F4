#ifndef __APP_CFG_H
#define __APP_CFG_H
#include "SEGGER_RTT.h"

#define DBUG_MAIN	


#ifdef DBUG_MAIN
#define main_printf		MY_RTT_printf
#else
#define main_printf(...)
#endif


#endif