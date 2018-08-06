/*
********************************************************************************
* @file    : DevRule.c 
* @author  : 
* @version : V1.0.0 
* @date    : 2014/03/20 
* @brief   : 
********************************************************************************
*/

#include <stdint.h> 
#include <stdio.h> 
#include <string.h> 

#include <cJSON.h> 
#include "hsf.h" 
#include "read_power.h"
#include "KeyDrv.h" 
#include "DevInfo.h" 
#include "Flash.h" 
#include "DevLib.h" 
#include "DevRule.h" 
#include "DevDataType.h" 
#include "RtTime.h" 
#include "PublicServerCom.h"
#include "Produce_Test.h"
#include "rn8209.h"

// 
int SwitchPinArr[PORT_NUM] = {HFGPIO_SW_ON_OFF_CTL_PIN}; 
// 
extern DevInfoType      DevInfoObj;
extern DevRuleInfoType  DevRuleInfoObj; 
extern TurnOffTimeType  CountDownTurnOff[PORT_NUM+1]; 
extern DevTimeType      DevTimeObj;
extern PowerRecordType  PowerRecordObj[PORT_NUM]; 
extern FwUpdateStatusType FwUpdateStatusObj; 


uint8_t SwOnOffData = 0;            //the package of switch on/off state
uint8_t portNum = PORT_NUM;
uint8_t LedFlashControl[6] = {0};
RelatedCtrlType	RelatedCtrl;			//关联控制机构体


#if (PORT_NUM==6)
SwInformationType    SwInformation = {  0,
                                        0,
                                        0,
                                        0,
                                        0,
                                        {{"PORT1",0,0,0,2,0},
                                         {"PORT2",0,0,0,2,0},
                                         {"PORT3",0,0,0,2,0},
                                         {"PORT4",0,0,0,2,0},
                                         {"PORT5",0,0,0,2,0},
                                         {"PORT6",0,0,0,2,0}}
                                      };
#elif (PORT_NUM == 1)
SwInformationType    SwInformation = {  0,
                                        0,
                                        0,
                                        0,
                                        0,
                                        {"PORT1",0,0,0,2,0},
                                      };
#endif                                    

                                   




/*******************************************************************************
* 功能描述: 读取设备规则
* 输入参数: pDataBuf:规则保存缓冲	pageID:读取哪一页的规则
* 输出参数: 
* -返回值-: 
* 使用说明: 
*******************************************************************************/
char *GetDevRuleInfo(char *pDataBuf,uint8_t pageID) 
{ 
    int     iLength;
    char    *pBuf = pDataBuf;
    uint8_t i, validDay,ruleCount;
	uint8_t defMode = 0;
    
    if(0x00 < DevRuleInfoObj.RuleCount) 
    { 
        pBuf = pDataBuf; 
		ruleCount = ((DevRuleInfoObj.RuleCount-pageID*PAGE_COUNT)/PAGE_COUNT>0)?((pageID+1)*PAGE_COUNT):DevRuleInfoObj.RuleCount;
		
        for(i=0+pageID*PAGE_COUNT; i<ruleCount; i++) 
        { 
            validDay   = DevRuleInfoObj.SwitchRulePool[i].SwOpDateObj;
           #if(PORT_NUM == 6)
			{
	            iLength = sprintf(pBuf, "{\"mode\":%d,"
	            						"\"id\":%d,"
	                                    "\"en\":%d,"
	                                    "\"port\":[%d,%d,%d,%d,%d,%d],"
	                                    "\"day\":%d,"
	                                    "\"time\":%d,",
	                                    defMode,
	                                    DevRuleInfoObj.SwitchRulePool[i].RuleID, 
	                                    DevRuleInfoObj.SwitchRulePool[i].RuleEnable, 
	                                    DevRuleInfoObj.SwitchRulePool[i].PortArr[0],DevRuleInfoObj.SwitchRulePool[i].PortArr[1],
	                                    DevRuleInfoObj.SwitchRulePool[i].PortArr[2],DevRuleInfoObj.SwitchRulePool[i].PortArr[3],
	                                    DevRuleInfoObj.SwitchRulePool[i].PortArr[4],DevRuleInfoObj.SwitchRulePool[i].PortArr[5],
	                                    DevRuleInfoObj.SwitchRulePool[i].CurTimeStamp,DevRuleInfoObj.SwitchRulePool[i].timeLong); 
			}
		   #elif(PORT_NUM == 1)
			{
				iLength = sprintf(pBuf, "{\"mode\":%d,"
	            						"\"id\":%d,"
	                                    "\"en\":%d,"
	                                    "\"port\":[%d],"
	                                    "\"day\":%d,"
	                                    "\"time\":%d,",
	                                    defMode,
	                                    DevRuleInfoObj.SwitchRulePool[i].RuleID, 
	                                    DevRuleInfoObj.SwitchRulePool[i].RuleEnable, 
	                                    DevRuleInfoObj.SwitchRulePool[i].PortArr[0],
	                                    DevRuleInfoObj.SwitchRulePool[i].CurTimeStamp,DevRuleInfoObj.SwitchRulePool[i].timeLong); 
			}
		   #endif
            pBuf += iLength;
          
            iLength = sprintf(pBuf, "\"week\":[%d,%d,%d,%d,%d,%d,%d]},", 
                                    (0x01&(validDay>>0)), (0x01&(validDay>>1)), (0x01&(validDay>>2)), 
                                    (0x01&(validDay>>3)), (0x01&(validDay>>4)), (0x01&(validDay>>5)), 
                                    (0x01&(validDay>>6))); 
            pBuf += iLength; 
        }
        pBuf--;                                                                 // 最后一条规则无逗号分隔符，故占据最后一条规则后的逗号分隔符的位置。
        *pBuf = '\0';
    } 
    return pBuf; 
} 


