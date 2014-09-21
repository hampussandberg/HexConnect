/**
 ******************************************************************************
 * @file	gui_uart1.c
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
#include "gui_uart1.h"

/* Private defines -----------------------------------------------------------*/
/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static GUITextBox prvTextBox = {0};
static GUIButton prvButton = {0};
static GUIContainer prvContainer = {0};

/* Private function prototypes -----------------------------------------------*/

/* Functions -----------------------------------------------------------------*/
/* UART1 GUI Elements ========================================================*/
/**
 * @brief	Manages how data is displayed in the main text box when the source is UART1
 * @param	None
 * @retval	None
 */
void guiUart1ManageMainTextBox()
{
	const uint32_t constStartFlashAddress = FLASH_ADR_UART1_DATA;

	/* Get the current write address, this is the address where the last data is */
	uint32_t currentWriteAddress = uart1GetCurrentWriteAddress();
	/* Get the current settings of the channel */
	UARTSettings* settings = uart1GetSettings();
	SemaphoreHandle_t* settingsSemaphore = uart1GetSettingsSemaphore();

	lcdManageGenericUartMainTextBox(constStartFlashAddress, currentWriteAddress, settings, settingsSemaphore);
}

/**
 * @brief	Callback for the enable button
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiUart1EnableButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	static bool enabled = false;

	if (Event == GUITouchEvent_Up)
	{
		if (enabled)
		{
			ErrorStatus status = uart1SetConnection(UARTConnection_Disconnected);
			if (status == SUCCESS)
			{
				enabled = false;
				GUI_SetButtonTextForRow(guiConfigUART1_ENABLE_BUTTON_ID, "Disabled", 1);
				GUI_SetButtonState(guiConfigUART1_TOP_BUTTON_ID, GUIButtonState_Disabled);

				GUI_SetButtonTextForRow(guiConfigUART1_BAUD_RATE_BUTTON_ID, "< Baud Rate:", 0);
				GUI_SetButtonState(guiConfigUART1_BAUD_RATE_BUTTON_ID, GUIButtonState_Disabled);
			}
		}
		else
		{
			ErrorStatus status = uart1SetConnection(UARTConnection_Connected);
			if (status == SUCCESS)
			{
				enabled = true;
				GUI_SetButtonTextForRow(guiConfigUART1_ENABLE_BUTTON_ID, "Enabled ", 1);
				GUI_SetButtonState(guiConfigUART1_TOP_BUTTON_ID, GUIButtonState_Enabled);

				GUI_SetButtonTextForRow(guiConfigUART1_BAUD_RATE_BUTTON_ID, "  Baud Rate:", 0);
				GUI_SetButtonState(guiConfigUART1_BAUD_RATE_BUTTON_ID, GUIButtonState_DisabledTouch);
			}
		}
	}
}

/**
 * @brief	Callback for the voltage level button
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiUart1VoltageLevelButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	static bool level5VisActive = false;

	if (Event == GUITouchEvent_Up)
	{
		if (level5VisActive)
		{
			ErrorStatus status = uart1SetPower(UARTPower_3V3);
			if (status == SUCCESS)
			{
				level5VisActive = false;
				GUI_SetButtonTextForRow(guiConfigUART1_VOLTAGE_LEVEL_BUTTON_ID, "3.3 V", 1);
			}
		}
		else
		{
			ErrorStatus status = uart1SetPower(UARTPower_5V);
			if (status == SUCCESS)
			{
				level5VisActive = true;
				GUI_SetButtonTextForRow(guiConfigUART1_VOLTAGE_LEVEL_BUTTON_ID, " 5 V ", 1);
			}
		}
	}
}

/**
 * @brief	Callback for the format button
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiUart1FormatButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		UARTSettings* settings = uart1GetSettings();
		SemaphoreHandle_t* settingsSemaphore = uart1GetSettingsSemaphore();
		/* Try to take the settings semaphore */
		if (*settingsSemaphore != 0 && xSemaphoreTake(*settingsSemaphore, 100) == pdTRUE)
		{
			if (settings->writeFormat == GUIWriteFormat_ASCII)
			{
				settings->writeFormat = GUIWriteFormat_Hex;
				settings->numOfCharactersPerByte = 3;
				GUI_SetButtonTextForRow(guiConfigUART1_FORMAT_BUTTON_ID, " Hex ", 1);
			}
			else if (settings->writeFormat == GUIWriteFormat_Hex)
			{
				settings->writeFormat = GUIWriteFormat_ASCII;
				settings->numOfCharactersPerByte = 1;
				GUI_SetButtonTextForRow(guiConfigUART1_FORMAT_BUTTON_ID, "ASCII", 1);
			}

			/* Give back the semaphore now that we are done */
			xSemaphoreGive(*settingsSemaphore);
		}
	}
}

