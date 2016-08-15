#include "stm32f10x.h"
#include "msg.h"
#include <string.h>
#include "encryption.h"
#include "sim900a_gprs.h"

extern T_MSG gMsgPro;
int MsgRSEPPackage(T_MSG_RECV *ptdev,u8 type)//设置指令回复
{ 
	u16 wlen; 	 
	//u8 checksum;
	T_MSG tMsg;
	T_MSG_RESP *tRespMsg = (T_MSG_RESP *)&tMsg.Data[0];
//	u8 *pCheck = tRespMsg->wDeviceID;
	 
	//wLength = ptdev->wLen[0] + (ptdev->wLen[1]<<8); 	
	memset(tRespMsg->AntiInterHead, 0xaa, 2); 
	memset(tRespMsg->SyncHead, 0xc0, 3); 
//	tRespMsg.ucSerial  = ptdev->ucSerial;
//	tRespMsg.wDeviceID[0] = ptdev->wDeviceID[0];	 
//	tRespMsg.wDeviceID[1] = ptdev->wDeviceID[1];	 	
//	tRespMsg.ucCmd    =   ptdev->ucCmd;
//	tRespMsg.wLen[0]  =  ptdev->wLen[0];//
//	tRespMsg.wLen[1]  =  ptdev->wLen[1];
//	tRespMsg.ucHeadSum = ptdev->ucHeadSum;
	memcpy(tRespMsg->wDeviceID,ptdev->wDeviceID,8);	
	wlen = tRespMsg->wLen[0] + (tRespMsg->wLen[1]<<8);
	if(wlen > 2)
	{
		memcpy(tRespMsg->Data,ptdev->Data,wlen-2);
		tRespMsg->Data[wlen-2] = ptdev->wDataCrc[0];
		tRespMsg->Data[wlen-1] = ptdev->wDataCrc[1];
		tRespMsg->wDataCrc[0] = ptdev->wDataCrc[0];
		tRespMsg->wDataCrc[1] = ptdev->wDataCrc[1];
	}
	else
	{
		__nop();
	}
	tMsg.wMsgType = type;
	memcpy((u8*)&gMsgPro,(u8*)&tMsg,sizeof(T_MSG));
	return 0;
} 

/*****************
***   for  test
***
*****************/

//int MsgPackage(T_MSG_RECV *ptdev)
int MsgPackage(T_MSG_RESP *ptRespMsg,const u8 type ) 
{ 
	//struct GPRS_DEV *pDev = GetGprsDev();
	
	u8	ucCheckSum;
	static u8 ucSerNum = 0;
	
	u8* pCheck = (u8*)&ptRespMsg->wDeviceID;
 
	u8 CRCDATATemp[2];
	
	u16 i,wLen; 	
	T_MSG tMsg;
//	ParameterStruct tGetPar;
//	GetSetedPar(&tGetPar);
	//u32 dwGprsStatus = 0;
	ucSerNum++;
//	Gprs_GetConnectStatus(&dwGprsSt atus);
	memset(ptRespMsg->AntiInterHead, 0xaa, 2); 
	memset(ptRespMsg->SyncHead, 0xc0, 3); 
	ptRespMsg->wDeviceID[0] = 1;//tGetPar.DeviceID[0];			
	ptRespMsg->wDeviceID[1] = 0;//tGetPar.DeviceID[1]; 	
	ptRespMsg->ucSerial = ucSerNum;
	ptRespMsg->ucFuc = 0;
	ucCheckSum = 0;
	for(i=0;i<7;i++)
		ucCheckSum += *pCheck++;
	ptRespMsg->ucHeadSum = ucCheckSum;
	wLen = ptRespMsg->wLen[0] + (ptRespMsg->wLen[1]<<8);
 
	if(wLen>2)
	{
		if((MSG_ALL_HEADSIZE+wLen)> MSG_MAXSIZE)
			return -1;
		CRC16(ptRespMsg->Data,wLen-2,CRCDATATemp); 
		ptRespMsg->wDataCrc[0]= CRCDATATemp[0];
		ptRespMsg->wDataCrc[1]= CRCDATATemp[1];
		ptRespMsg->Data[wLen-2] = CRCDATATemp[0];
		ptRespMsg->Data[wLen-1] = CRCDATATemp[1];
		memset((u8*)&tMsg,0,sizeof(tMsg));
		memcpy(tMsg.Data,ptRespMsg->AntiInterHead,MSG_ALL_HEADSIZE+wLen);
		
	}
	else
	{
		memcpy(tMsg.Data,ptRespMsg->AntiInterHead,MSG_ALL_HEADSIZE);
		
	}
	tMsg.wMsgType = type;	
	memcpy((u8*)&gMsgPro,(u8*)&tMsg,sizeof(T_MSG));
	return 0;
} 

