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

//****************************************************************************
// Include(s)
//****************************************************************************

#include <adc.h>
#include "main.h"
#include "isr.h"
#include "flexsea_user_structs.h"

//****************************************************************************
// Variable(s)
//****************************************************************************

ADC_HandleTypeDef hadc1;
ADC_ChannelConfTypeDef sConfig;
ADC_MultiModeTypeDef multimode;
int8_t temperature = 0;

DMA_HandleTypeDef hdma_adc1;
__IO volatile uint16_t adc_results[ADC_CHANNELS];
unsigned int adc_filtered_results[ADC_CHANNELS];

volatile uint8_t readyForNextAdcConversion = 0;

//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************

static void initAdcGpio(void);
static void initAdcDma(void);

//****************************************************************************
// Public Function(s)
//****************************************************************************

void init_adc1(void)
{
	//Enable peripheral clock and GPIOs:
	__ADC1_CLK_ENABLE();
	initAdcGpio();
	initAdcDma();

	//ADC1 config:
	//===========

	//Configure the global features of the ADC
	//(Clock, Resolution, Data Alignment and number of conversion)
	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV2;
	hadc1.Init.Resolution = ADC_RESOLUTION12b;
	hadc1.Init.ScanConvMode = ENABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = ADC_CHANNELS;
	hadc1.Init.DMAContinuousRequests = ENABLE;
	hadc1.Init.EOCSelection = EOC_SEQ_CONV;
	HAL_ADC_Init(&hadc1);

	//Configure for the selected ADC regular channel its corresponding rank in
	//the sequencer and its sample time.
	sConfig.Channel = ADC_CHANNEL_0;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
	HAL_ADC_ConfigChannel(&hadc1, &sConfig);

	sConfig.Channel = ADC_CHANNEL_1;
	sConfig.Rank = 2;
	sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
	HAL_ADC_ConfigChannel(&hadc1, &sConfig);

	sConfig.Channel = ADC_CHANNEL_2;
	sConfig.Rank = 3;
	sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
	HAL_ADC_ConfigChannel(&hadc1, &sConfig);

	sConfig.Channel = ADC_CHANNEL_3;
	sConfig.Rank = 4;
	sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
	HAL_ADC_ConfigChannel(&hadc1, &sConfig);

	//Internal temperature sensor:
	sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
	sConfig.Rank = 5;
	sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
	HAL_ADC_ConfigChannel(&hadc1, &sConfig);

	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc_results, ADC_CHANNELS);
}

void startAdcConversion(void)
{
	if(readyForNextAdcConversion)
	{
		readyForNextAdcConversion = 0;

		HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc_results, ADC_CHANNELS);
	}
}

/*
 * @brief 	This function gets the value at an analog pin at the index PA0-PA7
 * @param 	Index of the pin, use int 0-7
 * @ret		Value of an analog pin. 16 bit word
 */
unsigned int get_adc1(uint16_t idx)
{
	return adc_results[idx];
}

//The ADC reads the sensor. This function converts the result to degrees.
int8_t readInternalTempSensor(void)
{
	temperature = ((VSENSE_SLOPE * (adc_results[ADC_CHANNELS-1] - V25_TICKS) \
					/ TICK_TO_V) + 25);
	return temperature;
}

//Copies the latest ADC results in the Rigid structure
void updateADCbuffers(void)
{
	rigid1.mn.analog[0] = adc_results[0];
	rigid1.mn.analog[1] = adc_results[1];
	rigid1.mn.analog[2] = adc_results[2];
	rigid1.mn.analog[3] = adc_results[3];
}

//Code branches here after conversion is done:
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if(hadc->Instance == ADC1)	//ADC
	{
		//Start a new conversion:
		readyForNextAdcConversion = 1;
	}
}

//****************************************************************************
// Private Function(s)
//****************************************************************************

static void initAdcGpio(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	__GPIOA_CLK_ENABLE();

	//Config inputs:
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 \
			| GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

static void initAdcDma(void)
{
	__DMA2_CLK_ENABLE();

	hdma_adc1.Instance = DMA2_Stream0;
	hdma_adc1.Init.Channel = DMA_CHANNEL_0;
	hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
	hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
	hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
	hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
	hdma_adc1.Init.Mode = DMA_CIRCULAR;
	hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	hdma_adc1.Init.MemBurst = DMA_MBURST_SINGLE;
	hdma_adc1.Init.PeriphBurst = DMA_PBURST_SINGLE;

	hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
	hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	HAL_DMA_Init(&hdma_adc1);

	//Link handles:
	hdma_adc1.Parent = &hadc1;
	hadc1.DMA_Handle = &hdma_adc1;

	// DMA2_Stream0_IRQn interrupt configuration
	HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, ISR_DMA2_STREAM0, ISR_SUB_DMA2_STREAM0);
	HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
}
