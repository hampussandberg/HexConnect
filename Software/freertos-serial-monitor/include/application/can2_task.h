/**
 ******************************************************************************
 * @file	can2_task.h
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
#ifndef CAN2_TASK_H_
#define CAN2_TASK_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
typedef enum
{
	CAN2Termination_Connected,
	CAN2Termination_Disconnected,
} CAN2Termination;

typedef enum
{
	CAN2Connection_Connected,
	CAN2Connection_Disconnected,
} CAN2Connection;

/* Function prototypes -------------------------------------------------------*/
void can2Task(void *pvParameters);
ErrorStatus can2SetTermination(CAN2Termination Termination);
ErrorStatus can2SetConnection(CAN2Connection Connection);


#endif /* CAN2_TASK_H_ */
