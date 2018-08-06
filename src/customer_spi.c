/*
********************************************************************************
* @file    : customer_spi.c 
* @author  : 
* @version : V1.0.0 
* @date    : 2014/06/27 
* @brief   : 
********************************************************************************
*/
#include <string.h>
#include <hsf.h>

#include "ConfigAll.h"
#include "customer_spi.h"
// 
#define     SP_WR_BIT_FLAG              0x80 
// 
static void SpiRn8209WriteByte(unsigned char rn8205spi_writeData);
static void SpiRn8209ReadByte(unsigned char *rn8205spi_readData);
/*******************************************************************************
* 函数功能: 
* 输入参数: 
* 输出参数: 
* 备    注: 
*******************************************************************************/
void SpiRn8209_Init(void)
{
	if(hfspi_master_init(HFSPI_MODE_CPOL1_CPHA0, HFSPIM_CLK_DIV_325K) != HF_SUCCESS)
	//if(hfspi_master_init(HFSPI_MODE_CPOL1_CPHA0, HFSPIM_CLK_DIV_750K) != HF_SUCCESS)
	//if(hfspi_master_init(HFSPI_MODE_CPOL0_CPHA1, HFSPIM_CLK_DIV_750K) != HF_SUCCESS)
	{ 
        #if defined(__RN8209_SPI_DBG__) && defined(__UDP_PRINT_DBG__) 
		u_printf("Init spi fail\n"); 
		#endif 
    } 
    #if defined(__RN8209_SPI_DBG__) && defined(__UDP_PRINT_DBG__)
    else
    { 
		u_printf("Init spi success\n");
    } 
    #endif 
}
/*******************************************************************************
* 函数功能: 
* 输入参数: 
* 输出参数: 
* 备    注: 
*******************************************************************************/
void SpiRn8209_ReadFrame(SpiPack *SpiData)
{
	unsigned char i;

	hfspi_cs_low();
	SpiData->spi_Address &= ~SP_WR_BIT_FLAG;                    // read mode, bit7=0							
	SpiRn8209WriteByte(SpiData->spi_Address);                   // write address /* NoAck Detect */
	for (i=SpiData->spi_NumberOfByte; i>0; i--) 
	{
		SpiRn8209ReadByte(SpiData->spi_Data+i-1);               // read data 
	}
	hfspi_cs_high();
}
/*******************************************************************************
* 函数功能: 
* 输入参数: 
* 输出参数: 
* 备    注: 
*******************************************************************************/
void SpiRn8209_WriteFrame(SpiPack *SpiData)
{
	unsigned char i;

	hfspi_cs_low();
	(SpiData->spi_Address) |= SP_WR_BIT_FLAG;               // write mode, bit7=1
	SpiRn8209WriteByte(SpiData->spi_Address);               // write address
	for(i=SpiData->spi_NumberOfByte; i>0; i--) 
	{		
		SpiRn8209WriteByte(*((SpiData->spi_Data)+i-1));     // write data									
	}	
	hfspi_cs_high();
}
/*******************************************************************************
* 函数功能: 
* 输入参数: 
* 输出参数: 
* 备    注: 
*******************************************************************************/
static void SpiRn8209WriteByte(unsigned char rn8205spi_writeData) 
{ 
	hfspi_master_send_data((char*)&rn8205spi_writeData, 1);
} 
/*******************************************************************************
* 函数功能: 
* 输入参数: 
* 输出参数: 
* 备    注: 
*******************************************************************************/
static void SpiRn8209ReadByte(unsigned char *rn8205spi_readData) 
{ 
	hfspi_master_recv_data((char*)rn8205spi_readData, 1);
} 
/*******************************************************************************
* ========================= END OF PAGE 
*******************************************************************************/

