/* 
******************************************************************************** 
* @file     : DevInfo.h 
* @author   : 
* @version  : V1.0.0 
* @date     : 2014/06/28 
* @brief    : 
* -备注-    : 
******************************************************************************** 
*/ 
#include <string.h> 
#include <stdint.h> 
#include <stdlib.h> 
#include <stdio.h> 

#include "hsf.h"
// 
#ifndef   __DEV_INFO_H__ 
#define   __DEV_INFO_H__  
// 
#define         IS_SVR_OK                               0x55 
#define         IS_SVR_FAILE                            0x00 

#define         SERIAL_NUN_ID_LEN                       16                          // 产品序列号长度 
#define         SAK_LEN                                 12                          // SAK 长度 
#define         MAC_LEN                                 6                           // Mac 地址数据长度(Byte)
#define         MAC_STR_LEN                             17 
#define 		DOMAIN_MAX_LENGTH						32							//域名最大长度

// TimelyCheck Bit define 
#define         TIMELY_CKECK_BIT_0_IS_REG               0 
#define         TIMELY_CKECK_BIT_1_IS_MAIN_DOMAIN       1 

#define         TIMELY_CKECK_BIT_2_IS_GOT_SUB_DOMAIN    2 
#define         TIMELY_CKECK_BIT_3_IS_SUB_DOMAIN        3 

#define         TIMELY_CKECK_BIT_4_IS_GET_SOCKETA_FD    4 
#define         TIMELY_CKECK_BIT_5_IS_URL_UPDATE_FW     5 

#define         TIMELY_CKECK_BIT_6_IS_REBOOT_CMD        6 

#define         TIMELY_CKECK_BIT_7_IS_REPORT_RULE_CMD   7 

#define         TIMELY_CKECK_BIT_8_IS_SYNC_RT_DATA_CMD  8 
#define         TIMELY_CKECK_BIT_9_IS_HOURS_DATA_CMD    9 
#define         TIMELY_CKECK_BIT_10_ADD_REPORT_HOURS_DATA_CMD    10 
#define         TIMELY_EXCUTE_BIT_11_DEV_INIT           11

#define         TIMELY_CKECK_BIT_12_IS_ALL_DATA_CMD     12 

#define         TIMELY_CKECK_BIT_13_RULE_IS_DIRTY_CMD   13 

#define         TIMELY_CKECK_BIT_14_SW_STATE_IS_DIRTY_CMD   	14 
#define         TIMELY_CKECK_BIT_15_REPORT_COUNTDOWN_CMD   		15 
#define         TIMELY_CKECK_BIT_16_REPORT_MASTER_SLAVE_CMD   	16
#define         TIMELY_CKECK_BIT_17_REPORT_PORT_REBOOT_CMD   	17
#define         TIMELY_CKECK_BIT_18_REBOOT_CONTROL_CMD   		18

#define         TIMELY_DETECTE_BIT_19_DEV_SECURITY_MODE_CMD   	19
#define			TIMELY_REPORT_BIT_20_SECURITY_MODE_CMD			20

#define			TIMELY_SEND_BIT_21_ADD_HOUR_DATA_CMD			21
#define			TIMELY_SEND_BIT_22_HOUR_DATA_CMD				22

#define			TIMELY_SEND_BIT_23_BROADCAST_INFO_CMD			23




#define         TIMELY_CKECK_BIT_30_IS_SEC_TICK         30 

// 
#define         NAME_LENGTH                           15 
#define         SSID_LENGTH                           32
#define         KEY_LENGTH                            16
#define         IP_LENGTH                             4


// 
typedef struct SoftwareVer
{
    uint32_t        big_ver; 
    uint32_t        little_ver; 
}SoftwareVerType; 

