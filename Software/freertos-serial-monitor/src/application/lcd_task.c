/**
 ******************************************************************************
 * @file	lcd_task.c
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-09-07
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
#include "lcd_task.h"

#include "simple_gui.h"
#include "simple_gui_config.h"
#include "ft5206.h"
#include "spi_flash.h"
#include "mcp9808.h"
#include "spi_flash_memory_map.h"

#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "can1_task.h"
#include "can2_task.h"
#include "uart1_task.h"
#include "uart2_task.h"
#include "rs232_task.h"
#include "gpio0_task.h"
#include "gpio1_task.h"

#include "gui_can1.h"
#include "gui_can2.h"
#include "gui_uart1.h"
#include "gui_uart2.h"
#include "gui_rs232.h"
#include "gui_gpio.h"
#include "gui_adc.h"

/* Private defines -----------------------------------------------------------*/
#define FLASH_FETCH_BUFFER_SIZE		(64)

/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static GUITextBox prvTextBox = {0};
static GUIButton prvButton = {0};
static GUIContainer prvContainer = {0};
static uint32_t prvIdOfLastActiveSidebar = guiConfigINVALID_ID;
static uint32_t prvIdOfActiveSidebar = guiConfigSIDEBAR_EMPTY_CONTAINER_ID;
static bool prvDebugConsoleIsHidden = false;
float prvTemperature = 0.0;

static uint8_t prvFlashFetchBuffer[FLASH_FETCH_BUFFER_SIZE];
static int32_t prvMainTextBoxYPosOffset = 0;
static bool prvActiveChannelHasChanged = false;
static bool prcActiveMainTextBoxManagerShouldRefresh = false;

static xTimerHandle prvMainTextBoxRefreshTimer;

/* Private function prototypes -----------------------------------------------*/
static void prvDisplayDataInMainTextBox(uint32_t* pFromAddress, uint32_t ToAddress, GUIWriteFormat Format);
static void prvMainTextBoxRefreshTimerCallback();
static void prvManageEmptyMainTextBox();

static void prvHardwareInit();
static void prvMainTextBoxCallback(GUITouchEvent Event, uint16_t XPos, uint16_t YPos);
static void prvClearMainTextBox();
static bool prvAllChanneAreDoneInitializing();
static void prvInitGuiElements();

static void prvDebugToggleCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvSystemButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvSaveSettingsButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvSystemInitGuiElements();

/* Functions -----------------------------------------------------------------*/
/**
 * @brief	The main task for the LCD
 * @param	pvParameters:
 * @retval	None
 */
