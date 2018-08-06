/*
********************************************************************************
* @file    : Produce_Test.c 
* @author  : 
* @version : V1.0.0 
* @date    : 2014/06/27 
* @brief   : 
********************************************************************************
*/
#include <stdio.h>
#include <string.h>

#include <hsf.h>
#include "PowerStaticsRecord.h"
#include "ConfigAll.h" 
#include "hfuart.h" 
#include "cJson.h" 
#include "DevRule.h"
#include "HttpdCallBack.h" 
#include "Rn8209.h"
#include "DevDataType.h" 
#include "Produce_Test.h"
#include "DevRule.h" 
#include "rn8209.h"
#include "DevLib.h"

#define MaxCount    3

hfuart_handle_t huart1;
UsartSendOrder orderArray[ORDER_NUM+1];       //ADC校准的命令数组标号为0，其余的标号为各自的命令号

//uint32_t            MinutePowerConsum[PORT_NUM+1];                            //分钟功耗数组


#if  defined(__POWER_STATICS_ENABLE__) 
extern uint32_t    TempPowerStaticsSum[PORT_NUM+1];
#endif 


/*******************************************************************************
* 函数功能: 
* 输入参数: 
* 输出参数: 
* 备    注: 
*******************************************************************************/
extern int USER_FUNC uart_process_callback(uint32_t event, char *pRecvData, uint32_t len,uint32_t buf_len)
{
	uint8_t i = 0;
    char *dataBuffer = pRecvData;
    uint8_t slaveDataLength = 0;
    uint8_t checkSum = 0;
    uint16_t dataSum = 0;
	UsartOrderType slaveData;

	//char testBuf[100] = {0};

//	UdpDebugPrint(pRecvData, len);    

    if(0x0F == dataBuffer[0])
    {
        slaveData.length = dataBuffer[1];
        slaveData.cmd = dataBuffer[2];
        slaveData.portID= dataBuffer[3];
        slaveDataLength = slaveData.length-3;
        
        for(i=0;i<slaveDataLength;i++)
        {
            slaveData.data[i] = *(dataBuffer+4+i);
             dataSum += *(dataBuffer+4+i);        //filter the  start/length/CMD/ID
        }
        slaveData.checkSum = *(dataBuffer+4+i);
        slaveData.lEnd = *(dataBuffer+5+i);
        slaveData.hEnd = *(dataBuffer+6+i);

        checkSum = (slaveData.cmd+slaveData.portID+dataSum+1)&0xff;
        //sprintf(testBuf,"Length=%0X,cmd=%0X,portID=%0x,data_length=%d\n",slaveData.length,slaveData.cmd,slaveData.portID,slaveDataLength);
        //UdpDebugPrint(testBuf,strlen(testBuf));
       // u_printf("The original checkSum = %0X,checkSum = %0X\n",dataBuffer[len-3],checkSum);
        

        if(checkSum == dataBuffer[len-3])
        {
            dealSlaveData(&slaveData);
            //getPortData(slaveData.data);
            //getPortState(slaveData.data);
            
        }
        else
        {
            //u_printf("checksum error!\n");
        }
    }
    else
    {
        //u_printf("start code error!\n");
    }
    
    return len;
}



/*******************************************************************************
* 函数功能: 
* 输入参数: 
* 输出参数: 
* 备    注: 
*******************************************************************************/
void uart_process_init(void)
{
    char     AtAckMsg[BOOT_INFO_DATA_BUF_LENGTH+1]={0};
    
	//huart1 = hfuart_open(0);
	
    hfat_send_cmd("AT+UART=115200,8,1,NONE,NFC\r\n", sizeof("AT+UART=115200,8,1,NONE,NFC\r\n")-1, AtAckMsg, 19);

    huart1 = hfuart_open(0);
	
	#if  0 
	if(NULL == huart1)
	{
		u_printf("uart_process init err...\r\n");
	}
	else
	{
		//u_printf("uart_process init OK...\r\n");
	}
	
	#endif 
}


