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
	control loops. Code split in 2 files, see flexsea_cmd_control_1.c
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-11-07 | jfduval | New file
	*
****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

//****************************************************************************
// Include(s)
//****************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include "../../flexsea-comm/inc/flexsea.h"	//dependency: flexsea-comm
#include "../inc/flexsea_system.h"
#include "../inc/flexsea_cmd_control.h"
#include <flexsea_board.h>

//Execute boards only:
#ifdef BOARD_TYPE_FLEXSEA_EXECUTE
#include "main.h"
#include "control.h"
#endif	//BOARD_TYPE_FLEXSEA_EXECUTE

//****************************************************************************
// Variable(s)
//****************************************************************************

//Will change this, but for now the payloads will be stored in: (ToDo eliminate soon)
uint8_t tmp_payload_xmit[PAYLOAD_BUF_LEN];
struct execute_s *exPtr2 = &exec1;

//****************************************************************************
// Function(s)
//****************************************************************************

//This gets called by flexsea_system's init_flexsea_payload_ptr(). Map all
//functions from this file to the array here. Failure to do so will send all
//commands to flexsea_payload_catchall().
void init_flexsea_payload_ptr_control_2(void)
{
	//Current - Gains:
	flexsea_payload_ptr[CMD_CTRL_I_G][RX_PTYPE_READ] = &rx_cmd_ctrl_i_g_rw;
	flexsea_payload_ptr[CMD_CTRL_I_G][RX_PTYPE_WRITE] = &rx_cmd_ctrl_i_g_w;
	flexsea_payload_ptr[CMD_CTRL_I_G][RX_PTYPE_REPLY] = &rx_cmd_ctrl_i_g_rr;

	//Position - Gains:
	flexsea_payload_ptr[CMD_CTRL_P_G][RX_PTYPE_READ] = &rx_cmd_ctrl_p_g_rw;
	flexsea_payload_ptr[CMD_CTRL_P_G][RX_PTYPE_WRITE] = &rx_cmd_ctrl_p_g_w;
	flexsea_payload_ptr[CMD_CTRL_P_G][RX_PTYPE_REPLY] = &rx_cmd_ctrl_p_g_rr;

	//Impedance - Gains:
	flexsea_payload_ptr[CMD_CTRL_Z_G][RX_PTYPE_READ] = &rx_cmd_ctrl_z_g_rw;
	flexsea_payload_ptr[CMD_CTRL_Z_G][RX_PTYPE_WRITE] = &rx_cmd_ctrl_z_g_w;
	flexsea_payload_ptr[CMD_CTRL_Z_G][RX_PTYPE_REPLY] = &rx_cmd_ctrl_z_g_rr;
}

//Transmit Control Current Gains:
//===============================

//Test code? Yes
void tx_cmd_ctrl_i_g_w(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
						uint16_t *len, int16_t kp, int16_t ki, int16_t kd)
{
	//Variable(s) & command:
	uint16_t index = 0;
	(*cmd) = CMD_CTRL_I_G;
	(*cmdType) = CMD_WRITE;

	//Data:
	SPLIT_16((uint16_t)kp, shBuf, &index);
	SPLIT_16((uint16_t)ki, shBuf, &index);
	SPLIT_16((uint16_t)kd, shBuf, &index);

	//Payload length:
	(*len) = index;
}

//Pack tx_cmd_ctrl_i_g_w()
void ptx_cmd_ctrl_i_g_w(uint8_t slaveId, uint16_t *numb, uint8_t *commStr, \
							int16_t kp, int16_t ki, int16_t kd)
{
	tx_cmd_ctrl_i_g_w(TX_N_DEFAULT, kp, ki, kd);
	pack(P_AND_S_DEFAULT, slaveId, NULL, numb, commStr);
}

//Test code? Yes
void tx_cmd_ctrl_i_g_r(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
						uint16_t *len)
{
	//Variable(s) & command:
	uint16_t index = 0;
	(*cmd) = CMD_CTRL_I_G;
	(*cmdType) = CMD_READ;

	//Data:
	(void)shBuf; //(none)

	//Payload length:
	(*len) = index;
}

//Receive Control Current Gains:
//==============================

