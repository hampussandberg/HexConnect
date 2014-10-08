/**
 ******************************************************************************
 * @file	gui_system.c
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
#include "gui_system.h"

/* Private defines -----------------------------------------------------------*/
/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static GUITextBox prvTextBox = {0};
static GUIButton prvButton = {0};
static GUIContainer prvContainer = {0};

/* Private function prototypes -----------------------------------------------*/
/* Functions -----------------------------------------------------------------*/
/* System GUI Elements =======================================================*/
/**
 * @brief
 * @param	None
 * @retval	None
 */
void guiSystemInitGuiElements()
{
	/* Text boxes ----------------------------------------------------------------*/
	/* System Label text box */
	prvTextBox.object.id = GUITextBoxId_SystemLabel;
	prvTextBox.object.xPos = 650;
	prvTextBox.object.yPos = 50;
	prvTextBox.object.width = 150;
	prvTextBox.object.height = 50;
	prvTextBox.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvTextBox.object.borderThickness = 1;
	prvTextBox.object.borderColor = GUI_WHITE;
	prvTextBox.object.containerPage = GUIContainerPage_All;
	prvTextBox.textColor = GUI_SYSTEM_BLUE;
	prvTextBox.backgroundColor = GUI_WHITE;
	prvTextBox.staticText = "System";
	prvTextBox.textSize = LCDFontEnlarge_2x;
	GUI_AddTextBox(&prvTextBox);

	/* Debug Text Box */
	prvTextBox.object.id = GUITextBoxId_Debug;
	prvTextBox.object.xPos = 0;
	prvTextBox.object.yPos = 450;
	prvTextBox.object.width = 649;
	prvTextBox.object.height = 30;
	prvTextBox.object.border = GUIBorder_Top;
	prvTextBox.object.borderThickness = 1;
	prvTextBox.object.borderColor = GUI_WHITE;
	prvTextBox.textColor = GUI_WHITE;
	prvTextBox.backgroundColor = GUI_RED;
	prvTextBox.textSize = LCDFontEnlarge_1x;
	prvTextBox.xWritePos = 0;
	prvTextBox.yWritePos = 0;
	GUI_AddTextBox(&prvTextBox);

	/* Buttons -------------------------------------------------------------------*/
	/* Storage Button */
	prvButton.object.id = GUIButtonId_Storage;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 100;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.object.containerPage = GUIContainerPage_1;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_SYSTEM_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_SYSTEM_BLUE;
	prvButton.pressedTextColor = GUI_SYSTEM_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = 0;
	prvButton.text[0] = "Storage";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* Settings Button */
	prvButton.object.id = GUIButtonId_Settings;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 150;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.object.containerPage = GUIContainerPage_1;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_SYSTEM_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_SYSTEM_BLUE;
	prvButton.pressedTextColor = GUI_SYSTEM_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = 0;
	prvButton.text[0] = "Settings";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* Debug Button */
	prvButton.object.id = GUIButtonId_Debug;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 200;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.object.containerPage = GUIContainerPage_1;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_RED;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_RED;
	prvButton.pressedTextColor = GUI_RED;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiDebugToggleCallback;
	prvButton.text[0] = "Debug";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* Save settings Button */
	prvButton.object.id = GUIButtonId_SaveSettings;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 250;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.object.containerPage = GUIContainerPage_1;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_SYSTEM_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_SYSTEM_BLUE;
	prvButton.pressedTextColor = GUI_SYSTEM_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiSaveSettingsButtonCallback;
	prvButton.text[0] = "Save Channel";
	prvButton.text[1] = "Settings";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* Beep Button */
	prvButton.object.id = GUIButtonId_Beep;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 300;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.object.containerPage = GUIContainerPage_1;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_SYSTEM_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_SYSTEM_BLUE;
	prvButton.pressedTextColor = GUI_SYSTEM_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiBeepButtonCallback;
	prvButton.text[0] = "Button Beep:";
	prvButton.text[1] = "Off";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* System Button */
	prvButton.object.id = GUIButtonId_System;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 450;
	prvButton.object.width = 150;
	prvButton.object.height = 30;
	prvButton.object.layer = GUILayer_0;
	prvButton.object.displayState = GUIDisplayState_NotHidden;
	prvButton.object.border = GUIBorder_Top | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_SYSTEM_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_SYSTEM_BLUE;
	prvButton.pressedTextColor = GUI_SYSTEM_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiSystemButtonCallback;
	prvButton.text[0] = "System";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* Containers ----------------------------------------------------------------*/
	/* Debug container */
	prvContainer.object.id = GUIContainerId_Debug;
	prvContainer.object.xPos = 0;
	prvContainer.object.yPos = 450;
	prvContainer.object.width = 650;
	prvContainer.object.height = 30;
	prvContainer.object.border = GUIBorder_Right;
	prvContainer.object.borderThickness = 1;
	prvContainer.object.borderColor = GUI_WHITE;
	prvContainer.contentHideState = GUIHideState_KeepBorders;
	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(GUITextBoxId_Debug);
	GUI_AddContainer(&prvContainer);
	GUI_DrawContainer(GUIContainerId_Debug);
	GUI_HideContentInContainer(GUIContainerId_Debug);

	/* Side system container */
	prvContainer.object.id = GUIContainerId_SidebarSystem;
	prvContainer.object.xPos = 650;
	prvContainer.object.yPos = 50;
	prvContainer.object.width = 150;
	prvContainer.object.height = 400;
	prvContainer.object.layer = GUILayer_0;
	prvContainer.object.displayState = GUIDisplayState_Hidden;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 1;
	prvContainer.object.borderColor = GUI_WHITE;
	prvContainer.activePage = GUIContainerPage_1;
	prvContainer.contentHideState = GUIHideState_KeepBorders;
	prvContainer.buttons[0] = GUI_GetButtonFromId(GUIButtonId_Settings);
	prvContainer.buttons[1] = GUI_GetButtonFromId(GUIButtonId_Storage);
	prvContainer.buttons[2] = GUI_GetButtonFromId(GUIButtonId_Debug);
	prvContainer.buttons[3] = GUI_GetButtonFromId(GUIButtonId_SaveSettings);
	prvContainer.buttons[4] = GUI_GetButtonFromId(GUIButtonId_Beep);
	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(GUITextBoxId_SystemLabel);
	GUI_AddContainer(&prvContainer);

	/* Side empty container */
	prvContainer.object.id = GUIContainerId_SidebarEmpty;
	prvContainer.object.xPos = 650;
	prvContainer.object.yPos = 50;
	prvContainer.object.width = 150;
	prvContainer.object.height = 400;
	prvContainer.object.layer = GUILayer_0;
	prvContainer.object.displayState = GUIDisplayState_Hidden;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 1;
	prvContainer.object.borderColor = GUI_WHITE;
	prvContainer.activePage = GUIContainerPage_1;
	prvContainer.contentHideState = GUIHideState_KeepBorders;
	GUI_AddContainer(&prvContainer);
}

