/*
********************************************************************************
* @file    DevLib.c 
* @author  
* @version V1.0.0 
* @date    18/03/2013 
* @brief   
********************************************************************************
*/
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

#include "ConfigAll.h" 
#include "cJSON.h" 
#include "DevLib.h"
#include "DevInfo.h"
#include "DevDataType.h"
#include "flash.h"

#include "UdpServerCallBack.h"
#include "RtTime.h"
#include "TickClock.h"

#include "rn8209.h"
#include "customer_spi.h"
#include "PowerStaticsRecord.h"
#include "PublicServerCom.h"
#include "Produce_Test.h"
#include "SwitchApp.h"
#include "UdpSocket.h"
#include "read_power.h"
// 
#define     CUR_NOT_SET_TIME        0x00 
#define     CUR_HAD_SET_TIME        0x01 
// 
extern      int8_t                  UpdataFwFlag; 
extern      uint32_t                UpdateFwLength; 




extern DevInfoType         DevInfoObj; 
extern DevRuleInfoType     DevRuleInfoObj; 
extern TurnOffTimeType     CountDownTurnOff[PORT_NUM+1]; 
extern FwUpdateStatusType  FwUpdateStatusObj; 
extern SoftwareVerType     CurFwVersionObj;                                 // 本地固件版本 
extern FwUpdateArUrlType   FwUpdateObj;                                     //

#if defined(ZX_DETECT)
static void USER_FUNC check_user_zxInt(uint32_t arg1,uint32_t arg2);
#endif



/*******************************************************************************
* 功能描述: 
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/ 
extern void delay_us(int us)
{
	volatile int i;
	for(i=0; i<us; i++)
	{
		;
	}
}

/*******************************************************************************
* 功能描述:  
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/
extern void DevGPIO_Init(void)
{
	hfgpio_configure_fpin(HFGPIO_SW_KEY_CTL_PIN, HFPIO_PULLUP|HFM_IO_TYPE_INPUT);
	hfgpio_configure_fpin(HFGPIO_SW_ON_OFF_CTL_PIN, HFPIO_PULLDOWN|HFM_IO_OUTPUT_0);

	
#if defined(ZX_DETECT)
	if(hfgpio_configure_fpin_interrupt(ZX_INT_CHECK,HFPIO_IT_RISE_EDGE,check_user_zxInt,1)!=HF_SUCCESS)
	{
		u_printf("configure HFGPIO_F_USER_RELOAD fail\n");
		return;
	}
#endif

	 
	SpiRn8209_Init();

	RN8209_Init();

	uart_process_init();

	KeyTimer_Init();
}


#if defined(ZX_DETECT)
/*******************************************************************************
* 功能描述: GPIO外部中断回调函数
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/
static void USER_FUNC check_user_zxInt(uint32_t arg1,uint32_t arg2)
{
	ZX_detect_control();
}
#endif

/******************************************************************************* 
* 功能描述: 发送UDP数据 
* 入口参数: 
* 出口参数: 
* 使用说明: 向UDP端口9898广播Debug数据 
*******************************************************************************/ 
#if defined(__UDP_PRINT_DBG__) 
extern void UdpDebugPrint(char *pArgBuf, int ArgLength) 
{ 
    struct sockaddr_in  UdpPrintDebugAddr; 
    int                 fd; 
    
    UdpPrintDebugAddr.sin_family       = AF_INET; 
    UdpPrintDebugAddr.sin_port         = htons(8892); 
    UdpPrintDebugAddr.sin_addr.s_addr  = inet_addr("255.255.255.255"); 
    
    fd = FwUpdateStatusObj.UdpFd; 
    if(-1 == fd) 
    { 
        #if defined(__BSD_SOCKET_UDP_SERVER_ENABLE__) 
        fd = UdpServerThread_GetSocketFd();
        #else 
        fd = hfnet_socketa_fd(); 
        #endif 
        
        if(fd<0)
        { 
            return; 
        } 
    } 

    sendto( fd, pArgBuf, ArgLength, 0, 
            (struct sockaddr*)&UdpPrintDebugAddr, 
            sizeof(struct sockaddr)); 
} 
#endif 

