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

#ifndef INC_DIO_H
#define INC_DIO_H

//****************************************************************************
// Include(s)
//****************************************************************************

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"

//****************************************************************************
// Shared variable(s)
//****************************************************************************

//...

//****************************************************************************
// Public Function Prototype(s):
//****************************************************************************

void initHooks(void);

//****************************************************************************
// Definition(s):
//****************************************************************************

//Quick macros for debug outputs:
#ifndef BOARD_SUBTYPE_POCKET
#define DEBUG_H0(x) HAL_GPIO_WritePin(GPIOD, 1<<0, x);
#define DEBUG_H1(x) HAL_GPIO_WritePin(GPIOD, 1<<1, x);
#define DEBUG_H2(x) HAL_GPIO_WritePin(GPIOD, 1<<2, x);
#else
#define DEBUG_H0(x) HAL_GPIO_WritePin(GPIOB, 1<<0, x);
#define DEBUG_H1(x) HAL_GPIO_WritePin(GPIOB, 1<<1, x);
#define DEBUG_H2(x) HAL_GPIO_WritePin(GPIOB, 1<<2, x);
#endif	//BOARD_SUBTYPE_POCKET

#endif // INC_DIO_H

