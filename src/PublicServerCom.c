/*********************************************************************************
* @file    PublicServerCom.c
* @author  
* @version V1.0.0
* @date    03/20/2013
* @brief   
********************************************************************************/
#include <stdint.h> 
#include <stdio.h> 
#include "cJSON.h"
#include "ConfigAll.h" 
#include "flash.h" 
#include "DevDataType.h" 
#include "DevLib.h" 
#include "DevInfo.h" 
#include "DevRule.h" 
#include "rn8209.h"
#include "RtTime.h"
#include "HttpdCallBack.h"
#include "PowerStaticsRecord.h" 
#include "PublicServerCom.h" 
#include "UdpServerCallBack.h" 
#include "SwitchApp.h" 
#include "Produce_Test.h"
#include "read_power.h"
#include "security_mode.h"




#define NOT_ONLINE_COUNT        5

#define PUBLIC_SERVER_INIT_STATUS   (1<<TIMELY_CKECK_BIT_12_IS_ALL_DATA_CMD)    | \
                                    (1<<TIMELY_CKECK_BIT_8_IS_SYNC_RT_DATA_CMD)

uint8_t reportRTHourmark = 0;                                   //�ϱ�Сʱʵʱ���ı�־
uint8_t hasSynInfo = 0;             //ͬ�������Ƿ�ɹ�
uint16_t heartBeatCount = 0;         //��������������
uint8_t pageID = 0;					//���͹���ҳ���
uint8_t resetWifiMark = 0;				//����·������־



extern DevInfoType          DevInfoObj;
extern FwUpdateStatusType   FwUpdateStatusObj; 
extern SoftwareVerType      CurFwVersionObj;
extern DevTimeType          DevTimeObj;
extern char                 AppTaskSendBuf[1400];                           //
extern uint8_t              iStaticsFlag;                        //��־λ��ϱ���
extern DevRegInfoType       DevRegInformation;                                      //�豸ע����Ϣ
extern uint8_t              isDevReg;                            // �Ƿ�ע��ɹ���־ 
extern uint8_t 				isGetRegid;             //�Ƿ��ȡ�����豸�˺�
                                    
/*******************************************************************************
* ��������: ����ʱ����ꡢ�¡��ա�����ͬ���ı�־λ��һ
* ��ڲ���: 
* ���ڲ���: 
* ʹ��˵��: 
*******************************************************************************/ 
void PublicServer_Init(void)
{
    OrBit(FwUpdateStatusObj.TimelyCheck, (PUBLIC_SERVER_INIT_STATUS));
}

/*******************************************************************************
* ��������: ͬ���ꡢ�¡��յĹ����������ݼ���������״̬��Ϣ
* ��ڲ���: 
* ���ڲ���: 
* ʹ��˵��: 
*******************************************************************************/ 
void PublicServer_SyncAllWithServer(void)
{
    PublicServer_SyncBaseInfoWithServer();
    PublicServer_SyncCostWithServer();
}

/*******************************************************************************
* ��������: �������ͬ������״̬��Ϣ
* ��ڲ���: 
* ���ڲ���: 
* ʹ��˵��: 
*******************************************************************************/ 
void PublicServer_SyncBaseInfoWithServer(void)
{
    SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_12_IS_ALL_DATA_CMD);
}

/*******************************************************************************
* ��������: ͬ���ꡢ�¡��յĹ�����������
* ��ڲ���: 
* ���ڲ���: 
* ʹ��˵��: ����ʱ��������չ������ĵı�־λ��һ
*******************************************************************************/ 
void PublicServer_SyncCostWithServer(void)
{
    SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_9_IS_HOURS_DATA_CMD);
}


