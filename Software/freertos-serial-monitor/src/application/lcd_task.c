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
/* Private function prototypes -----------------------------------------------*/
static void prvHardwareInit();
static void prvInitGuiElements();

static void prvDebugToggleCallback(GUITouchEvent Event);

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

	GUI_WriteStringInTextBox(guiConfigTEMP_TEXT_BOX_ID, "20 C");
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

						LCD_SetForegroundColor(LCD_COLOR_GREEN);
						LCD_DrawCircle(receivedMessage.data[0], receivedMessage.data[1], 2, 1);

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
	static bool debugConsoleIsHidden = false;
	if (Event == GUITouchEvent_Up)
	{
		if (debugConsoleIsHidden)
		{
//			GUI_DrawTextBox(guiConfigDEBUG_TEXT_BOX_ID);
			GUI_DrawContainer(guiConfigDEBUG_CONTAINER_ID);
			debugConsoleIsHidden = false;
		}
		else
		{
//			GUI_RemoveTextBox(guiConfigDEBUG_TEXT_BOX_ID);
			GUI_HideContentInContainer(guiConfigDEBUG_CONTAINER_ID);
			debugConsoleIsHidden = true;
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
	GUITextBox textBox = {0};
	/* Main text box */
	textBox.object.id = guiConfigMAIN_TEXT_BOX_ID;
	textBox.object.xPos = 0;
	textBox.object.yPos = 50;
	textBox.object.width = 650;
	textBox.object.height = 400;
	textBox.object.layer = GUILayer_0;
	textBox.object.displayState = GUIDisplayState_NotHidden;
	textBox.object.border = GUIBorder_Top | GUIBorder_Right;
	textBox.object.borderThickness = 1;
	textBox.object.borderColor = LCD_COLOR_WHITE;
	textBox.textColor = LCD_COLOR_WHITE;
	textBox.backgroundColor = LCD_COLOR_BLACK;
	textBox.textSize = LCDFontEnlarge_1x;
	textBox.xWritePos = 0;
	textBox.yWritePos = 0;
	GUI_AddTextBox(&textBox);

	/* Clock Text Box */
	textBox.object.id = guiConfigCLOCK_TEXT_BOX_ID;
	textBox.object.xPos = 650;
	textBox.object.yPos = 0;
	textBox.object.width = 150;
	textBox.object.height = 25;
	textBox.object.layer = GUILayer_0;
	textBox.object.displayState = GUIDisplayState_NotHidden;
	textBox.object.border = GUIBorder_NoBorder;
	textBox.object.borderThickness = 0;
	textBox.object.borderColor = LCD_COLOR_WHITE;
	textBox.textColor = LCD_COLOR_WHITE;
	textBox.backgroundColor = LCD_COLOR_BLACK;
	textBox.textSize = LCDFontEnlarge_1x;
	textBox.xWritePos = 50;
	textBox.yWritePos = 3;
	GUI_AddTextBox(&textBox);

	/* Temperature Text Box */
	textBox.object.id = guiConfigTEMP_TEXT_BOX_ID;
	textBox.object.xPos = 650;
	textBox.object.yPos = 25;
	textBox.object.width = 150;
	textBox.object.height = 25;
	textBox.object.layer = GUILayer_0;
	textBox.object.displayState = GUIDisplayState_NotHidden;
	textBox.object.border = GUIBorder_NoBorder;
	textBox.object.borderThickness = 0;
	textBox.object.borderColor = LCD_COLOR_WHITE;
	textBox.textColor = LCD_COLOR_WHITE;
	textBox.backgroundColor = LCD_COLOR_BLACK;
	textBox.textSize = LCDFontEnlarge_1x;
	textBox.xWritePos = 100;
	textBox.yWritePos = 3;
	GUI_AddTextBox(&textBox);

	/* Debug Text Box */
	textBox.object.id = guiConfigDEBUG_TEXT_BOX_ID;
	textBox.object.xPos = 0;
	textBox.object.yPos = 450;
	textBox.object.width = 649;
	textBox.object.height = 30;
	textBox.object.layer = GUILayer_0;
	textBox.object.displayState = GUIDisplayState_NotHidden;
	textBox.object.border = GUIBorder_Top;
	textBox.object.borderThickness = 1;
	textBox.object.borderColor = LCD_COLOR_WHITE;
	textBox.textColor = LCD_COLOR_WHITE;
	textBox.backgroundColor = GUI_RED;
	textBox.textSize = LCDFontEnlarge_1x;
	textBox.xWritePos = 0;
	textBox.yWritePos = 0;
	GUI_AddTextBox(&textBox);

	GUI_DrawTextBox(guiConfigMAIN_TEXT_BOX_ID);

	GUI_WriteStringInTextBox(guiConfigMAIN_TEXT_BOX_ID, "Hello World!");

	/* Buttons -------------------------------------------------------------------*/
	GUIButton button = {0};
	/* CAN1 Button */
	button.object.id = guiConfigCAN1_BUTTON_ID;
	button.object.xPos = 0;
	button.object.yPos = 0;
	button.object.width = 100;
	button.object.height = 50;
	button.object.layer = GUILayer_0;
	button.object.displayState = GUIDisplayState_NotHidden;
	button.object.border = GUIBorder_Bottom | GUIBorder_Right;
	button.object.borderThickness = 1;
	button.object.borderColor = LCD_COLOR_WHITE;
	button.enabledTextColor = LCD_COLOR_WHITE;
	button.enabledBackgroundColor = GUI_BLUE;
	button.disabledTextColor = GUI_BLUE;
	button.disabledBackgroundColor = LCD_COLOR_BLACK;
	button.pressedTextColor = GUI_BLUE;
	button.pressedBackgroundColor = LCD_COLOR_WHITE;
	button.state = GUIButtonState_Disabled;
	button.touchCallback = testCallback;
	button.text = "CAN1";
	button.textSize = LCDFontEnlarge_2x;
	GUI_AddButton(&button);

	/* CAN2 Button */
	button.object.id = guiConfigCAN2_BUTTON_ID;
	button.object.xPos = 100;
	button.object.yPos = 0;
	button.object.width = 100;
	button.object.height = 50;
	button.object.layer = GUILayer_0;
	button.object.displayState = GUIDisplayState_NotHidden;
	button.object.border = GUIBorder_Bottom | GUIBorder_Right | GUIBorder_Left;
	button.object.borderThickness = 1;
	button.object.borderColor = LCD_COLOR_WHITE;
	button.enabledTextColor = LCD_COLOR_WHITE;
	button.enabledBackgroundColor = GUI_RED;
	button.disabledTextColor = GUI_RED;
	button.disabledBackgroundColor = LCD_COLOR_BLACK;
	button.pressedTextColor = GUI_RED;
	button.pressedBackgroundColor = LCD_COLOR_WHITE;
	button.state = GUIButtonState_Disabled;
	button.touchCallback = 0;
	button.text = "CAN2";
	button.textSize = LCDFontEnlarge_2x;
	GUI_AddButton(&button);

	/* UART1 Button */
	button.object.id = guiConfigUART1_BUTTON_ID;
	button.object.xPos = 200;
	button.object.yPos = 0;
	button.object.width = 100;
	button.object.height = 50;
	button.object.layer = GUILayer_0;
	button.object.displayState = GUIDisplayState_NotHidden;
	button.object.border = GUIBorder_Bottom | GUIBorder_Right | GUIBorder_Left;
	button.object.borderThickness = 1;
	button.object.borderColor = LCD_COLOR_WHITE;
	button.enabledTextColor = LCD_COLOR_WHITE;
	button.enabledBackgroundColor = GUI_GREEN;
	button.disabledTextColor = GUI_GREEN;
	button.disabledBackgroundColor = LCD_COLOR_BLACK;
	button.pressedTextColor = GUI_GREEN;
	button.pressedBackgroundColor = LCD_COLOR_WHITE;
	button.state = GUIButtonState_Disabled;
	button.touchCallback = 0;
	button.text = "UART1";
	button.textSize = LCDFontEnlarge_2x;
	GUI_AddButton(&button);

	/* UART2 Button */
	button.object.id = guiConfigUART2_BUTTON_ID;
	button.object.xPos = 300;
	button.object.yPos = 0;
	button.object.width = 100;
	button.object.height = 50;
	button.object.layer = GUILayer_0;
	button.object.displayState = GUIDisplayState_NotHidden;
	button.object.border = GUIBorder_Bottom | GUIBorder_Right | GUIBorder_Left;
	button.object.borderThickness = 1;
	button.object.borderColor = LCD_COLOR_WHITE;
	button.enabledTextColor = LCD_COLOR_WHITE;
	button.enabledBackgroundColor = GUI_YELLOW;
	button.disabledTextColor = GUI_YELLOW;
	button.disabledBackgroundColor = LCD_COLOR_BLACK;
	button.pressedTextColor = GUI_YELLOW;
	button.pressedBackgroundColor = LCD_COLOR_WHITE;
	button.state = GUIButtonState_Disabled;
	button.touchCallback = 0;
	button.text = "UART2";
	button.textSize = LCDFontEnlarge_2x;
	GUI_AddButton(&button);

	/* RS232 Button */
	button.object.id = guiConfigRS232_BUTTON_ID;
	button.object.xPos = 400;
	button.object.yPos = 0;
	button.object.width = 100;
	button.object.height = 50;
	button.object.layer = GUILayer_0;
	button.object.displayState = GUIDisplayState_NotHidden;
	button.object.border = GUIBorder_Bottom | GUIBorder_Right | GUIBorder_Left;
	button.object.borderThickness = 1;
	button.object.borderColor = LCD_COLOR_WHITE;
	button.enabledTextColor = LCD_COLOR_WHITE;
	button.enabledBackgroundColor = GUI_PURPLE;
	button.disabledTextColor = GUI_PURPLE;
	button.disabledBackgroundColor = LCD_COLOR_BLACK;
	button.pressedTextColor = GUI_PURPLE;
	button.pressedBackgroundColor = LCD_COLOR_WHITE;
	button.state = GUIButtonState_Disabled;
	button.touchCallback = 0;
	button.text = "RS232";
	button.textSize = LCDFontEnlarge_2x;
	GUI_AddButton(&button);

	/* GPIO Button */
	button.object.id = guiConfigGPIO_BUTTON_ID;
	button.object.xPos = 500;
	button.object.yPos = 0;
	button.object.width = 100;
	button.object.height = 50;
	button.object.layer = GUILayer_0;
	button.object.displayState = GUIDisplayState_NotHidden;
	button.object.border = GUIBorder_Bottom | GUIBorder_Right | GUIBorder_Left;
	button.object.borderThickness = 1;
	button.object.borderColor = LCD_COLOR_WHITE;
	button.enabledTextColor = LCD_COLOR_WHITE;
	button.enabledBackgroundColor = GUI_CYAN;
	button.disabledTextColor = GUI_CYAN;
	button.disabledBackgroundColor = LCD_COLOR_BLACK;
	button.pressedTextColor = GUI_CYAN;
	button.pressedBackgroundColor = LCD_COLOR_WHITE;
	button.state = GUIButtonState_Disabled;
	button.touchCallback = 0;
	button.text = "GPIO";
	button.textSize = LCDFontEnlarge_2x;
	GUI_AddButton(&button);

	/* ADC Button */
	button.object.id = guiConfigADC_BUTTON_ID;
	button.object.xPos = 600;
	button.object.yPos = 0;
	button.object.width = 50;
	button.object.height = 50;
	button.object.layer = GUILayer_0;
	button.object.displayState = GUIDisplayState_NotHidden;
	button.object.border = GUIBorder_Bottom | GUIBorder_Right | GUIBorder_Left;
	button.object.borderThickness = 1;
	button.object.borderColor = LCD_COLOR_WHITE;
	button.enabledTextColor = LCD_COLOR_WHITE;
	button.enabledBackgroundColor = GUI_MAGENTA;
	button.disabledTextColor = GUI_MAGENTA;
	button.disabledBackgroundColor = LCD_COLOR_BLACK;
	button.pressedTextColor = GUI_MAGENTA;
	button.pressedBackgroundColor = LCD_COLOR_WHITE;
	button.state = GUIButtonState_Disabled;
	button.touchCallback = 0;
	button.text = "ADC";
	button.textSize = LCDFontEnlarge_1x;
	GUI_AddButton(&button);

	/* Storage Button */
	button.object.id = guiConfigSTORAGE_BUTTON_ID;
	button.object.xPos = 650;
	button.object.yPos = 330;
	button.object.width = 150;
	button.object.height = 50;
	button.object.layer = GUILayer_0;
	button.object.displayState = GUIDisplayState_NotHidden;
	button.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	button.object.borderThickness = 1;
	button.object.borderColor = LCD_COLOR_WHITE;
	button.enabledTextColor = LCD_COLOR_WHITE;
	button.enabledBackgroundColor = GUI_DARK_BLUE;
	button.disabledTextColor = LCD_COLOR_WHITE;
	button.disabledBackgroundColor = GUI_DARK_BLUE;
	button.pressedTextColor = GUI_DARK_BLUE;
	button.pressedBackgroundColor = LCD_COLOR_WHITE;
	button.state = GUIButtonState_Disabled;
	button.touchCallback = 0;
	button.text = "Storage";
	button.textSize = LCDFontEnlarge_2x;
	GUI_AddButton(&button);

	/* Settings Button */
	button.object.id = guiConfigSETTINGS_BUTTON_ID;
	button.object.xPos = 650;
	button.object.yPos = 380;
	button.object.width = 150;
	button.object.height = 50;
	button.object.layer = GUILayer_0;
	button.object.displayState = GUIDisplayState_NotHidden;
	button.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	button.object.borderThickness = 1;
	button.object.borderColor = LCD_COLOR_WHITE;
	button.enabledTextColor = LCD_COLOR_WHITE;
	button.enabledBackgroundColor = GUI_DARK_BLUE;
	button.disabledTextColor = LCD_COLOR_WHITE;
	button.disabledBackgroundColor = GUI_DARK_BLUE;
	button.pressedTextColor = GUI_DARK_BLUE;
	button.pressedBackgroundColor = LCD_COLOR_WHITE;
	button.state = GUIButtonState_Disabled;
	button.touchCallback = 0;
	button.text = "Settings";
	button.textSize = LCDFontEnlarge_2x;
	GUI_AddButton(&button);

	/* Debug Button */
	button.object.id = guiConfigDEBUG_BUTTON_ID;
	button.object.xPos = 650;
	button.object.yPos = 430;
	button.object.width = 150;
	button.object.height = 50;
	button.object.layer = GUILayer_0;
	button.object.displayState = GUIDisplayState_NotHidden;
	button.object.border = GUIBorder_Top | GUIBorder_Left;
	button.object.borderThickness = 1;
	button.object.borderColor = LCD_COLOR_WHITE;
	button.enabledTextColor = LCD_COLOR_WHITE;
	button.enabledBackgroundColor = GUI_RED;
	button.disabledTextColor = LCD_COLOR_WHITE;
	button.disabledBackgroundColor = GUI_RED;
	button.pressedTextColor = GUI_RED;
	button.pressedBackgroundColor = LCD_COLOR_WHITE;
	button.state = GUIButtonState_Disabled;
	button.touchCallback = prvDebugToggleCallback;
	button.text = "Debug";
	button.textSize = LCDFontEnlarge_2x;
	GUI_AddButton(&button);

	GUI_DrawAllButtons();

	/* Containers ----------------------------------------------------------------*/
	GUIContainer container = {0};

	/* Status info container */
	container.object.id = guiConfigSTATUS_CONTAINER_ID;
	container.object.xPos = 650;
	container.object.yPos = 0;
	container.object.width = 150;
	container.object.height = 50;
	container.object.layer = GUILayer_0;
	container.object.displayState = GUIDisplayState_NotHidden;
	container.object.border = GUIBorder_Left | GUIBorder_Bottom;
	container.object.borderThickness = 1;
	container.object.borderColor = LCD_COLOR_WHITE;
	container.textBoxes[0] = GUI_GetTextBoxFromId(guiConfigCLOCK_TEXT_BOX_ID);
	container.textBoxes[1] = GUI_GetTextBoxFromId(guiConfigTEMP_TEXT_BOX_ID);
	GUI_AddContainer(&container);
	container.textBoxes[0] = 0;
	container.textBoxes[1] = 0;

	/* Debug container */
	container.object.id = guiConfigDEBUG_CONTAINER_ID;
	container.object.xPos = 0;
	container.object.yPos = 450;
	container.object.width = 650;
	container.object.height = 30;
	container.object.layer = GUILayer_0;
	container.object.displayState = GUIDisplayState_NotHidden;
	container.object.border = GUIBorder_Right;
	container.object.borderThickness = 1;
	container.object.borderColor = LCD_COLOR_WHITE;
	container.textBoxes[0] = GUI_GetTextBoxFromId(guiConfigDEBUG_TEXT_BOX_ID);
	GUI_AddContainer(&container);
	container.textBoxes[0] = 0;

	GUI_DrawContainer(guiConfigSTATUS_CONTAINER_ID);
	GUI_DrawContainer(guiConfigDEBUG_CONTAINER_ID);
}

/* Interrupt Handlers --------------------------------------------------------*/
