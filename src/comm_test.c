/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'flexsea-execute' Advanced Motion Controller
	Copyright (C) 2017 Dephy, Inc. <http://dephy.com/>
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
