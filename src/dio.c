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
	[This file] fm_dio: Deals with the 9 digital IOs
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-23 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

//****************************************************************************
// Include(s)
//****************************************************************************

#include <dio.h>
#include <spi.h>
#include <timer.h>
#include "main.h"
#include "flexsea.h"
#include "flexsea_sys_def.h"
#include "stm32_hal_legacy.h"
#include "rigid.h"

//****************************************************************************
// Variable(s)
//****************************************************************************


//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************

//...

//****************************************************************************
// Function(s)
//****************************************************************************

void initHooks(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	#ifndef BOARD_SUBTYPE_POCKET

	// Enable GPIO Peripheral clocks
	__GPIOD_CLK_ENABLE();

	//Configure pins as outputs
	GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

	#else

	// Enable GPIO Peripheral clocks
	__GPIOB_CLK_ENABLE();

	//Configure pins as outputs
	GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

	#endif	//BOARD_SUBTYPE_POCKET
}

//Pin interrupt on change callback:
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == GPIO_PIN_4)
	{
		//SPI NSS pin:
		SPI_NSS_Callback();
	}
	#ifdef BOARD_SUBTYPE_RIGID
		#ifndef BOARD_SUBTYPE_POCKET
		else if(GPIO_Pin == GPIO_PIN_15)
		#else
		else if(GPIO_Pin == GPIO_PIN_8)
		#endif	//BOARD_SUBTYPE_POCKET
		{
			//SYNC:
			timebases();

			//FSM monitoring. Previous FSM should have finished by this time, if not flag it.
			if(activeFSM != FSMS_INACTIVE)
			{
				//Report an error!
				if(timingError[activeFSM] < MAX_TIMING_ERR)
				{
					timingError[activeFSM]++;
				}
			}
		}
	#endif	//BOARD_SUBTYPE_RIGID
}