/******************************************************************************* 
* 功能描述:串口设置排插端口数目
* 入口参数: protNumber:端口数目
* 出口参数: 
* 使用说明: 
* 调用方法: 
*******************************************************************************/ 
bool UsartSetPortNumber()
{ 
    uint8_t i = 0;
    char OrderData[10] = {0x0f,0x06,0x01,0x00,portNum,0,0,0,0xff,0xff};    //命令码
    uint16_t DataSum = 0;
    
    for(i=0;i<5;i++)
    {
         
         DataSum += OrderData[2+i];
    }
		
    OrderData[7] = (DataSum+1)&0xff;
    
    hfuart_send(huart1,OrderData,sizeof(OrderData),NULL);
    orderArray[1].OrderMark = 0;
    orderArray[1].ACK = 0;
    
    return true;
} 


/******************************************************************************* 
* 功能描述:通过串口询问从机收集的实时信息
* 入口参数: 
* 出口参数: 
* 使用说明: 
* 调用方法:命令发送后，从机会返回所以的端口数据，需按协议将数据解析出来
*******************************************************************************/ 
bool UsartReadPortData(void)
{ 
    char OrderData[7] = {0x0f,0x03,0x02,0x00,0x03,0xff,0xff}; 
      
       
    hfuart_send(huart1,OrderData,sizeof(OrderData),NULL);
    
    return true;
} 

/******************************************************************************* 
* 功能描述:通过串口询问从机的开关状态
* 入口参数: 
* 出口参数: 
* 使用说明: 
* 调用方法:命令发送后，从机会返回所有端口的开关状态，需按协议将数据解析出来
*******************************************************************************/ 
bool UsartReadPortstatus(void)
{ 
    //char OrderData[7] = {0x0f,0x03,0x02,0x00,0x03,0xff,0xff}; 
    char OrderData[7] = {0x0f,0x03,0x05,0x00,0x06,0xff,0xff};
       
    hfuart_send(huart1,OrderData,sizeof(OrderData),NULL);
    
    return true;
} 



/******************************************************************************* 
* 功能描述:通过串口控制排插的PIN口开关
* 入口参数:onOffData:port口开关量,bit0-bit5分别对应port0-port5口的开关状态
* 出口参数: 
* 使用说明: 
* 调用方法:命令发送后，从机会返回应答型数据
*******************************************************************************/ 
bool UsartControlPortOnOff()
{ 	
    uint8_t i = 0;
    char OrderData[8] = {0x0f,0x04,0x03,0x00,SwOnOffData,0x00,0xff,0xff};    
    uint16_t DataSum = 0;
//	char test[100];
    
    for(i=0;i<3;i++)
    {       
         DataSum += OrderData[2+i];
    }
		
    OrderData[5] = (DataSum+1)&0xff;
    hfuart_send(huart1,OrderData,sizeof(OrderData),NULL);
    orderArray[3].OrderMark = 0;
    orderArray[3].ACK = 0;

#if 0
	memset(test,0,sizeof(test));
	sprintf(test,"\nSwOnOffData=%d\n",SwOnOffData);
	UdpDebugPrint(test,sizeof(test));
#endif
	
    return true;
} 


/******************************************************************************* 
* 功能描述:通过串口控制PIN口LED的闪烁
* 入口参数:
* 出口参数: 
* 使用说明: 
* 调用方法:命令发送后，从机会返回应答型数据
*******************************************************************************/ 
bool UsartControlPortLed()
{ 
    uint8_t i = 0;
    char OrderData[13] = {0x0f,0x09,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff};    
    uint16_t DataSum = 0;
    
    for(i=0;i<6;i++)        //计算LED的数据
    {
         OrderData[4+i] = *(LedFlashControl+i);
         DataSum += OrderData[2+i];
    }
    DataSum = DataSum + OrderData[8]+ OrderData[9];
    OrderData[10] = (DataSum+1)&0xff;

    hfuart_send(huart1,OrderData,sizeof(OrderData),NULL);
    orderArray[4].OrderMark = 0;
    orderArray[4].ACK = 0;
    
    return true;
} 


/******************************************************************************* 
* 功能描述:通过串口控制PIN口ADC校准
* 入口参数:
* 出口参数: 
* 使用说明: 
* 调用方法:命令发送后，从机会返回所以的端口数据，需按协议将数据解析出来
*******************************************************************************/ 
extern uint8_t CalibrateData[8];
bool UsartCalibrationPort(void)
{ 
    uint8_t i = 0;
    char OrderData[15] = {0x0f,0x0B,0xAA,0x00,0x00,0x00,0x01,0x86,0xA0,0x01,0xF4,0xDC,0xA2,0xFF,0xFF};  
    uint16_t DataSum = 0;
    
    for(i=0;i<8;i++)        //计算LED的数据
    {
         OrderData[4+i] = *(CalibrateData+i);
         DataSum += OrderData[2+i];
    }
    DataSum = DataSum + OrderData[10]+ OrderData[11];
    OrderData[12] = (DataSum+1)&0xff;
    
    hfuart_send(huart1,OrderData,sizeof(OrderData),NULL);
    orderArray[0].OrderMark = 0;
    orderArray[0].ACK = 0;
    return true;
} 



