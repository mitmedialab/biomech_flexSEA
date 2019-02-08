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
	[This file] fm_ui: Deals with the 2 green LEDs, the RGB LED and
	the switch
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-23 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

#ifndef UI_H_
#define UI_H_

//****************************************************************************
// Include(s)
//****************************************************************************

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"

//****************************************************************************
// Prototype(s):
//****************************************************************************

void init_leds(void);
void set_led_rgb(uint8_t r, uint8_t g, uint8_t b);
void rgb_led_ui(uint8_t err_l0, uint8_t err_l1, uint8_t err_l2,
		uint8_t new_comm);
void rgb_led_test_code_blocking(void);

//****************************************************************************
// Definition(s):
//****************************************************************************

#ifndef BOARD_SUBTYPE_POCKET
	#ifndef HW_BIOMECH
		#define LED0(x) HAL_GPIO_WritePin(GPIOD, 1<<12, x);
		#define LED1(x) HAL_GPIO_WritePin(GPIOD, 1<<11, x);
	#else
		#define LED0(x) HAL_GPIO_WritePin(GPIOE, 1<<1, x);
		#define LED1(x) HAL_GPIO_WritePin(GPIOE, 1<<0, x);
	#endif
#else
	#define LED0(x) HAL_GPIO_WritePin(GPIOE, 1<<12, x);
	#define LED1(x) HAL_GPIO_WritePin(GPIOE, 1<<11, x);
#endif	//BOARD_SUBTYPE_POCKET

#ifndef BOARD_SUBTYPE_POCKET
	#define LEDR(x) HAL_GPIO_WritePin(GPIOF, 1<<4, x);
	#define LEDG(x) HAL_GPIO_WritePin(GPIOF, 1<<3, x);
	#define LEDB(x) HAL_GPIO_WritePin(GPIOF, 1<<5, x);
#else
	#define LEDR(x) HAL_GPIO_WritePin(GPIOE, 1<<14, x);
	#define LEDG(x) HAL_GPIO_WritePin(GPIOE, 1<<13, x);
	#define LEDB(x) HAL_GPIO_WritePin(GPIOE, 1<<15, x);
#endif	//BOARD_SUBTYPE_POCKET

/*
 RGB LED:
 ========
 Green: everything's good
 Yellow: software error, close to a voltage or temperature limit
 Blue: didn't receive commands in the last 'comm_timeout' ms
 Red: error
 Flashing red: major error
 */

//Timings in ms:
#define UI_COMM_TIMEOUT			2000
#define UI_L0_TIMEOUT			1000
#define UI_L1_TIMEOUT			1000
#define UI_RED_FLASH_ON			100
#define UI_RED_FLASH_PERIOD		200

//global vars used for flipping LED safety lights in mainfsm.c
extern uint8_t l0, l1, l2;

#endif // UI_H_
