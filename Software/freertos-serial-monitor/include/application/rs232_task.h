/**
 ******************************************************************************
 * @file	rs232_task.h
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
#ifndef RS232_TASK_H_
#define RS232_TASK_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "timers.h"

#include "messages.h"
#include "simple_gui.h"

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
typedef enum
{
	RS232Connection_Connected,
	RS232Connection_Disconnected,
} RS232Connection;

typedef enum
{
	RS232BaudRate_4800 = 4800,
	RS232BaudRate_7200 = 7200,
	RS232BaudRate_9600 = 9600,
	RS232BaudRate_14400 = 14400,
	RS232BaudRate_19200 = 19200,
	RS232BaudRate_28800 = 28800,
	RS232BaudRate_38400 = 38400,
	RS232BaudRate_57600 = 57600,
	RS232BaudRate_115200 = 115200,
	RS232BaudRate_230400 = 230400,
	RS232BaudRate_250000 = 250000,
	RS232BaudRate_Custom = 0,
} RS232BaudRate;

typedef enum
{
	RS232Power_5V,
	RS232Power_3V3,
} RS232Power;

typedef enum
{
	RS232Mode_RX = UART_MODE_RX,
	RS232Mode_TX = UART_MODE_TX,
	RS232Mode_TX_RX = UART_MODE_TX_RX,
	RS232Mode_DebugTX,
} RS232Mode;

typedef struct
{
	RS232Connection connection;
	RS232BaudRate baudRate;
	RS232Power power;
	RS232Mode mode;

	GUIWriteFormat writeFormat;

	/* TODO: Parity bits, stop bits etc */
} RS232Settings;

/* Function prototypes -------------------------------------------------------*/
void rs232Task(void *pvParameters);
ErrorStatus rs232SetConnection(RS232Connection Connection);
RS232Settings rs232GetSettings();
ErrorStatus rs232SetSettings(RS232Settings* Settings);
uint32_t rs232GetCurrentWriteAddress();

void rs232Transmit(uint8_t* Data, uint32_t Size);

void rs232TxCpltCallback();
void rs232RxCpltCallback();
void rs232ErrorCallback();

#endif /* RS232_TASK_H_ */
