// #include "AllStruct.h" 
#ifndef __SIM900A_GPRS_H
#define __SIM900A_GPRS_H
#include "stm32f10x.h"
 



#define GPRS_SER_PORT  serCOM3
#define CENTERPHONEON   "18351871133\0"//管理手机号码
#define SQRSETSMSTEXT   "SMS Format of Server Addr:\"CMD<NEWADDR>\",\"len(IP+Port+5)<22>\",\"IP<192.168.1.1>\",\"Port<9000>\"\0"//请求短信内容

#define GPRSRECBUFDEEP  128 //接收数据缓冲区
#define GPRSTRABUFDEEP	256//发送帧的缓冲区

#define GPRSDATABUFTIME 30000   //发信周期
#define HEATBEATPERIOD  60000   //心跳周期
#define AUTOHEARTPACTET 1        //自动发送心跳包
#define HEARTPACKETSTRING  "\0\1"//心跳包内容，默认发4个字节长度

 
 
 
 #define GPRS_PORT  GPIOD
 #define GPRS_PWR_PIN		GPIO_Pin_14
 #define GPRS_RESET_PIN     GPIO_Pin_13
 #define GPRS_STATUS_PIN	GPIO_Pin_15
 #define GPRS_RI_PIN		GPIO_Pin_12
 //GPRS模块用的控制IO
#define GPRS_POWERKEY_H   GPIO_SetBits(GPRS_PORT, GPRS_PWR_PIN)		  //模块开关（SIM900A开关机设计及时序  PWRKEY 引脚内部有上拉电阻，程序中IO复位给低电平，电路集电极为高，即上电 ）
#define GPRS_POWERKEY_L	  GPIO_ResetBits(GPRS_PORT, GPRS_PWR_PIN)
#define GPRS_RESET_H	  GPIO_SetBits(GPRS_PORT, GPRS_RESET_PIN)		  //复位
#define GPRS_RESET_L	  GPIO_ResetBits(GPRS_PORT, GPRS_RESET_PIN)
#define GSM_RI_VAL	      GPIO_ReadInputDataBit(GPRS_PORT,GPRS_RI_PIN)  //判断接收数据
#define GPRS_STATUS_VAL   GPIO_ReadInputDataBit(GPRS_PORT,GPRS_STATUS_PIN)  //模块状态

#define GPRSMDBYTEGAP      100
//#define HEATBEATPERIOD     60

#define GPRSCONFIGTRYTIMES 3

#define GRPS_ModuleError   0x0001
#define GRPS_SIMError      0x0002
#define GRPS_GPRSNETWORK   0x0004
#define GRPS_CONNECTION    0x0008
/***********
**AT+CPIN?   AT+CSQ?   AT+CGATT?
** AT+CSTT  AT+CIICR  AT+CIFSR 
**AT+CIPSTART="TCP","IP",PORT
**
**
**
**
**
**
*****/
//#define HEATBEATPERIOD     60
//  typedef enum 
//  {
//     INIT0,
//     SIM900_POWERON,
// 	AT_TEST,
// 	GPRS_OPEN_FINISH,       /// GPRS 打开成功了
//     SIMCARD,	// 检测是否存在sim卡
//     INIT_GPRS,
// 	GPRS_CONFIG0,	// CGCLASS 设置移动台类别
// 	GPRS_CONFIG1, 	//CGDCONT
// 	GPRS_CONFIG2, 	//CGATT
// 	GPRS_CONFIG3, 	//CGACT
// 	 TCPIP_SENDAUTO,	//自动发送数据
// 	TCPIP_CGREG,	// 检测gprs网络状态 CGREG
//     TCPIP_HEAD_CONF,// 接受数据是否显示IP地址头
// 	TCPIP_TOCONNECTING,	// 建立tcp连接
//     TCP_IP_OK,//		tcp连接成功
//     TCP_IP_NO,//		tcp连接不成功
//     TCPIP_CONNECTING,
//     SENDDATA_START,		// 发送数据
//     SENDDATA_ING,		// 数据发送过程中
//     TCPIP_CLOSE_START,	// 关闭当前tcp连接
//     TCP_IP_CLOSE_OK,
//     TCP_IP_CLOSE_NO,
// 	SIM900_POWRDOWN		//关机
// }SIM900A_STATUS ;

struct GPRS_DEV
{
//	SemaphoreHandle_t pSem;
	u8 ucUartPort;
	u8 ucRecDataFlag;
	u8 buf[GPRSRECBUFDEEP];
	
	u16 wLen;
	u16 wReadPos;
	u32 dwStatus;
	u32 dwCmdAck;

	int (*Init)(struct GPRS_DEV *dev); 
	int (*Open)(struct GPRS_DEV *dev);
	int (*Close)(struct GPRS_DEV *dev,uint8_t bCloseMode);
	int (*SendCmd)(struct GPRS_DEV *dev, u8 *pCmd);
	int (*Read)(struct GPRS_DEV *dev, u8 *pBuf, u16 *pwLen);
	int (*Write)(struct GPRS_DEV *dev, u8 *pBuf, u16 wLen);
	int (*GetConnetStatus)(struct GPRS_DEV *dev);
	int (*GetSignal)(struct GPRS_DEV *dev,u8* dwVal);
	int (*HandleRecv)(struct GPRS_DEV *dev, u8 *pBuf, u16 wLen);
};

#define GPRS_INIT0						0x0000
#define GPRS_POWERON  					0x0001
#define GPRS_SIM_CARD  					0x0002
#define GPRS_TCP_CONFIG	  				0x0004
#define GPRS_TCP_CONNECTING  			0x0010
#define GPRS_TCP_OK						0x0020
#define GPRS_TCP_CLOSED  				0x0040
#define GPRS_POWERDOWN 					0x0080
struct GPRS_DEV *GetGprsDev(void);
int Gprs_Init(void);
int Gprs_Open(void);
int Gprs_SimCardCheck(void);
int Gprs_TcpConfig(void);
int Gprs_TcpConnect(void);
int Gprs_TcpClose(void);
int Gprs_Close(void);
int Gprs_SendCmd(u8 *pCmd, u16 wCmdLen);
int Gprs_Write(u8* pBuf, u16 wLen);
int Gprs_Read(u8* pBuf, u16 *pwLen);
int Gprs_GetConnectStatus(u32 *pdwStatus);

int Gprs_GetRecStatus(u8 *pucStatus);

void StartGprsRecvTasks(void);
  void  vGprsReceiveTask( void  );

#endif