/******************************************************************************* 
* 功能描述: 设备升级命令
* 输入参数:
* 输出参数:
* -返回值-:
* 使用说明:
*******************************************************************************/ 
extern uint8_t SetUpdateUrl(char *pDataBuf)
{ 
	cJSON       *json; 
	char        *str; 
	int         iLength, iRet, FwNewVer[2]={0}; 
	uint8_t     iRslt;
	uint16_t    PublicVer;
    // 
    #if defined(__URL_UPDATE_DBG__) && defined(__UDP_PRINT_DBG__)
    UdpDebugPrint("51 UpdateUrl:\r\n", sizeof("51 UpdateUrl:\r\n")-1);
    #endif
    iRslt = 1;
	json  = cJSON_Parse(pDataBuf);
	if(json)
    {
		str  = cJSON_GetObjectItem(json, "ver")->valuestring; 
        if(str) 
        { 
            iRet = sscanf(str, "%d.%d", &FwNewVer[0],&FwNewVer[1]); 
            if(-1 != iRet) 
            { 
                FwUpdateObj.PublicServerVer.big_ver    = FwNewVer[0]; 
                FwUpdateObj.PublicServerVer.little_ver = FwNewVer[1]; 
                PublicVer = (((FwNewVer[0]<<8)&0xFF00) | (FwNewVer[1]&0xff));
                if( PublicVer>CURRENT_VER_NUMBER )
                {
        		    FwUpdateObj.data_checksum = cJSON_GetObjectItem(json,"checksum")->valueint; 
                	str     = cJSON_GetObjectItem(json,"url")->valuestring;
                	iLength = strlen(str); 
                	if(iLength < UPDATE_URL_MAX_LENGTH) 
                	{ 
                        if(0x00 == hfthread_mutext_trylock(FwUpdateObj.FwUpdateMutx)) 
                        { 
                            memset(FwUpdateObj.UpdateUrl, '\0', UPDATE_URL_MAX_LENGTH); 
                            memcpy(FwUpdateObj.UpdateUrl, str, iLength); 
                		    SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_5_IS_URL_UPDATE_FW); 
                		    iRslt = 0; 
                            hfthread_mutext_unlock(FwUpdateObj.FwUpdateMutx); 
                            // 
                            #if defined(__URL_UPDATE_DBG__) && defined(__UDP_PRINT_DBG__) 
                            UdpDebugPrint("Record Url:\r\n", sizeof("Record Url:\r\n")-1); 
                            UdpDebugPrint(FwUpdateObj.UpdateUrl, iLength); 
                            UdpDebugPrint("\r\n", sizeof("\r\n")-1); 
                            #endif 
                        } 
                	} 
            	}
            } 
        } 
		cJSON_Delete(json); 
	} 
    return iRslt;
} 

/******************************************************************************* 
* 功能描述: 服务器要求终端重启，同步，恢复出厂
* 输入参数:
* 输出参数:
* -返回值-:
* 使用说明:cmd: 0: 重启     1:同步数据到服务器      2:恢复出厂设置
*******************************************************************************/ 
#define     CMD_VALID_MASK          0x03 
char RebootSyncDev(char *pBufData,int *cmd) 
{ 
    cJSON   *pJson; 
    char    iRet; 

    u_printf("Reset cmd is:%s\n",pBufData);
    
    iRet  = 1; 
    pJson = cJSON_Parse(pBufData); 
    if(pJson) 
    { 
        *cmd  = cJSON_GetObjectItem(pJson, "op")->valueint; 
        cJSON_Delete(pJson); 
        if(CMD_VALID_MASK != (*cmd&CMD_VALID_MASK)) 
        { 
            Reboot(*cmd);
            iRet = 0; 
        } 

        u_printf("cmd = %d\n",*cmd);
    } 
    return iRet;
} 

/******************************************************************************* 
* 功能描述: 3秒后执行对应的程序，重启、同步数据或者恢复出厂设置
* 输入参数:
* 输出参数:
* -返回值-:
* 使用说明:
*******************************************************************************/ 
extern void Reboot(int iRebootCode)
{
    FwUpdateStatusObj.SyncRebootCode  = (iRebootCode&CMD_VALID_MASK); 
    FwUpdateStatusObj.RebootCountDown = 5; 
    SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_6_IS_REBOOT_CMD);
}



