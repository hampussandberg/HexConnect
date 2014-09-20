/**
 ******************************************************************************
 * @file	uart_common.h
 * @author	Hampus
 * @version	0.1
 * @date	YYYY-MM-DD
 * @brief
 ******************************************************************************
	Copyright (c) 2014 Hampus.

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
#ifndef UART_COMMON_H_
#define UART_COMMON_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "simple_gui.h"

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
typedef enum
{
	UARTConnection_Disconnected,
	UARTConnection_Connected,
} UARTConnection;

typedef enum
{
	UARTBaudRate_4800 = 4800,
	UARTBaudRate_7200 = 7200,
	UARTBaudRate_9600 = 9600,
	UARTBaudRate_14400 = 14400,
	UARTBaudRate_19200 = 19200,
	UARTBaudRate_28800 = 28800,
	UARTBaudRate_38400 = 38400,
	UARTBaudRate_57600 = 57600,
	UARTBaudRate_115200 = 115200,
	UARTBaudRate_230400 = 230400,
	UARTBaudRate_Custom = 0,
} UARTBaudRate;

typedef enum
{
	UARTPower_5V,
	UARTPower_3V3,
} UARTPower;

typedef enum
{
	UARTMode_RX = UART_MODE_RX,
	UARTMode_TX = UART_MODE_TX,
	UARTMode_TX_RX = UART_MODE_TX_RX,
	UARTMode_DebugTX,
} UARTMode;

typedef enum
{
	BUFFERState_Idle,
	BUFFERState_Writing,
	BUFFERState_Reading,
} BUFFERState;

typedef struct
{
	UARTConnection connection;
	UARTBaudRate baudRate;
	UARTPower power;
	UARTMode mode;

	GUIWriteFormat writeFormat;
	uint32_t numOfCharactersPerByte;

	uint32_t displayedDataStartAddress;
	uint32_t lastDisplayDataStartAddress;
	uint32_t displayedDataEndAddress;
	uint32_t lastDisplayDataEndAddress;
	uint32_t readAddress;
	uint32_t numOfCharactersDisplayed;
	uint32_t amountOfDataSaved;
	bool scrolling;

	SemaphoreHandle_t xSettingsSemaphore;

	/* TODO: Parity bits, stop bits etc */
} UARTSettings;

/* Function prototypes -------------------------------------------------------*/


#endif /* UART_COMMON_H_ */
