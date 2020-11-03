/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'flexsea-system' System commands & functions
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
	[This file] flexsea_global_structs: contains all the data structures
	used across the project
****************************************************************************/

//****************************************************************************
// Include(s)
//****************************************************************************

#include <stdint.h>
#include <flexsea_board.h>
#include <flexsea_global_structs.h>

#if(defined BOARD_TYPE_FLEXSEA_EXECUTE)
	#include "mag_encoders.h"
	
	#if(!defined BOARD_SUBTYPE_RIGID && !defined BOARD_SUBTYPE_POCKET)
		#include "user-ex.h"
	#else
		#include "user-ex-rigid.h"
	#endif
	
	#ifdef USE_QEI
		#include "ext_input.h"
	#endif	//USE_QEI
#endif

//****************************************************************************
// Variable(s)
//****************************************************************************

//Data structures:
struct execute_s exec1, exec2, exec3, exec4;
struct ricnu_s ricnu_1;
struct battery_s batt1;
struct manage_s manag1 = {.battPtr = &batt1}, manag2 = {.battPtr = &batt1};
struct strain_s strain1;
struct in_control_s in_control_1;
struct gossip_s gossip1, gossip2;
struct user_data_s user_data_1;

#include <flexsea_board.h>

#if(defined BOARD_TYPE_FLEXSEA_PLAN || defined BOARD_TYPE_FLEXSEA_MANAGE)
int32_t exec1Ang, exec2Ang, exec3Ang, exec4Ang, execXAng;
int32_t exec1AngVel, exec2AngVel, exec3AngVel, exec4AngVel;
#endif

//****************************************************************************
// Function(s)
//****************************************************************************

void initializeGlobalStructs()
{
	#if(defined BOARD_TYPE_FLEXSEA_PLAN || defined BOARD_TYPE_FLEXSEA_MANAGE)
		exec1.enc_ang = &exec1Ang;
		exec2.enc_ang = &exec2Ang;
		exec3.enc_ang = &exec3Ang;
		exec4.enc_ang = &exec4Ang;

		exec1.enc_ang_vel = &exec1AngVel;
		exec2.enc_ang_vel = &exec2AngVel;
		exec3.enc_ang_vel = &exec3AngVel;
		exec4.enc_ang_vel = &exec4AngVel;
	#elif(defined BOARD_TYPE_FLEXSEA_EXECUTE)
		
		#ifdef BOARD_SUBTYPE_POCKET
		exec1.enc_ang = &encoder.count;
		#else
		exec1.enc_ang = &(as5047.signed_ang);
		exec1.enc_ang_vel = &(as5047.signed_ang_vel);
		#endif
        
        user_data_1.w[0] = 0;
        user_data_1.w[1] = 0;
        user_data_1.w[2] = 0;
        user_data_1.w[3] = 0;
	#endif
}

void init_diffarr(struct diffarr_s * das)
{
   static int jj;
   for (jj=0;jj<50;jj++)
	{
		das->vals[jj] = 0;
	}
	das->indx = 0;
	das->curval = 0;
	das->curdif = 0;
}

void update_diffarr(struct diffarr_s * das, int32_t cvl, int32_t difflen)
{
	das->curval = cvl;
	das->vals[(das->indx+1)%50] = das->curval;
	das->indx = (das->indx+1)%50;
	das->curdif = das->vals[das->indx]-das->vals[(das->indx-difflen+50)%50];
}

int32_t get_diffarr(struct diffarr_s * das, int32_t difflen)
{

	return das->vals[das->indx]-das->vals[(das->indx-difflen+50)%50];
}

int32_t get_diffarr_elmnt(struct diffarr_s * das, int32_t indx)
{
	return das->vals[(das->indx-indx+50)%50];
}

void update_diffarr_avg(struct diffarr_s * das, int32_t num)
{
	static uint8_t jj;
	static int32_t sum;

	sum = 0;
	for(jj = 0;jj < num; jj++)
	{
		sum += das->vals[(das->indx-jj+50) % 50];
	}
	das->avg = sum/num;
}

void init_circbuf_float(struct circbuf_float_s * cfs)
{
	   static uint8_t jj;
	   for (jj=0;jj<CIRCBUFFLOATLEN;jj++)
		{
			cfs->x[jj] = 0;
		}
		cfs->i = 0;
		cfs->curval = 0;
}

