#include <stdio.h>
#include <string.h>
#include <hsf.h>
#include "ConfigAll.h"
#include "customer_spi.h"
#include "read_power.h"
#include "rn8209.h"
#include "DevLib.h"
#include "PowerStaticsRecord.h"


RecoMeasurePack reco_measure_data; 
uint8_t plug_updata = 0;			//有控制命令下达
uint8_t giveDelayValue = 0;			//是否给延时时间赋值
/***********************60HZ******************************/
int delay_onTime = 11000;
int delay_offTime = 5000;

extern uint32_t            TempPowerStaticsSum[PORT_NUM+1];                          //小时功耗保存数组
//extern uint32_t            MinutePowerConsum[PORT_NUM+1];                            //分钟功耗数组

/*******************************************************************************
* 函数功能: 
* 输入参数: 
* 输出参数: 
* 备    注: 
*******************************************************************************/
#if (defined(__CARIBRATION_POWER_COEFFICIENT_DBG__) || defined(__RN8209_SPI_DBG__)) && defined(__UDP_PRINT_DBG__)
#include "UdpServerCallBack.h" 
uint8_t     isCalibratePwrOffSet = 0x00;
uint8_t     iCountCalibrateOffset= 0x00;
uint32_t    iCalibrateOffsetSum= 0x00;
uint16_t    iAPOSA_Value = 0x00;

extern uint8_t RN8209_Test(char *pArgBuf)
{
    char PowerDebugMsg[200] = {0};
    int  iLength; 
    
	//Read_Rn8209_deviceID();
    iLength = sprintf(PowerDebugMsg, "Power Init and Coefficient:"
                                     "Pi=%d Co=%d,\r\nVi=%d Vc=%d,\r\nAi=%d Ac=%d\r\n", 
                                     reco_measure_data.reco_powerp,
                                     PowerRecordObj.PowerCoefficent,
                                     reco_measure_data.reco_urms,
                                     PowerRecordObj.VoltCoefficent,
                                     reco_measure_data.reco_irms,
                                     PowerRecordObj.CurrentCoefficent
                                     );
    UdpDebugPrint(PowerDebugMsg, iLength); 
    // 
    iLength = sprintf(PowerDebugMsg, "\r\nAPOSA Coefficient:Dec=%d,Hex=0x%x, CalibrateSum:Dec=%d,Hex=0x%x\r\n", 
                                        iAPOSA_Value, iAPOSA_Value, 
                                        iCalibrateOffsetSum, iCalibrateOffsetSum);
    UdpDebugPrint(PowerDebugMsg, iLength); 
	return 0;
}


extern uint8_t RN8209_SetPwrCoffTest(char* pArgRecvBuf) 
{
    cJSON  *pJson;
    int     iValue, iRslt = 1, tmp;
    char    AckMsgArray[40]={0};
    
	SpiPack     R8209Framemain; 
    
    pJson = cJSON_Parse(pArgRecvBuf); 
    if(pJson) 
    { 
        iValue = cJSON_GetObjectItem(pJson, "aposa")->valueint; 
        
        iAPOSA_Value = (uint16_t)iValue; 
        
        iValue = cJSON_GetObjectItem(pJson, "isAuto")->valueint; 
        
        iCalibrateOffsetSum = iAPOSA_Value; 
        cJSON_Delete(pJson); 

        // 
    	tmp = RN8209_EA_WR_EN;
        R8209Framemain.spi_Address      = RN8209_EA_REG;
        R8209Framemain.spi_Data         = (unsigned char *)&tmp;
        R8209Framemain.spi_NumberOfByte = 0x01;
        SpiRn8209_WriteFrame(&R8209Framemain);
        hf_thread_delay(100);
        R8209Framemain.spi_Address      = RN8209_APOSA;
        R8209Framemain.spi_Data         = (unsigned char *)&iAPOSA_Value;
        R8209Framemain.spi_NumberOfByte = 0x02;
        SpiRn8209_WriteFrame(&R8209Framemain);
        // 
        tmp = RN8209_EA_WR_DIS_EN;
        R8209Framemain.spi_Address      = RN8209_EA_REG;
        R8209Framemain.spi_Data         = (unsigned char *)&tmp;
        R8209Framemain.spi_NumberOfByte = 0x01;
        SpiRn8209_WriteFrame(&R8209Framemain);
        // 
        SavePowerCoefficent(); 
        
        #if  defined(__AUTO_SET_APOSA__) 
        if(iValue) 
        {
            isCalibratePwrOffSet++;
            iCountCalibrateOffset=20;
            iCalibrateOffsetSum = 0x00;
        }
        #endif 
        iRslt = 0;
    } 
    iValue = sprintf(AckMsgArray, "{\"response\":0XCD,\"code\":%d}", (200<<iRslt)); 
    SendUdpData(AckMsgArray, iValue);
	return 0;
}


