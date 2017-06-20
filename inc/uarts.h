/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'flexsea-manage' Mid-level computing, and networking
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
	[This file] fm_uarts: Deals with the 3 USARTs
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-23 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

#ifndef INC_UARTS_H
#define INC_UARTS_H

//****************************************************************************
// Include(s)
//****************************************************************************

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"

//****************************************************************************
// Shared Variable(s)
//****************************************************************************

extern USART_HandleTypeDef husart1;			//RS-485 #1
extern USART_HandleTypeDef husart6;			//RS-485 #2
extern USART_HandleTypeDef husart3;			//Expansion port
extern DMA_HandleTypeDef hdma2_str2_ch4;	//DMA for RS-485 #1 RX
extern DMA_HandleTypeDef hdma2_str1_ch5;	//DMA for RS-485 #2 RX
extern DMA_HandleTypeDef hdma1_str1_ch4;	//DMA for USART3 RX
extern DMA_HandleTypeDef hdma1_str3_ch4;	//DMA for USART3 TX

//****************************************************************************
// Public Function Prototype(s):
//****************************************************************************

//General:
void init_rs485_outputs(void);
void rs485_set_mode(uint32_t port, uint8_t rx_tx);
void HAL_USART_TxCpltCallback(USART_HandleTypeDef *husart);
void HAL_USART_ErrorCallback(USART_HandleTypeDef *husart);

//RS-485 #1:
void init_usart1(uint32_t baudrate);
void puts_rs485_1(uint8_t *str, uint16_t length);
uint8_t reception_rs485_1_blocking(void);
void DMA2_Str2_CompleteTransfer_Callback(DMA_HandleTypeDef *hdma);

//UART - Execute:
void init_usart6(uint32_t baudrate);
void puts_uart_ex(uint8_t *str, uint16_t length);
uint8_t reception_uart_ex_blocking(void);
void DMA2_Str1_CompleteTransfer_Callback(DMA_HandleTypeDef *hdma);

//Other USART:
void init_usart3(uint32_t baudrate);
void puts_expUart(uint8_t *str, uint16_t length);

void resetBluetooth(void);

//****************************************************************************
// Definition(s):
//****************************************************************************

#define UART_TIMEOUT		5000	//ms?

#define BT_RST(x) HAL_GPIO_WritePin(GPIOA, 1<<4, x);

//Baud = fck/(8*(2-OVER8)*USARTDIV) and OVER8 = 1 so Baud = fck/(8*USARTDIV)
//USARTDIV = fck / (8*Baud)
//1Mbits/s: 84MHz / (8*1e6) = 10.5 => 1010 0 100 => 164
#define USART1_6_1MBAUD		164
#define USART1_6_2MBAUD		82
#define USART1_6_3MBAUD		49	//It's 3.333
#define USART1_6_4MBAUD		37

//USART1 (RS-485 #1):
//===================
//RE1:	 	PF12
//DE1: 		PF11

//USART6 (RS-485 #2):
//===================
//RE4:		PE11
//DE4:		PE10

//Common define
#define RS485_STANDBY				0x00
#define RS485_RX					0x01
#define RS485_TX					0x02
#define RS485_RX_TX					0x03

#endif // INC_UARTS_H

