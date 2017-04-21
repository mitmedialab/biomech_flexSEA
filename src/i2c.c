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
	[This file] fm_i2c: i2c comms
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
#include "user-mn.h"
#include "rigid.h"

//****************************************************************************
// Variable(s)
//****************************************************************************

I2C_HandleTypeDef hi2c1, hi2c2, hi2c3;
DMA_HandleTypeDef hdma_i2c1_tx;
DMA_HandleTypeDef hdma_i2c1_rx;

uint8_t i2c_2_r_buf[24], i2c_3_r_buf[EX_EZI2C_BUF_SIZE];
int8_t i2c1FsmState = I2C1_FSM_DEFAULT;
__attribute__ ((aligned (4))) uint8_t i2c1_dma_rx_buf[24];

//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c);
static void init_dma1_stream0_ch1(void);	//I2C1 RX
static void init_dma1_stream6_ch1(void);	//I2C1 TX

//****************************************************************************
// Public Function(s)
//****************************************************************************

//I2C1 State machine. Reads IMU via IT + DMA.
void i2c1_fsm(void)
{
	static uint8_t i2c_time_share = 0;

	i2c_time_share++;
	i2c_time_share %= 4;

	#ifdef USE_I2C_1
	#ifdef USE_IMU

	//Subdivided in 4 slots (250Hz)
	switch(i2c_time_share)
	{
		//Case 0.0: Write register
		case 0:

			i2c1FsmState = I2C1_FSM_TX_ADDR;
			IMUPrepareRead();

			break;

		//Case 0.1: Read data via DMA
		case 1:

			if(i2c1FsmState == I2C1_FSM_TX_ADDR_DONE)
			{
				//Start reading:
				i2c1FsmState = I2C1_FSM_RX_DATA;
				IMUReadAll();
			}

			break;

		//Case 0.2: Parse data
		case 2:

			if(i2c1FsmState == I2C1_FSM_RX_DATA_DONE)
			{
				//Decode received data
				IMUParseData();
			}

			break;

		//Case 0.3:
		case 3:

			break;

		default:
			break;
	}

	//ToDo: recover from errors:
	if(i2c1FsmState == I2C1_FSM_PROBLEM)
	{
		//Deal with it
	}

	#endif //USE_IMU
	#endif //USE_I2C_1
}

void i2c2_fsm(void)
{
	//Note: function named "fsm" for convention and future proof-ness, but
	//as of now there is only one slot

	#ifdef USE_I2C_2

		#ifdef USE_BATTBOARD

			static uint8_t cnt = 0;

			cnt++;
			cnt %= 4;
			if(!cnt)
			{
				//Refresh battery once every 4 cycles (250Hz):
				readBattery();
			}

		#endif	//USE_BATTBOARD

	#endif //USE_I2C_2
}

// Initialize i2c1. Currently connected to the IMU and the digital pot
void init_i2c1(void)
{
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = I2C1_CLOCK_RATE;  				//clock frequency; less than 400kHz
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2; 				//for fast mode (doesn't matter now)
	hi2c1.Init.OwnAddress1 = 0x0; 							//device address of the STM32 (doesn't matter)
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;	//using 7 bit addresses
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;  //disable dual address
	hi2c1.Init.OwnAddress2 = 0x0;							//second device addr (doesn't matter)
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;  //don't use 0x0 addr
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLED; 		//allow slave to stretch SCL
	hi2c1.State = HAL_I2C_STATE_RESET;
	HAL_I2C_Init(&hi2c1);

	//DMA:
	init_dma1_stream0_ch1();	//RX
	init_dma1_stream6_ch1();	//TX
}

// Disable I2C and free the I2C handle.
void disable_i2c1(void)
{
	HAL_I2C_DeInit(&hi2c1);
}

