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

#define         DEV_RULE_COUNT_MAX                                  6           // 规则最大数目

#define         RULE_SENSE_COUNT_MAX                                4           // 


#define         TARGE_ID_LENGTH                                     16          // 排插名字长度 
#define         SOCKET_INTERFACE_COUNT                              6           // 每个排插最大接口数 
#define         SOCKET_RULE_COUNT                                   12          // 设备最大规则数
#define			PAGE_COUNT											12			// 每页规则包含的规则条数
#define         SENSOR_NODE_ID_LENGTH                               4           // 
#define         SENSOR_NODE_DATA_LENGTH_MAX                         19          // Sensor数据结点数据最大长度 
#define         DAY_COUNT_OF_WEEK                                   7           // 
// 
#define         ONCE_RULE_INVALID_ING                               0x01        // 临时规则有效性标志 

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

//排插端口开关状态结构体
typedef struct PortStatus
{
    char            name[NAME_LENGTH];              //the port name
    uint8_t         OldStatus;
    uint8_t         CurStatus; 
    uint8_t         OnOffStatus;
    uint8_t         msType;                     //the type of port:master or salve
    uint8_t			rebootPort;					//the type of port:reboot or not
}PortStatusType;


//smart_link配置信息结构体
typedef struct DevSmartLinkInfoStruct
{
    int             isDevSmartLinkSaveFlg;                  //is save or not flag        
    char            ssid[SSID_LENGTH];                      //ssid
    char            password[KEY_LENGTH];                  //the type of port:master or salve
    uint8_t         security;                              //encry
}DevSmartLinkInfoType;



typedef struct SwitchOnOffStatus
{ 
	uint8_t         LimitData;					//下发是limit标号
	uint8_t         masterLimitWatt;
	uint8_t         isRebootEnable;				//Reboot配置是否生效
	uint8_t			portChangeReason;			//port 变化原因
    int             isDevStatusSaveFlg;     
    PortStatusType  PortInfor[PORT_NUM+1];
}SwInformationType;


typedef struct SwitchRuleStruct 
{ 
	int8_t         RuleEnable;                             // 规则是否有效标志 
	
    uint8_t         SwOpDateObj;                            // 按位表示星期几 
	uint8_t         PortArr[PORT_NUM+1];                    // 对该规则具体执行的动作:0(关) 1(开) 2(无关) 
	// 
    uint8_t         OnceRuleFlag;                           // 循环规则是否起用过标志 
    //uint8_t         isValidTurnOnOff;                       // 0x01:TurnOn; 0x02:TurnOff: 0x03:TurnOnOff 
    uint32_t        timeStamp;                              // 单次规则的格林尼治时间戳 
    uint32_t        CurTimeStamp;                          // 单次规则的本地时间戳 
    uint32_t        timeLong;                               // 表示打开或关闭的时间距离当日0点的分钟数
    uint32_t        exeTimeLong;                               // 表示打开或关闭的时间距离当日0点的分钟数
    uint32_t         RuleID;                                 // 规则ID
}SwitchRuleType; 

typedef struct SecurityModeStruct 
{ 
	uint8_t          enable;                             // 放到模式是否有效标志 	
    uint8_t         SwOpDateObj;                            // 按位表示星期几 
    uint8_t         PortArr[PORT_NUM+1];                    // 对该规则具体执行的动作:0(关) 1(开) 2(无关) 
    uint16_t        startTime;                              //起始时间距离零点的分钟差
    uint16_t        endTime;                          		// 结束时间
    int             isSaveFlg;     						//防盗信心是否保存成功
}SecurityModeType; 


typedef struct SecurityModeExcuteStruct 
{
	uint8_t			timeOut;										//下一次防盗时间超时
	uint8_t 		nextIntervalTime;								// 执行下一次防盗模式距离现在的时间
	uint8_t 		RunningMark;									// 循环模式是否在执行期
	int16_t         intervalTime;  									// 防盗模式的间隔时间
}SecurityModeExcuteType;



typedef struct DevRuleStruct 
{
	uint8_t         RuleCount;                            // 规则实际数量 
    int             isRuleSaveFlag;                         // 
    SwitchRuleType  SwitchRulePool[SOCKET_RULE_COUNT+1];   // 
}DevRuleInfoType;

typedef struct CountDownTimeStruct 
{ 
	// 动作生效时间 
	uint8_t         isCountDown;                            // 是否有效
	uint8_t         onOrOff;                                // 具体执行的动作:On:1  Off:0
    // 倒计时原始数据 
	uint32_t        StartTimePoint;                         // 开始倒计时时间 
	uint32_t        ReceiveStartTime;                       // 接收到的开始倒计时时间
	uint32_t        CountDownValue;                         // 倒计时延时时间(单位:s) 
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


//排插端口开关状态结构体
typedef struct RelatedCtrlStatus
{
    uint8_t preStates;			//关联控制之前状态
    uint8_t ctrStates;			//关联控制状态
	uint8_t ctlMark;				//关联控制标志位
	uint8_t afterMark;					//延迟控制执行标志位
	int delayTime;						//关联控制延后时间
	int holdTime;						//关联控制延时恢复时间
	uint32_t ctlTime;					//关联控制时间计时
}RelatedCtrlType;


#endif 
/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/
