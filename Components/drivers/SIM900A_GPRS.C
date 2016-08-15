#include "SIM900A_GPRS.h" 
#include <string.h>  
#include "CommonUse.h"
#include "Savepartoflash.h"
#include "stm32f10x_tim.h"
#include "bsp.h" 
#include "gprs_serial.h"
#include "msg.h"




#define GPRS_CMD_WAIT_TIMEOUT  10 
//QueueHandle_t g_GprsRecvQueue;
extern int DealMsg(T_MSG *msg);

//////////////////////////////////////////////////
#define SCANCOMMAND 		0x0000 	//
#define IPDATAHEAD  		0x0001 	//
#define DTSUCESSED   		0x0002   //
#define TCPIPCLOSED  		0x0004 	//
#define CALLIN       		0x0008 	//
#define MESSAGEIN    		0x0010 	//
#define COMMANDERROR 		0x0020 	//
#define CONNECTFAIL  		0x0040 	//
#define CANNCELOK    		0x0080 	//
#define TCPCONNECTOK		0x0100 	//
#define CONNECTEXIST 		0x0200 	//
#define COMMANDOK    		0x0400 	//
#define SINGALQUACK  		0x0800   //
#define GPRSSTATUS   		0x1000   //
#define SEVERHEART   		0x2000   //
#define REALTIMEACK  		0x4000   //取实时时间
#define RECUSEERCMD  		0x8000 	//
#define READMESSAGE  		0x10000   //读短信
#define SETNEWADDR   		0x20000   //设置中心地址和端口
#define SENDFAIL     		0x40000   //数据发送失败
#define ALREADYCONNECTED 	0x80000

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif
#define AT_CMD_TIMEOUT 			10
#define GPRSREC_IPD_HEAD   		0x00
#define GPRSREC_IPD_LEN   		0x01
#define GPRSREC_IPD_REC			0x02
// const GPRSACKCODE_STRUCT SIM900ACKCODE[] =
// {
// 	
// {CONNECTEXIST  ,12,  "EADY CONNECT"},   //已经在连接	 8
// {CONNECTFAIL   ,12,  "CONNECT FAIL"},   //连接失败		 4
// {MESSAGEIN     ,12,  "+CMTI: \"SM\","}, //,1//短信	2
// {READMESSAGE   ,6,   "+CMGR: "},        //阅读短信
// {SETNEWADDR    ,11,  "\"NEWADDR\",\""}, //设置新的IP地址和端口号
// {TCPCONNECTOK  ,10,  "CONNECT OK"},     //TCP连接成功	 7
// {DTSUCESSED    ,7 ,  "SEND OK"},	      //发送成功
// {SENDFAIL      ,9 ,  "SEND FAIL"},      //发送失败
// {CANNCELOK     ,7 ,  "SHUT OK"},       //取消场景AT+CIPSHUT : 6
// //{IPDATAHEAD    ,10,  "\r\r\r\r"},		    //用户数据头  /////////////////////////////////////////需要根据应用修改
// {TCPIPCLOSED   ,6 ,  "CLOSED"},         //TCP连接断开	0
// {COMMANDERROR  ,5 ,  "ERROR"},          //指令执行错误	3
// {CALLIN        ,4 ,  "RING"},           //电话				1
// {SINGALQUACK   ,6 ,  "+CSQ: "},          //信号强度1
// {REALTIMEACK   ,6 ,  "+CCLK:"},         //实时时间 +CCLK: "14/06/25,05:37:34+32"
// {REALTIMEACK   ,7 ,  "+QNITZ:"},        //+QNITZ: "14/06/25,07:00:22+32,0"
// {GPRSSTATUS    ,10,  "+CGREG: *,"},		  //GPRS连接状态
// {IPDATAHEAD    ,3 ,  "IPD"},            //接收到IP数据
// {COMMANDOK     ,2 ,  "OK"},				// 9
// //{SEVERHEART    ,2 ,  "$H"},			    //服务器心跳包////////////////////////////////////////需要根据应用修改
// {0x0000        ,0,   "  "}
// };
#if 0
uint8_t GPRS_SearchKeyChar(uint8_t KeyChar)
{
  uint16_t i;
   
   if(KeyChar == '*') return(1);
   for(i=15;i<16;i--)
     if(KeyChar == GPRSComBuf.GPRSCheckBuf[i]) return(1);
   return(0);
}

//SearchFirstChar
uint16_t SearchFirstChar(uint8_t * rdata,uint16_t len,uint8_t Dot)
{
  uint16_t i;
   for(i=0;i<len;i++)
     if(rdata[i] == Dot) break;
  return(i);
}
#endif

#if 0
uint32_t GPRS_Str2toint(uint8_t* rdata,uint8_t len)
{
  uint32_t temp,i;
	 
	 temp = 0;
	 for(i=0;i<len;i++)
	 {
      if((rdata[i] >= 0x30)&&(rdata[i] <= 0x39))
			{
          temp *= 10;
				  temp += (rdata[i] - 0x30);
			}
      else
         break;
   }
   return(temp);
}
#endif
/*********************************************************************************************
GPRS Driver

***********************************************************************************************/

