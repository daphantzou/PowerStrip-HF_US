/*
********************************************************************************
* @file    : Flash.c 
* @author  : 
* @version : V1.0.0 
* @date    : 2014/06/27 
* @brief   : 
********************************************************************************
*/
#include <stdio.h> 
#include <string.h> 

#include <hsf.h>
#include "ConfigAll.h" 
#include "DevDataType.h" 
#include "DevLib.h"
#include "DevInfo.h" 
#include "DevRule.h" 
#include "SwitchApp.h"
#include "PowerStaticsRecord.h"
#include "flash.h" 
#include "LED.h"
#include "HttpdCallBack.h"
#include "read_power.h"
#include "security_mode.h"
#include "SwitchApp.h" 



uint8_t isGetRegid = 0;             //是否获取到了设备账号
uint8_t factoryProgramMark = 0;			//工厂是否烧录
uint8_t curWifiStatus = 0;			//当前wifi状态
uint8_t	lastWifiStatus = 0;			//wifi前次状态



extern uint8_t             isDevReg;                                           // 是否在公网注册 


int     DevModeStatus = 0;
int     DevBootFactory = 0;




extern DevStatusType           DevStatusObj;
extern FwUpdateStatusType      FwUpdateStatusObj; 
extern DevRegInfoType          DevRegInformation;                                      //设备注册信息

/*******************************************************************************
* 功能描述: 
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/ 
void DevFlash_init(void) 
{
    DevBootFactory = 0x00;
}

/*************************************************************************************************
* 功能描述: 
* 入口参数: 
* 出口参数: 
* 使用说明: 
**************************************************************************************************/ 
#define     isInClientMode(BootMode)              (BOOT_CHG_CLIENT_MODE_FLAG == BootMode)
#define     isSmartLinkGotNetStatusMode(BootMode) (BOOT_CHG_CLIENT_SMTLK_MODE_FLAG == BootMode)
#define     isPrepareEnterSmartLinkMode(BootMode) (BOOT_CHG_CLIENT_SMTLK_MODE_FLAG == BootMode)
#define     SetEnterSmartLinkMode()               (BOOT_CHG_CLIENT_SMTLK_MODE_FLAG)
#define     EnteringClientMode()                  (0xFFFFFFFF)
#define     isDefaultMode(BootMode)               (BOOT_DEFAULT_MODE_FLAG == BootMode)


/******************************************************************************* 
* 功能描述: 重启设备wifi功能
* 输入参数:
* 输出参数:
* -返回值-:
* 使用说明:
*******************************************************************************/ 
void ResetWifiMode(void)
{
    char     AtAckMsg[BOOT_INFO_DATA_BUF_LENGTH+1]={0};
	
    hfat_send_cmd("AT+WIFI=down\r\n", sizeof("AT+WIFI=down\r\n")-1, AtAckMsg, 19);
	hfat_send_cmd("AT+WMODE=STA\r\n", sizeof("AT+WMODE=STA\r\n")-1, AtAckMsg, 19);
	hfat_send_cmd("AT+WIFI=up\r\n", sizeof("AT+WIFI=up\r\n")-1, AtAckMsg, 19);

	LedCtl(WIFI_LED_ON);
}



