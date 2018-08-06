/*
********************************************************************************
* @file    HttpdCallBack.c 
* @author  
* @version V1.0.0 
* @date    2013/06/21
* @brief   
* -备注-: u_printf,sprintf函数效率较低,因为其底层实现vsprintf效率低 
********************************************************************************
*/
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <hsf.h>

#include "ConfigAll.h"

#include "cJSON.h"
#include "DevInfo.h"
#include "DevLib.h"
#include "DevRule.h"
#include "DevDataType.h"

#include "HttpdCallBack.h" 
#include "remote_update.h" 
#include "RtTime.h" 
#include "rn8209.h"
#include "flash.h"
#include "PowerStaticsRecord.h"
#include "SwitchApp.h" 
#include "TcpSocket.h" 
#include "UdpServerCallBack.h" 

// 
#define         SWITCH_GET_SECOND_HTTP_888_CMD                          888         // 
#define         SWITCH_CALIBRATE_POWER_OFFSET_HTTP_777_CMD              777         // 
// 接收的控制命令 
#define         SWITCH_SET_WIFI_MODE_HTTP_1_CMD                         1           // 设置WIFI的工作模式 
#define         SWITCH_SCAN_NEARBY_WIFI_HTTP_2_CMD                      2           // 获取周边AP信号 
#define         SWITCH_GET_WIFI_STATE_HTTP_3_CMD                        3           // 获取当前wifi状态
#define         SWITCH_GET_DEVICE_REGID_HTTP_5_CMD                      5           // 设置设备和账号绑定
#define         SWITCH_RESTART_DEVICE_HTTP_120_CMD                      120         // 设备重启或恢复出厂设置


#define         SWITCH_MODIFY_STATIC_IP_HTTP_121_CMD                    121         // 设置设备的ip地址 
#define         SWITCH_MODIFY_PRODUCT_SN_HTTP_122_CMD                   122         // 设置设备sn
#define         SWITCH_CALIBRATE_POWER_CONSUME_HTTP_125_CMD             125         // 排插/插座校准
#define         SWITCH_ON_OFF_DEV_CTL_HTTP_200_CMD                      200         // 控制排插开关
#define         SWITCH_GET_DEV_TIME_HTTP_502_CMD                        502         // 获取设备时间
#define         SWITCH_GET_REAL_TIME_CONSUME_POWER_HTTP_511_CMD         511         // 获取设备实时数据
#define         SWITCH_GET_DEV_RULE_CONFIG_HTTP_514_CMD                 514         // 获取设备规则
#define         SWITCH_GET_DEV_COUNT_DOWN_HTTP_515_CMD                  515         // 获取设备倒计时





// 外部变量声明 
extern DevInfoType      DevInfoObj;                                                 // 设备信息 

// 函数声明 
static int HttpGetDevRegIDState(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf);
static int HttpModifyProductSn(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf);
static int HttpSetWifiMode(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf);
static int HttpScanNearbyWifiSignal(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf);
static int HttpGetDevWifiState(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf);
static int HttpSetStaticIp(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf);
static int HttpSwitch_OnOffCtl(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf);
static int HttpGetRealTimeConsumePower(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf);
static int HttpRestartMachine(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf);
static int HttpGetRule(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf);
static int HttpGetDevCountDown(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf);





// 发送的命令 
const HttpdServeReceiveType HttpdServerArr[] = 
{   //命令                                              ,   // 响应函数 
    { SWITCH_SET_WIFI_MODE_HTTP_1_CMD                   ,   HttpSetWifiMode             },      // 设置WIFI的工作模式
    { SWITCH_SCAN_NEARBY_WIFI_HTTP_2_CMD                ,   HttpScanNearbyWifiSignal    },      // 获取周边AP信号 
    { SWITCH_GET_WIFI_STATE_HTTP_3_CMD                  ,   HttpGetDevWifiState         },      // 获取当前wifi状态
    { SWITCH_GET_DEVICE_REGID_HTTP_5_CMD                ,   HttpGetDevRegIDState        },
    { SWITCH_RESTART_DEVICE_HTTP_120_CMD                ,   HttpRestartMachine          }, 
    { SWITCH_MODIFY_STATIC_IP_HTTP_121_CMD              ,   HttpSetStaticIp             },      // 设置设备的ip地址 
    { SWITCH_MODIFY_PRODUCT_SN_HTTP_122_CMD             ,   HttpModifyProductSn         },      // 设置设备sn
    { SWITCH_CALIBRATE_POWER_CONSUME_HTTP_125_CMD       ,   HttpCalibratePowerConsume   },      // 排插/插座校准
    { SWITCH_ON_OFF_DEV_CTL_HTTP_200_CMD                ,   HttpSwitch_OnOffCtl         },    	//OK 
    { SWITCH_GET_DEV_TIME_HTTP_502_CMD                  ,   HttpGetDevTime              }, 
    { SWITCH_GET_REAL_TIME_CONSUME_POWER_HTTP_511_CMD   ,   HttpGetRealTimeConsumePower }, 
    { SWITCH_GET_DEV_RULE_CONFIG_HTTP_514_CMD           ,   HttpGetRule                 }, 
    { SWITCH_GET_DEV_COUNT_DOWN_HTTP_515_CMD            ,   HttpGetDevCountDown         },

    #if defined(__PWR_OFFSET_DBG__) && defined(__UDP_PRINT_DBG__) 
    { SWITCH_CALIBRATE_POWER_OFFSET_HTTP_777_CMD        ,   HttpCalibratePwrOffset      }, 
    #endif 
    
    #if  defined(__DS1302_RUN_STATUS_DBG__)
    { SWITCH_GET_SECOND_HTTP_888_CMD                    ,   HttpGetSencond              }, 
    #endif 
    
}; 

extern uint8_t isGetRegid;                      //是否获得了用户账号标志位
extern DevTimeType          	DevTimeObj;
extern SwInformationType       	SwInformation;
extern DevRegInfoType           DevRegInformation;                                      //设备注册信息
//extern char    regidName[NAME_LENGTH];             //手机发送的用户账号名


char    *pHttpAckBuf;
int     iLengthHttpAckBuf;

char    MacChgFlag = 0;

