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
#include "isr.h"
#include "ui.h"

#ifdef USE_6CH_AMP
#include "strain.h"
#endif	//USE_6CH_AMP

#ifdef USE_MIT_EMG_I2C
#include "user-mn-MIT-EMG.h"
#endif
//****************************************************************************
// Variable(s)
//****************************************************************************

I2C_HandleTypeDef hi2c1, hi2c2, hi2c3;
DMA_HandleTypeDef hdma_i2c1_tx, hdma_i2c1_rx;
DMA_HandleTypeDef hdma_i2c2_tx, hdma_i2c2_rx;

uint8_t i2c_2_r_buf[24], i2c_3_r_buf[MN_WBUF_SIZE+1], i2c_3_t_buf[MN_WBUF_SIZE+1];
int8_t i2c1FsmState = I2C_FSM_DEFAULT;
int8_t i2c2FsmState = I2C_FSM_DEFAULT;
__attribute__ ((aligned (4))) uint8_t i2c1_dma_rx_buf[24];
__attribute__ ((aligned (4))) uint8_t i2c2_dma_rx_buf[24];
__attribute__ ((aligned (4))) uint8_t i2c2_dma_tx_buf[24];

uint8_t firstTransfers = 0;

//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c);
static void init_dma1_stream0_ch1(I2C_HandleTypeDef *i2cHandle);	//I2C1 RX
static void init_dma1_stream6_ch1(I2C_HandleTypeDef *i2cHandle);	//I2C1 TX
static void init_dma1_stream2_ch7(I2C_HandleTypeDef *i2cHandle);	//I2C2 RX
static void init_dma1_stream7_ch7(I2C_HandleTypeDef *i2cHandle);	//I2C2 TX

//****************************************************************************
// Public Function(s)
//****************************************************************************

//I2C1 State machine. Reads IMU via DMA.
void i2c1_fsm(void)
{
	#ifdef USE_I2C_1
	#ifdef USE_IMU

	static uint8_t i2c1_time_share = 0;
	static uint8_t errorCounter = 0;

	i2c1_time_share++;
	i2c1_time_share %= 4;

	//Subdivided in 4 slots (250Hz)
	switch(i2c1_time_share)
	{
		//Case 0: Start the read process
		case 0:

			IMUReadAll();

			break;

		//Cases 1-3: Decode data (when available)
		case 1:
		case 2:
		case 3:

			if(i2c1FsmState == I2C_FSM_RX_DATA_DONE)
			{
				IMUParseData();
				i2c1FsmState = I2C_FSM_DEFAULT;
				errorCounter = 0;
			}
			break;

		default:
			break;
	}

	//Error management:
	if(i2c1FsmState == I2C_FSM_PROBLEM)
	{
		errorCounter++;
		i2c1FsmState = I2C_FSM_DEFAULT;
	}

	if(errorCounter > 5)
	{
		//FlexSEA Error code
	}

	#endif //USE_IMU
	#endif //USE_I2C_1
}

//I2C2 State machine. Expansion connector
void i2c2_fsm(void)
{
	#ifdef USE_I2C_2

	static uint8_t i2c2_time_share = 0;
	static uint8_t errorCounter = 0;

	i2c2_time_share++;
	i2c2_time_share %= 4;

	#ifdef USE_MIT_EMG_I2C
	mitEmgI2c2Fsm();
	#else

	//Subdivided in 4 slots (250Hz)
	switch(i2c2_time_share)
	{
		//Case 0: Start the read process
		case 0:

			#ifdef USE_6CH_AMP
			get_6ch_strain();
			#endif	//USE_6CH_AMP

			break;

		//Case 1-3: Decode data (when available)
		case 1:
		case 2:
		case 3:

			if(i2c2FsmState == I2C_FSM_RX_DATA_DONE)
			{
				#ifdef USE_6CH_AMP
				decode6chAmp();
				#endif
				i2c2FsmState = I2C_FSM_DEFAULT;
				errorCounter = 0;
			}

			break;

		default:
			break;
	}

	#endif //USE_MIT_EMG_I2C
	//Error management:
	if(i2c2FsmState == I2C_FSM_PROBLEM)
	{
		errorCounter++;
		i2c2FsmState = I2C_FSM_DEFAULT;
	}

	if(errorCounter > 5)
	{
		//FlexSEA Error code
	}
	#endif //USE_I2C_2
}

