/*
********************************************************************************
* @file    : SwitchApp.c 
* @author  : 
* @version : V1.0.0 
* @date    : 2014/06/27 
* @brief   : 
********************************************************************************
*/
#include <stdio.h> 
#include <string.h> 

#include "hsf.h" 
#include "ConfigAll.h" 
#include "DevDataType.h" 
#include "DevLib.h"
#include "DevInfo.h" 
#include "DevRule.h" 
#include "PublicServerCom.h"
#include "HttpdCallBack.h" 
#include "UdpServerCallBack.h" 
#include "SwitchApp.h" 
#include "TickClock.h"
#include "RtTime.h"
#include "remote_update.h"
#include "KeyDrv.h"
#include "Flash.h"
#include "rn8209.h"
#include "PowerStaticsRecord.h"
#include "UdpSocket.h"
#include "read_power.h"
#include "security_mode.h"



// 
// �ڲ����� 
static void App_Init(void); 

static void ReadDevMac(void); 
void ReadDevIP(void);


static void RestartPoll(void);                                          // ������ 
static void UpdateFirmwareAsHttpClient(void); 
static void UpdateFirmwareAsUdp(void); 
// 
uint8_t	switchMark = 0;					//�л����������˿ڱ�־

uint8_t 							recycleResolveDomain = 0;					//��ʱ�����ӷ�������־λ
uint32_t                            SystickTimeCount = 0; 
uint32_t                            CurSynHour = 25; 

uint8_t big_version = 3;
uint8_t little_version = 42;
uint8_t regStep = 0;			//�󶨽׶��Ա�־




#if(PORT_NUM == 6)
 #if (VENDOR_CODE == VENDOR_MCL) 
 	//#define         SWITCH_SERVER_NET           "server.mclsamar.com"               // �������������� 
 	//#define         DEV_DEFAULT_NAME            "PRISEWIFI"                         // ����Ĭ������ 
 	char switch_server_net[DOMAIN_MAX_LENGTH] = "server.mclsamar.com";               // �������������� 
 	char dev_default_name[NAME_LENGTH] = "PRISEWIFI";
 #elif (VENDOR_CODE == VENDOR_KORINS) 
	//#define         SWITCH_SERVER_NET           "en.mywatt.biz"               		// �������������� 
	//#define         DEV_DEFAULT_NAME            "KorinsPlug"                         // ����Ĭ������ 
	char switch_server_net[DOMAIN_MAX_LENGTH] = "en.mywatt.biz";               // �������������� 
 	char dev_default_name[NAME_LENGTH] = "KorinsPlug";
 #elif (VENDOR_CODE == VENDOR_ZEON) 
	//#define         SWITCH_SERVER_NET           "smarthome.zeon.sk"               		// �������������� 
	//#define         DEV_DEFAULT_NAME            "ZEONPLUG"                         // ����Ĭ������ 
	char switch_server_net[DOMAIN_MAX_LENGTH] = "smarthome.zeon.sk";               // �������������� 
 	char dev_default_name[NAME_LENGTH] = "ZEONPLUG";
 #elif (VENDOR_CODE == VENDOR_REVOGI) 
	//#define         SWITCH_SERVER_NET           "server.revogi.com"					// �������������� 
	//#define         DEV_DEFAULT_NAME            "RevogiStrip"                        // ����Ĭ������ 
	char switch_server_net[DOMAIN_MAX_LENGTH] = "server.revogi.com";               // �������������� 
 	char dev_default_name[NAME_LENGTH] = "RevogiStrip";
 #elif (VENDOR_CODE == VENDOR_COCOSO) 
	//#define         SWITCH_SERVER_NET           "server.revogi.com"					// �������������� 
	//#define         DEV_DEFAULT_NAME            "RevogiStrip"                        // ����Ĭ������ 
	char switch_server_net[DOMAIN_MAX_LENGTH] = "server.revogi.com";               // �������������� 
 	char dev_default_name[NAME_LENGTH] = "CocosoStrip";
 #elif (VENDOR_CODE == VENDOR_EXTEL) 
	//#define         SWITCH_SERVER_NET           "server.revogi.com"                 // ��������������
	//#define         DEV_DEFAULT_NAME            "SOKY"                              // ����Ĭ������ 
	char switch_server_net[DOMAIN_MAX_LENGTH] = "server.revogi.com";               // �������������� 
 	char dev_default_name[NAME_LENGTH] = "SOKY";
 #else
	#error  "The Vendor Is Not Support Now...!"
 #endif
