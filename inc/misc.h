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

//****************************************************************************
// Definition(s):
//****************************************************************************

#define MAX_I			25
#define MAX_J			25

//Watchdog:
#define IWDG_RELOAD		50	//50 = 25ms when prescaler = 4

#endif // INC_MISC_H

