/**
 *******************************************************************************
 * @file    stm32f4xx_it.h
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-08-15
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

/** Define to prevent recursive inclusion ------------------------------------*/
#ifndef STM32F4XX_IT_H
#define STM32F4XX_IT_H

/** Includes -----------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/** Defines ------------------------------------------------------------------*/
/** Typedefs -----------------------------------------------------------------*/

/** Function prototypes ------------------------------------------------------*/
void EXTI0_IRQHandler(void);
void DMA2D_IRQHandler(void);

#endif /* STM32F4XX_IT_H */
