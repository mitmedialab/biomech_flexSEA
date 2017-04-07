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
	[This file] fm_uarts: Deals with the 3 USARTs
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-23 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

//IMPORTANT: USART1 was reworked to use DMA for TX. USART6 was modified (copy
//and paste from USART1), but not completely tested.

//****************************************************************************
// Include(s)
//****************************************************************************

#include <main.h>
#include "fm_uarts.h"
#include <flexsea_comm.h>
#include "flexsea_sys_def.h"
#include "flexsea_board.h"

//****************************************************************************
// Variable(s)
//****************************************************************************

USART_HandleTypeDef husart1;		//RS-485 #1
USART_HandleTypeDef husart6;		//RS-485 #2
USART_HandleTypeDef husart3;		//Expansion port
GPIO_InitTypeDef GPIO_InitStruct;
DMA_HandleTypeDef hdma2_str2_ch4;	//DMA for RS-485 #1 RX
DMA_HandleTypeDef hdma2_str7_ch4;	//DMA for RS-485 #1 TX
DMA_HandleTypeDef hdma2_str1_ch5;	//DMA for RS-485 #2 RX
DMA_HandleTypeDef hdma2_str6_ch5;	//DMA for RS-485 #2 TX
DMA_HandleTypeDef hdma1_str1_ch4;	//DMA for USART3 RX
DMA_HandleTypeDef hdma1_str3_ch4;	//DMA for USART3 TX

//DMA buffers and config:
__attribute__ ((aligned (4))) uint8_t uart1_dma_rx_buf[RX_BUF_LEN];
__attribute__ ((aligned (4))) uint8_t uart1_dma_tx_buf[RX_BUF_LEN];
uint32_t rs485_1_dma_xfer_len = COMM_STR_BUF_LEN;
__attribute__ ((aligned (4))) uint8_t uart6_dma_rx_buf[RX_BUF_LEN];
__attribute__ ((aligned (4))) uint8_t uart6_dma_tx_buf[RX_BUF_LEN];
uint32_t rs485_2_dma_xfer_len = COMM_STR_BUF_LEN;
__attribute__ ((aligned (4))) uint8_t uart3_dma_rx_buf[RX_BUF_LEN];
__attribute__ ((aligned (4))) uint8_t uart3_dma_tx_buf[RX_BUF_LEN];
uint32_t uart3_dma_xfer_len = COMM_STR_BUF_LEN;
//Note: Not sure if they have to be aligned, but can't hurt too much.

volatile uint8_t dmaRx1ConfigFlag = 0;
volatile uint8_t dmaRx2ConfigFlag = 0;

//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************

static void init_dma2_stream2_ch4(void);
static void init_dma2_stream1_ch5(void);
static void init_dma2_stream7_ch4(void);
static void init_dma2_stream6_ch5(void);
static void init_dma1_stream1_ch4(void);
static void init_dma1_stream3_ch4(void);

//****************************************************************************
// Public Function(s)
//****************************************************************************

//USART1 init function: RS-485 #1
//TX and RX are done via DMA
void init_usart1(uint32_t baudrate)
{
	husart1.Instance = USART1;

	//MSP Init (enables clock, GPIOs)
	HAL_USART_MspInit(&husart1);

	//Interrupts:
	HAL_NVIC_SetPriority(USART1_IRQn, UART1_IRQ_CHANNEL, UART1_IRQ_SUBCHANNEL);
	HAL_NVIC_EnableIRQ(USART1_IRQn);

	//UART1 module:
	husart1.Init.BaudRate = baudrate;	//Wrong, see below
	husart1.Init.WordLength = USART_WORDLENGTH_8B;
	husart1.Init.StopBits = USART_STOPBITS_1;
	husart1.Init.Parity = USART_PARITY_NONE;
	husart1.Init.Mode = USART_MODE_TX_RX;
	HAL_USART_Init(&husart1);

	//ToDo Add HAL_OK check and call
	//flexsea_error(SE_INIT_USART);

	//Manually setting some important bits:
	USART1->CR1 |= USART_CR1_OVER8;		//8x oversampling (for higher baudrates)
	USART1->CR1 |= USART_CR1_RE;		//Enable receiver
	USART1->CR2 &= ~USART_CR2_CLKEN;	//Disable synchronous clock
	USART1->CR3 |= USART_CR3_ONEBIT;	//1 sample per bit
	USART1->CR3 |= USART_CR3_DMAR;		//Enable DMA Reception
	USART1->CR3 |= USART_CR3_DMAT;		//Enable DMA Transmission

	//The baudrate calculated by the HAL function is wrong by 5% because
	//I manually change the OVER8 bit. Manually setting it:
	USART1->BRR = USART1_6_2MBAUD;

	//Enable DMA:
	init_dma2_stream2_ch4();	//RX
	init_dma2_stream7_ch4();	//TX
}