char    HttpPacketHead[sizeof("HTTP/1.1 200 OK\r\nContent-Length:0000\r\nContent-Type:text/plain\r\n\r\n")+1] = {0}; 
/*******************************************************************************
* 功能描述: 
* 输入参数: 
* 输出参数: 
* -返回值-: 
* 使用说明: 
*******************************************************************************/
void HttpInit(void)
{
    MacChgFlag = 0;
}
/******************************************************************************* 
* 功能描述: 系统提供的钩子函数，数据已经保存在pDataBuf和pAckContent中
* 输入参数: 
* 输出参数: 
* -返回值-: 
*           0  : 已经处理,通知系统无需再处理 
*           -1 : 未处理,系统需处理 
* 使用说明: 
* 调用方法: 
* pDataBuf数据格式:get /cmd=..........
*******************************************************************************/ 
extern int hfhttpd_url_callback(char* pDataBuf, char* pAckContent)
{ 
    int     cmd, iRet, iLength, iLenOffSet;
    char    *pDataFiled, *pCmdStart, *pTmp, *pAckTmp;
    char    iServerArrCount, i; 
    char    TempArr[4+1]  = {0};
    // 
	iRet = -1; 
    
    pCmdStart       = pDataBuf;
    pCmdStart       = strstr(pCmdStart, "cmd=");
    if(NULL == pCmdStart)
    {
        return iRet;
    }
    DecodeUrl(pDataBuf);
    
    #if defined(__HTTP_SET_DBG__) && defined(__UDP_PRINT_DBG__) 
    UdpDebugPrint("HttpRecv:\r\n", sizeof("HttpRecv:\r\n")-1);
    UdpDebugPrint(pDataBuf, strlen(pDataBuf));
    UdpDebugPrint("\r\n", sizeof("\r\n")-1);
    #endif 
    
    //u_printf("\r\nHttp Recv:%s\r\n", pDataBuf );
    
    pCmdStart      += (sizeof("cmd=")-1);
    cmd             = atoi(pCmdStart);
    iServerArrCount = Dim(HttpdServerArr);
    for(i=0; i<iServerArrCount; i++)
    {
        if(cmd == HttpdServerArr[i].iCmdField)
        {
            if(NULL != HttpdServerArr[i].pfHttpServerObj)
            {
                if((3 != cmd) && (2 != cmd))
                {
                    pDataFiled = strstr(pCmdStart, "json=");
                    if(pDataFiled)
                    {
                        pDataFiled += (sizeof("json=")-1);
                    }
                    else
                    {
                        iRet = 0;
                        break;
                    }
                } 
                #if  1 
                pTmp    = HttpPacketHead; 
                
                iLenOffSet = sizeof("HTTP/1.1 200 OK\r\nContent-Length:0000\r\nContent-Type:text/plain\r\n\r\n") - 1; 
	            memcpy(HttpPacketHead, "HTTP/1.1 200 OK\r\nContent-Length:0000\r\nContent-Type:text/plain\r\n\r\n",iLenOffSet); 
	            
                // 准备载荷数据空间起始地址 
                pAckTmp = pAckContent + iLenOffSet; 
            //函数指针数组的入口地址，参数由http下发解码所得
                iLength = HttpdServerArr[i].pfHttpServerObj(HttpdServerArr[i].iCmdField, pDataFiled, pAckTmp); 
                *(pAckTmp+iLength) = '\0'; 
                
                pTmp   += sizeof ("HTTP/1.1 200 OK\r\nContent-Length:") - 1; 
                sprintf(TempArr, "%04d", iLength); 
                memcpy(pTmp, TempArr, 4); 
                
                memcpy(pAckContent, HttpPacketHead, iLenOffSet); 
                #else 
                HttpdServerArr[i].pfHttpServerObj(HttpdServerArr[i].iCmdField, pDataFiled, pAckContent);
                #endif 
            }
            iRet = 0;
            break;
        }
    }
    return iRet;
}

/*******************************************************************************
* 功能描述: 设置wifi模式
* 输入参数: 
* 输出参数: 
* -返回值-: 
* 使用说明: 
*******************************************************************************/
#define     SECURITY_NONE                   0x00 
#define     SECURITY_WEP                    0x01 
#define     SECURITY_WPA_TKIP               0x02 
#define     SECURITY_WPA2_TKIP              0x03 
#define     SECURITY_WPA_AES                0x04 
#define     SECURITY_WPA2_AES               0x05 
#define     SECURITY_WPA_TKIP_AES           0x06 
#define     SECURITY_WPA2_TKIP_AES          0x07 
#define     SECURITY_WPA_WPA2_TKIP          0x08 
#define     SECURITY_WPA_WPA2_AES           0x09 
#define     SECURITY_WPA_WPA2_TKIP_AES      0x0A 

