/**
 ******************************************************************************
 * @file	gui_can2.c
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-09-21
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
#include "gui_can2.h"

/* Private defines -----------------------------------------------------------*/
/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static GUITextBox prvTextBox = {0};
static GUIButton prvButton = {0};
static GUIContainer prvContainer = {0};

/* Private function prototypes -----------------------------------------------*/
/* Functions -----------------------------------------------------------------*/
/* CAN2 GUI Elements ========================================================*/
/**
 * @brief	Manages how data is displayed in the main text box when the source is CAN2
 * @param	None
 * @retval	None
 */
void guiCan2ManageMainTextBox()
{

}

/**
 * @brief	Callback for the enable button
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiCan2EnableButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	static bool enabled = false;

	if (Event == GUITouchEvent_Up)
	{
		if (enabled)
		{
			ErrorStatus status = can2SetConnection(CANConnection_Disconnected);
			if (status == SUCCESS)
			{
				enabled = false;
				GUI_SetButtonTextForRow(guiConfigCAN2_ENABLE_BUTTON_ID, "Disabled", 1);
				GUI_SetButtonState(guiConfigCAN2_TOP_BUTTON_ID, GUIButtonState_Disabled);

				GUI_SetButtonTextForRow(guiConfigCAN2_BIT_RATE_BUTTON_ID, "< Bit Rate:", 0);
				GUI_SetButtonState(guiConfigCAN2_BIT_RATE_BUTTON_ID, GUIButtonState_Disabled);
			}
		}
		else
		{
			ErrorStatus status = can2SetConnection(CANConnection_Connected);
			if (status == SUCCESS)
			{
				enabled = true;
				GUI_SetButtonTextForRow(guiConfigCAN2_ENABLE_BUTTON_ID, "Enabled ", 1);
				GUI_SetButtonState(guiConfigCAN2_TOP_BUTTON_ID, GUIButtonState_Enabled);

				GUI_SetButtonTextForRow(guiConfigCAN2_BIT_RATE_BUTTON_ID, "  Bit Rate:", 0);
				GUI_SetButtonState(guiConfigCAN2_BIT_RATE_BUTTON_ID, GUIButtonState_DisabledTouch);
			}
		}
	}
}

/**
 * @brief	Callback for the termination button
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiCan2TerminationButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	static bool terminated = false;

	if (Event == GUITouchEvent_Up)
	{
		if (terminated)
		{
			ErrorStatus status = can2SetTermination(CANTermination_Disconnected);
			if (status == SUCCESS)
			{
				terminated = false;
				GUI_SetButtonTextForRow(guiConfigCAN2_TERMINATION_BUTTON_ID, "None ", 1);
			}
		}
		else
		{
			ErrorStatus status = can2SetTermination(CANTermination_Connected);
			if (status == SUCCESS)
			{
				terminated = true;
				GUI_SetButtonTextForRow(guiConfigCAN2_TERMINATION_BUTTON_ID, "120 R ", 1);
			}
		}
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiCan2TopButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(guiConfigSIDEBAR_CAN2_CONTAINER_ID);
		lcdChangeDisplayStateOfSidebar(guiConfigSIDEBAR_CAN2_CONTAINER_ID);
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiCan2BitRateButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(guiConfigPOPOUT_CAN2_BIT_RATE_CONTAINER_ID);

		if (displayState == GUIDisplayState_Hidden)
		{
			GUI_SetActiveLayer(GUILayer_1);
			GUI_SetLayerForButton(guiConfigCAN2_BIT_RATE_BUTTON_ID, GUILayer_1);
			GUI_SetButtonState(guiConfigCAN2_BIT_RATE_BUTTON_ID, GUIButtonState_Enabled);
			GUI_DrawContainer(guiConfigPOPOUT_CAN2_BIT_RATE_CONTAINER_ID);
		}
		else if (displayState == GUIDisplayState_NotHidden)
		{
			GUI_HideContainer(guiConfigPOPOUT_CAN2_BIT_RATE_CONTAINER_ID);
			GUI_SetActiveLayer(GUILayer_0);
			GUI_SetLayerForButton(guiConfigCAN2_BIT_RATE_BUTTON_ID, GUILayer_0);
			GUI_SetButtonState(guiConfigCAN2_BIT_RATE_BUTTON_ID, GUIButtonState_Disabled);

			/* Refresh the main text box */
			lcdActiveMainTextBoxManagerShouldRefresh();
		}
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiCan2BitRateSelectionCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		CANBitRate newBitRate;
		switch (ButtonId)
		{
			case guiConfigCAN2_BIT10K_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigCAN2_BIT_RATE_BUTTON_ID, " 10kbit/s", 1);
				newBitRate = CANBitRate_10k;
				break;
			case guiConfigCAN2_BIT20K_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigCAN2_BIT_RATE_BUTTON_ID, " 20kbit/s", 1);
				newBitRate = CANBitRate_20k;
				break;
			case guiConfigCAN2_BIT50K_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigCAN2_BIT_RATE_BUTTON_ID, " 50kbit/s", 1);
				newBitRate = CANBitRate_50k;
				break;
			case guiConfigCAN2_BIT100K_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigCAN2_BIT_RATE_BUTTON_ID, "100kbit/s", 1);
				newBitRate = CANBitRate_100k;
				break;
			case guiConfigCAN2_BIT125K_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigCAN2_BIT_RATE_BUTTON_ID, "125kbit/s", 1);
				newBitRate = CANBitRate_125k;
				break;
			case guiConfigCAN2_BIT250K_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigCAN2_BIT_RATE_BUTTON_ID, "250kbit/s", 1);
				newBitRate = CANBitRate_250k;
				break;
			case guiConfigCAN2_BIT500K_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigCAN2_BIT_RATE_BUTTON_ID, "500kbit/s", 1);
				newBitRate = CANBitRate_500k;
				break;
			case guiConfigCAN2_BIT1M_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigCAN2_BIT_RATE_BUTTON_ID, "  1Mbit/s", 1);
				newBitRate = CANBitRate_1M;
				break;
			default:
				newBitRate = 0;
				break;
		}

		CANSettings* settings = can2GetSettings();
		SemaphoreHandle_t* settingsSemaphore = can2GetSettingsSemaphore();
		/* Try to take the settings semaphore */
		if (newBitRate != 0 && *settingsSemaphore != 0 && xSemaphoreTake(*settingsSemaphore, 100) == pdTRUE)
		{
			settings->bitRate = newBitRate;
			can2UpdateWithNewSettings();

			/* Restart the channel if it was on */
			if (settings->connection == CANConnection_Connected)
				can2Restart();

			/* Give back the semaphore now that we are done */
			xSemaphoreGive(*settingsSemaphore);
		}

		/* Hide the pop out */
		GUI_HideContainer(guiConfigPOPOUT_CAN2_BIT_RATE_CONTAINER_ID);
		GUI_SetActiveLayer(GUILayer_0);
		GUI_SetLayerForButton(guiConfigCAN2_BIT_RATE_BUTTON_ID, GUILayer_0);
		GUI_SetButtonState(guiConfigCAN2_BIT_RATE_BUTTON_ID, GUIButtonState_Disabled);

		/* Refresh the main text box */
		lcdActiveMainTextBoxManagerShouldRefresh();
	}
}