//USART6 init function: RS-485 #2
//TX and RX are done via DMA
void init_usart6(uint32_t baudrate)
{
	husart6.Instance = USART6;

	//MSP Init (enables clock, GPIOs)
	HAL_USART_MspInit(&husart6);

	//Interrupts:
	HAL_NVIC_SetPriority(USART6_IRQn, UART6_IRQ_CHANNEL, UART6_IRQ_SUBCHANNEL);
	HAL_NVIC_EnableIRQ(USART6_IRQn);

	//UART1 module:
	husart6.Init.BaudRate = baudrate;	//Wrong, see below
	husart6.Init.WordLength = USART_WORDLENGTH_8B;
	husart6.Init.StopBits = USART_STOPBITS_1;
	husart6.Init.Parity = USART_PARITY_NONE;
	husart6.Init.Mode = USART_MODE_TX_RX;
	HAL_USART_Init(&husart6);

	//ToDo Add HAL_OK check and call
	//flexsea_error(SE_INIT_USART);

	//Manually setting some important bits:
	USART6->CR1 |= USART_CR1_OVER8;		//8x oversampling (for higher baudrates)
	USART6->CR1 |= USART_CR1_RE;		//Enable receiver
	USART6->CR2 &= ~USART_CR2_CLKEN;	//Disable synchronous clock
	USART6->CR3 |= USART_CR3_ONEBIT;	//1 sample per bit
	USART6->CR3 |= USART_CR3_DMAR;		//Enable DMA Reception
	USART6->CR3 |= USART_CR3_DMAT;		//Enable DMA Transmission

	//The baudrate calculated by the HAL function is wrong by 5% because
	//I manually change the OVER8 bit. Manually setting it:
	USART6->BRR = USART1_6_2MBAUD;

	//Enable DMA:
	init_dma2_stream1_ch5();	//RX
	init_dma2_stream6_ch5();	//TX
}

//USART3 init function: Expansion port
void init_usart3(uint32_t baudrate)
{
	husart3.Instance = USART3;

	//MSP Init (enables clock, GPIOs)
	HAL_USART_MspInit(&husart3);

	//Interrupts:
	HAL_NVIC_SetPriority(USART3_IRQn, UART3_IRQ_CHANNEL, UART3_IRQ_SUBCHANNEL);
	HAL_NVIC_EnableIRQ(USART3_IRQn);

	//UART3 module:
	husart3.Init.BaudRate = baudrate;
	husart3.Init.WordLength = USART_WORDLENGTH_8B;
	husart3.Init.StopBits = USART_STOPBITS_1;
	husart3.Init.Parity = USART_PARITY_NONE;
	husart3.Init.Mode = USART_MODE_TX_RX;
	HAL_USART_Init(&husart3);

	//ToDo Add HAL_OK check and call
	//flexsea_error(SE_INIT_USART);

	//With only HAL_USART_Init() I never get an interrupt. Manually setting 5 bits:
	//16x oversampling, Receive enable, enable RXNE interrupts:
	//USART3->CR1 |= USART_CR1_OVER8;		//8x oversampling (for higher baudrates)
	USART3->CR1 |= USART_CR1_RE;		//Enable receiver
	USART3->CR2 &= ~USART_CR2_CLKEN;	//Disable synchronous clock
	USART3->CR3 |= USART_CR3_ONEBIT;	//1 sample per bit
	USART3->CR3 |= USART_CR3_DMAR;		//Enable DMA Reception
	USART3->CR3 |= USART_CR3_DMAT;		//Enable DMA Transmission
	USART3->CR3 |= USART_CR3_CTSE;		//Enable CTS
	USART3->CR3 |= USART_CR3_RTSE;		//Enable RTS

	init_dma1_stream1_ch4();
	init_dma1_stream3_ch4();
}

