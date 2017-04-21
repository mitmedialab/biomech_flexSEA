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
	[This file] rigid: Interface with Rigid's other microcontrollers
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2017-04-19 | jfduval | Initial release
	*
****************************************************************************/

//****************************************************************************
// Include(s)
//****************************************************************************

#include "main.h"
#include "rigid.h"
#include "i2c.h"
#include "flexsea.h"
#include "flexsea_user_structs.h"
#include "stm32f4xx_hal_i2c.h"

//****************************************************************************
// Variable(s)
//****************************************************************************

uint8_t i2c3_tmp_buf[EX_EZI2C_BUF_SIZE];

//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************

static HAL_StatusTypeDef i2cWriteEx(uint8_t internal_reg_addr, uint8_t* pData,
		uint16_t Size);
static HAL_StatusTypeDef i2cReadEx(uint8_t internal_reg_addr, uint8_t *pData,
		uint16_t Size);

//****************************************************************************
// Function(s)
//****************************************************************************

void initRigidIO(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	//Enable GPIO Peripheral clocks
	__GPIOG_CLK_ENABLE();

	//SYNC as input with interrupt:
	GPIO_InitStructure.Pin = GPIO_PIN_15;
	GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStructure.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 7, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

	//***ToDo remove, temporary debugging:***
	//U2TX used as an output:
	__GPIOD_CLK_ENABLE();
	GPIO_InitStructure.Pin = GPIO_PIN_5;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
}

void readExecute(void)
{
	uint16_t index = EX_MEM_R_MOT_ANGLE3;
	i2cReadEx(EX_MEM_R_MOT_ANGLE3, &i2c3_tmp_buf[EX_MEM_R_MOT_ANGLE3], 18);

	*(rigid1.ex.enc_ang) = (int32_t) REBUILD_UINT32(i2c3_tmp_buf, &index);
	*(rigid1.ex.enc_ang_vel) = (int32_t) REBUILD_UINT32(i2c3_tmp_buf, &index);
	rigid1.ex.strain = REBUILD_UINT16(i2c3_tmp_buf, &index);
	rigid1.ex.current = (int32_t) REBUILD_UINT32(i2c3_tmp_buf, &index);
	rigid1.ex.ctrl.pwm = (int16_t)REBUILD_UINT16(i2c3_tmp_buf, &index);
	rigid1.ex.status = REBUILD_UINT16(i2c3_tmp_buf, &index);
}

//****************************************************************************
// Private Function(s)
//****************************************************************************

//***ToDo: this is copied from fm_batt. Can be much better! ****

//Write data to the shared memory
// uint8_t internal_reg_addr: internal register address of the IMU
// uint8_t* pData: pointer to the data we want to send to that address
// uint16_t Size: amount of bytes of data pointed to by pData

static HAL_StatusTypeDef i2cWriteEx(uint8_t internal_reg_addr, uint8_t* pData,
		uint16_t Size)
{
	uint8_t i = 0;
	HAL_StatusTypeDef retVal;

	i2c3_tmp_buf[0] = internal_reg_addr;
	for(i = 1; i < Size + 1; i++)
	{
		i2c3_tmp_buf[i] = pData[i-1];
	}

	//Try to write it up to 5 times
	for(i = 0; i < 5; i++)
	{
		retVal = HAL_I2C_Mem_Write(&hi2c3, EX_I2C_ADDR, (uint16_t) internal_reg_addr,
					I2C_MEMADD_SIZE_8BIT, pData, Size, EX_I2C_TIMEOUT);

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
static HAL_StatusTypeDef i2cReadEx(uint8_t internal_reg_addr, uint8_t *pData,
		uint16_t Size)
{
	uint8_t i = 0;
	HAL_StatusTypeDef retVal;

	//>>> Copy of Execute's code - todo improve
	//Currently having trouble detecting the flags to know when data is ready.
	//For now I'll transfer the previous buffer.
	for(i = 0; i < Size; i++)
	{
		pData[i] = i2c_3_r_buf[i];
	}

	//Store data:
	//assign_i2c2_data(&i2c_2_r_buf);
	//<<<<

	retVal = HAL_I2C_Mem_Read(&hi2c3, EX_I2C_ADDR, (uint16_t) internal_reg_addr,
	I2C_MEMADD_SIZE_8BIT, i2c_3_r_buf, Size, EX_I2C_TIMEOUT);

	return retVal;
}
