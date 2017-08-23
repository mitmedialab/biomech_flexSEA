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
	[This file] main_fsm: Contains all the case() code for the main FSM
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-23 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

//****************************************************************************
// Include(s)
//****************************************************************************

#include "user-mn.h"
#include <adc.h>
#include <dio.h>
#include <i2c.h>
#include <master_slave_comm.h>
#include <ui.h>
#include "main.h"
#include "main_fsm.h"
#include "flexsea_global_structs.h"
#include "flexsea_board.h"
#include "rgb_led.h"
#include "cmd-Rigid.h"
#include "flexsea_system.h"
#include "spi.h"
#include "misc.h"
#include "svm.h"

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

//Case 0: slaveComm
void mainFSM0(void)
{
	slaveTransmit(PORT_RS485_1);
}

//Case 1: I2C1 - IMU
void mainFSM1(void)
{
	i2c1_fsm();
}

//Case 2: I2C2 - Unused
void mainFSM2(void)
{
	i2c2_fsm();
}

//Case 3:
void mainFSM3(void)
{
	independentWatchdog();

	readInternalTempSensor();
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
	slaveTransmit(PORT_UART_EX);
}

//Case 6:
void mainFSM6(void)
{
	//ADC:
	startAdcConversion();
}

//Case 7:
void mainFSM7(void)
{
	autoStream();
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

	//SPI, USB or Wireless reception from a Plan board:
	flexsea_receive_from_master();

	//RS-485 or UART reception from an Execute board:
	flexsea_receive_from_slave();

	//Did we receive new commands? Can we parse them?
	parseMasterCommands(&new_cmd_led);
	parseSlaveCommands(&new_cmd_led);

	/*
	//Test: ToDo: this trick can be integrated in the stack, with a programmable
	//number of passes.
	if(commPeriph[PORT_USB].rx.bytesReadyFlag > 0)
	{
		//We still have bytes available, let's call the functions a second time
		flexsea_receive_from_master();
		parseMasterCommands(&new_cmd_led);
	}
	*/

	completeSpiTransmit();

	#ifdef USE_SVM
	svmBackgroundMath();
	#endif	//USE_SVM
}

//Asynchronous time slots:
//========================

void mainFSMasynchronous(void)
{

}
