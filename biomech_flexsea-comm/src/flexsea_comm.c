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
	[This file] flexsea_comm: Data-Link layer of the FlexSEA protocol
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-09 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

//FlexSEA comm. prototype:
//=======================
//[HEADER][# of BYTES][DATA...][CHECKSUM][FOOTER]
//=> Number of bytes includes the ESCAPE bytes
//=> Checksum is done on the payload (data + ESCAPEs) and on the BYTES byte.

//To transmit a message:
//======================
// 1) Place the payload in an array (no header, no footer: pure data)
// 2) Call comm_gen_str(your_data_array, number_of_bytes)
// 2b) It will return the index of the last byte of the message (add 1 for the length)
// 2c) The message is in comm_str[]
// 3) Send comm_str[] (x_puts(comm_str, msg_length));

//To receive a message:
//=====================
// 1) Assuming that you have dealt with all the previous messages, call comm_str_payload();
//    to fill the buffer with zeros
// 2) Every time you receive a byte update the buffer: comm_update_rx_buffer(your_new_byte);
// 3) Call payload_str_available_in_buffer = comm_decode_str(). If you get >= 1, read the
//    comm_str_payload buffer and do something with the data!
// 4) At this point you might want to flush the read payload from rx_buf

//****************************************************************************
// Include(s)
//****************************************************************************

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <flexsea_comm.h>

#include "flexsea_user_structs.h"
//****************************************************************************
// Variable(s)
//****************************************************************************

uint8_t comm_str_tmp[COMM_STR_BUF_LEN];

uint8_t comm_str_1[COMM_PERIPH_ARR_LEN];
uint8_t packed_1[COMM_PERIPH_ARR_LEN];
uint8_t rx_command_1[COMM_PERIPH_ARR_LEN];
uint8_t comm_str_2[COMM_PERIPH_ARR_LEN];
uint8_t packed_2[COMM_PERIPH_ARR_LEN];
uint8_t rx_command_2[COMM_PERIPH_ARR_LEN];
uint8_t comm_str_3[COMM_PERIPH_ARR_LEN];
uint8_t packed_3[COMM_PERIPH_ARR_LEN];
uint8_t rx_command_3[COMM_PERIPH_ARR_LEN];
uint8_t comm_str_4[COMM_PERIPH_ARR_LEN];
uint8_t packed_4[COMM_PERIPH_ARR_LEN];
uint8_t rx_command_4[COMM_PERIPH_ARR_LEN];
uint8_t comm_str_5[COMM_PERIPH_ARR_LEN];
uint8_t packed_5[COMM_PERIPH_ARR_LEN];
uint8_t rx_command_5[COMM_PERIPH_ARR_LEN];
uint8_t comm_str_6[COMM_PERIPH_ARR_LEN];
uint8_t packed_6[COMM_PERIPH_ARR_LEN];
uint8_t rx_command_6[COMM_PERIPH_ARR_LEN];

uint32_t cmd_valid = 0;
uint32_t cmd_bad_checksum = 0;

PacketWrapper packet[NUMBER_OF_PORTS][2];
CommPeriph commPeriph[NUMBER_OF_PORTS];



struct commSpy_s commSpy1 = {0,0,0,0,0,0,0};

//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************


//****************************************************************************
// Public Function(s)
//****************************************************************************

