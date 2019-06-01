#ifndef __BSP_INIT_H
#define __BSP_INIT_H

#include "stm32f4xx_ll.h"
#include "stm32f4xx_hal.h"

/**	FLASH HW **/
#define FLASH_CS_PORT		GPIOB
#define FLASH_CS_PIN		LL_GPIO_PIN_0

/**	SPI SD HW **/
#define SPI_SD_CS_PORT	    GPIOB
#define SPI_SD_CS_PIN		LL_GPIO_PIN_12


void bsp_init(void);


#endif