void lcdTask(void *pvParameters)
{
	/* Initialize the hardware */
	prvHardwareInit();

	/* Make sure all channels have started up before we initialize the GUI */
	while (!prvAllChanneAreDoneInitializing())
	{
		/* TODO: Display splash screen */
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}

	/* Initialize the GUI elements */
	prvInitGuiElements();

	xLCDEventQueue = xQueueCreate(10, sizeof(LCDEventMessage));
	if (xLCDEventQueue == 0)
	{
		// Queue was not created and must not be used.
	}

	prvMainTextBoxRefreshTimer = xTimerCreate("MainTextBoxTimer", 10 / portTICK_PERIOD_MS, 0, 0, prvMainTextBoxRefreshTimerCallback);
	if (prvMainTextBoxRefreshTimer != NULL)
		xTimerStart(prvMainTextBoxRefreshTimer, portMAX_DELAY);

	LCDEventMessage receivedMessage;

	GUI_WriteStringInTextBox(guiConfigCLOCK_TEXT_BOX_ID, "14:15:12");

	uint8_t text[2] = "A";

	/* The parameter in vTaskDelayUntil is the absolute time
	 * in ticks at which you want to be woken calculated as
	 * an increment from the time you were last woken. */
	TickType_t xNextWakeTime;
	/* Initialize xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();
	while (1)
	{
		/* Wait for a message to be received or the timeout to happen */
		if (xQueueReceive(xLCDEventQueue, &receivedMessage, 50) == pdTRUE)
		{
			/* Item sucessfully removed from the queue */
			switch (receivedMessage.event)
			{
				/* New touch data received */
				case LCDEvent_TouchEvent:
					if (receivedMessage.data[3] == FT5206Point_1)
					{
#if 0
						/* DEBUG */
						if (GUI_GetDisplayStateForTextBox(guiConfigDEBUG_TEXT_BOX_ID) == GUIDisplayState_NotHidden)
						{
							GUI_SetWritePosition(guiConfigDEBUG_TEXT_BOX_ID, 5, 5);
							GUI_ClearTextBox(guiConfigDEBUG_TEXT_BOX_ID);
							GUI_WriteStringInTextBox(guiConfigDEBUG_TEXT_BOX_ID, "X:");
							GUI_WriteNumberInTextBox(guiConfigDEBUG_TEXT_BOX_ID, receivedMessage.data[0]);
							GUI_WriteStringInTextBox(guiConfigDEBUG_TEXT_BOX_ID, ", Y:");
							GUI_WriteNumberInTextBox(guiConfigDEBUG_TEXT_BOX_ID, receivedMessage.data[1]);
							GUI_WriteStringInTextBox(guiConfigDEBUG_TEXT_BOX_ID, ", EVENT:");
							GUI_WriteNumberInTextBox(guiConfigDEBUG_TEXT_BOX_ID, receivedMessage.data[2]);
						}
#endif

#if 0
						/* Draw a dot on debug */
						if (!prvDebugConsoleIsHidden)
						{
							LCD_SetForegroundColor(LCD_COLOR_GREEN);
							LCD_DrawCircle(receivedMessage.data[0], receivedMessage.data[1], 2, 1);
						}
#endif

						GUITouchEvent touchEvent;
						if (receivedMessage.data[2] == FT5206Event_PutUp)
							touchEvent = GUITouchEvent_Up;
						else if (receivedMessage.data[2] == FT5206Event_PutDown)
							touchEvent = GUITouchEvent_Down;
						/* Check all buttons */
						GUI_CheckAllActiveButtonsForTouchEventAt(touchEvent, receivedMessage.data[0], receivedMessage.data[1]);
						/* Check all text boxes */
						GUI_CheckAllActiveTextBoxesForTouchEventAt(touchEvent, receivedMessage.data[0], receivedMessage.data[1]);
					}
					break;

				/* New temperature data received */
				case LCDEvent_TemperatureData:
					memcpy(&prvTemperature, receivedMessage.data, sizeof(float));
					int8_t currentTemp = (int8_t)prvTemperature;
					GUI_DrawTextBox(guiConfigTEMP_TEXT_BOX_ID);
					GUI_SetWritePosition(guiConfigTEMP_TEXT_BOX_ID, 50, 3);
					GUI_WriteNumberInTextBox(guiConfigTEMP_TEXT_BOX_ID, (int32_t)currentTemp);
					GUI_WriteStringInTextBox(guiConfigTEMP_TEXT_BOX_ID, " C");
					break;

				case LCDEvent_MainBoxText:
					text[0] = (uint8_t)receivedMessage.data[0];
					GUI_WriteStringInTextBox(guiConfigMAIN_TEXT_BOX_ID, text);
					break;

				case LCDEvent_DebugMessage:
					GUI_SetWritePosition(guiConfigDEBUG_TEXT_BOX_ID, 5, 5);
					GUI_ClearTextBox(guiConfigDEBUG_TEXT_BOX_ID);
					GUI_WriteNumberInTextBox(guiConfigDEBUG_TEXT_BOX_ID, receivedMessage.data[0]);
					GUI_WriteStringInTextBox(guiConfigDEBUG_TEXT_BOX_ID, " - ");
					GUI_WriteStringInTextBox(guiConfigDEBUG_TEXT_BOX_ID, (uint8_t*)receivedMessage.data[1]);
					break;


				default:
					break;
			}
		}
		else
		{
			/* Timeout has occured i.e. no message available */
	//		vTaskDelayUntil(&xNextWakeTime, 100 / portTICK_PERIOD_MS);
//			HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_2);
			/* Do something else */
		}
	}
}

/**
 * @brief	Callback a generic UART clear button
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void lcdGenericUartClearButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		bool channelWasReset = false;
		switch (prvIdOfActiveSidebar)
		{
			case guiConfigSIDEBAR_UART1_CONTAINER_ID:
				uart1Clear();
				channelWasReset = true;
				break;
			case guiConfigSIDEBAR_UART2_CONTAINER_ID:
				uart2Clear();
				channelWasReset = true;
				break;
			case guiConfigSIDEBAR_RS232_CONTAINER_ID:
				rs232Clear();
				channelWasReset = true;
				break;
			default:
				break;
		}

		/* If a channel was reset we should clear the main text box and save the settings so that the correct addresses are saved */
		if (channelWasReset)
		{
			prvClearMainTextBox();
			prvSaveSettingsButtonCallback(GUITouchEvent_Up, ButtonId);
		}
	}
}


