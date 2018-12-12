#include "flexsea.h"
#include <flexsea_system.h>
#include "flexsea_cmd_control.h"
#include "flexsea_cmd_calibration.h"
#include "calibration_tools.h"
#include "user-mn.h"
#include "eeprom_user.h"
#include "rigid.h"

uint8_t calibrationFlags = 0, calibrationNew = 0;
uint8_t ctInfo[2] = {PORT_RS485_2, PORT_RS485_2};

inline uint8_t isRunningCalibrationProcedure()
{
	return calibrationFlags;
}

inline uint8_t isFindingPoles()
{
	return (calibrationFlags & CALIBRATION_FIND_POLES);
}

inline uint8_t isFindingCurrentZeroes()
{
	return (calibrationFlags & CALIBRATION_FIND_CURRENT_ZEROES);
}

inline uint8_t isUVLO()
{
	return (calibrationFlags & CALIBRATION_UVLO);
}

inline uint8_t isI2T()
{
	return (calibrationFlags & CALIBRATION_I2T);
}

inline uint8_t isLegalCalibrationProcedure(uint8_t procedure)
{
	//ensure procedure is not out of bounds
	//ensure procedure has only 1 bit true
	return ((procedure == 1 || procedure % 2 == 0) && \
		procedure <= CALIBRATION_I2T);
}

int8_t runtimeCalibration(void)
{
	int8_t retVal = 0;

	if(isFindingPoles())
	{
		retVal = mnFindPolesFSM();
	}
	else if(isUVLO())
	{
		calibrationNew = 0;
		calibrationFlags = 0;

		writeUvloEEPROM(getUVLO());

		retVal = CALIB_DONE;
	}
	else if(isI2T())
	{
		calibrationNew = 0;
		calibrationFlags = 0;

		writeI2tEEPROM(i2tBatt);

		retVal = CALIB_DONE;
	}
	else
	{
		retVal = CALIB_NOT;
	}

	return retVal;
}

//Manage Find Poles state machine:
int8_t mnFindPolesFSM(void)
{
	static uint8_t calibFSMstate = 0;
	static uint16_t subCalibFSMstate = 0;
	static uint16_t calibTimer = 0;
	int8_t retVal = -1;

	//New calibration cycle?
	if(calibrationNew)
	{
		calibrationNew = 0;
		calibFSMstate = 1;
		subCalibFSMstate = 0;
	}

	switch(calibFSMstate)
	{
		case 0:
			//Waiting for a start
			calibTimer = 0;
			retVal = CALIB_NOT;
			break;

		case 1:
			//Send commands to Execute
			retVal = CALIB_ONGOING;
			switch(subCalibFSMstate)
			{
				case 100:
					//Disable controller
					tx_cmd_ctrl_mode_w(TX_N_DEFAULT, CTRL_NONE);
					packAndSend(P_AND_S_DEFAULT, FLEXSEA_EXECUTE_1, ctInfo, SEND_TO_SLAVE);
					subCalibFSMstate++;
					break;
				case 600:
					//Waited long enough - start pole finding
					tx_cmd_calibration_mode_rw(TX_N_DEFAULT, CALIBRATION_FIND_POLES);
					packAndSend(P_AND_S_DEFAULT, FLEXSEA_EXECUTE_1, ctInfo, SEND_TO_SLAVE);
					subCalibFSMstate++;
					break;
				case 601:
					//Done, exiting
					calibFSMstate = 2;
					break;
				default:
					subCalibFSMstate++;
					break;
			}
			break;

		case 2:
			//Ongoing
			calibTimer++;
			retVal = CALIB_ONGOING;
			if(calibTimer > CALIB_FINDPOLES_DELAY)
			{
				calibFSMstate = 3;
			}
			break;

		case 3:
			//Done
			retVal = CALIB_DONE;
			calibrationFlags = 0;
			calibFSMstate = 0;
			subCalibFSMstate = 0;
			break;
	}

	return retVal;
}
