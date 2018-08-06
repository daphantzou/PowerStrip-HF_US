/*
********************************************************************************
* @file    UdpServerCallBack.h
* @author  
* @version V1.0.0
* @date    2013/06/23
* @brief   
********************************************************************************
*/
#ifndef   __UDP_CLIENT_H__
#define   __UDP_CLIENT_H__ 
// 
#include <hsf.h>

#include "DevDataType.h"
  
// 
#define         UDP_SERVER_PORT                                     5000        // 公网服务器所使用的UDP端口 
#define         UDP_CLIENT_PORT                                     8888        // 排插使用的UDP端口 
// 接收的控制命令  
#define         DEV_QUERY_BROADCAST_DOWN_00_CMD                     0x00        // UDP广播包命令 



#define         SWITCH_ON_OFF_CTL_UDP_14_CMD                        0x14        // 服务器控制插座开关命令
#define         SWITCH_SET_PORT_HOST_SLAVE_15_CMD                   0x15        // 服务器设置排插主从命令
#define         SWITCH_RENAME_UDP_18_CMD                            0x18        // 服务器修改设备名称命令
#define         SWITCH_RELATED_CONTROL_UDP_28_CMD                   0x1C        // 服务器转发关联控制

#define         SWITCH_SET_DEV_REBOOT_1A_CMD						0X1A		// 服务器下发reboot指令

#define         SWTICH_RESTART_SYN_UDP_1E_CMD                       0x1E        // 服务器要求终端重启、同步、恢复出厂设置命令


#define         SWITCH_UPDATE_FW_UDP_33_CMD                         0x33        // 服务器要求设备更新固件命令

#define 		SWITCH_SET_DEVICE_WIFI_UDP_71_CMD					0x47        // 设置设备wifi模式
#define 		SWITCH_GET_WIFI_SIGNAL_UDP_72_CMD					0x48        // 搜索周围的wifi信号
#define 		SWITCH_GET_DEVICE_REGID_UDP_73_CMD					0x49        // 获取revogiID 
#define 		SWITCH_SET_DEVICE_MAC_UDP_74_CMD					0x4A        // 修改SN
#define 		SWITCH_CALIBRATE_POWER_CONSUME_UDP_75_CMD			0x4B        // 设置功率芯片校准值
#define 		SWITCH_GET_POWER_CONSUME_UDP_90_CMD					0x5A		// 获取设备实时功耗




// UDP升级固件 
#define         SWTICH_UPDATE_FW_UDP_81_CMD                         0x81        // 更新固件数据包 





#define         DEBUG_NET_UDP_CMD                                   0xAA        // UDP调试uart口中转指令 
#define         DEBUG_UART_UDP_CMD                                  0xCC        // UDP调试uart口中转指令 
#define         DEBUG_POWER_UDP_0XDD_CMD                            0xDD        // 
#define         DEBUG_POWER_UDP_0XCD_CMD                            0xCD        // 




typedef struct RuleACKStruct
{  
    uint8_t 	isReportSuccess;
    char        AckMsgArray[200];
}RuleACKType;



extern uint8_t     isDevReg;                                                        //是否注册成功，及账号绑定是否成功
extern struct sockaddr_in		PhoneSockAddr;											//手机端IP地址

extern int USER_FUNC UdpServer_RecvData(uint32_t event, char *pRecvData, uint32_t len,uint32_t buf_len);

extern void     SendUdpData(char *pData2Send, int ArgLength);                   // 发送UDP数据包 

extern void     SendDataToServer(int fd, struct sockaddr* TargetSockAddr, char *pArgBuf, int ArgLength);
extern uint8_t UdpSetHostSlave(char * pDataBuf);
extern void masterSlaveControl(void);

// 
#endif 

/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/

