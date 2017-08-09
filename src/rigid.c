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

//****************************************************************************
// Private Function(s)
//****************************************************************************

