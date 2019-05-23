#ifndef __LOW_POWER_H
#define __LOW_POWER_H
#include "includes.h"

#include "stm32f4xx_hal.h"
void pre_sleep_processing(u32 *expt_idletime);
void post_sleep_processing(u32 *expt_idletime);

#endif