/**
 * @brief	Callback for the debug button
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiUart1DebugButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	static bool enabled = false;
	static UARTMode lastMode;

	if (Event == GUITouchEvent_Up)
	{
		UARTSettings* settings = uart1GetSettings();
		SemaphoreHandle_t* settingsSemaphore = uart1GetSettingsSemaphore();
		/* Try to take the settings semaphore */
		if (*settingsSemaphore != 0 && xSemaphoreTake(*settingsSemaphore, 100) == pdTRUE)
		{
			if (enabled)
			{
				settings->mode = lastMode;
				enabled = false;
				GUI_SetButtonTextForRow(guiConfigUART1_DEBUG_BUTTON_ID, "Disabled", 1);
			}
			else
			{
				lastMode = settings->mode;
				settings->mode = UARTMode_DebugTX;
				enabled = true;
				GUI_SetButtonTextForRow(guiConfigUART1_DEBUG_BUTTON_ID, "Enabled ", 1);
			}

			/* Give back the semaphore now that we are done */
			xSemaphoreGive(*settingsSemaphore);
		}
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiUart1TopButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		/* Get the current display state of the sidebar */
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(guiConfigSIDEBAR_UART1_CONTAINER_ID);
		/* Change the state of the sidebar */
		lcdChangeDisplayStateOfSidebar(guiConfigSIDEBAR_UART1_CONTAINER_ID);
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiUart1BaudRateButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(guiConfigPOPOUT_UART1_BAUD_RATE_CONTAINER_ID);

		if (displayState == GUIDisplayState_Hidden)
		{
			GUI_SetActiveLayer(GUILayer_1);
			GUI_SetLayerForButton(guiConfigUART1_BAUD_RATE_BUTTON_ID, GUILayer_1);
			GUI_SetButtonState(guiConfigUART1_BAUD_RATE_BUTTON_ID, GUIButtonState_Enabled);
			GUI_DrawContainer(guiConfigPOPOUT_UART1_BAUD_RATE_CONTAINER_ID);
		}
		else if (displayState == GUIDisplayState_NotHidden)
		{
			GUI_HideContainer(guiConfigPOPOUT_UART1_BAUD_RATE_CONTAINER_ID);
			GUI_SetActiveLayer(GUILayer_0);
			GUI_SetLayerForButton(guiConfigUART1_BAUD_RATE_BUTTON_ID, GUILayer_0);
			GUI_SetButtonState(guiConfigUART1_BAUD_RATE_BUTTON_ID, GUIButtonState_Disabled);

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
void guiUart1BaudRateSelectionCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		UARTBaudRate newBaudRate;
		switch (ButtonId)
		{
			case guiConfigUART1_BAUD4800_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigUART1_BAUD_RATE_BUTTON_ID, "  4800 bps", 1);
				newBaudRate = UARTBaudRate_4800;
				break;
			case guiConfigUART1_BAUD7200_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigUART1_BAUD_RATE_BUTTON_ID, "  7200 bps", 1);
				newBaudRate = UARTBaudRate_7200;
				break;
			case guiConfigUART1_BAUD9600_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigUART1_BAUD_RATE_BUTTON_ID, "  9600 bps", 1);
				newBaudRate = UARTBaudRate_9600;
				break;
			case guiConfigUART1_BAUD19K2_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigUART1_BAUD_RATE_BUTTON_ID, " 19200 bps", 1);
				newBaudRate = UARTBaudRate_19200;
				break;
			case guiConfigUART1_BAUD28K8_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigUART1_BAUD_RATE_BUTTON_ID, " 28800 bps", 1);
				newBaudRate = UARTBaudRate_28800;
				break;
			case guiConfigUART1_BAUD38K4_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigUART1_BAUD_RATE_BUTTON_ID, " 38400 bps", 1);
				newBaudRate = UARTBaudRate_38400;
				break;
			case guiConfigUART1_BAUD57K6_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigUART1_BAUD_RATE_BUTTON_ID, " 57600 bps", 1);
				newBaudRate = UARTBaudRate_57600;
				break;
			case guiConfigUART1_BAUD115K_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigUART1_BAUD_RATE_BUTTON_ID, "115200 bps", 1);
				newBaudRate = UARTBaudRate_115200;
				break;
			default:
				newBaudRate = 0;
				break;
		}

		UARTSettings* settings = uart1GetSettings();
		SemaphoreHandle_t* settingsSemaphore = uart1GetSettingsSemaphore();
		/* Try to take the settings semaphore */
		if (newBaudRate != 0 && *settingsSemaphore != 0 && xSemaphoreTake(*settingsSemaphore, 100) == pdTRUE)
		{
			settings->baudRate = newBaudRate;
			uart1UpdateWithNewSettings();

			/* Restart the channel if it was on */
			if (settings->connection == UARTConnection_Connected)
				uart1Restart();

			/* Give back the semaphore now that we are done */
			xSemaphoreGive(*settingsSemaphore);
		}

		/* Hide the pop out */
		GUI_HideContainer(guiConfigPOPOUT_UART1_BAUD_RATE_CONTAINER_ID);
		GUI_SetActiveLayer(GUILayer_0);
		GUI_SetLayerForButton(guiConfigUART1_BAUD_RATE_BUTTON_ID, GUILayer_0);
		GUI_SetButtonState(guiConfigUART1_BAUD_RATE_BUTTON_ID, GUIButtonState_Disabled);

		/* Refresh the main text box */
		lcdActiveMainTextBoxManagerShouldRefresh();
	}
}

