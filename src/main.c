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
	[This file] main: FlexSEA-Manage
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-23 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

// FlexSEA: Flexible & Scalable Electronics Architecture

//****************************************************************************
// Include(s)
//****************************************************************************

#include <master_slave_comm.h>
#include <misc.h>
#include <timer.h>
#include "main.h"
#include "main_fsm.h"
#include "user-mn.h"
#include "flexsea_board.h"
#include "flexsea_system.h"
#include "flexsea_global_structs.h"

//****************************************************************************
// Variable(s)
//****************************************************************************

//Map fsm case to an index:
void (*fsmCases[10])(void) = {&mainFSM0, &mainFSM1, &mainFSM2, &mainFSM3, \
			&mainFSM4, &mainFSM5, &mainFSM6, &mainFSM7,	&mainFSM8, &mainFSM9};

//****************************************************************************
// Function(s)
//****************************************************************************

int main(void)
{
	//Prepare FlexSEA stack:
	init_flexsea_payload_ptr();

	//Initialize all the peripherals
	init_peripherals();

	initializeGlobalStructs();

	//Start receiving from master via interrupts
	flexsea_start_receiving_from_master();

	//Test code, use with care. Normal code might NOT run when enabled!
	test_code_blocking();
	//test_code_non_blocking();

	init_user();

	//Infinite loop
	while(1)
	{
		//Time bases:
		//===========

		if(tb_100us_flag == 1)
		{
			tb_100us_flag = 0;

			//Timing FSM:
			fsmCases[tb_100us_timeshare]();

			//Increment value, limits to 0-9
			tb_100us_timeshare++;
			tb_100us_timeshare %= 10;

			//The code below is executed every 100us, after the previous slot.
			//Keep it short!
			mainFSM10kHz();
		}
	}
}
