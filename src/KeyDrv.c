/*
********************************************************************************
* @file    KeyDrv.c
* @author  
* @version V1.0.0
* @date    03/20/2013
* @brief   
********************************************************************************
*/
#include <stdint.h> 
#include <stdio.h> 

#include "ConfigAll.h" 
#include "read_power.h"
#include "DevLib.h" 
#include "KeyDrv.h" 
#include "Flash.h"
#include "DevRule.h" 
#include "UdpServerCallBack.h" 


#define         KEY_POWER_AP_VALID_0_BIT                  0 

KeyType		    KeyObj;	

static void     Key_Check(void);

extern SwInformationType    SwInformation;
extern uint8_t keyFactorySuccess;      //按键恢复出厂设置是否成功标志
extern DevInfoType DevInfoObj;

/*******************************************************************************
* 功能描述: 按键扫描初始化 
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/
extern void Key_Init(void)
{
	KeyObj.KeyLastStatus = 0x00;
	KeyObj.KeyTemp       = 0x00;
	KeyObj.UpDown        = 0x00;
	KeyObj.KeyStatusCHK  = 0x00;
	//hfgpio_fpin_high(HFGPIO_SW_KEY_CTL_PIN);
}
/*******************************************************************************
* 功能描述: 
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/


/******************************************************************************* 
* 功能描述: 
* 入口参数: 
* 出口参数: 
* 使用说明: 
* 调用方法: 
*******************************************************************************/ 
extern uint8_t iSaveSwStateCount;
extern int SwitchPinArr[PORT_NUM];
void KeyTurnOnOffSwitch(void)
{ 
    if(SwInformation.PortInfor[0].CurStatus)
    { 	
		#if !defined(ZX_DETECT)
        hfgpio_fset_out_low(SwitchPinArr[0]);
		#endif
        SwInformation.PortInfor[0].OldStatus      = SwInformation.PortInfor[0].CurStatus;
        SwInformation.PortInfor[0].CurStatus      = 0;        
    } 
    else 
    { 
		#if !defined(ZX_DETECT)
        hfgpio_fset_out_high(SwitchPinArr[0]);
		#endif
        SwInformation.PortInfor[0].OldStatus      = SwInformation.PortInfor[0].CurStatus;
        SwInformation.PortInfor[0].CurStatus      = 1;
    } 
    
	#if  defined(__KEY_ON_OFF_DBG__) && defined(__UDP_PRINT_DBG__) 
	UdpDebugPrint("KeyOnOffSw\r\n", sizeof("KeyOnOffSw\r\n")-1);
	#endif 
    iSaveSwStateCount = 0x00; 
    SwInformation.PortInfor[0].OnOffStatus    = SwInformation.PortInfor[0].CurStatus;
} 




/******************************************************************************* 
* 功能描述: 复位键按键功能程序 
* 入口参数: 
* 出口参数: 
* 使用说明: 在ms时钟中断内调用 
*******************************************************************************/
extern FwUpdateStatusType      FwUpdateStatusObj; 
extern struct sockaddr_in      MainSvrSockAddr;                                    // 主服务器sockaddr 


extern void Key_Poll(void) 
{ 
    if(STATUS_STABLE == KeyObj.Data_Change_Flag) 
    { 
        return; 
    } 
    
    if((isBitSet(KeyObj.UpDown, KEY_0_NUM)) && (KEY_LONG_PRESS_3S < KeyObj.KeyDownTimeCount))                  // 按下大于3S KEY_LONG_PRESS_3S
    { 
        KeyObj.Data_Change_Flag = STATUS_STABLE; 
        ClrBit(KeyObj.UpDown, KEY_0_NUM); 
        if(isBitSet(KeyObj.KeyStatusCHK, KEY_POWER_AP_VALID_0_BIT))     // AP Key valid 
        {            
             FlashResetFactory(); 
        }
        return; 
    } 
    if(0 == isBitSet(KeyObj.UpDown, KEY_0_NUM))                         // 按键按下至弹起小于3S 
    { 
        KeyObj.Data_Change_Flag = STATUS_STABLE; 
        ClrBit(KeyObj.UpDown, KEY_0_NUM); 
        
        if(isBitSet(KeyObj.KeyStatusCHK, KEY_POWER_AP_VALID_0_BIT))     // AP Key valid 
        {
            KeyTurnOnOffSwitch();
			SwInformation.portChangeReason = PORT_CHANGE_BY_KEY;
			#if defined(ZX_DETECT)
            plug_updata = 1;
			#endif

			//u_printf("keyDrv.c--156\n");
        }
        return; 
    } 
} 
/******************************************************************************* 
* 功能描述: 按键扫描驱动脉搏,驱动按键扫描 
* 入口参数: 
* 出口参数: 
* 使用说明: 在ms时钟中断内调用 
*******************************************************************************/
extern void Key_DriveEngine(int32_t ArgTick)
{ 
    if(isBitSet(KeyObj.UpDown, KEY_0_NUM))                              // 按键按下 
    { 
        KeyObj.KeyDownTimeCount++; 
    } 
	if((STATUS_STABLE == KeyObj.Data_Change_Flag) || ((ArgTick%4) == 0)) 
	{ 
        KeyObj.Matrix_Scan_Flag = 0x00;
	} 
    
    return;
} 
/*******************************************************************************
* 功能描述: 
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/
extern void Key_Scan(void)
{
	if(0x01 == KeyObj.Matrix_Scan_Flag)
	{ 
		return;
	} 
	KeyObj.Matrix_Scan_Flag 	= 0x01; 
	
    KeyObj.KeyTemp   = 0x00; 
	if(KEY_PRESS_STATUS == KEY_IN) 
	{ 
        //u_printf("Key Press \r\n");
        
	    SetBit(KeyObj.KeyTemp, KEY_0_NUM); 
	}
	else 
	{
        //u_printf("Key Reset Factory\r\n");
        
	    SetBit(KeyObj.KeyStatusCHK, KEY_POWER_AP_VALID_0_BIT); 
	}
	Key_Check();													    // 检测按键状态是否发生变化，并检出变化状态
}
/*******************************************************************************
* 功能描述: 检查按键状态是否发生变化 
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/
static void Key_Check(void)
{
	uint8_t i = 0;
	
	for(i=0; i<portNum; i++)
	{ 
		if( (KeyObj.KeyLastStatus & (1<<i)) != (KeyObj.KeyTemp & (1<<i)) )  // 状态发生变化 
		{ 
			ClrBit(KeyObj.KeyLastStatus, i); 
            OrBit (KeyObj.KeyLastStatus, (KeyObj.KeyTemp & (1<<i))); 
			ClrBit(KeyObj.UpDown, i); 
            OrBit (KeyObj.UpDown, (KeyObj.KeyTemp & (1<<i))); 
            
            if(isBitSet(KeyObj.UpDown, KEY_0_NUM))                          // 按键按下 
            { 
                KeyObj.KeyDownTimeCount = 0;                                // 按键按下时间清0 
            } 
			KeyObj.Data_Change_Flag = STATUS_CHG_VAR;                       // 有按键更新事件 
		} 
	} 
}
/******************************************************************************* 
* ========================= END OF PAGE 
*******************************************************************************/ 

