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

//****************************************************************************
// Include(s)
//****************************************************************************

#include "main.h"
#include "filters.h"
#include <flexsea_user_structs.h>

//****************************************************************************
// Variable(s)
//****************************************************************************

//1st order Butterworth LPF coefficiencts for cutoff frequencies from 1 to 50
int64_t as_10k[50] = {-65495,-65454,-65413,-65371,-65330,-65289,-65248,-65207,-65166,-65126,-65085,-65044,-65003,-64962,-64921,-64880,-64840,-64799,-64758,-64718,-64677,-64636,-64596,-64555,-64515,-64474,-64434,-64393,-64353,-64312,-64272,-64231,-64191,-64151,-64110,-64070,-64030,-63990,-63949,-63909,-63869,-63829,-63789,-63749,-63709,-63669,-63629,-63589,-63549,-63509};
int64_t bs_10k[50] = {21076,42139,63189,84226,105249,126259,147257,168240,189211,210169,231114,252045,272964,293870,314762,335642,356508,377362,398202,419030,439845,460647,481436,502212,522975,543726,564463,585188,605900,626599,647286,667959,688620,709269,729904,750527,771137,791735,812320,832892,853452,873999,894534,915056,935565,956062,976547,997019,1017478,1037925};
int64_t as_1k[50] = {-32563,-32359,-32156,-31955,-31754,-31555,-31358,-31161,-30965,-30771,-30578,-30386,-30195,-30005,-29817,-29629,-29442,-29257,-29072,-28889,-28706,-28525,-28345,-28165,-27987,-27809,-27632,-27457,-27282,-27108,-26935,-26763,-26592,-26422,-26252,-26084,-25916,-25749,-25583,-25417,-25253,-25089,-24926,-24764,-24603,-24442,-24282,-24123,-23965,-23807};
int64_t bs_1k[50] = {3284,6547,9791,13014,16218,19402,22567,25713,28840,31949,35039,38112,41166,44203,47223,50225,53210,56179,59131,62066,64985,67888,70776,73647,76504,79345,82171,84982,87778,90559,93326,96079,98818,101543,104254,106952,109636,112306,114964,117609,120240,122859,125466,128060,130641,133211,135769,138314,140848,143370};
int64_t as_250[50] = {-7989,-7790,-7597,-7407,-7222,-7041,-6864,-6691,-6521,-6354,-6191,-6031,-5874,-5719,-5567,-5418,-5271,-5127,-4985,-4845,-4707,-4571,-4437,-4305,-4174,-4045,-3918,-3792,-3668,-3545,-3423,-3303,-3184,-3066,-2949,-2834,-2719,-2605,-2492,-2380,-2269,-2158,-2049,-1940,-1831,-1723,-1616,-1509,-1403,-1297};
int64_t bs_250[50] = {813,1607,2382,3139,3879,4603,5311,6005,6684,7351,8004,8645,9274,9892,10499,11096,11683,12260,12829,13389,13941,14485,15021,15550,16072,16587,17096,17599,18096,18588,19074,19555,20032,20504,20971,21434,21893,22348,22800,23248,23693,24135,24574,25010,25443,25875,26304,26730,27155,27578};


//****************************************************************************
// Function(s)
//****************************************************************************

//Filters raw signal at cut_off frequency if sampled at 10 kHz
//filt is 1024 x raw in order to maintain precision
//this function also shifts the arrays
int32_t filt_array_10khz(int64_t * raw, int64_t * filt, int cut_off, int64_t new_raw)
{  
    
    filt[1] = filt[0];
    raw[1] = raw[0];
    raw[0] = new_raw;
    //ensure the cut-off frequency is inbetween 1 and 50 Hz
    if (cut_off<1)
    cut_off = 1;
    else if (cut_off>50)
    cut_off = 50;
    
    filt[0] = (bs_10k[cut_off-1]*(raw[1]+raw[0])-as_10k[cut_off-1]*filt[1])>>16;   
    return (int32_t)(filt[0]>>10);
}

