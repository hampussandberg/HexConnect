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

#include "spi_flash.h"

/* Private defines -----------------------------------------------------------*/
#define MAX_MESSAGES_IN_LIST	1024

/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static GUITextBox prvTextBox = {0};
static GUIButton prvButton = {0};
static GUIContainer prvContainer = {0};

static CANDisplayedItem prvMessageList[MAX_MESSAGES_IN_LIST];
static uint32_t prvNextIndexInList = 0;

/* Private function prototypes -----------------------------------------------*/
void prvInsertMessageInList(CANMessage NewMessage);
void prvWriteMessageListToDisplay();

/* Functions -----------------------------------------------------------------*/

void prvInsertMessageInList(CANMessage NewMessage)
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

void prvWriteMessageListToDisplay()
{
	/* Clear the text box */
	GUITextBox_ClearAndResetWritePosition(GUITextBoxId_Can2Main);

	/* Loop through all messages */
	for (uint32_t i = 0; i < MAX_MESSAGES_IN_LIST; i++)
	{
		if (prvMessageList[i].count != 0)
		{
			uint8_t buffer[4] = {
					prvMessageList[i].message.id >> 24,
					prvMessageList[i].message.id >> 16,
					prvMessageList[i].message.id >> 8,
					prvMessageList[i].message.id};
			GUITextBox_WriteString(GUITextBoxId_Can2Main, "0x");
			GUITextBox_WriteBufferWithFormat(GUITextBoxId_Can2Main, buffer, sizeof(prvMessageList[i].message.id), GUITextFormat_HexWithoutSpaces);
			GUITextBox_WriteString(GUITextBoxId_Can2Main, " - ");

			GUITextBox_WriteString(GUITextBoxId_Can2Main, "0x");
			GUITextBox_WriteBufferWithFormat(GUITextBoxId_Can2Main, (uint8_t*)&prvMessageList[i].message.dlc, sizeof(prvMessageList[i].message.dlc), GUITextFormat_HexWithoutSpaces);
			GUITextBox_WriteString(GUITextBoxId_Can2Main, " - ");

			GUITextBox_WriteBufferWithFormat(GUITextBoxId_Can2Main, (uint8_t*)&prvMessageList[i].message.data, prvMessageList[i].message.dlc, GUITextFormat_HexWithSpaces);
			GUITextBox_WriteString(GUITextBoxId_Can2Main, " - ");

			GUITextBox_WriteString(GUITextBoxId_Can2Main, "Count: ");
			GUITextBox_WriteNumber(GUITextBoxId_Can2Main, (int32_t)prvMessageList[i].count);
			GUITextBox_NewLine(GUITextBoxId_Can2Main);
		}
	}
}

void guiCan2WriteNextCanMessageFromFlashToMainTextBox(const uint32_t constStartFlashAddress, uint32_t currentWriteAddress,
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
		/* For some reason the ID will get byte reversed when writing to textbox so reverse it first */
//		uint8_t buffer[4] = {message.id >> 24, message.id >> 16, message.id >> 8, message.id};
//		GUITextBox_WriteString(GUITextBoxId_Can2Main, "0x");
//		GUITextBox_WriteBuffer(GUITextBoxId_Can2Main, buffer, sizeof(message.id));
//		GUITextBox_WriteString(GUITextBoxId_Can2Main, " - ");

		/* Get the DLC */
		pData = (uint8_t*)&message.dlc;
		SPI_FLASH_ReadBufferDMA(pData, pSettings->readAddress, sizeof(message.dlc), 100);
		pSettings->readAddress += sizeof(message.dlc);
//		GUITextBox_WriteString(GUITextBoxId_Can2Main, "0x");
//		GUITextBox_WriteBuffer(GUITextBoxId_Can2Main, (uint8_t*)&message.dlc, sizeof(message.dlc));
//		GUITextBox_WriteString(GUITextBoxId_Can2Main, " - ");

		/* Get the amount of data that is specified in the DLC */
		pData = (uint8_t*)&message.data;
		SPI_FLASH_ReadBufferDMA(pData, pSettings->readAddress, message.dlc, 100);
		pSettings->readAddress += message.dlc;
//		GUITextBox_WriteBuffer(GUITextBoxId_Can2Main, (uint8_t*)&message.data, message.dlc);
//		GUITextBox_NewLine(GUITextBoxId_Can2Main);

		/* Give back the semaphore now that we are done */
		xSemaphoreGive(*pSemaphore);

		/* Insert the message in the message list */
		prvInsertMessageInList(message);

		/* Update the display */
		prvWriteMessageListToDisplay();
	}
}