//Initialize GPIOs for RS-485: RE, DE
//(doesn't do the UART pins)
void init_rs485_outputs(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	// Enable GPIO Peripheral clock on port F
	__GPIOF_CLK_ENABLE();

	// Configure pin in output push/pull mode
	GPIO_InitStructure.Pin = GPIO_PIN_11 | GPIO_PIN_12;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStructure);

	//Note: currently only configuring for asynch RS-485 #1 & #2
	//(so 2 of the 6 transceivers)
}

//Receive or Transmit
void rs485_set_mode(uint32_t port, uint8_t rx_tx)
{
	if(port == PORT_RS485_1)	//RS-485 #1 / USART1
	{
		//USART1 (RS-485 #1):
		//===================
		//RE1:	 	PF12
		//DE1: 		PF11

		if(rx_tx == RS485_TX)
		{
			//Half-duplex TX (Receive disabled):
			HAL_GPIO_WritePin(GPIOF, GPIO_PIN_12, 1);	//RE
			HAL_GPIO_WritePin(GPIOF, GPIO_PIN_11, 1);	//DE
		}
		else if(rx_tx == RS485_RX)
		{
			//Half-duplex RX (Transmit disabled):
			HAL_GPIO_WritePin(GPIOF, GPIO_PIN_12, 0);	//RE
			HAL_GPIO_WritePin(GPIOF, GPIO_PIN_11, 0);	//DE
		}
		else if(rx_tx == RS485_RX_TX)
		{
			//Read & Write:
			HAL_GPIO_WritePin(GPIOF, GPIO_PIN_12, 0);	//RE
			HAL_GPIO_WritePin(GPIOF, GPIO_PIN_11, 1);	//DE
		}
		else
		{
			//Standby: no transmission, no reception
			HAL_GPIO_WritePin(GPIOF, GPIO_PIN_12, 1);	//RE
			HAL_GPIO_WritePin(GPIOF, GPIO_PIN_11, 0);	//DE
		}
	}
}

//Sends a string via RS-485 #1 (USART1)
//Note: this is sending via interrupt, but not using the DMA
void puts_rs485_1(uint8_t *str, uint16_t length)
{
	unsigned int i = 0;
	uint8_t *uart1_dma_buf_ptr;
	uart1_dma_buf_ptr = (uint8_t*) &uart1_dma_tx_buf;

	//DEBUG_OUT_DIO4(1);	//ToDo remove, debug only

	//Transmit enable
	rs485_set_mode(PORT_RS485_1, RS485_TX);

	//Copy str to tx buffer:
	memcpy(uart1_dma_tx_buf, str, length);

	//ToDo replace by valid delay function!
	for(i = 0; i < 1000; i++);

	//Send data via DMA:
	HAL_USART_Transmit_DMA(&husart1, uart1_dma_buf_ptr, length);
}