/******************************************************************************* 
* 功能描述: 
* 输入参数:
* 输出参数:
* -返回值-:
* 使用说明:
*******************************************************************************/ 
extern uint32_t AnalyTime(char *pBuf)
{ 
    uint8_t     iPos = 0; 
    uint8_t     iTemp; 
    char        *pCharTem   = pBuf; 
    int         iTime[6+1]  = {0}; 
    int         *pArr; 
    
    struct tm   TempTime; 
    time_t      LocalSecond; 
    
    // 
    pArr     = &iTime[0]; 
    // 
    for(iTemp=0; iTemp<6; iTemp++) 
    { 
        iPos      = strcspn(pCharTem,"-,:;\""); 
        *pArr     = atoi(pCharTem); 
        
        pArr++; 
        pCharTem += iPos+1; 
    } 
    pArr              = &iTime[6];
    TempTime.tm_sec   = *(--pArr);                                              // 设置时间 
    TempTime.tm_min   = *(--pArr);
    TempTime.tm_hour  = *(--pArr);
    TempTime.tm_mday  = *(--pArr);
    TempTime.tm_mon   = *(--pArr);
    TempTime.tm_year  = *(--pArr);
    TempTime.tm_isdst = 0;                                                      // 冬令时 
    
    // 
	TempTime.tm_year -= 1900; 
    TempTime.tm_mon  -= 1;
    LocalSecond       = mktime(&TempTime); 
    
    return LocalSecond; 
} 
/******************************************************************************* 
* 功能描述: 设置时钟
* 输入参数:
* 输出参数:
* -返回值-:
* 使用说明:
*******************************************************************************/ 
#if  !defined(__DS1302_RTC_ENABLE__) 
#endif 
extern  DevTimeType                 DevTimeObj;
extern char SetCurTime(char *pBuf)
{ 
    uint8_t     iPos = 0; 
    uint8_t     iTemp; 
    char        *pCharTem   = pBuf; 
    int         iTime[6+1]  = {0}; 
    int         *pArr; 
    
    //static      uint8_t  CurHaveSetTime = CUR_NOT_SET_TIME;                     // 记录本地是否设置过时间,本地设置过时间则不再接受服务器时间 
    
    struct tm   TempTime, *pTime; 
    time_t      LocalCurSecond; 
    
    // 
    pArr     = &iTime[0]; 
    // 
    for(iTemp=0; iTemp<6; iTemp++) 
    { 
        iPos      = strcspn(pCharTem,"-,:;\""); 
        *pArr     = atoi(pCharTem); 
        pArr++; 
        pCharTem += iPos+1; 
    } 
    pArr              = &iTime[6];
    TempTime.tm_sec   = *(--pArr);                                              // 设置时间 
    TempTime.tm_min   = *(--pArr);
    TempTime.tm_hour  = *(--pArr);
    TempTime.tm_mday  = *(--pArr);
    TempTime.tm_mon   = *(--pArr);
    TempTime.tm_year  = *(--pArr);
    TempTime.tm_isdst = 0;                                                      // 冬令时 
  
    pCharTem --; 
    if(',' == *pCharTem)                                                        // 接收时间为本地时间 
    { 
        //if(CUR_HAD_SET_TIME == CurHaveSetTime)
        {
        //    return 0;
        }
        pCharTem ++;                                                            // 
        iTemp     = atoi(pCharTem);

        //u_printf("TimeZone=%d   getTimeZone=%d\n",DevInfoObj.TimeZone,iTemp);
        
        if(DevInfoObj.TimeZone != iTemp)
        {
            DevInfoObj.TimeZone = iTemp;                                        // 
            WriteDevTimeZone();                                           // Save Current Time Zone To flash  
        }
        
        //CurHaveSetTime   = CUR_HAD_SET_TIME;                                 // 
    	TempTime.tm_year-= 1900;
        TempTime.tm_mon -= 1;
        pTime            = &TempTime; 
        LocalCurSecond   = mktime(&TempTime);                                   //        
    } 
    else                                                 // 接收时间为UTC时间 ( 则根据本地时区来进行调整 ) 
    { 
	    TempTime.tm_year -= 1900; 
        TempTime.tm_mon  -= 1; 
        LocalCurSecond    = mktime(&TempTime); 
        if(DevInfoObj.TimeZone>12) 
        { 
            iTemp           = DevInfoObj.TimeZone - 12; 
            LocalCurSecond += iTemp * 3600; 
        } 
        else 
        { 
            iTemp           = 12 - DevInfoObj.TimeZone; 
            LocalCurSecond -= iTemp * 3600; 
        } 
        pTime = localtime(&LocalCurSecond);
    }

    DevInfoObj.isCalibrateTime = 0x01;
    
  
   
   #if defined(__UDP_SET_TIME_DBG__) && defined(__UDP_PRINT_DBG__) 
    UdpDebugPrint("SetTime\r\n", sizeof("SetTime\r\n")-1);
   #endif 

    DevTimeObj.CurTimeSec         = LocalCurSecond; 
    if(DevInfoObj.TimeZone>12) 
    { 
        iTemp           = DevInfoObj.TimeZone - 12; 
        DevTimeObj.NowTimeSec = DevTimeObj.CurTimeSec - iTemp * 3600; 
    } 
    else 
    { 
        iTemp           = 12 - DevInfoObj.TimeZone; 
        DevTimeObj.NowTimeSec = DevTimeObj.CurTimeSec + iTemp * 3600;  
    }    
    DevTimeObj.NowTimeTm          = *pTime; 
    DevTimeObj.NowTimeTm.tm_year += 1900; 
    DevTimeObj.NowTimeTm.tm_mon++; 
  
   #if  defined(__DS1302_RTC_ENABLE__)  
    set_rtctime(pTime); 
   #else 
    SysTime_Set(pTime); 
   #endif 

    
    return 0; 
} 