struct GPRS_DEV g_GprsDev;
//SemaphoreHandle_t xSemaphore_gprsrec; 
static int SerachKeyWord(const uint8_t* src,uint16_t src_len,uint8_t cChar);
int Sim900aGprsRecv(struct GPRS_DEV *dev, u8* pBuf, const u16 wLen);
//static int Back_Search_SubArray(const uint8_t* src,uint16_t src_len,const uint8_t * search,uint16_t search_len);
#define READ_BLOCK_TIME 10

//static void gprs_DelayMs(__IO u32 myMs)
//{
//	// 72M
//	u16 i;
//	while(myMs--)
//	{
//		i=8000;
//		while(i--);
//	}
//}
static void gprs_DelayMs(uint32_t ticks)
{
 
   uint32_t tempticks;

   tempticks = SysRunTicks;
   while((uint32_t)(SysRunTicks - tempticks) < ticks)
   ;
 
}
//设置TIM4的开关
//sta:0，关闭;1,开启;

void GprsCmdWait(void )
{
	struct GPRS_DEV *pDev = GetGprsDev();
	uint8_t timeout = GPRS_CMD_WAIT_TIMEOUT;
	while(timeout--)		
	{
		if(pDev->ucRecDataFlag)
			break;
		gprs_DelayMs(100);
			
	}
}

void  vGprsReceiveTask( void  )
{
	//u8 ucGprsRecSatus = 0;
//	T_MSG tMsg;
	struct GPRS_DEV *pDev = GetGprsDev();
	u8 buf[GPRSRECBUFDEEP];
	u16 gprs_ack_len;
	static u8 send_fail_time = 0;
	//g_GprsRecvQueue = xQueueCreate(5, sizeof(T_MSG));
 
		if( pDev->ucUartPort  != NULL ) 
		{ 
			pDev->Read(pDev, buf, &gprs_ack_len);
			if (gprs_ack_len > GPRSRECBUFDEEP )
				gprs_ack_len = 0;
			if(0 ==Search_SubArray(buf,gprs_ack_len,"+IPD",4))
			{
				pDev->Read(pDev, buf, &gprs_ack_len);
				buf[gprs_ack_len] = '\0';
				/******添加接收服务器的数据处理********/
				//					WriteStrLogToSdcard("recieve data  from gprs sever\r\n");
				Sim900aGprsRecv(pDev,buf,gprs_ack_len);
				memset(pDev->buf,0,GPRSRECBUFDEEP);
				//memset(buf,0,GPRSRECBUFDEEP);
				//printf("rec is %s\r\n",buf);
				pDev->wReadPos = 0;
				gprs_ack_len = 0;				
			}
			if(0 ==Search_SubArray(buf,gprs_ack_len,"SEND OK\r\n",9))
			{
				//printf("send  OK\r\n");
				pDev->wReadPos = 0;
				gprs_ack_len = 0;					
				send_fail_time = 0;
			
			}
			if(0 ==Search_SubArray(buf,gprs_ack_len,"SEND FAIL",9))
			{
				printf("send  fail\r\n");					
				send_fail_time ++;
				if(send_fail_time > 2)
				{
					send_fail_time = 0;
					pDev->dwStatus = GPRS_TCP_CONNECTING;
				}
			
			}
			if(0 ==Search_SubArray(buf,gprs_ack_len,"TCP CLOSED",10))
			{
				printf("TCP closed\r\n");
				pDev->wReadPos = 0;
				gprs_ack_len = 0;	
				memset(pDev->buf,0,GPRSRECBUFDEEP);
				pDev->dwStatus = GPRS_TCP_CONNECTING;		 
			
			}
			if(0 ==Search_SubArray(buf,gprs_ack_len,"CLOSED",6))
			{
				printf("closed\r\n");
				pDev->wReadPos = 0;
				gprs_ack_len = 0;	
				memset(pDev->buf,0,GPRSRECBUFDEEP);
				pDev->dwStatus = GPRS_TCP_CONNECTING;		 
			
			}
	
			if(0 ==Search_SubArray(buf,gprs_ack_len,"ERROR",5))
			{					 
//					pDev->wReadPos = 0;
//					memset(pDev->buf,0,GPRSRECBUFDEEP);
//					 Sim900aTcpShut(pDev);
//					//pDev->SendCmd(pDev, "AT+CIPSHUT\r\n\0");
//					Sim900aIPStatus(pDev);
//					pDev->dwStatus = pDev->dwStatus>>1;//GPRS_TCP_CONNECTING;//
//		 						
			}
			gprs_ack_len = 0;
			 
			 
	 		
	}
	
}



struct GPRS_DEV *GetGprsDev(void)
{
	return &g_GprsDev;
}