int SetWifiMode(char *pBufData)
{
	cJSON   *json;
	int     mode,security;
	int     iLength, iAck;
	char    *ssid, *password, AtCmdArr[100+1]={0}, iRet;
	char    AtAckMsg[AT_ACK_MSG_LENGTH+1];
	
	iRet = 1; 
	//u_printf("Set Wifi Mode:%s\r\n\r\n", pBufData);
	json = cJSON_Parse(pBufData);
	if (json) 
	{
		mode     = cJSON_GetObjectItem(json,"mode")->valueint;
		ssid     = cJSON_GetObjectItem(json,"ssid")->valuestring;
		security = cJSON_GetObjectItem(json,"security")->valueint;

        memset(AtCmdArr, '\0', 101);
        if(0x00 == mode)
        {
            if(SECURITY_NONE == security) 
            { 
		        iLength  = sprintf(AtCmdArr, "AT+WAKEY=OPEN,NONE\r\n"); 
            } 
            else 
            { 
		        password = cJSON_GetObjectItem(json, "password")->valuestring; 
		        if(strlen(password)<8)
		        {
                    iRet = 1;
                    goto SetWifiModeExit;
		        }
		        iLength  = sprintf(AtCmdArr, "AT+WAKEY=WPA2PSK,AES,%s\r\n", password); 
            }
            iAck    = hfat_send_cmd(AtCmdArr, iLength, AtAckMsg, AT_ACK_MSG_LENGTH);
            if(HF_SUCCESS != iAck)
            {
                iRet = 1;
                goto SetWifiModeExit;
            }
            memset(AtCmdArr, '\0', iLength);
		    iLength = sprintf(AtCmdArr, "AT+WAP=%s\r\n", ssid); 
            iAck    = hfat_send_cmd(AtCmdArr, iLength, AtAckMsg, AT_ACK_MSG_LENGTH);
            if(HF_SUCCESS != iAck)
            {
                iRet = 1;
                goto SetWifiModeExit;
            }
            memset(AtCmdArr, '\0', iLength);
            iAck    = hfat_send_cmd("AT+WMODE=AP\r\n", sizeof("AT+WMODE=AP\r\n")-1, AtAckMsg, AT_ACK_MSG_LENGTH);
            if(HF_SUCCESS != iAck)
            {
                iRet = 1;
                goto SetWifiModeExit;
            }
        }
        else 
        {
            if(0x00 == security) 
            { 
		        iLength  = sprintf(AtCmdArr, "AT+WSKEY=OPEN,NONE\r\n"); 
            } 
            else 
            { 
		        password = cJSON_GetObjectItem(json, "password")->valuestring; 
		        iLength  = SetClientKey(security, password, AtCmdArr);
		        if(0x00 == iLength)
		        {
                    iRet = 1;
                    goto SetWifiModeExit;
		        }
            } 
            iAck    = hfat_send_cmd(AtCmdArr, iLength, AtAckMsg, AT_ACK_MSG_LENGTH);
            if(HF_SUCCESS != iAck)
            {
                iRet = 1;
                goto SetWifiModeExit;
            }
            memset(AtCmdArr, '\0', iLength);
            // 
            iAck    = hfat_send_cmd("AT+WMODE=STA\r\n", sizeof("AT+WMODE=STA\r\n")-1, AtAckMsg, AT_ACK_MSG_LENGTH);
            if(HF_SUCCESS != iAck)
            {
                iRet = 1;
                goto SetWifiModeExit;
            }
		    iLength = sprintf(AtCmdArr, "AT+WSSSID=%s\r\n", ssid); 
            iAck    = hfat_send_cmd(AtCmdArr, iLength, AtAckMsg, AT_ACK_MSG_LENGTH);
            if(HF_SUCCESS != iAck)
            {
                iRet = 1;
                goto SetWifiModeExit;
            }
        }
		cJSON_Delete(json);
        iRet = 0;
                
        Reboot(0);
	}
  SetWifiModeExit:
	return iRet;
}

/*******************************************************************************
* 功能描述: 设置STA的加密参数
* 输入参数: 
* 输出参数: 
* -返回值-: 
* 使用说明: 
*******************************************************************************/
int SetClientKey(int Security, char *pKeyStr, char *pCmdSet)
{
    int iLength;
    if(SECURITY_WEP != Security) 
    {
        iLength = strlen(pKeyStr);
        if(8>iLength)
        {
            return 0;
        }
    }
    
    if(SECURITY_WEP == Security)
    {
        iLength = strlen(pKeyStr);
        if((5 == iLength) || (13 == iLength))
        {
            iLength  = sprintf(pCmdSet, "AT+WSKEY=SHARED,WEP-A,%s\r\n", pKeyStr);
        }
        else if((10 == iLength) || (26 == iLength))
        {
            iLength  = sprintf(pCmdSet, "AT+WSKEY=SHARED,WEP-H,%s\r\n", pKeyStr);
        }
        else
        {
            return 0;
        }
    }
    else if(SECURITY_WPA_TKIP == Security)
    {
        iLength  = sprintf(pCmdSet, "AT+WSKEY=WPAPSK,TKIP,%s\r\n", pKeyStr); 
    }
    else if(SECURITY_WPA2_TKIP == Security)
    {
        iLength  = sprintf(pCmdSet, "AT+WSKEY=WPA2PSK,TKIP,%s\r\n", pKeyStr); 
    }
    else if(SECURITY_WPA_AES == Security)
    {
        iLength  = sprintf(pCmdSet, "AT+WSKEY=WPAPSK,AES,%s\r\n", pKeyStr); 
    }
    else if(SECURITY_WPA2_AES == Security)
    {
        iLength  = sprintf(pCmdSet, "AT+WSKEY=WPA2PSK,AES,%s\r\n", pKeyStr); 
    }
    else if(SECURITY_WPA_TKIP_AES == Security)
    {
        iLength  = sprintf(pCmdSet, "AT+WSKEY=WPAPSK,AES,%s\r\n", pKeyStr); 
    }
    else if(SECURITY_WPA2_TKIP_AES == Security)
    {
        iLength  = sprintf(pCmdSet, "AT+WSKEY=WPA2PSK,AES,%s\r\n", pKeyStr); 
    }
    else if(SECURITY_WPA_WPA2_TKIP == Security)
    {
        iLength  = sprintf(pCmdSet, "AT+WSKEY=WPA2PSK,TKIP,%s\r\n", pKeyStr); 
    }
    else if(SECURITY_WPA_WPA2_AES == Security)
    {
        iLength  = sprintf(pCmdSet, "AT+WSKEY=WPA2PSK,AES,%s\r\n", pKeyStr); 
    }
    else if(SECURITY_WPA_WPA2_TKIP_AES == Security)
    {
        iLength  = sprintf(pCmdSet, "AT+WSKEY=WPA2PSK,AES,%s\r\n", pKeyStr); 
    }
    else
    {
        return 0;
    }
    return iLength;
}

/*******************************************************************************
* 功能描述: http设置wifi模式
* 输入参数: 
* 输出参数: 
* -返回值-: 
* 使用说明: 
*******************************************************************************/
static int HttpSetWifiMode(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf) 
{
    uint8_t iAck = 0;
    iAck = SetWifiMode(pArgRecvBuf);
    return sprintf(pAckBuf, "{\"response\":%d,\"code\":%d}", iCmd, (200<<iAck)); 
}

/***************************************************************************************************
* 功能描述: 扫描附近的AP,系统钩子函数，scan_ret中数据由系统调用时保存的AP集体信息
* 输入参数: 
* 输出参数: 
* -返回值-: 
* 使用说明: 
*****************************************************************************************************/
#define     AUTH_OPEN           0x00 
#define     AUTH_SHARE_KEY      0x01 
#define     AUTH_WPA            0x02 
#define     AUTH_WPA2           0x03 
#define     AUTH_WPA_WPA2       0x04 


