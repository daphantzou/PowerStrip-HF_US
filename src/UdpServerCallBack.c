/*********************************************************************************
* @file    UdpServerCallBack.c
* @author  
* @version V1.0.0
* @date    2013/06/23
* @brief   
*********************************************************************************/
#include <stdint.h> 
#include <string.h> 
#include <stdio.h> 
#include <stdlib.h>

#include "hsf.h"
#include "cJSON.h"

#include "ConfigAll.h"
#include "SwitchApp.h"
#include "DevLib.h"
#include "DevInfo.h"
#include "DevRule.h"
#include "DevDataType.h"
#include "read_power.h"
#include "UdpServerCallBack.h" 
#include "remote_update.h" 
#include "rn8209.h" 
#include "PublicServerCom.h" 
#include "UdpSocket.h" 
#include "flash.h" 
#include "Produce_Test.h"
#include "HttpdCallBack.h" 
#include "TcpSocket.h"
#include "PowerStaticsRecord.h" 
#include "security_mode.h"
#include "DevRule.h" 


#define UDP_FIELD_1_CMD_LENTH                       2 

#define SWITCH_RESERVE_CMD                          0xff                        // 



uint8_t     isDevReg = 0;                                                        //是否注册成功，及账号绑定是否成功
uint8_t     keyFactorySuccess = 0;                                              //按键恢复出厂设置是否成功标志
uint8_t		RuleReportMark = 0;
uint8_t   	reportCountDownMark = 0;											//上报倒计时信息
uint8_t		reportMasterSlaveInfo = 0;											//上报主从信息
uint8_t		reportRebootInfo = 0;												//上报端口Reboot信息
struct sockaddr_in		PhoneSockAddr;											//手机端IP地址


// 外总变量声明 
extern uint16_t heartBeatCount;                                                  //发送心跳包次数
extern DevRuleInfoType 		DevRuleInfoObj; 
extern uint8_t isGetRegid;                      //是否获得了用户账号标志位
extern char MacChgFlag;


extern DevInfoType          DevInfoObj;                                         // 设备信息 
extern FwUpdateStatusType   FwUpdateStatusObj;                                  // 设备状态 

extern hfuart_handle_t huart1;
//统计有多长时间的数据没有加上时间戳
extern DevRegInfoType          DevRegInformation;                                      //设备注册信息
extern uint32_t            totalNoTimeRecord;                                  //无时间保存的数据个数综总和

static uint8_t UdpUpdateAppFwAsUrl(char *pDataBuf);
static uint8_t UdpRestartMachine(char *pData);
static uint8_t UdpModifyName(char *pBuf);
static uint8_t ReportUdpBroadcast(char *pData);
static uint8_t Udp_TurnOnOffSwitch(char *pBuf);
static uint8_t UdpUpdateAppFwAsSlave(char *pDataBuf);
static uint8_t UdpSetDevReboot(char *pBuf);
static uint8_t UdpRelatedControlSwitch(char *pBuf);
static uint8_t UDPSetWifiMode(char* pArgRecvBuf) ;
static uint8_t UdpScanNearbyWifiSignal(char* pArgRecvBuf) ;
static uint8_t UdpModifyProductSn(char* pArgRecvBuf);
static uint8_t UdpCalibratePowerConsume(char* pArgRecvBuf);
static uint8_t UdpGetPowerConsume(char* pArgRecvBuf);




#if defined(__UDP_NET_STATE_PRINT_DBG__) && defined(__UDP_PRINT_DBG__)
static uint8_t UdpNetStateDebug(char *pBuf);
#endif 




// UDP服务器回调响应入口函数 
typedef uint8_t (*pfUdpServerCmdType)(char *pData); 
typedef struct UdpServerReceive 
{ 
    uint8_t             iCmdField;                                              // UDP命令 
    pfUdpServerCmdType  pfUdpServerObj;                                         // UDP命令响应函数 
}UdpServeReceiveType; 
const UdpServeReceiveType UdpServerArr[] = 
{   // 命令                                 ,   // 响应函数 
   #if (PORT_NUM!= 1) 
    { SWITCH_SET_PORT_HOST_SLAVE_15_CMD     ,   UdpSetHostSlave         },             //服务器设置插座主从
   #endif 
    { DEV_QUERY_BROADCAST_DOWN_00_CMD       ,   ReportUdpBroadcast      },              //UDP广播包
    { SWITCH_ON_OFF_CTL_UDP_14_CMD          ,   Udp_TurnOnOffSwitch     },              //服务器控制插座开关命令 //OK
    { SWITCH_RENAME_UDP_18_CMD              ,   UdpModifyName           },              //服务器修改终端名称
    { SWITCH_RELATED_CONTROL_UDP_28_CMD     ,   UdpRelatedControlSwitch },              //服务器转发关联控制命令
    { SWITCH_SET_DEV_REBOOT_1A_CMD          ,   UdpSetDevReboot         },              //服务器下发reboot指令
    { SWTICH_RESTART_SYN_UDP_1E_CMD         ,   UdpRestartMachine       },              //服务器命令终端重启或恢复出厂设置
    { SWITCH_UPDATE_FW_UDP_33_CMD           ,   UdpUpdateAppFwAsUrl     },              //服务器升级设备固件 
    { SWITCH_SET_DEVICE_WIFI_UDP_71_CMD     ,	UDPSetWifiMode			},				//设置wifi工作模式
    { SWITCH_GET_WIFI_SIGNAL_UDP_72_CMD     ,	UdpScanNearbyWifiSignal	},				//获取周围wifi信号
    { SWITCH_SET_DEVICE_MAC_UDP_74_CMD		,	UdpModifyProductSn		},				//设置设备的邋錗MAC和SN
    { SWITCH_CALIBRATE_POWER_CONSUME_UDP_75_CMD,UdpCalibratePowerConsume},				//校准功率芯片
    { SWITCH_GET_POWER_CONSUME_UDP_90_CMD	,	UdpGetPowerConsume		},				//UDP设置功率芯片校准值
    { SWTICH_UPDATE_FW_UDP_81_CMD           ,   UdpUpdateAppFwAsSlave   },              //UDP推送升级数据进行固件升级
    
    
   #if defined(__UDP_NET_STATE_PRINT_DBG__) && defined(__UDP_PRINT_DBG__)
    { DEBUG_NET_UDP_CMD                     ,   UdpNetStateDebug        }, 
   #endif 

}; 

