/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'flexsea-comm' Communication stack
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
	[This file] flexsea_payload: deals with the "intelligent" data packaged
	in a comm_str
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-09 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

//****************************************************************************
// Include(s)
//****************************************************************************

#include <string.h>
#include <flexsea_payload.h>
#include <flexsea_board.h>

//****************************************************************************
// Variable(s)
//****************************************************************************

uint8_t payload_str[PAYLOAD_BUF_LEN];
uint8_t lastPayloadParsed[2] = {0, 0};

//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************

static void route(PacketWrapper * p, PortType to);

//****************************************************************************
// Public Function(s):
//****************************************************************************

//Decode/parse received string
//ToDo improve: for now, supports only one command per string
uint8_t payload_parse_str(PacketWrapper* p)
{
	uint8_t *cp_str = p->unpaked;
	uint8_t info[2] = {0,0};
	uint8_t cmd = 0, cmd_7bits = 0;
	unsigned int id = 0;
	uint8_t pType = RX_PTYPE_INVALID;
	info[0] = (uint8_t)p->sourcePort;

	//Command
	cmd = cp_str[P_CMD1];		//CMD w/ R/W bit
	cmd_7bits = CMD_7BITS(cmd);	//CMD code, no R/W information

	//First, get RID code
	id = get_rid(cp_str);
	if(id == ID_MATCH)
	{
		p->destinationPort = PORT_NONE;	//We are home
		pType = packetType(cp_str);

		//It's addressed to me. Function pointer array will call
		//the appropriate handler (as defined in flexsea_system):
		if((cmd_7bits <= MAX_CMD_CODE) && (pType <= RX_PTYPE_MAX_INDEX))
		{
			//Save info about the last success:
			lastPayloadParsed[0] = cmd_7bits;
			lastPayloadParsed[1] = pType;
			//Call handler:
			(*flexsea_payload_ptr[cmd_7bits][pType]) (cp_str, info);
			return PARSE_SUCCESSFUL;
		}
		else
		{
			return PARSE_DEFAULT;
		}
	}
	else if(id == ID_SUB1_MATCH)
	{
		#ifndef USB_SPI_BRIDGE
		//For a slave on bus #1:
		p->destinationPort = PORT_RS485_1;
		route(p, SLAVE);
		#else
		//This will redirect the EX1 requests to SPI:
		p->destinationPort = PORT_EXP;
		route(p, SLAVE);
		#endif
	}
	else if(id == ID_SUB2_MATCH)
	{
		//For a slave on bus #2:
		p->destinationPort = PORT_RS485_2;
		route(p, SLAVE);
	}
	else if(id == ID_SUB3_MATCH)
	{
		//For a slave on the expansion port:
		p->destinationPort = PORT_EXP;
		route(p, SLAVE);
	}
	else if((id == ID_UP_MATCH) || (id == ID_OTHER_MASTER))
	{
		//For a master:

		#ifdef BOARD_TYPE_FLEXSEA_MANAGE

		//Manage is the only board that can receive a package destined to his master
		route(p, MASTER);

		#endif	//BOARD_TYPE_FLEXSEA_MANAGE
	}
	else
	{
		return PARSE_ID_NO_MATCH;
	}

	//Shouldn't get here...
	return PARSE_DEFAULT;
}

//Start a new payload string
void prepare_empty_payload(uint8_t from, uint8_t to, uint8_t *buf, uint32_t len)
{
	//Start fresh:
	memset(buf, 0, len);

	//Addresses:
	buf[P_XID] = from;
	buf[P_RID] = to;
}

//Returns one if it was sent from a slave, 0 otherwise
uint8_t sent_from_a_slave(uint8_t *buf)
{
	//Hint: slaves have higher addresses than their master
	return ((buf[P_XID] > buf[P_RID])? 1 : 0);
}

//We received a packet. Is it a Read, a Reply or a Write?
uint8_t packetType(uint8_t *buf)
{
	//Logic behind this code: slaves have higher addresses than their master.

	//From a Master:
	if(buf[P_XID] < buf[P_RID])
	{
		//Master is writing. Write or Read?
		return ((IS_CMD_RW(buf[P_CMD1]) == READ) ? \
					RX_PTYPE_READ : RX_PTYPE_WRITE);
	}

	//From a Slave:
	if(buf[P_XID] > buf[P_RID])
	{
		//The only thing we can get from a slave is a Reply
		return ((IS_CMD_RW(buf[P_CMD1]) == WRITE) ? \
					RX_PTYPE_REPLY : RX_PTYPE_INVALID);
	}

	//Equal addresses, shouldn't happen
	return RX_PTYPE_INVALID;
}

