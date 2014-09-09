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
int32_t prvGUI_itoa(int32_t Number, uint8_t* Buffer);

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
		button_list[i].object.id = guiConfigINVALID_ID;
		button_list[i].object.xPos = 0;
		button_list[i].object.yPos = 0;
		button_list[i].object.width = 0;
		button_list[i].object.height = 0;
		button_list[i].object.layer = GUILayer_0;
		button_list[i].object.displayState = GUIDisplayState_Hidden;
		button_list[i].object.border = GUIBorder_NoBorder;
		button_list[i].object.borderThickness = 0;
		button_list[i].object.borderThickness = 0;

		button_list[i].enabledTextColor = 0;
		button_list[i].enabledBackgroundColor = 0;
		button_list[i].disabledTextColor = 0;
		button_list[i].disabledBackgroundColor = 0;
		button_list[i].pressedTextColor = 0;
		button_list[i].pressedBackgroundColor = 0;
		button_list[i].state = GUIButtonState_NoState;
		button_list[i].touchCallback = 0;
		button_list[i].text = 0;
		button_list[i].textSize = LCDFontEnlarge_1x;
		button_list[i].numOfChar = 0;
		button_list[i].textWidth = 0;
		button_list[i].textHeight = 0;
	}

	/* Text boxes */
	for (uint32_t i = 0; i < guiConfigNUMBER_OF_TEXT_BOXES; i++)
	{
		textBox_list[i].object.id = guiConfigINVALID_ID;
		textBox_list[i].object.xPos = 0;
		textBox_list[i].object.yPos = 0;
		textBox_list[i].object.width = 0;
		textBox_list[i].object.height = 0;
		textBox_list[i].object.layer = GUILayer_0;
		textBox_list[i].object.displayState = GUIDisplayState_Hidden;
		textBox_list[i].object.border = GUIBorder_NoBorder;
		textBox_list[i].object.borderThickness = 0;
		textBox_list[i].object.borderThickness = 0;

		textBox_list[i].textColor = 0;
		textBox_list[i].backgroundColor = 0;
		textBox_list[i].textSize = LCDFontEnlarge_1x;
		textBox_list[i].xWritePos = 0;
		textBox_list[i].yWritePos = 0;
	}

	/* Pages */
	for (uint32_t i = 0; i < guiConfigNUMBER_OF_CONTAINERS; i++)
	{
		container_list[i].object.id = guiConfigINVALID_ID;
		container_list[i].object.xPos = 0;
		container_list[i].object.yPos = 0;
		container_list[i].object.width = 0;
		container_list[i].object.height = 0;
		container_list[i].object.layer = GUILayer_0;
		container_list[i].object.displayState = GUIDisplayState_Hidden;
		container_list[i].object.border = GUIBorder_NoBorder;
		container_list[i].object.borderThickness = 0;
		container_list[i].object.borderThickness = 0;

		for (uint32_t n = 0; n < guiConfigNUMBER_OF_BUTTONS; n++)
			container_list[i].buttons[n] = 0;
		for (uint32_t n = 0; n < guiConfigNUMBER_OF_TEXT_BOXES; n++)
			container_list[i].textBoxes[n] = 0;
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

		/* Fill in the rest of the data */
		button_list[index].numOfChar = strlen(button_list[index].text);
		button_list[index].textWidth = button_list[index].numOfChar * 8 * button_list[index].textSize;
		button_list[index].textHeight = 16 * button_list[index].textSize;

		/* If it's set to not hidden we should draw the button */
		if (Button->object.displayState == GUIDisplayState_NotHidden)
			GUI_DrawButton(Button->object.id);
	}
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
		/* Set the background color */
		LCD_SetBackgroundColor(LCD_COLOR_BLACK);
		/* Clear the active window */
		LCDActiveWindow window;
		window.xLeft = button_list[index].object.xPos;
		window.xRight = button_list[index].object.xPos + button_list[index].object.width - 1;
		window.yTop = button_list[index].object.yPos;
		window.yBottom = button_list[index].object.yPos + button_list[index].object.height - 1;
		LCD_ClearActiveWindow(window.xLeft, window.xRight, window.yTop, window.yBottom);
		button_list[index].object.displayState = GUIDisplayState_Hidden;
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
		/* Set state colors */
		uint16_t backgroundColor, textColor;
		switch (button_list[index].state) {
			case GUIButtonState_Enabled:
				/* Enabled state */
				backgroundColor = button_list[index].enabledBackgroundColor;
				textColor = button_list[index].enabledTextColor;
				break;
			case GUIButtonState_Disabled:
				/* Disabled state */
				backgroundColor = button_list[index].disabledBackgroundColor;
				textColor = button_list[index].disabledTextColor;
				break;
			case GUIButtonState_TouchDown:
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
							 button_list[index].object.yPos, button_list[index].object.yPos + button_list[index].object.height-1, LCDDrawType_Square, LCDFill_Fill);
		LCD_SetForegroundColor(textColor);
		LCD_SetTextWritePosition(button_list[index].object.xPos + (button_list[index].object.width - button_list[index].textWidth) / 2,
								 button_list[index].object.yPos + (button_list[index].object.height - button_list[index].textHeight) / 2 - 2);
		LCD_WriteString(button_list[index].text, LCDTransparency_Transparent, button_list[index].textSize);

		/* Draw the border */
		GUI_DrawBorder(button_list[index].object);

		button_list[index].object.displayState = GUIDisplayState_NotHidden;
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
 * @brief	Get a pointer to the textbox corresponding to the id
 * @param	TextBoxId: The id of the text box to get
 * @retval	Pointer the textbox or 0 if no textbox was found
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
		/* Copy the textbox to the list */
		memcpy(&textBox_list[index], TextBox, sizeof(GUITextBox));

		/* If it's set to not hidden we should draw the button */
		if (TextBox->object.displayState == GUIDisplayState_NotHidden)
			GUI_DrawTextBox(TextBox->object.id);
	}
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

/**
 * @brief	Get the display state of a textbox
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
