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

/* Private defines -----------------------------------------------------------*/
#define GUI_BLUE		0x237F
#define GUI_DARK_BLUE	0x0258
#define GUI_RED			0xF926
#define GUI_DARK_RED	0xA821
#define GUI_GREEN		0x362A
#define GUI_DARK_GREEN	0x1BC6
#define GUI_YELLOW		0xFEE6
#define GUI_DARK_YELLOW	0xC560
#define GUI_PURPLE		0xA8D6
#define GUI_DARK_PURPLE	0x788F
#define GUI_GRAY		0xB596
#define GUI_MAGENTA		0xF81F
#define GUI_CYAN_LIGHT	0x1F3C
#define GUI_CYAN_DARK	0x45F7
#define GUI_SYSTEM_BLUE	0x11CE
#define GUI_WHITE		LCD_COLOR_WHITE

#define GUI_MAIN_MAX_COLUMN_CHARACTERS	(81)
#define GUI_MAIN_MAX_ROW_CHARACTERS		(24)
#define GUI_MAIN_MAX_NUM_OF_CHARACTERS	(GUI_MAIN_MAX_COLUMN_CHARACTERS * (GUI_MAIN_MAX_ROW_CHARACTERS - 1))

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
static void prvManageGenericUartMainTextBox(const uint32_t constStartFlashAddress, uint32_t currentWriteAddress, UARTSettings* pSettings, SemaphoreHandle_t* pSemaphore);
static void prvGenericUartClearButtonCallback(GUITouchEvent Event, uint32_t ButtonId);

static void prvHardwareInit();
static void prvMainTextBoxCallback(GUITouchEvent Event, uint16_t XPos, uint16_t YPos);
static void prvClearMainTextBox();
static void prvInitGuiElements();

static void prvChangeDisplayStateOfSidebar(uint32_t SidebarId);

static void prvDebugToggleCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvSystemButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvSaveSettingsButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvInitSystemGuiElements();

/* CAN1 */
static void prvManageCan1MainTextBox();
static void prvCan1EnableButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvCan1TerminationButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvCan1TopButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvCan1BitRateButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvCan1BitRateSelectionCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvInitCan1GuiElements();

/* CAN2 */
static void prvManageCan2MainTextBox();
static void prvCan2EnableButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvCan2TerminationButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvCan2TopButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvCan2BitRateButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvCan2BitRateSelectionCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvInitCan2GuiElements();

/* UART1 */
static void prvManageUart1MainTextBox();
static void prvUart1EnableButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvUart1VoltageLevelButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvUart1FormatButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvUart1DebugButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvUart1TopButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvUart1BaudRateButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvUart1BaudRateSelectionCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvInitUart1GuiElements();

/* UART2 */
static void prvManageUart2MainTextBox();
static void prvUart2EnableButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvUart2VoltageLevelButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvUart2FormatButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvUart2DebugButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvUart2TopButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvUart2BaudRateButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvUart2BaudRateSelectionCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvInitUart2GuiElements();

/* RS232 */
static void prvManageRs232MainTextBox();
static void prvRs232EnableButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvRs232FormatButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvRs232DebugButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvRs232TopButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvRs232BaudRateButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvRs232BaudRateSelectionCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvInitRs232GuiElements();

/* GPIO */
static void prvManageGpioMainTextBox();
static void prvGpioTopButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvInitGpioGuiElements();

/* ADC */
static void prvManageAdcMainTextBox();
static void prvAdcTopButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvInitAdcGuiElements();

/* Functions -----------------------------------------------------------------*/
/**
 * @brief	The main task for the LCD
 * @param	pvParameters:
 * @retval	None
 */
void lcdTask(void *pvParameters)
{
	prvHardwareInit();
	prvInitGuiElements();

	xLCDEventQueue = xQueueCreate(10, sizeof(LCDEventMessage));
	if (xLCDEventQueue == 0)
	{
		// Queue was not created and must not be used.
	}

	prvMainTextBoxRefreshTimer = xTimerCreate("MainTextBoxTimer", 10 / portTICK_PERIOD_MS, 0, 0, prvMainTextBoxRefreshTimerCallback);
	if (prvMainTextBoxRefreshTimer != NULL)
		xTimerStart(prvMainTextBoxRefreshTimer, portMAX_DELAY);

	/* The parameter in vTaskDelayUntil is the absolute time
	 * in ticks at which you want to be woken calculated as
	 * an increment from the time you were last woken. */
	TickType_t xNextWakeTime;
	/* Initialize xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	LCDEventMessage receivedMessage;

	GUI_WriteStringInTextBox(guiConfigCLOCK_TEXT_BOX_ID, "14:15:12");

	uint8_t text[2] = "A";

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
				activeManageFunction = prvManageCan1MainTextBox;
				break;
			case guiConfigSIDEBAR_CAN2_CONTAINER_ID:
				activeManageFunction = prvManageCan2MainTextBox;
				break;
			case guiConfigSIDEBAR_UART1_CONTAINER_ID:
				activeManageFunction = prvManageUart1MainTextBox;
				break;
			case guiConfigSIDEBAR_UART2_CONTAINER_ID:
				activeManageFunction = prvManageUart2MainTextBox;
				break;
			case guiConfigSIDEBAR_RS232_CONTAINER_ID:
				activeManageFunction = prvManageRs232MainTextBox;
				break;
			case guiConfigSIDEBAR_GPIO_CONTAINER_ID:
				activeManageFunction = prvManageGpioMainTextBox;
				break;
			case guiConfigSIDEBAR_ADC_CONTAINER_ID:
				activeManageFunction = prvManageAdcMainTextBox;
				break;
			default:
				activeManageFunction = 0;
				break;

		}
		prcActiveMainTextBoxManagerShouldRefresh = true;
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
 * @brief	Manages how data is displayed in the main text box when the source is an UART channel
 * @param	constStartFlashAddress: The address in SPI FLASH where the first data is for the channel
 * @param	currentWriteAddress: The current write address for the channel
 * @param	pSettings: Pointer to the settings for the channel
 * @param	pSemaphore: Pointer to the settings semaphore
 * @retval	None
 */
static void prvManageGenericUartMainTextBox(const uint32_t constStartFlashAddress, uint32_t currentWriteAddress, UARTSettings* pSettings, SemaphoreHandle_t* pSemaphore)
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
				pSettings->displayedDataStartAddress = pSettings->displayedDataEndAddress - numOfCharactersToDisplay*pSettings->numOfCharactersPerByte;
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
			pSettings->displayedDataEndAddress = pSettings->displayedDataStartAddress + (GUI_MAIN_MAX_NUM_OF_CHARACTERS / pSettings->numOfCharactersPerByte);
			if (pSettings->displayedDataEndAddress > currentWriteAddress)
			{
				pSettings->displayedDataEndAddress = currentWriteAddress;
				/* If the display end is the same as current write address we are not scrolling any longer */
				pSettings->scrolling = false;
				pSettings->displayedDataStartAddress = pSettings->displayedDataEndAddress - pSettings->numOfCharactersDisplayed;
			}

			/* Make sure we only update the screen if we haven't hit the end points */
			if (pSettings->displayedDataStartAddress != pSettings->lastDisplayDataStartAddress || pSettings->displayedDataEndAddress != pSettings->lastDisplayDataEndAddress)
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
 * @brief	Callback a generic UART clear button
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
static void prvGenericUartClearButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
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

		/* If a channel was reset we should clear the main text box */
		if (channelWasReset)
			prvClearMainTextBox();
	}
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
	prvInitCan1GuiElements();

	/* CAN2 */
	prvInitCan2GuiElements();

	/* UART1 */
	prvInitUart1GuiElements();

	/* UART2 */
	prvInitUart2GuiElements();

	/* RS232 */
	prvInitRs232GuiElements();

	/* GPIO */
	prvInitGpioGuiElements();

	/* ADC */
	prvInitAdcGuiElements();

	/* System */
	prvInitSystemGuiElements();


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

/**
 * @brief
 * @param	None
 * @retval	None
 */
static void prvChangeDisplayStateOfSidebar(uint32_t SidebarId)
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
		prvChangeDisplayStateOfSidebar(guiConfigSIDEBAR_SYSTEM_CONTAINER_ID);
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
static void prvInitSystemGuiElements()
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

/* CAN1 GUI Elements ========================================================*/
/**
 * @brief	Manages how data is displayed in the main text box when the source is CAN1
 * @param	None
 * @retval	None
 */
static void prvManageCan1MainTextBox()
{

}

