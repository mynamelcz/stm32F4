#ifndef __BSP_SPI_H
#define __BSP_SPI_H
#include "includes.h"

void spi1_init(void);
uint8_t spi1_read_byte(void);
void spi1_send_byte(uint8_t dat);
#endif



