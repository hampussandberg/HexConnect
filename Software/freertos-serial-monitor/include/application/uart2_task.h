/**
 ******************************************************************************
 * @file	uart2_task.h
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
#ifndef UART2_TASK_H_
#define UART2_TASK_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

#include "messages.h"
#include "spi_flash_memory_map.h"

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
typedef enum
{
	UART2Connection_Disconnected,
	UART2Connection_Connected,
} UART2Connection;

typedef enum
{
	UART2BaudRate_4800 = 4800,
	UART2BaudRate_7200 = 7200,
	UART2BaudRate_9600 = 9600,
	UART2BaudRate_14400 = 14400,
	UART2BaudRate_19200 = 19200,
	UART2BaudRate_28800 = 28800,
	UART2BaudRate_38400 = 38400,
	UART2BaudRate_57600 = 57600,
	UART2BaudRate_115200 = 115200,
	UART2BaudRate_230400 = 230400,
	UART2BaudRate_250000 = 250000,
	UART2BaudRate_Custom = 0,
} UART2BaudRate;

typedef enum
{
	UART2Power_5V,
	UART2Power_3V3,
} UART2Power;

typedef enum
{
	UART2Mode_RX = USART_MODE_RX,
	UART2Mode_TX = USART_MODE_TX,
	UART2Mode_TX_RX = USART_MODE_TX_RX,
	UART2Mode_DebugTX,
} UART2Mode;

typedef struct
{
	UART2Connection connection;
	UART2BaudRate baudRate;
	UART2Power power;
	UART2Mode mode;

	/* TODO: Parity bits, stop bits etc */
} UART2Settings;

/* Function prototypes -------------------------------------------------------*/
void uart2Task(void *pvParameters);
ErrorStatus uart2SetPower(UART2Power Power);
ErrorStatus uart2SetConnection(UART2Connection Connection);
UART2Settings uart2GetSettings();
ErrorStatus uart2SetSettings(UART2Settings* Settings);

void uart2Transmit(uint8_t* Data, uint16_t Size);

void uart2TxCpltCallback();
void uart2RxCpltCallback();
void uart2ErrorCallback();

#endif /* UART2_TASK_H_ */
