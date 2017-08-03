//****************************************************************************
// MIT Media Lab - Biomechatronics
// Jean-Francois (Jeff) Duval
// jfduval@media.it.edu
// 05/2015
//****************************************************************************
// fm_stm32f4xx_it: Interrupt Handlers
//****************************************************************************
// Licensing: Please refer to 'software_license.txt'
//****************************************************************************

//****************************************************************************
// Include(s)
//****************************************************************************

#include <adc.h>
#include <i2c.h>
#include <spi.h>
#include <timer.h>
#include <uarts.h>
#include "main.h"
#include "ui.h"
#include "fm_stm32f4xx_hal_spi.h"
#include "fm_stm32f4xx_it.h"

//****************************************************************************
// Variable(s)
//****************************************************************************

volatile unsigned int spi_bytes_ready = 0;

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
extern TIM_HandleTypeDef htim7;

//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************

//...

//****************************************************************************
// Public Function(s)
//****************************************************************************

//System Timer, 1ms ISR
void SysTick_Handler(void)
{
	//Decrement to zero the counter used by the delay routine.
	if(timer_delayCount != 0u)
	{
		--timer_delayCount;
	}

	//For USB delays:
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}

void SPI4_IRQHandler(void)
{
	//Increment bytes counter
	spi_bytes_ready++;

	HAL_SPI_IRQHandler(&spi4_handle);
}

//SPI:
void EXTI4_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
}

//SYNC line:
void EXTI15_10_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
}

//Should not be used, everything is done via DMA
void USART1_IRQHandler(void)
{
	HAL_USART_IRQHandler(&husart1);
}

//Should not be used, everything is done via DMA
void USART3_IRQHandler(void)
{
	HAL_USART_IRQHandler(&husart3);
}

//Should not be used, everything is done via DMA
void USART6_IRQHandler(void)
{
	HAL_USART_IRQHandler(&husart6);
}

//DMA2 Stream2 - USART1 RX
void DMA2_Stream2_IRQHandler(void)
{
	HAL_NVIC_ClearPendingIRQ(DMA2_Stream2_IRQn);

	HAL_DMA_IRQHandler(&hdma2_str2_ch4);
}

//DMA2 Stream7 - USART1 TX
void DMA2_Stream7_IRQHandler(void)
{
	HAL_NVIC_ClearPendingIRQ(DMA2_Stream7_IRQn);

	HAL_DMA_IRQHandler(husart1.hdmatx);
}

//DMA1 Stream1 - USART3 RX
void DMA1_Stream1_IRQHandler(void)
{
	HAL_NVIC_ClearPendingIRQ(DMA1_Stream1_IRQn);

	HAL_DMA_IRQHandler(&hdma1_str1_ch4);
}

//DMA1 Stream3 - USART3 TX
void DMA1_Stream3_IRQHandler(void)
{
	HAL_NVIC_ClearPendingIRQ(DMA1_Stream3_IRQn);

	HAL_DMA_IRQHandler(husart3.hdmatx);
}

//DMA2 Stream1 - USART6 RX
void DMA2_Stream1_IRQHandler(void)
{
	HAL_NVIC_ClearPendingIRQ(DMA2_Stream1_IRQn);

	HAL_DMA_IRQHandler(&hdma2_str1_ch5);
}

//DMA2 Stream6 - USART6 TX
void DMA2_Stream6_IRQHandler(void)
{
	HAL_NVIC_ClearPendingIRQ(DMA2_Stream6_IRQn);

	HAL_DMA_IRQHandler(husart6.hdmatx);
}

//DMA1 Stream 0: I2C1 RX
void DMA1_Stream0_IRQHandler(void)
{
	HAL_DMA_IRQHandler(hi2c1.hdmarx);
}

//DMA1 Stream 6: I2C1 TX
void DMA1_Stream6_IRQHandler(void)
{
	HAL_DMA_IRQHandler(hi2c1.hdmatx);
}

//DMA1 Stream 2: I2C2 RX
void DMA1_Stream2_IRQHandler(void)
{
	HAL_DMA_IRQHandler(hi2c2.hdmarx);
}

//DMA1 Stream 7: I2C2 TX
void DMA1_Stream7_IRQHandler(void)
{
	HAL_DMA_IRQHandler(hi2c2.hdmatx);
}

//ADC DMA Interrupt
void DMA2_Stream0_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&hdma_adc1);
}

//USB:
void OTG_FS_IRQHandler(void)
{
	HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}

void TIM7_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim7);

	//FlexSEA timebase:
	//timebases();
}

void I2C3_EV_IRQHandler(void)
{
	HAL_I2C_EV_IRQHandler(&hi2c3);
}

//****************************************************************************
// Fault & Exceptions
//****************************************************************************

//We use this function to catch all other errors (easier to debug):
void genericFaultHandler(uint8_t i, uint8_t major)
{
	if(major)
	{
		while(1){LEDR(1);}
	}
	else
	{
		while(1)
		{
			//White
			LEDR(1);
			LEDG(1);
			LEDB(1);
		}
	}
}

void NMI_Handler(void)
{
	genericFaultHandler(0, 0);
}

void HardFault_Handler(void)
{
	//Go to infinite loop when Hard Fault exception occurs
	genericFaultHandler(1, 1);
}

void MemManage_Handler(void)
{
	//Go to infinite loop when Memory Manage exception occurs
	genericFaultHandler(2, 1);
}

void BusFault_Handler(void)
{
	//Go to infinite loop when Bus Fault exception occurs
	genericFaultHandler(3, 1);
}

void UsageFault_Handler(void)
{
	//Go to infinite loop when Usage Fault exception occurs
	genericFaultHandler(4, 1);
}

void SVC_Handler(void)
{
	genericFaultHandler(5, 0);
}

void DebugMon_Handler(void)
{
	genericFaultHandler(6, 0);
}

void PendSV_Handler(void)
{
	genericFaultHandler(7, 0);
}

//Processor ends up here if an unexpected interrupt occurs or a specific
// handler is not present in the application code.
void Default_Handler(void)
{
	genericFaultHandler(8, 1);
}
