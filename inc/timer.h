/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'flexsea-manage' Mid-level computing, and networking
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
extern volatile uint8_t tb_1ms_flag;
extern volatile uint8_t tb_10ms_flag;
extern volatile uint8_t tb_100ms_flag;
extern volatile uint8_t tb_1000ms_flag;

typedef uint32_t timer_ticks_t;

extern volatile timer_ticks_t timer_delayCount;

//****************************************************************************
// Public Function Prototype(s):
//****************************************************************************

extern void init_systick_timer(void);
extern void timer_sleep (timer_ticks_t ticks);

void init_timer_6(void);
uint16_t readTimer6(void);
//void init_timer_7(void);
void timebases(void);
void delayUsBlocking(uint16_t us);
void test_delayUsBlocking_blocking(void);

//****************************************************************************
// Definition(s):
//****************************************************************************

#define TIMER_FREQUENCY_HZ (10000u)

#endif // INC_TIMER_H

