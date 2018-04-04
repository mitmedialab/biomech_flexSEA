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
	[This file] main_fsm: Contains all the case() code for the main FSM
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-23 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

#ifndef INC_MAIN_FSM_H
#define INC_MAIN_FSM_H

//****************************************************************************
// Include(s)
//****************************************************************************

#include "main.h"

//****************************************************************************
// Shared variable(s)
//****************************************************************************

extern uint8_t newCmdLed;
extern struct actuation_parameters act_para;
extern int8_t isEnabledUpdateSensors;
//****************************************************************************
// Public Function Prototype(s):
//****************************************************************************
void updateSensorValues(struct actuation_parameters *actx);
void mainFSM0(void);
void mainFSM1(void);
void mainFSM2(void);
void mainFSM3(void);
void mainFSM4(void);
void mainFSM5(void);
void mainFSM6(void);
void mainFSM7(void);
void mainFSM8(void);
void mainFSM9(void);

void mainFSM10kHz(void);
void mainFSMasynchronous(void);

//****************************************************************************
// Definition(s):
//****************************************************************************

//****************************************************************************
// Structure(s)
//****************************************************************************


#endif	//INC_MAIN_FSM_H

