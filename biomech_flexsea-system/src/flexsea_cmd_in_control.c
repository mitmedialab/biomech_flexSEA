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
	[This file] flexsea_cmd_in_control: commands specific to the in control module
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-09 | jfduval | Initial GPL-3.0 release
	* 2016-11-07 | jfduval | Renamed *_1, updated to new convention
	* 2017-02-09 | dudds4  | Extracted in control commands to separate file
****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

//****************************************************************************
// Include(s)
//****************************************************************************
#include "../inc/flexsea_cmd_in_control.h"
#include <stdio.h>
#include <stdlib.h>
#include <flexsea_system.h>
#include "../../biomech_flexsea-comm/inc/flexsea.h"	//dependency: flexsea-comm
#include <flexsea_board.h>

#ifdef BOARD_TYPE_FLEXSEA_EXECUTE
#include "control.h"
#endif

//****************************************************************************
// Function(s)
//****************************************************************************

//This gets called by flexsea_system's init_flexsea_payload_ptr(). Map all
//functions from this file to the array here. Failure to do so will send all
//commands to flexsea_payload_catchall().
void init_flexsea_payload_ptr_in_control(void)
{
	//Control mode:
	flexsea_payload_ptr[CMD_IN_CONTROL][RX_PTYPE_READ] = &rx_cmd_in_control_rw;
	flexsea_payload_ptr[CMD_IN_CONTROL][RX_PTYPE_WRITE] = &rx_cmd_in_control_w;
	flexsea_payload_ptr[CMD_IN_CONTROL][RX_PTYPE_REPLY] = &rx_cmd_in_control_rr;
}

//Transmit In Control Command:
//============================
void tx_cmd_in_control_w(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
						uint16_t *len, uint8_t select_w)
{
	//Variable(s) & command:
	uint16_t index = 0;
	(*cmd) = CMD_IN_CONTROL;
	(*cmdType) = CMD_WRITE;

	//Data:
	#ifdef BOARD_TYPE_FLEXSEA_EXECUTE

		shBuf[index++] = select_w;	//Parameter written
		SPLIT_32((uint32_t)in_control.w[select_w], shBuf, &index);
		SPLIT_32((uint32_t)in_control.setp, shBuf, &index);
		SPLIT_32((uint32_t)in_control.actual_val, shBuf, &index);
		SPLIT_32((uint32_t)in_control.error, shBuf, &index);
		SPLIT_16((uint16_t)in_control.output, shBuf, &index);

		//Combine 3 fields in 1 uint16:
		in_control_get_pwm_dir();
		in_control_combine();
		SPLIT_16((uint16_t)in_control.combined, shBuf, &index);

		SPLIT_16((uint16_t)ctrl[0].current.actual_val, shBuf, &index);

		//User fields:
		SPLIT_32((uint32_t)in_control.r[0], shBuf, &index);
		SPLIT_32((uint32_t)in_control.r[1], shBuf, &index);

	#endif //BOARD_TYPE_FLEXSEA_EXECUTE

	#ifdef BOARD_TYPE_FLEXSEA_PLAN

		shBuf[index++] = select_w;	//Parameter written
		SPLIT_32((uint32_t)in_control_1.w[select_w], shBuf, &index);

	#endif  //BOARD_TYPE_FLEXSEA_PLAN

	//Payload length:
	(*len) = index;
}

void tx_cmd_in_control_r(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
						uint16_t *len)
{
	//Variable(s) & command:
	uint16_t index = 0;
	(*cmd) = CMD_IN_CONTROL;
	(*cmdType) = CMD_READ;

	//Data:
	(void)shBuf; //(none)

	//Payload length:
	(*len) = index;
}

//Receive In Control Command:
//===========================

//Test code? No
void rx_cmd_in_control_w(uint8_t *buf, uint8_t *info)
{
	uint16_t index = P_DATA1;
	int32_t w_val = 0;
	uint8_t w_select = 0;

	//Decode data:
	w_select =  buf[index++];
	w_val = (int32_t) REBUILD_UINT32(buf, &index);

	(void)info;

	#ifdef BOARD_TYPE_FLEXSEA_EXECUTE

		in_control.w[w_select] = w_val;

	#else

		(void)buf;
		(void)w_val;
		(void)w_select;

	#endif	//BOARD_TYPE_FLEXSEA_EXECUTE
}

//Test code? No
void rx_cmd_in_control_rw(uint8_t *buf, uint8_t *info)
{
	(void)info;

	#ifdef BOARD_TYPE_FLEXSEA_EXECUTE

		tx_cmd_in_control_w(TX_N_DEFAULT, 0);
		packAndSend(P_AND_S_DEFAULT, buf[P_XID], info, SEND_TO_MASTER);

	#else
		(void)buf;
	#endif	//BOARD_TYPE_FLEXSEA_EXECUTE
}

//Test code? No
void rx_cmd_in_control_rr(uint8_t *buf, uint8_t *info)
{
	uint16_t index = P_DATA1;

	(void)info;

	#if((defined BOARD_TYPE_FLEXSEA_MANAGE) || (defined BOARD_TYPE_FLEXSEA_PLAN))

		//Store value:
		in_control_1.setp = (int32_t) REBUILD_UINT32(buf, &index);
		in_control_1.actual_val = (int32_t) REBUILD_UINT32(buf, &index);

		in_control_1.error = (int32_t) REBUILD_UINT32(buf, &index);
		in_control_1.output = (int16_t) REBUILD_UINT16(buf, &index);

		in_control_1.combined = REBUILD_UINT16(buf, &index);
		in_control_1.current = (int16_t) REBUILD_UINT16(buf, &index);

		in_control_1.r[0] = (int32_t) REBUILD_UINT32(buf, &index);
		in_control_1.r[1] = (int32_t) REBUILD_UINT32(buf, &index);

		in_control_1.controller = IN_CONTROL_CONTROLLER(in_control_1.combined);
		in_control_1.mot_dir = IN_CONTROL_MOT_DIR(in_control_1.combined);
		in_control_1.pwm = IN_CONTROL_PWM(in_control_1.combined);

	#else

		(void)buf;

	#endif	//((defined BOARD_TYPE_FLEXSEA_MANAGE) || (defined BOARD_TYPE_FLEXSEA_PLAN))
}

#ifdef __cplusplus
}
#endif