/***********************************************************************************************************
* 功能描述: 初始化WiFi规工作模式
* 入口参数: 
* 出口参数: 
* 使用说明: 若开机时设备处于STA模式，则不做处理；若开机时处于AP模式，则发送AT命令进入smart_link模式
*****************************************************************************************************************/ 
void DevMode_Init(void) 
{
    char     AtAckMsg[BOOT_INFO_DATA_BUF_LENGTH+1]={0};
	char 	 AtBuffer[64] = {0};
    char    *pS8Tmp;
	uint8_t  iLength = 0;


	#if (PORT_NUM == 1)
	 hfat_send_cmd("AT+WRMID=SmartPlug\r\n", sizeof("AT+WRMID=SmartPlug\r\n")-1, AtAckMsg, 19);
	#elif (PORT_NUM == 6)
	 //hfat_send_cmd("AT+WRMID=SmartStrip\r\n", sizeof("AT+WRMID=SmartStrip\r\n")-1, AtAckMsg, 19);
	 iLength = sprintf(AtBuffer, "AT+WRMID=%s\r\n", DevInfoObj.dev_sn);       
     hfat_send_cmd(AtBuffer, iLength, AtAckMsg, BOOT_INFO_DATA_BUF_LENGTH); 
	#endif

    hfat_send_cmd("AT+WMODE\r\n", sizeof("AT+WMODE\r\n")-1, AtAckMsg, 19);
    pS8Tmp = strstr(AtAckMsg, "AP"); 

	//u_printf("AtAckMsg = %s\n",AtAckMsg);
	//u_printf("factoryProgramMark = %d\n",factoryProgramMark);
	
	//没有找到"AP"
    if(NULL == pS8Tmp)
    {
        SetBit(DevStatusObj.LedShowStatus, WIFI_LED_SHOW_0_BIT);            // Client LED Show 
        DevModeStatus = CLIENT_MODE_LED;
        //SetBit(DevStatusObj.LedShowStatus, WIFI_LED_SHOW_0_BIT);            // Client LED Show 

    }
    else
    {
		//if(NULL == strstr(AtAckMsg, "APSTA"))
		{
	        if(!factoryProgramMark)
			{
				hfat_send_cmd("AT+WMODE=STA\r\n", sizeof("AT+WMODE=STA\r\n")-1, AtAckMsg, 48);
				hfat_send_cmd("AT+WSSSID=UPGRADE-AP\r\n", sizeof("AT+WSSSID=UPGRADE-AP\r\n")-1, AtAckMsg, 48);
				//hfat_send_cmd("AT+WSKEY=WPA2PSK,AES,revogi2013\r\n", sizeof("AT+WSKEY=WPA2PSK,AES,revogi2013\r\n")-1, AtAckMsg, 48);
				
				hfat_send_cmd("AT+Z\r\n", sizeof("AT+Z\r\n")-1, AtAckMsg, 48); 
				
			}
			else
			{
	        	hfat_send_cmd("AT+SMTLK\r\n", sizeof("AT+SMTLK\r\n")-1, AtAckMsg, BOOT_INFO_DATA_BUF_LENGTH);       //进入smart_link模式
	        	//DevModeStatus = SMTLINK_MODE_LED;
			}

	      #if 0
	        //恢复出场后，要打开端口开关
	        #if (PORT_NUM == 6)
	    	  Switch_TurnOnOff(0, 1);
	    	#elif (PORT_NUM == 1)
	    	  Port_TurnOnOff(1);
	    	#endif
	      #endif
		}
    }
}


/***********************************************************************************************************
* 功能描述: 检测模块状态
* 入口参数: 
* 出口参数: 
* 使用说明: 每5秒检测一次
*****************************************************************************************************************/ 
extern DevInfoType DevInfoObj;
void CheckDevMode(void) 
{
	char	 AtAckMsg[BOOT_INFO_DATA_BUF_LENGTH+1]={0};
	char	*pS8Tmp;

	hfat_send_cmd("AT+WMODE\r\n", sizeof("AT+WMODE\r\n")-1, AtAckMsg, 19);
	pS8Tmp = strstr(AtAckMsg, "AP");				//没有找到"AP"

	
	if(NULL == pS8Tmp)
	{
		ReadDevIP();
		if(DevInfoObj.dev_ip[0]==0)
		{
			curWifiStatus = 0;
			//DevModeStatus = AP_MODE_LED;
		}
		else if(DevInfoObj.dev_ip[0]!=0)									//网络正常
		{	
			curWifiStatus = 1;		
		    ClrBit(DevStatusObj.LedShowStatus, WIFI_LED_SHOW_0_BIT);
			
			LedCtl(WIFI_LED_ON);
			//u_printf("Now the device network ok!!!!\n");
		}

	#if 1
		if(lastWifiStatus != curWifiStatus)		
		{		
			if(curWifiStatus == 1)				//网络变为正常
			{
				
				ClrBit(DevStatusObj.LedShowStatus, WIFI_LED_SHOW_0_BIT);
				LedCtl(WIFI_LED_ON);			
			}
			else
			{
				//DevModeStatus = AP_MODE_LED;
				
				SetBit(DevStatusObj.LedShowStatus, WIFI_LED_SHOW_0_BIT);			// Client LED Show 
				//LedBlink(1);
			}
			lastWifiStatus = curWifiStatus;
		}
	#endif
	}
}



