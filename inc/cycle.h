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
	[This file] Cycle Tester EEPROM API
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2017-06-13 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

#ifndef INC_CYCLE_H
#define INC_CYCLE_H

//****************************************************************************
// Include(s)
//****************************************************************************

#include "main.h"

//****************************************************************************
// Shared variable(s)
//****************************************************************************

extern uint16_t cycles;

//****************************************************************************
// Public Function Prototype(s):
//****************************************************************************

uint16_t increaseCycleCountEEPROM(void);
uint16_t resetCycleCountEEPROM(void);
uint16_t readCycleCountEEPROM(uint16_t *cc);

//****************************************************************************
// Definition(s):
//****************************************************************************

//User:
#define CYCLE_COUNT_ADDR				0


#endif // INC_CYCLE_H

