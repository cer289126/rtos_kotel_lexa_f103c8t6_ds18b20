#include "1wire.h"

// Буфер для приема/передачи по 1-wire
uint8_t ow_buf[8];

#define OW_0	0x00
#define OW_1	0xff
#define OW_R_1	0xff

//-----------------------------------------------------------------------------
// функция преобразует один байт в восемь, для передачи через USART
// ow_byte - байт, который надо преобразовать
// ow_bits - ссылка на буфер, размером не менее 8 байт
//-----------------------------------------------------------------------------
void OW_toBits(uint8_t ow_byte, uint8_t *ow_bits) {
	uint8_t i;
	for (i = 0; i < 8; i++) {
    if (ow_byte & 0x01) {
    	*ow_bits = OW_1;
    } else {
    	*ow_bits = OW_0;
    }
    ow_bits++;
    ow_byte = ow_byte >> 1;
	}
}

//-----------------------------------------------------------------------------
// обратное преобразование - из того, что получено через USART опять собирается байт
// ow_bits - ссылка на буфер, размером не менее 8 байт
//-----------------------------------------------------------------------------
uint8_t OW_toByte(uint8_t *ow_bits) {
	uint8_t ow_byte, i;
	ow_byte = 0;
	for (i = 0; i < 8; i++) {
    ow_byte = ow_byte >> 1;
    if (*ow_bits == OW_R_1) {
    	ow_byte |= 0x80;
    }
    ow_bits++;
	}

	return ow_byte;
}






//-----------------------------------------------------------------------------
// инициализиция USART и DMA
//-----------------------------------------------------------------------------
void OW_Init(void)
    {
    	//тактирование usart1 от шины PLC1 с частатой шины 72 Мгц
    	
    	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    	RCC->APB2ENR|=RCC_APB2ENR_USART1EN;
    	RCC->APB2ENR|=RCC_APB2ENR_IOPAEN;
    	
    	GPIOA->CRH&= ~(GPIO_CRH_MODE9);
    	GPIOA->CRH|=GPIO_CRH_MODE9_0|GPIO_CRH_MODE9_1;
    	GPIOA->CRH&= ~(GPIO_CRH_CNF9);
    	GPIOA->CRH|= GPIO_CRH_CNF9_0|GPIO_CRH_CNF9_1;
    	
    	USART1->BRR =(APB1CLK+BAUD/2)/BAUD;  // 115200
            USART1->CR1 |= USART_CR1_TE |USART_CR1_RE; // 
    	USART1->CR3 |=USART_CR3_HDSEL;

    	
    	RCC->AHBENR |=RCC_AHBENR_DMA1EN;       //
    	DMA1_Channel5->CPAR = (uint32_t) &(USART1->DR);
    	DMA1_Channel5->CMAR = (uint32_t) ow_buf;        //
    	DMA1_Channel5->CNDTR = 0x08;                //

    	DMA1_Channel5->CCR=0;
    	DMA1_Channel5->CCR  = 
                         
                        	DMA_CCR5_MINC
                        	//DMA_CCR_CIRC|
                        	//DMA_CCR_DIR
                          ;
    
    	DMA1_Channel4->CPAR = (uint32_t)&(USART1->DR);
    	DMA1_Channel4->CMAR = (uint32_t) ow_buf;        //
    	DMA1_Channel4->CNDTR = 0x08;                //

    	DMA1_Channel4->CCR=0;
    	DMA1_Channel4->CCR  = 
                        	 
                        	DMA_CCR6_MINC|
                        	//DMA_CCR_CIRC|
                        	DMA_CCR6_DIR
                        	 ;
                        	
    }