#elif(PORT_NUM == 1) 
	char switch_server_net[DOMAIN_MAX_LENGTH] = "server.revogi.com";               // �������������� 
 	char dev_default_name[NAME_LENGTH] = "RevogiPlug";
#endif

// 
struct tm                           *CurTm_Time; 

extern uint8_t              iStaticsFlag;                        //��־λ��ϱ���

extern uint16_t				heartBeatCount;                                                  //��������������


extern uint8_t             isDevReg;                  //�Ƿ�ע��ɹ���־

// �ڲ��������� 
// �ڲ�ȫ�ֱ������� 
#if (PORT_NUM == 1)
 #if  defined(HANK_JOE_CUSTOMER_REMOTE_CTL) 

    #define DEV_DEFAULT_MAC             {0x50,0xE5,0x49,0x00,0x03,0xA4} 
    #define DEV_DEFAULT_SN              "SWW1015010000006" 
    
    #define DEV_DEFAULT_SAK             "4D476A764B6D" 

 #else
    #define DEV_DEFAULT_MAC             {0x58,0xE5,0x49,0x16,0x89,0x36} 
    #define DEV_DEFAULT_SN              "SWW1011010000000" 

    #define DEV_DEFAULT_SAK             "55AA55AA55AA" 
 #endif
#else
 	#define DEV_DEFAULT_MAC             {0x58,0xE5,0x49,0x16,0x89,0x36} 
    #define DEV_DEFAULT_SN              "SWW6010010000068" 

    #define DEV_DEFAULT_SAK             "55AA55AA55AA" 
#endif
// 
extern  DevTimeType                 DevTimeObj;
static  hftimer_handle_t            app_timer = NULL; 
static  hftimer_handle_t            ms_clock  = NULL; 
// 
DevInfoType DevInfoObj= 
{ 
    DEV_DEFAULT_MAC,                // AT+WSMAC / AT+WSMAC= 
    DEV_DEFAULT_SN,                 // AT+ASWD / AT+ASWD= 
    DEV_DEFAULT_SAK,                // should same as SN or part of SN or( AT+WEBU/AT+WEBU= )
    "RevogiPlug",
    8, 
    #if defined(__STATIC_IP__) 
    , 
    0 
    #endif 
};

DevRuleInfoType         DevRuleInfoObj; 
TurnOffTimeType         CountDownTurnOff[PORT_NUM+1]; 
FwUpdateStatusType      FwUpdateStatusObj; 
SoftwareVerType         CurFwVersionObj;  // ���ع̼��汾 
DevStatusType           DevStatusObj;                                           // 
FwUpdateArUrlType       FwUpdateObj;                                            // 
DevRegInfoType          DevRegInformation;                                      //�豸ע����Ϣ
struct sockaddr_in      MainSvrSockAddr;                                    // ��������sockaddr 

char                    AppTaskSendBuf[1400];                                   // 

/*******************************************************************************
* ��������: 
* ��ڲ���: 
* ���ڲ���: 
* ʹ��˵��: 
*******************************************************************************/ 
extern int     DevModeStatus; 
extern uint8_t devModeInit;            //�豸������ʼ���Ƿ�ɹ�
USER_FUNC static void app_thread_func(void* arg)
{ 
	int socketaFD;

	hftimer_start(app_timer); 
	
   #if  defined(__WATCH_DOG_ENABLE__)
	hfthread_enable_softwatchdog(NULL,60);          //watchdog overtime 1minute
   #endif 


	while(1)
	{ 
	   #if  defined(__WATCH_DOG_ENABLE__)
	    FeedWatchDog(); 
	   #endif 
    	if(isBitSet(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_4_IS_GET_SOCKETA_FD)) 
    	{ 
    	    ClrBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_4_IS_GET_SOCKETA_FD); 
    	    
    	   #if defined(__BSD_SOCKET_UDP_SERVER_ENABLE__) 
    	    socketaFD = UdpServerThread_GetSocketFd();
    	    #else 
    	    socketaFD = hfnet_socketa_fd(); 
    	   #endif 
    	    
    	    if(socketaFD>=0) 
    	    { 
                FwUpdateStatusObj.UdpFd = socketaFD;                    // ��ȡsocketa fd 
                
                break; 
    	    } 
    	} 
		hf_thread_delay(200);                                           // ms 
	} 


	while(1) 
	{ 
	   #if  defined(__WATCH_DOG_ENABLE__)
	    FeedWatchDog();                                                 // OK 
	   #endif 

		#if  !defined(__SCAN_KEY_IN_TICK_TIMER__) 
        if(0x00 != DevModeStatus)
        {
            Key_Scan()
            Key_Poll()
        }
		#else 
        #endif
        
        GetDevTime();
		sendBroadcastinfo();							//�����ϱ��㲥��Ϣ
        CountDownProcess(DevTimeObj.NowTimeSec);                    // OK 

        RestartPoll();                                                  // 
          
        PowerRecord_StaticsData();                                      // ͳ�ƣ����沢�ϱ�Сʱ��������
		
		Reboot_Control();
 
		UpdateFirmwareAsHttpClient();                                   // OK 
	
		SysTime_GetTime();   
		

		ShowWorkMode();                                                 // 
		
        UpdateFirmwareAsUdp();                                          // OK 
  
        SaveDevRulePoll(); 

        SaveDevStatusPoll();                                            // дflash������������ 
        
		hf_thread_delay(500);                                           // ms 
	
	} 
}




