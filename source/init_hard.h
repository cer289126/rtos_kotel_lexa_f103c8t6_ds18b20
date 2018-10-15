#ifndef LIST_H
#define LIST_H

#include "stm32f10x.h"

extern unsigned int test;

// TIM2 enkoder

#define TIM2_CLK                72000000UL // Hz 
#define TIM2_Internal_Frequency 10000UL     // Hz 
#define TIM2_Out_Frequency      1UL        // Hz
	
#define Prescaler2 (TIM2_CLK /TIM2_Internal_Frequency)-1UL
#define Period2 (TIM2_Internal_Frequency/TIM2_Out_Frequency)-1UL

// TIM1 him

#define TIM1_CLK                72000000UL // Hz 
#define TIM1_Internal_Frequency 72000000UL     // Hz 
#define TIM1_Out_Frequency      72000UL        // Hz
	
#define Prescaler1 (TIM1_CLK /TIM1_Internal_Frequency)-1UL
#define Period1 (TIM1_Internal_Frequency/TIM1_Out_Frequency)-1UL

//TIM3 100mC=10 Hz zabiraem enkoder DMA

#define TIM3_CLK                72000000UL // Hz 
#define TIM3_Internal_Frequency 10000UL     // Hz 
#define TIM3_Out_Frequency      10UL        // Hz
	
#define Prescaler3 (TIM3_CLK /TIM3_Internal_Frequency)-1UL
#define Period3 (TIM3_Internal_Frequency/TIM3_Out_Frequency)-1UL

//TIM4 100mC=10 Hz period delay ds18b20

#define TIM4_CLK    72000000UL // Hz                                                                                                                                                                                                                                                                                                                                                                                                                 72000000UL // Hz 
#define TIM4_Internal_Frequency 10000UL     // Hz 
#define TIM4_Out_Frequency      10UL        // Hz
	
#define Prescaler4 (TIM4_CLK /TIM4_Internal_Frequency)-1UL
#define Period4 (TIM4_Internal_Frequency/TIM4_Out_Frequency)-1UL

extern void init_(void);

#endif

