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
	[This file] isr.h: Interrupt priority
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2017-05-23 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

#ifndef INC_ISR_H
#define INC_ISR_H

//****************************************************************************
// Definition(s):
//****************************************************************************

//Note: priority is coded on 4bits (0-15). Zero is the maximum priority.
//		Sub-priority is also 4bits.

//ADC DMA:
#define ISR_DMA2_STREAM0		10	//ADC1
#define ISR_SUB_DMA2_STREAM0	0

//UART - RS-485:
#define ISR_UART1				15	//RS-485 #1
#define ISR_SUB_UART1			0
#define ISR_UART6				15	//RS-485 #2
#define ISR_SUB_UART6			1
//(we use DMA, this doesn't matter)

//UART - Expansion:
#define ISR_UART3				8
#define ISR_SUB_UART3			0

//UART DMA - RS-485:
#define ISR_DMA2_STREAM2		5	//USART1 RX
#define ISR_SUB_DMA2_STREAM2	0
#define ISR_DMA2_STREAM1		5	//USART6 RX
#define ISR_SUB_DMA2_STREAM1	1
#define ISR_DMA2_STREAM7		4	//USART1 TX
#define ISR_SUB_DMA2_STREAM7	0
#define ISR_DMA2_STREAM6		4	//USART6 TX
#define ISR_SUB_DMA2_STREAM6	1

//UART DMA - Bluetooth:
#define ISR_DMA1_STREAM1		5	//USART3 RX
#define ISR_SUB_DMA1_STREAM1	0
#define ISR_DMA1_STREAM3		4	//USART3 TX
#define ISR_SUB_DMA1_STREAM3	0

//I2C1:
#define ISR_I2C1_EV				2
#define ISR_SUB_I2C1_EV			0
#define ISR_I2C1_ER				2
#define ISR_SUB_I2C1_ER			1
#define ISR_DMA1_STREAM0		0	//I2C1 RX DMA
#define ISR_SUB_DMA1_STREAM0	0
#define ISR_DMA1_STREAM6		2	//I2C1 TX DMA
#define ISR_SUB_DMA1_STREAM6	3

//I2C2 DMA:
#define ISR_DMA1_STREAM2		0	//I2C2 RX
#define ISR_SUB_DMA1_STREAM2	1
#define ISR_DMA1_STREAM7		2	//I2C2 TX
#define ISR_SUB_DMA1_STREAM7	4

//I2C2:
#define ISR_I2C2_EV				2
#define ISR_SUB_I2C2_EV			0
#define ISR_I2C2_ER				2
#define ISR_SUB_I2C2_ER			1

//I2C3:
#define ISR_I2C3				0
#define ISR_SUB_I2C3			0

//SPI4 - Plan:
#define ISR_SPI4				1
#define ISR_SUB_SPI4			1
#define ISR_EXTI4				0	//NSS line
#define ISR_SUB_EXTI4			0

//SPI5 - FLASH:
#define ISR_SPI5				9
#define ISR_SUB_SPI5			0

//SPI6 - Expansion:
#define ISR_SPI6				8
#define ISR_SUB_SPI6			0

//Timers:
#define ISR_SYSTICK				3
#define ISR_SUB_SYSTICK			1
#define ISR_TIMER7				1	//10kHz Timebase
#define ISR_SUB_TIMER7			0

//USB:
#define ISR_USB					3
#define ISR_SUB_USB				0

#endif // INC_ISR_H
