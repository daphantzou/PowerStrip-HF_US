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
#define         UDP_SERVER_PORT                                     5000        // ������������ʹ�õ�UDP�˿� 
#define         UDP_CLIENT_PORT                                     8888        // �Ų�ʹ�õ�UDP�˿� 
// ���յĿ�������  
#define         DEV_QUERY_BROADCAST_DOWN_00_CMD                     0x00        // UDP�㲥������ 



#define         SWITCH_ON_OFF_CTL_UDP_14_CMD                        0x14        // ���������Ʋ�����������
#define         SWITCH_SET_PORT_HOST_SLAVE_15_CMD                   0x15        // �����������Ų���������
#define         SWITCH_RENAME_UDP_18_CMD                            0x18        // �������޸��豸��������
#define         SWITCH_RELATED_CONTROL_UDP_28_CMD                   0x1C        // ������ת����������

#define         SWITCH_SET_DEV_REBOOT_1A_CMD						0X1A		// �������·�rebootָ��

#define         SWTICH_RESTART_SYN_UDP_1E_CMD                       0x1E        // ������Ҫ���ն�������ͬ�����ָ�������������


#define         SWITCH_UPDATE_FW_UDP_33_CMD                         0x33        // ������Ҫ���豸���¹̼�����

#define 		SWITCH_SET_DEVICE_WIFI_UDP_71_CMD					0x47        // �����豸wifiģʽ
#define 		SWITCH_GET_WIFI_SIGNAL_UDP_72_CMD					0x48        // ������Χ��wifi�ź�
#define 		SWITCH_GET_DEVICE_REGID_UDP_73_CMD					0x49        // ��ȡrevogiID 
#define 		SWITCH_SET_DEVICE_MAC_UDP_74_CMD					0x4A        // �޸�SN
#define 		SWITCH_CALIBRATE_POWER_CONSUME_UDP_75_CMD			0x4B        // ���ù���оƬУ׼ֵ
#define 		SWITCH_GET_POWER_CONSUME_UDP_90_CMD					0x5A		// ��ȡ�豸ʵʱ����




// UDP�����̼� 
#define         SWTICH_UPDATE_FW_UDP_81_CMD                         0x81        // ���¹̼����ݰ� 





#define         DEBUG_NET_UDP_CMD                                   0xAA        // UDP����uart����תָ�� 
#define         DEBUG_UART_UDP_CMD                                  0xCC        // UDP����uart����תָ�� 
#define         DEBUG_POWER_UDP_0XDD_CMD                            0xDD        // 
#define         DEBUG_POWER_UDP_0XCD_CMD                            0xCD        // 




typedef struct RuleACKStruct
{  
    uint8_t 	isReportSuccess;
    char        AckMsgArray[200];
}RuleACKType;



extern uint8_t     isDevReg;                                                        //�Ƿ�ע��ɹ������˺Ű��Ƿ�ɹ�
extern struct sockaddr_in		PhoneSockAddr;											//�ֻ���IP��ַ

extern int USER_FUNC UdpServer_RecvData(uint32_t event, char *pRecvData, uint32_t len,uint32_t buf_len);

extern void     SendUdpData(char *pData2Send, int ArgLength);                   // ����UDP���ݰ� 

extern void     SendDataToServer(int fd, struct sockaddr* TargetSockAddr, char *pArgBuf, int ArgLength);
extern uint8_t UdpSetHostSlave(char * pDataBuf);
extern void masterSlaveControl(void);

// 
#endif 

/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/

