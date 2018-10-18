

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "init_hard.h"
#include "ssd1306.h"
#include "1wire.h"


#include "semphr.h"
#include "timers.h"


/*** descript RTOS ***/
TaskHandle_t xTask_ds18b20_Handle,
             xTask_setup_Handle,
             xTask_main_Handle;
						 
TimerHandle_t xTimer_mig;
QueueHandle_t xQueue_ds18b20;

/////////////////////////////////////////////
//    Data and Struct

struct Message
{
	uint8_t znak_ds18b20;
	uint8_t temp_integer_ds18b20;
	uint8_t temp_divis_ds18b20;
	uint8_t crc_ds18b20;
} xMess_send_ds18b20,xMess_rec_ds18b20;


volatile unsigned long ulIdleCycleCount = 0; 
volatile unsigned char ext_but=0,temp3;

unsigned int test;

volatile unsigned char uc_flag_100mc=0, uc_flag_1sec=1;
volatile unsigned char uc_temp_flag=0;

/////////////////////////////////////////////////////////
#define reset_delay_1s() { uc_flag_1sec=1; TIM4->CNT=0; TIM4->SR &= ~TIM_SR_UIF; uc_temp_flag=0; }

/*-----------------------------------------------------------*/
void vTask_ds18b20( void *pvParametrs);
void vTask_main( void *pvParametrs);
void vTask_setup( void *pvParametrs);
void init_(void);
void output_ssd1306(void);

///////////////////////////////////////////////////////////////////
// INT

void DMA1_Channel3_IRQHandler(void)
{
	if(DMA1->ISR & DMA_ISR_TCIF3)
	{
		DMA1->IFCR |= DMA_IFCR_CTCIF3;
	}
}

void TIM4_IRQHandler(void)   // 100 mc
{
	if(TIM4->SR & TIM_SR_UIF)
	{
		TIM4->SR &= ~TIM_SR_UIF;
		uc_flag_100mc=1;
		uc_temp_flag++;
		if(uc_temp_flag>=10 && (~uc_flag_1sec))
		{
			uc_flag_1sec=0;
			uc_temp_flag=0;
		}
	}
}
//////////////////////////////////////////////////////////
// TIMER RTOS
void vAutoReloadTimerFunction(TimerHandle_t xTimer_mig) {

	GPIOC->ODR ^= GPIO_ODR_ODR13;
}


//////////////////////////////////////////////////////////
// MAIN
int main( void )
{
	char temp_2[20],error_queue;
	
	unsigned char ucHim;
	BaseType_t xReturned;
	
	
	// init_hard
	init_();
	OW_Init();
	//ssd1306_init();
	
	OW_Send(OW_SEND_RESET, "\xcc\x44", 2,  NULL,  NULL,  OW_NO_READ);
	
	
	//Queue -----------------------------------------------------------
	xQueue_ds18b20 = xQueueCreate(1, sizeof( struct Message * ));
	
	
	// error queue
	 if( xQueue_ds18b20 == 0 )
    {
        while(1) error_queue=1;
    }
		
		/////////////////////////////////////////////////////////////
		// Task------------------------------------------------------------
	  xReturned=xTaskCreate( vTask_main, 
		( const char * ) "Task_main",
		configMINIMAL_STACK_SIZE,
		NULL, 
		3,
		&xTask_main_Handle );
		
	if( xReturned == pdPASS )
    {
        /* The task was created.  Use the task's handle to delete the task. */
        //vTaskDelete( xTask_main_Handle );
    }

	
	   xReturned=xTaskCreate( vTask_ds18b20, 
		( const char * ) "Task_ds18b20",
		configMINIMAL_STACK_SIZE,
		NULL, 
		2,
		&xTask_ds18b20_Handle );
		
	if( xReturned == pdPASS )
    {
        /* The task was created.  Use the task's handle to delete the task. */
       // vTaskDelete( xTask_ds18b20_Handle );
    }
		
		xReturned=xTaskCreate( vTask_setup, 
		( const char * ) "Task_setup",
		configMINIMAL_STACK_SIZE,
		NULL, 
		2,
		&xTask_setup_Handle );
		
	if( xReturned == pdPASS )
    {
        /* The task was created.  Use the task's handle to delete the task. */
        //vTaskDelete( xTask_setup_Handle );
    }
	//Timer Rtos ---------------------------------------------------------------------
	xTimer_mig = xTimerCreate
                   ( /* Just a text name, not used by the RTOS
                     kernel. */
                     "Timer_mig",
                     /* The timer period in ticks, must be
                     greater than 0. */
                     500/portTICK_PERIOD_MS,
                     /* The timers will auto-reload themselves
                     when they expire. */
                     pdTRUE,
                     /* The ID is used to store a count of the
                     number of times the timer has expired, which
                     is initialised to 0. */
                     ( void * ) 0,
                     /* Each timer calls the same callback when
                     it expires. */
                     vAutoReloadTimerFunction
                   );

         if( xTimer_mig == NULL )
         {
             /* The timer was not created. */
         }
         else
         {
             /* Start the timer.  No block time is specified, and
             even if one was it would be ignored because the RTOS
             scheduler has not yet been started. */
             if( xTimerStart( xTimer_mig, 0 ) != pdPASS )
             {
                 /* The timer could not be set into the Active
                 state. */
             }
         }
	// start -----------------------------------------------------------------
	vTaskStartScheduler();
	
	// error
	while (1) {  }
//	
//	for( ;; )
//	{
//		sprintf(temp_2,"шимм= %d    ",ucHim);
//		ssd1306_Puts(temp_2,1);  // text and stroka
//	  ssd1306_update(1);
//		
////		 TIM2->ARR = 810 ;
////		 ucHim=TIM2->CNT/4;
////		 if(ucHim>=100)
////					ucHim=100;

//		
//		
//	}
//	// error
//	while (1) { __NOP ;}
}


