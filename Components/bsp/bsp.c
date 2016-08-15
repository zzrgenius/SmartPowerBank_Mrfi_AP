/**************************************************************************************************
  Revised:        $Date: 2007-07-06 11:19:00 -0700 (Fri, 06 Jul 2007) $
  Revision:       $Revision: 13579 $

  Copyright 2007 Texas Instruments Incorporated.  All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights granted under
  the terms of a software license agreement between the user who downloaded the software,
  his/her employer (which must be your employer) and Texas Instruments Incorporated (the
  "License"). You may not use this Software unless you agree to abide by the terms of the
  License. The License limits your use, and you acknowledge, that the Software may not be
  modified, copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio frequency
  transceiver, which is integrated into your product. Other than for the foregoing purpose,
  you may not use, reproduce, copy, prepare derivative works of, modify, distribute,
  perform, display or sell this Software and/or its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE PROVIDED “AS IS”
  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY
  WARRANTY OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
  IN NO EVENT SHALL TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE
  THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES INCLUDING BUT NOT LIMITED TO ANY
  INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST
  DATA, COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY
  THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 *   BSP (Board Support Package)
 *   Top-level BSP code file.
 * =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 */

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "bsp.h"
#include "bsp_driver_defs.h"
#include "stm32f10x_tim.h"
uint32_t SysTicksVal32Bit;
void TIM2_IRQHandler(void)
{
	((uint16_t*)&SysTicksVal32Bit)[1] += 1;
	((uint16_t*)&SysTicksVal32Bit)[0] = (uint32_t)TIM2 -> CNT;
	
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);	//Çå³ýÖÐ¶Ï±êÖ¾
}
//¸Ãº¯Êý²»ÄÜÔÚÖÐ¶Ïº¯ÊýÄÚµ÷ÓÃ
uint32_t PDInit_GetSysTick(void)
{
	 ((uint16_t*)&SysTicksVal32Bit)[0] = (uint32_t)TIM2 -> CNT;
	 return(SysTicksVal32Bit);	
}

