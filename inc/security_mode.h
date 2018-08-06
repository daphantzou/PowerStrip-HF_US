#ifndef  __SECURITY_MODE_H__  
#define  __SECURITY_MODE_H__  

#include "hsf.h" 
#include "stdint.h" 

#include "ConfigAll.h" 



extern uint8_t securityStatus;				//�ŵ�ģʽ״̬
extern uint8_t reportSecurityInfo;			//�ϱ�����ģʽ��־λ
extern SecurityModeType SecurityMode;			//����ģʽ�ṹ��
extern SecurityModeExcuteType SecurityModeExcute;			//����ģʽִ�нṹ��



extern uint8_t ParseSecurityMode(char *pArgBuf); 
extern void DevSecurityModePoll(void);


#endif
