/**
 ******************************************************************************
 * @file	gpio0_task.c
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

/* Includes ------------------------------------------------------------------*/
#include "gpio0_task.h"

/* Private defines -----------------------------------------------------------*/
#define GPIO0_PORT				(GPIOB)
#define GPIO0_PIN				(GPIO_PIN_0)
#define GPIO0_DIRECTION_PORT	(GPIOC)
#define GPIO0_DIRECTION_PIN		(GPIO_PIN_0)

/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void prvHardwareInit();

/* Functions -----------------------------------------------------------------*/
/**
 * @brief	The main task for the GPIO0 channel
 * @param	pvParameters:
 * @retval	None
 */
void gpio0Task(void *pvParameters)
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
		vTaskDelayUntil(&xNextWakeTime, 1000 / portTICK_PERIOD_MS);
	}
}

/**
 * @brief	Sets the direction of the GPIO
 * @param	Direction: The direction to use, can be any value of GPIO0Direction
 * @retval	None
 */
void gpio0SetDirection(GPIO0Direction Direction)
{
	/* Reset GPIO first */
	HAL_GPIO_DeInit(GPIO0_PORT, GPIO0_PIN);

	/* Init with new direction */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin  	= GPIO0_PIN;
	GPIO_InitStructure.Mode  	= GPIO_MODE_INPUT;
	GPIO_InitStructure.Pull		= GPIO_NOPULL;
	GPIO_InitStructure.Speed 	= GPIO_SPEED_HIGH;
	if (Direction == GPIO0Direction_Input)
		GPIO_InitStructure.Mode  	= GPIO_MODE_INPUT;
	else if (Direction == GPIO0Direction_Input)
		GPIO_InitStructure.Mode  	= GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(GPIO0_PORT, &GPIO_InitStructure);

	/* Set the direction pin */
	if (Direction == GPIO0Direction_Input)
		HAL_GPIO_WritePin(GPIO0_DIRECTION_PORT, GPIO0_DIRECTION_PIN, GPIO_PIN_SET);
	else if (Direction == GPIO0Direction_Output)
		HAL_GPIO_WritePin(GPIO0_DIRECTION_PORT, GPIO0_DIRECTION_PIN, GPIO_PIN_RESET);
}

/* Private functions .--------------------------------------------------------*/
/**
 * @brief	Initializes the hardware
 * @param	None
 * @retval	None
 */
static void prvHardwareInit()
{
	__GPIOB_CLK_ENABLE();
	__GPIOC_CLK_ENABLE();

	/* Direction pin */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin  	= GPIO0_DIRECTION_PIN;
	GPIO_InitStructure.Mode  	= GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull		= GPIO_NOPULL;
	GPIO_InitStructure.Speed 	= GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIO0_DIRECTION_PORT, &GPIO_InitStructure);

	/* Set as input as default */
	gpio0SetDirection(GPIO0Direction_Input);
}

/* Interrupt Handlers --------------------------------------------------------*/