// Initialize I2C1. Currently connected to the IMU
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
	HAL_I2C_Init(&hi2c1);

	//DMA:
	init_dma1_stream0_ch1(&hi2c1);	//RX
	init_dma1_stream6_ch1(&hi2c1);	//TX

	//I2C Event & Error interrupts:
	HAL_NVIC_SetPriority(I2C1_ER_IRQn, ISR_I2C1_ER, ISR_SUB_I2C1_ER);
	HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
	HAL_NVIC_SetPriority(I2C1_EV_IRQn, ISR_I2C1_EV, ISR_SUB_I2C1_EV);
	HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
}

// Disable I2C and free the I2C handle.
void disable_i2c1(void)
{
	HAL_I2C_DeInit(&hi2c1);
}

// Initialize i2c2. Currently connected to the expansion connector
void init_i2c2(void)
{
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

	//DMA:
	init_dma1_stream2_ch7(&hi2c2);	//RX
	init_dma1_stream7_ch7(&hi2c2);	//TX

	//I2C Event & Error interrupts:
	HAL_NVIC_SetPriority(I2C2_ER_IRQn, ISR_I2C2_ER, ISR_SUB_I2C2_ER);
	HAL_NVIC_EnableIRQ(I2C2_ER_IRQn);
	HAL_NVIC_SetPriority(I2C2_EV_IRQn, ISR_I2C2_EV, ISR_SUB_I2C2_EV);
	HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);
}

// Disable I2C and free the I2C handle.
void disable_i2c2(void)
{
	HAL_I2C_DeInit(&hi2c2);
}

// Initialize I2C3. Connected to Execute & Regulate. Slave interface.
void init_i2c3(void)
{
	hi2c3.Instance = I2C3;
	hi2c3.Init.ClockSpeed = I2C3_CLOCK_RATE;  				//clock frequency
	hi2c3.Init.DutyCycle = I2C_DUTYCYCLE_2; 				//for fast mode (doesn't matter now)
	hi2c3.Init.OwnAddress1 = 0x44; 							//device address of the STM32 (8-bits)
	hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;	//using 7 bit addresses
	hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;  //disable dual address
	hi2c3.Init.OwnAddress2 = 0x0;							//second device addr (doesn't matter)
	hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;  //don't use 0x0 addr
	hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLED; 		//allow slave to stretch SCL
	hi2c3.State = HAL_I2C_STATE_RESET;
	HAL_I2C_Init(&hi2c3);

	//I2C Event & Error interrupts:
	HAL_NVIC_SetPriority(I2C3_ER_IRQn, ISR_I2C3_ER, ISR_SUB_I2C3_ER);
	HAL_NVIC_EnableIRQ(I2C3_ER_IRQn);

	HAL_NVIC_SetPriority(I2C3_EV_IRQn, ISR_I2C3_EV, ISR_SUB_I2C3_EV);
	HAL_NVIC_EnableIRQ(I2C3_EV_IRQn);
}

// Disable I2C and free the I2C handle.
void disable_i2c3(void)
{
	HAL_I2C_DeInit(&hi2c3);
}

//Detects the end of a Master Receive (when DMA Mem isn't used):
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	#ifdef USE_MIT_EMG_I2C
	if(hi2c->Instance == I2C2)
		mitEmgI2CRxCpltCallback(hi2c);
	#endif
}

//Detects the end of a Master Receive in DMA Mem mode:
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	//I2C1:
	if(hi2c->Instance == I2C1)
	{
		//Indicate that it's done receiving:
		i2c1FsmState = I2C_FSM_RX_DATA_DONE;
	}

	else if(hi2c->Instance == I2C2)
	{
		//Indicate that it's done receiving:
		i2c2FsmState = I2C_FSM_RX_DATA_DONE;
	}
}