/**
 * @brief	Update the GUI elements for this channel that are dependent on the value of the settings
 * @param	None
 * @retval	None
 */
void guiUart1UpdateGuiElementsReadFromSettings()
{
	/* Get the current settings */
	UARTSettings* settings = uart1GetSettings();
	/* Update the baud rate text to match what is actually set */
	switch (settings->baudRate)
	{
		case UARTBaudRate_4800:
			GUI_SetButtonTextForRow(guiConfigUART1_BAUD_RATE_BUTTON_ID, "  4800 bps", 1);
			break;
		case UARTBaudRate_7200:
			GUI_SetButtonTextForRow(guiConfigUART1_BAUD_RATE_BUTTON_ID, "  7200 bps", 1);
			break;
		case UARTBaudRate_9600:
			GUI_SetButtonTextForRow(guiConfigUART1_BAUD_RATE_BUTTON_ID, "  9600 bps", 1);
			break;
		case UARTBaudRate_19200:
			GUI_SetButtonTextForRow(guiConfigUART1_BAUD_RATE_BUTTON_ID, " 19200 bps", 1);
			break;
		case UARTBaudRate_28800:
			GUI_SetButtonTextForRow(guiConfigUART1_BAUD_RATE_BUTTON_ID, " 28800 bps", 1);
			break;
		case UARTBaudRate_38400:
			GUI_SetButtonTextForRow(guiConfigUART1_BAUD_RATE_BUTTON_ID, " 38400 bps", 1);
			break;
		case UARTBaudRate_57600:
			GUI_SetButtonTextForRow(guiConfigUART1_BAUD_RATE_BUTTON_ID, " 57600 bps", 1);
			break;
		case UARTBaudRate_115200:
			GUI_SetButtonTextForRow(guiConfigUART1_BAUD_RATE_BUTTON_ID, "115200 bps", 1);
			break;
		default:
			break;
	}
	/* Update the write format text to match what is actually set */
	switch (settings->writeFormat)
	{
		case GUIWriteFormat_ASCII:
			GUI_SetButtonTextForRow(guiConfigUART1_FORMAT_BUTTON_ID, "ASCII", 1);
			break;
		case GUIWriteFormat_Hex:
			GUI_SetButtonTextForRow(guiConfigUART1_FORMAT_BUTTON_ID, " Hex ", 1);
			break;
		default:
			break;
	}
}

/**
 * @brief
 * @param	None
 * @retval	None
 */