///////////////////////////////////////////////////////
// Task

void vTask_main( void *pvParametrs)
{
	uint8_t txt[11], temp_rec,temp1_rec,crc_rec,znak_rec;
	uint8_t buf[10];
	struct Message *pxMessage_rec;
	//pxMessage_rec=&xMess_rec_ds18b20;
	for(;;)
	{
		if(xQueueReceive( xQueue_ds18b20, (void *) &pxMessage_rec , 0 ))

		temp_rec=pxMessage_rec->temp_integer_ds18b20;
		temp1_rec=pxMessage_rec->temp_divis_ds18b20;
		crc_rec=pxMessage_rec->crc_ds18b20;
		znak_rec=pxMessage_rec->znak_ds18b20;
		
		
		//temp1/=2;
  //  buf[0]=0;
  //  buf[1]=0;
		if(crc_rec)
		{
			sprintf(txt,"Темпер=error");
			//ssd1306_Puts(txt,2);  // text and stroka
			//ssd1306_update(2);
		}
		else
		{
			sprintf(txt,"Темпер=%d.%d",temp_rec,temp1_rec);
			//ssd1306_Puts(txt,2);  // text and stroka
			//ssd1306_update(2);
		}
		vTaskDelay( 500/portTICK_PERIOD_MS );
	}
	
}

// --------------------------------------

void vTask_ds18b20( void *pvParametrs)
{
	uint8_t error_crc_ds18b20=0,temp,temp1;
	uint8_t buff[10];
	TickType_t xLastWakeTime;
	
	struct Message *pxMessage_send;
	pxMessage_send=&xMess_send_ds18b20;
	
	xLastWakeTime = xTaskGetTickCount();
	
	for(;;)
	{
		
     __disable_irq (); 
		 temp3=OW_Send(OW_SEND_RESET, "\xcc\xbe\xff\xff\xff\xff\xff\xff\xff\xff\xff", 11,  buff, 11, 2) ;//2);
		if(Crc8Dallas(9,buff)==0) 
			error_crc_ds18b20=0;
		else 
			error_crc_ds18b20=1;
		
		 __enable_irq ();
		
		temp = ( ((buff[1]&0x07)<<4)|(buff[0]>>4));
		//temp/=2;
		temp1=(buff[0]&0x0F);	
		temp1=((temp1<<1)+(temp1<<3));	
		temp1=(temp1>>4);
		
		// send znak,temp,crc
		
		pxMessage_send->temp_integer_ds18b20=temp;
		pxMessage_send->temp_divis_ds18b20=temp1;
		pxMessage_send->crc_ds18b20=error_crc_ds18b20;
		pxMessage_send->znak_ds18b20=0;
		
		 xQueueSend( xQueue_ds18b20, ( void * ) &pxMessage_send, ( TickType_t ) 0 );
		
		OW_Send(OW_SEND_RESET, "\xcc\x44", 2,  NULL,  NULL,  OW_NO_READ);
		
		vTaskDelayUntil( &xLastWakeTime, 1000/portTICK_PERIOD_MS );
	}
	
}

//--------------------------------------------

void vTask_setup( void *pvParametrs)
{
	
	for(;;)
	{
		
		
	}
	
}


