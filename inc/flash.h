/*
********************************************************************************
* @file    : Flash.h
* @author  : 
* @version : V1.0.0
* @date    : 2014/06/24 
* @brief   : 
********************************************************************************
*/
#ifndef  __DEVICE_FLASH_H__  
#define  __DEVICE_FLASH_H__  
// 
#include "hsf.h" 
#include "stdint.h" 

#include "ConfigAll.h" 
// 
#define     AP_MODE_LED                                     0x01 
#define     CLIENT_MODE_LED                                 0x02
#define 	PASSWD_ERROR									0x03
#define 	SMTLINK_MODE_LED								0x04

#if  defined(__HF_BOARD__)
#define     BOARD_HF_KEY_PIN                                HFGPIO_F_SLEEP_RQ
#else 
#define     BOARD_HF_KEY_PIN                                HFGPIO_SW_KEY_CTL_PIN
#endif



// 设备注册信息保存
#define     FLASH_DEV_BIND_BUF_START                        0
#define     FLASH_REGID_FLAG_OFFSET                         4 
#define     FLASH_REGID_FLAG                                0xA55AA55A 
#define     FLASH_REGID_LENGTH                              NAME_LENGTH

//Bind device information
#define     FLASH_DEV_BIND_FLAG_OFFSET                      40
#define     FLASH_DEV_BIND_FLAG                             0X15151515
#define     FLASH_DEV_BIND_PORT_OFFSET                      46 
#define     FLASH_DEV_BIND_URL_OFFSET                       50


// Power Calibration 功率校准系数 
#define     FLASH_POWER_CALIBRATION_COEFFICIENT_BUF_START   (1*HFFLASH_PAGE_SIZE)
#define     FLASH_POWER_COEFFICENT_FLAG_OFFSET              0x00 
#define     FLASH_POWER_COEFFICENT_FLAG_LENGTH              0x04 
#define     FLASH_POWER_COEFFICENT_FLAG                     0x55AA55AA 

#define     FLASH_POWER_CURRENT_OFFSET                      0x04 
#define     FLASH_POWER_CURRENT_LENGTH                      0x04 

#define     FLASH_POWER_VOLT_COEFFICIENT_OFFSET             0x08 
#define     FLASH_POWER_VOLT_COEFFICIENT_LENGTH             0x04 

#define     FLASH_POWER_POWER_COEFFICIENT_OFFSET            0x0C 
#define     FLASH_POWER_POWER_COEFFICIENT_LENGTH            0x04 

// sn , sak , mac 
#define     FLASH_DEV_HARD_INFO_BUF_START                   (2*HFFLASH_PAGE_SIZE) 
#define     FLASH_DEV_FLAG_OFFSET                           0x00 
#define     FLASH_DEV_FLAG                                  0x55AA55AA 
#define     FLASH_DEV_LENGTH                                0x04 
#define     FLASH_SAK_OFFSET                                0x08 
//#define     FLASH_SAK_LENGTH                                0x10 

#define     FLASH_SN_FLAG_OFFSET                            24 
#define     FLASH_SN_FLAG                                   0x5A5A5A5A 
#define     FLASH_SN_FLAG_LENGTH                            4 
#define     FLASH_SN_LENGTH                                 16 

#define     FLASH_FAC_PROGRAM_FLAG_OFFSET                   18 
#define     FLASH_FAC_PROGRAM_FLAG                          0x45454545 



// Dev rule 
#define     FLASH_DEV_RULE_INFO_BUF_START                   (3*HFFLASH_PAGE_SIZE)
#define     FLASH_DEV_RULE_FLAG_OFFSET                      0x00 
#define     FLASH_DEV_RULE_LENGTH                           (2*HFFLASH_PAGE_SIZE)
#define     FLASH_DEV_RULE_FLAG                             0x33553355

// Dev status
#define     FLASH_DEV_STATUS_INFO_BUF_START                 (5*HFFLASH_PAGE_SIZE)
#define     FLASH_DEV_STATUS_FLAG_OFFSET                    0x00 
#define     FLASH_DEV_STATUS_FLAG                           0x35353535

// Power 历史功耗统计数据 
#define     FLASH_DEV_POWER_STATISTICS_BUF_START            (6*HFFLASH_PAGE_SIZE)
#define     FLASH_DEV_POWER_STATISTICS_FLAG_OFFSET          0x00 
#define     FLASH_DEV_POWER_STATISTICS_FLAG                 0x53535353
#define     FLASH_DEV_POWER_STATISTICS_LENGTH               (2*HFFLASH_PAGE_SIZE)

// DevName
#define     FLASH_DEV_NAME_BUF_START                        (8*HFFLASH_PAGE_SIZE)
#define     FLASH_DEV_NAME_FLAG_OFFSET                      0x00 
#define     FLASH_DEV_NAME_FLAG                             0x12121212
#define     FLASH_DEV_NAME_OFFSET                           0x04 




// Device Time Zone 
#define     FLASH_DEV_TIME_ZONE_BUF_START                   (10*HFFLASH_PAGE_SIZE)
#define     FLASH_DEV_TIME_ZONE_FLAG_OFFSET                 0x00 
#define     FLASH_DEV_TIME_ZONE_FLAG                        0x13131313
#define     FLASH_DEV_TIME_ZONE_OFFSET                      0x04 

// 设备功率偏移校准 
#define     FLASH_PWR_CALIBRATE_OFFSET_BUF_START            (11*HFFLASH_PAGE_SIZE)
#define     FLASH_PWR_CALIBRATE_FLAG_OFFSET                 0x00 
#define     FLASH_PWR_CALIBRATE_FLAG                        0x31313131 
#define     FLASH_PWR_CALIBRATE_OFFSET                      0x04 


#define     FLASH_DEV_SECURITY_INFO_BUF_START            	(12*HFFLASH_PAGE_SIZE)
#define     FLASH_DEV_SECURITY_INFO_FLAG_OFFSET             0x00 
#define     FLASH_DEV_SECURITY_INFO_FLAG                    0x43434343 






//extern uint8_t getWifiStatus(void);
//extern int SetWifiClientMode(void);
//extern uint8_t ReadDevRegidID(void);
//extern void WriteDevRegidIDInfo(void);
extern void ResetWifiMode(void);
extern void DevFlash_init(void); 
extern int  FlashBackupCpy(uint32_t dstaddr, uint32_t srcaddr, uint32_t len); 
extern void DevMode_Init(void); 
extern void CheckDevMode(void);
extern void FlashResetFactory(void); 
extern void EnterAPSTAMode(void); 
extern void WriteDevSakSN(void); 
extern void ReadDevSakSN(void); 
extern void WriteDevBindInfo(void);

extern void SaveDevRulePoll(void); 
extern void ReadDevRule(void); 


extern void ReadDevStatus(void); 
extern void SaveDevStatusPoll(void); 
extern void CheckSwState(void);

extern void ReadDevName(void);
extern void WriteDevName(void);

extern void ClearDevSmartLinkInfo(void);
extern void SetDevFactoryOpr(void);

extern void WriteDevTimeZone(void);
extern void ReadDevTimeZone(void);
// 
#endif

/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/