/***************************************************************************************************** 
* 功能描述: 设备切换到APSTA共存模式
* 入口参数: 
* 出口参数: 
* 使用说明: 
******************************************************************************************************/ 
void EnterAPSTAMode(void)
{
	char	 AtAckMsg[BOOT_INFO_DATA_BUF_LENGTH+1]={0};
	hfat_send_cmd("AT+WMODE=APSTA\r\n", sizeof("AT+WMODE=APSTA\r\n")-1, AtAckMsg, 48);
}


/***************************************************************************************************** 
* 功能描述: 清除设备中保存的工作模式、状态等信息
* 入口参数: 
* 出口参数: 
* 使用说明: flash页起始4个字节用于保存标志位，该标志位被清零后数据将被丢掉
******************************************************************************************************/ 
void ClearDevSmartLinkInfo(void)
{
    hfsys_reload();
	//msleep(500);
    hfsys_reset();
}

/***************************************************************************************************** 
* 功能描述: 2KB 用于保存设备名称，端口名称和设备名称及主从信息
* 入口参数: pDataBuf:保存设备名称的数组
* 出口参数: 
* 使用说明: flash起始4个字节用于保存标志位，该标志位可用于判断是否保存了设备名称，为0x12121212
******************************************************************************************************/ 
extern DevInfoType DevInfoObj;
void WriteDevName()
{
    char    DevInfoFlashArr[64+4+1+NAME_LENGTH*PORT_NUM] = {0}; 
    char    *pBuf; 
    int     *pTmp; 
    
    pTmp  = (int*)DevInfoFlashArr; 
    *pTmp = FLASH_DEV_NAME_FLAG; 
    // 
    pBuf  = DevInfoFlashArr; 
    pBuf += FLASH_DEV_NAME_OFFSET; 
    memcpy(pBuf, DevInfoObj.dev_name,NAME_LENGTH); 
    pBuf += NAME_LENGTH;

   #if (PORT_NUM == 6)
    uint8_t portID = 0;
    for(portID=0;portID<PORT_NUM;portID++)
    {
        memcpy(pBuf,(char*)SwInformation.PortInfor[portID].name,NAME_LENGTH);
        pBuf += NAME_LENGTH; 
    }
   #endif
    hfuflash_erase_page(FLASH_DEV_NAME_BUF_START, 1);           //erase one page :2KB
    hfuflash_write(FLASH_DEV_NAME_BUF_START, DevInfoFlashArr, sizeof(DevInfoFlashArr)); 


}

/***************************************************************************************************** 
* 功能描述: 从flash中读出设备名字
* 入口参数: 
* 出口参数: DevInfoObj:全局变量，用于保存设备名称
* 使用说明: flash起始4个字节用于保存标志位，该标志位可用于判断是否保存了设备名称，为0x12121212
******************************************************************************************************/ 
void ReadDevName(void)
{
    char    DevInfoFlashArr[NAME_LENGTH*(PORT_NUM+1)+1+4] = {0};      //port name and device name
    char    *pTmpBuf;
    int     *pTmp;
    uint8_t portID = 0;
    
    hfuflash_read(FLASH_DEV_NAME_BUF_START, DevInfoFlashArr, sizeof(DevInfoFlashArr));
    pTmp = (int*)DevInfoFlashArr;
    if(FLASH_DEV_NAME_FLAG == *pTmp)
    {
        pTmpBuf  = &DevInfoFlashArr[0];
        pTmpBuf += FLASH_DEV_NAME_OFFSET;
        memcpy(DevInfoObj.dev_name, pTmpBuf, NAME_LENGTH);
        //read the port name 
        pTmpBuf += NAME_LENGTH;
        for(portID=0;portID<PORT_NUM;portID++)
        {
            memcpy(SwInformation.PortInfor[portID].name,pTmpBuf,NAME_LENGTH);
            pTmpBuf += NAME_LENGTH; 
            //u_printf("PortInfor[%d] = %s\n",portID,SwInformation.PortInfor[portID].name);
        }
    }
    else
    {
        //memcpy(DevInfoObj.dev_name, DEV_DEFAULT_NAME, sizeof(DEV_DEFAULT_NAME)-1); 
        memcpy(DevInfoObj.dev_name, dev_default_name, NAME_LENGTH); 
       #if (PORT_NUM == 6)
        memcpy(SwInformation.PortInfor[0].name, "PORT1", sizeof("PORT1")-1);
        memcpy(SwInformation.PortInfor[1].name, "PORT2", sizeof("PORT2")-1);
        memcpy(SwInformation.PortInfor[2].name, "PORT3", sizeof("PORT3")-1);
        memcpy(SwInformation.PortInfor[3].name, "PORT4", sizeof("PORT4")-1);
        memcpy(SwInformation.PortInfor[4].name, "PORT5", sizeof("PORT5")-1);
        memcpy(SwInformation.PortInfor[5].name, "PORT6", sizeof("PORT6")-1);
       #endif
    }
}

