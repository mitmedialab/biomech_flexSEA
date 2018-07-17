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
	[This file] flexsea_cmd_data: Data Commands
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-09 | jfduval | Initial GPL-3.0 release
	* 2016-11-09 | jfduval | Updated to new stack standard
	* 2016-12-02 | jfduval | Re-integrating User R/W
****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

//****************************************************************************
// Include(s)
//****************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include "../../biomech_flexsea-comm/inc/flexsea.h"	//dependency: flexsea-comm
#include "../inc/flexsea_system.h"
#include "../inc/flexsea_cmd_data.h"
#include <flexsea_board.h>

//Manage boards only:
#ifdef BOARD_TYPE_FLEXSEA_MANAGE
#include "user-mn.h"
#include "main.h"
#include <imu.h>
#include <adc.h>
#endif	//BOARD_TYPE_FLEXSEA_MANAGE

#if (defined BOARD_TYPE_FLEXSEA_EXECUTE || defined BOARD_TYPE_FLEXSEA_GOSSIP \
	|| defined BOARD_TYPE_FLEXSEA_STRAIN_AMP)
	#include "main.h"
	#if(!defined BOARD_SUBTYPE_RIGID && !defined BOARD_SUBTYPE_POCKET)
		#include "user-ex.h"
	#else
		#include "user-ex-rigid.h"
	#endif
	#ifdef USE_IMU
		#include "imu.h"
	#endif
	#ifdef USE_AS5048B
		#include "flexsea_global_structs.h"
		#include "mag_encoders.h"
	#endif
	#include "strain.h"
	#ifndef BOARD_TYPE_FLEXSEA_STRAIN_AMP
		#include "safety.h"
		#include "analog.h"
		#include "control.h"
	#endif
#endif

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
void init_flexsea_payload_ptr_data(void)
{
	//Read All:
	flexsea_payload_ptr[CMD_READ_ALL][RX_PTYPE_READ] = &rx_cmd_data_read_all_rw;
	flexsea_payload_ptr[CMD_READ_ALL][RX_PTYPE_REPLY] = &rx_cmd_data_read_all_rr;

	//User R/W:
	flexsea_payload_ptr[CMD_USER_DATA][RX_PTYPE_READ] = &rx_cmd_data_user_rw;
	flexsea_payload_ptr[CMD_USER_DATA][RX_PTYPE_REPLY] = &rx_cmd_data_user_rr;
	flexsea_payload_ptr[CMD_USER_DATA][RX_PTYPE_WRITE] = &rx_cmd_data_user_w;

	flexsea_multipayload_ptr[CMD_USER_DATA][RX_PTYPE_READ] = &rx_multi_cmd_data_user_rw;
	flexsea_multipayload_ptr[CMD_USER_DATA][RX_PTYPE_REPLY] = &rx_multi_cmd_data_user_rr;
	flexsea_multipayload_ptr[CMD_USER_DATA][RX_PTYPE_WRITE] = &rx_multi_cmd_data_user_w;
}

void tx_cmd_data_read_all_r(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
					uint16_t *len)
{
	uint16_t index = 0;

	//Formatting:
	(*cmd) = CMD_READ_ALL;
	(*cmdType) = CMD_READ;

	//Data:
	//(none)
	(void)shBuf;

	//Payload length:
	(*len) = index;
}

