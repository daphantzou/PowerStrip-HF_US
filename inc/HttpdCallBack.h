/*
********************************************************************************
* @file    HttpdCallBack.h  
* @author  
* @version V1.0.0 
* @date    02/09/2013
* @brief   
* -��ע-: u_printf,sprintf����Ч�ʽϵ�,��Ϊ��ײ�ʵ��vsprintfЧ�ʵ� 
********************************************************************************
*/
#ifndef __RECV_NET_DATA_H__ 
#define __RECV_NET_DATA_H__ 
// 

#include "ConfigAll.h" 
// ����ָ������:����˵����icmd:�����  pArgRecvBuf:��������   pAckBuf:�ն˷��ص���������
typedef int (*pfHttpdServerCmdType)(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf); 

typedef struct HttpdServerReceive 
{ 
    int16_t                 iCmdField;                                          // Http����
    pfHttpdServerCmdType    pfHttpServerObj;                                    // Http������Ӧ����
}HttpdServeReceiveType; 

extern int     iLengthHttpAckBuf;
extern char    *pHttpAckBuf;

// ��������
extern int hfwifi_scan_callback(PWIFI_SCAN_RESULT_ITEM scan_ret);
extern int SetWifiMode(char *pBufData);
extern void AnalyHttpRecvData(void) ;                                           // ����Httpͨ�������� 

extern int  hfhttpd_url_callback(char* pDataBuf, char* pAckContent);            // socketa�ص� 

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

