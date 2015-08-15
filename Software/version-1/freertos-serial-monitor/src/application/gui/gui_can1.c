/**
 *******************************************************************************
 * @file  gui_can1.c
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
#include "gui_can1.h"

#include "spi_flash.h"

/* Private defines -----------------------------------------------------------*/
#define MAX_MESSAGES_IN_LIST	64

/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static GUITextBox prvTextBox = {0};
static GUIButton prvButton = {0};
static GUIContainer prvContainer = {0};

static CANDisplayedItem prvMessageList[MAX_MESSAGES_IN_LIST];
static uint32_t prvNextIndexInList = 0;

static uint32_t prvNumOfMessagesDisplayed = 0;
static bool prvClearingInProgress = false;

/* Private function prototypes -----------------------------------------------*/
static void prvInsertMessageInList(CANMessage NewMessage);
static void prvWriteMessageListToDisplay();

/* Functions -----------------------------------------------------------------*/
/**
 * @brief	Write the next message in flash to the main text box
 * @param	None
 * @retval	None
 */
void guiCan1WriteNextCanMessageFromFlashToMainTextBox(const uint32_t constStartFlashAddress, uint32_t currentWriteAddress,
											 CANSettings* pSettings, SemaphoreHandle_t* pSemaphore)
{
	CANMessage message = {0};

	/* Try to take the settings semaphore */
	if (*pSemaphore != 0 && xSemaphoreTake(*pSemaphore, 100) == pdTRUE)
	{
		/* Get the ID */
		uint8_t* pData = (uint8_t*)&message.id;
		SPI_FLASH_ReadBufferDMA(pData, pSettings->readAddress, sizeof(message.id), 100);
		pSettings->readAddress += sizeof(message.id);

		/* Get the DLC */
		pData = (uint8_t*)&message.dlc;
		SPI_FLASH_ReadBufferDMA(pData, pSettings->readAddress, sizeof(message.dlc), 100);
		pSettings->readAddress += sizeof(message.dlc);

		/* Get the amount of data that is specified in the DLC */
		pData = (uint8_t*)&message.data;
		SPI_FLASH_ReadBufferDMA(pData, pSettings->readAddress, message.dlc, 100);
		pSettings->readAddress += message.dlc;

		/* Give back the semaphore now that we are done */
		xSemaphoreGive(*pSemaphore);

		/* Insert the message in the message list */
		prvInsertMessageInList(message);

		/* Update the display */
		prvWriteMessageListToDisplay();
	}
}

/**
 * @brief	Manages how data is displayed in the main text box when the source is CAN1
 * @param	None
 * @retval	None
 */
