/**
 *******************************************************************************
 * @file    messages.h
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-08-16
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
#ifndef MESSAGES_H_
#define MESSAGES_H_

/** Includes -----------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

/** Defines ------------------------------------------------------------------*/
/** Typedefs -----------------------------------------------------------------*/
typedef enum
{
  LCDEvent_TouchEvent,      /* data[0]=x, data[1]=y, data[2]=FT5206Event, data[3]=FT5206Point_n */
  LCDEvent_TemperatureData, /* data[0]=temperature as a float */
  LCDEvent_DebugMessage,    /* data[0]=timestamp in ticks, data[1]=pointer to the message */
} LCDEvent;

typedef struct
{
  LCDEvent event;
  uint32_t data[8];
} LCDEventMessage;

/** Global Queues ------------------------------------------------------------*/
QueueHandle_t xLCDEventQueue;

/** Function prototypes ------------------------------------------------------*/
//static void MESSAGES_SendDebugMessage(uint8_t* pMessage)
//{
//  LCDEventMessage message;
//  message.event = LCDEvent_DebugMessage;
//  message.data[0] = xTaskGetTickCount();
//  message.data[1] = (uint32_t)pMessage;
//  xQueueSendToBack(xLCDEventQueue, &message, 100);
//}

#endif /* MESSAGES_H_ */
