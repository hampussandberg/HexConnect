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
/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static GUITextBox prvTextBox = {0};
static GUIButton prvButton = {0};
static GUIContainer prvContainer = {0};
static uint32_t prvIdOfLastActiveSidebar = guiConfigINVALID_ID;
static uint32_t prvIdOfActiveSidebar = GUIContainerId_SidebarEmpty;
static bool prvDebugConsoleIsHidden = false;
static float prvTemperature = 0.0;

static int32_t prvMainContainerYPosOffset = 0;
static bool prvActiveChannelHasChanged = false;
static bool prvForceRefresh = false;

static xTimerHandle prvMainTextBoxRefreshTimer;

/* Private function prototypes -----------------------------------------------*/
static void prvMainContainerRefreshTimerCallback();
static void prvManageEmptyMainTextBox(bool ShouldRefresh);

static void prvHardwareInit();
static void prvMainContentContainerCallback(GUITouchEvent Event, uint16_t XPos, uint16_t YPos);
static bool prvAllChanneAreDoneInitializing();
static void prvInitGuiElements();
static void prvSplashScreen();

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

	/* Display splash screen */
	prvSplashScreen();

	/* Make sure all channels have started up before we initialize the GUI */
	while (!prvAllChanneAreDoneInitializing())
	{
		vTaskDelay(1 / portTICK_PERIOD_MS);
	}

	/* Initialize the GUI elements */
	prvInitGuiElements();

	xLCDEventQueue = xQueueCreate(10, sizeof(LCDEventMessage));
	if (xLCDEventQueue == 0)
	{
		// Queue was not created and must not be used.
	}

	prvMainTextBoxRefreshTimer = xTimerCreate("MainTextBoxTimer", 10 / portTICK_PERIOD_MS, pdTRUE, 0, prvMainContainerRefreshTimerCallback);
	if (prvMainTextBoxRefreshTimer != NULL)
		xTimerStart(prvMainTextBoxRefreshTimer, portMAX_DELAY);

	LCDEventMessage receivedMessage;