/***************************************************************************************************** 
* 功能描述: 写本地时区到flash中
* 入口参数: 
* 出口参数: DevInfoObj:全局变量，用于保存设备本地时区
* 使用说明: flash起始4个字节用于保存标志位，该标志位可用于判断是否保存了设备名称，为0x13131313
******************************************************************************************************/ 
void WriteDevTimeZone(void)
{
    char    DevInfoFlashArr[26+1] = {0}; 
    int     *pTmp; 
    
    pTmp  = (int*)DevInfoFlashArr; 
    *pTmp = FLASH_DEV_TIME_ZONE_FLAG; 
    // 
    pTmp++;
    *pTmp = DevInfoObj.TimeZone;

	//写设备工厂烧录标志
    pTmp  =  (int*)&DevInfoFlashArr[FLASH_FAC_PROGRAM_FLAG_OFFSET];//从第44个字节开始保存设备的username
   *pTmp  =  FLASH_FAC_PROGRAM_FLAG;              //先写入regid保存标志0xA55AA55A
    pTmp++;
    *pTmp   = factoryProgramMark;
	
    hfuflash_erase_page(FLASH_DEV_TIME_ZONE_BUF_START, 1); 
    hfuflash_write(FLASH_DEV_TIME_ZONE_BUF_START, DevInfoFlashArr, 26); 
}

/***************************************************************************************************** 
* 功能描述: 从flash里面读出本地时区
* 入口参数: 
* 出口参数: DevInfoObj:全局变量，用于保存设备本地时区
* 使用说明: flash起始4个字节用于保存标志位，该标志位可用于判断是否保存了设备名称，为0x13131313
******************************************************************************************************/ 
void ReadDevTimeZone(void)
{
    char    DevInfoFlashArr[26+1] = {0};
    int     *pTmp;
    
    hfuflash_read(FLASH_DEV_TIME_ZONE_BUF_START, DevInfoFlashArr, 26);
    pTmp = (int*)DevInfoFlashArr;
    if(FLASH_DEV_TIME_ZONE_FLAG == *pTmp)
    {
        pTmp++;
        DevInfoObj.TimeZone = *pTmp;
    }

	pTmp  =  (int*)&DevInfoFlashArr[FLASH_FAC_PROGRAM_FLAG_OFFSET];
	if(FLASH_FAC_PROGRAM_FLAG == *pTmp)
    {
        pTmp++;
        factoryProgramMark = *pTmp;
    }
	
    //u_printf("TimeZone = %d		factoryProgramMark = %d\n",DevInfoObj.TimeZone,factoryProgramMark);
}