// UDP服务器当前接收到的数据、PCB连接、接口等 
#define             UDP_MAX_PACKAGET_LENGTH             1400 

char                       UdpAckMsg[UDP_MAX_PACKAGET_LENGTH+1]={0}; 
extern TurnOffTimeType     CountDownTurnOff[PORT_NUM+1]; 
extern FwUpdateArUrlType   FwUpdateObj;             //udp升级文件数据
extern uint8_t isDevReg;                           // 是否在公网注册 




/******************************************************************************* 
* 功能描述: UDP接收数据回调函数
* 输入参数: 
* 输出参数: 
* -返回值-: 
* 使用说明: 
*******************************************************************************/ 
extern int USER_FUNC UdpServer_RecvData(uint32_t event, char *pRecvData, uint32_t len,uint32_t buf_len)
{
    char        *pRecvBufTemp,*pJasonBufTemp; 
    char        *pContextData; 
    cJSON       *json; 
    int8_t      iCmdBuf[UDP_FIELD_1_CMD_LENTH+1] = {0}; 
    uint32_t    Cmd; 
    uint8_t     iArrCount,iTemp; 

    //u_printf("Receive data is %s\n",pRecvData);
    //UdpDebugPrint(pRecvData,strlen(pRecvData));
    
    if(event == HFNET_SOCKETA_DATA_READY)
    { 
        pRecvBufTemp  = pRecvData; 
        
        if('V' != *(pRecvBufTemp+0))
        {
            iCmdBuf[0]    = *(pRecvBufTemp+0); 
            iCmdBuf[1]    = *(pRecvBufTemp+1); 
            Cmd           = strtoul((char const*)iCmdBuf, NULL,16);

			//u_printf("Cmd = %D\n",Cmd);
        }
        else
        {
            pJasonBufTemp = pRecvBufTemp + UDP_FIELD_1_CMD_LENTH;
            
            json = cJSON_Parse(pJasonBufTemp);
            if(json)
            {
        		Cmd = cJSON_GetObjectItem(json,"cmd")->valueint;
            }
            cJSON_Delete(json);       
        }

        #if 0
        else
        { 
            u_printf("Receive data is %s\n",pRecvData);
            
            iCmdBuf[0]    = *(pRecvBufTemp+33); 
            iCmdBuf[1]    = *(pRecvBufTemp+34); 
            Cmd           = strtoul((char const*)iCmdBuf, NULL,10);
        }
        #endif
        
        
        pContextData = pRecvBufTemp + UDP_FIELD_1_CMD_LENTH;
        
        if(SWTICH_UPDATE_FW_UDP_81_CMD == Cmd) 
        { 
            pContextData += SERIAL_NUN_ID_LEN;                              // UDP广播包有设备ID 
        }

        #if 0
        //u_printf("Receive data is %s\n",pRecvData);
        u_printf("Jason data is %s\n",pContextData);
        u_printf("Cmd buffer is %s  Cmd = %d\n",iCmdBuf,Cmd);
        #endif

        iArrCount = Dim(UdpServerArr); 
        for(iTemp=0; iTemp<iArrCount; iTemp++) 
        { 
            if(Cmd == UdpServerArr[iTemp].iCmdField)  
            { 
                if(NULL != UdpServerArr[iTemp].pfUdpServerObj) 
                { 
                    UdpServerArr[iTemp].pfUdpServerObj(pContextData);
                } 
                len = 0;
                break; 
            } 
        } 
	} 
	return len;
}
/*******************************************************************************
* 功能描述: 发送UDP数据 
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/ 
extern void SendDataToServer(int fd, struct sockaddr* TargetSockAddr, char *pArgBuf, int ArgLength)
{
    sendto(fd, pArgBuf, ArgLength, 0, TargetSockAddr, sizeof(struct sockaddr)); 
}
/*******************************************************************************
* 功能描述: 发送UDP数据包 
* 入口参数: 
* 出口参数: 
* 使用说明: 
* -备--注-: 
*   备注数据可组成一个结构体，在函数参数中传递其地址即可，如此可提高效率 
*   目前最多可以发送1400个数据(超过此值则不能一次发送，只能由用户进行分包发送)
*******************************************************************************/
extern void SendUdpData(char* pData, int DataLen)
{
    #if  defined(__BSD_SOCKET_UDP_SERVER_ENABLE__)       
        UdpServer_SendData(pData, DataLen);
    #else 
        hfnet_socketa_send(pData, DataLen, 3);
    #endif 
}