/**
 * @brief	Callback for the enable button
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
static void prvCan1EnableButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
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
				GUI_SetButtonTextForRow(guiConfigCAN1_ENABLE_BUTTON_ID, "Disabled", 1);
				GUI_SetButtonState(guiConfigCAN1_TOP_BUTTON_ID, GUIButtonState_Disabled);

				GUI_SetButtonTextForRow(guiConfigCAN1_BIT_RATE_BUTTON_ID, "< Bit Rate:", 0);
				GUI_SetButtonState(guiConfigCAN1_BIT_RATE_BUTTON_ID, GUIButtonState_Disabled);
			}
		}
		else
		{
			ErrorStatus status = can1SetConnection(CANConnection_Connected);
			if (status == SUCCESS)
			{
				enabled = true;
				GUI_SetButtonTextForRow(guiConfigCAN1_ENABLE_BUTTON_ID, "Enabled ", 1);
				GUI_SetButtonState(guiConfigCAN1_TOP_BUTTON_ID, GUIButtonState_Enabled);

				GUI_SetButtonTextForRow(guiConfigCAN1_BIT_RATE_BUTTON_ID, "  Bit Rate:", 0);
				GUI_SetButtonState(guiConfigCAN1_BIT_RATE_BUTTON_ID, GUIButtonState_DisabledTouch);
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
static void prvCan1TerminationButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
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
				GUI_SetButtonTextForRow(guiConfigCAN1_TERMINATION_BUTTON_ID, "None ", 1);
			}
		}
		else
		{
			ErrorStatus status = can1SetTermination(CANTermination_Connected);
			if (status == SUCCESS)
			{
				terminated = true;
				GUI_SetButtonTextForRow(guiConfigCAN1_TERMINATION_BUTTON_ID, "120 R ", 1);
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
static void prvCan1TopButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(guiConfigSIDEBAR_CAN1_CONTAINER_ID);
		prvChangeDisplayStateOfSidebar(guiConfigSIDEBAR_CAN1_CONTAINER_ID);
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
static void prvCan1BitRateButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(guiConfigPOPOUT_CAN1_BIT_RATE_CONTAINER_ID);

		if (displayState == GUIDisplayState_Hidden)
		{
			GUI_SetActiveLayer(GUILayer_1);
			GUI_SetLayerForButton(guiConfigCAN1_BIT_RATE_BUTTON_ID, GUILayer_1);
			GUI_SetButtonState(guiConfigCAN1_BIT_RATE_BUTTON_ID, GUIButtonState_Enabled);
			GUI_DrawContainer(guiConfigPOPOUT_CAN1_BIT_RATE_CONTAINER_ID);
		}
		else if (displayState == GUIDisplayState_NotHidden)
		{
			GUI_HideContainer(guiConfigPOPOUT_CAN1_BIT_RATE_CONTAINER_ID);
			GUI_SetActiveLayer(GUILayer_0);
			GUI_SetLayerForButton(guiConfigCAN1_BIT_RATE_BUTTON_ID, GUILayer_0);
			GUI_SetButtonState(guiConfigCAN1_BIT_RATE_BUTTON_ID, GUIButtonState_Disabled);

			/* Refresh the main text box */
			prcActiveMainTextBoxManagerShouldRefresh = true;
		}
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
static void prvCan1BitRateSelectionCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		CANBitRate newBitRate;
		switch (ButtonId)
		{
			case guiConfigCAN1_BIT10K_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigCAN1_BIT_RATE_BUTTON_ID, " 10kbit/s", 1);
				newBitRate = CANBitRate_10k;
				break;
			case guiConfigCAN1_BIT20K_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigCAN1_BIT_RATE_BUTTON_ID, " 20kbit/s", 1);
				newBitRate = CANBitRate_20k;
				break;
			case guiConfigCAN1_BIT50K_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigCAN1_BIT_RATE_BUTTON_ID, " 50kbit/s", 1);
				newBitRate = CANBitRate_50k;
				break;
			case guiConfigCAN1_BIT100K_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigCAN1_BIT_RATE_BUTTON_ID, "100kbit/s", 1);
				newBitRate = CANBitRate_100k;
				break;
			case guiConfigCAN1_BIT125K_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigCAN1_BIT_RATE_BUTTON_ID, "125kbit/s", 1);
				newBitRate = CANBitRate_125k;
				break;
			case guiConfigCAN1_BIT250K_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigCAN1_BIT_RATE_BUTTON_ID, "250kbit/s", 1);
				newBitRate = CANBitRate_250k;
				break;
			case guiConfigCAN1_BIT500K_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigCAN1_BIT_RATE_BUTTON_ID, "500kbit/s", 1);
				newBitRate = CANBitRate_500k;
				break;
			case guiConfigCAN1_BIT1M_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigCAN1_BIT_RATE_BUTTON_ID, "  1Mbit/s", 1);
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
		GUI_HideContainer(guiConfigPOPOUT_CAN1_BIT_RATE_CONTAINER_ID);
		GUI_SetActiveLayer(GUILayer_0);
		GUI_SetLayerForButton(guiConfigCAN1_BIT_RATE_BUTTON_ID, GUILayer_0);
		GUI_SetButtonState(guiConfigCAN1_BIT_RATE_BUTTON_ID, GUIButtonState_Disabled);

		/* Refresh the main text box */
		prcActiveMainTextBoxManagerShouldRefresh = true;
	}
}

/**
 * @brief
 * @param	None
 * @retval	None
 */
static void prvInitCan1GuiElements()
{
	/* Text boxes ----------------------------------------------------------------*/
	/* CAN1 Label text box */
	prvTextBox.object.id = guiConfigCAN1_LABEL_TEXT_BOX_ID;
	prvTextBox.object.xPos = 650;
	prvTextBox.object.yPos = 50;
	prvTextBox.object.width = 150;
	prvTextBox.object.height = 50;
	prvTextBox.object.layer = GUILayer_0;
	prvTextBox.object.displayState = GUIDisplayState_Hidden;
	prvTextBox.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvTextBox.object.borderThickness = 1;
	prvTextBox.object.borderColor = GUI_WHITE;
	prvTextBox.textColor = GUI_BLUE;
	prvTextBox.backgroundColor = GUI_WHITE;
	prvTextBox.staticText = "CAN1";
	prvTextBox.textSize = LCDFontEnlarge_2x;
	GUI_AddTextBox(&prvTextBox);

	/* Buttons -------------------------------------------------------------------*/
	/* CAN1 Top Button */
	prvButton.object.id = guiConfigCAN1_TOP_BUTTON_ID;
	prvButton.object.xPos = 0;
	prvButton.object.yPos = 0;
	prvButton.object.width = 100;
	prvButton.object.height = 50;
	prvButton.object.layer = GUILayer_0;
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
	prvButton.touchCallback = prvCan1TopButtonCallback;
	prvButton.text[0] = "CAN1";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* CAN1 Enable Button */
	prvButton.object.id = guiConfigCAN1_ENABLE_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_BLUE;
	prvButton.pressedTextColor = GUI_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvCan1EnableButtonCallback;
	prvButton.text[0] = "Output:";
//	prvButton.text[1] = "Enabled ";
	prvButton.text[1] = "Disabled";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* CAN1 Bit Rate Button */
	prvButton.object.id = guiConfigCAN1_BIT_RATE_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_DARK_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_BLUE;
	prvButton.pressedTextColor = GUI_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvCan1BitRateButtonCallback;
	prvButton.text[0] = "< Bit Rate:";
	prvButton.text[1] = "125kbit/s";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* CAN1 Termination Button */
	prvButton.object.id = guiConfigCAN1_TERMINATION_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_BLUE;
	prvButton.pressedTextColor = GUI_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvCan1TerminationButtonCallback;
	prvButton.text[0] = "Termination:";
	prvButton.text[1] = "None ";
//	prvButton.text[1] = "120 R";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);


	/* CAN1 10k bit rate Button */
	prvButton.object.id = guiConfigCAN1_BIT10K_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_BLUE;
	prvButton.pressedTextColor = GUI_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvCan1BitRateSelectionCallback;
	prvButton.text[0] = "10kbit/s";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* CAN1 20k bit rate Button */
	prvButton.object.id = guiConfigCAN1_BIT20K_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_BLUE;
	prvButton.pressedTextColor = GUI_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvCan1BitRateSelectionCallback;
	prvButton.text[0] = "20kbit/s";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* CAN1 50k bit rate Button */
	prvButton.object.id = guiConfigCAN1_BIT50K_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_BLUE;
	prvButton.pressedTextColor = GUI_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvCan1BitRateSelectionCallback;
	prvButton.text[0] = "50kbit/s";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* CAN1 100k bit rate Button */
	prvButton.object.id = guiConfigCAN1_BIT100K_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_BLUE;
	prvButton.pressedTextColor = GUI_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvCan1BitRateSelectionCallback;
	prvButton.text[0] = "100kbit/s";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* CAN1 125k bit rate Button */
	prvButton.object.id = guiConfigCAN1_BIT125K_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_BLUE;
	prvButton.pressedTextColor = GUI_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvCan1BitRateSelectionCallback;
	prvButton.text[0] = "125kbit/s";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* CAN1 250k bit rate Button */
	prvButton.object.id = guiConfigCAN1_BIT250K_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_BLUE;
	prvButton.pressedTextColor = GUI_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvCan1BitRateSelectionCallback;
	prvButton.text[0] = "250kbit/s";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* CAN1 500k bit rate Button */
	prvButton.object.id = guiConfigCAN1_BIT500K_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_BLUE;
	prvButton.pressedTextColor = GUI_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvCan1BitRateSelectionCallback;
	prvButton.text[0] = "500kbit/s";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* CAN1 1M bit rate Button */
	prvButton.object.id = guiConfigCAN1_BIT1M_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_BLUE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_BLUE;
	prvButton.pressedTextColor = GUI_BLUE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvCan1BitRateSelectionCallback;
	prvButton.text[0] = "1Mbit/s";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* Containers ----------------------------------------------------------------*/
	/* Sidebar CAN1 container */
	prvContainer.object.id = guiConfigSIDEBAR_CAN1_CONTAINER_ID;
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
	prvContainer.buttons[0] = GUI_GetButtonFromId(guiConfigCAN1_ENABLE_BUTTON_ID);
	prvContainer.buttons[1] = GUI_GetButtonFromId(guiConfigCAN1_BIT_RATE_BUTTON_ID);
	prvContainer.buttons[2] = GUI_GetButtonFromId(guiConfigCAN1_TERMINATION_BUTTON_ID);
	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(guiConfigCAN1_LABEL_TEXT_BOX_ID);
	GUI_AddContainer(&prvContainer);

	/* CAN1 bit rate popout container */
	prvContainer.object.id = guiConfigPOPOUT_CAN1_BIT_RATE_CONTAINER_ID;
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
	prvContainer.buttons[0] = GUI_GetButtonFromId(guiConfigCAN1_BIT10K_BUTTON_ID);
	prvContainer.buttons[1] = GUI_GetButtonFromId(guiConfigCAN1_BIT20K_BUTTON_ID);
	prvContainer.buttons[2] = GUI_GetButtonFromId(guiConfigCAN1_BIT50K_BUTTON_ID);
	prvContainer.buttons[3] = GUI_GetButtonFromId(guiConfigCAN1_BIT100K_BUTTON_ID);
	prvContainer.buttons[4] = GUI_GetButtonFromId(guiConfigCAN1_BIT125K_BUTTON_ID);
	prvContainer.buttons[5] = GUI_GetButtonFromId(guiConfigCAN1_BIT250K_BUTTON_ID);
	prvContainer.buttons[6] = GUI_GetButtonFromId(guiConfigCAN1_BIT500K_BUTTON_ID);
	prvContainer.buttons[7] = GUI_GetButtonFromId(guiConfigCAN1_BIT1M_BUTTON_ID);
	GUI_AddContainer(&prvContainer);
}

