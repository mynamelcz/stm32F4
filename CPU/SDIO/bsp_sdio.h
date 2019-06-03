#ifndef __BSP_SDIO_H
#define __BSP_SDIO_H
#include "includes.h"



#define SDIO_DMA_NUM			DMA2
#define SDIO_DMA_CHL			LL_DMA_CHANNEL_4
#define SDIO_DMA_DMA_CLK		LL_AHB1_GRP1_PERIPH_DMA2
#define SDIO_FIFO_PBASE			(u32)(&(SDIO->FIFO))
#define SDIO_DMA_USE_FIFO		1


#define SDIO_DMA_STRAM_NUM		3		//6  or 3
#if(SDIO_DMA_STRAM_NUM == 6)
#define SDIO_DMA_STRAM			LL_DMA_STREAM_6
#define SDIO_DMA_INT			DMA2_Stream6_IRQn
#else
#define SDIO_DMA_STRAM			LL_DMA_STREAM_3
#define SDIO_DMA_INT			DMA2_Stream3_IRQn
#endif






#endif