/*******************************************************************************
* ��������: �ϱ��㲥����Ϣ 
* ��ڲ���: 
* ���ڲ���: 
* ʹ��˵��: ÿ��/��
*******************************************************************************/ 
void sendBroadcastinfo(void)
{
	char      pAck[256] = {0}; 
    int       iLength;
	
	if(isBitSet(FwUpdateStatusObj.TimelyCheck, TIMELY_SEND_BIT_23_BROADCAST_INFO_CMD))
	{
		ClrBit(FwUpdateStatusObj.TimelyCheck, TIMELY_SEND_BIT_23_BROADCAST_INFO_CMD);
		
		iLength = sprintf(pAck, "{\"response\":%X,\"data\":{"
                                    "\"sn\":\"%s\","
                                    "\"name\":\"%s\","
                                    "\"mac\":\"%02X:%02X:%02X:%02X:%02X:%02X\","
                                    "\"sak\":\"%s\","
                                    "\"regid\":\"%s\","
                                    "\"isGetRegid\":%d,"
                                    "\"isMainSvrOK\":%d,"
                                    "\"MainSveIP\":\"%08X\","
                                    "\"MainSvrPort\":\"%x\","
                                    "\"time\":%d,"
                                    "\"ver\":\"%d.%02d\"}}",
                                    DEV_QUERY_BROADCAST_DOWN_00_CMD, 
                                    DevInfoObj.dev_sn, 
                                    DevInfoObj.dev_name,
                                    DevInfoObj.Dev_Mac[0], DevInfoObj.Dev_Mac[1],
                                    DevInfoObj.Dev_Mac[2], DevInfoObj.Dev_Mac[3],
                                    DevInfoObj.Dev_Mac[4], DevInfoObj.Dev_Mac[5],
                                    DevInfoObj.dev_sak,
                                    DevRegInformation.regidName,            				 //�ֻ����͵��û��˺���,
                                    isGetRegid,FwUpdateStatusObj.isMainSvrOK,
	                      			MainSvrSockAddr.sin_addr.s_addr,
	                      			MainSvrSockAddr.sin_port,
	                      			DevTimeObj.NowTimeSec,
                                    CurFwVersionObj.big_ver, 
                                    CurFwVersionObj.little_ver
                                    );  
        if(-1 == iLength)
        { 
            return;
        } 
        *(pAck + iLength) = '\0'; 

		UdpDebugPrint(pAck, iLength);
	}
}






/**************************************************************************************************
* ��������: ����豸Reboot����
* ��ڲ���: 
* ���ڲ���: 
* ʹ��˵��: ÿ����һ�μ���豸�Ƿ����������䣬������--������������Reboot�˿�
****************************************************************************************************/ 
void Reboot_Control(void)
{
	static uint8_t CurState=0,LastState=0;
	
	if(isBitSet(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_18_REBOOT_CONTROL_CMD))
	{ 				
		ClrBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_18_REBOOT_CONTROL_CMD);
				
		if(SwInformation.isRebootEnable)			//�������øù���
		{
			if(isBitSet(iStaticsFlag, WIFI_WORK_NORMAL))
			{
				CurState = 1;
				UdpDebugPrint("The wifi is OK!!!!!!!!\r\n",strlen("The wifi is OK!!!!!!!!\r\n")-1); 
			}
			else
			{
				CurState = 0;
				UdpDebugPrint("The wifi can't connect!!!!!!!!\r\n",strlen("The wifi can't connect!!!!!!!!\r\n")-1); 
			}

			if(LastState != CurState)
			{			
				if(CurState == 0)
				{
			     #if(PORT_NUM == 6)
				   uint8_t i = 0,j = 0;
				   
				   for(j=0;j<PORT_NUM;j++)
				   {
				 		if(SwInformation.PortInfor[j].rebootPort==1)
						{
							 //Switch_TurnOnOff(j+1, 0);
							 SetPortState(j+1, 0);
						}
				   }

				   
				   for(i=0;i<PORT_NUM;i++)
				   {
					   if(SwInformation.PortInfor[i].OnOffStatus)
					   {
						   SwOnOffData |= SwInformation.PortInfor[i].OnOffStatus<<i;
					   }
					   else
					   {
						   SwOnOffData &= ~((!SwInformation.PortInfor[i].OnOffStatus)<<i);
					   }
				   }
				   
				   msleep(200);
				   UsartControlPortOnOff(); 
				   
				   msleep(8000);
				   for(j=0;j<PORT_NUM;j++)
				   {
						if(SwInformation.PortInfor[j].rebootPort==1)
						{
							 SetPortState(j+1,1);	 
						}
				   }

				   
				   for(i=0;i<PORT_NUM;i++)
				   {
					   if(SwInformation.PortInfor[i].OnOffStatus)
					   {
						   SwOnOffData |= SwInformation.PortInfor[i].OnOffStatus<<i;
					   }
					   else
					   {
						   SwOnOffData &= ~((!SwInformation.PortInfor[i].OnOffStatus)<<i);
					   }
				   }
				   
				   msleep(200);
				   UsartControlPortOnOff(); 

				 #elif(PORT_NUM == 1)
			   		if(SwInformation.PortInfor[0].rebootPort == 1)
			   		{
		    			Port_TurnOnOff(0);
						
				   		msleep(5000);
						Port_TurnOnOff(1);

						//u_printf("publicServerCom.c--353\n");
			   		}
		   		  #endif

				  	SwInformation.portChangeReason = PORT_CHANGE_BY_REBOOT;

				   UdpDebugPrint("Now excute the reboot port control\r\n",strlen("Now excute the reboot port control\r\n")-1); 
				   //u_printf("Now excute the reboot port control\r\n");
				}
				LastState = CurState;
			}
		}
	}
}





/******************************************************************************* 
* ========================= END OF PAGE 
*******************************************************************************/ 

