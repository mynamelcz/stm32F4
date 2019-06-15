#ifndef __SOFT_SPI_H
#define __SOFT_SPI_H
#include "includes.h"


typedef enum __SPI_TYPE{
	CPOL0_CPHA0,		// CLK  LOW,  1 age
	CPOL0_CPHA1,		// CLK  LOW,  2 age
	CPOL1_CPHA0,
	CPOL1_CPHA1,
}SPI_TYPE_E;

typedef struct __Soft_SPI_hd {
	SPI_TYPE_E type;
	u8  (*MISO)(void);
	void (*MOSI)(u8 en);
	void (*CLK)(u8 en);	
}Soft_SPI_hd;


typedef struct __Soft_SPI_IO_Type{
	u8   (*w_r_byte)(Soft_SPI_hd *hd, u8 s_dat);
	void (*write)(Soft_SPI_hd *hd, const u8 *buf, u32 len);
	void (*read)(Soft_SPI_hd *hd, u8 *buf, u32 len);
} const Soft_SPI_IO_Type;

extern Soft_SPI_IO_Type soft_spi_io;




#endif