void tx_cmd_data_read_all_w(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
					uint16_t *len)
{
	uint16_t index = 0;

	//Formatting:
	(*cmd) = CMD_READ_ALL;
	(*cmdType) = CMD_WRITE;

	//Data:
	#ifdef BOARD_TYPE_FLEXSEA_EXECUTE

	#ifdef USE_IMU
		SPLIT_16((uint16_t)imu.gyro.x, shBuf, &index);
		SPLIT_16((uint16_t)imu.gyro.y, shBuf, &index);
		SPLIT_16((uint16_t)imu.gyro.z, shBuf, &index);
		SPLIT_16((uint16_t)imu.accel.x, shBuf, &index);
		SPLIT_16((uint16_t)imu.accel.y, shBuf, &index);
		SPLIT_16((uint16_t)imu.accel.z, shBuf, &index);
	#else
		SPLIT_16((uint16_t)0, shBuf, &index);
		SPLIT_16((uint16_t)0, shBuf, &index);
		SPLIT_16((uint16_t)0, shBuf, &index);
		SPLIT_16((uint16_t)0, shBuf, &index);
		SPLIT_16((uint16_t)0, shBuf, &index);
		SPLIT_16((uint16_t)0, shBuf, &index);
	#endif

	SPLIT_16(strain_read(), shBuf, &index);
	SPLIT_16(read_analog(0), shBuf, &index);
	SPLIT_16(read_analog(1), shBuf, &index);

	SPLIT_32((uint32_t)(*exec1.enc_ang), shBuf, &index);
	SPLIT_32((uint32_t)ctrl[0].current.actual_val, shBuf, &index);

	#if(!defined BOARD_SUBTYPE_RIGID && !defined BOARD_SUBTYPE_POCKET)
		shBuf[index++] = safety_cop.v_vb;
		shBuf[index++] = safety_cop.v_vg;
		shBuf[index++] = safety_cop.temperature;
		shBuf[index++] = safety_cop.status1;
		shBuf[index++] = safety_cop.status2;
	#else
		shBuf[index++] = 0;
		shBuf[index++] = 0;
		shBuf[index++] = 0;
		shBuf[index++] = 0;
		shBuf[index++] = 0;
	#endif //BOARD_SUBTYPE_RIGID

	#endif	//BOARD_TYPE_FLEXSEA_EXECUTE

	#ifdef BOARD_TYPE_FLEXSEA_MANAGE

	SPLIT_16((uint16_t)imu.gyro.x, shBuf, &index);
	SPLIT_16((uint16_t)imu.gyro.y, shBuf, &index);
	SPLIT_16((uint16_t)imu.gyro.z, shBuf, &index);
	SPLIT_16((uint16_t)imu.accel.x, shBuf, &index);
	SPLIT_16((uint16_t)imu.accel.y, shBuf, &index);
	SPLIT_16((uint16_t)imu.accel.z, shBuf, &index);

	shBuf[index++] = manag1.sw1;

	SPLIT_16((uint16_t)adc_results[0], shBuf, &index);
	SPLIT_16((uint16_t)adc_results[1], shBuf, &index);
	SPLIT_16((uint16_t)adc_results[2], shBuf, &index);
	SPLIT_16((uint16_t)adc_results[3], shBuf, &index);
	SPLIT_16((uint16_t)adc_results[4], shBuf, &index);
	SPLIT_16((uint16_t)adc_results[5], shBuf, &index);
	SPLIT_16((uint16_t)adc_results[6], shBuf, &index);
	SPLIT_16((uint16_t)adc_results[7], shBuf, &index);

	//...

	#endif  //BOARD_TYPE_FLEXSEA_MANAGE

	#ifdef BOARD_TYPE_FLEXSEA_GOSSIP

	SPLIT_16((uint16_t)imu.gyro.x, shBuf, &index);
	SPLIT_16((uint16_t)imu.gyro.y, shBuf, &index);
	SPLIT_16((uint16_t)imu.gyro.z, shBuf, &index);
	SPLIT_16((uint16_t)imu.accel.x, shBuf, &index);
	SPLIT_16((uint16_t)imu.accel.y, shBuf, &index);
	SPLIT_16((uint16_t)imu.accel.z, shBuf, &index);
	SPLIT_16((uint16_t)imu.magneto.x, shBuf, &index);
	SPLIT_16((uint16_t)imu.magneto.y, shBuf, &index);
	SPLIT_16((uint16_t)imu.magneto.z, shBuf, &index);

	shBuf[index++] = 0;
	shBuf[index++] = 0;

	SPLIT_16((uint16_t)capSense[0], shBuf, &index);
	SPLIT_16((uint16_t)capSense[1], shBuf, &index);
	SPLIT_16((uint16_t)capSense[2], shBuf, &index);
	SPLIT_16((uint16_t)capSense[3], shBuf, &index);

	#endif  //BOARD_TYPE_FLEXSEA_GOSSIP

	#ifdef BOARD_TYPE_FLEXSEA_STRAIN_AMP

	//Arguments:

	//Compressed Strain:
	prepStrainDataForComm();
	shBuf[index++] = strain1.compressedBytes[0];
	shBuf[index++] = strain1.compressedBytes[1];
	shBuf[index++] = strain1.compressedBytes[2];
	shBuf[index++] = strain1.compressedBytes[3];
	shBuf[index++] = strain1.compressedBytes[4];
	shBuf[index++] = strain1.compressedBytes[5];
	shBuf[index++] = strain1.compressedBytes[6];
	shBuf[index++] = strain1.compressedBytes[7];
	shBuf[index++] = strain1.compressedBytes[8];

	#endif 	//BOARD_TYPE_FLEXSEA_STRAIN_AMP

	#ifdef BOARD_TYPE_FLEXSEA_PLAN

	(void)shBuf;

	#endif	//BOARD_TYPE_FLEXSEA_PLAN

	//Payload length:
	(*len) = index;
}

