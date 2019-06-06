#ifndef __COMMON_H
#define __COMMON_H
#include "includes.h"


#define BIT(x)  			(1<<x)
#define STE_BIT(x,b)  do{x |=  BIT(b);}while(0)
#define CLR_BIT(x,b)  do{x &= ~BIT(b);}while(0)
#define REV_BIT(x,b)  do{x ^=  BIT(b);}while(0)
#define GET_BIT(x,b)  ((x>>b)&0x1)


#define N_BIT1(x)			(BIT(x)-1)
#define BITS_SET(x,s,n,v)	do{x= (x&(~(N_BIT1(n)<<s)))|(v<<s);}while(0)



#ifdef __ASSERT_PARAM
void assert_fail(u8* file, u32 line);
#define ASSERT(expr) ((expr) ? (void)0U : assert_fail((u8 *)__FILE__, __LINE__))
#else
#define ASSERT(expr) ((void)0U)	 
#endif



void sys_delay_us(u32 us);
void print_code_version(void);

void my_memcpy(char *tar, const char *src, u32 len);




#endif




