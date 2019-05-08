#include "task_manage.h"
#include "cmsis_os.h"
#include "cpu_usage.h"


#define  TASK_1_PRIO        osPriorityNormal        
#define  TASK_1_STACK_SIZE  128
osThreadId tast_1_hd = NULL;
osMessageQDef(task1_queue, 16 ,u32);
osMessageQId task1_queue_hd = NULL;



#define  TASK_2_PRIO   		osPriorityHigh        
#define  TASK_2_STACK_SIZE  128
osThreadId tast_2_hd = NULL;

#define  TASK_3_PRIO   		osPriorityAboveNormal        
#define  TASK_3_STACK_SIZE  128
osThreadId tast_3_hd = NULL;


osSemaphoreId sem_test_id = NULL;






void task_1_fun(void const *param)
{
	(void)param;
	u32 last_wake_time = osKernelSysTick();
	static u32 cnt = 0;


	while(1)
	{ 
		task_printf("osPriorityNormal  running  start :%d  \n",++cnt);
    osSemaphoreWait(sem_test_id, osWaitForever);	
    task_printf("osPriorityNormal  get sem\n");		
		osDelay(3000);
		task_printf("osPriorityNormal  delay\n");
		osDelay(1000);
		

	 
		osSemaphoreRelease(sem_test_id);
	}
}

void task_2_fun(void const *param)
{
	(void)(param);
	static u32 cnt = 0;
	while(1)
	{
		task_printf("osPriorityHigh  running  start  :%d  \n",++cnt);
		osSemaphoreWait(sem_test_id, osWaitForever);	
    task_printf("osPriorityHigh  get sem\n");
		
		osSemaphoreRelease(sem_test_id);
		osDelay(1000);
		task_printf("osPriorityHigh  running  end\n");
	}
}


void task_3_fun(void const *param)
{
	(void)(param);
	static u32 cnt = 0;
	while(1)
	{
		task_printf("osPriorityAboveNormal  running  start :%d  \n",++cnt);
    task_printf("osPriorityAboveNormal  running  end\n");
		task_printf("osPriorityAboveNormal  delay\n");
		task_printf("osPriorityAboveNormal  delay\n");
		task_printf("osPriorityAboveNormal  delay\n");
		task_printf("osPriorityAboveNormal  delay\n");
		task_printf("osPriorityAboveNormal  delay\n");	
		osDelay(1000);
    task_printf("osPriorityAboveNormal  running  end\n");		
	}
}



u8 buf[5]={1,2,3,4,5};
void task_startup(void)
{
    task_printf(">>>>   FUN:%s <<<<\n",__func__);
		

	
    osThreadDef(task_1, task_1_fun, TASK_1_PRIO, 0, TASK_1_STACK_SIZE);
    tast_1_hd = osThreadCreate(osThread(task_1),(void *)buf); 
    osThreadDef(task_2, task_2_fun, TASK_2_PRIO, 0, TASK_2_STACK_SIZE);
    tast_2_hd = osThreadCreate(osThread(task_2),NULL); 

    osThreadDef(task_3, task_3_fun, TASK_3_PRIO, 0, TASK_2_STACK_SIZE);
    tast_3_hd = osThreadCreate(osThread(task_3),NULL); 
	
		sem_test_id = xSemaphoreCreateBinary();
		if(sem_test_id == NULL){
			task_printf("osSemaphoreCreate sem_test_id ERR !!\n");
		}
		osSemaphoreRelease(sem_test_id);

}

