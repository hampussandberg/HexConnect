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

#include <stdbool.h>

/* Private defines -----------------------------------------------------------*/
#define GUI_BLUE	0x237F
#define GUI_RED		0xF926
#define GUI_GREEN	0x362A
#define GUI_YELLOW	0xFEE6
#define GUI_PURPLE	0xA8D6
#define GUI_GRAY	0xB596
#define GUI_MAGENTA	0xF81F
#define GUI_CYAN	0x07FF
#define GUI_DARK_BLUE	0x11CE

/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
GUITextBox prvTextBox = {0};
GUIButton prvButton = {0};
GUIContainer prvContainer = {0};
uint32_t prvIdOfLastActiveSidebar = guiConfigINVALID_ID;
static bool prvDebugConsoleIsHidden = false;
uint32_t prvTempUpdateCounter = 1000;

/* Private function prototypes -----------------------------------------------*/
static void prvHardwareInit();
static void prvInitGuiElements();

static void prvDebugToggleCallback(GUITouchEvent Event);
static void prvSystemToggleCallback(GUITouchEvent Event);

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

	while (1)
	{
		/* Wait for a message to be received or the timeout to happen */
		if (xQueueReceive(xLCDEventQueue, &receivedMessage, 50) == pdTRUE)
		{
			/* Item sucessfully removed from the queue */
			switch (receivedMessage.event)
			{
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
				default:
					break;
			}
		}
		else
		{
			/* Timeout has occured i.e. no message available */
	//		vTaskDelayUntil(&xNextWakeTime, 100 / portTICK_PERIOD_MS);
			HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_2);
			/* Do something else */


			/* TODO: Do this in background task instead */
			if (prvTempUpdateCounter >= 1000)
			{
				prvTempUpdateCounter = 0;
				int8_t currentTemp = (int8_t)MCP9808_GetTemperature();
				GUI_DrawTextBox(guiConfigTEMP_TEXT_BOX_ID);
				GUI_SetWritePosition(guiConfigTEMP_TEXT_BOX_ID, 50, 3);
				GUI_WriteNumberInTextBox(guiConfigTEMP_TEXT_BOX_ID, (int32_t)currentTemp);
				GUI_WriteStringInTextBox(guiConfigTEMP_TEXT_BOX_ID, " C");
			}
			else
				prvTempUpdateCounter += 50;
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

static void testCallback(GUITouchEvent Event)
{
	GUI_ClearTextBox(guiConfigMAIN_TEXT_BOX_ID);
}

/**
 * @brief	Callback for the debug button, will toggle the debug textbox on and off
 * @param	Event: The event that caused the callback
 * @retval	None
 */
static void prvDebugToggleCallback(GUITouchEvent Event)
{
	if (Event == GUITouchEvent_Up)
	{
		if (prvDebugConsoleIsHidden)
		{
			GUI_DrawContainer(guiConfigDEBUG_CONTAINER_ID);
			prvDebugConsoleIsHidden = false;
		}
		else
		{
			GUI_HideContentInContainer(guiConfigDEBUG_CONTAINER_ID);
			GUI_ClearTextBox(guiConfigMAIN_TEXT_BOX_ID);
			prvDebugConsoleIsHidden = true;
		}
	}
}

/**
 * @brief	Callback for the system button, will toggle the side system sidebar on and off
 * @param	Event: The event that caused the callback
 * @retval	None
 */
static void prvSystemToggleCallback(GUITouchEvent Event)
{
	static bool systemSidebarIsHidden = false;
	if (Event == GUITouchEvent_Up)
	{
		if (systemSidebarIsHidden)
		{
			GUI_HideContainer(prvIdOfLastActiveSidebar);
			GUI_DrawContainer(guiConfigSIDEBAR_SYSTEM_CONTAINER_ID);
			systemSidebarIsHidden = false;
		}
		else
		{
			GUI_HideContainer(guiConfigSIDEBAR_SYSTEM_CONTAINER_ID);
			GUI_DrawContainer(prvIdOfLastActiveSidebar);
			systemSidebarIsHidden = true;
		}
	}
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
	/* CAN1 Button */
	prvButton.object.id = guiConfigCAN1_BUTTON_ID;
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
	prvButton.touchCallback = testCallback;
	prvButton.text = "CAN1";
	prvButton.textSize = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* CAN2 Button */
	prvButton.object.id = guiConfigCAN2_BUTTON_ID;
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
	prvButton.touchCallback = 0;
	prvButton.text = "CAN2";
	prvButton.textSize = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* UART1 Button */
	prvButton.object.id = guiConfigUART1_BUTTON_ID;
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
	prvButton.touchCallback = 0;
	prvButton.text = "UART1";
	prvButton.textSize = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* UART2 Button */
	prvButton.object.id = guiConfigUART2_BUTTON_ID;
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
	prvButton.touchCallback = 0;
	prvButton.text = "UART2";
	prvButton.textSize = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* RS232 Button */
	prvButton.object.id = guiConfigRS232_BUTTON_ID;
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
	prvButton.touchCallback = 0;
	prvButton.text = "RS232";
	prvButton.textSize = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* GPIO Button */
	prvButton.object.id = guiConfigGPIO_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_CYAN;
	prvButton.disabledTextColor = GUI_CYAN;
	prvButton.disabledBackgroundColor = LCD_COLOR_BLACK;
	prvButton.pressedTextColor = GUI_CYAN;
	prvButton.pressedBackgroundColor = LCD_COLOR_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = 0;
	prvButton.text = "GPIO";
	prvButton.textSize = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* ADC Button */
	prvButton.object.id = guiConfigADC_BUTTON_ID;
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
	prvButton.touchCallback = 0;
	prvButton.text = "ADC";
	prvButton.textSize = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

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
	prvButton.text = "Storage";
	prvButton.textSize = LCDFontEnlarge_2x;
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
	prvButton.text = "Settings";
	prvButton.textSize = LCDFontEnlarge_2x;
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
	prvButton.text = "Debug";
	prvButton.textSize = LCDFontEnlarge_2x;
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
	prvButton.touchCallback = prvSystemToggleCallback;
	prvButton.text = "System";
	prvButton.textSize = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	GUI_DrawAllButtons();

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
	prvContainer.textBoxes[0] = 0;
	prvContainer.textBoxes[1] = 0;

	/* Debug container */
	prvContainer.object.id = guiConfigDEBUG_CONTAINER_ID;
	prvContainer.object.xPos = 0;
	prvContainer.object.yPos = 450;
	prvContainer.object.width = 650;
	prvContainer.object.height = 30;
	prvContainer.object.layer = GUILayer_0;
	prvContainer.object.displayState = GUIDisplayState_NotHidden;
	prvContainer.object.border = GUIBorder_Right;
	prvContainer.object.borderThickness = 1;
	prvContainer.object.borderColor = LCD_COLOR_WHITE;
	prvContainer.contentHideState = GUIHideState_KeepBorders;
	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(guiConfigDEBUG_TEXT_BOX_ID);
	GUI_AddContainer(&prvContainer);
	prvContainer.textBoxes[0] = 0;

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
	GUI_AddContainer(&prvContainer);
	prvContainer.buttons[0] = 0;
	prvContainer.buttons[1] = 0;
	prvContainer.buttons[2] = 0;

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

	GUI_DrawContainer(guiConfigSTATUS_CONTAINER_ID);
	GUI_DrawContainer(guiConfigDEBUG_CONTAINER_ID);
//	GUI_HideContentInContainer(guiConfigDEBUG_CONTAINER_ID);
	GUI_DrawContainer(guiConfigSIDEBAR_SYSTEM_CONTAINER_ID);
	prvIdOfLastActiveSidebar = guiConfigSIDEBAR_EMPTY_CONTAINER_ID;
}

/* Interrupt Handlers --------------------------------------------------------*/