//Prepares the board for a Reply (reception). Blocking.
//ToDo: add timeout
uint8_t reception_rs485_1_blocking(void)
{
	int delay = 0;

	//DEBUG_OUT_DIO6(1);	//ToDo remove, debug only

	//Pointer to our storage buffer:
	uint32_t *uart1_dma_buf_ptr;
	uart1_dma_buf_ptr = (uint32_t*) &uart1_dma_rx_buf;
	uint32_t tmp = 0;

	//Do not enable if still transmitting:
	while(husart1.State == HAL_USART_STATE_BUSY_TX);
	for(delay = 0; delay < 600; delay++);		//Short delay

	//Receive enable
	rs485_set_mode(PORT_RS485_1, RS485_RX);
	tmp = USART1->DR;	//Read buffer to clear

	//Start the DMA peripheral
	//DEBUG_OUT_DIO5(1);	//ToDo remove, debug only
	dmaRx1ConfigFlag = 1;
	HAL_DMA_Start_IT(&hdma2_str2_ch4, (uint32_t) &USART1->DR,
			(uint32_t) uart1_dma_buf_ptr, rs485_1_dma_xfer_len);
	dmaRx1ConfigFlag = 0;
	//DEBUG_OUT_DIO5(0);	//ToDo remove, debug only

	//DEBUG_OUT_DIO6(0);	//ToDo remove, debug only

	return 0;
}

//Sends a string via RS-485 #2 (USART6)
void puts_rs485_2(uint8_t *str, uint16_t length)
{
	unsigned int i = 0;
	uint8_t *uart6_dma_buf_ptr;
	uart6_dma_buf_ptr = (uint8_t*) &uart6_dma_tx_buf;

	//Transmit enable
	rs485_set_mode(PORT_RS485_2, RS485_TX);

	//Copy str to tx buffer:
	memcpy(uart6_dma_tx_buf, str, length);

	//ToDo replace by valid delay function!
	for(i = 0; i < 1000; i++);

	//Send data
	HAL_USART_Transmit_DMA(&husart6, uart6_dma_buf_ptr, length);
}

//Prepares the board for a Reply (reception). Blocking.
//ToDo: add timeout
uint8_t reception_rs485_2_blocking(void)
{
	int delay = 0;

	//Pointer to our storage buffer:
	uint32_t *uart6_dma_buf_ptr;
	uart6_dma_buf_ptr = (uint32_t*) &uart6_dma_rx_buf;
	uint32_t tmp = 0;

	//Do not enable if still transmitting:
	while(husart1.State == HAL_USART_STATE_BUSY_TX);
	for(delay = 0; delay < 600; delay++);		//Short delay

	//Receive enable
	rs485_set_mode(PORT_RS485_2, RS485_RX);
	tmp = USART6->DR;	//Read buffer to clear

	//Start the DMA peripheral
	dmaRx2ConfigFlag = 1;
	HAL_DMA_Start_IT(&hdma2_str1_ch5, (uint32_t) &USART6->DR,
			(uint32_t) uart6_dma_buf_ptr, rs485_2_dma_xfer_len);
	dmaRx2ConfigFlag = 0;

	return 0;
}

//Sends a string via the expansion's port UART (USART3)
//Note: this is sending via interrupt, but not using the DMA
void puts_expUart(uint8_t *str, uint16_t length)
{
	unsigned int i = 0;
	uint8_t *uart3_dma_buf_ptr;
	uart3_dma_buf_ptr = (uint8_t*) &uart3_dma_tx_buf;

	//Copy str to tx buffer:
	memcpy(uart3_dma_tx_buf, str, length);

	//ToDo replace by valid delay function!
	for(i = 0; i < 1000; i++);

	//Send data via DMA:
	HAL_USART_Transmit_DMA(&husart3, uart3_dma_buf_ptr, length);
}

//Function called after a completed DMA transfer, UART1 RX
void DMA2_Str2_CompleteTransfer_Callback(DMA_HandleTypeDef *hdma)
{
	if(hdma->Instance == DMA2_Stream2)
	{
		//Clear the UART receiver. Might not be needed, but harmless
		//empty_dr = USART1->DR;

		//A false interrupt is generated when we configure the DMA.
		//Return when it happens.
		if(dmaRx1ConfigFlag == 1)
		{
			return;
		}
	}

	//Deal with FlexSEA buffers here:
	update_rx_buf_array_485_1(uart1_dma_rx_buf, rs485_1_dma_xfer_len);
	//Empty DMA buffer once it's copied:
	memset(uart1_dma_rx_buf, 0, rs485_1_dma_xfer_len);
	commPeriph[PORT_RS485_1].rx.bytesReadyFlag++;
}

