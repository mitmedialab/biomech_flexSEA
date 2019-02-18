/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'flexsea-manage' Mid-level computing, and networking
	Copyright (C) 2019 Dephy, Inc. <http://dephy.com/>
*****************************************************************************
	[Lead developer] Jean-Francois (JF) Duval, jfduval at dephy dot com.
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
#include "uarts.h"

//****************************************************************************
// Variable(s)
//****************************************************************************

uint8_t newMasterCmdLed = 0, newSlaveCmdLed = 0, newPacketsFlag = 0;
uint8_t dftWatch = 0;
volatile uint32_t calibTimeCounter = 0, latchCalibTimeCounter = 0;

//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************

uint16_t computeFsmStatus(volatile int8_t *timingError);
void transmitMultiFrame();

//****************************************************************************
// Public Function(s)
//****************************************************************************

//1kHz time slots:
//================

//Case 0:
void mainFSM0(void)
{
	//Calibration Tools or User FSM?:
	int8_t s = runtimeCalibration();
	if(s == CALIB_NOT)
	{
		calibTimeCounter = 0;
		user_fsm_2();
	}
	else if(s == CALIB_DONE)
	{
		reset_user_code();
		latchCalibTimeCounter = calibTimeCounter;
	}
	else
	{
		//Ongoing calibration
		calibTimeCounter++;
	}

	#if (MULTI_DOF_N == 0)
	slaveTransmit(PORT_RS485_1);
	#endif
}

//Case 1:
void mainFSM1(void)
{
	//UI RGB LED
	rgbLedRefreshFade();
	rgb_led_ui(0, 0, 0, newMasterCmdLed);	//ToDo add error codes
	if(newMasterCmdLed) {newMasterCmdLed = 0;}

	//Constant LED0 flashing while the code runs
	LED0(rgbLedCenterPulse(12));

	//Communication with Ex (TX):
	slaveTransmit(PORT_UART_EX);
}

//Case 2:
void mainFSM2(void)
{
	//IMU:
	i2c1_fsm();
}

//Case 3:
void mainFSM3(void)
{
	independentWatchdog();
	combineStatusFlags();
	readInternalTempSensor();

	//Analyze FSMs and set status bytes appropriately
	uint16_t mnFsmStatus = computeFsmStatus(timingError);
	uint16_t exFsmStatus = (rigid1.ex.status >> 8) & 0xFF;
	rigid1.mn.status = (mnFsmStatus << 8) | exFsmStatus;
	//For now we display mn.status in genVar[9]
	rigid1.mn.genVar[9] = mnFsmStatus;

	//Expansion port I2C
	i2c2_fsm();
}

//Case 4:
void mainFSM4(void)
{
	user_fsm_1();
}

//Case 5:
void mainFSM5(void)
{

}

//Case 6:
static uint8_t autoParsed;
void mainFSM6(void)
{
	//ADC:
	startAdcConversion();
	updateADCbuffers();

	//Auto-streaming:
	autoParsed = 0;
	if(receiveFxPacketByPeriph(comm_multi_periph + PORT_USB) && comm_multi_periph[PORT_USB].out.unpackedIdx > 0)
	{
		autoParsed++;
	}

	if(!autoParsed && receiveFxPacketByPeriph(comm_multi_periph + PORT_WIRELESS) && comm_multi_periph[PORT_WIRELESS].out.unpackedIdx > 0)
	{
		autoParsed++;
	}

	//LED turns green when commands are received
	if(autoParsed)
	{
		newMasterCmdLed = 1;
	}
}

//Case 7:
void mainFSM7(void)
{
	if(!autoParsed)
	{
		autoStream();
	}

	//Timestamp needed by GUI:
	rigid1.ctrl.timestamp++;
}

//Case 8:
void mainFSM8(void)
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
}

//Case 9:
void mainFSM9(void)
{
	//Receive data from Ex
	//receiveFlexSEAPacket(PORT_RS485_2, &newPacketsFlag, &newSlaveCmdLed, &dftWatch);		//Ex
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

	// Multi Packet stuff
	static uint8_t flip = 0;
	flip ^= 1;
	if(flip)
	{
		transmitMultiFrame();
	}

	//Test - used to be in FSM9:
	receiveFlexSEAPacket(PORT_RS485_2, &newPacketsFlag, &newSlaveCmdLed, &dftWatch);
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

// Private functions
// ==============================================

uint16_t computeFsmStatus(volatile int8_t *timingError)
{
	int8_t mostOffendingFSM = -1;
	uint8_t numOffenses = 0;
	int i;
	for(i = 0; i < 10; i++)
	{
		if(timingError[i] > numOffenses)
		{
			mostOffendingFSM = i;
			numOffenses = timingError[i];
		}
	}

	uint16_t fsmStatus = 0;
	if(mostOffendingFSM != -1)
	{
		fsmStatus = (mostOffendingFSM << 4) | (numOffenses & 0x0F);
	}

	return fsmStatus;
}

void transmitMultiFrame()
{
	transmitFxPacket(PORT_USB);
	transmitFxPacket(PORT_WIRELESS);
}