#define     ENCRYPT_NONE        0x00 
#define     ENCRYPT_WEP         0x01 
#define     ENCRYPT_TKIP        0x02 
#define     ENCRYPT_AES         0x03 
#define     ENCRYPT_TKIP_AES    0x04 
int hfwifi_scan_callback(PWIFI_SCAN_RESULT_ITEM scan_ret)
{
	int iLength, iAuthEnc;
	char    TmpBuf[150] = {0}; 
	
	#if  0 
	int i;
	u_printf("%s,", scan_ret->ssid);
	u_printf("%d,", scan_ret->auth);
	u_printf("%d,", scan_ret->encry);
	u_printf("%d,", scan_ret->channel);
	u_printf("%d,", scan_ret->rssi);
	for(i=0; i<6; i++)
	{
		u_printf("%X", ((uint8_t*)scan_ret->mac)[i]);
	}
	u_printf("\n");
	#endif 
	
	if((AUTH_OPEN == scan_ret->auth) && (ENCRYPT_NONE == scan_ret->encry))
	{
	    iAuthEnc = SECURITY_NONE;
	}
	else if((AUTH_SHARE_KEY == scan_ret->auth) && (ENCRYPT_WEP == scan_ret->encry))
	{
	    iAuthEnc = SECURITY_WEP;
	}
	else if((AUTH_WPA == scan_ret->auth) && (ENCRYPT_TKIP == scan_ret->encry))
	{
	    iAuthEnc = SECURITY_WPA_TKIP;
	}
	else if((AUTH_WPA2 == scan_ret->auth) && (ENCRYPT_TKIP == scan_ret->encry))
	{
	    iAuthEnc = SECURITY_WPA2_TKIP;
	}
	else if((AUTH_WPA == scan_ret->auth) && (ENCRYPT_AES == scan_ret->encry))
	{
	    iAuthEnc = SECURITY_WPA_AES;
	}
	else if((AUTH_WPA2 == scan_ret->auth) && (ENCRYPT_AES == scan_ret->encry))
	{
	    iAuthEnc = SECURITY_WPA2_AES;
	}
	else if((AUTH_WPA == scan_ret->auth) && (ENCRYPT_TKIP_AES == scan_ret->encry))
	{
	    iAuthEnc = SECURITY_WPA_TKIP_AES;
	}
	else if((AUTH_WPA2 == scan_ret->auth) && (ENCRYPT_TKIP_AES == scan_ret->encry))
	{
	    iAuthEnc = SECURITY_WPA2_TKIP_AES;
	}
	else if((AUTH_WPA_WPA2 == scan_ret->auth) && (ENCRYPT_TKIP == scan_ret->encry))
	{
	    iAuthEnc = SECURITY_WPA_WPA2_TKIP;
	}
	else if((AUTH_WPA_WPA2 == scan_ret->auth) && (ENCRYPT_AES == scan_ret->encry))
	{
	    iAuthEnc = SECURITY_WPA_WPA2_AES;
	}
	else if((AUTH_WPA_WPA2 == scan_ret->auth) && (ENCRYPT_TKIP_AES == scan_ret->encry))
	{
	    iAuthEnc = SECURITY_WPA_WPA2_TKIP_AES;
	}
	else
	{
	    return 0;
	}

 
	iLength = sprintf(TmpBuf,      "{\"ssid\":\"%s\","
                                   "\"range\":\"%d\","
                                   "\"security\":\"%d\"},", 
                                   scan_ret->ssid, 
                                   scan_ret->rssi,
                                   iAuthEnc
                                   );
    if(iLength>iLengthHttpAckBuf) 
    { 
        return 0;                   // 大于剩余Buf，则不装 
    } 
	memcpy(pHttpAckBuf, TmpBuf, iLength); 
	iLengthHttpAckBuf -= iLength; 
    pHttpAckBuf += iLength;
	return 0;
}

/**************************************************************************************************
* 功能描述: 搜寻附近的wifi信号
* 输入参数: 
* 输出参数: 
* -返回值-: 
* 使用说明: 
****************************************************************************************************/
static int HttpScanNearbyWifiSignal(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf) 
{ 
    int iAck = 0, iLength;
    char    *pDataStart;
    
    iLengthHttpAckBuf = HTTP_SERVER_ACK_LENGTH-3-1; 
    
    pDataStart   = pAckBuf;
    pHttpAckBuf  = pAckBuf;
    iLength      = sprintf(pAckBuf,"{\"response\":%d,"
                                    "\"code\":200,"
                                    "\"data\":{\"signal\":[", iCmd);
    pHttpAckBuf  = pAckBuf + iLength; 

    iLengthHttpAckBuf -= iLength; 
    
	iAck         = hfwifi_scan(hfwifi_scan_callback); 
	if(iAck < 0)
	{
        iLength  = sprintf(pAckBuf,"{\"response\":%d,\"code\":400}", iCmd);
	}
	else
	{
	    pHttpAckBuf--;
        *pHttpAckBuf = ']';
        pHttpAckBuf++;
        *pHttpAckBuf = '}';
        pHttpAckBuf++;
        *pHttpAckBuf = '}';
        pHttpAckBuf++;
        *pHttpAckBuf = '\0';
	}
	#if  defined(__WIFI_SCAN_DBG__) && defined(__UDP_PRINT_DBG__) 
	//UdpDebugPrint(pDataStart, strlen(pDataStart));
	#endif 
	return strlen(pDataStart);
} 

