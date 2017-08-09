/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'flexsea-execute' Advanced Motion Controller
	Copyright (C) 2017 Dephy, Inc. <http://dephy.com/>

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
	[This file] Comm Test: Communication quality testing tool
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2017-01-11 | jfduval | New file
	*
****************************************************************************/

//****************************************************************************
// Include(s)
//****************************************************************************

#include "main.h"
#include "comm_test.h"
#include <flexsea_system.h>
#include <flexsea_comm.h>

//****************************************************************************
// Variable(s)
//****************************************************************************

//Temporary - replace with what comes from Plan:
uint16_t divider = 100;

int32_t receivedPackets = 0;

//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************


//****************************************************************************
// Public Function(s)
//****************************************************************************

//Call this once before calling comm_test()
void init_comm_test(void)
{
	initRandomGenerator(HAL_GetTick());
	reset_comm_test_stats();
}

//Call this at 10kHz:
void comm_test(void)
{
	static uint16_t cnt = 0;
	uint8_t info[2] = {PORT_RS485_1, PORT_RS485_1};
	static uint8_t packetIndex = 0;

	//Protects from invalid dividers:
	(divider < 2) ? divider = 2 : divider;

	cnt++;
	cnt %= divider;
	if(!cnt)
	{
		packetIndex++;

		//Prepare and send command:
		tx_cmd_tools_comm_test_r(TX_N_DEFAULT, 1, 20, packetIndex);
		packAndSend(P_AND_S_DEFAULT, FLEXSEA_EXECUTE_1, info, SEND_TO_SLAVE);
	}
	else if(cnt == 1)
	{
		//Update stats:
		receivedPackets = goodPackets + badPackets;
	}
}

//Resets all stats
void reset_comm_test_stats(void)
{
	sentPackets = 0;
	goodPackets = 0;
	badPackets = 0;
	receivedPackets = 0;
}

//****************************************************************************
// Private Function(s)
//****************************************************************************