/*******************************************************************************
* 功能描述: 
* 输入参数: 
* 输出参数: 
* -返回值-: 
* 使用说明: 
*******************************************************************************/
static uint8_t UdpUpdateAppFwAsUrl(char *pDataBuf)
{ 
    int         iLength; 
    char        AckMsgArray[40]={0}; 
    uint8_t     iResult     = 0; 
    
    iResult = SetUpdateUrl(pDataBuf); 
    
    iLength = sprintf(AckMsgArray, "V3{\"sn\":\"%s\",\"response\":%d,\"code\":%d}", 
									DevInfoObj.dev_sn,
                                   (SWITCH_UPDATE_FW_UDP_33_CMD), 
                                   (200<<iResult) ); 
    SendUdpData(AckMsgArray, iLength); 
     
    return 0;
} 



/******************************************************************************* 
* 功能描述: 回应客户端，指示本机的存在. 
* 输入参数: 
*  upcb : the udp_pcb which received data 
*  p    : the packet buffer that was received 
*  addr : the remote IP address from which the packet was received  
*  port : the remote port from which the packet was received 
* 输出参数: 
* -返回值-: 
* 使用说明: 
*******************************************************************************/ 
extern SoftwareVerType         CurFwVersionObj ;                                // 本地固件版本 
extern DevRegInfoType          DevRegInformation;                                      //设备注册信息
//extern char    regidName[NAME_LENGTH];             //手机发送的用户账号名
static uint8_t ReportUdpBroadcast(char *pData)
{     
    char      *pAck; 
    int       iLength;
	static    uint8_t savePhoneIp = 0;			//是否保存手机端IP和端口号
    
    //u_printf("broadCast data is:%s\n",pData);
    //UdpDebugPrint("UDP broadcast date is :\n",sizeof("UDP broadcast date is :\n"));
    //UdpDebugPrint(pData, strlen(pData));
    
    
    // 增加识别SW=ALL与SW=ID; 
    if((0==memcmp(pData,"dv=",3)) || (0==memcmp(pData,"sw=",3)))
    { 
		if(!savePhoneIp)
		{
			savePhoneIp = 1;
			memset((char*)&PhoneSockAddr,0,sizeof(PhoneSockAddr));
			memcpy((char*)&PhoneSockAddr, (char*)&RecvAddr, sizeof(PhoneSockAddr));
		}
		
        pData += 3; 
        if(0 == memcmp(pData, "all", 3)) 
        { 
            pData += sizeof("all,")-1;
            SetCurTime(pData);
            goto respond_broadcast;
        } 
        else if(0 == memcmp(pData, (char const *)DevInfoObj.dev_sn, SERIAL_NUN_ID_LEN)) 
        { 
          respond_broadcast:
            pAck    = UdpAckMsg;  

          
            iLength = sprintf(pAck, "{\"response\":%X,\"data\":{"
                                    "\"sn\":\"%s\","
                                    "\"name\":\"%s\","
                                    "\"mac\":\"%02X:%02X:%02X:%02X:%02X:%02X\","
                                    "\"sak\":\"%s\","
                                    "\"regid\":\"%s\","
                                    "\"isGetRegid\":%d,"
                                    "\"isMainSvrOK\":%d,"
                                    "\"MainSveIP\":\"%08X\","
                                    "\"MainSvrPort\":\"%x\","
                                    "\"time\":%d,"
                                    "\"ver\":\"%d.%02d\"}}",
                                    DEV_QUERY_BROADCAST_DOWN_00_CMD, 
                                    DevInfoObj.dev_sn, 
                                    DevInfoObj.dev_name,
                                    DevInfoObj.Dev_Mac[0], DevInfoObj.Dev_Mac[1],
                                    DevInfoObj.Dev_Mac[2], DevInfoObj.Dev_Mac[3],
                                    DevInfoObj.Dev_Mac[4], DevInfoObj.Dev_Mac[5],
                                    DevInfoObj.dev_sak,
                                    DevRegInformation.regidName,            				 //手机发送的用户账号名,
                                    isGetRegid,FwUpdateStatusObj.isMainSvrOK,
	                      			MainSvrSockAddr.sin_addr.s_addr,
	                      			MainSvrSockAddr.sin_port,
	                      			DevTimeObj.NowTimeSec,
                                    CurFwVersionObj.big_ver, 
                                    CurFwVersionObj.little_ver
                                    );  
            if(-1 == iLength)
            { 
                return 0;
            } 
            *(pAck + iLength) = '\0';

			SendUdpData(pAck, iLength); 
			
        } 
    }
    return 0;
}



