/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'flexsea-manage' Mid-level computing, and networking
	Copyright (C) 2017 Dephy, Inc. <http://dephy.com/>
*****************************************************************************
	[Lead developer] Jean-Francois (JF) Duval, jfduval at dephy dot com.
	[Origin] Based on Jean-Francois Duval's work at the MIT Media Lab
	Biomechatronics research group <http://biomech.media.mit.edu/>
	[Contributors]
*****************************************************************************
	[This file] fm_ui: Deals with the 2 green LEDs, the RGB LED and
	the switch
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-23 | jfduval | Initial GPL-3.0 release
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

//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************

static void initGreenLeds(void);
static void initRgbLed(void);

//****************************************************************************
// Public Function(s)
//****************************************************************************

void init_leds(void)
{
	initGreenLeds();
	initRgbLed();
}

void set_led_rgb(uint8_t r, uint8_t g, uint8_t b)
{
	//No fading, we use 1 or 0 for now.

	LEDR(r & 0x01);
	LEDG(g & 0x01);
	LEDB(b & 0x01);
}

//Call this function every ms in main while()
void rgb_led_ui(uint8_t err_l0, uint8_t err_l1, uint8_t err_l2,
		uint8_t new_comm)
{
	static uint32_t cnt_comm = UI_COMM_TIMEOUT, cnt_flash = 0;
	static uint8_t latch_err_l2 = 0, flash_red = 0, comm_blue = 0;
	uint8_t r = 0, g = 0, b = 0;
	int8_t rgbStatus = 0;

	//Set variable for the flashing red light:
	if(cnt_flash < UI_RED_FLASH_ON)
	{
		flash_red = 1;
	}
	else
	{
		flash_red = 0;
	}

	cnt_flash++;
	if(cnt_flash >= UI_RED_FLASH_PERIOD)
	{
		cnt_flash = 0;
	}

	//New communication green/blue:
	if(new_comm)
	{
		//Received a new valid packet, resets the counter
		cnt_comm = UI_COMM_TIMEOUT;
	}

	if(cnt_comm > 0)
	{
		cnt_comm--;
	}

	if(!cnt_comm)
	{
		comm_blue = 1;
	}
	else
	{
		comm_blue = 0;
	}

	//From the highest priority to the lowest:
	//=======================================

	rgbStatus = -1;

	if((err_l2 == 1) || (latch_err_l2 == 1))
	{
		//Major error => flashing red

		latch_err_l2 = 1;	//Latching it, will require a reset to be normal again
		r = flash_red;
		g = 0;
		b = 0;
	}
	else
	{
		if(err_l1 == 1)
		{
			//Intermediate error => Steady Red
			r = 1;
			g = 0;
			b = 0;
		}
		else
		{
			if(err_l0 == 1)
			{
				//Software error => Steady Yellow
				r = 1;
				g = 1;
				b = 0;
			}
			else
			{
				if(comm_blue == 1)
				{
					//Communication timeout
					r = 0;
					g = 0;
					b = 1;
					rgbStatus = 0;
				}
				else
				{
					//Normal, green
					r = 0;
					g = 1;
					b = 0;
					rgbStatus = 1;
				}
			}
		}
	}

	//Use the Fading code.
	if(rgbStatus == 0)
	{
		rgbLedSet(0,0,rgbLedGetFade());
	}
	else if(rgbStatus == 1)
	{
		rgbLedSet(0,rgbLedGetFade(),0);
	}
	else
	{
		//Legacy code, used for all the errors
		rgbLedSet(255*r, 255*g, 255*b);
	}
}

//****************************************************************************
// Test Code
//****************************************************************************

//Cycles through all the colors
void rgb_led_test_code_blocking(void)
{
	uint8_t r = 0, g = 0, b = 0;
	uint8_t cnt = 0;
	unsigned long delay = 0;

	while(1)
	{
		cnt++;
		cnt %= 8;

		r = cnt & 0x01;
		g = (cnt & 0x02) >> 1;
		b = (cnt & 0x04) >> 2;

		set_led_rgb(r, g, b);

		//Waste some time:
		for(delay = 0; delay < 75000000; delay++);
	}
}

//****************************************************************************
// Private Function(s)
//****************************************************************************

static void initGreenLeds(void)
{
	#ifndef BOARD_SUBTYPE_POCKET

	#ifndef HW_BIOMECH

	//LED0 - D12, LED1 - D11
	GPIO_InitTypeDef LED_InitStructure;

	// Enable GPIO Peripheral clock
	__GPIOD_CLK_ENABLE();

	// Configure pin in output push/pull mode
	LED_InitStructure.Pin = GPIO_PIN_11 | GPIO_PIN_12;
	LED_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	LED_InitStructure.Speed = GPIO_SPEED_LOW;
	LED_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOD, &LED_InitStructure);

	#else

	//LED0 - PE1, LED1 - PE0
	GPIO_InitTypeDef LED_InitStructure;

	// Enable GPIO Peripheral clock
	__GPIOE_CLK_ENABLE();

	// Configure pin in output push/pull mode
	LED_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1;
	LED_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	LED_InitStructure.Speed = GPIO_SPEED_LOW;
	LED_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOE, &LED_InitStructure);

	#endif

	#else

	//LED0 - PE12, LED1 - PE11
	GPIO_InitTypeDef LED_InitStructure;

	// Enable GPIO Peripheral clock
	__GPIOE_CLK_ENABLE();

	// Configure pin in output push/pull mode
	LED_InitStructure.Pin = GPIO_PIN_11 | GPIO_PIN_12;
	LED_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	LED_InitStructure.Speed = GPIO_SPEED_LOW;
	LED_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOE, &LED_InitStructure);

	#endif	//BOARD_SUBTYPE_POCKET
}

static void initRgbLed(void)
{
	GPIO_InitTypeDef LED_InitStructure;

	#ifndef BOARD_SUBTYPE_POCKET

	//LEDR - F4, LEDG - F3, LEDB - F5

	// Enable GPIO Peripheral clock on port C & F
	__GPIOF_CLK_ENABLE();

	// Configure pin in output push/pull mode
	LED_InitStructure.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
	LED_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	LED_InitStructure.Speed = GPIO_SPEED_LOW;
	LED_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOF, &LED_InitStructure);

	#else

	//LEDR - E14, LEDG - E13, LEDB - E15

	// Enable GPIO Peripheral clock on port C & F
	__GPIOE_CLK_ENABLE();

	// Configure pin in output push/pull mode
	LED_InitStructure.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	LED_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	LED_InitStructure.Speed = GPIO_SPEED_LOW;
	LED_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOE, &LED_InitStructure);


	#endif	//BOARD_SUBTYPE_POCKET
}
