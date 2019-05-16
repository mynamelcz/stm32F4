#ifndef __COMMON_H
#define __COMMON_H

#define BIT(x)  			(1<<x)
#define STE_BIT(x,b)  do{x |=  BIT(b);}while(0)
#define CLR_BIT(x,b)  do{x &= ~BIT(b);}while(0)
#define REV_BIT(x,b)  do{x ^=  BIT(b);}while(0)
#define GET_BIT(x,b)  ((x>>b)&0x1)

void print_code_version(void);






#endif




