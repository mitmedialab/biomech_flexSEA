/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'flexsea-manage' Mid-level computing, and networking
	Copyright (C) 2016 Dephy, Inc. <http://dephy.com/>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
extern SPI_HandleTypeDef spi6_handle;

extern uint8_t aTxBuffer[COMM_STR_BUF_LEN];	//SPI TX buffer
extern uint8_t aRxBuffer[COMM_STR_BUF_LEN];	//SPI RX buffer

//****************************************************************************
// Public Function Prototype(s):
//****************************************************************************

void init_spi4(void);
void init_spi5(void);
void init_spi6(void);
void SPI_new_data_Callback(void);

//****************************************************************************
// Definition(s):
//****************************************************************************

//Interrupt priorities (lower number = higher priority)
#define EXT4_IRQ_CHANNEL			7
#define EXT4_IRQ_SUBCHANNEL			0

//Manage 0.1 SPI Hardware pins:
//=============================
//NSS4: 	PE4
//MOSI4: 	PE6
//MISO4: 	PE5
//SCK4: 	PE2

//NSS5: 	PF6
//MOSI5: 	PF9
//MISO5: 	PF8
//SCK5: 	PF7

//NSS6: 	PG8
//MOSI6: 	PG14
//MISO6: 	PG12
//SCK6: 	PG13


#endif // INC_FM_SPI_H