/* CAN2 GUI Elements ========================================================*/
/**
 * @brief	Manages how data is displayed in the main text box when the source is CAN2
 * @param	None
 * @retval	None
 */
void guiCan2ManageMainTextBox(bool ShouldRefresh)
{
	const uint32_t constStartFlashAddress = FLASH_ADR_CAN2_DATA;

	/* Get the current write address, this is the address where the last data is */
	uint32_t currentWriteAddress = can2GetCurrentWriteAddress();
	/* Get the current settings of the channel */
	CANSettings* settings = can2GetSettings();
	SemaphoreHandle_t* settingsSemaphore = can2GetSettingsSemaphore();


	uint32_t numOfMessagesSaved = settings->numOfMessagesSaved;
	static uint32_t numOfMessagesDisplayed = 0;
	if (numOfMessagesSaved != numOfMessagesDisplayed)
	{
		guiCan2WriteNextCanMessageFromFlashToMainTextBox(constStartFlashAddress, currentWriteAddress, settings, settingsSemaphore);
		numOfMessagesDisplayed++;
	}

//	static uint32_t lastNumOfMessagesSaved = 0;
//	uint32_t numOfMessagesSaved = settings->numOfMessagesSaved;
//	if (numOfMessagesSaved != lastNumOfMessagesSaved)
//	{
//		GUI_WriteNumberInTextBox(GUITextBoxId_Can2Main, (int32_t)numOfMessagesSaved);
//		GUITextBox_WriteString(GUITextBoxId_Can2Main, " ");
//		lastNumOfMessagesSaved = numOfMessagesSaved;
//	}
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
				GUIButton_SetTextForRow(GUIButtonId_Can2Enable, "Disabled", 1);
				GUIButton_SetState(GUIButtonId_Can2Top, GUIButtonState_Disabled);

				GUIButton_SetTextForRow(GUIButtonId_Can2BitRate, "< Bit Rate:", 0);
				GUIButton_SetState(GUIButtonId_Can2BitRate, GUIButtonState_Disabled);
			}
		}
		else
		{
			ErrorStatus status = can2SetConnection(CANConnection_Connected);
			if (status == SUCCESS)
			{
				enabled = true;
				GUIButton_SetTextForRow(GUIButtonId_Can2Enable, "Enabled ", 1);
				GUIButton_SetState(GUIButtonId_Can2Top, GUIButtonState_Enabled);

				GUIButton_SetTextForRow(GUIButtonId_Can2BitRate, "Bit Rate:", 0);
				GUIButton_SetState(GUIButtonId_Can2BitRate, GUIButtonState_DisabledTouch);
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
				GUIButton_SetTextForRow(GUIButtonId_Can2Termination, "None", 1);
			}
		}
		else
		{
			ErrorStatus status = can2SetTermination(CANTermination_Connected);
			if (status == SUCCESS)
			{
				terminated = true;
				GUIButton_SetTextForRow(GUIButtonId_Can2Termination, "120 R", 1);
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
		GUIDisplayState displayState = GUIContainer_GetDisplayState(GUIContainerId_SidebarCan2);
		lcdChangeDisplayStateOfSidebar(GUIContainerId_SidebarCan2);
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
		GUIDisplayState displayState = GUIContainer_GetDisplayState(GUIContainerId_PopoutCan2BitRate);

		if (displayState == GUIDisplayState_Hidden)
		{
			GUI_SetActiveLayer(GUILayer_1);
			GUIButton_SetLayer(GUIButtonId_Can2BitRate, GUILayer_1);
			GUIButton_SetState(GUIButtonId_Can2BitRate, GUIButtonState_Enabled);
			GUIContainer_Draw(GUIContainerId_PopoutCan2BitRate);
		}
		else if (displayState == GUIDisplayState_NotHidden)
		{
			GUIContainer_Hide(GUIContainerId_PopoutCan2BitRate);
			GUI_SetActiveLayer(GUILayer_0);
			GUIButton_SetLayer(GUIButtonId_Can2BitRate, GUILayer_0);
			GUIButton_SetState(GUIButtonId_Can2BitRate, GUIButtonState_Disabled);

			/* Refresh the main text box */
			guiCan2ManageMainTextBox(true);
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
			case GUIButtonId_Can2BitRate10k:
				GUIButton_SetTextForRow(GUIButtonId_Can2BitRate, "10kbit/s", 1);
				newBitRate = CANBitRate_10k;
				break;
			case GUIButtonId_Can2BitRate20k:
				GUIButton_SetTextForRow(GUIButtonId_Can2BitRate, "20kbit/s", 1);
				newBitRate = CANBitRate_20k;
				break;
			case GUIButtonId_Can2BitRate50k:
				GUIButton_SetTextForRow(GUIButtonId_Can2BitRate, "50kbit/s", 1);
				newBitRate = CANBitRate_50k;
				break;
			case GUIButtonId_Can2BitRate100k:
				GUIButton_SetTextForRow(GUIButtonId_Can2BitRate, "100kbit/s", 1);
				newBitRate = CANBitRate_100k;
				break;
			case GUIButtonId_Can2BitRate125k:
				GUIButton_SetTextForRow(GUIButtonId_Can2BitRate, "125kbit/s", 1);
				newBitRate = CANBitRate_125k;
				break;
			case GUIButtonId_Can2BitRate250k:
				GUIButton_SetTextForRow(GUIButtonId_Can2BitRate, "250kbit/s", 1);
				newBitRate = CANBitRate_250k;
				break;
			case GUIButtonId_Can2BitRate500k:
				GUIButton_SetTextForRow(GUIButtonId_Can2BitRate, "500kbit/s", 1);
				newBitRate = CANBitRate_500k;
				break;
			case GUIButtonId_Can2BitRate1M:
				GUIButton_SetTextForRow(GUIButtonId_Can2BitRate, "1Mbit/s", 1);
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
		GUIContainer_Hide(GUIContainerId_PopoutCan2BitRate);
		GUI_SetActiveLayer(GUILayer_0);
		GUIButton_SetLayer(GUIButtonId_Can2BitRate, GUILayer_0);
		GUIButton_SetState(GUIButtonId_Can2BitRate, GUIButtonState_Disabled);

		/* Refresh the main text box */
		guiCan2ManageMainTextBox(true);
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
	/* Update the bit rate text to match what is actually set */
	switch (settings->bitRate)
	{
		case CANBitRate_10k:
			GUIButton_SetTextForRow(GUIButtonId_Can2BitRate, "10kbit/s", 1);
			break;
		case CANBitRate_20k:
			GUIButton_SetTextForRow(GUIButtonId_Can2BitRate, "20kbit/s", 1);
			break;
		case CANBitRate_50k:
			GUIButton_SetTextForRow(GUIButtonId_Can2BitRate, "50kbit/s", 1);
			break;
		case CANBitRate_100k:
			GUIButton_SetTextForRow(GUIButtonId_Can2BitRate, "100kbit/s", 1);
			break;
		case CANBitRate_125k:
			GUIButton_SetTextForRow(GUIButtonId_Can2BitRate, "125kbit/s", 1);
			break;
		case CANBitRate_250k:
			GUIButton_SetTextForRow(GUIButtonId_Can2BitRate, "250kbit/s", 1);
			break;
		case CANBitRate_500k:
			GUIButton_SetTextForRow(GUIButtonId_Can2BitRate, "500kbit/s", 1);
			break;
		case CANBitRate_1M:
			GUIButton_SetTextForRow(GUIButtonId_Can2BitRate, "1Mbit/s", 1);
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
void guiCan2InitGuiElements()
{
	/* Text boxes ----------------------------------------------------------------*/
	/* CAN2 Label text box */
	prvTextBox.object.id = GUITextBoxId_Can2Label;
	prvTextBox.object.xPos = 650;
	prvTextBox.object.yPos = 50;
	prvTextBox.object.width = 150;
	prvTextBox.object.height = 50;
	prvTextBox.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvTextBox.object.borderThickness = 1;
	prvTextBox.object.borderColor = GUI_WHITE;
	prvTextBox.object.containerPage = GUIContainerPage_All;
	prvTextBox.textColor = GUI_RED;
	prvTextBox.backgroundColor = GUI_WHITE;
	prvTextBox.staticText = "CAN2";
	prvTextBox.textSize = LCDFontEnlarge_2x;
	GUITextBox_Add(&prvTextBox);

	/* CAN2 Main text box */
	prvTextBox.object.id = GUITextBoxId_Can2Main;
	prvTextBox.object.xPos = 0;
	prvTextBox.object.yPos = 50;
	prvTextBox.object.width = 650;
	prvTextBox.object.height = 400;
	prvTextBox.object.containerPage = GUIContainerPage_1;
	prvTextBox.textColor = GUI_WHITE;
	prvTextBox.backgroundColor = LCD_COLOR_BLACK;
	prvTextBox.textSize = LCDFontEnlarge_1x;
	GUITextBox_Add(&prvTextBox);


	/* Buttons -------------------------------------------------------------------*/
	/* CAN2 Top Button */
	prvButton.object.id = GUIButtonId_Can2Top;
	prvButton.object.xPos = 100;
	prvButton.object.yPos = 0;
	prvButton.object.width = 100;
	prvButton.object.height = 50;
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
	GUIButton_Add(&prvButton);

	/* CAN2 Enable Button */
	prvButton.object.id = GUIButtonId_Can2Enable;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 100;
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
	prvButton.touchCallback = guiCan2EnableButtonCallback;
	prvButton.text[0] = "Output:";
//	prvButton.text[1] = "Enabled";
	prvButton.text[1] = "Disabled";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUIButton_Add(&prvButton);

	/* CAN2 Bit Rate Button */
	prvButton.object.id = GUIButtonId_Can2BitRate;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 150;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.object.containerPage = GUIContainerPage_1;
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
	GUIButton_Add(&prvButton);

	/* CAN2 Termination Button */
	prvButton.object.id = GUIButtonId_Can2Termination;
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
	prvButton.touchCallback = guiCan2TerminationButtonCallback;
	prvButton.text[0] = "Termination:";
	prvButton.text[1] = "None";
//	prvButton.text[1] = "120 R";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUIButton_Add(&prvButton);


	/* CAN2 10k bit rate Button */
	prvButton.object.id = GUIButtonId_Can2BitRate10k;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 150;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
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
	GUIButton_Add(&prvButton);

	/* CAN2 20k bit rate Button */
	prvButton.object.id = GUIButtonId_Can2BitRate20k;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 190;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
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
	GUIButton_Add(&prvButton);

	/* CAN2 50k bit rate Button */
	prvButton.object.id = GUIButtonId_Can2BitRate50k;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 230;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
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
	GUIButton_Add(&prvButton);

	/* CAN2 100k bit rate Button */
	prvButton.object.id = GUIButtonId_Can2BitRate100k;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 270;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
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
	GUIButton_Add(&prvButton);

	/* CAN2 125k bit rate Button */
	prvButton.object.id = GUIButtonId_Can2BitRate125k;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 310;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
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
	GUIButton_Add(&prvButton);

	/* CAN2 250k bit rate Button */
	prvButton.object.id = GUIButtonId_Can2BitRate250k;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 350;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
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
	GUIButton_Add(&prvButton);

	/* CAN2 500k bit rate Button */
	prvButton.object.id = GUIButtonId_Can2BitRate500k;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 390;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
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
	GUIButton_Add(&prvButton);

	/* CAN2 1M bit rate Button */
	prvButton.object.id = GUIButtonId_Can2BitRate1M;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 430;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
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
	GUIButton_Add(&prvButton);

	/* Containers ----------------------------------------------------------------*/
	/* Sidebar CAN2 container */
	prvContainer.object.id = GUIContainerId_SidebarCan2;
	prvContainer.object.xPos = 650;
	prvContainer.object.yPos = 50;
	prvContainer.object.width = 150;
	prvContainer.object.height = 400;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 1;
	prvContainer.object.borderColor = GUI_WHITE;
	prvContainer.activePage = GUIContainerPage_1;
	prvContainer.contentHideState = GUIHideState_KeepBorders;
	prvContainer.buttons[0] = GUIButton_GetFromId(GUIButtonId_Can2Enable);
	prvContainer.buttons[1] = GUIButton_GetFromId(GUIButtonId_Can2BitRate);
	prvContainer.buttons[2] = GUIButton_GetFromId(GUIButtonId_Can2Termination);
	prvContainer.textBoxes[0] = GUITextBox_GetFromId(GUITextBoxId_Can2Label);
	GUIContainer_Add(&prvContainer);

	/* CAN2 bit rate popout container */
	prvContainer.object.id = GUIContainerId_PopoutCan2BitRate;
	prvContainer.object.xPos = 500;
	prvContainer.object.yPos = 150;
	prvContainer.object.width = 149;
	prvContainer.object.height = 320;
	prvContainer.object.layer = GUILayer_1;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 2;
	prvContainer.object.borderColor = GUI_WHITE;
	prvContainer.contentHideState = GUIHideState_HideAll;
	prvContainer.buttons[0] = GUIButton_GetFromId(GUIButtonId_Can2BitRate10k);
	prvContainer.buttons[1] = GUIButton_GetFromId(GUIButtonId_Can2BitRate20k);
	prvContainer.buttons[2] = GUIButton_GetFromId(GUIButtonId_Can2BitRate50k);
	prvContainer.buttons[3] = GUIButton_GetFromId(GUIButtonId_Can2BitRate100k);
	prvContainer.buttons[4] = GUIButton_GetFromId(GUIButtonId_Can2BitRate125k);
	prvContainer.buttons[5] = GUIButton_GetFromId(GUIButtonId_Can2BitRate250k);
	prvContainer.buttons[6] = GUIButton_GetFromId(GUIButtonId_Can2BitRate500k);
	prvContainer.buttons[7] = GUIButton_GetFromId(GUIButtonId_Can2BitRate1M);
	GUIContainer_Add(&prvContainer);

	/* CAN2 main container */
	prvContainer.object.id = GUIContainerId_Can2MainContent;
	prvContainer.object.xPos = 0;
	prvContainer.object.yPos = 50;
	prvContainer.object.width = 650;
	prvContainer.object.height = 400;
	prvContainer.object.containerPage = guiConfigMAIN_CONTAINER_CAN2_PAGE;
	prvContainer.object.border = GUIBorder_Right | GUIBorder_Top;
	prvContainer.object.borderThickness = 1;
	prvContainer.object.borderColor = GUI_WHITE;
	prvContainer.activePage = GUIContainerPage_1;
	prvContainer.backgroundColor = GUI_BLACK;
	prvContainer.contentHideState = GUIHideState_HideAll;
	prvContainer.textBoxes[0] = GUITextBox_GetFromId(GUITextBoxId_Can2Main);
	GUIContainer_Add(&prvContainer);
}

/* Interrupt Handlers --------------------------------------------------------*/