/*******************************************************************************
* 功能描述: 获取当前wifi状态
* 输入参数: 
* 输出参数: 
* -返回值-: 
* 使用说明: 
*******************************************************************************/
#define         AP_MODE                 0x00
#define         STATION_MODE            0x01
extern int     DevModeStatus; 
static int HttpGetDevWifiState(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf) 
{
    int     iLength, iAck, iCount, iAuth;
	char    AtAckMsg[AT_ACK_MSG_LENGTH+1], *pS8Tmp, *pS8payload, *pS8Tmp2, *pS8Tmp3;;

	char    *pTmp;
	pTmp = pAckBuf; 
	if(AP_MODE_LED == DevModeStatus)
	{
        iAck = hfat_send_cmd("AT+WAP\r\n", sizeof("AT+WAP\r\n")-1, AtAckMsg, AT_ACK_MSG_LENGTH);
        if(HF_SUCCESS != iAck)
        {
            goto GetDevNetStateLabel;
        }
        pS8Tmp = strchr(AtAckMsg, ',');
        if(NULL == pS8Tmp)
        {
            goto GetDevNetStateLabel;
        }
        pS8Tmp ++;
        iLength = sprintf(pAckBuf, "{\"response\":%d,\"code\":200,"
                                   "\"data\":{\"mode\":%d,"
                                   "\"ssid\":\"",
                                   iCmd, AP_MODE);
        pS8payload  = pAckBuf + iLength;
        pS8Tmp2     = strchr(pS8Tmp, ',');
        memcpy(pS8payload, pS8Tmp, pS8Tmp2-pS8Tmp);
        pS8payload += pS8Tmp2-pS8Tmp;
        memcpy(pS8payload, "\",\"password\":\"", sizeof("\",\"password\":\"")-1);
        pS8payload += sizeof("\",\"password\":\"")-1;
        // 
        memset(AtAckMsg, '\0', AT_ACK_MSG_LENGTH);
        iAck = hfat_send_cmd("AT+WAKEY\r\n", sizeof("AT+WAKEY\r\n")-1, AtAckMsg, AT_ACK_MSG_LENGTH);
        if(HF_SUCCESS != iAck)
        {
            goto GetDevNetStateLabel;
        }
        pS8Tmp = strstr(AtAckMsg, "+ok=");
        if(NULL == pS8Tmp)
        {
            goto GetDevNetStateLabel;
        }
        pS8Tmp += sizeof("+ok=")-1;
        pS8Tmp2 = strchr(pS8Tmp, ',');
        iCount  = pS8Tmp2 - pS8Tmp;
        if(0x00 == strncasecmp(pS8Tmp, "OPEN", iCount))
        {
            iAuth = 0;
            memcpy(pS8payload, "NONE", sizeof("NONE")-1);
            pS8payload += sizeof("NONE")-1;
        }
        else if(0x00 == strncasecmp(pS8Tmp, "WPA2PSK", iCount))
        {
            iAuth = 5;
            pS8Tmp2++;
            pS8Tmp2 = strstr(pS8Tmp2, "AES,");
            if(NULL == pS8Tmp2)
            {
                goto GetDevNetStateLabel;
            }
            pS8Tmp2    += sizeof("AES,")-1;
            iLength     = strlen(pS8Tmp2);
            strncpy(pS8payload, pS8Tmp2, iLength);
            pS8payload += iLength;
        }
        else
        {
            goto GetDevNetStateLabel;
        }
        iLength = sprintf(pS8payload, "\",\"security\":%d}}", iAuth);
        *(pS8payload + iLength) = '\0';
	}
	else
	{
        iAck = hfat_send_cmd("AT+WSSSID\r\n", sizeof("AT+WSSSID\r\n")-1, AtAckMsg, AT_ACK_MSG_LENGTH);
        if(HF_SUCCESS != iAck)
        {
            goto GetDevNetStateLabel;
        }
        pS8Tmp = strstr(AtAckMsg, "+ok=");
        if(NULL == pS8Tmp)
        {
            goto GetDevNetStateLabel;
        }
        pS8Tmp += sizeof("+ok=")-1;
        iLength = sprintf(pAckBuf, "{\"response\":%d,\"code\":200,"
                                   "\"data\":{\"mode\":%d,"
                                   "\"ssid\":\"",
                                   iCmd, STATION_MODE);
        pS8payload  = pAckBuf + iLength;
        iCount      = strlen(pS8Tmp);
        memcpy(pS8payload, pS8Tmp, iCount);
        pS8payload += iCount;
        
        memcpy(pS8payload, "\",\"password\":\"", sizeof("\",\"password\":\"")-1);
        pS8payload += sizeof("\",\"password\":\"")-1;
        // 
        memset(AtAckMsg, '\0', AT_ACK_MSG_LENGTH);
        iAck   = hfat_send_cmd("AT+WSKEY\r\n", sizeof("AT+WSKEY\r\n")-1, AtAckMsg, AT_ACK_MSG_LENGTH);
        if(HF_SUCCESS != iAck)
        {
            goto GetDevNetStateLabel;
        }
        pS8Tmp = strstr(AtAckMsg, "+ok=");
        if(NULL == pS8Tmp)
        {
            goto GetDevNetStateLabel;
        }
        // 
        pS8Tmp += sizeof("+ok=")-1;
        pS8Tmp2 = strchr(pS8Tmp, ',');
        iCount  = pS8Tmp2 - pS8Tmp;
        if(0x00 == strncasecmp(pS8Tmp, "OPEN", iCount))
        {
            iAuth  = 0;
            pS8Tmp = strchr(pS8Tmp, ',');
        }
        else if(0x00 == strncasecmp(pS8Tmp, "SHARED", iCount))
        {
            iAuth = 1;
        }
        else if(0x00 == strncasecmp(pS8Tmp, "WPAPSK", iCount))
        {
            iAuth = 2;
        }
        else if(0x00 == strncasecmp(pS8Tmp, "WPA2PSK", iCount))
        {
            iAuth = 3;
        }
        else
        {
            goto GetDevNetStateLabel;
        }
        if(iAuth)
        {
            pS8Tmp2++;
            pS8Tmp3  = pS8Tmp2;
            pS8Tmp2 = strchr(pS8Tmp3, ',');
            iCount  = pS8Tmp2 - pS8Tmp3;
            if(0x00 == strncasecmp(pS8Tmp3, "TKIP", iCount))
            {
                //iAuth = 1;
            }
            else if(0x00 == strncasecmp(pS8Tmp3, "AES", iCount))
            {
                iAuth += 2;
            }
            else if(0x00 == strncasecmp(pS8Tmp3, "WEP", 3))
            {
                iAuth = 1;
            }
            else
            {
                goto GetDevNetStateLabel;
            }
            pS8Tmp = strchr(pS8Tmp3, ',');
        }
        pS8Tmp ++;
        iCount      = strlen(pS8Tmp);
        memcpy(pS8payload, pS8Tmp, iCount);
        pS8payload += iCount;
        iLength     = sprintf(pS8payload, "\",\"security\":%d}}", iAuth);
        *(pS8payload + iLength) = '\0';
	}
	return strlen(pTmp);
	// 
  GetDevNetStateLabel:
    iLength = sprintf(pAckBuf, "{\"response\":%d,\"code\":400}", iCmd); 
    *(pAckBuf+iLength) = '\0';
    return iLength; 
}


