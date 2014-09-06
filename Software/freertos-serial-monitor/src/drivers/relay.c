/**
 ******************************************************************************
 * @file	relay.c
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
#include "relay.h"

/* Private defines -----------------------------------------------------------*/
/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Functions -----------------------------------------------------------------*/
/**
 * @brief	Text
 * @param	None
 * @retval	None
 */
void RELAY_Init(RelayDevice* Device)
{
	/* Enable all GPIO clocks */
	/* TODO: Do this case by case instead */
	__GPIOA_CLK_ENABLE();
	__GPIOB_CLK_ENABLE();
	__GPIOC_CLK_ENABLE();
	__GPIOD_CLK_ENABLE();
	__GPIOE_CLK_ENABLE();

	/* Initialize the GPIO */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin  	= Device->gpioPin;
	GPIO_InitStructure.Mode  	= GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull		= GPIO_NOPULL;
	GPIO_InitStructure.Speed 	= GPIO_SPEED_HIGH;
	HAL_GPIO_Init(Device->gpioPort, &GPIO_InitStructure);

	/* Go to start state */
	if (Device->startState == RelayState_On)
		HAL_GPIO_WritePin(Device->gpioPort, Device->gpioPin, GPIO_PIN_SET);
	else if (Device->startState == RelayState_Off)
		HAL_GPIO_WritePin(Device->gpioPort, Device->gpioPin, GPIO_PIN_RESET);
}

/**
 * @brief	Sets the state of a relay
 * @param	Device: The device to set
 * @retval	RelayStatus_Ok: Everything went fine
 * @retval	RelayStatus_NotEnoughTimePassed: Not enough time has passed
 */
RelayStatus RELAY_SetState(RelayDevice* Device, RelayState NewState)
{
	/* Make sure we don't change state if not enough time has passed since the last time */
	if (xTaskGetTickCount() - Device->lastStateChangeTick >= Device->msBetweenStateChange)
	{
		if (NewState == RelayState_On)
		{
			HAL_GPIO_WritePin(Device->gpioPort, Device->gpioPin, GPIO_PIN_SET);
		}
		else if (NewState == RelayState_Off)
		{
			HAL_GPIO_WritePin(Device->gpioPort, Device->gpioPin, GPIO_PIN_RESET);
		}
		Device->lastStateChangeTick = xTaskGetTickCount();
		return RelayStatus_Ok;
	}
	else
		return RelayStatus_NotEnoughTimePassed;
}

/**
 * @brief	Gets the state of a relay
 * @param	Device: The device to get the state on
 * @retval	RelayState_On: The relay is on
 * @retval	RelayState_Off: The relay is off
 */
RelayState RELAY_GetState(RelayDevice* Device)
{
	if (HAL_GPIO_ReadPin(Device->gpioPort, Device->gpioPin) == GPIO_PIN_SET)
		return RelayState_On;
	else
		return RelayState_Off;
}

/**
 * @brief	Toggle the state of a relay
 * @param	Device: The device to toggle
 * @retval	RelayStatus_Ok: Everything went fine
 * @retval	RelayStatus_NotEnoughTimePassed: Not enough time has passed
 */
RelayStatus RELAY_ToggleState(RelayDevice* Device)
{
	/* Make sure we don't change state if not enough time has passed since the last time */
	if (xTaskGetTickCount() - Device->lastStateChangeTick >= Device->msBetweenStateChange)
	{
		HAL_GPIO_TogglePin(Device->gpioPort, Device->gpioPin);
		Device->lastStateChangeTick = xTaskGetTickCount();
		return RelayStatus_Ok;
	}
	else
		return RelayStatus_NotEnoughTimePassed;
}

/* Interrupt Handlers --------------------------------------------------------*/
