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
	*
****************************************************************************/

#ifndef INC_FX_BUF_H
#define INC_FX_BUF_H

#ifdef __cplusplus
extern "C" {
#endif

//****************************************************************************
// Include(s)
//****************************************************************************

#include "flexsea.h"
#include <flexsea_circular_buffer.h>

//****************************************************************************
// Shared variable(s)
//****************************************************************************

extern uint8_t rx_buf_1[RX_BUF_LEN];
extern uint8_t rx_buf_2[RX_BUF_LEN];
extern uint8_t rx_buf_3[RX_BUF_LEN];
extern uint8_t rx_buf_4[RX_BUF_LEN];
extern uint8_t rx_buf_5[RX_BUF_LEN];
extern uint8_t rx_buf_6[RX_BUF_LEN];

//****************************************************************************
// New code - not integrated - test in progress:
//****************************************************************************

extern circularBuffer_t rx_buf_circ_1, rx_buf_circ_2, rx_buf_circ_3;
extern circularBuffer_t rx_buf_circ_4, rx_buf_circ_5, rx_buf_circ_6, rx_buf_circ_7;

#ifdef __cplusplus
}
#endif

#endif
