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

//****************************************************************************
// Include(s)
//****************************************************************************

#include "main.h"
#include "i2c.h"
#include "strain.h"
#include "flexsea_global_structs.h"
#include "flexsea.h"

//****************************************************************************
// Variable(s)
//****************************************************************************

//External 6-ch Strain Amplifier:
uint16_t ext_strain[6] = {0,0,0,0,0,0};
uint8_t ext_strain_bytes[12];

//****************************************************************************
// Function(s)
//****************************************************************************

//Reassembles the bytes we read in words
void strain_6ch_bytes_to_words(uint8_t *buf)
{
	ext_strain[0] = ((((uint16_t)buf[0] << 8) & 0xFF00) | (uint16_t)buf[1]);
	ext_strain[1] = ((((uint16_t)buf[2] << 8) & 0xFF00) | (uint16_t)buf[3]);
	ext_strain[2] = ((((uint16_t)buf[4] << 8) & 0xFF00) | (uint16_t)buf[5]);
	ext_strain[3] = ((((uint16_t)buf[6] << 8) & 0xFF00) | (uint16_t)buf[7]);
	ext_strain[4] = ((((uint16_t)buf[8] << 8) & 0xFF00) | (uint16_t)buf[9]);
	ext_strain[5] = ((((uint16_t)buf[10] << 8) & 0xFF00) | (uint16_t)buf[11]);
}

//Get latest readings from the 6-ch strain sensor. Uses the Compressed version,
//9 bytes, 12-bits per sensor.
void get_6ch_strain(void) 
{
	HAL_StatusTypeDef retVal;

	retVal = HAL_I2C_Mem_Read_DMA(&hi2c2, I2C_SLAVE_ADDR_6CH, (uint16_t) MEM_R_CH1_H,
								I2C_MEMADD_SIZE_8BIT, i2c2_dma_rx_buf, 9);

	if(retVal == HAL_OK){i2c2FsmState = I2C_FSM_RX_DATA;}
	else{i2c2FsmState = I2C_FSM_PROBLEM;}
}

//Compress 6x uint16_t to 9 bytes (12bits per sensor).
//Needed to send all data with RIC/NU Read All function
uint8_t compressAndSplit6ch(uint8_t *buf, uint16_t ch0, uint16_t ch1, uint16_t ch2, \
							uint16_t ch3, uint16_t ch4, uint16_t ch5)
{
	//uint8_t tmp0 = 0, tmp1 = 0;
	uint16_t tmp[6] = {0,0,0,0,0,0};
	uint16_t combo[5] = {0,0,0,0,0};
	uint16_t index = 0;

	//Compress to 12bits
	tmp[0] = (ch0 >> 4) & 0x0FFF;
	tmp[1] = (ch1 >> 4) & 0x0FFF;
	tmp[2] = (ch2 >> 4) & 0x0FFF;
	tmp[3] = (ch3 >> 4) & 0x0FFF;
	tmp[4] = (ch4 >> 4) & 0x0FFF;
	tmp[5] = (ch5 >> 4) & 0x0FFF;

	//We want:
	//combo[0]: 0000 0000 0000 1111
	//combo[1]: 1111 1111 2222 2222
	//combo[2]: 2222 3333 3333 3333
	//combo[3]: 4444 4444 4444 5555
	//combo[4]: ____ ____ 5555 5555

	//Combine:
	combo[0] = (tmp[0] << 4) | ((tmp[1] >> 8) & 0xFF);
	combo[1] = (tmp[1] << 8) | ((tmp[2] >> 4) & 0xFFFF);
	combo[2] = (tmp[2] << 12) | (tmp[3]);
	combo[3] = (tmp[4] << 4) | ((tmp[5] >> 8) & 0xFF);
	combo[4] = (tmp[5] & 0xFF);

	//Stock in uint8_t buffer:
	SPLIT_16((uint16_t)combo[0], buf, &index);
	SPLIT_16((uint16_t)combo[1], buf, &index);
	SPLIT_16((uint16_t)combo[2], buf, &index);
	SPLIT_16((uint16_t)combo[3], buf, &index);
	buf[index++] = (uint8_t)combo[4];

	return 0;
}

//Unpack from buffer
void unpackCompressed6ch(uint8_t *buf, uint16_t *v0, uint16_t *v1, uint16_t *v2, \
							uint16_t *v3, uint16_t *v4, uint16_t *v5)
{
	*v0 = ((*(buf+0) << 8 | *(buf+1)) >> 4) << 4;
	*v1 = (((*(buf+1) << 8 | *(buf+2))) & 0xFFF) << 4;
	*v2 = ((*(buf+3) << 8 | *(buf+4)) >> 4) << 4;
	*v3 = (((*(buf+4) << 8 | *(buf+5))) & 0xFFF) << 4;
	*v4 = ((*(buf+6) << 8 | *(buf+7)) >> 4) << 4;
	*v5 = (((*(buf+7) << 8 | *(buf+8))) & 0xFFF) << 4;
}

void decode6chAmp(void)
{
	strain_6ch_bytes_to_words(i2c2_dma_rx_buf);
}

void compress6chTestCodeBlocking(void)
{
	uint8_t buffer[20];
	uint16_t strainValues[6] = {0x1111, 0x2222, 0x3333, 0x4444, 0x5555, 0x6666};
	uint16_t results[6] = {0,0,0,0,0,0};

	while(1)
	{
		compressAndSplit6ch(buffer, strainValues[0], strainValues[1], \
							strainValues[2],strainValues[3],\
							strainValues[4],strainValues[5]);

		unpackCompressed6ch(buffer, &results[0], &results[1], &results[2], \
							&results[3], &results[4], &results[5]);
	}
}

//****************************************************************************
// Test Function(s) - Use with care!
//****************************************************************************

#endif //USE_6CH_AMP
