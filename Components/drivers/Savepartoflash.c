#include "Savepartoflash.h"
#include "stm32f10x_flash.h" 
#include <string.h>
 
 
//使用内部Flash ISP 需开启内部高速晶振
//////////////////////////////////////////////////////////////////////////////////////////////
// extern WORKPARA_STRUCT WorkPara
#define WORKPARAAVR     SetParameters	       //修改成对应的结构体变量
#define WORKPARASTRUCT  ParameterStruct	   //修改成对应的结构体
 ParameterStruct SetParameters ;
 

int GetSetedPar(ParameterStruct *ptPar)
{ 
    if(ptPar == NULL)
        return -1;
   
        memcpy(ptPar, &SetParameters, sizeof(ParameterStruct));
 
    return 0;     
}
void ParameterDefault(void)
{
	uint16_t serID[6];
	u16 dev_id;
	memset(SetParameters.GPRSSeverAddr, 0, sizeof(SetParameters.GPRSSeverAddr));
	//SetParameters.GPRSSeverAddr[0] = 0;
	//SetParameters.SensorState[0] = 0;
	memset(SetParameters.SensorState, 1, sizeof(SetParameters.SensorState));
	SetParameters.ucSampSwitch = 1;
	SetParameters.ucBLEmode = 0;
	SetParameters.ucGPRS_Switch = 1;
	SetParameters.ucDataUpmode = 0x03;
	SetParameters.ucSampletime = 30;
	SetParameters.backup =0;
	SetParameters.backup1 =0;
	SetParameters.backup2 =0;
	//GetSTM32ID((u32*)serID);
	dev_id = (u16)(serID[0]^serID[2]^(serID[3]>>5)^(serID[4]<<9)^(serID[5] >> 7));
	SetParameters.DeviceID[0] = dev_id>>8;//
	SetParameters.DeviceID[1] = (u8)dev_id;//
	SetParameters.VerPCB = 0x01;
	SetParameters.VerSystem = 0x01;
		
}

////加载参数错误时，恢复默认值
void CFG_LoadDefaultVal(void)			   //修改成配置默认值
{
// 	WORKPARAAVR.Para1 = 0x1234;
// 	WORKPARAAVR.Para2 = 0x5634;
// 	WORKPARAAVR.Para3 = 0x7834; 
	ParameterDefault();
	
}
///////////////////////////////////////////////////////////////////////////////////////////////

void CFG_SaveWorkPara( ParameterStruct *ptPar)
{
   uint16_t i,CheckSum;
   uint32_t WriteAddr;

   CheckSum = 0xAA55;
	 memcpy(&SetParameters,ptPar,  sizeof(ParameterStruct));
   FLASH_Unlock();				   //解锁;
   FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
   FLASH_ErasePage(CONFIGDATAADDR);//擦除这个扇区

   WriteAddr = CONFIGDATAADDR;
   for(i=0;i<sizeof(WORKPARASTRUCT)/2;i++)
   {
		FLASH_ProgramHalfWord(WriteAddr,((uint16_t*)&WORKPARAAVR)[i]);
		CheckSum ^= ((uint16_t*)&WORKPARAAVR)[i];

		WriteAddr += 2;//地址增加2.
   } 

   FLASH_ProgramHalfWord(WriteAddr,CheckSum);

   FLASH_Lock(); //上锁
}

void CFG_LoadWorkPara(void)
{
   uint16_t i,CheckSum;
   uint32_t WriteAddr;

   WriteAddr = CONFIGDATAADDR;
   CheckSum = 0xAA55;
	 

   for(i=0;i<sizeof(WORKPARASTRUCT)/2;i++)
   {
        ((uint16_t*)&WORKPARAAVR)[i] = *(uint16_t *)WriteAddr;
		CheckSum ^= *(uint16_t *)WriteAddr; 

		WriteAddr += 2;//地址增加2.
   }   

   CheckSum ^= *(uint16_t *)WriteAddr; 

   /////////////////////////////////////////////////////////////////////////////////////
   if(CheckSum != 0)//参数校验不正确，恢复默认设置
   {
	    CFG_LoadDefaultVal();
   }
   /////////////////////////////////////////////////////////////////////////////////////
}

