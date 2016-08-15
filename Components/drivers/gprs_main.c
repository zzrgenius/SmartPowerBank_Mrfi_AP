 
/* Scheduler includes. */
#include "bsp.h"
#include "sim900a_gprs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include "zRTC_Timer.h"
#include "stm32f10x_iwdg.h"
#include "msg.h"  
 
extern int MsgPackage(T_MSG_RESP *ptRespMsg,const u8 type ) ;
static uint32_t GprsTicks = 0;
void vGprsConnectTask( void  )
{
	static u8 FirstSendVerFlag = 0;	//  连接到服务器后发送一次 系统版本号
	static 	uint32_t dwGprsStatus = 0;
	
	

//	  struct GPRS_DEV *pDev = GetGprsDev();
	static T_MSG_RESP tRespMsg;
//	ParameterStruct tGetPar;
//	GetSetedPar(&tGetPar);

 
	//Gprs_Init();
	
	if((uint32_t)(SysRunTicks -  GprsTicks) > 2)
	{
		GprsTicks = SysRunTicks;
		
		/*send data to gprs server*/
		Gprs_GetConnectStatus(&dwGprsStatus);
		if((FirstSendVerFlag == 0 )&&(dwGprsStatus == GPRS_TCP_OK))
		{
			FirstSendVerFlag = 1;
		}
		else if(dwGprsStatus != GPRS_TCP_OK)
		{
			FirstSendVerFlag = 0 ;
		}
 
		switch(dwGprsStatus)
		{
			case  GPRS_INIT0:
				Gprs_Init();				 
				break;
			case GPRS_POWERON:
				  Gprs_Open() ;				 
				break;
			case  GPRS_SIM_CARD:
				Gprs_SimCardCheck();
				break;
			case GPRS_TCP_CONFIG:
				Gprs_TcpConfig();			
				break;
			case  GPRS_TCP_CONNECTING :
				Gprs_TcpConnect();
				break;
			case GPRS_TCP_OK:
				if(FirstSendVerFlag == 1)
				{
					FirstSendVerFlag = 2;
					tRespMsg.ucCmd = GET_DEVICE_VER;
					tRespMsg.wLen[0] = 4;
					tRespMsg.wLen[1] = 0;
					tRespMsg.Data[0] = 1;//tGetPar.VerPCB;
					tRespMsg.Data[1] = 0;//tGetPar.VerSystem;
					MsgPackage(&tRespMsg,MSG_TYPE_RESP_GPRS);
					// send data
				}
				break;
			case  GPRS_TCP_CLOSED:
				
				break;
			case GPRS_POWERDOWN:
				break;
		}
		//Gprs_GetSinal()
		//vTaskDelay(2000);
	}
}
 

