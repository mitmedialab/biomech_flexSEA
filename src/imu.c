/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'flexsea-manage' Mid-level computing, and networking
	Copyright (C) 2017 Dephy, Inc. <http://dephy.com/>
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

//****************************************************************************
// Public Function(s)
//****************************************************************************

//Initialize the IMU w/ default values in config registers
void init_imu(void)
{
	//Accel + Gyro:
	//=============

	//Reset the IMU.
	reset_imu();
	HAL_Delay(25);

	rigid1.mn.accel.x = 0;
	rigid1.mn.accel.y = 0;
	rigid1.mn.accel.z = 0;
	rigid1.mn.gyro.x = 0;
	rigid1.mn.gyro.y = 0;
	rigid1.mn.gyro.z = 0;
	rigid1.mn.magneto.x = 0;
	rigid1.mn.magneto.y = 0;
	rigid1.mn.magneto.z = 0;

	//Initialize the config registers.
	uint8_t config[4] = {D_IMU_CONFIG, D_IMU_GYRO_CONFIG, D_IMU_ACCEL_CONFIG, \
							D_IMU_ACCEL_CONFIG2};
	imu_write(IMU_CONFIG, config, 4);
	HAL_Delay(10);
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

//Read all of the relevant IMU data (accel, gyro, temp)
void IMUReadAll(void)
{
	HAL_StatusTypeDef retVal;

	retVal = HAL_I2C_Mem_Read_DMA(&hi2c1, IMU_ADDR, (uint16_t) IMU_ACCEL_XOUT_H,
								I2C_MEMADD_SIZE_8BIT, i2c1_dma_rx_buf, 15);

	if(retVal == HAL_OK){i2c1FsmState = I2C_FSM_RX_DATA;}
	else{i2c1FsmState = I2C_FSM_PROBLEM;}
}

void IMUParseData(void)
{
	uint16_t tmp[7] = {0,0,0,0,0,0,0};
	uint16_t first = 0;
	uint8_t i = 0, index = 0;

	//Rebuilt 7x 16bits words (MSB first):
	for(i = 0; i < 7; i++)
	{
		first = (uint16_t)i2c1_dma_rx_buf[index++] << 8;
		tmp[i] = first | ((uint16_t) i2c1_dma_rx_buf[index++]);
	}

	//Assign:
	//imu.accel.x = -12178;
	//imu.accel.y = 2320;
	//imu.accel.z = -2022;
	//imu.temp = (int16_t)tmp[3];
	//imu.gyro.x = -1308;
	//imu.gyro.y = -1511;
	//imu.gyro.z = 5563;

	imu.accel.x = (int16_t)tmp[0];
    imu.accel.y = (int16_t)tmp[1];
    imu.accel.z = (int16_t)tmp[2];
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

		retVal = HAL_I2C_Mem_Read_DMA(&hi2c1, IMU_ADDR, (uint16_t) IMU_ACCEL_XOUT_H,
							I2C_MEMADD_SIZE_8BIT, i2c1_dma_rx_buf, 21);
		HAL_Delay(1);
	}
}
