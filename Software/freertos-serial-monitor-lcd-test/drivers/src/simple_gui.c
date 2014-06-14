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
 * @brief	GUI function
 * @param	None
 * @retval	None
 */
void GUI_Func()
{

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

		button_list[buttonListIndex].state = Button->state;

		button_list[buttonListIndex].xPos = Button->xPos;
		button_list[buttonListIndex].yPos = Button->yPos;
		button_list[buttonListIndex].width = Button->width;
		button_list[buttonListIndex].height = Button->height;

		/* Allocate memory for the text string and then copy */
		button_list[buttonListIndex].text = malloc(strlen(Button->text)+1);
		strcpy(button_list[buttonListIndex].text, Button->text);

		button_list[buttonListIndex].textSize = Button->textSize;

		buttonListIndex++;
	}
}

/**
 * @brief	Draw a specific button in the button_list
 * @param	None
 * @retval	None
 */
void GUI_DrawButton(uint32_t ButtonIndex)
{
	if (ButtonIndex < guiConfigNUMBER_OF_BUTTONS && button_list[ButtonIndex].text != 0)
	{
		/* TODO: Do these calculations once in GUI_AddButton and save instead? */
		uint8_t numChar = strlen(button_list[ButtonIndex].text);
		uint8_t textWidth = numChar * 8 * button_list[ButtonIndex].textSize;
		uint8_t textHeight = 16 * button_list[ButtonIndex].textSize;

		/* Set state colors */
		uint16_t backgroundColor, textColor;
		switch (button_list[ButtonIndex].state) {
			case ENABLED:
				/* Enabled state */
				backgroundColor = button_list[ButtonIndex].enabledBackgroundColor;
				textColor = button_list[ButtonIndex].enabledTextColor;
				break;
			case DISABLED:
				/* Disabled state */
				backgroundColor = button_list[ButtonIndex].disabledBackgroundColor;
				textColor = button_list[ButtonIndex].disabledTextColor;
				break;
			case PRESSED:
				/* Disabled state */
				backgroundColor = button_list[ButtonIndex].pressedBackgroundColor;
				textColor = button_list[ButtonIndex].pressedTextColor;
				break;
			default:
				break;
		}

		/* Draw the button */
		LCD_SetForegroundColor(backgroundColor);
		LCD_DrawSquareOrLine(button_list[ButtonIndex].xPos, button_list[ButtonIndex].xPos + button_list[ButtonIndex].width-1,
							 button_list[ButtonIndex].yPos, button_list[ButtonIndex].yPos + button_list[ButtonIndex].height-1, SQUARE, FILLED);
		LCD_SetForegroundColor(textColor);
		LCD_SetTextWritePosition(button_list[ButtonIndex].xPos + (button_list[ButtonIndex].width-textWidth) / 2,
								 button_list[ButtonIndex].yPos + (button_list[ButtonIndex].height-textHeight) / 2 - 2);
		LCD_WriteString(button_list[ButtonIndex].text, TRANSPARENT, button_list[ButtonIndex].textSize);
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
		if (XPos >= button_list[i].xPos && XPos <= button_list[i].xPos + button_list[i].width &&
			YPos >= button_list[i].yPos && YPos <= button_list[i].yPos + button_list[i].height)
		{
			/* Touch Up has occurred for button i */
		}
	}
}

/* Private functions ---------------------------------------------------------*/
/* Interrupt Handlers --------------------------------------------------------*/