/******************************************************************************************************
* 功能描述: 写产品唯一标准，如sak,SN码等
* 入口参数: 
* 出口参数: 
* 使用说明: flash起始4个字节用于保存标志位，该标志位可用于判断是否保存了设备名称为0x55AA55AA
******************************************************************************************************/ 
extern DevInfoType DevInfoObj; 
extern void WriteDevSakSN(void)
{
    char    DevInfoFlashArr[128+1] = {0}; 
    char    *pS8Buf; 
    int     *pS32Tmp; 
    
    // 
    pS32Tmp  = (int*)DevInfoFlashArr; 
    *pS32Tmp = FLASH_DEV_FLAG; 
    pS8Buf   = &DevInfoFlashArr[FLASH_SAK_OFFSET];          //从第8个字节开始保存设备的sak码
    memcpy(pS8Buf, DevInfoObj.dev_sak, SAK_LEN);
    // 
    pS32Tmp  =  (int*)&DevInfoFlashArr[FLASH_SN_FLAG_OFFSET];//从第24个字节开始保存设备的SN码，
   *pS32Tmp  =  FLASH_SN_FLAG;              //先写入SN码标志0x5A5A5A5A
    pS32Tmp++;
    pS8Buf   = (char*)pS32Tmp;
    memcpy(pS8Buf, DevInfoObj.dev_sn, SERIAL_NUN_ID_LEN);

    hfuflash_erase_page(FLASH_DEV_HARD_INFO_BUF_START, 1); 
    hfuflash_write(FLASH_DEV_HARD_INFO_BUF_START, DevInfoFlashArr, 128); 
}

/******************************************************************************************************
* 功能描述: 从flash中读出设备的sak,SN码等
* 入口参数: 
* 出口参数: DevInfoObj:结构体全局变量，用于保存设备的sak/sn/和mac参数
* 使用说明: flash起始4个字节用于保存标志位，该标志位可用于判断是否保存了设备名称为0x55AA55AA
******************************************************************************************************/ 
extern void ReadDevSakSN(void)
{
    char    DevInfoFlashArr[128+1] = {0};
    char    *pS8TmpBuf;
    int     *pS32Tmp;
   #if  defined(HANK_JOE_CUSTOMER_REMOTE_CTL) 
    char    iWrFlg;
    iWrFlg = 1;
   #endif 
    
    hfuflash_read(FLASH_DEV_HARD_INFO_BUF_START, DevInfoFlashArr, 128);
    pS32Tmp = (int*)DevInfoFlashArr;
    if(FLASH_DEV_FLAG == *pS32Tmp)
    { 
        pS8TmpBuf  = &DevInfoFlashArr[FLASH_SAK_OFFSET];
        memcpy(DevInfoObj.dev_sak, pS8TmpBuf, SAK_LEN);
    } 
   #if  defined(HANK_JOE_CUSTOMER_REMOTE_CTL) 
    else
    {
        iWrFlg = 0x00;                  // default 
    }
   #endif 
    // 
    pS32Tmp    = (int*)&DevInfoFlashArr[FLASH_SN_FLAG_OFFSET];
    if(FLASH_SN_FLAG == *pS32Tmp)
    { 
        pS32Tmp++;
        pS8TmpBuf = (char*)pS32Tmp; 
        memcpy(DevInfoObj.dev_sn, pS8TmpBuf, SERIAL_NUN_ID_LEN);
    } 
#if 0
    pS32Tmp    = (int*)&DevInfoFlashArr[FLASH_REGID_FLAG_OFFSET];
    if(FLASH_REGID_FLAG == *pS32Tmp)
    { 
        pS32Tmp++;
        pS8TmpBuf = (char*)pS32Tmp; 
        memcpy(DevInfoObj.userName, pS8TmpBuf, NAME_LENGTH);
        isGetRegid = 1;             //已经获取到了设备的用户名
    } 
#endif    
   #if  defined(HANK_JOE_CUSTOMER_REMOTE_CTL) 
    else
    {
        iWrFlg = 0x00;                  // default 
    }
    if(0x00 == iWrFlg)
    {
        WriteDevSakSN();                // default 
    }
   #endif 
}