//Code branches here once a TX transfer is complete (either: ISR or DMA)
void HAL_USART_TxCpltCallback(USART_HandleTypeDef *husart)
{
	Port transceiver = 0;
	//Reset state to Ready:
	husart->State = HAL_USART_STATE_READY;

	//Ready to start receiving?
	if(husart->Instance == USART1)
	{
		//DEBUG_OUT_DIO4(0);	//ToDo remove, debug only
		transceiver = PORT_RS485_1;
	}
	else if(husart->Instance == USART6)
	{
		transceiver = PORT_RS485_2;
	}
	else
	{
		return;
	}

	//Change state:
	if(commPeriph[transceiver].transState == TS_TRANSMIT_THEN_RECEIVE)
	{
		commPeriph[transceiver].transState = TS_PREP_TO_RECEIVE;
	}
}

//Function called after a completed DMA transfer, UART6 RX
void DMA2_Str1_CompleteTransfer_Callback(DMA_HandleTypeDef *hdma)
{
	if(hdma->Instance == DMA2_Stream1)
	{
		//Clear the UART receiver. Might not be needed, but harmless
		//empty_dr = USART6->DR;

		//A false interrupt is generated when we configure the DMA.
		//Return when it happens.
		if(dmaRx2ConfigFlag == 1)
		{
			return;
		}
	}

	//Deal with FlexSEA buffers here:
	update_rx_buf_array_485_2(uart6_dma_rx_buf, rs485_2_dma_xfer_len);
	//Empty DMA buffer once it's copied:
	memset(uart6_dma_rx_buf, 0, rs485_2_dma_xfer_len);
	commPeriph[PORT_RS485_2].rx.bytesReadyFlag++;
}

//Function called after a completed DMA transfer, UART3 RX
void DMA1_Str1_CompleteTransfer_Callback(DMA_HandleTypeDef *hdma)
{
	if(hdma->Instance == DMA1_Stream1)
	{
		//Clear the UART receiver. Might not be needed, but harmless
		//empty_dr = USART1->DR;
	}

	//Deal with FlexSEA buffers here:
	update_rx_buf_array_wireless(uart3_dma_rx_buf, uart3_dma_xfer_len);
	//Empty DMA buffer once it's copied:
	memset(uart3_dma_rx_buf, 0, uart3_dma_xfer_len);
	//masterComm[2].rx.bytesReady++;
	commPeriph[PORT_WIRELESS].rx.bytesReadyFlag++;
}

//****************************************************************************
// Private Function(s)
//****************************************************************************

void HAL_USART_MspInit(USART_HandleTypeDef* husart)
{
	if(husart->Instance == USART1)		//RS-485
	{
		//Peripheral clock enable:
		__USART1_CLK_ENABLE();
		__GPIOA_CLK_ENABLE();

		//GPIOs:
		//PA9   ------> USART1_TX
		//PA10   ------> USART1_RX

		GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;	//Transceiver's R is Hi-Z when !RE=1
		GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
		GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	}
	else if(husart->Instance == USART6)	//Execute
	{
		//Peripheral clock enable:
		__USART6_CLK_ENABLE();
		__GPIOG_CLK_ENABLE();

		//GPIOs:
		//PG14  ------> USART6_TX
		//PG9   ------> USART6_RX

		GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_14;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;	//Transceiver's R is Hi-Z when !RE=1
		GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
		GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
		HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
	}
	else if(husart->Instance == USART3)	//Expansion port
	{
		//Peripheral clock enable:
		__USART3_CLK_ENABLE();
		__GPIOB_CLK_ENABLE();

		//GPIOs:
		//PB10  ------> USART3_TX
		//PB11  ------> USART3_RX
		//PB13  ------> USART3_CTS
		//PB14  ------> USART3_RTS

		GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
		GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
		GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	}
	else
	{
		//Invalid peripheral
		flexsea_error(SE_INVALID_USART);
	}
}

