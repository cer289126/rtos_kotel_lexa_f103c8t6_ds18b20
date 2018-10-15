#include "ssd1306.h"
#include "font_my.h"

// screen ssd1306
uint8_t s_ssd1306[8][128];  // 64/12=4 strok 1024 bait

void i2c_init(void)
	{
		
		RCC->APB2ENR|=RCC_APB2ENR_IOPCEN;
	  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // TIM2
		RCC->APB2ENR|=RCC_APB2ENR_AFIOEN; // 
	  RCC->APB2ENR|=RCC_APB2ENR_IOPAEN;
	  RCC->APB2ENR|=RCC_APB2ENR_IOPBEN;
	  RCC->APB1ENR|=RCC_APB1ENR_I2C1EN;  //  i2c1
	  RCC->AHBENR |= RCC_AHBENR_DMA1EN;  // DMA1
		
				// настраиваем PB6-SCL и PB7-SDA для i2c
		GPIOB->CRL|=GPIO_CRL_MODE6_1;
		GPIOB->CRL|=GPIO_CRL_MODE6_0;  // альтернативный опен_драйв
		GPIOB->CRL&=~GPIO_CRL_CNF6;
		GPIOB->CRL|=GPIO_CRL_CNF6_1;  // 11= 50 Mhz
		GPIOB->CRL|=GPIO_CRL_CNF6_0;

    GPIOB->CRL|=GPIO_CRL_MODE7_1;
		GPIOB->CRL|=GPIO_CRL_MODE7_0;  // альтернативный опен_драйв
		GPIOB->CRL&=~GPIO_CRL_CNF7;
		GPIOB->CRL|=GPIO_CRL_CNF7_1;  // 11= 50 Mhz
		GPIOB->CRL|=GPIO_CRL_CNF7_0;
		
    // Настройка модуля i2c 400 Khz
		I2C1->CR2 &= ~I2C_CR2_FREQ;
		I2C1->CR2|=36; // 100100=36 Mhz  
		I2C1->CCR &= ~I2C_CCR_DUTY; // задержка =0
		I2C1->CCR|=I2C_CCR_FS;  // FS=1  для 400 Кгц
		I2C1->CCR&=~I2C_CCR_CCR; // сбрасываем ccr
		I2C1->CCR|= 30;  // CCR= 2500nS/ 3*  TPCLK1 = 2500/3*28= 30
		I2C1->TRISE=12; // (TRISE=RISE/tPLCK)+1=(300 нС/28 нС)+1=12
		
		// прерывания включаем
		//I2C1->CR2|=I2C_CR2_ITEVTEN;
		
		//NVIC_EnableIRQ(I2C1_EV_IRQn);
		//NVIC_SetPriority(I2C1_EV_IRQn,1);
		//__enable_fiq ();
		//__enable_irq ();

    I2C1->CR1|=I2C_CR1_PE;
 
	}

