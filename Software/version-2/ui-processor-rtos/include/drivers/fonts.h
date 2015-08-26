/**
 *******************************************************************************
 * @file    fonts.h
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-08-25
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
#ifndef FONTS_H
#define FONTS_H

/** Includes -----------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdbool.h>

/** Typedefs -----------------------------------------------------------------*/
typedef struct
{
  const uint8_t* DataTable;
  const uint8_t* WidthTable;
  const uint32_t* OffsetTable;
  const uint8_t SpaceWidth;
  const uint16_t Height;
  const uint8_t firstValidCharacter;
  const uint8_t lastValidCharacter;
  const bool fixedWidth;
} FONT;

/** Defines ------------------------------------------------------------------*/
extern FONT font8x16_fixedWidth;
extern FONT font8x16_fixedWidth_bold;
extern FONT font16x32_fixedWidth;

extern FONT font_12pt_variableWidth;
extern FONT font_15pt_variableWidth;
extern FONT font_15pt_bold_variableWidth;
extern FONT font_18pt_variableWidth;
extern FONT font_24pt_variableWidth;


/** Function prototypes ------------------------------------------------------*/
uint32_t FONTS_GetAddressForCharacterWithFont(char Character, FONT* Font);
uint8_t FONTS_GetWidthForCharacter(char Character, FONT* Font);
uint8_t FONTS_GetOffsetForCharacter(char Character, FONT* Font);
void FONTS_GetAddressAndWidthForCharacter(uint32_t* Address, uint8_t* Width, char Character, FONT* Font);
uint32_t FONTS_GetTotalWidthForString(char* String, FONT* Font);

#endif /* FONTS_H */
