/**
 ******************************************************************************
 * @file	millis.h
 * @author	Hampus Sandberg
 * @version	0.1
 * @date  2015-08-15
 * @brief	Manage a millisecond counter
 ******************************************************************************
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
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MILLIS_H_
#define MILLIS_H_

/* Includes ------------------------------------------------------------------*/
#if defined(STM32F40_41xxx)
#include "stm32f4xx.h"
#elif defined(STM32F10X_MD) || defined(STM32F10X_MD_VL)
#include "stm32f10x.h"
#endif

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/
void MILLIS_Init(void);
uint32_t millis(void);
void millisDelay(uint32_t);

#endif /* MILLIS_H_ */
