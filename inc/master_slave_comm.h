/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'flexsea-manage' Mid-level computing, and networking
	Copyright (C) 2017 Dephy, Inc. <http://dephy.com/>
*****************************************************************************
	[Lead developper] Jean-Francois (JF) Duval, jfduval at dephy dot com.
	[Origin] Based on Jean-Francois Duval's work at the MIT Media Lab
	Biomechatronics research group <http://biomech.media.mit.edu/>
	[Contributors]
*****************************************************************************
	[This file] fm_slave_comm: Slave R/W
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-23 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

#ifndef INC_MAST_SLAVE_COMM_H
#define INC_MAST_SLAVE_COMM_H

//****************************************************************************
// Include(s)
//****************************************************************************

#include "main.h"
#include "flexsea_comm.h"

//****************************************************************************
// Public Function Prototype(s):
//****************************************************************************

void initMasterSlaveComm(void);
//void parseMasterCommands(uint8_t *new_cmd);
//void parseSlaveCommands(uint8_t *new_cmd);
void slaveTransmit(uint8_t port);
void autoStream(void);
void sendMasterDelayedResponse(void);

//****************************************************************************
// Definition(s):
//****************************************************************************

//****************************************************************************
// Shared Variable(s):
//****************************************************************************

#endif // INC_MAST_SLAVE_COMM_H
