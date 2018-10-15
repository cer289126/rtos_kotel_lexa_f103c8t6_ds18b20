

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
						 
						 
volatile unsigned long ulIdleCycleCount = 0; 
volatile unsigned char ext_but=0,temp3;
uint8_t buf[10];
unsigned int test;

volatile unsigned char uc_flag_100mc=0, uc_flag_1sec=1;
volatile unsigned char uc_temp_flag=0;

/////////////////////////////////////////////////////////
#define reset_delay_1s() { uc_flag_1sec=1; TIM4->CNT=0; TIM4->SR &= ~TIM_SR_UIF; uc_temp_flag=0; }

/*-----------------------------------------------------------*/
void vTask_ds18b20( void);
void vTask_main( void);
void vTask_setup( void);
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
//void vAutoReloadTimerFunction(TimerHandle_t xTimer_mig) {
//	static char ds=0;
//	if(ds==1)
//		OW_Send(OW_SEND_RESET, "\xcc\x44", 2,  NULL,  NULL,  OW_NO_READ);
//	else
//		temp3=OW_Send(OW_SEND_RESET, "\xcc\xbe\xff\xff", 4,  buf, 2, 2);
//	GPIOC->ODR ^= GPIO_ODR_ODR13;
//}


//////////////////////////////////////////////////////////
// MAIN
int main( void )
{
	char temp_2[20],temp,temp1;
	char txt[11];
	unsigned char ucHim,error_crc_ds18b20=0;
	
	// init_hard
	init_();
	OW_Init();
	
	
	
	reset_delay_1s();
	while(uc_flag_1sec) __NOP;
	
	
	OW_Send(OW_SEND_RESET, "\xcc\x44", 2,  NULL,  NULL,  OW_NO_READ);
	
	reset_delay_1s();
	while(uc_flag_1sec) __NOP;
	
	ssd1306_init();

	
	
	for( ;; )
	{
		sprintf(temp_2,"шимм= %d    ",ucHim);
		ssd1306_Puts(temp_2,1);  // text and stroka
	  ssd1306_update(1);
		
//		 TIM2->ARR = 810 ;
//		 ucHim=TIM2->CNT/4;
//		 if(ucHim>=100)
//					ucHim=100;
		OW_Send(OW_SEND_RESET, "\xcc\x44", 2,  NULL,  NULL,  OW_NO_READ);
	  reset_delay_1s();
	  while(uc_flag_1sec) __NOP;
		GPIOC->ODR ^= GPIO_ODR_ODR13;
     __disable_irq (); 
		 temp3=OW_Send(OW_SEND_RESET, "\xcc\xbe\xff\xff\xff\xff\xff\xff\xff\xff\xff", 11,  buf, 11, 2) ;//2);
		if(Crc8Dallas(9,buf)==0) 
			error_crc_ds18b20=0;
		else 
			error_crc_ds18b20=1;
		
		 __enable_irq ();
		
		temp = ( ((buf[1]&0x07)<<4)|(buf[0]>>4));
		//temp/=2;
		temp1=(buf[0]&0x0F);	
		temp1=((temp1<<1)+(temp1<<3));	
		temp1=(temp1>>4);
		//temp1/=2;
  //  buf[0]=0;
  //  buf[1]=0;		
		sprintf(txt,"Темпер=%d.%d",temp,temp1);
		ssd1306_Puts(txt,2);  // text and stroka
	  ssd1306_update(2);
	}
	// error
	while (1) { __NOP ;}
}