void rx_cmd_data_read_all_rw(uint8_t *buf, uint8_t *info)
{
	tx_cmd_data_read_all_w(TX_N_DEFAULT);
	packAndSend(P_AND_S_DEFAULT, buf[P_XID], info, SEND_TO_MASTER);
}

void rx_cmd_data_read_all_rr(uint8_t *buf, uint8_t *info)
{
	(void)info;	//Unused for now

	#if((defined BOARD_TYPE_FLEXSEA_MANAGE) || (defined BOARD_TYPE_FLEXSEA_PLAN))

		uint16_t index = P_DATA1;
		uint8_t tmp = 0, baseAddr = 0;

		//Structure pointers:
		struct execute_s *exec_s_ptr = &exec1;
		struct manage_s *mn_s_ptr = &manag1;
		struct gossip_s *go_s_ptr = &gossip1;
		struct strain_s *st_s_ptr = &strain1;
		executePtrXid(&exec_s_ptr, buf[P_XID]);
		managePtrXid(&mn_s_ptr, buf[P_XID]);
		gossipPtrXid(&go_s_ptr, buf[P_XID]);
		strainPtrXid(&st_s_ptr, buf[P_XID]);

		//Extract base address:
		tmp = buf[P_XID]/10;
		baseAddr = 10*tmp;

		switch(baseAddr)
		{
			case FLEXSEA_EXECUTE_BASE:

				exec_s_ptr->gyro.x = (int16_t) REBUILD_UINT16(buf, &index);
				exec_s_ptr->gyro.y = (int16_t) REBUILD_UINT16(buf, &index);
				exec_s_ptr->gyro.z = (int16_t) REBUILD_UINT16(buf, &index);
				exec_s_ptr->accel.x = (int16_t) REBUILD_UINT16(buf, &index);
				exec_s_ptr->accel.y = (int16_t) REBUILD_UINT16(buf, &index);
				exec_s_ptr->accel.z = (int16_t) REBUILD_UINT16(buf, &index);

				exec_s_ptr->strain = (int16_t) REBUILD_UINT16(buf, &index);
				exec_s_ptr->analog[0] = (int16_t) REBUILD_UINT16(buf, &index);
				exec_s_ptr->analog[1] = (int16_t) REBUILD_UINT16(buf, &index);
				*(exec_s_ptr->enc_ang) = (int32_t) REBUILD_UINT32(buf, &index);
				exec_s_ptr->current = (int32_t) REBUILD_UINT32(buf, &index);

				exec_s_ptr->volt_batt = buf[index++];
				exec_s_ptr->volt_int = buf[index++];
				exec_s_ptr->temp = buf[index++];
				exec_s_ptr->status1 = buf[index++];
				exec_s_ptr->status2 = buf[index++];
				break;

			case FLEXSEA_MANAGE_BASE:

				mn_s_ptr->gyro.x = (int16_t) REBUILD_UINT16(buf, &index);
				mn_s_ptr->gyro.y = (int16_t) REBUILD_UINT16(buf, &index);
				mn_s_ptr->gyro.z = (int16_t) REBUILD_UINT16(buf, &index);
				mn_s_ptr->accel.x = (int16_t) REBUILD_UINT16(buf, &index);
				mn_s_ptr->accel.y = (int16_t) REBUILD_UINT16(buf, &index);
				mn_s_ptr->accel.z = (int16_t) REBUILD_UINT16(buf, &index);

				mn_s_ptr->sw1 = buf[index++];

				mn_s_ptr->analog[0] = REBUILD_UINT16(buf, &index);
				mn_s_ptr->analog[1] = REBUILD_UINT16(buf, &index);
				mn_s_ptr->analog[2] = REBUILD_UINT16(buf, &index);
				mn_s_ptr->analog[3] = REBUILD_UINT16(buf, &index);
				mn_s_ptr->analog[4] = REBUILD_UINT16(buf, &index);
				mn_s_ptr->analog[5] = REBUILD_UINT16(buf, &index);
				mn_s_ptr->analog[6] = REBUILD_UINT16(buf, &index);
				mn_s_ptr->analog[7] = REBUILD_UINT16(buf, &index);

				//...
				break;

			case FLEXSEA_GOSSIP_BASE:

				go_s_ptr->gyro.x = (int16_t) REBUILD_UINT16(buf, &index);
				go_s_ptr->gyro.y = (int16_t) REBUILD_UINT16(buf, &index);
				go_s_ptr->gyro.z = (int16_t) REBUILD_UINT16(buf, &index);
				go_s_ptr->accel.x = (int16_t) REBUILD_UINT16(buf, &index);
				go_s_ptr->accel.y = (int16_t) REBUILD_UINT16(buf, &index);
				go_s_ptr->accel.z = (int16_t) REBUILD_UINT16(buf, &index);
				go_s_ptr->magneto.x = (int16_t) REBUILD_UINT16(buf, &index);
				go_s_ptr->magneto.y = (int16_t) REBUILD_UINT16(buf, &index);
				go_s_ptr->magneto.z = (int16_t) REBUILD_UINT16(buf, &index);

				go_s_ptr->io[0] = buf[index++];
				go_s_ptr->io[1] = buf[index++];

				go_s_ptr->capsense[0] = REBUILD_UINT16(buf, &index);
				go_s_ptr->capsense[1] = REBUILD_UINT16(buf, &index);
				go_s_ptr->capsense[2] = REBUILD_UINT16(buf, &index);
				go_s_ptr->capsense[3] = REBUILD_UINT16(buf, &index);

				break;

			case FLEXSEA_STRAIN_BASE:

				st_s_ptr->compressedBytes[0] = buf[index++];
				st_s_ptr->compressedBytes[1] = buf[index++];
				st_s_ptr->compressedBytes[2] = buf[index++];
				st_s_ptr->compressedBytes[3] = buf[index++];
				st_s_ptr->compressedBytes[4] = buf[index++];
				st_s_ptr->compressedBytes[5] = buf[index++];
				st_s_ptr->compressedBytes[6] = buf[index++];
				st_s_ptr->compressedBytes[7] = buf[index++];
				st_s_ptr->compressedBytes[8] = buf[index++];
				st_s_ptr->preDecoded = 0;
				break;
		}

	#else

		(void)buf;

	#endif	//((defined BOARD_TYPE_FLEXSEA_MANAGE) || (defined BOARD_TYPE_FLEXSEA_PLAN))
}