/******************************************************************************************************
* 功能描述: 写设备从服务器的URL，是否绑定成功标志
* 入口参数: 
* 出口参数: 
* 使用说明: flash起始4个字节用于保存标志位，该标志位可用于判断是否保存了设备名称为0x14141414
******************************************************************************************************/
extern FwUpdateStatusType      FwUpdateStatusObj;
extern void WriteDevBindInfo(void)
{
    char    DevInfoFlashArr[256] = {0}; 
    char    *pS8Buf; 
    int     *pS32Tmp; 

    if(isGetRegid)             //已经获取到了设备的用户名
    {
        pS32Tmp  = (int*)DevInfoFlashArr; 
        *pS32Tmp = FLASH_REGID_FLAG; 
        //写设备username
        pS8Buf  =  (char*)&DevInfoFlashArr[FLASH_REGID_FLAG_OFFSET];//从第8个字节开始保存设备的username
        memcpy(pS8Buf, DevRegInformation.regidName, NAME_LENGTH);
     

        if(isDevReg == IS_ON)
        {
            //设备是否绑定成功标志
            pS32Tmp  = (int*)&DevInfoFlashArr[FLASH_DEV_BIND_FLAG_OFFSET];
            *pS32Tmp = FLASH_DEV_BIND_FLAG;
            pS32Tmp++;
            pS8Buf   = (char *)pS32Tmp;
            *pS8Buf =  isDevReg;

            //写次服务器的端口号
            pS32Tmp  = (int *)&DevInfoFlashArr[FLASH_DEV_BIND_PORT_OFFSET];
            *pS32Tmp  = DevRegInformation.iSockPort;                   //端口
            
            pS8Buf  = (char *)&DevInfoFlashArr[FLASH_DEV_BIND_URL_OFFSET];                   //url
            memcpy(pS8Buf, DevRegInformation.SubSvrUrl, SLAVE_SERVER_URL_MAX_LENGTH);
        }
      
        hfuflash_erase_page(FLASH_DEV_BIND_BUF_START, 1); 
        hfuflash_write(FLASH_DEV_BIND_BUF_START, DevInfoFlashArr, 256); 
    }
}




void ReadDevStatus(void)
{
	char  *pBufStatus; 
	   
	pBufStatus = (char*)&SwInformation; 
	hfuflash_read(FLASH_DEV_STATUS_INFO_BUF_START, pBufStatus, sizeof(SwInformation));
	if(SwInformation.isDevStatusSaveFlg != FLASH_DEV_STATUS_FLAG) 
	{
	   memset(&SwInformation, 0, sizeof(SwInformation));
	}
	SwInformation.portChangeReason = PORT_NO_CHANGE;

#if(PORT_NUM == 1)
    
    Port_TurnOnOff(SwInformation.PortInfor[0].CurStatus);
	
    plug_updata = 1;
#endif
}



/****************************************************************************************************************
* 功能描述: 5次判断端口开关状态发送变化，则拉高标志位图位，用于写flash！
* 入口参数: 
* 出口参数: 
* 使用说明: 该函数用于按键控制开关的检测,滤除抖动
******************************************************************************************************************/ 
#define         SAVE_SW_STATE_AFTER_CHANGE_TIME_SEC         5 
uint8_t iSaveSwStateCount;

void CheckSwState(void)
{
    uint8_t portID = 0;
    for(portID=0;portID<portNum;portID++)
    {
        if(SwInformation.PortInfor[portID].OldStatus != SwInformation.PortInfor[portID].CurStatus)
        {
            iSaveSwStateCount++;
        }       
    }
    
    if(iSaveSwStateCount > SAVE_SW_STATE_AFTER_CHANGE_TIME_SEC) 
    {
        iSaveSwStateCount = 0x00; 
        SwInformation.PortInfor[portID].OldStatus = SwInformation.PortInfor[portID].CurStatus; 
        SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_14_SW_STATE_IS_DIRTY_CMD); 
    }
}


/****************************************************************************************************************
* 功能描述: 写flash会导致timer停跑，故写flash必须与timer有关的应用错开时间，该函数用于写flash！
* 入口参数: 
* 出口参数: 
* 使用说明: 该函数用于错开写flash与开关按键检测之间的时序
******************************************************************************************************************/
//extern int     DevModeStatus; 
void SaveDevStatusPoll(void)
{
    char    *pSaveBuf;
   // u_printf("DevModeStatus = %d\n",DevModeStatus);

    if(isBitSet(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_14_SW_STATE_IS_DIRTY_CMD))	    //写flash操作和按键检测会有冲突，需间隔开
    {
        ClrBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_14_SW_STATE_IS_DIRTY_CMD); 

       // u_printf("now we save the ms_information in flash!\n");
        
        pSaveBuf = (char*)&SwInformation; 
        SwInformation.isDevStatusSaveFlg = FLASH_DEV_STATUS_FLAG; 
        hfuflash_erase_page(FLASH_DEV_STATUS_INFO_BUF_START, 1); 
        hfuflash_write(FLASH_DEV_STATUS_INFO_BUF_START, pSaveBuf, sizeof(SwInformationType) ); 
    }
}



