/**
 *******************************************************************************
 * @file  gpio1_task.h
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
#ifndef GPIO1_TASK_H_
#define GPIO1_TASK_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

#include <stdbool.h>

#include "gpio_common.h"

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/
void gpio1Task(void *pvParameters);
void gpio1SetDirection(GPIODirection Direction);
GPIODirection gpio1GetDirection();
GPIO_PinState gpio1ReadPin();
void gpio1WritePin(GPIO_PinState PinState);
void gpio1TogglePin();
void gpio1SetPwmDuty(float Duty);
float gpio1GetPwmDuty();
void gpio1SetFrequency(uint32_t Frequency);
uint32_t gpio1GetFrequency();
uint32_t gpio1GetMaxFrequency();
void gpio1Enable();
void gpio1Disable();
bool gpio1IsEnabled();
GPIOSettings* gpio1GetSettings();

#endif /* GPIO1_TASK_H_ */
