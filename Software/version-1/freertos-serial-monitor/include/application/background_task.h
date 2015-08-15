/**
 ******************************************************************************
 * @file	background_task.h
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-08-22
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
#ifndef BACKGROUND_TASK_H_
#define BACKGROUND_TASK_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "messages.h"
#include "spi_flash_memory_map.h"

/* Defines -------------------------------------------------------------------*/
#define backgroundLED_0		(GPIO_PIN_13)
#define backgroundLED_1		(GPIO_PIN_2)
#define backgroundLED_2		(GPIO_PIN_3)

/* Typedefs ------------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/
void backgroundTask(void *pvParameters);


#endif /* BACKGROUND_TASK_H_ */
