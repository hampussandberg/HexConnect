/**
 ******************************************************************************
 * @file	can1_task.c
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
#include "can1_task.h"

#include "relay.h"

/* Private defines -----------------------------------------------------------*/
/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static RelayDevice switchRelay = {
		.gpioPort = GPIOE,
		.gpioPin = GPIO_PIN_2,
		.startState = RelayState_Off,
		.msBetweenStateChange = 1000};
static RelayDevice terminationRelay = {
		.gpioPort = GPIOE,
		.gpioPin = GPIO_PIN_3,
		.startState = RelayState_Off,
		.msBetweenStateChange = 1000};

/* Private function prototypes -----------------------------------------------*/
static void prvHardwareInit();

/* Functions -----------------------------------------------------------------*/
/**
 * @brief	The main task for the CAN1 channel
 * @param	pvParameters:
 * @retval	None
 */
void can1Task(void *pvParameters)
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
 * @brief	Set whether or not to use termination on the output
 * @param	Termination: Can be any value of CAN1Termination
 * @retval	SUCCES: Everything went ok
 * @retval	ERROR: Something went wrong
 */
ErrorStatus can1SetTermination(CAN1Termination Termination)
{
	RelayStatus status;
	if (Termination == CAN1Termination_Connected)
		status = RELAY_SetState(&terminationRelay, RelayState_On);
	else if (Termination == CAN1Termination_Disconnected)
		status = RELAY_SetState(&terminationRelay, RelayState_Off);

	if (status == RelayStatus_Ok)
		return SUCCESS;
	else
		return ERROR;
}

/**
 * @brief	Set whether or not the output should be connected to the connector
 * @param	Connection: Can be any value of CAN1Connection
 * @retval	SUCCES: Everything went ok
 * @retval	ERROR: Something went wrong
 */
ErrorStatus can1SetConnection(CAN1Connection Connection)
{
	RelayStatus status;
	if (Connection == CAN1Connection_Connected)
		status = RELAY_SetState(&switchRelay, RelayState_On);
	else if (Connection == CAN1Connection_Disconnected)
		status = RELAY_SetState(&switchRelay, RelayState_Off);

	if (status == RelayStatus_Ok)
		return SUCCESS;
	else
		return ERROR;
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
	RELAY_Init(&terminationRelay);
}

/* Interrupt Handlers --------------------------------------------------------*/
