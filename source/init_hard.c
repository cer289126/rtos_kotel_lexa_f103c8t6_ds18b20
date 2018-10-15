#include "init_hard.h"
#include "ssd1306.h"




void init_(void)
{
		RCC->APB2ENR|=RCC_APB2ENR_IOPCEN;
	  RCC->APB2ENR|=RCC_APB2ENR_TIM1EN; // TIM1 Encoder
	  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // TIM2 clock
	  RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; // TIM3 button drebezg
		RCC->APB2ENR|=RCC_APB2ENR_AFIOEN; // 
	  RCC->APB2ENR|=RCC_APB2ENR_IOPAEN;
	  RCC->APB2ENR|=RCC_APB2ENR_IOPBEN;
	  RCC->APB1ENR|=RCC_APB1ENR_I2C1EN;  //  i2c1
	  RCC->AHBENR |= RCC_AHBENR_DMA1EN;  // DMA1
	  //RCC->APB1ENR|=RCC_APB1ENR_PWREN|RCC_APB1ENR_BKPEN;  // RTC
	
	// miganie PC13
		GPIOC->CRH&=~GPIO_CRH_MODE13;
	  GPIOC->CRH|=GPIO_CRH_MODE13_1;   //  50 mgh
	  GPIOC->CRH|=GPIO_CRH_MODE13_0;
	  GPIOC->CRH&=~GPIO_CRH_CNF13; 
	   
	
	  // TIM2 enkoder
		// remap input in PA0- TIM_CH1   PA1-TIM_CH2
				// enkoder
		// input float a0,a1
	  GPIOA->CRL&=~GPIO_CRL_MODE1;  // input
	  GPIOA->CRL&=~GPIO_CRL_CNF1;     
	  GPIOA->CRL|=GPIO_CRL_CNF1_0;  // 01- input float
	
		GPIOA->CRL&=~GPIO_CRL_MODE0;  // input
	  GPIOA->CRL&=~GPIO_CRL_CNF0;     
	  GPIOA->CRL|=GPIO_CRL_CNF0_0;  // 01- input float
		
		//TIM2 encoder
		TIM2->ARR=202;
		//AFIO->MAPR |= AFIO_MAPR_TIM2_REMAP_PARTIALREMAP1; /*!< Partial remap (CH1/ETR/PA15, CH2/PB3, CH3/PA2, CH4/PA3) */
	  TIM2->CCMR1|= TIM_CCMR1_CC1S_0|TIM_CCMR1_CC2S_0 ; // IC2 is mapped on TI2
																											// IC1 is mapped on TI1
	  TIM2->CCMR1 |= TIM_CCMR1_IC1F | TIM_CCMR1_IC2F;   // filtr
	  TIM2->SMCR |= TIM_SMCR_SMS_0 | TIM_SMCR_SMS_1 ;  // SMS=011
	  TIM2->CR1|=TIM_CR1_CEN;
		
		//TIM3 100 mS DMA1 Chanel3 
		TIM3->ARR=Period3;
		TIM3->PSC=Prescaler3;
		
		
		DMA1_Channel3->CPAR = (uint32_t)&(TIM2->CNT); //
		DMA1_Channel3->CMAR = (uint32_t)&test; //
		DMA1_Channel3->CCR &= ~DMA_CCR1_DIR; // 
		DMA1_Channel3->CNDTR = 0x01; // 
		DMA1_Channel3->CCR &= ~DMA_CCR1_PINC; // 
		DMA1_Channel3->CCR |= DMA_CCR1_MINC; // 
		DMA1_Channel3->CCR |= DMA_CCR1_PSIZE_0; // 
		DMA1_Channel3->CCR |= DMA_CCR1_MSIZE_0; // 
		DMA1_Channel3->CCR |= DMA_CCR1_PL; // 
		DMA1_Channel3->CCR |= DMA_CCR1_CIRC; // 
		DMA1_Channel3->CCR |= DMA_CCR1_EN; // Enable chanel DMA
		DMA1_Channel3->CCR |= DMA_CCR1_TCIE;
		
		TIM3->DIER |= TIM_DIER_UDE;
		//TIM3->DIER |= TIM_DIER_UIE;
		TIM3->CR2 |= TIM_CR2_CCDS;
		TIM3->CR1 |= TIM_CR1_CEN;
		
		

		// TIM1 him
		// output pa7-ch1n(output altern push-pull) 
		//pa8-ch1(output altern push-pull) 
		//pa6-break(input float) in reset
		AFIO->MAPR |= AFIO_MAPR_TIM1_REMAP_PARTIALREMAP;
		GPIOA->CRL |= GPIO_CRL_MODE7;
		GPIOA->CRL |= GPIO_CRL_CNF7_1; 
		GPIOA->CRL &= ~GPIO_CRL_CNF7_0;
		GPIOA->CRH |= GPIO_CRH_MODE8;
		GPIOA->CRH |= GPIO_CRH_CNF8_1;
		GPIOA->CRH &= ~GPIO_CRH_CNF8_0;
		
		TIM1->CNT = 0 ;
		TIM1->ARR = Period1 ;   //
		TIM1->PSC = Prescaler1;
		
		TIM1->CR1 |=TIM_CR1_ARPE; // bufer arr
		TIM1->CCMR1 |= TIM_CCMR1_OC1M_2|TIM_CCMR1_OC1M_1|TIM_CCMR1_OC1PE;
		
		TIM1->CCER |= TIM_CCER_CC1NE|TIM_CCER_CC1E;  // low ccn1
		TIM1->CCR1 = 500 ;  // period 1000  72 kHz
	  //NVIC_SetPriority(TIM1_UP_IRQn ,0);
		//NVIC_EnableIRQ(TIM1_UP_IRQn);
		
		TIM1->BDTR |= TIM_BDTR_DTG_6|TIM_BDTR_DTG_2|TIM_BDTR_DTG_1|TIM_BDTR_DTG_0; // 71*14 nS=1000nS
		//TIM1->BDTR |=TIM_BDTR_OSSR;
		TIM1->CR2 |= TIM_CR2_OIS1N;
		TIM1->BDTR |=TIM_BDTR_BKP|TIM_BDTR_BKE|TIM_BDTR_AOE|TIM_BDTR_OSSI;  // TIM1_BKIN
		TIM1->BDTR |= TIM_BDTR_MOE;
		//TIM1->CCER |= TIM_CCER_CC1E;
		TIM1->CR1 |= TIM_CR1_CEN;
		
		TIM4->ARR=Period4;
		TIM4->PSC=Prescaler4;
		TIM4->DIER=TIM_DIER_UIE; // preriv tim4
		TIM4->CR1 |= TIM_CR1_CEN;
}

