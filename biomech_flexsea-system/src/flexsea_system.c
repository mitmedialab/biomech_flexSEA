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
	[This file] flexsea_system: configuration and functions for this
	particular system
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-09 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

/* Function naming convention:
==============================
1st: tx_* or rx_* for Transmission or Reception
2nd: *cmd_* always there to indicate it's a command
3rd: *Type_* where Type refers to the category (see filename)
4th: *Cmd_* command name/code
5th: *Dir where Dir can be w (write), rw (read/write), or r (read)
Ex.: tx_cmd_ctrl_mode_w(): Transmission of a command from the Control family,
		that will Write the Control Mode
For rx_* functions, the suffix options are:
		* rw (read/write): Master Reading Slave (might also be writing)
		* rr (reply from read): Slave replying to Master (after receiving a rw)
		* w (write): Master Writing to a slave
*/

//****************************************************************************
// Include(s)
//****************************************************************************

#include <string.h>
#include "../../biomech_flexsea-comm/inc/flexsea.h"	//dependency: flexsea-comm
#include "../../biomech_flexsea-comm/inc/flexsea_payload.h"	//dependency: flexsea-comm
#include <flexsea_cmd_stream.h>
#include <flexsea_system.h>
#include <flexsea_board.h>
#include "../inc/flexsea_cmd_calibration.h"
#include "../inc/flexsea_cmd_in_control.h"
#include "flexsea_cmd_sysdata.h"
#include "flexsea_user_structs.h"
#include "flexsea_cmd_user.h"
#include "flexsea_dataformats.h"

#ifndef NULL
#define NULL 0
#endif

//****************************************************************************
// Variable(s)
//****************************************************************************

//We use this buffer to exchange information between tx_N() and tx_cmd():
uint8_t tmpPayload[PAYLOAD_BUF_LEN];	//tx_N() => tx_cmd()
uint8_t tmp_comm_str[COMM_STR_BUF_LEN];
//Similarly, we exchange command code, type and length:
uint8_t cmdCode = 0, cmdType = 0;
uint16_t cmdLen = 0;

//****************************************************************************
// Function(s)
//****************************************************************************

void flexsea_multi_payload_catchall(uint8_t *msgBuf, MultiPacketInfo *info, uint8_t *responseBuf, uint16_t* responseLen) {

	(void) msgBuf;
	(void) info;
	(void) responseBuf;
	*responseLen = 0;
	return;
}

//Initialize function pointer array
void init_flexsea_payload_ptr(void)
{
	int i = 0;

	//By default, they all point to 'flexsea_payload_catchall()'
	for(i = 0; i < MAX_CMD_CODE; i++)
	{
		flexsea_payload_ptr[i][RX_PTYPE_READ] = &flexsea_payload_catchall;
		flexsea_payload_ptr[i][RX_PTYPE_WRITE] = &flexsea_payload_catchall;
		flexsea_payload_ptr[i][RX_PTYPE_REPLY] = &flexsea_payload_catchall;

		flexsea_multipayload_ptr[i][RX_PTYPE_READ] = &flexsea_multi_payload_catchall;
		flexsea_multipayload_ptr[i][RX_PTYPE_WRITE] = &flexsea_multi_payload_catchall;
		flexsea_multipayload_ptr[i][RX_PTYPE_REPLY] = &flexsea_multi_payload_catchall;
	}

	//Associate pointers to your project-specific functions:
	//(index = command code)
	//======================================================

	//External:
	init_flexsea_payload_ptr_external();

	//Control:
	init_flexsea_payload_ptr_control_1();
	init_flexsea_payload_ptr_control_2();
	init_flexsea_payload_ptr_in_control();

	//Stream:
	init_flexsea_payload_ptr_stream();

	//Data:
	init_flexsea_payload_ptr_data();

	//Sys Data Device Interface:
	init_flexsea_payload_ptr_sysdata();

	//Sensors:
	init_flexsea_payload_ptr_sensors();

	//Tools:
	init_flexsea_payload_ptr_tools();

	//Calibration:
	init_flexsea_payload_ptr_calibration();

	//User functions:
	init_flexsea_payload_ptr_user();
}

//Catch all function - does nothing. Note: error catching code can be added here
void flexsea_payload_catchall(uint8_t *buf, uint8_t *info)
{
	(void)buf;
	(void)info;
	return;
}

