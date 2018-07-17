/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'flexsea-system' System commands & functions
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
	[This file] flexsea_cmd_sensors: commands specific sensors
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-09 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

//ToDo: the ENCODER command is too "QEI" centric. Generalize.

//****************************************************************************
// Include(s)
//****************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <flexsea_system.h>
#include "../../biomech_flexsea-comm/inc/flexsea.h"	//dependency: flexsea-comm
#include <flexsea_board.h>

//Plan boards only:
#ifdef BOARD_TYPE_FLEXSEA_PLAN
//...
#endif	//BOARD_TYPE_FLEXSEA_PLAN

//Manage boards only:
#ifdef BOARD_TYPE_FLEXSEA_MANAGE
#include "main.h"
#include <ui.h>
#endif	//BOARD_TYPE_FLEXSEA_MANAGE

//Execute boards only:
#ifdef BOARD_TYPE_FLEXSEA_EXECUTE
#include "main.h"
#endif	//BOARD_TYPE_FLEXSEA_EXECUTE

//Strain Amplifier boards only:
#ifdef BOARD_TYPE_FLEXSEA_STRAIN_AMP
#include "main.h"
#endif	//BOARD_TYPE_FLEXSEA_STRAIN_AMP

//****************************************************************************
// Variable(s)
//****************************************************************************

//Will change this, but for now the payloads will be stored in:
uint8_t tmp_payload_xmit[PAYLOAD_BUF_LEN];

//****************************************************************************
// Function(s)
//****************************************************************************

//This gets called by flexsea_system's init_flexsea_payload_ptr(). Map all
//functions from this file to the array here. Failure to do so will send all
//commands to flexsea_payload_catchall().
void init_flexsea_payload_ptr_sensors(void)
{
	flexsea_payload_ptr[CMD_SWITCH][RX_PTYPE_READ] = &rx_cmd_sensors_switch_rw;
	flexsea_payload_ptr[CMD_SWITCH][RX_PTYPE_WRITE] = &rx_cmd_sensors_switch_w;
	flexsea_payload_ptr[CMD_SWITCH][RX_PTYPE_REPLY] = &rx_cmd_sensors_switch_rr;

	flexsea_payload_ptr[CMD_ENCODER][RX_PTYPE_READ] = &rx_cmd_sensors_encoder_rw;
	flexsea_payload_ptr[CMD_ENCODER][RX_PTYPE_WRITE] = &rx_cmd_sensors_encoder_w;
	flexsea_payload_ptr[CMD_ENCODER][RX_PTYPE_REPLY] = &rx_cmd_sensors_encoder_rr;

	//flexsea_payload_ptr[CMD_STRAIN][RX_PTYPE_READ] = &rx_cmd_strain;
}

//Transmit Switch:
//================

//Test code? No
void tx_cmd_sensors_switch_w(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
								uint16_t *len)
{
	//Variable(s) & command:
	uint16_t index = 0;
	(*cmd) = CMD_SWITCH;
	(*cmdType) = CMD_WRITE;

	#ifdef BOARD_TYPE_FLEXSEA_MANAGE

		#if(!(defined BOARD_SUBTYPE_RIGID || defined BOARD_SUBTYPE_POCKET))

			//Data:
			shBuf[index++] = read_sw1();

		#endif

	#else

		(void)shBuf;

	#endif	//BOARD_TYPE_FLEXSEA_MANAGE

	//Payload length:
	(*len) = index;
}

//Test code? No
void tx_cmd_sensors_switch_r(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
								uint16_t *len)
{
	//Variable(s) & command:
	uint16_t index = 0;
	(*cmd) = CMD_SWITCH;
	(*cmdType) = CMD_READ;

	//Data:
	(void)shBuf; //(none)

	//Payload length:
	(*len) = index;
}

//Receive Switch:
//===============

//Test code? No
void rx_cmd_sensors_switch_w(uint8_t *buf, uint8_t *info)
{
	(void)info;
	(void)buf;

	//(We should never receive a Write switch)
}

//Test code? No
void rx_cmd_sensors_switch_rw(uint8_t *buf, uint8_t *info)
{
	(void)info;

	#ifdef BOARD_TYPE_FLEXSEA_EXECUTE

		tx_cmd_sensors_switch_w(TX_N_DEFAULT);
		packAndSend(P_AND_S_DEFAULT, buf[P_XID], info, SEND_TO_MASTER);

	#else

		(void)buf;

	#endif	//BOARD_TYPE_FLEXSEA_EXECUTE
}