/* CAN2 GUI Elements ========================================================*/
/**
 * @brief	Manages how data is displayed in the main text box when the source is CAN2
 * @param	None
 * @retval	None
 */
static void prvManageCan2MainTextBox()
{

}

/**
 * @brief	Callback for the enable button
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
static void prvCan2EnableButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
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
static void prvCan2TerminationButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
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
static void prvCan2TopButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(guiConfigSIDEBAR_CAN2_CONTAINER_ID);
		prvChangeDisplayStateOfSidebar(guiConfigSIDEBAR_CAN2_CONTAINER_ID);
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
static void prvCan2BitRateButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
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
			prcActiveMainTextBoxManagerShouldRefresh = true;
		}
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
static void prvCan2BitRateSelectionCallback(GUITouchEvent Event, uint32_t ButtonId)
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
		SemaphoreHandle_t* settingsSemaphore = can1GetSettingsSemaphore();
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
		prcActiveMainTextBoxManagerShouldRefresh = true;
	}
}

/**
 * @brief
 * @param	None
 * @retval	None
 */
static void prvInitCan2GuiElements()
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
	prvButton.touchCallback = prvCan2TopButtonCallback;
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
	prvButton.touchCallback = prvCan2EnableButtonCallback;
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
	prvButton.touchCallback = prvCan2BitRateButtonCallback;
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
	prvButton.touchCallback = prvCan2TerminationButtonCallback;
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
	prvButton.touchCallback = prvCan2BitRateSelectionCallback;
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
	prvButton.touchCallback = prvCan2BitRateSelectionCallback;
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
	prvButton.touchCallback = prvCan2BitRateSelectionCallback;
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
	prvButton.touchCallback = prvCan2BitRateSelectionCallback;
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
	prvButton.touchCallback = prvCan2BitRateSelectionCallback;
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
	prvButton.touchCallback = prvCan2BitRateSelectionCallback;
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
	prvButton.touchCallback = prvCan2BitRateSelectionCallback;
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
	prvButton.touchCallback = prvCan2BitRateSelectionCallback;
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

/* UART1 GUI Elements ========================================================*/
/**
 * @brief	Manages how data is displayed in the main text box when the source is UART1
 * @param	None
 * @retval	None
 */
static void prvManageUart1MainTextBox()
{
	const uint32_t constStartFlashAddress = FLASH_ADR_UART1_DATA;

	/* Get the current write address, this is the address where the last data is */
	uint32_t currentWriteAddress = uart1GetCurrentWriteAddress();
	/* Get the current settings of the channel */
	UARTSettings* settings = uart1GetSettings();
	SemaphoreHandle_t* settingsSemaphore = uart1GetSettingsSemaphore();

	prvManageGenericUartMainTextBox(constStartFlashAddress, currentWriteAddress, settings, settingsSemaphore);
}

/**
 * @brief	Callback for the enable button
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
static void prvUart1EnableButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
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
static void prvUart1VoltageLevelButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
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
static void prvUart1FormatButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
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
static void prvUart1DebugButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
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
static void prvUart1TopButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		/* Get the current display state of the sidebar */
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(guiConfigSIDEBAR_UART1_CONTAINER_ID);

		if (displayState == GUIDisplayState_Hidden)
		{
			/* Update the baud rate text to match what is actually set */
			UARTSettings* settings = uart1GetSettings();
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
					GUI_SetButtonTextForRow(guiConfigUART1_FORMAT_BUTTON_ID, " Hex ", 1);
					break;
				case GUIWriteFormat_Hex:
					GUI_SetButtonTextForRow(guiConfigUART1_FORMAT_BUTTON_ID, "ASCII", 1);
					break;
				default:
					break;
			}
		}
		/* Change the state of the sidebar */
		prvChangeDisplayStateOfSidebar(guiConfigSIDEBAR_UART1_CONTAINER_ID);
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
static void prvUart1BaudRateButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
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
			prcActiveMainTextBoxManagerShouldRefresh = true;
		}
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
static void prvUart1BaudRateSelectionCallback(GUITouchEvent Event, uint32_t ButtonId)
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
		prcActiveMainTextBoxManagerShouldRefresh = true;
	}
}

/**
 * @brief
 * @param	None
 * @retval	None
 */
static void prvInitUart1GuiElements()
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
	prvButton.touchCallback = prvUart1TopButtonCallback;
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
	prvButton.touchCallback = prvUart1EnableButtonCallback;
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
	prvButton.touchCallback = prvUart1BaudRateButtonCallback;
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
	prvButton.touchCallback = prvUart1VoltageLevelButtonCallback;
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
	prvButton.touchCallback = prvUart1FormatButtonCallback;
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
	prvButton.touchCallback = prvGenericUartClearButtonCallback;
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
	prvButton.touchCallback = prvUart1DebugButtonCallback;
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
	prvButton.touchCallback = prvUart1BaudRateSelectionCallback;
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
	prvButton.touchCallback = prvUart1BaudRateSelectionCallback;
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
	prvButton.touchCallback = prvUart1BaudRateSelectionCallback;
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
	prvButton.touchCallback = prvUart1BaudRateSelectionCallback;
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
	prvButton.touchCallback = prvUart1BaudRateSelectionCallback;
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
	prvButton.touchCallback = prvUart1BaudRateSelectionCallback;
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
	prvButton.touchCallback = prvUart1BaudRateSelectionCallback;
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
	prvButton.touchCallback = prvUart1BaudRateSelectionCallback;
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

/* UART2 GUI Elements ========================================================*/
/**
 * @brief	Manages how data is displayed in the main text box when the source is UART2
 * @param	None
 * @retval	None
 */
static void prvManageUart2MainTextBox()
{
	const uint32_t constStartFlashAddress = FLASH_ADR_UART2_DATA;

	/* Get the current write address, this is the address where the last data is */
	uint32_t currentWriteAddress = uart2GetCurrentWriteAddress();
	/* Get the current settings of the channel */
	UARTSettings* settings = uart2GetSettings();
	SemaphoreHandle_t* settingsSemaphore = uart1GetSettingsSemaphore();

	prvManageGenericUartMainTextBox(constStartFlashAddress, currentWriteAddress, settings, settingsSemaphore);
}

