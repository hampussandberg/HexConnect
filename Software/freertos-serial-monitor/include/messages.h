/**
 ******************************************************************************
 * @file	messages.h
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-09-07
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
#ifndef MESSAGES_H_
#define MESSAGES_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "queue.h"

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
typedef enum
{
	LCDEvent_TouchEvent,	/* data[0]=x, data[1]=y, data[2]=FT5206Event, data[3]=FT5206Point_n */
} LCDEvent;

typedef struct
{
	LCDEvent event;
	uint32_t data[8];
} LCDEventMessage;

/* Global Queues -------------------------------------------------------------*/
QueueHandle_t xLCDEventQueue;

/* Function prototypes -------------------------------------------------------*/


#endif /* MESSAGES_H_ */