/*******************************************************************************
* ��������: �ն�������ͬ�����ָ���������
* ��ڲ���: 
* ���ڲ���: 
* ʹ��˵��: 
*******************************************************************************/ 
#define     RESTART_CODE        0       // ������
#define     RESET_CODE          2       //�ָ���������
#define     SYNC_DATA_CODE      1       //����ͬ�����ݵ�������
void RestartPoll(void)                                                   
{
    char    AtAckMsg[AT_ACK_MSG_LENGTH+1] = {0}; 
	
    if(isBitSet(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_6_IS_REBOOT_CMD)
        &&(0x00 == FwUpdateStatusObj.RebootCountDown))
    {
        ClrBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_6_IS_REBOOT_CMD);
        if(RESTART_CODE == FwUpdateStatusObj.SyncRebootCode)
        {
           //u_printf("Cmd Reset system!\r\n\n");
           
           hfat_send_cmd("AT+Z\r\n", sizeof("AT+Z\r\n")-1, AtAckMsg, 72); 
        }
        else if(SYNC_DATA_CODE == FwUpdateStatusObj.SyncRebootCode)
        { 
            if(IS_OFF == isDevReg)
            { 
                SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_0_IS_REG);
                //u_printf("Now we sysc_data\n");
            } 
            else 
            { 
		        PublicServer_SyncAllWithServer(); 
            } 
        } 
        else if(RESET_CODE == FwUpdateStatusObj.SyncRebootCode)
        { 
            FlashResetFactory();

            //hfat_send_cmd("AT+Z\r\n", sizeof("AT+Z\r\n")-1, AtAckMsg, 19);
        } 
    }
}
/*******************************************************************************
* ��������: ִ�����UDP�����̼����� 
* ��ڲ���: 
* ���ڲ���: 
* ʹ��˵��: 
*******************************************************************************/ 
static void UpdateFirmwareAsUdp(void)
{ 
	//char testBuf[100] = {0};	
    if(0x02 == FwUpdateObj.FirmwareCompleted)
    {     
		//sprintf(testBuf,"intFwLength=%d\n",FwUpdateObj.intFwLength);
		//UdpDebugPrint(testBuf,strlen(testBuf));
		
        //if(hfupdate_complete(HFUPDATE_SW, FwUpdateObj.intFwLength) == HF_SUCCESS) 
        hfupdate_complete(HFUPDATE_SW, FwUpdateObj.intFwLength);
        {
			FwUpdateObj.FirmwareCompleted = 0;
            Reboot(RESTART_CODE);
        }

		//UdpDebugPrint("UDP updata finished,Now we restart!\n",strlen("UDP updata finished,Now we restart!\n"));
    }
}
/*******************************************************************************
* ��������: Http Client�����̼� 
* ��ڲ���: 
* ���ڲ���: 
* ʹ��˵��: 
*******************************************************************************/ 
static void UpdateFirmwareAsHttpClient(void)
{ 
    if(isBitSet(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_5_IS_URL_UPDATE_FW))
    {
       #if defined(__URL_UPDATE_DBG__) && defined(__UDP_PRINT_DBG__) 
        UdpDebugPrint("Url bit set:\r\n", sizeof("Url bit set:\r\n")-1); 
        UdpDebugPrint(FwUpdateObj.UpdateUrl, strlen(FwUpdateObj.UpdateUrl)); 
        UdpDebugPrint("\r\n", sizeof("\r\n")-1); 
       #endif 
		
        if(0x00 == hfthread_mutext_trylock(FwUpdateObj.FwUpdateMutx)) 
        { 
	        hfthread_enable_softwatchdog(NULL, 300);                        // 300��������� 
	        
            update_as_http(FwUpdateObj.UpdateUrl); 
            
            ClrBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_5_IS_URL_UPDATE_FW); 
            hfthread_mutext_unlock(FwUpdateObj.FwUpdateMutx); 
            // 
            hfthread_reset_softwatchdog(NULL);
	        hfthread_enable_softwatchdog(NULL, 30);                         // �ָ�Ϊ�߳�30����Watchdog 
        } 
    }
} 


