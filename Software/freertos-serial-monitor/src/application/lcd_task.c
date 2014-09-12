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

#include "can1_task.h"
#include "can2_task.h"
#include "uart1_task.h"
#include "uart2_task.h"
#include "rs232_task.h"
#include "gpio0_task.h"
#include "gpio1_task.h"

/* Private defines -----------------------------------------------------------*/
#define GUI_BLUE		0x237F
#define GUI_RED			0xF926
#define GUI_GREEN		0x362A
#define GUI_YELLOW		0xFEE6
#define GUI_PURPLE		0xA8D6
#define GUI_GRAY		0xB596
#define GUI_MAGENTA		0xF81F
#define GUI_CYAN_LIGHT	0x1F3C
#define GUI_CYAN_DARK	0x45F7
#define GUI_DARK_BLUE	0x11CE

/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
GUITextBox prvTextBox = {0};
GUIButton prvButton = {0};
GUIContainer prvContainer = {0};
uint32_t prvIdOfLastActiveSidebar = guiConfigINVALID_ID;
uint32_t prvIdOfActiveSidebar = guiConfigSIDEBAR_EMPTY_CONTAINER_ID;
static bool prvDebugConsoleIsHidden = false;
uint32_t prvTempUpdateCounter = 1000;
float prvTemperature = 0.0;

static uint8_t prvTestBuffer[1024];

/* Private function prototypes -----------------------------------------------*/
static void prvHardwareInit();
static void prvInitGuiElements();

static void prvChangeDisplayStateOfSidebar(uint32_t SidebarId);

static void prvDebugToggleCallback(GUITouchEvent Event);
static void prvSystemButtonCallback(GUITouchEvent Event);
static void prvInitSystemGuiElements();

/* CAN1 */
static void prvCan1EnableButtonCallback(GUITouchEvent Event);
static void prvCan1TerminationButtonCallback(GUITouchEvent Event);
static void prvCan1TopButtonCallback(GUITouchEvent Event);
static void prvInitCan1GuiElements();

/* CAN2 */
static void prvCan2EnableButtonCallback(GUITouchEvent Event);
static void prvCan2TerminationButtonCallback(GUITouchEvent Event);
static void prvCan2TopButtonCallback(GUITouchEvent Event);
static void prvInitCan2GuiElements();

/* UART1 */
static void prvUart1EnableButtonCallback(GUITouchEvent Event);
static void prvUart1VoltageLevelButtonCallback(GUITouchEvent Event);
static void prvUart1DebugButtonCallback(GUITouchEvent Event);
static void prvUart1TopButtonCallback(GUITouchEvent Event);
static void prvInitUart1GuiElements();

/* UART2 */
static void prvUart2EnableButtonCallback(GUITouchEvent Event);
static void prvUart2VoltageLevelButtonCallback(GUITouchEvent Event);
static void prvUart2DebugButtonCallback(GUITouchEvent Event);
static void prvUart2TopButtonCallback(GUITouchEvent Event);
static void prvInitUart2GuiElements();

/* RS232 */
static void prvRs232EnableButtonCallback(GUITouchEvent Event);
static void prvRs232TopButtonCallback(GUITouchEvent Event);
static void prvInitRs232GuiElements();

/* GPIO */
static void prvGpioTopButtonCallback(GUITouchEvent Event);
static void prvInitGpioGuiElements();

/* ADC */
static void prvAdcTopButtonCallback(GUITouchEvent Event);
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
						if (GUI_GetDisplayStateForTextBox(guiConfigDEBUG_TEXT_BOX_ID) == GUIDisplayState_NotHidden)
						{
							GUI_ClearTextBox(guiConfigDEBUG_TEXT_BOX_ID);
							GUI_SetWritePosition(guiConfigDEBUG_TEXT_BOX_ID, 5, 5);
							GUI_WriteStringInTextBox(guiConfigDEBUG_TEXT_BOX_ID, "X:");
							GUI_WriteNumberInTextBox(guiConfigDEBUG_TEXT_BOX_ID, receivedMessage.data[0]);
							GUI_WriteStringInTextBox(guiConfigDEBUG_TEXT_BOX_ID, ", Y:");
							GUI_WriteNumberInTextBox(guiConfigDEBUG_TEXT_BOX_ID, receivedMessage.data[1]);
							GUI_WriteStringInTextBox(guiConfigDEBUG_TEXT_BOX_ID, ", EVENT:");
							GUI_WriteNumberInTextBox(guiConfigDEBUG_TEXT_BOX_ID, receivedMessage.data[2]);
						}


						/* Draw a dot on debug */
						if (!prvDebugConsoleIsHidden)
						{
							LCD_SetForegroundColor(LCD_COLOR_GREEN);
							LCD_DrawCircle(receivedMessage.data[0], receivedMessage.data[1], 2, 1);
						}

						GUITouchEvent touchEvent;
						if (receivedMessage.data[2] == FT5206Event_PutUp)
							touchEvent = GUITouchEvent_Up;
						else if (receivedMessage.data[2] == FT5206Event_PutDown)
							touchEvent = GUITouchEvent_Down;
						GUI_CheckAllActiveButtonsForTouchEventAt(touchEvent, receivedMessage.data[0], receivedMessage.data[1]);
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

			static uint32_t uart1Counter = 0;
			static uint32_t readAddress = FLASH_ADR_UART1_DATA;
			uart1Counter += 50;
			if (uart1Counter >= 100)
			{
				uint32_t currentWriteAddress = uart1GetCurrentWriteAddress();
				if (readAddress != currentWriteAddress)
				{
					SPI_FLASH_ReadBuffer(prvTestBuffer, readAddress, currentWriteAddress-readAddress);
					GUI_WriteBufferInTextBox(guiConfigMAIN_TEXT_BOX_ID, prvTestBuffer, currentWriteAddress-readAddress);
					readAddress = currentWriteAddress;
				}
				uart1Counter = 0;
			}
		}
	}
}


