#ifndef __READ_POWER_H__
#define __READ_POWER_H__

 
#include "ConfigAll.h"
#include "rn8209.h"

#include <stdint.h>



//------------------------------------------------------------------------
//              RN8209寄存器定义 
//------------------------------------------------------------------------
// 校表参数和计量控制
#define RN8209_SYSCON           0x00
#define RN8209_EMUCON           0x01
#define RN8209_HFConst          0x02
#define RN8209_PStart           0x03
#define RN8209_GPQA             0x05
#define RN8209_PhsA             0x07
#define RN8209_PhsB             0x08
#define RN8209_QPhsCal          0x09
#define RN8209_APOSA            0x0A

#define RN8209_APOSB            0x0B
#define RN8209_RPOSA            0x0C
#define RN8209_RPOSB            0x0D

#define RN8209_IARMSOS          0x0E
#define RN8209_IBRMSOS          0x0F
#define RN8209_IBGain           0x10
// 计量参数和状态寄存器 
#define RN8209_PFCnt            0x20
#define RN8209_QFCnt            0x21
#define RN8209_IARMS            0x22
#define RN8209_IBRMS            0x23
#define RN8209_URMS             0x24
#define RN8209_UFreq            0x25
#define RN8209_PowerPA          0x26
#define RN8209_PowerPB          0x27
#define RN8209_PowerQ           0x28
#define RN8209_EnergyP          0x29
#define RN8209_EnergyP2         0x2A
#define RN8209_EnergyQ          0x2B
#define RN8209_EnergyQ2         0x2C
#define RN8209_EMUStatus        0x2D
// 中断寄存器 
#define RN8209_IE               0X40
#define RN8209_IF               0X41
#define RN8209_RIF              0X42

#define RN8209_SysStatus        0x43
#define RN8209_RData            0x44
#define RN8209_WData            0x45
#define RN8209_DeviceID         0x7F

#define RN8209_DeviceID_Length  0x03

#define RN8209_EA_REG           0xea

#define RN8209_EA_WR_EN         0xe5
#define RN8209_EA_WR_DIS_EN     0xdc
#define RN8209_EA_A_CHA         0x5a
#define RN8209_EA_B_CHA         0xa5
#define RN8209_EA_RST           0xfa

extern int delay_onTime;			//开动作时的过零检测延时参数
extern int delay_offTime;			//关动作时的过零检测延时参数
extern uint8_t plug_updata;			//有控制命令下达
extern RecoMeasurePack reco_measure_data;




extern uint8_t RN8209_Test(char *pArgBuf);
extern uint8_t RN8209_SetPwrCoffTest(char* pArgRecvBuf);
extern void RN8209_Calibrate8209PwrOffset(void);
extern void RN8209_Init(void);
void ZX_detect_control(void);
void HttpCalibratePwrOffset(int16_t iCmd, char* pArgRecvBuf, char *pAckBuf);
extern void RN8209_READ(void);
int Read_Rn8209_deviceID(void);






#endif

