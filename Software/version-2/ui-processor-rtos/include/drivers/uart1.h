/**
 *******************************************************************************
 * @file    uart1.h
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-09-11
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

/** Define to prevent recursive inclusion ------------------------------------*/
#ifndef UART1_H_
#define UART1_H_

/** Includes -----------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include <stdbool.h>

/** Global variables ---------------------------------------------------------*/
UART_HandleTypeDef UART_Handle;

/** Defines ------------------------------------------------------------------*/
/** Typedefs -----------------------------------------------------------------*/
/** Function prototypes ------------------------------------------------------*/
ErrorStatus UART1_Init();
uint32_t UART1_BytesAvailable();
void UART1_GetDataFromBuffer(uint8_t* pStorage, uint32_t Size);
uint8_t UART1_GetByteFromBuffer();
void UART1_SendByte(uint8_t Byte);
void UART1_SendBuffer(uint8_t* pData, uint16_t Size);
void UART1_DataReceivedHandler();

#endif /* UART1_H_ */
