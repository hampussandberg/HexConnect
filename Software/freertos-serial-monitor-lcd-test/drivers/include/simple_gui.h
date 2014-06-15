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

typedef enum
{
	LAYER0,
	LAYER1
} GUI_Layer_TypeDef;

typedef enum
{
	NOT_HIDDEN,
	HIDDEN
} GUI_Hidden_TypeDef;

typedef enum
{
	NO_BORDER = 0x00,
	BORDER_LEFT = 0x01,
	BORDER_RIGHT = 0x02,
	BORDER_TOP = 0x04,
	BORDER_BOTTOM = 0x08
} GUI_Border_TypeDef;

typedef struct
{
	/* Unique ID set in simple_gui_config.h for each GUI object */
	uint32_t id;

	/* Position and size */
	uint16_t xPos;
	uint16_t yPos;
	uint16_t width;
	uint16_t height;

	/* Layer where the object is */
	GUI_Layer_TypeDef layer;

	GUI_Hidden_TypeDef hidden;

	/* Border */
	GUI_Border_TypeDef border;
	uint32_t borderThickness;
	uint16_t borderColor;

} GUI_Object_TypeDef;

typedef struct
{
	/* Basic information about the object */
	GUI_Object_TypeDef object;

	/* Colors */
	uint16_t enabledTextColor;
	uint16_t disabledTextColor;
	uint16_t enabledBackgroundColor;
	uint16_t disabledBackgroundColor;
	uint16_t pressedTextColor;
	uint16_t pressedBackgroundColor;

	GUI_ButtonState_TypeDef state;

	/* Text */
	uint8_t *text;
	LCD_FontEnlargement_TypeDef textSize;

	uint8_t numOfChar;		/* These three are calculated automatically in GUI_AddButton */
	uint8_t textWidth;		/* --------------------------------------------------------- */
	uint8_t textHeight;		/* --------------------------------------------------------- */

} GUIButton_TypeDef;

typedef struct
{
	/* Basic information about the object */
	GUI_Object_TypeDef object;

	LCD_FontEnlargement_TypeDef textSize;

	/* Position where the next character will be written. Referenced from the objects origin (xPos, yPos) */
	uint16_t xWritePos;
	uint16_t yWritePos;

} GUI_TextBox_TypeDef;

/* Function prototypes -------------------------------------------------------*/
void GUI_DrawBorder(GUI_Object_TypeDef Object);
void GUI_RedrawLayer(GUI_Layer_TypeDef Layer);

/* Button functions */
void GUI_AddButton(GUIButton_TypeDef* Button);
void GUI_DrawButton(uint32_t ButtonId);
void GUI_DrawAllButtons();
void GUI_SetButtonState(uint32_t ButtonId, GUI_ButtonState_TypeDef State);
void GUI_CheckButtonTouchUpEvent(uint16_t XPos, uint16_t YPos);

/* Text box functions */
void GUI_AddTextBox(GUI_TextBox_TypeDef* TextBox);
void GUI_DrawTextBox(uint32_t TextBoxId);
void GUI_DrawAllTextBoxes();
void GUI_WriteStringInTextBox(uint32_t TextBoxId, uint8_t* String);
void GUI_WriteNumberInTextBox(uint32_t TextBoxId, int32_t Number);
void GUI_SetWritePosition(uint32_t TextBoxId, uint16_t XPos, uint16_t YPos);
void GUI_ClearTextBox(uint32_t TextBoxId);

#endif /* SIMPLE_GUI_H_ */