//Generic TX command
//ToDo: this function crashes if len is too long! (35 is too much...)
uint16_t tx_cmd(uint8_t *payloadData, uint8_t cmdCode, uint8_t cmd_type, \
				uint32_t len, uint8_t receiver, uint8_t *buf)
{
	uint16_t bytes = 0;
	uint16_t index = 0;
	uint32_t length = 0;

	//Protection against long len:
	length = (len > PAYLOAD_BYTES) ? PAYLOAD_BYTES : len;

	prepare_empty_payload(getBoardID(), receiver, buf, sizeof(buf));
	buf[P_CMDS] = 1;	//Fixed, 1 command

	if(cmd_type == CMD_READ)
	{
		buf[P_CMD1] = CMD_R(cmdCode);
	}
	else if(cmd_type == CMD_WRITE)
	{
		buf[P_CMD1] = CMD_W(cmdCode);
	}
	else
	{
		flexsea_error(SE_INVALID_READ_TYPE);
		return 0;
	}

	index = P_DATA1;
	memcpy(&buf[index], payloadData, length);
	bytes = index+length;

	return bytes;
}

//Package payload, generate communication string. Returns buffer + numb
void pack(uint8_t *shBuf, uint8_t cmd, uint8_t cmdType, uint16_t len, \
			uint8_t rid, uint8_t *info, uint16_t *numBytes, uint8_t *commStr)
{
	uint8_t finalPayload[PAYLOAD_BUF_LEN];
	uint16_t numb = 0;

	(void)info;	//Unused for now

	numb = tx_cmd(shBuf, cmd, cmdType, len, rid, finalPayload);
	numb = comm_gen_str(finalPayload, commStr, numb);
	numb = COMM_STR_BUF_LEN;	//Fixed length
	(*numBytes) = numb;
}

//Call pack(), and send result to master/slave.
//Use that after your tx_N() function.
//ToDo: not elegant, and way too Manage-centric. Fix! ********************
void packAndSend(uint8_t *shBuf, uint8_t cmd, uint8_t cmdType, uint16_t len, \
				 uint8_t rid, uint8_t *info, uint8_t ms)
{
	uint16_t numb = 0;
	PacketWrapper *p = NULL;
	pack(shBuf, cmd, cmdType, len, rid, info, &numb, tmp_comm_str);

	//Assign packet
	p = &packet[info[0]][OUTBOUND];
	//Fill in some of the data
	memcpy(p->packed, tmp_comm_str, numb);
	p->cmd = (cmdType == CMD_READ) ? CMD_R(cmd) : CMD_W(cmd);

	//Send it where it needs to go:
	if(ms == SEND_TO_SLAVE)
	{
		flexsea_send_serial_slave(p);
	}
	else
	{
		flexsea_send_serial_master(p);
	}
}

//Assign a pointer based on P_XID:
void executePtrXid(struct execute_s **myPtr, uint8_t p_xid)
{
	//Based on selected slave, what structure do we use?
	switch(p_xid)
	{
		case FLEXSEA_EXECUTE_1:
			*myPtr = &exec1;
			break;
		case FLEXSEA_EXECUTE_2:
			*myPtr = &exec2;
			break;
		case FLEXSEA_EXECUTE_3:
			*myPtr = &exec3;
			break;
		case FLEXSEA_EXECUTE_4:
			*myPtr = &exec4;
			break;
		default:
			*myPtr = &exec1;
			break;
	}
}

//Assign a pointer based on P_XID:
void managePtrXid(struct manage_s **myPtr, uint8_t p_xid)
{
	//Based on selected slave, what structure do we use?
	switch(p_xid)
	{
		case FLEXSEA_MANAGE_1:
			*myPtr = &manag1;
			break;
		case FLEXSEA_MANAGE_2:
			*myPtr = &manag2;
			break;
		default:
			*myPtr = &manag1;
			break;
	}
}

//Assign a pointer based on P_XID:
void gossipPtrXid(struct gossip_s **myPtr, uint8_t p_xid)
{
	//Based on selected slave, what structure do we use?
	switch(p_xid)
	{
		case FLEXSEA_GOSSIP_1:
			*myPtr = &gossip1;
			break;
		case FLEXSEA_GOSSIP_2:
			*myPtr = &gossip2;
			break;
		default:
			*myPtr = &gossip1;
			break;
	}
}

//Assign a pointer based on P_XID:
void strainPtrXid(struct strain_s **myPtr, uint8_t p_xid)
{
	//Based on selected slave, what structure do we use?
	switch(p_xid)
	{
		case FLEXSEA_STRAIN_1:
			*myPtr = &strain1;
			break;
		default:
			*myPtr = &strain1;
			break;
	}
}

//Assign a pointer based on P_XID:
void rigidPtrXid(struct rigid_s **myPtr, uint8_t p_xid)
{
	//Based on selected slave, what structure do we use?
	switch(p_xid)
	{
		case FLEXSEA_MANAGE_1:
			*myPtr = &rigid1;
			break;
		case FLEXSEA_MANAGE_2:
			*myPtr = &rigid2;
			break;
		default:
			*myPtr = &rigid1;
			break;
	}
}

const int FORMAT_SIZE_MAP[] = {4, 4, 2, 2, 1, 1, -1, 0, -1};
