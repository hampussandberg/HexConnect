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
GUIButton button_list[guiConfigNUMBER_OF_BUTTONS];
GUITextBox textBox_list[guiConfigNUMBER_OF_TEXT_BOXES];
GUIContainer container_list[guiConfigNUMBER_OF_CONTAINERS];

/* Private function prototypes -----------------------------------------------*/
static int32_t prvItoa(int32_t Number, uint8_t* Buffer);

/* Functions -----------------------------------------------------------------*/
/**
 * @brief	Initializes the GUI by setting the items in the lists to appropriate values
 * @param	None
 * @retval	None
 */
void GUI_Init()
{
	/* Buttons */
	for (uint32_t i = 0; i < guiConfigNUMBER_OF_BUTTONS; i++)
	{
		memset(&button_list[i], 0, sizeof(GUIButton));
	}

	/* Text boxes */
	for (uint32_t i = 0; i < guiConfigNUMBER_OF_TEXT_BOXES; i++)
	{
		memset(&textBox_list[i], 0, sizeof(GUITextBox));
	}

	/* Pages */
	for (uint32_t i = 0; i < guiConfigNUMBER_OF_CONTAINERS; i++)
	{
		memset(&container_list[i], 0, sizeof(GUIContainer));
	}
}

/**
 * @brief	Draw the border on an object
 * @param	Object: The object to draw for
 * @retval	None
 */
void GUI_DrawBorder(GUIObject Object)
{
	/* Draw the border */
	LCD_SetForegroundColor(Object.borderColor);
	if (Object.border & GUIBorder_Left)
	{
		LCD_DrawSquareOrLine(Object.xPos, Object.xPos + Object.borderThickness - 1,
							 Object.yPos, Object.yPos + Object.height - 1,
							 LCDDrawType_Square, LCDFill_Fill);
	}
	if (Object.border & GUIBorder_Right)
	{
		LCD_DrawSquareOrLine(Object.xPos + Object.width - Object.borderThickness, Object.xPos + Object.width - 1,
							 Object.yPos, Object.yPos + Object.height - 1,
							 LCDDrawType_Square, LCDFill_Fill);
	}
	if (Object.border & GUIBorder_Top)
	{
		LCD_DrawSquareOrLine(Object.xPos, Object.xPos + Object.width - 1,
							 Object.yPos, Object.yPos + Object.borderThickness - 1,
							 LCDDrawType_Square, LCDFill_Fill);
	}
	if (Object.border & GUIBorder_Bottom)
	{
		LCD_DrawSquareOrLine(Object.xPos, Object.xPos + Object.width - 1,
							 Object.yPos + Object.height - Object.borderThickness, Object.yPos + Object.height - 1,
							 LCDDrawType_Square, LCDFill_Fill);
	}
}

/**
 * @brief	Redraw all objects on a specified layer
 * @param	Layer: The layer to redraw
 * @retval	None
 */
void GUI_RedrawLayer(GUILayer Layer)
{
	/* Buttons */
	for (uint32_t i = 0; i < guiConfigNUMBER_OF_BUTTONS; i++)
	{
		if (button_list[i].object.layer == Layer && button_list[i].object.displayState != GUIDisplayState_Hidden)
		{
			GUI_DrawButton(button_list[i].object.id);
		}
	}

	/* Text Boxes */
	for (uint32_t i = 0; i < guiConfigNUMBER_OF_TEXT_BOXES; i++)
	{
		if (textBox_list[i].object.layer == Layer && textBox_list[i].object.displayState != GUIDisplayState_Hidden)
		{
			GUI_DrawTextBox(textBox_list[i].object.id);
		}
	}
}

/* Button --------------------------------------------------------------------*/
/**
 * @brief	Get a pointer to the button corresponding to the id
 * @param	ButtonId: The id of the button to get
 * @retval	Pointer the button or 0 if no button was found
 */
