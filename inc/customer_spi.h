/*
********************************************************************************
* @file    : customer_spi.h 
* @author  : 
* @version : V1.0.0 
* @date    : 2014/06/27 
* @brief   : 
********************************************************************************
*/
#ifndef __CUSTOMER_SPI_H__
#define __CUSTOMER_SPI_H__
// 
#include "ConfigAll.h"
// 
typedef struct
{
	unsigned char spi_Address;
   	unsigned char *spi_Data;
   	unsigned char spi_NumberOfByte;
}SpiPack;

// 
void SpiRn8209_Init(void);
void SpiRn8209_ReadFrame(SpiPack *SpiData);
void SpiRn8209_WriteFrame(SpiPack *SpiData);

#endif /* __CUSTOMER_SPI_H__ */
/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/

