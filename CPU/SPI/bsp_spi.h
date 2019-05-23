#ifndef __BSP_SPI_H
#define __BSP_SPI_H
#include "includes.h"

void spi1_init(void);
void spi1_send_buf(const u8 *pData, u16 Size);
void spi1_read_buf(u8 *pData, u16 Size);


u8 spi1_read_byte(void);
u8 spi1_send_read_byte(u8 dat);
#endif