//Uart3_RecvHandler 接收有效数据填充至 T_MSG tMsg
//DealMsg 将T_MSG中DATA 给	T_MSG_RECV *ptRecvMsg; 
int DealMsg(T_MSG *msg)
{
	//u8 i;
 
 
 
	struct GPRS_DEV *pDev = GetGprsDev();
 
	T_MSG_RECV *ptRecvMsg; 
	T_MSG_RESP tRespMsg; 
 
//	ParameterStruct tGetPar;
//	DateTime tTime;
//	GetSetedPar(&tGetPar);
	ptRecvMsg = (T_MSG_RECV *)&msg->Data[0];
	
	//DateTime *start;
//	DateTime *end;
	//ptRespMsg = (T_MSG_RESP *)&msg->Data[0];
 
 //CRC 校验
	
	switch(ptRecvMsg->ucCmd)
	{
// 		case 0:  /*for test*/
// 			memset(tRespMsg.SyncHead, 0xc0, 3);
// // 			memcpy(&tRespMsg.ucAddr, &ptRecvMsg->ucAddr, HEAD_SIZE);
// // 			tRespMsg.ucAddr = 0x10;
// // 			tRespMsg.Data[0] = 0x55;
// 			Serial_PutData(serCOM3, (char*)&tRespMsg, 16);
// 			break;	
		case MCU_RESET:
			//重启MCU
			MsgRSEPPackage(ptRecvMsg,MSG_TYPE_RESP_485);
			__nop();__nop();
//			WriteStrLogToSdcard("soft reset\r\n");
			//RestartMCU();		 
			break;
		case SENSOR_SWITCH:
			//传感器开关，默认全开 第一个字节为传感器编号第二个字节0x01使能
			//(ptRecvMsg->Data[0])
			//保存参数
//			if(ptRecvMsg->Data[0]<16)
//			{
//				if(ptRecvMsg->Data[1] == 0x01)
//				{
//					tGetPar.SensorState[ptRecvMsg->Data[0]] = 0xff;				
//				}
//				else
//					tGetPar.SensorState[ptRecvMsg->Data[0]] = 0x0;
//			}
//			WriteStrLogToSdcard("sensors switch set\r\n");
//			CFG_SaveWorkPara(&tGetPar);
			break;
		case SENSOR_SAMPTIME:
//			//默认10s 5s～255s
//			if(ptRecvMsg->Data[0] >4)
//				tGetPar.ucSampletime = ptRecvMsg->Data[0]; 
//			else
//				tGetPar.ucSampletime = 10;
//			sprintf((char*)logbuf,"Sampletime setted as %d\r\n",ptRecvMsg->Data[0]);
//			WriteStrLogToSdcard((char*)logbuf);			
//			CFG_SaveWorkPara(&tGetPar);
			break;
		case RTC_SETTING:
//			RTCtime.year = (u16)((ptRecvMsg->Data[1]<<8) | ptRecvMsg->Data[0]);
//			RTCtime.month = ptRecvMsg->Data[2];
//			RTCtime.day = ptRecvMsg->Data[3];
//			RTCtime.hour = ptRecvMsg->Data[4];
//			RTCtime.min = ptRecvMsg->Data[5];
//			RTCtime.sec = ptRecvMsg->Data[6];
//			if(PFC_SET_TIME(&RTCtime) != 0)  // DateTime_Modify
//			{
//				ptRecvMsg->ucFuc = 0xFF;
//			}
//			sprintf((char*)logbuf,"rtc time setted as %d-%d-%d %d:%d:%d\r\n",RTCtime.year,RTCtime.month,RTCtime.day,RTCtime.hour,RTCtime.min,RTCtime.sec );
//			WriteStrLogToSdcard((char*)logbuf);
			break;
		case GPRS_SWITCH:
//			if(tGetPar.ucGPRS_Switch == 1)
//			{
//				//如果此时已经是开机状态
//				if(ptRecvMsg->Data[0] == 0x00)// 关闭gprs
//				{
//					tGetPar.ucGPRS_Switch = 0;
//					//Sim900aClose(pDev,1);	//硬件关机
//					pDev->Close(pDev,1);
//					pDev->dwStatus = GPRS_POWERON;
//					CFG_SaveWorkPara(&tGetPar);	
//				}
//				else
//				{
//					tGetPar.ucGPRS_Switch = 1;
//					__nop();
//				}
////				WriteStrLogToSdcard("open gprs\r\n");
//			}
//			else
//			{
//				//如果此时已经是关机状态
//				if(ptRecvMsg->Data[0] == 0x01)//open
//				{
//					tGetPar.ucGPRS_Switch = 1;
//					pDev->Open(pDev);
//					CFG_SaveWorkPara(&tGetPar);			
//				}
//				else
//				{
//					tGetPar.ucGPRS_Switch = 0;
//					__nop();
//				}
////				WriteStrLogToSdcard("close gprs\r\n");				
//			}
			break;
		case GPRS_PSETTING:
//			for(i=0;i < 32;i++)
//			{
//				if(ptRecvMsg->Data[i] != '\0')
//					tGetPar.GPRSSeverAddr[i] = ptRecvMsg->Data[i];
//				else
//					break;
//			}
//			tGetPar.GPRSSeverAddr[i] = '\0';
////			strcpy((char*)ptRecvMsg->Data,(char*)&SetParameters.GPRSSeverAddr);
//			//断开gprs链接 重新连接
//			//Gprs_TcpClose();
//			pDev->dwStatus = GPRS_TCP_CONNECTING;
////			WriteStrLogToSdcard("set gprs: \r\n");
////			WriteStrLogToSdcard(tGetPar.GPRSSeverAddr);
//			CFG_SaveWorkPara(&tGetPar);
			break;
		case DEVICEID:
//			tGetPar.DeviceID[0] =ptRecvMsg->Data[0] ;
//			tGetPar.DeviceID[1] =ptRecvMsg->Data[1] ;
////			sprintf((char*)logbuf,"device id is %d \r\n",(int)ptRecvMsg->Data);
////			WriteStrLogToSdcard((char*)logbuf);
//			CFG_SaveWorkPara(&tGetPar);
			break;
 
		case DATAUP_MODE:
//			tGetPar.ucDataUpmode = ptRecvMsg->Data[0];
////			sprintf((char*)logbuf,"data upload mode is %d \r\n",ptRecvMsg->Data[0]);
////			WriteStrLogToSdcard((char*)logbuf);
//			CFG_SaveWorkPara(&tGetPar);
			break;
		
		case FORMAT_SDCARD:
			break ;
			
		case FACTORY_RESET:
//			ParameterDefault();
//			//重新获取par
//			GetSetedPar(&tGetPar);
//		 
//			CFG_SaveWorkPara(&tGetPar);
			break;
		/*查询*/		
	 	case DEVICEID_INQUIRE://设备id			
//			tRespMsg.wLen[0] = 0x04;//
//			tRespMsg.wLen[1] = 0x00;
//			tRespMsg.Data[0] = tGetPar.DeviceID[0];
//			tRespMsg.Data[1] = tGetPar.DeviceID[1];		
			break;
		case  GPRSPAR_INQUIRE:
		//	tRespMsg.ucCmd
//			for(i=0;i<31;i++)
//			{
//				if(tGetPar.GPRSSeverAddr[i]!= '\0')
//				{
//					tRespMsg.Data[i] = tGetPar.GPRSSeverAddr[i];
//					//break;
//				}
//				else
//				{
//					break;
//				}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               
//			}
//			tRespMsg.Data[i] = '\0';
//			tRespMsg.wLen[0] = i+2;	//2个字节 crc
//			tRespMsg.wLen[1] = 0x00;
//			
			break;
		case  SENSORS_STATUS :	
			// 传感器 状态获取
			tRespMsg.Data[0] = ptRecvMsg->Data[0];
			//tRespMsg.Data[1] = Check_SensorsSta(ptRecvMsg->Data[0]);
			tRespMsg.Data[1] = 0;
			tRespMsg.wLen[0] = 4;	//2个字节 crc
			tRespMsg.wLen[1] = 0x00;
		
			break; 
		case RTDATA_INQUIRE:
//			GetEnvData((T_DATA_STORE *)tRespMsg.Data);
//			tRespMsg.wLen[0] = sizeof(T_DATA_STORE)+2;	//2个字节 crc
//			tRespMsg.wLen[1] = 0x00;           
			break;
		case  GETSLAVEDATA:
 
			 
		case GET_TIME :
//			if(PFC8563_GET_TIME(&tTime) == 0)//DateTime_Now(&tTime);
//			{
//				memcpy(&tRespMsg.Data[0],(u8*)&tTime,7);
//				tRespMsg.wLen[0] = 7+2;	//2个字节 crc
//				tRespMsg.wLen[1] = 0x00;			
//			}
//			else
//			{
//				tRespMsg.wLen[0] = 0;	//2个字节 crc
//				tRespMsg.wLen[1] = 0x00;
//			}
			break;
		case SELFCHECK:
			
			break;
		case SET_QUERY_TIME:
			 
			break;
		 
		case GET_DEVICE_VER:
//			tRespMsg.ucCmd = GET_DEVICE_VER;
//			tRespMsg.wLen[0] = 4;
//			tRespMsg.wLen[1] = 0;
//			tRespMsg.Data[0] = tGetPar.VerPCB;
//			tRespMsg.Data[1] = tGetPar.VerSystem;
			break;
 
	 
		default: 
			tRespMsg.ucCmd = 0xff;
			break;
	}
	if(msg->wMsgType == MSG_TYPE_RECV_485)
	{		 
		if((ptRecvMsg->ucCmd & 0xf0) == 0xd0)
			MsgRSEPPackage(ptRecvMsg,MSG_TYPE_RESP_485);
		else if((ptRecvMsg->ucCmd & 0xf0) == 0xa0)
		{
			tRespMsg.ucCmd = ptRecvMsg->ucCmd;
			MsgPackage(&tRespMsg,MSG_TYPE_RESP_485);		
		}
	}
	else if(msg->wMsgType == MSG_TYPE_RECV_GPRS)
	{		 
		if((ptRecvMsg->ucCmd & 0xf0) == 0xd0)
			MsgRSEPPackage(ptRecvMsg,MSG_TYPE_RESP_GPRS);
		else if((ptRecvMsg->ucCmd & 0xf0) == 0xa0)
		{
			tRespMsg.ucCmd = ptRecvMsg->ucCmd;
			MsgPackage(&tRespMsg,MSG_TYPE_RESP_GPRS);		
		}
	}
 
	
	return 0; 
}