// Initialize i2c2. Currently connected to the expansion connector
void init_i2c2(void)
{
	//I2C_HandleTypeDef *hi2c2 contains our handle information
	//set config for the initial state of the i2c.
	hi2c2.Instance = I2C2;
	hi2c2.Init.ClockSpeed = I2C2_CLOCK_RATE;  				//clock frequency
	hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2; 				//for fast mode (doesn't matter now)
	hi2c2.Init.OwnAddress1 = 0x0; 							//device address of the STM32 (doesn't matter)
	hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;	//using 7 bit addresses
	hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;  //disable dual address
	hi2c2.Init.OwnAddress2 = 0x0;							//second device addr (doesn't matter)
	hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;  //don't use 0x0 addr
	hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLED; 		//allow slave to stretch SCL
	hi2c2.State = HAL_I2C_STATE_RESET;
	HAL_I2C_Init(&hi2c2);
}

// Disable I2C and free the I2C handle.
void disable_i2c2(void)
{
	HAL_I2C_DeInit(&hi2c2);
}

// Initialize I2C3. Connected to Execute & Regulate
void init_i2c3(void)
{
	//I2C_HandleTypeDef *hi2c3 contains our handle information
	//set config for the initial state of the i2c.
	hi2c3.Instance = I2C3;
	hi2c3.Init.ClockSpeed = I2C3_CLOCK_RATE;  				//clock frequency
	hi2c3.Init.DutyCycle = I2C_DUTYCYCLE_2; 				//for fast mode (doesn't matter now)
	hi2c3.Init.OwnAddress1 = 0x0; 							//device address of the STM32 (doesn't matter)
	hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;	//using 7 bit addresses
	hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;  //disable dual address
	hi2c3.Init.OwnAddress2 = 0x0;							//second device addr (doesn't matter)
	hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;  //don't use 0x0 addr
	hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLED; 		//allow slave to stretch SCL
	hi2c3.State = HAL_I2C_STATE_RESET;
	HAL_I2C_Init(&hi2c3);
}

// Disable I2C and free the I2C handle.
void disable_i2c3(void)
{
	HAL_I2C_DeInit(&hi2c3);
}

//Detects the end of a Master Receive:
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if(hi2c->Instance == I2C1)
	{
		if(i2c1FsmState == I2C1_FSM_RX_DATA)
		{
			//Indicate that it's done receiving:
			i2c1FsmState = I2C1_FSM_RX_DATA_DONE;
		}
		else
		{
			i2c1FsmState = I2C1_FSM_PROBLEM;
		}
	}
}

//Detects the end of a Master Transmit:
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if(hi2c->Instance == I2C1)
	{
		if(i2c1FsmState == I2C1_FSM_TX_ADDR)
		{
			//Indicate that it's done transmitting:
			i2c1FsmState = I2C1_FSM_TX_ADDR_DONE;
		}
		else
		{
			i2c1FsmState = I2C1_FSM_PROBLEM;
		}
	}
}

//****************************************************************************
// Private Function(s)
//****************************************************************************

