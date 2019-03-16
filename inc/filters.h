/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'flexsea-execute' Advanced Motion Controller
	Copyright (C) 2017 Dephy, Inc. <http://dephy.com/>
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


//****************************************************************************
// Shared variable(s)
//****************************************************************************


//****************************************************************************
// Structure(s):
//****************************************************************************

#endif	//INC_FILTERS_H