//Transmit User R/W:
//==================

void tx_cmd_data_user_r(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
						uint16_t *len)
{
	uint16_t index = 0;

	//Formatting:
	(*cmd) = CMD_USER_DATA;
	(*cmdType) = CMD_READ;

	//Data:
	(void)shBuf;	//(none)

	//Payload length:
	(*len) = index;
}

void ptx_cmd_data_user_r(uint8_t slaveId, uint16_t *numb, uint8_t *commStr)
{
	tx_cmd_data_user_r(TX_N_DEFAULT);
	pack(P_AND_S_DEFAULT, slaveId, NULL, numb, commStr);
}

/*Writes one of the User W fields. Note: only Plan can set a W field; this
command is made to be used by Plan and a slave, not between say Manage &
Execute */
void tx_cmd_data_user_w(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
						uint16_t *len, uint8_t select_w)
{
	uint16_t index = 0;

	//Formatting:
	(*cmd) = CMD_USER_DATA;
	(*cmdType) = CMD_WRITE;

	//Data:

	#ifdef BOARD_TYPE_FLEXSEA_PLAN

		//Plan can set W:
		shBuf[index++] = select_w;
		SPLIT_32((uint32_t)user_data_1.w[select_w], shBuf, &index);

	#else
		(void)select_w;

		//All other boards can only reply
		SPLIT_32((uint32_t)user_data_1.r[0], shBuf, &index);
		SPLIT_32((uint32_t)user_data_1.r[1], shBuf, &index);
		SPLIT_32((uint32_t)user_data_1.r[2], shBuf, &index);
		SPLIT_32((uint32_t)user_data_1.r[3], shBuf, &index);

	#endif	//BOARD_TYPE_FLEXSEA_PLAN

	//Payload length:
	(*len) = index;
}