//-----------------------------------------------------------------------------
// осуществляет сброс и проверку на наличие устройств на шине
//-----------------------------------------------------------------------------    
	uint8_t OW_Reset() {
	uint8_t ow_presence;
    
    USART1->CR1 &= ~USART_CR1_UE; //   USART1  	
    USART1->BRR =(APB1CLK+BAUD2/2)/BAUD2; //usart1 9600
    
    USART1->CR1 |= USART_CR1_UE; //   USART1  
    	
    	
    //USART1->ICR|=USART_ICR_TCCF;
    USART1->SR|=USART_SR_TC;
    USART1->DR= 0xf0;
    
    	
     while(!(USART1->SR & USART_SR_TC));
    ow_presence = USART1->DR;
    	
    	
    USART1->CR1 &= ~USART_CR1_UE; //   USART1  	
    USART1->BRR =(APB1CLK+BAUD/2)/BAUD; //usart1  115200
    USART1->CR1 |= USART_CR1_UE; //   USART1  	
	
	if (ow_presence != 0xf0) {
    return OW_OK;
	  }

	return OW_NO_DEVICE;
   }
	
	 
	 
//-----------------------------------------------------------------------------
// процедура общения с шиной 1-wire
// sendReset - посылать RESET в начале общения.
//     OW_SEND_RESET или OW_NO_RESET
// command - массив байт, отсылаемых в шину. Если нужно чтение - отправляем OW_READ_SLOTH
// cLen - длина буфера команд, столько байт отошлется в шину
// data - если требуется чтение, то ссылка на буфер для чтения
// dLen - длина буфера для чтения. Прочитается не более этой длины
// readStart - с какого символа передачи начинать чтение (нумеруются с 0)
//    можно указать OW_NO_READ, тогда можно не задавать data и dLen
//-----------------------------------------------------------------------------
//OW_Send(OW_SEND_RESET, "\xcc\xbe\xff\xff", 4,  buf, 2, 2);	 
uint8_t OW_Send(uint8_t sendReset, char *command, uint8_t cLen,
    uint8_t *data, uint8_t dLen, uint8_t readStart) {

	uint32_t usart_temp=0;
			
	if (sendReset == OW_SEND_RESET) {
    if (OW_Reset() == OW_NO_DEVICE) {
    	return OW_NO_DEVICE;
    }
	}

	while (cLen > 0) {
    USART1->CR1 &= ~USART_CR1_UE; //   USART1 
    OW_toBits(*command, ow_buf);
    command++;
    cLen--;

		   USART1->BRR =(APB1CLK+BAUD/2)/BAUD; //usart1  115200
    USART1->CR1 |= USART_CR1_UE; //   USART1
		
    DMA1_Channel5->CNDTR = 0x08; 
    DMA1_Channel4->CNDTR = 0x08; 
    
    DMA1_Channel5->CCR  |=  DMA_CCR5_EN; 
    DMA1_Channel4->CCR  |=  DMA_CCR4_EN;   
    USART1->CR3|=USART_CR3_DMAR | USART_CR3_DMAT;
    
	
    while(!(DMA1->ISR & DMA_ISR_TCIF5)){};
    DMA1->IFCR |= DMA_IFCR_CTCIF5;   // прием завершен
    
    DMA1_Channel5->CCR  &=  ~DMA_CCR5_EN; 
    DMA1_Channel4->CCR  &=  ~DMA_CCR4_EN;   
    USART1->CR3&= ~(USART_CR3_DMAR | USART_CR3_DMAT);

  
// for(usart_temp=0;usart_temp<8;usart_temp++)
// {
//	   //USART1->SR|=USART_SR_TC;
//     while(!(USART1->SR & USART_SR_TXE)) __NOP;  // write in shit = 1
//         USART1->DR= ow_buf[usart_temp];
//	  while(!(USART1->SR & USART_SR_RXNE)) __NOP ; 
//	        ow_buf[usart_temp]=USART1->DR;
// }
 
    // 
    if (readStart == 0 && dLen > 0) {
    	*data = OW_toByte(ow_buf);
    	data++;
    	dLen--;
    } else {
    	if (readStart != OW_NO_READ) {
        readStart--;
    	}
    }
	}

	return OW_OK;
}

unsigned char Crc8Dallas(unsigned char len, unsigned char *pData)
{
 	unsigned char crc = 0;
 	unsigned char i;

 	while (len--)
 	{
 		crc ^= *pData++;
		for (i = 0; i < 8; i++)
 		crc = crc & 0x01 ? (crc >> 1) ^ 0x8C : crc >> 1;
 	}
	return crc;
}
