

#include "bsp_sdio.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_ll.h"






void bsp_sdio_init(SD_HandleTypeDef *sdio_hd)
{
  sdio_hd->Instance = SDIO;
  sdio_hd->Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
  sdio_hd->Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
  sdio_hd->Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
  sdio_hd->Init.BusWide = SDIO_BUS_WIDE_1B;
  sdio_hd->Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_ENABLE;
  sdio_hd->Init.ClockDiv = 0;
}




__sdio_ctr_obj sdio_ctr_obj = {
	.init = bsp_sdio_init, 
};