void guiCan1ManageMainTextBox(bool ShouldRefresh)
{
	const uint32_t constStartFlashAddress = FLASH_ADR_CAN1_DATA;

	/* Get the current write address, this is the address where the last data is */
	uint32_t currentWriteAddress = can1GetCurrentWriteAddress();
	/* Get the current settings of the channel */
	CANSettings* settings = can1GetSettings();
	SemaphoreHandle_t* settingsSemaphore = can1GetSettingsSemaphore();

	/* Make sure we don't try to update the display if we are clearing the channel */
	if (!prvClearingInProgress)
	{
		uint32_t numOfMessagesSaved = settings->numOfMessagesSaved;
		static uint32_t numOfMessagesDisplayed = 0;
		if (numOfMessagesSaved != prvNumOfMessagesDisplayed)
		{
			guiCan1WriteNextCanMessageFromFlashToMainTextBox(constStartFlashAddress, currentWriteAddress, settings, settingsSemaphore);
			prvNumOfMessagesDisplayed++;
		}

		/* Update the display */
		if (ShouldRefresh)
		{
			prvWriteMessageListToDisplay();
		}
	}
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
				GUIButton_SetTextForRow(GUIButtonId_Can1Enable, "Disabled", 1);
				GUIButton_SetState(GUIButtonId_Can1Top, GUIButtonState_Disabled);

				GUIButton_SetTextForRow(GUIButtonId_Can1BitRate, "< Bit Rate:", 0);
				GUIButton_SetState(GUIButtonId_Can1BitRate, GUIButtonState_Disabled);
			}
		}
		else
		{
			ErrorStatus status = can1SetConnection(CANConnection_Connected);
			if (status == SUCCESS)
			{
				enabled = true;
				GUIButton_SetTextForRow(GUIButtonId_Can1Enable, "Enabled", 1);
				GUIButton_SetState(GUIButtonId_Can1Top, GUIButtonState_Enabled);

				GUIButton_SetTextForRow(GUIButtonId_Can1BitRate, "Bit Rate:", 0);
				GUIButton_SetState(GUIButtonId_Can1BitRate, GUIButtonState_DisabledTouch);
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
				GUIButton_SetTextForRow(GUIButtonId_Can1Termination, "None", 1);
			}
		}
		else
		{
			ErrorStatus status = can1SetTermination(CANTermination_Connected);
			if (status == SUCCESS)
			{
				terminated = true;
				GUIButton_SetTextForRow(GUIButtonId_Can1Termination, "120 R", 1);
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
		GUIDisplayState displayState = GUIContainer_GetDisplayState(GUIContainerId_SidebarCan1);
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
		GUIDisplayState displayState = GUIContainer_GetDisplayState(GUIContainerId_PopoutCan1BitRate);

		if (displayState == GUIDisplayState_Hidden)
		{
			GUI_SetActiveLayer(GUILayer_1);
			GUIButton_SetLayer(GUIButtonId_Can1BitRate, GUILayer_1);
			GUIButton_SetState(GUIButtonId_Can1BitRate, GUIButtonState_Enabled);
			GUIContainer_Draw(GUIContainerId_PopoutCan1BitRate);
		}
		else if (displayState == GUIDisplayState_NotHidden)
		{
			GUIContainer_Hide(GUIContainerId_PopoutCan1BitRate);
			GUI_SetActiveLayer(GUILayer_0);
			GUIButton_SetLayer(GUIButtonId_Can1BitRate, GUILayer_0);
			GUIButton_SetState(GUIButtonId_Can1BitRate, GUIButtonState_Disabled);

			/* Refresh the main text box */
			guiCan1ManageMainTextBox(true);
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
				GUIButton_SetTextForRow(GUIButtonId_Can1BitRate, "10kbit/s", 1);
				newBitRate = CANBitRate_10k;
				break;
			case GUIButtonId_Can1BitRate20k:
				GUIButton_SetTextForRow(GUIButtonId_Can1BitRate, "20kbit/s", 1);
				newBitRate = CANBitRate_20k;
				break;
			case GUIButtonId_Can1BitRate50k:
				GUIButton_SetTextForRow(GUIButtonId_Can1BitRate, "50kbit/s", 1);
				newBitRate = CANBitRate_50k;
				break;
			case GUIButtonId_Can1BitRate100k:
				GUIButton_SetTextForRow(GUIButtonId_Can1BitRate, "100kbit/s", 1);
				newBitRate = CANBitRate_100k;
				break;
			case GUIButtonId_Can1BitRate125k:
				GUIButton_SetTextForRow(GUIButtonId_Can1BitRate, "125kbit/s", 1);
				newBitRate = CANBitRate_125k;
				break;
			case GUIButtonId_Can1BitRate250k:
				GUIButton_SetTextForRow(GUIButtonId_Can1BitRate, "250kbit/s", 1);
				newBitRate = CANBitRate_250k;
				break;
			case GUIButtonId_Can1BitRate500k:
				GUIButton_SetTextForRow(GUIButtonId_Can1BitRate, "500kbit/s", 1);
				newBitRate = CANBitRate_500k;
				break;
			case GUIButtonId_Can1BitRate1M:
				GUIButton_SetTextForRow(GUIButtonId_Can1BitRate, "1Mbit/s", 1);
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
		GUIContainer_Hide(GUIContainerId_PopoutCan1BitRate);
		GUI_SetActiveLayer(GUILayer_0);
		GUIButton_SetLayer(GUIButtonId_Can1BitRate, GUILayer_0);
		GUIButton_SetState(GUIButtonId_Can1BitRate, GUIButtonState_Disabled);

		/* Refresh the main text box */
		guiCan1ManageMainTextBox(true);
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
	/* Update the bit rate text to match what is actually set */
	switch (settings->bitRate)
	{
		case CANBitRate_10k:
			GUIButton_SetTextForRow(GUIButtonId_Can1BitRate, "10kbit/s", 1);
			break;
		case CANBitRate_20k:
			GUIButton_SetTextForRow(GUIButtonId_Can1BitRate, "20kbit/s", 1);
			break;
		case CANBitRate_50k:
			GUIButton_SetTextForRow(GUIButtonId_Can1BitRate, "50kbit/s", 1);
			break;
		case CANBitRate_100k:
			GUIButton_SetTextForRow(GUIButtonId_Can1BitRate, "100kbit/s", 1);
			break;
		case CANBitRate_125k:
			GUIButton_SetTextForRow(GUIButtonId_Can1BitRate, "125kbit/s", 1);
			break;
		case CANBitRate_250k:
			GUIButton_SetTextForRow(GUIButtonId_Can1BitRate, "250kbit/s", 1);
			break;
		case CANBitRate_500k:
			GUIButton_SetTextForRow(GUIButtonId_Can1BitRate, "500kbit/s", 1);
			break;
		case CANBitRate_1M:
			GUIButton_SetTextForRow(GUIButtonId_Can1BitRate, "1Mbit/s", 1);
			break;
		default:
			break;
	}
}

/**
 * @brief	Callback for the clear button
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiCan1ClearButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		prvClearingInProgress = true;
		can1Clear();
		GUITextBox_ClearAndResetWritePosition(GUITextBoxId_Can1Main);
		for (uint32_t i = 0; i < MAX_MESSAGES_IN_LIST; i++)
		{
			prvMessageList[i].count = 0;
//			memset(&prvMessageList[i], 0, sizeof(CANDisplayedItem));
		}
		prvNumOfMessagesDisplayed = 0;
		prvClearingInProgress = false;
	}
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
	GUITextBox_Add(&prvTextBox);

	/* CAN1 Main text box */
	prvTextBox.object.id = GUITextBoxId_Can1Main;
	prvTextBox.object.xPos = 0;
	prvTextBox.object.yPos = 50;
	prvTextBox.object.width = 650;
	prvTextBox.object.height = 400;
	prvTextBox.object.border = GUIBorder_Top | GUIBorder_Right;
	prvTextBox.object.borderThickness = 1;
	prvTextBox.object.borderColor = GUI_WHITE;
	prvTextBox.object.containerPage = GUIContainerPage_1;
	prvTextBox.textColor = GUI_WHITE;
	prvTextBox.backgroundColor = LCD_COLOR_BLACK;
	prvTextBox.textSize = LCDFontEnlarge_1x;
//	prvTextBox.padding.bottom = guiConfigFONT_HEIGHT_UNIT;
//	prvTextBox.padding.top = guiConfigFONT_HEIGHT_UNIT;
//	prvTextBox.padding.left = guiConfigFONT_WIDTH_UNIT;
//	prvTextBox.padding.right = guiConfigFONT_WIDTH_UNIT;
	GUITextBox_Add(&prvTextBox);

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
	GUIButton_Add(&prvButton);

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
	GUIButton_Add(&prvButton);

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
	GUIButton_Add(&prvButton);

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
	GUIButton_Add(&prvButton);

	/* CAN1 Clear Button */
	prvButton.object.id = GUIButtonId_Can1Clear;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 250;
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
	prvButton.touchCallback = guiCan1ClearButtonCallback;
	prvButton.text[0] = "Clear";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUIButton_Add(&prvButton);


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
	GUIButton_Add(&prvButton);

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
	GUIButton_Add(&prvButton);

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
	GUIButton_Add(&prvButton);

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
	GUIButton_Add(&prvButton);

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
	GUIButton_Add(&prvButton);

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
	GUIButton_Add(&prvButton);

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
	GUIButton_Add(&prvButton);

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
	GUIButton_Add(&prvButton);

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
	prvContainer.buttons[0] = GUIButton_GetFromId(GUIButtonId_Can1Enable);
	prvContainer.buttons[1] = GUIButton_GetFromId(GUIButtonId_Can1BitRate);
	prvContainer.buttons[2] = GUIButton_GetFromId(GUIButtonId_Can1Termination);
	prvContainer.buttons[3] = GUIButton_GetFromId(GUIButtonId_Can1Clear);
	prvContainer.textBoxes[0] = GUITextBox_GetFromId(GUITextBoxId_Can1Label);
	GUIContainer_Add(&prvContainer);

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
	prvContainer.buttons[0] = GUIButton_GetFromId(GUIButtonId_Can1BitRate10k);
	prvContainer.buttons[1] = GUIButton_GetFromId(GUIButtonId_Can1BitRate20k);
	prvContainer.buttons[2] = GUIButton_GetFromId(GUIButtonId_Can1BitRate50k);
	prvContainer.buttons[3] = GUIButton_GetFromId(GUIButtonId_Can1BitRate100k);
	prvContainer.buttons[4] = GUIButton_GetFromId(GUIButtonId_Can1BitRate125k);
	prvContainer.buttons[5] = GUIButton_GetFromId(GUIButtonId_Can1BitRate250k);
	prvContainer.buttons[6] = GUIButton_GetFromId(GUIButtonId_Can1BitRate500k);
	prvContainer.buttons[7] = GUIButton_GetFromId(GUIButtonId_Can1BitRate1M);
	GUIContainer_Add(&prvContainer);
}

/* Private functions .--------------------------------------------------------*/
/**
 * @brief	Insert a new message in the list of displayed messages
 * @param	NewMessage: The new message to insert in the list
 * @retval	None
 */
static void prvInsertMessageInList(CANMessage NewMessage)
{
	bool foundMatch = false;

	/* Loop through all messages */
	for (uint32_t i = 0; i < MAX_MESSAGES_IN_LIST; i++)
	{
		if (prvMessageList[i].message.id == NewMessage.id)
		{
			/* Increase the count */
			prvMessageList[i].count++;
			/* Save the message again so that the newest DLC and data is shown */
			prvMessageList[i].message = NewMessage;
			foundMatch = true;
			break;
		}
	}

	/* If we didn't find any match in the list we should add it */
	if (!foundMatch)
	{
		prvMessageList[prvNextIndexInList].message = NewMessage;
		prvMessageList[prvNextIndexInList].count = 1;
		prvNextIndexInList++;
	}
}

/**
 * @brief	Display all the messages in the list on the display
 * @param	None
 * @retval	None
 */
static void prvWriteMessageListToDisplay()
{
	/* Clear the text box */
	GUITextBox_ClearAndResetWritePosition(GUITextBoxId_Can1Main);

	/* Loop through all messages */
	for (uint32_t i = 0; i < MAX_MESSAGES_IN_LIST; i++)
	{
		if (prvMessageList[i].count != 0)
		{
			/* ID */
			uint8_t buffer[4] = {
					prvMessageList[i].message.id >> 24,
					prvMessageList[i].message.id >> 16,
					prvMessageList[i].message.id >> 8,
					prvMessageList[i].message.id};
			GUITextBox_WriteString(GUITextBoxId_Can1Main, "0x");
			GUITextBox_WriteBufferWithFormat(GUITextBoxId_Can1Main, buffer, sizeof(prvMessageList[i].message.id), GUITextFormat_HexWithoutSpaces);
			GUITextBox_WriteString(GUITextBoxId_Can1Main, " - ");

			/* DLC */
			GUITextBox_WriteString(GUITextBoxId_Can1Main, "0x");
			GUITextBox_WriteBufferWithFormat(GUITextBoxId_Can1Main, (uint8_t*)&prvMessageList[i].message.dlc, sizeof(prvMessageList[i].message.dlc), GUITextFormat_HexWithoutSpaces);
			GUITextBox_WriteString(GUITextBoxId_Can1Main, " - ");

			/* Data */
			GUITextBox_WriteBufferWithFormat(GUITextBoxId_Can1Main, (uint8_t*)&prvMessageList[i].message.data, prvMessageList[i].message.dlc, GUITextFormat_HexWithSpaces);
			GUITextBox_WriteString(GUITextBoxId_Can1Main, " - ");

			/* Count */
			GUITextBox_WriteString(GUITextBoxId_Can1Main, "Count: ");
			GUITextBox_WriteNumber(GUITextBoxId_Can1Main, (int32_t)prvMessageList[i].count);
			GUITextBox_NewLine(GUITextBoxId_Can1Main);
		}
	}
}


/* Interrupt Handlers --------------------------------------------------------*/
