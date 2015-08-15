/**
 *******************************************************************************
 * @file  can2_task.h
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
#ifndef CAN2_TASK_H_
#define CAN2_TASK_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "timers.h"

#include "can_common.h"

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/
void can2Task(void *pvParameters);
bool can2IsDoneInitializing();
void can2Restart();
ErrorStatus can2SetTermination(CANTermination Termination);
ErrorStatus can2SetConnection(CANConnection Connection);
CANSettings* can2GetSettings();
ErrorStatus can2UpdateWithNewSettings();
SemaphoreHandle_t* can2GetSettingsSemaphore();
uint32_t can2GetCurrentWriteAddress();
ErrorStatus can2Clear();
void can2ClearFlash();

void can2TxCpltCallback();
void can2RxCpltCallback();
void can2ErrorCallback();


#endif /* CAN2_TASK_H_ */
