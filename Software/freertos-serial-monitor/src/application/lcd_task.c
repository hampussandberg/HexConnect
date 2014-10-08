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
#include "buzzer.h"

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
#include "gui_system.h"

/* Private defines -----------------------------------------------------------*/
#define FLASH_FETCH_BUFFER_SIZE		(64)

/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static GUITextBox prvTextBox = {0};
static GUIButton prvButton = {0};
static GUIContainer prvContainer = {0};
static uint32_t prvIdOfLastActiveSidebar = guiConfigINVALID_ID;
static uint32_t prvIdOfActiveSidebar = GUIContainerId_SidebarEmpty;
static bool prvDebugConsoleIsHidden = false;
float prvTemperature = 0.0;

static uint8_t prvFlashFetchBuffer[FLASH_FETCH_BUFFER_SIZE];
static int32_t prvMainContainerYPosOffset = 0;
static bool prvActiveChannelHasChanged = false;
static bool prcActiveMainTextBoxManagerShouldRefresh = false;

static xTimerHandle prvMainTextBoxRefreshTimer;

/* Private function prototypes -----------------------------------------------*/
static void prvDisplayDataInMainTextBoxWithId(uint32_t* pFromAddress, uint32_t ToAddress, GUIWriteFormat Format, uint32_t TextBoxId);
static void prvMainContainerRefreshTimerCallback();
static void prvManageEmptyMainTextBox();

