/**
 ******************************************************************************
 * @file	gui_can1.c
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
#include "gui_can1.h"

/* Private defines -----------------------------------------------------------*/
/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static GUITextBox prvTextBox = {0};
static GUIButton prvButton = {0};
static GUIContainer prvContainer = {0};

/* Private function prototypes -----------------------------------------------*/
/* Functions -----------------------------------------------------------------*/
/* CAN1 GUI Elements ========================================================*/
/**
 * @brief	Manages how data is displayed in the main text box when the source is CAN1
 * @param	None
 * @retval	None
 */
void guiCan1ManageMainTextBox()
{

}

/**
 * @brief	Callback for the enable button
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiCan1EnableButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	static bool enabled = false;

	if (Event == GUITouchEvent_Up)
	{
		if (enabled)
		{
			ErrorStatus status = can1SetConnection(CANConnection_Disconnected);
			if (status == SUCCESS)
			{
				enabled = false;
				GUI_SetButtonTextForRow(GUIButtonId_Can1Enable, "Disabled", 1);
				GUI_SetButtonState(GUIButtonId_Can1Top, GUIButtonState_Disabled);

				GUI_SetButtonTextForRow(GUIButtonId_Can1BitRate, "< Bit Rate:", 0);
				GUI_SetButtonState(GUIButtonId_Can1BitRate, GUIButtonState_Disabled);
			}
		}
		else
		{
			ErrorStatus status = can1SetConnection(CANConnection_Connected);
			if (status == SUCCESS)
			{
				enabled = true;
				GUI_SetButtonTextForRow(GUIButtonId_Can1Enable, "Enabled", 1);
				GUI_SetButtonState(GUIButtonId_Can1Top, GUIButtonState_Enabled);

				GUI_SetButtonTextForRow(GUIButtonId_Can1BitRate, "Bit Rate:", 0);
				GUI_SetButtonState(GUIButtonId_Can1BitRate, GUIButtonState_DisabledTouch);
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
void guiCan1TerminationButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	static bool terminated = false;

	if (Event == GUITouchEvent_Up)
	{
		if (terminated)
		{
			ErrorStatus status = can1SetTermination(CANTermination_Disconnected);
			if (status == SUCCESS)
			{
				terminated = false;
				GUI_SetButtonTextForRow(GUIButtonId_Can1Termination, "None", 1);
			}
		}
		else
		{
			ErrorStatus status = can1SetTermination(CANTermination_Connected);
			if (status == SUCCESS)
			{
				terminated = true;
				GUI_SetButtonTextForRow(GUIButtonId_Can1Termination, "120 R", 1);
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
void guiCan1TopButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(GUIContainerId_SidebarCan1);
		lcdChangeDisplayStateOfSidebar(GUIContainerId_SidebarCan1);
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiCan1BitRateButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(GUIContainerId_PopoutCan1BitRate);

		if (displayState == GUIDisplayState_Hidden)
		{
			GUI_SetActiveLayer(GUILayer_1);
			GUI_SetLayerForButton(GUIButtonId_Can1BitRate, GUILayer_1);
			GUI_SetButtonState(GUIButtonId_Can1BitRate, GUIButtonState_Enabled);
			GUI_DrawContainer(GUIContainerId_PopoutCan1BitRate);
		}
		else if (displayState == GUIDisplayState_NotHidden)
		{
			GUI_HideContainer(GUIContainerId_PopoutCan1BitRate);
			GUI_SetActiveLayer(GUILayer_0);
			GUI_SetLayerForButton(GUIButtonId_Can1BitRate, GUILayer_0);
			GUI_SetButtonState(GUIButtonId_Can1BitRate, GUIButtonState_Disabled);

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
void guiCan1BitRateSelectionCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		CANBitRate newBitRate;
		switch (ButtonId)
		{
			case GUIButtonId_Can1BitRate10k:
				GUI_SetButtonTextForRow(GUIButtonId_Can1BitRate, "10kbit/s", 1);
				newBitRate = CANBitRate_10k;
				break;
			case GUIButtonId_Can1BitRate20k:
				GUI_SetButtonTextForRow(GUIButtonId_Can1BitRate, "20kbit/s", 1);
				newBitRate = CANBitRate_20k;
				break;
			case GUIButtonId_Can1BitRate50k:
				GUI_SetButtonTextForRow(GUIButtonId_Can1BitRate, "50kbit/s", 1);
				newBitRate = CANBitRate_50k;
				break;
			case GUIButtonId_Can1BitRate100k:
				GUI_SetButtonTextForRow(GUIButtonId_Can1BitRate, "100kbit/s", 1);
				newBitRate = CANBitRate_100k;
				break;
			case GUIButtonId_Can1BitRate125k:
				GUI_SetButtonTextForRow(GUIButtonId_Can1BitRate, "125kbit/s", 1);
				newBitRate = CANBitRate_125k;
				break;
			case GUIButtonId_Can1BitRate250k:
				GUI_SetButtonTextForRow(GUIButtonId_Can1BitRate, "250kbit/s", 1);
				newBitRate = CANBitRate_250k;
				break;
			case GUIButtonId_Can1BitRate500k:
				GUI_SetButtonTextForRow(GUIButtonId_Can1BitRate, "500kbit/s", 1);
				newBitRate = CANBitRate_500k;
				break;
			case GUIButtonId_Can1BitRate1M:
				GUI_SetButtonTextForRow(GUIButtonId_Can1BitRate, "1Mbit/s", 1);
				newBitRate = CANBitRate_1M;
				break;
			default:
				newBitRate = 0;
				break;
		}

		CANSettings* settings = can1GetSettings();
		SemaphoreHandle_t* settingsSemaphore = can1GetSettingsSemaphore();
		/* Try to take the settings semaphore */
		if (newBitRate != 0 && *settingsSemaphore != 0 && xSemaphoreTake(*settingsSemaphore, 100) == pdTRUE)
		{
			settings->bitRate = newBitRate;
			can1UpdateWithNewSettings();

			/* Restart the channel if it was on */
			if (settings->connection == CANConnection_Connected)
				can1Restart();

			/* Give back the semaphore now that we are done */
			xSemaphoreGive(*settingsSemaphore);
		}

		/* Hide the pop out */
		GUI_HideContainer(GUIContainerId_PopoutCan1BitRate);
		GUI_SetActiveLayer(GUILayer_0);
		GUI_SetLayerForButton(GUIButtonId_Can1BitRate, GUILayer_0);
		GUI_SetButtonState(GUIButtonId_Can1BitRate, GUIButtonState_Disabled);

		/* Refresh the main text box */
		lcdActiveMainTextBoxManagerShouldRefresh();
	}
}

