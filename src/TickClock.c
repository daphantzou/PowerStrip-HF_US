/*
********************************************************************************
* @file    TickClock.c
* @author  
* @version V1.0.0
* @date    03/20/2013
* @brief   
********************************************************************************
*/
#include <stdint.h> 
#include <time.h> 
#include <stdio.h> 
#include <stdlib.h>
#include "ConfigAll.h" 
#include "DevInfo.h" 
#include "DevLib.h" 
#include "DevRule.h"
#include "TickClock.h" 
#include "rn8209.h" 
#include "RtTime.h" 
#include "KeyDrv.h" 
#include "LED.h" 
#include "flash.h" 
#include "PowerStaticsRecord.h"
#include "SwitchApp.h"
#include "HttpdCallBack.h"
#include "Produce_Test.h"
#include "UdpServerCallBack.h" 
#include "read_power.h"
#include "PublicServerCom.h" 
#include "security_mode.h"




uint32_t testSec = 0;
SysTimeType                 SysTimeObj;
time_t reportHourTime = 0;

//uint8_t getStatusMark = 0;			//是否读到开关状态



extern FwUpdateStatusType   FwUpdateStatusObj;
extern DevStatusType        DevStatusObj;
extern int                  DevModeStatus; 
extern DevTimeType          DevTimeObj;
extern uint8_t              addReportRtHourMark;            //补报实时功耗标志
extern uint8_t testMark;
extern uint8_t reportRTHourmark;                                   //上报小时实时功耗标志
extern uint8_t isReportRTHourSuccess;                              //上报小时实时功耗成功与否标志
extern uint8_t reportCountDownMark;								  //上报倒计时信息
extern uint8_t RuleReportMark;									//上报规则信息
extern uint8_t reportMasterSlaveInfo;					 		//上报主从信息
extern uint8_t reportRebootInfo;								//上报端口Reboot信息
extern uint8_t recycleResolveDomain;					//定时解析从服务器标志位
extern uint8_t isGetRegid;             					//是否获取到了设备账号

extern DevRuleInfoType         DevRuleInfoObj; 




