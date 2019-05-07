#include "task_manage.h"
#include "cmsis_os.h"

#define  TASK_1_PRIO        osPriorityNormal        
#define  TASK_1_STACK_SIZE  128
osThreadId tast_1_hd;

#define  TASK_2_PRIO    osPriorityNormal        
#define  TASK_2_STACK_SIZE  128
osThreadId tast_1_hd;




void task_1_fun(void const *param)
{
	(void)(param);
    
	while(1)
	{
	    osDelay(500);	
		task_printf("task_1_fun\n");
	}
}
	

void task_startup(void)
{
	task_printf(">>>>   FUN:%s <<<<\n",__func__);
    osThreadDef(task_1, task_1_fun, TASK_1_PRIO, 0, TASK_1_STACK_SIZE);
    tast_1_hd = osThreadCreate(osThread(task_1),NULL); 
}
