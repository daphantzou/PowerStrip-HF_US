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

// 硬件相关定义 
#define     LOW_VOL                 0                                   // 低电平 
#define     HIGHT_VOL               1                                   // 高电平 

#if     defined(__STM32_PLATFORM_BOARD__)
#define     KEY_PIN_NUM             14                                  // 按键Pin脚号 
#define     KEY_IN                  PBin(KEY_PIN_NUM)                   // Pin脚口位 
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
    uint8_t     KeyStatusCHK;                                           // 按键有效性校验 
    
	uint8_t     KeyTemp;                                                // 当前按键电平状态 
	uint8_t     KeyLastStatus;                                          // 上一次按键电平状态 
	
	uint8_t     UpDown;                                                 // 此标志若为枚举形则可用于识别上升还是下降沿，或者是长按
	uint16_t    KeyDownTimeCount;                                       // 按键按下时间计数(若要支持多个按键,则此为数组)
}KeyType; 

// 
extern void     Key_Init(void);                                         // 按键初始化 
extern void     Key_Scan(void);                                         // 按键扫描 
extern void     Key_DriveEngine(int32_t ArgTick);                       // 驱动按键扫描时钟 
extern void     Key_Poll(void);                                         // 按键事件处理 


// 
#endif 
/******************************************************************************* 
* ========================= END OF PAGE 
*******************************************************************************/ 

