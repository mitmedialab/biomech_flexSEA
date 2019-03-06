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
	[This file] fm_adv_analog: Advanced analog functions
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-23 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

#ifndef INC_ADV_ANALOG_H
#define INC_ADV_ANALOG_H

//****************************************************************************
// Include(s)
//****************************************************************************

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"

//****************************************************************************
// Shared variable(s)
//****************************************************************************

//...

//****************************************************************************
// Public Function Prototype(s):
//****************************************************************************

void init_adva_fc_pins(void);
void set_an0_fc(unsigned int fc);
void set_an1_fc(unsigned int fc);
void set_an2_fc(unsigned int fc);
void set_an3_fc(unsigned int fc);
HAL_StatusTypeDef mcp4661_write(uint8_t i2c_addr, uint8_t internal_reg_addr, \
				uint8_t* pData, uint16_t Size);
unsigned int set_gain_ain2(uint8_t gain);
unsigned int set_gain_ain3(uint8_t gain);
unsigned int set_resistor_ain6(uint8_t res);
unsigned int set_resistor_ain7(uint8_t res);
void disableResistorDividers(void);
void set_default_analog(void);
//****************************************************************************
// Definition(s):
//****************************************************************************

//Addresses:
#define MCP4661_I2C_ADDR1		0x50
#define MCP4661_I2C_ADDR2		0x54

#define MCP_BLOCK_TIMEOUT 10000 //ToDo: may want to reduce this?

//MCP4661
#define MCP4661_REG_RAM_W0		0x00
#define MCP4661_REG_RAM_W1		0x10
#define MCP4661_REG_EEP_W0		0x20
#define MCP4661_REG_EEP_W1		0x30
#define MCP4661_REG_TCON		0x40
#define MCP4661_REG_STATUS		0x50
#define MCP4661_WRITE_CMD		0x00
#define MCP4661_READ_CMD		0x0A
#define MCP4661_CONFIG			0xFF //(POR default)

#endif 	// INC_ADV_ANALOG_H