void update_circbuf_float(struct circbuf_float_s * das, float cvl)
{
	das->x[(das->i+1)%CIRCBUFFLOATLEN] = cvl;
	das->i = (das->i+1)%CIRCBUFFLOATLEN;
	das->curval = cvl;
}

float get_circbuf_float_val(struct circbuf_float_s * das, int16_t indx)
{
	return das->x[(das->i-indx+CIRCBUFFLOATLEN)%CIRCBUFFLOATLEN];
}

void init_filt_float(struct filt_float_s * ffs, int16_t co)
{
	ffs->curval = 0;
	ffs->curdiff = 0;
	ffs->cutoff = co;
	ffs->cntr = 0;
	ffs->newvalsum = 0;
	init_circbuf_float(&(ffs->x));
	init_circbuf_float(&(ffs->y));
	update_filt_float_cutoff(ffs,co);
}

// b and as for frequencies [10:20 25:5:100] matlab script found in technical/simulations/c-filter-function
// cutoffs below 10 are done by downsampling by a factor of 10
float butter_as[27][4] = {
     {1.00000000, -2.87435689, 2.75648320, -0.88189313},
     {1.00000000, -2.86179732, 2.73298010, -0.87087431},
     {1.00000000, -2.84923910, 2.70962913, -0.85999198},
     {1.00000000, -2.83668233, 2.68642975, -0.84924436},
     {1.00000000, -2.82412716, 2.66338139, -0.83862971},
     {1.00000000, -2.81157368, 2.64048349, -0.82814628},
     {1.00000000, -2.79902201, 2.61773550, -0.81779236},
     {1.00000000, -2.78647228, 2.59513685, -0.80756628},
     {1.00000000, -2.77392460, 2.57268696, -0.79746637},
     {1.00000000, -2.76137907, 2.55038528, -0.78749100},
     {1.00000000, -2.74883581, 2.52823122, -0.77763856},
     {1.00000000, -2.68615740, 2.41965511, -0.73016535},
     {1.00000000, -2.62355181, 2.31468258, -0.68553598},
     {1.00000000, -2.56103158, 2.21324020, -0.64357271},
     {1.00000000, -2.49860834, 2.11525413, -0.60410970},
     {1.00000000, -2.43629282, 2.02065048, -0.56699213},
     {1.00000000, -2.37409474, 1.92935567, -0.53207537},
     {1.00000000, -2.31202289, 1.84129669, -0.49922419},
     {1.00000000, -2.25008508, 1.75640138, -0.46831211},
     {1.00000000, -2.18828816, 1.67459862, -0.43922073},
     {1.00000000, -2.12663806, 1.59581853, -0.41183913},
     {1.00000000, -2.06513978, 1.51999264, -0.38606337},
     {1.00000000, -2.00379748, 1.44705402, -0.36179593},
     {1.00000000, -1.94261443, 1.37693738, -0.33894529},
     {1.00000000, -1.88159313, 1.30957920, -0.31742548},
     {1.00000000, -1.82073529, 1.24491776, -0.29715569},
     {1.00000000, -1.76004188, 1.18289326, -0.27805992}};
