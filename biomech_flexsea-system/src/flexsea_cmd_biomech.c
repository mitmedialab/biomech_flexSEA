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
	[This file] flexsea_cmd_biomech: commands for general transmission of system data to/from GUI
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2018-10-04 | dweisdorf | Initial GPL-3.0 release
	*
****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include "flexsea_cmd_biomech.h"
#include "flexsea_system.h"
#include <flexsea.h>
#include <stdlib.h>
#include "flexsea_device_spec.h"
#include "flexsea_board.h"
#include <string.h>
#include "flexsea_comm_def.h"
#include "user-mn-MIT-DLeg.h"

#define IS_FIELD_HIGH(i, map) ( (map)[(i)/32] & (1U << ((i)%32)) )
#define SET_MAP_HIGH(i, map) ( (map)[(i)/32] |= (1U << ((i)%32)) )
#define SET_MAP_LOW(i, map) ( (map)[(i)/32] &= (~(1U << ((i)%32))) )

/* Initializes part of the array of function pointers which determines which
	function to call upon receiving a message
*/
void init_flexsea_payload_ptr_biomech(void) {

#ifndef BOARD_TYPE_FLEXSEA_PLAN
	flexsea_multipayload_ptr[CMD_BIOMECH][RX_PTYPE_READ] = &rx_cmd_biomech_r;
	flexsea_multipayload_ptr[CMD_BIOMECH][RX_PTYPE_WRITE] = &rx_cmd_biomech_w;
#endif

	flexsea_multipayload_ptr[CMD_BIOMECH][RX_PTYPE_REPLY] = &rx_cmd_biomech_rr;
}

// Flexsea General System Data Passing:

/* Called by master to send a message to the slave, attempting to initiate a read, using a bitmap to specify which fields to read.
 * Current implementation allows for 3 * 32 = 96 readable fields, using 96 / 8 = 12 bytes.
 * Slave will respond with the appropriate data.
*/

void tx_cmd_biomech_r(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
						uint16_t *len, uint8_t subcmd, float desiredJointAngleDeg_f, float desiredJointK_f) {

	uint16_t index = 0;
	(*cmd) = CMD_BIOMECH;
	(*cmdType) = CMD_READ;

	shBuf[index++] = subcmd; //replace this with biomech subflags
	SPLIT_16((int16_t) (desiredJointAngleDeg_f*INT_SCALING), shBuf, &index);
	SPLIT_16((uint16_t) (desiredJointK_f*INT_SCALING), shBuf, &index);
	(*len) = index;
}

/* Called by master to send a message to the slave, attempting to initiate a read and write a data bitmap.
*/

void tx_cmd_biomech_w(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
						uint16_t *len, uint32_t *flags, uint8_t lenFlags, uint8_t subcmd,  float desiredJointAngleDeg_f, float desiredJointK_f)
{

	uint16_t index = 0;
	(*cmd) = CMD_BIOMECH;
	(*cmdType) = CMD_WRITE;

	if(lenFlags > FX_BITMAP_WIDTH_C)
		lenFlags = FX_BITMAP_WIDTH_C;

	shBuf[index++] = lenFlags;

	uint8_t i=0;
	while(i < lenFlags)
		SPLIT_32(flags[i++], shBuf, &index);

	shBuf[index++] = subcmd;
	SPLIT_16((int16_t) (desiredJointAngleDeg_f*INT_SCALING), shBuf, &index);
	SPLIT_16((uint16_t) (desiredJointK_f*INT_SCALING), shBuf, &index);

	(*len) = index;
}

#ifndef BOARD_TYPE_FLEXSEA_PLAN

/* Slave calls this function automatically after receiving a read from master.
	It determines what to do with the information passed to it,
	And it replies indicating the resulting decision
*/
void rx_cmd_biomech_r(uint8_t *msgBuf, MultiPacketInfo *info, uint8_t *responseBuf, uint16_t* responseLen) {

	uint16_t index = 0;
	uint8_t subcmd = msgBuf[index++];
//	uint16_t rawTau = REBUILD_UINT16(msgBuf, &index);
	int16_t raw_desiredJointAngleDeg = (int16_t) REBUILD_UINT16(msgBuf, &index);
	uint16_t raw_desiredJointK = REBUILD_UINT16(msgBuf, &index);

//	act1.tauDes = (*(float*) &rawTau)/INT_SCALING;
	act1.desiredJointAngleDeg_f = (*(float*) &raw_desiredJointAngleDeg)/INT_SCALING;
	act1.desiredJointK_f = (*(float*) &raw_desiredJointK)/INT_SCALING;

	//populate return datafields (ints)
	act1.desiredJointAngleDeg = raw_desiredJointAngleDeg;
	act1.desiredJointK = raw_desiredJointK;

	//set motors off by default
	act1.motorOnFlag = 0;
	act1.commandTimer = 0;

	switch (subcmd) {
		case 0:
			break;
		case 1:
			act1.motorOnFlag = 1;
			break;
		//more cases add here
		default:
			break;
	}

	tx_cmd_biomech_rr(responseBuf, responseLen, subcmd);

	(void)info;
}