extern void RN8209_Calibrate8209PwrOffset(void) 
{
	SpiPack     R8209Framemain; 
	uint32_t    varPower; 
	uint16_t    tmp; 
	
    if(0x00 == isCalibratePwrOffSet)
    {
        return; 
    }
    while(iCountCalibrateOffset)
    {
        iCountCalibrateOffset--;
    	// 读有功功率 
    	varPower = 0;
    	R8209Framemain.spi_Address      = RN8209_PowerPA;
    	R8209Framemain.spi_Data         = (unsigned char *)&varPower;
    	R8209Framemain.spi_NumberOfByte = 0x04;
    	SpiRn8209_ReadFrame(&R8209Framemain);
    	iCalibrateOffsetSum += varPower; 
    	
    	if(0x00 == iCountCalibrateOffset) 
    	{
    	    isCalibratePwrOffSet            = 0x00; 
    	    iCalibrateOffsetSum            ^= 0xFFFFFFFF;
    	    iCalibrateOffsetSum++;
    	    // 
    	    tmp = RN8209_EA_WR_EN;
        	R8209Framemain.spi_Address      = RN8209_EA_REG;
        	R8209Framemain.spi_Data         = (unsigned char *)&tmp;
        	R8209Framemain.spi_NumberOfByte = 0x01;
        	SpiRn8209_WriteFrame(&R8209Framemain);
        	hf_thread_delay(100);
            // 写有功功率校正 
            iAPOSA_Value = (uint16_t)iCalibrateOffsetSum; 
        
            tmp = (uint16_t)iCalibrateOffsetSum;
            R8209Framemain.spi_Address      = RN8209_APOSA;
            R8209Framemain.spi_Data         = (unsigned char *)&tmp;
            R8209Framemain.spi_NumberOfByte = 0x02;
            SpiRn8209_WriteFrame(&R8209Framemain);
            // 
        	tmp = RN8209_EA_WR_DIS_EN;
        	R8209Framemain.spi_Address      = RN8209_EA_REG;
        	R8209Framemain.spi_Data         = (unsigned char *)&tmp;
        	R8209Framemain.spi_NumberOfByte = 0x01;
        	SpiRn8209_WriteFrame(&R8209Framemain);
            // 
            SavePowerCoefficent();
            
            UdpDebugPrint("Set Pwr Offset OK \r\n", sizeof("Set Pwr Offset OK \r\n")-1); 
    	}
    	break;
    }
}
#endif 



/*******************************************************************************
* 函数功能: 
* 输入参数: 
* 输出参数: 
* 备    注: 
*******************************************************************************/
extern void RN8209_Init(void)
{
	uint16_t data;
	int     tmp;
	SpiPack R8209Framemain; 

	tmp = RN8209_EA_WR_EN; 
	R8209Framemain.spi_Address      = RN8209_EA_REG; 
	R8209Framemain.spi_Data         = (unsigned char *)&tmp; 
	R8209Framemain.spi_NumberOfByte = 0x01; 
	SpiRn8209_WriteFrame(&R8209Framemain); 

    data = 0x0073;
	R8209Framemain.spi_Address      = 0x00;
	R8209Framemain.spi_Data         = (unsigned char *)&data;
	R8209Framemain.spi_NumberOfByte = 0x02;
	SpiRn8209_WriteFrame(&R8209Framemain);
	
	
	//data[0] = 0x02;
	//data[1] = 0x83; 				//引脚IRQ_N/ZX作为ZX脚
	data = 0x0283;
	R8209Framemain.spi_Address      = 0x01;
	R8209Framemain.spi_Data         = (unsigned char *)&data;
	R8209Framemain.spi_NumberOfByte = 0x02;
	SpiRn8209_WriteFrame(&R8209Framemain);


	tmp = RN8209_EA_WR_DIS_EN; 
	R8209Framemain.spi_Address      = RN8209_EA_REG;
	R8209Framemain.spi_Data         = (unsigned char *)&tmp;
	R8209Framemain.spi_NumberOfByte = 0x01;
	SpiRn8209_WriteFrame(&R8209Framemain);
}


