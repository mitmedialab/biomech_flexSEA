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
struct i2t_s i2tBatt = {7, 6104, 76294, 125};

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

	i2c_3_t_buf[index++] = I2C_READ_KEY;

	//Data:
	SPLIT_16((uint16_t)vMin, i2c_3_t_buf, &index);		//Min. voltage
	SPLIT_16((uint16_t)i2t.leak, i2c_3_t_buf, &index);	//I2T_LEAK
	SPLIT_32((uint32_t)i2t.limit, i2c_3_t_buf, &index);	//I2T_LIMIT
	i2c_3_t_buf[index++] = i2t.nonLinThreshold;			//I2T_NON_LIN_THRESHOLD
	i2c_3_t_buf[index++] = 0;							//General config

	//Checksum:
	for(i = 0; i < index; i++)
	{
		checksum += i2c_3_t_buf[i];
	}
	i2c_3_t_buf[index++] = checksum;
}

//****************************************************************************
// Private Function(s)
//****************************************************************************

