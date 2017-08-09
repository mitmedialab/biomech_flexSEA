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
	[Contributors] Erin Main (ermain@mit.edu)
*****************************************************************************
	[This file] fm_i2c: IMU configuration
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-23 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

//****************************************************************************
// Include(s)
//****************************************************************************

#include <i2c.h>
#include <imu.h>
#include "main.h"
#include "flexsea_global_structs.h"
#include "flexsea_user_structs.h"
#include "stm32f4xx_hal_i2c.h"
#include "timer.h"

//****************************************************************************
// Variable(s)
//****************************************************************************

struct imu_s imu;
volatile uint8_t i2c_tmp_buf[IMU_MAX_BUF_SIZE];
uint8_t i2c_ak8963_tmp_buf[6] = {0,0,0,0,0,0};

//****************************************************************************
// Private Function Prototype(s)
//****************************************************************************

static HAL_StatusTypeDef imu_write(uint8_t internal_reg_addr, uint8_t* pData,
		uint16_t Size);
static HAL_StatusTypeDef magneto_write(uint8_t internal_reg_addr, uint8_t* pData,
		uint16_t Size);

//****************************************************************************
// Public Function(s)
//****************************************************************************

//Initialize the IMU w/ default values in config registers
void init_imu(void)
{
	//uint8_t tmp = 0;

	//Accel + Gyro:
	//=============

	//Reset the IMU.
	reset_imu();
	HAL_Delay(25);

	//Initialize the config registers.
	uint8_t config[4] = {D_IMU_CONFIG, D_IMU_GYRO_CONFIG, D_IMU_ACCEL_CONFIG, \
							D_IMU_ACCEL_CONFIG2};
	imu_write(IMU_CONFIG, config, 4);
	HAL_Delay(10);

	//Magneto:
	//========

	//Disable Master, enable bypass:
	config[0] = D_BYPASS_ENABLED;
	imu_write(IMU_INT_PIN_CFG, config, 1);
	HAL_Delay(10);
	config[0] = 0x00;
	imu_write(IMU_USER_CTRL, config, 1);
	HAL_Delay(10);

	//AK8963 basic config:
	config[0] = AKM_POWER_DOWN;
	magneto_write(AK8963_REG_CNTL1, config, 1);
	HAL_Delay(100);
	config[0] = AKM_FUSE_ROM_ACCESS;
	magneto_write(AK8963_REG_CNTL1, config, 1);
	HAL_Delay(100);
	config[0] = AKM_POWER_DOWN;
	magneto_write(AK8963_REG_CNTL1, config, 1);
	HAL_Delay(100);
	config[0] = AK8963_SET_CNTL1;	//Continuous 2, 16-bits
	magneto_write(AK8963_REG_CNTL1, config, 1);
	HAL_Delay(100);

	//Disable bypass:
	config[0] = D_BYPASS_DISABLED;
	imu_write(IMU_INT_PIN_CFG, config, 1);
	HAL_Delay(10);

	//Set Master 400kHz:
	config[0] = 13;
	//config[0] = 0x40;
	imu_write(IMU_I2C_MASTER_CONTROL, config, 1);
	HAL_Delay(10);

	//Slave 0 (Reads from AKM):
	config[0] = AK8963_ADDRESS | BIT_I2C_READ;
	imu_write(IMU_I2C_SLV0_ADDR, config, 1);
	HAL_Delay(10);
	config[0] = AK8963_REG_XOUT_L;
	//config[0] = AK8963_REG_WIA;
	imu_write(IMU_I2C_SLV0_REG, config, 1);
	HAL_Delay(10);
	config[0] = BIT_SLAVE_EN | 7;
	imu_write(IMU_I2C_SLV0_CTRL, config, 1);
	HAL_Delay(10);

	//Enable Master:
	config[0] = 0x20;
	imu_write(IMU_USER_CTRL, config, 1);
	HAL_Delay(10);
}

void imu_write_ak8963(uint8_t reg, uint8_t *val)
{
	uint8_t config[4] = {0,0,0,0};

	config[0] = AK8963_ADDRESS;
	imu_write(IMU_I2C_SLV0_ADDR, config, 1);

	config[0] = reg;
	imu_write(IMU_I2C_SLV0_REG, config, 1);

	config[0] = (*val);	//Continuous measurement 1, 16 bits
	imu_write(IMU_I2C_SLV0_DO, config, 1);

	config[0] = MPU9250_I2C_SLV_CTRL_EN | (1 & MPU9250_I2C_SLV_CTRL_LENG_MASK);	//Continuous measurement 1, 16 bits
	imu_write(IMU_I2C_SLV0_CTRL, config, 1);
}

// Reset the IMU to default settings
void reset_imu(void)
{
	uint8_t config = D_DEVICE_RESET;
	imu_write(IMU_PWR_MGMT_1, &config, 1);
	HAL_Delay(100);

	config = 0x01;
	imu_write(IMU_PWR_MGMT_1, &config, 1);
	//imu_write(IMU_SIGNAL_PATH_RESET, &config, 1);
	HAL_Delay(200);

	config = 0x00;
	imu_write(IMU_PWR_MGMT_2, &config, 1);
	HAL_Delay(10);
}

//Sends the register address. Needed before a Read
void IMUPrepareRead(void)
{
	uint8_t i2c_1_t_buf[4] = {IMU_ACCEL_XOUT_H, 0, 0, 0};
	HAL_I2C_Master_Transmit_DMA(&hi2c1, IMU_ADDR, i2c_1_t_buf, 1);
}