/**
 * @brief	Update the GUI elements for this channel that are dependent on the value of the settings
 * @param	None
 * @retval	None
 */
void guiCan2UpdateGuiElementsReadFromSettings()
{
	/* Get the current settings */
	CANSettings* settings = can2GetSettings();
//	/* Update the baud rate text to match what is actually set */
//	switch (settings->baudRate)
//	{
//		case UARTBaudRate_4800:
//			GUI_SetButtonTextForRow(guiConfigUART1_BAUD_RATE_BUTTON_ID, "  4800 bps", 1);
//			break;
//		case UARTBaudRate_7200:
//			GUI_SetButtonTextForRow(guiConfigUART1_BAUD_RATE_BUTTON_ID, "  7200 bps", 1);
//			break;
//		case UARTBaudRate_9600:
//			GUI_SetButtonTextForRow(guiConfigUART1_BAUD_RATE_BUTTON_ID, "  9600 bps", 1);
//			break;
//		case UARTBaudRate_19200:
//			GUI_SetButtonTextForRow(guiConfigUART1_BAUD_RATE_BUTTON_ID, " 19200 bps", 1);
//			break;
//		case UARTBaudRate_28800:
//			GUI_SetButtonTextForRow(guiConfigUART1_BAUD_RATE_BUTTON_ID, " 28800 bps", 1);
//			break;
//		case UARTBaudRate_38400:
//			GUI_SetButtonTextForRow(guiConfigUART1_BAUD_RATE_BUTTON_ID, " 38400 bps", 1);
//			break;
//		case UARTBaudRate_57600:
//			GUI_SetButtonTextForRow(guiConfigUART1_BAUD_RATE_BUTTON_ID, " 57600 bps", 1);
//			break;
//		case UARTBaudRate_115200:
//			GUI_SetButtonTextForRow(guiConfigUART1_BAUD_RATE_BUTTON_ID, "115200 bps", 1);
//			break;
//		default:
//			break;
//	}
//	/* Update the write format text to match what is actually set */
//	switch (settings->writeFormat)
//	{
//		case GUIWriteFormat_ASCII:
//			GUI_SetButtonTextForRow(guiConfigUART1_FORMAT_BUTTON_ID, "ASCII", 1);
//			break;
//		case GUIWriteFormat_Hex:
//			GUI_SetButtonTextForRow(guiConfigUART1_FORMAT_BUTTON_ID, " Hex ", 1);
//			break;
//		default:
//			break;
//	}
}

