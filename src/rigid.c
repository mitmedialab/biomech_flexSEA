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
	[This file] rigid: Interface with Rigid's other microcontrollers
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2017-04-19 | jfduval | Initial release
	*
****************************************************************************/

//****************************************************************************
// Include(s)
//****************************************************************************

#include "main.h"
#include "rigid.h"
#include "i2c.h"
#include "flexsea.h"
#include "flexsea_user_structs.h"
#include "stm32f4xx_hal_i2c.h"
#include "user-mn.h"
#include "eeprom_user.h"

//****************************************************************************
// Variable(s)
//****************************************************************************


volatile int8_t activeFSM = FSMS_INACTIVE;
volatile int8_t timingError[10] = {0};
uint16_t buttonClicked = 0;
uint8_t i2c3_tmp_buf[EX_EZI2C_BUF_SIZE];
struct i2t_s i2tBatt;
uint16_t uvlo = 0;

//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************

static void updateButtonClicked(uint16_t i);

//****************************************************************************
// Function(s)
//****************************************************************************

void initRigidIO(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	#ifndef BOARD_SUBTYPE_POCKET

	//Enable GPIO Peripheral clocks
	__GPIOG_CLK_ENABLE();

	//SYNC as input with interrupt:
	GPIO_InitStructure.Pin = GPIO_PIN_15;
	GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStructure.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 7, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

	#else

	//Enable GPIO Peripheral clocks
	__GPIOC_CLK_ENABLE();

	//SYNC as input with interrupt:
	GPIO_InitStructure.Pin = GPIO_PIN_8;
	GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStructure.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 7, 0);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

	#endif	//BOARD_SUBTYPE_POCKET
}

//Decodes the bytes received over I2C
void decodeRegulate(void)
{
	uint16_t index = 0;
	uint8_t ctrl1 = 0, ctrl2 = 0;

	ctrl1 = i2c_3_r_buf[index++];
	ctrl2 = i2c_3_r_buf[index++];
	rigid1.re.vb = REBUILD_UINT16(i2c_3_r_buf, &index);
	rigid1.re.vg = REBUILD_UINT16(i2c_3_r_buf, &index);
	rigid1.re.v5 = REBUILD_UINT16(i2c_3_r_buf, &index);
	rigid1.re.current = (int16_t)REBUILD_UINT16(i2c_3_r_buf, &index);
	rigid1.re.temp = i2c_3_r_buf[index++];
	rigid1.re.status = i2c_3_r_buf[index++];	//This fills the lower byte.

	(void)ctrl1;
	updateButtonClicked(ctrl2);
}

//Prepares the I2C3 buffer for Re to read at boot:
void setRegulateLimits(uint16_t vMin, struct i2t_s i2t)
{
	uint16_t index = 0;
	uint8_t i = 0, checksum = 0;

	//Config byte contains shift + non-lin enable bit
	LIMIT(i2t.shift, 0, 15);	//Limit to 4 bits
	i2t.config = i2t.shift | i2t.useNL;

	i2c_3_t_buf[index++] = I2C_READ_KEY;

	//Data:
	SPLIT_16((uint16_t)vMin, i2c_3_t_buf, &index);		//Min. voltage
	SPLIT_16((uint16_t)i2t.leak, i2c_3_t_buf, &index);	//I2T_LEAK
	SPLIT_32((uint32_t)i2t.limit, i2c_3_t_buf, &index);	//I2T_LIMIT
	i2c_3_t_buf[index++] = i2t.nonLinThreshold;			//I2T_NON_LIN_THRESHOLD
	i2c_3_t_buf[index++] = i2t.config;					//I2T Config

	//Checksum:
	for(i = 0; i < index; i++)
	{
		checksum += i2c_3_t_buf[i];
	}
	i2c_3_t_buf[index++] = checksum;
}

void saveUVLO(uint16_t v)
{
	LIMIT(v, MIN_UVLO, MAX_UVLO);
	uvlo = v;
}

uint16_t getUVLO(void)
{
	return uvlo;
}

void loadNvUVLO(void)
{
	#ifdef USE_EEPROM

	uint16_t v = 0;

	//Get UVLO from EEPROM (if it was ever written)
	readUvloEEPROM();
	v = getNvUVLO();
	if(v != 0)
	{
		//We read a valid value
		saveUVLO(v);
	}
	else
	{
		saveUVLO(DEFAULT_UVLO);
	}

	#else

	saveUVLO(DEFAULT_UVLO);

	#endif	//USE_EEPROM
}

void saveI2t(struct i2t_s newI2t)
{
	//ToDo include safety checks here
	i2tBatt = newI2t;
}

void loadNvI2t(void)
{
	#ifdef USE_EEPROM

	uint8_t v = 0;
	struct i2t_s tmpI2t;

	//Get I2t from EEPROM (if it was ever written)
	readI2tEEPROM();
	v = getNvI2t(&tmpI2t);

	//Invalid? Use defaults
	if(v == 0){setDefaultI2t(&tmpI2t);}

	//Save it
	saveI2t(tmpI2t);

	#else

	setDefaultI2t(&tmpI2t);
	saveI2t(tmpI2t);

	#endif	//USE_EEPROM
}

//We store EEPROM in 16-bit values
void packI2t(struct i2t_s val, uint16_t *b, uint8_t *index)
{
	//Note: we could pack 2x uint8 per uint16, but we decided to KISS for now

	b[(*index)++] = val.shift;									//I2T_SHIFT
	b[(*index)++] = val.leak;									//I2T_LEAK

	b[(*index)++] = (uint16_t) ((val.limit >> 16) & 0xFFFF);	//I2T_LIMIT
	b[(*index)++] = (uint16_t) (val.limit & 0xFFFF);

	b[(*index)++] = val.nonLinThreshold;						//I2T_NON_LIN_THRESHOLD
	b[(*index)++] = val.config;									//I2T_CONFIG
}

void unpackI2t(struct i2t_s *val, uint16_t *b, uint8_t *index)
{
	uint32_t tmpA = 0, tmpB = 0;

	val->shift = b[(*index)++]; 								//I2T_SHIFT
	val->leak = b[(*index)++];									//I2T_LEAK

	tmpA = b[(*index)++];
	tmpB = b[(*index)++];
	val->limit = ((tmpA << 16) & 0xFFFF0000) + tmpB;			//I2T_LIMIT

	val->nonLinThreshold = b[(*index)++];						//I2T_NON_LIN_THRESHOLD
	val->config = b[(*index)++];								//I2T_CONFIG

	//Compute other data:
	val->warning = (8 * val->limit) / 10;
}

void setDefaultI2t(struct i2t_s *s)
{
	s->shift = 7;
	s->leak = 6105;
	s->limit = 76295;
	s->nonLinThreshold = 125;
	s->useNL = I2T_ENABLE_NON_LIN;
	s->config = 128;
}

//****************************************************************************
// Private Function(s)
//****************************************************************************

static void updateButtonClicked(uint16_t i)
{
	if(i == 1)
	{
		buttonClicked++;
	}
}