/******************************************************************************* 
* 功能描述: 从flash中读取设备的规则信息
* 入口参数: 
* 出口参数: DevRuleInfoObj:结构体全局变量，用于保存设备的规则信息
* 使用说明: 
*******************************************************************************/ 
extern DevRuleInfoType      DevRuleInfoObj; 
extern FwUpdateStatusType   FwUpdateStatusObj; 
void ReadDevRule(void) 
{ 
    char  *pBufRule; 
    
    pBufRule = (char*)&DevRuleInfoObj; 
    hfuflash_read(FLASH_DEV_RULE_INFO_BUF_START, pBufRule, sizeof(DevRuleInfoType));
    if(FLASH_DEV_RULE_FLAG != DevRuleInfoObj.isRuleSaveFlag) 
    {
        memset(&DevRuleInfoObj, '\0', sizeof(DevRuleInfoType));
    }
} 







/*********************************************************************************************************** 
* 功能描述: 将设备的规则信息写到flash中
* 入口参数: 
* 出口参数: 
* 使用说明: flash起始4个字节用于保存标志位，该标志位可用于判断是否保存了设备端口信息:为0x33553355
************************************************************************************************************/ 
void SaveDevRulePoll(void) 
{ 
    char  *pBufRule; 
    
    if(0x00 == isBitSet(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_13_RULE_IS_DIRTY_CMD))
    {
        return;
    }
    ClrBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_13_RULE_IS_DIRTY_CMD);
    pBufRule = (char*)&DevRuleInfoObj; 
    hfuflash_erase_page(FLASH_DEV_RULE_INFO_BUF_START, 1); 
    hfuflash_write(FLASH_DEV_RULE_INFO_BUF_START, pBufRule, sizeof(DevRuleInfoType)); 
} 


