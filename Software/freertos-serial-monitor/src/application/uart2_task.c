/**
 ******************************************************************************
 * @file	uart2_task.c
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
#include "uart2_task.h"

#include "relay.h"

/* Private defines -----------------------------------------------------------*/
/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static RelayDevice switchRelay = {
		.gpioPort = GPIOC,
		.gpioPin = GPIO_PIN_1,
		.startState = RelayState_Off,
		.msBetweenStateChange = 1000};
static RelayDevice powerRelay = {
		.gpioPort = GPIOE,
		.gpioPin = GPIO_PIN_1,
		.startState = RelayState_Off,
		.msBetweenStateChange = 1000};

/* Private function prototypes -----------------------------------------------*/
static void prvHardwareInit();

/* Functions -----------------------------------------------------------------*/
/**
 * @brief	The main task for the UART2 channel
 * @param	pvParameters:
 * @retval	None
 */
void uart2Task(void *pvParameters)
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
 * @brief	Set the power of the UART2
 * @param	Power: The power to set, UART2Power_3V3 or UART2Power_5V
 * @retval	None
 */
void uart2SetPower(UART2Power Power)
{
	if (Power == UART2Power_3V3)
		RELAY_SetState(&powerRelay, RelayState_On);
	else if (Power == UART2Power_5V)
		RELAY_SetState(&powerRelay, RelayState_Off);
}

/* Private functions .--------------------------------------------------------*/
/**
 * @brief	Initializes the hardware
 * @param	None
 * @retval	None
 */
static void prvHardwareInit()
{
	/* Init relays */
	RELAY_Init(&switchRelay);
	RELAY_Init(&powerRelay);
}

/* Interrupt Handlers --------------------------------------------------------*/