/******************************************************************************* 
* 功能描述: 设置时钟，根据格林尼治时间戳
* 输入参数:
* 输出参数:
* -返回值-:
* 使用说明:
*******************************************************************************/ 
extern void SetCurTimeFromSec(time_t timeStack)
{
    struct tm  *pTime;
    uint8_t     iTemp; 

    DevTimeObj.NowTimeSec         = timeStack; 
    
    if(DevInfoObj.TimeZone>12) 
    { 
        iTemp           = DevInfoObj.TimeZone - 12; 
        timeStack += iTemp * 3600; 
        DevTimeObj.CurTimeSec = timeStack;
    } 
    else 
    { 
        iTemp           = 12 - DevInfoObj.TimeZone; 
        timeStack       -= iTemp * 3600; 
        DevTimeObj.CurTimeSec = timeStack;
    } 
    
    pTime = localtime(&DevTimeObj.CurTimeSec);
   #if  defined(__DS1302_RTC_ENABLE__)  
    DevTimeObj.NowTimeTm          = *pTime; 
//    DevTimeObj.NowTimeTm.tm_year += 1900; 
//    DevTimeObj.NowTimeTm.tm_mon++; 
    set_rtctime(pTime); 
   #else 
    SysTime_Set(pTime); 
   #endif 

   DevInfoObj.isCalibrateTime = 0x01;

   //u_printf("isCalibrateTime = %d   RecordEndTime=%d\n",DevInfoObj.isCalibrateTime,RecordEndTime);
}