#if defined(ZX_DETECT)
extern int SwitchPinArr[PORT_NUM];
extern uint8_t zxMark;
void ZX_detect_control(void)
{
	hfthread_suspend_all();

	if(plug_updata)
    {			

		if(SwInformation.PortInfor[0].CurStatus)
        {
			delay_us(delay_onTime);
            hfgpio_fset_out_high(SwitchPinArr[0]);               // On          
        }
        else
        {
			delay_us(delay_offTime);
            hfgpio_fset_out_low(SwitchPinArr[0]);                // Off 
        }
		
		plug_updata = 0;

    }
	hfthread_resume_all();

}
#endif



/*******************************************************************************
* 功能描述: 
* 入口参数: 
* 出口参数: 
* 使用说明: 
*******************************************************************************/
#if defined(__PWR_OFFSET_DBG__) && defined(__UDP_PRINT_DBG__) 
#define     CALI_AVERAGE_COUNT       4 
void HttpCalibratePwrOffset(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf) 
{
	SpiPack R8209Framemain;
	int     tmp, tmp1; 
	int     varCurrent = 0, varPower; 
	
	#if defined(__PWR_OFFSET_DBG__) && defined(__UDP_PRINT_DBG__) 
	#define DBG_LENGTH  100 
	int     iLength;
	char    DbgMsgArr[DBG_LENGTH]={0}; 
	#endif 
	int     i, iFlag, iCount, iSum = 0x95aa55aa;
	
	iFlag = 0;
    // 读有功功率 
    iCount = CALI_AVERAGE_COUNT; 
    i = 0;
    while(iCount) 
    { 
        iCount --; 
        varPower = 0; 
        R8209Framemain.spi_Address      = RN8209_PowerPA; 
        R8209Framemain.spi_Data         = (unsigned char *)&varPower; 
        R8209Framemain.spi_NumberOfByte = 0x04; 
        SpiRn8209_ReadFrame(&R8209Framemain); 
        #if  defined(__PWR_OFFSET_CALI__)
        if(varPower & (0x80000000))
        { 
            varPower = 0; 
            continue; 
        } 
        i++; 
        //iSum = (varPower>iSum) ? iSum : varPower; 
        #endif 
        iSum += varPower; 
    	hf_thread_delay(100); 
    } 
    if(i) 
    { 
        varPower = iSum/CALI_AVERAGE_COUNT; 
        //varPower = iSum; 
    } 
    else 
    { 
        varPower = iSum; 
        iFlag = 1; 
    } 
    
    // 读电流有效值 
    iCount = CALI_AVERAGE_COUNT; 
    iSum   = 0; 
    while(iCount) 
    { 
        iCount --; 
        varCurrent = 0;
        R8209Framemain.spi_Address      = RN8209_IARMS;
        R8209Framemain.spi_Data         = (unsigned char *)&varCurrent;
        R8209Framemain.spi_NumberOfByte = 0x04;
        SpiRn8209_ReadFrame(&R8209Framemain);
        iSum += varCurrent; 
    	hf_thread_delay(100); 
    } 
    varCurrent = iSum/CALI_AVERAGE_COUNT; 
    varCurrent = varCurrent*varCurrent; 
    
	// 使能写操作 
	tmp = RN8209_EA_WR_EN; 
	R8209Framemain.spi_Address      = RN8209_EA_REG; 
	R8209Framemain.spi_Data         = (unsigned char *)&tmp; 
	R8209Framemain.spi_NumberOfByte = 0x01; 
	SpiRn8209_WriteFrame(&R8209Framemain); 
	
	hf_thread_delay(100);
	
	// 写有功功率校正 
	tmp = (~varPower)+1; 
	R8209Framemain.spi_Address      = RN8209_APOSA; 
	R8209Framemain.spi_Data         = (unsigned char *)&tmp; 
	R8209Framemain.spi_NumberOfByte = 0x02; 
	SpiRn8209_WriteFrame(&R8209Framemain); 
	
	hf_thread_delay(100);
	
	// 写电流校正 
	tmp1 = (~varCurrent)+1; 
	R8209Framemain.spi_Address      = RN8209_IARMSOS; 
	R8209Framemain.spi_Data         = (unsigned char *)&tmp1; 
	R8209Framemain.spi_NumberOfByte = 0x02; 
	SpiRn8209_WriteFrame(&R8209Framemain); 
	
	hf_thread_delay(100); 
	
   #if defined(__PWR_OFFSET_DBG__) && defined(__UDP_PRINT_DBG__) 
    iLength = snprintf(DbgMsgArr, DBG_LENGTH, "Calibrate PwrOffset=%08x, tmp=%08x, Curennt=%08x, tmp1=%08x\r\n", 
                                                varPower, tmp, varCurrent, tmp1); 
    UdpDebugPrint(DbgMsgArr, iLength); 
   #endif 
    
	// 关闭写操作 
	tmp = RN8209_EA_WR_DIS_EN; 
	R8209Framemain.spi_Address      = RN8209_EA_REG;
	R8209Framemain.spi_Data         = (unsigned char *)&tmp;
	R8209Framemain.spi_NumberOfByte = 0x01;
	SpiRn8209_WriteFrame(&R8209Framemain);
	
	hf_thread_delay(100);
	
   #if defined(__PWR_OFFSET_DBG__) && defined(__UDP_PRINT_DBG__) 
    varPower = 0;
    R8209Framemain.spi_Address      = RN8209_APOSA;
    R8209Framemain.spi_Data         = (unsigned char *)&varPower;
    R8209Framemain.spi_NumberOfByte = 0x02;
    SpiRn8209_ReadFrame(&R8209Framemain);
    
    varCurrent = 0;
    R8209Framemain.spi_Address      = RN8209_IARMSOS;
    R8209Framemain.spi_Data         = (unsigned char *)&varCurrent;
    R8209Framemain.spi_NumberOfByte = 0x02;
    SpiRn8209_ReadFrame(&R8209Framemain);
    
    iLength = snprintf(DbgMsgArr, DBG_LENGTH, "Read PwrOffset=%08x, Curennt=%08x\r\n", varPower, varCurrent); 
    UdpDebugPrint(DbgMsgArr, iLength); 
   #endif 
    sprintf(pAckBuf, "{\"response\":%d,\"code\":%d}", iCmd, (200<<iFlag)); 
}
#endif 



