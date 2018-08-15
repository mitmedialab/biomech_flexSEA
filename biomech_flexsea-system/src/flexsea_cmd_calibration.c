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
	[This file] flexsea_cmd_calibration: commands specific to the calibration tools
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2017-02-07 | dweisdorf | Initial GPL-3.0 release
	*
****************************************************************************/

#include "flexsea_cmd_calibration.h"
#include <flexsea.h>
#include "flexsea_system.h"
#include <flexsea_board.h>
#include "user-mn-MIT-DLeg.h"
#include "state_variables.h"

#ifdef __cplusplus
extern "C" {
#endif

#if((defined BOARD_TYPE_FLEXSEA_EXECUTE) || (defined BOARD_TYPE_FLEXSEA_MANAGE))
	#include "calibration_tools.h"
#else
	uint8_t calibrationProgress;
	#define CALIB_DONE 3
#endif

#ifdef BOARD_TYPE_FLEXSEA_EXECUTE
	#include "control.h"
#endif //BOARD_TYPE_FLEXSEA_EXECUTE

#ifndef NULL
#define NULL   ((void *) 0)
#endif

uint8_t handleCalibrationMessage(uint8_t *buf);

void init_flexsea_payload_ptr_calibration(void)
{
	//Control mode:
	flexsea_payload_ptr[CMD_CALIBRATION_MODE][RX_PTYPE_READ] = &rx_cmd_calibration_mode_rw;
	flexsea_payload_ptr[CMD_CALIBRATION_MODE][RX_PTYPE_WRITE] = &rx_cmd_calibration_mode_w;
	flexsea_payload_ptr[CMD_CALIBRATION_MODE][RX_PTYPE_REPLY] = &rx_cmd_calibration_mode_rr;

	flexsea_multipayload_ptr[CMD_CALIBRATION_MODE][RX_PTYPE_READ] = &rx_multi_cmd_calibration_mode_rw;
	flexsea_multipayload_ptr[CMD_CALIBRATION_MODE][RX_PTYPE_WRITE] = &rx_multi_cmd_calibration_mode_w;
	flexsea_multipayload_ptr[CMD_CALIBRATION_MODE][RX_PTYPE_REPLY] = &rx_multi_cmd_calibration_mode_rr;
}

void tx_cmd_calibration_mode_r(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
						uint16_t *len, uint8_t calibrationMode)
{
	uint8_t index = 0;
	(*cmd) = CMD_CALIBRATION_MODE;
	(*cmdType) = CMD_READ;

	shBuf[index++] = calibrationMode;

	(*len) = index;
}

void tx_cmd_calibration_mode_rw(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
						uint16_t *len, uint8_t calibrationMode)
{
	//Variable(s) & command:
	uint16_t index = 0;
	(*cmd) = CMD_CALIBRATION_MODE;
	(*cmdType) = CMD_WRITE;

	//Data:
	shBuf[index++] = calibrationMode;
	shBuf[index++] = getDeviceId();

	//Payload length:
	(*len) = index;
}

//Pack tx_cmd_calibration_mode_rw()
void ptx_cmd_calibration_mode_rw(uint8_t slaveId, uint16_t *numb, uint8_t *commStr, uint8_t calibrationMode)
{
	tx_cmd_calibration_mode_rw(TX_N_DEFAULT, calibrationMode);
	pack(P_AND_S_DEFAULT, slaveId, NULL, numb, commStr);
}

void rx_cmd_calibration_mode_rr(uint8_t *buf, uint8_t *info)
{
	MultiPacketInfo mInfo;
	fillMultiInfoFromBuf(&mInfo, buf, info);
	mInfo.portOut = info[1];
	rx_multi_cmd_calibration_mode_rr( buf + P_DATA1, &mInfo, tmpPayload, &cmdLen );
}

void rx_multi_cmd_calibration_mode_rr(uint8_t *msgBuf, MultiPacketInfo *mInfo, uint8_t *responseBuf, uint16_t* responseLen)
{
	(void)msgBuf;
	(void)mInfo;
	(void)responseBuf;
	(void)responseLen;
}
void rx_cmd_calibration_mode_rw(uint8_t *buf, uint8_t *info)
{
	MultiPacketInfo mInfo;
	fillMultiInfoFromBuf(&mInfo, buf, info);
	mInfo.portOut = info[1];
	rx_multi_cmd_calibration_mode_rw( buf + P_DATA1, &mInfo, tmpPayload, &cmdLen );
	packAndSend(P_AND_S_DEFAULT, buf[P_XID], info, SEND_TO_MASTER);
}

void rx_multi_cmd_calibration_mode_rw(uint8_t *msgBuf, MultiPacketInfo *mInfo, uint8_t *responseBuf, uint16_t* responseLen)
{

	if (!(calibrationProgress == CALIB_DONE)) {
		handleCalibrationMessage(msgBuf);
		tx_cmd_calibration_mode_rw(responseBuf, &cmdCode, &cmdType, responseLen, calibrationProgress);
	} else {
		tx_cmd_calibration_mode_rw(responseBuf, &cmdCode, &cmdType, responseLen, calibrationProgress);
	}

}

void rx_cmd_calibration_mode_w(uint8_t *buf, uint8_t *info)
{
	MultiPacketInfo mInfo;
	fillMultiInfoFromBuf(&mInfo, buf, info);
	mInfo.portOut = info[1];
	rx_multi_cmd_calibration_mode_w( buf + P_DATA1, &mInfo, tmpPayload, &cmdLen );
}

void rx_multi_cmd_calibration_mode_w(uint8_t *msgBuf, MultiPacketInfo *mInfo, uint8_t *responseBuf, uint16_t* responseLen)
{
	handleCalibrationMessage(msgBuf);

	#ifndef BOARD_TYPE_FLEXSEA_EXECUTE
		(void)msgBuf;
	#endif //BOARD_TYPE_FLEXSEA_EXECUTE

	(void)mInfo;
}


uint8_t handleCalibrationMessage(uint8_t *buf)
{
	//Be really careful with multipacket. buf points to first index of user data with no overhead!
	uint16_t index = 0; //THIS USED TO BE P_DATA1 BUT THAT IS WRONG FOR MN -Tony
	uint8_t procedure = buf[index];

	uint8_t calibrationFlagToRunOrIsRunning = 0;

	#if((defined BOARD_TYPE_FLEXSEA_EXECUTE) || (defined BOARD_TYPE_FLEXSEA_MANAGE))

		if(!isRunningCalibrationProcedure() && isLegalCalibrationProcedure(procedure))
		{
			calibrationFlags |= procedure;
			calibrationNew = 1;
			#if(defined BOARD_TYPE_FLEXSEA_EXECUTE)
			control_strategy(buf[P_DATA1], 0);
			#endif
		}
		calibrationFlagToRunOrIsRunning = calibrationFlags;

	#else
		(void)procedure;	//Unused
	#endif

	return calibrationFlagToRunOrIsRunning;
}


#ifdef __cplusplus
}
#endif
