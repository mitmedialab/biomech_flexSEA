/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'flexsea-manage' Mid-level computing, and networking
	Copyright (C) 2017 Dephy, Inc. <http://dephy.com/>
*****************************************************************************
	[Lead developper] Jean-Francois (JF) Duval, jfduval at dephy dot com.
	[Origin] Based on Jean-Francois Duval's work at the MIT Media Lab
	Biomechatronics research group <http://biomech.media.mit.edu/>
	[Contributors]
*****************************************************************************
	[This file] fm_spi: SPI Slave
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-23 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

#ifndef INC_FM_SPI_H
#define INC_FM_SPI_H

//****************************************************************************
// Include(s)
//****************************************************************************

#include "flexsea.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"

//****************************************************************************
// Shared variable(s)
//****************************************************************************

extern SPI_HandleTypeDef spi4_handle;
extern SPI_HandleTypeDef spi5_handle;

extern uint8_t aTxBuffer4[COMM_STR_BUF_LEN];	//SPI TX buffer
extern uint8_t aRxBuffer4[COMM_STR_BUF_LEN];	//SPI RX buffer

extern uint8_t spi4Watch;

//****************************************************************************
// Public Function Prototype(s):
//****************************************************************************

void init_spi4(void);
void init_spi5(void);
void SPI_NSS_Callback(void);
void SPI_new_data_Callback(void);
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi);
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi);
void completeSpiTransmit(void);
void restartSpi(uint8_t port);
void spiMonitoring(uint8_t portNum);

//****************************************************************************
// Definition(s):
//****************************************************************************

//Monitoring: how many missing packets do we tolerate before restarting SPI?

//Macros from newer Cube libs:
#define UNUSED(x) ((void)(x))
#define __HAL_SPI_CLEAR_OVRFLAG_NEW(__HANDLE__)		\
do{													\
	__IO uint32_t tmpreg_ovr = 0x00U;				\
	tmpreg_ovr = (__HANDLE__)->Instance->DR;		\
	tmpreg_ovr = (__HANDLE__)->Instance->SR;		\
	UNUSED(tmpreg_ovr);								\
} while(0)

#endif // INC_FM_SPI_H