/*******************************************************************************
* 函数功能: 
* 输入参数: 
* 输出参数: 
* 备    注: 
*******************************************************************************/
extern void RN8209_READ(void)
{
	SpiPack R8209Framemain;
	int     varCurrent = 0, varVolt, varPower,varFreq; 
	//char testBuffer[100] = {0};
	
	#if defined(__AP_PWR_DBG__)
	static  int iRd8209GDbg=0;
	#define DBG_LENGTH  40 
	int     iLength;
	char    DbgMsgArr[DBG_LENGTH]={0};
	#endif 

	// 读电流有效值 
	varCurrent = 0;
	R8209Framemain.spi_Address      = RN8209_IARMS;
	R8209Framemain.spi_Data         = (unsigned char *)&varCurrent;
	R8209Framemain.spi_NumberOfByte = 0x03;
	SpiRn8209_ReadFrame(&R8209Framemain);

   #if defined(__AP_PWR_DBG__)
	iRd8209GDbg++;
	if(0x00 ==(iRd8209GDbg%10))
	{
        iLength = snprintf(DbgMsgArr, DBG_LENGTH, "A=%d Cal_A=%d\r\n", varCurrent,PowerRecordObj[0].CurrentCoefficent);
        UdpDebugPrint(DbgMsgArr, iLength); 
	}
   #endif 
   
	if(varCurrent&(0x800000))
	{
		varCurrent = 0;
	}
	reco_measure_data.reco_irms = varCurrent;
	PowerRecordObj[0].record_ValidCurrent  = (reco_measure_data.reco_irms*10)/(PowerRecordObj[0].CurrentCoefficent); 
 
	// 读电压有效值 
	varVolt = 0;
	R8209Framemain.spi_Address      = RN8209_URMS;
	R8209Framemain.spi_Data         = (unsigned char *)&varVolt;
	R8209Framemain.spi_NumberOfByte = 0x03;
	SpiRn8209_ReadFrame(&R8209Framemain);
	
   #if defined(__AP_PWR_DBG__)
	if(0x00 ==(iRd8209GDbg%10))
	{
        iLength = snprintf(DbgMsgArr, DBG_LENGTH, "V=%d Cal_V=%d\r\n", varVolt,PowerRecordObj[0].VoltCoefficent);
        UdpDebugPrint(DbgMsgArr, iLength); 
	}
   #endif 
   
	if(varVolt&(0x800000))
	{
		varVolt = 0;
	}
	reco_measure_data.reco_urms = varVolt;
	PowerRecordObj[0].record_ValidVolt  = (reco_measure_data.reco_urms*10)/(PowerRecordObj[0].VoltCoefficent); 

	
	// 读电源频率有效值 
	varFreq = 0;
	R8209Framemain.spi_Address		= RN8209_UFreq;
	R8209Framemain.spi_Data 		= (unsigned char *)&varFreq;
	R8209Framemain.spi_NumberOfByte = 0x02;
	SpiRn8209_ReadFrame(&R8209Framemain);
   
	if(varFreq&(0x8000))
	{
		varFreq = 0;
	}
	reco_measure_data.reco_freq = varFreq;

#if 0
	iRd8209GDbg++;
	if(0x00 ==(iRd8209GDbg%10))
	{
        iLength = snprintf(DbgMsgArr, DBG_LENGTH, "Frequency = %d\r\n", reco_measure_data.reco_freq);
        UdpDebugPrint(DbgMsgArr, iLength); 
	}
#endif

	// 读有功功率 
	varPower = 0;
	R8209Framemain.spi_Address      = RN8209_PowerPA;
	R8209Framemain.spi_Data         = (unsigned char *)&varPower;
	R8209Framemain.spi_NumberOfByte = 0x04;
	SpiRn8209_ReadFrame(&R8209Framemain);
	
	if(varPower&0x80000000)
	{
		varPower = 0;
	}
   #if defined(__AP_PWR_DBG__)       
	if(0x00 ==(iRd8209GDbg%10))
	{
        iLength = snprintf(DbgMsgArr, DBG_LENGTH, "P=%d Cal_P=%d\r\n", varPower,PowerRecordObj[0].PowerCoefficent);
        UdpDebugPrint(DbgMsgArr, iLength); 
	}
   #endif 
   
	reco_measure_data.reco_powerp = varPower;
	PowerRecordObj[0].record_ValidPower = (reco_measure_data.reco_powerp*10)/(PowerRecordObj[0].PowerCoefficent); 

#if  defined(__DEL_BELOW_05_W__)
	if(500 > PowerRecordObj[0].record_ValidPower) 
	{
	    PowerRecordObj[0].record_ValidPower = 0;
	}
#endif 

	if(0x00 == SwInformation.PortInfor[0].CurStatus)
	{
	    PowerRecordObj[0].record_ValidCurrent = 0; 
	    PowerRecordObj[0].record_ValidVolt    = 0; 
	    PowerRecordObj[0].record_ValidPower   = 0; 

	    reco_measure_data.reco_irms        = 0;
	    reco_measure_data.reco_urms        = 0;
	    reco_measure_data.reco_powerp      = 0;
	}

	if(PowerRecordObj[0].record_ValidPower < 4000000)
	{
		TempPowerStaticsSum[0]             += PowerRecordObj[0].record_ValidPower/10; 
		readPowerCount++;
	}

#if 0	
	if(readPowerCount%5 == 0)
	{
		memset(testBuffer,0,sizeof(testBuffer));
 		sprintf(testBuffer,"power=%d\t	sumPower=%d\t readPowerCount=%d\n",
 				PowerRecordObj[0].record_ValidPower,TempPowerStaticsSum[0],readPowerCount);
		UdpDebugPrint(testBuffer,strlen(testBuffer));
	}
 #endif
}


