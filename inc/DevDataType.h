/* 
********************************************************************************
* @file    : DevDataType.h 
* @author  : 
* @version : V1.0.0 
* @date    : 2014/06/17
* @brief   : 
********************************************************************************
*/ 
#ifndef   __DEVICE_DATA_TYPE_H__
#define   __DEVICE_DATA_TYPE_H__ 
// 
#include <stdint.h> 

#include "hsf.h"
#include "DevInfo.h"
#include "ConfigAll.h"
// 
#define         BOOT_INFO_DATA_BUF_LENGTH       48 

#define         DEV_RULE_COUNT_MAX                                  6           // ���������Ŀ

#define         RULE_SENSE_COUNT_MAX                                4           // 


#define         TARGE_ID_LENGTH                                     16          // �Ų����ֳ��� 
#define         SOCKET_INTERFACE_COUNT                              6           // ÿ���Ų����ӿ��� 
#define         SOCKET_RULE_COUNT                                   12          // �豸��������
#define			PAGE_COUNT											12			// ÿҳ��������Ĺ�������
#define         SENSOR_NODE_ID_LENGTH                               4           // 
#define         SENSOR_NODE_DATA_LENGTH_MAX                         19          // Sensor���ݽ��������󳤶� 
#define         DAY_COUNT_OF_WEEK                                   7           // 
// 
#define         ONCE_RULE_INVALID_ING                               0x01        // ��ʱ������Ч�Ա�־ 

////////////////////////////////////////////////////////////////////////////////// 
#define         HFGPIO_SW_KEY_CTL_PIN		        (HFGPIO_F_USER_DEFINE + 0) 
#define         HFGPIO_SW_ON_OFF_CTL_PIN		    (HFGPIO_F_USER_DEFINE + 1) 

#define         SCLK 		                        (HFGPIO_F_USER_DEFINE + 2)
#define         SDA	 		                        (HFGPIO_F_USER_DEFINE + 3)
#define         RST			                        (HFGPIO_F_USER_DEFINE + 4)

#define         ZX_INT_CHECK		                (HFGPIO_F_USER_DEFINE + 5) 
#define         HFGPIO_SW_LED		                HFGPIO_F_NLINK  
//#define         HFGPIO_SW_LED		                (HFGPIO_F_USER_DEFINE + 6) 


// 
typedef enum 
{ 
     IS_ON   = 1,
     IS_OFF  = 0,
}RestartType; 

typedef enum  EnState
{
    ENABLE_OFF      = 0,
    ENABLE_ON       = 1,
}EnStateType;

typedef enum  EnG_State
{
    OP_NULL         = 0,
    OP_START_END    = 1,
    OP_START        = 2,
    OP_END          = 3,
}OpStateType; 

typedef enum  SwitchOperation
{
    SW_CLOSE        = 0,
    SW_OPEN         = 1,
    SW_NULL         = 2,
}SwitchOpType; 

typedef enum  PortChangeReason
{
    PORT_NO_CHANGE        			= 0,
    PORT_CHANGE_BY_SERVER         	= 1,
    PORT_CHANGE_BY_KEY				= 2,
    PORT_CHANGE_BY_REBOOT			= 3,
    PORT_CHANGE_BY_OVERCURRENT		= 4,
    PORT_CHANGE_BY_MSCONTROL		= 5,
    PORT_CHANGE_BY_RULE				= 6,
    PORT_CHANGE_BY_COUNTDOWN		= 7,
    PORT_CHANGE_BY_AUTO				= 8,
}PortChangeReasonType; 


typedef struct OpHour
{ 
    uint8_t             iHour;
    uint8_t             iMinute;
    uint32_t            iTotalMin;
}OpHourType;

//�Ų�˿ڿ���״̬�ṹ��
typedef struct PortStatus
{
    char            name[NAME_LENGTH];              //the port name
    uint8_t         OldStatus;
    uint8_t         CurStatus; 
    uint8_t         OnOffStatus;
    uint8_t         msType;                     //the type of port:master or salve
    uint8_t			rebootPort;					//the type of port:reboot or not
}PortStatusType;


//smart_link������Ϣ�ṹ��
typedef struct DevSmartLinkInfoStruct
{
    int             isDevSmartLinkSaveFlg;                  //is save or not flag        
    char            ssid[SSID_LENGTH];                      //ssid
    char            password[KEY_LENGTH];                  //the type of port:master or salve
    uint8_t         security;                              //encry
}DevSmartLinkInfoType;



