/*
********************************************************************************
* @file    PowerStaticsRecord.h
* @author  
* @version V1.0.0
* @date    2014/06/24
* @brief   
********************************************************************************
*/
#ifndef  __POWER_STATICS_RECORD_H__ 
#define  __POWER_STATICS_RECORD_H__ 

#include <hsf.h> 
#include "ConfigAll.h" 

#define             STATICS_POWER_AND_SAVE_0_FLAG       0x00 
#define             WIFI_WORK_NORMAL      0x01 
#define             HAD_COMBINE_STATICS_TO_TIME_2_FLAG  0x02 

#define             RECORD_HOUR_TYPE                    0x00 

#if (PORT_NUM == 6)
 #define             RECORD_DAYS                         3
#elif (PORT_NUM == 1)
 #define             RECORD_DAYS                         30
#endif
#define             RECORD_HOURS                        RECORD_DAYS*24


//#define TEST 	1





extern uint32_t	readPowerCount;										//¶ÁÈ¡¹¦ºÄ´ÎÊý
extern uint32_t TempPowerStaticsSum[PORT_NUM+1];


#if 0
//ÓÐÊ±¼ä´ÁÊ±¶ÔÓ¦µÄÊý¾Ý½á¹¹
typedef struct PortHistoryDataStruct
{ 
    int32_t     WattRec_Hour[30+1][24+1];     // Ò»¸öÔÂµÄÐ¡Ê±µÄ¹¦ºÄÊý¾Ý,WattRec_Hour[0][0]¶ÔÓ¦31ºÅ24Ð¡Ê±µÄÊý¾Ý
}PortHistoryDataObj;


//ÎÞÊ±¼ä´ÁÊ±¶ÔÓ¦µÄÊý¾Ý½á¹¹
typedef struct PortNoTimeHistoryDataStruct
{ 
    int32_t     timeStack[RECORD_DAYS*24+1];            //Êý¾Ý¶ÔÓ¦µÄÊ±¼ä´Á
    int32_t     WattRec_Hour[RECORD_DAYS*24+1];    // ±£´æRECORD_DAYSÌìµÄÎÞÊ±¼äÊý¾Ý
}PortNoTimeHistoryDataObj;



typedef struct HistoryRecordStruct 
{
    uint8_t  isRecorverFlag;                     //ÎÞÊ±¼äÊý¾ÝÊÇ·ñ»Ö¸´±êÖ¾
    uint8_t  LastMonthDayCount;                  // ÉÏ¸öÔÂÌìÊý     
    uint8_t  iHourPos;                           //Õûµã:0--23
    uint8_t  iDayPos;                            //ÈÕÆÚ:0--(ÉÏ¸öÔÂÌìÊý-1) 
    uint32_t iHourLastTime;                      //Ê±¼ä´Á¶ÔÓ¦µÄÃëÊý
    int  isHaveDataSaveFlag;
}RecordDataType; 
#endif

typedef struct CurrentDataStruct
{ 
    int ValidFreq;       		//ÆµÂÊÓÐÐ§Öµ  
	int ValidCurrent;    		// µçÁ÷ÓÐÐ§Öµ 	
	int ValidVolt;       		// µçÑ¹ÓÐÐ§Öµ 	
	int ValidPower;      		// ÓÐ¹¦¹¦ÂÊ
	int hourPower;				//Ã¿Ð¡Ê±×Ü¹¦ºÄ
}CurrentDataType;




typedef struct HistoryDataStruct
{ 
    time_t timeStack;            //Êý¾Ý¶ÔÓ¦µÄÊ±¼ä´Á
    int power[PORT_NUM];    // ±£´æRECORD_DAYSÌìµÄÎÞÊ±¼äÊý¾Ý
}HistoryDataObj;


typedef struct DevOffLineDataStruct
{ 
    uint16_t recordCount;           				 //Êý¾ÝÌõÊý
    HistoryDataObj recordData[RECORD_DAYS*24+1];    // ±£´æRECORD_DAYSÌìµÄ¶ÏÍøÊý¾Ý
}DevOffLineDataObj;

extern uint8_t iStaticsFlag;                        //±êÖ¾Î»×éºÏ±äÁ¿


//extern uint8_t reportHourDataFlag;			//ÉÏ±¨ÀúÊ·Êý¾Ý±êÖ¾Î»
extern uint16_t sendRecordCount;		//²¹±¨µÄÊý¾Ý¸öÊý
extern uint16_t surplusRecordCount;						//Ê£ÓàµÄÊý¾Ý¸öÊý
extern HistoryDataObj HistoryData;					//Ð¡Ê±¹¦ºÄÊý¾
extern DevOffLineDataObj devOffLineData;				//¶ÏÍøÊ±Êý¾Ý
extern CurrentDataType CurrentDataObj[PORT_NUM];

extern void PowerRecord_StaticsData(void);
extern void PowerRecord_StartCombinesRecord(void);

//extern void PowerRecord_RecorverPowerStatics(void);
extern char CaculateTmpRecord(void);

//extern void ReportHourPowerConsume(int dataID);

//extern void DevAddReportPowerConsume(void);
extern void send_history_data(void);
extern void PowerStatics_Init(void); 
 
#endif 
/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/

