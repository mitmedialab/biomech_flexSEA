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
	[This file] fm_spi: SPI Slave
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-23 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

//****************************************************************************
// Include(s)
//****************************************************************************

#include "main.h"
#include <flexsea_comm.h>
#include <spi.h>
#include <timer.h>
#include "flexsea_board.h"
#include "flexsea_sys_def.h"
#include "ui.h"
#include "isr.h"

//****************************************************************************
// Variable(s)
//****************************************************************************

SPI_HandleTypeDef spi4_handle;
SPI_HandleTypeDef spi5_handle;
SPI_HandleTypeDef spi6_handle;

uint8_t aTxBuffer4[COMM_STR_BUF_LEN];	//SPI4 TX buffer
uint8_t aRxBuffer4[COMM_STR_BUF_LEN];	//SPI4 RX buffer
uint8_t aTxBuffer6[100];				//SPI6 TX buffer
uint8_t aRxBuffer6[100];				//SPI6 RX buffer
uint8_t endSpi6TxFlag = 0, endSpi4Flag = 0;
uint16_t errorCnt = 0, ovrCnt = 0, busyCnt = 0, badCommCnt = 0;
uint32_t spi4_sr = 0, spi6_sr = 0;
uint8_t spi4Watch = 0, spi6Watch = 0;
uint8_t spi6Block = 0;
volatile uint8_t catch = 0;

//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi);

//****************************************************************************
// Public Function(s)
//****************************************************************************

//SPI4 peripheral initialization. SPI4 is used to communicate with the Plan
//board, via the expansion connector.
void init_spi4(void)
{
	//Configure SPI4 in Mode 0, Slave
	//CPOL = 0 --> clock is low when idle
	//CPHA = 0 --> data is sampled at the first edge

	spi4_handle.Instance = SPI4;
	spi4_handle.Init.Direction = SPI_DIRECTION_2LINES; 				// Full duplex
	spi4_handle.Init.Mode = SPI_MODE_SLAVE;							// Slave to the Plan board
	spi4_handle.Init.DataSize = SPI_DATASIZE_8BIT; 					// 8bits words
	spi4_handle.Init.CLKPolarity = SPI_POLARITY_LOW;				// clock is low when idle (CPOL = 0)
	spi4_handle.Init.CLKPhase = SPI_PHASE_1EDGE;					// data sampled at first (rising) edge (CPHA = 0)
	spi4_handle.Init.NSS = SPI_NSS_SOFT; 							// uses software slave select
	spi4_handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;	// SPI frequency is APB2 frequency (84MHz) / Prescaler
	spi4_handle.Init.FirstBit = SPI_FIRSTBIT_MSB;					// data is transmitted MSB first
	spi4_handle.Init.TIMode = SPI_TIMODE_DISABLED;					//
	spi4_handle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
	spi4_handle.Init.CRCPolynomial = 7;

	if(HAL_SPI_Init(&spi4_handle) != HAL_OK)
	{
		flexsea_error(SE_INIT_SPI);
	}
}

//SPI5 peripheral initialization. SPI5 is connected to the SD card
void init_spi5(void)
{
	//Configure SPI5 in Mode 0, Master
	//CPOL = 0 --> clock is low when idle
	//CPHA = 0 --> data is sampled at the first edge

	spi5_handle.Instance = SPI5;
	spi5_handle.Init.Direction = SPI_DIRECTION_2LINES; 				// Full duplex
	spi5_handle.Init.Mode = SPI_MODE_MASTER;     					// Master
	spi5_handle.Init.DataSize = SPI_DATASIZE_8BIT; 					// 8bits words
	spi5_handle.Init.CLKPolarity = SPI_POLARITY_LOW;    			// clock is low when idle (CPOL = 0)
	spi5_handle.Init.CLKPhase = SPI_PHASE_1EDGE;    				// data sampled at first (rising) edge (CPHA = 0)
	spi5_handle.Init.NSS = SPI_NSS_HARD_OUTPUT; 					// uses hardware slave select
	spi5_handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;	// SPI frequency is APB2 frequency (84MHz) / Prescaler
	spi5_handle.Init.FirstBit = SPI_FIRSTBIT_MSB;					// data is transmitted MSB first
	spi5_handle.Init.TIMode = SPI_TIMODE_DISABLED;					//
	spi5_handle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
	spi5_handle.Init.CRCPolynomial = 7;

	if(HAL_SPI_Init(&spi5_handle) != HAL_OK)
	{
		flexsea_error(SE_INIT_SPI);
	}
}

void SPI_NSS_Callback(void)
{
	//At this point, the SPI transfer is complete
	endSpi4Flag = 1;
}

void SPI_new_data_Callback(void)
{
	//update_rx_buf_array_spi(aRxBuffer4, 48);	//Legacy
	update_rx_buf_spi(aRxBuffer4, 48);			//Using circular buffer
	commPeriph[PORT_SPI].rx.bytesReadyFlag = 1;

	//Empty DMA buffer once it's copied:
	memset(aRxBuffer4, 0, 48);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{

}

//Error callback
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
	if(hspi->Instance == SPI4)	//Plan
	{
		//Re-init the bus:
		init_spi4();
		errorCnt++;
	}
}

