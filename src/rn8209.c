/*
********************************************************************************
* @file    : Rn8209.c 
* @author  : 
* @version : V1.0.0 
* @date    : 2014/06/27 
* 
* @brief   : Rn8209G:Big endian 
*          : LPB100 little endian 
********************************************************************************
*/
#include <stdio.h>
#include <string.h>

#include <hsf.h>
#include "read_power.h"
#include "ConfigAll.h"
#include "customer_spi.h"
#include "rn8209.h"
#include "DevLib.h"
#include "Flash.h"
#include "cJSON.h"
#include "DevDataType.h"
#include "DevInfo.h"
#include "DevRule.h"
#include "Produce_Test.h"




extern SwInformationType       SwInformation;
extern RecoMeasurePack         reco_measure_data; 

PowerRecordType PowerRecordObj[PORT_NUM]; 

uint8_t CalibrateData[8] = {0};
//uint8_t findMark = 0;		//���ҵ�ĳ���ڵ�������


#if (PORT_NUM == 6)
const int OverCurrentValueArr[] = 
{
    17600,         // ŷ���������ֵ 16+1.6 
    16500,         // �����������ֵ 15+1.5 
    11000,         // �й��������ֵ 10+1.0 
    17600,         // ��ʿ���������ֵ 16+1.6 
    14300,         // Ӣ���������ֵ 13+1.3 
    17600          // �����������ֵ 16+1.6 
};
#else
const int OverCurrentValueArr[] = 
{
    17600,         // ŷ���������ֵ 16+1.6 
    16500,         // �����������ֵ 15+1.5 
    11000,         // �й��������ֵ 10+1.0 
    17600,         // ��ʿ���������ֵ 16+1.6 
    14300,         // Ӣ���������ֵ 13+1.3 
    17600,          // �����������ֵ 16+1.6 
    17600          // ��ɫ�й��������ֵ 16+1.6 
};
#endif




/*******************************************************************************
* ��������: ���ܱ������ܺ�����ѭ�����ÿ���ڵĹ����������ȡ��Ӧ������ʩ
* ��ڲ���: 
* ���ڲ���: 
* ʹ��˵��: 
*******************************************************************************/

#if   defined(__PROTECT_MOD_ENABLE__) 
extern DevInfoType DevInfoObj;
#define     OVER_CURRENT_STATUS     0x02 
void OverCurrentProtectPoll(void)
{
    uint8_t portID = 0,maxID = 0;
	
	//char *pBuf;
	//char testBuf[100];
	//int iLength;
	
	static uint8_t confirmCount = 0;
    int totle_current = 0,iStandard=0;
    int max_current = PowerRecordObj[0].record_ValidCurrent;
	
	//memset(testBuf,0,sizeof(testBuf));
	//pBuf = testBuf;

    iStandard        = DevInfoObj.dev_sn[6] - '0';
    if(iStandard > Dim(OverCurrentValueArr))
    {
        return;
    }
    
    for(portID=0;portID<PORT_NUM;portID++)
    {
        totle_current += PowerRecordObj[portID].record_ValidCurrent;

        if(PowerRecordObj[portID].record_ValidCurrent > max_current)
		{
			max_current = PowerRecordObj[portID].record_ValidCurrent;
			maxID = portID;
		}	
    }
    
    if(totle_current > OverCurrentValueArr[iStandard])
    {
		confirmCount++;
    }
	else
	{
		confirmCount = 0;
	}

	//iLength = sprintf(pBuf,"MAX_CURRENT=%d\t totle_current=%d\t confirmCount=%d\n",OverCurrentValueArr[iStandard],totle_current,confirmCount);
	//UdpDebugPrint(testBuf,sizeof(testBuf));


	if(confirmCount >= CONFIRM_COUNT)
	{
		if(SwInformation.PortInfor[maxID].OnOffStatus)
		{
		    #if(PORT_NUM == 6)
		    {
	            Switch_TurnOnOff(maxID+1, 0);                                 // �����������ر�����ʵĲ�� 
	        }
	        #elif(PORT_NUM == 1)
	        {
	            Port_TurnOnOff(0);
				plug_updata = 1;

				//u_printf("rn8209.c--120\n");
	        }
	        #endif
	        SwInformation.PortInfor[maxID].OnOffStatus = OVER_CURRENT_STATUS; 
			SwInformation.portChangeReason = PORT_CHANGE_BY_OVERCURRENT;
	    }

    	UdpDebugPrint("Now excute OverCurrentOff control\r\n",sizeof("Now excute OverCurrentOff control\r\n")-1);
    	confirmCount = 0;
	}
     
}
#endif 





