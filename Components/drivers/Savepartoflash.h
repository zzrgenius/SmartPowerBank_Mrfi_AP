#ifndef __SAVEPARTOFLASH_H
#define __SAVEPARTOFLASH_H
#include "stm32f10x.h"

/*
配置结构体大小需满足4字节整数倍，不足请补齐
如：
*/
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct 
{
   uint16_t  Para1;   
   uint16_t  Para2; 
   uint16_t Para3; 
   uint16_t  backup;//占位,总大小补成双字         
}WORKPARA_STRUCT;

//xl 768kflash 0xc0000 
#define CONFIGDATAADDR   0x080BF000 //hd产品flash  最后一页

//使用内部Flash ISP 需开启内部高速晶振


 typedef struct  //注意结构体字节对齐 
{
	char	   	GPRSSeverAddr[32];
	uint8_t		SensorState[16];

	uint8_t 	ucSampSwitch;	
	uint8_t 	ucBLEmode;
	uint8_t    	ucGPRS_Switch;
	uint8_t   	ucDataUpmode;
	
	uint8_t 	ucSampletime; 
	uint8_t     backup;
	uint8_t   	DeviceID[2];//
	
	uint8_t 	VerPCB;
	uint8_t		VerSystem;
	uint8_t		backup1;
	uint8_t		backup2;
 
	//	backup;//占位,总大小补成双字         
}ParameterStruct ;

void CFG_SaveWorkPara( ParameterStruct *ptPar);
void CFG_LoadWorkPara(void); 
void ParameterDefault(void);
int GetSetedPar(ParameterStruct *ptPar);
#endif
