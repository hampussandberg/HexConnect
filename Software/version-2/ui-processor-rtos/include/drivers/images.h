/**
 ******************************************************************************
 * @file	images.h
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2015-04-16
 * @brief
 ******************************************************************************
	Copyright (c) 2015 Hampus Sandberg.

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
#ifndef IMAGES_H
#define IMAGES_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

/* Typedefs ------------------------------------------------------------------*/
typedef struct
{
	const uint8_t* DataTable;
	const uint32_t Width;
	const uint32_t Height;

} ALPHA_IMAGE;

typedef struct
{
	const uint32_t* DataTable;
	const uint32_t Width;
	const uint32_t Height;

} ARGB8888_IMAGE;

typedef struct
{
  const uint16_t* DataTable;
  const uint32_t Width;
  const uint32_t Height;

} RGB565_IMAGE;

/* Defines -------------------------------------------------------------------*/
extern RGB565_IMAGE splash_screen;

/* Function prototypes -------------------------------------------------------*/


#endif /* IMAGES_H */
