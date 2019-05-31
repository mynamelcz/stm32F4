#ifndef __BSP_SPI_H
#define __BSP_SPI_H
#include "includes.h"

typedef struct{
	void (*init)(void(*cs_fun)(u8));
	void (*cs_str)(u8 en);
	void (*read)(u8 *buf, u32 len);
	void (*write)(const u8 *buf, u32 len);	
}__spi_ctr_obj;

extern __spi_ctr_obj spi1_obj;
extern __spi_ctr_obj spi2_obj;


#endif



