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
#include <flexsea_comm.h>
#include <uarts.h>
#include "flexsea_sys_def.h"
#include "flexsea_board.h"
#include "isr.h"
#include "timer.h"
#include "stm32f4xx_hal_def.h"

//****************************************************************************
// Variable(s)
//****************************************************************************

USART_HandleTypeDef husart1;		//RS-485 #1
USART_HandleTypeDef husart6;		//RS-485 #2
USART_HandleTypeDef husart3;		//Expansion port
GPIO_InitTypeDef GPIO_InitStruct;
DMA_HandleTypeDef hdma2_str2_ch4;	//DMA for RS-485 #1 RX
DMA_HandleTypeDef hdma2_str7_ch4;	//DMA for RS-485 #1 TX
DMA_HandleTypeDef hdma2_str1_ch5;	//DMA for UART-Execute RX
DMA_HandleTypeDef hdma2_str6_ch5;	//DMA for UART-Execute TX
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
//static void init_dma2_stream7_ch4(void);
static void init_dma2_stream7_ch4(USART_HandleTypeDef *usartHandle);
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
	HAL_NVIC_SetPriority(USART1_IRQn, ISR_UART1, ISR_SUB_UART1);
	//HAL_NVIC_EnableIRQ(USART1_IRQn);	//(All done via DMA)

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
	init_dma2_stream7_ch4(&husart1);	//TX
}

//USART6 init function: UART, Execute
//TX and RX are done via DMA
void init_usart6(uint32_t baudrate)
{
	husart6.Instance = USART6;

	//MSP Init (enables clock, GPIOs)
	HAL_USART_MspInit(&husart6);

	//Interrupts:
	HAL_NVIC_SetPriority(USART6_IRQn, ISR_UART6, ISR_SUB_UART6);
	//HAL_NVIC_EnableIRQ(USART6_IRQn);	//(All done via DMA)

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
	HAL_NVIC_SetPriority(USART3_IRQn, ISR_UART3, ISR_SUB_UART3);
	//HAL_NVIC_EnableIRQ(USART3_IRQn);

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
	//USART3->CR3 |= USART_CR3_CTSE;		//Enable CTS
	//USART3->CR3 |= USART_CR3_RTSE;		//Enable RTS

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

	//Transmit enable
	rs485_set_mode(PORT_RS485_1, RS485_TX);

	//Copy str to tx buffer:
	memcpy(uart1_dma_tx_buf, str, length);

	//ToDo replace by valid delay function!
	for(i = 0; i < 1000; i++);

	//Send data via DMA:
	__HAL_DMA_CLEAR_FLAG(husart1.hdmatx, DMA_FLAG_TCIF3_7 | DMA_FLAG_DMEIF3_7 | \
					DMA_FLAG_TEIF3_7 | DMA_FLAG_HTIF3_7 | DMA_FLAG_TCIF3_7);
	HAL_USART_Transmit_DMA(&husart1, uart1_dma_buf_ptr, length);
}

//Prepares the board for a Reply (reception). Blocking.
//ToDo: add timeout
uint8_t reception_rs485_1_blocking(void)
{
	int delay = 0;

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
	dmaRx1ConfigFlag = 1;
	HAL_DMA_Start_IT(&hdma2_str2_ch4, (uint32_t) &USART1->DR,
			(uint32_t) uart1_dma_buf_ptr, rs485_1_dma_xfer_len);
	dmaRx1ConfigFlag = 0;

	return 0;
}

//Sends a string via UART (USART6)
void puts_uart_ex(uint8_t *str, uint16_t length)
{
	unsigned int i = 0;
	uint8_t *uart6_dma_buf_ptr;
	static uint32_t errCnt = 0;
	uart6_dma_buf_ptr = (uint8_t*) &uart6_dma_tx_buf;

	//Copy str to tx buffer:
	memcpy(uart6_dma_tx_buf, str, length);

	//ToDo replace by valid delay function!
	for(i = 0; i < 1000; i++);

	//Send data
	//HAL_USART_Transmit_DMA(&husart6, uart6_dma_buf_ptr, length);

	//Send data via DMA:
	if(HAL_USART_Transmit_DMA(&husart6, uart6_dma_buf_ptr, length) != HAL_OK)
	{
		errCnt++;
	}

	if(errCnt > 100)
	{
		//Something major is going on...
		//ToDo
		CLEAR_BIT(husart6.Instance->CR1, (USART_CR1_TXEIE | USART_CR1_TCIE));
		  /* At end of Tx process, restore husart->State to Ready */
		husart6.State = HAL_USART_STATE_READY;
	}

	if(errCnt > 110)
	{
		errCnt = 0;
	}
}

