/*
********************************************************************************
* @file    KeyDrv.h
* @author  
* @version V1.0.0
* @date    03/20/2013
* @brief   
********************************************************************************
*/
#ifndef __KEY_SCAN_DRV_H_FIL__ 
#define __KEY_SCAN_DRV_H_FIL__ 

#include <stdint.h>
#include "ConfigAll.h" 

#if     defined(__STM32_PLATFORM_BOARD__)
#include "stm32f10x.h" 
#include "PortMapLib.h" 
#else
#include "hsf.h" 
#include "DevDataType.h"
#endif 

// Ӳ����ض��� 
#define     LOW_VOL                 0                                   // �͵�ƽ 
#define     HIGHT_VOL               1                                   // �ߵ�ƽ 

#if     defined(__STM32_PLATFORM_BOARD__)
#define     KEY_PIN_NUM             14                                  // ����Pin�ź� 
#define     KEY_IN                  PBin(KEY_PIN_NUM)                   // Pin�ſ�λ 
#else 
  #if  defined(__HF_BOARD__)
    #define     KEY_IN                  hfgpio_fpin_is_high(HFGPIO_F_SLEEP_RQ)
  #else
    #define     KEY_IN                  hfgpio_fpin_is_high(HFGPIO_SW_KEY_CTL_PIN)
  #endif 
#endif 

#define     KEY_PRESS_STATUS        LOW_VOL 

// 
#define     KEY_LONG_PRESS_3S       500                                 // 

#define     KEY_SCAN_ON			    0x00 
#define     KEY_SCAN_OFF		    0x01 

#define     KEY_0_NUM               0 
// 
typedef enum 
{ 
    STATUS_STABLE=0, 
    STATUS_CHG_VAR 
}KeyChgType; 
typedef struct KeyStruct 
{ 
    KeyChgType  Data_Change_Flag;
    uint8_t     Matrix_Scan_Flag;                                       // Scan Key 
    uint8_t     KeyStatusCHK;                                           // ������Ч��У�� 
    
	uint8_t     KeyTemp;                                                // ��ǰ������ƽ״̬ 
	uint8_t     KeyLastStatus;                                          // ��һ�ΰ�����ƽ״̬ 
	
	uint8_t     UpDown;                                                 // �˱�־��Ϊö�����������ʶ�����������½��أ������ǳ���
	uint16_t    KeyDownTimeCount;                                       // ��������ʱ�����(��Ҫ֧�ֶ������,���Ϊ����)
}KeyType; 

// 
extern void     Key_Init(void);                                         // ������ʼ�� 
extern void     Key_Scan(void);                                         // ����ɨ�� 
extern void     Key_DriveEngine(int32_t ArgTick);                       // ��������ɨ��ʱ�� 
extern void     Key_Poll(void);                                         // �����¼����� 


// 
#endif 
/******************************************************************************* 
* ========================= END OF PAGE 
*******************************************************************************/ 

