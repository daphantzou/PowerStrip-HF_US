/*
********************************************************************************
* @file    : LED.c
* @author  :
* @version : V1.0.0
* @date    : 03/20/2013
* @brief   : 
********************************************************************************
*/ 
#include <stdint.h> 
#include <stdio.h> 

#include "ConfigAll.h" 
#include "hsf.h"

#include "DevDataType.h"
#include "DevInfo.h" 
#include "DevLib.h" 
#include "LED.h" 

// 
/*******************************************************************************
* ��������: 
* ��ڲ���: 
* ���ڲ���: 
* ʹ��˵��: 
*******************************************************************************/ 
extern void LedCtl(char ArgOnOff) 
{ 
    if(ArgOnOff)
    {
        hfgpio_fset_out_low(HFGPIO_SW_LED);
    }
    else
    {
        hfgpio_fset_out_high(HFGPIO_SW_LED);
    }
} 
/*******************************************************************************
* ��������: 
* ��ڲ���: 
* ���ڲ���: 
* ʹ��˵��: 
*******************************************************************************/ 
extern void LedBlink(int Time) 
{ 
    static int iCount = 0, isOn;

    iCount ++;
    if(iCount > Time)
    {
        iCount = 0; 
        if(isOn)
        {
            LedCtl(WIFI_LED_ON);
            isOn = 0;
        }
        else
        {
            LedCtl(WIFI_LED_OFF);
            isOn = 1;
        }
    }
} 

/******************************************************************************* 
* ========================= END OF PAGE 
*******************************************************************************/ 
