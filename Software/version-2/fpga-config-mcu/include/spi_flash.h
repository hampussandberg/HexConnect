/**
 *******************************************************************************
 * @file    spi_flash.h
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-09-06
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
#ifndef SPI_FLASH_H_
#define SPI_FLASH_H_

/** Includes -----------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include <stdbool.h>

/** Defines ------------------------------------------------------------------*/
#define SPI_FLASH_BYTES_IN_BLOCK    (64*1024)
#define SPI_FLASH_BYTES_IN_SECTOR   (4*1024)
#define SPI_FLASH_BYTES_IN_PAGE     (256)

/** Typedefs -----------------------------------------------------------------*/
/** Function prototypes ------------------------------------------------------*/
ErrorStatus SPI_FLASH_Init();
uint32_t SPI_FLASH_ReadID();
void SPI_FLASH_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddress, uint32_t NumByteToWrite);
void SPI_FLASH_WriteByte(uint32_t WriteAddress, uint8_t Byte);
void SPI_FLASH_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddress, uint32_t NumByteToRead);
ErrorStatus SPI_FLASH_EraseSector(uint32_t SectorAddress);
ErrorStatus SPI_FLASH_EraseBlock(uint32_t BlockAddress);
void SPI_FLASH_EraseChip();
bool SPI_FLASH_Initialized();

#endif /* SPI_FLASH_H_ */
