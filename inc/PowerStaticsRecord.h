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





extern uint32_t	readPowerCount;										//读取功耗次数
extern uint32_t TempPowerStaticsSum[PORT_NUM+1];


#if 0
//有时间戳时对应的数据结构
typedef struct PortHistoryDataStruct
{ 
    int32_t     WattRec_Hour[30+1][24+1];     // 一个月的小时的功耗数据,WattRec_Hour[0][0]对应31号24小时的数据
}PortHistoryDataObj;


//无时间戳时对应的数据结构
typedef struct PortNoTimeHistoryDataStruct
{ 
    int32_t     timeStack[RECORD_DAYS*24+1];            //数据对应的时间戳
    int32_t     WattRec_Hour[RECORD_DAYS*24+1];    // 保存RECORD_DAYS天的无时间数据
}PortNoTimeHistoryDataObj;



typedef struct HistoryRecordStruct 
{
    uint8_t  isRecorverFlag;                     //无时间数据是否恢复标志
    uint8_t  LastMonthDayCount;                  // 上个月天数     
    uint8_t  iHourPos;                           //整点:0--23
    uint8_t  iDayPos;                            //日期:0--(上个月天数-1) 
    uint32_t iHourLastTime;                      //时间戳对应的秒数
    int  isHaveDataSaveFlag;
}RecordDataType; 
#endif

typedef struct CurrentDataStruct
{ 
    int ValidFreq;       		//频率有效值  
	int ValidCurrent;    		// 电流有效值 	
	int ValidVolt;       		// 电压有效值 	
	int ValidPower;      		// 有功功率
	int hourPower;				//每小时总功耗
}CurrentDataType;




typedef struct HistoryDataStruct
{ 
    time_t timeStack;            //数据对应的时间戳
    int power[PORT_NUM];    // 保存RECORD_DAYS天的无时间数据
}HistoryDataObj;


typedef struct DevOffLineDataStruct
{ 
    uint16_t recordCount;           				 //数据条数
    HistoryDataObj recordData[RECORD_DAYS*24+1];    // 保存RECORD_DAYS天的断网数据
}DevOffLineDataObj;

extern uint8_t iStaticsFlag;                        //标志位组合变量


//extern uint8_t reportHourDataFlag;			//上报历史数据标志位
extern uint16_t sendRecordCount;		//补报的数据个数
extern uint16_t surplusRecordCount;						//剩余的数据个数
extern HistoryDataObj HistoryData;					//小时功耗数�
extern DevOffLineDataObj devOffLineData;				//断网时数据
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

