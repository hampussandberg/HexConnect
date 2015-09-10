/**
 *******************************************************************************
 * @file    spi2.h
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-09-10
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
#ifndef SPI2_H_
#define SPI2_H_

/** Includes -----------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include <stdbool.h>

/** Defines ------------------------------------------------------------------*/
/** Typedefs -----------------------------------------------------------------*/
typedef enum
{
  SPI2_Device_SdCard,
  SPI2_Device_Fpga,
} SPI2_Device;

/** Function prototypes ------------------------------------------------------*/
ErrorStatus SPI2_InitGpio();
ErrorStatus SPI2_InitForFpgaConfig();
ErrorStatus SPI2_InitForSdCard();
ErrorStatus SPI2_DeInit();

void SPI2_WriteBuffer(uint8_t* pBuffer, uint32_t NumByteToWrite);
void SPI2_SelectDevice(SPI2_Device Device);
void SPI2_DeselectDevice(SPI2_Device Device);

#endif /* SPI_FLASH_H_ */
