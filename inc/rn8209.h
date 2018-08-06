/*
********************************************************************************
* @file    : rn8209.h 
* @author  : 
* @version : V1.0.0 
* @date    : 2014/06/27 
* @brief   : 
*********************************************************************************/
#ifndef __RECO_RN8209_H__
#define __RECO_RN8209_H__
 
#include "ConfigAll.h"
#include "DevRule.h"
#include "rn8209.h"

#include <stdint.h>


#define CONFIRM_COUNT	3


#if  defined(HANK_JOE_CUSTOMER_REMOTE_CTL) 
 #define CHECK_DATA_VOLT_DEFALUT         95317   // 12045 
 #define CHECK_DATA_CURR_DEFALUT         437     // 337 
 #define CHECK_DATA_POWER_DEFALUT        100     // 
#else 
 #define CHECK_DATA_VOLT_DEFALUT         97247  // 12045 
 #define CHECK_DATA_CURR_DEFALUT         337      // 337 
 #define CHECK_DATA_POWER_DEFALUT        99   // 
#endif 
 

typedef struct 
{ 
    int record_ValidFreq;       //频率有效值
    
	int record_ValidCurrent;    // 电流有效值 
	int CurrentCoefficent;  
	
	int record_ValidVolt;       // 电压有效值 
	int VoltCoefficent;  
	
	int record_ValidPower;      // 有功功率 
	int PowerCoefficent;  
}PowerRecordType; 

typedef struct 
{ 
	int reco_irms;              // 电流有效值原始值 
	int reco_urms;              // 电压有效值原始值 
	int reco_freq;              // 频率原始值 
	int reco_powerp;            // 有功功率原始值 
}RecoMeasurePack; 


 
//extern RecoMeasurePack reco_measure_data[PORT_NUM];
extern PowerRecordType PowerRecordObj[PORT_NUM]; 
extern uint8_t CalibrateData[8];


#if (defined(__CARIBRATION_POWER_COEFFICIENT_DBG__) || defined(__RN8209_SPI_DBG__)) && defined(__UDP_PRINT_DBG__)
extern uint8_t RN8209_Test(char *pArgBuf);

extern uint8_t RN8209_SetPwrCoffTest(char *pArgBuf);
#endif 


extern int Read_Rn8209_deviceID(void);

extern int HttpCalibratePowerConsume(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf) ;

extern void SavePowerCoefficent(void);
extern void ReadPowerCoefficent(void);
//extern void GetValidData(void);


#if defined(__PROTECT_MOD_ENABLE__)
extern void OverCurrentProtectPoll(void);
#endif 

extern void CurrentProtect(void);

#if defined(__PWR_OFFSET_DBG__) && defined(__UDP_PRINT_DBG__) 
extern int HttpCalibratePwrOffset(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf);
#endif 

// 

#endif 
/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/