/**
 * @brief
 * @param	None
 * @retval	None
 */
void guiCan2InitGuiElements()
{
	/* Text boxes ----------------------------------------------------------------*/
	/* CAN2 Label text box */
	prvTextBox.object.id = guiConfigCAN2_LABEL_TEXT_BOX_ID;
	prvTextBox.object.xPos = 650;
	prvTextBox.object.yPos = 50;
	prvTextBox.object.width = 150;
	prvTextBox.object.height = 50;
	prvTextBox.object.layer = GUILayer_0;
	prvTextBox.object.displayState = GUIDisplayState_Hidden;
	prvTextBox.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvTextBox.object.borderThickness = 1;
	prvTextBox.object.borderColor = GUI_WHITE;
	prvTextBox.textColor = GUI_RED;
	prvTextBox.backgroundColor = GUI_WHITE;
	prvTextBox.staticText = "CAN2";
	prvTextBox.textSize = LCDFontEnlarge_2x;
	GUI_AddTextBox(&prvTextBox);

	/* Buttons -------------------------------------------------------------------*/
	/* CAN2 Top Button */
	prvButton.object.id = guiConfigCAN2_TOP_BUTTON_ID;
	prvButton.object.xPos = 100;
	prvButton.object.yPos = 0;
	prvButton.object.width = 100;
	prvButton.object.height = 50;
	prvButton.object.layer = GUILayer_0;
	prvButton.object.displayState = GUIDisplayState_NotHidden;
	prvButton.object.border = GUIBorder_Bottom | GUIBorder_Right | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_RED;
	prvButton.disabledTextColor = GUI_RED;
	prvButton.disabledBackgroundColor = LCD_COLOR_BLACK;
	prvButton.pressedTextColor = GUI_RED;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiCan2TopButtonCallback;
	prvButton.text[0] = "CAN2";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* CAN2 Enable Button */
	prvButton.object.id = guiConfigCAN2_ENABLE_BUTTON_ID;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 100;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.layer = GUILayer_0;
	prvButton.object.displayState = GUIDisplayState_Hidden;
	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_RED;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_RED;
	prvButton.pressedTextColor = GUI_RED;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiCan2EnableButtonCallback;
	prvButton.text[0] = "Output:";
//	prvButton.text[1] = "Enabled ";
	prvButton.text[1] = "Disabled";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* CAN2 Bit Rate Button */
	prvButton.object.id = guiConfigCAN2_BIT_RATE_BUTTON_ID;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 150;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.layer = GUILayer_0;
	prvButton.object.displayState = GUIDisplayState_Hidden;
	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_DARK_RED;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_RED;
	prvButton.pressedTextColor = GUI_RED;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiCan2BitRateButtonCallback;
	prvButton.text[0] = "< Bit Rate:";
	prvButton.text[1] = "125kbit/s";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* CAN2 Termination Button */
	prvButton.object.id = guiConfigCAN2_TERMINATION_BUTTON_ID;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 200;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.layer = GUILayer_0;
	prvButton.object.displayState = GUIDisplayState_Hidden;
	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_RED;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_RED;
	prvButton.pressedTextColor = GUI_RED;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiCan2TerminationButtonCallback;
	prvButton.text[0] = "Termination:";
	prvButton.text[1] = "None ";
//	prvButton.text[1] = "120 R";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);


	/* CAN2 10k bit rate Button */
	prvButton.object.id = guiConfigCAN2_BIT10K_BUTTON_ID;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 150;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.displayState = GUIDisplayState_Hidden;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_RED;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_RED;
	prvButton.pressedTextColor = GUI_RED;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiCan2BitRateSelectionCallback;
	prvButton.text[0] = "10kbit/s";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* CAN2 20k bit rate Button */
	prvButton.object.id = guiConfigCAN2_BIT20K_BUTTON_ID;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 190;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.displayState = GUIDisplayState_Hidden;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_RED;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_RED;
	prvButton.pressedTextColor = GUI_RED;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiCan2BitRateSelectionCallback;
	prvButton.text[0] = "20kbit/s";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* CAN2 50k bit rate Button */
	prvButton.object.id = guiConfigCAN2_BIT50K_BUTTON_ID;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 230;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.displayState = GUIDisplayState_Hidden;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_RED;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_RED;
	prvButton.pressedTextColor = GUI_RED;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiCan2BitRateSelectionCallback;
	prvButton.text[0] = "50kbit/s";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* CAN2 100k bit rate Button */
	prvButton.object.id = guiConfigCAN2_BIT100K_BUTTON_ID;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 270;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.displayState = GUIDisplayState_Hidden;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_RED;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_RED;
	prvButton.pressedTextColor = GUI_RED;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiCan2BitRateSelectionCallback;
	prvButton.text[0] = "100kbit/s";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* CAN2 125k bit rate Button */
	prvButton.object.id = guiConfigCAN2_BIT125K_BUTTON_ID;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 310;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.displayState = GUIDisplayState_Hidden;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_RED;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_RED;
	prvButton.pressedTextColor = GUI_RED;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiCan2BitRateSelectionCallback;
	prvButton.text[0] = "125kbit/s";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* CAN2 250k bit rate Button */
	prvButton.object.id = guiConfigCAN2_BIT250K_BUTTON_ID;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 350;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.displayState = GUIDisplayState_Hidden;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_RED;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_RED;
	prvButton.pressedTextColor = GUI_RED;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiCan2BitRateSelectionCallback;
	prvButton.text[0] = "250kbit/s";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* CAN2 500k bit rate Button */
	prvButton.object.id = guiConfigCAN2_BIT500K_BUTTON_ID;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 390;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.displayState = GUIDisplayState_Hidden;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_RED;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_RED;
	prvButton.pressedTextColor = GUI_RED;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiCan2BitRateSelectionCallback;
	prvButton.text[0] = "500kbit/s";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* CAN2 1M bit rate Button */
	prvButton.object.id = guiConfigCAN2_BIT1M_BUTTON_ID;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 430;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.displayState = GUIDisplayState_Hidden;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_RED;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_RED;
	prvButton.pressedTextColor = GUI_RED;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiCan2BitRateSelectionCallback;
	prvButton.text[0] = "1Mbit/s";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* Containers ----------------------------------------------------------------*/
	/* Sidebar CAN2 container */
	prvContainer.object.id = guiConfigSIDEBAR_CAN2_CONTAINER_ID;
	prvContainer.object.xPos = 650;
	prvContainer.object.yPos = 50;
	prvContainer.object.width = 150;
	prvContainer.object.height = 405;
	prvContainer.object.layer = GUILayer_0;
	prvContainer.object.displayState = GUIDisplayState_Hidden;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 1;
	prvContainer.object.borderColor = GUI_WHITE;
	prvContainer.contentHideState = GUIHideState_KeepBorders;
	prvContainer.buttons[0] = GUI_GetButtonFromId(guiConfigCAN2_ENABLE_BUTTON_ID);
	prvContainer.buttons[1] = GUI_GetButtonFromId(guiConfigCAN2_BIT_RATE_BUTTON_ID);
	prvContainer.buttons[2] = GUI_GetButtonFromId(guiConfigCAN2_TERMINATION_BUTTON_ID);
	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(guiConfigCAN2_LABEL_TEXT_BOX_ID);
	GUI_AddContainer(&prvContainer);

	/* CAN2 bit rate popout container */
	prvContainer.object.id = guiConfigPOPOUT_CAN2_BIT_RATE_CONTAINER_ID;
	prvContainer.object.xPos = 500;
	prvContainer.object.yPos = 150;
	prvContainer.object.width = 149;
	prvContainer.object.height = 320;
	prvContainer.object.layer = GUILayer_1;
	prvContainer.object.displayState = GUIDisplayState_Hidden;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 2;
	prvContainer.object.borderColor = GUI_WHITE;
	prvContainer.contentHideState = GUIHideState_HideAll;
	prvContainer.buttons[0] = GUI_GetButtonFromId(guiConfigCAN2_BIT10K_BUTTON_ID);
	prvContainer.buttons[1] = GUI_GetButtonFromId(guiConfigCAN2_BIT20K_BUTTON_ID);
	prvContainer.buttons[2] = GUI_GetButtonFromId(guiConfigCAN2_BIT50K_BUTTON_ID);
	prvContainer.buttons[3] = GUI_GetButtonFromId(guiConfigCAN2_BIT100K_BUTTON_ID);
	prvContainer.buttons[4] = GUI_GetButtonFromId(guiConfigCAN2_BIT125K_BUTTON_ID);
	prvContainer.buttons[5] = GUI_GetButtonFromId(guiConfigCAN2_BIT250K_BUTTON_ID);
	prvContainer.buttons[6] = GUI_GetButtonFromId(guiConfigCAN2_BIT500K_BUTTON_ID);
	prvContainer.buttons[7] = GUI_GetButtonFromId(guiConfigCAN2_BIT1M_BUTTON_ID);
	GUI_AddContainer(&prvContainer);
}

/* Interrupt Handlers --------------------------------------------------------*/