//Prepares the board for a Reply (reception). Blocking.
//ToDo: add timeout
uint8_t reception_uart_ex_blocking(void)
{
	int delay = 0;

	//Pointer to our storage buffer:
	uint32_t *uart6_dma_buf_ptr;
	uart6_dma_buf_ptr = (uint32_t*) &uart6_dma_rx_buf;
	uint32_t tmp = 0;

	//Do not enable if still transmitting:
	while(husart1.State == HAL_USART_STATE_BUSY_TX);	//ToDo why is that relating to UART1???
	for(delay = 0; delay < 600; delay++);		//Short delay

	//Receive enable
	tmp = USART6->DR;	//Read buffer to clear

	//Start the DMA peripheral
	dmaRx2ConfigFlag = 1;
	HAL_DMA_Start_IT(&hdma2_str1_ch5, (uint32_t) &USART6->DR,
			(uint32_t) uart6_dma_buf_ptr, rs485_2_dma_xfer_len);
	dmaRx2ConfigFlag = 0;

	return 0;
}

//Sends a string over Bluetooth (USART3)
void puts_expUart(uint8_t *str, uint16_t length)
{
	unsigned int i = 0;
	uint8_t *uart3_dma_buf_ptr;
	static uint32_t errCnt = 0;
	uart3_dma_buf_ptr = (uint8_t*) &uart3_dma_tx_buf;

	//Copy str to tx buffer:
	memcpy(uart3_dma_tx_buf, str, length);

	//ToDo replace by valid delay function!
	for(i = 0; i < 1000; i++);

	//Send data via DMA:
	if(HAL_USART_Transmit_DMA(&husart3, uart3_dma_buf_ptr, length) != HAL_OK)
	{
		errCnt++;
	}

	if(errCnt > 100)
	{
		//Something major is going on...
		//ToDo
		CLEAR_BIT(husart3.Instance->CR1, (USART_CR1_TXEIE | USART_CR1_TCIE));
		  /* At end of Tx process, restore husart->State to Ready */
		husart3.State = HAL_USART_STATE_READY;

		//init_usart3(230400);	//Expansion port
		//BT_RST(0);

		//errCnt = 0;
	}

	if(errCnt > 110)
	{
		errCnt = 0;
	}
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
	update_rx_buf_485_1(uart1_dma_rx_buf, rs485_1_dma_xfer_len);			//Circular Buffer
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
		//A false interrupt is generated when we configure the DMA.
		//Return when it happens.
		if(dmaRx2ConfigFlag == 1)
		{
			return;
		}
	}

	//Deal with FlexSEA buffers here:
	update_rx_buf_uart(uart6_dma_rx_buf, rs485_2_dma_xfer_len);				//Circular Buffer
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
	//update_rx_buf_array_wireless(uart3_dma_rx_buf, uart3_dma_xfer_len);	//Legacy
	update_rx_buf_wireless(uart3_dma_rx_buf, uart3_dma_xfer_len);			//Circular Buffer
	//Empty DMA buffer once it's copied:
	memset(uart3_dma_rx_buf, 0, uart3_dma_xfer_len);
	commPeriph[PORT_WIRELESS].rx.bytesReadyFlag++;
}

//USART Error callback:
void HAL_USART_ErrorCallback(USART_HandleTypeDef *husart)
{
	if(husart->Instance == USART1)
	{
		//...
		//Test - doesn't seem to do much:
		if(husart->ErrorCode != HAL_USART_ERROR_NONE)
		{
			husart->ErrorCode = HAL_USART_ERROR_NONE;
			husart->State= HAL_USART_STATE_READY;
		}

		if(husart->hdmatx->State == HAL_DMA_STATE_ERROR || \
				husart->hdmatx->State == HAL_DMA_STATE_TIMEOUT)
		{
			husart->hdmatx->State = HAL_DMA_STATE_RESET;
		}

	}
	if(husart->Instance == USART3)
	{
		/*
		//...
		//Test - doesn't seem to do much:
		if(husart->ErrorCode != HAL_USART_ERROR_NONE)
		{
			husart->ErrorCode = HAL_USART_ERROR_NONE;
			husart->State= HAL_USART_STATE_READY;
		}

		if(husart->hdmatx->State == HAL_DMA_STATE_ERROR || \
				husart->hdmatx->State == HAL_DMA_STATE_TIMEOUT)
		{
			husart->hdmatx->State = HAL_DMA_STATE_RESET;
		}
*/
	}
	else if(husart->Instance == USART6)
	{
		//...
	}
}

