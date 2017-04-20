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
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

void init_dio(void);
void initHooks(void);
void dio_set_pin_direction(unsigned int pin, unsigned int dir);
void dio_set_port_direction(unsigned int dir);
//unsigned int dio_pin_read(unsigned int pin);	//Not implemented, read port & mask
unsigned int dio_port_read(void);
void dio_pin_write(unsigned int pin, unsigned int value);
void dio_port_write(unsigned int value);
void dio_set_af(unsigned int periph);
unsigned int dio_read_port_direction(void);
void dio_map_pin_port(unsigned int pin, GPIO_TypeDef *port, uint16_t *gpio);

//****************************************************************************
// Definition(s):
//****************************************************************************

//Manage 0.1 Hardware pins:
//============================
//DIO0-1: PF0-1 (also I2C2)
//DIO2-3: PD8-9 (also UART3)
//DIO4-7: PG8/13/12/14 (also SPI6)
//DIO8 ToDo ADD DIO8!!!

#define MAX_DIO			8
#define DIO_AF_NULL		0
#define DIO_AF_USART	1
#define DIO_AF_SPI		2
#define DIO_AF_I2C		3

//Quick macros for debug outputs:
#define DEBUG_H0(x) HAL_GPIO_WritePin(GPIOD, 1<<0, x);
#define DEBUG_H1(x) HAL_GPIO_WritePin(GPIOD, 1<<1, x);
#define DEBUG_H2(x) HAL_GPIO_WritePin(GPIOD, 1<<2, x);

#endif // INC_DIO_H