/*******************************************************************************
* ��������: ���ö�ʱ�������߳�
* ��ڲ���: 
* ���ڲ���: 
* ʹ��˵��: 
* 600x=595 * 1000 => 991.66666
*******************************************************************************/ 
USER_FUNC void app_thread_and_timer_start(void) 
{ 
    App_Init(); 
    
    // ����һ���Զ���ʱ����ÿ1s�Ӵ���һ�Ρ�
    app_timer = hftimer_create("timer_1s", ONE_SEC_COUNT, true, APP_TIMER_ID, app_timer_callback1,  0); 
    
    hfthread_create(app_thread_func, "app_main", 256, (void*)1, HFTHREAD_PRIORITIES_LOW, NULL, NULL); 
    
    #if defined(__INDEPENT_SVR_REPORT_THREAD__)
    hfthread_create(PublicSvrfunc, "SvrRP", 256, (void*)1, HFTHREAD_PRIORITIES_LOW, NULL, NULL); 
    #endif 
} 

/******************************************************************************* 
* ��������: 
* ��ڲ���: 
* ���ڲ���: 
* ʹ��˵��: 
*******************************************************************************/ 
void KeyTimer_Init(void)
{
    ms_clock  = hftimer_create("clk_timer", 2, true, CLOCK_TIMER_ID, clock_timer_callback, 0); 
	hftimer_start(ms_clock); 
}

/******************************************************************************* 
* ��������: ��ʼ���豸����flash����
* ��ڲ���: 
* ���ڲ���: 
* ʹ��˵��: 
*******************************************************************************/ 
static void App_Init(void) 
{ 
    char        AtAckMsg[AT_ACK_MSG_LENGTH+1] = {0}; 
    char        *pProtocolMode;
    
    memset(&FwUpdateStatusObj, '\0', sizeof(FwUpdateStatusType)); 
    FwUpdateStatusObj.UdpFd = -1; 
 
    if(HF_SUCCESS == hfthread_mutext_new(&FwUpdateObj.FwUpdateMutx))
    { 
        hfat_send_cmd(  "AT+NETP\r\n", sizeof("AT+NETP\r\n")-1, 
                        AtAckMsg, AT_ACK_MSG_LENGTH); 
        pProtocolMode = strstr(AtAckMsg, "TCP");
        
        if(pProtocolMode)
        {
            hfat_send_cmd(  "AT+NETP=UDP,SERVER,8888,10.10.100.254\r\n", 
                            sizeof("AT+NETP=UDP,SERVER,8888,10.10.100.254\r\n")-1, 
                            AtAckMsg, AT_ACK_MSG_LENGTH); 

            //hfat_send_cmd("AT+Z\r\n", sizeof("AT+Z\r\n")-1, AtAckMsg, AT_ACK_MSG_LENGTH); 
 
        }

        Key_Init(); 
      
        ReadDevMac(); 
       
        ReadDevSakSN(); 
       
        ReadDevName();

        ReadPowerCoefficent();

        ServerNetworkInit(); 
        
        
        #if  defined(__DS1302_RTC_ENABLE__)  
        Rtc_Init(); 
        #else 
        SysTime_Init(); 
        #endif 
        
		PublicServer_Init();
		
		ReadDevRule();

		ReadDevStatus();

		PowerStatics_Init();		

		ReadDevTimeZone();		
		
        CurFwVersionObj.big_ver    = big_version; 
        CurFwVersionObj.little_ver = little_version; 
    }
} 
/******************************************************************************* 
* ��������: 
* ��ڲ���: 
* ���ڲ���: 
* ʹ��˵��: 
*******************************************************************************/ 
#if  defined(__WATCH_DOG_ENABLE__)
void FeedWatchDog(void)
{
    hfthread_reset_softwatchdog(NULL);                              // feed watchdog 
}
#endif 
/******************************************************************************* 
* ��������: 
* ��ڲ���: 
* ���ڲ���: 
* ʹ��˵��: 
*******************************************************************************/ 
void ServerNetworkInit(void)
{ 
    MainSvrSockAddr.sin_family      = AF_INET; 
	if(!switchMark)
	{
    	MainSvrSockAddr.sin_port        = htons(MAIN_SERVER_UDP_PORT); 
	}
	else
	{
		MainSvrSockAddr.sin_port        = htons(MAIN_SERVER_UDP_PORT_BACKUP); 
	}
//    MainSvrSockAddr.sin_addr.s_addr = inet_addr("255.255.255.255"); 

//    u_printf("Init main server:port = %x\n",MainSvrSockAddr.sin_port);
}