/*******************************************************************************
* 功能描述: UDP要求终端设备重启、同步或恢复出厂设置
* 入口参数:
* 出口参数:
* 使用说明:
* 调用方法:
*******************************************************************************/
extern struct sockaddr_in      MainSvrSockAddr;                                    // 主服务器sockaddr 

static uint8_t UdpRestartMachine(char *pData) 
{ 
    char    iAck, AckMsgArray[100]={0}; 
    int     iLength,option; 
    
    iAck    = RebootSyncDev(pData,&option); 

    iLength = sprintf(AckMsgArray, "V3{\"sn\":\"%s\",\"sak\":\"%s\",\"response\":%d,\"op\":%d,\"code\":%d}", 
                                    DevInfoObj.dev_sn, 
                                    DevInfoObj.dev_sak,
                                   (SWTICH_RESTART_SYN_UDP_1E_CMD), 
                                    option,
                                   (200<<iAck) ); 

    SendDataToServer(FwUpdateStatusObj.UdpFd,(struct sockaddr*)&MainSvrSockAddr,AckMsgArray, iLength);
    SendUdpData(AckMsgArray, iLength); 

    #if 0
        UdpDebugPrint("The reback od reboot is :\r\n",strlen("The reback od reboot is :\r\n"));
        UdpDebugPrint(AckMsgArray, iLength);
    #endif
    
    return 0; 
} 


/*******************************************************************************
* 功能描述: 升级从机程序
* 输入参数: 
* 输出参数: 
* -返回值-: 
* 使用说明: 
*******************************************************************************/
static char SaveUpdateFw(char *pBufData, int ArgLength, uint32_t iNumPackage)
{
    int      i;
    char     CheckSum,iAck;
    int16_t  TmpChk;
    
    iAck     = 1; 
    TmpChk   = 0; 
    CheckSum = 0;
    for(i=0; i<ArgLength; i++) 
    {
        TmpChk   += *(pBufData+i);
        CheckSum += *(pBufData+i);
    }
    CheckSum = (char)TmpChk;
	
    //u_printf("save chk=%d,file_check=%d, len:%d\r\n", CheckSum, *(pBufData+ArgLength), ArgLength);

    if(CheckSum == *(pBufData+ArgLength))
    { 
        hfupdate_write_file(HFUPDATE_SW, FwUpdateObj.intFwLength, pBufData, ArgLength); 
    	FwUpdateObj.CheckSumWhole += TmpChk; 
        FwUpdateObj.intFwLength   += ArgLength; 
        iAck      = 0; 

	#if 0
		u_printf("Package[%d]: check passed!\n",iNumPackage);
		if(iNumPackage%10 == 0)
		{
			u_printf("\r\n");
		}
	#endif
    }
	//else
	{
		//u_printf("Package[%d]: check errored!\n",iNumPackage);
	}
    return iAck;
}



static uint8_t UdpUpdateAppFwAsSlave(char *pDataBuf)
{ 
    int         iLength;
    uint32_t    iNumPackage;
	static uint32_t lastPackID = 0;
    char        *pFwData;
    char        AckMsgArray[120]={0};
    char        iAck;
    //char		testBuf[100] = {0};
    int16_t     TotalCheck;
    
   #if  defined(__WATCH_DOG_ENABLE__)
	FeedWatchDog(); 
   #endif 

	iNumPackage = ((pDataBuf[0]<<24)&(0xff000000))|((pDataBuf[1]<<16)&(0xff0000))|((pDataBuf[2]<<8)&(0xff00))|((pDataBuf[3]<<0)&(0xff<<0));
	            
	iLength     = ((pDataBuf[4]<<8)&0xFF00) | (pDataBuf[5]); 
	pFwData     = pDataBuf; 
	pFwData    += 6; 

    //u_printf("Now we updata!	iLength = %d	iNumPackage = %d\n",iLength,iNumPackage);
	
	iAck = 1; 
	if(iLength != 0)
    { 
        if(0x01 == iNumPackage)
        { 	
			hfuart_close(0);			
            
            FwUpdateObj.CheckSumWhole = 0; 
            FwUpdateObj.intFwLength   = 0; 
            hfupdate_start(HFUPDATE_SW);
            iAck = SaveUpdateFw(pFwData, iLength,iNumPackage); 
			lastPackID = 1;
            
        } 
        else 
        { 
			if(iNumPackage>lastPackID)
			{
            	iAck = SaveUpdateFw(pFwData, iLength, iNumPackage);			

			#if 0
				if(iNumPackage-lastPackID >= 2)
				{
					memset(testBuf,0,sizeof(testBuf));
					sprintf(testBuf,"curPackage	%d.Lost %d package.Lost package ID=%d\n",iNumPackage,iNumPackage-lastPackID-1,(iNumPackage+lastPackID)/2);
					UdpDebugPrint(testBuf,strlen(testBuf));

					iAck = 1;
				}
			#endif
				lastPackID = iNumPackage;
			}
			else
			{
				iAck = 0;
			}
        } 
    } 
    else 
    { 
		iLength    = (pFwData[0]<<24&0xFF000000)|(pFwData[1]<<16&0xFFFF0000)|(pFwData[2]<<8&0xFFFFF00)|(pFwData[3]);
		TotalCheck = ((pFwData[4]<<8)&0xFF00) | pFwData[5];

	#if 0
		memset(testBuf,0,sizeof(testBuf));
        sprintf(testBuf,"TotalCheck = %d      FwUpdateObj.CheckSumWhole = %d\r\n", TotalCheck, FwUpdateObj.CheckSumWhole);
		UdpDebugPrint(testBuf,strlen(testBuf));
	#endif
	
        huart1 = hfuart_open(0);
        
		if(TotalCheck == FwUpdateObj.CheckSumWhole)
		{

            FwUpdateObj.FirmwareCompleted = 0x02;
            FwUpdateObj.intFwLength       = iLength;
			iAck = 0;

		#if 0
			memset(testBuf,0,sizeof(testBuf));
			UdpDebugPrint("checkSum ok!\n",strlen("checkSum ok!\n"));	 
	        sprintf(testBuf,"FwUpdateObj.FirmwareCompleted=%x\n",FwUpdateObj.FirmwareCompleted);
			UdpDebugPrint(testBuf,strlen(testBuf));
		#endif
	       
		}
    }

    iLength = sprintf(AckMsgArray,"%02X%s{\"code\":%d,\"num\":%d}", 
                                  (SWTICH_UPDATE_FW_UDP_81_CMD), 
                                  DevInfoObj.dev_sn, 
                                  (200<<iAck), iNumPackage ); 
    AckMsgArray[iLength] = '\0'; 
    SendUdpData(AckMsgArray, iLength); 

	//u_printf("AckMsgArray=%s\n",AckMsgArray);
    return 0;
} 


