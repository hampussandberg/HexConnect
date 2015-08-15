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

#include "uart_common.h"
#include "messages.h"
#include "simple_gui.h"

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/
void rs232Task(void *pvParameters);
bool rs232IsDoneInitializing();
void rs232Restart();
ErrorStatus rs232SetConnection(UARTConnection Connection);
UARTSettings* rs232GetSettings();
ErrorStatus rs232UpdateWithNewSettings();
SemaphoreHandle_t* rs232GetSettingsSemaphore();
ErrorStatus rs232Clear();
uint32_t rs232GetCurrentWriteAddress();

void rs232Transmit(uint8_t* Data, uint32_t Size);
void rs232ClearFlash();

void rs232TxCpltCallback();
void rs232RxCpltCallback();
void rs232ErrorCallback();

#endif /* RS232_TASK_H_ */
