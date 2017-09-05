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

//****************************************************************************
// Definition(s):
//****************************************************************************

//<FlexSEA User>
//==============

//Board type: define as a global symbol. List of options:
//(and make sure it matches with board_id!)
//#define BOARD_TYPE_FLEXSEA_PLAN
//#define BOARD_TYPE_FLEXSEA_MANAGE
//#define BOARD_TYPE_FLEXSEA_EXECUTE

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

//#define comm_str_485_1					comm_str_1
//#define unpack_payload_485_1			unpack_payload_1
//#define rx_command_485_1				rx_command_1
//#define update_rx_buf_byte_485_1		update_rx_buf_byte_1
//#define update_rx_buf_array_485_1		update_rx_buf_array_1
#define update_rx_buf_485_1(x, y)		circ_buff_write(commPeriph[PORT_RS485_1].rx.circularBuff, (x), (y))

//#define comm_str_uart					comm_str_2
//#define unpack_payload_uart				unpack_payload_2
//#define rx_command_uart					rx_command_2
//#define update_rx_buf_byte_uart			update_rx_buf_byte_2
//#define update_rx_buf_array_uart		update_rx_buf_array_2
#define update_rx_buf_uart(x, y)		circ_buff_write(commPeriph[PORT_SUB2].rx.circularBuff, (x), (y))

//comm_str_spi is the only one used. Replace by something uniue to this peripheral
#define comm_str_spi					comm_str_3
//#define unpack_payload_spi				unpack_payload_3
#define rx_command_spi					rx_command_3
//#define update_rx_buf_byte_spi			update_rx_buf_byte_3
//#define update_rx_buf_array_spi			update_rx_buf_array_3
#define update_rx_buf_spi(x, y)			circ_buff_write(commPeriph[PORT_SPI].rx.circularBuff, (x), (y))

//#define comm_str_usb					comm_str_4
//#define unpack_payload_usb				unpack_payload_4
//#define rx_command_usb					rx_command_4
//#define update_rx_buf_byte_usb			update_rx_buf_byte_4
//#define update_rx_buf_array_usb			update_rx_buf_array_4
#define update_rx_buf_usb(x, y)			circ_buff_write(commPeriph[PORT_USB].rx.circularBuff, (x), (y))

//#define comm_str_wireless				comm_str_5
//#define unpack_payload_wireless			unpack_payload_5
//#define rx_command_wireless				rx_command_5
//#define update_rx_buf_byte_wireless		update_rx_buf_byte_5
//#define update_rx_buf_array_wireless	update_rx_buf_array_5
#define update_rx_buf_wireless(x, y)	circ_buff_write(commPeriph[PORT_WIRELESS].rx.circularBuff, (x), (y))

//#define comm_str_exp					comm_str_6
//#define unpack_payload_exp				unpack_payload_6
//#define rx_command_exp					rx_command_6
//#define update_rx_buf_byte_exp			update_rx_buf_byte_6
//#define update_rx_buf_array_exp			update_rx_buf_array_6
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

#endif	//INC_FLEXSEA_BOARD_H

