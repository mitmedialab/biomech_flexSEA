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
	[This file] FlexSEA-Mn EEPROM API
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
extern uint16_t angleMap[];

//****************************************************************************
// Public Function Prototype(s):
//****************************************************************************

//Cycle Tester:
uint16_t increaseCycleCountEEPROM(void);
uint16_t resetCycleCountEEPROM(void);
uint16_t readCycleCountEEPROM(uint16_t *cc);

//Angle Maps:
void initAngleMapRAM(void);
uint8_t writeAngleMapEEPROM(void);
uint8_t readAngleMapEEPROM(void);
void testAngleMapEEPROMblocking(void);

//UVLO:
uint8_t writeUvloEEPROM(uint16_t newValue);
uint8_t readUvloEEPROM(void);
uint16_t getNvUVLO(void);

//****************************************************************************
// Definition(s):
//****************************************************************************

#define EE_ANGLE_CNT					128
#define EE_CYCLE_TESTER_CNT				3
#define EE_NV_UVLO						2
//Total memory size (keep < 256):
#define EE_CNT							(EE_ANGLE_CNT + EE_CYCLE_TESTER_CNT + EE_NV_UVLO)

//Memory areas:
#define ANGLE_MAP_ADDR					0
#define CYCLE_COUNT_ADDR				(EE_ANGLE_CNT + 1)
#define NV_UVLO_ADDR					(CYCLE_COUNT_ADDR + EE_CYCLE_TESTER_CNT + 2)

#define NV_UVLO_LOCK					12345

#endif // INC_CYCLE_H

