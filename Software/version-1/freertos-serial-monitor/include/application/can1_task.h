/**
 *******************************************************************************
 * @file  can1_task.h
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
#ifndef CAN1_TASK_H_
#define CAN1_TASK_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "messages.h"
#include "can_common.h"

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/
void can1Task(void *pvParameters);
bool can1IsDoneInitializing();
void can1Restart();
ErrorStatus can1SetTermination(CANTermination Termination);
ErrorStatus can1SetConnection(CANConnection Connection);
CANSettings* can1GetSettings();
ErrorStatus can1UpdateWithNewSettings();
SemaphoreHandle_t* can1GetSettingsSemaphore();

uint32_t can1GetCurrentWriteAddress();
ErrorStatus can1Clear();
void can1ClearFlash();

ErrorStatus can1Transmit(uint32_t MessageId, uint8_t* pData, CANDataLength DataLength, uint32_t Timeout);

void can1TxCpltCallback();
void can1RxCpltCallback();
void can1ErrorCallback();

#endif /* CAN1_TASK_H_ */