//Takes payload, adds ESCAPES, checksum, header, ...
uint8_t comm_gen_str(uint8_t payload[], uint8_t *cstr, uint8_t bytes)
{
	unsigned int i = 0, escapes = 0, idx = 0, total_bytes = 0;
	uint8_t checksum = 0;

	//Fill comm_str with known values ('a')
	memset(cstr, 0xAA, COMM_STR_BUF_LEN);

	//Fill comm_str with payload and add ESCAPE characters
	escapes = 0;
	idx = 2;
	for(i = 0; i < bytes && idx < COMM_STR_BUF_LEN; i++)
	{
		if ((payload[i] == HEADER) || (payload[i] == FOOTER) || (payload[i] == ESCAPE))
		{
			escapes = escapes + 1;
			cstr[idx] = ESCAPE;
			cstr[idx+1] = payload[i];
			checksum += cstr[idx];
			checksum += cstr[idx+1];
			idx = idx + 1;
		}
		else
		{
			cstr[idx] = payload[i];
			checksum += cstr[idx];
		}
		idx++;
	}

	if((idx + 2) >= COMM_STR_BUF_LEN)
	{
		//Too long, abort:
		memset(cstr, 0, COMM_STR_BUF_LEN);	//Clear string
		return 0;
	}

	total_bytes = bytes + escapes;

	commSpy1.bytes = bytes;
	commSpy1.escapes = (uint8_t) escapes;
	commSpy1.total_bytes = (uint8_t) total_bytes;
	commSpy1.error++;

	//String length?
	if(total_bytes >= COMM_STR_BUF_LEN)
	{
		//Too long, abort:
		memset(cstr, 0, COMM_STR_BUF_LEN);	//Clear string
		commSpy1.retVal = 0;
		return 0;
	}

	commSpy1.checksum = checksum;

	//Build comm_str:
	cstr[0] = HEADER;
	cstr[1] = total_bytes;
	cstr[2 + total_bytes] = checksum;
	cstr[3 + total_bytes] = FOOTER;

	//Return the last index of the valid data
	commSpy1.retVal = 3 + (uint8_t)total_bytes;
	return (3 + total_bytes);
}

/*
//Special wrapper for unit test code:
int8_t unpack_payload_test(uint8_t *buf, uint8_t *packed, uint8_t rx_cmd[PACKAGED_PAYLOAD_LEN])
{
	return unpack_payload(buf, packed, rx_cmd);
}
*/

void initRandomGenerator(int seed)
{
	srand(seed);
}

uint8_t generateRandomUint8_t(void)
{
	int r = rand();
	return (uint8_t)(r % 255);
}

void generateRandomUint8_tArray(uint8_t *arr, uint8_t size)
{
	int i = 0;

	for(i = 0; i < size; i++)
	{
		arr[i] = generateRandomUint8_t();
	}
}

uint16_t unpack_payload_cb(circularBuffer_t *cb, uint8_t *packed, uint8_t unpacked[PACKAGED_PAYLOAD_LEN])
{
	int bufSize = circ_buff_get_size(cb);

	int foundString = 0, foundFrame = 0, bytes, possibleFooterPos;
	int lastPossibleHeaderIndex = bufSize - 4;
	int headerPos = -1, lastHeaderPos = -1;
	uint8_t checksum = 0;

	int headers = 0, footers = 0;
	while(!foundString && lastHeaderPos < lastPossibleHeaderIndex)
	{
		headerPos = circ_buff_search(cb, HEADER, lastHeaderPos+1);
		//if we can't find a header, we quit searching for strings
		if(headerPos == -1) break;

		headers++;
		foundFrame = 0;
		if(headerPos <= lastPossibleHeaderIndex)
		{
			bytes = circ_buff_peak(cb, headerPos + 1);
			possibleFooterPos = headerPos + 3 + bytes;
			foundFrame = (possibleFooterPos < bufSize && circ_buff_peak(cb, possibleFooterPos) == FOOTER);
		}

		if(foundFrame)
		{
			footers++;
			checksum = circ_buff_checksum(cb, headerPos+2, possibleFooterPos-1);

			//if checksum is valid than we found a valid string
			foundString = (checksum == circ_buff_peak(cb, possibleFooterPos-1));
		}

		//either we found a frame and it had a valid checksum, or we want to try the next value of i
		lastHeaderPos = headerPos;
	}

	int numBytesInPackedString = 0;
	if(foundString)
	{
		numBytesInPackedString = headerPos + bytes + 4;

		circ_buff_read_section(cb, packed, headerPos, bytes + 4);

		int k, skip = 0, unpacked_idx = 0;
		for(k = 0; k < bytes; k++)
		{
			int index = k+2; //first value is header, next value is bytes, next value is first data
			if(packed[index] == ESCAPE && skip == 0)
			{
				skip = 1;
			}
			else
			{
				skip = 0;
				unpacked[unpacked_idx++] = packed[index];
			}
		}
	}

	return numBytesInPackedString;
}

