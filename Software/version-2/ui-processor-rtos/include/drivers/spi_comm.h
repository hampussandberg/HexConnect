/**
 *******************************************************************************
 * @file    spi_comm.h
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-08-16
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
#ifndef SPI_COMM_H_
#define SPI_COMM_H_

/** Includes -----------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include <stdbool.h>

/** Defines ------------------------------------------------------------------*/
/** Typedefs -----------------------------------------------------------------*/
/** Function prototypes ------------------------------------------------------*/
ErrorStatus SPI_COMM_Init();
uint32_t SPI_COMM_ReadID();
void SPI_COMM_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddress, uint32_t NumByteToWrite);
void SPI_COMM_WriteByte(uint32_t WriteAddress, uint8_t Byte);
void SPI_COMM_WriteByteFromISR(uint32_t WriteAddress, uint8_t Byte);
void SPI_COMM_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddress, uint32_t NumByteToRead);
ErrorStatus SPI_COMM_EraseSector(uint32_t SectorAddress);
void SPI_COMM_EraseChip();
bool SPI_COMM_Initialized();

#endif /* SPI_COMM_H_ */