static void prvHardwareInit();
static void prvMainContentContainerCallback(GUITouchEvent Event, uint16_t XPos, uint16_t YPos);
static void prvClearMainTextBoxWithId(uint32_t TextBoxId);
static bool prvAllChanneAreDoneInitializing();
static void prvInitGuiElements();

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

	prvMainTextBoxRefreshTimer = xTimerCreate("MainTextBoxTimer", 10 / portTICK_PERIOD_MS, 0, 0, prvMainContainerRefreshTimerCallback);
	if (prvMainTextBoxRefreshTimer != NULL)
		xTimerStart(prvMainTextBoxRefreshTimer, portMAX_DELAY);

	LCDEventMessage receivedMessage;

	GUI_WriteStringInTextBox(GUITextBoxId_Clock, "14:15:12");

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
						if (GUI_GetDisplayStateForTextBox(GUITextBoxId_Debug) == GUIDisplayState_NotHidden)
						{
							GUI_SetWritePosition(GUITextBoxId_Debug, 5, 5);
							GUI_ClearTextBox(GUITextBoxId_Debug);
							GUI_WriteStringInTextBox(GUITextBoxId_Debug, "X:");
							GUI_WriteNumberInTextBox(GUITextBoxId_Debug, receivedMessage.data[0]);
							GUI_WriteStringInTextBox(GUITextBoxId_Debug, ", Y:");
							GUI_WriteNumberInTextBox(GUITextBoxId_Debug, receivedMessage.data[1]);
							GUI_WriteStringInTextBox(GUITextBoxId_Debug, ", EVENT:");
							GUI_WriteNumberInTextBox(GUITextBoxId_Debug, receivedMessage.data[2]);
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
						/* Check all containers */
						GUI_CheckAllContainersForTouchEventAt(touchEvent, receivedMessage.data[0], receivedMessage.data[1]);
					}
					break;

				/* New temperature data received */
				case LCDEvent_TemperatureData:
					memcpy(&prvTemperature, receivedMessage.data, sizeof(float));
					int8_t currentTemp = (int8_t)prvTemperature;
					GUI_DrawTextBox(GUITextBoxId_Temperature);
					GUI_SetWritePosition(GUITextBoxId_Temperature, 50, 3);
					GUI_WriteNumberInTextBox(GUITextBoxId_Temperature, (int32_t)currentTemp);
					GUI_WriteStringInTextBox(GUITextBoxId_Temperature, " C");
					break;

				case LCDEvent_MainBoxText:
					text[0] = (uint8_t)receivedMessage.data[0];
					GUI_WriteStringInTextBox(GUITextBoxId_Main, text);
					break;

				case LCDEvent_DebugMessage:
					GUI_SetWritePosition(GUITextBoxId_Debug, 5, 5);
					GUI_ClearTextBox(GUITextBoxId_Debug);
					GUI_WriteNumberInTextBox(GUITextBoxId_Debug, receivedMessage.data[0]);
					GUI_WriteStringInTextBox(GUITextBoxId_Debug, " - ");
					GUI_WriteStringInTextBox(GUITextBoxId_Debug, (uint8_t*)receivedMessage.data[1]);
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
			case GUIContainerId_SidebarUart1:
				uart1Clear();
				prvClearMainTextBoxWithId(GUITextBoxId_Uart1Main);
				channelWasReset = true;
				break;
			case GUIContainerId_SidebarUart2:
				uart2Clear();
				prvClearMainTextBoxWithId(GUITextBoxId_Uart2Main);
				channelWasReset = true;
				break;
			case GUIContainerId_SidebarRs232:
				rs232Clear();
				prvClearMainTextBoxWithId(GUITextBoxId_Rs232Main);
				channelWasReset = true;
				break;
			default:
				break;
		}

		/* If a channel was reset we should save the settings so that the correct addresses are saved */
		if (channelWasReset)
		{
			guiSaveSettingsButtonCallback(GUITouchEvent_Up, ButtonId);
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
 * @param	TextBoxId: ID for the text box which should be used
 * @retval	None
 */
void lcdManageGenericUartMainTextBox(const uint32_t constStartFlashAddress, uint32_t currentWriteAddress,
									 UARTSettings* pSettings, SemaphoreHandle_t* pSemaphore, uint32_t TextBoxId)
{
	/* Try to take the settings semaphore */
	if (*pSemaphore != 0 && xSemaphoreTake(*pSemaphore, 100) == pdTRUE)
	{
		/* Get how many rows the offset equals */
		int32_t rowDiff = prvMainContainerYPosOffset / 16;

		/* If we should refresh we set the current read address to the display start address */
		if (prcActiveMainTextBoxManagerShouldRefresh)
		{
			prvClearMainTextBoxWithId(TextBoxId);
			prcActiveMainTextBoxManagerShouldRefresh = false;

			/*
			 * If we are not scrolling it means we should have the newest data on the bottom of the page
			 * therefore we set the end address to where the newest data is and the start address
			 * numOfCharactersDisplayed before that. This should avoid the problem of loading all new data
			 * if the page has not been displayed for a while.
			 */
			if (!pSettings->scrolling)
			{
				uint32_t numOfCharactersToDisplay = pSettings->numOfCharactersDisplayed;
				uint32_t amountOfDataSaved = pSettings->amountOfDataSaved;
				/*
				 * If we are not scrolling and the amount of data is larger then the number of characters displayed
				 * it means there's new data we haven't shown yet.
				 */
				if (amountOfDataSaved > numOfCharactersToDisplay)
					numOfCharactersToDisplay = amountOfDataSaved;

				/* Make sure we only display as many character as it can fit on the screen */
				if (numOfCharactersToDisplay*pSettings->numOfCharactersPerByte > GUI_MAIN_MAX_NUM_OF_CHARACTERS)
					numOfCharactersToDisplay = GUI_MAIN_MAX_NUM_OF_CHARACTERS / pSettings->numOfCharactersPerByte;

				/* Set the end and start address */
				pSettings->displayedDataEndAddress = currentWriteAddress;
				pSettings->displayedDataStartAddress = pSettings->displayedDataEndAddress - numOfCharactersToDisplay;
			}

			pSettings->readAddress = pSettings->displayedDataStartAddress;
			while (pSettings->readAddress != pSettings->displayedDataEndAddress)
			{
				prvDisplayDataInMainTextBoxWithId(&pSettings->readAddress, pSettings->displayedDataEndAddress,
												  pSettings->writeFormat, TextBoxId);
			}
		}

		/* Manage offset caused by scrolling */
		if (prvMainContainerYPosOffset != 0 && rowDiff != 0)
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
				prvClearMainTextBoxWithId(TextBoxId);

				while (pSettings->readAddress != pSettings->displayedDataEndAddress)
				{
					prvDisplayDataInMainTextBoxWithId(&pSettings->readAddress, pSettings->displayedDataEndAddress,
													  pSettings->writeFormat, TextBoxId);
				}
			}

			/* Set it to 0 now that we have managed it */
			prvMainContainerYPosOffset = 0;
		}

		/*
		 * Check that we are not scrolling, if we do we don't want to show new data.
		 * We also check that the current read address is less then current write address which means there's new
		 * data we haven't written yet.
		 */
		if (!pSettings->scrolling && pSettings->readAddress < currentWriteAddress)
		{
			/* Display data in the main text box */
			prvDisplayDataInMainTextBoxWithId(&pSettings->readAddress, currentWriteAddress,
											  pSettings->writeFormat, TextBoxId);

			/* The end of the displayed data will be where we last read */
			pSettings->displayedDataEndAddress = pSettings->readAddress;

			/* Check if we are near the bottom */
			uint16_t xWritePos, yWritePos;
			GUI_GetWritePosition(TextBoxId, &xWritePos, &yWritePos);
			uint32_t currentRow = yWritePos / 16;
			if (currentRow == GUI_MAIN_MAX_ROW_CHARACTERS - 1)
			{
				/* Get the start address */
				pSettings->displayedDataStartAddress += GUI_MAIN_MAX_COLUMN_CHARACTERS / pSettings->numOfCharactersPerByte;
				/* Set the read address to the beginning of the start address so
				 * that it will start reading from there the next time */
				pSettings->readAddress = pSettings->displayedDataStartAddress;

				/* Clear the main text box */
				prvClearMainTextBoxWithId(TextBoxId);
				/* Update the screen with the old data we still want to see */
				while (pSettings->readAddress != pSettings->displayedDataEndAddress)
				{
					prvDisplayDataInMainTextBoxWithId(&pSettings->readAddress, pSettings->displayedDataEndAddress,
													  pSettings->writeFormat, TextBoxId);
				}

				/* The end of the displayed data will be where we last read */
				pSettings->displayedDataEndAddress = pSettings->readAddress;
			}

			/* Save how many characters are displayed on the screen */
			pSettings->numOfCharactersDisplayed = pSettings->displayedDataEndAddress - pSettings->displayedDataStartAddress;

#if 0
			/* DEBUG */
			GUI_SetWritePosition(GUITextBoxId_Debug, 5, 5);
			GUI_ClearTextBox(GUITextBoxId_Debug);
			GUI_WriteStringInTextBox(GUITextBoxId_Debug, "Data Count: ");
			GUI_WriteNumberInTextBox(GUITextBoxId_Debug, currentWriteAddress-constStartFlashAddress);
			GUI_WriteStringInTextBox(GUITextBoxId_Debug, ", numChar: ");
			GUI_WriteNumberInTextBox(GUITextBoxId_Debug, numOfCharactersDisplayed);
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
		prvIdOfLastActiveSidebar = GUIContainerId_SidebarEmpty;
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

	if (prvIdOfActiveSidebar != GUIContainerId_SidebarSystem)
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
 * @param	TextBoxId: ID of the text box to display data in
 * @retval	None
 * @note	Will only write FLASH_FETCH_BUFFER_SIZE amount of bytes. If the difference
 * 			between the to and from address is larger you must loop this function until
 * 			the two addresses are the same.
 * @time	~410 us when numOfBytesToFetch = 64
 */
static void prvDisplayDataInMainTextBoxWithId(uint32_t* pFromAddress, uint32_t ToAddress, GUIWriteFormat Format, uint32_t TextBoxId)
{
	uint32_t numOfBytesToFetch = ToAddress - *pFromAddress;
	if (numOfBytesToFetch > FLASH_FETCH_BUFFER_SIZE)
		numOfBytesToFetch = FLASH_FETCH_BUFFER_SIZE;
	SPI_FLASH_ReadBufferDMA(prvFlashFetchBuffer, *pFromAddress, numOfBytesToFetch);
	/* Make sure we only update the from address if we successfully could write to the screen */
	if (GUI_WriteBufferInTextBox(TextBoxId, prvFlashFetchBuffer, numOfBytesToFetch, Format) != ERROR)
		*pFromAddress += numOfBytesToFetch;
}

/**
 * @brief	Callback function for the main text box refresh timer
 * @param	None
 * @retval	None
 */
static void prvMainContainerRefreshTimerCallback()
{
	/* Function pointer to the currently active managing function */
	static void (*activeManageFunction)() = 0;

	/* Check if a new channel has been selected */
	if (prvActiveChannelHasChanged)
	{
		prvActiveChannelHasChanged = false;

		switch (prvIdOfActiveSidebar)
		{
			/* Empty */
			case GUIContainerId_SidebarEmpty:
				activeManageFunction = prvManageEmptyMainTextBox;
				GUI_ChangePageOfContainer(GUIContainerId_MainContent, guiConfigMAIN_CONTAINER_EMPTY_PAGE);
				break;
			/* CAN1 */
			case GUIContainerId_SidebarCan1:
				activeManageFunction = guiCan1ManageMainTextBox;
				GUI_ChangePageOfContainer(GUIContainerId_MainContent, guiConfigMAIN_CONTAINER_CAN1_PAGE);
				break;
			/* CAN2 */
			case GUIContainerId_SidebarCan2:
				activeManageFunction = guiCan2ManageMainTextBox;
				GUI_ChangePageOfContainer(GUIContainerId_MainContent, guiConfigMAIN_CONTAINER_CAN2_PAGE);
				break;
			/* UART1 */
			case GUIContainerId_SidebarUart1:
				activeManageFunction = guiUart1ManageMainTextBox;
				GUI_ChangePageOfContainer(GUIContainerId_MainContent, guiConfigMAIN_CONTAINER_UART1_PAGE);
				break;
			/* UART2 */
			case GUIContainerId_SidebarUart2:
				activeManageFunction = guiUart2ManageMainTextBox;
				GUI_ChangePageOfContainer(GUIContainerId_MainContent, guiConfigMAIN_CONTAINER_UART2_PAGE);
				break;
			/* RS232 */
			case GUIContainerId_SidebarRs232:
				activeManageFunction = guiRs232ManageMainTextBox;
				GUI_ChangePageOfContainer(GUIContainerId_MainContent, guiConfigMAIN_CONTAINER_RS232_PAGE);
				break;
			/* GPIO */
			case GUIContainerId_SidebarGpio:
				activeManageFunction = guiGpioManageMainTextBox;
				GUI_ChangePageOfContainer(GUIContainerId_MainContent, guiConfigMAIN_CONTAINER_GPIO_PAGE);
				break;
			/* ADC */
			case GUIContainerId_SidebarAdc:
				activeManageFunction = guiAdcManageMainTextBox;
				GUI_ChangePageOfContainer(GUIContainerId_MainContent, guiConfigMAIN_CONTAINER_ADC_PAGE);
				break;
			/* Should not happen */
			default:
				activeManageFunction = 0;
				GUI_ChangePageOfContainer(GUIContainerId_MainContent, guiConfigMAIN_CONTAINER_EMPTY_PAGE);
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

	/* Buzzer init */
	BUZZER_Init();

	/* Beep once */
	BUZZER_BeepNumOfTimes(1);
}

/**
 * @brief	Callback for the main content container
 * @param	Event: The event that caused the callback
 * @param	XPos: The X coordinate for the event
 * @param	YPos: The Y coordinate for the event
 * @retval	None
 */
static void prvMainContentContainerCallback(GUITouchEvent Event, uint16_t XPos, uint16_t YPos)
{
	static int32_t yDelta = 0;
	static uint32_t lastYValue = 0;
	static GUITouchEvent lastEvent = GUITouchEvent_Up;

	if (Event == GUITouchEvent_Up)
	{
		/* Update the delta one last time */
		yDelta =  YPos - lastYValue;
		prvMainContainerYPosOffset += yDelta;

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
			prvMainContainerYPosOffset += yDelta;
			lastYValue = YPos;
		}

		lastEvent = GUITouchEvent_Down;
	}

#if 0
	/* DEBUG */
	GUI_SetWritePosition(GUITextBoxId_Debug, 5, 5);
	GUI_ClearTextBox(GUITextBoxId_Debug);
	GUI_WriteStringInTextBox(GUITextBoxId_Debug, "yDelta:");
	GUI_WriteNumberInTextBox(GUITextBoxId_Debug, yDelta);
	GUI_SetWritePosition(GUITextBoxId_Debug, 200, 5);
	GUI_WriteStringInTextBox(GUITextBoxId_Debug, "prvMainTextBoxYPosOffset:");
	GUI_WriteNumberInTextBox(GUITextBoxId_Debug, prvMainTextBoxYPosOffset);
#endif
}

/**
 * @brief	Clears a text box
 * @param	TextBoxId: Id of the text box to clear
 * @retval	None
 */
static void prvClearMainTextBoxWithId(uint32_t TextBoxId)
{
	/* Clear the specified text box */
	GUI_SetWritePosition(TextBoxId, 0, 0);
	GUI_ClearTextBox(TextBoxId);
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
	guiSystemInitGuiElements();


	/* Text boxes ----------------------------------------------------------------*/
	/* Main text box */
	prvTextBox.object.id = GUITextBoxId_Main;
	prvTextBox.object.xPos = 0;
	prvTextBox.object.yPos = 50;
	prvTextBox.object.width = 650;
	prvTextBox.object.height = 400;
	prvTextBox.textColor = GUI_WHITE;
	prvTextBox.backgroundColor = LCD_COLOR_BLACK;
	prvTextBox.textSize = LCDFontEnlarge_1x;
	prvTextBox.xWritePos = 0;
	prvTextBox.yWritePos = 0;
	GUI_AddTextBox(&prvTextBox);

	/* Clock Text Box */
	prvTextBox.object.id = GUITextBoxId_Clock;
	prvTextBox.object.xPos = 650;
	prvTextBox.object.yPos = 0;
	prvTextBox.object.width = 150;
	prvTextBox.object.height = 25;
	prvTextBox.textColor = GUI_WHITE;
	prvTextBox.backgroundColor = LCD_COLOR_BLACK;
	prvTextBox.textSize = LCDFontEnlarge_1x;
	prvTextBox.xWritePos = 50;
	prvTextBox.yWritePos = 3;
	GUI_AddTextBox(&prvTextBox);

	/* Temperature Text Box */
	prvTextBox.object.id = GUITextBoxId_Temperature;
	prvTextBox.object.xPos = 651;
	prvTextBox.object.yPos = 25;
	prvTextBox.object.width = 149;
	prvTextBox.object.height = 24;
	prvTextBox.textColor = GUI_WHITE;
	prvTextBox.backgroundColor = LCD_COLOR_BLACK;
	prvTextBox.textSize = LCDFontEnlarge_1x;
	prvTextBox.xWritePos = 50;
	prvTextBox.yWritePos = 3;
	GUI_AddTextBox(&prvTextBox);

	/* Buttons -------------------------------------------------------------------*/

	/* Containers ----------------------------------------------------------------*/
	/* Main content container */
	prvContainer.object.id = GUIContainerId_MainContent;
	prvContainer.object.xPos = 0;
	prvContainer.object.yPos = 50;
	prvContainer.object.width = 650;
	prvContainer.object.height = 400;
	prvContainer.object.border = GUIBorder_Right | GUIBorder_Top;
	prvContainer.object.borderThickness = 1;
	prvContainer.object.borderColor = GUI_WHITE;
	prvContainer.contentHideState = GUIHideState_KeepBorders;
	prvContainer.activePage = guiConfigMAIN_CONTAINER_EMPTY_PAGE;
	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(GUITextBoxId_Uart1Main);
	prvContainer.textBoxes[1] = GUI_GetTextBoxFromId(GUITextBoxId_Uart2Main);
	prvContainer.textBoxes[2] = GUI_GetTextBoxFromId(GUITextBoxId_Rs232Main);
	prvContainer.containers[0] = GUI_GetContainerFromId(GUIContainerId_Can2MainContent);
	prvContainer.containers[1] = GUI_GetContainerFromId(GUIContainerId_Gpio0MainContent);
	prvContainer.containers[2] = GUI_GetContainerFromId(GUIContainerId_Gpio1MainContent);
	prvContainer.containers[3] = GUI_GetContainerFromId(GUIContainerId_AdcMainContent);
	prvContainer.touchCallback = prvMainContentContainerCallback;
	GUI_AddContainer(&prvContainer);

	/* Status info container */
	prvContainer.object.id = GUIContainerId_Status;
	prvContainer.object.xPos = 650;
	prvContainer.object.yPos = 0;
	prvContainer.object.width = 150;
	prvContainer.object.height = 50;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 1;
	prvContainer.object.borderColor = GUI_WHITE;
	prvContainer.contentHideState = GUIHideState_KeepBorders;
//	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(GUITextBoxId_Clock);
	prvContainer.textBoxes[1] = GUI_GetTextBoxFromId(GUITextBoxId_Temperature);
	GUI_AddContainer(&prvContainer);

	/* Draw the main container */
	GUI_DrawContainer(GUIContainerId_MainContent);

	/* Draw the status container */
	GUI_DrawContainer(GUIContainerId_Status);

	/* Draw the empty sidebar container */
	GUI_DrawContainer(GUIContainerId_SidebarEmpty);
	prvIdOfActiveSidebar = prvIdOfLastActiveSidebar = GUIContainerId_SidebarEmpty;
}

/* Interrupt Handlers --------------------------------------------------------*/
