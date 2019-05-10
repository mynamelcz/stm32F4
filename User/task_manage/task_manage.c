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
osMutexId mute_test_id = NULL;
osTimerId timer1_id = NULL;
osTimerId timer2_id = NULL;

void soft_timer1_isr(TimerHandle_t Timer_hd)
{

	task_printf("soft_timer1_isr name:%s\n", pcTimerGetTimerName((TimerHandle_t)(Timer_hd)));
}
void soft_timer2_isr(void const *Timer_hd)
{
	(void )Timer_hd;
	task_printf("soft_timer2_isr\n");
}

//osTimerDef(timer1, soft_timer1_isr);
osTimerDef(timer2, soft_timer2_isr);


void task_1_fun(void const *param)
{
	(void)param;

	static u32 cnt = 0;

	while(1)
	{ 
		task_printf("task_1_fun  running  start :%d  \n",++cnt);

		osDelay(1000);
	}
}

void task_2_fun(void const *param)
{
	(void)(param);
	static u32 cnt = 0;
	while(1)
	{
		task_printf("task_2_fun  running  start  :%d  \n",++cnt);

		task_printf("task_2_fun  running  end\n");
		osDelay(1000);
		
	}
}


void task_3_fun(void const *param)
{
	(void)(param);
	static u32 cnt = 0;
	while(1)
	{
		task_printf("task_3_fun  running  start :%d  \n",++cnt);
		
    task_printf("task_3_fun  running  end\n");
		osDelay(1000);
    		
	}
}



u8 buf[5]={1,2,3,4,5};
void task_startup(void)
{
    task_printf(">>>>   FUN:%s <<<<\n",__func__);
		

	
    osThreadDef(task_1, task_1_fun, TASK_1_PRIO, 0, TASK_1_STACK_SIZE);
    tast_1_hd = osThreadCreate(osThread(task_1),(void *)1); 
    osThreadDef(task_2, task_2_fun, TASK_2_PRIO, 0, TASK_2_STACK_SIZE);
    tast_2_hd = osThreadCreate(osThread(task_2),NULL); 
    osThreadDef(task_3, task_3_fun, TASK_3_PRIO, 0, TASK_2_STACK_SIZE);
    tast_3_hd = osThreadCreate(osThread(task_3),NULL); 
	  
	  //timer1_id = osTimerCreate(osTimer(timer1), osTimerOnce,     (void *)1);
	timer1_id =  xTimerCreate((const char *)"timer1",
                      1000, 
                      pdTRUE,
                      (void *) 1,
                      soft_timer1_isr);
	  timer2_id = osTimerCreate(osTimer(timer2), osTimerPeriodic, (void *)2);
		task_printf("osTimerCreate\n");
	  xTimerStart( timer1_id, 3000);
			task_printf("xTimerStart\n");

		sem_test_id = xSemaphoreCreateBinary();
		mute_test_id = xSemaphoreCreateMutex();
		

}

