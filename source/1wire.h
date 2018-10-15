#ifndef _1WIRE_H
#define _1WIRE_H 

#include "stm32f10x.h"

#define OW_SEND_RESET    1
#define OW_NO_RESET    2

#define OW_OK    	1
#define OW_ERROR    2
#define OW_NO_DEVICE	3

#define OW_NO_READ    0xff

#define OW_READ_SLOT	0xff

#define APB1CLK  72000000UL
#define  BAUD       115200UL
#define  BAUD2       9600UL

void OW_Init(void);
uint8_t OW_Reset(void);
uint8_t OW_Send(uint8_t sendReset, char *command, uint8_t cLen, uint8_t *data, uint8_t dLen, uint8_t readStart);
unsigned char Crc8Dallas(unsigned char len, unsigned char *pData);
#endif //1WIRE_H
