#include <stdint.h> 
#include <stdio.h> 
#include <string.h> 
#include <time.h> 
#include <stdlib.h>

#include <cJSON.h> 
#include "hsf.h" 
#include "Flash.h" 
#include "DevDataType.h" 
#include "PublicServerCom.h"
#include "DevLib.h"
#include "read_power.h"
#include "Produce_Test.h"
#include "security_mode.h"


uint8_t securityStatus = 0;				//放到模式状态
uint8_t reportSecurityInfo = 1;			//上报防盗模式标志位

SecurityModeType SecurityMode;			//防盗模式结构体
SecurityModeExcuteType SecurityModeExcute;			//防盗模式执行结构体	

extern FwUpdateStatusType  FwUpdateStatusObj; 
extern DevInfoType         DevInfoObj;
extern  DevTimeType        DevTimeObj;



static void ProduceRotateSecurityMode(SecurityModeType  *pSecurityModeObj);
static void SwitchTurnOver(SecurityModeType  *pSecurityModeObj);



/*******************************************************************************
* 功能描述: 解析防盗模式
* 入口参数:
* 出口参数:
* 使用说明:
* 调用方法:
*******************************************************************************/
extern uint8_t ParseSecurityMode(char *pArgBuf) 
{ 
	cJSON   *json,*timeArray,*week,*port;

	uint8_t  iRet,j,port_num;

	//UdpDebugPrint(pArgBuf,strlen(pArgBuf));
	memset((char*)&SecurityMode,0,sizeof(SecurityMode));
	json = cJSON_Parse(pArgBuf);
	iRet = FAILED; 
	if(json)
	{
		SecurityMode.enable = cJSON_GetObjectItem(json, "enable")->valueint;
			
		timeArray        = cJSON_GetObjectItem(json, "time");			
		SecurityMode.startTime = cJSON_GetArrayItem(timeArray, 0)->valueint;
		SecurityMode.endTime = cJSON_GetArrayItem(timeArray, 1)->valueint;

		port        = cJSON_GetObjectItem(json, "port");
		port_num    = cJSON_GetArraySize(port);		
		for(j=0; j<port_num; j++)
		{
			SecurityMode.PortArr[j] = cJSON_GetArrayItem(port, j)->valueint;
			//u_printf("port[%d] = %d\n",ruleID,cJSON_GetArrayItem(port,j)->valueint);
	    }

		week   = cJSON_GetObjectItem(json, "week");
		for(j=0; j<DAY_COUNT_OF_WEEK; j++)
		{
			SecurityMode.SwOpDateObj |= ((cJSON_GetArrayItem(week,j)->valueint)<<j);
			
			//u_printf("rule[%d]:day[%d] = %d\n",ruleID,j,cJSON_GetArrayItem(week,j)->valueint);
		}
		

		iRet = PASSED;
		cJSON_Delete(json);
    }
    return iRet; 
} 



/*******************************************************************************
* 功能描述: 轮询防盗模式
* 输入参数: 
* 输出参数: 
* -返回值-: 
* 使用说明: 
*******************************************************************************/
void DevSecurityModePoll(void)
{
    SecurityModeType  *pSecurityModeObj = NULL; 

	if(isBitSet(FwUpdateStatusObj.TimelyCheck, TIMELY_DETECTE_BIT_19_DEV_SECURITY_MODE_CMD))
	{
		ClrBit(FwUpdateStatusObj.TimelyCheck, TIMELY_DETECTE_BIT_19_DEV_SECURITY_MODE_CMD);
		
		if(0x00 == DevInfoObj.isCalibrateTime)			//未对时
		{
			return;
		}
		pSecurityModeObj = &SecurityMode;

		if(pSecurityModeObj->enable)
		{
	        if(0x00 != pSecurityModeObj->SwOpDateObj) 		//周期性有效
	        {
	            ProduceRotateSecurityMode(pSecurityModeObj);
	        }
		}
	}
}