/**
 * @brief	Tells the active main text box that it should refresh
 * @param	None
 * @retval	None
 */
void lcdActiveMainTextBoxManagerShouldRefresh()
{
	prcActiveMainTextBoxManagerShouldRefresh = true;
}

/**
 * @brief	Manages how data is displayed in the main text box when the source is an UART channel
 * @param	constStartFlashAddress: The address in SPI FLASH where the first data is for the channel
 * @param	currentWriteAddress: The current write address for the channel
 * @param	pSettings: Pointer to the settings for the channel
 * @param	pSemaphore: Pointer to the settings semaphore
 * @retval	None
 */
void lcdManageGenericUartMainTextBox(const uint32_t constStartFlashAddress, uint32_t currentWriteAddress,
									 UARTSettings* pSettings, SemaphoreHandle_t* pSemaphore)
{
	/* Try to take the settings semaphore */
	if (*pSemaphore != 0 && xSemaphoreTake(*pSemaphore, 100) == pdTRUE)
	{
		/* Get how many rows the offset equals */
		int32_t rowDiff = prvMainTextBoxYPosOffset / 16;

		/* If we should refresh we set the current read address to the display start address */
		if (prcActiveMainTextBoxManagerShouldRefresh)
		{
			prvClearMainTextBox();
			prcActiveMainTextBoxManagerShouldRefresh = false;

			/*
			 * If we are not scrolling it means we should have the newest data on the bottom of the page
			 * therefore we set the end address to where the newest data is and the start address
			 * numOfCharactersDisplayed before that. This should avoid the problem of loading all new data
			 * if the page has not been displayed for a while.
			 */
			if (!pSettings->scrolling)
			{
				uint32_t numOfCharactersToDisplay = pSettings->amountOfDataSaved;
				if (numOfCharactersToDisplay > GUI_MAIN_MAX_NUM_OF_CHARACTERS)
					numOfCharactersToDisplay = GUI_MAIN_MAX_NUM_OF_CHARACTERS;
				pSettings->displayedDataEndAddress = currentWriteAddress;
				pSettings->displayedDataStartAddress = pSettings->displayedDataEndAddress -
													   numOfCharactersToDisplay*pSettings->numOfCharactersPerByte;
			}

			pSettings->readAddress = pSettings->displayedDataStartAddress;
			while (pSettings->readAddress != pSettings->displayedDataEndAddress)
			{
				prvDisplayDataInMainTextBox(&pSettings->readAddress, pSettings->displayedDataEndAddress, pSettings->writeFormat);
			}
		}

		/* Manage offset caused by scrolling */
		if (prvMainTextBoxYPosOffset != 0 && rowDiff != 0)
		{
			/* Say that we are scrolling */
			pSettings->scrolling = true;

			/* Update display start address */
			pSettings->displayedDataStartAddress -= rowDiff * (GUI_MAIN_MAX_COLUMN_CHARACTERS / pSettings->numOfCharactersPerByte);
			/* Stop if it's smaller than the start of the FLASH address as this is where the data starts */
			if (pSettings->displayedDataStartAddress < constStartFlashAddress)
				pSettings->displayedDataStartAddress = constStartFlashAddress;


			/* Update display end address - we can only display GUI_MAIN_MAX_NUM_OF_CHARACTERS number of characters*/
			pSettings->displayedDataEndAddress = pSettings->displayedDataStartAddress +
												 (GUI_MAIN_MAX_NUM_OF_CHARACTERS / pSettings->numOfCharactersPerByte);
			if (pSettings->displayedDataEndAddress > currentWriteAddress)
			{
				pSettings->displayedDataEndAddress = currentWriteAddress;
				/* If the display end is the same as current write address we are not scrolling any longer */
				pSettings->scrolling = false;
				pSettings->displayedDataStartAddress = pSettings->displayedDataEndAddress - pSettings->numOfCharactersDisplayed;
			}

			/* Make sure we only update the screen if we haven't hit the end points */
			if (pSettings->displayedDataStartAddress != pSettings->lastDisplayDataStartAddress ||
				pSettings->displayedDataEndAddress != pSettings->lastDisplayDataEndAddress)
			{
				/* Save the current start and end address for next time */
				pSettings->lastDisplayDataStartAddress = pSettings->displayedDataStartAddress;
				pSettings->lastDisplayDataEndAddress = pSettings->displayedDataEndAddress;

				/* Update the display */
				pSettings->readAddress = pSettings->displayedDataStartAddress;
				prvClearMainTextBox();

				while (pSettings->readAddress != pSettings->displayedDataEndAddress)
				{
					prvDisplayDataInMainTextBox(&pSettings->readAddress, pSettings->displayedDataEndAddress, pSettings->writeFormat);
				}
			}

			/* Set it to 0 now that we have managed it */
			prvMainTextBoxYPosOffset = 0;
		}

		/*
		 * Check that we are not scrolling, if we do we don't want to show new data.
		 * We also check that the current read address is less then current write address which means there's new
		 * data we haven't written yet.
		 */
		if (!pSettings->scrolling && pSettings->readAddress < currentWriteAddress)
		{
			/* Display data in the main text box */
			prvDisplayDataInMainTextBox(&pSettings->readAddress, currentWriteAddress, pSettings->writeFormat);

			/* The end of the displayed data will be where we last read */
			pSettings->displayedDataEndAddress = pSettings->readAddress;

			/* Check if we are near the bottom */
			uint16_t xWritePos, yWritePos;
			GUI_GetWritePosition(guiConfigMAIN_TEXT_BOX_ID, &xWritePos, &yWritePos);
			uint32_t currentRow = yWritePos / 16;
			if (currentRow == GUI_MAIN_MAX_ROW_CHARACTERS - 1)
			{
				/* Get the start address */
				pSettings->displayedDataStartAddress += GUI_MAIN_MAX_COLUMN_CHARACTERS / pSettings->numOfCharactersPerByte;
				/* Set the read address to the beginning of the start address so
				 * that it will start reading from there the next time */
				pSettings->readAddress = pSettings->displayedDataStartAddress;

				/* Clear the main text box */
				prvClearMainTextBox();
				/* Update the screen with the old data we still want to see */
				while (pSettings->readAddress != pSettings->displayedDataEndAddress)
				{
					prvDisplayDataInMainTextBox(&pSettings->readAddress, pSettings->displayedDataEndAddress, pSettings->writeFormat);
				}

				/* The end of the displayed data will be where we last read */
				pSettings->displayedDataEndAddress = pSettings->readAddress;
			}

			/* Save how many characters are displayed on the screen */
			pSettings->numOfCharactersDisplayed = pSettings->displayedDataEndAddress - pSettings->displayedDataStartAddress;

#if 0
			/* DEBUG */
			GUI_SetWritePosition(guiConfigDEBUG_TEXT_BOX_ID, 5, 5);
			GUI_ClearTextBox(guiConfigDEBUG_TEXT_BOX_ID);
			GUI_WriteStringInTextBox(guiConfigDEBUG_TEXT_BOX_ID, "Data Count: ");
			GUI_WriteNumberInTextBox(guiConfigDEBUG_TEXT_BOX_ID, currentWriteAddress-constStartFlashAddress);
			GUI_WriteStringInTextBox(guiConfigDEBUG_TEXT_BOX_ID, ", numChar: ");
			GUI_WriteNumberInTextBox(guiConfigDEBUG_TEXT_BOX_ID, numOfCharactersDisplayed);
#endif
		}

		/* Give back the semaphore now that we are done */
		xSemaphoreGive(*pSemaphore);
	}
}

