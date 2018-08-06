/*********************************************************************************
* @file    PowerStaticsRecord.c
* @author  
* @version V1.0.0
* @date    06/06/2014
* @brief   
*********************************************************************************/
#include <stdint.h> 
#include <stdio.h> 
#include <time.h> 

#include "ConfigAll.h" 
#include "hsf.h"
#include "cJSON.h"

#include "DevDataType.h"
#include "DevInfo.h" 
#include "DevLib.h" 
#include "PowerStaticsRecord.h"
#include "RtTime.h"

#include "Flash.h"
#include "PublicServerCom.h"
#include "DevRule.h "
#include "UdpServerCallBack.h"

#define         RECORD_HOUR_MAX                 24 
#define         RECORD_HOUR_POS_MAX             (RECORD_HOUR_MAX-1) 
#define         TEMPERATURE_EMPTY_DATA_VALUE    0x00 

#define         RECORD_DAY_MAX                  31
#define         RECORD_DAY_POS_MAX              (RECORD_DAY_MAX-1)




uint8_t             iStaticsFlag = 0x00;                                    //标志位组合变量
uint8_t             hasReportHourData = 0;                                  //上报小时历史功耗成功
uint8_t             isReportRTHourSuccess = 0;                              //上报小时实时功耗成功与否标志
uint8_t             addReportRtHourMark = 0;                                //补报实时功耗标志
//uint8_t				reportHourDataFlag = 0;



//统计有多长时间的数据没有加上时间戳
uint32_t            totalNoTimeRecord = 0;                                  

uint32_t            TempPowerStaticsSum[PORT_NUM+1];                          //小时功耗保存数组
uint32_t			readPowerCount = 0;										//读取功耗次数


int                 dataID = 0;                                             //无时间数据对应的数据ID号
uint16_t sendRecordCount = 0;		//补报的数据个数
uint16_t surplusRecordCount = 0;	//剩余的数据个数

HistoryDataObj	HistoryData;					//小时功耗数据
DevOffLineDataObj devOffLineData;				//断网时数据
CurrentDataType CurrentDataObj[PORT_NUM];


extern DevInfoType  DevInfoObj;  
extern FwUpdateStatusType   FwUpdateStatusObj;                              // 
extern char   AppTaskSendBuf[1400];
extern DevTimeType  DevTimeObj;
extern DevRegInfoType          DevRegInformation;                                      //设备注册信息
extern uint8_t isDevReg;                                           //是否注册成功标志






/*******************************************************************************
* 功能描述: 初始化RTC时间
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/
extern void PowerStatics_Init(void) 
{ 
    struct tm   iStartTime;  
    // 2000.01.01,00:00:00 
    iStartTime.tm_year          = 2000-1900;
    iStartTime.tm_mon           = 0;
    iStartTime.tm_mday          = 1;
    iStartTime.tm_hour          = 0;
    iStartTime.tm_min           = 0;
    iStartTime.tm_sec           = 0;
    iStartTime.tm_wday          = 6;
    iStartTime.tm_isdst         = 0;
    set_rtctime(&iStartTime); 
   
    DevInfoObj.isCalibrateTime  = 0x00; 
    iStaticsFlag                = 0x00; 
} 




/*******************************************************************************
* 功能描述: 统计数据，每小时统计一次
* 入口参数: 
* 出口参数: 
* 使用说明: 
* 备    注: 此处待优化成查表状 
*******************************************************************************/
void PowerRecord_StaticsData(void)
{ 
    DevTimeType     *pCurTime;
    int             iStatisticsPara[PORT_NUM]; 
    uint8_t i=0,portID = 0;
    static int nowTimeStack = 0;
    char testBuf[100] = {0};
        
    pCurTime  = GetCurTime();

    if(0x00 != isBitSet(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_9_IS_HOURS_DATA_CMD))
    {
    	ClrBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_9_IS_HOURS_DATA_CMD);

    	if(readPowerCount >= 10)
    	{
		
#ifdef TEST
			nowTimeStack = pCurTime->NowTimeSec;			//test
#else
	        nowTimeStack = (pCurTime->NowTimeSec+100) - (pCurTime->NowTimeSec+100)%3600;	
#endif       
	        
	        for(portID=0;portID<PORT_NUM;portID++)
	        {
	            iStatisticsPara[portID] = TempPowerStaticsSum[portID]/readPowerCount;         //统计该段时间内所用功耗

	            memset(testBuf,0,sizeof(testBuf));
	 			sprintf(testBuf,"port[%d]:sumPower=%d\t readPowerCount=%d\n",portID,TempPowerStaticsSum[portID],readPowerCount);
				UdpDebugPrint(testBuf,strlen(testBuf));
	                                      
	            TempPowerStaticsSum[portID] = 0;  
	        }
	        readPowerCount = 0;
	    
        	if(DevInfoObj.isCalibrateTime) 
	        {
		        if(isBitSet(iStaticsFlag, WIFI_WORK_NORMAL))                          //对时成功且网络正常
		        {
		        	for(portID=0;portID<PORT_NUM;portID++)
		        	{
		        		HistoryData.power[portID] = iStatisticsPara[portID]*10;
		        		HistoryData.timeStack = nowTimeStack;

		        		sprintf(testBuf,"report_data[%d]:power=%d\t timeStack=%d\n",portID,HistoryData.power[portID],HistoryData.timeStack);
		            	UdpDebugPrint(testBuf,strlen(testBuf));
		        	}
		        }
		        else
		        {	        	
					for(portID=0;portID<PORT_NUM;portID++)
					{
						devOffLineData.recordData[devOffLineData.recordCount].power[portID] = iStatisticsPara[portID]*10;
						devOffLineData.recordData[devOffLineData.recordCount].timeStack = nowTimeStack; 				
					}
					devOffLineData.recordCount++;
					devOffLineData.recordCount %= RECORD_HOURS;
					surplusRecordCount = devOffLineData.recordCount;

					for(i=0;i<devOffLineData.recordCount;i++)
		            {
		            	sprintf(testBuf,"record_data[%d]:power=%d\t timeStack=%ld\n",i,devOffLineData.recordData[i].power[0],
		            						devOffLineData.recordData[i].timeStack);
		            	UdpDebugPrint(testBuf,strlen(testBuf));
		            }
		    	
		        }
		    }
		}
    }

} 







/******************************************************************************* 
* ========================= END OF PAGE 
*******************************************************************************/ 