void ssd1306_init(void)
{
	
	i2c_init();
    	
  ssd1306_SendCommand(SSD1306_DISPLAY_OFF);
  ssd1306_SendCommand(SSD1306_SET_DISPLAY_CLOCK_DIV);
  ssd1306_SendCommand(0x80);  
  ssd1306_SendCommand(SSD1306_SET_MULTIPLEX_RATIO);
  ssd1306_SendCommand(0x3F);
  ssd1306_SendCommand(SSD1306_SET_DISPLAY_OFFSET);
  ssd1306_SendCommand(0x00);
  ssd1306_SendCommand(SSD1306_SET_START_LINE | 0x00);
  ssd1306_SendCommand(SSD1306_SET_CHARGE_PUMP);
  ssd1306_SendCommand(0x14);
  ssd1306_SendCommand(SSD1306_MEMORY_ADDRESS_MODE);
  ssd1306_SendCommand(0x00);
  ssd1306_SendCommand(SSD1306_SEGMENT_REMAP /*| 0x01*/);
	ssd1306_SendCommand(SSD1306_COM_SCAN_NORMAL );
  //ssd1306_SendCommand(SSD1306_COM_SCAN_INVERSE);
  ssd1306_SendCommand(SSD1306_SET_COM_PINS_CONFIG);
  ssd1306_SendCommand(0x12);
  ssd1306_SendCommand(SSD1306_SET_CONTRAST);
  ssd1306_SendCommand(0x7f);  //  f0
//	ssd1306_SendCommand(0x10);
  ssd1306_SendCommand(SSD1306_SET_PRECHARGE_PERIOD);
  ssd1306_SendCommand(0xF1);
  ssd1306_SendCommand(SSD1306_SET_VCOM_DESELECT_LVL);
  ssd1306_SendCommand(0x40);
  ssd1306_SendCommand(SSD1306_ENTIRE_DISPLAY_RESUME);
  ssd1306_SendCommand(SSD1306_NORMAL_DISPLAY);
  ssd1306_FillDisplay(0x00);
  ssd1306_SendCommand(SSD1306_DISPLAY_ON);
}



	void ssd1306_SendCommand(unsigned char command)
{
  
	ssd1306_send(SSD1306_COMMAND_MODE, command);
	
	
	
}
void 	ssd1306_send(uint8_t control_byte, uint8_t data)
    {


		I2C1->CR1 |= I2C_CR1_START;

    	
    	while (!(I2C1->SR1 & I2C_SR1_SB))
    	{
    	}
    	(void) I2C1->SR1;

    
      
    	I2C1->DR = SSD1306_ADDRESS;
           	
    	while (!(I2C1->SR1 & I2C_SR1_ADDR))
            {
            }
            	(void) I2C1->SR1;
            	(void) I2C1->SR2;
    	
         I2C1->DR=control_byte;
        	while (!(I2C1->SR1 & I2C_SR1_TXE))
            {
            }

		I2C1->DR=data;
    
		while (!(I2C1->SR1 & I2C_SR1_BTF))
		{
		}
		I2C1->CR1 |= I2C_CR1_STOP;
    
    
	}
    
void ssd1306_SendData(uint8_t *data, uint8_t count)
{ 
    uint8_t index=0;
	I2C1->CR1 |= I2C_CR1_START;

    	
    while (!(I2C1->SR1 & I2C_SR1_SB))
    {
    }
    (void) I2C1->SR1;

    
     
    I2C1->DR = SSD1306_ADDRESS;
           	
    while (!(I2C1->SR1 & I2C_SR1_ADDR))
    {
    }
    (void) I2C1->SR1;
    (void) I2C1->SR2;
    	
    I2C1->DR=SSD1306_DATA_MODE;
    while (!(I2C1->SR1 & I2C_SR1_TXE))
    {
    }
	
	for(index=0;index<=count-1;index++)
		{
			I2C1->DR=*data++ ;
			while (!(I2C1->SR1 & I2C_SR1_TXE))
            {
            }
		}	
	 
	 while (!(I2C1->SR1 & I2C_SR1_BTF))
    {
    }
    I2C1->CR1 |= I2C_CR1_STOP;
	 
}        	
    	
void ssd1306_SetCursor(unsigned char x, unsigned char p)
{
  ssd1306_SendCommand(SSD1306_SET_LCOL_START_ADDRESS | (x & 0x0F));
  ssd1306_SendCommand(SSD1306_SET_HCOL_START_ADDRESS | (x >> 4));
  ssd1306_SendCommand(SSD1306_SET_PAGE_START_ADDRESS | p);
}        	
        	
        	

	void ssd1306_FillDisplay(unsigned char data)
{
  unsigned char page, x;
 
  
  uint8_t ssd1306_buf1[128];
  
  for (page=0; page<8; page++)
  {	
    ssd1306_SetCursor(0, page);     
    for (x=0; x<SSD1306_WIDTH; x++)
    {
      ssd1306_buf1[x] = data;
    };
    ssd1306_SendData(ssd1306_buf1, SSD1306_WIDTH );
  };
  ssd1306_SetCursor(0, 0);
  //free(ssd1306_buf1);  
}


