#ifndef __BSP_SPI_H
#define __BSP_SPI_H
#include "includes.h"

void spi1_init(void);
void spi1_send_buf(uint8_t *pData, uint16_t Size);
void spi1_read_buf(uint8_t *pData, uint16_t Size);


uint8_t spi1_read_byte(void);
uint8_t spi1_send_read_byte(uint8_t dat);
#endif



