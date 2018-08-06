/*
********************************************************************************
* @file    Produce_Test.h
* @author  
* @version V1.0.0
* @date    2014/06/24
* @brief   
********************************************************************************
*/
#ifndef  __PRODUCE_TEST_APP_H__ 
#define  __PRODUCE_TEST_APP_H__ 
// 
#include <hsf.h> 
#include "DevDataType.h" 

// 
extern hfuart_handle_t huart1;
extern UsartSendOrder orderArray[ORDER_NUM+1];       //ADC校准的命令数组标号为0，其余的标号为各自的命令号


extern void uart_process_init(void);

bool UsartSetPortNumber(void);
bool UsartReadPortData(void);
bool UsartReadPortstatus(void);

bool UsartControlPortOnOff(void);

bool UsartControlPortLed(void);
bool UsartCalibrationPort(void);
void OrderResend(char cmd);





extern int USER_FUNC uart_process_callback(uint32_t event, char *pRecvData, uint32_t len,uint32_t buf_len);
extern void dealSlaveData(UsartOrderType *slaveRecvData);
void dealReplyACK(UsartOrderType *slaveRecvData);
char getPortNumber(char *data);
void getPortData(char *data);
void getPortState(char *data);
bool SendAckToSlave(void);
bool IfOrderControl(void);
void IsOrderExecuteOK(void);






//void printRcvData(UsartOrderType recvData);

#endif


//

/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/

