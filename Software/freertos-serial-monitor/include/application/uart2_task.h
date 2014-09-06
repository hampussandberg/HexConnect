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
#include "FreeRTOS.h"
#include "task.h"

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
typedef enum
{
	UART2Power_5V,
	UART2Power_3V3,
} UART2Power;

typedef enum
{
	UART2Connection_Connected,
	UART2Connection_Disconnected,
} UART2Connection;

/* Function prototypes -------------------------------------------------------*/
void uart2Task(void *pvParameters);
void uart2SetPower(UART2Power Power);
void uart2SetConnection(UART2Connection Connection);


#endif /* UART2_TASK_H_ */