/*******************************************************************************
* 函数功能: 分析处理从串口接收到的从机数据
* 输入参数: recvData:解码出来的从机反馈数据
* 输出参数: 
* 备    注: 
*******************************************************************************/
void dealSlaveData(UsartOrderType *slaveRecvData)
{
    if(0xAA == (int)slaveRecvData->cmd)
    {
        slaveRecvData->cmd = 0x00;
    }
    
    switch (slaveRecvData->cmd)
    {
        case 0x00:
        case 0x01:
        case 0x03:
        case 0x04:
        {
            //dealReplyACK(slaveRecvData);  
            break;
        }

        case 0x02:
        {
        	//UdpDebugPrint("Now we get the switch info!!!",strlen("Now we get the switch info!!!"));
            getPortData(slaveRecvData->data);             //提取出每个port的电压，电流，功率
            break;
        }
        case 0x05:
        {
            getPortState(slaveRecvData->data);             //提取出每个port的开关状态
            SendAckToSlave();                              //给下位机发送应答码
            break;
        }
        default:
        {
            break;
        }
    }
}


/*******************************************************************************
* 函数功能: 串口重新下发命令
* 输入参数: recvData:解码出从机反馈数据
* 输出参数: 
* 备    注: 
*******************************************************************************/
void OrderResend(char cmd)
{

    if(0xAA == (int)cmd)
    {
        cmd = 0x00;
    }
    
    switch(cmd)
    {
        case 0x00:
        {
           UsartCalibrationPort();
           break;
        }
        case 0x01:
        {
            UsartSetPortNumber();
            break;
        }
        case 0x03:
        {
            UsartControlPortOnOff();
            break;
        }
        case 0x04:
        {
            UsartControlPortLed();
            break;
        }
        default:
        {
            break;
        }
    }
}



/*******************************************************************************
* 函数功能: 处理应答型数据
* 输入参数: recvData:解码出从机反馈数据
* 输出参数: 
* 备    注: 
*******************************************************************************/
void dealReplyACK(UsartOrderType *slaveRecvData)
{

    if(0x00 == slaveRecvData->data[0])
    {
        orderArray[slaveRecvData->cmd].ACK = 1;        //the order excute ok
        orderArray[slaveRecvData->cmd].OrderMark = 0;
        orderArray[slaveRecvData->cmd].sendCount = 0;
        //u_printf("order excute succeed!\n");
    }
    else
    {
        orderArray[slaveRecvData->cmd].ACK = 0;        //the order excute failed,resend the order
        //u_printf("order excute failed! order[%d].sendCount = %d\n",slaveRecvData->cmd,orderArray[slaveRecvData->cmd].sendCount); 
    }
}



/*******************************************************************************
* 函数功能: 检查是否有控制命令下发
* 输入参数: 
* 输出参数: 
* 备    注: return:true,有控制命令下发      false:无控制命令下发
*******************************************************************************/
bool IfOrderControl()
{

    if(orderArray[0].OrderMark || orderArray[1].OrderMark || orderArray[3].OrderMark || orderArray[4].OrderMark)
    {
        return true;
    }
    else
    {
        return false;
    }
    
}

#if 0
/*******************************************************************************
* 函数功能: 检查命令执行是否成功
* 输入参数:
* 输出参数: 
* 备    注: 
*******************************************************************************/
void IsOrderExecuteOK()
{
    uint8_t orderID = 0;
    for(orderID=0;orderID<ORDER_NUM;orderID++)
    {
        if(orderArray[orderID].OrderMark == 1)
        {
            OrderResend(orderID);       //重发命令     
            orderArray[orderID].sendCount++;
        }

        if(orderArray[orderID].sendCount >= MaxCount)
        {
            orderArray[PORT_NUM].ACK = 1;
            orderArray[orderID].OrderMark = 0;
            orderArray[orderID].sendCount = 0;
        }
     }
}
#endif


