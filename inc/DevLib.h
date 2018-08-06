/*
********************************************************************************
* @file    : DevLib.h 
* @author  : 
* @version : V1.0.0
* @date    : 2014/06/30
* @brief   : 
********************************************************************************
*/
#ifndef   __SENSOR_LIB_H__ 
#define   __SENSOR_LIB_H__ 

#include "stdint.h" 
#include "DevDataType.h "
#include "ConfigAll.h" 
#include <time.h>


#define     Dim(arr)                (sizeof(arr)/sizeof(arr[0])) 

// λ������ 
#define     OrBit(DstData,SrcData)  DstData |= (SrcData) 
#define     SetBit(Data,nBit)		Data |= (1<<(nBit)) 
#define     ClrBit(Data,nBit)		Data &= (~(1<<(nBit))) 
#define     isBitSet(Data,nBit)		((Data) & (1<<(nBit))) 

// ��������ĺ� 
#define     MACRO_(xMacro)          #xMacro 
#define     SHOW_MACOR(iMacro)      MACRO_(iMacro)                          // ��������ĺ� 

// 
typedef enum {
    FAILED = 1,
    PASSED = 0
} StatusType;                                                               // 
typedef struct DevRuleRet{
    StatusType  RuleAnalyStatus;
    char        *pNextRule;
}AnalyRuleRetType;


	


// 
extern char     SetCurTime(char *pBuf);                                     // ���õ�ǰʱ�� 
extern void     SetCurTimeFromSec(time_t timeStack);                           // ͨ��ʱ������õ�ǰʱ��

extern void     DecodeUrl(char *pBuf);                                      // ����URL
extern uint8_t  ParseRule(char *pArgBuf);                                   // ��������
extern uint8_t  Increment(uint32_t ruleID,SwitchRuleType tempRule);
extern uint8_t  DeleteRule(uint32_t ruleID);                                    // ɾ������
extern uint8_t  ReFreshRule(uint32_t ruleID,SwitchRuleType tempRule);           // �༭����
extern void PrintRule(void);                                                // ��ӡ������Ϣ


extern uint32_t AnalyTime(char *pBuf);                                          

extern uint8_t  CaculateUartDataCheckSum(uint8_t *pBuf, uint8_t iLen);      // 

extern char     RebootSyncDev(char *pBufData,int *cmd);
extern uint8_t  SetUpdateUrl(char *pDataBuf);

extern void     Reboot(int iRebootCode);

extern void     DevGPIO_Init(void);
extern void     delay_us(int us);
extern int      GetDaysInMonth(int y, int m);

#if defined(__UDP_PRINT_DBG__) 
extern void     UdpDebugPrint(char *pArgBuf, int ArgLength); 
#endif 

//

// 
#endif 

/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/