/*******************************************************************************
* 功能描述: 手机发送终端设备账号让设备与账号绑定
* 输入参数: 
* 输出参数: 
* -返回值-: 
* 使用说明: 
*******************************************************************************/
extern TurnOffTimeType         CountDownTurnOff[];
extern FwUpdateStatusType      FwUpdateStatusObj; 
static int HttpGetDevRegIDState(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf) 
{
    cJSON   *pJson; 
    int      iLength;
    char    iRet, *pTmp; 
	char	 *userName = NULL;

	u_printf("\r\nRegid reback info is:%s\n",pArgRecvBuf);

    pTmp  = pAckBuf;
    iRet  = 1; 
    pJson = cJSON_Parse(pArgRecvBuf); 
    
    if(pJson) 
    { 
        userName  = cJSON_GetObjectItem(pJson, "regid")->valuestring; 
         
        if(userName != NULL) 
        {
			//if(strncmp(userName,REGIDID,3) == 0)
			{
	            if(isGetRegid==0)
	            {
	                isGetRegid = 1;                     //已经获取到了设备的用户名
	                memset(DevRegInformation.regidName, 0, NAME_LENGTH);
	                memcpy(DevRegInformation.regidName, userName, NAME_LENGTH-3);
	                WriteDevBindInfo();                //保存用户账号到flash中
	                
	                iRet = 0; 
	             }
	             else
	             {
	                if(strncmp(DevRegInformation.regidName, userName, NAME_LENGTH-3) == 0)
	                {
	                    iRet = 0; 
	                }
	             }
			}
        }
        cJSON_Delete(pJson);
    }    

    iLength = sprintf(pTmp, "{\"response\":%d,\"code\":%d}",iCmd, (200<<iRet)); 
    pTmp += iLength;
    *pTmp = '\0';

    UdpDebugPrint("The regidID information get:\r\n", sizeof("The regidID information get:\r\n")-1);
 
    return (pTmp-pAckBuf);
}


/*******************************************************************************
* 功能描述: 设置设备的IP地址
* 输入参数: 
* 输出参数: 
* -返回值-: 
* 使用说明: 
*******************************************************************************/
static int SetStaticIp(char* pArgRecvBuf)
{
    cJSON   *pJson; 
    char    iRet,*ipAddress,*netMask,*gataWay,*dns;
    int     ipType;
    uint8_t iLength;

    
    iRet  = 1; 
    pJson = cJSON_Parse(pArgRecvBuf); 
    
    if(pJson) 
    {
        ipType = cJSON_GetObjectItem(pJson, "dhcp")->valueint;
        if(ipType == 0)
        {
            ipAddress = cJSON_GetObjectItem(pJson, "ip")->valuestring; 
            iLength   = strlen(ipAddress);
            if(iLength <5)
            {
                 return iRet;
            }
            
            netMask = cJSON_GetObjectItem(pJson, "netmask")->valuestring; 
            iLength   = strlen(netMask);
            if(iLength <5)
            {
                 return iRet;
            }
            
            gataWay = cJSON_GetObjectItem(pJson, "gateway")->valuestring; 
            iLength   = strlen(gataWay);
            if(iLength <5)
            {
                 return iRet;
            }
            
            dns = cJSON_GetObjectItem(pJson, "dns")->valuestring; 
            iLength   = strlen(dns);
            if(iLength <5)
            {
                 return iRet;
            }

            iRet = 0;

        }
        else
        {
           iRet = 0; 
        }
    }

    return iRet;
}


/*******************************************************************************
* 功能描述: 手机设置设备的IP地址
* 输入参数: 
* 输出参数: 
* -返回值-: 
* 使用说明: 
*******************************************************************************/
static int HttpSetStaticIp(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf)
{
    uint8_t iAck = 0;
    iAck = SetStaticIp(pArgRecvBuf);
    return sprintf(pAckBuf, "{\"response\":%d,\"code\":%d}", iCmd, (200<<iAck)); 
}