//Test code? No
void rx_cmd_sensors_switch_rr(uint8_t *buf, uint8_t *info)
{
	(void)info;

	#ifdef BOARD_TYPE_FLEXSEA_PLAN

		manag1.sw1 = buf[P_DATA1];

	#else

		(void)buf;

	#endif	//BOARD_TYPE_FLEXSEA_PLAN
}

//Transmit Encoder:
//=================

//Test code? No
void tx_cmd_sensors_encoder_w(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
								uint16_t *len, int32_t enc)
{
	//Variable(s) & command:
	uint16_t index = 0;
	(*cmd) = CMD_ENCODER;
	(*cmdType) = CMD_WRITE;

	SPLIT_32((uint32_t) enc, shBuf, &index);

	//Payload length:
	(*len) = index;
}

//Test code? No
void tx_cmd_sensors_encoder_r(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
								uint16_t *len)
{
	//Variable(s) & command:
	uint16_t index = 0;
	(*cmd) = CMD_ENCODER;
	(*cmdType) = CMD_READ;

	//Data:
	(void)shBuf; //(none)

	//Payload length:
	(*len) = index;
}

//Receive Encoder:
//===============

//Test code? No
void rx_cmd_sensors_encoder_w(uint8_t *buf, uint8_t *info)
{
	uint16_t index = P_DATA1;
	int32_t tmpEnc = 0;

	(void)info;	//Unused for now

	tmpEnc = (int32_t) REBUILD_UINT32(buf, &index);

	#ifdef BOARD_TYPE_FLEXSEA_EXECUTE

		#ifdef USE_QEI

			qei_write(tmpEnc);

		#endif

	#else

		(void)tmpEnc;

	#endif	//BOARD_TYPE_FLEXSEA_EXECUTE

}

//Test code? No
void rx_cmd_sensors_encoder_rw(uint8_t *buf, uint8_t *info)
{
	(void)info;
	int32_t encVal = 0;

	#ifdef BOARD_TYPE_FLEXSEA_EXECUTE

		#ifdef USE_QEI

			encVal = qei_read();

		#endif

		tx_cmd_sensors_encoder_w(TX_N_DEFAULT, encVal);
		packAndSend(P_AND_S_DEFAULT, buf[P_XID], info, SEND_TO_MASTER);

	#else

		(void)buf;
		(void)encVal;

	#endif	//BOARD_TYPE_FLEXSEA_EXECUTE
}

//Test code? No
void rx_cmd_sensors_encoder_rr(uint8_t *buf, uint8_t *info)
{
	(void)info;

	#ifdef BOARD_TYPE_FLEXSEA_PLAN

		//ToDo store
		(void)buf; //(for now)

	#else

		(void)buf;

	#endif	//BOARD_TYPE_FLEXSEA_PLAN
}

//****************************************************************************
// Antiquated but valid function(s) - rework & integrate:
//****************************************************************************