/**
 * @brief	Callback for the enable button
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
static void prvUart2EnableButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	static bool enabled = false;

	if (Event == GUITouchEvent_Up)
	{
		if (enabled)
		{
			ErrorStatus status = uart2SetConnection(UARTConnection_Disconnected);
			if (status == SUCCESS)
			{
				enabled = false;
				GUI_SetButtonTextForRow(guiConfigUART2_ENABLE_BUTTON_ID, "Disabled", 1);
				GUI_SetButtonState(guiConfigUART2_TOP_BUTTON_ID, GUIButtonState_Disabled);

				GUI_SetButtonTextForRow(guiConfigUART2_BAUD_RATE_BUTTON_ID, "< Baud Rate:", 0);
				GUI_SetButtonState(guiConfigUART2_BAUD_RATE_BUTTON_ID, GUIButtonState_Disabled);
			}
		}
		else
		{
			ErrorStatus status = uart2SetConnection(UARTConnection_Connected);
			if (status == SUCCESS)
			{
				enabled = true;
				GUI_SetButtonTextForRow(guiConfigUART2_ENABLE_BUTTON_ID, "Enabled ", 1);
				GUI_SetButtonState(guiConfigUART2_TOP_BUTTON_ID, GUIButtonState_Enabled);

				GUI_SetButtonTextForRow(guiConfigUART2_BAUD_RATE_BUTTON_ID, "  Baud Rate:", 0);
				GUI_SetButtonState(guiConfigUART2_BAUD_RATE_BUTTON_ID, GUIButtonState_DisabledTouch);
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
static void prvUart2VoltageLevelButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	static bool level5VisActive = false;

	if (Event == GUITouchEvent_Up)
	{
		if (level5VisActive)
		{
			ErrorStatus status = uart2SetPower(UARTPower_3V3);
			if (status == SUCCESS)
			{
				level5VisActive = false;
				GUI_SetButtonTextForRow(guiConfigUART2_VOLTAGE_LEVEL_BUTTON_ID, "3.3 V", 1);
			}
		}
		else
		{
			ErrorStatus status = uart2SetPower(UARTPower_5V);
			if (status == SUCCESS)
			{
				level5VisActive = true;
				GUI_SetButtonTextForRow(guiConfigUART2_VOLTAGE_LEVEL_BUTTON_ID, " 5 V ", 1);
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
static void prvUart2FormatButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		UARTSettings* settings = uart2GetSettings();
		SemaphoreHandle_t* settingsSemaphore = uart1GetSettingsSemaphore();
		/* Try to take the settings semaphore */
		if (*settingsSemaphore != 0 && xSemaphoreTake(*settingsSemaphore, 100) == pdTRUE)
		{
			if (settings->writeFormat == GUIWriteFormat_ASCII)
			{
				settings->writeFormat = GUIWriteFormat_Hex;
				settings->numOfCharactersPerByte = 3;
				GUI_SetButtonTextForRow(guiConfigUART2_FORMAT_BUTTON_ID, " Hex ", 1);
			}
			else if (settings->writeFormat == GUIWriteFormat_Hex)
			{
				settings->writeFormat = GUIWriteFormat_ASCII;
				settings->numOfCharactersPerByte = 1;
				GUI_SetButtonTextForRow(guiConfigUART2_FORMAT_BUTTON_ID, "ASCII", 1);
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
static void prvUart2DebugButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	static bool enabled = false;
	static UARTMode lastMode;

	if (Event == GUITouchEvent_Up)
	{
		UARTSettings* settings = uart2GetSettings();
		SemaphoreHandle_t* settingsSemaphore = uart1GetSettingsSemaphore();
		/* Try to take the settings semaphore */
		if (*settingsSemaphore != 0 && xSemaphoreTake(*settingsSemaphore, 100) == pdTRUE)
		{
			if (enabled)
			{
				settings->mode = lastMode;
				enabled = false;
				GUI_SetButtonTextForRow(guiConfigUART2_DEBUG_BUTTON_ID, "Disabled", 1);
			}
			else
			{
				lastMode = settings->mode;
				settings->mode = UARTMode_DebugTX;
				enabled = true;
				GUI_SetButtonTextForRow(guiConfigUART2_DEBUG_BUTTON_ID, "Enabled ", 1);
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
static void prvUart2TopButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		/* Get the current display state of the sidebar */
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(guiConfigSIDEBAR_UART2_CONTAINER_ID);

		if (displayState == GUIDisplayState_Hidden)
		{
			/* Update the baud rate text to match what is actually set */
			UARTSettings* settings = uart2GetSettings();
			switch (settings->baudRate)
			{
				case UARTBaudRate_4800:
					GUI_SetButtonTextForRow(guiConfigUART2_BAUD_RATE_BUTTON_ID, "  4800 bps", 1);
					break;
				case UARTBaudRate_7200:
					GUI_SetButtonTextForRow(guiConfigUART2_BAUD_RATE_BUTTON_ID, "  7200 bps", 1);
					break;
				case UARTBaudRate_9600:
					GUI_SetButtonTextForRow(guiConfigUART2_BAUD_RATE_BUTTON_ID, "  9600 bps", 1);
					break;
				case UARTBaudRate_19200:
					GUI_SetButtonTextForRow(guiConfigUART2_BAUD_RATE_BUTTON_ID, " 19200 bps", 1);
					break;
				case UARTBaudRate_28800:
					GUI_SetButtonTextForRow(guiConfigUART2_BAUD_RATE_BUTTON_ID, " 28800 bps", 1);
					break;
				case UARTBaudRate_38400:
					GUI_SetButtonTextForRow(guiConfigUART2_BAUD_RATE_BUTTON_ID, " 38400 bps", 1);
					break;
				case UARTBaudRate_57600:
					GUI_SetButtonTextForRow(guiConfigUART2_BAUD_RATE_BUTTON_ID, " 57600 bps", 1);
					break;
				case UARTBaudRate_115200:
					GUI_SetButtonTextForRow(guiConfigUART2_BAUD_RATE_BUTTON_ID, "115200 bps", 1);
					break;
				default:
					break;
			}
			/* Update the write format text to match what is actually set */
			switch (settings->writeFormat)
			{
				case GUIWriteFormat_ASCII:
					GUI_SetButtonTextForRow(guiConfigUART2_FORMAT_BUTTON_ID, " Hex ", 1);
					break;
				case GUIWriteFormat_Hex:
					GUI_SetButtonTextForRow(guiConfigUART2_FORMAT_BUTTON_ID, "ASCII", 1);
					break;
				default:
					break;
			}
		}
		/* Change the state of the sidebar */
		prvChangeDisplayStateOfSidebar(guiConfigSIDEBAR_UART2_CONTAINER_ID);
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
static void prvUart2BaudRateButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(guiConfigPOPOUT_UART2_BAUD_RATE_CONTAINER_ID);

		if (displayState == GUIDisplayState_Hidden)
		{
			GUI_SetActiveLayer(GUILayer_1);
			GUI_SetLayerForButton(guiConfigUART2_BAUD_RATE_BUTTON_ID, GUILayer_1);
			GUI_SetButtonState(guiConfigUART2_BAUD_RATE_BUTTON_ID, GUIButtonState_Enabled);
			GUI_DrawContainer(guiConfigPOPOUT_UART2_BAUD_RATE_CONTAINER_ID);
		}
		else if (displayState == GUIDisplayState_NotHidden)
		{
			GUI_HideContainer(guiConfigPOPOUT_UART2_BAUD_RATE_CONTAINER_ID);
			GUI_SetActiveLayer(GUILayer_0);
			GUI_SetLayerForButton(guiConfigUART2_BAUD_RATE_BUTTON_ID, GUILayer_0);
			GUI_SetButtonState(guiConfigUART2_BAUD_RATE_BUTTON_ID, GUIButtonState_Disabled);

			/* Refresh the main text box */
			prcActiveMainTextBoxManagerShouldRefresh = true;
		}
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
static void prvUart2BaudRateSelectionCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		UARTBaudRate newBaudRate;
		switch (ButtonId)
		{
			case guiConfigUART2_BAUD4800_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigUART2_BAUD_RATE_BUTTON_ID, "  4800 bps", 1);
				newBaudRate = UARTBaudRate_4800;
				break;
			case guiConfigUART2_BAUD7200_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigUART2_BAUD_RATE_BUTTON_ID, "  7200 bps", 1);
				newBaudRate = UARTBaudRate_7200;
				break;
			case guiConfigUART2_BAUD9600_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigUART2_BAUD_RATE_BUTTON_ID, "  9600 bps", 1);
				newBaudRate = UARTBaudRate_9600;
				break;
			case guiConfigUART2_BAUD19K2_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigUART2_BAUD_RATE_BUTTON_ID, " 19200 bps", 1);
				newBaudRate = UARTBaudRate_19200;
				break;
			case guiConfigUART2_BAUD28K8_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigUART2_BAUD_RATE_BUTTON_ID, " 28800 bps", 1);
				newBaudRate = UARTBaudRate_28800;
				break;
			case guiConfigUART2_BAUD38K4_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigUART2_BAUD_RATE_BUTTON_ID, " 38400 bps", 1);
				newBaudRate = UARTBaudRate_38400;
				break;
			case guiConfigUART2_BAUD57K6_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigUART2_BAUD_RATE_BUTTON_ID, " 57600 bps", 1);
				newBaudRate = UARTBaudRate_57600;
				break;
			case guiConfigUART2_BAUD115K_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigUART2_BAUD_RATE_BUTTON_ID, "115200 bps", 1);
				newBaudRate = UARTBaudRate_115200;
				break;
			default:
				newBaudRate = 0;
				break;
		}

		UARTSettings* settings = uart2GetSettings();
		SemaphoreHandle_t* settingsSemaphore = uart1GetSettingsSemaphore();
		/* Try to take the settings semaphore */
		if (newBaudRate != 0 && *settingsSemaphore != 0 && xSemaphoreTake(*settingsSemaphore, 100) == pdTRUE)
		{
			settings->baudRate = newBaudRate;
			uart2UpdateWithNewSettings();

			/* Restart the channel if it was on */
			if (settings->connection == UARTConnection_Connected)
				uart2Restart();

			/* Give back the semaphore now that we are done */
			xSemaphoreGive(*settingsSemaphore);
		}

		/* Hide the pop out */
		GUI_HideContainer(guiConfigPOPOUT_UART2_BAUD_RATE_CONTAINER_ID);
		GUI_SetActiveLayer(GUILayer_0);
		GUI_SetLayerForButton(guiConfigUART2_BAUD_RATE_BUTTON_ID, GUILayer_0);
		GUI_SetButtonState(guiConfigUART2_BAUD_RATE_BUTTON_ID, GUIButtonState_Disabled);

		/* Refresh the main text box */
		prcActiveMainTextBoxManagerShouldRefresh = true;
	}
}

