/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'flexsea-execute' System commands & functions
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
	[This file] calibration_tools: functionality & logic tracking the state
	of calibration procedures
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2017-02-07 | dweisdorf | Initial GPL-3.0 release
	*
****************************************************************************/

#ifndef CALIBRATION_TOOLS_H
#define CALIBRATION_TOOLS_H

#include <stdint.h>
#include "flexsea_system.h"

// DEFINITIONS

// EXTERNS
extern uint8_t calibrationFlags, calibrationNew;

// UTILITY FUNCTIONS
uint8_t isFindingPoles();
uint8_t isFindingCurrentZeroes();
uint8_t isUVLO();
uint8_t isI2T();
uint8_t isRunningCalibrationProcedure();
uint8_t isLegalCalibrationProcedure(uint8_t procedure);
int8_t runtimeCalibration(void);
int8_t mnFindPolesFSM(void);

#define CALIB_FINDPOLES_DELAY	(126*410+4000)		//ms

#define CALIB_NOT		0
#define CALIB_ONGOING	1
#define CALIB_DONE		2

#endif	//CALIBRATION_TOOLS_H
