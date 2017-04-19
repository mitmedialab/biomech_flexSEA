/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'flexsea-execute' Advanced Motion Controller
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
	[Lead developper] Jean-Francois Duval, jfduval at dephy dot com.
	[Origin] Based on Jean-Francois Duval's work at the MIT Media Lab 
	Biomechatronics research group <http://biomech.media.mit.edu/>
	[Contributors]
*****************************************************************************
	[This file] RGB LED: Onboard LED Driver that supports fading
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-10-05 | jfduval | Copied from UI to make it more portable
	*
****************************************************************************/

#ifndef INC_RGBLED_H
#define INC_RGBLED_H

//****************************************************************************
// Include(s)
//****************************************************************************	

#include "main.h"

//****************************************************************************
// Shared variable(s)
//****************************************************************************	


//****************************************************************************
// Public Function Prototype(s):
//****************************************************************************

void rgbLedSet(uint8_t r, uint8_t g, uint8_t b);
void rgbLedRefresh(void);
void rgbLedRefreshFade(void);
uint8_t rgbLedGetFade(void);
uint8_t rgbLedCenterPulse(uint8_t halfWidth);
void rgbLedRefresh_testcode_blocking(void);

//****************************************************************************
// Definition(s):
//****************************************************************************

#define FADE_PERIOD_MS				1000
#define FADE_MIDPOINT				500
#define FADE_MIDPOINT_UINT8			250

#endif	//INC_RGBLED_H