/*******************************************************************************
* 函数功能: 提取port的个数
* 输入参数: recvData:解码出从机反馈数据
* 输出参数: 
* 备    注: 
*******************************************************************************/
char getPortNumber(char *data)
{
    return (portNum=data[0]);
}


/*******************************************************************************
* 函数功能: 提取port的电压、电流和功率
* 输入参数: recvData:解码出从机反馈数据
* 输出参数: 
* 备    注: 
*******************************************************************************/
void getPortData(char *data)
{
    uint8_t i = 0;

	//char testBuffer[100];

    
    for(i=0;i<portNum;i++)
    {   
        PowerRecordObj[i].record_ValidVolt = data[0];
        PowerRecordObj[i].record_ValidFreq = data[1];
        PowerRecordObj[i].record_ValidPower = (data[2+6*i]<<24)+(data[3+6*i]<<16)+(data[4+6*i]<<8)+(data[5+6*i]<<0);
	
		if((500>PowerRecordObj[i].record_ValidPower) || (SwInformation.PortInfor[i].OnOffStatus==0)) 
		{
		    PowerRecordObj[i].record_ValidPower = 0;
		} 

        PowerRecordObj[i].record_ValidCurrent = (data[6+6*i]<<8)+(data[7+6*i]<<0);

        TempPowerStaticsSum[i] += ((PowerRecordObj[i].record_ValidPower+5)/10);        //每2s读取一次数据
        
        //MinutePowerConsum[i] += PowerRecordObj[i].record_ValidPower*2;        //每2s读取一次数据
        
 //       UdpDebugPrint("we've got the power!\n",sizeof("we've got the power!\n")-1);   
#if 0
		if(readPowerCount%5 == 0)
		{
			memset(testBuffer,0,sizeof(testBuffer));
	 		sprintf(testBuffer,"port[%d]:power=%d\t	sumPower=%d\t readPowerCount=%d\n",i,
	 				PowerRecordObj[i].record_ValidPower,TempPowerStaticsSum[i],readPowerCount);
			UdpDebugPrint(testBuffer,strlen(testBuffer));
		}
#endif
    }
	readPowerCount++; 
	
}

/*******************************************************************************
* 函数功能: 读取port开关状态
* 输入参数: recvData:解码出从机反馈数据
* 输出参数: 
* 备    注: 
*******************************************************************************/
void getPortState(char *Data)
{   
     uint8_t portID = 0;
     char OnOffData = 0;
     
     OnOffData=*Data; 
	 
     for(portID = 0;portID <PORT_NUM;portID++)
     { 
		if(SwInformation.PortInfor[portID].OnOffStatus)
        {
            SwOnOffData |= SwInformation.PortInfor[portID].OnOffStatus<<portID;
        }
        else
        {
            SwOnOffData &= ~((!SwInformation.PortInfor[portID].OnOffStatus)<<portID);
        }
		
        SwInformation.PortInfor[portID].OnOffStatus = ((OnOffData&(1<<portID))>>portID); 
        //u_printf("port[%d] status = %d\n",portID,SwInformation.PortInfor[portID].OnOffStatus);
     }

	 if(OnOffData != SwOnOffData)
	 {
	 	SwInformation.portChangeReason = PORT_CHANGE_BY_KEY;
	 }
}

/*******************************************************************************
* 函数功能: 主机下发应答码给从机
* 输入参数: recvData:解码出从机反馈数据
* 输出参数: 
* 备    注: 
*******************************************************************************/
bool SendAckToSlave(void)
{     
    char AckData[8] = {0x0f,0x04,0x05,0x00,0x00,0x0A,0xFF,0xFF}; 
    hfuart_send(huart1,AckData,sizeof(AckData),NULL);
    return true;
}


#if 0
void printRcvData(UsartOrderType recvData)
{
    uint8_t i =0;
    
    u_printf("The receive data:\r\n");
    u_printf("Length:%d\t CMD:%d\t ID:%d\t checkSum:%d\r\n",recvData.Length,recvData.CMD,recvData.ID,recvData.CheckSum);
    
    for(i=0;i<recvData.Length-4;i++)
    {
        u_printf("recvData.Data[%d]:\t",i,recvData.Data[i]);
        if(0 == i%5)
        {
            u_printf("\n");
        }
    }
}
#endif
/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/
