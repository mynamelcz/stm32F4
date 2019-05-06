#include "task_manage.h"
#include "cmsis_os.h"

#define  TASK_1_PRIO        osPriorityNormal        
#define  TASK_1_STACK_SIZE  128
osThreadId tast_1_hd;

#define  TASK_2_PRIO    		osPriorityNormal        
#define  TASK_2_STACK_SIZE  128
osThreadId tast_1_hd;




void task_1_fun(void const *param)
{
	
	u8 *ptr = (u8 *)param;
	u32 last_wake_time = osKernelSysTick();
	while(1)
	{
	    osDelayUntil(&last_wake_time, 1000);	
		  task_printf("task_1 tick: %d\n",osKernelSysTick());
	}
}
void task_2_fun(void const *param)
{
	(void)(param);
	while(1)
	{
	    osDelay(1000);	
		  task_printf("task_2 tick: %d\n",osKernelSysTick());

		
	}
}
		
u8 buf[5]={1,2,3,4,5};
void task_startup(void)
{
	  task_printf(">>>>   FUN:%s <<<<\n",__func__);
		
    osThreadDef(task_1, task_1_fun, TASK_1_PRIO, 0, TASK_1_STACK_SIZE);
    tast_1_hd = osThreadCreate(osThread(task_1),(void *)buf); 
	
	  osThreadDef(task_2, task_2_fun, TASK_2_PRIO, 0, TASK_2_STACK_SIZE);
    tast_1_hd = osThreadCreate(osThread(task_2),NULL); 
}

/*
** idle hook 
*/
void vApplicationIdleHook( void )
{
	
}