/*******************************************************************************
* 功能描述:改变机器的Product ID,Seral number,Mac address
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/
static int HttpModifyProductSn(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf) 
{
    char    iAck = 0, isSaveSakSn;
    
	cJSON   *json;
	char    *pValue;
	int     mac[7];
	
    int     i, iLength;

    //u_printf("Modify sn cmd is:%s\n",pArgRecvBuf);
    
	json = cJSON_Parse(pArgRecvBuf);
	if(json)
	{
	    isSaveSakSn = 1;
		pValue = cJSON_GetObjectItem(json,"sn")->valuestring;
		if(strcmp(pValue, "null") != 0)
		{
            iLength = strlen(pValue);
            i       = HF_SUCCESS+1;
            if(SERIAL_NUN_ID_LEN != iLength)
            {
                goto ModSnExit;
            }
            i           = HF_SUCCESS;
            isSaveSakSn = 0;
            memcpy(DevInfoObj.dev_sn, pValue, iLength);
		}
		// 
		pValue = cJSON_GetObjectItem(json,"sak")->valuestring;
		if(strcmp(pValue, "null") != 0)
		{ 
            iLength = strlen(pValue);
            i       = HF_SUCCESS+1;
            if(SAK_LEN != iLength)
            {
                goto ModSnExit;
            }
            i           = HF_SUCCESS;
            isSaveSakSn = 0;
            memcpy(DevInfoObj.dev_sak, pValue, iLength);
		} 
		// 
        if(0x00 == isSaveSakSn)
        {

            WriteDevSakSN();

        }
		// 
		pValue = cJSON_GetObjectItem(json,"mac")->valuestring;
		if(strcmp(pValue, "null") != 0)
		{ 
            i       = HF_SUCCESS+1;
			iLength = sscanf(pValue, "%02X:%02X:%02X:%02X:%02X:%02X",
                                    &mac[0], &mac[1],
                                    &mac[2], &mac[3],
                                    &mac[4], &mac[5]); 
            if(iLength)
            { 
                for(i=0; i<6; i++)
                {
                    DevInfoObj.Dev_Mac[i] = (uint8_t)mac[i];
                }
                i = HF_SUCCESS;
                MacChgFlag = 1;
            } 
		} 
      ModSnExit:
		cJSON_Delete(json);
	}
    iAck = (i==HF_SUCCESS)?(0):(1);
    return sprintf(pAckBuf, "{\"response\":%d,\"code\":%d}", iCmd, (200<<iAck));

    #if defined(__SN_SET_DBG__) && defined(__UDP_PRINT_DBG__)
    UdpDebugPrint("SetSnDebug:\r\n", sizeof("SetSnDebug:\r\n")-1);
    UdpDebugPrint(pAckBuf, strlen(pAckBuf));
    UdpDebugPrint("\r\n", sizeof("\r\n")-1);
    #endif 
} 
void ModifyDevMac(void)
{
    int     iLength, i;
	char    AtCmdBuf[40], AtRsltMsg[19+1]={0};
	if(0x00 == MacChgFlag)
	{
	    return;
	}
    iLength = sprintf(AtCmdBuf, "AT+WSMAC=8888,%02X%02X%02X%02X%02X%02X\r\n",
                                DevInfoObj.Dev_Mac[0], DevInfoObj.Dev_Mac[1],
                                DevInfoObj.Dev_Mac[2], DevInfoObj.Dev_Mac[3],
                                DevInfoObj.Dev_Mac[4], DevInfoObj.Dev_Mac[5]);

    //u_printf("set mac cmd is:%s\n",AtCmdBuf);
                                
    i       = hfat_send_cmd(AtCmdBuf, iLength, AtRsltMsg, 19);
    if(HF_SUCCESS == i)
    {
		i = hfat_send_cmd(AtCmdBuf, iLength, AtRsltMsg, 19);   
		if(HF_SUCCESS == i)
		{
	    	MacChgFlag = 0;
	    	Reboot(0);
		}
	}
}



/*******************************************************************************
* 功能描述:重启机器 
* 入口参数:
* 出口参数:
* 使用说明:
* 调用方法:
*******************************************************************************/
static int HttpRestartMachine(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf)
{
    char        iRslt = 0;
	  int 				option;
    
    iRslt    = RebootSyncDev(pArgRecvBuf,&option); 
	return sprintf(pAckBuf, "{\"response\":%d,\"code\":%d}", iCmd, (200<<iRslt));    // 回应HTTP Ack数据 
}




/*******************************************************************************
* 功能描述: Http控制排插开关 
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/
static int HttpSwitch_OnOffCtl(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf)
{
    char iRslt;
    
    iRslt   = 1;
    iRslt   = Switch_Ctl(pArgRecvBuf);
    return sprintf(pAckBuf, "{\"response\":%d,\"code\":%d}", iCmd, 200<<iRslt); 
} 


/*******************************************************************************
* 功能描述: 
* 输入参数: 
* 输出参数: 
* -返回值-: 
* 使用说明: 
*******************************************************************************/
int HttpGetDevTime(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf) 
{
    uint8_t     iAck = 0;
    
    return sprintf(pAckBuf, "{\"response\":%d,"
                     "\"code\":%d,"
                     "\"zone\":%d,"
                     "\"nowTimeSec\":%d,"
                     "\"data\":{"
                     "\"time\":\"%d-%02d-%02d,%02d:%02d:%02d\","
                     "\"server\":\"%s\"}}",
                     iCmd, (200<<iAck),
                     DevInfoObj.TimeZone,
                     DevTimeObj.NowTimeSec,
                     DevTimeObj.NowTimeTm.tm_year, 
                     DevTimeObj.NowTimeTm.tm_mon, 
                     DevTimeObj.NowTimeTm.tm_mday,
                     DevTimeObj.NowTimeTm.tm_hour, 
                     DevTimeObj.NowTimeTm.tm_min, 
                     DevTimeObj.NowTimeTm.tm_sec,
                     switch_server_net
                     ); 
}



/*******************************************************************************
* 功能描述: 手机获取时实时功耗和开关状态
* 输入参数: 
* 输出参数: 
* -返回值-: 
* 使用说明: 
*******************************************************************************/
extern PowerRecordType PowerRecordObj[PORT_NUM];

static int HttpGetRealTimeConsumePower(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf)
{
    int     iLength;
    
    uint8_t iAck = 0;
    uint8_t portID = 0;
    char *pData,*pTemp;

    pData = pAckBuf;
    pTemp = pData;

    iLength = sprintf(pTemp,  "{\"response\":%d,\"code\":%d,\"data\":{\"watt\":[",iCmd, (200<<iAck));
    pTemp += iLength;
    for(portID=0;portID<PORT_NUM;portID++)
    {
        iLength = sprintf(pTemp, "%d,",PowerRecordObj[portID].record_ValidPower); //每个口的功耗
        pTemp  += iLength; 
    }
    *(pTemp-1) = ']';
    
    iLength = sprintf(pTemp, ",\"amp\":[");
    pTemp += iLength;
    for(portID=0;portID<PORT_NUM;portID++)
    {
        iLength = sprintf(pTemp, "%d,", PowerRecordObj[portID].record_ValidCurrent); //每个口的电流
        pTemp  += iLength; 
    }
    *(pTemp-1) = ']';

    iLength = sprintf(pTemp, ",\"switch\":[");
    pTemp += iLength;
    for(portID=0;portID<PORT_NUM;portID++)
    {
        iLength = sprintf(pTemp, "%d,", SwInformation.PortInfor[portID].OnOffStatus ); //每个口的开关
        pTemp  += iLength; 
    }
    *(pTemp-1) = ']';
    *pTemp = '}';
     pTemp++;
    *pTemp = '}';
    pTemp++;
    *pTemp = '\0';

    #if defined(__HTTP_SET_DBG__) && defined(__UDP_PRINT_DBG__) 
        UdpDebugPrint("The RTdata is:\r\n", sizeof("The data is:\r\n")-1);
        UdpDebugPrint(pAckBuf, pTemp-pData);                                         
    #endif 
    //u_printf("The new data = %s,the length is %d\n",pAckBuf,pTemp-pData);
   
    return (pTemp-pData); 
                        
}