// Implement I2C MSP Init, as called for in the stm32f4xx_hal_i2c.c file
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
	if(hi2c->Instance == I2C1)
	{
		///// SET UP GPIO /////
		//GPIO initialization constants
		GPIO_InitTypeDef GPIO_InitStruct;

		//Enable peripheral and GPIO clockS
		__GPIOB_CLK_ENABLE();
		__I2C1_CLK_ENABLE();

		 //SCL1	-> PB8 (pin 23 on MPU6500)
		 //SDA1	-> PB9 (pin 24 on MPU6500)

		//Config inputs:
		//We are configuring these pins.
		GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
		//I2C wants to have open drain lines pulled up by resistors
		GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
		//Although we need pullups for I2C, we have them externally on
		// the board.
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		//Set GPIO speed to fastest speed.
		GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
		//Assign function to pins.
		GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
		//Initialize the pins.
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		///// SET UP NVIC ///// (interrupts!)
		#if I2C1_USE_INT == 1

			HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 1);    //event interrupt
			HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
			HAL_NVIC_SetPriority(I2C1_ER_IRQn, 0, 1);//error interrupt
			HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);

		#endif
	}
	else if(hi2c->Instance == I2C2)
	{
		/*Apparently I2C2 is buggy. AFter experimenting and reading on ST
		 * forums I used the workaround of 1) not enabling the I2C clock before
		 * the GPIO and 2) enabling SCL first, then SDA. It solved the "I2C is
		 * always busy" error. */

		GPIO_InitTypeDef GPIO_InitStruct;

		//Enable peripheral and GPIO clockS
		__GPIOF_CLK_ENABLE();
		initOptionalPullUps();

		//SCL -> PF1
		GPIO_InitStruct.Pin = GPIO_PIN_1;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
		HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

		HAL_Delay(1);

		//SDA -> PF0
		GPIO_InitStruct.Pin = GPIO_PIN_0;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
		HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

		HAL_Delay(1);

		__I2C2_CLK_ENABLE();
	}
	else if(hi2c->Instance == I2C3)
	{
		/*Apparently I2C2 is buggy. AFter experimenting and reading on ST
		 * forums I used the workaround of 1) not enabling the I2C clock before
		 * the GPIO and 2) enabling SCL first, then SDA. It solved the "I2C is
		 * always busy" error. */

		GPIO_InitTypeDef GPIO_InitStruct;

		//Enable peripheral and GPIO clockS
		__GPIOA_CLK_ENABLE();
		__GPIOC_CLK_ENABLE();

		//SCL -> PA8
		GPIO_InitStruct.Pin = GPIO_PIN_8;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		HAL_Delay(1);

		//SDA -> PC9
		GPIO_InitStruct.Pin = GPIO_PIN_9;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		HAL_Delay(1);

		__I2C3_CLK_ENABLE();
	}
}

//I2C2 has optional pull-ups, controlled by PC2
void initOptionalPullUps(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	__GPIOC_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_2;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	//Set GPIO speed to fastest speed.
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	//Always ON:
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, 1);
}

// Implement I2C MSP DeInit
void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
	if(hi2c->Instance == I2C1)
	{
		__I2C1_CLK_DISABLE();
	}
	else if(hi2c->Instance == I2C2)
	{
		__I2C2_CLK_DISABLE();
	}
}

//Using DMA1 Ch 1 Stream 0 for I2C1 TX
static void init_dma1_stream0_ch1(void)
{
	//Enable clock
	__DMA1_CLK_ENABLE();

	//Initialization:
	hdma_i2c1_rx.Instance = DMA1_Stream0;
	hdma_i2c1_rx.Init.Channel = DMA_CHANNEL_1;
	hdma_i2c1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
	hdma_i2c1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_i2c1_rx.Init.MemInc = DMA_MINC_ENABLE;
	hdma_i2c1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_i2c1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	hdma_i2c1_rx.Init.Mode = DMA_NORMAL;
	hdma_i2c1_rx.Init.Priority = DMA_PRIORITY_LOW;
	hdma_i2c1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

	//Link DMA handle and I2C1 RX:
	hi2c1.hdmarx = &hdma_i2c1_rx;
	//hi2c1 is the parent:
	hi2c1.hdmarx->Parent = &hi2c1;

	HAL_DMA_Init(hi2c1.hdmarx);

	//Interrupts:
	HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
}

//Using DMA1 Ch 1 Stream 6 for I2C1 TX
static void init_dma1_stream6_ch1(void)
{
	//Enable clock
	__DMA1_CLK_ENABLE();

	//Initialization:
	hdma_i2c1_tx.Instance = DMA1_Stream6;
	hdma_i2c1_tx.Init.Channel = DMA_CHANNEL_1;
	hdma_i2c1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
	hdma_i2c1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_i2c1_tx.Init.MemInc = DMA_MINC_ENABLE;
	hdma_i2c1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_i2c1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	hdma_i2c1_tx.Init.Mode = DMA_NORMAL;
	hdma_i2c1_tx.Init.Priority = DMA_PRIORITY_LOW;
	hdma_i2c1_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

	//Link DMA handle and I2C1 TX:
	hi2c1.hdmatx = &hdma_i2c1_tx;
	//hi2c1 is the parent:
	hi2c1.hdmatx->Parent = &hi2c1;

	HAL_DMA_Init(hi2c1.hdmatx);

	//Interrupts:
	HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
}
