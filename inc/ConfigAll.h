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
* ͨ�����ú� 
********************************************/ 
// ��ǰ�̼��汾   
//#define         DEFAULT_BIG_VER             4                                 // ��ǰ�̼��汾��� 
//#define         DEFAULT_LITTLE_VER          11						// ��ǰ�̼��汾С�� 
#define         CURRENT_VER_NUMBER          (((big_version<<8)&0xFF00) | (little_version&0xff)) 
// 
/************************************************************************** 
* ����ʹ�ܺ궨�� 
***************************************************************************/ 
//#define                     __KEY_PRINTF_DBG__                                //  

/********************************************************************** 
* �����ӡ������Ϣ��ʹ�ܺ궨�� 
*********************************************************************/ 
// 
//#define                     __SERVER_UPDATE_FIRMWARE_DBG__                    // �ϱ��·�����������Ϣ 

#define                     __UDP_PRINT_DBG__                                   //ͨ������ڴ�ӡ������Ϣ 
//#define                     __UART_PRINT_DBG__                                //ͨ������ڴ�ӡ���ڽ�����Ϣ

//#define                     __TIMELY_STATE_UDP_PRINT_DBG__                    // 

//#define                     __SUB_DOMAIN_UDP_PRINT_DBG__                      // �ӷ�����������Ϣ 

//#define                     __PUBLIC_SERVER_SYNC_UDP_PRINT_DBG__              // 

//#define                     __URL_UPDATE_DBG__                                // Http ���ù��� 

//#define                     __RN8209_SPI_DBG__                                // 

//#define                     __HTTP_SET_DBG__                                  // 

//#define                     __USER_BIN_FILE_DBG__                             // 

//#define                     __UDP_SET_TIME_DBG__                              // ����ʱ�������Ϣ 

//#define                     __DEV_RULE_DBG__                                  // ���������Ϣ 

//#define                     __REPORT_2_SERVER_UDP_PRINT_DBG__                 // �����ӡ�ϱ������� 

//#define                     __KEY_DRV_UDP_PRINT_DBG__                         // ����������Ϣ 

#define                     __DOWN_SHUT_DBG__                                 // ϵͳ崻�ԭ����� 

//#define                     __DS1302_RUN_STATUS_DBG__                         // DS1302����״̬������Ϣ 

//#define                     __CARIBRATION_POWER_COEFFICIENT_DBG__             // ����У׼ 

//#define                     __SN_SET_DBG__                                    // 

//#define                     __AP_PWR_DBG__                                    // 

//#define                     __PWR_OFFSET_DBG__                                // ����ƫ��У׼ 

//#define                     __PWR_STATICS_DBG__                               // �ܺ�ͳ�Ƶ��� 

//#define                     __PWR_HANG_DBG__                                    // 

//#define                     __WIFI_SCAN_DBG__                               

//#define                     __KEY_ON_OFF_DBG__                               

//#define                     __UDP_NET_STATE_PRINT_DBG__                       //  SmrtLink���ӵ��� 
// ------------------------------ 
//#define                     __SMLK_FREUENCY_TEST_DBG__                        // ��Ƶ���ԣ�ȥ��SmartLink�汾 

#define                         __FRANCE_OPR__                                  // 

#define                         __BSD_SOCKET_UDP_SERVER_ENABLE__                // �Խ�socket����, 

#define                         __BSD_SOCKET_HTTP_SERVER_ENABLE__               // �Խ�Socket����Httpd������ 

#define                         __DEL_BELOW_05_W__                              // �˳�0.5w���µĹ��� 

// Hank Joe�ͻ�����1.32�汾���޷�Զ�̿���Bug�޸��ر�汾�����ƺ� 
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
**************************** ������������ַ�� 
** �ӷ����� "server.revogi.cn" "aws.revogi.com"
*********************************************************************************/ 
#define         MAIN_SERVER_UDP_PORT        		8088                                // 
//#define         MAIN_SERVER_UDP_PORT        		8080          

#define         MAIN_SERVER_UDP_PORT_BACKUP         5000  

#define         PORT_NUM                     6		//the max switch port number
#define         ORDER_NUM                    6         //the order number of STM8s 


#define			ZX_DETECT													// ʹ�ܹ�����

#define         AT_ACK_MSG_LENGTH           72 

#define         __SCAN_KEY_IN_TICK_TIMER__                                      // ����ɨ����ʱ�� 

#define         __DS1302_RTC_ENABLE__                                           // �Ƿ�ʹ��DS1302 RTC 

#define         __WATCH_DOG_ENABLE__                                            // ʹ��������Ź��� 

#define         __POWER_STATICS_ENABLE__                                        // ʹ�ܹ���ͳ�ƹ��� 

#define         __PROTECT_MOD_ENABLE__                                          // ʹ�ܹ������� 

#define			DEBUG					0

/********************************************
* ���ú� 
********************************************/ 
// Private define 
//#define         __HF_BOARD__              // HF������ʹ�� 

// 
///////////////////////////////////////////// 
#endif //__CONFIG_ALL_H__ 
/******************************************************************************* 
* ========================= END OF PAGE 
*******************************************************************************/ 

