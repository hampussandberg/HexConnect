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
typedef struct
{
	uint8_t SPI_Channel;		/* Channel for the SPI periperal */
	SPI_TypeDef* SPIx;			/* SPI peripheral to use */

	uint8_t receivedByte;

	SemaphoreHandle_t xTxSemaphore;
	SemaphoreHandle_t xRxSemaphore;

	bool initialized;			/* Set to true if the initialization has been done successfully */
} SPI_Device;

/* Function prototypes -------------------------------------------------------*/
void SPI_Device_InitDefault(SPI_Device* SPIDevice);
void SPI_InitWithStructure(SPI_Device* SPIDevice, SPI_InitTypeDef* SPI_InitStructure);
uint8_t SPI_WriteRead(SPI_Device* SPIDevice, uint8_t Data);

void SPI_Interrupt(SPI_Device* SPIDevice);

#endif /* SPI_H_ */
