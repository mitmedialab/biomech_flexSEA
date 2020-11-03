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
	[This file] flexsea_cmd_control_1: commands specific to the motor &
	control loops. Code split in 2 files, see flexsea_cmd_control_2.c
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-09 | jfduval | Initial GPL-3.0 release
	* 2016-11-07 | jfduval | Renamed *_1, updated to new convention
****************************************************************************/

/*Note: flexsea_cmd_control.c was getting too long so it was split in two
files. *_1 has all the setpoints, plus the In Control command. *_2 has the
Set Gains functions.
*/

#ifdef __cplusplus
extern "C" {
#endif

//****************************************************************************
// Include(s)
//****************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include "../inc/flexsea_system.h"
#include "../inc/flexsea_cmd_control.h"
#include "../../flexsea-comm/inc/flexsea.h"	//dependency: flexsea-comm
#include <flexsea_board.h>

//Execute boards only:
#ifdef BOARD_TYPE_FLEXSEA_EXECUTE
#include "main.h"
#include "motor.h"
#include "trapez.h"
#include "control.h"
#endif	//BOARD_TYPE_FLEXSEA_EXECUTE

//****************************************************************************
// Variable(s)
//****************************************************************************

struct execute_s *exPtr1 = &exec1;

//****************************************************************************
// Function(s)
//****************************************************************************

//This gets called by flexsea_system's init_flexsea_payload_ptr(). Map all
//functions from this file to the array here. Failure to do so will send all
//commands to flexsea_payload_catchall().
void init_flexsea_payload_ptr_control_1(void)
{
	//Control mode:
	flexsea_payload_ptr[CMD_CTRL_MODE][RX_PTYPE_READ] = &rx_cmd_ctrl_mode_rw;
	flexsea_payload_ptr[CMD_CTRL_MODE][RX_PTYPE_WRITE] = &rx_cmd_ctrl_mode_w;
	flexsea_payload_ptr[CMD_CTRL_MODE][RX_PTYPE_REPLY] = &rx_cmd_ctrl_mode_rr;

	//Open - Setpoint:
	flexsea_payload_ptr[CMD_CTRL_O][RX_PTYPE_READ] = &rx_cmd_ctrl_o_rw;
	flexsea_payload_ptr[CMD_CTRL_O][RX_PTYPE_WRITE] = &rx_cmd_ctrl_o_w;
	flexsea_payload_ptr[CMD_CTRL_O][RX_PTYPE_REPLY] = &rx_cmd_ctrl_o_rr;

	//Position - Setpoint:
	flexsea_payload_ptr[CMD_CTRL_P][RX_PTYPE_READ] = &rx_cmd_ctrl_p_rw;
	flexsea_payload_ptr[CMD_CTRL_P][RX_PTYPE_WRITE] = &rx_cmd_ctrl_p_w;
	flexsea_payload_ptr[CMD_CTRL_P][RX_PTYPE_REPLY] = &rx_cmd_ctrl_p_rr;

	//Current - Setpoint:
	flexsea_payload_ptr[CMD_CTRL_I][RX_PTYPE_READ] = &rx_cmd_ctrl_i_rw;
	flexsea_payload_ptr[CMD_CTRL_I][RX_PTYPE_WRITE] = &rx_cmd_ctrl_i_w;
	flexsea_payload_ptr[CMD_CTRL_I][RX_PTYPE_REPLY] = &rx_cmd_ctrl_i_rr;

	//Misc:
//	flexsea_payload_ptr[CMD_IN_CONTROL][RX_PTYPE_READ] = &rx_cmd_in_control;
}

//Transmit Control Mode:
//======================

//Test code? Yes
void tx_cmd_ctrl_mode_w(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
						uint16_t *len, uint8_t ctrlMode)
{
	//Variable(s) & command:
	uint16_t index = 0;
	(*cmd) = CMD_CTRL_MODE;
	(*cmdType) = CMD_WRITE;

	//Data:
	shBuf[index++] = ctrlMode;

	//Payload length:
	(*len) = index;
}

//Pack tx_cmd_ctrl_mode_w()
void ptx_cmd_ctrl_mode_w(uint8_t slaveId, uint16_t *numb, uint8_t *commStr, \
							uint8_t ctrlMode)
{
	tx_cmd_ctrl_mode_w(TX_N_DEFAULT, ctrlMode);
	pack(P_AND_S_DEFAULT, slaveId, NULL, numb, commStr);
}

//Test code? Yes
void tx_cmd_ctrl_mode_r(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
						uint16_t *len)
{
	//Variable(s) & command:
	uint16_t index = 0;
	(*cmd) = CMD_CTRL_MODE;
	(*cmdType) = CMD_READ;

	//Data:
	(void)shBuf; //(none)

	//Payload length:
	(*len) = index;
}

//Receive Control Mode:
//======================

//Test code? No
void rx_cmd_ctrl_mode_w(uint8_t *buf, uint8_t *info)
{
	(void)info;

	#ifdef BOARD_TYPE_FLEXSEA_EXECUTE
		control_strategy(buf[P_DATA1],0);
	#else
		(void)buf;
	#endif	//BOARD_TYPE_FLEXSEA_EXECUTE
}

//Test code? No
void rx_cmd_ctrl_mode_rw(uint8_t *buf, uint8_t *info)
{
	(void)info;

	#ifdef BOARD_TYPE_FLEXSEA_EXECUTE

		tx_cmd_ctrl_mode_w(TX_N_DEFAULT, ctrl[0].active_ctrl);
		packAndSend(P_AND_S_DEFAULT, buf[P_XID], info, SEND_TO_MASTER);

	#else
		(void)buf;
	#endif	//BOARD_TYPE_FLEXSEA_EXECUTE
}

//Test code? No
void rx_cmd_ctrl_mode_rr(uint8_t *buf, uint8_t *info)
{
	(void)info;

	#if((defined BOARD_TYPE_FLEXSEA_MANAGE) || (defined BOARD_TYPE_FLEXSEA_PLAN))

		//Decode data:
		uint8_t controller = buf[P_DATA1];

		//Store value:
		executePtrXid(&exPtr1, buf[P_XID]);
		exPtr1->ctrl.active_ctrl = controller;

	#else
		(void)buf;
	#endif	//((defined BOARD_TYPE_FLEXSEA_MANAGE) || (defined BOARD_TYPE_FLEXSEA_PLAN))
}

//Transmit Control Current Setpoint:
//==================================

//Test code? Yes
void tx_cmd_ctrl_i_w(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
						uint16_t *len, int32_t currentSetpoint)
{
	//Variable(s) & command:
	uint16_t index = 0;
	(*cmd) = CMD_CTRL_I;
	(*cmdType) = CMD_WRITE;

	//Data:
	#ifdef BOARD_TYPE_FLEXSEA_EXECUTE
		//Execute: reply only
		(void)currentSetpoint;
		SPLIT_32((uint32_t)ctrl[0].current.actual_val, shBuf, &index);
		SPLIT_32((uint32_t)ctrl[0].current.setpoint_val, shBuf, &index);
	#else
		//Other boards can write a new setpoint
		SPLIT_32((uint32_t)0, shBuf, &index);
		SPLIT_32((uint32_t)currentSetpoint, shBuf, &index);
	#endif

	//Payload length:
	(*len) = index;
}

//Pack tx_cmd_ctrl_i_w()
void ptx_cmd_ctrl_i_w(uint8_t slaveId, uint16_t *numb, uint8_t *commStr, \
						int32_t currentSetpoint)
{
	tx_cmd_ctrl_i_w(TX_N_DEFAULT, currentSetpoint);
	pack(P_AND_S_DEFAULT, slaveId, NULL, numb, commStr);
}

//Test code? Yes
void tx_cmd_ctrl_i_r(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
						uint16_t *len)
{
	//Variable(s) & command:
	uint16_t index = 0;
	(*cmd) = CMD_CTRL_I;
	(*cmdType) = CMD_READ;

	//Data:
	(void)shBuf; //(none)

	//Payload length:
	(*len) = index;
}

//Receive Control Current Setpoint:
//=================================

//Test code? No
void rx_cmd_ctrl_i_w(uint8_t *buf, uint8_t *info)
{
	uint16_t index = P_DATA1;
	int32_t tmp_wanted_current = 0, tmp_measured_current = 0;

	tmp_measured_current = (int32_t) REBUILD_UINT32(buf, &index);
	tmp_wanted_current = (int32_t) REBUILD_UINT32(buf, &index);

	(void)info;

	#ifdef BOARD_TYPE_FLEXSEA_EXECUTE

		//Only change the setpoint if we are in current control mode:
		if(ctrl[0].active_ctrl == CTRL_CURRENT)
		{
			ctrl[0].current.setpoint_val = tmp_wanted_current;
		}
		
		(void)tmp_measured_current;

	#else

		(void)buf;
		(void)tmp_measured_current;
		(void)tmp_wanted_current;

	#endif	//BOARD_TYPE_FLEXSEA_EXECUTE
}

//Test code? No
void rx_cmd_ctrl_i_rw(uint8_t *buf, uint8_t *info)
{
	(void)info;

	#ifdef BOARD_TYPE_FLEXSEA_EXECUTE

		tx_cmd_ctrl_i_w(TX_N_DEFAULT, exec1.ctrl.current.setpoint_val);
		packAndSend(P_AND_S_DEFAULT, buf[P_XID], info, SEND_TO_MASTER);

	#else

		(void)buf;

	#endif	//BOARD_TYPE_FLEXSEA_EXECUTE
}

//Test code? No
void rx_cmd_ctrl_i_rr(uint8_t *buf, uint8_t *info)
{
	uint16_t index = P_DATA1;
	int32_t tmp_wanted_current = 0, tmp_measured_current = 0;

	tmp_measured_current = (int32_t) REBUILD_UINT32(buf, &index);
	tmp_wanted_current = (int32_t) REBUILD_UINT32(buf, &index);

	(void)info;
	(void)tmp_wanted_current;

	#if((defined BOARD_TYPE_FLEXSEA_MANAGE) || (defined BOARD_TYPE_FLEXSEA_PLAN))

	//Store value:
	executePtrXid(&exPtr1, buf[P_XID]);
	exPtr1->current = tmp_measured_current;

	#else

		(void)buf;
		(void)tmp_measured_current;

	#endif	//((defined BOARD_TYPE_FLEXSEA_MANAGE) || (defined BOARD_TYPE_FLEXSEA_PLAN))
}

//Transmit Control Open Setpoint:
//===============================

//Test code? Yes
void tx_cmd_ctrl_o_w(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
						uint16_t *len, int32_t setpoint)
{
	//Variable(s) & command:
	uint16_t index = 0;
	(*cmd) = CMD_CTRL_O;
	(*cmdType) = CMD_WRITE;

	//Data:
	#ifdef BOARD_TYPE_FLEXSEA_EXECUTE
		//Execute: reply only
		(void)setpoint;
		SPLIT_32((uint32_t)exec1.ctrl.pwm, shBuf, &index);
		//ToDo: not sure that this field is updated
	#else
		//Other boards can write a new setpoint
		SPLIT_32((uint32_t)setpoint, shBuf, &index);
	#endif

	//Payload length:
	(*len) = index;
}

//Pack tx_cmd_ctrl_o()
void ptx_cmd_ctrl_o_w(	uint8_t slaveId, uint16_t *numb, uint8_t *commStr, \
						int32_t setpoint)
{
	tx_cmd_ctrl_o_w(TX_N_DEFAULT, setpoint);
	pack(P_AND_S_DEFAULT, slaveId, NULL, numb, commStr);
}

//Test code? Yes
void tx_cmd_ctrl_o_r(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
						uint16_t *len)
{
	//Variable(s) & command:
	uint16_t index = 0;
	(*cmd) = CMD_CTRL_O;
	(*cmdType) = CMD_READ;

	//Data:
	(void)shBuf; //(none)

	//Payload length:
	(*len) = index;
}

//Receive Control Open Setpoint:
//==============================

//Test code? No
void rx_cmd_ctrl_o_w(uint8_t *buf, uint8_t *info)
{
	uint16_t index = P_DATA1;
	int32_t tmp = 0;

	tmp = (int32_t) REBUILD_UINT32(buf, &index);

	(void)info;

	#ifdef BOARD_TYPE_FLEXSEA_EXECUTE

		//Only change the setpoint if we are in open control mode:
		if(ctrl[0].active_ctrl == CTRL_OPEN)
		{
			setMotorVoltage(tmp,0);
		}

	#else

		(void)buf;
		(void)tmp;

	#endif	//BOARD_TYPE_FLEXSEA_EXECUTE
}

//Test code? No
void rx_cmd_ctrl_o_rw(uint8_t *buf, uint8_t *info)
{
	(void)info;

	#ifdef BOARD_TYPE_FLEXSEA_EXECUTE

		tx_cmd_ctrl_o_w(TX_N_DEFAULT, exec1.ctrl.pwm);	//ToDo: is that the right variable?
		packAndSend(P_AND_S_DEFAULT, buf[P_XID], info, SEND_TO_MASTER);

	#else

		(void)buf;

	#endif	//BOARD_TYPE_FLEXSEA_EXECUTE
}

//Test code? No
void rx_cmd_ctrl_o_rr(uint8_t *buf, uint8_t *info)
{
	uint16_t index = P_DATA1;
	int32_t tmp = 0;

	tmp = (int32_t) REBUILD_UINT32(buf, &index);

	(void)info;

	#if((defined BOARD_TYPE_FLEXSEA_MANAGE) || (defined BOARD_TYPE_FLEXSEA_PLAN))

		//Store value:
		executePtrXid(&exPtr1, buf[P_XID]);
		exPtr1->ctrl.pwm = tmp;

	#else

		(void)buf;
		(void)tmp;

	#endif	//((defined BOARD_TYPE_FLEXSEA_MANAGE) || (defined BOARD_TYPE_FLEXSEA_PLAN))
}

//Transmit Control Position Setpoint:
//===================================

//Test code? Yes
void tx_cmd_ctrl_p_w(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
						uint16_t *len, int32_t pos, int32_t posi, int32_t posf,\
						int32_t spdm, int32_t acc)
{
	//Variable(s) & command:
	uint16_t index = 0;
	(*cmd) = CMD_CTRL_P;
	(*cmdType) = CMD_WRITE;

	//Data:
	SPLIT_32((uint32_t)pos, shBuf, &index);
	SPLIT_32((uint32_t)posi, shBuf, &index);
	SPLIT_32((uint32_t)posf, shBuf, &index);
	SPLIT_32((uint32_t)spdm, shBuf, &index);
	SPLIT_32((uint32_t)acc, shBuf, &index);

	//Payload length:
	(*len) = index;
}

//Pack tx_cmd_ctrl_p_w()
void ptx_cmd_ctrl_p_w(uint8_t slaveId, uint16_t *numb, uint8_t *commStr, \
						int32_t pos, int32_t posi, int32_t posf,\
						int32_t spdm, int32_t acc)
{
	tx_cmd_ctrl_p_w(TX_N_DEFAULT, pos, posi, posf, spdm, acc);
	pack(P_AND_S_DEFAULT, slaveId, NULL, numb, commStr);
}

//Test code? Yes
void tx_cmd_ctrl_p_r(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
						uint16_t *len)
{
	//Variable(s) & command:
	uint16_t index = 0;
	(*cmd) = CMD_CTRL_P;
	(*cmdType) = CMD_READ;

	//Data:
	(void)shBuf; //(none)

	//Payload length:
	(*len) = index;
}

//Receive Control Position Setpoint:
//==================================

//Test code? No
void rx_cmd_ctrl_p_w(uint8_t *buf, uint8_t *info)
{
	uint16_t index = P_DATA1;
	int32_t tmp_pos = 0, tmp_posi = 0, tmp_posf = 0, tmp_spdm = 0, tmp_acc = 0;

	(void)info;	//Unused for now

	//Decode data:
	tmp_pos = (int32_t) REBUILD_UINT32(buf, &index);
	tmp_posi = (int32_t) REBUILD_UINT32(buf, &index);
	tmp_posf = (int32_t) REBUILD_UINT32(buf, &index);
	tmp_spdm = (int32_t) REBUILD_UINT32(buf, &index);
	tmp_acc = (int32_t) REBUILD_UINT32(buf, &index);

	#ifdef BOARD_TYPE_FLEXSEA_EXECUTE

		int16_t tmp_z_k = 0, tmp_z_b = 0, tmp_z_i = 0;

		ctrl[0].position.posf = tmp_posf;
		ctrl[0].position.spdm = tmp_spdm;
		ctrl[0].position.acc = tmp_acc;

		if(ctrl[0].active_ctrl == CTRL_POSITION)
		{
			#ifdef USE_TRAPEZ
			ctrl[0].position.posi = ctrl[0].position.setp;
			steps = trapez_gen_motion_1(ctrl[0].position.posi, ctrl[0].position.posf,\
										ctrl[0].position.spdm, \
										ctrl[0].position.acc = tmp_acc);
			#else
			ctrl[0].position.setp = tmp_posf;
			(void)buf;
			(void)tmp_pos;
			(void)tmp_posi;
			(void)tmp_spdm;
			(void)tmp_acc;
			#endif
		}
		else if(ctrl[0].active_ctrl == CTRL_IMPEDANCE)
		{
			//Backup gains
			tmp_z_k = ctrl[0].impedance.gain.Z_K;
			tmp_z_b = ctrl[0].impedance.gain.Z_B;
			tmp_z_i = ctrl[0].impedance.gain.Z_I;

			//Zero them
			ctrl[0].impedance.gain.Z_K = 0;
			ctrl[0].impedance.gain.Z_B = 0;
			ctrl[0].impedance.gain.Z_I = 0;

			#ifdef USE_TRAPEZ
			//New trajectory
			ctrl[0].position.posi = ctrl[0].impedance.setpoint_val;
			steps = trapez_gen_motion_1(ctrl[0].position.posi, ctrl[0].position.posf,\
										ctrl[0].position.spdm, \
										ctrl[0].position.acc = tmp_acc);
			#else
			ctrl[0].impedance.setpoint_val = tmp_posf;
			#endif //USE_TRAPEZ

			//Restore gains
			ctrl[0].impedance.gain.Z_K = tmp_z_k;
			ctrl[0].impedance.gain.Z_B = tmp_z_b;
			ctrl[0].impedance.gain.Z_I = tmp_z_i;
		}

	#else

		(void)buf;
		(void)tmp_pos;
		(void)tmp_posi;
		(void)tmp_posf;
		(void)tmp_spdm;
		(void)tmp_acc;

	#endif	//BOARD_TYPE_FLEXSEA_EXECUTE
}

//Test code? No
void rx_cmd_ctrl_p_rw(uint8_t *buf, uint8_t *info)
{
	(void)info;

	#ifdef BOARD_TYPE_FLEXSEA_EXECUTE

		tx_cmd_ctrl_p_w(TX_N_DEFAULT, ctrl[0].position.pos, ctrl[0].position.posi, \
						ctrl[0].position.posf, ctrl[0].position.spdm, \
						ctrl[0].position.acc);
		packAndSend(P_AND_S_DEFAULT, buf[P_XID], info, SEND_TO_MASTER);

	#else
		(void)buf;
	#endif	//BOARD_TYPE_FLEXSEA_EXECUTE
}

//Test code? No
void rx_cmd_ctrl_p_rr(uint8_t *buf, uint8_t *info)
{
	uint16_t index = P_DATA1;
	int32_t tmp_pos = 0, tmp_posi = 0, tmp_posf = 0, tmp_spdm = 0, tmp_acc = 0;

	(void)info;	//Unused for now

	//Decode data:
	tmp_pos = (int32_t) REBUILD_UINT32(buf, &index);
	tmp_posi = (int32_t) REBUILD_UINT32(buf, &index);
	tmp_posf = (int32_t) REBUILD_UINT32(buf, &index);
	tmp_spdm = (int32_t) REBUILD_UINT32(buf, &index);
	tmp_acc = (int32_t) REBUILD_UINT32(buf, &index);

	#if((defined BOARD_TYPE_FLEXSEA_MANAGE) || (defined BOARD_TYPE_FLEXSEA_PLAN))

		//Store value:
		executePtrXid(&exPtr1, buf[P_XID]);
		exPtr1->ctrl.position.pos = tmp_pos;
		exPtr1->ctrl.position.posi = tmp_posi;
		exPtr1->ctrl.position.posf = tmp_posf;
		exPtr1->ctrl.position.spdm = tmp_spdm;
		exPtr1->ctrl.position.acc = tmp_acc;

	#else

		(void)buf;
		(void)tmp_pos;
		(void)tmp_posi;
		(void)tmp_posf;
		(void)tmp_spdm;
		(void)tmp_acc;

	#endif	//((defined BOARD_TYPE_FLEXSEA_MANAGE) || (defined BOARD_TYPE_FLEXSEA_PLAN))
}

#ifdef __cplusplus
}
#endif