/* Private functions .--------------------------------------------------------*/
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
	prvTextBox.object.height = 400;
	prvTextBox.object.layer = GUILayer_0;
	prvTextBox.object.displayState = GUIDisplayState_NotHidden;
	prvTextBox.object.border = GUIBorder_Top | GUIBorder_Right;
	prvTextBox.object.borderThickness = 1;
	prvTextBox.object.borderColor = LCD_COLOR_WHITE;
	prvTextBox.textColor = LCD_COLOR_WHITE;
	prvTextBox.backgroundColor = LCD_COLOR_BLACK;
	prvTextBox.textSize = LCDFontEnlarge_1x;
	prvTextBox.xWritePos = 0;
	prvTextBox.yWritePos = 0;
	GUI_AddTextBox(&prvTextBox);

	/* Clock Text Box */
	prvTextBox.object.id = guiConfigCLOCK_TEXT_BOX_ID;
	prvTextBox.object.xPos = 650;
	prvTextBox.object.yPos = 0;
	prvTextBox.object.width = 150;
	prvTextBox.object.height = 25;
	prvTextBox.object.layer = GUILayer_0;
	prvTextBox.object.displayState = GUIDisplayState_NotHidden;
	prvTextBox.object.border = GUIBorder_NoBorder;
	prvTextBox.object.borderThickness = 0;
	prvTextBox.object.borderColor = LCD_COLOR_WHITE;
	prvTextBox.textColor = LCD_COLOR_WHITE;
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
	prvTextBox.object.borderColor = LCD_COLOR_WHITE;
	prvTextBox.textColor = LCD_COLOR_WHITE;
	prvTextBox.backgroundColor = LCD_COLOR_BLACK;
	prvTextBox.textSize = LCDFontEnlarge_1x;
	prvTextBox.xWritePos = 50;
	prvTextBox.yWritePos = 3;
	GUI_AddTextBox(&prvTextBox);

	/* Debug Text Box */
	prvTextBox.object.id = guiConfigDEBUG_TEXT_BOX_ID;
	prvTextBox.object.xPos = 0;
	prvTextBox.object.yPos = 450;
	prvTextBox.object.width = 649;
	prvTextBox.object.height = 30;
	prvTextBox.object.layer = GUILayer_0;
	prvTextBox.object.displayState = GUIDisplayState_NotHidden;
	prvTextBox.object.border = GUIBorder_Top;
	prvTextBox.object.borderThickness = 1;
	prvTextBox.object.borderColor = LCD_COLOR_WHITE;
	prvTextBox.textColor = LCD_COLOR_WHITE;
	prvTextBox.backgroundColor = GUI_RED;
	prvTextBox.textSize = LCDFontEnlarge_1x;
	prvTextBox.xWritePos = 0;
	prvTextBox.yWritePos = 0;
	GUI_AddTextBox(&prvTextBox);

	GUI_DrawTextBox(guiConfigMAIN_TEXT_BOX_ID);

	GUI_WriteStringInTextBox(guiConfigMAIN_TEXT_BOX_ID, "Hello World!");

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
	prvContainer.object.borderColor = LCD_COLOR_WHITE;
	prvContainer.contentHideState = GUIHideState_KeepBorders;
	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(guiConfigCLOCK_TEXT_BOX_ID);
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
}

static void prvClearMainTextBox(GUITouchEvent Event)
{
	if (Event == GUITouchEvent_Up)
	{
		GUI_ClearTextBox(guiConfigMAIN_TEXT_BOX_ID);
		GUI_SetWritePosition(guiConfigMAIN_TEXT_BOX_ID, 0, 0);
	}
}

/* System GUI Elements =======================================================*/
/**
 * @brief	Callback for the debug button, will toggle the debug textbox on and off
 * @param	Event: The event that caused the callback
 * @retval	None
 */
static void prvDebugToggleCallback(GUITouchEvent Event)
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
 * @retval	None
 */
