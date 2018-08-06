/*
********************************************************************************
* @file    : TickClock.h
* @author  :
* @version : V1.0.0
* @date    : 2014/06/24 
* @brief   : 
* @Modify  : 
********************************************************************************
*/
#ifndef   __TICK_CLOCK_H__ 
#define   __TICK_CLOCK_H__  
// 
#include "ConfigAll.h" 
#include "hsf.h" 
#include "time.h" 
// 
#define APP_TIMER_ID		        (1)
#define CLOCK_TIMER_ID		        (2)

// 
// 1S计数值 1008:(330) fast 
// 1S计数值 1015:(?) low 
// 1S计数值 1013:(?) fast 1s/hour
// 1S计数值 1014:(?)
#define         ONE_SEC_COUNT       1013 
// 
extern void USER_FUNC clock_timer_callback( hftimer_handle_t htimer );
extern void USER_FUNC app_timer_callback1( hftimer_handle_t htimer );

#if  !defined(__DS1302_RTC_ENABLE__) 
extern void SysTime_Set(struct tm *iTime);
extern void SysTime_Init(void);
#endif 

extern void ShowWorkMode(void);
extern void SysTime_GetTime(void);
extern void ReportHourData(void);
//
#endif 
/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/