/**
 * @brief
 * @param	None
 * @retval	None
 */
static void prvInitUart2GuiElements()
{
	/* Text boxes ----------------------------------------------------------------*/
	/* UART2 Label text box */
	prvTextBox.object.id = guiConfigUART2_LABEL_TEXT_BOX_ID;
	prvTextBox.object.xPos = 650;
	prvTextBox.object.yPos = 50;
	prvTextBox.object.width = 150;
	prvTextBox.object.height = 50;
	prvTextBox.object.layer = GUILayer_0;
	prvTextBox.object.displayState = GUIDisplayState_Hidden;
	prvTextBox.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvTextBox.object.borderThickness = 1;
	prvTextBox.object.borderColor = GUI_WHITE;
	prvTextBox.textColor = GUI_YELLOW;
	prvTextBox.backgroundColor = GUI_WHITE;
	prvTextBox.staticText = "UART2";
	prvTextBox.textSize = LCDFontEnlarge_2x;
	GUI_AddTextBox(&prvTextBox);

	/* Buttons -------------------------------------------------------------------*/
	/* UART2 Top Button */
	prvButton.object.id = guiConfigUART2_TOP_BUTTON_ID;
	prvButton.object.xPos = 300;
	prvButton.object.yPos = 0;
	prvButton.object.width = 100;
	prvButton.object.height = 50;
	prvButton.object.layer = GUILayer_0;
	prvButton.object.displayState = GUIDisplayState_NotHidden;
	prvButton.object.border = GUIBorder_Bottom | GUIBorder_Right | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_YELLOW;
	prvButton.disabledTextColor = GUI_YELLOW;
	prvButton.disabledBackgroundColor = LCD_COLOR_BLACK;
	prvButton.pressedTextColor = GUI_YELLOW;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvUart2TopButtonCallback;
	prvButton.text[0] = "UART2";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* UART2 Enable Button */
	prvButton.object.id = guiConfigUART2_ENABLE_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_YELLOW;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_YELLOW;
	prvButton.pressedTextColor = GUI_YELLOW;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvUart2EnableButtonCallback;
	prvButton.text[0] = "Output:";
//	prvButton.text[1] = "Enabled ";
	prvButton.text[1] = "Disabled";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* UART2 Baud Rate Button */
	prvButton.object.id = guiConfigUART2_BAUD_RATE_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_DARK_YELLOW;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_YELLOW;
	prvButton.pressedTextColor = GUI_YELLOW;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvUart2BaudRateButtonCallback;
	prvButton.text[0] = "< Baud Rate:";
	prvButton.text[1] = "115200 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* UART2 Voltage Level Button */
	prvButton.object.id = guiConfigUART2_VOLTAGE_LEVEL_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_YELLOW;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_YELLOW;
	prvButton.pressedTextColor = GUI_YELLOW;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvUart2VoltageLevelButtonCallback;
	prvButton.text[0] = "Voltage Level:";
	prvButton.text[1] = " 5 V ";
//	prvButton.text[1] = "3.3 V";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* UART2 Format Button */
	prvButton.object.id = guiConfigUART2_FORMAT_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_YELLOW;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_YELLOW;
	prvButton.pressedTextColor = GUI_YELLOW;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvUart2FormatButtonCallback;
	prvButton.text[0] = "Display Format:";
	prvButton.text[1] = "ASCII";
//	prvButton.text[1] = " HEX ";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* UART2 Clear Button */
	prvButton.object.id = guiConfigUART2_CLEAR_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_YELLOW;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_YELLOW;
	prvButton.pressedTextColor = GUI_YELLOW;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvGenericUartClearButtonCallback;
	prvButton.text[0] = "Clear";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* UART2 Debug Button */
	prvButton.object.id = guiConfigUART2_DEBUG_BUTTON_ID;
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
	prvButton.touchCallback = prvUart2DebugButtonCallback;
	prvButton.text[0] = "Debug TX:";
	prvButton.text[1] = "Disabled";
//	prvButton.text[1] = "Enabled ";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* UART2 4800 bps Button */
	prvButton.object.id = guiConfigUART2_BAUD4800_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_YELLOW;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_YELLOW;
	prvButton.pressedTextColor = GUI_YELLOW;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvUart2BaudRateSelectionCallback;
	prvButton.text[0] = "4800 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* UART2 7200 bps Button */
	prvButton.object.id = guiConfigUART2_BAUD7200_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_YELLOW;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_YELLOW;
	prvButton.pressedTextColor = GUI_YELLOW;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvUart2BaudRateSelectionCallback;
	prvButton.text[0] = "7200 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* UART2 9600 bps Button */
	prvButton.object.id = guiConfigUART2_BAUD9600_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_YELLOW;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_YELLOW;
	prvButton.pressedTextColor = GUI_YELLOW;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvUart2BaudRateSelectionCallback;
	prvButton.text[0] = "9600 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* UART2 19200 bps Button */
	prvButton.object.id = guiConfigUART2_BAUD19K2_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_YELLOW;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_YELLOW;
	prvButton.pressedTextColor = GUI_YELLOW;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvUart2BaudRateSelectionCallback;
	prvButton.text[0] = "19200 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* UART2 28800 bps Button */
	prvButton.object.id = guiConfigUART2_BAUD28K8_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_YELLOW;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_YELLOW;
	prvButton.pressedTextColor = GUI_YELLOW;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvUart2BaudRateSelectionCallback;
	prvButton.text[0] = "28800 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* UART2 38400 bps Button */
	prvButton.object.id = guiConfigUART2_BAUD38K4_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_YELLOW;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_YELLOW;
	prvButton.pressedTextColor = GUI_YELLOW;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvUart2BaudRateSelectionCallback;
	prvButton.text[0] = "38400 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* UART2 57600 bps Button */
	prvButton.object.id = guiConfigUART2_BAUD57K6_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_YELLOW;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_YELLOW;
	prvButton.pressedTextColor = GUI_YELLOW;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvUart2BaudRateSelectionCallback;
	prvButton.text[0] = "57600 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* UART2 115200 bps Button */
	prvButton.object.id = guiConfigUART2_BAUD115K_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_YELLOW;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_YELLOW;
	prvButton.pressedTextColor = GUI_YELLOW;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvUart2BaudRateSelectionCallback;
	prvButton.text[0] = "115200 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* Containers ----------------------------------------------------------------*/
	/* Sidebar UART2 container */
	prvContainer.object.id = guiConfigSIDEBAR_UART2_CONTAINER_ID;
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
	prvContainer.buttons[0] = GUI_GetButtonFromId(guiConfigUART2_ENABLE_BUTTON_ID);
	prvContainer.buttons[1] = GUI_GetButtonFromId(guiConfigUART2_BAUD_RATE_BUTTON_ID);
	prvContainer.buttons[2] = GUI_GetButtonFromId(guiConfigUART2_VOLTAGE_LEVEL_BUTTON_ID);
	prvContainer.buttons[3] = GUI_GetButtonFromId(guiConfigUART2_FORMAT_BUTTON_ID);
	prvContainer.buttons[4] = GUI_GetButtonFromId(guiConfigUART2_CLEAR_BUTTON_ID);
	prvContainer.buttons[5] = GUI_GetButtonFromId(guiConfigUART2_DEBUG_BUTTON_ID);
	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(guiConfigUART2_LABEL_TEXT_BOX_ID);
	GUI_AddContainer(&prvContainer);

	/* UART2 baud rate popout container */
	prvContainer.object.id = guiConfigPOPOUT_UART2_BAUD_RATE_CONTAINER_ID;
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
	prvContainer.buttons[0] = GUI_GetButtonFromId(guiConfigUART2_BAUD4800_BUTTON_ID);
	prvContainer.buttons[1] = GUI_GetButtonFromId(guiConfigUART2_BAUD7200_BUTTON_ID);
	prvContainer.buttons[2] = GUI_GetButtonFromId(guiConfigUART2_BAUD9600_BUTTON_ID);
	prvContainer.buttons[3] = GUI_GetButtonFromId(guiConfigUART2_BAUD19K2_BUTTON_ID);
	prvContainer.buttons[4] = GUI_GetButtonFromId(guiConfigUART2_BAUD28K8_BUTTON_ID);
	prvContainer.buttons[5] = GUI_GetButtonFromId(guiConfigUART2_BAUD38K4_BUTTON_ID);
	prvContainer.buttons[6] = GUI_GetButtonFromId(guiConfigUART2_BAUD57K6_BUTTON_ID);
	prvContainer.buttons[7] = GUI_GetButtonFromId(guiConfigUART2_BAUD115K_BUTTON_ID);
	GUI_AddContainer(&prvContainer);
}