//This gets called once an SPI transfer is complete
void completeSpiTransmit(void)
{
	if(endSpi4Flag)
	{
		endSpi4Flag = 0;
		spi4Watch++;

		//Overrun?
		spi4_sr = SPI4->SR;
		if(spi4_sr & SPI_SR_OVR)
		{
			//We had an overrun condition:
			__HAL_SPI_CLEAR_OVRFLAG_NEW(&spi4_handle);
			init_spi4();
			ovrCnt++;
		}

		//Busy when it shouldn't be?
		if(__HAL_SPI_GET_FLAG(&spi4_handle, SPI_FLAG_BSY))
		{
			SPI4->SR &= ~SPI_SR_BSY;
			busyCnt++;
		}

		// handle the new data however this device wants to
		update_rx_buf_spi(aRxBuffer4, 48);
		commPeriph[PORT_SPI].rx.bytesReadyFlag = 1;

		//Empty DMA buffer once it's copied:
		memset(aRxBuffer4, 0, 48);

		//Data for the next cycle:
		//comm_str was already generated, now we place it in the buffer:
		if(commPeriph[PORT_SPI].tx.packetReady == 1)
		{
			memcpy(aTxBuffer4, comm_str_spi, COMM_STR_BUF_LEN);
			commPeriph[PORT_SPI].tx.packetReady = 0;
		}
		else
		{
			memset(aTxBuffer4, 0xCC, 48);
		}

		if(HAL_SPI_TransmitReceive_IT(&spi4_handle, (uint8_t *) aTxBuffer4,
				(uint8_t *) aRxBuffer4, COMM_STR_BUF_LEN) != HAL_OK)
		{
			// Transfer error in transmission process
			//flexsea_error(SE_SEND_SERIAL_MASTER); (un-implemented)
		}
	}
}

//When all else fails...
void restartSpi(uint8_t port)
{
	//LED1 informs user about restart events:
	static uint8_t toggle = 0;
	toggle ^= 1;
	LED1(toggle);

	//Restart SPI:
	if(port == 4)
	{
		spi4Watch = 0;
		__HAL_SPI_CLEAR_OVRFLAG_NEW(&spi4_handle);
		SPI4->SR &= ~SPI_SR_BSY;
		init_spi4();
	}

	badCommCnt++;
}

void spiMonitoring(uint8_t portNum)
{
	if(portNum == 4)
	{
		//Getting many SPI transactions but no packets is a sign that something is wrong
		if(spi4Watch > 5)
		{
			//After N SPI transfers with 0 packets we restart the peripheral:
			restartSpi(4);
		}
	}
}

//****************************************************************************
// Private Function(s)
//****************************************************************************

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	if(hspi->Instance == SPI4)		//Plan board, SPI Slave
	{
		//Enable GPIO clock
		__GPIOE_CLK_ENABLE();
		//Enable peripheral clock
		__SPI4_CLK_ENABLE();

		//SPI4 pins:
		//=-=-=-=-=
		//NSS4: 	PE4
		//MOSI4: 	PE6
		//MISO4: 	PE5
		//SCK4: 	PE2

		//All but NSS:
		GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_5 | GPIO_PIN_6;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Alternate = GPIO_AF5_SPI4;
		HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

		//It seems that NSS can't be used as a good CS => set as input, ISR on change
		GPIO_InitStruct.Pin = GPIO_PIN_4;
		GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

		//Enable interrupts/NVIC for SPI data lines
		HAL_NVIC_SetPriority(SPI4_IRQn, ISR_SPI4, ISR_SUB_SPI4);
		HAL_NVIC_EnableIRQ(SPI4_IRQn);
		//And for the the CS pin
		HAL_NVIC_SetPriority(EXTI4_IRQn, ISR_EXTI4, ISR_SUB_EXTI4);
		HAL_NVIC_EnableIRQ(EXTI4_IRQn);
	}
	else if(hspi->Instance == SPI5)    //SD card, SPI Master
	{
		//Enable GPIO clock
		__GPIOF_CLK_ENABLE();
		//Enable peripheral clock
		__SPI5_CLK_ENABLE();

		//SPI5 pins:
		//=-=-=-=-=
		//NSS5: 	PF6
		//MOSI5: 	PF9
		//MISO5: 	PF8
		//SCK5: 	PF7

		//All but NSS:
		GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Alternate = GPIO_AF5_SPI5;
		HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

		//Enable interrupts/NVIC for SPI data lines
		HAL_NVIC_SetPriority(SPI5_IRQn, ISR_SPI5, ISR_SPI5);
		HAL_NVIC_EnableIRQ(SPI5_IRQn);
	}
	else
	{
		//Trying to configure a port that doesn't exist, flag the error
		flexsea_error(SE_INVALID_SPI);
	}
}