//	GUITextBox_WriteString(GUITextBoxId_Clock, "14:15:12");

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
			/* Item successfully removed from the queue */
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
							GUITextBox_SetWritePosition(GUITextBoxId_Debug, 5, 5);
							GUITextBox_Clear(GUITextBoxId_Debug);
							GUITextBox_WriteString(GUITextBoxId_Debug, "X:");
							GUITextBox_WriteNumber(GUITextBoxId_Debug, receivedMessage.data[0]);
							GUITextBox_WriteString(GUITextBoxId_Debug, ", Y:");
							GUITextBox_WriteNumber(GUITextBoxId_Debug, receivedMessage.data[1]);
							GUITextBox_WriteString(GUITextBoxId_Debug, ", EVENT:");
							GUITextBox_WriteNumber(GUITextBoxId_Debug, receivedMessage.data[2]);
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
						GUIButton_CheckAllActiveForTouchEventAt(touchEvent, receivedMessage.data[0], receivedMessage.data[1]);
						/* Check all text boxes */
						GUITextBox_CheckAllActiveForTouchEventAt(touchEvent, receivedMessage.data[0], receivedMessage.data[1]);
						/* Check all containers */
						GUIContainer_CheckAllActiveForTouchEventAt(touchEvent, receivedMessage.data[0], receivedMessage.data[1]);
					}
					break;

				/* New temperature data received */
				case LCDEvent_TemperatureData:
					memcpy(&prvTemperature, receivedMessage.data, sizeof(float));
					int8_t currentTemp = (int8_t)prvTemperature - 11;
					GUITextBox_Draw(GUITextBoxId_Temperature);
					GUITextBox_SetWritePosition(GUITextBoxId_Temperature, 50, 3);
					GUITextBox_WriteNumber(GUITextBoxId_Temperature, (int32_t)currentTemp);
					GUITextBox_WriteString(GUITextBoxId_Temperature, " C");
					break;

				/* Debug message received */
				case LCDEvent_DebugMessage:
					GUITextBox_SetWritePosition(GUITextBoxId_Debug, 5, 5);
					GUITextBox_Clear(GUITextBoxId_Debug);
					GUITextBox_WriteNumber(GUITextBoxId_Debug, receivedMessage.data[0]);
					GUITextBox_WriteString(GUITextBoxId_Debug, " - ");
					GUITextBox_WriteString(GUITextBoxId_Debug, (uint8_t*)receivedMessage.data[1]);
					break;


				default:
					break;
			}
		}
		else
		{
			/* Timeout has occured i.e. no message available */
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
	/* TODO: BUG, something goes wrong if you clear while a lot of data is being received at the same time */

	if (Event == GUITouchEvent_Up)
	{
		bool channelWasReset = false;
		switch (ButtonId)
		{
			case GUIButtonId_Uart1Clear:
				uart1Clear();
				GUITextBox_ClearDisplayedDataInBuffer(GUITextBoxId_Uart1Main);
				GUITextBox_SetAddressesTo(GUITextBoxId_Uart1Main, FLASH_ADR_UART1_DATA);
				channelWasReset = true;
				break;
			case GUIButtonId_Uart2Clear:
				uart2Clear();
				GUITextBox_ClearDisplayedDataInBuffer(GUITextBoxId_Uart2Main);
				GUITextBox_SetAddressesTo(GUITextBoxId_Uart2Main, FLASH_ADR_UART2_DATA);
				channelWasReset = true;
				break;
			case GUIButtonId_Rs232Clear:
				rs232Clear();
				GUITextBox_ClearDisplayedDataInBuffer(GUITextBoxId_Rs232Main);
				GUITextBox_SetAddressesTo(GUITextBoxId_Rs232Main, FLASH_ADR_RS232_DATA);
				channelWasReset = true;
				break;
			default:
				break;
		}

//		/* If a channel was reset we should save the settings so that the correct addresses are saved */
//		if (channelWasReset)
//		{
//			guiSaveSettingsButtonCallback(GUITouchEvent_Up, ButtonId);
//		}
	}
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
void lcdManageGenericUartMainTextBox(const uint32_t constStartFlashAddress, uint32_t currentWriteAddress, UARTSettings* pSettings,
									 SemaphoreHandle_t* pSemaphore, uint32_t TextBoxId, bool ShouldRefresh)
{
	/* Try to take the settings semaphore */
	if (*pSemaphore != 0 && xSemaphoreTake(*pSemaphore, 100) == pdTRUE)
	{
		/* The text box should refresh the data that is displayed */
		if (ShouldRefresh)
		{
			/* Update the text format for the text box */
			GUITextBox_ChangeTextFormat(TextBoxId, pSettings->textFormat, GUITextFormatChangeStyle_LockEnd);
			GUITextBox_RefreshCurrentDataFromMemory(TextBoxId);
		}

		uint32_t readEndAddress = GUITextBox_GetReadEndAddress(TextBoxId);
		/* New data has been written that we have not displayed yet */
		if (readEndAddress != 0 && readEndAddress < currentWriteAddress)
		{
			/* If we are not scrolling we should append this new data to the end of the displayed data */
			if (!GUITextBox_IsScrolling(TextBoxId))
			{
				GUITextBox_AppendDataFromMemory(TextBoxId, currentWriteAddress);
			}
			/* If we are scrolling just update the last valid address of the text box */
			else
			{
				GUITextBox_SetLastValidByteAddress(TextBoxId, currentWriteAddress);
			}
		}

		/* Get how many rows the offset equals */
		int32_t rowDiff = prvMainContainerYPosOffset / 16;

		/* Manage offset caused by scrolling */
		if (prvMainContainerYPosOffset != 0 && rowDiff != 0)
		{
			GUITextBox_MoveDisplayedDataNumOfRows(TextBoxId, rowDiff);

			/* Set it to 0 now that we have managed it */
			prvMainContainerYPosOffset = 0;
		}

#if 0
		/* DEBUG */
		GUITextBox_SetWritePosition(GUITextBoxId_Debug, 5, 5);
		GUITextBox_Clear(GUITextBoxId_Debug);
		GUITextBox_WriteString(GUITextBoxId_Debug, "Data Count: ");
		GUITextBox_WriteNumber(GUITextBoxId_Debug, currentWriteAddress-constStartFlashAddress);
		GUITextBox_WriteString(GUITextBoxId_Debug, ", numChar: ");
		GUITextBox_WriteNumber(GUITextBoxId_Debug, numOfCharactersDisplayed);
#endif

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
	GUIDisplayState displayState = GUIContainer_GetDisplayState(SidebarId);
	if (displayState == GUIDisplayState_NotHidden)
	{
		/* Hide this sidebar and display the last active instead */
		GUIContainer_Hide(SidebarId);
		GUIContainer_Draw(prvIdOfLastActiveSidebar);
		/* Set the last active as active now */
		prvIdOfActiveSidebar = prvIdOfLastActiveSidebar;
		prvIdOfLastActiveSidebar = GUIContainerId_SidebarEmpty;
	}
	else if (displayState == GUIDisplayState_Hidden)
	{
		/* Hide the active sidebar and display this sidebar instead */
		GUIContainer_Hide(prvIdOfActiveSidebar);
		GUIContainer_Draw(SidebarId);
		/* Save the old active sidebar and set this sidebar as the new active one */
		prvIdOfLastActiveSidebar = prvIdOfActiveSidebar;
		prvIdOfActiveSidebar = SidebarId;
	}

	if (prvIdOfActiveSidebar != GUIContainerId_SidebarSystem)
	{
		prvActiveChannelHasChanged = true;
	}
}


/**
 * @brief	Force the currently active window to refresh
 * @param	None
 * @retval	None
 */
void lcdForceRefreshOfActiveMainContent()
{
	prvForceRefresh = true;
}


/* Private functions .--------------------------------------------------------*/
/**
 * @brief	Callback function for the main text box refresh timer
 * @param	None
 * @retval	None
 */
static void prvMainContainerRefreshTimerCallback()
{
	/* Function pointer to the currently active managing function */
	static void (*activeManageFunction)(bool) = 0;

	bool shouldRefresh = false;

	/* Check if a new channel has been selected */
	if (prvActiveChannelHasChanged)
	{
		prvActiveChannelHasChanged = false;

		switch (prvIdOfActiveSidebar)
		{
			/* Empty */
			case GUIContainerId_SidebarEmpty:
				activeManageFunction = prvManageEmptyMainTextBox;
				GUIContainer_ChangePage(GUIContainerId_MainContent, guiConfigMAIN_CONTAINER_EMPTY_PAGE);
				break;
			/* CAN1 */
			case GUIContainerId_SidebarCan1:
				activeManageFunction = guiCan1ManageMainTextBox;
				GUIContainer_ChangePage(GUIContainerId_MainContent, guiConfigMAIN_CONTAINER_CAN1_PAGE);
				break;
			/* CAN2 */
			case GUIContainerId_SidebarCan2:
				activeManageFunction = guiCan2ManageMainTextBox;
				GUIContainer_ChangePage(GUIContainerId_MainContent, guiConfigMAIN_CONTAINER_CAN2_PAGE);
				break;
			/* UART1 */
			case GUIContainerId_SidebarUart1:
				activeManageFunction = guiUart1ManageMainTextBox;
				GUIContainer_ChangePage(GUIContainerId_MainContent, guiConfigMAIN_CONTAINER_UART1_PAGE);
				break;
			/* UART2 */
			case GUIContainerId_SidebarUart2:
				activeManageFunction = guiUart2ManageMainTextBox;
				GUIContainer_ChangePage(GUIContainerId_MainContent, guiConfigMAIN_CONTAINER_UART2_PAGE);
				break;
			/* RS232 */
			case GUIContainerId_SidebarRs232:
				activeManageFunction = guiRs232ManageMainTextBox;
				GUIContainer_ChangePage(GUIContainerId_MainContent, guiConfigMAIN_CONTAINER_RS232_PAGE);
				break;
			/* GPIO */
			case GUIContainerId_SidebarGpio:
				activeManageFunction = guiGpioManageMainTextBox;
				GUIContainer_ChangePage(GUIContainerId_MainContent, guiConfigMAIN_CONTAINER_GPIO_PAGE);
				break;
			/* ADC */
			case GUIContainerId_SidebarAdc:
				activeManageFunction = guiAdcManageMainTextBox;
				GUIContainer_ChangePage(GUIContainerId_MainContent, guiConfigMAIN_CONTAINER_ADC_PAGE);
				break;
			/* Should not happen */
			default:
				activeManageFunction = 0;
				GUIContainer_ChangePage(GUIContainerId_MainContent, guiConfigMAIN_CONTAINER_EMPTY_PAGE);
				break;

		}

		/* If we have changed the function we should refresh */
		shouldRefresh = true;
	}

	/* Check if we should force a refresh */
	if (prvForceRefresh)
	{
		shouldRefresh = true;
		prvForceRefresh = false;
	}

	/* Only call the managing function if it's set */
	if (activeManageFunction != 0)
	{
		activeManageFunction(shouldRefresh);
	}
}

/**
 * @brief	Manages how data is displayed in the main text box when the source is None
 * @param	None
 * @retval	None
 */
static void prvManageEmptyMainTextBox(bool ShouldRefresh)
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
	GUITextBox_SetWritePosition(GUITextBoxId_Debug, 5, 5);
	GUITextBox_Clear(GUITextBoxId_Debug);
	GUITextBox_WriteString(GUITextBoxId_Debug, "yDelta:");
	GUITextBox_WriteNumber(GUITextBoxId_Debug, yDelta);
	GUITextBox_SetWritePosition(GUITextBoxId_Debug, 200, 5);
	GUITextBox_WriteString(GUITextBoxId_Debug, "prvMainTextBoxYPosOffset:");
	GUITextBox_WriteNumber(GUITextBoxId_Debug, prvMainTextBoxYPosOffset);
#endif
}

/**
 * @brief	Checks if all channels are done with their initialization
 * @param	None
 * @retval	true if they are
 * @retval	false if they are not
 */
static bool prvAllChanneAreDoneInitializing()
{
	if (can1IsDoneInitializing() &&
		can2IsDoneInitializing() &&
		uart1IsDoneInitializing() &&
		uart2IsDoneInitializing() &&
		rs232IsDoneInitializing())
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
	GUITextBox_Add(&prvTextBox);

//	/* Clock Text Box */
//	prvTextBox.object.id = GUITextBoxId_Clock;
//	prvTextBox.object.xPos = 650;
//	prvTextBox.object.yPos = 0;
//	prvTextBox.object.width = 150;
//	prvTextBox.object.height = 25;
//	prvTextBox.textColor = GUI_WHITE;
//	prvTextBox.backgroundColor = LCD_COLOR_BLACK;
//	prvTextBox.textSize = LCDFontEnlarge_1x;
//	prvTextBox.xWritePos = 50;
//	prvTextBox.yWritePos = 3;
//	GUITextBox_Add(&prvTextBox);

	/* Serial Monitor Text Box */
	prvTextBox.object.id = GUITextBoxId_SerialMonitor;
	prvTextBox.object.xPos = 650;
	prvTextBox.object.yPos = 0;
	prvTextBox.object.width = 150;
	prvTextBox.object.height = 25;
	prvTextBox.textColor = GUI_WHITE;
	prvTextBox.backgroundColor = LCD_COLOR_BLACK;
	prvTextBox.textSize = LCDFontEnlarge_1x;
	prvTextBox.staticText = "Serial Monitor";
	GUITextBox_Add(&prvTextBox);

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
	GUITextBox_Add(&prvTextBox);

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
	prvContainer.containers[0] = GUIContainer_GetFromId(GUIContainerId_Can2MainContent);
	prvContainer.containers[1] = GUIContainer_GetFromId(GUIContainerId_Uart1MainContent);
	prvContainer.containers[2] = GUIContainer_GetFromId(GUIContainerId_Uart2MainContent);
	prvContainer.containers[3] = GUIContainer_GetFromId(GUIContainerId_Rs232MainContent);
	prvContainer.containers[4] = GUIContainer_GetFromId(GUIContainerId_Gpio0MainContent);
	prvContainer.containers[5] = GUIContainer_GetFromId(GUIContainerId_Gpio1MainContent);
	prvContainer.containers[6] = GUIContainer_GetFromId(GUIContainerId_AdcMainContent);
	prvContainer.touchCallback = prvMainContentContainerCallback;
	GUIContainer_Add(&prvContainer);

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
//	prvContainer.textBoxes[0] = GUITextBox_GetFromId(GUITextBoxId_Clock);
	prvContainer.textBoxes[0] = GUITextBox_GetFromId(GUITextBoxId_SerialMonitor);
	prvContainer.textBoxes[1] = GUITextBox_GetFromId(GUITextBoxId_Temperature);
	GUIContainer_Add(&prvContainer);

	/* Draw the main container */
	GUIContainer_Draw(GUIContainerId_MainContent);

	/* Draw the status container */
	GUIContainer_Draw(GUIContainerId_Status);

	/* Draw the empty sidebar container */
	GUIContainer_Draw(GUIContainerId_SidebarEmpty);
	prvIdOfActiveSidebar = prvIdOfLastActiveSidebar = GUIContainerId_SidebarEmpty;
}

/**
 * @brief	Displays the splash screen
 * @param	None
 * @retval	None
 */
static void prvSplashScreen()
{
	/* TODO: BUG? We need to clear the active window one time first for some reason */
	LCD_ClearActiveWindow(0, 0, 0, 0);

	/* Serial Monitor */
	uint8_t* title = "Serial Monitor";
	uint16_t xPos = guiConfigDISPLAY_WIDTH / 2 - guiConfigFONT_WIDTH_UNIT * strlen(title) * LCDFontEnlarge_4x / 2;
	uint16_t yPos = 150;
	LCD_SetTextWritePosition(xPos, yPos);
	LCD_SetForegroundColor(LCD_COLOR_WHITE);
	LCD_WriteString(title, LCDTransparency_Transparent, LCDFontEnlarge_4x);

	/* Hardware Revision */
	uint8_t* hardwareRev = "Hardware Revision: 1.0";
	xPos = guiConfigDISPLAY_WIDTH / 2 - guiConfigFONT_WIDTH_UNIT * strlen(hardwareRev) * LCDFontEnlarge_2x / 2;
	yPos += guiConfigFONT_HEIGHT_UNIT*LCDFontEnlarge_4x + 10;
	LCD_SetTextWritePosition(xPos, yPos);
	LCD_SetForegroundColor(LCD_COLOR_WHITE);
	LCD_WriteString(hardwareRev, LCDTransparency_Transparent, LCDFontEnlarge_2x);

	/* Software Version */
	uint8_t* softwareVersion = "Software Version: 0.1";
	xPos = guiConfigDISPLAY_WIDTH / 2 - guiConfigFONT_WIDTH_UNIT * strlen(softwareVersion) * LCDFontEnlarge_2x / 2;
	yPos += guiConfigFONT_HEIGHT_UNIT*LCDFontEnlarge_2x + 10;
	LCD_SetTextWritePosition(xPos, yPos);
	LCD_SetForegroundColor(LCD_COLOR_WHITE);
	LCD_WriteString(softwareVersion, LCDTransparency_Transparent, LCDFontEnlarge_2x);

	vTaskDelay(4000 / portTICK_PERIOD_MS);
}

/* Interrupt Handlers --------------------------------------------------------*/