//Test code? No
void rx_cmd_ctrl_i_g_w(uint8_t *buf, uint8_t *info)
{
	uint16_t index = P_DATA1;
	int16_t tmp_kp = 0, tmp_ki = 0, tmp_kd = 0;

	(void)info;	//Unused for now

	//Decode data:
	tmp_kp = (int16_t) REBUILD_UINT16(buf, &index);
	tmp_ki = (int16_t) REBUILD_UINT16(buf, &index);
	tmp_kd = (int16_t) REBUILD_UINT16(buf, &index);

	#ifdef BOARD_TYPE_FLEXSEA_EXECUTE

		//Store value:
		ctrl[0].current.gain.I_KP = tmp_kp;
		ctrl[0].current.gain.I_KI = tmp_ki;
		ctrl[0].current.gain.I_KD = tmp_kd;
		ctrl[0].current.error_sum = 0;
		//ToDo: do we need to call something?

	#else

		(void)buf;
		(void)tmp_kp;
		(void)tmp_ki;
		(void)tmp_kd;

	#endif	//BOARD_TYPE_FLEXSEA_EXECUTE
}

//Test code? No
void rx_cmd_ctrl_i_g_rw(uint8_t *buf, uint8_t *info)
{
	(void)info;

	#ifdef BOARD_TYPE_FLEXSEA_EXECUTE

		tx_cmd_ctrl_i_g_w(TX_N_DEFAULT, ctrl[0].current.gain.I_KP, \
						  ctrl[0].current.gain.I_KI, ctrl[0].current.gain.I_KD);
		//ToDo: should it be exec1.ctrl?
		packAndSend(P_AND_S_DEFAULT, buf[P_XID], info, SEND_TO_MASTER);

	#else
		(void)buf;
	#endif	//BOARD_TYPE_FLEXSEA_EXECUTE
}

//Test code? No
void rx_cmd_ctrl_i_g_rr(uint8_t *buf, uint8_t *info)
{
	uint16_t index = P_DATA1;
	int16_t tmp_kp = 0, tmp_ki = 0, tmp_kd = 0;

	(void)info;	//Unused for now

	//Decode data:
	tmp_kp = (int16_t) REBUILD_UINT16(buf, &index);
	tmp_ki = (int16_t) REBUILD_UINT16(buf, &index);
	tmp_kd = (int16_t) REBUILD_UINT16(buf, &index);

	#if((defined BOARD_TYPE_FLEXSEA_MANAGE) || (defined BOARD_TYPE_FLEXSEA_PLAN))

		//Store value:
		executePtrXid(&exPtr2, buf[P_XID]);
		exPtr2->ctrl.current.gain.I_KP = tmp_kp;
		exPtr2->ctrl.current.gain.I_KI = tmp_ki;
		exPtr2->ctrl.current.gain.I_KD = tmp_kd;

	#else

		(void)buf;
		(void)tmp_kp;
		(void)tmp_ki;
		(void)tmp_kd;

	#endif	//((defined BOARD_TYPE_FLEXSEA_MANAGE) || (defined BOARD_TYPE_FLEXSEA_PLAN))
}

//Transmit Control Position Gains:
//================================

//Test code? Yes
void tx_cmd_ctrl_p_g_w(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
						uint16_t *len, int16_t kp, int16_t ki, int16_t kd)
{
	//Variable(s) & command:
	uint16_t index = 0;
	(*cmd) = CMD_CTRL_P_G;
	(*cmdType) = CMD_WRITE;

	//Data:
	SPLIT_16((uint16_t)kp, shBuf, &index);
	SPLIT_16((uint16_t)ki, shBuf, &index);
	SPLIT_16((uint16_t)kd, shBuf, &index);

	//Payload length:
	(*len) = index;
}

//Pack tx_cmd_ctrl_mode_w()
void ptx_cmd_ctrl_p_g_w(uint8_t slaveId, uint16_t *numb, uint8_t *commStr, \
						int16_t kp, int16_t ki, int16_t kd)
{
	tx_cmd_ctrl_p_g_w(TX_N_DEFAULT, kp, ki, kd);
	pack(P_AND_S_DEFAULT, slaveId, NULL, numb, commStr);
}

