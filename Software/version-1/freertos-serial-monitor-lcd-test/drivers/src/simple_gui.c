/**
 *******************************************************************************
 * @file  simple_gui.c
 * @author  Hampus Sandberg
 * @version 0.1
 * @date  2015-08-15
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

/* Includes ------------------------------------------------------------------*/
#include "simple_gui.h"

/* Private defines -----------------------------------------------------------*/
/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
GUIButton_TypeDef button_list[guiConfigNUMBER_OF_BUTTONS];
GUI_TextBox_TypeDef textBox_list[guiConfigNUMBER_OF_TEXT_BOXES];

/* Private function prototypes -----------------------------------------------*/
int32_t prvGUI_itoa(int32_t Number, uint8_t* Buffer);

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

/* Button --------------------------------------------------------------------*/
/**
 * @brief	Add a button to the button list
 * @param	Button: Pointer to a GUIButton_TypeDef struct which data should be copied from
 * @retval	None
 */
void GUI_AddButton(GUIButton_TypeDef* Button)
{
	uint32_t index = Button->object.id - guiConfigBUTTON_ID_OFFSET;

	/* Make sure we don't try to create more button than there's room for in the button_list */
	if (index < guiConfigNUMBER_OF_BUTTONS)
	{
		button_list[index].enabledTextColor = Button->enabledTextColor;
		button_list[index].enabledBackgroundColor = Button->enabledBackgroundColor;
		button_list[index].disabledTextColor = Button->disabledTextColor;
		button_list[index].disabledBackgroundColor = Button->disabledBackgroundColor;
		button_list[index].pressedTextColor = Button->pressedTextColor;
		button_list[index].pressedBackgroundColor = Button->pressedBackgroundColor;

		button_list[index].state = Button->state;

		button_list[index].object.xPos = Button->object.xPos;
		button_list[index].object.yPos = Button->object.yPos;
		button_list[index].object.width = Button->object.width;
		button_list[index].object.height = Button->object.height;
		button_list[index].object.layer = Button->object.layer;
		button_list[index].object.hidden = Button->object.hidden;
		button_list[index].object.borderColor = Button->object.borderColor;
		button_list[index].object.border = Button->object.border;
		button_list[index].object.borderThickness = Button->object.borderThickness;

		/* Allocate memory for the text string and then copy */
		button_list[index].text = malloc(strlen(Button->text)+1);
		strcpy(button_list[index].text, Button->text);

		button_list[index].textSize = Button->textSize;
		button_list[index].numOfChar = strlen(button_list[index].text);
		button_list[index].textWidth = button_list[index].numOfChar * 8 * button_list[index].textSize;
		button_list[index].textHeight = 16 * button_list[index].textSize;
	}
}

/**
 * @brief	Draw a specific button in the button_list
 * @param	None
 * @retval	None
 */
