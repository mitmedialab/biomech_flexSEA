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
	[This file] fm_slave_comm: Slave R/W
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-23 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

//****************************************************************************
// Include(s)
//****************************************************************************
#include "flexsea_buffers.h"
#include "main.h"
#include <flexsea_payload.h>
#include <flexsea_board.h>
#include <master_slave_comm.h>
#include <stdbool.h>
#include "spi.h"

//****************************************************************************
// Variable(s)
//****************************************************************************

//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************

//****************************************************************************
// Public Function(s)
//****************************************************************************

//Prepares the structures:
void initMasterSlaveComm(void)
{
	//Default state:
	initCommPeriph(&commPeriph[PORT_USB], PORT_USB, MASTER, rx_buf_4, \
			comm_str_4, rx_command_4, &rx_buf_circ_4, \
			&packet[PORT_USB][INBOUND], &packet[PORT_USB][OUTBOUND]);
	initCommPeriph(&commPeriph[PORT_RS485_1], PORT_RS485_1, SLAVE, rx_buf_1, \
			comm_str_1, rx_command_1, &rx_buf_circ_1, \
			&packet[PORT_RS485_1][INBOUND], &packet[PORT_RS485_1][OUTBOUND]);
	initCommPeriph(&commPeriph[PORT_RS485_2], PORT_RS485_2, SLAVE, rx_buf_2, \
			comm_str_2, rx_command_2, &rx_buf_circ_2, \
			&packet[PORT_RS485_2][INBOUND], &packet[PORT_RS485_2][OUTBOUND]);

	//SPI:
	initCommPeriph(&commPeriph[PORT_SPI], PORT_SPI, MASTER, rx_buf_3, \
				comm_str_3, rx_command_3, &rx_buf_circ_3, \
				&packet[PORT_SPI][INBOUND], &packet[PORT_SPI][OUTBOUND]);

	//Bluetooth:
	initCommPeriph(&commPeriph[PORT_WIRELESS], PORT_WIRELESS, MASTER, rx_buf_5, \
				comm_str_5, rx_command_5, &rx_buf_circ_5, \
				&packet[PORT_WIRELESS][INBOUND], &packet[PORT_WIRELESS][OUTBOUND]);
}

//Did we receive new commands? Can we parse them?
void parseMasterCommands(uint8_t *new_cmd)
{
	uint8_t parseResult = 0, newCmdLed = 0;

	//USB
	if(commPeriph[PORT_USB].rx.unpackedPacketsAvailable > 0)
	{
		commPeriph[PORT_USB].rx.unpackedPacketsAvailable = 0;
		parseResult = payload_parse_str(&packet[PORT_USB][INBOUND]);
		newCmdLed += (parseResult == PARSE_SUCCESSFUL) ? 1 : 0;
	}

	//SPI
	if(commPeriph[PORT_SPI].rx.unpackedPacketsAvailable > 0)
	{
		commPeriph[PORT_SPI].rx.unpackedPacketsAvailable = 0;
		parseResult = payload_parse_str(&packet[PORT_SPI][INBOUND]);
		newCmdLed += (parseResult == PARSE_SUCCESSFUL) ? 1 : 0;
		spi4Watch = 0;	//Valid packets restart the count
	}
	else
	{
		//Getting many SPI transactions but no packets is a sign that something is wrong
		if(spi4Watch > 5)
		{
			//After 5 SPI transfers with 0 packets we restart the peripheral:
			restartSpi(4);
		}
	}

	//Wireless
	if(commPeriph[PORT_WIRELESS].rx.unpackedPacketsAvailable > 0)
	{
		commPeriph[PORT_WIRELESS].rx.unpackedPacketsAvailable = 0;
		parseResult = payload_parse_str(&packet[PORT_WIRELESS][INBOUND]);
		newCmdLed += (parseResult == PARSE_SUCCESSFUL) ? 1 : 0;
	}

	if(newCmdLed > 0) {*new_cmd = 1;}
}

//Did we receive new commands? Can we parse them?
void parseSlaveCommands(uint8_t *new_cmd)
{
	//Valid communication from RS-485 #1?
	if(commPeriph[PORT_RS485_1].rx.unpackedPacketsAvailable > 0)
	{
		commPeriph[PORT_RS485_1].rx.unpackedPacketsAvailable = 0;
		payload_parse_str(&packet[PORT_RS485_1][INBOUND]);
	}

	//Valid communication from RS-485 #2?
	if(commPeriph[PORT_RS485_2].rx.unpackedPacketsAvailable > 0)
	{
		commPeriph[PORT_RS485_2].rx.unpackedPacketsAvailable = 0;
		payload_parse_str(&packet[PORT_RS485_2][INBOUND]);
	}
}

//Slave Communication function. Call at 1kHz.
void slaveTransmit(Port port)
{
	/*Note: this is only a demonstration. In the final application, we want
			 * to send the commands accumulated on a ring buffer here.*/
	PacketWrapper *p;

	if((port == PORT_RS485_1) || (port == PORT_RS485_2))
	{
		p = &packet[port][OUTBOUND];

		if(commPeriph[port].tx.packetReady == 1)
		{
			commPeriph[port].tx.packetReady = 0;

			if(IS_CMD_RW(p->cmd) == READ)
			{
				commPeriph[port].transState = TS_TRANSMIT_THEN_RECEIVE;
			}
			else
			{
				commPeriph[port].transState = TS_TRANSMIT;
			}

			flexsea_send_serial_slave(p);
		}
	}
}

//****************************************************************************
// Private Function(s)
//****************************************************************************
