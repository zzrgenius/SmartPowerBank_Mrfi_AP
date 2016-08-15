#ifndef __DELAY_H
#define __DELAY_H 			   
#include "stm32f10x.h"
void DelayUs(__IO u32 myUs) ;
void DelayMs(__IO u32 myMs);
void delay_s(u16 s);

void Sys_delay_init(void); 		    								   
void Sys_delay_us(u32 nus);
void Sys_delay_ms(u16 nms);
 

#endif



