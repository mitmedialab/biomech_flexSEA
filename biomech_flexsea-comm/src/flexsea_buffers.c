/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'flexsea-comm' Communication stack
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
	[This file] flexsea_buffers: everything related to the reception buffers
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-09 | jfduval | Initial GPL-3.0 release
	* 2016-10-14 | jfduval | Cleaned code org, moved test to end.
****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

//****************************************************************************
// Include(s)
//****************************************************************************

#include <flexsea_buffers.h>

//****************************************************************************
// Variable(s)
//****************************************************************************

//Reception buffers - generic names:

uint8_t rx_buf_1[RX_BUF_LEN];
uint8_t rx_buf_2[RX_BUF_LEN];
uint8_t rx_buf_3[RX_BUF_LEN];
uint8_t rx_buf_4[RX_BUF_LEN];
uint8_t rx_buf_5[RX_BUF_LEN];
uint8_t rx_buf_6[RX_BUF_LEN];


//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************

//For all the buffers:
/*
static void update_rx_buf_byte(uint8_t *buf, uint32_t *idx, uint8_t new_byte);
static void update_rx_buf_array(uint8_t *buf, uint32_t *idx, \
								uint8_t *new_data, uint32_t len);
								*/

/*
//Specific:
static void update_rx_buf_1(uint8_t byte_array, uint8_t new_byte, \
							uint8_t *new_array, uint32_t len);
static void update_rx_buf_2(uint8_t byte_array, uint8_t new_byte, \
							uint8_t *new_array, uint32_t len);
static void update_rx_buf_3(uint8_t byte_array, uint8_t new_byte, \
							uint8_t *new_array, uint32_t len);
static void update_rx_buf_4(uint8_t byte_array, uint8_t new_byte, \
							uint8_t *new_array, uint32_t len);
static void update_rx_buf_5(uint8_t byte_array, uint8_t new_byte, \
							uint8_t *new_array, uint32_t len);
static void update_rx_buf_6(uint8_t byte_array, uint8_t new_byte, \
							uint8_t *new_array, uint32_t len);
*/

//****************************************************************************
// Public Function(s)
//****************************************************************************

/*
//Add one byte to buffer #1
void update_rx_buf_byte_1(uint8_t new_byte)
{
	uint8_t empty_array[1] = {0};
	update_rx_buf_1(UPDATE_BYTE, new_byte, empty_array, 0);
}

//Add an array of bytes to buffer #1
void update_rx_buf_array_1(uint8_t *new_array, uint32_t len)
{
	update_rx_buf_1(UPDATE_ARRAY, 0, new_array, len);
}

//Add one byte to buffer #2
void update_rx_buf_byte_2(uint8_t new_byte)
{
	uint8_t empty_array[1] = {0};
	update_rx_buf_2(UPDATE_BYTE, new_byte, empty_array, 0);
}

//Add an array of bytes to buffer #2
void update_rx_buf_array_2(uint8_t *new_array, uint32_t len)
{
	update_rx_buf_2(UPDATE_ARRAY, 0, new_array, len);
}

//Add one byte to buffer #3
void update_rx_buf_byte_3(uint8_t new_byte)
{
	uint8_t empty_array[1] = {0};
	update_rx_buf_3(UPDATE_BYTE, new_byte, empty_array, 0);
}

//Add an array of bytes to buffer #3
void update_rx_buf_array_3(uint8_t *new_array, uint32_t len)
{
	update_rx_buf_3(UPDATE_ARRAY, 0, new_array, len);
}

//Add one byte to buffer #4
void update_rx_buf_byte_4(uint8_t new_byte)
{
	uint8_t empty_array[1] = {0};
	update_rx_buf_4(UPDATE_BYTE, new_byte, empty_array, 0);
}

//Add an array of bytes to buffer #4
void update_rx_buf_array_4(uint8_t *new_array, uint32_t len)
{
	update_rx_buf_4(UPDATE_ARRAY, 0, new_array, len);
}

//Add one byte to buffer #5
void update_rx_buf_byte_5(uint8_t new_byte)
{
	uint8_t empty_array[1] = {0};
	update_rx_buf_5(UPDATE_BYTE, new_byte, empty_array, 0);
}

//Add an array of bytes to buffer #5
void update_rx_buf_array_5(uint8_t *new_array, uint32_t len)
{
	update_rx_buf_5(UPDATE_ARRAY, 0, new_array, len);
}

//Add one byte to buffer #6
void update_rx_buf_byte_6(uint8_t new_byte)
{
	uint8_t empty_array[1] = {0};
	update_rx_buf_6(UPDATE_BYTE, new_byte, empty_array, 0);
}

//Add an array of bytes to buffer #6
void update_rx_buf_array_6(uint8_t *new_array, uint32_t len)
{
	update_rx_buf_6(UPDATE_ARRAY, 0, new_array, len);
}
*/

