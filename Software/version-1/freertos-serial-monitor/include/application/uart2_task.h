/**
 *******************************************************************************
 * @file  uart2_task.h
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
#ifndef UART2_TASK_H_
#define UART2_TASK_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "timers.h"

#include "uart_common.h"
#include "messages.h"
#include "simple_gui.h"

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/
void uart2Task(void *pvParameters);
bool uart2IsDoneInitializing();
void uart2Restart();
ErrorStatus uart2SetPower(UARTPower Power);
ErrorStatus uart2SetConnection(UARTConnection Connection);
UARTSettings* uart2GetSettings();
ErrorStatus uart2UpdateWithNewSettings();
SemaphoreHandle_t* uart2GetSettingsSemaphore();
ErrorStatus uart2Clear();
uint32_t uart2GetCurrentWriteAddress();

void uart2Transmit(uint8_t* Data, uint32_t Size);
void uart2ClearFlash();

void uart2TxCpltCallback();
void uart2RxCpltCallback();
void uart2ErrorCallback();

#endif /* UART2_TASK_H_ */