/*******************************************************************************
* 功能描述: 
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/ 
#if  !defined(__DS1302_RTC_ENABLE__)  
void SysTime_Init(void)
{
    //if(HF_SUCCESS == hfthread_mutext_new(&SysTimeObj.SysUnixTimeMutx))
    { 
        SysTimeObj.SysUnixTime  = 946656000;                    // 2000.1.1 0:0:0 
        SysTimeObj.SysTimeCount = 0;
    } 
}
/*******************************************************************************
* 功能描述: 
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/ 
extern void SysTime_Set(struct tm *iTime) 
{ 
    //if(HF_SUCCESS == hfthread_mutext_trylock(SysTimeObj.SysUnixTimeMutx)) 
    { 
        SysTimeObj.SysUnixTime  = mktime(iTime); 
        SysTimeObj.SysTimeCount = 0; 
    	
        //hfthread_mutext_unlock(SysTimeObj.SysUnixTimeMutx);
    } 
} 
#endif 


#define     READ_POWER_PER_1_SEC        3 
uint8_t testMark = 0;
extern void SysTime_GetTime(void) 
{
    static int  Read8209TimeCtl = 100;
    //char testBuf[100] = {0};

    #if  !defined(__DS1302_RTC_ENABLE__)  
	time_t      iTime;
    struct tm   *pTime;
	#endif 
    
    if(isBitSet(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_30_IS_SEC_TICK))
    {
        ClrBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_30_IS_SEC_TICK); 
        // 
       #if  defined(__DS1302_RTC_ENABLE__) 
        get_rtctime(&DevTimeObj.NowTimeTm);
        #else 
    	iTime                   = (time_t)SysTimeObj.SysUnixTime;            //获取当前RTC时间 
        pTime                   = localtime(&iTime);                         //将当前RTC时间转换成struct tm时间 
        pTime->tm_year         += 1900;
        pTime->tm_mon          += 1;                                         //标准的struct tm月份范围为:0-11,将其转换成用户时间
        DevTimeObj.NowTimeTm    = *pTime; 
        DevTimeObj.NowTimeSec   = SysTimeObj.SysUnixTime;
       #endif 
	   
        //calHourPowerTime++;   
        Read8209TimeCtl++;
        CheckSwState();

	#ifndef TEST	
		if(DevTimeObj.NowTimeTm.tm_min==0 && DevTimeObj.NowTimeTm.tm_sec<5)			//每小时统计一次功耗
		{
			SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_9_IS_HOURS_DATA_CMD);
		}
     
        if(Read8209TimeCtl>=130)
        {
            if(DevTimeObj.NowTimeTm.tm_min<2 && reportHourTime==0)			//每小时上报一次功耗
            { 
                Read8209TimeCtl = 0;
                srand(DevTimeObj.NowTimeSec);
                reportHourTime = DevTimeObj.NowTimeSec+rand()%110+10;				//整点随机两分钟内上报
            }
        }
	#else
		if((DevTimeObj.NowTimeTm.tm_min%5 == 0) && DevTimeObj.NowTimeTm.tm_sec<5)			//每5分钟统计一次功耗
		{
			SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_9_IS_HOURS_DATA_CMD);
		}
	
        if(Read8209TimeCtl>=70)
        {
            if((DevTimeObj.NowTimeTm.tm_min%5 ==0) && reportHourTime==0)
            { 
                Read8209TimeCtl = 0;
                srand(DevTimeObj.NowTimeSec);
                reportHourTime = DevTimeObj.NowTimeSec+rand()%50+10;				//整点随机两分钟内上报
                //u_printf("set:reportHourTime = %d   rand=%d\n",reportHourTime,rand()%300);
            }
        }
	#endif
		ReportHourData();                                               //查询上报小时功耗时间点是否到达

     
     	if(RelatedCtrl.ctlMark)
	    {
	   		//RelatedCtrl.ctlTime++;

			//sprintf(testBuf,"ctlTime=%d\t holdTime=%d\n",RelatedCtrl.ctlTime,RelatedCtrl.holdTime*60);
			//UdpDebugPrint(testBuf, sizeof(testBuf));
			//u_printf("curTime=%ld\t ctlTime=%ld\t holdTime=%d\n",DevTimeObj.CurTimeSec,RelatedCtrl.ctlTime,RelatedCtrl.holdTime*60);
			
			if(DevTimeObj.CurTimeSec >= RelatedCtrl.delayTime+RelatedCtrl.ctlTime)
			{
				if(!RelatedCtrl.afterMark)
				{
					Switch_delayCtl(RelatedCtrl.ctrStates);
					RelatedCtrl.afterMark = 1;

					//UdpDebugPrint("Now excute related control!\r\n",sizeof("Now excute related control!\r\n")-1);
				}		
			}

			if(RelatedCtrl.holdTime > 0)
			{
				if(DevTimeObj.CurTimeSec >= RelatedCtrl.holdTime+RelatedCtrl.delayTime+RelatedCtrl.ctlTime)
				{
					Switch_delayCtl(RelatedCtrl.preStates);
					RelatedCtrl.ctlMark = 0;
					RelatedCtrl.ctlTime = 0;

					//UdpDebugPrint("Now recover related control!\r\n",sizeof("Now recover related control!\r\n")-1);
				}
			}
	    }

     
        if(0x00 == (Read8209TimeCtl%5))                                  
        {
            ModifyDevMac();
        }

    }      
}



/*******************************************************************************
* 功能描述:查询上报小时功耗时间是否到达
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/ 
void ReportHourData(void)
{
    if(DevTimeObj.NowTimeSec>=reportHourTime && (reportHourTime!=0))
    {  
        reportHourTime = 0;
         SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_SEND_BIT_22_HOUR_DATA_CMD);
        //u_printf("reportHourTime = %d    DevTimeObj.NowTimeSec=%ld\n",reportHourTime,DevTimeObj.NowTimeSec);         
    }  
}


/*******************************************************************************
* 功能描述:秒定时器中断
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/ 
void USER_FUNC app_timer_callback1( hftimer_handle_t htimer )
{ 
	#if(PORT_NUM == 6)
	uint8_t i = 0;
	#endif
    static int TimerCount = 0; 	
	static uint8_t readDataStatusMark = 0;
	//char testBuf[100] = {0};
	

    if(isBitSet(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_6_IS_REBOOT_CMD)) 
    { 
        FwUpdateStatusObj.RebootCountDown--; 
    }

#if 1
	if(0x00 == (TimerCount%2))     //每2S检测一次wifi状态
	{
		CheckDevMode();
	}
#endif
	

		readDataStatusMark = !readDataStatusMark;
       #if (PORT_NUM==6)
	/************************每2s交替读取一次端口数据和状态值,其他命令的下发优先********************************/	
		if(readDataStatusMark)
		{
			//UdpDebugPrint("Now we get the switch info!!!",strlen("Now we get the switch info!!!"));
			UsartReadPortData();		//通过串口查询从机搜寻的实时数据
			if(TimerCount >= 5)
		    {
				masterSlaveControl();		//主从控制
			}
		}
		else
		{
			//getStatusMark = 1;
			UsartReadPortstatus();		  //每秒一次读取端口的开关状态
		}
			
      #elif (PORT_NUM==1)
		   RN8209_READ();
      #endif
	  OverCurrentProtectPoll();   //过流保护

	


	if(0x00 == (TimerCount%5))     //每5S上报规则信息
    { 
		
        SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_0_IS_REG); 

		
        
        if((surplusRecordCount>0) && (isBitSet(iStaticsFlag, WIFI_WORK_NORMAL)))
		{
			SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_SEND_BIT_21_ADD_HOUR_DATA_CMD);		  //上报历史功耗
		}
		
		
    	if(RuleReportMark)
    	{
        	SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_7_IS_REPORT_RULE_CMD);
    	}
		if(reportCountDownMark)
		{
			SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_15_REPORT_COUNTDOWN_CMD);
		}
		if(reportMasterSlaveInfo)
		{
			SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_16_REPORT_MASTER_SLAVE_CMD);	
		}
		if(reportRebootInfo)
		{
			SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_17_REPORT_PORT_REBOOT_CMD);	
		}
		if(reportSecurityInfo)
		{
			SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_REPORT_BIT_20_SECURITY_MODE_CMD);		
		}

		if(!hasSynInfo)
		{
			SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_12_IS_ALL_DATA_CMD);
		}
	}				

    if(0x00 == (TimerCount%10))     //每10秒钟检测主、从服务器域名
    { 
        SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_1_IS_MAIN_DOMAIN);
        SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_3_IS_SUB_DOMAIN);

		//PublicServer_SyncBaseInfoWithServer();				//同步设备信息
    } 
   
    if(0x00 == (TimerCount%30)) 
    { 
    	
        SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_8_IS_SYNC_RT_DATA_CMD);        
        SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_4_IS_GET_SOCKETA_FD); 

        if((IS_ON == isDevReg)&&(IS_ON == FwUpdateStatusObj.isSubSvrOK)) 
		{
			heartBeatCount++;
		}
		
		ReadPowerFreq();												//读电源频率
    } 
   
    if(0x00 == (TimerCount%60))
    {
        SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_10_ADD_REPORT_HOURS_DATA_CMD);   

		SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_18_REBOOT_CONTROL_CMD);					//检测是否执行端口Reboot		
		SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_DETECTE_BIT_19_DEV_SECURITY_MODE_CMD); 

		if(reportRTHourmark && !isReportRTHourSuccess)              //上报实时功耗未成功，则每分钟补报一次
        {
        	SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_SEND_BIT_22_HOUR_DATA_CMD);		  //上报当前小时功耗
        }

		if(SecurityModeExcute.intervalTime > 0)
		{
			SecurityModeExcute.intervalTime--;
			if(0 == SecurityModeExcute.intervalTime)
			{
				SecurityModeExcute.RunningMark = 0;
				SecurityModeExcute.nextIntervalTime = 0;

			  #if(PORT_NUM == 6)
			  {
				while((i<PORT_NUM))
			    {
			        if(SecurityMode.PortArr[i]==1)
			        {	
				          SetPortState(i+1,0);            				       
			    	}
				
			        i++;
			    }  
				msleep(200);
			    UsartControlPortOnOff(); 
			  }
			  #elif (PORT_NUM == 1)
			  {
			    if(SecurityMode.PortArr[0]==1)			//翻转对应的IO口
			    {				
			        Port_TurnOnOff(0);					
					plug_updata = 1;
			     }
			  }
			  #endif
			}
		}

		if(SecurityModeExcute.RunningMark)
		{
			if(SecurityModeExcute.nextIntervalTime > 0)
			{
				SecurityModeExcute.nextIntervalTime--;
				if(0 == SecurityModeExcute.nextIntervalTime)
				{
					SecurityModeExcute.timeOut = 1;				//防盗时间超时
				}
			}
		}
		

    }

	if(0x00 == (TimerCount%120))
	{
		recycleResolveDomain = 1;
	}

    if(!isGetRegid)
    {
		 SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_SEND_BIT_23_BROADCAST_INFO_CMD);			//上报广播报信息
    }
	
    SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_30_IS_SEC_TICK);
    
    SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_EXCUTE_BIT_11_DEV_INIT);
    
    TimerCount++; 

    //testMark++;
    
   #if  !defined(__DS1302_RTC_ENABLE__)  
    { 
        SysTimeObj.SysUnixTime++; 
        SysTimeObj.SysTimeCount++; 

    } 
   #endif

   testSec++;
} 


void ShowWorkMode(void)
{
#if 0
    if(0x00 == isBitSet(DevStatusObj.LedShowStatus, WIFI_LED_SHOW_1_BIT))
    {
        return;
    }
    ClrBit(DevStatusObj.LedShowStatus, WIFI_LED_SHOW_1_BIT);

    if(AP_MODE_LED == DevModeStatus)
    {
        LedBlink(2);
    }
#endif

    if(isBitSet(DevStatusObj.LedShowStatus, WIFI_LED_SHOW_0_BIT)) 
    {
        LedBlink(1);
    }
}
/*******************************************************************************
* 功能描述: 
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/ 
void USER_FUNC clock_timer_callback( hftimer_handle_t htimer )
{ 
    static int ClockCount = 0; 
    // 
    Key_DriveEngine(ClockCount); 

    ClockCount++; 
    // 
    #if  defined(__SCAN_KEY_IN_TICK_TIMER__)
    Key_Scan(); 
    Key_Poll(); 
    #endif 
    
    //SetBit(DevStatusObj.LedShowStatus, WIFI_LED_SHOW_1_BIT);
} 
/******************************************************************************* 
* ========================= END OF PAGE 
*******************************************************************************/ 

