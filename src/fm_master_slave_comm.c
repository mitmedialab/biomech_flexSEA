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
#include "fm_master_slave_comm.h"
//#include <fm_block_allocator.h>
#include <flexsea_payload.h>
#include <flexsea_board.h>
#include <stdbool.h>

//****************************************************************************
// Variable(s)
//****************************************************************************

uint8_t tmp_rx_command_spi[PAYLOAD_BUF_LEN];
uint8_t tmp_rx_command_usb[PAYLOAD_BUF_LEN];
uint8_t tmp_rx_command_wireless[PAYLOAD_BUF_LEN];
uint8_t tmp_rx_command_485_1[PAYLOAD_BUF_LEN];
uint8_t tmp_rx_command_485_2[PAYLOAD_BUF_LEN];

//MsgQueue slave_queue;
PacketWrapper PWpsc[2];
PacketWrapper PWst[2];

//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************

//****************************************************************************
// Public Function(s)
//****************************************************************************

void initSlaveComm(void)
{
	//...
	//fm_queue_init(&slave_queue, 10);
}

//Prepares the structures:
void init_master_slave_comm(void)
{
	//Legacy - remove soon:
	//=====================

	//Slave Port #1:
	slaveComm[0].port = PORT_RS485_1;
	slaveComm[0].rx.bytesReady = 0;
	slaveComm[0].rx.cmdReady = 0;
	slaveComm[0].rx.commStr = comm_str_485_1;
	//slaveComm[0].rx.rxBuf = rx_buf_1;
	slaveComm[0].rx.rxCmd = rx_command_485_1;

	//Slave Port #2:
	slaveComm[1].port = PORT_RS485_2;
	slaveComm[1].rx.bytesReady = 0;
	slaveComm[1].rx.cmdReady = 0;
	slaveComm[1].rx.commStr = comm_str_485_2;
	//slaveComm[0].rx.rxBuf = rx_buf_1;
	slaveComm[1].rx.rxCmd = rx_command_485_2;

	//Master Port #3:
	masterComm[2].port = PORT_WIRELESS;
	masterComm[2].rx.bytesReady = 0;
	masterComm[2].rx.cmdReady = 0;
	masterComm[2].rx.commStr = comm_str_wireless;
	//slaveComm[2].rx.rxBuf = rx_buf_1;
	masterComm[2].rx.rxCmd = rx_command_wireless;

	//New approach:
	//=============

	//Default state:
	initCommPeriph(&masterCommPeriph[0], PORT_USB, MASTER);
	initCommPeriph(&slaveCommPeriph[0], PORT_RS485_1, SLAVE);
	initCommPeriph(&slaveCommPeriph[1], PORT_RS485_2, SLAVE);

	//Personalize specific fields:
	//...
}

//Initialize CommPeriph to defaults:
void initCommPeriph(CommPeriph *cp, Port port, PortType pt)
{
	cp->port = port;
	cp->portType = pt;
	cp->transState = TS_UNKNOWN;

	cp->rx.bytesReadyFlag = 0;
	cp->rx.unpackedPacketsAvailable = 0;
	cp->rx.unpackedPtr = cp->rx.unpacked;
	cp->rx.packedPtr = cp->rx.packed;
	memset(cp->rx.packed, 0, COMM_PERIPH_ARR_LEN);
	memset(cp->rx.unpacked, 0, COMM_PERIPH_ARR_LEN);

	cp->tx.bytesReadyFlag = 0;
	cp->tx.unpackedPacketsAvailable = 0;
	cp->tx.unpackedPtr = cp->tx.unpacked;
	cp->tx.packedPtr = cp->tx.packed;
	memset(cp->tx.packed, 0, COMM_PERIPH_ARR_LEN);
	memset(cp->tx.unpacked, 0, COMM_PERIPH_ARR_LEN);
}

//Did we receive new commands? Can we parse them?
void parseMasterCommands(uint8_t *new_cmd)
{
	uint8_t info[2] = {0,0};

	/*
	//Valid communication from any port?
	PacketWrapper* p = fm_queue_get(&unpacked_packet_queue);
	while (p != NULL)
	{
		info[0] = p->port;
		payload_parse_str(p);

		//LED:
		*new_cmd = 1;
		p = fm_queue_get(&unpacked_packet_queue);
	}
	*/

	//ToDo *****IMPORTANT***** This is an old-school implementation,
	//and it's missing SPI & Bluetooth!

	/*
	if(cmd_ready_usb > 0)
	{
		//LED:
		*new_cmd = 1;
		cmd_ready_usb = 0;
		payload_parse_str(&freshUSBpacket);
	}
	*/
	if(masterCommPeriph[0].rx.unpackedPacketsAvailable > 0)
	{
		masterCommPeriph[0].rx.unpackedPacketsAvailable = 0;
		*new_cmd = 1;
		payload_parse_str(&masterInbound[0]);
	}
}

