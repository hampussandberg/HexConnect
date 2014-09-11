/**
 ******************************************************************************
 * @file	uart1_task.h
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-09-06
 * @brief
 ******************************************************************************
	Copyright (c) 2014 Hampus Sandberg.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation, either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef UART1_TASK_H_
#define UART1_TASK_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
typedef enum
{
	UART1Connection_Disconnected,
	UART1Connection_Connected,
} UART1Connection;

typedef enum
{
	UART1BaudRate_4800 = 4800,
	UART1BaudRate_7200 = 7200,
	UART1BaudRate_9600 = 9600,
	UART1BaudRate_14400 = 14400,
	UART1BaudRate_19200 = 19200,
	UART1BaudRate_28800 = 28800,
	UART1BaudRate_38400 = 38400,
	UART1BaudRate_57600 = 57600,
	UART1BaudRate_115200 = 115200,
	UART1BaudRate_230400 = 230400,
	UART1BaudRate_250000 = 250000,
	UART1BaudRate_Custom = 0,
} UART1BaudRate;

typedef enum
{
	UART1Power_5V,
	UART1Power_3V3,
} UART1Power;

typedef enum
{
	UART1Mode_RX = USART_MODE_RX,
	UART1Mode_TX = USART_MODE_TX,
	UART1Mode_TX_RX = USART_MODE_TX_RX,
	UART1Mode_DebugTX,
} UART1Mode;

typedef struct
{
	UART1Connection connection;
	UART1BaudRate baudRate;
	UART1Power power;
	UART1Mode mode;

	/* TODO: Parity bits, stop bits etc */
} UART1Settings;

/* Function prototypes -------------------------------------------------------*/
void uart1Task(void *pvParameters);
ErrorStatus uart1SetPower(UART1Power Power);
ErrorStatus uart1SetConnection(UART1Connection Connection);
UART1Settings uart1GetSettings();
ErrorStatus uart1SetSettings(UART1Settings* Settings);

void uart1Transmit(uint8_t* Data, uint16_t Size);


#endif /* UART1_TASK_H_ */
