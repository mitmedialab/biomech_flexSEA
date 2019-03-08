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
	[This file] fm_stm32f4xx_it: Interrupt Handlers
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	*
****************************************************************************/

#ifndef __STM32F4xx_IT_H
#define __STM32F4xx_IT_H

#ifdef __cplusplus
 extern "C" {
#endif

void genericFaultHandler(uint8_t i, uint8_t major);
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void Default_Handler(void);

void SysTick_Handler(void);
void SPI4_IRQHandler(void);	//ToDo Wrong, should be 1
void USART1_IRQHandler(void);
void OTG_FS_IRQHandler(void);

void EXTI4_IRQHandler(void);

#ifdef BOARD_SUBTYPE_RIGID
#ifndef BOARD_SUBTYPE_POCKET
void EXTI15_10_IRQHandler(void);
#else
void EXTI9_5_IRQHandler(void);
#endif 	//BOARD_SUBTYPE_POCKET
#else	//BOARD_SUBTYPE_RIGID
void TIM7_IRQHandler(void);
#endif

void USART1_IRQHandler(void);
void USART3_IRQHandler(void);
void USART6_IRQHandler(void);
void DMA2_Stream2_IRQHandler(void);
void DMA2_Stream7_IRQHandler(void);
void DMA1_Stream1_IRQHandler(void);
void DMA1_Stream3_IRQHandler(void);
void DMA2_Stream1_IRQHandler(void);
void DMA2_Stream6_IRQHandler(void);
void DMA1_Stream7_IRQHandler(void);
void DMA1_Stream2_IRQHandler(void);
void DMA1_Stream0_IRQHandler(void);
void DMA1_Stream6_IRQHandler(void);
void DMA2_Stream0_IRQHandler(void);
void I2C3_EV_IRQHandler(void);
void I2C3_ER_IRQHandler(void);
void I2C1_EV_IRQHandler(void);
void I2C1_ER_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F4xx_IT_H */