/**
 * @brief	Update the GUI elements for this channel that are dependent on the value of the settings
 * @param	None
 * @retval	None
 */
void guiCan1UpdateGuiElementsReadFromSettings()
{
	/* Get the current settings */
	CANSettings* settings = can1GetSettings();
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
void guiCan1InitGuiElements()
{
	/* Text boxes ----------------------------------------------------------------*/
	/* CAN1 Label text box */
	prvTextBox.object.id = GUITextBoxId_Can1Label;
	prvTextBox.object.xPos = 650;
	prvTextBox.object.yPos = 50;
	prvTextBox.object.width = 150;
	prvTextBox.object.height = 50;
	prvTextBox.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvTextBox.object.borderThickness = 1;
	prvTextBox.object.borderColor = GUI_WHITE;
	prvTextBox.object.containerPage = GUIContainerPage_All;
	prvTextBox.textColor = GUI_BLUE;
	prvTextBox.backgroundColor = GUI_WHITE;
	prvTextBox.staticText = "CAN1";
	prvTextBox.textSize = LCDFontEnlarge_2x;
	GUI_AddTextBox(&prvTextBox);

	/* Buttons -------------------------------------------------------------------*/
	/* CAN1 Top Button */
	prvButton.object.id = GUIButtonId_Can1Top;
	prvButton.object.xPos = 0;
	prvButton.object.yPos = 0;
	prvButton.object.width = 100;
	prvButton.object.height = 50;
	prvButton.object.displayState = GUIDisplayState_NotHidden;
	prvButton.object.border = GUIBorder_Bottom | GUIBorder_Right;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_BLUE;
	prvButton.disabledTextColor = GUI_BLUE;
	prvButton.disabledBackgroundColor = LCD_COLOR_BLACK;
	prvButton.pressedTextColor = GUI_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiCan1TopButtonCallback;
	prvButton.text[0] = "CAN1";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* CAN1 Enable Button */
	prvButton.object.id = GUIButtonId_Can1Enable;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 100;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.object.containerPage = GUIContainerPage_1;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_BLUE;
	prvButton.pressedTextColor = GUI_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiCan1EnableButtonCallback;
	prvButton.text[0] = "Output:";
//	prvButton.text[1] = "Enabled ";
	prvButton.text[1] = "Disabled";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* CAN1 Bit Rate Button */
	prvButton.object.id = GUIButtonId_Can1BitRate;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 150;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.object.containerPage = GUIContainerPage_1;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_DARK_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_BLUE;
	prvButton.pressedTextColor = GUI_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiCan1BitRateButtonCallback;
	prvButton.text[0] = "< Bit Rate:";
	prvButton.text[1] = "125kbit/s";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* CAN1 Termination Button */
	prvButton.object.id = GUIButtonId_Can1Termination;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 200;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.object.containerPage = GUIContainerPage_1;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_BLUE;
	prvButton.pressedTextColor = GUI_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiCan1TerminationButtonCallback;
	prvButton.text[0] = "Termination:";
	prvButton.text[1] = "None";
//	prvButton.text[1] = "120 R";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);


	/* CAN1 10k bit rate Button */
	prvButton.object.id = GUIButtonId_Can1BitRate10k;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 150;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_BLUE;
	prvButton.pressedTextColor = GUI_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiCan1BitRateSelectionCallback;
	prvButton.text[0] = "10kbit/s";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* CAN1 20k bit rate Button */
	prvButton.object.id = GUIButtonId_Can1BitRate20k;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 190;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_BLUE;
	prvButton.pressedTextColor = GUI_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiCan1BitRateSelectionCallback;
	prvButton.text[0] = "20kbit/s";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* CAN1 50k bit rate Button */
	prvButton.object.id = GUIButtonId_Can1BitRate50k;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 230;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_BLUE;
	prvButton.pressedTextColor = GUI_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiCan1BitRateSelectionCallback;
	prvButton.text[0] = "50kbit/s";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* CAN1 100k bit rate Button */
	prvButton.object.id = GUIButtonId_Can1BitRate100k;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 270;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_BLUE;
	prvButton.pressedTextColor = GUI_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiCan1BitRateSelectionCallback;
	prvButton.text[0] = "100kbit/s";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* CAN1 125k bit rate Button */
	prvButton.object.id = GUIButtonId_Can1BitRate125k;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 310;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_BLUE;
	prvButton.pressedTextColor = GUI_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiCan1BitRateSelectionCallback;
	prvButton.text[0] = "125kbit/s";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* CAN1 250k bit rate Button */
	prvButton.object.id = GUIButtonId_Can1BitRate250k;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 350;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_BLUE;
	prvButton.pressedTextColor = GUI_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiCan1BitRateSelectionCallback;
	prvButton.text[0] = "250kbit/s";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* CAN1 500k bit rate Button */
	prvButton.object.id = GUIButtonId_Can1BitRate500k;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 390;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_BLUE;
	prvButton.pressedTextColor = GUI_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiCan1BitRateSelectionCallback;
	prvButton.text[0] = "500kbit/s";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* CAN1 1M bit rate Button */
	prvButton.object.id = GUIButtonId_Can1BitRate1M;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 430;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_BLUE;
	prvButton.pressedTextColor = GUI_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiCan1BitRateSelectionCallback;
	prvButton.text[0] = "1Mbit/s";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* Containers ----------------------------------------------------------------*/
	/* Sidebar CAN1 container */
	prvContainer.object.id = GUIContainerId_SidebarCan1;
	prvContainer.object.xPos = 650;
	prvContainer.object.yPos = 50;
	prvContainer.object.width = 150;
	prvContainer.object.height = 400;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 1;
	prvContainer.object.borderColor = GUI_WHITE;
	prvContainer.activePage = GUIContainerPage_1;
	prvContainer.contentHideState = GUIHideState_KeepBorders;
	prvContainer.buttons[0] = GUI_GetButtonFromId(GUIButtonId_Can1Enable);
	prvContainer.buttons[1] = GUI_GetButtonFromId(GUIButtonId_Can1BitRate);
	prvContainer.buttons[2] = GUI_GetButtonFromId(GUIButtonId_Can1Termination);
	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(GUITextBoxId_Can1Label);
	GUI_AddContainer(&prvContainer);

	/* CAN1 bit rate popout container */
	prvContainer.object.id = GUIContainerId_PopoutCan1BitRate;
	prvContainer.object.xPos = 500;
	prvContainer.object.yPos = 150;
	prvContainer.object.width = 149;
	prvContainer.object.height = 320;
	prvContainer.object.layer = GUILayer_1;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 2;
	prvContainer.object.borderColor = GUI_WHITE;
	prvContainer.contentHideState = GUIHideState_HideAll;
	prvContainer.buttons[0] = GUI_GetButtonFromId(GUIButtonId_Can1BitRate10k);
	prvContainer.buttons[1] = GUI_GetButtonFromId(GUIButtonId_Can1BitRate20k);
	prvContainer.buttons[2] = GUI_GetButtonFromId(GUIButtonId_Can1BitRate50k);
	prvContainer.buttons[3] = GUI_GetButtonFromId(GUIButtonId_Can1BitRate100k);
	prvContainer.buttons[4] = GUI_GetButtonFromId(GUIButtonId_Can1BitRate125k);
	prvContainer.buttons[5] = GUI_GetButtonFromId(GUIButtonId_Can1BitRate250k);
	prvContainer.buttons[6] = GUI_GetButtonFromId(GUIButtonId_Can1BitRate500k);
	prvContainer.buttons[7] = GUI_GetButtonFromId(GUIButtonId_Can1BitRate1M);
	GUI_AddContainer(&prvContainer);
}


/* Interrupt Handlers --------------------------------------------------------*/
