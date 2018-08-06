/*
********************************************************************************
* @file    TcpSocket.c
* @author  
* @version V1.0.0
* @date    2015.03.09 
* @brief   
********************************************************************************
*/
#include <hsf.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ConfigAll.h" 
#include "HttpdCallBack.h" 
#include "DevLib.h" 
#include "TcpSocket.h"

#define HTTP_SERVER_RECV_LENGTH  1400 

char HttpServerRecvBuf[HTTP_SERVER_RECV_LENGTH+1] = {0}; 

USER_FUNC static void HttpdServer_ThreadFunc(void* arg);

struct sockaddr_in  HttpClientAddr;
/*******************************************************************************
* Description: 
* Input      : 
* Output     : 
*******************************************************************************/ 
extern void HttpdServerThread_Create(void) 
{
    hfthread_create(HttpdServer_ThreadFunc, "HttpServer", 512, (void*)1, HFTHREAD_PRIORITIES_MID, NULL, NULL); 
}
/*******************************************************************************
* 功能描述: 
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/ 
USER_FUNC static void HttpdServer_ThreadFunc(void* arg)
{ 
	int                 recv_num = 0; 
	int                 iTmp=1, AddrSize, iListenHttpFD = -1; 
    int                 iClientHttpFD = -1;
    struct sockaddr_in  ServerAddr; 

	iListenHttpFD = socket(AF_INET, SOCK_STREAM, 0); 
	if( iListenHttpFD < 0 ) 
	{ 
		//u_printf("create Tcp socket fail\n"); 
	} 

	memset((char*)&ServerAddr,0, sizeof(ServerAddr));
	
    ServerAddr.sin_family       = AF_INET;
    ServerAddr.sin_port         = htons(80);
    ServerAddr.sin_addr.s_addr  = htonl(INADDR_ANY);
    
	iTmp = 1; 
	if(setsockopt(iListenHttpFD, SOL_SOCKET, SO_KEEPALIVE, &iTmp, sizeof(iTmp))<0)
	{
		//u_printf("set SO_KEEPALIVE fail\n");
	}
	iTmp = 60; 
	if(setsockopt(iListenHttpFD, IPPROTO_TCP, TCP_KEEPIDLE, &iTmp, sizeof(iTmp))<0)
	{
		//u_printf("set TCP_KEEPIDLE fail\n");
	}
	iTmp = 6;
	if(setsockopt(iListenHttpFD, IPPROTO_TCP, TCP_KEEPINTVL, &iTmp, sizeof(iTmp))<0)
	{
		//u_printf("set TCP_KEEPINTVL fail\n");
	}
	iTmp = 5;
	if(setsockopt(iListenHttpFD, IPPROTO_TCP, TCP_KEEPCNT, &iTmp, sizeof(iTmp))<0)
	{
		//u_printf("set TCP_KEEPCNT fail\n");
	}
	// 
	if(bind(iListenHttpFD, (struct sockaddr*)&ServerAddr, sizeof(ServerAddr)) < 0)
	{
		//u_printf("bind Tcp socket error\n"); 
	}
    if(listen(iListenHttpFD, 5) < 0)
    {
		//u_printf("Listen Tcp socket error\n");
    }
    HttpServerRecvBuf[HTTP_SERVER_RECV_LENGTH] = '\0'; 
    AddrSize = sizeof(struct sockaddr_in);

	for(;;) 
	{ 
        iClientHttpFD = accept(iListenHttpFD, (struct sockaddr *)&HttpClientAddr, (unsigned int*)&AddrSize); 
        if(-1 == iClientHttpFD) 
		{ 
            continue;
		} 
        //u_printf("++++++++++++++++++++++++++++++++++++++++++++++\r\n");

   
    	recv_num = recv(iClientHttpFD, HttpServerRecvBuf, HTTP_SERVER_RECV_LENGTH, 0);

    	if((recv_num)>0) 
    	{ 
    	    HttpServerRecvBuf[recv_num] = '\0'; 
    	  #if 1
    	    if(0x00 == hfhttpd_url_callback(HttpServerRecvBuf, HttpServerRecvBuf))
    	    {
	            send(iClientHttpFD, HttpServerRecvBuf, strlen(HttpServerRecvBuf), 0); 
	        }
	      #endif
    	} 

        close(iClientHttpFD);
   
	} 


}

/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/