void RCC_Configuration(void)
{
	//²Î¿¼ÍøÖ·£ºhttp://blog.csdn.net/iamlvshijie/article/details/9249545
	ErrorStatus HSEStartUpStatus; 
	RCC_DeInit();     //ÉèÖÃRCC¼Ä´æÆ÷ÖØÐÂÉèÖÃÎªÄ¬ÈÏÖµ
	RCC_HSEConfig (RCC_HSE_ON);//´ò¿ªÍâ²¿¸ßËÙÊ±ÖÓ¾§Õñ£¨HSE£©
	HSEStartUpStatus = RCC_WaitForHSEStartUp();//µÈ´ýÍâ²¿Ê±ÖÓ¾§ÕñÎÈ¶¨¹¤×÷
	if(HSEStartUpStatus == SUCCESS)//SUCCESS£ºHSE¾§ÕñÎÈ¶¨ÇÒ¾ÍÐ÷
	{
		//FLASH_PrefetchBufferCmd(ENABLE);          //¿ªÆôFLASHµÄÔ¤È¡¹¦ÄÜ
		//FLASH_SetLatency(FLASH_Latency_2);      //FLASHÑÓ³¤Á½¸öÖÜÆÚ
		RCC_HCLKConfig(RCC_SYSCLK_Div1);//ÉèÖÃAHBÊ±ÖÓ²»·ÖÆµ
		RCC_PCLK2Config(RCC_HCLK_Div1); //ÉèÖÃAPB2Ê±ÖÓ²»·ÖÆµ
		RCC_PCLK1Config(RCC_HCLK_Div2); //ÉèÖÃAPB1Ê±ÖÓ¶þ·ÖÆµ ¶ÔÓ¦µÄ¶¨Ê±Æ÷µÄÊ±ÖÓÎª2±¶Æµ
//	FLASH_SetLatency(FLASH_Latency_2);//ÉèÖÃFLASH´æ´¢Æ÷ÑÓÊ±ÖÜÆÚÊý£¬FLASH_Latency_2 2ÑÓÊ±ÖÜÆÚ
//	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);//Ñ¡ÔñFLASHÔ¤È¡Ö¸»º´æµÄÄ£Ê½£¬Ô¤È¡Ö¸»º´æÊ¹ÄÜ
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1,RCC_PLLMul_9);//ÉèÖÃPLL  11.0592Îå±¶Æµ=55.296¡£
		RCC_PLLCmd(ENABLE); //PLLÊ±ÖÓÊ¹ÄÜ
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		while(RCC_GetSYSCLKSource()!=0x08);//µÈ´ý¹¤×÷¡£
	}
	
	 RCC_LSICmd(ENABLE);
  while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);
 
	//¿ªÆôÊäÈë¶Ë¿ÚÊ±ÖÓ
	//Ê¹ÄÜPA~PE¶Ë¿ÚÊ±ÖÓIN1~IN32
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);	 
//	//¿ªÆô´®¿Ú1¡¢2¡¢3Ê±ÖÓ
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);	//Ê¹ÄÜUSART1Ê±ÖÓ
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//Ê¹ÄÜUSART2£¬USART3Ê±ÖÓ
//	
	/* Configure HCLK clock as SysTick clock source. */
	SysTick_CLKSourceConfig( SysTick_CLKSource_HCLK );
	//¿ªÆô¶¨Ê±Æ÷
	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);//
	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);//
}
// USE TIMER2
void BSP_INIT_SYS_TICKS(void)  
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	  NVIC_InitTypeDef NVIC_InitStructure;
	  /* Enable the TIM2 gloabal Interrupt */
	  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	  NVIC_Init(&NVIC_InitStructure);	
	
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_DeInit(TIM2);                                           //ÖØÐÂ½«TimerÉèÖÃÎªÈ±Ê¡Öµ
       
    TIM_InternalClockConfig(TIM2);                              //²ÉÓÃÄÚ²¿Ê±ÖÓ¸øTIM2Ìá¹©Ê±ÖÓÔ´      
    TIM_TimeBaseStructure.TIM_Prescaler = 36000-1;               //Ô¤·ÖÆµÏµÊýÎª36000-1£¬ÕâÑù¼ÆÊýÆ÷Ê±ÖÓÎª72MHz/36000 = 2kHz       
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //ÉèÖÃÊ±ÖÓ·Ö¸î      
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //ÉèÖÃ¼ÆÊýÆ÷Ä£Ê½ÎªÏòÉÏ¼ÆÊýÄ£Ê½       
    TIM_TimeBaseStructure.TIM_Period = 0xffff;                  //ÉèÖÃ¼ÆÊýÒç³ö´óÐ¡£¬ ¾Í²úÉúÒ»¸ö¸üÐÂÊÂ¼þ
    TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);              //½«ÅäÖÃÓ¦ÓÃµ½TIM2ÖÐ

	  TIM_ClearITPendingBit(TIM2, TIM_IT_Update);	//Çå³ýÖÐ¶Ï±êÖ¾
	  TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);	//¿ªÖÐ¶Ï

	  TIM_Cmd(TIM2, ENABLE);	
}
/**************************************************************************************************
 * @fn          BSP_Init
 *
 * @brief       Initialize the board and drivers.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void BSP_Init(void)
{
	RCC_Configuration();
	BSP_INIT_BOARD();
	BSP_INIT_DRIVERS();
	Sys_delay_init();
	BSP_INIT_SYS_TICKS()  ;
  /*-------------------------------------------------------------
   *  Run time integrity checks.  Perform only if asserts
   *  are enabled.
   */
#ifdef BSP_ASSERTS_ARE_ON
	/* verify endianess is correctly specified */
	{
		uint16_t test = 0x00AA; /* first storage byte of 'test' is non-zero for little endian */
		BSP_ASSERT(!(*((uint8_t *)&test)) == !BSP_LITTLE_ENDIAN); /* endianess mismatch */
	}
#endif
}


/* ================================================================================================
 *                                        C Code Includes
 * ================================================================================================
 */
#ifdef BSP_BOARD_C
#include BSP_BOARD_C
#endif

#ifdef BSP_DRIVERS_C
#include BSP_DRIVERS_C
#endif


/* ************************************************************************************************
 *                                   Compile Time Integrity Checks
 * ************************************************************************************************
 */
BSP_STATIC_ASSERT( sizeof(  uint8_t ) == 1 );
BSP_STATIC_ASSERT( sizeof(   int8_t ) == 1 );
BSP_STATIC_ASSERT( sizeof( uint16_t ) == 2 );
BSP_STATIC_ASSERT( sizeof(  int16_t ) == 2 );
BSP_STATIC_ASSERT( sizeof( uint32_t ) == 4 );
BSP_STATIC_ASSERT( sizeof(  int32_t ) == 4 );


/**************************************************************************************************
 */
