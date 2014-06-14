/**
 ******************************************************************************
 * @file	simple_gui.h
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-06-14
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
#ifndef SIMPLE_GUI_H_
#define SIMPLE_GUI_H_

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "stm32f4xx_hal.h"
#include "simple_gui_config.h"

#include "color.h"
#include "string.h"
#include "stdlib.h"

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
typedef enum
{
	ENABLED,
	DISABLED,
	PRESSED
} GUI_ButtonState_TypeDef;

typedef struct
{
	/* Colors */
	uint16_t enabledTextColor;
	uint16_t disabledTextColor;
	uint16_t enabledBackgroundColor;
	uint16_t disabledBackgroundColor;
	uint16_t pressedTextColor;
	uint16_t pressedBackgroundColor;

	GUI_ButtonState_TypeDef state;

	/* Position and size */
	uint16_t xPos;
	uint16_t yPos;
	uint16_t width;
	uint16_t height;

	/* Text */
	uint8_t *text;
	LCD_FontEnlargement_TypeDef textSize;

} GUIButton_TypeDef;

/* Function prototypes -------------------------------------------------------*/
void GUI_Func();

/* Button functions */
void GUI_AddButton(GUIButton_TypeDef* Button);
void GUI_DrawButton(uint32_t ButtonIndex);
void GUI_DrawAllButtons();
void GUI_SetButtonState(uint32_t ButtonIndex, GUI_ButtonState_TypeDef State);

#endif /* SIMPLE_GUI_H_ */