/*******************************************************************************
* 功能描述:终端倒计时实现函数
* 输入参数:
* 输出参数:
* -返回值-: 
* 使用说明:
*******************************************************************************/
void CountDownProcess(time_t ArgCurTime)
{
    uint8_t i;

    //u_printf("ArgCurTime=%ld\n",ArgCurTime);
    
    for(i=0; i<PORT_NUM; i++)
    {
        if(CountDownTurnOff[i].isCountDown)
        {
             if((ArgCurTime >= CountDownTurnOff[i].StartTimePoint+CountDownTurnOff[i].CountDownValue)&&
                            (ArgCurTime < CountDownTurnOff[i].StartTimePoint+CountDownTurnOff[i].CountDownValue+5))
            {
               
                CountDownTurnOff[i].isCountDown = 0; 

        	    if(CountDownTurnOff[i].onOrOff == 0)
        	    {
        	       #if (PORT_NUM == 6)
                    Switch_TurnOnOff(i+1, 0); 
                   #elif (PORT_NUM == 1)
                    Port_TurnOnOff(0);
					plug_updata = 1;

					//u_printf("devRule.c--328\n");
                   #endif                    
                }
                else
                {
                   #if (PORT_NUM == 6)
                    Switch_TurnOnOff(i+1, 1);
                   #elif(PORT_NUM == 1)
                    Port_TurnOnOff(1);
				    plug_updata = 1;

					//u_printf("devRule.c--339\n");
                   #endif
                }
                SwInformation.portChangeReason = PORT_CHANGE_BY_COUNTDOWN;
                //UdpDebugPrint("Now excute count down!\r\n",sizeof("Now excute count down!\r\n")-1);
            }
            else if(ArgCurTime > CountDownTurnOff[i].StartTimePoint+CountDownTurnOff[i].CountDownValue+5)
            {
                CountDownTurnOff[i].isCountDown = 0;
            } 

            //u_printf("ArgCurTime=%ld,CountDownTurnOff[%d].StartTimePoint=%d delaySec=%d\n",
            //            ArgCurTime,i,CountDownTurnOff[i].StartTimePoint,CountDownTurnOff[i].CountDownValue);
        }
    }
}


