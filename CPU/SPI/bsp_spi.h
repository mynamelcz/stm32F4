#ifndef __BSP_SPI_H
#define __BSP_SPI_H
#include "includes.h"

#define SPI_CLK_PRE2		0    	    
#define SPI_CLK_PRE4		1		       
#define SPI_CLK_PRE8		2       
#define SPI_CLK_PRE16		3       
#define SPI_CLK_PRE32		4       
#define SPI_CLK_PRE64		5         
#define SPI_CLK_PRE128		6       
#define SPI_CLK_PRE256		7       


typedef enum{
	SPI_SET_SPEED_CMD,
	SPI_SET_SPEED_LOW,
	SPI_SET_SPEED_HIGH,

}__spi_bsp_cmd;

typedef struct{
	void (*init)(void(*cs_fun)(u8));
	void (*cs_str)(u8 en);
	void (*read)(u8 *buf, u32 len);
	void (*write)(const u8 *buf, u32 len);
	u8   (*w_r_byte)(u8 dat);
    bool (*io_ctr)(u8 cmd, void *buf);	
}__spi_ctr_obj;

extern __spi_ctr_obj spi1_obj;
extern __spi_ctr_obj spi2_obj;
extern __spi_ctr_obj soft_spi_obj;
 
#endif