/**
 * @brief
 * @param	None
 * @retval	None
 */
void lcdChangeDisplayStateOfSidebar(uint32_t SidebarId)
{
	GUIDisplayState displayState = GUI_GetDisplayStateForContainer(SidebarId);
	if (displayState == GUIDisplayState_NotHidden)
	{
		/* Hide this sidebar and display the last active instead */
		GUI_HideContainer(SidebarId);
		GUI_DrawContainer(prvIdOfLastActiveSidebar);
		/* Set the last active as active now */
		prvIdOfActiveSidebar = prvIdOfLastActiveSidebar;
		prvIdOfLastActiveSidebar = guiConfigSIDEBAR_EMPTY_CONTAINER_ID;
	}
	else if (displayState == GUIDisplayState_Hidden)
	{
		/* Hide the active sidebar and display this sidebar instead */
		GUI_HideContainer(prvIdOfActiveSidebar);
		GUI_DrawContainer(SidebarId);
		/* Save the old active sidebar and set this sidebar as the new active one */
		prvIdOfLastActiveSidebar = prvIdOfActiveSidebar;
		prvIdOfActiveSidebar = SidebarId;
	}

	if (prvIdOfActiveSidebar != guiConfigSIDEBAR_SYSTEM_CONTAINER_ID)
	{
		prvActiveChannelHasChanged = true;
	}
}



