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



extern uint8_t hasSynInfo;														//ͬ�������Ƿ�ɹ�
extern uint16_t heartBeatCount;         //��������������
extern uint8_t reportRTHourmark;                                   //�ϱ�Сʱʵʱ���ı�־
extern uint8_t resetWifiMark;				//����·������־


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