void GUI_DrawButton(uint32_t ButtonId)
{
	uint32_t index = ButtonId - guiConfigBUTTON_ID_OFFSET;

	if (index < guiConfigNUMBER_OF_BUTTONS && button_list[index].text != 0 && button_list[index].object.hidden != HIDDEN)
	{
		/* Set state colors */
		uint16_t backgroundColor, textColor;
		switch (button_list[index].state) {
			case ENABLED:
				/* Enabled state */
				backgroundColor = button_list[index].enabledBackgroundColor;
				textColor = button_list[index].enabledTextColor;
				break;
			case DISABLED:
				/* Disabled state */
				backgroundColor = button_list[index].disabledBackgroundColor;
				textColor = button_list[index].disabledTextColor;
				break;
			case PRESSED:
				/* Disabled state */
				backgroundColor = button_list[index].pressedBackgroundColor;
				textColor = button_list[index].pressedTextColor;
				break;
			default:
				break;
		}

		/* Draw the button */
		LCD_SetForegroundColor(backgroundColor);
		LCD_DrawSquareOrLine(button_list[index].object.xPos, button_list[index].object.xPos + button_list[index].object.width-1,
							 button_list[index].object.yPos, button_list[index].object.yPos + button_list[index].object.height-1, SQUARE, FILLED);
		LCD_SetForegroundColor(textColor);
		LCD_SetTextWritePosition(button_list[index].object.xPos + (button_list[index].object.width - button_list[index].textWidth) / 2,
								 button_list[index].object.yPos + (button_list[index].object.height - button_list[index].textHeight) / 2 - 2);
		LCD_WriteString(button_list[index].text, TRANSPARENT, button_list[index].textSize);

		/* Draw the border */
		GUI_DrawBorder(button_list[index].object);
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
void GUI_SetButtonState(uint32_t ButtonId, GUI_ButtonState_TypeDef State)
{
	uint32_t index = ButtonId - guiConfigBUTTON_ID_OFFSET;
	if (index < guiConfigNUMBER_OF_BUTTONS)
	{
		button_list[index].state = State;
		GUI_DrawButton(index);
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

/* Text box ------------------------------------------------------------------*/
/**
 * @brief	Add a text box to the list
 * @param	TextBox: The text box to add
 * @retval	None
 */
void GUI_AddTextBox(GUI_TextBox_TypeDef* TextBox)
{
	uint32_t index = TextBox->object.id - guiConfigTEXT_BOX_ID_OFFSET;

	/* Make sure we don't try to create more text boxes than there's room for in the textBox_list */
	if (index < guiConfigNUMBER_OF_TEXT_BOXES)
	{
		textBox_list[index].object.xPos = TextBox->object.xPos;
		textBox_list[index].object.yPos = TextBox->object.yPos;
		textBox_list[index].object.width = TextBox->object.width;
		textBox_list[index].object.height = TextBox->object.height;
		textBox_list[index].object.layer = TextBox->object.layer;
		textBox_list[index].object.hidden = TextBox->object.hidden;
		textBox_list[index].object.borderColor = TextBox->object.borderColor;
		textBox_list[index].object.border = TextBox->object.border;
		textBox_list[index].object.borderThickness = TextBox->object.borderThickness;

		textBox_list[index].textSize = TextBox->textSize;
		textBox_list[index].xWritePos = TextBox->xWritePos;
		textBox_list[index].yWritePos = TextBox->yWritePos;
	}
}

/**
 * @brief	Draw a text box
 * @param	TextBoxId: The id of the text box to draw
 * @retval	None
 */
void GUI_DrawTextBox(uint32_t TextBoxId)
{
	uint32_t index = TextBoxId - guiConfigTEXT_BOX_ID_OFFSET;

	if (index < guiConfigNUMBER_OF_TEXT_BOXES)
	{
		/* Clear the active window */
		LCD_ActiveWindow_TypeDef window;
		window.xLeft = textBox_list[index].object.xPos;
		window.xRight = textBox_list[index].object.xPos + textBox_list[index].object.width - 1;
		window.yTop = textBox_list[index].object.yPos;
		window.yBottom = textBox_list[index].object.yPos + textBox_list[index].object.height - 1;
		LCD_ClearActiveWindow(window.xLeft, window.xRight, window.yTop, window.yBottom);

		/* Draw the border */
		GUI_DrawBorder(textBox_list[index].object);
	}
}

/**
 * @brief	Draw all text boxes in the textBox_list
 * @param	None
 * @retval	None
 */
void GUI_DrawAllTextBoxes()
{
	for (uint32_t i = 0; i < guiConfigNUMBER_OF_TEXT_BOXES; i++)
	{
		GUI_DrawTextBox(guiConfigTEXT_BOX_ID_OFFSET + i);
	}
}

/**
 * @brief	Write a string in a text box
 * @param	TextBoxId: The id of the text box to write in
 * @param	String: The string to write
 * @retval	None
 */
void GUI_WriteStringInTextBox(uint32_t TextBoxId, uint8_t* String)
{
	uint32_t index = TextBoxId - guiConfigTEXT_BOX_ID_OFFSET;

	if (index < guiConfigNUMBER_OF_TEXT_BOXES)
	{
		LCD_ActiveWindow_TypeDef window;
		window.xLeft = textBox_list[index].object.xPos;
		window.xRight = textBox_list[index].object.xPos + textBox_list[index].object.width - 1;
		window.yTop = textBox_list[index].object.yPos;
		window.yBottom = textBox_list[index].object.yPos + textBox_list[index].object.height - 1;

		uint16_t xWritePosTemp = textBox_list[index].object.xPos + textBox_list[index].xWritePos;
		uint16_t yWritePosTemp = textBox_list[index].object.yPos + textBox_list[index].yWritePos;

		LCD_WriteStringInActiveWindowAtPosition(String, TRANSPARENT, textBox_list[index].textSize, window,
												&xWritePosTemp, &yWritePosTemp);

		textBox_list[index].xWritePos = xWritePosTemp - textBox_list[index].object.xPos;
		textBox_list[index].yWritePos = yWritePosTemp - textBox_list[index].object.yPos;
	}
}

/**
 * @brief	Write a number in a text box
 * @param	TextBoxId: The id of the text box to write in
 * @param	Number: The number to write
 * @retval	None
 */
void GUI_WriteNumberInTextBox(uint32_t TextBoxId, int32_t Number)
{
	uint32_t index = TextBoxId - guiConfigTEXT_BOX_ID_OFFSET;

	if (index < guiConfigNUMBER_OF_TEXT_BOXES)
	{
		uint8_t buffer[20];
		prvGUI_itoa(Number, buffer);
		GUI_WriteStringInTextBox(TextBoxId, buffer);
	}
}

/**
 * @brief	Set where the next character should be written
 * @param	TextBoxId:
 * @param	XPos:
 * @param	YPos:
 * @retval	None
 */
void GUI_SetWritePosition(uint32_t TextBoxId, uint16_t XPos, uint16_t YPos)
{
	uint32_t index = TextBoxId - guiConfigTEXT_BOX_ID_OFFSET;

	if (index < guiConfigNUMBER_OF_TEXT_BOXES)
	{
		textBox_list[index].xWritePos = XPos;
		textBox_list[index].yWritePos = YPos;
	}
}

/**
 * @brief	Clear the text box of any text
 * @param	TextBoxId:
 * @retval	None
 */
void GUI_ClearTextBox(uint32_t TextBoxId)
{
	uint32_t index = TextBoxId - guiConfigTEXT_BOX_ID_OFFSET;

	if (index < guiConfigNUMBER_OF_TEXT_BOXES)
	{
		GUI_DrawTextBox(TextBoxId);
	}
}

/* Private functions ---------------------------------------------------------*/
/**
 * @brief	Convert an int32_t to a c-string
 * @param	Number:
 * @param	Buffer:
 * @retval	None
 * @note	Code from https://code.google.com/p/my-itoa/
 */
int32_t prvGUI_itoa(int32_t Number, uint8_t* Buffer)
{
    const uint32_t radix = 10;

    uint8_t* p;
    uint32_t a;		/* every digit */
    int32_t len;
    uint8_t* b;		/* start of the digit uint8_t */
    uint8_t temp;
    uint32_t u;

    p = Buffer;

    if (Number < 0)
    {
        *p++ = '-';
        Number = 0 - Number;
    }
    u = (uint32_t)Number;

    b = p;

    do
    {
        a = u % radix;
        u /= radix;

        *p++ = a + '0';

    } while (u > 0);

    len = (int32_t)(p - Buffer);

    *p-- = 0;

    /* swap */
    do
    {
        temp = *p;
        *p = *b;
        *b = temp;
        --p;
        ++b;

    } while (b < p);

    return len;
}
/* Interrupt Handlers --------------------------------------------------------*/
