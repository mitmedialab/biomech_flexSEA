/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'user/ActPack' Dephy's Actuator Package (ActPack)
	Copyright (C) 2017 Dephy, Inc. <http://dephy.com/>
*****************************************************************************
	[Lead developper] Jean-Francois Duval, jfduval at dephy dot com.
	[Origin] Based on Jean-Francois Duval's work at the MIT Media Lab
	Biomechatronics research group <http://biomech.media.mit.edu/>
	[Contributors]
*****************************************************************************
	[This file] user-mn-ActPack: User code running on Mn
****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2017-09-27 | jfduval | Initial release
	*
****************************************************************************/

#ifdef INCLUDE_UPROJ_ACTPACK
#ifdef BOARD_TYPE_FLEXSEA_MANAGE

#ifndef INC_TEDANKLE_MN_H
#define INC_TEDANKLE_MN_H

//****************************************************************************
// Include(s)
//****************************************************************************

#include "main.h"

//****************************************************************************
// Public Function Prototype(s):
//****************************************************************************

void init_TedAnkle(void);
void TedAnkle_fsm_1(void);
void TedAnkle_fsm_2(void);
void init_current_controller(void);
//added functions (Matt C)
void findPoles(void);
void setAngleLimits(void);
//end added (MC)

void setMotorVoltage(int32_t v);
void setMotorCurrent(int32_t i);
void setControlMode(uint8_t m);
void setControlGains(int16_t g0, int16_t g1, int16_t g2, int16_t g3);
void setMotorPosition(int32_t i);

//****************************************************************************
// Accessor(s)
//****************************************************************************


//****************************************************************************
// Definition(s):
//****************************************************************************

#define AP_FSM2_POWER_ON_DELAY		1000
#define CTRL_I_KP					100
#define CTRL_I_KI					1
#define CTRL_P_KP					200
#define CTRL_P_KD					10
#define CTRL_P_KI					8

//****************************************************************************
// Structure(s)
//****************************************************************************

//****************************************************************************
// Shared variable(s)
//****************************************************************************

extern struct ctrl_s ctrl;
extern struct rigid_s dpRigid;
extern int32_t dp_ank_ang_zero;
extern int32_t dp_mot_ang_zero;
extern int32_t mot_ang_offset;
extern int32_t ank_ang_offset;

#endif	//INC_TEDANKLE_MN_H

#endif //BOARD_TYPE_FLEXSEA_EXECUTE
#endif //INCLUDE_UPROJ_ACTPACK