/**********************************************************************************************************
* 功能描述: 执行防盗模式
* 输入参数: 
* 输出参数: 
* -返回值-: 
* 使用说明: 循环执行防盗模式
**********************************************************************************************************/
static void ProduceRotateSecurityMode(SecurityModeType  *pSecurityModeObj)
{
    uint8_t     iTmp;
    uint32_t    NowTimeMin;
	//static uint8_t autoMode = 0;			//自动警戒模式
	//char testBuf[100] = {0};

    NowTimeMin = DevTimeObj.NowTimeTm.tm_hour * 60 + DevTimeObj.NowTimeTm.tm_min; 
    iTmp       = DevTimeObj.NowTimeTm.tm_wday; 							//星期几

	srand(DevTimeObj.NowTimeSec);

	if(!SecurityModeExcute.RunningMark)
	{
		if(isBitSet(pSecurityModeObj->SwOpDateObj,iTmp))
		{
			if(pSecurityModeObj->startTime < (pSecurityModeObj->endTime-20))		//取20分钟的随机值
			{
		        if(NowTimeMin>=pSecurityModeObj->startTime && NowTimeMin<(pSecurityModeObj->endTime-20))
		        {
					SecurityModeExcute.intervalTime = pSecurityModeObj->endTime - NowTimeMin;
					SecurityModeExcute.RunningMark = 1;				//警戒模式开始执行
					SecurityModeExcute.nextIntervalTime = rand()%10+10;
		        }
			}
			else
			{
				if(NowTimeMin>=pSecurityModeObj->startTime)
		        {							
					SecurityModeExcute.intervalTime = pSecurityModeObj->endTime+24*60 - NowTimeMin;
					SecurityModeExcute.RunningMark = 1;				//警戒模式开始执行
					SecurityModeExcute.nextIntervalTime = rand()%10+10;
				#if 0
					memset(testBuf,0,sizeof(testBuf));
					sprintf(testBuf,"intervalTime=%d\t NowTimeMin=%d\t endTime=%d\n",
						SecurityModeExcute.intervalTime,NowTimeMin,pSecurityModeObj->endTime);
					UdpDebugPrint(testBuf,sizeof(testBuf));
				#endif
		        }
			}
		}
	}
	else
    {	
	#if 0
		memset(testBuf,0,sizeof(testBuf));
		sprintf(testBuf,"intervalTime=%d\t RunningMark=%d\t nextIntervalTime=%d\n",
			SecurityModeExcute.intervalTime,SecurityModeExcute.RunningMark,SecurityModeExcute.nextIntervalTime);
		UdpDebugPrint(testBuf,sizeof(testBuf));
	#endif
		
		if(SecurityModeExcute.timeOut)
		{
			SecurityModeExcute.timeOut = 0;
			//执行开关跳变
			SwitchTurnOver(pSecurityModeObj);
			//计算下一次跳边距离现在的分钟数
			
			SecurityModeExcute.nextIntervalTime = rand()%10+10;
			
       		//sprintf(testBuf,"Now reset next security time:nextIntervalTime=%d\n",SecurityModeExcute.nextIntervalTime);
			//UdpDebugPrint(testBuf,sizeof(testBuf));
		}
    }
}



/**********************************************************************************************************
* 功能描述: 执行防盗模式IO口翻转
* 输入参数: 
* 输出参数: 
* -返回值-: 
* 使用说明: 循环执行防盗模式
**********************************************************************************************************/
static void SwitchTurnOver(SecurityModeType  *pSecurityModeObj)
{
    uint8_t iTmp;
    iTmp = 0;
  
  #if(PORT_NUM == 6)
  {
    while((iTmp<PORT_NUM))
    {
        if(pSecurityModeObj->PortArr[iTmp]==1)
        {
			if(SwInformation.PortInfor[iTmp].OnOffStatus)
	        {
	            SetPortState(iTmp+1,0);            
	        }
	        else 
	        {
	            SetPortState(iTmp+1,1);
	        }
    	}
	
        iTmp++;
    }  
	msleep(200);
    UsartControlPortOnOff(); 
  }
  #elif (PORT_NUM == 1)
  {
    if(pSecurityModeObj->PortArr[iTmp]==1)			//翻转对应的IO口
    {
		if(SwInformation.PortInfor[0].OnOffStatus)
		{
        	Port_TurnOnOff(0);
		}
		else
		{
        	Port_TurnOnOff(1);
		}
		
		plug_updata = 1;
     }
  }
  #endif

  //UdpDebugPrint("Now excute OnOffSwitch,DevRule.c 150 line!\r\n",sizeof("Now excute OnOffSwitch,DevRule.c 150 line!\r\n")-1);
}




