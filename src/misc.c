/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'flexsea-manage' Mid-level computing, and networking
	Copyright (C) 2016 Dephy, Inc. <http://dephy.com/>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.	If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************
	[Lead developper] Jean-Francois (JF) Duval, jfduval at dephy dot com.
	[Origin] Based on Jean-Francois Duval's work at the MIT Media Lab
	Biomechatronics research group <http://biomech.media.mit.edu/>
	[Contributors]
*****************************************************************************
	[This file] fm_misc: when it doesn't belong in any another file, it
	ends up here...
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-23 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

//****************************************************************************
// Include(s)
//****************************************************************************

#include <adc.h>
#include <dio.h>
#include "main.h"
#include <i2c.h>
#include <imu.h>
#include <master_slave_comm.h>
#include <misc.h>
#include <spi.h>
#include <timer.h>
#include <uarts.h>
#include <ui.h>
#include "rigid.h"
#include "usb_device.h"
#include "user-mn.h"

//****************************************************************************
// Variable(s)
//****************************************************************************

//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************

//****************************************************************************
// Public Function(s)
//****************************************************************************

//Initialize all the peripherals
void init_peripherals(void)
{
	//Hardware modules:
	init_systick_timer();		//SysTick timer (1kHz)
	init_timer_6();				//For us delay function
	init_usart1(2000000);		//USART1 (RS-485 #1)
	init_usart6(2000000);		//USART6 (RS-485 #2)
	init_rs485_outputs();
	init_leds();
	initHooks();
	init_adc1();
	init_spi4();				//Plan
	initRigidIO();				//Interface with Rigid's other uC

	#ifdef USE_UART3

		init_usart3(230400);	//Expansion port

	#endif

	#ifdef USE_I2C_1

		init_i2c1();

		#ifdef USE_IMU

			init_imu();

		#endif	//USE_IMU

	#endif	//USE_I2C_1

	#ifdef USE_I2C_2

		init_i2c2();

	#endif	//USE_I2C_2

	#ifdef USE_I2C_3

		init_i2c3();

	#endif	//USE_I2C_3

	//USB
	#ifdef USE_USB

		MX_USB_DEVICE_Init();

	#endif	//USE_USB

	#ifdef USE_COMM_TEST

		init_comm_test();

	#endif	//USE_COMM_TEST

	//Software:
	initMasterSlaveComm();

	//All RGB LEDs OFF
	LEDR(0);
	LEDG(0);
	LEDB(0);
}

//Computes a bunch of stuff to maximize calculations:
long long bunchOfUselessMath(void)
{
	float resArray[MAX_I*MAX_J];
	float		 tmp1, tmp2;
	long long sum = 0;

	int i = 0, j = 0, k = 0;
	for(i = 0; i < MAX_I; i++)
	{
		for(j = 0; j < MAX_J; j++)
		{
			tmp1 = 1.37*(float)i;
			tmp2 = -0.1234*(float)j;
			resArray[MAX_I*i+j] = abs(tmp1+tmp2);
		}
	}

	sum = 0;
	for(k = 0; k < MAX_I*MAX_J; k++)
	{
		resArray[k] = resArray[k] * resArray[k];
		sum += (long long)resArray[k];
	}

	return sum;
}

void fpu_testcode_blocking(void)
{
	long long myRes = 0;
	while(1)
	{
		DEBUG_H0(1);
		myRes = bunchOfUselessMath();
		DEBUG_H0(0);
		delayUsBlocking(10);
		if(myRes != 0)
		{
			delayUsBlocking(1);
		}
		else
		{
			delayUsBlocking(10);
		}
	}
}

void test_code_blocking(void)
{
	//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	//Blocking Test code - enable one and only one for special
	//debugging. Normal code WILL NOT EXECUTE when this is enabled!
	//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	//rgb_led_test_code_blocking();
	//user_button_test_blocking();
	//imu_test_code_blocking();
	//test_delayUsBlocking_blocking();
	//fpu_testcode_blocking();
	//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
}

void test_code_non_blocking(void)
{
	//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	//Non-Blocking Test code
	//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

	//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
}

//****************************************************************************
// Private Function(s)
//****************************************************************************

//...
