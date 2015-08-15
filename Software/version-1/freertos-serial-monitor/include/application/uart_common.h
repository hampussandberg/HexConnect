/**
 *******************************************************************************
 * @file  uart_common.h
 * @author  Hampus Sandberg
 * @version 0.1
 * @date  2015-08-15
 * @brief
 *******************************************************************************
  Copyright (c) 2015 Hampus Sandberg.

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef UART_COMMON_H_
#define UART_COMMON_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "simple_gui.h"

/* Defines -------------------------------------------------------------------*/
#define IS_UART_CONNECTION(X)	(((X) == UARTConnection_Disconnected) || \
								 ((X) == UARTConnection_Connected))

#define IS_UART_BAUD_RATE(X)	(((X) == UARTBaudRate_4800) || \
								 ((X) == UARTBaudRate_7200) || \
								 ((X) == UARTBaudRate_9600) || \
								 ((X) == UARTBaudRate_7200) || \
								 ((X) == UARTBaudRate_14400) || \
								 ((X) == UARTBaudRate_19200) || \
								 ((X) == UARTBaudRate_28800) || \
								 ((X) == UARTBaudRate_38400) || \
								 ((X) == UARTBaudRate_57600) || \
								 ((X) == UARTBaudRate_115200) || \
								 ((X) == UARTBaudRate_230400))

#define IS_UART_POWER(X)		(((X) == UARTPower_5V) || \
								 ((X) == UARTPower_3V3))

#define IS_UART_MODE_APP(X)		(((X) == UARTMode_RX) || \
								 ((X) == UARTMode_TX) || \
								 ((X) == UARTMode_TX_RX) || \
								 ((X) == UARTMode_DebugTX))

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
	UARTParity_None = UART_PARITY_NONE,
	UARTParity_Odd = UART_PARITY_ODD,
	UARTParity_Even = UART_PARITY_EVEN,
} UARTParity;

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
	UARTParity parity;
	UARTPower power;
	UARTMode mode;

	GUITextFormat textFormat;

	uint32_t writeAddress;
	uint32_t amountOfDataSaved;

	/* TODO: Parity bits, stop bits etc */
} UARTSettings;

/* Function prototypes -------------------------------------------------------*/


#endif /* UART_COMMON_H_ */