/******************************************************************************* 
* ��������: 
* ��ڲ���: 
* ���ڲ���: 
* ʹ��˵��: ����ԴƵ�ʣ����ö�Ӧ�Ĺ�������ʱ����
*******************************************************************************/ 
void ReadPowerFreq(void)
{
	if(reco_measure_data.reco_freq>7000 && reco_measure_data.reco_freq<8000)				//60HZ
	{

		delay_onTime = 11000;
		delay_offTime = 5000;
	}
	else if(reco_measure_data.reco_freq>8000)									//50HZ
	{

		//delay_onTime = 17500;
		//delay_offTime = 11200;
		delay_onTime = 19000;					//powercube
		delay_offTime = 12000;
	}
}



/******************************************************************************* 
* ��������: ��ȡ�豸��MAC��ַ
* ��ڲ���: 
* ���ڲ���: 
* ʹ��˵��: 
*******************************************************************************/ 
static void ReadDevMac(void)
{ 
    char        AtAckMsg[AT_ACK_MSG_LENGTH+1] = {0}; 
    uint32_t    DevMac[MAC_LEN+1]={0}; 
    char        *pTmp; 
    int         i; 
    i = hfat_send_cmd("AT+WSMAC\r\n", sizeof("AT+WSMAC\r\n")-1, AtAckMsg, AT_ACK_MSG_LENGTH); 
    if(i == HF_SUCCESS)
    { 
        pTmp = strchr(AtAckMsg, '='); 
        if(NULL != pTmp)
        { 
            pTmp++; 
            sscanf(pTmp, "%02X%02X%02X%02X%02X%02X", 
                          &DevMac[0],&DevMac[1],&DevMac[2], 
                          &DevMac[3],&DevMac[4],&DevMac[5]); 
            for(i=0; i<MAC_LEN; i++) 
            { 
                DevInfoObj.Dev_Mac[i] = DevMac[i];
            } 
        } 
    } 
}

/******************************************************************************* 
* ��������: ��ȡ�豸�ɣе�ַ
* ��ڲ���: 
* ���ڲ���: 
* ʹ��˵��: 
*******************************************************************************/ 
void ReadDevIP(void)
{ 
    char        AtAckMsg[AT_ACK_MSG_LENGTH+1] = {0}; 
    uint32_t    DevIP[IP_LENGTH+1]={0}; 
    char        *pTmp; 
    int         i; 
    i = hfat_send_cmd("AT+WANN\r\n", sizeof("AT+WANN\r\n")-1, AtAckMsg, AT_ACK_MSG_LENGTH); 
	
	//u_printf("AT+WANN=%s\n",AtAckMsg);
	
    if(i == HF_SUCCESS)
    { 
        pTmp = strchr(AtAckMsg, ','); 
        if(NULL != pTmp)
        { 
            pTmp++; 
            sscanf(pTmp, "%d.%d.%d.%d", &DevIP[0],&DevIP[1],&DevIP[2],&DevIP[3]);
			
            for(i=0; i<IP_LENGTH; i++) 
            { 
                DevInfoObj.dev_ip[i] = DevIP[i];
            } 

			//u_printf("DevIP:%d.%d.%d.%d\n",DevInfoObj.dev_ip[0],DevInfoObj.dev_ip[1],DevInfoObj.dev_ip[2],DevInfoObj.dev_ip[3]);
        } 
    } 
} 