//Filters raw signal at cut_off frequency if sampled at 1 kHz
//filt is 32 x raw in order to maintain precision
//returns the filtered value at the correct scaling
int32_t filt_array_1khz(int64_t * raw, int64_t * filt, int cut_off, int64_t new_raw)
{  
    filt[1] = filt[0];
    raw[1] = raw[0];
    raw[0] = new_raw;
    //ensure the cut-off frequency is inbetween 1 and 50 Hz
    if (cut_off<1)
    cut_off = 1;
    else if (cut_off>50)
    cut_off = 50;
    
    filt[0] = (bs_1k[cut_off-1]*(raw[1]+raw[0])-as_1k[cut_off-1]*filt[1])>>15;   
    return (int32_t)(filt[0]>>5);
}

//Filters raw signal at cut_off frequency if sampled at 250 Hz
//filt is 8 x raw in order to maintain precision
//returns the filtered value at the correct scaling
int32_t filt_array_250hz(int64_t * raw, int64_t * filt, int cut_off, int64_t new_raw)
{  
    filt[1] = filt[0];
    raw[1] = raw[0];
    raw[0] = new_raw;
    //ensure the cut-off frequency is inbetween 1 and 50 Hz
    if (cut_off<1)
    cut_off = 1;
    else if (cut_off>50)
    cut_off = 50;
    
    filt[0] = (bs_250[cut_off-1]*(raw[1]+raw[0])-as_250[cut_off-1]*filt[1])>>13;   
    return (int32_t)(filt[0]>>3);
}

int get_median(int a, int b, int c)
{
    if ((a>=b && a<=c) || (a>=c && a<=b))
    return a;
    else if ((b>=a && b<=c) || (b>=c && b<=a))
    return b;
    
    return c;
}

//Filters raw signal at cut_off frequency if sampled at 1 kHz
//filt is 32 x raw in order to maintain precision
//returns the filtered value at the correct scaling
void filt_array_1khz_struct(struct filtvar_s *fv, int cut_off)
{  
    fv->filts[1] = fv->filts[0];
    fv->raws[1] = fv->raws[0];
    fv->raws[0] = fv->raw;
    //ensure the cut-off frequency is inbetween 1 and 50 Hz
    if (cut_off<1)
    cut_off = 1;
    else if (cut_off>50)
    cut_off = 50;
    
    fv->filts[0] = (bs_1k[cut_off-1]*(fv->raws[1]+fv->raws[0])-as_1k[cut_off-1]*fv->filts[1])>>15;
    fv->filt = (int32_t)(fv->filts[0]>>5);
}


int32_t acc_reg_coef_1k_down[5] = {2915,-1458,-2915,-1458,2915};
int32_t get_accl_1k_5samples_downsampled(struct diffarr_s * das)
{
    uint8_t jj;
    int32_t accsum;
    int32_t initval = get_diffarr_elmnt(das,0);
    accsum = 0;
    
    for (jj=0; jj<4; jj++)
    {
        accsum+= (get_diffarr_elmnt(das,28-jj*7)-initval)*acc_reg_coef_1k_down[jj];
    }
    return accsum*2;   
}

int32_t vel_reg_coef_1k_5samples[5] = {-200,-100,0,100,200};
int32_t get_vel_1k_5samples(struct diffarr_s * das)
{
    uint8_t jj;
    int32_t accsum;
    accsum = 0;
    int32_t initval = get_diffarr_elmnt(das,0);
    for (jj=0; jj<4; jj++)
    {
        accsum+= (get_diffarr_elmnt(das,4-jj)-initval)*vel_reg_coef_1k_5samples[jj];
    }
    return accsum; // /10
}

int32_t vel_reg_coef_1k_5samples_downsampled[5] = {-286,-143,0,143,286};
int32_t get_vel_1k_5samples_downsampled(struct diffarr_s * das)
{
    uint8_t jj;
    int32_t accsum;
    accsum = 0;
    int32_t initval = get_diffarr_elmnt(das,0);
    for (jj=0; jj<4; jj++)
    {
        accsum+= (get_diffarr_elmnt(das,28-jj*7)-initval)*vel_reg_coef_1k_5samples_downsampled[jj];
    }
    return accsum/10; // /10
}
