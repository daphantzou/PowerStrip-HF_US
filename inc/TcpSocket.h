/*
********************************************************************************
* @file    : TcpSocket.h
* @author  :
* @version : V1.0.0
* @date    : 2015/03/09 
* @brief   : 
* @Modify  : 
********************************************************************************
*/
#ifndef   __TCP_SERVER_SOCKET_H__ 
#define   __TCP_SERVER_SOCKET_H__ 
// 
#include "ConfigAll.h" 
// 

#if defined(__BSD_SOCKET_HTTP_SERVER_ENABLE__) 
#include "hsf.h"

#define HTTP_SERVER_ACK_LENGTH   1400 
// 
extern void HttpdServerThread_Create(void); 
// 
#endif 
//
#endif 
/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/

