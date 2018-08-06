/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *-pipe -fno-strict-aliasing -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror-implicit-function-declaration 
 -Wpointer-arith -std=gnu99 -ffunction-sections -fdata-sections -Wchar-subscripts -Wcomment -Wformat=2 -Wimplicit-int 
 -Wmain -Wparentheses -Wsequence-point -Wreturn-type -Wswitch -Wtrigraphs -Wunused -Wuninitialized -Wunknown-pragmas 
 -Wfloat-equal -Wundef -Wshadow -Wbad-function-cast -Wwrite-strings -Wsign-compare -Waggregate-return  
 -Wmissing-declarations -Wformat -Wmissing-format-attribute -Wno-deprecated-declarations -Wpacked -Wredundant-decls 
 -Wnested-externs -Wlong-long -Wunreachable-code -Wcast-align --param max-inline-insns-single=500
 */
#include <hsf.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "DevInfo.h"
#include "DevLib.h"
#include "LED.h"

#include "HttpdCallBack.h"
#include "UdpServerCallBack.h"
#include "SwitchApp.h"

#include "flash.h"

#include "Produce_Test.h"

#include "UdpSocket.h" 
#include "TcpSocket.h" 


#define     HFM_TYPE        HFM_TYPE_LPB100 
const int hf_gpio_fid_to_pid_map_table[HFM_MAX_FUNC_CODE]=
{
	HF_M_PIN(2),	//HFGPIO_F_JTAG_TCK
	HFM_NOPIN,	    //HFGPIO_F_JTAG_TDO
	HFM_NOPIN,	    //HFGPIO_F_JTAG_TDI
	HF_M_PIN(5),	//HFGPIO_F_JTAG_TMS
	HFM_NOPIN,		//HFGPIO_F_USBDP

	HFM_NOPIN,		//HFGPIO_F_USBDM
	HF_M_PIN(39),	//HFGPIO_F_UART0_TX
	HF_M_PIN(40),	//HFGPIO_F_UART0_RTS
	HF_M_PIN(41),	//HFGPIO_F_UART0_RX
	HF_M_PIN(42),	//HFGPIO_F_UART0_CTS
	
	HF_M_PIN(27),	//HFGPIO_F_SPI_MISO
	HF_M_PIN(28),	//HFGPIO_F_SPI_CLK
	HF_M_PIN(29),	//HFGPIO_F_SPI_CS
	HF_M_PIN(30),	//HFGPIO_F_SPI_MOSI
	
	HFM_NOPIN,	    //HFGPIO_F_UART1_TX,
	HFM_NOPIN,	    //HFGPIO_F_UART1_RTS,
	HFM_NOPIN,	    //HFGPIO_F_UART1_RX,
	HFM_NOPIN,	    //HFGPIO_F_UART1_CTS,

	// 
	#if  1 
	HF_M_PIN(43),	// HFGPIO_F_NLINK
	#else 
	HFM_NOPIN,	// HFGPIO_F_NLINK
	#endif 
	
	HF_M_PIN(44),	// HFGPIO_F_NREADY
	HF_M_PIN(45),	// HFGPIO_F_NRELOAD
	HF_M_PIN(7),	// HFGPIO_F_SLEEP_RQ
	HF_M_PIN(8),	// HFGPIO_F_SLEEP_ON

    #if  1 
	HF_M_PIN(15),	// HFGPIO_F_WPS
	#else 
	HFM_NOPIN,		//
	#endif 
	
	HFM_NOPIN,		// HFGPIO_F_RESERVE1
	HFM_NOPIN,		// HFGPIO_F_RESERVE2
	HFM_NOPIN,		// HFGPIO_F_RESERVE3
	HFM_NOPIN,		// HFGPIO_F_RESERVE4
	HFM_NOPIN,		// HFGPIO_F_RESERVE5
	
	//HFM_NOPIN,	// HFGPIO_F_USER_DEFINE
	HF_M_PIN(13),	// (HFGPIO_F_USER_DEFINE+0)--HFGPIO_SW_KEY_CTL_PIN 
	
	HF_M_PIN(20),	// HFGPIO_SW_ON_OFF_CTL_PIN 
	
	HF_M_PIN(11),	// SCLK 
	HF_M_PIN(12),	// SDA 
	HF_M_PIN(18),	// RST 
	HF_M_PIN(23),	// ZX
	//HF_M_PIN(43),	// wifi LED
};

const hfat_cmd_t user_define_at_cmds_table[]=
{
	{NULL,NULL,NULL,NULL} //the last item must be null 
};

