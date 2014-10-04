/**
 ******************************************************************************
 * @file	spi.h
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-08-23
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
#ifndef SPI_H_
#define SPI_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "semphr.h"

#include <stdbool.h>

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
typedef enum
{
	SPIChannel_FLASH,
	SPIChannel_ADC,
	SPIChannel_Thermocouple,
} SPIChannel;

/* Function prototypes -------------------------------------------------------*/
void SPI_Init(SPIChannel Channel);
uint8_t SPI_SendReceiveByte(SPIChannel Channel, uint8_t Byte);

void SPI_SelectChannel(SPIChannel Channel);
void SPI_DeselectChannel(SPIChannel Channel);

#endif /* SPI_H_ */
