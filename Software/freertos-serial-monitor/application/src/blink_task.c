/**
 ******************************************************************************
 * @file	blink_task.c
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

/* Includes ------------------------------------------------------------------*/
#include "blink_task.h"

/* Private defines -----------------------------------------------------------*/
/* LEDs on STM32F4 Discovery Board */
#define mainLED_3	GPIO_PIN_13
#define mainLED_4	GPIO_PIN_12
#define mainLED_5	GPIO_PIN_14
#define mainLED_6	GPIO_PIN_15

/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void prvHardwareInit();

/* Functions -----------------------------------------------------------------*/
/**
 * @brief	Text
 * @param	None
 * @retval	None
 */
void blinkTask(void *pvParameters)
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
		/* LED on for 25 ms */
		HAL_GPIO_WritePin(GPIOD, mainLED_6, GPIO_PIN_SET);
		vTaskDelayUntil(&xNextWakeTime, 25 / portTICK_PERIOD_MS);

		/* LED off for 1000 ms */
		HAL_GPIO_WritePin(GPIOD, mainLED_6, GPIO_PIN_RESET);
		vTaskDelayUntil(&xNextWakeTime, 1000 / portTICK_PERIOD_MS);
	}
}

/* Private functions .--------------------------------------------------------*/
/**
 * @brief	Initializes the hardware
 * @param	None
 * @retval	None
 */
static void prvHardwareInit()
{
	/* Set up the LED outputs */
	// Enable clock for GPIOD
	__GPIOD_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin  	= mainLED_3 | mainLED_4 | mainLED_5 | mainLED_6;
	GPIO_InitStructure.Mode  	= GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull		= GPIO_NOPULL;
	GPIO_InitStructure.Speed 	= GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

	HAL_GPIO_WritePin(GPIOD, mainLED_3, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD, mainLED_4, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD, mainLED_5, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD, mainLED_6, GPIO_PIN_RESET);
}

/* Interrupt Handlers --------------------------------------------------------*/