extern FwUpdateStatusType      FwUpdateStatusObj; 
extern DevStatusType           DevStatusObj;
#if   defined(__DOWN_SHUT_DBG__) 

/***************打印模块重启原因******************************/
static void show_reset_reason(void)
{
	uint32_t reset_reason=0;
	reset_reason = hfsys_get_reset_reason();     //API:Get the module reset 
	// reason       
	u_printf("reset_reasion:%08x\n",reset_reason);
	if(reset_reason&HFSYS_RESET_REASON_ERESET)
	{
		u_printf("ERESET\n");
		//UdpDebugPrint("ERESET\n",sizeof("ERESET\n")-1);    
	}
	if(reset_reason&HFSYS_RESET_REASON_IRESET0)
	{
		u_printf("IRESET0\n");
		//UdpDebugPrint("IRESET0\n",sizeof("IRESET0\n")-1);    
	}
	if(reset_reason&HFSYS_RESET_REASON_IRESET1)
	{
		u_printf("IRESET1\n");
		//UdpDebugPrint("IRESET1\n",sizeof("IRESET1\n")-1);    
	}
	if(reset_reason==HFSYS_RESET_REASON_NORMAL)
	{
		u_printf("RESET NORMAL\n");
		//UdpDebugPrint("RESET NORMAL\n",sizeof("RESET NORMAL\n")-1);    
	}
	if(reset_reason&HFSYS_RESET_REASON_WPS)
	{
		u_printf("RESET FOR WPS\n");
		//UdpDebugPrint("RESET FOR WPS\n",sizeof("RESET FOR WPS\n")-1);    
	}
	if(reset_reason&HFSYS_RESET_REASON_SMARTLINK_START)
	{
		u_printf("RESET FOR SMARTLINK START\n");
		//UdpDebugPrint("RESET FOR SMARTLINK START\n",sizeof("RESET FOR SMARTLINK START\n")-1);    
	}
	if(reset_reason&HFSYS_RESET_REASON_SMARTLINK_OK)
	{
		u_printf("RESET FOR SMARTLINK OK\n");
		//UdpDebugPrint("RESET FOR SMARTLINK OK\n",sizeof("RESET FOR SMARTLINK OK\n")-1);    
	}
	if(reset_reason&HFSYS_RESET_REASON_WPS_OK)
	{
		u_printf("RESET FOR WPS OK\n");
		//UdpDebugPrint("RESET FOR WPS OK\n",sizeof("RESET FOR WPS OK\n")-1);    
	}
	return;
}
#endif 

/*************register the system callback********************/
static int hfsys_event_callback( uint32_t event_id,void *param)
{
	switch(event_id)
	{
		case HFE_WIFI_STA_CONNECTED:         //when STA connext success trigger
		{
		    ClrBit(DevStatusObj.LedShowStatus, WIFI_LED_SHOW_0_BIT);
		    LedCtl(WIFI_LED_ON); 
		}
		break;
		case HFE_WIFI_STA_DISCONNECTED:
		{
		    // wifi快闪烁 
		    SetBit(DevStatusObj.LedShowStatus, WIFI_LED_SHOW_0_BIT);
		}
		break;
		case HFE_DHCP_OK:
		{
			#if  0 
			uint32_t *p_ip;
			p_ip = (uint32_t*)param;
			u_printf("dhcp ok %08X!\n", *p_ip);
			#endif 
		}
		break;
		case HFE_SMTLK_OK:
		{ 
            //EnterAPSTAMode();
			return 0; 
		}
		//break;
		case HFE_CONFIG_RELOAD:
		{
		}
		break;
		default: break;
	}
	return 0;
}

extern SwInformationType    SwInformation;
int USER_FUNC app_main (void)
{
    u_printf("The version is :%d.%02d\n",big_version,little_version);
    
	if(hfgpio_fmap_check(HFM_TYPE)!=0)
	{
		while(1)
		{
			msleep(1000);
		}
	}

   #if   defined(__DOWN_SHUT_DBG__) 
	show_reset_reason();
   #endif 

	DevGPIO_Init();                 //Init the device
	hfsys_register_system_event((hfsys_event_callback_t)hfsys_event_callback); 
 	
	while(!hfnet_wifi_is_active()) 
	{
		msleep(50); 
	}    

	app_thread_and_timer_start();
	
	//EnterAPSTAMode();
	DevMode_Init();
	
	hfnet_start_uart(HFTHREAD_PRIORITIES_LOW, (hfnet_callback_t)uart_process_callback);

     
    HttpdServerThread_Create();
 	UdpServerThread_Create();

	return 1;
}