/************************************************************************************************************ 
* 功能描述: 恢复出厂设置
* 入口参数: 
* 出口参数: 
* 使用说明: 
*************************************************************************************************************/ 
void FlashResetFactory(void) 
{ 
    char     AtAckMsg[BOOT_INFO_DATA_BUF_LENGTH+1]={0}; 
    char     ModifyNameArr[64];
    char     *pTmp; 
    int      iLength; 
	
	
    
    hfat_send_cmd("AT+WSMAC\r\n", sizeof("AT+WSMAC\r\n")-1, AtAckMsg, 19); 
    pTmp    = strstr(AtAckMsg, "+ok=");
    memset(DevInfoObj.dev_name, '\0', NAME_LENGTH); 
    if(pTmp) 
    { 
        pTmp += sizeof("+ok=")-1;
        pTmp += 8; 
        #if (VENDOR_CODE == VENDOR_MCL) 
    	iLength = sprintf(ModifyNameArr, "AT+WAP=11BGN,PRISEWIFI-%s,CH1\r\n", pTmp); 
        #warning "Vendor is MCL, device name is PRISEWIFI, SSID is PRISEWIFI-xxxx" 
        #elif (VENDOR_CODE == VENDOR_REVOGI) 
    	iLength = sprintf(ModifyNameArr, "AT+WAP=11BGN,SmartStrip-%s,CH1\r\n", pTmp); 
        #warning "Vendor is revogi, device name is SmartPlug, SSID is SmartPlug-xxxx" 
        #elif (VENDOR_CODE == VENDOR_EXTEL) 
    	iLength = sprintf(ModifyNameArr, "AT+WAP=11BGN,SOKY-%s,CH1\r\n", pTmp); 
        #warning "Vendor is Extel, device name is SOKY-XXXX, SSID is SOKY-xxxx" 
        #endif 
        //snprintf((char*)&DevInfoObj.dev_name[0], NAME_LENGTH+1, "%s-%s", DEV_DEFAULT_NAME, pTmp);
        snprintf((char*)&DevInfoObj.dev_name[0], NAME_LENGTH+1, "%s-%s", dev_default_name, pTmp);  
        hfat_send_cmd(ModifyNameArr, iLength, AtAckMsg, BOOT_INFO_DATA_BUF_LENGTH); 
    } 
    else 
    { 
        //memcpy(DevInfoObj.dev_name, DEV_DEFAULT_NAME, sizeof(DEV_DEFAULT_NAME)-1);
        memcpy(DevInfoObj.dev_name, dev_default_name, NAME_LENGTH); 
    } 

  #if (PORT_NUM == 6)
    memcpy(SwInformation.PortInfor[0].name, "PORT1", sizeof("PORT1")-1);
    memcpy(SwInformation.PortInfor[1].name, "PORT2", sizeof("PORT2")-1);
    memcpy(SwInformation.PortInfor[2].name, "PORT3", sizeof("PORT3")-1);
    memcpy(SwInformation.PortInfor[3].name, "PORT4", sizeof("PORT4")-1);
    memcpy(SwInformation.PortInfor[4].name, "PORT5", sizeof("PORT5")-1);
    memcpy(SwInformation.PortInfor[5].name, "PORT6", sizeof("PORT6")-1);
 #endif
    // Reset DeviceName and port name 
   
    WriteDevName();  

 	factoryProgramMark = 1;
	WriteDevTimeZone();

	//hfat_send_cmd("AT+WMODE=AP\r\n", sizeof("AT+WMODE=AP\r\n")-1, AtAckMsg, 48);
	//hfat_send_cmd("AT+SMTLK\r\n", sizeof("AT+SMTLK\r\n")-1, AtAckMsg, BOOT_INFO_DATA_BUF_LENGTH);       //进入smart_link模式

    hfuflash_erase_page(FLASH_DEV_RULE_INFO_BUF_START, 1);              // erase rule information 
    hfuflash_erase_page(FLASH_DEV_BIND_BUF_START, 1);                  //erase regidID information:1KB
    hfuflash_erase_page(FLASH_DEV_STATUS_INFO_BUF_START, 1);			// erase the MS/Reboot port information
	hfuflash_erase_page(FLASH_DEV_SECURITY_INFO_BUF_START, 1);

	

#if 1
   #if(PORT_NUM == 1)
   	if(!SwInformation.PortInfor[0].CurStatus)
   	{
		//SwInformation.PortInfor[0].CurStatus = 1;
		Port_TurnOnOff(1);
   	}
   #elif(PORT_NUM == 6)
   {
   		//uint8_t i = 0;
		
   		//for(i=0;i<PORT_NUM;i++)
   		{
		//	SwInformation.PortInfor[i].CurStatus = 1;
   		}

		Switch_TurnOnOff(0,1);
   }
   #endif
   
   SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_14_SW_STATE_IS_DIRTY_CMD);   //save the master status to flash
   SaveDevStatusPoll();
#endif
	msleep(500);
    ClearDevSmartLinkInfo();                //擦除之前保存的WIFI配置信息，设备将重新进入smart_link模式
} 



/******************************************************************************* 
* 功能描述:从flash中复制数据 
* 入口参数: dstaddr:flash 中的数据目的地址；srcaddr:flash 的数据源地址
* 出口参数: 
* 使用说明: 
*******************************************************************************/ 
int FlashBackupCpy(uint32_t dstaddr, uint32_t srcaddr, uint32_t len) 
{ 
    char     buf[16]; 
    uint32_t i,rlen; 

    for(i=0; i<len; ) 
    { 
        if(i>=len) 
        { 
            break; 
        } 
        else if(i+16>len) 
        { 
            rlen = len-i; 
        } 
        else 
        { 
            rlen = 16; 
        } 
        hfuflash_read(srcaddr+i, buf, rlen); 
        hfuflash_write(dstaddr+i, buf, rlen); 
        i += rlen; 
    }
    return len; 
}
/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/