/*******************************************************************************
* ��������: �κ�һ���ڴﵽ13.5A���ص�������
* ��ڲ���: 
* ���ڲ���: 
* ʹ��˵��: 
*******************************************************************************/
void CurrentProtect(void)
{
	uint8_t i = 0;
	uint8_t curPortID = 10;

	for(i=0;i<PORT_NUM;i++)
	{
		if(PowerRecordObj[i].record_ValidCurrent>=13500 && PowerRecordObj[i].record_ValidCurrent<14100)
		{
			curPortID = i;
			//UdpDebugPrint("Now the port exceed the limit current!\r\n",sizeof("Now the port exceed the limit current!\r\n")-1);
			break;
		}
	}
	if(curPortID != 10)
	{
		for(i=0;i<PORT_NUM;i++)
		{
			if(i != curPortID)
			{
				SetPortState(i+1, 0);
			}
		}

		UsartControlPortOnOff(); 
	}
}




/**************************************************************************************************
* ��������: �����������ѹ�����ʵ�У׼ֵ
* ��ڲ���: 
* ���ڲ���: 
* ʹ��˵��: ����ͨ�������������ôӻ���У׼ֵ��Ҳ���Զ����ӻ���ԭʼֵ�ڱ�������У׼
***************************************************************************************************/
int HttpCalibratePowerConsume(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf) 
{

    cJSON *pJson;
    int    iValue,portID,iRslt = 1;
    //char testBuf[100] = {0};

    pJson = cJSON_Parse(pArgRecvBuf); 
    if(pJson) 
    { 
        portID = cJSON_GetObjectItem(pJson, "port")->valueint; 
        portID = portID > PORT_NUM ? PORT_NUM : portID;

       #if(PORT_NUM == 6)
        CalibrateData[0] = portID;        
        PowerRecordObj[portID].PowerCoefficent = cJSON_GetObjectItem(pJson, "watt")->valueint; 

        CalibrateData[1] = (PowerRecordObj[portID].PowerCoefficent&0xff000000)>>24;
        CalibrateData[2] = (PowerRecordObj[portID].PowerCoefficent&0x00ff0000)>>16;
        CalibrateData[3] = (PowerRecordObj[portID].PowerCoefficent&0x0000ff00)>>8;
        CalibrateData[4] = PowerRecordObj[portID].PowerCoefficent&0xff;
        
        PowerRecordObj[portID].VoltCoefficent = cJSON_GetObjectItem(pJson, "vol")->valueint; 

        CalibrateData[7] = (PowerRecordObj[portID].VoltCoefficent&0xff);
        
        PowerRecordObj[portID].CurrentCoefficent = cJSON_GetObjectItem(pJson, "amp")->valueint; 

        CalibrateData[5] = (PowerRecordObj[portID].CurrentCoefficent&0xff00)>>8;
        CalibrateData[6] = (PowerRecordObj[portID].CurrentCoefficent&0xff);
       #elif(PORT_NUM == 1)
        iValue = cJSON_GetObjectItem(pJson, "watt")->valueint;
        PowerRecordObj[0].PowerCoefficent   = reco_measure_data.reco_powerp*10 / iValue;
        iValue = cJSON_GetObjectItem(pJson, "vol")->valueint;
        PowerRecordObj[0].VoltCoefficent    = reco_measure_data.reco_urms*10 / iValue;
        iValue = cJSON_GetObjectItem(pJson, "amp")->valueint; 
        PowerRecordObj[0].CurrentCoefficent = reco_measure_data.reco_irms*10 / iValue;
       #endif

        cJSON_Delete(pJson); 

	#if 0
        sprintf(testBuf,"PowerCoefficent=%d\t VoltCoefficent=%d\t CurrentCoefficent=%d\n",PowerRecordObj[0].PowerCoefficent,
        		PowerRecordObj[0].VoltCoefficent,PowerRecordObj[0].CurrentCoefficent);
        UdpDebugPrint(testBuf, strlen(testBuf));
    #endif
        
        SavePowerCoefficent();
        UsartCalibrationPort();
        if(PowerRecordObj[portID].PowerCoefficent && PowerRecordObj[portID].VoltCoefficent && PowerRecordObj[portID].CurrentCoefficent)
        {
            iRslt = 0;
        }

        //u_printf("portID = %d\n",portID);
     } 
    return sprintf(pAckBuf, "{\"response\":%d,\"code\":%d}", iCmd, (200<<iRslt)); 
}





