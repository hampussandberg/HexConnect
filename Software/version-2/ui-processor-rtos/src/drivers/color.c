/**
 *******************************************************************************
 * @file    color.c
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-08-27
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

/** Includes -----------------------------------------------------------------*/
#include "color.h"

/** Private defines ----------------------------------------------------------*/
/** Private typedefs ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
/** Private function prototypes ----------------------------------------------*/
/** Functions ----------------------------------------------------------------*/
/**
 * @brief	  Convert HSB to RGB565
 * @param	  HSB: Pointer to a HSB struct
 * @param	  RGB: Pointer to a RGB565 struct
 * @retval	None
 */
void COLOR_HSBtoRGB888(HSB_TypeDef* HSB, RGB888_TypeDef* RGB)
{
	float saturation = HSB->saturation;
	float brightness = HSB->brightness;
	float h_i, f, p, q, t, R, G, B;

	uint16_t hue = HSB->hue % 360;
	h_i = hue / 60;
	f = (float)(hue) / 60.0 - h_i;
	p = brightness * (1 - saturation);
	q = brightness * (1 - saturation * f);
	t = brightness * (1 - saturation * (1 - f));

	if (h_i == 0)
	{
		R = brightness;
		G = t;
		B = p;
	}
	else if (h_i == 1)
	{
		R = q;
		G = brightness;
		B = p;
	}
	else if (h_i == 2)
	{
		R = p;
		G = brightness;
		B = t;
	}
	else if (h_i == 3)
	{
		R = p;
		G = q;
		B = brightness;
	}
	else if (h_i == 4)
	{
		R = t;
		G = p;
		B = brightness;
	}
	else {
		R = brightness;
		G = p;
		B = q;
	}

	RGB->red = (uint8_t)(R * 255.0);
	RGB->green = (uint8_t)(G * 255.0);
	RGB->blue = (uint8_t)(B * 255.0);
}

uint16_t COLOR_HSBtoRGB565_UIN16_T(HSB_TypeDef* HSB)
{
	RGB888_TypeDef tempRgb888;
	COLOR_HSBtoRGB888(HSB, &tempRgb888);

	return (((tempRgb888.red & 0x1F) << 11) | ((tempRgb888.green & 0x3F) << 5) | (tempRgb888.blue & 0x1F));
}

uint32_t COLOR_HSBtoRGB888_UINT32_T(HSB_TypeDef* HSB)
{
	RGB888_TypeDef tempRgb888;
	COLOR_HSBtoRGB888(HSB, &tempRgb888);

	return (((tempRgb888.red) << 16) | ((tempRgb888.green) << 8) | (tempRgb888.blue));
}

/**
 * @brief   Convert RGB565 to ARGB8888
 * @param   Rgb565: The RGB565 color
 * @retval  The color in ARGB8888 format
 */
uint32_t COLOR_RGB565ToARGB8888(uint16_t Rgb565)
{
  return ((Rgb565 & 0xF800) << 8) | ((Rgb565 & 0x07E0) << 5) | ((Rgb565 & 0x001F) << 3);
}

/** Private functions --------------------------------------------------------*/
/** Interrupt Handlers -------------------------------------------------------*/