/* Slave calls this function automatically after receiving a write from master.
	It determines what to do with the information passed to it.
*/
void rx_cmd_biomech_w(uint8_t *msgBuf, MultiPacketInfo *info, uint8_t *responseBuf, uint16_t* responseLen)
{
	uint16_t index = 0;

	uint8_t i=0, lenFlags = msgBuf[index++];

	while(i < lenFlags && i < FX_BITMAP_WIDTH_C)
		fx_active_bitmap[i++] = REBUILD_UINT32(msgBuf, &index);

	while(i < FX_BITMAP_WIDTH_C)
		fx_active_bitmap[i++] = 0;

	// set any non existent fields low
	i = fx_this_device_spec->numFields;
	while(i < 32*FX_BITMAP_WIDTH_C)
	{
		SET_MAP_LOW(i, fx_active_bitmap);
		++i;
	}

	// check that for all fields we have valid data pointers
	i = 0;
	while(i < fx_this_device_spec->numFields)
	{
		if(!_dev_data_pointers[i])
			SET_MAP_LOW(i, fx_active_bitmap);
		++i;
	}

	setActiveFieldsByMap(fx_active_bitmap);

 	index = FX_BITMAP_WIDTH_C > lenFlags ? index + lenFlags : index + FX_BITMAP_WIDTH_C; //move index past bitmap flags

	uint8_t subcmd = msgBuf[index++];
//	uint16_t rawTau = REBUILD_UINT16(msgBuf, &index);
	int16_t raw_desiredJointAngleDeg = (int16_t) REBUILD_UINT16(msgBuf, &index);
	uint16_t raw_desiredJointK = REBUILD_UINT16(msgBuf, &index);

//	act1.tauDes = (*(float*) &rawTau)/INT_SCALING;
	act1.desiredJointAngleDeg_f = (*(float*) &raw_desiredJointAngleDeg)/INT_SCALING;
	act1.desiredJointK_f = (*(float*) &raw_desiredJointK)/INT_SCALING;

	//populate return datafields (ints)
	act1.desiredJointAngleDeg = raw_desiredJointAngleDeg;
	act1.desiredJointK = raw_desiredJointK;

	act1.motorOnFlag = 1; //turn motor flag on or off. This is flipped to 0 in safetyLimit() if comms drop.
	act1.commandTimer = 0;

	switch (subcmd) {
		case 0:
			break;

		default:
			break;
	}

	tx_cmd_biomech_rr(responseBuf, responseLen, subcmd);
}

/* Called by slave to send a read response to the master. Master will not respond */
void tx_cmd_biomech_rr(uint8_t *responseBuf, uint16_t* responseLen, uint8_t subcmd) {

	uint16_t l = 0;

	// send the actual data
	int i, j;
	uint8_t* dest = responseBuf + l;
	for(i=0;i< *read_num_fields_active; ++i)
	{
		// unclear which of these copying methods is fastest, try both and measure

		// memcpy(dest, read_device_active_field_pointers[i], read_device_active_field_lengths[i]);
		for(j=0; j<read_device_active_field_lengths[i]; ++j)
			dest[j] = read_device_active_field_pointers[i][j];

		dest = dest + read_device_active_field_lengths[i];
	}

	*responseLen = dest - responseBuf;

}

#endif //BOARD_TYPE_FLEXSEA_PLAN


/* SHOULD NEVER REACH if using plan 3.0
*/
void rx_cmd_biomech_rr(uint8_t *msgBuf, MultiPacketInfo *info, uint8_t *responseBuf, uint16_t* responseLen) {

	(void) msgBuf; (void) info; (void) responseBuf; (void) responseLen;
}

#ifdef __cplusplus
}
#endif

