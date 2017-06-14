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
	[This file] Cycle Tester EEPROM API
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2017-06-13 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

//****************************************************************************
// Include(s)
//****************************************************************************

#include "main.h"
#include "cycle.h"
#include "flexsea_system.h"
#include "flexsea_sys_def.h"
#include "flexsea.h"
#include "eeprom.h"

//****************************************************************************
// Variable(s)
//****************************************************************************

//User code:
uint16_t cycles = 0;

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

//****************************************************************************
// Private Function(s)
//****************************************************************************

//...