/*******************************************************************************
* ��������: �����ʡ���������ѹУ׼ϵ��д�뵽flash�б�������
* ��ڲ���: 
* ���ڲ���: 
* ʹ��˵��: 
*******************************************************************************/ 
#define     POWER_COEFFICENT_INFO_DATA_BUF_LENGTH       32*PORT_NUM
#if  defined(HANK_JOE_CUSTOMER_REMOTE_CTL) 
 #define     POWER_COEFFICENT_SAVE_FLAG                  0x51515151     //���ݱ����׼
#else
 #define     POWER_COEFFICENT_SAVE_FLAG                  0x5A5A5A5A 
#endif 
void SavePowerCoefficent(void)
{ 
    int      *pCoefficent;
    char     PowerCoefficentMsg[POWER_COEFFICENT_INFO_DATA_BUF_LENGTH+1]={0}; 
    uint8_t portID;
    
    pCoefficent  = (int*)PowerCoefficentMsg; 
    *pCoefficent = POWER_COEFFICENT_SAVE_FLAG; 
    for(portID=0;portID<portNum;portID++)
    {
        pCoefficent++; 
        *pCoefficent = PowerRecordObj[portID].CurrentCoefficent;
        pCoefficent++; 
        *pCoefficent = PowerRecordObj[portID].VoltCoefficent;
        pCoefficent++; 
        *pCoefficent = PowerRecordObj[portID].PowerCoefficent;
    }
    
    hfuflash_erase_page(FLASH_POWER_CALIBRATION_COEFFICIENT_BUF_START, 1); 
    hfuflash_write(FLASH_POWER_CALIBRATION_COEFFICIENT_BUF_START, 
                  PowerCoefficentMsg, POWER_COEFFICENT_INFO_DATA_BUF_LENGTH); 
} 

/*******************************************************************************
* ��������: ��flash����������ʡ���������ѹУ׼ϵ��
* ��ڲ���: 
* ���ڲ���: 
* ʹ��˵��: 
*******************************************************************************/ 
void ReadPowerCoefficent(void)
{ 
    uint8_t portID;
    
    int      *pCoefficent;
    
    char     PowerCoefficentMsg[POWER_COEFFICENT_INFO_DATA_BUF_LENGTH+1]={0}; 
    
    hfuflash_read(FLASH_POWER_CALIBRATION_COEFFICIENT_BUF_START, 
                  PowerCoefficentMsg, POWER_COEFFICENT_INFO_DATA_BUF_LENGTH); 
                  
    pCoefficent = (int*)PowerCoefficentMsg; 

    
    if(POWER_COEFFICENT_SAVE_FLAG == *pCoefficent)
    {
        for(portID=0;portID<portNum;portID++)
        {
            pCoefficent++; 
            PowerRecordObj[portID].CurrentCoefficent = *pCoefficent;
            pCoefficent++; 
            PowerRecordObj[portID].VoltCoefficent    = *pCoefficent;
            pCoefficent++; 
            PowerRecordObj[portID].PowerCoefficent   = *pCoefficent;
        }
    }
    else
    {
        for(portID=0;portID<portNum;portID++)
        {
            PowerRecordObj[portID].CurrentCoefficent = CHECK_DATA_CURR_DEFALUT;
            PowerRecordObj[portID].VoltCoefficent    = CHECK_DATA_VOLT_DEFALUT;
            PowerRecordObj[portID].PowerCoefficent   = CHECK_DATA_POWER_DEFALUT;
         }
        
       #if  defined(HANK_JOE_CUSTOMER_REMOTE_CTL) 
        SavePowerCoefficent();
       #endif 
    }
} 

/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/