static int Sim900aReset(void)
{
 
	GPRS_POWERKEY_L; /*enable*/
	gprs_DelayMs(50); 
	GPRS_RESET_L;
	gprs_DelayMs(60);
	GPRS_RESET_H;
	gprs_DelayMs(1000);
	return 0;		
}
int Sim900aPowerOn(void)
{	
	Sim900aReset();
	
	gprs_DelayMs(100);
	GPRS_POWERKEY_L; /*enable*/
	gprs_DelayMs(100);  /*delay 10tick(10ms)*/
	GPRS_POWERKEY_H; /*reset*/
	gprs_DelayMs(1500);  /*delay 10tick(10ms)*/
	GPRS_POWERKEY_L; /*enable*/
	gprs_DelayMs(3000);
	
	if(GPRS_STATUS_VAL)
	{
//		printf("gprs power on OK\n");
		gprs_DelayMs(100);
		return 0;
	}
//	printf("gprs power on fail\n");
	return -1;
}
static int Sim900aAck_ok(void)
{
	struct GPRS_DEV *pDev = GetGprsDev();
	int i = 0;
	
	//u16 wLen ;
	//u8 buf[256];
	//pDev->Read(pDev, buf, &wLen);
	while(i < AT_CMD_TIMEOUT)
	{
		if(pDev->wReadPos >= 4)
		{
			if(0 ==Search_SubArray(pDev->buf,16,"OK\r\n",4))
			{
				pDev->wReadPos = 0;
				return 0;
			}
			else
			{
				break;
			}
		}
		i++;
		gprs_DelayMs(100);
	}
	return -1;
}

