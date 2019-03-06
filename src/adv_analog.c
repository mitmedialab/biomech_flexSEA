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

//Digital potentiometer 1 (U3):
//	Use MCP4661_I2C_ADDR1
//	P0 = AIN3 Gain
//	P1 = AIN2 Gain

//Digital potentiometer 2 (U4):
//	Use MCP4661_I2C_ADDR2
//	P0 = AIN7 Divider
//	P1 = AIN6 Divider

//****************************************************************************
// Include(s)
//****************************************************************************

#include "adv_analog.h"
#include <i2c.h>
#include <timer.h>
#include "main.h"
#include "stm32f4xx_hal_i2c.h"
#include "user-mn.h"

//****************************************************************************
// Variable(s)
//****************************************************************************

uint8_t mcp_data[2] = { '0', '0' };

//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************

//****************************************************************************
// Public Function(s)
//****************************************************************************

//Initialize all the FCx pins as Inputs (high-z)
//It means that fc = 10kHz by default
void init_adva_fc_pins(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	//Enable peripheral and GPIO clocks
	__GPIOD_CLK_ENABLE();
	__GPIOG_CLK_ENABLE();

	//Pins: FC0 = PB15, FC1 = PD10, FC2 = PB13, FC3 = PB12

	/* ToDo: disabled only because of USART3 RTS/CTS experiments.
	 * Re-enable if you need this! */

//	//Config inputs:
	GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);


	GPIO_InitStruct.Pin = GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

//Programmable filter on AN0, uses FC0 (PB15)
void set_an0_fc(unsigned int fc)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	if(fc == 1)
	{
		//fc = 1 => 1kHz => Low output
		GPIO_InitStruct.Pin = GPIO_PIN_15;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		//Now that it's an output, set low:
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 0);
	}
	else
	{
		//otherwise fc = 10kHz => High-Z input

		//fc = 10 => 10kHz => Low output
		GPIO_InitStruct.Pin = GPIO_PIN_15;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	}
}

//Programmable filter on AN1, uses FC1 (PD10)
void set_an1_fc(unsigned int fc)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	if(fc == 1)
	{
		//fc = 1 => 1kHz => Low output
		GPIO_InitStruct.Pin = GPIO_PIN_10;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
		HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

		//Now that it's an output, set low:
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_10, 0);
	}
	else
	{
		//otherwise fc = 10kHz => High-Z input

		//fc = 10 => 10kHz => Low output
		GPIO_InitStruct.Pin = GPIO_PIN_10;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	}
}

//Programmable filter on AN2, uses FC2 (PB13)
void set_an2_fc(unsigned int fc)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	if(fc == 1)
	{
		//fc = 1 => 1kHz => Low output
		GPIO_InitStruct.Pin = GPIO_PIN_13;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		//Now that it's an output, set low:
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
	}
	else
	{
		//otherwise fc = 10kHz => High-Z input

		//fc = 10 => 10kHz => Low output
		GPIO_InitStruct.Pin = GPIO_PIN_13;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	}
}

//Programmable filter on AN3, uses FC3 (PB12)
void set_an3_fc(unsigned int fc)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	if(fc == 1)
	{
		//fc = 1 => 1kHz => Low output
		GPIO_InitStruct.Pin = GPIO_PIN_12;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		//Now that it's an output, set low:
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 0);
	}
	else
	{
		//otherwise fc = 10kHz => High-Z input

		//fc = 10 => 10kHz => Low output
		GPIO_InitStruct.Pin = GPIO_PIN_12;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	}
}

//Volatile write, AIN2 gain
//Manage 0.1: 0 = x1, 255 = x10.
unsigned int set_gain_ain2(uint8_t gain)
{
	uint8_t i2c_1_t_buf[4] = {MCP4661_REG_RAM_W1, gain, 0, 0};
	// HAL_I2C_Master_Transmit_DMA(&hi2c1, MCP4661_I2C_ADDR1, i2c_1_t_buf, 2);
	HAL_I2C_Master_Transmit(&hi2c1, MCP4661_I2C_ADDR1, i2c_1_t_buf, 2,10);

	return 0;
}

//Volatile write, AIN3 gain
//Manage 0.1: 0 = x1, 255 = x10.
unsigned int set_gain_ain3(uint8_t gain)
{
	uint8_t i2c_1_t_buf[4] = {MCP4661_REG_RAM_W0, gain, 0, 0};
	// HAL_I2C_Master_Transmit_DMA(&hi2c1, MCP4661_I2C_ADDR1, i2c_1_t_buf, 2);
	HAL_I2C_Master_Transmit(&hi2c1, MCP4661_I2C_ADDR1, i2c_1_t_buf, 2,10);

	return 0;
}

//Volatile write, AIN6 voltage divider
//Manage 0.1: 0 = x1, 255 = x10.
unsigned int set_resistor_ain6(uint8_t res)
{
	uint8_t i2c_1_t_buf[4] = {MCP4661_REG_RAM_W1, res, 0, 0};
	HAL_I2C_Master_Transmit_DMA(&hi2c1, MCP4661_I2C_ADDR2, i2c_1_t_buf, 2);

	return 0;
}

//Volatile write, AIN7 voltage divider
//Manage 0.1: 0 = x1, 255 = x10.
unsigned int set_resistor_ain7(uint8_t res)
{
	uint8_t i2c_1_t_buf[4] = {MCP4661_REG_RAM_W1, res, 0, 0};
	HAL_I2C_Master_Transmit_DMA(&hi2c1, MCP4661_I2C_ADDR2, i2c_1_t_buf, 2);

	return 0;
}

//Disconnect the resistors:
void disableResistorDividers(void)
{
	uint8_t i2c_1_t_buf[4] = {MCP4661_REG_TCON, 0, 0, 0};
	HAL_I2C_Master_Transmit_DMA(&hi2c1, MCP4661_I2C_ADDR2, i2c_1_t_buf, 2);
}

//Default configuration: high Fc, minimum gains, maximum voltage dividers
void set_default_analog(void)
{
	//10kHz LPF on AN0..3:
	set_an0_fc(0);
	set_an1_fc(0);
	set_an2_fc(0);
	set_an3_fc(0);

	#ifdef USE_I2C_1

		//Minimum gains:
		set_gain_ain2(0);
		delayUsBlocking(10000);
		set_gain_ain3(0);
		delayUsBlocking(10000);

		//Voltage divider:
		/*
		set_resistor_ain6(0);
		delayUsBlocking(10000);
		set_resistor_ain7(0);
		delayUsBlocking(10000);
		*/
		disableResistorDividers();
		delayUsBlocking(10000);

	#endif	//USE_I2C0
}


//****************************************************************************
// Private Function(s)
//****************************************************************************

//...
