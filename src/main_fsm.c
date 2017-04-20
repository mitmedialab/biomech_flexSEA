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
	[This file] main_fsm: Contains all the case() code for the main FSM
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-23 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

//****************************************************************************
// Include(s)
//****************************************************************************

#include <adc.h>
#include <dio.h>
#include <i2c.h>
#include "main.h"
#include "main_fsm.h"
#include "fm_master_slave_comm.h"
#include "flexsea_cmd_stream.h"
#include "flexsea_global_structs.h"
#include "flexsea_board.h"
#include "fm_ui.h"
#include "rgb_led.h"
#include "user-mn.h"

//****************************************************************************
// Variable(s)
//****************************************************************************

uint8_t new_cmd_led = 0;

//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************

//****************************************************************************
// Public Function(s)
//****************************************************************************

//1kHz time slots:
//================

//Case 0: I2C1 + slaveComm
void mainFSM0(void)
{
	slaveTransmit(PORT_RS485_1);
}

//Case 1: I2C2
void mainFSM1(void)
{
	i2c1_fsm();
}

//Case 2:
void mainFSM2(void)
{
	i2c2_fsm();
}

//Case 3:
void mainFSM3(void)
{

}

//Case 4: User Functions
void mainFSM4(void)
{
	#if(RUNTIME_FSM1 == ENABLED)
	user_fsm_1();
	#endif //RUNTIME_FSM1 == ENABLED
}

//Case 5:
void mainFSM5(void)
{

}

//Case 6:
void mainFSM6(void)
{
	//User switch:
	manag1.sw1 = read_sw1();

	//ADC:
	startAdcConversion();
}

//Case 7:
void mainFSM7(void)
{
	static int sinceLastStreamSend[MAX_STREAMS] = {0};
	uint8_t cp_str[256] = {0};
	int i;
	for(i=0;i<isStreaming;i++)
		sinceLastStreamSend[i]++;

	for(i=0;i<isStreaming;i++)
	{
		if(sinceLastStreamSend[i] >= streamPeriods[i])
		{
			cp_str[P_XID] = streamReceivers[i];
			(*flexsea_payload_ptr[streamCmds[i]][RX_PTYPE_READ]) (cp_str, &streamPortInfos[i]);

			sinceLastStreamSend[i] -= streamPeriods[i];

			//return so as not to try to send multiple messages in the same cycle
			//since we already incremented counter, we will still average to the proper frequency
			//assuming that it is possible to hit the desired frequencies all at once (based on perforamce of comm stack)
			return;
		}
	}
}

//Case 8: User functions
void mainFSM8(void)
{
	#if(RUNTIME_FSM2 == ENABLED)
	user_fsm_2();
	#endif //RUNTIME_FSM2 == ENABLED
}

//Case 9: User Interface
void mainFSM9(void)
{
	//UI RGB LED
	rgbLedRefreshFade();
	rgb_led_ui(0, 0, 0, new_cmd_led);    //ToDo add error codes
	if(new_cmd_led) {new_cmd_led = 0;}

	//Constant LED0 flashing while the code runs
	LED0(rgbLedCenterPulse(12));
}

//10kHz time slot:
//================

void mainFSM10kHz(void)
{
	#ifdef USE_COMM_TEST

		comm_test();

	#endif	//USE_COMM_TEST

	//RGB:
	rgbLedRefresh();

	//Communication with our Master & Slave(s):
	//=========================================

	//SPI or USB reception from a Plan board:
	flexsea_receive_from_master();

	//RS-485 reception from an Execute board:
	flexsea_receive_from_slave();

	//Did we receive new commands? Can we parse them?
	parseMasterCommands(&new_cmd_led);
	parseSlaveCommands(&new_cmd_led);
}

//Asynchronous time slots:
//========================

void mainFSMasynchronous(void)
{

}