#ifdef __cplusplus
}
#endif

//****************************************************************************
// Private function(s)
//****************************************************************************

/*
//Add one byte to the FIFO buffer
//Do not call that function directly, call update_rx_buf_byte_n() where n is
//your communication port/buffer name
static void update_rx_buf_byte(uint8_t *buf, uint32_t *idx, uint8_t new_byte)
{
	uint32_t i = 0;

	if((*idx) < RX_BUF_LEN)
	{
		//Buffer isn't full yet, no need to discard "old" bytes
		buf[(*idx)] = new_byte;
		(*idx)++;
	}
	else
	{
		//Shift buffer to clear one spot
		for(i = 1; i < RX_BUF_LEN; i++)
		{
			buf[i-1] = buf[i];
		}
		//Add last byte to the buffer
		buf[RX_BUF_LEN-1] = new_byte;
	}

	//buf[] is now up to date
}

//Add 'len' bytes from the 'new_data' array to the FIFO buffer
//Do not call that function directly, call update_rx_buf_array_n() where n is
//your communication port/buffer name
static void update_rx_buf_array(uint8_t *buf, uint32_t *idx, \
								uint8_t *new_data, uint32_t len)
{
	uint32_t i = 0, cnt = 0, remaining = 0;

	if((*idx) < RX_BUF_LEN)
	{
		//Buffer isn't full yet, no need to discard "old" bytes yet

		while(((*idx) + len) < RX_BUF_LEN)
		{
			//As long as we did not over fill it we keep adding bytes and
			//increasing the index
			buf[(*idx)] = new_data[cnt];
			(*idx)++;
			cnt++;	//Number of bytes we could add before shifting old data
		}

		if((cnt + 1) == len)
		{
			//All the bytes were added without requiring shifting, exit
			return;
		}
		else
		{
			//We need to shift and add 'remaining' bytes:
			remaining = len - (cnt + 1);
		}
	}
	else
	{
		//Buffer was already full when we started
		remaining = len;
	}

	if(remaining != 0)
	{
		//Shift buffer to clear 'remaining' spots
		for(i = remaining; i < (RX_BUF_LEN) ; i++)
		{
			buf[i-remaining] = buf[i];
		}

		//Then add the new data bytes at the end of the buffer:
		for(i = (RX_BUF_LEN - remaining); i < RX_BUF_LEN; i++)
		{
			buf[i] = new_data[cnt];
			cnt++;
		}
	}
}

//Wraps update_rx_buf_byte()/update_rx_buf_array() for buffer #N. Keeps track
//of the index. Do not use directly, call update_rx_buf_byte_N() or
//update_rx_buf_array_N()
static void update_rx_buf_1(uint8_t byte_array, uint8_t new_byte, \
							uint8_t *new_array, uint32_t len)
{
	static uint32_t idx_1 = 0;

	if(byte_array == UPDATE_BYTE)
	{
		//Updating buffer with one byte
		update_rx_buf_byte(rx_buf_1, &idx_1, new_byte);
	}
	else if(byte_array == UPDATE_ARRAY)
	{
		//Updating buffer with an array
		update_rx_buf_array(rx_buf_1, &idx_1, new_array, len);
	}
	else
	{
		//Error
		//flexsea_error(0);	ToDo
	}
}

//See comment above update_rx_buf_1()
static void update_rx_buf_2(uint8_t byte_array, uint8_t new_byte, \
							uint8_t *new_array, uint32_t len)
{
	static uint32_t idx_2 = 0;

	if(byte_array == UPDATE_BYTE)
	{
		//Updating buffer with one byte
		update_rx_buf_byte(rx_buf_2, &idx_2, new_byte);
	}
	else if(byte_array == UPDATE_ARRAY)
	{
		//Updating buffer with an array
		update_rx_buf_array(rx_buf_2, &idx_2, new_array, len);
	}
	else
	{
		//Error
		//flexsea_error(0);	ToDo
	}
}

//See comment above update_rx_buf_1()
static void update_rx_buf_3(uint8_t byte_array, uint8_t new_byte, \
							uint8_t *new_array, uint32_t len)
{
	static uint32_t idx_3 = 0;

	if(byte_array == UPDATE_BYTE)
	{
		//Updating buffer with one byte
		update_rx_buf_byte(rx_buf_3, &idx_3, new_byte);
	}
	else if(byte_array == UPDATE_ARRAY)
	{
		//Updating buffer with an array
		update_rx_buf_array(rx_buf_3, &idx_3, new_array, len);
	}
	else
	{
		//Error
		//flexsea_error(0);	ToDo
	}
}

//See comment above update_rx_buf_1()
static void update_rx_buf_4(uint8_t byte_array, uint8_t new_byte, \
							uint8_t *new_array, uint32_t len)
{
	static uint32_t idx_4 = 0;

	if(byte_array == UPDATE_BYTE)
	{
		//Updating buffer with one byte
		update_rx_buf_byte(rx_buf_4, &idx_4, new_byte);
	}
	else if(byte_array == UPDATE_ARRAY)
	{
		//Updating buffer with an array
		update_rx_buf_array(rx_buf_4, &idx_4, new_array, len);
	}
	else
	{
		//Error
		//flexsea_error(0);	ToDo
	}
}

//See comment above update_rx_buf_1()
static void update_rx_buf_5(uint8_t byte_array, uint8_t new_byte, \
							uint8_t *new_array, uint32_t len)
{
	static uint32_t idx_5 = 0;

	if(byte_array == UPDATE_BYTE)
	{
		//Updating buffer with one byte
		update_rx_buf_byte(rx_buf_5, &idx_5, new_byte);
	}
	else if(byte_array == UPDATE_ARRAY)
	{
		//Updating buffer with an array
		update_rx_buf_array(rx_buf_5, &idx_5, new_array, len);
	}
	else
	{
		//Error
		//flexsea_error(0);	ToDo
	}
}

//See comment above update_rx_buf_1()
static void update_rx_buf_6(uint8_t byte_array, uint8_t new_byte, \
							uint8_t *new_array, uint32_t len)
{
	static uint32_t idx_6 = 0;

	if(byte_array == UPDATE_BYTE)
	{
		//Updating buffer with one byte
		update_rx_buf_byte(rx_buf_6, &idx_6, new_byte);
	}
	else if(byte_array == UPDATE_ARRAY)
	{
		//Updating buffer with an array
		update_rx_buf_array(rx_buf_6, &idx_6, new_array, len);
	}
	else
	{
		//Error
		//flexsea_error(0);	ToDo
	}
}
*/

//****************************************************************************
// New circular buffers - partially integrated - test in progress:
//****************************************************************************

circularBuffer_t rx_buf_circ_1, rx_buf_circ_2, rx_buf_circ_3;
circularBuffer_t rx_buf_circ_4, rx_buf_circ_5, rx_buf_circ_6;
