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
static void prvUpdateScreenBrightnessGuiValue();

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
	GUITextBox_Add(&prvTextBox);

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
	GUITextBox_Add(&prvTextBox);

	/* Screen brightness value text box */
	prvTextBox.object.id = GUITextBoxId_ScreenBrightnessValue;
	prvTextBox.object.xPos = 500;
	prvTextBox.object.yPos = 175;
	prvTextBox.object.width = 80;
	prvTextBox.object.height = 50;
	prvTextBox.object.layer = GUILayer_1;
	prvTextBox.textColor = GUI_WHITE;
	prvTextBox.backgroundColor = GUI_SYSTEM_BLUE;
	prvTextBox.textSize = LCDFontEnlarge_2x;
	GUITextBox_Add(&prvTextBox);

	/* Buttons -------------------------------------------------------------------*/
	/* Beep Button */
	prvButton.object.id = GUIButtonId_Beep;
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
	prvButton.touchCallback = guiBeepButtonCallback;
	prvButton.text[0] = "Button Beep:";
	prvButton.text[1] = "On";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUIButton_Add(&prvButton);

	/* Screen brightness Button */
	prvButton.object.id = GUIButtonId_ScreenBrightness;
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
	prvButton.touchCallback = guiScreenBrightnessButtonCallback;
	prvButton.text[0] = "< Screen Brightn.";
	prvButton.text[1] = "100%";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUIButton_Add(&prvButton);

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
	GUIButton_Add(&prvButton);

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
	GUIButton_Add(&prvButton);

//	/* Storage Button */
//	prvButton.object.id = GUIButtonId_Storage;
//	prvButton.object.xPos = 650;
//	prvButton.object.yPos = 300;
//	prvButton.object.width = 150;
//	prvButton.object.height = 50;
//	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
//	prvButton.object.borderThickness = 1;
//	prvButton.object.borderColor = GUI_WHITE;
//	prvButton.object.containerPage = GUIContainerPage_1;
//	prvButton.enabledTextColor = GUI_WHITE;
//	prvButton.enabledBackgroundColor = GUI_SYSTEM_BLUE;
//	prvButton.disabledTextColor = GUI_WHITE;
//	prvButton.disabledBackgroundColor = GUI_SYSTEM_BLUE;
//	prvButton.pressedTextColor = GUI_SYSTEM_BLUE;
//	prvButton.pressedBackgroundColor = GUI_WHITE;
//	prvButton.state = GUIButtonState_Disabled;
//	prvButton.touchCallback = 0;
//	prvButton.text[0] = "Storage";
//	prvButton.textSize[0] = LCDFontEnlarge_1x;
//	GUIButton_Add(&prvButton);

//	/* Settings Button */
//	prvButton.object.id = GUIButtonId_Settings;
//	prvButton.object.xPos = 650;
//	prvButton.object.yPos = 350;
//	prvButton.object.width = 150;
//	prvButton.object.height = 50;
//	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
//	prvButton.object.borderThickness = 1;
//	prvButton.object.borderColor = GUI_WHITE;
//	prvButton.object.containerPage = GUIContainerPage_1;
//	prvButton.enabledTextColor = GUI_WHITE;
//	prvButton.enabledBackgroundColor = GUI_SYSTEM_BLUE;
//	prvButton.disabledTextColor = GUI_WHITE;
//	prvButton.disabledBackgroundColor = GUI_SYSTEM_BLUE;
//	prvButton.pressedTextColor = GUI_SYSTEM_BLUE;
//	prvButton.pressedBackgroundColor = GUI_WHITE;
//	prvButton.state = GUIButtonState_Disabled;
//	prvButton.touchCallback = 0;
//	prvButton.text[0] = "Settings";
//	prvButton.textSize[0] = LCDFontEnlarge_1x;
//	GUIButton_Add(&prvButton);

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
	GUIButton_Add(&prvButton);

	/* Screen Brightness Down Button */
	prvButton.object.id = GUIButtonId_ScreenBrightnessDown;
	prvButton.object.xPos = 425;
	prvButton.object.yPos = 175;
	prvButton.object.width = 50;
	prvButton.object.height = 50;
	prvButton.object.layer = GUILayer_1;
	prvButton.enabledTextColor = GUI_SYSTEM_BLUE;
	prvButton.enabledBackgroundColor = GUI_SYSTEM_BLUE;
	prvButton.disabledTextColor = GUI_SYSTEM_BLUE;
	prvButton.disabledBackgroundColor = GUI_WHITE;
	prvButton.pressedTextColor = GUI_SYSTEM_BLUE;
	prvButton.pressedBackgroundColor = GUI_SYSTEM_BLUE_DARK;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiScreenBrightnessCallback;
	prvButton.text[0] = "-";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUIButton_Add(&prvButton);

	/* Screen Brightness Up Button */
	prvButton.object.id = GUIButtonId_ScreenBrightnessUp;
	prvButton.object.xPos = 575;
	prvButton.object.yPos = 175;
	prvButton.object.width = 50;
	prvButton.object.height = 50;
	prvButton.object.layer = GUILayer_1;
	prvButton.enabledTextColor = GUI_SYSTEM_BLUE;
	prvButton.enabledBackgroundColor = GUI_SYSTEM_BLUE;
	prvButton.disabledTextColor = GUI_SYSTEM_BLUE;
	prvButton.disabledBackgroundColor = GUI_WHITE;
	prvButton.pressedTextColor = GUI_SYSTEM_BLUE;
	prvButton.pressedBackgroundColor = GUI_SYSTEM_BLUE_DARK;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiScreenBrightnessCallback;
	prvButton.text[0] = "+";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUIButton_Add(&prvButton);

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
	prvContainer.textBoxes[0] = GUITextBox_GetFromId(GUITextBoxId_Debug);
	GUIContainer_Add(&prvContainer);
