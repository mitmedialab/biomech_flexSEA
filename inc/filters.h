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
	[Lead developper] Jean-Francois (JF) Duval, jfduval at dephy dot com.
	[Origin] Based on Jean-Francois Duval's work at the MIT Media Lab 
	Biomechatronics research group <http://biomech.media.mit.edu/>
	[Contributors] Luke Mooney
*****************************************************************************
	[This file] filters: Digital filters
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2017-03-27 | jfduval | Released under GPL-3.0 release
****************************************************************************/

#ifndef INC_FILTERS_H
#define INC_FILTERS_H

//****************************************************************************
// Include(s)
//****************************************************************************	

#include "main.h"
#include <flexsea_user_structs.h>
#include "flexsea_global_structs.h"

//****************************************************************************
// Prototype(s):
//****************************************************************************

int32_t filt_array_10khz(int64_t *,int64_t *,int,int64_t);
int32_t filt_array_1khz(int64_t *,int64_t *,int,int64_t);
int32_t filt_array_250hz(int64_t *,int64_t *,int,int64_t);
int get_median(int, int, int);
void filt_array_1khz_struct(struct filtvar_s *, int);

int32_t get_accl_1k_5samples_downsampled(struct diffarr_s *);
int32_t get_vel_1k_5samples(struct diffarr_s *);
int32_t get_vel_1k_5samples_downsampled(struct diffarr_s *);

//****************************************************************************
// Definition(s):
//****************************************************************************

//Motor current ADC:

//****************************************************************************
// Shared variable(s)
//****************************************************************************	


//****************************************************************************
// Structure(s):
//****************************************************************************

#endif	//INC_FILTERS_H