/******************************************************************************* 
* 功能描述: 修改设备名称
* 入口参数: 
* 出口参数: 
* 使用说明: 
* 调用方法: 0表示修改排插名称   1~N表示修改相应插口名称
*******************************************************************************/ 
uint8_t ModifyDevName(char *pCharArg)
{   
	cJSON   *json;
	char    *name; 
	int     name_len,iLength; 
	uint8_t portID,iRet; 
    
	iRet = FAILED;	
	json = cJSON_Parse(pCharArg); 
	if(json) 
    {       
        portID     = cJSON_GetObjectItem(json, "port")->valueint;
		name     = cJSON_GetObjectItem(json, "name")->valuestring; 
		name_len = strlen(name); 
		iLength  = (name_len > NAME_LENGTH)?(NAME_LENGTH):(name_len); 
      #if(PORT_NUM == 6)
		if(0 == portID)
		{
		    memset(DevInfoObj.dev_name, 0, NAME_LENGTH); 
		    memcpy(DevInfoObj.dev_name, name, iLength); 
		}
		else
		{
    		memset(SwInformation.PortInfor[portID-1].name, 0, NAME_LENGTH); 
    		memcpy(SwInformation.PortInfor[portID-1].name, name, iLength); 

    		//u_printf("PortInfor[%d].name=%s\n",portID-1,SwInformation.PortInfor[portID-1].name);
        }
      #elif(PORT_NUM == 1)
        memset(DevInfoObj.dev_name, 0, NAME_LENGTH); 
	    memcpy(DevInfoObj.dev_name, name, iLength); 
	  #endif
        WriteDevName();

		iRet     = PASSED;
//		PublicServer_SyncBaseInfoWithServer();
		    
		cJSON_Delete(json); 

        //u_printf("DevInfoObj.dev_name=%s\n",DevInfoObj.dev_name);
		
	} 
    return iRet;
}

/******************************************************************************* 
* 功能描述: 
* 入口参数: 
* 出口参数: 
* 使用说明: 
* 调用方法: 
*******************************************************************************/ 
uint8_t Switch_Ctl(char *pBuf)
{
	cJSON   *json;
	int     state;
    int     port;
	uint8_t iRet;
	
    //u_printf("Onoff pBuf = %s\n",pBuf);
    // UdpDebugPrint("Now report OnOff CMD\r\n", strlen("Now report OnOff CMD\r\n"));
    // UdpDebugPrint(pBuf, strlen(pBuf));    
    
	iRet = FAILED;
	json = cJSON_Parse(pBuf);
	
	if(json)
	{
		state = cJSON_GetObjectItem(json,"state")->valueint;
        port  = cJSON_GetObjectItem(json,"port")->valueint;
        
		cJSON_Delete(json);

		
		orderArray[3].OrderMark = 1;
       
		if(state == 0)
		{
		   #if(PORT_NUM == 6)
		    PowerRecordObj[port].record_ValidPower = 0;
		    PowerRecordObj[port].record_ValidCurrent = 0;
		   #elif(PORT_NUM == 1)
		    PowerRecordObj[0].record_ValidPower = 0;
		    PowerRecordObj[0].record_ValidCurrent = 0;
		   #endif
		}
	   #if (PORT_NUM == 6)
	    Switch_TurnOnOff(port, state);
	   #elif (PORT_NUM == 1)
	    Port_TurnOnOff(state);
	   	#if defined(ZX_DETECT)
	    plug_updata = 1;
		#endif

		//u_printf("devRule.c--461\n");
		
	   #endif

		iRet = PASSED;
		SwInformation.portChangeReason = PORT_CHANGE_BY_SERVER;
        
        SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_8_IS_SYNC_RT_DATA_CMD);          //上报当前状态    
		//u_printf("After:state=%d port=%d\n",state,port);
		//UdpDebugPrint("Now excute Switch_Ctl,DevRule.c 445 line!\r\n",sizeof("Now excute Switch_Ctl,DevRule.c 445 line!\r\n")-1);
	}
	
	return iRet;

}


