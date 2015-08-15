/**
 *******************************************************************************
 * @file  relay.h
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
#ifndef RELAY_H_
#define RELAY_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
typedef enum
{
	RelayStatus_NotEnoughTimePassed,
	RelayStatus_Ok,
} RelayStatus;

typedef enum
{
	RelayState_Off,
	RelayState_On,
} RelayState;

typedef struct
{
	GPIO_TypeDef* gpioPort;				/* GPIO port */
	uint16_t gpioPin;					/* GPIO pin */
	RelayState startState;				/* The state which the relay should start in */
	TickType_t lastStateChangeTick;		/* The tick when the relay changed state the last time */
	uint32_t msBetweenStateChange;		/* ms that need to pass between state changes */
} RelayDevice;

/* Function prototypes -------------------------------------------------------*/
void RELAY_Init(RelayDevice* Device);
RelayStatus RELAY_SetState(RelayDevice* Device, RelayState NewState);
RelayState RELAY_GetState(RelayDevice* Device);
RelayStatus RELAY_ToggleState(RelayDevice* Device);


#endif /* RELAY_H_ */