/*******************************************************************************
* 功能描述: UDP设置wifi模式
* 输入参数: 
* 输出参数: 
* -返回值-: 
* 使用说明: 
*******************************************************************************/
static uint8_t UDPSetWifiMode(char* pArgRecvBuf) 
{
    uint8_t iAck = 0;
	int     iLength;
	char    AckMsgArray[100]={0}; 
	
    iAck = SetWifiMode(pArgRecvBuf);
    iLength = sprintf(AckMsgArray,"V3{\"sn\":\"%s\",\"response\":%d,\"code\":%d}", 
											DevInfoObj.dev_sn,
										  (SWITCH_GET_DEVICE_REGID_UDP_73_CMD), 
										  (200<<iAck)); 

	SendUdpData(AckMsgArray, iLength);
 
    return 0;
}



/**************************************************************************************************
* 功能描述: 搜寻附近的wifi信号
* 输入参数: 
* 输出参数: 
* -返回值-: 
* 使用说明: 
****************************************************************************************************/
static uint8_t UdpScanNearbyWifiSignal(char* pArgRecvBuf) 
{ 
    int iAck = 0, iLength;
	char AckMsgArray[1400];
    
    iLengthHttpAckBuf = HTTP_SERVER_ACK_LENGTH-3-1; 
    

    pHttpAckBuf  = AckMsgArray;
    iLength      = sprintf(pHttpAckBuf,"V3{\"response\":%d,"
                                    "\"code\":200,"
                                    "\"data\":{\"signal\":[", SWITCH_GET_WIFI_SIGNAL_UDP_72_CMD);
    pHttpAckBuf  += iLength; 

    iLengthHttpAckBuf -= iLength; 
    
	iAck         = hfwifi_scan(hfwifi_scan_callback); 
	if(iAck < 0)
	{
		memset(AckMsgArray,0,sizeof(AckMsgArray));
        iLength  = sprintf(AckMsgArray,"V3{\"response\":%d,\"code\":400}", SWITCH_GET_WIFI_SIGNAL_UDP_72_CMD);
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
	SendUdpData(AckMsgArray, sizeof(AckMsgArray)-1);
	
	UdpDebugPrint(AckMsgArray, sizeof(AckMsgArray)-1);
 
	return 0;
} 




/*******************************************************************************
* 功能描述:改变机器的Product ID,Seral number,Mac address
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/
static uint8_t UdpModifyProductSn(char* pArgRecvBuf) 
{
    char    iAck = 0, isSaveSakSn;
    char    AckMsgArray[100]={0};
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
    iLength = sprintf(AckMsgArray,"V3{\"sn\":\"%s\",\"response\":%d,\"code\":%d}",		
								DevInfoObj.dev_sn,
								  (SWITCH_SET_DEVICE_MAC_UDP_74_CMD), 
								  (200<<iAck));

	SendUdpData(AckMsgArray, iLength);
	return 0;
} 



/*******************************************************************************
* 功能描述: 手机获取时实时功耗和开关状态
* 输入参数: 
* 输出参数: 
* -返回值-: 
* 使用说明: 
*******************************************************************************/
extern PowerRecordType PowerRecordObj[PORT_NUM];

static uint8_t UdpGetPowerConsume(char* pArgRecvBuf)
{
    int     iLength;
    
    uint8_t iAck = 0;
    uint8_t portID = 0;
    char *pData,*pTemp;
	char  pAckBuf[200]={0};

    pData = pAckBuf;
    pTemp = pData;

    iLength = sprintf(pTemp,  "V3{\"response\":%d,\"code\":%d,\"data\":{\"watt\":[",SWITCH_GET_POWER_CONSUME_UDP_90_CMD, (200<<iAck));
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

	SendUdpData(pAckBuf, strlen(pAckBuf));
    return 0; 
                        
}





/**************************************************************************************************
* 功能描述: 计算电流、电压、功率的校准值
* 入口参数: 
* 出口参数: 
* 使用说明: 可以通过串口命令设置从机的校准值，也可以读出从机的原始值在本机计算校准
***************************************************************************************************/
static uint8_t UdpCalibratePowerConsume(char* pArgRecvBuf) 
{

    cJSON *pJson;
	char    AckMsgArray[100]={0};
    int    iValue,portID,iRslt = 1;
	int    iLength;

    pJson = cJSON_Parse(pArgRecvBuf); 
    if(pJson) 
    { 
        portID = cJSON_GetObjectItem(pJson, "port")->valueint; 
        portID = portID > PORT_NUM ? PORT_NUM : portID;

       #if(PORT_NUM == 6)
        CalibrateData[0] = portID;        
        PowerRecordObj[portID].PowerCoefficent = cJSON_GetObjectItem(pJson, "watt")->valueint; 

        CalibrateData[1] = (PowerRecordObj[portID].PowerCoefficent&0xff000000)>>24;
        CalibrateData[2] = (PowerRecordObj[portID].PowerCoefficent&0x00ff0000)>>16;
        CalibrateData[3] = (PowerRecordObj[portID].PowerCoefficent&0x0000ff00)>>8;
        CalibrateData[4] = PowerRecordObj[portID].PowerCoefficent&0xff;
        
        PowerRecordObj[portID].VoltCoefficent = cJSON_GetObjectItem(pJson, "vol")->valueint; 

        CalibrateData[7] = (PowerRecordObj[portID].VoltCoefficent&0xff);
        
        PowerRecordObj[portID].CurrentCoefficent = cJSON_GetObjectItem(pJson, "amp")->valueint; 

        CalibrateData[5] = (PowerRecordObj[portID].CurrentCoefficent&0xff00)>>8;
        CalibrateData[6] = (PowerRecordObj[portID].CurrentCoefficent&0xff);
       #elif(PORT_NUM == 1)
        iValue = cJSON_GetObjectItem(pJson, "watt")->valueint;
        PowerRecordObj[0].PowerCoefficent   = reco_measure_data.reco_powerp*10 / iValue;
        iValue = cJSON_GetObjectItem(pJson, "vol")->valueint;
        PowerRecordObj[0].VoltCoefficent    = reco_measure_data.reco_urms*10 / iValue;
        iValue = cJSON_GetObjectItem(pJson, "amp")->valueint; 
        PowerRecordObj[0].CurrentCoefficent = reco_measure_data.reco_irms*10 / iValue;
       #endif

        cJSON_Delete(pJson); 
        
        SavePowerCoefficent();
        UsartCalibrationPort();
        if(PowerRecordObj[portID].PowerCoefficent && PowerRecordObj[portID].VoltCoefficent && PowerRecordObj[portID].CurrentCoefficent)
        {
            iRslt = 0;
        }

        //u_printf("portID = %d\n",portID);
     }
	 iLength = sprintf(AckMsgArray,"V3{\"sn\":\"%s\",\"response\":%d,\"code\":%d}",		
								  DevInfoObj.dev_sn,
								  (SWITCH_CALIBRATE_POWER_CONSUME_UDP_75_CMD), 
								  (200<<iRslt));

	SendUdpData(AckMsgArray, iLength);
	
    return 0; 
}




/*******************************************************************************
* 功能描述:
* 入口参数:
* 出口参数:
* 使用说明:
* 调用方法:
*******************************************************************************/
static uint8_t Udp_TurnOnOffSwitch(char *pBuf) 
{
    int     iLength;
    char    AckMsgArray[100]={0};
    uint8_t iRslt;
    
	iRslt   = Switch_Ctl(pBuf);
    iLength = sprintf(AckMsgArray, "V3{\"sn\":\"%s\",\"response\":%d,\"code\":%d}",
									DevInfoObj.dev_sn,
                                   (SWITCH_ON_OFF_CTL_UDP_14_CMD), 
                                   (200<<iRslt)); 
    SendUdpData(AckMsgArray, iLength); 

   // UdpDebugPrint("Now report OnOff reback\r\n", strlen("Now report OnOff reback\r\n"));
   // UdpDebugPrint(AckMsgArray, iLength);
    return 0;
}

/******************************************************************************* 
* 功能描述:服务器修改Sensor名字 
* 入口参数:
* 出口参数:
* 使用说明:
* 调用方法:
*******************************************************************************/ 
static uint8_t UdpModifyName(char *pBuf)
{ 
    int         iLength; 
    char        AckMsgArray[100]={0}; 
    uint8_t     iResult        = 0; 
    char        *pTempData     = pBuf; 
    
    iResult = ModifyDevName(pTempData);
    iLength = sprintf(AckMsgArray,"V3{\"sn\":\"%s\",\"response\":%d,\"code\":%d}",
									DevInfoObj.dev_sn,
                                  (SWITCH_RENAME_UDP_18_CMD), 
                                  (200<<iResult)); 

    SendUdpData(AckMsgArray, iLength); 

    return 0; 
} 



/*******************************************************************************
* 功能描述:服务器转发关联控制命令
* 入口参数:
* 出口参数:
* 使用说明:
* 调用方法:
*******************************************************************************/
static uint8_t UdpRelatedControlSwitch(char *pBuf) 
{
    int     iLength;
    char    AckMsgArray[100]={0};
    uint8_t iRslt = 0;
    
	iRslt   = Switch_RelatedCtl(pBuf);
	
    iLength = sprintf(AckMsgArray, "V3{\"sn\":\"%s\",\"response\":%d,\"code\":%d}", 
									DevInfoObj.dev_sn,
                                   (SWITCH_RELATED_CONTROL_UDP_28_CMD), 
                                   (200<<iRslt)); 

    SendUdpData(AckMsgArray, iLength); 
#if 0

    UdpDebugPrint("Now report OnOff reback\r\n", strlen("Now report OnOff reback\r\n"));
    UdpDebugPrint(AckMsgArray, iLength);
#endif
    return 0;
}



/******************************************************************************* 
* 功能描述:服务器下发Reboot指令 
* 入口参数:
* 出口参数:
* 使用说明:
* 调用方法:
*******************************************************************************/ 
static uint8_t UdpSetDevReboot(char *pBuf)
{ 
    int         iLength; 
    char        AckMsgArray[100]={0}; 
	uint8_t		port_num=0,j=0,en=0, iRet;
	cJSON   	*json,*port;

	json = cJSON_Parse(pBuf);
	iRet = FAILED; 
	if(json)
	{

		en   		= cJSON_GetObjectItem(json, "en")->valueint;
		port        = cJSON_GetObjectItem(json, "port");
		port_num    = cJSON_GetArraySize(port);		
		for(j=0; j<port_num; j++)
		{
			SwInformation.PortInfor[j].rebootPort= cJSON_GetArrayItem(port, j)->valueint;
			//u_printf("port[%d] = %d\n",ruleID,cJSON_GetArrayItem(port,j)->valueint);
	    }
		SwInformation.isRebootEnable = en;
			
		iRet = PASSED;

		reportRebootInfo = 1;
		SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_14_SW_STATE_IS_DIRTY_CMD);   //save the master status to flash
		SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_17_REPORT_PORT_REBOOT_CMD);	
        
		cJSON_Delete(json); 
	}
	
	iLength = sprintf(AckMsgArray,"V3{\"sn\":\"%s\",\"response\":%d,\"code\":%d}", 
											DevInfoObj.dev_sn,
										  (SWITCH_SET_DEV_REBOOT_1A_CMD), 
										  (200<<iRet)); 
    
    
    SendUdpData(AckMsgArray, iLength); 

    return 0; 
} 