/*
//Transmission of a STRAIN command.
//TODO: add support for gains & offsets
//Note: the Reading part can be (and should be) done via Read All
uint32_t tx_cmd_strain(uint8_t receiver, uint8_t cmd_type, uint8_t *buf, uint32_t len)
{
	uint32_t bytes = 0;
	uint8_t tmp0 = 0, tmp1 = 0, tmp2 = 0, tmp3 = 0;

	//Fresh payload string:
	prepare_empty_payload(board_id, receiver, buf, len);

	//Command:
	buf[P_CMDS] = 1;                     //1 command in string

	if(cmd_type == CMD_READ)
	{
		buf[P_CMD1] = CMD_R(CMD_STRAIN);

		//Arguments:
		//(none)

		bytes = P_CMD1 + 1;     //Bytes is always last+1
	}
	else if(cmd_type == CMD_WRITE)
	{
		buf[P_CMD1] = CMD_W(CMD_STRAIN);

		//ToDo gains & offsets

		#ifdef BOARD_TYPE_FLEXSEA_STRAIN_AMP

		//Arguments:
		uint16_to_bytes((uint16_t)strain1.ch[0].strain_filtered, &tmp0, &tmp1);
		buf[P_DATA1 + 0] = tmp0;
		buf[P_DATA1 + 1] = tmp1;
		uint16_to_bytes((uint16_t)strain1.ch[1].strain_filtered, &tmp0, &tmp1);
		buf[P_DATA1 + 2] = tmp0;
		buf[P_DATA1 + 3] = tmp1;
		uint16_to_bytes((uint16_t)strain1.ch[2].strain_filtered, &tmp0, &tmp1);
		buf[P_DATA1 + 4] = tmp0;
		buf[P_DATA1 + 5] = tmp1;
		uint16_to_bytes((uint16_t)strain1.ch[3].strain_filtered, &tmp0, &tmp1);
		buf[P_DATA1 + 6] = tmp0;
		buf[P_DATA1 + 7] = tmp1;
		uint16_to_bytes((uint16_t)strain1.ch[4].strain_filtered, &tmp0, &tmp1);
		buf[P_DATA1 + 8] = tmp0;
		buf[P_DATA1 + 9] = tmp1;
		uint16_to_bytes((uint16_t)strain1.ch[5].strain_filtered, &tmp0, &tmp1);
		buf[P_DATA1 + 10] = tmp0;
		buf[P_DATA1 + 11] = tmp1;

		#endif 	//BOARD_TYPE_FLEXSEA_STRAIN_AMP

		bytes = P_DATA1 + 12;     //Bytes is always last+1
	}
	else
	{
		//Invalid
		flexsea_error(SE_INVALID_READ_TYPE);
		bytes = 0;
	}

	return bytes;
}

//Reception of a STRAIN command
void rx_cmd_strain(uint8_t *buf)
{
	uint32_t numb = 0;
	int32_t tmp = 0;
	struct strain_s *strainPtr = &strain1;

	if(IS_CMD_RW(buf[P_CMD1]) == READ)
	{
		//Received a Read command from our master.

		#ifdef BOARD_TYPE_FLEXSEA_EXECUTE

		//Generate the reply:
		//===================

		//ToDo...

		#endif	//BOARD_TYPE_FLEXSEA_EXECUTE

		#ifdef BOARD_TYPE_FLEXSEA_STRAIN_AMP

		//Generate the reply:
		//===================

		numb = tx_cmd_strain(buf[P_XID], CMD_WRITE, tmp_payload_xmit, PAYLOAD_BUF_LEN);
		numb = comm_gen_str(tmp_payload_xmit, comm_str_485_1, numb);
		numb = COMM_STR_BUF_LEN;    //Fixed length for now
		//rs485_puts(comm_str_485_1, numb);

		#ifdef USE_USB
		usb_puts(comm_str_485_1, (numb));
		#endif

		#endif 	//BOARD_TYPE_FLEXSEA_STRAIN_AMP
	}
	else if(IS_CMD_RW(buf[P_CMD1]) == WRITE)
	{
		//Two options: from Master of from slave (a read reply)

		if(sent_from_a_slave(buf))
		{
			//We received a reply to our read request

			#ifdef BOARD_TYPE_FLEXSEA_PLAN

			strainPtr->ch[0].strain_filtered = (uint16_t)BYTES_TO_UINT16(buf[P_DATA1], buf[P_DATA1+1]);
			strainPtr->ch[1].strain_filtered = (uint16_t)BYTES_TO_UINT16(buf[P_DATA1+2], buf[P_DATA1+3]);
			strainPtr->ch[2].strain_filtered = (uint16_t)BYTES_TO_UINT16(buf[P_DATA1+4], buf[P_DATA1+5]);
			strainPtr->ch[3].strain_filtered = (uint16_t)BYTES_TO_UINT16(buf[P_DATA1+6], buf[P_DATA1+7]);
			strainPtr->ch[4].strain_filtered = (uint16_t)BYTES_TO_UINT16(buf[P_DATA1+8], buf[P_DATA1+9]);
			strainPtr->ch[5].strain_filtered = (uint16_t)BYTES_TO_UINT16(buf[P_DATA1+10], buf[P_DATA1+11]);

			_USE_PRINTF("Strain[0] = %i.\n", strain[0].strain_filtered);

			#endif	//BOARD_TYPE_FLEXSEA_PLAN
		}
		else
		{
			//Master is writing a value to this board

			#ifdef BOARD_TYPE_FLEXSEA_EXECUTE

			qei_write(tmp);

			#endif	//BOARD_TYPE_FLEXSEA_EXECUTE

			#ifdef BOARD_TYPE_FLEXSEA_STRAIN_AMP

			//ToDo

			#endif	//BOARD_TYPE_FLEXSEA_STRAIN_AMP

		}
	}
}
*/

#ifdef __cplusplus
}
#endif