float butter_bs[27][4] = {
     {0.000029146494465671, 0.000087439483397012, 0.000087439483397012, 0.000029146494465671},
     {0.000038558861408716, 0.000115676584226149, 0.000115676584226149, 0.000038558861408716},
     {0.000049757435768683, 0.000149272307306048, 0.000149272307306048, 0.000049757435768683},
     {0.000062881179879459, 0.000188643539638378, 0.000188643539638378, 0.000062881179879459},
     {0.000078065535864091, 0.000234196607592274, 0.000234196607592274, 0.000078065535864091},
     {0.000095442508423749, 0.000286327525271246, 0.000286327525271246, 0.000095442508423749},
     {0.000115140745768738, 0.000345422237306214, 0.000345422237306214, 0.000115140745768738},
     {0.000137285618734606, 0.000411856856203818, 0.000411856856203818, 0.000137285618734606},
     {0.000161999298125576, 0.000485997894376727, 0.000485997894376727, 0.000161999298125576},
     {0.000189400830326758, 0.000568202490980274, 0.000568202490980274, 0.000189400830326758},
     {0.000219606211225382, 0.000658818633676145, 0.000658818633676145, 0.000219606211225382},
     {0.000416546139075769, 0.001249638417227306, 0.001249638417227306, 0.000416546139075769},
     {0.000699349649900982, 0.002098048949702946, 0.002098048949702946, 0.000699349649900982},
     {0.001079489822069346, 0.003238469466208038, 0.003238469466208038, 0.001079489822069346},
     {0.001567010350588316, 0.004701031051764948, 0.004701031051764948, 0.001567010350588316},
     {0.002170690841847567, 0.006512072525542700, 0.006512072525542700, 0.002170690841847567},
     {0.002898194633721374, 0.008694583901164121, 0.008694583901164121, 0.002898194633721374},
     {0.003756201027920902, 0.011268603083762707, 0.011268603083762707, 0.003756201027920902},
     {0.004750523610980883, 0.014251570832942650, 0.014251570832942650, 0.004750523610980883},
     {0.005886216155083779, 0.017658648465251336, 0.017658648465251336, 0.005886216155083779},
     {0.007167667426644542, 0.021503002279933625, 0.021503002279933625, 0.007167667426644542},
     {0.008598686086321944, 0.025796058258965832, 0.025796058258965832, 0.008598686086321944},
     {0.010182576736436910, 0.030547730209310731, 0.030547730209310731, 0.010182576736436910},
     {0.011922208058589956, 0.035766624175769868, 0.035766624175769868, 0.011922208058589956},
     {0.013820073883791945, 0.041460221651375834, 0.041460221651375834, 0.013820073883791945},
     {0.015878347948126134, 0.047635043844378402, 0.047635043844378402, 0.015878347948126134},
     {0.018098933007514445, 0.054296799022543335, 0.054296799022543335, 0.018098933007514445}};

//call at 1000 Hz for the cutoff frequency to match
// cutoffs below 10 are done by downsampling by a factor of 10
void update_filt_float(struct filt_float_s * ffs, float cvl)
{
	static float tmp;
	static uint8_t jj;

	ffs->cntr = ffs->cntr+1;
	ffs->newvalsum += cvl;

	if ((ffs->cutoff < 10 && ffs->cntr == 10) || ffs->cutoff >= 10)
	{
		update_circbuf_float(&(ffs->x),ffs->newvalsum/ffs->cntr);
		tmp = butter_bs[ffs->cutoff_indx][0]*get_circbuf_float_val(&(ffs->x),0);
		for (jj=1;jj<(sizeof(butter_as[0])/sizeof(butter_as[0][0]));jj++)
		{
			tmp = tmp + butter_bs[ffs->cutoff_indx][jj]*get_circbuf_float_val(&(ffs->x),jj);
			tmp = tmp - butter_as[ffs->cutoff_indx][jj]*get_circbuf_float_val(&(ffs->y),jj-1);
		}
	   update_circbuf_float(&(ffs->y),tmp);

	   ffs->cntr = 0;
	   ffs->newvalsum = 0;
	}
   ffs->curval = ffs->y.curval;

   ffs->curdiff = get_circbuf_float_val(&(ffs->y),0)-get_circbuf_float_val(&(ffs->y),5);
   if (ffs->cutoff<10)
   {
	   ffs->curdiff = ffs->curdiff*20.0;
   }
   else
   {
	   ffs->curdiff = ffs->curdiff*200.0;
   }
}

//newco is the cutoff frequency in Hz, it can range from 1-100
// cutoffs below 10 are done by downsampling by a factor of 10
void update_filt_float_cutoff(struct filt_float_s * ffs, int16_t newco)
{
	if (newco<=1)
	{
		ffs->cutoff = 1;
		ffs->cutoff_indx = 0;
	}
	else if (newco<10)
	{
		ffs->cutoff = newco;
		ffs->cutoff_indx = 10+((newco-2)*2);
	}
	else
	{
		if (newco>100) newco = 100;
		ffs->cutoff = (newco/5)*5;
		if (ffs->cutoff<=20)
		{
			ffs->cutoff_indx = ffs->cutoff-10;
		}
		else
		{
			ffs->cutoff_indx = ffs->cutoff/5+6;
		}
	}
}
