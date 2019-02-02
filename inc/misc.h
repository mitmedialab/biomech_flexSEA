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
	[This file] fm_misc: when it doesn't belong in any another file, it
	ends up here...
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-23 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

#ifndef INC_MISC_H
#define INC_MISC_H

//****************************************************************************
// Include(s)
//****************************************************************************

#include "main.h"

//****************************************************************************
// Shared variable(s)
//****************************************************************************

extern IWDG_HandleTypeDef hiwdg;

//****************************************************************************
// Public Function Prototype(s):
//****************************************************************************

void init_peripherals(void);
void init_iwdg(void);
void independentWatchdog(void);
void saveCauseOFLastReset(void);
void test_iwdg_blocking(void);
void test_code_blocking(void);
void test_code_non_blocking(void);
void combineStatusFlags(void);

//****************************************************************************
// Definition(s):
//****************************************************************************

#define MAX_I			25
#define MAX_J			25

//Watchdog:
#define IWDG_RELOAD		50	//50 = 25ms when prescaler = 4

//Partial flags - ToDo replace with stack version:
#define STATUS_MOT_CURRENT_WARN		32
#define STATUS_MOT_CURRENT_LIM		64

//battery voltage limits
#define UVLO_BIOMECH 19200
#define UVLO_BIOMECH_NOTIFY_OFFSET 500
#define UVLO_NOTIFY UVLO_BIOMECH + UVLO_BIOMECH_NOTIFY_OFFSET
#define UVHI_BIOMECH 52800 //25800
#define UV_USB_BIOMECH 6000

#endif // INC_MISC_H