//Detects the end of a Master Transmit:
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	//I2C1:
	if(hi2c->Instance == I2C1)
	{
		if(i2c1FsmState == I2C_FSM_TX_ADDR)
		{
			//Indicate that it's done transmitting:
			i2c1FsmState = I2C_FSM_TX_ADDR_DONE;
		}
		else
		{
			i2c1FsmState = I2C_FSM_PROBLEM;
		}
	}

	//I2C2:
	if(hi2c->Instance == I2C2)
	{
		if(i2c2FsmState == I2C_FSM_TX_ADDR)
		{
			//Indicate that it's done transmitting:
			i2c2FsmState = I2C_FSM_TX_ADDR_DONE;
		}
		else
		{
			i2c2FsmState = I2C_FSM_PROBLEM;
		}
	}
}

void i2c3SlaveReceiveFromMaster(void)
{
	HAL_I2C_Slave_Receive_IT(&hi2c3, i2c_3_r_buf, MN_WBUF_SIZE);
}

//I2C Slave Receive callback:
void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if(hi2c->Instance == I2C3)
	{
		decodeRegulate();
		i2c3SlaveReceiveFromMaster();
	}
}

//I2C Slave Transmit callback:
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if(hi2c->Instance == I2C3)
	{
		firstTransfers++;
		if(firstTransfers >= 2)
		{
			i2c3SlaveReceiveFromMaster();
		}
		else
		{
			i2c3SlaveTransmitToMaster();
		}
	}
}

//I2C Error Callback:
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	if(hi2c->Instance == I2C3)
	{
		LEDR(1);
		i2c3SlaveReceiveFromMaster();
	}
}

void i2c3SlaveTransmitToMaster(void)
{
	HAL_I2C_Slave_Transmit_IT(&hi2c3, i2c_3_t_buf, MN_WBUF_SIZE);
}

//****************************************************************************
// Private Function(s)
//****************************************************************************

// Implement I2C MSP Init, as called for in the stm32f4xx_hal_i2c.c file
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
	if(hi2c->Instance == I2C1)
	{
		GPIO_InitTypeDef GPIO_InitStruct;

		//Enable peripheral and GPIO clockS
		__GPIOB_CLK_ENABLE();

		 //SCL1	-> PB8 (pin 23 on MPU6500)
		 //SDA1	-> PB9 (pin 24 on MPU6500)

		//Config inputs:
		GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		#if I2C1_USE_INT == 1

			HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 1);
			HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
			HAL_NVIC_SetPriority(I2C1_ER_IRQn, 0, 1);
			HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);

		#endif

		HAL_Delay(1);
		__I2C1_CLK_ENABLE();
	}
	else if(hi2c->Instance == I2C2)
	{
		/*Apparently I2C2 is buggy. AFter experimenting and reading on ST
		 * forums I used the workaround of 1) not enabling the I2C clock before
		 * the GPIO and 2) enabling SCL first, then SDA. It solved the "I2C is
		 * always busy" error. */

		GPIO_InitTypeDef GPIO_InitStruct;

		#ifndef BOARD_SUBTYPE_POCKET

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

		#else

		//Enable peripheral and GPIO clockS
		__GPIOB_CLK_ENABLE();
		initOptionalPullUps();

		//SCL2 -> PB10
		GPIO_InitStruct.Pin = GPIO_PIN_10;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		HAL_Delay(1);

		//SDA2 -> PB11
		GPIO_InitStruct.Pin = GPIO_PIN_11;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		#endif	//BOARD_SUBTYPE_POCKET

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

//I2C2 has optional pull-ups, controlled by PC14 (0.1) or PE3 (0.2)
void initOptionalPullUps(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	#ifndef BOARD_SUBTYPE_POCKET

		#if (HW_VER == 1)

		__GPIOC_CLK_ENABLE();

		GPIO_InitStruct.Pin = GPIO_PIN_14;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		//Always ON:
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, 1);

		#else

		//HW_VER > 1 can be Rigid 0.2 or Biomech Logic

		#ifndef HW_BIOMECH

		//Rigid 0.2:
		__GPIOE_CLK_ENABLE();

		GPIO_InitStruct.Pin = GPIO_PIN_3;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
		HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

		//Always ON:
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, 1);

		#else

		//Biomech Logic:
		__GPIOF_CLK_ENABLE();

		GPIO_InitStruct.Pin = GPIO_PIN_2;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
		HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

		//Always ON:
		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_2, 1);

		#endif

		#endif

	#else

		//Pocket:
		__GPIOB_CLK_ENABLE();

		GPIO_InitStruct.Pin = GPIO_PIN_12;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		//Always ON:
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 1);

	#endif	//BOARD_SUBTYPE_POCKET
}