/******************************************************************************* 
* 功能描述: 控制关联设备
* 入口参数: 
* 出口参数: 
* 使用说明: 
* 调用方法: 
*******************************************************************************/ 
uint8_t Switch_RelatedCtl(char *pBuf)
{
	cJSON   *json,*port,*after;
	uint8_t port_num,iRet,i;
	uint8_t state = 0;

	//char testBuf[100];
    //u_printf("Onoff pBuf = %s\n",pBuf);
    // UdpDebugPrint("Now report OnOff CMD\r\n", strlen("Now report OnOff CMD\r\n"));
    // UdpDebugPrint(pBuf, strlen(pBuf));
    
	iRet = FAILED;
	json = cJSON_Parse(pBuf);
	
	if(json)
	{
		port        = cJSON_GetObjectItem(json, "state");
		port_num    = cJSON_GetArraySize(port);	

		for(i=0; i<port_num; i++)
		{
			if((cJSON_GetArrayItem(port,i)->valueint) != 2)
			{
				state |= ((cJSON_GetArrayItem(port,i)->valueint)<<i);
			}
			else
			{
				state |= SwInformation.PortInfor[i].OnOffStatus<<i;
			}
			
			//u_printf("rule[%d]:day[%d] = %d\n",ruleID,j,cJSON_GetArrayItem(week,j)->valueint);
		}

		after = cJSON_GetObjectItem(json, "after");
		if(!after)
		{
			RelatedCtrl.delayTime = 0;
		}
		else
		{
			RelatedCtrl.delayTime = cJSON_GetObjectItem(json,"after")->valueint;
		}
		
		//RelatedCtrl.delayTime = cJSON_GetObjectItem(json,"after")->valueint;
		RelatedCtrl.holdTime = cJSON_GetObjectItem(json,"delay")->valueint;
        
		cJSON_Delete(json);
		
	   #if(PORT_NUM == 6)
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

		RelatedCtrl.preStates = SwOnOffData;
		RelatedCtrl.ctrStates = state;
			
	    if(state != SwOnOffData)
	    {
//			sprintf(testBuf,"state = %d\t	SwOnOffData = %d\n",state,SwOnOffData);
//			UdpDebugPrint(testBuf,sizeof(testBuf));
					
			if(0 == RelatedCtrl.delayTime)
			{
				SwOnOffData = state;
				UsartControlPortOnOff(); 
				SwInformation.portChangeReason = PORT_CHANGE_BY_AUTO;
			}		
			//UdpDebugPrint("Now excute related control!\r\n",sizeof("Now excute related control!\r\n")-1);
	    }

	  #elif(PORT_NUM == 1)
	   
	    //sprintf(testBuf,"cmd_state = %d\t	cur_state = %d\n",state,SwInformation.PortInfor[0].OnOffStatus);
		//UdpDebugPrint(testBuf,sizeof(testBuf));

	  	RelatedCtrl.preStates = SwInformation.PortInfor[0].OnOffStatus;
		RelatedCtrl.ctrStates = state;
			
	    if(state != SwInformation.PortInfor[0].OnOffStatus)
	    {
			if(0 == RelatedCtrl.delayTime)
			{
				SwInformation.PortInfor[0].OnOffStatus = state;
				Port_TurnOnOff(state);
				SwInformation.portChangeReason = PORT_CHANGE_BY_AUTO;
			}
	    }
	  #endif

	  	RelatedCtrl.ctlTime = DevTimeObj.CurTimeSec;
		RelatedCtrl.ctlMark = 1;
		RelatedCtrl.afterMark = 0;

		iRet = PASSED;
        
        SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_8_IS_SYNC_RT_DATA_CMD);          //上报当前状态    
		//u_printf("After:state=%d port=%d\n",state,port);
		//UdpDebugPrint("Now excute Switch_Ctl,DevRule.c 445 line!\r\n",sizeof("Now excute Switch_Ctl,DevRule.c 445 line!\r\n")-1);
	}
	
	return iRet;

}



/******************************************************************************* 
* 功能描述: 控制关联设备恢复之前状态
* 入口参数: 
* 出口参数: 
* 使用说明: 
* 调用方法: 
*******************************************************************************/ 
void Switch_delayCtl(uint8_t ctlStates)
{
//		char testBuf[100];
		
	
#if(PORT_NUM == 6)
		SwOnOffData = ctlStates;

//		sprintf(testBuf,"SwOnOffData = %d\n",SwOnOffData);
//		UdpDebugPrint(testBuf,sizeof(testBuf));
		
		UsartControlPortOnOff(); 
#elif(PORT_NUM == 1)
		SwInformation.PortInfor[0].OnOffStatus = ctlStates;
		Port_TurnOnOff(ctlStates);
		#if defined(ZX_DETECT)
	     plug_updata = 1;
		#endif

		//u_printf("devRule.c--594\n");
#endif
	  SwInformation.portChangeReason = PORT_CHANGE_BY_AUTO;

	  SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_8_IS_SYNC_RT_DATA_CMD);          //上报当前状态SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_8_IS_SYNC_RT_DATA_CMD);          //上报当前状态

	  //UdpDebugPrint("Now recover related control!\r\n",sizeof("Now recover related control!\r\n")-1);
}