//Using DMA2 Ch 4 Stream 2 for USART1 RX
static void init_dma2_stream2_ch4(void)
{
	//Pointer to our storage buffer:
	uint32_t *uart1_dma_buf_ptr;
	uart1_dma_buf_ptr = (uint32_t*) &uart1_dma_rx_buf;

	//Enable clock
	__DMA2_CLK_ENABLE();

	//Instance:
	hdma2_str2_ch4.Instance = DMA2_Stream2;

	//Initialization:
	hdma2_str2_ch4.Init.Channel = DMA_CHANNEL_4;
	hdma2_str2_ch4.Init.Direction = DMA_PERIPH_TO_MEMORY; 	//Receive, so Periph to Mem
	hdma2_str2_ch4.Init.PeriphInc = DMA_PINC_DISABLE;		//No Periph increment
	hdma2_str2_ch4.Init.MemInc = DMA_MINC_ENABLE;			//Memory increment
	hdma2_str2_ch4.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;	//Align: bytes
	hdma2_str2_ch4.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;		//Align: bytes
	hdma2_str2_ch4.Init.Mode = DMA_CIRCULAR;
	hdma2_str2_ch4.Init.Priority = DMA_PRIORITY_MEDIUM;
	hdma2_str2_ch4.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	hdma2_str2_ch4.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
	hdma2_str2_ch4.Init.MemBurst = DMA_MBURST_SINGLE;
	hdma2_str2_ch4.Init.PeriphBurst = DMA_PBURST_SINGLE;

	hdma2_str2_ch4.XferCpltCallback = DMA2_Str2_CompleteTransfer_Callback;

	HAL_DMA_Init(&hdma2_str2_ch4);

	//Interrupts:
	HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, DMA_STR2_IRQ_CHANNEL,
			DMA_STR2_IRQ_SUBCHANNEL);
	HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
	__HAL_DMA_ENABLE_IT(&hdma2_str2_ch4, DMA_IT_TC);

	//Start the DMA peripheral
	HAL_DMA_Start_IT(&hdma2_str2_ch4, (uint32_t) &USART1->DR,
			(uint32_t) uart1_dma_buf_ptr, rs485_1_dma_xfer_len);
}

//Using DMA2 Ch 5 Stream 1 for USART6 RX
static void init_dma2_stream1_ch5(void)
{
	//Pointer to our storage buffer:
	uint32_t *uart6_dma_buf_ptr;
	uart6_dma_buf_ptr = (uint32_t*) &uart6_dma_rx_buf;

	//Enable clock
	__DMA2_CLK_ENABLE();

	//Instance:
	hdma2_str1_ch5.Instance = DMA2_Stream1;

	//Initialization:
	hdma2_str1_ch5.Init.Channel = DMA_CHANNEL_5;
	hdma2_str1_ch5.Init.Direction = DMA_PERIPH_TO_MEMORY; 	//Receive, so Periph to Mem
	hdma2_str1_ch5.Init.PeriphInc = DMA_PINC_DISABLE;		//No Periph increment
	hdma2_str1_ch5.Init.MemInc = DMA_MINC_ENABLE;			//Memory increment
	hdma2_str1_ch5.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;	//Align: bytes
	hdma2_str1_ch5.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;		//Align: bytes
	hdma2_str1_ch5.Init.Mode = DMA_CIRCULAR;
	hdma2_str1_ch5.Init.Priority = DMA_PRIORITY_MEDIUM;
	hdma2_str1_ch5.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	hdma2_str1_ch5.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
	hdma2_str1_ch5.Init.MemBurst = DMA_MBURST_SINGLE;
	hdma2_str1_ch5.Init.PeriphBurst = DMA_PBURST_SINGLE;

	hdma2_str1_ch5.XferCpltCallback = DMA2_Str1_CompleteTransfer_Callback;

	HAL_DMA_Init(&hdma2_str1_ch5);

	//Interrupts:
	HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, DMA_STR1_IRQ_CHANNEL,
			DMA_STR1_IRQ_SUBCHANNEL);
	HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
	__HAL_DMA_ENABLE_IT(&hdma2_str1_ch5, DMA_IT_TC);

	//Start the DMA peripheral
	HAL_DMA_Start_IT(&hdma2_str1_ch5, (uint32_t) &USART6->DR,
			(uint32_t) uart6_dma_buf_ptr, rs485_2_dma_xfer_len);
}

