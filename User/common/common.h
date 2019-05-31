#ifndef __COMMON_H
#define __COMMON_H
#include "includes.h"


#define BIT(x)  			(1<<x)
#define STE_BIT(x,b)  do{x |=  BIT(b);}while(0)
#define CLR_BIT(x,b)  do{x &= ~BIT(b);}while(0)
#define REV_BIT(x,b)  do{x ^=  BIT(b);}while(0)
#define GET_BIT(x,b)  ((x>>b)&0x1)




#ifdef __ASSERT_PARAM
void assert_fail(u8* file, u32 line);
#define ASSERT(expr) ((expr) ? (void)0U : assert_fail((uint8_t *)__FILE__, __LINE__))
#else
#define ASSERT(expr) ((void)0U)	 
#endif


void sys_delay_us(u32 us);
void print_code_version(void);






#endif




