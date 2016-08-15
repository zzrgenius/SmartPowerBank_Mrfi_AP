#include "stm32f10x.h"
#include "CommonUse.h"
/**********************************************************
* 函数功能 ---> HEX 转为BCD码
* 入口参数 ---> BCD_Data：要转换的BCD数据
* 返回数值 ---> HEX码
* 功能说明 ---> none
**********************************************************/	
uint8_t HEX_to_BCD(uint8_t HEX_Data)
{
	return((HEX_Data / 10) << 4 | (HEX_Data % 10));
}
/**********************************************************
* 函数功能 ---> BCD码转为HEX
* 入口参数 ---> HEX_Data：要转换的BCD数据
* 返回数值 ---> BCD码
* 功能说明 ---> none
**********************************************************/	
uint8_t BCD_to_HEX(uint8_t BCD_Data)
{
	return((BCD_Data >> 4) * 10 + (BCD_Data & 0x0f));
}
/**********************************************************
* 函数功能 ---> 10进制码转为16进制
* 入口参数 ---> DX_Data：要转换的10进制数据
* 返回数值 ---> 16进制
* 功能说明 ---> none
**********************************************************/
uint16_t DX_to_HX(uint16_t DX_Data)
{
	return(((DX_Data/1000)<<12) | ((DX_Data%1000/100)<<8) | ((DX_Data%100/10)<<4) | (DX_Data%10));
}
/**********************************************************
* 函数功能 ---> 16进制码转为10进制
* 入口参数 ---> HX_Data：要转换的16进制数据
* 返回数值 ---> 10进制
* 功能说明 ---> none
**********************************************************/
uint16_t HX_to_DX(uint16_t HX_Data)
{
	return((HX_Data>>12)*1000+((HX_Data&0x0f00)>>8)*100+((HX_Data&0x00f0)>>4)*10+(HX_Data&0x000f));
}	

// src  源数组 ,src_len 源数组长度,const uint8 * search 子数组  search_len 子数组长度
  int Search_SubArray(const uint8_t* src,uint16_t src_len,const uint8_t * search,uint16_t search_len)
{
	uint16_t i,j;
	if(src_len < search_len)
		return -1;
	if((src_len == 0)||(search_len == 0))
		return -1;
	
	for(i = 0; i< src_len;i++) 
	{
		if(src[i] == search[0])
		{
			for(j=0;j< search_len;j++)
			{
				if(search[j] == src[j+i])
					continue;
				else
					break;					
			}
			if(j == search_len)
			{
				return 0;
			 
			}
		}
	}
	return -1;
	
}
