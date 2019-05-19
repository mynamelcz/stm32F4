#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H
#include "includes.h"

void spi_flash_init(void);
u16 flash_read_id(void);
#endif


