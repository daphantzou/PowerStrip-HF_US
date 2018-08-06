/*
********************************************************************************
* @file    remote_update.c
* @author  
* @version V1.0.0
* @date    2014/06/24
* @brief   
********************************************************************************
*/
#include <hsf.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 
#include <httpc/httpc.h> 
#include <md5.h> 

#include "DevLib.h"
// 

/******************************************************************************* 
* 功能描述: 通过URL远程升级App Firmware. 
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/ 
#define     UPDATE_PACKET_LENGTH            512 
#define     UPDATE_URL_LENGTH               256 
extern int update_as_http(char *purl)
{ 
	httpc_req_t         http_req; 
	char                *content_data=NULL; 
	char                *temp_buf=NULL; 
	parsed_url_t        url   = {0}; 
	http_session_t      hhttp = 0; 
	int                 total_size, read_size=0, rv=0; 
	tls_init_config_t   *tls_cfg  = NULL; 
	char                *test_url = purl; 
	struct MD5Context   md5_ctx; 
	uint8_t             digest[16]={0}; 
    #if defined(__URL_UPDATE_DBG__) && defined(__UDP_PRINT_DBG__) 
    char                HttpReaMsg[100];
    int                 iLength;
    #endif 
	
	// 
    #if defined(__URL_UPDATE_DBG__) && defined(__UDP_PRINT_DBG__) 
    UdpDebugPrint("Start Url Update\r\n", sizeof("Start Url Update\r\n")-1);
    UdpDebugPrint(purl, strlen(purl)); 
    #endif 
	memset(&http_req, '\0', sizeof(http_req));
	http_req.type    = HTTP_GET;
	http_req.version = HTTP_VER_1_1;
	
	if((temp_buf = (char*)hfmem_malloc(UPDATE_URL_LENGTH))==NULL)
	{
        #if defined(__URL_UPDATE_DBG__) && defined(__UDP_PRINT_DBG__) 
        UdpDebugPrint("Http hfmem_malloc fail\r\n", sizeof("Http hfmem_malloc fail\r\n")-1);
        #endif 
		rv = -HF_E_NOMEM;
		goto exit;
	}
	memset(temp_buf, '\0', UPDATE_URL_LENGTH);
	if((rv = hfhttp_parse_URL(test_url, temp_buf, UPDATE_URL_LENGTH, &url))!=HF_SUCCESS)
	{
        #if defined(__URL_UPDATE_DBG__) && defined(__UDP_PRINT_DBG__) 
        UdpDebugPrint("Http hfhttp_parse_URL fail\r\n", sizeof("Http hfhttp_parse_URL fail\r\n")-1);
        #endif 
		goto exit;
	}
	if((rv = hfhttp_open_session(&hhttp, test_url, 0, tls_cfg, 3))!=HF_SUCCESS)
	{
        #if defined(__URL_UPDATE_DBG__) && defined(__UDP_PRINT_DBG__) 
        UdpDebugPrint("Http hfhttp_open_session fail\r\n", sizeof("Http hfhttp_open_session fail\r\n")-1);
        #endif 
		goto exit;
	}
	hfsys_disable_all_soft_watchdogs();
	
	http_req.resource = url.resource;
	hfhttp_prepare_req(hhttp, &http_req, HDR_ADD_CONN_CLOSE);
	hfhttp_add_header(hhttp, "Range", "bytes=0");
	if((rv = hfhttp_send_request(hhttp, &http_req))!=HF_SUCCESS)
	{
        #if defined(__URL_UPDATE_DBG__) && defined(__UDP_PRINT_DBG__) 
        UdpDebugPrint("Http hfhttp_send_request fail\r\n", sizeof("Http hfhttp_send_request fail\r\n")-1);
        #endif 
		goto exit;
	}
	content_data = (char*)hfmem_malloc(UPDATE_PACKET_LENGTH);
	if(content_data == NULL)
	{
        #if defined(__URL_UPDATE_DBG__) && defined(__UDP_PRINT_DBG__) 
        UdpDebugPrint("Http content_data fail\r\n", sizeof("Http content_data fail\r\n")-1);
        #endif 
		rv = -HF_E_NOMEM;
		goto exit;
	}
	
    #if defined(__URL_UPDATE_DBG__) && defined(__UDP_PRINT_DBG__) 
    UdpDebugPrint("Http Update\r\n", sizeof("Http Update\r\n")-1);
    #endif 
    
	total_size = 0;
	memset(content_data, '\0', UPDATE_PACKET_LENGTH);
	hfupdate_start(HFUPDATE_SW);
	MD5Init(&md5_ctx);
	while((read_size = hfhttp_read_content(hhttp, content_data, UPDATE_PACKET_LENGTH))>0)
	{ 
		hfupdate_write_file(HFUPDATE_SW, total_size, content_data, read_size);
		MD5Update(&md5_ctx, (uint8_t*)content_data, read_size);
		total_size += read_size;
	} 
    MD5Final(digest, &md5_ctx);
    
    #if defined(__URL_UPDATE_DBG__) && defined(__UDP_PRINT_DBG__) 
    iLength = sprintf(HttpReaMsg, "\r\nUpdate total size=%08x, read_size=%08x\r\n", 
                                    total_size, read_size); 
    UdpDebugPrint(HttpReaMsg, iLength); 
    #endif 
	if(hfupdate_complete(HFUPDATE_SW, total_size) == HF_SUCCESS)
	{ 
        #if defined(__URL_UPDATE_DBG__) && defined(__UDP_PRINT_DBG__) 
        UdpDebugPrint("hfupdate_complete success ^_^\r\n", sizeof("hfupdate_complete success ^_^\r\n")-1);
        #endif 

        Reboot(0);
	} 
exit:
	if(temp_buf != NULL) 
	{ 
		hfmem_free(temp_buf);
	} 
	if(content_data != NULL)
	{ 
		hfmem_free(content_data);
	} 
	if(hhttp != 0)
	{ 
		hfhttp_close_session(&hhttp); 
	} 
	hfsys_enable_all_soft_watchdogs();
	return rv;
}
/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/

