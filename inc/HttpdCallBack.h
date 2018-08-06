/*
********************************************************************************
* @file    HttpdCallBack.h  
* @author  
* @version V1.0.0 
* @date    02/09/2013
* @brief   
* -备注-: u_printf,sprintf函数效率较低,因为其底层实现vsprintf效率低 
********************************************************************************
*/
#ifndef __RECV_NET_DATA_H__ 
#define __RECV_NET_DATA_H__ 
// 

#include "ConfigAll.h" 
// 函数指针声明:参数说明，icmd:命令号  pArgRecvBuf:命令内容   pAckBuf:终端返回的数据内容
typedef int (*pfHttpdServerCmdType)(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf); 

typedef struct HttpdServerReceive 
{ 
    int16_t                 iCmdField;                                          // Http命令
    pfHttpdServerCmdType    pfHttpServerObj;                                    // Http命令响应函数
}HttpdServeReceiveType; 

extern int     iLengthHttpAckBuf;
extern char    *pHttpAckBuf;

// 函数声明
extern int hfwifi_scan_callback(PWIFI_SCAN_RESULT_ITEM scan_ret);
extern int SetWifiMode(char *pBufData);
extern void AnalyHttpRecvData(void) ;                                           // 分析Http通道的数据 

extern int  hfhttpd_url_callback(char* pDataBuf, char* pAckContent);            // socketa回调 

extern void ModifyDevMac(void);
extern int SetClientKey(int Security, char *pKeyStr, char *pCmdSet);
extern char *GetDevRule(char *pDataBuf);
extern int GetCountDown(int ArgPort,char *pArgBuf);
extern int HttpGetDevTime(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf);

// 
#endif 
/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/