//We have a packet ready, but we want to wait a little while before sending it
void rs485Transmit(PacketWrapper* p)
{
	if(p->destinationPort == PORT_RS485_1)
	{
		//packet[PORT_RS485_1][OUTBOUND] should already be filled

		commPeriph[PORT_RS485_1].tx.packetReady = 1;
		//We reply 2 slots later:
		commPeriph[PORT_RS485_1].tx.timeStamp = (tb_100us_timeshare + 2) % 10;

		//ToDo: make sure this isn't done somewhere else, or that this data isn't already in commPeriph
		memcpy(commPeriph[PORT_RS485_1].out->packed, p->packed, COMM_STR_BUF_LEN);
		commPeriph[PORT_RS485_1].out->numb = COMM_STR_BUF_LEN;
	}
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
	else if(husart->Instance == USART3)	//Bluetooth
	{
		//Peripheral clock enable:
		__USART3_CLK_ENABLE();
		__GPIOB_CLK_ENABLE();

		//GPIOs:
		//PB10  ------> USART3_TX
		//PB11  ------> USART3_RX
		//PB13  ------> USART3_CTS
		//PB14  ------> USART3_RTS

		/*
		GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
		GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_13;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = GPIO_PIN_14;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
		HAL_GPIO_WritePin(GPIOB, 1<<13, 0);

		GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
		GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		//Other Bluetooth pins:
		__GPIOA_CLK_ENABLE();

		//PA4  ------> Reset
		//PA5  ------> P4 (Factory reset)
		//PA6  ------> P5 (Connection LED)
		//PA7  ------> P8 (Comm LED)

		GPIO_InitStruct.Pin = GPIO_PIN_4;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;		//RN-42 has a PU, we match it
		GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		BT_RST(1);

		//P4 - Factory reset: we use input w/ PU to avoid messing with it
		GPIO_InitStruct.Pin = GPIO_PIN_5;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	}
	else
	{
		//Invalid peripheral
		flexsea_error(SE_INVALID_USART);
	}
}

void resetBluetooth(void)
{
	BT_RST(1);
	delayUsBlocking(10);
	BT_RST(0);
	delayUsBlocking(10);
	BT_RST(1);
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
	HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, ISR_DMA2_STREAM2, ISR_SUB_DMA2_STREAM2);
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
	HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, ISR_DMA2_STREAM1, ISR_SUB_DMA2_STREAM1);
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
	HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, ISR_DMA1_STREAM1, ISR_SUB_DMA1_STREAM1);
	HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
	__HAL_DMA_ENABLE_IT(&hdma1_str1_ch4, DMA_IT_TC);

	//Start the DMA peripheral
	HAL_DMA_Start_IT(&hdma1_str1_ch4, (uint32_t) &USART3->DR,
			(uint32_t) uart3_dma_buf_ptr, uart3_dma_xfer_len);
}

//Using DMA2 Ch 4 Stream 7 for USART1 TX
static void init_dma2_stream7_ch4(USART_HandleTypeDef *usartHandle)
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
	hdma2_str7_ch4.Init.Priority = DMA_PRIORITY_HIGH;
	hdma2_str7_ch4.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
	hdma2_str7_ch4.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
	hdma2_str7_ch4.Init.MemBurst = DMA_MBURST_SINGLE;
	hdma2_str7_ch4.Init.PeriphBurst = DMA_PBURST_SINGLE;

	//Link DMA handle and UART TX:
	husart1.hdmatx = &hdma2_str7_ch4;
	//husart1 is the parent:
	husart1.hdmatx->Parent = &husart1;

	//HAL_DMA_Init(husart1.hdmatx);
	HAL_DMA_Init(&hdma2_str7_ch4);

	//__HAL_LINKDMA(usartHandle, hdmatx, hdma2_str7_ch4);

	//Interrupts:
	HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, ISR_DMA2_STREAM7, ISR_SUB_DMA2_STREAM7);
	HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);
	__HAL_DMA_ENABLE_IT(husart1.hdmatx, DMA_IT_TC);	//??
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
	hdma2_str6_ch5.Init.Priority = DMA_PRIORITY_HIGH;
	hdma2_str6_ch5.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
	hdma2_str6_ch5.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
	hdma2_str6_ch5.Init.MemBurst = DMA_MBURST_SINGLE;
	hdma2_str6_ch5.Init.PeriphBurst = DMA_PBURST_SINGLE;

	//Link DMA handle and UART TX:
	husart6.hdmatx = &hdma2_str6_ch5;
	//husart1 is the parent:
	husart6.hdmatx->Parent = &husart6;

	HAL_DMA_Init(&hdma2_str6_ch5);

	//Interrupts:
	HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, ISR_DMA2_STREAM6, ISR_SUB_DMA2_STREAM6);
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
	HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, ISR_DMA1_STREAM3, ISR_SUB_DMA1_STREAM3);
	HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);
	__HAL_DMA_ENABLE_IT(husart3.hdmatx, DMA_IT_TC);
}