// Using these ifdefs is a non ideal approach
// plan should refactor to use Manage's tryParseRx
// done this way so that this commit doesn't break plan, but it should be removed ASAP (ToDo)
#ifdef BOARD_TYPE_FLEXSEA_PLAN
inline uint8_t tryParseRx(CommPeriph *cp, PacketWrapper *pw)
{
	if(!(cp->rx.bytesReadyFlag > 0)) return 0;
	cp->rx.bytesReadyFlag--;
	uint8_t successfulParse = 0, error;

	uint16_t numBytesConverted = unpack_payload_cb(\
			cp->rx.circularBuff, \
			cp->rx.packedPtr, \
			cp->rx.unpackedPtr);

	if(numBytesConverted > 0)
	{
		error = circ_buff_move_head(cp->rx.circularBuff, numBytesConverted);

		#ifdef USE_PRINTF

			if(error){printf() << "circ_buff_move_head error:" << error;}

		#else

			(void)error;

		#endif

		fillPacketFromCommPeriph(cp, pw);
		// payload_parse_str returns 2 on successful parse
		successfulParse = payload_parse_str(pw) == 2;
	}

	return successfulParse;
}
#endif
#ifndef BOARD_TYPE_FLEXSEA_PLAN
inline uint8_t tryParseRx(CommPeriph *cp, PacketWrapper *pw)
{
	if(!(cp->rx.bytesReadyFlag > 0)) return 0;
	cp->rx.bytesReadyFlag--;	// = 0;
	uint8_t error = 0;

	uint16_t numBytesConverted = unpack_payload_cb(\
			cp->rx.circularBuff, \
			cp->rx.packedPtr, \
			cp->rx.unpackedPtr);

	if(numBytesConverted > 0)
	{
		error = circ_buff_move_head(cp->rx.circularBuff, numBytesConverted);

#ifdef USE_PRINTF
		if(error)
			printf() << "circ_buff_move_head error:" << error;
#endif
		fillPacketFromCommPeriph(cp, pw);
		// payload_parse_str returns 2 on successful parse
		//successfulParse = payload_parse_str(pw) == 2;
	}

	return numBytesConverted > 0 && !error;
}
#endif

//Accessor function for the API: what did we last parse?
void getSignatureOfLastPayloadParsed(uint8_t *cmd, uint8_t *type)
{
	(*cmd) = lastPayloadParsed[0];
	(*type) = lastPayloadParsed[1];
}

//****************************************************************************
// Private Function(s):
//****************************************************************************

static void route(PacketWrapper * p, PortType to)
{
	#ifdef BOARD_TYPE_FLEXSEA_MANAGE

		Port idx = PORT_NONE;

		if(to == SLAVE)
		{
			idx = p->destinationPort;
			copyPacket(p, &packet[idx][OUTBOUND], DOWNSTREAM);
			packet[idx][OUTBOUND].cmd = packet[idx][OUTBOUND].unpaked[P_CMD1];

			//Next line is a test:
			packet[idx][INBOUND].destinationPort = packet[idx][OUTBOUND].sourcePort;

			commPeriph[idx].tx.packetReady = 1;
		}
		else
		{
			idx = p->destinationPort;
			copyPacket(p, &packet[idx][OUTBOUND], UPSTREAM);
			packet[idx][OUTBOUND].cmd = packet[idx][OUTBOUND].unpaked[P_CMD1];
			flexsea_send_serial_master(p);
		}

	#else

		(void)p;
		(void)to;

	#endif 	//BOARD_TYPE_FLEXSEA_MANAGE
}

//Is it addressed to me? To a board "below" me? Or to my Master?
uint8_t get_rid(uint8_t *pldata)
{
	uint8_t cp_rid = pldata[P_RID];
	uint8_t i = 0;

	if(cp_rid == 0) return ID_NO_MATCH;

	if(cp_rid == getBoardID() || cp_rid == getDeviceId()) //This board?
	{
		return ID_MATCH;
	}
	else if(cp_rid == getBoardUpID())		//Master?
	{
		return ID_UP_MATCH;
	}
	else if(cp_rid < getBoardID())
	{
		//Special case: it's for a master that's not "our" master
		return ID_OTHER_MASTER;
	}
	else
	{
		//Can be on a slave bus, or can be invalid.

		//Search on slave bus #1:
		for(i = 0; i < getSlaveCnt(0); i++)
		{
			if(cp_rid == getBoardSubID(0,i))
			{
				return ID_SUB1_MATCH;
			}
		}

		//Then on bus #2:
		for(i = 0; i < getSlaveCnt(1); i++)
		{
			if(cp_rid == getBoardSubID(1,i))
			{
				return ID_SUB2_MATCH;
			}
		}

		//Then on bus #3:
		for(i = 0; i < getSlaveCnt(2); i++)
		{
			if(cp_rid == getBoardSubID(2,i))
			{
				return ID_SUB3_MATCH;
			}
		}
	}

	//If we end up here it's because we didn't get a match:
	return ID_NO_MATCH;
}

#ifdef __cplusplus
}
#endif
