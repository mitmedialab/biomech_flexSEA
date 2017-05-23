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
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.	If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************
	[Lead developper] Jean-Francois (JF) Duval, jfduval at dephy dot com.
	[Origin] Based on Jean-Francois Duval's work at the MIT Media Lab
	Biomechatronics research group <http://biomech.media.mit.edu/>
	[Contributors]
*****************************************************************************
	[This file] Test code - use (with care) with PROJECT_DEV
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2017-05-10 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

//****************************************************************************
// Include(s)
//****************************************************************************

#include "main.h"
#include "dev.h"
#include "cmd-RICNU_Knee_v1.h"
#include "flexsea_system.h"
#include "flexsea_sys_def.h"
#include "flexsea.h"

//****************************************************************************
// Variable(s)
//****************************************************************************

//Use this to share info between the two FSM:
uint8_t dev_control = CTRL_NONE;
int16_t dev_pwm = 0;
int16_t dev_cur = 0;
int32_t dev_pos = 0, last_dev_pos = 0;
#ifdef SPI_MASTER
uint8_t info[2] = {PORT_EXP, PORT_EXP};
#else
uint8_t info[2] = {PORT_RS485_1, PORT_RS485_1};
#endif

#define DEV_DEMO1
//#define DEV_DEMO2

//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************

//****************************************************************************
// Public Function(s)
//****************************************************************************

void dev_fsm_1(void)
{
	#if(ACTIVE_PROJECT == PROJECT_DEV)

	static uint32_t time = 0;
	static int8_t state = 0;

	//Increment time (1 tick = 1ms)
	time++;

	//Before going to a state we refresh values:
	//...

	switch(state)
	{
		case 0:	//Wait for 5 seconds to let everything load

			dev_pwm = 0;

			if (time >= 5000)
			{
				time = 0;
				state = 1;
			}

			break;

		case 1:	//Set to Position Control mode

			dev_pwm = 0;

			tx_cmd_ctrl_mode_w(TX_N_DEFAULT, CTRL_POSITION);
			packAndSend(P_AND_S_DEFAULT, FLEXSEA_EXECUTE_1, info, SEND_TO_SLAVE);

			if (time >= 5)
			{
				time = 0;
				state = 2;
			}

			break;

		case 2:	//Set Position Gains

			dev_pwm = 0;

			#ifdef DEV_DEMO1
			tx_cmd_ctrl_p_g_w(TX_N_DEFAULT, 50, 0, 0);
			#else
			tx_cmd_ctrl_p_g_w(TX_N_DEFAULT, 10, 0, 0);
			#endif
			packAndSend(P_AND_S_DEFAULT, FLEXSEA_EXECUTE_1, info, SEND_TO_SLAVE);

			if (time >= 5)
			{
				time = 0;
				state = 3;
			}

			break;

		#ifdef DEV_DEMO1
		case 3:	//Position setpoint depends on analog reading

			dev_pos = 10 * exec1.analog[0];

			break;
		#endif	//DEV_DEMO1

		#ifdef DEV_DEMO2
		case 3:	//Position setpoint moves up and down

			dev_pos = 10 * time;
			if (time >= 4000)
			{
				//time = 0;
				state = 4;
			}

			break;

		case 4:	//Position setpoint moves up and down

			dev_pos = 10 * (8000-time);
			if (time >= 8000)
			{
				time = 0;
				state = 3;
			}

			break;

		#endif	//DEV_DEMO1

		default:
			//Handle exceptions here
			break;
	}

	#endif 	//(ACTIVE_PROJECT == PROJECT_DEV)
}

//Second state machine for the Dev project:
//Deals with the communication between Manage and 1x Execute
//This function is called at 1kHz. We divide it down to 250Hz for now
void dev_fsm_2(void)
{
	#if(ACTIVE_PROJECT == PROJECT_DEV)

	static uint8_t ex_refresh_fsm_state = 0;
	static uint32_t timer = 0;
	//uint8_t info[2] = {PORT_RS485_1, PORT_RS485_1};

	//This FSM talks to the slaves at 250Hz each
	switch(ex_refresh_fsm_state)
	{
		case 0:		//Power-up

			if(timer < 10000)
			{
				//We wait 7s before sending the first commands
				timer++;
			}
			else
			{
				//Ready to start transmitting
				ex_refresh_fsm_state = 1;
			}

			break;

		case 1:	//Communicating with Execute #1 - Read All

			tx_cmd_data_read_all_r(TX_N_DEFAULT);
			packAndSend(P_AND_S_DEFAULT, FLEXSEA_EXECUTE_1, info, SEND_TO_SLAVE);
			ex_refresh_fsm_state++;

			break;

		case 2:	//

			//Skipping one cycle
			ex_refresh_fsm_state++;

			break;

		case 3:	//Communicating with Execute #1 - Open Setpoint

			if((dev_pos > (last_dev_pos + 5)) || (dev_pos < (last_dev_pos - 5)))
			{
				tx_cmd_ctrl_p_w(TX_N_DEFAULT, dev_pos, dev_pos, dev_pos, 200000, 200000);
				packAndSend(P_AND_S_DEFAULT, FLEXSEA_EXECUTE_1, info, SEND_TO_SLAVE);
				last_dev_pos = dev_pos;
			}

			ex_refresh_fsm_state++;

			break;

		case 4:	//

			//Skipping one cycle
			ex_refresh_fsm_state = 1;

			break;
	}

	#endif	//ACTIVE_PROJECT == PROJECT_DEV
}

//****************************************************************************
// Private Function(s)
//****************************************************************************

//...