//	GUIContainer_Draw(GUIContainerId_Debug);
//	GUIContainer_HideContent(GUIContainerId_Debug);

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
	prvContainer.buttons[0] = GUIButton_GetFromId(GUIButtonId_Beep);
	prvContainer.buttons[1] = GUIButton_GetFromId(GUIButtonId_ScreenBrightness);
	prvContainer.buttons[2] = GUIButton_GetFromId(GUIButtonId_Debug);
	prvContainer.buttons[3] = GUIButton_GetFromId(GUIButtonId_SaveSettings);
//	prvContainer.buttons[4] = GUIButton_GetFromId(GUIButtonId_Settings);
//	prvContainer.buttons[5] = GUIButton_GetFromId(GUIButtonId_Storage);
	prvContainer.textBoxes[0] = GUITextBox_GetFromId(GUITextBoxId_SystemLabel);
	GUIContainer_Add(&prvContainer);

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
	GUIContainer_Add(&prvContainer);

	/* Screen brightness popout container */
	prvContainer.object.id = GUIContainerId_PopoutScreenBrightness;
	prvContainer.object.xPos = 400;
	prvContainer.object.yPos = 150;
	prvContainer.object.width = 249;
	prvContainer.object.height = 100;
	prvContainer.object.layer = GUILayer_1;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 2;
	prvContainer.object.borderColor = GUI_WHITE;
	prvContainer.backgroundColor = GUI_SYSTEM_BLUE;
	prvContainer.contentHideState = GUIHideState_HideAll;
	prvContainer.buttons[0] = GUIButton_GetFromId(GUIButtonId_ScreenBrightnessUp);
	prvContainer.buttons[1] = GUIButton_GetFromId(GUIButtonId_ScreenBrightnessDown);
	prvContainer.textBoxes[0] = GUITextBox_GetFromId(GUITextBoxId_ScreenBrightnessValue);
	GUIContainer_Add(&prvContainer);
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
		GUIDisplayState displayState = GUIContainer_GetDisplayState(GUIContainerId_Debug);
		if (displayState == GUIDisplayState_Hidden || displayState == GUIDisplayState_ContentHidden)
		{
			GUIContainer_Draw(GUIContainerId_Debug);
		}
		else
		{
			GUIContainer_HideContent(GUIContainerId_Debug);
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
		GUIDisplayState displayState = GUIContainer_GetDisplayState(GUIContainerId_SidebarSystem);
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
			GUIButton_SetTextForRow(GUIButtonId_Beep, "Off", 1);
		}
		else
		{
			GUI_SetBeepOn();
			GUIButton_SetTextForRow(GUIButtonId_Beep, "On", 1);
		}
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiScreenBrightnessButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		GUIDisplayState displayState = GUIContainer_GetDisplayState(GUIContainerId_PopoutScreenBrightness);

		if (displayState == GUIDisplayState_Hidden)
		{
			GUI_SetActiveLayer(GUILayer_1);
			GUIButton_SetLayer(GUIButtonId_ScreenBrightness, GUILayer_1);
			GUIButton_SetState(GUIButtonId_ScreenBrightness, GUIButtonState_Enabled);
			GUIContainer_Draw(GUIContainerId_PopoutScreenBrightness);
			prvUpdateScreenBrightnessGuiValue();
		}
		else if (displayState == GUIDisplayState_NotHidden)
		{
			GUIContainer_Hide(GUIContainerId_PopoutScreenBrightness);
			GUI_SetActiveLayer(GUILayer_0);
			GUIButton_SetLayer(GUIButtonId_ScreenBrightness, GUILayer_0);
			GUIButton_SetState(GUIButtonId_ScreenBrightness, GUIButtonState_Disabled);

			/* Refresh the main text box */
			lcdForceRefreshOfActiveMainContent();
		}
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiScreenBrightnessCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	static TickType_t lastBrightnessUpdate = 0;

	if ((Event == GUITouchEvent_Up || Event == GUITouchEvent_Down) && xTaskGetTickCount() - lastBrightnessUpdate > 25)
	{
		lastBrightnessUpdate = xTaskGetTickCount();

		uint8_t currentBrightness = LCD_GetBrightness();	/* 0 to 255 */
		/* Do different things depending on which button was pressed */
		switch (ButtonId)
		{
			/* Up */
			case GUIButtonId_ScreenBrightnessUp:
				if (currentBrightness != 255)
					currentBrightness += 1;
				break;

			/* Down */
			case GUIButtonId_ScreenBrightnessDown:
				if (currentBrightness != 0)
					currentBrightness -= 1;
				break;

			default:
				break;
		}

		/* Set the updated brightness */
		LCD_SetBrightness(currentBrightness);

		prvUpdateScreenBrightnessGuiValue();
	}
}

/* Private functions .--------------------------------------------------------*/
/**
 * @brief
 * @param	None
 * @retval	None
 */
static void prvUpdateScreenBrightnessGuiValue()
{
	/* Get the current value */
	uint8_t currentBrightness = LCD_GetBrightness();	/* 0 to 255 */
	float brightnessAsPercentage = currentBrightness / 2.55;	/* 0 to 100.0 */

	/* Update GUI */
	GUITextBox_ClearAndResetWritePosition(GUITextBoxId_ScreenBrightnessValue);
	GUITextBox_SetYWritePositionToCenter(GUITextBoxId_ScreenBrightnessValue);
	GUITextBox_WriteNumber(GUITextBoxId_ScreenBrightnessValue, (int32_t)brightnessAsPercentage);
	GUITextBox_WriteString(GUITextBoxId_ScreenBrightnessValue, "%");
}

/* Interrupt Handlers --------------------------------------------------------*/
