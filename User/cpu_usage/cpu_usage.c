#include "cpu_usage.h"
#include "cmsis_os.h"


#define CALCULATION_PERIOD 		1000
volatile u32  os_idle_total_time = 0;
volatile u32  os_idle_start_time = 0;
volatile u32  os_idle_spent_time = 0;
volatile u32  os_cpu_usage = 0;

osThreadId idle_task_hd = NULL;
/*
** idle hook 
*/
void vApplicationIdleHook(void)
{
	if(idle_task_hd == NULL){
		idle_task_hd = osThreadGetId();
	}
}

void vApplicationTickHook(void)
{
	static u32 tick = 0;
	if(++tick >= CALCULATION_PERIOD){
		tick  = 0;
		if(os_idle_total_time > CALCULATION_PERIOD){
			os_idle_total_time = CALCULATION_PERIOD;
		}
		os_cpu_usage = (100 - os_idle_total_time*100/CALCULATION_PERIOD);
		os_idle_total_time = 0;
	}
}


void trace_TASK_IN(void)
{
	if(idle_task_hd == osThreadGetId()){
		os_idle_start_time = osKernelSysTick();
	}
}
void trace_TASK_OUT(void)
{
	if(idle_task_hd == osThreadGetId()){
		os_idle_spent_time = osKernelSysTick() - os_idle_start_time;
		os_idle_total_time += os_idle_spent_time;
	}
}

u32 os_get_cpu_usage(void)
{
	return os_cpu_usage;
}

