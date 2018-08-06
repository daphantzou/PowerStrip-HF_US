/*
********************************************************************************
* @file    : RtTime.h
* @author  :
* @version : V1.0.0
* @date    : 2014/06/24 
* @brief   : 
* @Modify  : 
********************************************************************************
*/
#ifndef   __RT_TIME_H__ 
#define   __RT_TIME_H__ 
// 
#include "hsf.h"
#include <time.h>

typedef struct 
{
    struct tm NowTimeTm;
    time_t       NowTimeSec;               //格林尼治时间
    time_t       CurTimeSec;               //本地时间换算成的秒数
}DevTimeType;
//

 
extern	DevTimeType 				DevTimeObj;


USER_FUNC void  get_rtctime(struct tm *p);
USER_FUNC int   set_rtctime(struct tm *timep);

#if  defined(__DS1302_RUN_STATUS_DBG__)
extern    int          HttpGetSencond(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf);
#endif 

extern    void          Rtc_Init(void);
extern    DevTimeType   *GetCurTime(void);
extern    void GetDevTime(void);

// 
#endif 
/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/

