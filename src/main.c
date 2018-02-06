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
#include "main_fsm.h" //various FSMs required for communication, LEDs, and 10kHz loop
#include "user-mn.h" //define active project and hardware components needed for project
#include "flexsea_board.h" //designate slaves and masters for multiboard systems
#include "flexsea_system.h" //tx and rx commands for interboard comms
#include "flexsea_global_structs.h" //data structs for motors, sensors, etc.

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
	//This is a large 2D array with all system tx and rx function pointers
	//JF indexes into the array to reference serial functions
	//Adding a project requires editing init_flexsea_payload_ptr_user() within
	init_flexsea_payload_ptr();

	//Initialize all the peripherals
	//Start clocks and peripherals specified in "user-mn.h"
	//Note: seems like USE_RS485 always active regardless of user settings
	//may be result of always pairing manage to exec
	init_peripherals();

	//encoder angles are global bw manage and execute
	initializeGlobalStructs();

	//Start receiving from master via interrupts
	//SPI4 is link to plan board via expansion connector
	//Will use SPI4 for odroid?
	flexsea_start_receiving_from_master();

	//Test code, use with care. Normal code might NOT run when enabled!
	//test_code_blocking();
	//test_code_non_blocking();

	//initializes active project vars
	//Note: ignore all BILATERAL_MASTER and BILATERAL_SLAVE defs
	//new projects must be included in this function
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
			//fsm 4 and 8 are user-defined
			//during user-defined fsms, we can tx_rw to slave
			//tx_w from slave unpacked during mainFSM10kHz() by calling appropriate function
			//each of these fsms will run once per millisecond
			fsmCases[tb_100us_timeshare]();

			//Increment value, limits to 0-9
			tb_100us_timeshare++;
			tb_100us_timeshare %= 10;

			//The code below is executed every 100us, after the previous slot.
			//receive packets on both master and slave ports
			//packets contain data used to index into payload ptr array to call serial functions
			//Keep it short!
			mainFSM10kHz();
		}
	}
}
