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
#include "eeprom_user.h"
#include "misc.h"
#include "rigid.h"

//****************************************************************************
// Variable(s)
//****************************************************************************

//Cycle tester:
uint16_t cycles = 0;

//Angle map:
uint16_t angleMap[EE_ANGLE_CNT];

//UVLO:
uint16_t nvUVLO = 0;

//I2t:
uint8_t validEepromI2t = 0;
struct i2t_s eepromI2t;

//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************

static void testFillAngleMapRAM(void);

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

		#ifdef USE_WATCHDOG
		//EE_WriteVariable is slow: prevent a Watchdog reset:
		independentWatchdog();
		#endif
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

		#ifdef USE_WATCHDOG
		//EE_ReadVariable is slow: prevent a Watchdog reset:
		independentWatchdog();
		#endif
	}

	//Success
	return 1;
}

uint8_t writeUvloEEPROM(uint16_t newValue)
{
	//Write value:
	if(EE_WriteVariable(NV_UVLO_ADDR, newValue) != HAL_OK)
	{
		//Error
		return 0;
	}
	//Write lock key:
	if(EE_WriteVariable(NV_UVLO_ADDR + 1, NV_UVLO_LOCK) != HAL_OK)
	{
		//Error
		return 0;
	}

	#ifdef USE_WATCHDOG
	//EE_WriteVariable is slow: prevent a Watchdog reset:
	independentWatchdog();
	#endif

	//Success
	return 1;
}

uint8_t readUvloEEPROM(void)
{
	uint16_t tmpKey = 0, tmpVal = 0;

	//Read value:
	if(EE_ReadVariable(NV_UVLO_ADDR, &tmpVal) != HAL_OK)
	{
		//Error
		return 0;
	}
	//Read lock key:
	if(EE_ReadVariable(NV_UVLO_ADDR + 1, &tmpKey) != HAL_OK)
	{
		//Error
		return 0;
	}

	if(tmpKey == NV_UVLO_LOCK)
	{
		//Valid number was read
		nvUVLO = tmpVal;
	}
	else
	{
		nvUVLO = 0;
		return 2;
	}

	#ifdef USE_WATCHDOG
	//EE_ReadVariable is slow: prevent a Watchdog reset:
	independentWatchdog();
	#endif

	//Success
	return 1;
}

uint16_t getNvUVLO(void)
{
	return nvUVLO;
}

uint8_t writeI2tEEPROM(struct i2t_s i)
{
	//From struct to uint16 array:
	uint16_t i2tWords[5] = {0,0,0,0,0};
	uint8_t idx = 0;
	packI2t(i, i2tWords, &idx);

	//Write values:
	uint16_t addr = NV_I2T_ADDR;
	idx = 0;
	if(EE_WriteVariable(addr++, i2tWords[idx++]) != HAL_OK){return 0;}
	if(EE_WriteVariable(addr++, i2tWords[idx++]) != HAL_OK){return 0;}
	if(EE_WriteVariable(addr++, i2tWords[idx++]) != HAL_OK){return 0;}
	if(EE_WriteVariable(addr++, i2tWords[idx++]) != HAL_OK){return 0;}
	if(EE_WriteVariable(addr++, i2tWords[idx++]) != HAL_OK){return 0;}

	//Write lock key:
	if(EE_WriteVariable(addr++, NV_I2T_LOCK) != HAL_OK){return 0;}

	#ifdef USE_WATCHDOG
	//EE_WriteVariable is slow: prevent a Watchdog reset:
	independentWatchdog();
	#endif

	//Success
	return 1;
}

uint8_t readI2tEEPROM(void)
{
	uint16_t tmpKey = 0;
	uint16_t i2tWords[5] = {0,0,0,0,0};
	uint8_t idx = 0;

	//Read values:
	uint16_t addr = NV_I2T_ADDR;
	if(EE_ReadVariable(addr++, &i2tWords[idx++]) != HAL_OK){return 0;}
	if(EE_ReadVariable(addr++, &i2tWords[idx++]) != HAL_OK){return 0;}
	if(EE_ReadVariable(addr++, &i2tWords[idx++]) != HAL_OK){return 0;}
	if(EE_ReadVariable(addr++, &i2tWords[idx++]) != HAL_OK){return 0;}
	if(EE_ReadVariable(addr++, &i2tWords[idx++]) != HAL_OK){return 0;}

	//Read lock key:
	if(EE_ReadVariable(addr++, &tmpKey) != HAL_OK){return 0;}

	if(tmpKey == NV_I2T_LOCK)
	{
		//Valid data was read
		idx = 0;
		validEepromI2t = 1;
		unpackI2t(&eepromI2t, i2tWords, &idx);
	}
	else
	{
		validEepromI2t = 0;
		return 2;
	}

	#ifdef USE_WATCHDOG
	//EE_ReadVariable is slow: prevent a Watchdog reset:
	independentWatchdog();
	#endif

	//Success
	return 1;
}

uint8_t getNvI2t(struct i2t_s *v)
{
	(*v) = eepromI2t;
	return validEepromI2t;
}

void testAngleMapEEPROMblocking(void)
{
	uint8_t ret1 = 0, ret2 = 0;

	while(1)
	{
		ret2 = readAngleMapEEPROM();
		HAL_Delay(500);

		testFillAngleMapRAM();
		HAL_Delay(500);

		ret1 = writeAngleMapEEPROM();
		HAL_Delay(2000);
		//(void)ret1;

		ret2 = readAngleMapEEPROM();

		HAL_Delay(2000);
	}

	(void) ret1;
	(void) ret2;
}

//****************************************************************************
// Private Function(s)
//****************************************************************************

//Fills the array with incrementing values
static void testFillAngleMapRAM(void)
{
	uint8_t i = 0;
	static uint16_t val = 0;

	for(i = 0; i < EE_ANGLE_CNT; i++)
	{
		angleMap[i] = val++;
	}
}
