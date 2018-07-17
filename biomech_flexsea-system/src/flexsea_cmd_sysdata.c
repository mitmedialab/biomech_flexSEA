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
	[This file] flexsea_cmd_sysdata: commands for general transmission of system data to/from GUI
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2018-10-04 | dweisdorf | Initial GPL-3.0 release
	*
****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include "flexsea_cmd_sysdata.h"
#include "flexsea_system.h"
#include <flexsea.h>
#include <stdlib.h>
#include "flexsea_device_spec.h"
#include "flexsea_board.h"
#include <string.h>
#include "flexsea_comm_def.h"

#define IS_FIELD_HIGH(i, map) ( (map)[(i)/32] & (1U << ((i)%32)) )
#define SET_MAP_HIGH(i, map) ( (map)[(i)/32] |= (1U << ((i)%32)) )
#define SET_MAP_LOW(i, map) ( (map)[(i)/32] &= (~(1U << ((i)%32))) )

/* Initializes part of the array of function pointers which determines which
	function to call upon receiving a message
*/
void init_flexsea_payload_ptr_sysdata(void) {

#ifndef BOARD_TYPE_FLEXSEA_PLAN
	flexsea_multipayload_ptr[CMD_SYSDATA][RX_PTYPE_READ] = &rx_cmd_sysdata_r;
	flexsea_multipayload_ptr[CMD_SYSDATA][RX_PTYPE_WRITE] = &rx_cmd_sysdata_w;
#endif

	flexsea_multipayload_ptr[CMD_SYSDATA][RX_PTYPE_REPLY] = &rx_cmd_sysdata_rr;
	// this should perhaps go in a different initialization function
	initializeDeviceSpecs();
}

// Flexsea General System Data Passing:

/* Called by master to send a message to the slave, attempting to initiate a read, using a bitmap to specify which fields to read.
 * Current implementation allows for 3 * 32 = 96 readable fields, using 96 / 8 = 12 bytes.
 * Slave will respond with the appropriate data.
*/

void tx_cmd_sysdata_r(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
						uint16_t *len, uint32_t *flags, uint8_t lenFlags) {

	uint16_t index = 0;
	(*cmd) = CMD_SYSDATA;
	(*cmdType) = CMD_READ;

	shBuf[index++] = SYSDATA_REG_READ_FLAG;

//	if(lenFlags > MAX_BYTES_OF_FLAGS)
//		lenFlags = MAX_BYTES_OF_FLAGS;
//
//	shBuf[index++] = lenFlags;
//
//	uint8_t i=0;
//	while(i < lenFlags)
//		SPLIT_32(flags[i++], shBuf, &index);

	(*len) = index;
}

#ifndef BOARD_TYPE_FLEXSEA_PLAN

void tx_cmd_sysdata_rr(uint8_t *responseBuf, uint16_t* responseLen, uint8_t sendMetaData);

/* Slave calls this function automatically after receiving a read from master.
	It determines what to do with the information passed to it,
	And it replies indicating the resulting decision
*/
void rx_cmd_sysdata_r(uint8_t *msgBuf, MultiPacketInfo *info, uint8_t *responseBuf, uint16_t* responseLen) {

	uint8_t isWhoAmI = msgBuf[0] == SYSDATA_WHO_AM_I_FLAG;
	if(isWhoAmI)
	{
		// set flags low and respond with metadata
		memset(fx_active_bitmap, 0, sizeof(uint32_t)*FX_BITMAP_WIDTH_C);
	}

	tx_cmd_sysdata_rr(responseBuf, responseLen, isWhoAmI);
	(void)info;
}

