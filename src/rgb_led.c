/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'flexsea-manage' Mid-level computing, and networking
	Copyright (C) 2017 Dephy, Inc. <http://dephy.com/>
*****************************************************************************
	[Lead developper] Jean-Francois Duval, jfduval at dephy dot com.
	[Origin] Based on Jean-Francois Duval's work at the MIT Media Lab
	Biomechatronics research group <http://biomech.media.mit.edu/>
	[Contributors]
*****************************************************************************
	[This file] RGB LED: Onboard LED Driver that supports fading
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-10-05 | jfduval | Copied from UI to make it more portable
	*
****************************************************************************/

//****************************************************************************
// Include(s)
//****************************************************************************

#include <ui.h>
#include "main.h"
#include "rgb_led.h"

//****************************************************************************
// Variable(s)
//****************************************************************************

uint8_t rgbFade = 0;

//RGB LED:
uint8_t rgbPeriodR = 0, rgbPeriodG = 0, rgbPeriodB = 0;

//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************


//****************************************************************************
// Public Function(s)
//****************************************************************************

//Use this to set a new value
void rgbLedSet(uint8_t r, uint8_t g, uint8_t b)
{
	rgbPeriodR = r;
	rgbPeriodG = g;
	rgbPeriodB = b;
}

//Timer-based RGB driver - w/ fading.
//Call this function at 10kHz
void rgbLedRefresh(void)
{
	static uint8_t cnt = 0;
	static uint8_t rON = 0, gON = 0, bON = 0;

	//New cycle?
	if(!cnt)
	{
		//All ON
		LEDR(1);
		LEDG(1);
		LEDB(1);
		rON = 1;
		gON = 1;
		bON = 1;
	}

	//Ready to turn OFF?

	if(rON && cnt >= rgbPeriodR)
	{
		LEDR(0);
		rON = 0;
	}

	if(gON && cnt >= rgbPeriodG)
	{
		LEDG(0);
		gON = 0;
	}

	if(bON && cnt >= rgbPeriodB)
	{
		LEDB(0);
		bON = 0;
	}

	//Increment counter. It will eventually roll over.
	cnt += 2;
}

//Accessor
uint8_t rgbLedGetFade(void)
{
	return rgbFade;
}

//Outputs 1 around the mid point
uint8_t rgbLedCenterPulse(uint8_t halfWidth)
{
	if (rgbFade > (250-halfWidth))
		return 1;
	else
		return 0;
}

//Call this function every ms. It will update the rgbFade variable.
void rgbLedRefreshFade(void)
{
	static uint16_t fade = 0, val = 0;

	val++;
	val %= FADE_PERIOD_MS;

	if(val > FADE_MIDPOINT-2)
		fade = FADE_PERIOD_MS - val;
	else
		fade = val;

	rgbFade = (uint8_t) (fade>>1 & 0xFF);
}

//Test code
void rgbLedRefresh_testcode_blocking(void)
{
	uint8_t div = 0;

	while(1)
	{
		rgbLedSet(0, rgbFade, 0);
		HAL_Delay(100);

		div++;
		div %= 10;
		if(!div)
		{
			//1ms
			rgbLedRefreshFade();
		}
	}
}

//****************************************************************************
// Private Function(s)
//****************************************************************************
