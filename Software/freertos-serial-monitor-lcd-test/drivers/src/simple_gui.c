/**
 ******************************************************************************
 * @file	simple_gui.c
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

/* Includes ------------------------------------------------------------------*/
#include "simple_gui.h"

/* Private defines -----------------------------------------------------------*/
/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
GUIButton_TypeDef button_list[guiConfigNUMBER_OF_BUTTONS];

/* Private function prototypes -----------------------------------------------*/
/* Functions -----------------------------------------------------------------*/
/**
 * @brief	Draw the border on an object
 * @param	Object: The object to draw for
 * @retval	None
 */
void GUI_DrawBorder(GUI_Object_TypeDef Object)
{
	/* Draw the border */
	LCD_SetForegroundColor(Object.borderColor);
	if (Object.border & BORDER_LEFT)
	{
		LCD_DrawSquareOrLine(Object.xPos, Object.xPos + Object.borderThickness - 1,
							 Object.yPos, Object.yPos + Object.height - 1,
							 SQUARE, FILLED);
	}
	if (Object.border & BORDER_RIGHT)
	{
		LCD_DrawSquareOrLine(Object.xPos + Object.width - Object.borderThickness, Object.xPos + Object.width - 1,
							 Object.yPos, Object.yPos + Object.height - 1,
							 SQUARE, FILLED);
	}
	if (Object.border & BORDER_TOP)
	{
		LCD_DrawSquareOrLine(Object.xPos, Object.xPos + Object.width - 1,
							 Object.yPos, Object.yPos + Object.borderThickness - 1,
							 SQUARE, FILLED);
	}
	if (Object.border & BORDER_BOTTOM)
	{
		LCD_DrawSquareOrLine(Object.xPos, Object.xPos + Object.width - 1,
							 Object.yPos + Object.height - Object.borderThickness, Object.yPos + Object.height - 1,
							 SQUARE, FILLED);
	}
}

/**
 * @brief	Add a button to the button list
 * @param	Button: Pointer to a GUIButton_TypeDef struct which data should be copied from
 * @retval	None
 */
void GUI_AddButton(GUIButton_TypeDef* Button)
{
	static uint32_t buttonListIndex = 0;

	/* Make sure we don't try to create more button than there's room for in the button_list */
	if (buttonListIndex < guiConfigNUMBER_OF_BUTTONS)
	{
		button_list[buttonListIndex].enabledTextColor = Button->enabledTextColor;
		button_list[buttonListIndex].enabledBackgroundColor = Button->enabledBackgroundColor;
		button_list[buttonListIndex].disabledTextColor = Button->disabledTextColor;
		button_list[buttonListIndex].disabledBackgroundColor = Button->disabledBackgroundColor;
		button_list[buttonListIndex].pressedTextColor = Button->pressedTextColor;
		button_list[buttonListIndex].pressedBackgroundColor = Button->pressedBackgroundColor;
		button_list[buttonListIndex].object.borderColor = Button->object.borderColor;

		button_list[buttonListIndex].object.border = Button->object.border;
		button_list[buttonListIndex].object.borderThickness = Button->object.borderThickness;

		button_list[buttonListIndex].state = Button->state;

		button_list[buttonListIndex].object.xPos = Button->object.xPos;
		button_list[buttonListIndex].object.yPos = Button->object.yPos;
		button_list[buttonListIndex].object.width = Button->object.width;
		button_list[buttonListIndex].object.height = Button->object.height;
		button_list[buttonListIndex].object.layer = Button->object.layer;
		button_list[buttonListIndex].object.hidden = Button->object.hidden;

		/* Allocate memory for the text string and then copy */
		button_list[buttonListIndex].text = malloc(strlen(Button->text)+1);
		strcpy(button_list[buttonListIndex].text, Button->text);

		button_list[buttonListIndex].textSize = Button->textSize;
		button_list[buttonListIndex].numOfChar = strlen(button_list[buttonListIndex].text);
		button_list[buttonListIndex].textWidth = button_list[buttonListIndex].numOfChar * 8 * button_list[buttonListIndex].textSize;
		button_list[buttonListIndex].textHeight = 16 * button_list[buttonListIndex].textSize;

		buttonListIndex++;
	}
}