static int Sim900aSendAck(struct GPRS_DEV *dev, u8 *pCmd)
{
	int i = 0;

	while(i< 3)
	{
		dev->SendCmd(dev, pCmd);
		//gprs_DelayMs(400);
		GprsCmdWait(); 
		if(0 == Sim900aAck_ok())
		{
			break;
		}
		i++;
	}
	if(i == 3)
		return -1;
	
	return 0;
}
// 是否关闭回显 1 为关闭回显  0 为 打开回显
static int Sim900aATE(struct GPRS_DEV *dev,uint8_t closeflag)
{
	u8 ticks = AT_CMD_TIMEOUT;
	if(closeflag == 1)
	{
		while(ticks --)
		{
			dev->SendCmd(dev, "ATE0\r\n\0");
			//gprs_DelayMs(300);
			GprsCmdWait();
			if(0 == Sim900aAck_ok())
			{
				//printf("ate0 ok\r\n");
				return 0;
			}
			gprs_DelayMs(100);
		}		
	}
	if(closeflag == 0)
	{
		while(ticks --)
		{
			dev->SendCmd(dev, "ATE1\r\n\0");
			//gprs_DelayMs(300);	
			GprsCmdWait()	;
			if(0 == Sim900aAck_ok())
			{
				//printf("ate1 ok\r\n");
				return 0;
			}
			gprs_DelayMs(100);
		}	
		
	}
	return -1;

	
}
static int Sim900aATtest(struct GPRS_DEV *dev)
{	
	u8 ticks = AT_CMD_TIMEOUT;
	  
	while(ticks --)
	{
		dev->SendCmd(dev, "AT\r\n\0");
		GprsCmdWait()	;	 
		if(0 ==Sim900aAck_ok())
		{
//			printf("at test ok \n");
			return 0;
		}
		gprs_DelayMs(100);
	}
	return -1;
}
static int Sim900aCardCheck(struct GPRS_DEV *dev)
{
	u8 timeout = AT_CMD_TIMEOUT;
	u16 wLen;
	u8 buf[GPRSRECBUFDEEP];
	static u8 err_times = 0;
	err_times++;
	if(err_times > 10)
	{
		err_times = 0;
		dev->dwStatus = GPRS_POWERON;
	}
	while(timeout--)
	{
		if(0 ==Sim900aATE(dev,1))
			break;
	}
	timeout = AT_CMD_TIMEOUT;
	while(timeout --)
	{ 
		dev->SendCmd(dev, "AT+CPIN?\r\n\0");
		GprsCmdWait()	;
		dev->Read(dev, buf, &wLen);
		if(0 ==Search_SubArray(buf,wLen,"+CPIN: READY\r\n",14))
		{
			err_times = 0;
//			printf("SIM Card Check OK! \n");
			dev->dwStatus =  GPRS_TCP_CONFIG ;
			return 0;
		}
		gprs_DelayMs(100);
	}
	return -1;
	
}
// 关机  bHardClose 1 为硬件关机 
int Sim900aClose(struct GPRS_DEV *dev,uint8_t bHardClose)
{
	/*close sim900a*/
	u16 wLen;
	u8 buf[GPRSRECBUFDEEP];
	if(1 == bHardClose)
	{
		// 1 硬件关机 
		if(GPRS_STATUS_VAL)
		{
			GPRS_POWERKEY_L; 
			gprs_DelayMs(100); 
			GPRS_POWERKEY_H; 
			gprs_DelayMs(1500);  
			GPRS_POWERKEY_L; /*enable*/
			gprs_DelayMs(2000);
			if(0 == GPRS_STATUS_VAL)
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}
		
	if(0 == bHardClose)
	{
		// 0 软件关机
		dev->SendCmd(dev,"AT+CPOWD=1\r\n\0");
		GprsCmdWait()	;
		dev->Read(dev, buf, &wLen);
		if(0 == Search_SubArray(buf,wLen,"NORMAL POWER DOWN",17))
		{
			//指令关机
				return 0;
		}
		return -1;
	}
	return -1;
	
}
int Sim900aOpen(struct GPRS_DEV *dev)
{
	 //开机
	 
	u8 ticks = AT_CMD_TIMEOUT;
	//Sim900aClose(dev,0);
	Sim900aReset();
	// 先发AT指令 如果返回OK 则表示已经开机
	if(0 ==Sim900aATtest(dev))
	{
		// entry next
		dev->dwStatus =  GPRS_SIM_CARD;
		return 0;
	}
	while(ticks--)
	{
		if(0 == Sim900aPowerOn())
		{			 		 
			break;			
		}
	}
	if(0 ==Sim900aATtest(dev))
	{
		// entry next
		dev->dwStatus =  GPRS_SIM_CARD;
		return 0;
	}
	else
	{
		dev->dwStatus = GPRS_POWERON;
		return -1;
	}	
}
//关闭移动场景
int Sim900aTcpShut(struct GPRS_DEV *dev)
{
	u8 buf[GPRSRECBUFDEEP];
	u16 wLen;
	u8 ticks = AT_CMD_TIMEOUT;
  	while(ticks --)
	{
		// 关闭移动场景
		// AT+CIPSHUT
		dev->SendCmd(dev,"AT+CIPSHUT\r\n\0");
		GprsCmdWait()	;
		dev->Read(dev, buf, &wLen);
		if(0 ==Search_SubArray(buf,wLen,"SHUT OK",7))
		{
			//printf("SHUT OK! \n");
			return 0;
		}
		if(0 ==Search_SubArray(buf,wLen,"ERROR",5))
		{
			//printf("SHUT ERROR ! \n");
			return 0;
		}
	}
	return -1;
	
	
}

int Sim900aInit(struct GPRS_DEV *dev)
{
	dev->dwStatus =  GPRS_POWERON; 
	//Sim900aTcpShut(dev);
	/**/
//	Gprs_TcpConnect();
	return 0;
}
// 查询网络状态
int Sim900aCREG(struct GPRS_DEV *dev,u8 reg)
{
	u8 ticks = AT_CMD_TIMEOUT;
	int wPos;
	u8 buf[GPRSRECBUFDEEP];
	u16 wLen;
	if(reg == 0)	// creg   gsm网络
	{
		while(ticks --)
		{
			dev->SendCmd(dev, "AT+CREG?\r\n\0");
		//	gprs_DelayMs(400);
			GprsCmdWait()	;
			dev->Read(dev, buf, &wLen);
			if(0 ==Search_SubArray(buf,wLen,"+CREG",5))
			{
				wPos = SerachKeyWord(buf,wLen,':');
				if(-1 !=  wPos)
				{
					if((buf[wPos+4] == '1') ||(buf[wPos+4] == '5'))
					{
						//printf("creg ok \r\n");
						return 0;
					}
				}
			}
		}
	}
	if(1 == reg)			//gprs 网络
	{
		while(ticks --)
		{
			dev->SendCmd(dev, "AT+CGREG?\r\n\0");
			//gprs_DelayMs(400);
			GprsCmdWait()	;
			dev->Read(dev, buf, &wLen);
			if(0 ==Search_SubArray(buf,wLen,"+CGREG",6))
			{
				wPos = SerachKeyWord(buf,wLen,':');
				if(-1 !=  wPos)
				{
					if((buf[wPos+4] == '1') ||(buf[wPos+4] == '5'))
					{
						//printf("cgreg ok \r\n");
						return 0;
					}
				}
			}				
		}
		
	}
	return -1;
}

 

int Sim900aTcpClose(struct GPRS_DEV *dev)
{
	// AT+CIPCLOSE=1\r\n\0
	u8 buf[GPRSRECBUFDEEP];
	u16 wLen;
	u8 ticks = AT_CMD_TIMEOUT;		
	// 关闭移动场景
	// AT+CIPSHUT
	while(ticks --)
	{
		dev->SendCmd(dev,"AT+CIPCLOSE=1\r\n\0");
		//gprs_DelayMs(400);
		GprsCmdWait()	;
		dev->Read(dev, buf, &wLen);
		if(0 ==Search_SubArray(buf,wLen,"CLOSE OK",8))
		{
			//printf("CLOSE OK! \n");
			dev->dwStatus =  GPRS_TCP_CONFIG ;
			return 0;
		}
		gprs_DelayMs(100);
	}
	return -1;
	
}


// 检测IP 状态
#define	 		IP_INITIAL 			0x00	//  初始化
#define   		IP_START			0x01	//启动任务
#define			IP_CONFIG			0x02	//配置场景
#define 		IP_GPRSACT			0x03	//接受场景配置
#define 		IP_STATUS			0x04	//获取ip
#define 		TCP_CONNECTING 		0x05	//连接中
#define   		TCP_CONNECTOK  		0x06	//连接成功
#define 		TCP_CLOSING			0X07	//关闭tcp 中
#define 		TCP_CLOSED			0x08	//已关闭
#define 		PDP_DEACT			0x09	// 场景被释放
int Sim900aIPStatus(struct GPRS_DEV *dev)
{
	u8 buf[GPRSRECBUFDEEP];
	u16 wLen;
	u8 ticks = AT_CMD_TIMEOUT;
	while(ticks --)
	{
		dev->SendCmd(dev, "AT+CIPSTATUS\r\n\0");
		gprs_DelayMs(600);
		dev->Read(dev, buf, &wLen);
		if(Search_SubArray(buf,wLen,"IP INITIAL",10) == 0)
		{
			//printf("IP_INITIAL\r\n");
			return IP_INITIAL;
		}
		if(0 ==Search_SubArray(buf,wLen,"IP START",8))
		{
			//printf("IP_START\r\n");
			return IP_START;
		}
		if(0 ==Search_SubArray(buf,wLen,"IP CONFIG",9))
		{
			//printf("IP_CONFIG\r\n");
			return IP_CONFIG;
		}
		if(0 ==Search_SubArray(buf,wLen,"IP GPRSACT",10))
		{
			//printf("IP GPRSACT\r\n");
			return IP_GPRSACT;
		}
		
		if(0 ==Search_SubArray(buf,wLen,"IP STATUS",8))
		{
			//printf("IP_START\r\n");
			return IP_STATUS;
		}
		if(0 ==Search_SubArray(buf,wLen,"TCP CONNECTING",14))
		{
			
			//printf("TCP CONNECTING\r\n");
			return TCP_CONNECTING;
		}	
		
		if(0 ==Search_SubArray(buf,wLen,"CONNECT OK",8))
		{
			//printf("CONNECT OK\r\n");
			return TCP_CONNECTOK;
		}	

		if(0 ==Search_SubArray(buf,wLen,"TCP CLOSING",8))
		{
			//printf("TCP CLOSING\r\n");
			return TCP_CLOSING;
		}	
		
		if(0 ==Search_SubArray(buf,wLen,"TCP CLOSED",8))
		{			
			return TCP_CLOSED;
		}	
		
		if(0 ==Search_SubArray(buf,wLen,"PDP DEACT",8))
		{
			Sim900aTcpShut(dev);
			return PDP_DEACT;
		}	

	}
	return -1;
}
// 配置tcp连接
int Sim900aTcpConfig(struct GPRS_DEV *dev)
{ 
	u8 buf[GPRSRECBUFDEEP];  
	u16 wLen;
	int wPos;
	u16 usTicks = AT_CMD_TIMEOUT;
	int ret = -1;
	u8 gprs_signal_val;
	static u8 err_times = 0;
	//uint8_t cong_sta = 0;
	/*******
	**AT+CPIN?   AT+CSQ?   AT+CGATT?
** AT+CSTT  AT+CIICR  AT+CIFSR 
**AT+CIPSTART="TCP","IP",PORT
	**AT+CIPSHUT  关闭移动场景
	**AT+CIPCLOSE=1 快关 tcp
	*********/
	err_times++;
	if(err_times> 2)
	{
		err_times = 0;
		dev->dwStatus = GPRS_POWERON;
		return -1;
	}
	while(usTicks --)
	{
		 if(0 !=dev->GetSignal(dev,&gprs_signal_val))
			 continue;	
		gprs_DelayMs(300);	
		if(0 != Sim900aCREG(dev,0))
			continue;

//		dev->SendCmd(dev, "AT+CGREG=1\r\n\0");
//		gprs_DelayMs(300);
		
		if(0 != Sim900aCREG(dev,1))
			continue;
// 		//Sim900aTcpShut	
// 		if(0 != Sim900aTcpShut(dev))
// 			continue;

		dev->SendCmd(dev, "AT+CGATT=?\r\n\0");
		gprs_DelayMs(300);
		dev->Read(dev, buf, &wLen);
		if(wLen > 6)
		{
			wPos = SerachKeyWord(buf,wLen,':');
			if(-1 !=  wPos)
			{
				if((buf[wPos+5] == '1') ||(buf[wPos+5] == '5'))
				{
					//printf("CGATT ok \r\n");
				}
			}
			else
			{
				dev->SendCmd(dev, "AT+CGATT=1\r\n\0");
				gprs_DelayMs(300);
				dev->Read(dev, buf, &wLen);
				continue;
			}
		}
		else
			continue;
		dev->SendCmd(dev, "AT+CIPSTATUS\r\n\0");
		gprs_DelayMs(600);
		dev->Read(dev, buf, &wLen);
		if(wLen > 4)
		{
			gprs_DelayMs(100);
		}
		else
			continue;
		if(IP_INITIAL == Sim900aIPStatus(dev))
		{
			//设置APN
			ret = Sim900aSendAck(dev, "AT+CSTT\r\n\0");
			if(ret != 0)
			{
				continue;
			}			
		}
		if( IP_START == Sim900aIPStatus(dev))
		{
			//激活移动场景
			ret = Sim900aSendAck(dev, "AT+CIICR\r\n\0");
			if(ret != 0)
			{
				//printf("CIICR error \r\n");
				continue;
			}
			else
			{
				//printf("CIICR OK \r\n");
				
			}
		}
		
		if(Sim900aIPStatus(dev) > IP_START)
		{
			//获得本地IP
			dev->SendCmd(dev, "AT+CIFSR\r\n\0");
			gprs_DelayMs(600);
			dev->Read(dev, buf, &wLen);
			if(wLen > 4)
			{
				//输出 IP
		//		Serial_PutData(serCOM1,buf,wLen);
				//WriteDataLogToSdCard(buf,wLen );
				gprs_DelayMs(100);
			}
			else
				continue;	 			
		}
		Sim900aIPStatus(dev);	


		ret = Sim900aSendAck(dev, "AT+CIPHEAD=1\r\n\0");
		if(ret != 0)
		{
			//printf("CIPHEAD fail \r\n");
			continue;
		}
		else
		{
			//printf("CIPHEAD config ok\r\n");
			
		}
//		// 设置数据 5s后自动发送
//		ret = Sim900aSendAck(dev, "AT+CIPATS=1,1\r\n\0");
//		if(ret != 0)
//		{
//		//	printf("CIPATS fail \r\n");
//			continue;
//		}
//		else
//		{
//			//printf("CIPATS ok\r\n");
//			
//		}		 
		Sim900aIPStatus(dev);
		
		dev->dwStatus = GPRS_TCP_CONNECTING;
		err_times = 0;
		return 0;
	}	
//	WriteStrLogToSdcard("tcp config fail\r\n");
	//printfk("tcp config fail\r\n");
	
	return -1;
}
int Sim900aTcpConnect(struct GPRS_DEV *dev)
{
		static uint8_t error_ticks;
 
	uint8_t buf[GPRSRECBUFDEEP];
	uint16_t wLen;
	int ret;
	ParameterStruct tGetPar;
	GetSetedPar(&tGetPar);
 
	gprs_DelayMs(50);
	dev->SendCmd(dev,"AT+CIPSTART=\"TCP\",\0");
	//dev->SendCmd(dev,"\"114.221.124.76\",\"7903\"\0"); //q447552640.gicp.net
 	if(tGetPar.GPRSSeverAddr[0] == '\"')
 		dev->SendCmd(dev,(u8*)tGetPar.GPRSSeverAddr);
 	else
	{
 	 
		dev->SendCmd(dev,"\"lorytest.lorytech.com\",\"7903\"\0");  // //ctr.lorytech.com

	}
	dev->SendCmd(dev,"\r\n\0");	
	memset(dev->buf,0,GPRSRECBUFDEEP);
	dev->wReadPos = 0;
	//vTaskDelay(2000);
	gprs_DelayMs(3000);
	dev->Read(dev, buf, &wLen);
	error_ticks++;
 
	if(0 == Search_SubArray(buf,wLen,"CONNECT OK",10))
	{
//		WriteStrLogToSdcard(" tcp connected OK \r\n");
		memset(dev->buf,0,GPRSRECBUFDEEP);
		dev->dwStatus = GPRS_TCP_OK;
		dev->wReadPos = 0;
		error_ticks = 0;
		return 0;
	}
	if(error_ticks > 3)
	{
		error_ticks = 0;
		dev->dwStatus = GPRS_TCP_CONFIG ;	
		return -1;
	}
	if(0 == Search_SubArray(buf,wLen,"OK",2))
	{
		//vTaskDelay(1000);
                dev->Read(dev, buf, &wLen);
		if(0 == Search_SubArray(buf,wLen,"CONNECT OK",10))
		{
//			WriteStrLogToSdcard(" tcp connected OK \r\n");
			memset(dev->buf,0,GPRSRECBUFDEEP);
			dev->dwStatus = GPRS_TCP_OK;
			dev->wReadPos = 0;
			error_ticks = 0;
			return 0;
		}
	 
	}
	if(0 == Search_SubArray(buf,wLen,"ERROR\r\n\r\nALREADY CONNECT",20))
	{
		// 连接已经存在
//		WriteStrLogToSdcard("tcp already connected \r\n");
	 
		memset(dev->buf,0,GPRSRECBUFDEEP);
		dev->wReadPos = 0;
		ret = Sim900aIPStatus(dev);
		if(ret ==PDP_DEACT )
		{
			Gprs_TcpClose();
		}
		else if (ret== TCP_CONNECTING)
		{
			gprs_DelayMs(10);
		}
		 Sim900aTcpShut(dev);
		 dev->dwStatus = GPRS_TCP_CONNECTING;//GPRS_TCP_OK
		//dev->dwStatus = GPRS_TCP_OK;//
		return 1;
	}

	if(0 == Search_SubArray(buf,wLen,"READY CONNECT",13))
	{
		// 连接已经存在
//		WriteStrLogToSdcard("tcp already connected \r\n");
		 error_ticks = 0;
		 
		memset(dev->buf,0,GPRSRECBUFDEEP);
		dev->wReadPos = 0;
		 dev->dwStatus = GPRS_TCP_OK; 		 
		return 0;
	}
	if(0 == Search_SubArray(buf,wLen,"ERROR",5))
	{
		//printfk(" tcp connecting error! \r\n");
		dev->dwStatus = GPRS_TCP_CONNECTING;
		 
		return 1;
		 
	}
	if(0 == Search_SubArray(buf,wLen,"CONNECT FAIL",12))
	{
		////printfk("tcp connect fail! \r\n");
                	Gprs_TcpClose();
		Sim900aTcpShut(dev);
		dev->dwStatus = GPRS_TCP_CONNECTING;
		return 1;
		
	}
	return -1;
	
}
int Sim900aGetLocal(struct GPRS_DEV *dev)
{
	/**
	** ：AT+SAPBR=3,1,"Contype","GPRS"
	**
	**：AT+SAPBR=3,1,"APN","CMNET"
	**：AT+SAPBR=1,1
	**AT+SAPBR=2,1
	**：AT+CIPGSMLOC=1,1
	*******/
	return 0;
}


int Sim900aCmd(struct GPRS_DEV *dev, u8 *pCmd)
{
	dev->wReadPos = 0;
	dev->ucRecDataFlag = 0;
	Serial_PutString(dev->ucUartPort, pCmd);
	
	return 0;
}
int Sim900aGprsRead(struct GPRS_DEV *dev, u8* pBuf, u16 *pwLen)
{
	/*wait data from interrupt*/
	if(dev->wReadPos > GPRSRECBUFDEEP)
		return -1;
	if(dev->wReadPos != 0)
	{
		memcpy(pBuf, dev->buf, dev->wReadPos);
		*pwLen = dev->wReadPos;	
	}
	
	return 0;
}


int Sim900aGetConnectStatus(struct GPRS_DEV *dev)
{
	

	return dev->dwStatus;
}
//获取信号强度
int Sim900aGetSignal(struct GPRS_DEV *dev, u8* dwVal)
{
	u8 ticks = AT_CMD_TIMEOUT;
	u16 wLen;
	u8 buf[GPRSRECBUFDEEP];
	int wPos;
	while(ticks --)
	{
		dev->SendCmd(dev, "AT+CSQ\r\n\0");
		gprs_DelayMs(500);
		dev->Read(dev, buf, &wLen);
		if(0 ==Search_SubArray(buf,wLen,"+CSQ:",5))
		{
			wPos = SerachKeyWord(buf,wLen,':');
			buf[wLen] = '\0';
			if(-1 !=  wPos)
			{
				*dwVal = (buf[wPos+2] - '0')*10+ (buf[wPos+3] - '0');
				//printf("%s\r\n",buf);
				//printf("%d \r\n",dwVal);
				return 0;
			}
		}
		gprs_DelayMs(100);
	}
	return -1;
 
}
// gprs 发送数据
int Sim900aGprsWrite(struct GPRS_DEV *dev, u8* pBuf, const u16 wLen)
{
	/*发送数据*/
	uint8_t temp[5];
	static u8 timeout_nums = 0;
	u16  wlen_temp; 
	uint16_t timeout = 0;
	dev->wReadPos = 0;
	wlen_temp = wLen;
	if(wLen > GPRSTRABUFDEEP) wlen_temp = GPRSTRABUFDEEP;
	if(wLen == 0) return -1;
	dev->SendCmd(dev,"AT+CIPSEND=\0");
	
	temp[0] = (wlen_temp / 1000) + 0x30; 
	wlen_temp = wlen_temp % 1000;
	temp[1] = (wlen_temp / 100) + 0x30; 
	wlen_temp = wlen_temp % 100;
	temp[2] = (wlen_temp / 10) + 0x30; 
	temp[3] = (wlen_temp % 10) + 0x30;
	temp[4] = '\0';
	dev->SendCmd(dev,temp);
	dev->SendCmd(dev,"\r\n\0"); 
	//3e
	while(1)
	{
		 gprs_DelayMs(500); 
		//vTaskDelay(1000);
 		timeout ++;
 		if(timeout > 4)
 		{
			timeout = 0;
			timeout_nums++;
			if(timeout_nums> 2)
			{
				timeout_nums = 0;
				dev->dwStatus = GPRS_TCP_CONNECTING;
				//printfk("gprs send err\r\n");
				return -1;
			}
			break;

 		}
		if(dev->wReadPos >0)
		{
		  
			if(dev->buf[2] == 0x3e)
				break;
		}		
	}
 
	//dev->Read(dev, buf, &wReadLen);
	if(wLen > GPRSTRABUFDEEP)
		Serial_PutData(dev->ucUartPort,pBuf,GPRSTRABUFDEEP);
	else
		Serial_PutData(dev->ucUartPort,pBuf,wLen);

	 
	dev->wReadPos = 0;
 	memset(dev->buf,0,GPRSRECBUFDEEP);
	gprs_DelayMs(200);	
	return 0;
}

void Gprs_IOConfig(void)
{
	  GPIO_InitTypeDef GPIO_InitStructure;
	////*----------------------------*//////////
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);
 
	//GSM_PWRKEY  gprs 开关		
  GPIO_InitStructure.GPIO_Pin = GPRS_RESET_PIN| GPRS_PWR_PIN;//  | GPIO_Pin_9  | GPIO_Pin_14 ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPRS_PORT,&GPIO_InitStructure);

//   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
//   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
//   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//   GPIO_Init(GPIOB,&GPIO_InitStructure);
//   //GPRS 管脚////////////
  GPIO_InitStructure.GPIO_Pin =   GPRS_RI_PIN | GPRS_STATUS_PIN;//GPIO_Pin_11;// | GPIO_Pin_12 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
  GPIO_Init(GPRS_PORT,&GPIO_InitStructure);	
 }
