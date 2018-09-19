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
struct i2t_s i2tBatt = {.shift = 7, .leak = 6105, .limit = 76295, \
						.nonLinThreshold = 125, .useNL = I2T_ENABLE_NON_LIN};

uint16_t uvlo = 0;

//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************

//****************************************************************************
// Function(s)
//****************************************************************************

void initRigidIO(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	#ifndef BOARD_SUBTYPE_POCKET

	//Enable GPIO Peripheral clocks
	__GPIOG_CLK_ENABLE();

	//SYNC as input with interrupt:
	GPIO_InitStructure.Pin = GPIO_PIN_15;
	GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStructure.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 7, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

	#else

	//Enable GPIO Peripheral clocks
	__GPIOC_CLK_ENABLE();

	//SYNC as input with interrupt:
	GPIO_InitStructure.Pin = GPIO_PIN_8;
	GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStructure.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 7, 0);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

	#endif	//BOARD_SUBTYPE_POCKET
}

//Decodes the bytes received over I2C
void decodeRegulate(void)
{
	uint16_t index = MN_W_VB_MSB;

	rigid1.re.vb = REBUILD_UINT16(i2c_3_r_buf, &index);
	rigid1.re.vg = REBUILD_UINT16(i2c_3_r_buf, &index);
	rigid1.re.v5 = REBUILD_UINT16(i2c_3_r_buf, &index);
	rigid1.re.current = (int16_t)REBUILD_UINT16(i2c_3_r_buf, &index);
	rigid1.re.temp = i2c_3_r_buf[index++];
	rigid1.re.status = i2c_3_r_buf[index++];
}

//Prepares the I2C3 buffer for Re to read at boot:
void setRegulateLimits(uint16_t vMin, struct i2t_s i2t)
{
	uint16_t index = 0;
	uint8_t i = 0, checksum = 0;

	//Config byte contains shift + non-lin enable bit
	LIMIT(i2t.shift, 0, 15);	//Limit to 4 bits
	i2t.config = i2t.shift | i2t.useNL;

	i2c_3_t_buf[index++] = I2C_READ_KEY;

	//Data:
	SPLIT_16((uint16_t)vMin, i2c_3_t_buf, &index);		//Min. voltage
	SPLIT_16((uint16_t)i2t.leak, i2c_3_t_buf, &index);	//I2T_LEAK
	SPLIT_32((uint32_t)i2t.limit, i2c_3_t_buf, &index);	//I2T_LIMIT
	i2c_3_t_buf[index++] = i2t.nonLinThreshold;			//I2T_NON_LIN_THRESHOLD
	i2c_3_t_buf[index++] = i2t.config;					//I2T Config

	//Checksum:
	for(i = 0; i < index; i++)
	{
		checksum += i2c_3_t_buf[i];
	}
	i2c_3_t_buf[index++] = checksum;
}

void saveUVLO(uint16_t v)
{
	LIMIT(v, MIN_UVLO, MAX_UVLO);
	uvlo = v;
}

uint16_t getUVLO(void)
{
	return uvlo;
}

void loadNvUVLO(void)
{
	#ifdef USE_EEPROM

	uint16_t v = 0;

	//Get UVLO from EEPROM (if it was ever written)
	readUvloEEPROM();
	v = getNvUVLO();
	if(v != 0)
	{
		//We read a valid value
		saveUVLO(v);
	}
	else
	{
		saveUVLO(DEFAULT_UVLO);
	}

	#else

	saveUVLO(DEFAULT_UVLO);

	#endif	//USE_EEPROM
}

//****************************************************************************
// Private Function(s)
//****************************************************************************

