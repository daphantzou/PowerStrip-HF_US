/*
********************************************************************************
* @file    RtTime.c
* @author  
* @version V1.0.0
* @date    03/20/2013
* @brief   
********************************************************************************
*/
#include <stdint.h> 
#include <stdio.h> 

#include "ConfigAll.h" 

#include <hsf.h>
#include <hfsys.h>

#include "DevDataType.h" 
#include "DevInfo.h" 
#include "DevLib.h"
#include "TickClock.h"

#include "RtTime.h"

// 
#define WRITE_SECOND	0x80
#define WRITE_MINUTE	0x82
#define WRITE_HOUR		0x84
#define WRITE_DAY		0x86
#define WRITE_MON		0x88
#define WRITE_WDAY		0x8a
#define WRITE_YEAR		0x8c

#define READ_SECOND		0x81
#define READ_MINUTE		0x83
#define READ_HOUR		0x85
#define READ_DAY		0x87
#define READ_MON		0x89
#define READ_WDAY		0x8b
#define READ_YEAR		0x8d

#define WRITE_PROTECT	0x8E
// 
USER_FUNC static void           Writebyte(unsigned char addr, unsigned char data);
USER_FUNC static unsigned char  Readbyte(unsigned char addr);
USER_FUNC static void           num2hex(struct tm *timep);
// 
DevTimeType  DevTimeObj;
/*******************************************************************************
* 功能描述: 
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/ 
extern void Rtc_Init(void)
{
	Writebyte(WRITE_PROTECT, 0x00);
	Writebyte(WRITE_SECOND,  0);
	Writebyte(WRITE_PROTECT, 0x80);
}
/*******************************************************************************
* 功能描述: 
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/ 
#if  defined(__DS1302_RUN_STATUS_DBG__)
extern int HttpGetSencond(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf) 
{
    return sprintf(pAckBuf, "{\"response\":%d,\"code\":%d,\"sec\":%d}", 
					  iCmd, (200), DevTimeObj.NowTimeSec); 
}
#endif 
/*******************************************************************************
* 功能描述: 
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/ 
extern DevTimeType *GetCurTime(void)
{
    return &DevTimeObj;
}

/*******************************************************************************
* 功能描述: 获取当前系统时间戳
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/ 
extern FwUpdateStatusType      FwUpdateStatusObj;
extern uint32_t testSec;


extern void GetDevTime(void)
{
    //DevTimeType     *pCurTime;

    //pCurTime    = GetCurTime();

    if(isBitSet(FwUpdateStatusObj.TimelyCheck, TIMELY_EXCUTE_BIT_11_DEV_INIT))
    {
        ClrBit(FwUpdateStatusObj.TimelyCheck, TIMELY_EXCUTE_BIT_11_DEV_INIT);

       #if 0
        u_printf("Print NowTimeSec = %d,%d-%d-%d %02d:%02d:%02d\r\n", 
	            pCurTime->NowTimeSec, 
	            pCurTime->NowTimeTm.tm_year,
	            pCurTime->NowTimeTm.tm_mon,
	            pCurTime->NowTimeTm.tm_mday,
	            pCurTime->NowTimeTm.tm_hour,
	            pCurTime->NowTimeTm.tm_min,
	            pCurTime->NowTimeTm.tm_sec
	            );
	   #endif
	   //u_printf("testSec = %d\r\n",testSec);
	}  

}





/*******************************************************************************
* 功能描述: 
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/ 
extern DevInfoType      DevInfoObj;

void get_rtctime(struct tm *p)
{
    uint8_t iTemp;
    
	p->tm_sec  = Readbyte(READ_SECOND);
	p->tm_min  = Readbyte(READ_MINUTE);
	p->tm_hour = Readbyte(READ_HOUR);
	p->tm_mday = Readbyte(READ_DAY);
	p->tm_wday = Readbyte(READ_WDAY)-1;
	p->tm_mon  = Readbyte(READ_MON)-1;
	p->tm_year = Readbyte(READ_YEAR)+100;
	
	DevTimeObj.CurTimeSec = mktime(p);
	
   
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
#if  0      
	u_printf("NowTimeSec is = %d,%d-%d-%d %d:%d:%d\r\n", 
	            DevTimeObj.NowTimeSec, 
	            p->tm_year,
	            p->tm_mon,
	            p->tm_mday,
	            p->tm_hour,
	            p->tm_min,
	            p->tm_sec
	            );
 #endif
 
    p->tm_mon++;
	p->tm_year += 1900;
	return;
}
/*******************************************************************************
* 功能描述: 
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/ 
USER_FUNC int set_rtctime(struct tm *timep)
{
	#if  0 
	u_printf("Before NowTime T : %d-%d-%d %d:%d:%d\r\n", 
	            timep->tm_year,
	            timep->tm_mon,
	            timep->tm_mday,
	            timep->tm_hour,
	            timep->tm_min,
	            timep->tm_sec
	            );
	#endif 
	num2hex(timep);
	
	#if  0 
	u_printf("After NowTime T : %d-%d-%d %d:%d:%d\r\n", 
	            timep->tm_year,
	            timep->tm_mon,
	            timep->tm_mday,
	            timep->tm_hour,
	            timep->tm_min,
	            timep->tm_sec
	            );
	#endif 
	// 
	Writebyte(WRITE_PROTECT, 0x00);
	Writebyte(WRITE_SECOND,  timep->tm_sec);
	Writebyte(WRITE_MINUTE,  timep->tm_min);
	Writebyte(WRITE_HOUR,    timep->tm_hour);
	Writebyte(WRITE_DAY,     timep->tm_mday);
	Writebyte(WRITE_WDAY,    timep->tm_wday);
	Writebyte(WRITE_MON,     timep->tm_mon);
	Writebyte(WRITE_YEAR,    timep->tm_year);
	Writebyte(WRITE_PROTECT, 0x80);
	return 0;
}
/*******************************************************************************
* 功能描述: 
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/ 
USER_FUNC static void Writebyte(unsigned char addr, unsigned char data)
{
	unsigned char i, tmp;
	
	hfthread_suspend_all();
	hfgpio_fset_out_low(RST);
	hfgpio_fset_out_low(SCLK);
	delay_us(1);
	hfgpio_fset_out_high(RST);
	for(i=0; i<8; i++)
	{
		hfgpio_fset_out_low(SCLK);
		delay_us(1);
		tmp = addr;
		if(tmp & 0x01)
		{
			hfgpio_fset_out_high(SDA);
		}
		else
		{
			hfgpio_fset_out_low(SDA);
		}
		addr >>= 1;
		hfgpio_fset_out_high(SCLK);
		delay_us(1);
	}
	for(i=0; i<8; i++)
	{
		hfgpio_fset_out_low(SCLK);
		delay_us(1);
		tmp = data;
		if(tmp & 0x01)
		{
			hfgpio_fset_out_high(SDA);
		}
		else
		{
			hfgpio_fset_out_low(SDA);
		}
		data >>= 1;
		hfgpio_fset_out_high(SCLK);
	}
	hfgpio_fset_out_low(RST);
	hfthread_resume_all();
}
/*******************************************************************************
* 功能描述: 
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/ 
USER_FUNC static unsigned char Readbyte(unsigned char addr)
{
	unsigned char i,tmp,dat1;
	unsigned char dat2[5] = {0};
	
	hfthread_suspend_all();
	hfgpio_fset_out_low(RST);
	hfgpio_fset_out_low(SCLK);
	delay_us(1);
	hfgpio_fset_out_high(RST);
	for(i=0; i<8; i++)
	{
		hfgpio_fset_out_low(SCLK);
		tmp = addr;
		if(tmp & 0x01)
		{
	        hfgpio_fset_out_high(SDA);
		}
		else
		{
            hfgpio_fset_out_low(SDA);
		}
		addr >>= 1;
		delay_us(1);
		hfgpio_fset_out_high(SCLK);
		delay_us(1);
	}
	hfgpio_configure_fpin(SDA, HFM_IO_TYPE_INPUT);
	tmp = 0;
	for(i=0; i<8; i++)
	{
		tmp >>= 1;
		hfgpio_fset_out_low(SCLK);	
		if(hfgpio_fpin_is_high(SDA))
		{
			tmp |= 0x80;
		}
		hfgpio_fset_out_high(SCLK);
		delay_us(1);
	}
	hfgpio_fset_out_low(RST);
	sprintf((char*)dat2, "%x", tmp);
	dat1 = atoi((char*)dat2);
	hfthread_resume_all();
	return dat1;
}
/*******************************************************************************
* 功能描述: 
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/ 
USER_FUNC static void num2hex(struct tm *timep)
{
	timep->tm_sec   = timep->tm_sec/10*16  + timep->tm_sec%10;
	timep->tm_min   = timep->tm_min/10*16  + timep->tm_min%10;
	timep->tm_hour  = timep->tm_hour/10*16 + timep->tm_hour%10;
	timep->tm_mday  = timep->tm_mday/10*16 + timep->tm_mday%10;
	
	timep->tm_wday  = timep->tm_wday + 1;
	timep->tm_mon += 1;
	//timep->tm_mon   = timep->tm_mon/10*16  + timep->tm_mon%10 + 1;
	timep->tm_mon   = timep->tm_mon/10*16  + timep->tm_mon%10;
	
	timep->tm_year -= 100;
	timep->tm_year  = timep->tm_year/10*16 + timep->tm_year%10;
	//timep->tm_year  = timep->tm_year/10*16 + timep->tm_year%10+1;
}
/******************************************************************************* 
* ========================= END OF PAGE 
*******************************************************************************/ 