// gprs 初始化
int Gprs_Init(void)
{
	struct GPRS_DEV *pDev = GetGprsDev();
	Gprs_IOConfig();
	Gprs_SerInit();
	ConfigGPSDMARec();
	//Serial_RegRecvHandler(GPRS_SER_PORT, Gprs_RecvHandler);
	TIM4_Init(30);    // 用于串口中断 定时
	TIM4_Set(0);
	memset(pDev->buf,0,GPRSRECBUFDEEP);
	
	pDev->wReadPos = 0;
	pDev->ucUartPort = GPRS_SER_PORT;
 
	
	pDev->Init = Sim900aInit;
	pDev->Open = Sim900aOpen;
	pDev->Close = Sim900aClose;
	pDev->SendCmd = Sim900aCmd;
	pDev->Read = Sim900aGprsRead;
	pDev->Write = Sim900aGprsWrite;
	pDev->GetConnetStatus = Sim900aGetConnectStatus;
	pDev->GetSignal = Sim900aGetSignal;
	pDev->HandleRecv = Sim900aGprsRecv;

	pDev->Init(pDev);
	
	return 0;
}
int Gprs_Open(void)
{
	struct GPRS_DEV *pDev = GetGprsDev();

	return pDev->Open(pDev);
}
int Gprs_SimCardCheck(void)
{
	struct GPRS_DEV *pDev = GetGprsDev();
	
	return Sim900aCardCheck(pDev);
}
int Gprs_TcpConfig(void)
{
	struct GPRS_DEV *pDev = GetGprsDev();
	
	return Sim900aTcpConfig(pDev);
}
int Gprs_TcpConnect(void)
{
	struct GPRS_DEV *pDev = GetGprsDev();
	
	return Sim900aTcpConnect(pDev);
}
int Gprs_TcpClose(void)
{
	struct GPRS_DEV *pDev = GetGprsDev();
	
	return Sim900aTcpClose(pDev);
}
int Gprs_Close(void)
{
	struct GPRS_DEV *pDev = GetGprsDev();
	
	return pDev->Close(pDev,0);
}
int Gprs_SendCmd(u8 *pCmd, u16 wCmdLen)
{
	struct GPRS_DEV *pDev = GetGprsDev();
	
	return pDev->Write(pDev, pCmd, wCmdLen);
}
int Gprs_Write(u8* pBuf, u16 wLen)
{   
	struct GPRS_DEV *pDev = GetGprsDev();

	return pDev->Write(pDev, pBuf, wLen);
}

