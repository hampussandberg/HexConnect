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
static void guiTestInit();
static void guiTest();

/* Functions -----------------------------------------------------------------*/
/**
 * @brief	The main task for the LCD
 * @param	pvParameters:
 * @retval	None
 */
void lcdTask(void *pvParameters)
{
	prvHardwareInit();
	guiTestInit();

	/* The parameter in vTaskDelayUntil is the absolute time
	 * in ticks at which you want to be woken calculated as
	 * an increment from the time you were last woken. */
	TickType_t xNextWakeTime;
	/* Initialize xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	while (1)
	{
//		vTaskDelayUntil(&xNextWakeTime, 1000 / portTICK_PERIOD_MS);
		guiTest();
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
	LCD_Init();
}

static void guiTestInit()
{
	LCD_SetBackgroundColor(LCD_COLOR_BLACK);
	LCD_ClearFullWindow();

	GUIButton_TypeDef button;
	button.disabledBackgroundColor = LCD_COLOR_BLACK;
	button.pressedBackgroundColor = LCD_COLOR_WHITE;
	button.enabledTextColor = LCD_COLOR_WHITE;
	button.object.borderColor = LCD_COLOR_WHITE;
	button.object.borderThickness = 1;
	button.state = DISABLED;

	button.object.yPos = 0;
	button.object.height = 50;
	button.object.layer = LAYER0;
	button.object.hidden = NOT_HIDDEN;

	/* CAN1 Button */
	button.object.id = guiConfigCAN1_BUTTON_ID;
	button.enabledBackgroundColor = button.disabledTextColor = button.pressedTextColor = GUI_BLUE;
	button.text = "CAN1";
	button.textSize = ENLARGE_2X;
	button.object.xPos = 0;
	button.object.width = 100;
	button.object.border = BORDER_BOTTOM | BORDER_RIGHT;
	GUI_AddButton(&button);

	/* CAN2 Button */
	button.object.id = guiConfigCAN2_BUTTON_ID;
	button.enabledBackgroundColor = button.disabledTextColor = button.pressedTextColor = GUI_RED;
	button.text = "CAN2";
	button.textSize = ENLARGE_2X;
	button.object.xPos = 100;
	button.object.border = BORDER_BOTTOM | BORDER_RIGHT | BORDER_LEFT;
	GUI_AddButton(&button);

	/* UART1 Button */
	button.object.id = guiConfigUART1_BUTTON_ID;
	button.enabledBackgroundColor = button.disabledTextColor = button.pressedTextColor = GUI_GREEN;
	button.text = "UART1";
	button.textSize = ENLARGE_2X;
	button.object.xPos = 200;
	GUI_AddButton(&button);

	/* UART2 Button */
	button.object.id = guiConfigUART2_BUTTON_ID;
	button.enabledBackgroundColor = button.disabledTextColor = button.pressedTextColor = GUI_YELLOW;
	button.text = "UART2";
	button.textSize = ENLARGE_2X;
	button.object.xPos = 300;
	GUI_AddButton(&button);

	/* RS232 Button */
	button.object.id = guiConfigRS232_BUTTON_ID;
	button.enabledBackgroundColor = button.disabledTextColor = button.pressedTextColor = GUI_PURPLE;
	button.text = "RS232";
	button.textSize = ENLARGE_2X;
	button.object.xPos = 400;
	GUI_AddButton(&button);

	/* I2C Button */
	button.object.id = guiConfigI2C_BUTTON_ID;
	button.enabledBackgroundColor = button.disabledTextColor = button.pressedTextColor = GUI_GRAY;
	button.text = "I2C";
	button.textSize = ENLARGE_2X;
	button.object.xPos = 500;
	GUI_AddButton(&button);

	/* ADC Button */
	button.object.id = guiConfigADC_BUTTON_ID;
	button.enabledBackgroundColor = button.disabledTextColor = button.pressedTextColor = GUI_MAGENTA;
	button.text = "ADC";
	button.textSize = ENLARGE_1X;
	button.object.xPos = 600;
	button.object.yPos = 0;
	button.object.width = 50;
	button.object.height = 25;
	button.object.border = BORDER_RIGHT | BORDER_LEFT;
	GUI_AddButton(&button);

	/* GPIO Button */
	button.object.id = guiConfigGPIO_BUTTON_ID;
	button.enabledBackgroundColor = button.disabledTextColor = button.pressedTextColor = GUI_CYAN;
	button.text = "GPIO";
	button.textSize = ENLARGE_1X;
	button.object.yPos = 25;
	button.object.height = 25;
	button.object.border = BORDER_BOTTOM | BORDER_RIGHT | BORDER_LEFT;
	GUI_AddButton(&button);

	/* Settings Button */
	button.object.id = guiConfigSETTINGS_BUTTON_ID;
	button.disabledBackgroundColor = button.enabledBackgroundColor = button.pressedTextColor = GUI_DARK_BLUE;
	button.disabledTextColor = LCD_COLOR_WHITE;
	button.text = "Settings";
	button.textSize = ENLARGE_2X;
	button.object.xPos = 650;
	button.object.yPos = 430;
	button.object.width = 150;
	button.object.height = 50;
	button.object.border = BORDER_TOP | BORDER_LEFT;
	GUI_AddButton(&button);

	/* Storage Button */
	button.object.id = guiConfigSTORAGE_BUTTON_ID;
	button.disabledBackgroundColor = button.enabledBackgroundColor = button.pressedTextColor = GUI_DARK_BLUE;
	button.disabledTextColor = LCD_COLOR_WHITE;
	button.text = "Storage";
	button.textSize = ENLARGE_2X;
	button.object.xPos = 650;
	button.object.yPos = 380;
	button.object.width = 150;
	button.object.height = 50;
	button.object.border = BORDER_TOP | BORDER_BOTTOM | BORDER_LEFT;
	GUI_AddButton(&button);

	GUI_DrawAllButtons();

//	LCD_SetForegroundColor(LCD_COLOR_RED);
//	LCD_DrawSquareOrLine(0, 799, 50, 50, LINE, FILLED);
}

static void guiTest()
{
	static uint32_t index = 0;
	static uint32_t state = ENABLED;

	GUI_SetButtonState(index, state);

	index++;
	if (index >= guiConfigNUMBER_OF_BUTTONS)
	{
		index = 0;
		state++;
		if (state > PRESSED)
			state = 0;
	}

	vTaskDelay(1000 / portTICK_PERIOD_MS);
}

/* Interrupt Handlers --------------------------------------------------------*/
