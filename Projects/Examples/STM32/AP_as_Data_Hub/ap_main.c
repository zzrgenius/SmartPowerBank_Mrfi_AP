#include "stm32f10x.h"
#include "bsp.h"
#include "sim900a_gprs.h"
#include "msg.h"
extern int mrfi_loop(void);
extern void vGprsConnectTask( void  );
extern int mrif_config (void);
void ProMsg(void);
T_MSG gMsgPro;
int main(void)
{

	
	BSP_Init();
	Gprs_Init();
	mrif_config();
	//while(1); 
	while(1)
	{
	 	mrfi_loop();
		vGprsConnectTask();
		ProMsg();
	}
	
}

void ProMsg(void)
{
	T_MSG_RESP *ptRespMsg;
	u32 dwGprsStatus;
	u16 wlen = 0;
	ptRespMsg = (T_MSG_RESP *)&gMsgPro.Data[0];
	wlen = ptRespMsg->wLen[0] +(ptRespMsg->wLen[1]<<8);
	switch(gMsgPro.wMsgType)
	{ 
		case MSG_TYPE_BLANK:
			break;
		case MSG_TYPE_RESP_GPRS:
			Gprs_GetConnectStatus(&dwGprsStatus);
			if(dwGprsStatus == GPRS_TCP_OK)
			{						 
				Gprs_Write((unsigned char*)&gMsgPro.Data,13+wlen );
				gMsgPro.wMsgType = MSG_TYPE_BLANK;
			}
			break;
		default:
			break;
	}
	
}