/*******************************************************************************
* 函数功能: 
* 输入参数: 
* 输出参数: 
* 备    注: 
*******************************************************************************/
int Read_Rn8209_deviceID(void)
{
	SpiPack         R8209Framemain;
	unsigned char   recvdata[10] = {0};
	
    #if (defined(__CARIBRATION_POWER_COEFFICIENT_DBG__) || defined(__RN8209_SPI_DBG__)) && defined(__UDP_PRINT_DBG__)
	int             *pS32Buf;
    int             iLength;
    char            DbgMsgArr[60] = {0};
    #endif 
	
	R8209Framemain.spi_Address      = RN8209_DeviceID;                 // deviceID
	R8209Framemain.spi_Data         = (unsigned char *)&recvdata;
	R8209Framemain.spi_NumberOfByte = RN8209_DeviceID_Length;
	SpiRn8209_ReadFrame(&R8209Framemain);

    #if (defined(__CARIBRATION_POWER_COEFFICIENT_DBG__) || defined(__RN8209_SPI_DBG__)) && defined(__UDP_PRINT_DBG__)
    pS32Buf = (int*)&recvdata[0];
    iLength = sprintf(DbgMsgArr,"8209G-DeviceID:[%02x][%02x][%02x]--%08x\r\n",
                                recvdata[2], recvdata[1], recvdata[0], *pS32Buf);
    UdpDebugPrint(DbgMsgArr, iLength); 
    #endif 
	if((0x82==recvdata[2]) && (0x09==recvdata[1]) && (0x00==recvdata[0]))
	{
	    return 0;
	}
	return 1;
}
/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/



