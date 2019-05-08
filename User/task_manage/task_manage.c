#include "task_manage.h"
#include "cmsis_os.h"
#include "cpu_usage.h"


#define  TASK_1_PRIO        osPriorityNormal        
#define  TASK_1_STACK_SIZE  128
osThreadId tast_1_hd = NULL;
osMessageQDef(task1_queue, 16 ,u32);
osMessageQId task1_queue_hd = NULL;



#define  TASK_2_PRIO   		osPriorityNormal        
#define  TASK_2_STACK_SIZE  128
osThreadId tast_2_hd = NULL;

osMessageQDef(task2_queue, 16, u32);
osMessageQId task2_queue_hd = NULL;

osSemaphoreDef(sem_test);
osSemaphoreId sem_test_id = NULL;



osSemaphoreId osSemaphoreCreate (const osSemaphoreDef_t *semaphore_def, int32_t count);
int32_t osSemaphoreWait (osSemaphoreId semaphore_id, uint32_t millisec);
osStatus osSemaphoreRelease (osSemaphoreId semaphore_id);


void task_1_fun(void const *param)
{
	u8 *ptr = (u8 *)param;
	u32 last_wake_time = osKernelSysTick();
	u32 cpu_usage = 0;


	while(1)
	{    
		cpu_usage = os_get_cpu_usage();
		task_printf("cpu_usage: %d\n",cpu_usage);
		osDelayUntil(&last_wake_time, 1000);
			task_printf("osSemaphoreRelease\n");
		osSemaphoreRelease (sem_test_id);
	
//		if(task2_queue_hd != NULL){
//			if(osMessagePut(task2_queue_hd, cpu_usage, 10) != osOK){
//				task_printf("osMessagePut task2_queue_hd ERR !!\n");
//			}
//		}
		
	
	}
}

void task_2_fun(void const *param)
{
	(void)(param);
	u32 ret = 0;
	while(1)
	{
		ret = osSemaphoreWait(sem_test_id, 2000);
		if(ret == osOK){
			task_printf("osSemaphoreWait\n");
		}
//        msg_event = osMessageGet (task2_queue_hd, 100);
//		if(msg_event.status == osEventMessage){
//			task_printf("msg_event: %d \n", msg_event.value.v);
//		}else{
//			task_printf("osMessageGet task2_queue_hd ERR !!\n");
//		}
//	    osDelay(1000);	
//		task_printf("task_2 tick: %d\n",osKernelSysTick());	
	}
}






u8 buf[5]={1,2,3,4,5};
void task_startup(void)
{
    task_printf(">>>>   FUN:%s <<<<\n",__func__);
		
    task1_queue_hd = osMessageCreate(osMessageQ(task1_queue),NULL);
	task2_queue_hd = osMessageCreate(osMessageQ(task2_queue),NULL);
	
    osThreadDef(task_1, task_1_fun, TASK_1_PRIO, 0, TASK_1_STACK_SIZE);
    tast_1_hd = osThreadCreate(osThread(task_1),(void *)buf); 
    osThreadDef(task_2, task_2_fun, TASK_2_PRIO, 0, TASK_2_STACK_SIZE);
    tast_2_hd = osThreadCreate(osThread(task_2),NULL); 

	sem_test_id = osSemaphoreCreate(osSemaphore(sem_test), 1);
	if(sem_test_id == NULL){
		task_printf("osSemaphoreCreate sem_test_id ERR !!\n");
	}

}

