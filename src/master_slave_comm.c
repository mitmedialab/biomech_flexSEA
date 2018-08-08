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
#include "flexsea_cmd_stream.h"
#include "user-mn.h"
#include "uarts.h"
#include "timer.h"
#include "flexsea_comm_multi.h"

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
	//USB:
	initCommPeriph(&commPeriph[PORT_USB], PORT_USB, MASTER, rx_buf_4, \
			comm_str_4, rx_command_4, &rx_buf_circ_4, \
			&packet[PORT_USB][INBOUND], &packet[PORT_USB][OUTBOUND]);

	uint8_t i;
	for(i=0;i<NUMBER_OF_PORTS;i++)
		initMultiPeriph(comm_multi_periph+i, i, i < 2 ? SLAVE : MASTER);



	//RS-485 #1:
	#if(MULTI_DOF_N == 0)
	PortType ms = SLAVE;
	#endif	//(MULTI_DOF_N == 0)

	#if (MULTI_DOF_N == 1)
	PortType ms = MASTER;
	#endif	//(MULTI_DOF_N == 1)

	initCommPeriph(&commPeriph[PORT_RS485_1], PORT_RS485_1, ms, rx_buf_1, \
			comm_str_1, rx_command_1, &rx_buf_circ_1, \
			&packet[PORT_RS485_1][INBOUND], &packet[PORT_RS485_1][OUTBOUND]);


	//UART:
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

//Slave Communication function. Call at 1kHz.
//ToDo: this can also be used to transmit to a master
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

			//ToDo: this is slave only... but does it matter?
			flexsea_send_serial_slave(p);
		}
	}
}

//Call this to send any pending delayed reply on RS-485
//ToDo: this doesn't use timestamp and doesn't guarantee a delay!
void sendMasterDelayedResponse(void)
{
	Port port = PORT_RS485_1;
	//uint8_t* str = commPeriph[port].out.packed[0];
	//uint16_t length = COMM_STR_BUF_LEN;

	if((commPeriph[port].tx.packetReady) == 1 && (commPeriph[port].tx.timeStamp == tb_100us_timeshare))
	{
		/*puts_rs485_1(packet[port][OUTBOUND].packed, \
					packet[port][OUTBOUND].numb);*/
		puts_rs485_1(commPeriph[port].out->packed, commPeriph[port].out->numb);
		//Drop flag
		commPeriph[port].tx.packetReady = 0;
	}
}

inline uint8_t isMultiAutoStream(uint8_t cmdCode) {
	return cmdCode == CMD_SYSDATA;
}

void autoStream(void)
{
	static int sinceLastStreamSend[MAX_STREAMS];

	static MultiCommPeriph autoPeriph;
	static MultiPacketInfo pInfo;
	static uint8_t notInitialized = 1;

	if(notInitialized)
	{
		notInitialized = 0;
		pInfo.rid = getDeviceId();
		initMultiPeriph(&autoPeriph, PORT_NONE, MASTER);
		memset(sinceLastStreamSend, 0, MAX_STREAMS * sizeof(int));
	}

	if(isStreaming)
	{
		int i;
		for(i = 0; i < isStreaming; i++)
		{
			sinceLastStreamSend[i]++;
		}

		for(i = 0; i < isStreaming; i++)
		{
			if(sinceLastStreamSend[i] >= streamPeriods[i])
			{
				//Determine what offset to use:
				streamCurrentOffset[i]++;
				if(streamCurrentOffset[i] > streamIndex[i][1])
				{
					streamCurrentOffset[i] = streamIndex[i][0];
				}

				if(isMultiAutoStream(streamCmds[i]))
				{
					autoPeriph.in.currentMultiPacket++;
					pInfo.xid = streamReceivers[i];
					pInfo.portIn = streamPortInfos[i];
					uint8_t error = receiveAndFillResponse(streamCmds[i], RX_PTYPE_READ, &pInfo, &autoPeriph);

					if(!error && autoPeriph.out.unpackedIdx)
					{
						MultiCommPeriph *cp = comm_multi_periph + streamPortInfos[i];
						cp->out.currentMultiPacket = autoPeriph.out.currentMultiPacket;
						cp->out.unpackedIdx = autoPeriph.out.unpackedIdx;
						memcpy( (cp->out.unpacked), (autoPeriph.out.unpacked), autoPeriph.out.unpackedIdx);
					}

				}
				else
				{
					uint8_t cp_str[256] = {0};
					cp_str[P_XID] = streamReceivers[i];
					cp_str[P_DATA1] = streamCurrentOffset[i];
					(*flexsea_payload_ptr[streamCmds[i]][RX_PTYPE_READ]) (cp_str, &streamPortInfos[i]);
				}

				sinceLastStreamSend[i] -= streamPeriods[i];

				//we return to avoid sending two msgs in one cycle
				//since counters were already incremented, we will still try to hit other stream frequencies
				return;
			}
		}
	}
}

//****************************************************************************
// Private Function(s)
//****************************************************************************
