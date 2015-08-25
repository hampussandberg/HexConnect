/**
 *******************************************************************************
 * @file    sdram.h
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-08-23
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
#ifndef SDRAM_H_
#define SDRAM_H_

/** Includes -----------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

#include <stdbool.h>

/** Defines ------------------------------------------------------------------*/
#define SDRAM_BANK_ADDR     ((uint32_t)0xD0000000)  /* Bank 2 */
#define SDRAM_SIZE          (0x10000000)            /* 256 MBit */
#define SDRAM_END           (SDRAM_BANK_ADDR + SDRAM_SIZE / 8)

/** Typedefs -----------------------------------------------------------------*/
/** Function prototypes ------------------------------------------------------*/
void SDRAM_Init();
void SDRAM_EraseAll(uint32_t EndAddress);
void SDRAM_WriteBuffer(uint32_t* pBuffer, uint32_t WriteAddress, uint32_t BufferSize);
void SDRAM_ReadBuffer(uint32_t* pBuffer, uint32_t ReadAddress, uint32_t BufferSize);

bool SDRAM_Test8bit(uint32_t EndAddress);
bool SDRAM_Test16bit(uint32_t EndAddress);
bool SDRAM_Test32bit(uint32_t EndAddress);


#endif /* SDRAM_H_ */