typedef struct DevInfo 
{ 
    uint8_t             Dev_Mac[MAC_LEN+1];                                 // 设备Mac地址 
    uint8_t             dev_sn[SERIAL_NUN_ID_LEN+1];                        // 
    uint8_t             dev_sak[SAK_LEN+1];                                 // 
    char                dev_name[NAME_LENGTH+1];                            // 设备名称 
    uint8_t             dev_ip[IP_LENGTH+1];                            // 设备名称 
    uint8_t             TimeZone;                                           // 本地时区 
    
    hfthread_mutex_t    DevNameMutex;                                       // 设备名称信号量 
    uint8_t             isCalibrateTime;                                    // 设备是否对时标志

   #ifdef  _STATIC_IP_ 
    uint8_t             stastic_ip;                                         // 
    uint32_t            ipaddr;                                             // 
    uint32_t            netmask;                                            // 
    uint32_t            gw;                                                 // 
    uint32_t            dns;                                                // 
   #endif 
}DevInfoType;

#define         SLAVE_SERVER_URL_MAX_LENGTH         32
#define         UPDATE_URL_MAX_LENGTH               255
typedef struct FwUpdateStatusStruct 
{
    uint32_t            TimelyCheck;                                        // 标志位图 
    
    
    uint8_t             isSubSvrOK;                                         // 子服务器Dns地址解析与否标志 
    uint8_t             isMainSvrOK;                                        // 是否存在公网

//    char                regidID[NAME_LENGTH+1];                               // 绑定账号名
//    char                SubSvrUrl[SLAVE_SERVER_URL_MAX_LENGTH+1];           // 从服务器Url 
    
//    uint8_t             isDevReg;                                           // 是否在公网注册 
    
    uint8_t             RebootCountDown;                                    // 重启计时 
    uint8_t             SyncRebootCode;                                     // 
    
    int                 UdpFd;                                              // UDP接口Fd 
//    struct sockaddr_in  MainSvrSockAddr;                                    // 主服务器sockaddr 
    struct sockaddr_in  SlaveSvrSockAddr;                                   // 从服务器sockaddr 
    
    SoftwareVerType     PublicServerVer;                                    // 
}FwUpdateStatusType; 

//设备注册保存信息
typedef struct DevRegInfoStruct
{
    char    regidName[NAME_LENGTH];                 //账号名
    //uint8_t isDevReg;                               // 是否在公网注册 
    int     iSockPort;                              //从服务器端口号
    //struct sockaddr_in  MainSvrSockAddr;                                    // 主服务器sockaddr 
    //struct sockaddr_in  SlaveSvrSockAddr;                                   // 从服务器sockaddr 
    char    SubSvrUrl[SLAVE_SERVER_URL_MAX_LENGTH+1];           // 从服务器Url 
}DevRegInfoType;










#define         WIFI_LED_SHOW_0_BIT                 0 
#define         WIFI_LED_SHOW_1_BIT                 1 
typedef struct DevStatusStruct 
{ 
    uint8_t             LedShowStatus; 
}DevStatusType; 

typedef struct fw_update_info 
{ 
    hfthread_mutex_t    FwUpdateMutx;                                       // 固件升级信号量 
    SoftwareVerType     PublicServerVer;                                    // 
    // Http Client升级Firmware 
	uint16_t            down_port; 
	uint32_t            data_checksum; 
	char                UpdateUrl[UPDATE_URL_MAX_LENGTH+1]; 
	// UDP升级Firmware 
    int                 intFwLength; 
    uint16_t            FirmwareCompleted; 
    int16_t             CheckSumWhole; 
}FwUpdateArUrlType; 
// Udp升级Firmware数据包相关结构 
typedef struct UdpUpdateFileStruct 
{
    uint8_t        NumPackage[4];
    char        PackageLength[2];
}UdpUpdatePacketHeadType;
typedef struct LastPackCheckStruct 
{
    int         UpdateLength;
    int16_t     TotChkSum;
}LastPackCheckType;
// 
typedef struct SysTimeStruct 
{ 
    uint32_t            SysTimeCount;           // 
    uint32_t            SysUnixTime;            // 
    hfthread_mutex_t    SysUnixTimeMutx;        // 
}SysTimeType; 
// 
#endif 
/******************************************************************************* 
* ========================= END OF PAGE 
*******************************************************************************/ 