//Implement I2C MSP DeInit
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

//Using DMA1 Ch 1 Stream 0 for I2C1 RX
static void init_dma1_stream0_ch1(I2C_HandleTypeDef *i2cHandle)
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
	HAL_DMA_Init(&hdma_i2c1_rx);
	__HAL_LINKDMA(i2cHandle, hdmarx, hdma_i2c1_rx);

	//Interrupts:
	HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, ISR_DMA1_STREAM0, ISR_SUB_DMA1_STREAM0);
	HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
}

//Using DMA1 Ch 1 Stream 6 for I2C1 TX
static void init_dma1_stream6_ch1(I2C_HandleTypeDef *i2cHandle)
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
	HAL_DMA_Init(&hdma_i2c1_tx);
	__HAL_LINKDMA(i2cHandle,hdmatx,hdma_i2c1_tx);

	//Interrupts:
	HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, ISR_DMA1_STREAM6, ISR_SUB_DMA1_STREAM6);
	HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
}

//Using DMA1 Ch 7 Stream 2 for I2C2 RX
static void init_dma1_stream2_ch7(I2C_HandleTypeDef *i2cHandle)
{
	//Enable clock
	__DMA1_CLK_ENABLE();

	//Initialization:
	hdma_i2c2_rx.Instance = DMA1_Stream2;
	hdma_i2c2_rx.Init.Channel = DMA_CHANNEL_7;
	hdma_i2c2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
	hdma_i2c2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_i2c2_rx.Init.MemInc = DMA_MINC_ENABLE;
	hdma_i2c2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_i2c2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	hdma_i2c2_rx.Init.Mode = DMA_NORMAL;
	hdma_i2c2_rx.Init.Priority = DMA_PRIORITY_LOW;
	hdma_i2c2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

	//Link DMA handle and I2C1 RX:
	HAL_DMA_Init(&hdma_i2c2_rx);
	__HAL_LINKDMA(i2cHandle, hdmarx, hdma_i2c2_rx);

	//Interrupts:
	HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, ISR_DMA1_STREAM2, ISR_SUB_DMA1_STREAM2);
	HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);
}

//Using DMA1 Ch 7 Stream 7 for I2C2 TX
static void init_dma1_stream7_ch7(I2C_HandleTypeDef *i2cHandle)
{
	//Enable clock
	__DMA1_CLK_ENABLE();

	//Initialization:
	hdma_i2c2_tx.Instance = DMA1_Stream7;
	hdma_i2c2_tx.Init.Channel = DMA_CHANNEL_7;
	hdma_i2c2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
	hdma_i2c2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_i2c2_tx.Init.MemInc = DMA_MINC_ENABLE;
	hdma_i2c2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_i2c2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	hdma_i2c2_tx.Init.Mode = DMA_NORMAL;
	hdma_i2c2_tx.Init.Priority = DMA_PRIORITY_LOW;
	hdma_i2c2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

	//Link DMA handle and I2C1 TX:
	HAL_DMA_Init(&hdma_i2c2_tx);
	__HAL_LINKDMA(i2cHandle,hdmatx,hdma_i2c2_tx);

	//Interrupts:
	HAL_NVIC_SetPriority(DMA1_Stream7_IRQn, ISR_DMA1_STREAM7, ISR_SUB_DMA1_STREAM7);
	HAL_NVIC_EnableIRQ(DMA1_Stream7_IRQn);
}
