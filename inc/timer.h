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
	[This file] fm_timer: SysTick & Timer functions
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-23 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

#ifndef INC_TIMER_H
#define INC_TIMER_H

//****************************************************************************
// Include(s)
//****************************************************************************

#include "cmsis_device.h"

//****************************************************************************
// Shared Variable(s)
//****************************************************************************

extern volatile uint8_t tb_100us_flag;
extern volatile uint8_t tb_100us_timeshare;
typedef uint32_t timer_ticks_t;

//****************************************************************************
// Public Function Prototype(s):
//****************************************************************************

extern void init_systick_timer(void);
void init_timer_6(void);
uint16_t readTimer6(void);
#ifndef BOARD_SUBTYPE_RIGID
void init_timer_7(void);
#endif
void timebases(void);
void delayUsBlocking(uint16_t us);
void test_delayUsBlocking_blocking(void);

//****************************************************************************
// Definition(s):
//****************************************************************************

#endif // INC_TIMER_H

