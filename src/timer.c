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
	[This file] fm_timer: SysTick & Timer functions
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-23 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

//****************************************************************************
// Include(s)
//****************************************************************************

#include <dio.h>
#include <timer.h>
#include "main.h"
#include "isr.h"

//****************************************************************************
// Variable(s)
//****************************************************************************

volatile timer_ticks_t timer_delayCount;
volatile uint8_t tb_100us_flag = 0;
volatile uint8_t tb_100us_timeshare = 0;
volatile uint8_t tb_1ms_flag = 0;
volatile uint8_t tb_10ms_flag = 0;
volatile uint8_t tb_100ms_flag = 0;
volatile uint8_t tb_1000ms_flag = 0;

TIM_HandleTypeDef htim6, htim7;

//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base);
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base);

//****************************************************************************
// Public Function Prototype(s):
//****************************************************************************

//Init SysTick timer, 1ms
void init_systick_timer(void)
{
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
	// SysTick_IRQn interrupt configuration
	HAL_NVIC_SetPriority(SysTick_IRQn, ISR_SYSTICK, ISR_SUB_SYSTICK);
}

//Timebases for all the main functions:
void timebases(void)
{
	//100us flag:
	tb_100us_flag = 1;
}

//Timer 6: free running timer used by the us delay functions
void init_timer_6(void)
{
	TIM_MasterConfigTypeDef sMasterConfig;

	htim6.Instance = TIM6;
	htim6.Init.Prescaler = 0;
	htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim6.Init.Period = 65535;
	HAL_TIM_Base_Init(&htim6);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig);

	//Start timer, no interrupts
	HAL_TIM_Base_Start(&htim6);
}

uint16_t readTimer6(void)
{
	return (uint16_t)htim6.Instance->CNT;
}

#ifndef BOARD_SUBTYPE_RIGID
//Timer 7: 10kHz timebase
void init_timer_7(void)
{
	TIM_MasterConfigTypeDef sMasterConfig;

	htim7.Instance = TIM7;
	htim7.Init.Prescaler = 0;
	htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim7.Init.Period = 8400;
	HAL_TIM_Base_Init(&htim7);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig);

	HAL_TIM_Base_Start_IT(&htim7);
}
#endif

//Blocking function based on Timer 6. Max delay = 500us (0.5ms).
//Please be aware that the delay passed as a parameter is the minimum: it can
//get longer due to interrupts.
void delayUsBlocking(uint16_t us)
{
	uint16_t timerLimit = 0;

	if(us > 500)
	{
		//Cap below the true max (65536) to simplify code
		us = 500;
	}
	//From us to ticks:
	timerLimit = (84 * us) - 42;	//(42 compensates for the overhead)

	//Reset the counter
	htim6.Instance->CNT = 0;

	//Waste time:
	while(htim6.Instance->CNT < timerLimit);
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
	if (htim_base->Instance == TIM6)
	{
		__TIM6_CLK_ENABLE();
	}

	if (htim_base->Instance == TIM7)
	{
		__TIM7_CLK_ENABLE();

		HAL_NVIC_SetPriority(TIM7_IRQn, ISR_TIMER7, ISR_SUB_TIMER7);
		HAL_NVIC_EnableIRQ(TIM7_IRQn);
	}

}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{
	if (htim_base->Instance == TIM6)
	{
		__TIM6_CLK_DISABLE();
	}

	if (htim_base->Instance == TIM7)
	{
		__TIM7_CLK_DISABLE();

		HAL_NVIC_DisableIRQ(TIM7_IRQn);
	}
}

//****************************************************************************
// Test code:
//****************************************************************************

//Blocking delay us test code:
void test_delayUsBlocking_blocking(void)
{
	while(1)
	{
		//1us:
		DEBUG_H0(1);
		delayUsBlocking(1);
		DEBUG_H0(0);
		delayUsBlocking(1);

		//2us
		DEBUG_H0(1);
		delayUsBlocking(2);
		DEBUG_H0(0);
		delayUsBlocking(2);

		//5us:
		DEBUG_H0(1);
		delayUsBlocking(5);
		DEBUG_H0(0);
		delayUsBlocking(5);

		//10us:
		DEBUG_H0(1);
		delayUsBlocking(10);
		DEBUG_H0(0);
		delayUsBlocking(10);

		//100us:
		DEBUG_H0(1);
		delayUsBlocking(100);
		DEBUG_H0(0);
		delayUsBlocking(100);
	}
}
