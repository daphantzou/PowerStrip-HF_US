#ifndef  __SECURITY_MODE_H__  
#define  __SECURITY_MODE_H__  

#include "hsf.h" 
#include "stdint.h" 

#include "ConfigAll.h" 



extern uint8_t securityStatus;				//放到模式状态
extern uint8_t reportSecurityInfo;			//上报防盗模式标志位
extern SecurityModeType SecurityMode;			//防盗模式结构体
extern SecurityModeExcuteType SecurityModeExcute;			//防盗模式执行结构体



extern uint8_t ParseSecurityMode(char *pArgBuf); 
extern void DevSecurityModePoll(void);


#endif
