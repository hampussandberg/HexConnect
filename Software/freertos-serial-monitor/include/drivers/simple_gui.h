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
	GUIButtonState_Enabled,
	GUIButtonState_Disabled,
	GUIButtonState_TouchUp,
	GUIButtonState_TouchDown,
} GUIButtonState;

typedef enum
{
	GUILayer_0,
	GUILayer_1,
} GUILayer;

typedef enum
{
	GUIDisplayState_NotHidden,
	GUIDisplayState_Hidden,
} GUIDisplayState;

typedef enum
{
	GUIBorder_NoBorder = 0x00,
	GUIBorder_Left = 0x01,
	GUIBorder_Right = 0x02,
	GUIBorder_Top = 0x04,
	GUIBorder_Bottom = 0x08,
} GUIBorder;

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
	GUILayer layer;

	GUIDisplayState hidden;

	/* Border */
	GUIBorder border;
	uint32_t borderThickness;
	uint16_t borderColor;

} GUIObject;

typedef struct
{
	/* Basic information about the object */
	GUIObject object;

	/* Colors */
	uint16_t enabledTextColor;
	uint16_t disabledTextColor;
	uint16_t enabledBackgroundColor;
	uint16_t disabledBackgroundColor;
	uint16_t pressedTextColor;
	uint16_t pressedBackgroundColor;

	GUIButtonState state;

	/* Text */
	uint8_t *text;
	LCD_FontEnlargement_TypeDef textSize;

	uint8_t numOfChar;		/* These three are calculated automatically in GUI_AddButton */
	uint8_t textWidth;		/* --------------------------------------------------------- */
	uint8_t textHeight;		/* --------------------------------------------------------- */

} GUIButton;

typedef struct
{
	/* Basic information about the object */
	GUIObject object;

	LCD_FontEnlargement_TypeDef textSize;

	/* Position where the next character will be written. Referenced from the objects origin (xPos, yPos) */
	uint16_t xWritePos;
	uint16_t yWritePos;

} GUITextBox;

/* Function prototypes -------------------------------------------------------*/
void GUI_DrawBorder(GUIObject Object);
void GUI_RedrawLayer(GUILayer Layer);

/* Button functions */
void GUI_AddButton(GUIButton* Button);
void GUI_DrawButton(uint32_t ButtonId);
void GUI_DrawAllButtons();
void GUI_SetButtonState(uint32_t ButtonId, GUIButtonState State);
void GUI_CheckAllNonHiddenButtonsForTouchUpEventAt(uint16_t XPos, uint16_t YPos);

/* Text box functions */
void GUI_AddTextBox(GUITextBox* TextBox);
void GUI_DrawTextBox(uint32_t TextBoxId);
void GUI_DrawAllTextBoxes();
void GUI_WriteStringInTextBox(uint32_t TextBoxId, uint8_t* String);
void GUI_WriteNumberInTextBox(uint32_t TextBoxId, int32_t Number);
void GUI_SetWritePosition(uint32_t TextBoxId, uint16_t XPos, uint16_t YPos);
void GUI_ClearTextBox(uint32_t TextBoxId);

#endif /* SIMPLE_GUI_H_ */
