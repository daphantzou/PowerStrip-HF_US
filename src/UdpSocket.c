/*
********************************************************************************
* @file    UdpSocket.c
* @author  
* @version V1.0.0
* @date    2015.01.14 
* @brief   
********************************************************************************
*/
#include <hsf.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "PublicServerCom.h" 
#include "ConfigAll.h" 
#include "UdpServerCallBack.h" 
#include "DevLib.h" 

// 
#if defined(__BSD_SOCKET_UDP_SERVER_ENABLE__) 
// 
#define UDP_SERVER_RECV_LENGTH  1400 

char UdpServerRecvBuf[UDP_SERVER_RECV_LENGTH+1] = {0};

USER_FUNC static void UdpServer_ThreadFunc(void* arg);

// 
int                 ufd = -1;
struct sockaddr_in  RecvAddr;
/*******************************************************************************
* Description: 
* Input      : 
* Output     : 
*******************************************************************************/ 
extern void UdpServerThread_Create(void) 
{
    ufd = -1;
    hfthread_create(UdpServer_ThreadFunc, "UdpServer", 512, (void*)1, HFTHREAD_PRIORITIES_LOW, NULL, NULL); 
}



extern int UdpServerThread_GetSocketFd(void)
{
    return ufd; 
}
/*******************************************************************************
* 功能描述: 发送UDP数据 
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/ 
extern void UdpServer_SendData(char *pArgBuf, int ArgLength)
{
    sendto(ufd, pArgBuf, ArgLength, 0, (struct sockaddr*)&RecvAddr, sizeof(struct sockaddr)); 
}
/*******************************************************************************
* 功能描述: 
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/ 
USER_FUNC extern void UdpServer_ThreadFunc(void* arg)
{ 
	int                 recv_num = 0, alen; 
	uint8_t             mac_addr[6]={0}; 
    struct sockaddr_in  ServerAddr;
    
   #if  defined(__SELECT_MECHANISM__)
    int				    ret;
	fd_set 			    rset;
	struct timeval 	    timeout;
   #endif 

    (void)arg;
    
UDP_start:
	ufd = socket(AF_INET, SOCK_DGRAM, 0);
	if(ufd<0)
	{
		//u_printf("create udp socket fail\n");
	}

	hfnet_set_udp_broadcast_port_valid(8888,8889);
	
	memset((char*)&ServerAddr,0, sizeof(ServerAddr));
	
    alen                        = sizeof(struct sockaddr_in);
    ServerAddr.sin_family       = AF_INET;
    ServerAddr.sin_port         = htons(8888);
    ServerAddr.sin_addr.s_addr  = htonl(INADDR_ANY);
    memcpy((char*)&RecvAddr, (char*)&ServerAddr, sizeof(ServerAddr));
    
	if(bind(ufd, (struct sockaddr*)&ServerAddr, sizeof(ServerAddr)) < 0)
	{
		//u_printf("bind udp socket error\n");
	}
    //UdpServerRecvBuf[UDP_SERVER_RECV_LENGTH] = '\0'; 
    memset(UdpServerRecvBuf,0,UDP_SERVER_RECV_LENGTH);
    
	for(;;)
	{ 
	   #if  defined(__SELECT_MECHANISM__)
		FD_ZERO(&rset); 
		FD_SET(ufd, &rset); 
		timeout.tv_sec  = 3; 
		timeout.tv_usec = 0; 
		ret = hfuart_select(ufd+1, &rset, NULL, NULL, &timeout); 
		if( ret<=0 )
		{
		    continue;
		}
	   #endif 
	   
    	recv_num = hfnet_recvfrom(ufd, UdpServerRecvBuf, UDP_SERVER_RECV_LENGTH, 0, 
    	                          (struct sockaddr*)&RecvAddr, (socklen_t*)&alen, (char*)mac_addr);

		//u_printf("Receive_bytes = %d\n",recv_num);
    	
    	if((recv_num)>0)
    	{ 
    	    UdpServerRecvBuf[recv_num] = '\0'; 
    	    UdpServer_RecvData( HFNET_SOCKETA_DATA_READY, 
    	                        UdpServerRecvBuf, 
    	                        recv_num,
    	                        UDP_SERVER_RECV_LENGTH);
    	} 
    	else
    	{
    	     //hfsys_reset();
    	     u_printf("Now the udp receive zreo bytes\n");
    	     close(ufd);
    	     
			 goto UDP_start;
    	}

		if(resetWifiMark)
    	{
    		 resetWifiMark = 0;
    		 
    		 close(ufd);   	     
			 goto UDP_start;
    	}
		//hf_thread_delay(500);
	} 
}
#endif 
/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/