int Gprs_Read(u8* pBuf, u16 *pwLen)
{
	struct GPRS_DEV *pDev = GetGprsDev();

	return pDev->Read(pDev, pBuf, pwLen);
}
int Gprs_GetConnectStatus(u32 *pdwStatus)
{
	struct GPRS_DEV *pDev = GetGprsDev();

	*pdwStatus = pDev->dwStatus;
	
	return 0;
}
int Gprs_GetRecStatus(u8 *pucStatus)
{
	struct GPRS_DEV *pDev = GetGprsDev();

	*pucStatus = pDev->ucRecDataFlag;
	return 0;
}

int Gprs_RecHandle(u8* pBuf, const u16 pwLen)
{
	struct GPRS_DEV *pDev = GetGprsDev();

	return pDev->HandleRecv(pDev, pBuf, pwLen);
	
}
int Sim900aGprsRecv(struct GPRS_DEV *dev, u8* pBuf, const u16 wLen)
{
	u8 ucHeadSum;
	u16 i,headstart;
	u16 wMsgLen;
	
	T_MSG tMsg;
    T_MSG_RECV *ptRecvMsg = (T_MSG_RECV *)&tMsg.Data[0];
	if(wLen < HEAD_SIZE)
		return -1;
	if(wLen > GPRSRECBUFDEEP)
		return -1;
	for(i=0;i<wLen;i++)
	{
		if(pBuf[i] == 0xc0)
			if(pBuf[i+1] == 0xc0)
				if(pBuf[i+2] == 0xc0)
					break;
	}
	if(i == wLen)
		return -1;
	headstart = i+3;
	ucHeadSum = 0;
	for(i=headstart;i<headstart+7;i++)
		ucHeadSum += pBuf[i];
	if(ucHeadSum != pBuf[headstart+7])
		return -1;
	tMsg.wMsgType = MSG_TYPE_RECV_GPRS;
	memcpy((u8*)tMsg.Data,&pBuf[headstart],wLen-headstart);
	wMsgLen = ptRecvMsg->wLen[0] + (ptRecvMsg->wLen[1]<<8);
    ptRecvMsg->wDataCrc[0] = tMsg.Data[wMsgLen+MSG_HEADSIZE-2];
    ptRecvMsg->wDataCrc[1] = tMsg.Data[wMsgLen+MSG_HEADSIZE-1];
//	xQueueSend(g_pDataProcQueue, &tMsg, 20);	
	return 0;
}




 

 

static int SerachKeyWord(const uint8_t* src,uint16_t src_len,uint8_t cChar)
{
	uint16_t i;
	
	for(i = 0; i< src_len;i++)
	{
		if(src[i] == cChar)
			return i;
	}
	return -1;
}