/* RS232 GUI Elements ========================================================*/
/**
 * @brief	Manages how data is displayed in the main text box when the source is RS232
 * @param	None
 * @retval	None
 */
static void prvManageRs232MainTextBox()
{
	const uint32_t constStartFlashAddress = FLASH_ADR_RS232_DATA;

	/* Get the current write address, this is the address where the last data is */
	uint32_t currentWriteAddress = rs232GetCurrentWriteAddress();
	/* Get the current settings of the channel */
	UARTSettings* settings = rs232GetSettings();
	SemaphoreHandle_t* settingsSemaphore = uart1GetSettingsSemaphore();

	prvManageGenericUartMainTextBox(constStartFlashAddress, currentWriteAddress, settings, settingsSemaphore);
}

/**
 * @brief	Callback for the enable button
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
static void prvRs232EnableButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	static bool enabled = false;

	if (Event == GUITouchEvent_Up)
	{
		if (enabled)
		{
			ErrorStatus status = rs232SetConnection(UARTConnection_Disconnected);
			if (status == SUCCESS)
			{
				enabled = false;
				GUI_SetButtonTextForRow(guiConfigRS232_ENABLE_BUTTON_ID, "Disabled", 1);
				GUI_SetButtonState(guiConfigRS232_TOP_BUTTON_ID, GUIButtonState_Disabled);

				GUI_SetButtonTextForRow(guiConfigRS232_BAUD_RATE_BUTTON_ID, "< Baud Rate:", 0);
				GUI_SetButtonState(guiConfigRS232_BAUD_RATE_BUTTON_ID, GUIButtonState_Disabled);
			}
		}
		else
		{
			ErrorStatus status = rs232SetConnection(UARTConnection_Connected);
			if (status == SUCCESS)
			{
				enabled = true;
				GUI_SetButtonTextForRow(guiConfigRS232_ENABLE_BUTTON_ID, "Enabled ", 1);
				GUI_SetButtonState(guiConfigRS232_TOP_BUTTON_ID, GUIButtonState_Enabled);

				GUI_SetButtonTextForRow(guiConfigRS232_BAUD_RATE_BUTTON_ID, "  Baud Rate:", 0);
				GUI_SetButtonState(guiConfigRS232_BAUD_RATE_BUTTON_ID, GUIButtonState_DisabledTouch);
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
static void prvRs232FormatButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		UARTSettings* settings = rs232GetSettings();
		SemaphoreHandle_t* settingsSemaphore = uart1GetSettingsSemaphore();
		/* Try to take the settings semaphore */
		if (*settingsSemaphore != 0 && xSemaphoreTake(*settingsSemaphore, 100) == pdTRUE)
		{
			if (settings->writeFormat == GUIWriteFormat_ASCII)
			{
				settings->writeFormat = GUIWriteFormat_Hex;
				settings->numOfCharactersPerByte = 3;
				GUI_SetButtonTextForRow(guiConfigRS232_FORMAT_BUTTON_ID, " Hex ", 1);
			}
			else if (settings->writeFormat == GUIWriteFormat_Hex)
			{
				settings->writeFormat = GUIWriteFormat_ASCII;
				settings->numOfCharactersPerByte = 1;
				GUI_SetButtonTextForRow(guiConfigRS232_FORMAT_BUTTON_ID, "ASCII", 1);
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
static void prvRs232DebugButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	static bool enabled = false;
	static UARTMode lastMode;

	if (Event == GUITouchEvent_Up)
	{
		UARTSettings* settings = rs232GetSettings();
		SemaphoreHandle_t* settingsSemaphore = uart1GetSettingsSemaphore();
		/* Try to take the settings semaphore */
		if (*settingsSemaphore != 0 && xSemaphoreTake(*settingsSemaphore, 100) == pdTRUE)
		{
			if (enabled)
			{
				settings->mode = lastMode;
				enabled = false;
				GUI_SetButtonTextForRow(guiConfigRS232_DEBUG_BUTTON_ID, "Disabled", 1);
			}
			else
			{
				lastMode = settings->mode;
				settings->mode = UARTMode_DebugTX;
				enabled = true;
				GUI_SetButtonTextForRow(guiConfigRS232_DEBUG_BUTTON_ID, "Enabled ", 1);
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
static void prvRs232TopButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		/* Get the current display state of the sidebar */
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(guiConfigSIDEBAR_RS232_CONTAINER_ID);

		if (displayState == GUIDisplayState_Hidden)
		{
			/* Update the baud rate text to match what is actually set */
			UARTSettings* settings = rs232GetSettings();
			switch (settings->baudRate)
			{
				case UARTBaudRate_4800:
					GUI_SetButtonTextForRow(guiConfigRS232_BAUD_RATE_BUTTON_ID, "  4800 bps", 1);
					break;
				case UARTBaudRate_7200:
					GUI_SetButtonTextForRow(guiConfigRS232_BAUD_RATE_BUTTON_ID, "  7200 bps", 1);
					break;
				case UARTBaudRate_9600:
					GUI_SetButtonTextForRow(guiConfigRS232_BAUD_RATE_BUTTON_ID, "  9600 bps", 1);
					break;
				case UARTBaudRate_19200:
					GUI_SetButtonTextForRow(guiConfigRS232_BAUD_RATE_BUTTON_ID, " 19200 bps", 1);
					break;
				case UARTBaudRate_28800:
					GUI_SetButtonTextForRow(guiConfigRS232_BAUD_RATE_BUTTON_ID, " 28800 bps", 1);
					break;
				case UARTBaudRate_38400:
					GUI_SetButtonTextForRow(guiConfigRS232_BAUD_RATE_BUTTON_ID, " 38400 bps", 1);
					break;
				case UARTBaudRate_57600:
					GUI_SetButtonTextForRow(guiConfigRS232_BAUD_RATE_BUTTON_ID, " 57600 bps", 1);
					break;
				case UARTBaudRate_115200:
					GUI_SetButtonTextForRow(guiConfigRS232_BAUD_RATE_BUTTON_ID, "115200 bps", 1);
					break;
				default:
					break;
			}
			/* Update the write format text to match what is actually set */
			switch (settings->writeFormat)
			{
				case GUIWriteFormat_ASCII:
					GUI_SetButtonTextForRow(guiConfigRS232_FORMAT_BUTTON_ID, " Hex ", 1);
					break;
				case GUIWriteFormat_Hex:
					GUI_SetButtonTextForRow(guiConfigRS232_FORMAT_BUTTON_ID, "ASCII", 1);
					break;
				default:
					break;
			}
		}
		/* Change the state of the sidebar */
		prvChangeDisplayStateOfSidebar(guiConfigSIDEBAR_RS232_CONTAINER_ID);
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
static void prvRs232BaudRateButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(guiConfigPOPOUT_RS232_BAUD_RATE_CONTAINER_ID);

		if (displayState == GUIDisplayState_Hidden)
		{
			GUI_SetActiveLayer(GUILayer_1);
			GUI_SetLayerForButton(guiConfigRS232_BAUD_RATE_BUTTON_ID, GUILayer_1);
			GUI_SetButtonState(guiConfigRS232_BAUD_RATE_BUTTON_ID, GUIButtonState_Enabled);
			GUI_DrawContainer(guiConfigPOPOUT_RS232_BAUD_RATE_CONTAINER_ID);
		}
		else if (displayState == GUIDisplayState_NotHidden)
		{
			GUI_HideContainer(guiConfigPOPOUT_RS232_BAUD_RATE_CONTAINER_ID);
			GUI_SetActiveLayer(GUILayer_0);
			GUI_SetLayerForButton(guiConfigRS232_BAUD_RATE_BUTTON_ID, GUILayer_0);
			GUI_SetButtonState(guiConfigRS232_BAUD_RATE_BUTTON_ID, GUIButtonState_Disabled);

			/* Refresh the main text box */
			prcActiveMainTextBoxManagerShouldRefresh = true;
		}
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
static void prvRs232BaudRateSelectionCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		UARTBaudRate newBaudRate;
		switch (ButtonId)
		{
			case guiConfigRS232_BAUD4800_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigRS232_BAUD_RATE_BUTTON_ID, "  4800 bps", 1);
				newBaudRate = UARTBaudRate_4800;
				break;
			case guiConfigRS232_BAUD7200_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigRS232_BAUD_RATE_BUTTON_ID, "  7200 bps", 1);
				newBaudRate = UARTBaudRate_7200;
				break;
			case guiConfigRS232_BAUD9600_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigRS232_BAUD_RATE_BUTTON_ID, "  9600 bps", 1);
				newBaudRate = UARTBaudRate_9600;
				break;
			case guiConfigRS232_BAUD19K2_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigRS232_BAUD_RATE_BUTTON_ID, " 19200 bps", 1);
				newBaudRate = UARTBaudRate_19200;
				break;
			case guiConfigRS232_BAUD28K8_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigRS232_BAUD_RATE_BUTTON_ID, " 28800 bps", 1);
				newBaudRate = UARTBaudRate_28800;
				break;
			case guiConfigRS232_BAUD38K4_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigRS232_BAUD_RATE_BUTTON_ID, " 38400 bps", 1);
				newBaudRate = UARTBaudRate_38400;
				break;
			case guiConfigRS232_BAUD57K6_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigRS232_BAUD_RATE_BUTTON_ID, " 57600 bps", 1);
				newBaudRate = UARTBaudRate_57600;
				break;
			case guiConfigRS232_BAUD115K_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigRS232_BAUD_RATE_BUTTON_ID, "115200 bps", 1);
				newBaudRate = UARTBaudRate_115200;
				break;
			default:
				newBaudRate = 0;
				break;
		}

		UARTSettings* settings = rs232GetSettings();
		SemaphoreHandle_t* settingsSemaphore = uart1GetSettingsSemaphore();
		/* Try to take the settings semaphore */
		if (newBaudRate != 0 && *settingsSemaphore != 0 && xSemaphoreTake(*settingsSemaphore, 100) == pdTRUE)
		{
			settings->baudRate = newBaudRate;
			rs232UpdateWithNewSettings();
			/* Give back the semaphore now that we are done */
			xSemaphoreGive(*settingsSemaphore);

			/* Restart the channel as we made some changes */
			rs232Restart();
		}

		/* Hide the pop out */
		GUI_HideContainer(guiConfigPOPOUT_RS232_BAUD_RATE_CONTAINER_ID);
		GUI_SetActiveLayer(GUILayer_0);
		GUI_SetLayerForButton(guiConfigRS232_BAUD_RATE_BUTTON_ID, GUILayer_0);
		GUI_SetButtonState(guiConfigRS232_BAUD_RATE_BUTTON_ID, GUIButtonState_Disabled);

		/* Refresh the main text box */
		prcActiveMainTextBoxManagerShouldRefresh = true;
	}
}

