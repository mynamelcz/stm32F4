#ifndef __STM32F4xx_MY_LL_DMA_H
#define __STM32F4xx_MY_LL_DMA_H

#include "stm32f4xx_ll_dma.h"

/**  ADD By LCZ      
  * @brief Clear Stream x transfer complete flag.
  * @rmtoll HIFCR or LIFCR CTCIFx    LL_DMA_ClearFlag_TCx
  * @param  DMAx DMAx Instance
  * @retval None
  */
__STATIC_INLINE void LL_DMA_ClearFlag_TCx(DMA_TypeDef *DMAx, uint32_t stream_num)
{
  uint32_t CTCIFx_t[8] = {DMA_LIFCR_CTCIF0, DMA_LIFCR_CTCIF1, DMA_LIFCR_CTCIF2, DMA_LIFCR_CTCIF3,
				          DMA_HIFCR_CTCIF4, DMA_HIFCR_CTCIF5, DMA_HIFCR_CTCIF6, DMA_HIFCR_CTCIF7};
  
  if(stream_num > 8)
	  return;
  if(stream_num < 4)
		WRITE_REG(DMAx->LIFCR , CTCIFx_t[stream_num]);
  else
		WRITE_REG(DMAx->HIFCR , CTCIFx_t[stream_num]);
}



/**
  * @brief Get Stream x transfer complete flag.
  * @rmtoll LISR or HISR TCIFx    LL_DMA_IsActiveFlag_TCx
  * @param  DMAx DMAx Instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_DMA_IsActiveFlag_TCx(DMA_TypeDef *DMAx, uint32_t stream_num)
{
	uint32_t TCIFx_t[8] = {DMA_LISR_TCIF0, DMA_LISR_TCIF1, DMA_LISR_TCIF2, DMA_LISR_TCIF3,
						   DMA_HISR_TCIF4, DMA_HISR_TCIF5, DMA_HISR_TCIF6, DMA_HISR_TCIF7};
	
	uint32_t Lisr_TCIFx = TCIFx_t[stream_num];
	if(stream_num < 4)
		return (READ_BIT(DMAx->LISR ,Lisr_TCIFx)==(Lisr_TCIFx));
	else
		return (READ_BIT(DMAx->HISR ,Lisr_TCIFx)==(Lisr_TCIFx));						   
}


#endif









