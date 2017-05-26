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
	[Contributors] Erin Main (ermain@mit.edu)
*****************************************************************************
	[This file] fm_i2c: i2c comms
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-23 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

#ifndef INC_FM_I2C_H
#define INC_FM_I2C_H

//****************************************************************************
// Include(s)
//****************************************************************************

#include <stdlib.h>
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_i2c.h"
#include "rigid.h"

//****************************************************************************
// Public Function Prototype(s):
//****************************************************************************

void i2c1_fsm(void);
void init_i2c1(void);
void disable_i2c1(void);
void init_i2c2(void);
void i2c2_fsm(void);
void disable_i2c2(void);
void init_i2c3(void);
void disable_i2c3(void);
void initOptionalPullUps(void);
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c);

void i2c3Receive(void);
void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c);

//****************************************************************************
// Definition(s):
//****************************************************************************

//set to 1 if we want to use interrupt driven I2C.
#define I2C1_USE_INT 			0
//#define I2C1_CLOCK_RATE 		100000 	//in Hz, corresponds to "Regular Speed" I2C
#define I2C1_CLOCK_RATE 		400000 	//in Hz, corresponds to "Full Speed" I2C
#define I2C2_CLOCK_RATE 		400000 	//in Hz, corresponds to "Full Speed" I2C
#define I2C3_CLOCK_RATE 		400000 	//in Hz, corresponds to "Full Speed" I2C


//ISR reading of I2C1 sensors (IMU, Battery, etc):
#define I2C1_RQ_GYRO			1
#define I2C1_RQ_ACCEL			2

//I2C State Machine:
#define I2C_FSM_PROBLEM			-1
#define I2C_FSM_DEFAULT			0
#define I2C_FSM_TX_ADDR		1
#define I2C_FSM_TX_ADDR_DONE	2
#define I2C_FSM_RX_DATA		3
#define I2C_FSM_RX_DATA_DONE	4

//From Regulate: ToDo move to rigidComm.c/h?

//Manage:
#define MN_WBUF_SIZE			12
#define MN_RBUF_SIZE			4
#define MN_BUF_SIZE				(MN_WBUF_SIZE + MN_RBUF_SIZE)

//Manage shared memory locations:
//(Re writes to Manage)
#define MN_W_CONTROL1			0
#define MN_W_CONTROL2			1
#define MN_W_VB_MSB				2
#define MN_W_VB_LSB				3
#define MN_W_VG_MSB				4
#define MN_W_VG_LSB				5
#define MN_W_5V_MSB				6
#define MN_W_5V_LSB				7
#define MN_W_CURRENT_MSB		8
#define MN_W_CURRENT_LSB		9
#define MN_W_TEMP				10
#define MN_W_STATUS				11
//(Re reads from Manage) //ToDo
#define MN_R_B0					0
#define MN_R_B1					1
#define MN_R_B2					2
#define MN_R_B3					3

//****************************************************************************
// Shared variable(s)
//****************************************************************************

extern I2C_HandleTypeDef hi2c1, hi2c2, hi2c3;
extern uint8_t i2c_2_r_buf[24], i2c_3_r_buf[MN_WBUF_SIZE];

extern int8_t i2c1FsmState, i2c2FsmState;
extern uint8_t i2c1_dma_rx_buf[24], i2c2_dma_rx_buf[24];


#endif //INC_FM_I2C_H
