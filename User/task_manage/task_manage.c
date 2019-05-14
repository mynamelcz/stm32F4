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


EventGroupHandle_t evnt_hd = NULL;

#define TASK_1_EVENT	BIT(0)
#define TASK_2_EVENT	BIT(1)
#define ALL_EVENT			(TASK_1_EVENT | TASK_2_EVENT)

void task_1_fun(void const *param)
{
	(void)param;

	static u32 cnt = 0;

	while(1)
	{ 
		task_printf("task_1_fun  running  start :%d  \n",++cnt);
     xEventGroupSetBits(evnt_hd, TASK_1_EVENT);
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
    xEventGroupSetBits(evnt_hd, TASK_2_EVENT);
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
		if(evnt_hd){
				xEventGroupWaitBits( evnt_hd,
														ALL_EVENT, 
														pdTRUE, 					// pdTRUE :clear when get event,  pdFALSE:
														pdTRUE,						// pdTRUE :wait all events,       pdFALSE: wait one of the events
														portMAX_DELAY);		
		
		}else{
				;
		
		}

    osDelay(10);
    task_printf("task_3_fun  running  end\n");	
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
	
	  evnt_hd = xEventGroupCreate();
		if(evnt_hd == NULL){
			 task_printf("xEventGroupCreate		NULL\n",__func__);
		}
		

}

