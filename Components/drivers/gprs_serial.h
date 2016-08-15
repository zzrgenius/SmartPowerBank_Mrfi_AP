#ifndef G_SERIAL_H
#define G_SERIAL_H

#include "stm32f10x.h"
#define 	GPRSREC_USART     USART3		 
#define 	GPRSREC_HANDLER   USART3_IRQHandler
#define  	GPRSREC_VEC		  USART3_IRQn
#define 	GPRS_DMA_CHANNEL  DMA1_Channel3

typedef enum
{ 
	serCOM0,
	serCOM1, 
	serCOM2, 
	serCOM3, 
	serCOM4, 
	serCOM5,
	serMaxPort,
} eCOMPort;

 
void Gprs_SerInit(void);
void ConfigGPSDMARec(void);
void  Uart_PutChar(uint32_t dwPort,u8 data);

int Serial_PutData(uint32_t dwPort, unsigned char *pcData, uint16_t wLen);
void Serial_PutString(uint32_t dwPort,unsigned char *pcStr);
 
#endif