/*******************************************************************************
* 功能描述: 解析规则
* 入口参数:
* 出口参数:
* 使用说明:
* 调用方法:
*******************************************************************************/
extern uint8_t ParseRule(char *pArgBuf) 
{ 
	cJSON   *json,*port,*week;
	uint32_t  ruleID;

	uint8_t  iRet,port_num,j,mode;
	SwitchRuleType  tempRule;

	//u_printf("rule data is:%s\n",pArgBuf);
	
	json = cJSON_Parse(pArgBuf);
	iRet = FAILED; 
	if(json)
	{
		memset((char*)&tempRule, 0, sizeof(tempRule));
		
		mode      = cJSON_GetObjectItem(json, "mode")->valueint;
		ruleID    = cJSON_GetObjectItem(json, "id")->valueint;
		tempRule.RuleID = ruleID;
		tempRule.RuleEnable = cJSON_GetObjectItem(json, "en")->valueint;
		tempRule.CurTimeStamp= cJSON_GetObjectItem(json, "day")->valueint;
		
		if(tempRule.CurTimeStamp > 0)
		{
		    tempRule.timeStamp = tempRule.CurTimeStamp - (DevInfoObj.TimeZone-12)*3600;
		}
        //u_printf("timeStamp = %d    DevTimeObj.CurTimeSec=%d\n",tempRule.timeStamp,DevTimeObj.CurTimeSec);
    
		tempRule.timeLong   = cJSON_GetObjectItem(json, "time")->valueint;
		if(tempRule.timeLong == 0)
		{
			tempRule.exeTimeLong = 1;
		}
		else
		{
			tempRule.exeTimeLong = tempRule.timeLong;
		}
		
		port        = cJSON_GetObjectItem(json, "port");
		port_num    = cJSON_GetArraySize(port);		
		for(j=0; j<port_num; j++)
		{
			tempRule.PortArr[j] = cJSON_GetArrayItem(port, j)->valueint;
			//u_printf("port[%d] = %d\n",ruleID,cJSON_GetArrayItem(port,j)->valueint);
	    }
	    
	    week   = cJSON_GetObjectItem(json, "week");
		for(j=0; j<DAY_COUNT_OF_WEEK; j++)
		{
			tempRule.SwOpDateObj |= ((cJSON_GetArrayItem(week,j)->valueint)<<j);
			
			//u_printf("rule[%d]:day[%d] = %d\n",ruleID,j,cJSON_GetArrayItem(week,j)->valueint);
		}
        
		if(mode == 0)                //添加规则
		{  
		    if(DevRuleInfoObj.RuleCount < SOCKET_RULE_COUNT)
		    {	
	            if(Increment(ruleID,tempRule))
	            {
	                iRet = PASSED;
	            }
	            
		        //PrintRule();
		    }
		}
		else if(mode == 2)           //删除规则
		{
		    //u_printf("Delete ruleID is :%d\n",ruleID);
		    
		    if(DevRuleInfoObj.RuleCount>0)
		    {
		        if(DeleteRule(ruleID))
		        {
		            iRet = PASSED;
		        }
		        
		        //PrintRule();
		    }
		}
		else                       //编辑规则
		{
		    if(ReFreshRule(ruleID,tempRule))
		    {
		        iRet = PASSED;
		    }
		    
		    //PrintRule();
		}

		cJSON_Delete(json);

		
		if(iRet == PASSED)
		{ 
            SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_13_RULE_IS_DIRTY_CMD);
            DevRuleInfoObj.isRuleSaveFlag = FLASH_DEV_RULE_FLAG;
        }

//		PublicServer_SyncBaseInfoWithServer();
    }
    return iRet; 
} 




/*******************************************************************************
* 功能描述: 添加规则
* 入口参数: ruleID:规则对应的ID号
* 出口参数:
* 使用说明:
* 调用方法:
*******************************************************************************/
extern uint8_t Increment(uint32_t ruleID,SwitchRuleType tempRule) 
{
    uint8_t i = 0;
    
    for(i=0;i<DevRuleInfoObj.RuleCount;i++)
    {
        if(DevRuleInfoObj.SwitchRulePool[i].RuleID == ruleID)
        {
            return 0;
        }
    }

     DevRuleInfoObj.RuleCount++;
     DevRuleInfoObj.SwitchRulePool[DevRuleInfoObj.RuleCount-1] = tempRule;  

     return 1;
}




