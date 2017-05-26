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
	[This file] flexsea_board: configuration and functions for this
	particular board
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-23 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

//****************************************************************************
// Include(s)
//****************************************************************************

#include "main.h"
#include "flexsea_board.h"
#include "../../flexsea-system/inc/flexsea_system.h"
#include <flexsea_comm.h>
#include <flexsea_payload.h>
#include <spi.h>
#include <uarts.h>
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "fm_stm32f4xx_hal_spi.h"

//****************************************************************************
// Variable(s)
//****************************************************************************

//<FlexSEA User>
//==============
//Board architecture. Has to be changed in all the flexsea_board files!

uint8_t board_id = FLEXSEA_MANAGE_1;		//This board
uint8_t board_up_id = FLEXSEA_PLAN_1;		//This board's master

//Slave bus #1 (RS-485 #1):
//=========================
uint8_t board_sub1_id[SLAVE_BUS_1_CNT] = {FLEXSEA_EXECUTE_2, FLEXSEA_EXECUTE_4};

//Slave bus #2 (RS-485 #2):
//=========================
uint8_t board_sub2_id[SLAVE_BUS_2_CNT] = {FLEXSEA_EXECUTE_1, FLEXSEA_EXECUTE_3};

uint8_t board_sub3_id[SLAVE_BUS_3_CNT] = {FLEXSEA_MANAGE_1};

//(make sure to update SLAVE_BUS_x_CNT in flexsea_board.h!)

//===============
//</FlexSEA User>

//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************

static void transitionToReception(CommPeriph *cp, uint8_t (*f)(void));

//****************************************************************************
// Function(s)
//****************************************************************************

//Wrapper for the specific serial functions. Useful to keep flexsea_network
//platform independent (for example, we don't need need puts_rs485() for Plan)
void flexsea_send_serial_slave(PacketWrapper* p)
{
	uint8_t port = p->destinationPort;
	uint8_t* str = p->packed;
	uint16_t length = COMM_STR_BUF_LEN;

	//If it's a valid slave port, send message...
	if(port == PORT_RS485_1)
	{
		puts_rs485_1(str, length);
	}
	else if(port == PORT_RS485_2)
	{
		puts_uart_ex(str, length);
	}
	else
	{
		//Unknown port, call flexsea_error()
		flexsea_error(SE_INVALID_SLAVE);
		return;
	}

	//...then take care of the transceiver state to allow reception (if needed)
	if(IS_CMD_RW(p->cmd))
	{
		commPeriph[port].transState = TS_TRANSMIT_THEN_RECEIVE;
	}
	else
	{
		commPeriph[port].transState = TS_TRANSMIT;
	}
}

void flexsea_send_serial_master(PacketWrapper* p)
{
	Port port = p->destinationPort;
	uint8_t *str = p->packed;
	uint16_t length = COMM_STR_BUF_LEN;
	uint8_t retVal = 0;

	if(port == PORT_SPI)
	{
		commPeriph[PORT_SPI].tx.packetReady = 1;
		//This will be sent during the next SPI transaction
		memcpy(comm_str_spi, str, length);
	}
	else if(port == PORT_USB)
	{
		retVal = CDC_Transmit_FS(str, length);
		if(retVal != USBD_OK)
		{
			//(Handle errors here)
		}
	}
	else if(port == PORT_WIRELESS)
	{
		puts_expUart(str, length);
	}
}

void flexsea_receive_from_master(void)
{
	//USB:
	commPeriph[PORT_USB].rx.unpackedPacketsAvailable = tryParseRx(&commPeriph[PORT_USB], &packet[PORT_USB][INBOUND]);

	//SPI:
	//commPeriph[PORT_SPI].rx.unpackedPacketsAvailable = tryUnpacking(&commPeriph[PORT_SPI], &packet[PORT_SPI][INBOUND]);	//Legacy
	commPeriph[PORT_SPI].rx.unpackedPacketsAvailable = tryParseRx(&commPeriph[PORT_SPI], &packet[PORT_SPI][INBOUND]);		//Circular buffer

	//Incomplete, ToDo (flag won't be raised)
	tryUnpacking(&commPeriph[PORT_WIRELESS], &packet[PORT_WIRELESS][INBOUND]);
}

void flexsea_start_receiving_from_master(void)
{
	// start receive over SPI
	if (HAL_SPI_GetState(&spi4_handle) == HAL_SPI_STATE_READY)
	{
		if(HAL_SPI_TransmitReceive_IT(&spi4_handle, (uint8_t *)aTxBuffer4, (uint8_t *)aRxBuffer4, COMM_STR_BUF_LEN) != HAL_OK)
		{
			// Transfer error in transmission process
			flexsea_error(SE_RECEIVE_FROM_MASTER);
		}
	}
}

//Receive data from a slave
void flexsea_receive_from_slave(void)
{
	//Transceiver state:
	//==================
	transitionToReception(&commPeriph[PORT_RS485_1], \
			reception_rs485_1_blocking);
	transitionToReception(&commPeriph[PORT_RS485_2], \
				reception_uart_ex_blocking);

	//Did we get new bytes?
	//=====================
	//tryUnpacking(&commPeriph[PORT_RS485_1], &packet[PORT_RS485_1][INBOUND]);
	//tryUnpacking(&commPeriph[PORT_RS485_2], &packet[PORT_RS485_2][INBOUND]);

	commPeriph[PORT_RS485_1].rx.unpackedPacketsAvailable = tryParseRx(&commPeriph[PORT_RS485_1], &packet[PORT_RS485_1][INBOUND]);		//Circular buffer
	commPeriph[PORT_RS485_2].rx.unpackedPacketsAvailable = tryParseRx(&commPeriph[PORT_RS485_2], &packet[PORT_RS485_2][INBOUND]);		//Circular buffer

	commPeriph[PORT_EXP].rx.unpackedPacketsAvailable = tryParseRx(&commPeriph[PORT_EXP], &packet[PORT_EXP][INBOUND]);		//Circular buffer
}

uint8_t getBoardID(void)
{
	return board_id;
}

uint8_t getBoardUpID(void)
{
	return board_up_id;
}

uint8_t getBoardSubID(uint8_t sub, uint8_t idx)
{
	if(sub == 0){return board_sub1_id[idx];}
	else if(sub == 1){return board_sub2_id[idx];}
	else if(sub == 2){return board_sub3_id[idx];}

	return 0;
}

uint8_t getSlaveCnt(uint8_t sub)
{
	if(sub == 0){return SLAVE_BUS_1_CNT;}
	else if(sub == 1){return SLAVE_BUS_2_CNT;}
	else if(sub == 2){return SLAVE_BUS_3_CNT;}

	return 0;
}

//****************************************************************************
// Private Function(s)
//****************************************************************************

//Special function for the RS-485 reception and transceiver state management
static void transitionToReception(CommPeriph *cp, uint8_t (*f)(void))
{
	//We only listen if we requested a reply:
	if(cp->transState == TS_PREP_TO_RECEIVE)
	{
		cp->transState = TS_RECEIVE;

		f();	//Sets the transceiver to Receive
		//From this point on data will be received via the interrupt.
	}
}