/******************************************************************************* 
* 功能描述:
* 入口参数:
* 出口参数:
* 使用说明:
* 调用方法:
*******************************************************************************/ 
#if defined(__UDP_NET_STATE_PRINT_DBG__) && defined(__UDP_PRINT_DBG__)
static uint8_t UdpNetStateDebug(char *pBuf)
{ 
    char        AckMsgArray[AT_ACK_MSG_LENGTH]={0}; 
    
    hfat_send_cmd("AT+WSSSID\r\n", sizeof("AT+WSSSID\r\n")-1, AckMsgArray, AT_ACK_MSG_LENGTH);
    UdpDebugPrint("WSSSID:\r\n", sizeof("WSSSID:\r\n")-1);
    UdpDebugPrint(AckMsgArray, strlen(AckMsgArray));
    
    memset(AckMsgArray, '\0', AT_ACK_MSG_LENGTH);
    hfat_send_cmd("AT+WSKEY\r\n", sizeof("AT+WSKEY\r\n")-1, AckMsgArray, AT_ACK_MSG_LENGTH);
    UdpDebugPrint("WSKEY:\r\n", sizeof("WSKEY:\r\n")-1);
    UdpDebugPrint(AckMsgArray, strlen(AckMsgArray));
    // 
    return 0; 
} 
#endif 