/**
 * @brief	Draw a specific button in the button_list
 * @param	None
 * @retval	None
 */
void GUI_DrawButton(uint32_t Index)
{
	if (Index < guiConfigNUMBER_OF_BUTTONS && button_list[Index].text != 0 && button_list[Index].object.hidden != HIDDEN)
	{
		/* Set state colors */
		uint16_t backgroundColor, textColor;
		switch (button_list[Index].state) {
			case ENABLED:
				/* Enabled state */
				backgroundColor = button_list[Index].enabledBackgroundColor;
				textColor = button_list[Index].enabledTextColor;
				break;
			case DISABLED:
				/* Disabled state */
				backgroundColor = button_list[Index].disabledBackgroundColor;
				textColor = button_list[Index].disabledTextColor;
				break;
			case PRESSED:
				/* Disabled state */
				backgroundColor = button_list[Index].pressedBackgroundColor;
				textColor = button_list[Index].pressedTextColor;
				break;
			default:
				break;
		}

		/* Draw the button */
		LCD_SetForegroundColor(backgroundColor);
		LCD_DrawSquareOrLine(button_list[Index].object.xPos, button_list[Index].object.xPos + button_list[Index].object.width-1,
							 button_list[Index].object.yPos, button_list[Index].object.yPos + button_list[Index].object.height-1, SQUARE, FILLED);
		LCD_SetForegroundColor(textColor);
		LCD_SetTextWritePosition(button_list[Index].object.xPos + (button_list[Index].object.width - button_list[Index].textWidth) / 2,
								 button_list[Index].object.yPos + (button_list[Index].object.height - button_list[Index].textHeight) / 2 - 2);
		LCD_WriteString(button_list[Index].text, TRANSPARENT, button_list[Index].textSize);

		/* Draw the border */
		GUI_DrawBorder(button_list[Index].object);
	}
}

/**
 * @brief	Draw all buttons in the button_list
 * @param	None
 * @retval	None
 */
void GUI_DrawAllButtons()
{
	for (uint32_t i = 0; i < guiConfigNUMBER_OF_BUTTONS; i++)
	{
		GUI_DrawButton(i);
	}
}

/**
 * @brief	Set the state of the button
 * @param	ButtonIndex: The index for the button
 * @param	State: The new state
 * @retval	None
 */
void GUI_SetButtonState(uint32_t ButtonIndex, GUI_ButtonState_TypeDef State)
{
	if (ButtonIndex < guiConfigNUMBER_OF_BUTTONS)
	{
		button_list[ButtonIndex].state = State;
		GUI_DrawButton(ButtonIndex);
	}
}

/**
 * @brief	Check if a button is located at the position where a touch up event occurred
 * @param	XPos: X-position for event
 * @param	XPos: Y-position for event
 * @retval	None
 */
void GUI_CheckButtonTouchUpEvent(uint16_t XPos, uint16_t YPos)
{
	for (uint32_t i = 0; i < guiConfigNUMBER_OF_BUTTONS; i++)
	{
		if (XPos >= button_list[i].object.xPos && XPos <= button_list[i].object.xPos + button_list[i].object.width &&
			YPos >= button_list[i].object.yPos && YPos <= button_list[i].object.yPos + button_list[i].object.height)
		{
			/* Touch Up has occurred for button i */
		}
	}
}

/**
 * @brief	Redraw all objects on a specified layer
 * @param	Layer: The layer to redraw
 * @retval	None
 */
void GUI_RedrawLayer(GUI_Layer_TypeDef Layer)
{
	/* Buttons */
	for (uint32_t i; i < guiConfigNUMBER_OF_BUTTONS; i++)
	{
		if (button_list[i].object.layer == Layer)
		{
			GUI_DrawButton(i);
		}
	}

	/* Text Boxes */

}

/* Private functions ---------------------------------------------------------*/
/* Interrupt Handlers --------------------------------------------------------*/