/* Called by slave to send a read response to the master. Master will not respond */
void tx_cmd_sysdata_rr(uint8_t *responseBuf, uint16_t* responseLen, uint8_t sendMetaData) {

	uint16_t l = 0;
	responseBuf[l++] = sendMetaData;

	if(sendMetaData)
	{
		// {devType, devId, lenFlags, flag0, ... , flagn-1)
		responseBuf[l++] = getDeviceType();
		responseBuf[l++] = getDeviceId();
		// send info about which flags are high
		responseBuf[l++] = FX_BITMAP_WIDTH_C;
		uint8_t i;
		for(i=0;i<FX_BITMAP_WIDTH_C;i++)
			SPLIT_32(fx_active_bitmap[i], responseBuf, &l);

		responseBuf[l++] = getBoardID();
		*responseLen = l;
	}
	else
	{
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

}

/* Slave calls this function automatically after receiving a write from master.
	It determines what to do with the information passed to it,
	And it does not reply.
*/
void rx_cmd_sysdata_w(uint8_t *msgBuf, MultiPacketInfo *info, uint8_t *responseBuf, uint16_t* responseLen)
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

	tx_cmd_sysdata_rr(responseBuf, responseLen, 1);
}

#endif //BOARD_TYPE_FLEXSEA_PLAN

/* Called by master to send a message to the slave, attempting to initiate a
	Slave will not respond.
	TODO: I don't think this is needed even??
	TODO: rename this to 'write'
*/

void tx_cmd_sysdata_w(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
						uint16_t *len, uint32_t *flags, uint8_t lenFlags)
{

	uint16_t index = 0;
	(*cmd) = CMD_SYSDATA;
	(*cmdType) = CMD_WRITE;

	if(lenFlags > FX_BITMAP_WIDTH_C)
		lenFlags = FX_BITMAP_WIDTH_C;

	shBuf[index++] = lenFlags;

	uint8_t i=0;
	while(i < lenFlags)
		SPLIT_32(flags[i++], shBuf, &index);

	(*len) = index;
}



/* Master calls this function automatically after receiving a response from slave
*/
#ifdef BOARD_TYPE_FLEXSEA_PLAN
void rx_cmd_sysdata_rr(uint8_t *msgBuf, MultiPacketInfo *info, uint8_t *responseBuf, uint16_t* responseLen) {

	uint16_t index = 0;
	uint8_t lenFlags;
	uint32_t flags[FX_BITMAP_WIDTH_C];

	lenFlags = msgBuf[index++];

	//read in our fields
	int i, j, k, fieldOffset;
	for(i=0;i<lenFlags;i++)
		flags[i]=REBUILD_UINT32(msgBuf, &index);

	// first two fields are always device type and id
	// if these fields are not sent then we can't do much with the response
	if(!IS_FIELD_HIGH(0, flags) || !IS_FIELD_HIGH(1, flags)) return;
	// get the device type
	uint8_t devType = msgBuf[index++];

	// messages are little endian
	uint16_t devId = msgBuf[index] + (msgBuf[index+1] << 8);
	index+=2;

	// match this message to a connected device
	for(i = 0; i < fx_spec_numConnectedDevices; i++)
	{
		if(	devType == (deviceData[i][0]) &&
			devId 	== *(uint16_t*)(deviceData[i]+1))
					break;
	}

	//in the event we found no match and we have no more space to allocate we are screwed
	if(i >= MAX_CONNECTED_DEVICES)
		return;

	// in the event we found no connected device we need to add a new connected device
	if(i == fx_spec_numConnectedDevices)
		addConnectedDevice(devType, devId);

	// read into the appropriate device
	FlexseaDeviceSpec ds = connectedDeviceSpecs[i];
	uint8_t *dataPtr = deviceData[i];
	if(dataPtr)
	{
		// first two fields are devType and devId, we skip for reading in
		fieldOffset = ds.fieldTypes[0] + ds.fieldTypes[1];
		for(j = 2; j < ds.numFields; j++)
		{
			uint8_t fw = FORMAT_SIZE_MAP[ds.fieldTypes[j]];
			if(IS_FIELD_HIGH(j, flags))
			{
				for(k = 0; k < fw; k++)
					deviceData[i][fieldOffset + k] = msgBuf[index++];
			}

			fieldOffset += fw;
		}
	}
	else
	{
		; // log error?
	}
}
#else
void rx_cmd_sysdata_rr(uint8_t *msgBuf, MultiPacketInfo *info, uint8_t *responseBuf, uint16_t* responseLen)
	{ (void) msgBuf; (void) info; (void) responseBuf; (void) responseLen; }
#endif

#ifdef __cplusplus
}
#endif