/*******************************************************************************
* 功能描述: 获取设备规则
* 输入参数: 
* 输出参数: 
* -返回值-: 
* 使用说明: 
*******************************************************************************/
extern DevRuleInfoType         DevRuleInfoObj; 

char *GetDevRule(char *pDataBuf) 
{ 
    int     iLength;
    char    *pBuf = pDataBuf;
    uint8_t i, j, validDay;
    char    *pTemp, PortListBuf[50*2+1];
    
    if(0x00 < DevRuleInfoObj.RuleCount) 
    { 
        pBuf = pDataBuf; 
        for(i=0; i<DevRuleInfoObj.RuleCount; i++) 
        { 
            validDay   = DevRuleInfoObj.SwitchRulePool[i].SwOpDateObj;
            pTemp      = PortListBuf; 
            for(j=0; j<PORT_NUM; j++) 
            { 
                iLength = sprintf(pTemp, "%d,", DevRuleInfoObj.SwitchRulePool[i].PortArr[j]); 
                pTemp  += iLength; 
            } 
            *(pTemp-1) = '\0';
            // 
            iLength = sprintf(pBuf, "{\"id\":%d,"
                                    "\"en\":%d,"
                                    "\"port\":[\"%s\"],"
                                    "\"day\":%d,"
                                    "\"time\":%d,",
                                    DevRuleInfoObj.SwitchRulePool[i].RuleID, 
                                    DevRuleInfoObj.SwitchRulePool[i].RuleEnable, 
                                    PortListBuf,DevRuleInfoObj.SwitchRulePool[i].timeStamp,DevRuleInfoObj.SwitchRulePool[i].timeLong); 
            pBuf += iLength;
          
            iLength = sprintf(pBuf, "\"week\":[%d,%d,%d,%d,%d,%d,%d]},", 
                                    (0x01&(validDay>>0)), (0x01&(validDay>>1)), (0x01&(validDay>>2)), 
                                    (0x01&(validDay>>3)), (0x01&(validDay>>4)), (0x01&(validDay>>5)), 
                                    (0x01&(validDay>>6)) ); 
            pBuf += iLength; 
        }
        pBuf--;                                                                 // 最后一条规则无逗号分隔符，故占据最后一条规则后的逗号分隔符的位置。
        *pBuf = '\0';
    } 
    return pBuf; 
} 




/******************************************************************************* 
* 功能描述:返回设备规则 
* 输入参数:
* 输出参数:
* -返回值-:
* 使用说明:
* 调用方法:
*******************************************************************************/
static int HttpGetRule(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf)   
{ 
	char     *pTemp, *pData;
    int16_t  iLength;
    
    pTemp   = pAckBuf;                                                      // 申请将要发送的数据的存储区 
    iLength = sprintf(pTemp, "{\"response\":%d,\"code\":200,"
                             "\"data\":{\"rule\":[", iCmd);
	if(-1 == iLength)
    {
        return sprintf(pTemp, "{\"response\":%d,\"code\":400}", iCmd);             // 数据错误 回应 
    }
    else
    {   // 载荷数据体 
        pData = pTemp+iLength;
        pData = GetDevRule(pData);
        strcat(pData,"]}}");
        return strlen(pAckBuf);
    }
} 

/*******************************************************************************
* 功能描述: 
* 输入参数: 
* 输出参数: 
* -返回值-: 
* 使用说明: 
*******************************************************************************/
int GetCountDown(int ArgPort,char *pArgBuf)
{
    struct tm *pStartTime;
    int        iLen;
    pStartTime = localtime(&CountDownTurnOff[ArgPort].StartTimePoint);
    iLen = sprintf(pArgBuf, "{\"start\":\"%d-%02d-%02d,%02d:%02d:%02d\",\"switch\":%d,\"delay\":%d}",
                             pStartTime->tm_year+1900,
                             pStartTime->tm_mon+1,
                             pStartTime->tm_mday,
                             pStartTime->tm_hour+DevInfoObj.TimeZone-12,            //加上时区
                             pStartTime->tm_min,
                             pStartTime->tm_sec,
                             CountDownTurnOff[ArgPort].onOrOff,
                             CountDownTurnOff[ArgPort].CountDownValue);
    return iLen;
}



/*******************************************************************************
* 功能描述: 手机获取终端倒计时
* 输入参数: 
* 输出参数: 
* -返回值-: 
* 使用说明: 
*******************************************************************************/
extern TurnOffTimeType         CountDownTurnOff[];

static int HttpGetDevCountDown(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf) 
{
    cJSON   *pJson; 
    int     iPort, iLength;
    char    iRet, *pTmp; 

    pTmp  = pAckBuf;
    iRet  = 1; 
    pJson = cJSON_Parse(pArgRecvBuf); 
    
    if(pJson) 
    { 
        iPort  = cJSON_GetObjectItem(pJson, "port")->valueint; 
        cJSON_Delete(pJson); 
        if((iPort<(PORT_NUM+1)) && (iPort>=0)) 
        {
            iRet = 0; 
        }
    } 
    
    if(0x00 == iRet)
    {
        iLength = sprintf(pTmp, "{\"response\":%d,\"code\":%d,"
                                   "\"data\":", 
                                   iCmd, (200<<iRet) ); 
        pTmp += iLength; 
        iLength = GetCountDown(iPort, pTmp);
        pTmp += iLength; 
        *pTmp = '}';
        pTmp++;
        *pTmp = '\0';
        
       // u_printf("The new length = %d\n",iLength);
    }
    else
    {
        sprintf(pTmp, "{\"response\":%d,\"code\":%d,"
                         "\"data\":{\"start\":\"\",\"delay\":0}}", 
                         iCmd, (200<<1)); 
    }

    #if defined(__HTTP_SET_DBG__) && defined(__UDP_PRINT_DBG__) 
        UdpDebugPrint("The data is:\r\n", sizeof("The data is:\r\n")-1);
        UdpDebugPrint(pAckBuf, pTmp-pAckBuf);                                         
    #endif 
   // u_printf("iPort = %d\n",iPort);
   // u_printf("The length = %d,pAckBuf = %s\n",pTmp-pAckBuf,pAckBuf);
    return (pTmp-pAckBuf);
}


/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/ 

