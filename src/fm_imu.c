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

#include "main.h"
#include "fm_i2c.h"
#include "fm_imu.h"
#include "flexsea_global_structs.h"
#include "stm32f4xx_hal_i2c.h"

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

//****************************************************************************
// Public Function(s)
//****************************************************************************

//Initialize the IMU w/ default values in config registers
void init_imu(void)
{
	//Reset the IMU.
	reset_imu();
	HAL_Delay(25);

	// Initialize the config registers.
	uint8_t config[4] = { D_IMU_CONFIG, D_IMU_GYRO_CONFIG, D_IMU_ACCEL_CONFIG, \
							D_IMU_ACCEL_CONFIG2 };
	imu_write(IMU_CONFIG, config, 4);

	/*
	//Bypass I2C:
	HAL_Delay(10);
	config[0] = D_BYPASS_ENABLED;
	imu_write(IMU_INT_PIN_CFG, config, 1);

	//Disable Master:
	HAL_Delay(10);
	config[0] = 0;
	imu_write(IMU_USER_CTRL, config, 1);

	//Bypass I2C:
	HAL_Delay(10);
	config[0] = D_BYPASS_ENABLED;
	imu_write(IMU_INT_PIN_CFG, config, 1);

	HAL_Delay(10);
	*/
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

	/*
	config = 0x01;
	imu_write(IMU_PWR_MGMT_1, &config, 1);
	HAL_Delay(25);
	*/

	config = 0x00;
	imu_write(IMU_PWR_MGMT_2, &config, 1);
	HAL_Delay(10);


	/*
	config = 1;
	imu_write(IMU_USER_CTRL, &config, 1);
	HAL_Delay(10);

	config = 0;
	imu_write(IMU_SIGNAL_PATH_RESET, &config, 1);
	HAL_Delay(10);
	*/
}

//Sends the register address. Needed before a Read
void IMUPrepareRead(void)
{
	uint8_t i2c_1_t_buf[4] = {IMU_ACCEL_XOUT_H, 0, 0, 0};
	//uint8_t i2c_1_t_buf[4] = {IMU_GYRO_XOUT_H, 0, 0, 0};
	HAL_I2C_Master_Transmit_DMA(&hi2c1, IMU_ADDR, i2c_1_t_buf, 1);
}

//Read all of the relevant IMU data (accel, gyro, temp)
void IMUReadAll(void)
{
	HAL_I2C_Master_Receive_DMA(&hi2c1, IMU_ADDR, i2c1_dma_rx_buf, 14);
}

void IMUParseData(void)
{
	uint16_t tmp[7] = {0,0,0,0,0,0,0};
	uint8_t i = 0, index = 0;

	//Rebuilt 7x 16bits words:
	for(i = 0; i < 7; i++)
	{
		tmp[i] = ((uint16_t)i2c1_dma_rx_buf[index++] << 8) | ((uint16_t) i2c1_dma_rx_buf[index++]);
	}

	//Assign:
	imu.accel.x = (int16_t)tmp[0];
	imu.accel.y = (int16_t)tmp[1];
	imu.accel.z = (int16_t)tmp[2];
	//imu.temp = (int16_t)tmp[3];
	imu.gyro.x = (int16_t)tmp[4];
	imu.gyro.y = (int16_t)tmp[5];
	imu.gyro.z = (int16_t)tmp[6];
}

//===================
//AK8963 Magnetometer
//===================

/*
void init_ak8963(void)
{
	uint8_t tries = 0, works = 0;

	//First, can we talk to the sensor?
	for(tries = 0; tries < 10; tries++)
	{
		if(ak8963_read_status() == 0x48)
		{
			works = 1;
			break;
		}
		HAL_Delay(10);
	}

	if(works)
	{
		//Configures CNTL1:
		HAL_Delay(10);
		i2c_ak8963_tmp_buf[0] = AK8963_REG_CNTL1;
		i2c_ak8963_tmp_buf[1] = AK8963_SET_CNTL1;
      	I2C_1_MasterWriteBuf( AK8963_ADDRESS, (uint8_t *)i2c_ak8963_tmp_buf, 2, (I2C_1_MODE_COMPLETE_XFER ));
		HAL_Delay(10);
	}
}

//Reads WIA, supposed to return 0x48
uint8_t ak8963_read_status(void)
{
	//i2c1_read(AK8963_ADDRESS, AK8963_REG_WIA, i2c_ak8963_tmp_buf, 1);
	uint8_t status = 0;

	//Start, address, Write mode
	status = I2C_1_MasterSendStart(AK8963_ADDRESS, 0);
	if(status != I2C_1_MSTR_NO_ERROR)
		HAL_Delay(1);

	//Write to register
	status = I2C_1_MasterWriteByteTimeOut(0x00, 500);
	if(status != I2C_1_MSTR_NO_ERROR)
	{
		//Release bus:
		I2C_1_BUS_RELEASE;
	}

	I2C_1_MasterReadBuf(AK8963_ADDRESS, (uint8_t *)i2c_ak8963_tmp_buf, 1, (I2C_1_MODE_COMPLETE_XFER | I2C_1_MODE_REPEAT_START));

	return i2c_ak8963_tmp_buf[0];
}

//Puts all the gyroscope values in the structure:
void get_magneto_xyz(void)
{
	uint8_t tmp_data[8] = {0,0,0,0,0,0,0,0};

	//According to the documentation it's X_L, X_H, Y_L, ...
	//We need to read ST2 everytime, hence the 7th byte
	i2c1_read(AK8963_ADDRESS, AK8963_REG_XOUT_L, tmp_data, 7);

}
*/

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
