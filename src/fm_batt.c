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
	[This file] fm_batt: Battery board driver
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-12-27 | jfduval | Initial release
	*
****************************************************************************/

//The Battery Board uses EZI2C (EEPROM emulation)

//****************************************************************************
// Include(s)
//****************************************************************************

#include <i2c.h>
#include "main.h"
#include "fm_batt.h"
#include "flexsea_global_structs.h"
#include "stm32f4xx_hal_i2c.h"

//****************************************************************************
// Variable(s)
//****************************************************************************

volatile uint8_t i2c2_tmp_buf[BATT_MAX_BUF_SIZE];

//****************************************************************************
// Private Function Prototype(s)
//****************************************************************************

static HAL_StatusTypeDef battery_write(uint8_t internal_reg_addr, uint8_t* pData,
		uint16_t Size);
static HAL_StatusTypeDef battery_read(uint8_t internal_reg_addr, uint8_t *pData,
		uint16_t Size);

//****************************************************************************
// Public Function(s)
//****************************************************************************

//Init
void init_battery(void)
{
	//Nothing for now
}

//Read from Battery Board:
void readBattery(void)
{
	battery_read(MEM_R_STATUS1, batt1.rawBytes, 7);

	batt1.status = batt1.rawBytes[0];
	batt1.voltage = (batt1.rawBytes[2] << 8) + batt1.rawBytes[3];
	batt1.current = (int16_t)(batt1.rawBytes[4] << 8) + batt1.rawBytes[5];
	batt1.temp = batt1.rawBytes[6];
}

//****************************************************************************
// Private Function(s)
//****************************************************************************

//Write data to the shared memory
// uint8_t internal_reg_addr: internal register address of the IMU
// uint8_t* pData: pointer to the data we want to send to that address
// uint16_t Size: amount of bytes of data pointed to by pData

static HAL_StatusTypeDef battery_write(uint8_t internal_reg_addr, uint8_t* pData,
		uint16_t Size)
{
	uint8_t i = 0;
	HAL_StatusTypeDef retVal;

	i2c2_tmp_buf[0] = internal_reg_addr;
	for(i = 1; i < Size + 1; i++)
	{
		i2c2_tmp_buf[i] = pData[i-1];
	}

	//Try to write it up to 5 times
	for(i = 0; i < 5; i++)
	{
		retVal = HAL_I2C_Mem_Write(&hi2c2, BATT_ADDR, (uint16_t) internal_reg_addr,
					I2C_MEMADD_SIZE_8BIT, pData, Size, BATT_BLOCK_TIMEOUT);

		if(retVal == HAL_OK)
		{
			break;
		}

		HAL_Delay(10);
	}

	return retVal;
}

//Read data from the shared memory
// uint8_t internal_reg_addr: internal register address of the IMU
// uint8_t* pData: pointer to where we want to save the data from the IMU
// uint16_t Size: amount of bytes we want to read
static HAL_StatusTypeDef battery_read(uint8_t internal_reg_addr, uint8_t *pData,
		uint16_t Size)
{
	uint8_t i = 0;
	HAL_StatusTypeDef retVal;

	//>>> Copy of Execute's code - todo improve
	//Currently having trouble detecting the flags to know when data is ready.
	//For now I'll transfer the previous buffer.
	for(i = 0; i < Size; i++)
	{
		pData[i] = i2c_2_r_buf[i];
	}

	//Store data:
	//assign_i2c2_data(&i2c_2_r_buf);
	//<<<<

	retVal = HAL_I2C_Mem_Read(&hi2c2, BATT_ADDR, (uint16_t) internal_reg_addr,
	I2C_MEMADD_SIZE_8BIT, i2c_2_r_buf, Size, BATT_BLOCK_TIMEOUT);

	return retVal;
}

//****************************************************************************
// Test Function(s) - Use with care!
//****************************************************************************