/* Private functions .--------------------------------------------------------*/
/**
 * @brief	Will display data read from SPI FLASH in the main text box
 * @param	pFromAddress: Pointer to the start address
 * @param	pToAddress: Pointer to the end address
 * @param	Format: The format to use, can be any value of GUIWriteFormat
 * @retval	None
 * @note	Will only write FLASH_FETCH_BUFFER_SIZE amount of bytes. If the difference
 * 			between the to and from address is larger you must loop this function until
 * 			the two addresses are the same.
 * @time	~410 us when numOfBytesToFetch = 64
 */
static void prvDisplayDataInMainTextBox(uint32_t* pFromAddress, uint32_t ToAddress, GUIWriteFormat Format)
{
	uint32_t numOfBytesToFetch = ToAddress - *pFromAddress;
	if (numOfBytesToFetch > FLASH_FETCH_BUFFER_SIZE)
		numOfBytesToFetch = FLASH_FETCH_BUFFER_SIZE;
	SPI_FLASH_ReadBufferDMA(prvFlashFetchBuffer, *pFromAddress, numOfBytesToFetch);
	/* Make sure we only update the from address if we successfully could write to the screen */
	if (GUI_WriteBufferInTextBox(guiConfigMAIN_TEXT_BOX_ID, prvFlashFetchBuffer, numOfBytesToFetch, Format) != ERROR)
		*pFromAddress += numOfBytesToFetch;
}

/**
 * @brief	Callback function for the main text box refresh timer
 * @param	None
 * @retval	None
 */
static void prvMainTextBoxRefreshTimerCallback()
{
	/* Function pointer to the currently active managing function */
	static void (*activeManageFunction)() = 0;

	/* Check if a new channel has been selected */
	if (prvActiveChannelHasChanged)
	{
		prvActiveChannelHasChanged = false;
		prvClearMainTextBox();

		switch (prvIdOfActiveSidebar) {
			case guiConfigSIDEBAR_EMPTY_CONTAINER_ID:
				activeManageFunction = prvManageEmptyMainTextBox;
				break;
			case guiConfigSIDEBAR_CAN1_CONTAINER_ID:
				activeManageFunction = guiCan1ManageMainTextBox;
				break;
			case guiConfigSIDEBAR_CAN2_CONTAINER_ID:
				activeManageFunction = guiCan2ManageMainTextBox;
				break;
			case guiConfigSIDEBAR_UART1_CONTAINER_ID:
				activeManageFunction = guiUart1ManageMainTextBox;
				break;
			case guiConfigSIDEBAR_UART2_CONTAINER_ID:
				activeManageFunction = guiUart2ManageMainTextBox;
				break;
			case guiConfigSIDEBAR_RS232_CONTAINER_ID:
				activeManageFunction = guiRs232ManageMainTextBox;
				break;
			case guiConfigSIDEBAR_GPIO_CONTAINER_ID:
				activeManageFunction = guiGpioManageMainTextBox;
				break;
			case guiConfigSIDEBAR_ADC_CONTAINER_ID:
				activeManageFunction = guiAdcManageMainTextBox;
				break;
			default:
				activeManageFunction = 0;
				break;

		}
		lcdActiveMainTextBoxManagerShouldRefresh();
	}

	/* Only call the managing function if it's set */
	if (activeManageFunction != 0)
	{
		activeManageFunction();
	}

	/* Start the timer again */
	xTimerStart(prvMainTextBoxRefreshTimer, portMAX_DELAY);
}

/**
 * @brief	Manages how data is displayed in the main text box when the source is None
 * @param	None
 * @retval	None
 */
static void prvManageEmptyMainTextBox()
{

}

/**
 * @brief	Initializes the hardware
 * @param	None
 * @retval	None
 */
static void prvHardwareInit()
{
	/* LCD */
	LCD_Init();

	/* Capacitive Touch */
	FT5206_Init();
}

/**
 * @brief	Callback for the main text box
 * @param	Event: The event that caused the callback
 * @param	XPos: The X coordinate for the event
 * @param	YPos: The Y coordinate for the event
 * @retval	None
 */
