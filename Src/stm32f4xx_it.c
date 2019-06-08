/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_it.h"
#include "cmsis_os.h"
#include "includes.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

typedef struct{
	u32  time;
	u32  cnt;
	void (*hdl_fun)(void);
	struct list_head list;
}__irq_hd_t;



#define __IRQ_HdlTypedef(fun, times) 	   \
 	 __irq_hd_t fun##irq = {			   \
	 .time  = times,					   \
	 .cnt   = 0,						   \
	 .hdl_fun = fun,          			   \
	 .list = LIST_HEAD_INIT(fun##irq.list) \
	}
	 


#define IRQ_HdlList(fun, time)	&(fun##irq.list)
#define IRQ_Hdl(fun)	    	&(fun##irq)	
	
		
LIST_HEAD(TIM2_IRQHandler_ListHead);
	
void register_timer2_handler(__irq_hd_t *hd_t)
{
	list_add_tail(&hd_t->list, &TIM2_IRQHandler_ListHead);	
}	
	
void register_timer2_handler_malloc(void(*fun)(void), u32 time)
{
	__irq_hd_t *hd_t = malloc(sizeof(__irq_hd_t));
	ASSERT(hd_t);
	hd_t->cnt = 0;
	hd_t->time = time;
	hd_t->hdl_fun = fun;
	hd_t->list.next = &(hd_t->list);
	hd_t->list.prev = &(hd_t->list);
	
	list_add_tail(&hd_t->list, &TIM2_IRQHandler_ListHead);	
}	
	

	
	
	
void TIM2_IRQHandler(void)
{
	volatile static u32 cnt = 0;
	__irq_hd_t *pos = NULL;
	cnt++;
	list_for_each_entry(pos, &TIM2_IRQHandler_ListHead, list){
		if(pos->hdl_fun){
			if(++pos->cnt >= pos->time){
				pos->hdl_fun();
				pos->cnt = 0;
			}
		}else{
			DBUG_Printf("TIM2_IRQHandler err\n");
		}	
	}	  
  
  
}

	 





/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */ 
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */

  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */
  DBUG_Printf("HardFault_Handler\n");
  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
	  
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}


void SysTick_Handler(void)
{
	
  HAL_IncTick();
  osSystickHandler();

}
/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}




void SDIO_IRQHandler(void)
{
 


}

/**
  * @brief This function handles DMA2 stream7 global interrupt.
  */

extern void SD_ProcessDMAIRQ(void);
void DMA2_Stream7_IRQHandler(void)
{	
	if(LL_DMA_IsActiveFlag_TC7(DMA2)){
	//Transfer Complete
		LL_DMA_ClearFlag_TC7(DMA2);
	}
	SD_ProcessDMAIRQ();
	
}	

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
