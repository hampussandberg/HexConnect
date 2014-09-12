/**
 ******************************************************************************
 * @file	spi_flash.h
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
#ifndef SPI_FLASH_H_
#define SPI_FLASH_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdbool.h>

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/
ErrorStatus SPI_FLASH_Init();
uint32_t SPI_FLASH_ReadID();
void SPI_FLASH_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddress, uint32_t NumByteToWrite);
void SPI_FLASH_WriteByte(uint32_t WriteAddress, uint8_t Byte);
void SPI_FLASH_WriteByteFromISR(uint32_t WriteAddress, uint8_t Byte);
void SPI_FLASH_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddress, uint32_t NumByteToRead);
void SPI_FLASH_EraseSector(uint32_t SectorAddress);
void SPI_FLASH_EraseBulk();

#endif /* SPI_FLASH_H_ */
