/**
 ******************************************************************************
 * @file	color.h
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-06-02
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
#ifndef COLOR_H_
#define COLOR_H_

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "stm32f4xx_hal.h"

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
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

/* Function prototypes -------------------------------------------------------*/
void COLOR_HSBtoRGB565(HSB_TypeDef* HSB, RGB565_TypeDef* RGB);

#endif /* COLOR_H_ */