/**
 * @brief	Callback for the debug button, will toggle the debug text box on and off
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiDebugToggleCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(GUIContainerId_Debug);
		if (displayState == GUIDisplayState_Hidden || displayState == GUIDisplayState_ContentHidden)
		{
			GUI_DrawContainer(GUIContainerId_Debug);
		}
		else
		{
			GUI_HideContentInContainer(GUIContainerId_Debug);
		}
	}
}

/**
 * @brief	Callback for the system button, will toggle the side system sidebar on and off
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiSystemButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(GUIContainerId_SidebarSystem);
		lcdChangeDisplayStateOfSidebar(GUIContainerId_SidebarSystem);
	}
}

/**
 * @brief	Callback for the save settings button, will save the settings of the channels to SPI FLASH
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiSaveSettingsButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		volatile uint32_t currentFlashAddress = FLASH_ADR_CHANNEL_SETTINGS;

		/* Start by erasing all the old settings */
		SPI_FLASH_EraseSector(currentFlashAddress);

		/* CAN */
		SPI_FLASH_WriteBuffer((uint8_t*)can1GetSettings(), currentFlashAddress, sizeof(CANSettings));
		currentFlashAddress += sizeof(CANSettings);
		SPI_FLASH_WriteBuffer((uint8_t*)can2GetSettings(), currentFlashAddress, sizeof(CANSettings));
		currentFlashAddress += sizeof(CANSettings);

		/* UART */
		SPI_FLASH_WriteBuffer((uint8_t*)uart1GetSettings(), currentFlashAddress, sizeof(UARTSettings));
		currentFlashAddress += sizeof(UARTSettings);
		SPI_FLASH_WriteBuffer((uint8_t*)uart2GetSettings(), currentFlashAddress, sizeof(UARTSettings));
		currentFlashAddress += sizeof(UARTSettings);
		SPI_FLASH_WriteBuffer((uint8_t*)rs232GetSettings(), currentFlashAddress, sizeof(UARTSettings));
		currentFlashAddress += sizeof(UARTSettings);
	}
}

/**
 * @brief	Callback for the beep button. Turns the beep on or off
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiBeepButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		if (GUI_BeepIsOn())
		{
			GUI_SetBeepOff();
			GUI_SetButtonTextForRow(GUIButtonId_Beep, "Off", 1);
		}
		else
		{
			GUI_SetBeepOn();
			GUI_SetButtonTextForRow(GUIButtonId_Beep, "On", 1);
		}
	}
}

/* Interrupt Handlers --------------------------------------------------------*/