void guiUart1InitGuiElements()
{
	/* Text boxes ----------------------------------------------------------------*/
	/* UART1 Label text box */
	prvTextBox.object.id = guiConfigUART1_LABEL_TEXT_BOX_ID;
	prvTextBox.object.xPos = 650;
	prvTextBox.object.yPos = 50;
	prvTextBox.object.width = 150;
	prvTextBox.object.height = 50;
	prvTextBox.object.layer = GUILayer_0;
	prvTextBox.object.displayState = GUIDisplayState_Hidden;
	prvTextBox.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvTextBox.object.borderThickness = 1;
	prvTextBox.object.borderColor = GUI_WHITE;
	prvTextBox.textColor = GUI_GREEN;
	prvTextBox.backgroundColor = GUI_WHITE;
	prvTextBox.staticText = "UART1";
	prvTextBox.textSize = LCDFontEnlarge_2x;
	GUI_AddTextBox(&prvTextBox);

	/* Buttons -------------------------------------------------------------------*/
	/* UART1 Top Button */
	prvButton.object.id = guiConfigUART1_TOP_BUTTON_ID;
	prvButton.object.xPos = 200;
	prvButton.object.yPos = 0;
	prvButton.object.width = 100;
	prvButton.object.height = 50;
	prvButton.object.layer = GUILayer_0;
	prvButton.object.displayState = GUIDisplayState_NotHidden;
	prvButton.object.border = GUIBorder_Bottom | GUIBorder_Right | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_GREEN;
	prvButton.disabledTextColor = GUI_GREEN;
	prvButton.disabledBackgroundColor = LCD_COLOR_BLACK;
	prvButton.pressedTextColor = GUI_GREEN;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiUart1TopButtonCallback;
	prvButton.text[0] = "UART1";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* UART1 Enable Button */
	prvButton.object.id = guiConfigUART1_ENABLE_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_GREEN;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_GREEN;
	prvButton.pressedTextColor = GUI_GREEN;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiUart1EnableButtonCallback;
	prvButton.text[0] = "Output:";
//	prvButton.text[1] = "Enabled ";
	prvButton.text[1] = "Disabled";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* UART1 Baud Rate Button */
	prvButton.object.id = guiConfigUART1_BAUD_RATE_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_DARK_GREEN;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_GREEN;
	prvButton.pressedTextColor = GUI_GREEN;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiUart1BaudRateButtonCallback;
	prvButton.text[0] = "< Baud Rate:";
	prvButton.text[1] = "115200 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* UART1 Voltage Level Button */
	prvButton.object.id = guiConfigUART1_VOLTAGE_LEVEL_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_GREEN;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_GREEN;
	prvButton.pressedTextColor = GUI_GREEN;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiUart1VoltageLevelButtonCallback;
	prvButton.text[0] = "Voltage Level:";
	prvButton.text[1] = " 5 V ";
//	prvButton.text[1] = "3.3 V";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* UART1 Format Button */
	prvButton.object.id = guiConfigUART1_FORMAT_BUTTON_ID;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 250;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.layer = GUILayer_0;
	prvButton.object.displayState = GUIDisplayState_Hidden;
	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_GREEN;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_GREEN;
	prvButton.pressedTextColor = GUI_GREEN;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiUart1FormatButtonCallback;
	prvButton.text[0] = "Display Format:";
	prvButton.text[1] = "ASCII";
//	prvButton.text[1] = " HEX ";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* UART1 Clear Button */
	prvButton.object.id = guiConfigUART1_CLEAR_BUTTON_ID;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 300;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.layer = GUILayer_0;
	prvButton.object.displayState = GUIDisplayState_Hidden;
	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_GREEN;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_GREEN;
	prvButton.pressedTextColor = GUI_GREEN;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = lcdGenericUartClearButtonCallback;
	prvButton.text[0] = "Clear";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* UART1 Debug Button */
	prvButton.object.id = guiConfigUART1_DEBUG_BUTTON_ID;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 350;
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
	prvButton.touchCallback = guiUart1DebugButtonCallback;
	prvButton.text[0] = "Debug TX:";
	prvButton.text[1] = "Disabled";
//	prvButton.text[1] = "Enabled ";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);


	/* UART1 4800 bps Button */
	prvButton.object.id = guiConfigUART1_BAUD4800_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_GREEN;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_GREEN;
	prvButton.pressedTextColor = GUI_GREEN;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiUart1BaudRateSelectionCallback;
	prvButton.text[0] = "4800 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* UART1 7200 bps Button */
	prvButton.object.id = guiConfigUART1_BAUD7200_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_GREEN;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_GREEN;
	prvButton.pressedTextColor = GUI_GREEN;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiUart1BaudRateSelectionCallback;
	prvButton.text[0] = "7200 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* UART1 9600 bps Button */
	prvButton.object.id = guiConfigUART1_BAUD9600_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_GREEN;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_GREEN;
	prvButton.pressedTextColor = GUI_GREEN;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiUart1BaudRateSelectionCallback;
	prvButton.text[0] = "9600 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* UART1 19200 bps Button */
	prvButton.object.id = guiConfigUART1_BAUD19K2_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_GREEN;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_GREEN;
	prvButton.pressedTextColor = GUI_GREEN;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiUart1BaudRateSelectionCallback;
	prvButton.text[0] = "19200 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* UART1 28800 bps Button */
	prvButton.object.id = guiConfigUART1_BAUD28K8_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_GREEN;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_GREEN;
	prvButton.pressedTextColor = GUI_GREEN;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiUart1BaudRateSelectionCallback;
	prvButton.text[0] = "28800 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* UART1 38400 bps Button */
	prvButton.object.id = guiConfigUART1_BAUD38K4_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_GREEN;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_GREEN;
	prvButton.pressedTextColor = GUI_GREEN;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiUart1BaudRateSelectionCallback;
	prvButton.text[0] = "38400 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* UART1 57600 bps Button */
	prvButton.object.id = guiConfigUART1_BAUD57K6_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_GREEN;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_GREEN;
	prvButton.pressedTextColor = GUI_GREEN;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiUart1BaudRateSelectionCallback;
	prvButton.text[0] = "57600 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* UART1 115200 bps Button */
	prvButton.object.id = guiConfigUART1_BAUD115K_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_GREEN;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_GREEN;
	prvButton.pressedTextColor = GUI_GREEN;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiUart1BaudRateSelectionCallback;
	prvButton.text[0] = "115200 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);


	/* Containers ----------------------------------------------------------------*/
	/* Sidebar UART1 container */
	prvContainer.object.id = guiConfigSIDEBAR_UART1_CONTAINER_ID;
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
	prvContainer.buttons[0] = GUI_GetButtonFromId(guiConfigUART1_ENABLE_BUTTON_ID);
	prvContainer.buttons[1] = GUI_GetButtonFromId(guiConfigUART1_BAUD_RATE_BUTTON_ID);
	prvContainer.buttons[2] = GUI_GetButtonFromId(guiConfigUART1_VOLTAGE_LEVEL_BUTTON_ID);
	prvContainer.buttons[3] = GUI_GetButtonFromId(guiConfigUART1_FORMAT_BUTTON_ID);
	prvContainer.buttons[4] = GUI_GetButtonFromId(guiConfigUART1_CLEAR_BUTTON_ID);
	prvContainer.buttons[5] = GUI_GetButtonFromId(guiConfigUART1_DEBUG_BUTTON_ID);
	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(guiConfigUART1_LABEL_TEXT_BOX_ID);
	GUI_AddContainer(&prvContainer);

	/* UART1 baud rate popout container */
	prvContainer.object.id = guiConfigPOPOUT_UART1_BAUD_RATE_CONTAINER_ID;
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
	prvContainer.buttons[0] = GUI_GetButtonFromId(guiConfigUART1_BAUD4800_BUTTON_ID);
	prvContainer.buttons[1] = GUI_GetButtonFromId(guiConfigUART1_BAUD7200_BUTTON_ID);
	prvContainer.buttons[2] = GUI_GetButtonFromId(guiConfigUART1_BAUD9600_BUTTON_ID);
	prvContainer.buttons[3] = GUI_GetButtonFromId(guiConfigUART1_BAUD19K2_BUTTON_ID);
	prvContainer.buttons[4] = GUI_GetButtonFromId(guiConfigUART1_BAUD28K8_BUTTON_ID);
	prvContainer.buttons[5] = GUI_GetButtonFromId(guiConfigUART1_BAUD38K4_BUTTON_ID);
	prvContainer.buttons[6] = GUI_GetButtonFromId(guiConfigUART1_BAUD57K6_BUTTON_ID);
	prvContainer.buttons[7] = GUI_GetButtonFromId(guiConfigUART1_BAUD115K_BUTTON_ID);
	GUI_AddContainer(&prvContainer);
}

/* Interrupt Handlers --------------------------------------------------------*/
