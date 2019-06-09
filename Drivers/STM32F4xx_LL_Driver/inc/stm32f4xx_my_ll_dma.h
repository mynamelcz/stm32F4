#ifndef __STM32F4xx_MY_LL_DMA_H
#define __STM32F4xx_MY_LL_DMA_H

#include "stm32f4xx_ll_dma.h"
/* Clear 传输一半：  Stream x half transfer flag.*/
__STATIC_INLINE void LL_DMA_ClearFlag_HTx(DMA_TypeDef *DMAx ,uint32_t stream_num)
{
  uint32_t flag_t[8] =   {DMA_LIFCR_CHTIF0, DMA_LIFCR_CHTIF1, DMA_LIFCR_CHTIF2, DMA_LIFCR_CHTIF3,
				          DMA_HIFCR_CHTIF4, DMA_HIFCR_CHTIF5, DMA_HIFCR_CHTIF6, DMA_HIFCR_CHTIF7};
  
  if(stream_num > 8)
	  return;
  if(stream_num < 4)
		WRITE_REG(DMAx->LIFCR , flag_t[stream_num]);
  else
		WRITE_REG(DMAx->HIFCR , flag_t[stream_num]);
}

/* Clear 传输完成：  */
__STATIC_INLINE void LL_DMA_ClearFlag_TCx(DMA_TypeDef *DMAx, uint32_t stream_num)
{
  uint32_t flag_t[8] =   {DMA_LIFCR_CTCIF0, DMA_LIFCR_CTCIF1, DMA_LIFCR_CTCIF2, DMA_LIFCR_CTCIF3,
				          DMA_HIFCR_CTCIF4, DMA_HIFCR_CTCIF5, DMA_HIFCR_CTCIF6, DMA_HIFCR_CTCIF7};
  
  if(stream_num > 8)
	  return;
  if(stream_num < 4)
		WRITE_REG(DMAx->LIFCR , flag_t[stream_num]);
  else
		WRITE_REG(DMAx->HIFCR , flag_t[stream_num]);
}

/* Clear 传输错误  */
__STATIC_INLINE void LL_DMA_ClearFlag_TEx(DMA_TypeDef *DMAx, uint32_t stream_num)
{
  uint32_t flag_t[8] =   {DMA_LIFCR_CTEIF0, DMA_LIFCR_CTEIF1, DMA_LIFCR_CTEIF2, DMA_LIFCR_CTEIF3,
				          DMA_HIFCR_CTEIF4, DMA_HIFCR_CTEIF5, DMA_HIFCR_CTEIF6, DMA_HIFCR_CTEIF7};
  
  if(stream_num > 8)
	  return;
  if(stream_num < 4)
		WRITE_REG(DMAx->LIFCR , flag_t[stream_num]);
  else
		WRITE_REG(DMAx->HIFCR , flag_t[stream_num]);
}

/* Clear 方向模式错误  */
__STATIC_INLINE void LL_DMA_ClearFlag_DMEx(DMA_TypeDef *DMAx, uint32_t stream_num)
{
  uint32_t flag_t[8] =   {DMA_LIFCR_CDMEIF0, DMA_LIFCR_CDMEIF1, DMA_LIFCR_CDMEIF2, DMA_LIFCR_CDMEIF3,
				          DMA_HIFCR_CDMEIF4, DMA_HIFCR_CDMEIF5, DMA_HIFCR_CDMEIF6, DMA_HIFCR_CDMEIF7};
  
  if(stream_num > 8)
	  return;
  if(stream_num < 4)
		WRITE_REG(DMAx->LIFCR , flag_t[stream_num]);
  else
		WRITE_REG(DMAx->HIFCR , flag_t[stream_num]);
}

/* Clear FIFO错误  */
__STATIC_INLINE void LL_DMA_ClearFlag_FEx(DMA_TypeDef *DMAx, uint32_t stream_num)
{
  uint32_t flag_t[8] =   {DMA_LIFCR_CFEIF0, DMA_LIFCR_CFEIF1, DMA_LIFCR_CFEIF2, DMA_LIFCR_CFEIF3,
				          DMA_HIFCR_CFEIF4, DMA_HIFCR_CFEIF5, DMA_HIFCR_CFEIF6, DMA_HIFCR_CFEIF7};
  
  if(stream_num > 8)
	  return;
  if(stream_num < 4)
		WRITE_REG(DMAx->LIFCR , flag_t[stream_num]);
  else
		WRITE_REG(DMAx->HIFCR , flag_t[stream_num]);
}