//Using DMA1 Ch 4 Stream 1 for USART3 RX
static void init_dma1_stream1_ch4(void)
{
	//Pointer to our storage buffer:
	uint32_t *uart3_dma_buf_ptr;
	uart3_dma_buf_ptr = (uint32_t*) &uart3_dma_rx_buf;

	//Enable clock
	__DMA1_CLK_ENABLE();

	//Initialization:
	hdma1_str1_ch4.Instance = DMA1_Stream1;
	hdma1_str1_ch4.Init.Channel = DMA_CHANNEL_4;
	hdma1_str1_ch4.Init.Direction = DMA_PERIPH_TO_MEMORY; 	//Receive, so Periph to Mem
	hdma1_str1_ch4.Init.PeriphInc = DMA_PINC_DISABLE;		//No Periph increment
	hdma1_str1_ch4.Init.MemInc = DMA_MINC_ENABLE;			//Memory increment
	hdma1_str1_ch4.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;	//Align: bytes
	hdma1_str1_ch4.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;		//Align: bytes
	hdma1_str1_ch4.Init.Mode = DMA_CIRCULAR;
	hdma1_str1_ch4.Init.Priority = DMA_PRIORITY_MEDIUM;
	hdma1_str1_ch4.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	hdma1_str1_ch4.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
	hdma1_str1_ch4.Init.MemBurst = DMA_MBURST_SINGLE;
	hdma1_str1_ch4.Init.PeriphBurst = DMA_PBURST_SINGLE;

	hdma1_str1_ch4.XferCpltCallback = DMA1_Str1_CompleteTransfer_Callback;

	HAL_DMA_Init(&hdma1_str1_ch4);

	//Interrupts:
	HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 0, 0);	//ToDo adjust
	HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
	__HAL_DMA_ENABLE_IT(&hdma1_str1_ch4, DMA_IT_TC);

	//Start the DMA peripheral
	HAL_DMA_Start_IT(&hdma1_str1_ch4, (uint32_t) &USART3->DR,
			(uint32_t) uart3_dma_buf_ptr, uart3_dma_xfer_len);
}

//Using DMA2 Ch 4 Stream 7 for USART1 TX
static void init_dma2_stream7_ch4(void)
{
	//Pointer to our storage buffer:
	uint32_t *uart1_dma_buf_ptr;
	uart1_dma_buf_ptr = (uint32_t*) &uart1_dma_tx_buf;

	//Enable clock
	__DMA2_CLK_ENABLE();

	//Initialization:
	hdma2_str7_ch4.Instance = DMA2_Stream7;
	hdma2_str7_ch4.Init.Channel = DMA_CHANNEL_4;
	hdma2_str7_ch4.Init.Direction = DMA_MEMORY_TO_PERIPH; 	//Transmit, so Mem to Periph
	hdma2_str7_ch4.Init.PeriphInc = DMA_PINC_DISABLE;		//No Periph increment
	hdma2_str7_ch4.Init.MemInc = DMA_MINC_ENABLE;			//Memory increment
	hdma2_str7_ch4.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;	//Align: bytes
	hdma2_str7_ch4.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;		//Align: bytes
	hdma2_str7_ch4.Init.Mode = DMA_NORMAL;
	hdma2_str7_ch4.Init.Priority = DMA_PRIORITY_MEDIUM;

	//Link DMA handle and UART TX:
	husart1.hdmatx = &hdma2_str7_ch4;
	//husart1 is the parent:
	husart1.hdmatx->Parent = &husart1;

	HAL_DMA_Init(husart1.hdmatx);

	//Interrupts:
	HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, DMA_STR7_IRQ_CHANNEL,
			DMA_STR7_IRQ_SUBCHANNEL);
	HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);
	__HAL_DMA_ENABLE_IT(husart1.hdmatx, DMA_IT_TC);
}

