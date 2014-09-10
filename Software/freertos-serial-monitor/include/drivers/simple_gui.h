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
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
typedef enum
{
	GUIButtonState_NoState,
	GUIButtonState_Enabled,
	GUIButtonState_Disabled,
	GUIButtonState_TouchUp,
	GUIButtonState_TouchDown,
} GUIButtonState;

typedef enum
{
	GUITouchEvent_Down,
	GUITouchEvent_Up,
} GUITouchEvent;

typedef enum
{
	GUILayer_0,
	GUILayer_1,
} GUILayer;

typedef enum
{
	GUIDisplayState_Hidden,
	GUIDisplayState_NotHidden,
	GUIDisplayState_ContentHidden,
	GUIDisplayState_NoState,
} GUIDisplayState;

typedef enum
{
	GUIBorder_NoBorder = 0x00,
	GUIBorder_Left = 0x01,
	GUIBorder_Right = 0x02,
	GUIBorder_Top = 0x04,
	GUIBorder_Bottom = 0x08,
} GUIBorder;

typedef enum
{
	GUIHideState_HideAll,
	GUIHideState_KeepBorders,
} GUIHideState;

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

	GUIDisplayState displayState;

	/* Border */
	GUIBorder border;
	uint32_t borderThickness;
	uint16_t borderColor;
} GUIObject;

/*
 * @name	GUIButton
 * @brief	A button with a callback function
 */
typedef struct
{
	/* Basic information about the object */
	GUIObject object;

	/* Colors */
	uint16_t enabledTextColor;
	uint16_t enabledBackgroundColor;
	uint16_t disabledTextColor;
	uint16_t disabledBackgroundColor;
	uint16_t pressedTextColor;
	uint16_t pressedBackgroundColor;

	GUIButtonState state;

	/* Pointer to a callback function called when a touch event has happened */
	void (*touchCallback)(GUITouchEvent);

	/* Text - Two rows of text can be displayed and it must have at least one row */
	uint8_t* text[2];
	LCDFontEnlarge textSize[2];

	uint8_t numOfChar[2];		/* These three are calculated automatically in GUI_AddButton */
	uint8_t textWidth[2];		/* --------------------------------------------------------- */
	uint8_t textHeight[2];		/* --------------------------------------------------------- */
} GUIButton;

/*
 * @name	GUITextBox
 * @brief	A box that can display text
 */
typedef struct
{
	/* Basic information about the object */
	GUIObject object;

	/* Colors */
	uint16_t textColor;
	uint16_t backgroundColor;

	/* Text */
	uint8_t* staticText;
	LCDFontEnlarge textSize;

	uint8_t staticTextNumOfChar;	/* These three are calculated automatically in GUI_AddTextBox */
	uint8_t staticTextWidth;		/* --------------------------------------------------------- */
	uint8_t staticTextHeight;		/* --------------------------------------------------------- */

	/* Position where the next character will be written. Referenced from the objects origin (xPos, yPos) */
	uint16_t xWritePos;
	uint16_t yWritePos;
} GUITextBox;

/*
 * @name	GUIContainer
 * @brief	A collection of other GUI items to more easily hide/show groups of items
 */
typedef struct
{
	/* Basic information about the object */
	GUIObject object;

	GUIHideState contentHideState;

	/* Store a pointer to all the object on the page for easy access and small footprint */
	GUIButton* buttons[guiConfigNUMBER_OF_BUTTONS];
	GUITextBox* textBoxes[guiConfigNUMBER_OF_TEXT_BOXES];
} GUIContainer;

/* Function prototypes -------------------------------------------------------*/
void GUI_Init();
void GUI_DrawBorder(GUIObject Object);
void GUI_RedrawLayer(GUILayer Layer);

/* Button functions */
GUIButton* GUI_GetButtonFromId(uint32_t ButtonId);
void GUI_AddButton(GUIButton* Button);
void GUI_HideButton(uint32_t ButtonId);
void GUI_DrawButton(uint32_t ButtonId);
void GUI_DrawAllButtons();
void GUI_SetButtonState(uint32_t ButtonId, GUIButtonState State);
void GUI_SetButtonTextForRow(uint32_t ButtonId, uint8_t* Text, uint32_t Row);
void GUI_CheckAllActiveButtonsForTouchEventAt(GUITouchEvent Event, uint16_t XPos, uint16_t YPos);
GUIDisplayState GUI_GetDisplayStateForButton(uint32_t ButtonId);

/* Text box functions */
GUITextBox* GUI_GetTextBoxFromId(uint32_t TextBoxId);
void GUI_AddTextBox(GUITextBox* TextBox);
void GUI_HideTextBox(uint32_t TextBoxId);
void GUI_DrawTextBox(uint32_t TextBoxId);
void GUI_DrawAllTextBoxes();
void GUI_WriteStringInTextBox(uint32_t TextBoxId, uint8_t* String);
void GUI_WriteNumberInTextBox(uint32_t TextBoxId, int32_t Number);
void GUI_SetWritePosition(uint32_t TextBoxId, uint16_t XPos, uint16_t YPos);
void GUI_ClearTextBox(uint32_t TextBoxId);
GUIDisplayState GUI_GetDisplayStateForTextBox(uint32_t TextBoxId);

/* Container functions */
void GUI_AddContainer(GUIContainer* Container);
void GUI_DrawContainer(uint32_t ContainerId);
void GUI_HideContentInContainer(uint32_t ContainerId);
void GUI_HideContainer(uint32_t ContainerId);
GUIDisplayState GUI_GetDisplayStateForContainer(uint32_t ContainerId);


#endif /* SIMPLE_GUI_H_ */