/*Get 传输 一半 */
__STATIC_INLINE uint32_t LL_DMA_IsActiveFlag_HTx(DMA_TypeDef *DMAx, uint32_t stream_num)
{
  uint32_t flag_t[8] =  {DMA_LISR_HTIF0, DMA_LISR_HTIF1, DMA_LISR_HTIF2, DMA_LISR_HTIF3,
				         DMA_HISR_HTIF4, DMA_HISR_HTIF5, DMA_HISR_HTIF6, DMA_HISR_HTIF7};
  
	uint32_t flag = flag_t[stream_num];
	if(stream_num < 4)
		return (READ_BIT(DMAx->LISR ,flag)==(flag));
	else
		return (READ_BIT(DMAx->HISR ,flag)==(flag));	
}
/*Get 传输完成 */
__STATIC_INLINE uint32_t LL_DMA_IsActiveFlag_TCx(DMA_TypeDef *DMAx, uint32_t stream_num)
{
	uint32_t flag_t[8] = {DMA_LISR_TCIF0, DMA_LISR_TCIF1, DMA_LISR_TCIF2, DMA_LISR_TCIF3,
						  DMA_HISR_TCIF4, DMA_HISR_TCIF5, DMA_HISR_TCIF6, DMA_HISR_TCIF7};
	
	uint32_t flag = flag_t[stream_num];
	if(stream_num < 4)
		return (READ_BIT(DMAx->LISR ,flag)==(flag));
	else
		return (READ_BIT(DMAx->HISR ,flag)==(flag));						   
}


/*Get  传输错误 */
__STATIC_INLINE uint32_t LL_DMA_IsActiveFlag_TEx(DMA_TypeDef *DMAx ,uint32_t stream_num)
{
  uint32_t flag_t[8] =   {DMA_LISR_TEIF0, DMA_LISR_TEIF1, DMA_LISR_TEIF2, DMA_LISR_TEIF3,
				          DMA_HISR_TEIF4, DMA_HISR_TEIF5, DMA_HISR_TEIF6, DMA_HISR_TEIF7};
  
	uint32_t flag = flag_t[stream_num];
	if(stream_num < 4)
		return (READ_BIT(DMAx->LISR ,flag)==(flag));
	else
		return (READ_BIT(DMAx->HISR ,flag)==(flag));	

}
/* Get 方向模式错误*/
__STATIC_INLINE uint32_t LL_DMA_IsActiveFlag_DMEx(DMA_TypeDef *DMAx ,uint32_t stream_num)
{
  uint32_t flag_t[8] =   {DMA_LISR_DMEIF0, DMA_LISR_DMEIF1, DMA_LISR_DMEIF2, DMA_LISR_DMEIF3,
				          DMA_HISR_DMEIF4, DMA_HISR_DMEIF5, DMA_HISR_DMEIF6, DMA_HISR_DMEIF7};
  
	uint32_t flag = flag_t[stream_num];
	if(stream_num < 4)
		return (READ_BIT(DMAx->LISR ,flag)==(flag));
	else
		return (READ_BIT(DMAx->HISR ,flag)==(flag));
} 

/*Get  FIFO错误 */
__STATIC_INLINE uint32_t LL_DMA_IsActiveFlag_FEx(DMA_TypeDef *DMAx ,uint32_t stream_num)
{
  uint32_t flag_t[8] =   {DMA_LISR_FEIF0, DMA_LISR_FEIF1, DMA_LISR_FEIF2, DMA_LISR_FEIF3,
				          DMA_HISR_FEIF4, DMA_HISR_FEIF5, DMA_HISR_FEIF6, DMA_HISR_FEIF7};
  
	uint32_t flag = flag_t[stream_num];
	if(stream_num < 4)
		return (READ_BIT(DMAx->LISR ,flag)==(flag));
	else
		return (READ_BIT(DMAx->HISR ,flag)==(flag));	

}

























#endif