//Using DMA2 Ch 5 Stream 6 for USART6 TX
static void init_dma2_stream6_ch5(void)
{
	//Pointer to our storage buffer:
	uint32_t *uart6_dma_buf_ptr;
	uart6_dma_buf_ptr = (uint32_t*) &uart6_dma_tx_buf;

	//Enable clock
	__DMA2_CLK_ENABLE();

	//Initialization:
	hdma2_str6_ch5.Instance = DMA2_Stream6;
	hdma2_str6_ch5.Init.Channel = DMA_CHANNEL_5;
	hdma2_str6_ch5.Init.Direction = DMA_MEMORY_TO_PERIPH; 	//Transmit, so Mem to Periph
	hdma2_str6_ch5.Init.PeriphInc = DMA_PINC_DISABLE;		//No Periph increment
	hdma2_str6_ch5.Init.MemInc = DMA_MINC_ENABLE;			//Memory increment
	hdma2_str6_ch5.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;	//Align: bytes
	hdma2_str6_ch5.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;	//Align: bytes
	hdma2_str6_ch5.Init.Mode = DMA_NORMAL;
	hdma2_str6_ch5.Init.Priority = DMA_PRIORITY_MEDIUM;

	//Link DMA handle and UART TX:
	husart6.hdmatx = &hdma2_str6_ch5;
	//husart1 is the parent:
	husart6.hdmatx->Parent = &husart6;

	HAL_DMA_Init(&hdma2_str6_ch5);

	//Interrupts:
	HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, DMA_STR6_IRQ_CHANNEL,
			DMA_STR6_IRQ_SUBCHANNEL);
	HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);
	__HAL_DMA_ENABLE_IT(husart6.hdmatx, DMA_IT_TC);
}

//Using DMA1 Ch 4 Stream 1 for USART3 TX
static void init_dma1_stream3_ch4(void)
{
	//Pointer to our storage buffer:
	uint32_t *uart3_dma_buf_ptr;
	uart3_dma_buf_ptr = (uint32_t*) &uart3_dma_tx_buf;

	//Enable clock
	__DMA1_CLK_ENABLE();

	//Initialization:
	hdma1_str3_ch4.Instance = DMA1_Stream3;
	hdma1_str3_ch4.Init.Channel = DMA_CHANNEL_4;
	hdma1_str3_ch4.Init.Direction = DMA_MEMORY_TO_PERIPH; 	//Transmit, so Mem to Periph
	hdma1_str3_ch4.Init.PeriphInc = DMA_PINC_DISABLE;		//No Periph increment
	hdma1_str3_ch4.Init.MemInc = DMA_MINC_ENABLE;			//Memory increment
	hdma1_str3_ch4.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;	//Align: bytes
	hdma1_str3_ch4.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;		//Align: bytes
	hdma1_str3_ch4.Init.Mode = DMA_NORMAL;
	hdma1_str3_ch4.Init.Priority = DMA_PRIORITY_MEDIUM;

	//Link DMA handle and UART TX:
	husart3.hdmatx = &hdma1_str3_ch4;
	//husart1 is the parent:
	husart3.hdmatx->Parent = &husart3;

	HAL_DMA_Init(husart3.hdmatx);

	//Interrupts:
	HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 0, 0);	//ToDo adjust priority
	HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);
	__HAL_DMA_ENABLE_IT(husart3.hdmatx, DMA_IT_TC);
}