/**
 * @brief
 * @param	None
 * @retval	None
 */
static void prvInitRs232GuiElements()
{
	/* Text boxes ----------------------------------------------------------------*/
	/* RS232 Label text box */
	prvTextBox.object.id = guiConfigRS232_LABEL_TEXT_BOX_ID;
	prvTextBox.object.xPos = 650;
	prvTextBox.object.yPos = 50;
	prvTextBox.object.width = 150;
	prvTextBox.object.height = 50;
	prvTextBox.object.layer = GUILayer_0;
	prvTextBox.object.displayState = GUIDisplayState_Hidden;
	prvTextBox.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvTextBox.object.borderThickness = 1;
	prvTextBox.object.borderColor = GUI_WHITE;
	prvTextBox.textColor = GUI_PURPLE;
	prvTextBox.backgroundColor = GUI_WHITE;
	prvTextBox.staticText = "RS232";
	prvTextBox.textSize = LCDFontEnlarge_2x;
	GUI_AddTextBox(&prvTextBox);

	/* Buttons -------------------------------------------------------------------*/
	/* RS232 Button */
	prvButton.object.id = guiConfigRS232_TOP_BUTTON_ID;
	prvButton.object.xPos = 400;
	prvButton.object.yPos = 0;
	prvButton.object.width = 100;
	prvButton.object.height = 50;
	prvButton.object.layer = GUILayer_0;
	prvButton.object.displayState = GUIDisplayState_NotHidden;
	prvButton.object.border = GUIBorder_Bottom | GUIBorder_Right | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_PURPLE;
	prvButton.disabledBackgroundColor = LCD_COLOR_BLACK;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvRs232TopButtonCallback;
	prvButton.text[0] = "RS232";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* RS232 Enable Button */
	prvButton.object.id = guiConfigRS232_ENABLE_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvRs232EnableButtonCallback;
	prvButton.text[0] = "Output:";
//	prvButton.text[1] = "Enabled ";
	prvButton.text[1] = "Disabled";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* RS232 Baud Rate Button */
	prvButton.object.id = guiConfigRS232_BAUD_RATE_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_DARK_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvRs232BaudRateButtonCallback;
	prvButton.text[0] = "< Baud Rate:";
	prvButton.text[1] = "115200 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* RS232 Format Button */
	prvButton.object.id = guiConfigRS232_FORMAT_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvRs232FormatButtonCallback;
	prvButton.text[0] = "Display Format:";
	prvButton.text[1] = "ASCII";
//	prvButton.text[1] = " HEX ";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* RS232 Clear Button */
	prvButton.object.id = guiConfigRS232_CLEAR_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvGenericUartClearButtonCallback;
	prvButton.text[0] = "Clear";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* RS232 Debug Button */
	prvButton.object.id = guiConfigRS232_DEBUG_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_RED;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_RED;
	prvButton.pressedTextColor = GUI_RED;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvRs232DebugButtonCallback;
	prvButton.text[0] = "Debug TX:";
	prvButton.text[1] = "Disabled";
//	prvButton.text[1] = "Enabled ";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* RS232 4800 bps Button */
	prvButton.object.id = guiConfigRS232_BAUD4800_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvRs232BaudRateSelectionCallback;
	prvButton.text[0] = "4800 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* RS232 7200 bps Button */
	prvButton.object.id = guiConfigRS232_BAUD7200_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvRs232BaudRateSelectionCallback;
	prvButton.text[0] = "7200 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* RS232 9600 bps Button */
	prvButton.object.id = guiConfigRS232_BAUD9600_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvRs232BaudRateSelectionCallback;
	prvButton.text[0] = "9600 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* RS232 19200 bps Button */
	prvButton.object.id = guiConfigRS232_BAUD19K2_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvRs232BaudRateSelectionCallback;
	prvButton.text[0] = "19200 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* RS232 28800 bps Button */
	prvButton.object.id = guiConfigRS232_BAUD28K8_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvRs232BaudRateSelectionCallback;
	prvButton.text[0] = "28800 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* RS232 38400 bps Button */
	prvButton.object.id = guiConfigRS232_BAUD38K4_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvRs232BaudRateSelectionCallback;
	prvButton.text[0] = "38400 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* RS232 57600 bps Button */
	prvButton.object.id = guiConfigRS232_BAUD57K6_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvRs232BaudRateSelectionCallback;
	prvButton.text[0] = "57600 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* RS232 115200 bps Button */
	prvButton.object.id = guiConfigRS232_BAUD115K_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvRs232BaudRateSelectionCallback;
	prvButton.text[0] = "115200 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* Containers ----------------------------------------------------------------*/
	/* Sidebar RS232 container */
	prvContainer.object.id = guiConfigSIDEBAR_RS232_CONTAINER_ID;
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
	prvContainer.buttons[0] = GUI_GetButtonFromId(guiConfigRS232_ENABLE_BUTTON_ID);
	prvContainer.buttons[1] = GUI_GetButtonFromId(guiConfigRS232_BAUD_RATE_BUTTON_ID);
	prvContainer.buttons[2] = GUI_GetButtonFromId(guiConfigRS232_FORMAT_BUTTON_ID);
	prvContainer.buttons[3] = GUI_GetButtonFromId(guiConfigRS232_CLEAR_BUTTON_ID);
	prvContainer.buttons[4] = GUI_GetButtonFromId(guiConfigRS232_DEBUG_BUTTON_ID);
	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(guiConfigRS232_LABEL_TEXT_BOX_ID);
	GUI_AddContainer(&prvContainer);

	/* RS232 baud rate popout container */
	prvContainer.object.id = guiConfigPOPOUT_RS232_BAUD_RATE_CONTAINER_ID;
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
	prvContainer.buttons[0] = GUI_GetButtonFromId(guiConfigRS232_BAUD4800_BUTTON_ID);
	prvContainer.buttons[1] = GUI_GetButtonFromId(guiConfigRS232_BAUD7200_BUTTON_ID);
	prvContainer.buttons[2] = GUI_GetButtonFromId(guiConfigRS232_BAUD9600_BUTTON_ID);
	prvContainer.buttons[3] = GUI_GetButtonFromId(guiConfigRS232_BAUD19K2_BUTTON_ID);
	prvContainer.buttons[4] = GUI_GetButtonFromId(guiConfigRS232_BAUD28K8_BUTTON_ID);
	prvContainer.buttons[5] = GUI_GetButtonFromId(guiConfigRS232_BAUD38K4_BUTTON_ID);
	prvContainer.buttons[6] = GUI_GetButtonFromId(guiConfigRS232_BAUD57K6_BUTTON_ID);
	prvContainer.buttons[7] = GUI_GetButtonFromId(guiConfigRS232_BAUD115K_BUTTON_ID);
	GUI_AddContainer(&prvContainer);
}