static void prvSystemButtonCallback(GUITouchEvent Event)
{
	if (Event == GUITouchEvent_Up)
	{
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(guiConfigSIDEBAR_SYSTEM_CONTAINER_ID);
		prvChangeDisplayStateOfSidebar(guiConfigSIDEBAR_SYSTEM_CONTAINER_ID);
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
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_DARK_BLUE;
	prvButton.disabledTextColor = LCD_COLOR_WHITE;
	prvButton.disabledBackgroundColor = GUI_DARK_BLUE;
	prvButton.pressedTextColor = GUI_DARK_BLUE;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
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
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_DARK_BLUE;
	prvButton.disabledTextColor = LCD_COLOR_WHITE;
	prvButton.disabledBackgroundColor = GUI_DARK_BLUE;
	prvButton.pressedTextColor = GUI_DARK_BLUE;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
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
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_RED;
	prvButton.disabledTextColor = LCD_COLOR_WHITE;
	prvButton.disabledBackgroundColor = GUI_RED;
	prvButton.pressedTextColor = GUI_RED;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvDebugToggleCallback;
	prvButton.text[0] = "Debug";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* Clear Main TextBox Button */
	prvButton.object.id = guiConfigCLEAR_BUTTON_ID;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 200;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.layer = GUILayer_0;
	prvButton.object.displayState = GUIDisplayState_Hidden;
	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_DARK_BLUE;
	prvButton.disabledTextColor = LCD_COLOR_WHITE;
	prvButton.disabledBackgroundColor = GUI_DARK_BLUE;
	prvButton.pressedTextColor = GUI_DARK_BLUE;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvClearMainTextBox;
	prvButton.text[0] = "Clear";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* System Button */
	prvButton.object.id = guiConfigSYSTEM_BUTTON_ID;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 430;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.layer = GUILayer_0;
	prvButton.object.displayState = GUIDisplayState_NotHidden;
	prvButton.object.border = GUIBorder_Top | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_DARK_BLUE;
	prvButton.disabledTextColor = LCD_COLOR_WHITE;
	prvButton.disabledBackgroundColor = GUI_DARK_BLUE;
	prvButton.pressedTextColor = GUI_DARK_BLUE;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvSystemButtonCallback;
	prvButton.text[0] = "System";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* Containers ----------------------------------------------------------------*/
	/* Debug container */
	prvContainer.object.id = guiConfigDEBUG_CONTAINER_ID;
	prvContainer.object.xPos = 0;
	prvContainer.object.yPos = 450;
	prvContainer.object.width = 650;
	prvContainer.object.height = 30;
	prvContainer.object.layer = GUILayer_0;
	prvContainer.object.displayState = GUIDisplayState_Hidden;
	prvContainer.object.border = GUIBorder_Right;
	prvContainer.object.borderThickness = 1;
	prvContainer.object.borderColor = LCD_COLOR_WHITE;
	prvContainer.contentHideState = GUIHideState_KeepBorders;
	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(guiConfigDEBUG_TEXT_BOX_ID);
	GUI_AddContainer(&prvContainer);

	/* Side system container */
	prvContainer.object.id = guiConfigSIDEBAR_SYSTEM_CONTAINER_ID;
	prvContainer.object.xPos = 650;
	prvContainer.object.yPos = 50;
	prvContainer.object.width = 150;
	prvContainer.object.height = 380;
	prvContainer.object.layer = GUILayer_0;
	prvContainer.object.displayState = GUIDisplayState_Hidden;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 1;
	prvContainer.object.borderColor = LCD_COLOR_WHITE;
	prvContainer.contentHideState = GUIHideState_KeepBorders;
	prvContainer.buttons[0] = GUI_GetButtonFromId(guiConfigSETTINGS_BUTTON_ID);
	prvContainer.buttons[1] = GUI_GetButtonFromId(guiConfigSTORAGE_BUTTON_ID);
	prvContainer.buttons[2] = GUI_GetButtonFromId(guiConfigDEBUG_BUTTON_ID);
	prvContainer.buttons[3] = GUI_GetButtonFromId(guiConfigCLEAR_BUTTON_ID);
	GUI_AddContainer(&prvContainer);

	/* Side empty container */
	prvContainer.object.id = guiConfigSIDEBAR_EMPTY_CONTAINER_ID;
	prvContainer.object.xPos = 650;
	prvContainer.object.yPos = 50;
	prvContainer.object.width = 150;
	prvContainer.object.height = 380;
	prvContainer.object.layer = GUILayer_0;
	prvContainer.object.displayState = GUIDisplayState_Hidden;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 1;
	prvContainer.object.borderColor = LCD_COLOR_WHITE;
	prvContainer.contentHideState = GUIHideState_KeepBorders;
	GUI_AddContainer(&prvContainer);
}

/* CAN1 GUI Elements ========================================================*/
/**
 * @brief	Callback for the enable button
 * @param	Event: The event that caused the callback
 * @retval	None
 */
static void prvCan1EnableButtonCallback(GUITouchEvent Event)
{
	static bool enabled = false;

	if (Event == GUITouchEvent_Up)
	{
		if (enabled)
		{
			ErrorStatus status = can1SetConnection(CAN1Connection_Disconnected);
			if (status == SUCCESS)
			{
				enabled = false;
				GUI_SetButtonTextForRow(guiConfigCAN1_ENABLE_BUTTON_ID, "Disabled", 1);
				GUI_SetButtonState(guiConfigCAN1_TOP_BUTTON_ID, GUIButtonState_Disabled);
			}
		}
		else
		{
			ErrorStatus status = can1SetConnection(CAN1Connection_Connected);
			if (status == SUCCESS)
			{
				enabled = true;
				GUI_SetButtonTextForRow(guiConfigCAN1_ENABLE_BUTTON_ID, "Enabled ", 1);
				GUI_SetButtonState(guiConfigCAN1_TOP_BUTTON_ID, GUIButtonState_Enabled);
			}
		}
	}
}

/**
 * @brief	Callback for the termination button
 * @param	Event: The event that caused the callback
 * @retval	None
 */
static void prvCan1TerminationButtonCallback(GUITouchEvent Event)
{
	static bool terminated = false;

	if (Event == GUITouchEvent_Up)
	{
		if (terminated)
		{
			ErrorStatus status = can1SetTermination(CAN1Termination_Disconnected);
			if (status == SUCCESS)
			{
				terminated = false;
				GUI_SetButtonTextForRow(guiConfigCAN1_TERMINATION_BUTTON_ID, "None ", 1);
			}
		}
		else
		{
			ErrorStatus status = can1SetTermination(CAN1Termination_Connected);
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
 * @retval	None
 */
static void prvCan1TopButtonCallback(GUITouchEvent Event)
{
	if (Event == GUITouchEvent_Up)
	{
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(guiConfigSIDEBAR_CAN1_CONTAINER_ID);
		prvChangeDisplayStateOfSidebar(guiConfigSIDEBAR_CAN1_CONTAINER_ID);
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
	prvTextBox.object.borderColor = LCD_COLOR_WHITE;
	prvTextBox.textColor = GUI_BLUE;
	prvTextBox.backgroundColor = LCD_COLOR_WHITE;
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
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_BLUE;
	prvButton.disabledTextColor = GUI_BLUE;
	prvButton.disabledBackgroundColor = LCD_COLOR_BLACK;
	prvButton.pressedTextColor = GUI_BLUE;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
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
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_BLUE;
	prvButton.disabledTextColor = LCD_COLOR_WHITE;
	prvButton.disabledBackgroundColor = GUI_BLUE;
	prvButton.pressedTextColor = GUI_BLUE;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
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
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_BLUE;
	prvButton.disabledTextColor = LCD_COLOR_WHITE;
	prvButton.disabledBackgroundColor = GUI_BLUE;
	prvButton.pressedTextColor = GUI_BLUE;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = 0;
	prvButton.text[0] = "Bit Rate:";
	prvButton.text[1] = "125 k";
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
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_BLUE;
	prvButton.disabledTextColor = LCD_COLOR_WHITE;
	prvButton.disabledBackgroundColor = GUI_BLUE;
	prvButton.pressedTextColor = GUI_BLUE;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvCan1TerminationButtonCallback;
	prvButton.text[0] = "Termination:";
	prvButton.text[1] = "None ";
//	prvButton.text[1] = "120 R";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* Containers ----------------------------------------------------------------*/
	/* Sidebar CAN1 container */
	prvContainer.object.id = guiConfigSIDEBAR_CAN1_CONTAINER_ID;
	prvContainer.object.xPos = 650;
	prvContainer.object.yPos = 50;
	prvContainer.object.width = 150;
	prvContainer.object.height = 380;
	prvContainer.object.layer = GUILayer_0;
	prvContainer.object.displayState = GUIDisplayState_Hidden;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 1;
	prvContainer.object.borderColor = LCD_COLOR_WHITE;
	prvContainer.contentHideState = GUIHideState_KeepBorders;
	prvContainer.buttons[0] = GUI_GetButtonFromId(guiConfigCAN1_ENABLE_BUTTON_ID);
	prvContainer.buttons[1] = GUI_GetButtonFromId(guiConfigCAN1_BIT_RATE_BUTTON_ID);
	prvContainer.buttons[2] = GUI_GetButtonFromId(guiConfigCAN1_TERMINATION_BUTTON_ID);
	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(guiConfigCAN1_LABEL_TEXT_BOX_ID);
	GUI_AddContainer(&prvContainer);
}

/* CAN2 GUI Elements ========================================================*/
/**
 * @brief	Callback for the enable button
 * @param	Event: The event that caused the callback
 * @retval	None
 */
static void prvCan2EnableButtonCallback(GUITouchEvent Event)
{
	static bool enabled = false;

	if (Event == GUITouchEvent_Up)
	{
		if (enabled)
		{
			ErrorStatus status = can2SetConnection(CAN2Connection_Disconnected);
			if (status == SUCCESS)
			{
				enabled = false;
				GUI_SetButtonTextForRow(guiConfigCAN2_ENABLE_BUTTON_ID, "Disabled", 1);
				GUI_SetButtonState(guiConfigCAN2_TOP_BUTTON_ID, GUIButtonState_Disabled);
			}
		}
		else
		{
			ErrorStatus status = can2SetConnection(CAN2Connection_Connected);
			if (status == SUCCESS)
			{
				enabled = true;
				GUI_SetButtonTextForRow(guiConfigCAN2_ENABLE_BUTTON_ID, "Enabled ", 1);
				GUI_SetButtonState(guiConfigCAN2_TOP_BUTTON_ID, GUIButtonState_Enabled);
			}
		}
	}
}

/**
 * @brief	Callback for the termination button
 * @param	Event: The event that caused the callback
 * @retval	None
 */
static void prvCan2TerminationButtonCallback(GUITouchEvent Event)
{
	static bool terminated = false;

	if (Event == GUITouchEvent_Up)
	{
		if (terminated)
		{
			ErrorStatus status = can2SetTermination(CAN2Termination_Disconnected);
			if (status == SUCCESS)
			{
				terminated = false;
				GUI_SetButtonTextForRow(guiConfigCAN2_TERMINATION_BUTTON_ID, "None ", 1);
			}
		}
		else
		{
			ErrorStatus status = can2SetTermination(CAN2Termination_Connected);
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
 * @retval	None
 */
static void prvCan2TopButtonCallback(GUITouchEvent Event)
{
	if (Event == GUITouchEvent_Up)
	{
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(guiConfigSIDEBAR_CAN2_CONTAINER_ID);
		prvChangeDisplayStateOfSidebar(guiConfigSIDEBAR_CAN2_CONTAINER_ID);
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
	prvTextBox.object.borderColor = LCD_COLOR_WHITE;
	prvTextBox.textColor = GUI_RED;
	prvTextBox.backgroundColor = LCD_COLOR_WHITE;
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
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_RED;
	prvButton.disabledTextColor = GUI_RED;
	prvButton.disabledBackgroundColor = LCD_COLOR_BLACK;
	prvButton.pressedTextColor = GUI_RED;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
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
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_RED;
	prvButton.disabledTextColor = LCD_COLOR_WHITE;
	prvButton.disabledBackgroundColor = GUI_RED;
	prvButton.pressedTextColor = GUI_RED;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
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
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_RED;
	prvButton.disabledTextColor = LCD_COLOR_WHITE;
	prvButton.disabledBackgroundColor = GUI_RED;
	prvButton.pressedTextColor = GUI_RED;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = 0;
	prvButton.text[0] = "Bit Rate:";
	prvButton.text[1] = "125 k";
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
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_RED;
	prvButton.disabledTextColor = LCD_COLOR_WHITE;
	prvButton.disabledBackgroundColor = GUI_RED;
	prvButton.pressedTextColor = GUI_RED;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvCan2TerminationButtonCallback;
	prvButton.text[0] = "Termination:";
	prvButton.text[1] = "None ";
//	prvButton.text[1] = "120 R";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* Containers ----------------------------------------------------------------*/
	/* Sidebar CAN2 container */
	prvContainer.object.id = guiConfigSIDEBAR_CAN2_CONTAINER_ID;
	prvContainer.object.xPos = 650;
	prvContainer.object.yPos = 50;
	prvContainer.object.width = 150;
	prvContainer.object.height = 380;
	prvContainer.object.layer = GUILayer_0;
	prvContainer.object.displayState = GUIDisplayState_Hidden;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 1;
	prvContainer.object.borderColor = LCD_COLOR_WHITE;
	prvContainer.contentHideState = GUIHideState_KeepBorders;
	prvContainer.buttons[0] = GUI_GetButtonFromId(guiConfigCAN2_ENABLE_BUTTON_ID);
	prvContainer.buttons[1] = GUI_GetButtonFromId(guiConfigCAN2_BIT_RATE_BUTTON_ID);
	prvContainer.buttons[2] = GUI_GetButtonFromId(guiConfigCAN2_TERMINATION_BUTTON_ID);
	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(guiConfigCAN2_LABEL_TEXT_BOX_ID);
	GUI_AddContainer(&prvContainer);
}

/* UART1 GUI Elements ========================================================*/
/**
 * @brief	Callback for the enable button
 * @param	Event: The event that caused the callback
 * @retval	None
 */
static void prvUart1EnableButtonCallback(GUITouchEvent Event)
{
	static bool enabled = false;

	if (Event == GUITouchEvent_Up)
	{
		if (enabled)
		{
			ErrorStatus status = uart1SetConnection(UART1Connection_Disconnected);
			if (status == SUCCESS)
			{
				enabled = false;
				GUI_SetButtonTextForRow(guiConfigUART1_ENABLE_BUTTON_ID, "Disabled", 1);
				GUI_SetButtonState(guiConfigUART1_TOP_BUTTON_ID, GUIButtonState_Disabled);
			}
		}
		else
		{
			ErrorStatus status = uart1SetConnection(UART1Connection_Connected);
			if (status == SUCCESS)
			{
				enabled = true;
				GUI_SetButtonTextForRow(guiConfigUART1_ENABLE_BUTTON_ID, "Enabled ", 1);
				GUI_SetButtonState(guiConfigUART1_TOP_BUTTON_ID, GUIButtonState_Enabled);
			}
		}
	}
}

/**
 * @brief	Callback for the voltage level button
 * @param	Event: The event that caused the callback
 * @retval	None
 */
static void prvUart1VoltageLevelButtonCallback(GUITouchEvent Event)
{
	static bool level5VisActive = false;

	if (Event == GUITouchEvent_Up)
	{
		if (level5VisActive)
		{
			ErrorStatus status = uart1SetPower(UART1Power_3V3);
			if (status == SUCCESS)
			{
				level5VisActive = false;
				GUI_SetButtonTextForRow(guiConfigUART1_VOLTAGE_LEVEL_BUTTON_ID, "3.3 V", 1);
			}
		}
		else
		{
			ErrorStatus status = uart1SetPower(UART1Power_5V);
			if (status == SUCCESS)
			{
				level5VisActive = true;
				GUI_SetButtonTextForRow(guiConfigUART1_VOLTAGE_LEVEL_BUTTON_ID, " 5 V ", 1);
			}
		}
	}
}

/**
 * @brief	Callback for the debug button
 * @param	Event: The event that caused the callback
 * @retval	None
 */
static void prvUart1DebugButtonCallback(GUITouchEvent Event)
{
	static bool enabled = false;
	static UART1Mode lastMode;

	if (Event == GUITouchEvent_Up)
	{
		UART1Settings settings = uart1GetSettings();
		if (enabled)
		{
			settings.mode = lastMode;
			enabled = false;
			GUI_SetButtonTextForRow(guiConfigUART1_DEBUG_BUTTON_ID, "Disabled", 1);
		}
		else
		{
			lastMode = settings.mode;
			settings.mode = UART1Mode_DebugTX;
			enabled = true;
			GUI_SetButtonTextForRow(guiConfigUART1_DEBUG_BUTTON_ID, "Enabled ", 1);
		}
		uart1SetSettings(&settings);
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @retval	None
 */
static void prvUart1TopButtonCallback(GUITouchEvent Event)
{
	if (Event == GUITouchEvent_Up)
	{
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(guiConfigSIDEBAR_UART1_CONTAINER_ID);
		prvChangeDisplayStateOfSidebar(guiConfigSIDEBAR_UART1_CONTAINER_ID);
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
	prvTextBox.object.borderColor = LCD_COLOR_WHITE;
	prvTextBox.textColor = GUI_GREEN;
	prvTextBox.backgroundColor = LCD_COLOR_WHITE;
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
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_GREEN;
	prvButton.disabledTextColor = GUI_GREEN;
	prvButton.disabledBackgroundColor = LCD_COLOR_BLACK;
	prvButton.pressedTextColor = GUI_GREEN;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
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
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_GREEN;
	prvButton.disabledTextColor = LCD_COLOR_WHITE;
	prvButton.disabledBackgroundColor = GUI_GREEN;
	prvButton.pressedTextColor = GUI_GREEN;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
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
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_GREEN;
	prvButton.disabledTextColor = LCD_COLOR_WHITE;
	prvButton.disabledBackgroundColor = GUI_GREEN;
	prvButton.pressedTextColor = GUI_GREEN;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = 0;
	prvButton.text[0] = "Baud Rate:";
	prvButton.text[1] = "115 200 bps";
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
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_GREEN;
	prvButton.disabledTextColor = LCD_COLOR_WHITE;
	prvButton.disabledBackgroundColor = GUI_GREEN;
	prvButton.pressedTextColor = GUI_GREEN;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvUart1VoltageLevelButtonCallback;
	prvButton.text[0] = "Voltage Level:";
	prvButton.text[1] = " 5 V ";
//	prvButton.text[1] = "3.3 V";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* UART1 Debug Button */
	prvButton.object.id = guiConfigUART1_DEBUG_BUTTON_ID;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 250;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.layer = GUILayer_0;
	prvButton.object.displayState = GUIDisplayState_Hidden;
	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_RED;
	prvButton.disabledTextColor = LCD_COLOR_WHITE;
	prvButton.disabledBackgroundColor = GUI_RED;
	prvButton.pressedTextColor = GUI_RED;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvUart1DebugButtonCallback;
	prvButton.text[0] = "Debug TX:";
	prvButton.text[1] = "Disabled";
//	prvButton.text[1] = "Enabled ";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* Containers ----------------------------------------------------------------*/
	/* Sidebar UART1 container */
	prvContainer.object.id = guiConfigSIDEBAR_UART1_CONTAINER_ID;
	prvContainer.object.xPos = 650;
	prvContainer.object.yPos = 50;
	prvContainer.object.width = 150;
	prvContainer.object.height = 380;
	prvContainer.object.layer = GUILayer_0;
	prvContainer.object.displayState = GUIDisplayState_Hidden;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 1;
	prvContainer.object.borderColor = LCD_COLOR_WHITE;
	prvContainer.contentHideState = GUIHideState_KeepBorders;
	prvContainer.buttons[0] = GUI_GetButtonFromId(guiConfigUART1_ENABLE_BUTTON_ID);
	prvContainer.buttons[1] = GUI_GetButtonFromId(guiConfigUART1_BAUD_RATE_BUTTON_ID);
	prvContainer.buttons[2] = GUI_GetButtonFromId(guiConfigUART1_VOLTAGE_LEVEL_BUTTON_ID);
	prvContainer.buttons[3] = GUI_GetButtonFromId(guiConfigUART1_DEBUG_BUTTON_ID);
	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(guiConfigUART1_LABEL_TEXT_BOX_ID);
	GUI_AddContainer(&prvContainer);
}

/* UART2 GUI Elements ========================================================*/
/**
 * @brief	Callback for the enable button
 * @param	Event: The event that caused the callback
 * @retval	None
 */
static void prvUart2EnableButtonCallback(GUITouchEvent Event)
{
	static bool enabled = false;

	if (Event == GUITouchEvent_Up)
	{
		if (enabled)
		{
			ErrorStatus status = uart2SetConnection(UART2Connection_Disconnected);
			if (status == SUCCESS)
			{
				enabled = false;
				GUI_SetButtonTextForRow(guiConfigUART2_ENABLE_BUTTON_ID, "Disabled", 1);
				GUI_SetButtonState(guiConfigUART2_TOP_BUTTON_ID, GUIButtonState_Disabled);
			}
		}
		else
		{
			ErrorStatus status = uart2SetConnection(UART2Connection_Connected);
			if (status == SUCCESS)
			{
				enabled = true;
				GUI_SetButtonTextForRow(guiConfigUART2_ENABLE_BUTTON_ID, "Enabled ", 1);
				GUI_SetButtonState(guiConfigUART2_TOP_BUTTON_ID, GUIButtonState_Enabled);
			}
		}
	}
}

/**
 * @brief	Callback for the voltage level button
 * @param	Event: The event that caused the callback
 * @retval	None
 */
static void prvUart2VoltageLevelButtonCallback(GUITouchEvent Event)
{
	static bool level5VisActive = false;

	if (Event == GUITouchEvent_Up)
	{
		if (level5VisActive)
		{
			ErrorStatus status = uart2SetPower(UART2Power_3V3);
			if (status == SUCCESS)
			{
				level5VisActive = false;
				GUI_SetButtonTextForRow(guiConfigUART2_VOLTAGE_LEVEL_BUTTON_ID, "3.3 V", 1);
			}
		}
		else
		{
			ErrorStatus status = uart2SetPower(UART2Power_5V);
			if (status == SUCCESS)
			{
				level5VisActive = true;
				GUI_SetButtonTextForRow(guiConfigUART2_VOLTAGE_LEVEL_BUTTON_ID, " 5 V ", 1);
			}
		}
	}
}

/**
 * @brief	Callback for the debug button
 * @param	Event: The event that caused the callback
 * @retval	None
 */
static void prvUart2DebugButtonCallback(GUITouchEvent Event)
{
	static bool enabled = false;
	static UART2Mode lastMode;

	if (Event == GUITouchEvent_Up)
	{
		UART2Settings settings = uart2GetSettings();
		if (enabled)
		{
			settings.mode = lastMode;
			enabled = false;
			GUI_SetButtonTextForRow(guiConfigUART2_DEBUG_BUTTON_ID, "Disabled", 1);
		}
		else
		{
			lastMode = settings.mode;
			settings.mode = UART2Mode_DebugTX;
			enabled = true;
			GUI_SetButtonTextForRow(guiConfigUART2_DEBUG_BUTTON_ID, "Enabled ", 1);
		}
		uart2SetSettings(&settings);
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @retval	None
 */
static void prvUart2TopButtonCallback(GUITouchEvent Event)
{
	if (Event == GUITouchEvent_Up)
	{
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(guiConfigSIDEBAR_UART2_CONTAINER_ID);
		prvChangeDisplayStateOfSidebar(guiConfigSIDEBAR_UART2_CONTAINER_ID);
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
	prvTextBox.object.borderColor = LCD_COLOR_WHITE;
	prvTextBox.textColor = GUI_YELLOW;
	prvTextBox.backgroundColor = LCD_COLOR_WHITE;
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
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_YELLOW;
	prvButton.disabledTextColor = GUI_YELLOW;
	prvButton.disabledBackgroundColor = LCD_COLOR_BLACK;
	prvButton.pressedTextColor = GUI_YELLOW;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
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
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_YELLOW;
	prvButton.disabledTextColor = LCD_COLOR_WHITE;
	prvButton.disabledBackgroundColor = GUI_YELLOW;
	prvButton.pressedTextColor = GUI_YELLOW;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
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
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_YELLOW;
	prvButton.disabledTextColor = LCD_COLOR_WHITE;
	prvButton.disabledBackgroundColor = GUI_YELLOW;
	prvButton.pressedTextColor = GUI_YELLOW;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = 0;
	prvButton.text[0] = "Baud Rate:";
	prvButton.text[1] = "115 200 bps";
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
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_YELLOW;
	prvButton.disabledTextColor = LCD_COLOR_WHITE;
	prvButton.disabledBackgroundColor = GUI_YELLOW;
	prvButton.pressedTextColor = GUI_YELLOW;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvUart2VoltageLevelButtonCallback;
	prvButton.text[0] = "Voltage Level:";
	prvButton.text[1] = " 5 V ";
//	prvButton.text[1] = "3.3 V";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* UART2 Debug Button */
	prvButton.object.id = guiConfigUART2_DEBUG_BUTTON_ID;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 250;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.layer = GUILayer_0;
	prvButton.object.displayState = GUIDisplayState_Hidden;
	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_RED;
	prvButton.disabledTextColor = LCD_COLOR_WHITE;
	prvButton.disabledBackgroundColor = GUI_RED;
	prvButton.pressedTextColor = GUI_RED;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = prvUart2DebugButtonCallback;
	prvButton.text[0] = "Debug TX:";
	prvButton.text[1] = "Disabled";
//	prvButton.text[1] = "Enabled ";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* Containers ----------------------------------------------------------------*/
	/* Sidebar UART2 container */
	prvContainer.object.id = guiConfigSIDEBAR_UART2_CONTAINER_ID;
	prvContainer.object.xPos = 650;
	prvContainer.object.yPos = 50;
	prvContainer.object.width = 150;
	prvContainer.object.height = 380;
	prvContainer.object.layer = GUILayer_0;
	prvContainer.object.displayState = GUIDisplayState_Hidden;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 1;
	prvContainer.object.borderColor = LCD_COLOR_WHITE;
	prvContainer.contentHideState = GUIHideState_KeepBorders;
	prvContainer.buttons[0] = GUI_GetButtonFromId(guiConfigUART2_ENABLE_BUTTON_ID);
	prvContainer.buttons[1] = GUI_GetButtonFromId(guiConfigUART2_BAUD_RATE_BUTTON_ID);
	prvContainer.buttons[2] = GUI_GetButtonFromId(guiConfigUART2_VOLTAGE_LEVEL_BUTTON_ID);
	prvContainer.buttons[3] = GUI_GetButtonFromId(guiConfigUART2_DEBUG_BUTTON_ID);
	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(guiConfigUART2_LABEL_TEXT_BOX_ID);
	GUI_AddContainer(&prvContainer);
}

/* RS232 GUI Elements ========================================================*/
/**
 * @brief	Callback for the enable button
 * @param	Event: The event that caused the callback
 * @retval	None
 */
static void prvRs232EnableButtonCallback(GUITouchEvent Event)
{
	static bool enabled = false;

	if (Event == GUITouchEvent_Up)
	{
		if (enabled)
		{
			ErrorStatus status = rs232SetConnection(RS232Connection_Disconnected);
			if (status == SUCCESS)
			{
				enabled = false;
				GUI_SetButtonTextForRow(guiConfigRS232_ENABLE_BUTTON_ID, "Disabled", 1);
				GUI_SetButtonState(guiConfigRS232_TOP_BUTTON_ID, GUIButtonState_Disabled);
			}
		}
		else
		{
			ErrorStatus status = rs232SetConnection(RS232Connection_Connected);
			if (status == SUCCESS)
			{
				enabled = true;
				GUI_SetButtonTextForRow(guiConfigRS232_ENABLE_BUTTON_ID, "Enabled ", 1);
				GUI_SetButtonState(guiConfigRS232_TOP_BUTTON_ID, GUIButtonState_Enabled);
			}
		}
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @retval	None
 */
static void prvRs232TopButtonCallback(GUITouchEvent Event)
{
	if (Event == GUITouchEvent_Up)
	{
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(guiConfigSIDEBAR_RS232_CONTAINER_ID);
		prvChangeDisplayStateOfSidebar(guiConfigSIDEBAR_RS232_CONTAINER_ID);
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
	prvTextBox.object.borderColor = LCD_COLOR_WHITE;
	prvTextBox.textColor = GUI_PURPLE;
	prvTextBox.backgroundColor = LCD_COLOR_WHITE;
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
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_PURPLE;
	prvButton.disabledBackgroundColor = LCD_COLOR_BLACK;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
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
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = LCD_COLOR_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
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
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = LCD_COLOR_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = 0;
	prvButton.text[0] = "Baud Rate:";
	prvButton.text[1] = "115 200 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* Containers ----------------------------------------------------------------*/
	/* Sidebar RS232 container */
	prvContainer.object.id = guiConfigSIDEBAR_RS232_CONTAINER_ID;
	prvContainer.object.xPos = 650;
	prvContainer.object.yPos = 50;
	prvContainer.object.width = 150;
	prvContainer.object.height = 380;
	prvContainer.object.layer = GUILayer_0;
	prvContainer.object.displayState = GUIDisplayState_Hidden;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 1;
	prvContainer.object.borderColor = LCD_COLOR_WHITE;
	prvContainer.contentHideState = GUIHideState_KeepBorders;
	prvContainer.buttons[0] = GUI_GetButtonFromId(guiConfigRS232_ENABLE_BUTTON_ID);
	prvContainer.buttons[1] = GUI_GetButtonFromId(guiConfigRS232_BAUD_RATE_BUTTON_ID);
	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(guiConfigRS232_LABEL_TEXT_BOX_ID);
	GUI_AddContainer(&prvContainer);
}

/* GPIO GUI Elements ========================================================*/
/**
 * @brief
 * @param	Event: The event that caused the callback
 * @retval	None
 */
static void prvGpioTopButtonCallback(GUITouchEvent Event)
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
	prvTextBox.object.borderColor = LCD_COLOR_WHITE;
	prvTextBox.textColor = GUI_CYAN_LIGHT;
	prvTextBox.backgroundColor = LCD_COLOR_WHITE;
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
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_CYAN_LIGHT;
	prvButton.disabledTextColor = GUI_CYAN_LIGHT;
	prvButton.disabledBackgroundColor = LCD_COLOR_BLACK;
	prvButton.pressedTextColor = GUI_CYAN_LIGHT;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
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
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_CYAN_LIGHT;
	prvButton.disabledTextColor = LCD_COLOR_WHITE;
	prvButton.disabledBackgroundColor = GUI_CYAN_LIGHT;
	prvButton.pressedTextColor = GUI_CYAN_LIGHT;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
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
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_CYAN_LIGHT;
	prvButton.disabledTextColor = LCD_COLOR_WHITE;
	prvButton.disabledBackgroundColor = GUI_CYAN_LIGHT;
	prvButton.pressedTextColor = GUI_CYAN_LIGHT;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
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
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_CYAN_DARK;
	prvButton.disabledTextColor = LCD_COLOR_WHITE;
	prvButton.disabledBackgroundColor = GUI_CYAN_DARK;
	prvButton.pressedTextColor = GUI_CYAN_DARK;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
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
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_CYAN_DARK;
	prvButton.disabledTextColor = LCD_COLOR_WHITE;
	prvButton.disabledBackgroundColor = GUI_CYAN_DARK;
	prvButton.pressedTextColor = GUI_CYAN_DARK;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
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
	prvContainer.object.height = 380;
	prvContainer.object.layer = GUILayer_0;
	prvContainer.object.displayState = GUIDisplayState_Hidden;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 1;
	prvContainer.object.borderColor = LCD_COLOR_WHITE;
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
 * @brief
 * @param	Event: The event that caused the callback
 * @retval	None
 */
static void prvAdcTopButtonCallback(GUITouchEvent Event)
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
	prvTextBox.object.borderColor = LCD_COLOR_WHITE;
	prvTextBox.textColor = GUI_MAGENTA;
	prvTextBox.backgroundColor = LCD_COLOR_WHITE;
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
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_MAGENTA;
	prvButton.disabledTextColor = GUI_MAGENTA;
	prvButton.disabledBackgroundColor = LCD_COLOR_BLACK;
	prvButton.pressedTextColor = GUI_MAGENTA;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
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
	prvButton.object.borderColor = LCD_COLOR_WHITE;
	prvButton.enabledTextColor = LCD_COLOR_WHITE;
	prvButton.enabledBackgroundColor = GUI_MAGENTA;
	prvButton.disabledTextColor = LCD_COLOR_WHITE;
	prvButton.disabledBackgroundColor = GUI_MAGENTA;
	prvButton.pressedTextColor = GUI_MAGENTA;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
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
	prvContainer.object.height = 380;
	prvContainer.object.layer = GUILayer_0;
	prvContainer.object.displayState = GUIDisplayState_Hidden;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 1;
	prvContainer.object.borderColor = LCD_COLOR_WHITE;
	prvContainer.contentHideState = GUIHideState_KeepBorders;
	prvContainer.buttons[0] = GUI_GetButtonFromId(guiConfigADC_ENABLE_BUTTON_ID);
	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(guiConfigADC_LABEL_TEXT_BOX_ID);
	GUI_AddContainer(&prvContainer);
}

/* Interrupt Handlers --------------------------------------------------------*/