typedef struct SwitchOnOffStatus
{ 
	uint8_t         LimitData;					//�·���limit���
	uint8_t         masterLimitWatt;
	uint8_t         isRebootEnable;				//Reboot�����Ƿ���Ч
	uint8_t			portChangeReason;			//port �仯ԭ��
    int             isDevStatusSaveFlg;     
    PortStatusType  PortInfor[PORT_NUM+1];
}SwInformationType;


typedef struct SwitchRuleStruct 
{ 
	int8_t         RuleEnable;                             // �����Ƿ���Ч��־ 
	
    uint8_t         SwOpDateObj;                            // ��λ��ʾ���ڼ� 
	uint8_t         PortArr[PORT_NUM+1];                    // �Ըù������ִ�еĶ���:0(��) 1(��) 2(�޹�) 
	// 
    uint8_t         OnceRuleFlag;                           // ѭ�������Ƿ����ù���־ 
    //uint8_t         isValidTurnOnOff;                       // 0x01:TurnOn; 0x02:TurnOff: 0x03:TurnOnOff 
    uint32_t        timeStamp;                              // ���ι���ĸ�������ʱ��� 
    uint32_t        CurTimeStamp;                          // ���ι���ı���ʱ��� 
    uint32_t        timeLong;                               // ��ʾ�򿪻�رյ�ʱ����뵱��0��ķ�����
    uint32_t        exeTimeLong;                               // ��ʾ�򿪻�رյ�ʱ����뵱��0��ķ�����
    uint32_t         RuleID;                                 // ����ID
}SwitchRuleType; 

typedef struct SecurityModeStruct 
{ 
	uint8_t          enable;                             // �ŵ�ģʽ�Ƿ���Ч��־ 	
    uint8_t         SwOpDateObj;                            // ��λ��ʾ���ڼ� 
    uint8_t         PortArr[PORT_NUM+1];                    // �Ըù������ִ�еĶ���:0(��) 1(��) 2(�޹�) 
    uint16_t        startTime;                              //��ʼʱ��������ķ��Ӳ�
    uint16_t        endTime;                          		// ����ʱ��
    int             isSaveFlg;     						//���������Ƿ񱣴�ɹ�
}SecurityModeType; 


typedef struct SecurityModeExcuteStruct 
{
	uint8_t			timeOut;										//��һ�η���ʱ�䳬ʱ
	uint8_t 		nextIntervalTime;								// ִ����һ�η���ģʽ�������ڵ�ʱ��
	uint8_t 		RunningMark;									// ѭ��ģʽ�Ƿ���ִ����
	int16_t         intervalTime;  									// ����ģʽ�ļ��ʱ��
}SecurityModeExcuteType;



typedef struct DevRuleStruct 
{
	uint8_t         RuleCount;                            // ����ʵ������ 
    int             isRuleSaveFlag;                         // 
    SwitchRuleType  SwitchRulePool[SOCKET_RULE_COUNT+1];   // 
}DevRuleInfoType;

typedef struct CountDownTimeStruct 
{ 
	// ������Чʱ�� 
	uint8_t         isCountDown;                            // �Ƿ���Ч
	uint8_t         onOrOff;                                // ����ִ�еĶ���:On:1  Off:0
    // ����ʱԭʼ���� 
	uint32_t        StartTimePoint;                         // ��ʼ����ʱʱ�� 
	uint32_t        ReceiveStartTime;                       // ���յ��Ŀ�ʼ����ʱʱ��
	uint32_t        CountDownValue;                         // ����ʱ��ʱʱ��(��λ:s) 
}TurnOffTimeType; 

typedef struct UsartOrderStruct
{
    char     start;
    char     length;
    char     cmd;
    char     portID;
    char     data[45];
    char     checkSum;
    char     lEnd;
    char     hEnd;
}UsartOrderType;



typedef struct UsartSendOrderStruct
{  
    uint8_t OrderMark;
    uint8_t ACK;
    uint8_t sendCount;
}UsartSendOrder;


//�Ų�˿ڿ���״̬�ṹ��
typedef struct RelatedCtrlStatus
{
    uint8_t preStates;			//��������֮ǰ״̬
    uint8_t ctrStates;			//��������״̬
	uint8_t ctlMark;				//�������Ʊ�־λ
	uint8_t afterMark;					//�ӳٿ���ִ�б�־λ
	int delayTime;						//���������Ӻ�ʱ��
	int holdTime;						//����������ʱ�ָ�ʱ��
	uint32_t ctlTime;					//��������ʱ���ʱ
}RelatedCtrlType;


#endif 
/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/
