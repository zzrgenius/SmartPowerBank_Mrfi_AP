
#ifndef __COMMONUSE_H
#define __COMMONUSE_H
#include "stm32f10x.h" 
/**********************************************************
* �������� ---> BCD��תΪHEX
* ��ڲ��� ---> BCD_Data��Ҫת����BCD����
* ������ֵ ---> HEX��
* ����˵�� ---> none
**********************************************************/	
uint8_t BCD_to_HEX(uint8_t BCD_Data);
 
/**********************************************************
* �������� ---> HEX��תΪBCD
* ��ڲ��� ---> HEX_Data��Ҫת����BCD����
* ������ֵ ---> BCD��
* ����˵�� ---> none
**********************************************************/	
uint8_t HEX_to_BCD(uint8_t HEX_Data);
 
/**********************************************************
* �������� ---> 10������תΪ16����
* ��ڲ��� ---> DX_Data��Ҫת����10��������
* ������ֵ ---> 16����
* ����˵�� ---> none
**********************************************************/
uint16_t DX_to_HX(uint16_t DX_Data);
 
/**********************************************************
* �������� ---> 16������תΪ10����
* ��ڲ��� ---> HX_Data��Ҫת����16��������
* ������ֵ ---> 10����
* ����˵�� ---> none
**********************************************************/
uint16_t HX_to_DX(uint16_t HX_Data);
int Search_SubArray(const uint8_t* src,uint16_t src_len,const uint8_t * search,uint16_t search_len);


#endif