//Did we receive new commands? Can we parse them?
void parseSlaveCommands(uint8_t *new_cmd)
{
	//Valid communication from RS-485 #1?
	if(slaveComm[0].rx.cmdReady > 0)
	{
		slaveComm[0].rx.cmdReady = 0;
		/*
		PacketWrapper* p = fm_pool_allocate_block();
		if (p == NULL)
			return;

		memcpy(p->unpaked, &rx_command_485_1[0], COMM_STR_BUF_LEN);
		memcpy(p->packed, rx_buf_1, COMM_STR_BUF_LEN);
		*/
		/*
		//Cheap trick to get first line	//ToDo: support more than 1
		for(i = 0; i < PAYLOAD_BUF_LEN; i++)
		{
			tmp_rx_command_485_1[i] = rx_command_485_1[0][i];
		}*/

		/*
		p->port = slaveComm[0].reply_port;
		payload_parse_str(p);
		*/

		PWpsc[0].port = slaveComm[0].port;
		PWpsc[0].reply_port = slaveComm[0].reply_port;
		memcpy(PWpsc[0].unpaked, &rx_command_485_1[0], COMM_STR_BUF_LEN);
		memcpy(PWpsc[0].packed, rx_buf_1, COMM_STR_BUF_LEN);

		payload_parse_str(&PWpsc[0]);
	}

	//Valid communication from RS-485 #2?
	if(slaveComm[1].rx.cmdReady > 0)
	{
		slaveComm[1].rx.cmdReady = 0;
		/*
		PacketWrapper* p = fm_pool_allocate_block();
		if (p == NULL)
			return;

		memcpy(p->unpaked, &rx_command_485_2[0], COMM_STR_BUF_LEN);
		memcpy(p->packed, rx_buf_2, COMM_STR_BUF_LEN);
		// parse the command and execute it
		p->port = slaveComm[1].reply_port;
		payload_parse_str(p);
		*/

		PWpsc[1].port = slaveComm[1].port;
		PWpsc[1].reply_port = slaveComm[1].reply_port;
		memcpy(PWpsc[1].unpaked, &rx_command_485_2[0], COMM_STR_BUF_LEN);
		memcpy(PWpsc[1].packed, rx_buf_2, COMM_STR_BUF_LEN);

		payload_parse_str(&PWpsc[1]);
	}
}

//Slave Communication function. Call at 1kHz.
//ToDo: this ignores the parameter 'port'. It was there to offset the comm between the 2 buses.
/*
void slaveTransmit(uint8_t port)
{
	PacketWrapper* p = fm_queue_get(&slave_queue);

	if (p == NULL)
		return;

	//Send to slave port:
	if((p->port == PORT_RS485_1) || (p->port == PORT_RS485_2))
	{
		flexsea_send_serial_slave(p);
	}
}
*/
//Slave Communication function. Call at 1kHz.
void slaveTransmit(uint8_t port)
{
	/*Note: this is only a demonstration. In the final application, we want
			 * to send the commands accumulated on a ring buffer here.*/
	uint8_t slaveIndex = 0;
	PacketWrapper *p;

	if(port == PORT_RS485_1)
	{
		slaveIndex = 0;
	}
	else if(port == PORT_RS485_2)
	{
		slaveIndex = 1;
	}
	p = &slaveOutbound[slaveIndex];

	if(slaveCommPeriph[slaveIndex].tx.packetReady == 1)
	{
		slaveCommPeriph[slaveIndex].tx.packetReady = 0;

		if(IS_CMD_RW(p->cmd) == READ)
		{
			slaveCommPeriph[slaveIndex].transState = TS_TRANSMIT_THEN_RECEIVE;
		}
		else
		{
			slaveCommPeriph[slaveIndex].transState = TS_TRANSMIT;
		}

		flexsea_send_serial_slave(p);
	}

	/*
	//Packet injection:
	if(slaveComm[slaveIndex].tx.inject == 1)
	{
		slaveComm[slaveIndex].tx.inject = 0;
		if(IS_CMD_RW(slaveComm[slaveIndex].tx.cmd) == READ)
		{
			slaveComm[slaveIndex].transceiverState = TRANS_STATE_TX_THEN_RX;
		}
		else
		{
			slaveComm[slaveIndex].transceiverState = TRANS_STATE_TX;
		}

		flexsea_send_serial_slave(&slaveOutbound[slaveIndex]);
		*/
		/*
		PWst[slaveIndex].port = port;
		PWst[slaveIndex].reply_port = slaveComm[slaveIndex].reply_port;

		memcpy(PWst[slaveIndex].packed, slaveComm[slaveIndex].tx.txBuf, \
				slaveComm[slaveIndex].tx.len);

		flexsea_send_serial_slave(&PWst[slaveIndex]);
		*/
	//}
}

//****************************************************************************
// Private Function(s)
//****************************************************************************
