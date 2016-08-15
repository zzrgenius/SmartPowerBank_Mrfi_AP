#ifndef __MSG_H
#define __MSG_H

#include "stm32f10x.h" 

#define RECV_PAYLOAD_SIZE 	64
#define RESP_PAYLOAD_SIZE 	200
#define MSG_MAXSIZE			256
#define HEAD_SIZE			8
#define MSG_HEADSIZE		8

#define MSG_ALL_HEADSIZE 	13

#define ANTIINTER_DATA  	0
#define SYNC_DATA	    	1
#define HEAD_DATA 			2
#define REL_DATA			3


typedef struct _MSG
{
	u32 wMsgType; 
	u8 Data[MSG_MAXSIZE];
}T_MSG;

typedef struct _MSG_RESP
{ 
	u8 AntiInterHead[2];
	u8 SyncHead[3];
	u8 wDeviceID[2];
	u8 ucSerial;
	
	u8 ucFuc;
	u8 ucCmd;
	u8 wLen[2];
	
	u8 ucHeadSum;
	u8 Data[RESP_PAYLOAD_SIZE];
	u8 wDataCrc[2];//
	u8 backup;
}T_MSG_RESP;

typedef struct _MSG_RECV
{
	u8 wDeviceID[2];
	u8 ucSerial;
	u8 ucFuc;
	u8 ucCmd;
	u8 wLen[2];
	u8 ucHeadSum;
	u8 Data[RECV_PAYLOAD_SIZE];
	u8 wDataCrc[2];//
}T_MSG_RECV;

typedef struct _DATA_STORE
{
	u8 	Timedata[8];
	
	u8 	Longitude[7];
	u8 	Latitude[7];	
	
	s16 wTemperature;
	u16 wHumidity;
	u16 wAt_pressure ;
	u16 wWind_Direction ;
	
	u16 wWind_Speed;
	u16 wRain;
	u16 wVisibility;
	u16 wPM25;
	
	u16 wPM10;
	u16 wCO;
	u16 wNO2;
	u16 wSO2;
	
	u16 wO3;
	u16 wHCHO;
	u16 wLightIntensity;
	u16 wUVA;	
//	u8  takeup[2];
}T_DATA_STORE;

//typedef	struct 
//{
//	uint16_t year;
//	uint8_t month;
//	uint8_t day;
//	uint8_t hour;
//	uint8_t min;
//	uint8_t second;
//}DateTime;
/*cmd*/
#define MCU_RESET 					0xd1
#define SENSOR_SWITCH 				0xd2
#define SENSOR_SAMPTIME 			0xd3
#define RTC_SETTING 				0xd4
#define GPRS_SWITCH 				0xd5
#define GPRS_PSETTING 				0xd6
#define DEVICEID 					0xd7
#define BT_MODE 					0xd8
#define DATAUP_MODE 				0xd9
#define FORMAT_SDCARD				0xdb
#define FACTORY_RESET 				0xdf
/* query */
#define DEVICEID_INQUIRE 					0xa1
#define GPRSPAR_INQUIRE 					0xa2
#define SENSORS_STATUS 						0xa3 
#define GET_TIME	 						0xa4
#define RTDATA_INQUIRE 						0xa5
/*******only com with slave device *********/
#define GETSLAVEDATA						0x25
/*****************/
//  蓝牙通信 命令  从机主动上传数据
#define BLE_TRANSDATE						0x45


#define SET_QUERY_TIME 						0xa6
#define SD_DATA_QUERY_START 				0xa7
#define SD_DATA_QUERY_STOP  				0xa8

#define SELFCHECK	 						0xab
#define GPRSPARFACTORY_INQUIRE				0xac
#define GET_DEVICE_VER						0xad

#define SYS_RAMLOG_OUT						0xb0
#define SYS_SDLOG_OUT		    			0xb1

#define MSG_TYPE_BLANK				0
#define MSG_TYPE_RECV_485 			1
#define MSG_TYPE_RESP_485			2
#define MSG_TYPE_RECV_GPRS  		3
#define MSG_TYPE_RESP_GPRS   		4
#define MSG_TYPE_RECV_MRFI			5
#define MSG_TYPE_RESP_MRFI   		6
//#define SensorsOpen GPIO_ResetBits(GPIOB,GPIO_Pin_15)
//#define SensorsClose GPIO_SetBits(GPIOB,GPIO_Pin_15)
int MsgPackage(T_MSG_RESP *ptRespMsg,const u8 type ) ;
#endif