/* GPIO GUI Elements ========================================================*/
/**
 * @brief	Manages how data is displayed in the main text box when the source is GPIO
 * @param	None
 * @retval	None
 */
static void prvManageGpioMainTextBox()
{

}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
static void prvGpioTopButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(guiConfigSIDEBAR_GPIO_CONTAINER_ID);
		prvChangeDisplayStateOfSidebar(guiConfigSIDEBAR_GPIO_CONTAINER_ID);
	}
}

/**
 * @brief
 * @param	None
 * @retval	None
 */
static void prvInitGpioGuiElements()
{
	/* Text boxes ----------------------------------------------------------------*/
	/* GPIO Label text box */
	prvTextBox.object.id = guiConfigGPIO_LABEL_TEXT_BOX_ID;
	prvTextBox.object.xPos = 650;
	prvTextBox.object.yPos = 50;
	prvTextBox.object.width = 150;
	prvTextBox.object.height = 50;
	prvTextBox.object.layer = GUILayer_0;
	prvTextBox.object.displayState = GUIDisplayState_Hidden;
	prvTextBox.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvTextBox.object.borderThickness = 1;
	prvTextBox.object.borderColor = GUI_WHITE;
	prvTextBox.textColor = GUI_CYAN_LIGHT;
	prvTextBox.backgroundColor = GUI_WHITE;
	prvTextBox.staticText = "GPIO";
	prvTextBox.textSize = LCDFontEnlarge_2x;
	GUI_AddTextBox(&prvTextBox);

	/* Buttons -------------------------------------------------------------------*/
	/* GPIO Top Button */
	prvButton.object.id = guiConfigGPIO_TOP_BUTTON_ID;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 0;
	prvButton.object.width = 100;
	prvButton.object.height = 50;
	prvButton.object.layer = GUILayer_0;
	prvButton.object.displayState = GUIDisplayState_NotHidden;
	prvButton.object.border = GUIBorder_Bottom | GUIBorder_Right | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_CYAN_LIGHT;
	prvButton.disabledTextColor = GUI_CYAN_LIGHT;
	prvButton.disabledBackgroundColor = LCD_COLOR_BLACK;
	prvButton.pressedTextColor = GUI_CYAN_LIGHT;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvGpioTopButtonCallback;
	prvButton.text[0] = "GPIO";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* GPIO0 Type Button */
	prvButton.object.id = guiConfigGPIO0_TYPE_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_CYAN_LIGHT;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_CYAN_LIGHT;
	prvButton.pressedTextColor = GUI_CYAN_LIGHT;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = 0;
	prvButton.text[0] = "Ch0 Type:";
	prvButton.text[1] = "Output";
//	prvButton.text[1] = "Input ";
//	prvButton.text[1] = "PWM   ";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* GPIO0 Settings Button */
	prvButton.object.id = guiConfigGPIO0_SETTINGS_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_CYAN_LIGHT;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_CYAN_LIGHT;
	prvButton.pressedTextColor = GUI_CYAN_LIGHT;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = 0;
	prvButton.text[0] = "Ch0 Settings:";
	prvButton.text[1] = "25% 30kHz";
//	prvButton.text[1] = "HIGH ";
//	prvButton.text[1] = "LOW   ";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* GPIO1 Type Button */
	prvButton.object.id = guiConfigGPIO1_TYPE_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_CYAN_DARK;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_CYAN_DARK;
	prvButton.pressedTextColor = GUI_CYAN_DARK;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = 0;
	prvButton.text[0] = "Ch1 Type:";
	prvButton.text[1] = "Output";
//	prvButton.text[1] = "Input ";
//	prvButton.text[1] = "PWM   ";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* GPIO1 Settings Button */
	prvButton.object.id = guiConfigGPIO1_SETTINGS_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_CYAN_DARK;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_CYAN_DARK;
	prvButton.pressedTextColor = GUI_CYAN_DARK;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = 0;
	prvButton.text[0] = "Ch1 Settings:";
	prvButton.text[1] = "25% 30kHz";
//	prvButton.text[1] = "HIGH ";
//	prvButton.text[1] = "LOW   ";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* Containers ----------------------------------------------------------------*/
	/* Sidebar GPIO container */
	prvContainer.object.id = guiConfigSIDEBAR_GPIO_CONTAINER_ID;
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
	prvContainer.buttons[0] = GUI_GetButtonFromId(guiConfigGPIO0_TYPE_BUTTON_ID);
	prvContainer.buttons[1] = GUI_GetButtonFromId(guiConfigGPIO0_SETTINGS_BUTTON_ID);
	prvContainer.buttons[2] = GUI_GetButtonFromId(guiConfigGPIO1_TYPE_BUTTON_ID);
	prvContainer.buttons[3] = GUI_GetButtonFromId(guiConfigGPIO1_SETTINGS_BUTTON_ID);
	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(guiConfigGPIO_LABEL_TEXT_BOX_ID);
	GUI_AddContainer(&prvContainer);
}

/* GPIO GUI Elements ========================================================*/
/**
 * @brief	Manages how data is displayed in the main text box when the source is ADC
 * @param	None
 * @retval	None
 */
static void prvManageAdcMainTextBox()
{

}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
static void prvAdcTopButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(guiConfigSIDEBAR_ADC_CONTAINER_ID);
		prvChangeDisplayStateOfSidebar(guiConfigSIDEBAR_ADC_CONTAINER_ID);
	}
}

/**
 * @brief
 * @param	None
 * @retval	None
 */
static void prvInitAdcGuiElements()
{
	/* Text boxes ----------------------------------------------------------------*/
	/* ADC Label text box */
	prvTextBox.object.id = guiConfigADC_LABEL_TEXT_BOX_ID;
	prvTextBox.object.xPos = 650;
	prvTextBox.object.yPos = 50;
	prvTextBox.object.width = 150;
	prvTextBox.object.height = 50;
	prvTextBox.object.layer = GUILayer_0;
	prvTextBox.object.displayState = GUIDisplayState_Hidden;
	prvTextBox.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvTextBox.object.borderThickness = 1;
	prvTextBox.object.borderColor = GUI_WHITE;
	prvTextBox.textColor = GUI_MAGENTA;
	prvTextBox.backgroundColor = GUI_WHITE;
	prvTextBox.staticText = "ADC";
	prvTextBox.textSize = LCDFontEnlarge_2x;
	GUI_AddTextBox(&prvTextBox);

	/* Buttons -------------------------------------------------------------------*/
	/* ADC Top Button */
	prvButton.object.id = guiConfigADC_TOP_BUTTON_ID;
	prvButton.object.xPos = 600;
	prvButton.object.yPos = 0;
	prvButton.object.width = 50;
	prvButton.object.height = 50;
	prvButton.object.layer = GUILayer_0;
	prvButton.object.displayState = GUIDisplayState_NotHidden;
	prvButton.object.border = GUIBorder_Bottom | GUIBorder_Right | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_MAGENTA;
	prvButton.disabledTextColor = GUI_MAGENTA;
	prvButton.disabledBackgroundColor = LCD_COLOR_BLACK;
	prvButton.pressedTextColor = GUI_MAGENTA;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvAdcTopButtonCallback;
	prvButton.text[0] = "ADC";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* ADC Enable Button */
	prvButton.object.id = guiConfigADC_ENABLE_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_MAGENTA;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_MAGENTA;
	prvButton.pressedTextColor = GUI_MAGENTA;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = 0;
	prvButton.text[0] = "Output:";
//	prvButton.text[1] = "Enabled ";
	prvButton.text[1] = "Disabled";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* Containers ----------------------------------------------------------------*/
	/* Sidebar ADC container */
	prvContainer.object.id = guiConfigSIDEBAR_ADC_CONTAINER_ID;
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
	prvContainer.buttons[0] = GUI_GetButtonFromId(guiConfigADC_ENABLE_BUTTON_ID);
	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(guiConfigADC_LABEL_TEXT_BOX_ID);
	GUI_AddContainer(&prvContainer);
}

/* Interrupt Handlers --------------------------------------------------------*/