//From CommPeriph to PacketWrapper:
void fillPacketFromCommPeriph(CommPeriph *cp, PacketWrapper *pw)
{
	pw->sourcePort = cp->port;
	if(cp->portType == MASTER)
	{
		pw->travelDir = DOWNSTREAM;
	}
	else
	{
		pw->travelDir = UPSTREAM;
	}

	//Copy data. As the source is the peripheral, we always use rx.
	uint16_t len = COMM_STR_BUF_LEN;	//ToDo something smarter!
	memcpy(pw->packed, cp->rx.packedPtr, len);
	memcpy(pw->unpaked, cp->rx.unpackedPtr, len);
}

//Functions that can copy packets between PacketWrapper objects:
//ToDo: delete 'TravelDirection td'?
void copyPacket(PacketWrapper *from, PacketWrapper *to, TravelDirection td)
{
	(void)td;
	to->sourcePort = from->sourcePort;
	to->destinationPort = from->destinationPort;
	to->travelDir = from->travelDir;
	memcpy(to->packed, from->packed, PACKET_WRAPPER_LEN);
	memcpy(to->unpaked, from->unpaked, PACKET_WRAPPER_LEN);
}

//Initialize CommPeriph to defaults:
void initCommPeriph(CommPeriph *cp, Port port, PortType pt, uint8_t *input, \
					uint8_t *unpacked, uint8_t *packed, circularBuffer_t* rx_cb, \
					PacketWrapper *inbound, PacketWrapper *outbound)
{
	cp->port = port;
	cp->portType = pt;
	cp->transState = TS_UNKNOWN;

	cp->rx.bytesReadyFlag = 0;
	cp->rx.unpackedPacketsAvailable = 0;
	cp->rx.inputBufferPtr = input;
	cp->rx.unpackedPtr = unpacked;
	cp->rx.packedPtr = packed;
	memset(cp->rx.packedPtr, 0, COMM_PERIPH_ARR_LEN);
	memset(cp->rx.unpackedPtr, 0, COMM_PERIPH_ARR_LEN);

	circ_buff_init(rx_cb);
	cp->rx.circularBuff = rx_cb;


	cp->tx.bytesReadyFlag = 0;
	cp->tx.unpackedPacketsAvailable = 0;
	linkCommPeriphPacketWrappers(cp, inbound, outbound);
}

//Each CommPeriph is associated to two PacketWrappers (inbound & outbound)
void linkCommPeriphPacketWrappers(CommPeriph *cp, PacketWrapper *inbound, \
					PacketWrapper *outbound)
{
	//Force family:
	cp->in = inbound;
	cp->out = outbound;
	inbound->parent = (CommPeriph*)cp;
	outbound->parent = (CommPeriph*)cp;

	//Children inherit from parent:
	if(cp->portType == MASTER)
	{
		inbound->travelDir = DOWNSTREAM;
		inbound->sourcePort = cp->port;
		inbound->destinationPort = PORT_NONE;

		outbound->travelDir = UPSTREAM;
		outbound->sourcePort = PORT_NONE;
		outbound->destinationPort = cp->port;
	}
	else
	{
		inbound->travelDir = UPSTREAM;
		inbound->sourcePort = cp->port;
		inbound->destinationPort = PORT_NONE;

		outbound->travelDir = DOWNSTREAM;
		outbound->sourcePort = PORT_NONE;
		outbound->destinationPort = cp->port;
	}
}

#ifdef __cplusplus
}
#endif