//Test code? Yes
void tx_cmd_ctrl_p_g_r(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
						uint16_t *len)
{
	//Variable(s) & command:
	uint16_t index = 0;
	(*cmd) = CMD_CTRL_P_G;
	(*cmdType) = CMD_READ;

	//Data:
	(void)shBuf; //(none)

	//Payload length:
	(*len) = index;
}

//Receive Control Position Gains:
//===============================

//Test code? No
void rx_cmd_ctrl_p_g_w(uint8_t *buf, uint8_t *info)
{
	uint16_t index = P_DATA1;
	int16_t tmp_kp = 0, tmp_ki = 0, tmp_kd = 0;

	(void)info;	//Unused for now

	//Decode data:
	tmp_kp = (int16_t) REBUILD_UINT16(buf, &index);
	tmp_ki = (int16_t) REBUILD_UINT16(buf, &index);
	tmp_kd = (int16_t) REBUILD_UINT16(buf, &index);

	#ifdef BOARD_TYPE_FLEXSEA_EXECUTE

		//Store value:
		ctrl[0].position.gain.P_KP = tmp_kp;
		ctrl[0].position.gain.P_KI = tmp_ki;
		ctrl[0].position.gain.P_KD = tmp_kd;
		ctrl[0].position.error_sum = 0;
		//ToDo: do we need to call something?

	#else

		(void)buf;
		(void)tmp_kp;
		(void)tmp_ki;
		(void)tmp_kd;

	#endif	//BOARD_TYPE_FLEXSEA_EXECUTE
}

//Test code? No
void rx_cmd_ctrl_p_g_rw(uint8_t *buf, uint8_t *info)
{
	(void)info;

	#ifdef BOARD_TYPE_FLEXSEA_EXECUTE

		tx_cmd_ctrl_p_g_w(TX_N_DEFAULT, ctrl[0].position.gain.P_KP, \
						  ctrl[0].position.gain.P_KI, ctrl[0].position.gain.P_KD);
		//ToDo: should it be exec1.ctrl?
		packAndSend(P_AND_S_DEFAULT, buf[P_XID], info, SEND_TO_MASTER);

	#else
		(void)buf;
	#endif	//BOARD_TYPE_FLEXSEA_EXECUTE
}

//Test code? No
void rx_cmd_ctrl_p_g_rr(uint8_t *buf, uint8_t *info)
{
	uint16_t index = P_DATA1;
	int16_t tmp_kp = 0, tmp_ki = 0, tmp_kd = 0;

	(void)info;	//Unused for now

	//Decode data:
	tmp_kp = (int16_t) REBUILD_UINT16(buf, &index);
	tmp_ki = (int16_t) REBUILD_UINT16(buf, &index);
	tmp_kd = (int16_t) REBUILD_UINT16(buf, &index);

	#if((defined BOARD_TYPE_FLEXSEA_MANAGE) || (defined BOARD_TYPE_FLEXSEA_PLAN))

		//Store value:
		executePtrXid(&exPtr2, buf[P_XID]);
		exPtr2->ctrl.position.gain.P_KP = tmp_kp;
		exPtr2->ctrl.position.gain.P_KI = tmp_ki;
		exPtr2->ctrl.position.gain.P_KD = tmp_kd;
		//ToDo: can't be exec1!

	#else

		(void)buf;
		(void)tmp_kp;
		(void)tmp_ki;
		(void)tmp_kd;

	#endif	//((defined BOARD_TYPE_FLEXSEA_MANAGE) || (defined BOARD_TYPE_FLEXSEA_PLAN))
}

//Transmit Control Impedance Gains:
//=================================

//Test code? Yes
void tx_cmd_ctrl_z_g_w(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
						uint16_t *len, int16_t zk, int16_t zb, int16_t zi)
{
	//Variable(s) & command:
	uint16_t index = 0;
	(*cmd) = CMD_CTRL_Z_G;
	(*cmdType) = CMD_WRITE;

	//Data:
	SPLIT_16((uint16_t)zk, shBuf, &index);
	SPLIT_16((uint16_t)zb, shBuf, &index);
	SPLIT_16((uint16_t)zi, shBuf, &index);

	//Payload length:
	(*len) = index;
}