void copyUserWtoStack(struct user_data_s u)
{
	user_data_1.w[0] = u.w[0];
	user_data_1.w[1] = u.w[1];
	user_data_1.w[2] = u.w[2];
	user_data_1.w[3] = u.w[3];
}

void readUserRfromStack(struct user_data_s *u)
{
	u->r[0] = user_data_1.r[0];
	u->r[1] = user_data_1.r[1];
	u->r[2] = user_data_1.r[2];
	u->r[3] = user_data_1.r[3];
}

void ptx_cmd_data_user_w(uint8_t slaveId, uint16_t *numb, uint8_t *commStr, \
							uint8_t select_w)
{
	tx_cmd_data_user_w(TX_N_DEFAULT, select_w);
	pack(P_AND_S_DEFAULT, slaveId, NULL, numb, commStr);
}

//Receive User R/W:
//==================

void rx_cmd_data_user_w(uint8_t *buf, uint8_t *info)
{
	MultiPacketInfo mInfo;
	fillMultiInfoFromBuf(&mInfo, buf, info);
	mInfo.portOut = info[1];
	rx_multi_cmd_data_user_w( buf + P_DATA1, &mInfo, tmpPayload, &cmdLen );
}

void rx_multi_cmd_data_user_w(uint8_t *msgBuf, MultiPacketInfo *mInfo, uint8_t *responseBuf, uint16_t* responseLen)
{
	uint16_t index = 0;
	uint8_t wSelect = 0;
	uint32_t wVal = 0;
	(void)mInfo;	//Unused for now

	wSelect = msgBuf[index++];
	wVal = (int32_t) REBUILD_UINT32(msgBuf, &index);
	user_data_1.w[wSelect] = wVal;
}

void rx_cmd_data_user_rw(uint8_t *buf, uint8_t *info)
{
	MultiPacketInfo mInfo;
	fillMultiInfoFromBuf(&mInfo, buf, info);
	mInfo.portOut = info[1];
	rx_multi_cmd_data_user_rw( buf + P_DATA1, &mInfo, tmpPayload, &cmdLen );
	packAndSend(P_AND_S_DEFAULT, buf[P_XID], info, SEND_TO_MASTER);
}

void rx_multi_cmd_data_user_rw(uint8_t *msgBuf, MultiPacketInfo *mInfo, uint8_t *responseBuf, uint16_t* responseLen)
{
	(void)mInfo;	//Unused for now

	tx_cmd_data_user_w(responseBuf, &cmdCode, &cmdType, responseLen, 0);
}

void rx_cmd_data_user_rr(uint8_t *buf, uint8_t *info)
{
	MultiPacketInfo mInfo;
	fillMultiInfoFromBuf(&mInfo, buf, info);
	mInfo.portOut = info[1];
	rx_multi_cmd_data_user_rr( buf + P_DATA1, &mInfo, tmpPayload, &cmdLen );
}

void rx_multi_cmd_data_user_rr(uint8_t *msgBuf, MultiPacketInfo *mInfo, uint8_t *responseBuf, uint16_t* responseLen)
{
	uint16_t index = 0;
	(void)mInfo;	//Unused for now

	user_data_1.r[0] = (int32_t)REBUILD_UINT32(msgBuf, &index);
	user_data_1.r[1] = (int32_t)REBUILD_UINT32(msgBuf, &index);
	user_data_1.r[2] = (int32_t)REBUILD_UINT32(msgBuf, &index);
	user_data_1.r[3] = (int32_t)REBUILD_UINT32(msgBuf, &index);
}
#ifdef __cplusplus
}
#endif
