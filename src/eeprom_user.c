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
	[This file] FlexSEA-Mn EEPROM API
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2017-06-13 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

//****************************************************************************
// Include(s)
//****************************************************************************

#include <eeprom_user.h>
#include "main.h"
#include "flexsea_system.h"
#include "flexsea_sys_def.h"
#include "flexsea.h"
#include "eeprom.h"
#include "misc.h"

//****************************************************************************
// Variable(s)
//****************************************************************************

//Cycle tester:
uint16_t cycles = 0;

//Angle map:
uint16_t angleMap[EE_ANGLE_CNT];

//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************

//****************************************************************************
// Public Function(s)
//****************************************************************************

//Note: this will also take care of the read
uint16_t increaseCycleCountEEPROM(void)
{
	//Read current value:
	uint16_t tmp = 0;
	if((EE_ReadVariable(CYCLE_COUNT_ADDR,  &tmp)) != HAL_OK)
	{
		//Error
		return 0;
	}

	//Store value in RAM for application code:
	cycles = tmp;

	//Write value + 1:
	if(EE_WriteVariable(CYCLE_COUNT_ADDR,  (tmp + 1)) != HAL_OK)
	{
		//Error
		return 0;
	}

	//Success
	cycles = tmp + 1;	//Account for the +1 just added
	return 1;
}

//Use this to read the cycle count at boot, before writing:
uint16_t readCycleCountEEPROM(uint16_t *cc)
{
	//Read current value:
	uint16_t tmp = 0;
	if((EE_ReadVariable(CYCLE_COUNT_ADDR,  &tmp)) != HAL_OK)
	{
		//Error
		(*cc) = 0;
		return 0;
	}
	else
	{
		cycles = tmp;
		(*cc) = tmp;
	}

	//Success
	return 1;
}

uint16_t resetCycleCountEEPROM(void)
{
	//Write 0:
	if(EE_WriteVariable(CYCLE_COUNT_ADDR, 0) != HAL_OK)
	{
		//Error
		return 0;
	}

	//Success
	return 1;
}

//Fills the array with zeros:
void initAngleMapRAM(void)
{
	uint8_t i = 0;

	for(i = 0; i < EE_ANGLE_CNT; i++)
	{
		angleMap[i] = 0;
	}
}

uint8_t writeAngleMapEEPROM(void)
{
	uint8_t i = 0;

	for(i = 0; i < EE_ANGLE_CNT; i++)
	{
		//Write value (index for now):
		if(EE_WriteVariable(ANGLE_MAP_ADDR + i, angleMap[i]) != HAL_OK)
		{
			//Error
			return 0;
		}

		//EE_WriteVariable is slow: prevent a Watchdog reset:
		independentWatchdog();
	}

	//Success
	return 1;
}

uint8_t readAngleMapEEPROM(void)
{
	uint8_t i = 0;
	uint16_t tmp = 0;

	for(i = 0; i < EE_ANGLE_CNT; i++)
	{
		//Write value (index for now):
		if(EE_ReadVariable(ANGLE_MAP_ADDR + i, &tmp) != HAL_OK)
		{
			//Error
			return 0;
		}
		else
		{
			angleMap[i] = tmp;
		}

		//EE_ReadVariable is slow: prevent a Watchdog reset:
		independentWatchdog();
	}

	//Success
	return 1;
}

void testAngleMapEEPROMblocking(void)
{
	volatile uint8_t ret1 = 0, ret2 = 0;

	while(1)
	{
		initAngleMapRAM();

		//ret1 = writeAngleMapEEPROM();
		//HAL_Delay(2000);
		(void)ret1;

		ret2 = readAngleMapEEPROM();

		HAL_Delay(2000);
	}
}

//****************************************************************************
// Private Function(s)
//****************************************************************************

//...