/******************************************************************************* 
* 功能描述:
* 输入参数:
* 输出参数:
* -返回值-: 
* 使用说明:
*******************************************************************************/ 
uint8_t UdpSetHostSlave(char *pDataBuf)
{    
    cJSON *pJson,*master;
    uint8_t num, portID;
    char    AckMsgArray[40]={0};
    int     iLength,iRet;
    
    iRet = 1;
    pJson = cJSON_Parse(pDataBuf); 
    if(pJson) 
    { 
        master = cJSON_GetObjectItem(pJson,"master");
		num = cJSON_GetArraySize(master);
		for(portID=0;portID<num;portID++)
		{
			SwInformation.PortInfor[portID].msType = cJSON_GetArrayItem(master,portID)->valueint;
			//u_printf("PortInfor[%d].msType = %d\n",portID,SwInformation.PortInfor[portID].msType);
		}
		
		SwInformation.LimitData = cJSON_GetObjectItem(pJson,"limit")->valueint;
		switch (SwInformation.LimitData)
		{
		    case 0:
		    {
		        SwInformation.masterLimitWatt = 2;
		        break;
		    }
		    case 1:
		    {
		        SwInformation.masterLimitWatt = 5;
		        break;
		    }
		    case 2:
		    {
		        SwInformation.masterLimitWatt = 10;
		        break;
		    }
		    case 3:
		    {
		        SwInformation.masterLimitWatt = 35;
		        break;
		    }
		    default:
		    {
		        break;
		    }
		}
		iRet = 0;
        
		cJSON_Delete(pJson); 

        //u_printf("masterLimitWatt=%d\n",SwInformation.masterLimitWatt);
        
        SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_14_SW_STATE_IS_DIRTY_CMD);   //save the master status to flash
        SaveDevStatusPoll();

        iLength = sprintf(AckMsgArray, "V3{\"response\":%d,\"code\":\"200\"}", SWITCH_SET_PORT_HOST_SLAVE_15_CMD); 

		SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_16_REPORT_MASTER_SLAVE_CMD);	
		reportMasterSlaveInfo = 1;
        SendUdpData(AckMsgArray, iLength);

	} 
	return iRet;
}



