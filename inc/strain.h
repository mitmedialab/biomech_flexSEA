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
	[This file] strain: external 6-ch Strain Amp
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2017-09-19 | jfduval | Initial release
	*
****************************************************************************/

#include "user-mn.h"
#ifdef USE_6CH_AMP

#ifndef INC_STRAINGAUGE_H
#define INC_STRAINGAUGE_H

//****************************************************************************
// Include(s)
//****************************************************************************

#include "main.h"
#include "flexsea_global_structs.h"

//****************************************************************************
// Shared variable(s)
//****************************************************************************

//External:
extern uint16_t ext_strain[6];

//****************************************************************************
// Prototype(s):
//****************************************************************************

int strain_6ch_read(uint8_t internal_reg_addr, uint8_t *pData, uint16_t length);
void strain_amp_6ch_test_code_blocking(void);
void strain_6ch_bytes_to_words(uint8_t *buf);
void get_6ch_strain(void);
uint8_t compressAndSplit6ch(uint8_t *buf, uint16_t ch0, uint16_t ch1, uint16_t ch2, \
							uint16_t ch3, uint16_t ch4, uint16_t ch5);
void unpackCompressed6ch(uint8_t *buf, uint16_t *v0, uint16_t *v1, uint16_t *v2, \
							uint16_t *v3, uint16_t *v4, uint16_t *v5);
void compress6chTestCodeBlocking(void);
void decode6chAmp(void);

//****************************************************************************
// Definition(s):
//****************************************************************************

//6-ch Strain Amplifier:
#define I2C_SLAVE_ADDR_6CH		0x66	//I'm assuming this is 7bits
#define MEM_R_CH1_H				8		//First Read address of the EZI2C mem

//****************************************************************************
// Structure(s):
//****************************************************************************

#endif	//INC_STRAINGAUGE_H
#endif 	//USE_6CH_AMP
