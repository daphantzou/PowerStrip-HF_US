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





extern uint32_t	readPowerCount;										//��ȡ���Ĵ���
extern uint32_t TempPowerStaticsSum[PORT_NUM+1];


#if 0
//��ʱ���ʱ��Ӧ�����ݽṹ
typedef struct PortHistoryDataStruct
{ 
    int32_t     WattRec_Hour[30+1][24+1];     // һ���µ�Сʱ�Ĺ�������,WattRec_Hour[0][0]��Ӧ31��24Сʱ������
}PortHistoryDataObj;


//��ʱ���ʱ��Ӧ�����ݽṹ
typedef struct PortNoTimeHistoryDataStruct
{ 
    int32_t     timeStack[RECORD_DAYS*24+1];            //���ݶ�Ӧ��ʱ���
    int32_t     WattRec_Hour[RECORD_DAYS*24+1];    // ����RECORD_DAYS�����ʱ������
}PortNoTimeHistoryDataObj;



typedef struct HistoryRecordStruct 
{
    uint8_t  isRecorverFlag;                     //��ʱ�������Ƿ�ָ���־
    uint8_t  LastMonthDayCount;                  // �ϸ�������     
    uint8_t  iHourPos;                           //����:0--23
    uint8_t  iDayPos;                            //����:0--(�ϸ�������-1) 
    uint32_t iHourLastTime;                      //ʱ�����Ӧ������
    int  isHaveDataSaveFlag;
}RecordDataType; 
#endif

typedef struct CurrentDataStruct
{ 
    int ValidFreq;       		//Ƶ����Чֵ  
	int ValidCurrent;    		// ������Чֵ 	
	int ValidVolt;       		// ��ѹ��Чֵ 	
	int ValidPower;      		// �й�����
	int hourPower;				//ÿСʱ�ܹ���
}CurrentDataType;




typedef struct HistoryDataStruct
{ 
    time_t timeStack;            //���ݶ�Ӧ��ʱ���
    int power[PORT_NUM];    // ����RECORD_DAYS�����ʱ������
}HistoryDataObj;


typedef struct DevOffLineDataStruct
{ 
    uint16_t recordCount;           				 //��������
    HistoryDataObj recordData[RECORD_DAYS*24+1];    // ����RECORD_DAYS��Ķ�������
}DevOffLineDataObj;

extern uint8_t iStaticsFlag;                        //��־λ��ϱ���


//extern uint8_t reportHourDataFlag;			//�ϱ���ʷ���ݱ�־λ
extern uint16_t sendRecordCount;		//���������ݸ���
extern uint16_t surplusRecordCount;						//ʣ������ݸ���
extern HistoryDataObj HistoryData;					//Сʱ�������
extern DevOffLineDataObj devOffLineData;				//����ʱ����
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

