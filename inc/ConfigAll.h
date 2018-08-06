/*********************************************************************************
* @file    : ConfigAll.h 
* @author  : SteveHuang 
* @version : V1.0.0 
* @date    : 2013-4-8 18:03:29 
* @brief   : 
* @Modify  : 
*********************************************************************************/ 
#ifndef __CONFIG_ALL_H__ 
#define __CONFIG_ALL_H__ 
//////////////////////// 

#include "hsf.h" 

/********************************************
* 通用配置宏 
********************************************/ 
// 当前固件版本   
//#define         DEFAULT_BIG_VER             4                                 // 当前固件版本大版 
//#define         DEFAULT_LITTLE_VER          11						// 当前固件版本小版 
#define         CURRENT_VER_NUMBER          (((big_version<<8)&0xFF00) | (little_version&0xff)) 
// 
/************************************************************************** 
* 调试使能宏定义 
***************************************************************************/ 
//#define                     __KEY_PRINTF_DBG__                                //  

/********************************************************************** 
* 网络打印调试信息宏使能宏定义 
*********************************************************************/ 
// 
//#define                     __SERVER_UPDATE_FIRMWARE_DBG__                    // 上报新服务器调试信息 

#define                     __UDP_PRINT_DBG__                                   //通过网络口打印调试信息 
//#define                     __UART_PRINT_DBG__                                //通过网络口打印串口接收信息

//#define                     __TIMELY_STATE_UDP_PRINT_DBG__                    // 

//#define                     __SUB_DOMAIN_UDP_PRINT_DBG__                      // 从服务器调试信息 

//#define                     __PUBLIC_SERVER_SYNC_UDP_PRINT_DBG__              // 

//#define                     __URL_UPDATE_DBG__                                // Http 设置规则 

//#define                     __RN8209_SPI_DBG__                                // 

//#define                     __HTTP_SET_DBG__                                  // 

//#define                     __USER_BIN_FILE_DBG__                             // 

//#define                     __UDP_SET_TIME_DBG__                              // 设置时间调试信息 

//#define                     __DEV_RULE_DBG__                                  // 规则调试信息 

//#define                     __REPORT_2_SERVER_UDP_PRINT_DBG__                 // 网络打印上报服务器 

//#define                     __KEY_DRV_UDP_PRINT_DBG__                         // 按键调试信息 

#define                     __DOWN_SHUT_DBG__                                 // 系统宕机原因调试 

//#define                     __DS1302_RUN_STATUS_DBG__                         // DS1302运行状态调试信息 

//#define                     __CARIBRATION_POWER_COEFFICIENT_DBG__             // 功率校准 

//#define                     __SN_SET_DBG__                                    // 

//#define                     __AP_PWR_DBG__                                    // 

//#define                     __PWR_OFFSET_DBG__                                // 功率偏移校准 

//#define                     __PWR_STATICS_DBG__                               // 能耗统计调试 

//#define                     __PWR_HANG_DBG__                                    // 

//#define                     __WIFI_SCAN_DBG__                               

//#define                     __KEY_ON_OFF_DBG__                               

//#define                     __UDP_NET_STATE_PRINT_DBG__                       //  SmrtLink连接调试 
// ------------------------------ 
//#define                     __SMLK_FREUENCY_TEST_DBG__                        // 定频测试，去掉SmartLink版本 

#define                         __FRANCE_OPR__                                  // 

#define                         __BSD_SOCKET_UDP_SERVER_ENABLE__                // 自建socket运行, 

#define                         __BSD_SOCKET_HTTP_SERVER_ENABLE__               // 自建Socket运行Httpd服务器 

#define                         __DEL_BELOW_05_W__                              // 滤除0.5w以下的功率 

// Hank Joe客户低于1.32版本，无法远程控制Bug修复特别版本，控制宏 
//#define                       HANK_JOE_CUSTOMER_REMOTE_CTL                    // 

// 
#define         VENDOR_REVOGI               0x01 
#define         VENDOR_MH                   0x02 
#define         VENDOR_SHOOTING             0x03 
#define         VENDOR_ELV                  0x04
#define         VENDOR_SATECHI              0x05 
#define         VENDOR_BLING                0x06  
#define         VENDOR_MCL                  0x07 
#define         VENDOR_EXTEL                0x08 
#define         VENDOR_KORINS               0x09
#define         VENDOR_ZEON               	0x0A
#define         VENDOR_COCOSO               0x0B


#define         VENDOR_CODE                 VENDOR_REVOGI                         // 

//#define			REGIDID						"rvg"

/********************************************************************************* 
**************************** 公网服务器地址宏 
** 从服务器 "server.revogi.cn" "aws.revogi.com"
*********************************************************************************/ 
#define         MAIN_SERVER_UDP_PORT        		8088                                // 
//#define         MAIN_SERVER_UDP_PORT        		8080          

#define         MAIN_SERVER_UDP_PORT_BACKUP         5000  

#define         PORT_NUM                     6		//the max switch port number
#define         ORDER_NUM                    6         //the order number of STM8s 


#define			ZX_DETECT													// 使能过零检测

#define         AT_ACK_MSG_LENGTH           72 

#define         __SCAN_KEY_IN_TICK_TIMER__                                      // 按键扫描于时钟 

#define         __DS1302_RTC_ENABLE__                                           // 是否使能DS1302 RTC 

#define         __WATCH_DOG_ENABLE__                                            // 使能软件看门够狗 

#define         __POWER_STATICS_ENABLE__                                        // 使能功率统计功能 

#define         __PROTECT_MOD_ENABLE__                                          // 使能过流保护 

#define			DEBUG					0

/********************************************
* 配置宏 
********************************************/ 
// Private define 
//#define         __HF_BOARD__              // HF开发板使能 

// 
///////////////////////////////////////////// 
#endif //__CONFIG_ALL_H__ 
/******************************************************************************* 
* ========================= END OF PAGE 
*******************************************************************************/ 