GUIButton* GUI_GetButtonFromId(uint32_t ButtonId)
{
	uint32_t index = ButtonId - guiConfigBUTTON_ID_OFFSET;

	if (index < guiConfigNUMBER_OF_BUTTONS)
		return &button_list[index];
	else
		return 0;
}
/**
 * @brief	Add a button to the button list
 * @param	Button: Pointer to a GUIButton_TypeDef struct which data should be copied from
 * @retval	None
 */
void GUI_AddButton(GUIButton* Button)
{
	uint32_t index = Button->object.id - guiConfigBUTTON_ID_OFFSET;

	/* Make sure we don't try to create more button than there's room for in the button_list */
	if (index < guiConfigNUMBER_OF_BUTTONS)
	{
		/* Copy the button to the list */
		memcpy(&button_list[index], Button, sizeof(GUIButton));

		/* Get a pointer to the current button */
		GUIButton* button = &button_list[index];

		/* Two rows of text */
		if (button->text[0] != 0 && button->text[1] != 0)
		{
			button->numOfChar[0] = strlen(button->text[0]);
			button->textWidth[0] = button->numOfChar[0] * 8 * button->textSize[0];
			button->textHeight[0] = 16 * button->textSize[0];

			button->numOfChar[1] = strlen(button->text[1]);
			button->textWidth[1] = button->numOfChar[1] * 8 * button->textSize[1];
			button->textHeight[1] = 16 * button->textSize[1];
		}
		/* One row of text */
		else
		{
			/* Fill in the rest of the data */
			button->numOfChar[0] = strlen(button->text[0]);
			button->textWidth[0] = button->numOfChar[0] * 8 * button->textSize[0];
			button->textHeight[0] = 16 * button->textSize[0];
		}

		/* If it's set to not hidden we should draw the button */
		if (Button->object.displayState == GUIDisplayState_NotHidden)
			GUI_DrawButton(Button->object.id);
	}
	/* Set all the data in the Button we received as a parameter to 0 so that it can be reused easily */
	memset(Button, 0, sizeof(GUIButton));
}


/**
 * @brief	Hides a button by drawing a black box over it
 * @param	ButtonId: The id of the button to hide
 * @retval	None
 */
