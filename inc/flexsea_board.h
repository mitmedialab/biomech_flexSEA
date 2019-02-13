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
	[This file] flexsea_board: configuration and functions for this
	particular board
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-23 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

#ifndef INC_FLEXSEA_BOARD_H
#define INC_FLEXSEA_BOARD_H

//****************************************************************************
// Include(s)
//****************************************************************************

#include <stdint.h>
#include "../flexsea-comm/inc/flexsea_comm.h"

//****************************************************************************
// Prototype(s):
//****************************************************************************

void flexsea_send_serial_slave(PacketWrapper* p);
void flexsea_send_serial_master(PacketWrapper* p);
void flexsea_receive_from_master(void);
void flexsea_start_receiving_from_master(void);
void flexsea_receive_from_slave(void);
uint8_t getBoardID(void);
uint8_t getBoardUpID(void);
uint8_t getBoardSubID(uint8_t sub, uint8_t idx);
uint8_t getSlaveCnt(uint8_t sub);

uint8_t getDeviceId();
uint8_t getDeviceType();

//****************************************************************************
// Definition(s):
//****************************************************************************

//<FlexSEA User>
//==============

//How many slave busses?
#define COMM_SLAVE_BUS				3

//How many slaves per bus?
#define SLAVE_BUS_1_CNT				2
#define SLAVE_BUS_2_CNT				2
#define SLAVE_BUS_3_CNT				1
//Note: only Manage can have a value different than 0 or 1

//How many possible masters?
#define COMM_MASTERS				3

//Overload buffer & function names (for user convenience):

#define update_rx_buf_485_1(x, y)		circ_buff_write(commPeriph[PORT_RS485_1].rx.circularBuff, (x), (y))
#define update_rx_buf_uart(x, y)		circ_buff_write(commPeriph[PORT_SUB2].rx.circularBuff, (x), (y))

#define update_rx_buf_spi(x, y)			circ_buff_write(commPeriph[PORT_SPI].rx.circularBuff, (x), (y))
#define update_rx_buf_usb(x, y)			circ_buff_write(commPeriph[PORT_USB].rx.circularBuff, (x), (y))
#define update_rx_buf_wireless(x, y)	circ_buff_write(commPeriph[PORT_WIRELESS].rx.circularBuff, (x), (y))
#define update_rx_buf_exp(x, y)			circ_buff_write(commPeriph[PORT_EXP].rx.circularBuff, (x), (y))

#define PORT_UART_EX					PORT_RS485_2

//===============
//</FlexSEA User>

//****************************************************************************
// Structure(s):
//****************************************************************************

//****************************************************************************
// Shared variable(s)
//****************************************************************************

extern PortType masterSlave[NUMBER_OF_PORTS];

#endif	//INC_FLEXSEA_BOARD_H

