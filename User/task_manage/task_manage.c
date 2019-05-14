#include "task_manage.h"
#include "cmsis_os.h"
#include "cpu_usage.h"


#define  TASK_1_PRIO        osPriorityNormal 
#define  TASK_1_STACK_SIZE  128
osThreadId tast_1_hd = NULL;
osMessageQDef(task1_queue, 16 ,u32);
osMessageQId task1_queue_hd = NULL;



#define  TASK_2_PRIO   		     osPriorityHigh          
#define  TASK_2_STACK_SIZE  128
osThreadId tast_2_hd = NULL;

#define  TASK_3_PRIO   		osPriorityAboveNormal        
#define  TASK_3_STACK_SIZE  128
osThreadId tast_3_hd = NULL;

typedef struct {                      /* Mail object structure               */
   u32    voltage;                   /* AD result of measured voltage       */
   u32    current;                   /* AD result of measured current       */
   u32 		counter;                   /* A counter value                     */
} T_MEAS;
 
osMailQDef(mail, 16, T_MEAS);         /* Define mail queue                   */

//osMailQId  mail;

osMailQId		mail_id	=	NULL;






void task_1_fun(void const *param)
{
	(void)param;

	static u32 cnt = 0;
  T_MEAS *mptr;
	while(1)
	{ 
		task_printf("1111111  running  start :%d  \n",++cnt);
		mptr = osMailAlloc(mail_id, osWaitForever);           /* Allocate memory      */
		mptr->voltage = 1;             /* Set the mail content                */
		mptr->current = 2;
		mptr->counter = 3;
		osMailPut(mail_id, mptr);              /* Send Mail                           */

		mptr = osMailAlloc(mail_id, osWaitForever);           /* Allocate memory      */
		mptr->voltage = 4;             /* Set the mail content                */
		mptr->current = 5;
		mptr->counter = 6;
		osMailPut(mail_id, mptr);              /* Send Mail                           */
		
		task_printf("1111111  running  end \n");
		osDelay(1000);
	}
}

void task_2_fun(void const *param)
{
	(void)(param);
	static u32 cnt = 0;
	T_MEAS  *rptr;
	osEvent  evt;
	while(1)
	{
		task_printf("2222222  running  start  :%d  \n",++cnt);
    evt = osMailGet(mail_id, osWaitForever);              /* Send Mail  */
		if(evt.status == osEventMail){
			rptr = (T_MEAS  *)evt.value.p;
			task_printf("counter :%d  \n",rptr->counter);
			task_printf("current :%d  \n",rptr->current);
			task_printf("voltage :%d  \n",rptr->voltage);
		  osMailFree(mail_id, rptr);  
		}
		
		task_printf("2222222  running  end\n");

		
	}
}


void task_3_fun(void const *param)
{
	(void)(param);
	static u32 cnt = 0;
	while(1)
	{
		task_printf("task_3_fun  running  start :%d  \n",++cnt);


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
//    osThreadDef(task_3, task_3_fun, TASK_3_PRIO, 0, TASK_2_STACK_SIZE);
//    tast_3_hd = osThreadCreate(osThread(task_3),NULL); 
	
		mail_id = osMailCreate (osMailQ(mail), NULL);
		

}

