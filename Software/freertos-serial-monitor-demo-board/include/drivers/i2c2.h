/**
 ******************************************************************************
 * @file	i2c2.h
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-09-07
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