/*******************************************************************************
* 功能描述: 删除规则
* 入口参数: ruleID:规则对应的ID号
* 出口参数:
* 使用说明:
* 调用方法:
*******************************************************************************/
extern uint8_t DeleteRule(uint32_t ruleID) 
{
    uint8_t i = 0,j = 0;
    
    for(i=0;i<DevRuleInfoObj.RuleCount;i++)
    {
        if(DevRuleInfoObj.SwitchRulePool[i].RuleID == ruleID)
        {
            for(j=i;j<DevRuleInfoObj.RuleCount;j++)
            {
                DevRuleInfoObj.SwitchRulePool[j] = DevRuleInfoObj.SwitchRulePool[j+1];
            }
            
		    DevRuleInfoObj.RuleCount--;

            return 1;
        }
    }
    return 0;
}



/*******************************************************************************
* 功能描述: 编辑规则
* 入口参数: ruleID:规则对应的ID号
* 出口参数:
* 使用说明:
* 调用方法:
*******************************************************************************/
extern uint8_t ReFreshRule(uint32_t ruleID,SwitchRuleType tempRule) 
{
    uint8_t i = 0;
    
    for(i=0;i<DevRuleInfoObj.RuleCount;i++)
    {
        if(DevRuleInfoObj.SwitchRulePool[i].RuleID == ruleID)
        {
              DevRuleInfoObj.SwitchRulePool[i] = tempRule;

              return 1;
        }   
    }

    return 0;
}


#if 0
/*******************************************************************************
* 功能描述: 打印规则
* 入口参数: 
* 出口参数:
* 使用说明:
* 调用方法:
*******************************************************************************/
extern void PrintRule(void) 
{
    uint8_t i = 0;

    u_printf("RuleCount = %d\n",DevRuleInfoObj.RuleCount);

    for(i=0;i<DevRuleInfoObj.RuleCount;i++)
    {
         u_printf("Rule[%d]:RuleID = %d SwOpDateObj=%d RuleEnable=%d   timeStamp=%d  delayTime=%d\n",i,
                     DevRuleInfoObj.SwitchRulePool[i].RuleID,DevRuleInfoObj.SwitchRulePool[i].SwOpDateObj,
                     DevRuleInfoObj.SwitchRulePool[i].RuleEnable,DevRuleInfoObj.SwitchRulePool[i].timeStamp,
                     DevRuleInfoObj.SwitchRulePool[i].timeLong);
    }
}
#endif




/******************************************************************************* 
* 功能描述:获取某月对应的天数
* 输入参数:
* 输出参数: 
* -返回值-: 
* 使用说明: 
*******************************************************************************/ 
extern int GetDaysInMonth(int y, int m)
{
    int d;
    int day[]= {31,28,31,30,31,30,31,31,30,31,30,31};
    if (2==m)
    {
        d = ((((0==y%4)&&(0!=y%100))||(0==y%400))?29:28);
    }
    else
    {
        d = day[m-1];

    }
    return d;
}

/******************************************************************************* 
* 功能描述: 计算串口数据的CheckSum 
* 输入参数:
* 输出参数: 
* -返回值-: 
* 使用说明: 
*******************************************************************************/ 
extern uint8_t CaculateUartDataCheckSum(uint8_t *pBuf, uint8_t iLen)
{ 
    uint8_t i; 
    uint8_t CheckSum; 
    
    CheckSum = 1; 
    for(i=0; i<iLen; i++) 
    { 
        CheckSum += *(pBuf+i); 
    } 
    return CheckSum; 
} 

/******************************************************************************* 
* 功能描述:解析url
* 输入参数:
* 输出参数: 
* -返回值-: 
* 使用说明: 
*******************************************************************************/ 
extern void DecodeUrl(char *pBuf)
{
    uint16_t i=0;
    while(*(pBuf+i))
    {
        if ((*pBuf=*(pBuf+i)) == '%')
        {
            *pBuf  = *(pBuf+i+1) >= 'A' ? ((*(pBuf+i+1) & 0xDF) - 'A') + 10 : (*(pBuf+i+1) - '0');
            *pBuf  = (*pBuf) * 16;
            *pBuf += *(pBuf+i+2) >= 'A' ? ((*(pBuf+i+2) & 0xDF) - 'A') + 10 : (*(pBuf+i+2) - '0');
            i  += 2;
        }
        else if (*(pBuf+i)=='+')
        {
            *pBuf  = ' ';
        }
        pBuf++;
    }
    *pBuf = '\0';
}
/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/