static void prvMainTextBoxCallback(GUITouchEvent Event, uint16_t XPos, uint16_t YPos)
{
	static int32_t yDelta = 0;
	static uint32_t lastYValue = 0;
	static GUITouchEvent lastEvent = GUITouchEvent_Up;

	if (Event == GUITouchEvent_Up)
	{
		/* Update the delta one last time */
		yDelta =  YPos - lastYValue;
		prvMainTextBoxYPosOffset += yDelta;

		lastYValue = 0;
		lastEvent = GUITouchEvent_Up;
	}
	else if (Event == GUITouchEvent_Down)
	{
		/* Save the first Y coordinate */
		if (lastEvent == GUITouchEvent_Up)
		{
			lastYValue = YPos;
		}
		else
		{
			/* Update the delta */
			yDelta =  YPos - lastYValue;
			prvMainTextBoxYPosOffset += yDelta;
			lastYValue = YPos;
		}

		lastEvent = GUITouchEvent_Down;
	}

#if 0
	/* DEBUG */
	GUI_SetWritePosition(guiConfigDEBUG_TEXT_BOX_ID, 5, 5);
	GUI_ClearTextBox(guiConfigDEBUG_TEXT_BOX_ID);
	GUI_WriteStringInTextBox(guiConfigDEBUG_TEXT_BOX_ID, "yDelta:");
	GUI_WriteNumberInTextBox(guiConfigDEBUG_TEXT_BOX_ID, yDelta);
	GUI_SetWritePosition(guiConfigDEBUG_TEXT_BOX_ID, 200, 5);
	GUI_WriteStringInTextBox(guiConfigDEBUG_TEXT_BOX_ID, "prvMainTextBoxYPosOffset:");
	GUI_WriteNumberInTextBox(guiConfigDEBUG_TEXT_BOX_ID, prvMainTextBoxYPosOffset);
#endif
}

/**
 * @brief	Clears the main text box
 * @param	None
 * @retval	None
 */
static void prvClearMainTextBox()
{
	/* Clear the main text box */
	GUI_SetWritePosition(guiConfigMAIN_TEXT_BOX_ID, 0, 0);
	GUI_ClearTextBox(guiConfigMAIN_TEXT_BOX_ID);
}

/**
 * @brief	Checks if all channels are done with their initialization
 * @param	None
 * @retval	true if they are
 * @retval	false if they are not
 */
static bool prvAllChanneAreDoneInitializing()
{
	if (uart1IsDoneInitializing() && uart2IsDoneInitializing() && rs232IsDoneInitializing())
		return true;
	else
		return false;
}

/**
 * @brief	Initializes the GUI elements
 * @param	None
 * @retval	None
 */
