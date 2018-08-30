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
#include "eeprom.h"
#include "usb_device.h"
#include "user-mn.h"
#include "eeprom.h"
#include <math.h>
#include "flexsea_user_structs.h"
#ifdef INCLUDE_UPROJ_SVM
#include "svm.h"
#endif
#include "arm_math.h"

#ifdef USE_6CH_AMP
#include "strain.h"
#endif	//USE_6CH_AMP

//****************************************************************************
// Variable(s)
//****************************************************************************

uint32_t causeOfLastReset = 0;
IWDG_HandleTypeDef hiwdg;

//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************

//****************************************************************************
// Public Function(s)
//****************************************************************************

//Initialize all the peripherals
void init_peripherals(void)
{
	SystemCoreClockUpdate();

	//Hardware modules:
	init_systick_timer();		//SysTick timer (1kHz)
	init_timer_6();				//For us delay function
	init_usart1(2000000);		//USART1 (RS-485 #1)
	init_usart6(2000000);		//USART6 (RS-485 #2)
	init_rs485_outputs();
	init_leds();
	initHooks();
	init_adc1();

	#ifdef USE_SPI_PLAN

		init_spi4();			//Plan

	#endif //USE_SPI_PLAN

	initRigidIO();				//Interface with Rigid's other uC

	#ifdef USE_UART3

		init_usart3(230400);	//Expansion port
		BT_RST(0);

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

	#ifdef USE_EEPROM

		//Unlock the Flash Program Erase controller
		HAL_FLASH_Unlock();

		//EEPROM Init
		if(EE_Init() != EE_OK)
		{
			HAL_Delay(1);		//ToDo remove
		}

	#endif	//USE_EEPROM


	//Software:
	initMasterSlaveComm();

	//We start I2C3 in Transmit mode to send limits to Re:
	setRegulateLimits(19000, i2tBatt);
	i2c3SlaveTransmitToMaster();

	//All RGB LEDs OFF
	LEDR(0);
	LEDG(0);
	LEDB(0);

	resetBluetooth();
}

void init_iwdg(void)
{
	hiwdg.Instance = IWDG;
	hiwdg.Init.Prescaler = IWDG_PRESCALER_4;
	hiwdg.Init.Reload = IWDG_RELOAD;

	if(HAL_IWDG_Init(&hiwdg) != HAL_OK)
	{
		//ToDo...
	}
}

void independentWatchdog(void)
{
	#ifdef USE_WATCHDOG

	static uint8_t firstTime = 1;
	if(firstTime)
	{
		init_iwdg();
		HAL_IWDG_Refresh(&hiwdg);
	}
	else
	{
		//Refresh watchdog to avoid a reset:
		HAL_IWDG_Refresh(&hiwdg);
	}

	#endif	//USE_INDEPEDENT_WATCHDOG
}

//We receive flags from Re and Ex. This combines them.
void combineStatusFlags(void)
{
	//Start by clearing them:
	rigid1.re.status &= ~ STATUS_MOT_CURRENT_WARN;
	rigid1.re.status &= ~ STATUS_MOT_CURRENT_LIM;
	rigid1.re.status |= ((rigid1.ex.status & 0x01) << 5);
	rigid1.re.status |= ((rigid1.ex.status & 0x02) << 5);
}

void saveCauseOFLastReset(void)
{
	//Save it:
	causeOfLastReset = RCC->CSR & 0xFE000000;

	//Clear flags:
	RCC->CSR &= ~RCC_CSR_RMVF;
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

#define MAX_BLOCKSIZE     32
#define DELTA           (0.000001f)
float32_t srcA_buf_f32[MAX_BLOCKSIZE] =
{
  -0.4325648115282207,  -1.6655843782380970,  0.1253323064748307,
   0.2876764203585489,  -1.1464713506814637,  1.1909154656429988,
   1.1891642016521031,  -0.0376332765933176,  0.3272923614086541,
   0.1746391428209245,  -0.1867085776814394,  0.7257905482933027,
  -0.5883165430141887,   2.1831858181971011, -0.1363958830865957,
   0.1139313135208096,   1.0667682113591888,  0.0592814605236053,
  -0.0956484054836690,  -0.8323494636500225,  0.2944108163926404,
  -1.3361818579378040,   0.7143245518189522,  1.6235620644462707,
  -0.6917757017022868,   0.8579966728282626,  1.2540014216025324,
  -1.5937295764474768,  -1.4409644319010200,  0.5711476236581780,
  -0.3998855777153632,   0.6899973754643451
};

/* ----------------------------------------------------------------------
** Test input data of srcB for blockSize 32
** ------------------------------------------------------------------- */
float32_t srcB_buf_f32[MAX_BLOCKSIZE] =
{
   1.7491401329284098,  0.1325982188803279,   0.3252281811989881,
  -0.7938091410349637,  0.3149236145048914,  -0.5272704888029532,
   0.9322666565031119,  1.1646643544607362,  -2.0456694357357357,
  -0.6443728590041911,  1.7410657940825480,   0.4867684246821860,
   1.0488288293660140,  1.4885752747099299,   1.2705014969484090,
  -1.8561241921210170,  2.1343209047321410,  1.4358467535865909,
  -0.9173023332875400, -1.1060770780029008,   0.8105708062681296,
   0.6985430696369063, -0.4015827425012831,   1.2687512030669628,
  -0.7836083053674872,  0.2132664971465569,   0.7878984786088954,
   0.8966819356782295, -0.1869172943544062,   1.0131816724341454,
   0.2484350696132857,  0.0596083377937976
};

void fpu_testcode_blocking(void)
{
	float32_t multOutput[MAX_BLOCKSIZE];  /* Intermediate output */
	float32_t dotProdResult = 0.0;
	volatile float32_t res = 0.0;
	long long myRes = 0;
	while(1)
	{
		//DEBUG_H0(1);
		myRes = bunchOfUselessMath();
		//DEBUG_H0(0);
		delayUsBlocking(10);

		if(myRes != 0)
		{
			delayUsBlocking(1);
		}
		else
		{
			delayUsBlocking(10);
		}

		//Testing the ARM math functions:
		arm_mult_f32(srcA_buf_f32, srcB_buf_f32, multOutput, MAX_BLOCKSIZE);
		arm_dot_prod_f32(srcA_buf_f32, srcB_buf_f32, MAX_BLOCKSIZE, &dotProdResult);
		res = dotProdResult;
	}
}

void test_iwdg_blocking(void)
{
	uint16_t cnt = 0;

	LEDR(1);
	LEDG(1);

	//The IWDG should break that loop...
	while(1)
	{
		//This shouldn't cause a reset:
		for(cnt = 0; cnt < 300; cnt++)
		{
			//Refresh watchdog to avoid a reset:
			HAL_IWDG_Refresh(&hiwdg);
			HAL_Delay(10);
		}

		//This should:
		for(cnt = 0; cnt < 10; cnt++)
		{
			//Refresh watchdog to avoid a reset:
			HAL_IWDG_Refresh(&hiwdg);
			HAL_Delay(50);
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
	//eeprom_test_code_blocking_1();
	//testAngleMapEEPROMblocking();
	//test_iwdg_blocking();
	//test_svm_blocking();
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
