/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'flexsea-comm' Communication stack
	Copyright (C) 2017 Dephy, Inc. <http://dephy.com/>

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
	[This file] flexsea_interface: simple in & out functions
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2017-09-11 | jfduval | Initial release
****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

//****************************************************************************
// Include(s)
//****************************************************************************

#include "flexsea.h"
#include "flexsea_comm.h"
#include "flexsea_comm_multi.h"
#include "flexsea_payload.h"
#include "flexsea_circular_buffer.h"
#include "user-mn.h"
#include <ui.h>

#ifndef BOARD_TYPE_FLEXSEA_PLAN

#ifndef DEPHY
#include "uarts.h"
#else
#include "usart.h"
#endif // DEPHY

#include "usbd_cdc_if.h"
#endif // BOARD_TYPE_FLEXSEA_PLAN

//****************************************************************************
// Variable(s)
//****************************************************************************

uint8_t npFlag = 0, ppFlag = 0;
uint8_t noWatch = 0;

//****************************************************************************
// Private Function Prototype(s)
//****************************************************************************

//****************************************************************************
// Public Function(s)
//****************************************************************************

//This function replaces flexsea_receive_from_X() and parseXCommands()
//ToDo: add support for 1) RS-485 transceivers reception, 2) SPI error handling
void receiveFlexSEAPacket(Port p, uint8_t *newPacketFlag,  \
							uint8_t *parsedPacketFlag, uint8_t *watch)
{
	uint8_t parseResult = 0;

	//This replaces flexsea_receive_from_X():
	commPeriph[p].rx.unpackedPacketsAvailable = tryParseRx(&commPeriph[p], &packet[p][INBOUND]);
	(*newPacketFlag) = commPeriph[p].rx.unpackedPacketsAvailable;

	//And this replaces parseXCommands():
	if(commPeriph[p].rx.unpackedPacketsAvailable > 0)
	{
		commPeriph[p].rx.unpackedPacketsAvailable = 0;
		parseResult = payload_parse_str(&packet[p][INBOUND]);
		(*parsedPacketFlag) += (parseResult == PARSE_SUCCESSFUL) ? 1 : 0;
		(*watch) = 0; //Valid packets restart the watch count
	}
}

//Host program can use this to feed bytes into a reception buffer
//When autoParse is > 0 we parse the new data
uint8_t receiveFlexSEABytes(uint8_t *d, uint8_t len, uint8_t autoParse)
{
	circ_buff_write(commPeriph[PORT_USB].rx.circularBuff, d, len);
	commPeriph[PORT_USB].rx.bytesReadyFlag++;

	//Parse if needed:
	if(autoParse){receiveFlexSEAPacket(PORT_USB, &npFlag, &ppFlag, &noWatch);}

	return ppFlag;
}

// This function receives a comm string stored in the multipacket comm periph specified by p
// TODO: implement for everything outside USB

uint8_t receiveFxPacketByPeriph(MultiCommPeriph *cp)
{

	if(!(cp->bytesReadyFlag > 0))
		return 1;

	cp->bytesReadyFlag--;	// = 0;

	cp->in.isMultiComplete = 0;

	uint16_t numBytesConverted, parsed = 0;

	do {

		numBytesConverted = unpack_multi_payload_cb_cached(&cp->circularBuff, &cp->in, &cp->parsingCachedIndex);
		advanceMultiInput(cp, cp->parsingCachedIndex);

		if(cp->in.isMultiComplete)
		{

			if(parseReadyMultiString(cp) == PARSE_SUCCESSFUL)
				parsed++;

		}

	} while(numBytesConverted);


	return parsed;
}

uint8_t receiveFxPacket(Port p) {

	MultiCommPeriph *cp = comm_multi_periph + p;
	return receiveFxPacketByPeriph(cp);
}

#ifdef BOARD_TYPE_FLEXSEA_MANAGE
// TODO: this function is completely manage specific, and it ought not be


uint8_t transmitFxPacket(Port p) {

	MultiCommPeriph *cp = comm_multi_periph + p;

	if (cp->out.frameMap != 0) {
		LEDB(1);
	}

	//check if the periph has anything to send
	if(cp->out.frameMap > 0 && !cp->out.isMultiComplete)
	{
		LEDR(1);
		uint8_t frameId = 0;
		//figure out the next frame to send
		while((cp->out.frameMap & (1 << frameId)) == 0)
			frameId++;

		//check that the frameid is valid
		if(frameId >= MAX_FRAMES_PER_MULTI_PACKET)
		{
			// if its not valid we just discard the multi packet frames, setting the flags accordingly
			cp->out.frameMap = 0;
			cp->out.isMultiComplete = 1;
			return 1;	// return an error
		}

		uint8_t success = 0;
		if(p == PORT_WIRELESS)
		{
#if(defined USE_UART3)
			puts_expUart(cp->out.packed[frameId], SIZE_OF_MULTIFRAME(cp->out.packed[frameId]));
			success = 1;
#elif(defined USE_UART4)
			puts_expUart2(cp->out.packed[frameId], SIZE_OF_MULTIFRAME(cp->out.packed[frameId]));
			success = 1;
#endif
		}
		else if(p == PORT_USB)
		{
			success = !CDC_CheckBusy_FS() && USBD_OK == CDC_Transmit_FS(cp->out.packed[frameId], SIZE_OF_MULTIFRAME(cp->out.packed[frameId]));
		}
		else
			success = 1; // unimplemented port we just pretend its all good? TODO: something smarter?

		if(success)
		{
			//mark frame as sent
			cp->out.frameMap &= (   ~(1 << frameId)   );

			if(cp->out.frameMap == 0)
				cp->out.isMultiComplete = 1;

		}

		// maybe we should be checking for USBD_BUSY or USBD_FAIL
		return success ? 0 : 1;

	}

	return -1;
}
#endif

//****************************************************************************
// Private Function(s):
//****************************************************************************

#ifdef __cplusplus
}
#endif