// Vsego 3 stroki heigh 3 baita
void ssd1306_Puts(char *mas_ssd,char stroka)
{
	  //int clear_mas=0;
	  //char len_128=0;
	  uint32_t ykaz_ssd1306=0;  // ykazatel v kakoe mesto simvol zapici
	  uint32_t temp_mas=0;
	  uint32_t i,t;
	  uint32_t mas_stroka[20]; // max 128/11= 11 символов строка
	  uint32_t mas_ykaz[20];   // ykaz v mas_ind
	  uint32_t mas_razmer[20]; // razmer simvola odnogo
	  char sym;
	
	  sym=strlenn(mas_ssd);
	  if(stroka>3) stroka=0;
	  // parsim mas_ssd
	 if(stroka==3)
	 {
		  for(i=0; i < sym;i++)
		 {
				mas_stroka[i]=ssd1306_ascii_index_table_h16[mas_ssd[i]];
				mas_ykaz[i]=ssd1306_offset_table_h16[mas_stroka[i]];
				mas_razmer[i]=ssd1306__width_table_h16[mas_stroka[i]];
				//razmer vsei stroki
		 }
	 }
	 else
	 {
			for(i=0; i < sym;i++)
		 {
				mas_stroka[i]=ssd1306_ascii_index_table[mas_ssd[i]];
				mas_ykaz[i]=ssd1306_offset_table[mas_stroka[i]];
				mas_razmer[i]=ssd1306__width_table[mas_stroka[i]];
				//razmer vsei stroki
		 }
	}
	switch(stroka){
		case 1:
			for(i=0; i < sym;i++)
			{
					for(t=0; t<mas_razmer[i];t++)
					{
						temp_mas=mas_ind[mas_ykaz[i]+t];
						s_ssd1306[0][ykaz_ssd1306+t]=temp_mas;
					}
					for(t=0; t<mas_razmer[i];t++)
					{
						temp_mas=mas_ind[mas_ykaz[i]+mas_razmer[i]+t];
						s_ssd1306[1][ykaz_ssd1306+t]=temp_mas;
					}
					for(t=0; t<mas_razmer[i];t++)
					{
						temp_mas=mas_ind[mas_ykaz[i]+2*mas_razmer[i]+t];
						s_ssd1306[2][ykaz_ssd1306+t]=temp_mas;
					}
					ykaz_ssd1306+=mas_razmer[i];
			}
			break;
		case 2:
			for(i=0; i < sym;i++)
			{
				  
					for(t=0; t<mas_razmer[i];t++)
					{
						temp_mas=mas_ind[mas_ykaz[i]+t];
						s_ssd1306[3][ykaz_ssd1306+t]=temp_mas;
					}
					for(t=0; t<mas_razmer[i];t++)
					{
						temp_mas=mas_ind[mas_ykaz[i]+mas_razmer[i]+t];
						s_ssd1306[4][ykaz_ssd1306+t]=temp_mas;
					}
					for(t=0; t<mas_razmer[i];t++)
					{
						temp_mas=mas_ind[mas_ykaz[i]+2*mas_razmer[i]+t];
						s_ssd1306[5][ykaz_ssd1306+t]=temp_mas;
					}
					ykaz_ssd1306+=mas_razmer[i];
			}
			break;
			case 3:
			for(i=0; i < sym;i++)
			{
					for(t=0; t<mas_razmer[i];t++)
					{
						temp_mas=mas_ind_16h[mas_ykaz[i]+t];
						s_ssd1306[6][ykaz_ssd1306+t]=temp_mas;
					}
					for(t=0; t<mas_razmer[i];t++)
					{
						temp_mas=mas_ind_16h[mas_ykaz[i]+mas_razmer[i]+t];
						s_ssd1306[7][ykaz_ssd1306+t]=temp_mas;
					}
//					for(t=0; t<mas_razmer[i];t++)
//					{
//						temp_mas=mas_ind[mas_ykaz[i]+2*mas_razmer[i]+t];
//						s_ssd1306[7][ykaz_ssd1306+t]=temp_mas;
//					}
					ykaz_ssd1306+=mas_razmer[i];
			}
			break;
		default:
			break;
	}
}

void ssd1306_update(char stroka_update)
{
	unsigned char page;
	switch(stroka_update){
		case 1:
			for (page=0; page<4; page++)
			{	
				ssd1306_SetCursor(0, page);     
				ssd1306_SendData(s_ssd1306[page], SSD1306_WIDTH );
			};
			break;
			
		case 2:
			for (page=3; page<6; page++)
			{	
				ssd1306_SetCursor(0, page);     
				ssd1306_SendData(s_ssd1306[page], SSD1306_WIDTH );
			};
			break;
			
		case 3:
			for (page=6; page<8; page++)
			{	
				ssd1306_SetCursor(0, page);     
				ssd1306_SendData(s_ssd1306[page], SSD1306_WIDTH );
			};
			break;
			
		default:
			break;
	}
  //ssd1306_SetCursor(0, 0);
}

char strlenn(char const *s)
{
	char i;

	i= 0;
	while(s[i]) {
		i+= 1;
	}

	return i;
}

