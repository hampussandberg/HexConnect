/**
 *******************************************************************************
 * @file  i2c2.h
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
#ifndef I2C2_H_
#define I2C2_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "semphr.h"

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/
void I2C2_Init();
void I2C2_Transmit(uint8_t DevAddress, uint8_t *Data, uint16_t Size);
void I2C2_TransmitFromISR(uint8_t DevAddress, uint8_t* pBuffer, uint16_t Size);
void I2C2_Receive(uint8_t DevAddress, uint8_t *Data, uint16_t Size);
void I2C2_ReceiveFromISR(uint8_t DevAddress, uint8_t* pBuffer, uint16_t Size);

#endif /* I2C2_H_ */
