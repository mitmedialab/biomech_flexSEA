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
	* 2018-08-02 | tshu | Initial GPL-3.0 release
	*
****************************************************************************/

#ifndef FLEXSEA_SYSTEM_INC_FLEXSEA_CMD_BIOMECH_H_
#define FLEXSEA_SYSTEM_INC_FLEXSEA_CMD_BIOMECH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "user-mn-MIT-DLeg.h"
#include "state_variables.h"

/* Functions for message passing declared below.
 * CMD_BIOMECH == 105
 * */

#define INT_SCALING 100

/* Initializes part of the array of function pointers which determines which
	function to call upon receiving a message
*/
void init_flexsea_payload_ptr_biomech(void);

// Flexsea General System Data Passing:

/* Called by master to send a message to the slave, attempting to initiate a read, using a bitmap to specify which fields to read.
 * Current implementation allows for 3 * 32 = 96 readable fields, using 96 / 8 = 12 bytes.
 * Slave will respond with the appropriate data.
*/

void tx_cmd_biomech_r(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
						uint16_t *len, uint8_t subcmd,  float desiredJointAngleDeg_f, float desiredJointK_f);

/* Called by master to send a message to the slave, attempting to initiate a write, using a bitmap to specify which fields to read.
 * Current implementation allows for 3 * 32 = 96 readable fields, using 96 / 8 = 12 bytes.
 * Slave will respond with the appropriate data.
*/

void tx_cmd_biomech_w(uint8_t *shBuf, uint8_t *cmd, uint8_t *cmdType, \
						uint16_t *len, uint32_t *flags, uint8_t lenFlags, uint8_t subcmd,  float desiredJointAngleDeg_f, float desiredJointK_f);

struct _MultiPacketInfo_s;
typedef struct _MultiPacketInfo_s MultiPacketInfo;

/* Master calls this function automatically after receiving a response from slave. SHOULD NEVER REACH HERE on Plan 3.0.
*/
void rx_cmd_biomech_rr(uint8_t *msgBuf, MultiPacketInfo *info, uint8_t *responseBuf, uint16_t* responseLen);

#ifndef BOARD_TYPE_FLEXSEA_PLAN
/* Slave calls this function automatically after receiving a read from master.
	It determines what to do with the information passed to it,
	And it replies indicating the resulting decision
*/
void rx_cmd_biomech_r(uint8_t *msgBuf, MultiPacketInfo *info, uint8_t *responseBuf, uint16_t* responseLen);

/* Slave calls this function automatically after receiving a write from master.
	It determines what to do with the information passed to it.
*/
void rx_cmd_biomech_w(uint8_t *msgBuf, MultiPacketInfo *info, uint8_t *responseBuf, uint16_t* responseLen);

void tx_cmd_biomech_rr(uint8_t *responseBuf, uint16_t* responseLen, uint8_t subcmd);

#endif // BOARD_TYPE_FLEXSEA_PLAN

#ifdef __cplusplus
}
#endif

#endif /* FLEXSEA_SYSTEM_INC_FLEXSEA_CMD_BIOMECH_H_ */
