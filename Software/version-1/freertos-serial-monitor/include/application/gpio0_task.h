/**
 ******************************************************************************
 * @file	gpio0_task.h
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
#ifndef GPIO0_TASK_H_
#define GPIO0_TASK_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

#include <stdbool.h>

#include "gpio_common.h"

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/
void gpio0Task(void *pvParameters);
void gpio0SetDirection(GPIODirection Direction);
GPIODirection gpio0GetDirection();
GPIO_PinState gpio0ReadPin();
void gpio0WritePin(GPIO_PinState PinState);
void gpio0TogglePin();
void gpio0SetPwmDuty(float Duty);
float gpio0GetPwmDuty();
void gpio0SetFrequency(uint32_t Frequency);
uint32_t gpio0GetFrequency();
uint32_t gpio0GetMaxFrequency();
void gpio0Enable();
void gpio0Disable();
bool gpio0IsEnabled();
GPIOSettings* gpio0GetSettings();

#endif /* GPIO0_TASK_H_ */