void GUI_HideButton(uint32_t ButtonId)
{
	uint32_t index = ButtonId - guiConfigBUTTON_ID_OFFSET;

	if (index < guiConfigNUMBER_OF_BUTTONS)
	{
		/* Get a pointer to the current button */
		GUIButton* button = &button_list[index];

		/* Set the background color */
		LCD_SetBackgroundColor(LCD_COLOR_BLACK);
		/* Clear the active window */
		LCDActiveWindow window;
		window.xLeft = button->object.xPos;
		window.xRight = button->object.xPos + button->object.width - 1;
		window.yTop = button->object.yPos;
		window.yBottom = button->object.yPos + button->object.height - 1;
		LCD_ClearActiveWindow(window.xLeft, window.xRight, window.yTop, window.yBottom);
		button->object.displayState = GUIDisplayState_Hidden;
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

	if (index < guiConfigNUMBER_OF_BUTTONS && button_list[index].text != 0)
	{
		/* Get a pointer to the current button */
		GUIButton* button = &button_list[index];

		/* Set state colors */
		uint16_t backgroundColor, textColor;
		switch (button->state) {
			case GUIButtonState_Enabled:
				/* Enabled state */
				backgroundColor = button->enabledBackgroundColor;
				textColor = button->enabledTextColor;
				break;
			case GUIButtonState_Disabled:
				/* Disabled state */
				backgroundColor = button->disabledBackgroundColor;
				textColor = button->disabledTextColor;
				break;
			case GUIButtonState_TouchDown:
				/* Disabled state */
				backgroundColor = button->pressedBackgroundColor;
				textColor = button->pressedTextColor;
				break;
			default:
				break;
		}

		/* Draw the button */
		LCD_SetForegroundColor(backgroundColor);
		LCD_DrawSquareOrLine(button->object.xPos, button->object.xPos + button->object.width-1,
							 button->object.yPos, button->object.yPos + button->object.height-1, LCDDrawType_Square, LCDFill_Fill);
		LCD_SetForegroundColor(textColor);

		/* Two rows of text */
		if (button->text[0] != 0 && button->text[1] != 0)
		{
			uint16_t xPos = button->object.xPos + (button->object.width - button->textWidth[0]) / 2;
			uint16_t yPos = button->object.yPos + button->object.height/2 - button->textHeight[0] - 1;
			LCD_SetTextWritePosition(xPos, yPos);
			LCD_WriteString(button->text[0], LCDTransparency_Transparent, button->textSize[0]);

			xPos = button->object.xPos + (button->object.width - button->textWidth[1]) / 2;
			yPos = button->object.yPos + button->object.height/2 + 1;
			LCD_SetTextWritePosition(xPos, yPos);
			LCD_WriteString(button->text[1], LCDTransparency_Transparent, button->textSize[1]);
		}
		/* One row of text */
		else
		{
			uint16_t xPos = button->object.xPos + (button->object.width - button->textWidth[0]) / 2;
			uint16_t yPos = button->object.yPos + (button->object.height - button->textHeight[0]) / 2 - 2;
			LCD_SetTextWritePosition(xPos, yPos);
			LCD_WriteString(button->text[0], LCDTransparency_Transparent, button->textSize[0]);
		}

		/* Draw the border */
		GUI_DrawBorder(button->object);

		button->object.displayState = GUIDisplayState_NotHidden;
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
 * @param	ButtonId: The Id for the button
 * @param	State: The new state
 * @retval	None
 */
void GUI_SetButtonState(uint32_t ButtonId, GUIButtonState State)
{
	uint32_t index = ButtonId - guiConfigBUTTON_ID_OFFSET;
	if (index < guiConfigNUMBER_OF_BUTTONS)
	{
		button_list[index].state = State;
		GUI_DrawButton(index);
	}
}

/**
 * @brief	Set the state of the button
 * @param	ButtonId: The Id for the button
 * @param	Text: The text to set
 * @param	Row: The row to set, can be 0 or 1
 * @retval	None
 * @warning Make sure the Text is smaller than or equal to the text set during init
 */
void GUI_SetButtonTextForRow(uint32_t ButtonId, uint8_t* Text, uint32_t Row)
{
	uint32_t index = ButtonId - guiConfigBUTTON_ID_OFFSET;
	if (index < guiConfigNUMBER_OF_BUTTONS && Row < 2)
	{
		/* Something strange happens when I do a memcpy of only the text[1] so do a memcpy on a whole GUIButton instead */
		GUIButton* tempButton = &button_list[index];
		tempButton->text[1] = Text;
		memcpy(&button_list[index], tempButton, sizeof(GUIButton));

		/* Draw the button so that the changes appear */
		if (tempButton->object.displayState == GUIDisplayState_NotHidden)
			GUI_DrawButton(ButtonId);
	}
}

/**
 * @brief	Check if a button is located at the position where a touch up event occurred
 * @param	GUITouchEvent: The event that happened, can be any value of GUITouchEvent
 * @param	XPos: X-position for event
 * @param	XPos: Y-position for event
 * @retval	None
 */
void GUI_CheckAllActiveButtonsForTouchEventAt(GUITouchEvent Event, uint16_t XPos, uint16_t YPos)
{
	static GUIButton* lastActiveButton = 0;
	static GUIButtonState lastState = GUIButtonState_NoState;

	for (uint32_t index = 0; index < guiConfigNUMBER_OF_BUTTONS; index++)
	{
		/* Check if the button is not hidden and enabled and if it's hit */
		if (button_list[index].object.displayState == GUIDisplayState_NotHidden && button_list[index].state != GUIButtonState_NoState &&
			XPos >= button_list[index].object.xPos && XPos <= button_list[index].object.xPos + button_list[index].object.width &&
			YPos >= button_list[index].object.yPos && YPos <= button_list[index].object.yPos + button_list[index].object.height)
		{
			if (Event == GUITouchEvent_Up)
			{
				GUI_SetButtonState(index + guiConfigBUTTON_ID_OFFSET, lastState);
				lastActiveButton = 0;
				lastState = GUIButtonState_Disabled;
				if (button_list[index].touchCallback != 0)
					button_list[index].touchCallback(Event);
			}
			else if (Event == GUITouchEvent_Down)
			{
				/*
				 * Check if the new button we have hit is different from the last time,
				 * if so change back the state of the old button and activate the new one
				 * and save a reference to it
				 */
				if (lastActiveButton == 0 || lastActiveButton->object.id != button_list[index].object.id)
				{
					/*
					 * If we had a last active button it means the user has moved away from the button while
					 * still holding down on the screen. We therefore have to reset the state of that button
					 */
					if (lastActiveButton != 0)
						GUI_SetButtonState(lastActiveButton->object.id, lastState);

					/* Save the new button and change it's state */
					lastState = button_list[index].state;
					lastActiveButton = &button_list[index];
					GUI_SetButtonState(index + guiConfigBUTTON_ID_OFFSET, GUIButtonState_TouchDown);
				}
				/* Otherwise don't do anything as the user is still touching the same button */
			}
			/* Only one button should be active on an event so return when we have found one */
			return;
		}
	}

	/* If no button hit was found, check if a button is still in touch down state and if so change it's state */
	if (lastActiveButton != 0)
	{
		GUI_SetButtonState(lastActiveButton->object.id, lastState);
		lastActiveButton = 0;
		lastState = GUIButtonState_NoState;
	}
}

/**
 * @brief	Get the display state of a button
 * @param	ContainerId: The button to get the state for
 * @retval	The display state if valid ID, otherwise GUIDisplayState_NoState
 */
GUIDisplayState GUI_GetDisplayStateForButton(uint32_t ButtonId)
{
	uint32_t index = ButtonId - guiConfigBUTTON_ID_OFFSET;

	if (index < guiConfigNUMBER_OF_BUTTONS)
		return button_list[index].object.displayState;
	else
		return GUIDisplayState_NoState;
}

/* Text box ------------------------------------------------------------------*/
/**
 * @brief	Get a pointer to the text box corresponding to the id
 * @param	TextBoxId: The id of the text box to get
 * @retval	Pointer the text box or 0 if no text box was found
 */
GUITextBox* GUI_GetTextBoxFromId(uint32_t TextBoxId)
{
	uint32_t index = TextBoxId - guiConfigTEXT_BOX_ID_OFFSET;

	if (index < guiConfigNUMBER_OF_TEXT_BOXES)
		return &textBox_list[index];
	else
		return 0;
}

/**
 * @brief	Add a text box to the list
 * @param	TextBox: The text box to add
 * @retval	None
 */
void GUI_AddTextBox(GUITextBox* TextBox)
{
	uint32_t index = TextBox->object.id - guiConfigTEXT_BOX_ID_OFFSET;

	/* Make sure we don't try to create more text boxes than there's room for in the textBox_list */
	if (index < guiConfigNUMBER_OF_TEXT_BOXES)
	{
		/* Copy the text box to the list */
		memcpy(&textBox_list[index], TextBox, sizeof(GUITextBox));

		/* If there's a static text - Fill in the rest of the data */
		if (textBox_list[index].staticText != 0)
		{
			textBox_list[index].staticTextNumOfChar = strlen(textBox_list[index].staticText);
			textBox_list[index].staticTextWidth = textBox_list[index].staticTextNumOfChar * 8 * textBox_list[index].textSize;
			textBox_list[index].staticTextHeight = 16 * textBox_list[index].textSize;

			/* Overwrite the write positions so that the text is centered */
			textBox_list[index].xWritePos = (textBox_list[index].object.width - textBox_list[index].staticTextWidth) / 2;
			textBox_list[index].yWritePos = (textBox_list[index].object.height - textBox_list[index].staticTextHeight) / 2 - 2;
		}

		/* If it's set to not hidden we should draw the button */
		if (TextBox->object.displayState == GUIDisplayState_NotHidden)
			GUI_DrawTextBox(TextBox->object.id);
	}
	/* Set all the data in the TextBox we received as a parameter to 0 so that it can be reused easily */
	memset(TextBox, 0, sizeof(GUITextBox));
}

/**
 * @brief	Hides a text box by drawing a black box over it
 * @param	TextBoxId: The id of the text box to hide
 * @retval	None
 */
void GUI_HideTextBox(uint32_t TextBoxId)
{
	uint32_t index = TextBoxId - guiConfigTEXT_BOX_ID_OFFSET;

	if (index < guiConfigNUMBER_OF_TEXT_BOXES)
	{
		/* Set the background color */
		LCD_SetBackgroundColor(LCD_COLOR_BLACK);
		/* Clear the active window */
		LCDActiveWindow window;
		window.xLeft = textBox_list[index].object.xPos;
		window.xRight = textBox_list[index].object.xPos + textBox_list[index].object.width - 1;
		window.yTop = textBox_list[index].object.yPos;
		window.yBottom = textBox_list[index].object.yPos + textBox_list[index].object.height - 1;
		LCD_ClearActiveWindow(window.xLeft, window.xRight, window.yTop, window.yBottom);
		textBox_list[index].object.displayState = GUIDisplayState_Hidden;
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
		/* Set the background color */
		LCD_SetBackgroundColor(textBox_list[index].backgroundColor);
		/* Clear the active window */
		LCDActiveWindow window;
		window.xLeft = textBox_list[index].object.xPos;
		window.xRight = textBox_list[index].object.xPos + textBox_list[index].object.width - 1;
		window.yTop = textBox_list[index].object.yPos;
		window.yBottom = textBox_list[index].object.yPos + textBox_list[index].object.height - 1;
		LCD_ClearActiveWindow(window.xLeft, window.xRight, window.yTop, window.yBottom);

		/* Draw the border */
		GUI_DrawBorder(textBox_list[index].object);

		textBox_list[index].object.displayState = GUIDisplayState_NotHidden;

		/* If there's a static text we should write it */
		if (textBox_list[index].staticText != 0)
		{
			GUI_WriteStringInTextBox(TextBoxId, textBox_list[index].staticText);
		}
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
		/* Set the text color */
		LCD_SetForegroundColor(textBox_list[index].textColor);
		/* Get the active window and then write the text in it */
		LCDActiveWindow window;
		window.xLeft = textBox_list[index].object.xPos;
		window.xRight = textBox_list[index].object.xPos + textBox_list[index].object.width - 1;
		window.yTop = textBox_list[index].object.yPos;
		window.yBottom = textBox_list[index].object.yPos + textBox_list[index].object.height - 1;

		uint16_t xWritePosTemp = textBox_list[index].object.xPos + textBox_list[index].xWritePos;
		uint16_t yWritePosTemp = textBox_list[index].object.yPos + textBox_list[index].yWritePos;

		LCD_WriteStringInActiveWindowAtPosition(String, LCDTransparency_Transparent, textBox_list[index].textSize, window,
												&xWritePosTemp, &yWritePosTemp);

		/* Only save the next writeposition if the text box is not static */
		if (textBox_list[index].staticText == 0)
		{
			textBox_list[index].xWritePos = xWritePosTemp - textBox_list[index].object.xPos;
			textBox_list[index].yWritePos = yWritePosTemp - textBox_list[index].object.yPos;
		}
	}
}

/**
 * @brief	Write a buffer in a text box
 * @param	TextBoxId: The id of the text box to write in
 * @param	pBuffer: The buffer to write
 * @param	Size: Size of the buffer
 * @param	Format: The format to use when writing the buffer, can be any value of GUIWriteFormat
 * @retval	None
 */
void GUI_WriteBufferInTextBox(uint32_t TextBoxId, uint8_t* pBuffer, uint32_t Size, GUIWriteFormat Format)
{
	uint32_t index = TextBoxId - guiConfigTEXT_BOX_ID_OFFSET;

	if (index < guiConfigNUMBER_OF_TEXT_BOXES)
	{
		/* Set the text color */
		LCD_SetForegroundColor(textBox_list[index].textColor);
		/* Get the active window and then write the text in it */
		LCDActiveWindow window;
		window.xLeft = textBox_list[index].object.xPos;
		window.xRight = textBox_list[index].object.xPos + textBox_list[index].object.width - 1;
		window.yTop = textBox_list[index].object.yPos;
		window.yBottom = textBox_list[index].object.yPos + textBox_list[index].object.height - 1;

		uint16_t xWritePosTemp = textBox_list[index].object.xPos + textBox_list[index].xWritePos;
		uint16_t yWritePosTemp = textBox_list[index].object.yPos + textBox_list[index].yWritePos;

		LCD_WriteBufferInActiveWindowAtPosition(pBuffer, Size, LCDTransparency_Transparent, textBox_list[index].textSize,
												window,	&xWritePosTemp, &yWritePosTemp, Format);

		/* Only save the next writeposition if the text box is not static */
		if (textBox_list[index].staticText == 0)
		{
			textBox_list[index].xWritePos = xWritePosTemp - textBox_list[index].object.xPos;
			textBox_list[index].yWritePos = yWritePosTemp - textBox_list[index].object.yPos;
		}
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
		prvItoa(Number, buffer);
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

/**
 * @brief	Get the display state of a text box
 * @param	TextBoxId: The text box to get the state for
 * @retval	The display state if valid ID, otherwise GUIDisplayState_NoState
 */
GUIDisplayState GUI_GetDisplayStateForTextBox(uint32_t TextBoxId)
{
	uint32_t index = TextBoxId - guiConfigTEXT_BOX_ID_OFFSET;

	if (index < guiConfigNUMBER_OF_TEXT_BOXES)
		return textBox_list[index].object.displayState;
	else
		return GUIDisplayState_NoState;
}

/* Container -----------------------------------------------------------------*/
/**
 * @brief	Get a pointer to the container corresponding to the id
 * @param	ContainerId: The id of the container to get
 * @retval	Pointer the container or 0 if no container was found
 */
GUIContainer* GUI_GetContainerFromId(uint32_t ContainerId)
{
	uint32_t index = ContainerId - guiConfigCONTAINER_ID_OFFSET;

	if (index < guiConfigNUMBER_OF_CONTAINERS)
		return &container_list[index];
	else
		return 0;
}

/**
 * @brief	Add a container to the list
 * @param	Container: Pointer to the container to add
 * @retval	None
 */
void GUI_AddContainer(GUIContainer* Container)
{
	uint32_t index = Container->object.id - guiConfigCONTAINER_ID_OFFSET;

	/* Make sure we don't try to create more containers than there's room for in the textBox_list */
	if (index < guiConfigNUMBER_OF_CONTAINERS)
	{
		/* Copy the container to the list */
		memcpy(&container_list[index], Container, sizeof(GUIContainer));

		/* If it's set to not hidden we should draw the button */
		if (Container->object.displayState == GUIDisplayState_NotHidden)
			GUI_DrawContainer(Container->object.id);
	}
	/* Set all the data in the Container we received as a parameter to 0 so that it can be reused easily */
	memset(Container, 0, sizeof(GUIContainer));
}

/**
 * @brief	Hide the content in a container
 * @param	ContainerId: The id of the container to hide content of
 * @retval	None
 */
void GUI_HideContentInContainer(uint32_t ContainerId)
{
	uint32_t index = ContainerId - guiConfigCONTAINER_ID_OFFSET;

	if (index < guiConfigNUMBER_OF_CONTAINERS)
	{
		/* Hide the buttons */
		for (uint32_t i = 0; i < guiConfigNUMBER_OF_BUTTONS; i++)
		{
			if (container_list[index].buttons[i] != 0)
				GUI_HideButton(container_list[index].buttons[i]->object.id);
		}

		/* Hide the text boxes */
		for (uint32_t i = 0; i < guiConfigNUMBER_OF_TEXT_BOXES; i++)
		{
			if (container_list[index].textBoxes[i] != 0)
				GUI_HideTextBox(container_list[index].textBoxes[i]->object.id);
		}

		/* Check if borders should be drawn */
		if (container_list[index].contentHideState == GUIHideState_KeepBorders)
			GUI_DrawBorder(container_list[index].object);

		container_list[index].object.displayState = GUIDisplayState_ContentHidden;
	}
}

/**
 * @brief	Hide a cointainer
 * @param	ContainerId: The id of the container to hide
 * @retval	None
 */
void GUI_HideContainer(uint32_t ContainerId)
{
	uint32_t index = ContainerId - guiConfigCONTAINER_ID_OFFSET;

	if (index < guiConfigNUMBER_OF_CONTAINERS)
	{
		/* Set the background color */
		LCD_SetBackgroundColor(LCD_COLOR_BLACK);
		/* Clear the active window */
		LCDActiveWindow window;
		window.xLeft = container_list[index].object.xPos;
		window.xRight = container_list[index].object.xPos + container_list[index].object.width - 1;
		window.yTop = container_list[index].object.yPos;
		window.yBottom = container_list[index].object.yPos + container_list[index].object.height - 1;
		LCD_ClearActiveWindow(window.xLeft, window.xRight, window.yTop, window.yBottom);

		/* Hide the buttons */
		for (uint32_t i = 0; i < guiConfigNUMBER_OF_BUTTONS; i++)
		{
			if (container_list[index].buttons[i] != 0)
				GUI_HideButton(container_list[index].buttons[i]->object.id);
		}

		/* Hide the text boxes */
		for (uint32_t i = 0; i < guiConfigNUMBER_OF_TEXT_BOXES; i++)
		{
			if (container_list[index].textBoxes[i] != 0)
				GUI_HideTextBox(container_list[index].textBoxes[i]->object.id);
		}

		container_list[index].object.displayState = GUIDisplayState_Hidden;
	}
}

/**
 * @brief	Draw a specific container with the specified id
 * @param	ContainerId: The id of the container to draw
 * @retval	None
 */
void GUI_DrawContainer(uint32_t ContainerId)
{
	uint32_t index = ContainerId - guiConfigCONTAINER_ID_OFFSET;

	if (index < guiConfigNUMBER_OF_CONTAINERS)
	{
		/* Set the background color */
		LCD_SetBackgroundColor(LCD_COLOR_BLACK);
		/* Clear the active window */
		LCDActiveWindow window;
		window.xLeft = container_list[index].object.xPos;
		window.xRight = container_list[index].object.xPos + container_list[index].object.width - 1;
		window.yTop = container_list[index].object.yPos;
		window.yBottom = container_list[index].object.yPos + container_list[index].object.height - 1;
		LCD_ClearActiveWindow(window.xLeft, window.xRight, window.yTop, window.yBottom);

		/* Draw the buttons */
		for (uint32_t i = 0; i < guiConfigNUMBER_OF_BUTTONS; i++)
		{
			if (container_list[index].buttons[i] != 0)
				GUI_DrawButton(container_list[index].buttons[i]->object.id);
		}

		/* Draw the text boxes */
		for (uint32_t i = 0; i < guiConfigNUMBER_OF_TEXT_BOXES; i++)
		{
			if (container_list[index].textBoxes[i] != 0)
				GUI_DrawTextBox(container_list[index].textBoxes[i]->object.id);
		}

		/* Draw the border */
		GUI_DrawBorder(container_list[index].object);

		container_list[index].object.displayState = GUIDisplayState_NotHidden;
	}
}

/**
 * @brief	Get the display state of a container
 * @param	ContainerId: The container to get the state for
 * @retval	The display state if valid ID, otherwise GUIDisplayState_NoState
 */
GUIDisplayState GUI_GetDisplayStateForContainer(uint32_t ContainerId)
{
	uint32_t index = ContainerId - guiConfigCONTAINER_ID_OFFSET;

	if (index < guiConfigNUMBER_OF_CONTAINERS)
		return container_list[index].object.displayState;
	else
		return GUIDisplayState_NoState;
}

/* Private functions ---------------------------------------------------------*/
/**
 * @brief	Convert an int32_t to a c-string
 * @param	Number:
 * @param	Buffer:
 * @retval	None
 * @note	Code from https://code.google.com/p/my-itoa/
 */
static int32_t prvItoa(int32_t Number, uint8_t* Buffer)
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
