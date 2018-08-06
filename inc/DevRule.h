/*
********************************************************************************
* @file    : DevRule.h 
* @author  : 
* @version : V1.0.0 
* @date    : 2014/06/24 
* @brief   : 
********************************************************************************
*/
#ifndef  __DEVICE_RULE_H__  
#define  __DEVICE_RULE_H__  
// 
#include <hsf.h>
#include "DevDataType.h "
// 
extern uint8_t SwOnOffData;
extern uint8_t portNum;
extern uint8_t LedFlashControl[6];
extern RelatedCtrlType	RelatedCtrl;			//关联控制机构体


extern SwInformationType    SwInformation;


extern char *GetDevRuleInfo(char *pDataBuf,uint8_t pageID); 
extern char *GetCountDownInfo(char *pArgBuf);

extern uint8_t ModifyDevName(char *pCharArg); 
extern uint8_t Switch_Ctl(char *pBuf);
extern uint8_t Switch_RelatedCtl(char *pBuf);
extern void Switch_delayCtl(uint8_t ctlStates);
extern void Switch_TurnOnOff(uint8_t Port, uint8_t state);

extern void CountDownProcess(time_t ArgCurTime);

extern void SetPortState(uint8_t portID,uint8_t onOffState);
//extern uint8_t GetPortState(uint8_t port);
extern void Port_TurnOnOff( uint8_t state);




#endif 
/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/

