/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'flexsea-comm' Communication stack
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
	[This file] flexsea: Master file for the FlexSEA stack.
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-09 | jfduval | Initial GPL-3.0 release
	* 2016-10-14 | jfduval | New macros & functions to simplify RX/TX fcts
****************************************************************************/

// FlexSEA: Flexible & Scalable Electronics Architecture

// v0.0 Limitations and known bugs:
// ================================
// - The board config is pretty much fixed, at compile time.
// - Only 1 command per transmission
// - Fixed payload length: ? bytes (allows you to send 1 command with up to
//   ? arguments (uint8_t) (update this)
// - Fixed comm_str length: 24 bytes (min. to accomodate a payload where all the
//   data bytes need escaping)
// - In comm_str #OfBytes isn't escaped. Ok as long as the count is less than
//   the decimal value of the flags ('a', 'z', 'e') so max 97 bytes.
// - Data transfer could be faster with shorter ACK sequence, more than 1 command
//   per packet and no repackaging on the Manage board (straight pass-through)
//   To be optimized later.

#ifdef __cplusplus
extern "C" {
#endif

//****************************************************************************
// Include(s)
//****************************************************************************

#include <flexsea.h>

//****************************************************************************
// Variable(s)
//****************************************************************************

uint8_t test_payload[PAYLOAD_BUF_LEN];

//Function pointer array:
void (*flexsea_payload_ptr[MAX_CMD_CODE][RX_PTYPE_MAX_INDEX+1]) \
		(uint8_t *buf, uint8_t *info);

void (*flexsea_multipayload_ptr[MAX_CMD_CODE][RX_PTYPE_MAX_INDEX+1]) \
		(uint8_t *msgBuf, MultiPacketInfo *info, uint8_t *responseBuf, uint16_t* responseLen);
//****************************************************************************
// Private Function Prototype(s)
//****************************************************************************

//****************************************************************************
// Public Function(s)
//****************************************************************************

//When something goes wrong in the code it will land here:
unsigned int flexsea_error(unsigned int err_code)
{
	//ToDo something useful
	return err_code;
}

void fillMultiInfoFromBuf(MultiPacketInfo *mInfo, uint8_t* buf, uint8_t* info)
{
	mInfo->portIn = info[0];
//	mInfo->portOut = info[1];
	mInfo->xid = buf[P_XID];
	mInfo->rid = buf[P_RID];
}

//Splits 1 uint16 in 2 bytes, stores them in buf[index] and increments index
inline void SPLIT_16(uint16_t var, uint8_t *buf, uint16_t *index)
{
	buf[*index] = (uint8_t) ((var >> 8) & 0xFF);
	buf[(*index)+1] = (uint8_t) (var & 0xFF);
	(*index) += 2;
}

//Inverse of SPLIT_16()
uint16_t REBUILD_UINT16(uint8_t *buf, uint16_t *index)
{
	uint16_t tmp = 0;

	tmp = (((uint16_t)buf[(*index)] << 8) + ((uint16_t)buf[(*index)+1] ));
	(*index) += 2;
	return tmp;
}

//Splits 1 uint32 in 4 bytes, stores them in buf[index] and increments index
inline void SPLIT_32(uint32_t var, uint8_t *buf, uint16_t *index)
{
	buf[(*index)] = (uint8_t) ((var >> 24) & 0xFF);
	buf[(*index)+1] = (uint8_t) ((var >> 16) & 0xFF);
	buf[(*index)+2] = (uint8_t) ((var >> 8) & 0xFF);
	buf[(*index)+3] = (uint8_t) (var & 0xFF);
	(*index) += 4;
}

//Inverse of SPLIT_32()
uint32_t REBUILD_UINT32(uint8_t *buf, uint16_t *index)
{
	uint32_t tmp = 0;

	tmp = (((uint32_t)buf[(*index)] << 24) + ((uint32_t)buf[(*index)+1] << 16) \
			+ ((uint32_t)buf[(*index)+2] << 8) + ((uint32_t)buf[(*index)+3]));
	(*index) += 4;
	return tmp;
}

//****************************************************************************
// Private Function(s):
//****************************************************************************

#ifdef __cplusplus
}
#endif