//Pack tx_cmd_ctrl_mode_w()
void ptx_cmd_ctrl_z_g_w(uint8_t slaveId, uint16_t *numb, uint8_t *commStr, \
						int16_t zk, int16_t zb, int16_t zi)
{
	tx_cmd_ctrl_z_g_w(TX_N_DEFAULT, zk, zb, zi);
	pack(P_AND_S_DEFAULT, slaveId, NULL, numb, commStr);
}

//Test code? Yes
void tx_cmd_ctrl_z_g_r(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
						uint16_t *len)
{
	//Variable(s) & command:
	uint16_t index = 0;
	(*cmd) = CMD_CTRL_Z_G;
	(*cmdType) = CMD_READ;

	//Data:
	(void)shBuf; //(none)

	//Payload length:
	(*len) = index;
}

//Receive Control Impedance Gains:
//================================

//Test code? No
void rx_cmd_ctrl_z_g_w(uint8_t *buf, uint8_t *info)
{
	uint16_t index = P_DATA1;
	int16_t tmp_zk = 0, tmp_zb = 0, tmp_zi = 0;

	(void)info;	//Unused for now

	//Decode data:
	tmp_zk = (int16_t) REBUILD_UINT16(buf, &index);
	tmp_zb = (int16_t) REBUILD_UINT16(buf, &index);
	tmp_zi = (int16_t) REBUILD_UINT16(buf, &index);

	#ifdef BOARD_TYPE_FLEXSEA_EXECUTE

		//Store value:
		ctrl[0].impedance.gain.Z_K = tmp_zk;
		ctrl[0].impedance.gain.Z_B = tmp_zb;
		ctrl[0].impedance.gain.Z_I = tmp_zi;
		ctrl[0].impedance.error_sum = 0;
		//ToDo: do we need to call something?

	#else

		(void)buf;
		(void)tmp_zk;
		(void)tmp_zb;
		(void)tmp_zi;

	#endif	//BOARD_TYPE_FLEXSEA_EXECUTE
}

//Test code? No
void rx_cmd_ctrl_z_g_rw(uint8_t *buf, uint8_t *info)
{
	(void)info;

	#ifdef BOARD_TYPE_FLEXSEA_EXECUTE

		tx_cmd_ctrl_z_g_w(TX_N_DEFAULT, ctrl[0].impedance.gain.Z_K, \
						  ctrl[0].impedance.gain.Z_B, ctrl[0].impedance.gain.Z_I);
		//ToDo: should it be exec1.ctrl?
		packAndSend(P_AND_S_DEFAULT, buf[P_XID], info, SEND_TO_MASTER);

	#else
		(void)buf;
	#endif	//BOARD_TYPE_FLEXSEA_EXECUTE
}

//Test code? No
void rx_cmd_ctrl_z_g_rr(uint8_t *buf, uint8_t *info)
{
	uint16_t index = P_DATA1;
	int16_t tmp_zk = 0, tmp_zb = 0, tmp_zi = 0;

	(void)info;	//Unused for now

	//Decode data:
	tmp_zk = (int16_t) REBUILD_UINT16(buf, &index);
	tmp_zb = (int16_t) REBUILD_UINT16(buf, &index);
	tmp_zi = (int16_t) REBUILD_UINT16(buf, &index);

	#if((defined BOARD_TYPE_FLEXSEA_MANAGE) || (defined BOARD_TYPE_FLEXSEA_PLAN))

		//Store value:
		executePtrXid(&exPtr2, buf[P_XID]);
		exPtr2->ctrl.impedance.gain.Z_K = tmp_zk;
		exPtr2->ctrl.impedance.gain.Z_B = tmp_zb;
		exPtr2->ctrl.impedance.gain.Z_I = tmp_zi;

	#else

		(void)buf;
		(void)tmp_zk;
		(void)tmp_zb;
		(void)tmp_zi;

	#endif	//((defined BOARD_TYPE_FLEXSEA_MANAGE) || (defined BOARD_TYPE_FLEXSEA_PLAN))
}

#ifdef __cplusplus
}
#endif
