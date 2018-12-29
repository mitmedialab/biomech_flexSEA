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
#include "flexsea_interface.h"
#include "flexsea_comm_multi.h"

#include "spi.h"
#include "misc.h"
#include "calibration_tools.h"

#if ACTIVE_PROJECT == PROJECT_MIT_DLEG
#include "user-mn-MIT-DLeg.h"
#include "walking_state_machine.h"
#include "actuator_functions.h"
#endif


//****************************************************************************
// Variable(s)
//****************************************************************************

uint8_t newMasterCmdLed = 0, newSlaveCmdLed = 0, newPacketsFlag = 0;
uint8_t dftWatch = 0;

//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************

void transmitMultiFrame()
{
	transmitFxPacket(PORT_USB);
	transmitFxPacket(PORT_WIRELESS);
}

//****************************************************************************
// Public Function(s)
//****************************************************************************

//1kHz time slots:
//================

//Case 0: slaveComm
void mainFSM0(void)
{
	#if (MULTI_DOF_N == 0)
	slaveTransmit(PORT_RS485_1);
	#endif
}

//Case 1: I2C1 - IMU
void mainFSM1(void)
{
	i2c1_fsm();
}

//Case 2: I2C2 - Expansion
void mainFSM2(void)
{
	i2c2_fsm();
}

//Case 3:
void mainFSM3(void)
{
	independentWatchdog();
	combineStatusFlags();
	readInternalTempSensor();
}

//Case 4: User Functions
void mainFSM4(void)
{
	user_fsm_1();
	DEBUG_H0(1);
	if (isEnabledUpdateSensors) {
    	updateSensorValues(&act1);	// updates all actuator sensors, will throw safety flags. takes about 33us run
    }
	DEBUG_H0(0);
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
	updateADCbuffers();

	uint8_t parsed = 0;
	parsed += receiveFxPacketByPeriph(comm_multi_periph + PORT_USB);
	parsed += receiveFxPacketByPeriph(comm_multi_periph + PORT_WIRELESS);

	if(!parsed){autoStream();}
}

//Case 7:
void mainFSM7(void)
{
	int i;

	for(i = 0; i < NUMBER_OF_PORTS; ++i)
	{
		if(comm_multi_periph[i].out.unpackedIdx)
		{
			packMultiPacket(&(comm_multi_periph[i].out));
			comm_multi_periph[i].out.unpackedIdx = 0;
		}
	}

	rigid1.ctrl.timestamp++;
}

//Case 8: User functions
void mainFSM8(void)
{
	//Calibration Tools or User FSM?:
	int8_t s = runtimeCalibration();
	if(s == CALIB_NOT)
	{
		user_fsm_2();
	}
	else if(s == CALIB_DONE)
	{
		reset_user_code();
	}
}

//Case 9: User Interface
void mainFSM9(void)
{
	//UI RGB LED
	rgbLedRefreshFade();
	rgb_led_ui(0, 0, 0, newMasterCmdLed);    //ToDo add error codes
	if(newMasterCmdLed) {newMasterCmdLed = 0;}

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

//maybe this should be not a "#ifdef FLEXSEA_MANAGE", but instead "am I connected to plan stack"
#ifdef BOARD_TYPE_FLEXSEA_MANAGE

	static uint8_t flip = 0;
	flip ^= 1;
	if(flip)
	{
		transmitMultiFrame();
	}

#endif //FLEXSEA_MANAGE

	//New approach - WiP:
	flexsea_receive_from_slave();	//Only for the RS-485 transceivers
	//Master:
	receiveFlexSEAPacket(PORT_SPI, &newPacketsFlag, &newMasterCmdLed, &spi4Watch);

	receiveFlexSEAPacket(PORT_USB, &newPacketsFlag, &newMasterCmdLed, &dftWatch);
	receiveFlexSEAPacket(PORT_WIRELESS, &newPacketsFlag, &newMasterCmdLed, &dftWatch);

	//Slave:
	receiveFlexSEAPacket(PORT_RS485_2, &newPacketsFlag, &newSlaveCmdLed, &dftWatch);	//Ex
	receiveFlexSEAPacket(PORT_EXP, &newPacketsFlag, &newSlaveCmdLed, &dftWatch);

	//Variable:
	#if (MULTI_DOF_N == 0)

		receiveFlexSEAPacket(PORT_RS485_1, &newPacketsFlag, &newSlaveCmdLed, &dftWatch);

	#endif	//(MULTI_DOF_N == 0)

	#if (MULTI_DOF_N == 1)

		receiveFlexSEAPacket(PORT_RS485_1, &newPacketsFlag, &newMasterCmdLed, &dftWatch);

		//Time to reply - RS-485?
		sendMasterDelayedResponse();

	#endif	//(MULTI_DOF_N == 1)

	//Error recovery:
	spiMonitoring(4);

	completeSpiTransmit();
}

//Asynchronous time slots:
//========================

void mainFSMasynchronous(void)
{

}