//Read all of the relevant IMU data (accel, gyro, temp)
void IMUReadAll(void)
{
	HAL_StatusTypeDef retVal;
	//HAL_I2C_Master_Receive_DMA(&hi2c1, IMU_ADDR, i2c1_dma_rx_buf, 21);
	retVal = HAL_I2C_Mem_Read_DMA(&hi2c1, IMU_ADDR, (uint16_t) IMU_ACCEL_XOUT_H,
								I2C_MEMADD_SIZE_8BIT, i2c1_dma_rx_buf, 21);
}

void IMUParseData(void)
{
	uint16_t tmp[10] = {0,0,0,0,0,0,0,0,0,0};
	uint16_t first = 0;
	uint8_t i = 0, index = 0;

	//Rebuilt 7x 16bits words (MSB first):
	for(i = 0; i < 7; i++)
	{
		first = (uint16_t)i2c1_dma_rx_buf[index++] << 8;
		tmp[i] = first | ((uint16_t) i2c1_dma_rx_buf[index++]);
	}
	//And 3x 16bits words (LSB first):
	for(i = 0; i < 3; i++)
	{
		first = (uint16_t)i2c1_dma_rx_buf[index++];
		tmp[i+7] = first | ((uint16_t) i2c1_dma_rx_buf[index++] << 8);
	}

	//Assign:
	imu.accel.x = (int16_t)tmp[0];
	imu.accel.y = (int16_t)tmp[1];
	imu.accel.z = (int16_t)tmp[2];
	//imu.temp = (int16_t)tmp[3];
	imu.gyro.x = (int16_t)tmp[4];
	imu.gyro.y = (int16_t)tmp[5];
	imu.gyro.z = (int16_t)tmp[6];
	imu.magneto.x = (int16_t)tmp[7];
	imu.magneto.y = (int16_t)tmp[8];
	imu.magneto.z = (int16_t)tmp[9];

	//Copy to new structure: **ToDo unify
	rigid1.mn.accel.x = imu.accel.x;
	rigid1.mn.accel.y = imu.accel.y;
	rigid1.mn.accel.z = imu.accel.z;

	rigid1.mn.gyro.x = imu.gyro.x;
	rigid1.mn.gyro.y = imu.gyro.y;
	rigid1.mn.gyro.z = imu.gyro.z;

	rigid1.mn.magneto.x = imu.magneto.x;
	rigid1.mn.magneto.y = imu.magneto.y;
	rigid1.mn.magneto.z = imu.magneto.z;
}

//****************************************************************************
// Private Function(s)
//****************************************************************************

//write data to an internal register of the IMU.
// you would use this function if you wanted to set configuration values
// for a particular feature of the IMU.
// uint8_t internal_reg_addr: internal register address of the IMU
// uint8_t* pData: pointer to the data we want to send to that address
// uint16_t Size: amount of bytes of data pointed to by pData

static HAL_StatusTypeDef imu_write(uint8_t internal_reg_addr, uint8_t* pData,
		uint16_t Size)
{
	uint8_t i = 0;
	HAL_StatusTypeDef retVal;

	i2c_tmp_buf[0] = internal_reg_addr;
	for(i = 1; i < Size + 1; i++)
	{
		i2c_tmp_buf[i] = pData[i-1];
	}

	//Try to write it up to 5 times
	for(i = 0; i < 5; i++)
	{
		retVal = HAL_I2C_Mem_Write(&hi2c1, IMU_ADDR, (uint16_t) internal_reg_addr,
					I2C_MEMADD_SIZE_8BIT, pData, Size, IMU_BLOCK_TIMEOUT);

		if(retVal == HAL_OK)
		{
			break;
		}

		HAL_Delay(10);
	}

	return retVal;
}

static HAL_StatusTypeDef magneto_write(uint8_t internal_reg_addr, uint8_t* pData,
		uint16_t Size)
{
	uint8_t i = 0;
	HAL_StatusTypeDef retVal;

	i2c_tmp_buf[0] = internal_reg_addr;
	for(i = 1; i < Size + 1; i++)
	{
		i2c_tmp_buf[i] = pData[i-1];
	}

	//Try to write it up to 3 times
	for(i = 0; i < 3; i++)
	{
		retVal = HAL_I2C_Mem_Write(&hi2c1, AK8963_ADDRESS_8BITS, (uint16_t) internal_reg_addr,
					I2C_MEMADD_SIZE_8BIT, pData, Size, IMU_BLOCK_TIMEOUT);

		if(retVal == HAL_OK)
		{
			break;
		}

		HAL_Delay(10);
	}

	return retVal;
}

//****************************************************************************
// Test Function(s)
//****************************************************************************

void imu_test_code_blocking(void)
{
	HAL_StatusTypeDef retVal;

	while(1)
	{
		/*
		init_imu();
		HAL_Delay(50);
		*/

		/*
		//Old code:
		IMUPrepareRead();
		delayUsBlocking(100);
		IMUReadAll();
		HAL_Delay(1);
		*/

		//New approach:
		retVal = HAL_I2C_Mem_Read_DMA(&hi2c1, IMU_ADDR, (uint16_t) IMU_ACCEL_XOUT_H,
							I2C_MEMADD_SIZE_8BIT, i2c1_dma_rx_buf, 21);
		HAL_Delay(1);
	}
}