/*******************************************************************************
* 功能描述: 获取倒计时信息
* 输入参数: 倒计时信息保存缓冲
* 输出参数: 
* -返回值-: 
* 使用说明: 
*******************************************************************************/
char *GetCountDownInfo(char *pArgBuf)
{

	int     iLength;
    char    *pBuf = pArgBuf;
    uint8_t i;
    
    for(i=0; i<PORT_NUM; i++)  
    { 
		if(CountDownTurnOff[i].isCountDown)
		{
	        iLength = sprintf(pBuf, "{\"port\":%d,"
	                                "\"switch\":%d,"
	                                "\"start\":%d,"
	                                "\"delay\":%d},",
	                                i+1, 
	                                CountDownTurnOff[i].onOrOff, 
	                                CountDownTurnOff[i].ReceiveStartTime,
	                                CountDownTurnOff[i].CountDownValue); 
	        pBuf += iLength;
			pBuf--;                                                                 // 最后一条倒计时无逗号分隔符，故占据最后一条规则后的逗号分隔符的位置。
    		*pBuf = '\0';
		}
    } 



	//u_printf("countDown buffer:%s\n",pBuf);
    return pBuf; 
	
}




/******************************************************************************* 
* 功能描述:设置排插端口的开关状态
* 入口参数: cmd:order cmd;port ID:the switch port ID;data:the order data
* 出口参数: 
* 使用说明: 
* 调用方法: 
*******************************************************************************/ 
void SetPortState(uint8_t portID,uint8_t onOffState)
{
    uint8_t i=0;
    
    if(0x00 != portID)
    {
        
        SwInformation.PortInfor[portID-1].OldStatus      = SwInformation.PortInfor[portID-1].CurStatus;
        SwInformation.PortInfor[portID-1].CurStatus      = onOffState;
        SwInformation.PortInfor[portID-1].OnOffStatus    = SwInformation.PortInfor[portID-1].CurStatus;
    }
    else
    {
        for(i=0;i<PORT_NUM;i++)
        {
            SwInformation.PortInfor[i].OldStatus      = SwInformation.PortInfor[i].CurStatus;
            SwInformation.PortInfor[i].CurStatus      = onOffState;
            SwInformation.PortInfor[i].OnOffStatus    = SwInformation.PortInfor[i].CurStatus;
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

   // u_printf("SwOnOffData = %d\n",SwOnOffData);
}

#if 0
/******************************************************************************* 
* 功能描述:获取排插端口的开关状态
* 入口参数: 
* 出口参数: 0:关；1:开
* 使用说明: 
* 调用方法: 
*******************************************************************************/ 
uint8_t GetPortState(uint8_t port)
{    
      return ((SwOnOffData&(1<<port))>>port);    
}
#endif

/******************************************************************************* 
* 功能描述: 针对每个口的开关控制
* 入口参数: 
* 出口参数: 
* 使用说明: 
* 调用方法: 
*******************************************************************************/ 
void Switch_TurnOnOff(uint8_t Port, uint8_t state)
{
    SetPortState(Port,state);
    UsartControlPortOnOff();                    	       

	SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_8_IS_SYNC_RT_DATA_CMD);			//上报当前状态	  
	//UdpDebugPrint("Now OnOffSwitch--DevRule.c---829\r\n",strlen("Now OnOffSwitch--DevRule.c---829\r\n"));
    //u_printf("portNum = %d,SwOnOffData = %d, port=%d,state=%d\n",portNum,SwOnOffData,Port,state);
}



/******************************************************************************* 
* 功能描述: 单口排插的开关控制
* 入口参数: 
* 出口参数: 
* 使用说明: 
* 调用方法: 
*******************************************************************************/ 
extern uint8_t iSaveSwStateCount;
void Port_TurnOnOff( uint8_t state)
{
	#if !defined(ZX_DETECT)
        if(state)
        {
            hfgpio_fset_out_high(SwitchPinArr[0]);               // On 
        }
        else
        {
            hfgpio_fset_out_low(SwitchPinArr[0]);                // Off 
        }
	#endif
	
        iSaveSwStateCount = 0x00; 
    	
        SwInformation.PortInfor[0].OldStatus      = SwInformation.PortInfor[0].CurStatus;
        SwInformation.PortInfor[0].CurStatus      = state;
        SwInformation.PortInfor[0].OnOffStatus    = SwInformation.PortInfor[0].CurStatus;

        plug_updata = 1;

		
        SetBit(FwUpdateStatusObj.TimelyCheck, TIMELY_CKECK_BIT_8_IS_SYNC_RT_DATA_CMD);          //上报当前状态    
		//UdpDebugPrint("Port_TurnOnOff,DevRule.c 688 line!\r\n", sizeof("Port_TurnOnOff,DevRule.c 688 line!\r\n")-1);
		//u_printf("Now we control the port!!!!!\n");
}



/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/

