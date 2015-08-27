/**
 *******************************************************************************
 * @file    color.h
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-08-28
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
#ifndef COLOR_H_
#define COLOR_H_

/** Includes -----------------------------------------------------------------*/
#include "stm32f4xx.h"

/** Defines ------------------------------------------------------------------*/
/** Typedefs -----------------------------------------------------------------*/
typedef struct
{
	uint16_t hue;
	float saturation;
	float brightness;
} HSB_TypeDef;

typedef struct
{
	uint8_t red;	/* 5-bits used for red */
	uint8_t green;	/* 6-bits used for green */
	uint8_t blue;	/* 5-bits used for blue */
} RGB565_TypeDef;

typedef struct
{
	uint8_t red;	/* 8-bits used for red */
	uint8_t green;	/* 8-bits used for green */
	uint8_t blue;	/* 8-bits used for blue */
} RGB888_TypeDef;

/** Function prototypes ------------------------------------------------------*/
void COLOR_HSBtoRGB888(HSB_TypeDef* HSB, RGB888_TypeDef* RGB);
uint16_t COLOR_HSBtoRGB565_UIN16_T(HSB_TypeDef* HSB);
uint32_t COLOR_HSBtoRGB888_UINT32_T(HSB_TypeDef* HSB);
uint32_t COLOR_RGB565ToARGB8888(uint16_t Rgb565);

#endif /* COLOR_H_ */