static void prvInitGuiElements()
{
	GUI_Init();

	/* TODO: BUG? We need to clear the active window one time first for some reason */
	LCD_ClearActiveWindow(0, 0, 0, 0);

	/* CAN1 */
	guiCan1InitGuiElements();
	guiCan1UpdateGuiElementsReadFromSettings();

	/* CAN2 */
	guiCan2InitGuiElements();
	guiCan2UpdateGuiElementsReadFromSettings();

	/* UART1 */
	guiUart1InitGuiElements();
	guiUart1UpdateGuiElementsReadFromSettings();

	/* UART2 */
	guiUart2InitGuiElements();
	guiUart2UpdateGuiElementsReadFromSettings();

	/* RS232 */
	guiRs232InitGuiElements();
	guiRs232UpdateGuiElementsReadFromSettings();

	/* GPIO */
	guiGpioInitGuiElements();

	/* ADC */
	guiAdcInitGuiElements();

	/* System */
	prvSystemInitGuiElements();


	/* Text boxes ----------------------------------------------------------------*/
	/* Main text box */
	prvTextBox.object.id = guiConfigMAIN_TEXT_BOX_ID;
	prvTextBox.object.xPos = 0;
	prvTextBox.object.yPos = 50;
	prvTextBox.object.width = 650;
	prvTextBox.object.height = 405;
	prvTextBox.object.layer = GUILayer_0;
	prvTextBox.object.displayState = GUIDisplayState_NotHidden;
	prvTextBox.object.border = GUIBorder_Top | GUIBorder_Right;
	prvTextBox.object.borderThickness = 1;
	prvTextBox.object.borderColor = GUI_WHITE;
	prvTextBox.textColor = GUI_WHITE;
	prvTextBox.backgroundColor = LCD_COLOR_BLACK;
	prvTextBox.textSize = LCDFontEnlarge_1x;
	prvTextBox.xWritePos = 0;
	prvTextBox.yWritePos = 0;
	prvTextBox.touchCallback = prvMainTextBoxCallback;
	GUI_AddTextBox(&prvTextBox);

	/* Clock Text Box */
	prvTextBox.object.id = guiConfigCLOCK_TEXT_BOX_ID;
	prvTextBox.object.xPos = 650;
	prvTextBox.object.yPos = 0;
	prvTextBox.object.width = 150;
	prvTextBox.object.height = 25;
	prvTextBox.object.layer = GUILayer_0;
	prvTextBox.object.displayState = GUIDisplayState_Hidden;
	prvTextBox.object.border = GUIBorder_NoBorder;
	prvTextBox.object.borderThickness = 0;
	prvTextBox.object.borderColor = GUI_WHITE;
	prvTextBox.textColor = GUI_WHITE;
	prvTextBox.backgroundColor = LCD_COLOR_BLACK;
	prvTextBox.textSize = LCDFontEnlarge_1x;
	prvTextBox.xWritePos = 50;
	prvTextBox.yWritePos = 3;
	GUI_AddTextBox(&prvTextBox);

	/* Temperature Text Box */
	prvTextBox.object.id = guiConfigTEMP_TEXT_BOX_ID;
	prvTextBox.object.xPos = 651;
	prvTextBox.object.yPos = 25;
	prvTextBox.object.width = 149;
	prvTextBox.object.height = 25;
	prvTextBox.object.layer = GUILayer_0;
	prvTextBox.object.displayState = GUIDisplayState_NotHidden;
	prvTextBox.object.border = GUIBorder_NoBorder;
	prvTextBox.object.borderThickness = 0;
	prvTextBox.object.borderColor = GUI_WHITE;
	prvTextBox.textColor = GUI_WHITE;
	prvTextBox.backgroundColor = LCD_COLOR_BLACK;
	prvTextBox.textSize = LCDFontEnlarge_1x;
	prvTextBox.xWritePos = 50;
	prvTextBox.yWritePos = 3;
	GUI_AddTextBox(&prvTextBox);

	/* Debug Text Box */
	prvTextBox.object.id = guiConfigDEBUG_TEXT_BOX_ID;
	prvTextBox.object.xPos = 0;
	prvTextBox.object.yPos = 455;
	prvTextBox.object.width = 649;
	prvTextBox.object.height = 25;
	prvTextBox.object.layer = GUILayer_0;
	prvTextBox.object.displayState = GUIDisplayState_NotHidden;
	prvTextBox.object.border = GUIBorder_Top;
	prvTextBox.object.borderThickness = 1;
	prvTextBox.object.borderColor = GUI_WHITE;
	prvTextBox.textColor = GUI_WHITE;
	prvTextBox.backgroundColor = GUI_RED;
	prvTextBox.textSize = LCDFontEnlarge_1x;
	prvTextBox.xWritePos = 0;
	prvTextBox.yWritePos = 0;
	GUI_AddTextBox(&prvTextBox);

	GUI_DrawTextBox(guiConfigMAIN_TEXT_BOX_ID);

	/* Buttons -------------------------------------------------------------------*/

	/* Containers ----------------------------------------------------------------*/
	/* Status info container */
	prvContainer.object.id = guiConfigSTATUS_CONTAINER_ID;
	prvContainer.object.xPos = 650;
	prvContainer.object.yPos = 0;
	prvContainer.object.width = 150;
	prvContainer.object.height = 50;
	prvContainer.object.layer = GUILayer_0;
	prvContainer.object.displayState = GUIDisplayState_Hidden;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 1;
	prvContainer.object.borderColor = GUI_WHITE;
	prvContainer.contentHideState = GUIHideState_KeepBorders;
//	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(guiConfigCLOCK_TEXT_BOX_ID);
	prvContainer.textBoxes[1] = GUI_GetTextBoxFromId(guiConfigTEMP_TEXT_BOX_ID);
	GUI_AddContainer(&prvContainer);

	GUI_DrawContainer(guiConfigSTATUS_CONTAINER_ID);
	GUI_DrawContainer(guiConfigDEBUG_CONTAINER_ID);

	GUI_DrawContainer(guiConfigSIDEBAR_EMPTY_CONTAINER_ID);
	prvIdOfActiveSidebar = prvIdOfLastActiveSidebar = guiConfigSIDEBAR_EMPTY_CONTAINER_ID;
}

