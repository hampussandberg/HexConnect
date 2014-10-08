/**
 ******************************************************************************
 * @file	buzzer.h
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-10-08
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
#ifndef BUZZER_H_
#define BUZZER_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "timers.h"

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
typedef enum
{
	BUZZERVolume_Off = 0,
	BUZZERVolume_Low = 1,
	BUZZERVolume_Normal = 10,
	BUZZERVolume_Max = 50,
} BUZZERVolume;

typedef struct
{
	uint32_t volume;				/* Can be any value between 0 and 50 or BUZZERVolume */
	uint32_t frequency;				/* In Hertz (Hz) */
	uint32_t delayBetweenBeeps;		/* In ms */
	uint32_t numOfBeeps;
} BUZZERSettings;

/* Function prototypes -------------------------------------------------------*/
void BUZZER_Init();
void BUZZER_DeInit();
void BUZZER_SetVolume(uint32_t Volume);
void BUZZER_Off();
void BUZZER_SetFrequency(uint32_t Frequency);
void BUZZER_BeepNumOfTimes(uint32_t NumOfBeeps);

#endif /* BUZZER_H_ */