uint8_t findMasterPort(uint8_t *masterID)
{
	uint8_t portID = 0;
	 	
	for(portID=0;portID<PORT_NUM;portID++)
    {    
        if(SwInformation.PortInfor[portID].msType == 1)
        {
            *masterID = portID; 
			return 1;
		}	
	}

	return 0;  
}



void masterSlaveControl(void)
{
    uint8_t i,j,masterID = 0;
    static uint8_t msCurState=0,msLastState=0;
	//char testBuf[100];

	if(findMasterPort(&masterID))
	{
		if((PowerRecordObj[masterID].record_ValidPower/1000+0.5) >= SwInformation.masterLimitWatt)     //主用电器开启
	    {
	       msCurState = 1;
		   //UdpDebugPrint("The master portID turned on!\n",sizeof("The master portID turned on!\n")-1);
	    }
	    else                            //主用电器关闭
	    {
	       msCurState = 0;
		   //UdpDebugPrint("The master portID turned off!\n",sizeof("The master portID turned off!\n")-1);
	    }

		if(msLastState != msCurState)
		{			
			if(msCurState == 1)
			{
				for(j=0;j<PORT_NUM;j++)
				{
					if(SwInformation.PortInfor[j].msType==0 && (SwInformation.PortInfor[j].OnOffStatus==0))
					{
						SetPortState(j+1, 1);
						SwInformation.portChangeReason = PORT_CHANGE_BY_MSCONTROL;
						//UdpDebugPrint("\r\nNow excute master_slave On control",sizeof("\r\nNow excute master_slave On control")-1);
						//u_printf("Now turn on the port:%d\n",j);
					}
				}
				//UdpDebugPrint("\r\nNow turn on the device",sizeof("\r\nNow turn on the device")-1);
			}
			else
			{
				for(j=0;j<PORT_NUM;j++)
				{
					if(SwInformation.PortInfor[j].msType==0 && (0x01 == SwInformation.PortInfor[j].OnOffStatus))
	                {
	                    SetPortState(j+1, 0);
						SwInformation.portChangeReason = PORT_CHANGE_BY_MSCONTROL;
	                    //UdpDebugPrint("\r\nNow excute master_slave Off control",sizeof("\r\nNow excute master_slave Off control")-1);
	                    //u_printf("Now turn off the port:%d\n",j);
	                }
	            }
	            //UdpDebugPrint("\r\nNow turn off the device",sizeof("\r\nNow turn off the device")-1);
			}
		#if 0
			memset(testBuf,0,sizeof(testBuf));
			sprintf(testBuf,"\n masterID = %d	masterLimitWatt = %d\t msLastState=%d\t msCurState=%d\n",masterID,SwInformation.masterLimitWatt,msLastState,msCurState);
			UdpDebugPrint(testBuf,sizeof(testBuf));
		#endif

			for(i=0;i<PORT_NUM;i++)
		    {
		        if(SwInformation.PortInfor[i].OnOffStatus)
		        {
		            SwOnOffData |= SwInformation.PortInfor[i].OnOffStatus<<i;
		        }
		        else
		        {
		            SwOnOffData &= ~((!SwInformation.PortInfor[i].OnOffStatus)<<i);
		        }
		    }

	        UsartControlPortOnOff(); 
			msleep(500);
			UsartControlPortOnOff(); 
			msLastState = msCurState;

		}
	}
        
	//UdpDebugPrint("\r\nNow excute master_slave control",sizeof("\r\nNow excute master_slave control")-1);
}

/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/