/* System GUI Elements =======================================================*/
/**
 * @brief	Callback for the debug button, will toggle the debug textbox on and off
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
static void prvDebugToggleCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(guiConfigDEBUG_CONTAINER_ID);
		if (displayState == GUIDisplayState_Hidden || displayState == GUIDisplayState_ContentHidden)
		{
			GUI_DrawContainer(guiConfigDEBUG_CONTAINER_ID);
		}
		else
		{
			GUI_HideContentInContainer(guiConfigDEBUG_CONTAINER_ID);
		}
	}
}

/**
 * @brief	Callback for the system button, will toggle the side system sidebar on and off
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
static void prvSystemButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(guiConfigSIDEBAR_SYSTEM_CONTAINER_ID);
		lcdChangeDisplayStateOfSidebar(guiConfigSIDEBAR_SYSTEM_CONTAINER_ID);
	}
}

/**
 * @brief	Callback for the save settings button, will save the settings of the channels to SPI FLASH
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
static void prvSaveSettingsButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
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
 * @brief
 * @param	None
 * @retval	None
 */
static void prvSystemInitGuiElements()
{
	/* Buttons -------------------------------------------------------------------*/
	/* Storage Button */
	prvButton.object.id = guiConfigSTORAGE_BUTTON_ID;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 50;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.layer = GUILayer_0;
	prvButton.object.displayState = GUIDisplayState_Hidden;
	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_SYSTEM_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_SYSTEM_BLUE;
	prvButton.pressedTextColor = GUI_SYSTEM_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = 0;
	prvButton.text[0] = "Storage";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* Settings Button */
	prvButton.object.id = guiConfigSETTINGS_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_SYSTEM_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_SYSTEM_BLUE;
	prvButton.pressedTextColor = GUI_SYSTEM_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = 0;
	prvButton.text[0] = "Settings";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* Debug Button */
	prvButton.object.id = guiConfigDEBUG_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_RED;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_RED;
	prvButton.pressedTextColor = GUI_RED;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvDebugToggleCallback;
	prvButton.text[0] = "Debug";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* Save settings Button */
	prvButton.object.id = guiConfigSAVE_SETTINGS_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_SYSTEM_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_SYSTEM_BLUE;
	prvButton.pressedTextColor = GUI_SYSTEM_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvSaveSettingsButtonCallback;
	prvButton.text[0] = "Save Channel";
	prvButton.text[1] = "Settings";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* System Button */
	prvButton.object.id = guiConfigSYSTEM_BUTTON_ID;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 455;
	prvButton.object.width = 150;
	prvButton.object.height = 25;
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
	prvButton.touchCallback = prvSystemButtonCallback;
	prvButton.text[0] = "System";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* Containers ----------------------------------------------------------------*/
	/* Debug container */
	prvContainer.object.id = guiConfigDEBUG_CONTAINER_ID;
	prvContainer.object.xPos = 0;
	prvContainer.object.yPos = 455;
	prvContainer.object.width = 650;
	prvContainer.object.height = 25;
	prvContainer.object.layer = GUILayer_0;
	prvContainer.object.displayState = GUIDisplayState_Hidden;
	prvContainer.object.border = GUIBorder_Right;
	prvContainer.object.borderThickness = 1;
	prvContainer.object.borderColor = GUI_WHITE;
	prvContainer.contentHideState = GUIHideState_KeepBorders;
	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(guiConfigDEBUG_TEXT_BOX_ID);
	GUI_AddContainer(&prvContainer);

	/* Side system container */
	prvContainer.object.id = guiConfigSIDEBAR_SYSTEM_CONTAINER_ID;
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
	prvContainer.buttons[0] = GUI_GetButtonFromId(guiConfigSETTINGS_BUTTON_ID);
	prvContainer.buttons[1] = GUI_GetButtonFromId(guiConfigSTORAGE_BUTTON_ID);
	prvContainer.buttons[2] = GUI_GetButtonFromId(guiConfigDEBUG_BUTTON_ID);
	prvContainer.buttons[3] = GUI_GetButtonFromId(guiConfigSAVE_SETTINGS_BUTTON_ID);
	GUI_AddContainer(&prvContainer);

	/* Side empty container */
	prvContainer.object.id = guiConfigSIDEBAR_EMPTY_CONTAINER_ID;
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
	GUI_AddContainer(&prvContainer);
}

/* Interrupt Handlers --------------------------------------------------------*/
