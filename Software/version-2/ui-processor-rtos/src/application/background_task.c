/**
 ******************************************************************************
 * @file	background_task.c
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

/** Includes -----------------------------------------------------------------*/
#include "background_task.h"

#include <string.h>

/** Private defines ----------------------------------------------------------*/
/** Private typedefs ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
/** Private function prototypes ----------------------------------------------*/
static void prvHardwareInit();

/** Functions ----------------------------------------------------------------*/
/**
 * @brief	Text
 * @param	None
 * @retval	None
 */
void backgroundTask(void *pvParameters)
{
	prvHardwareInit();

	/* The parameter in vTaskDelayUntil is the absolute time
	 * in ticks at which you want to be woken calculated as
	 * an increment from the time you were last woken. */
	TickType_t xNextWakeTime;
	/* Initialize xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

  while (1)
  {
      /* Toggle the LED every 500ms */
      HAL_GPIO_TogglePin(GPIOD, backgroundLED_0 | backgroundLED_1 | backgroundLED_2);
      vTaskDelayUntil(&xNextWakeTime, 500 / portTICK_PERIOD_MS);
  }
}

/** Private functions .-------------------------------------------------------*/
/**
 * @brief	Initializes the hardware
 * @param	None
 * @retval	None
 */
static void prvHardwareInit()
{
	/* Set up the LED outputs */
	__GPIOD_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin  	= backgroundLED_0 | backgroundLED_1 | backgroundLED_2;
	GPIO_InitStructure.Mode  	= GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull		= GPIO_NOPULL;
	GPIO_InitStructure.Speed 	= GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

	HAL_GPIO_WritePin(GPIOD, backgroundLED_0, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD, backgroundLED_1, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD, backgroundLED_2, GPIO_PIN_SET);
}

/** Interrupt Handlers -------------------------------------------------------*/
