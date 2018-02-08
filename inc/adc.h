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
	[This file] fm_adc: Analog to digital converter
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-23 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

#ifndef INC_FM_ADC_H
#define INC_FM_ADC_H

//****************************************************************************
// Include(s)
//****************************************************************************

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"

//****************************************************************************
// Shared variable(s)
//****************************************************************************

extern DMA_HandleTypeDef hdma_adc1;
extern __IO volatile uint16_t adc_results[];

//****************************************************************************
// Public Function Prototype(s):
//****************************************************************************

void init_adc1(void);
unsigned int get_adc1(uint16_t idx);	// get value at analog input channel idx
void startAdcConversion(void);
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);
int8_t readInternalTempSensor(void);
void updateADCbuffers(void);

//****************************************************************************
// Definition(s):
//****************************************************************************

#define ADC_CHANNELS	5		//4 external channels + internal temp sensor

#define V25_TICKS		943		//760mV/3.3V * 4096 = 943
#define VSENSE_SLOPE	400		//1/2.5mV
#define TICK_TO_V		1241	//ticks/V

#endif 	// INC_FM_ADC_H

