/*
********************************************************************************
* @file    : UdpSocket.h
* @author  :
* @version : V1.0.0
* @date    : 2014/06/24 
* @brief   : 
* @Modify  : 
********************************************************************************
*/
#ifndef   __UDP_SERVER_SOCKET_H__ 
#define   __UDP_SERVER_SOCKET_H__ 
// 
#include "ConfigAll.h" 
// 

#if defined(__BSD_SOCKET_UDP_SERVER_ENABLE__) 
#include "hsf.h"

// 
extern struct sockaddr_in  RecvAddr;

extern void UdpServerThread_Create(void); 

extern int  UdpServerThread_GetSocketFd(void); 

extern void UdpServer_SendData(char *pArgBuf, int ArgLength);
// 
#endif 
//
#endif 
/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/

