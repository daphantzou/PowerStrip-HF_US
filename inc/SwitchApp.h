/*
********************************************************************************
* @file    SwitchApp.h
* @author  
* @version V1.0.0
* @date    11/20/2009
* @brief   
********************************************************************************
*/
#ifndef  __INT_APP_H__ 
#define  __INT_APP_H__ 

#include "ConfigAll.h" 
#include <time.h> 
#include "hsf.h" 
#include "DevInfo.h"



extern char switch_server_net[DOMAIN_MAX_LENGTH];
extern char dev_default_name[NAME_LENGTH];
extern uint8_t big_version;
extern uint8_t little_version;
extern uint8_t regStep;			//绑定阶段性标志
extern uint8_t	switchMark;					//切换主服务器端口标志
extern FwUpdateStatusType      FwUpdateStatusObj;
extern struct sockaddr_in	   MainSvrSockAddr; 								   // 主服务器sockaddr 
extern DevStatusType           DevStatusObj;
extern DevInfoType         	   DevInfoObj; 


// 
//USER_FUNC 
extern void app_thread_and_timer_start(void);
extern void TimelyQuery_isDevReg(void);                          // 
extern void KeyTimer_Init(void);
extern void ReadPowerFreq(void);
extern void ReadDevIP(void);
extern void ServerNetworkInit(void); 

#if  defined(__WATCH_DOG_ENABLE__)
extern void FeedWatchDog(void);
#endif 
//
#endif 
/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/

