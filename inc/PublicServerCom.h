/*********************************************************************************
* @file    : PublicServerCom.h
* @author  :
* @version : V1.0.0
* @date    : 2014/06/24 
* @brief   : 
* @Modify  : 
********************************************************************************/
#ifndef   __PUBLIC_SERVER_COMMUNICATE_H__ 
#define   __PUBLIC_SERVER_COMMUNICATE_H__  

#include "hsf.h"
#include "RtTime.h"



extern uint8_t hasSynInfo;														//同步数据是否成功
extern uint16_t heartBeatCount;         //发送心跳包次数
extern uint8_t reportRTHourmark;                                   //上报小时实时功耗标志
extern uint8_t resetWifiMark;				//重启路由器标志


extern void sendBroadcastinfo(void);


extern void PublicServer_SyncBaseInfoWithServer(void);
extern void PublicServer_SyncAllWithServer(void);
extern void PublicServer_SyncCostWithServer(void);
extern void Reboot_Control(void);


extern void PublicServer_Init(void);

#endif 
/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/

