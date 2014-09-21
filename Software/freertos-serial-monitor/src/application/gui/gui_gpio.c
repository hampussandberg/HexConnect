/**
 ******************************************************************************
 * @file	gui_gpio.c
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
#include "gui_gpio.h"

/* Private defines -----------------------------------------------------------*/
/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static GUITextBox prvTextBox = {0};
static GUIButton prvButton = {0};
static GUIContainer prvContainer = {0};

/* Private function prototypes -----------------------------------------------*/
/* Functions -----------------------------------------------------------------*/
/* GPIO GUI Elements ========================================================*/
/**
 * @brief	Manages how data is displayed in the main text box when the source is GPIO
 * @param	None
 * @retval	None
 */
void guiGpioManageMainTextBox()
{

}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiGpioTopButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(guiConfigSIDEBAR_GPIO_CONTAINER_ID);
		lcdChangeDisplayStateOfSidebar(guiConfigSIDEBAR_GPIO_CONTAINER_ID);

		if (displayState == GUIDisplayState_Hidden)
		{
			GUI_DrawContainer(guiConfigMAIN_GPIO0_CONTAINER_ID);
			GUI_DrawContainer(guiConfigMAIN_GPIO1_CONTAINER_ID);
		}
		else if (displayState == GUIDisplayState_NotHidden)
		{
			GUI_HideContainer(guiConfigMAIN_GPIO0_CONTAINER_ID);
			GUI_HideContainer(guiConfigMAIN_GPIO1_CONTAINER_ID);
		}
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiGpio0TypeButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(guiConfigPOPOUT_GPIO0_TYPE_CONTAINER_ID);

		if (displayState == GUIDisplayState_Hidden)
		{
			GUI_SetActiveLayer(GUILayer_1);
			GUI_SetLayerForButton(guiConfigGPIO0_TYPE_BUTTON_ID, GUILayer_1);
			GUI_SetButtonState(guiConfigGPIO0_TYPE_BUTTON_ID, GUIButtonState_Enabled);
			GUI_DrawContainer(guiConfigPOPOUT_GPIO0_TYPE_CONTAINER_ID);
		}
		else if (displayState == GUIDisplayState_NotHidden)
		{
			GUI_HideContainer(guiConfigPOPOUT_GPIO0_TYPE_CONTAINER_ID);
			GUI_SetActiveLayer(GUILayer_0);
			GUI_SetLayerForButton(guiConfigGPIO0_TYPE_BUTTON_ID, GUILayer_0);
			GUI_SetButtonState(guiConfigGPIO0_TYPE_BUTTON_ID, GUIButtonState_Disabled);

			/* Refresh the main container */
			GUI_DrawContainer(guiConfigMAIN_CONTENT_CONTAINER_ID);
		}
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiGpio1TypeButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(guiConfigPOPOUT_GPIO1_TYPE_CONTAINER_ID);

		if (displayState == GUIDisplayState_Hidden)
		{
			GUI_SetActiveLayer(GUILayer_1);
			GUI_SetLayerForButton(guiConfigGPIO1_TYPE_BUTTON_ID, GUILayer_1);
			GUI_SetButtonState(guiConfigGPIO1_TYPE_BUTTON_ID, GUIButtonState_Enabled);
			GUI_DrawContainer(guiConfigPOPOUT_GPIO1_TYPE_CONTAINER_ID);
		}
		else if (displayState == GUIDisplayState_NotHidden)
		{
			GUI_HideContainer(guiConfigPOPOUT_GPIO1_TYPE_CONTAINER_ID);
			GUI_SetActiveLayer(GUILayer_0);
			GUI_SetLayerForButton(guiConfigGPIO1_TYPE_BUTTON_ID, GUILayer_0);
			GUI_SetButtonState(guiConfigGPIO1_TYPE_BUTTON_ID, GUIButtonState_Disabled);

			/* Refresh the main container */
			GUI_DrawContainer(guiConfigMAIN_CONTENT_CONTAINER_ID);
		}
	}
}

/**
 * @brief
 * @param	None
 * @retval	None
 */
void guiGpioInitGuiElements()
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

	/* GPIO0 Value text box */
	prvTextBox.object.id = guiConfigGPIO0_VALUE_TEXT_BOX_ID;
	prvTextBox.object.xPos = 450;
	prvTextBox.object.yPos = 87;
	prvTextBox.object.width = 150;
	prvTextBox.object.height = 150;
	prvTextBox.object.layer = GUILayer_0;
	prvTextBox.object.displayState = GUIDisplayState_Hidden;
	prvTextBox.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left | GUIBorder_Right;
	prvTextBox.object.borderThickness = 1;
	prvTextBox.object.borderColor = GUI_WHITE;
	prvTextBox.textColor = GUI_CYAN_LIGHT;
	prvTextBox.backgroundColor = GUI_WHITE;
	prvTextBox.staticText = "HIGH";
	prvTextBox.textSize = LCDFontEnlarge_3x;
	GUI_AddTextBox(&prvTextBox);

	/* GPIO1 Value text box */
	prvTextBox.object.id = guiConfigGPIO1_VALUE_TEXT_BOX_ID;
	prvTextBox.object.xPos = 450;
	prvTextBox.object.yPos = 287;
	prvTextBox.object.width = 150;
	prvTextBox.object.height = 150;
	prvTextBox.object.layer = GUILayer_0;
	prvTextBox.object.displayState = GUIDisplayState_Hidden;
	prvTextBox.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left | GUIBorder_Right;
	prvTextBox.object.borderThickness = 1;
	prvTextBox.object.borderColor = GUI_WHITE;
	prvTextBox.textColor = GUI_CYAN_DARK;
	prvTextBox.backgroundColor = GUI_WHITE;
	prvTextBox.staticText = "LOW";
	prvTextBox.textSize = LCDFontEnlarge_3x;
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
	prvButton.touchCallback = guiGpioTopButtonCallback;
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
	prvButton.enabledBackgroundColor = GUI_CYAN;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_CYAN_LIGHT;
	prvButton.pressedTextColor = GUI_CYAN_LIGHT;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiGpio0TypeButtonCallback;
	prvButton.text[0] = "< Ch0 Type:";
	prvButton.text[1] = "Output";
//	prvButton.text[1] = "Input ";
//	prvButton.text[1] = "PWM   ";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* GPIO1 Type Button */
	prvButton.object.id = guiConfigGPIO1_TYPE_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_CYAN_VERY_DARK;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_CYAN_DARK;
	prvButton.pressedTextColor = GUI_CYAN_DARK;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiGpio1TypeButtonCallback;
	prvButton.text[0] = "< Ch1 Type:";
	prvButton.text[1] = "Output";
//	prvButton.text[1] = "Input ";
//	prvButton.text[1] = "PWM   ";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* GPIO0 Output type Button */
	prvButton.object.id = guiConfigGPIO0_TYPE_OUT_BUTTON_ID;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 100;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.displayState = GUIDisplayState_Hidden;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
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
	prvButton.text[0] = "Output";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* GPIO0 Input type Button */
	prvButton.object.id = guiConfigGPIO0_TYPE_IN_BUTTON_ID;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 140;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.displayState = GUIDisplayState_Hidden;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
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
	prvButton.text[0] = "Input";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* GPIO0 PWM type Button */
	prvButton.object.id = guiConfigGPIO0_TYPE_PWM_BUTTON_ID;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 180;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.displayState = GUIDisplayState_Hidden;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
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
	prvButton.text[0] = "PWM";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* GPIO1 Output type Button */
	prvButton.object.id = guiConfigGPIO1_TYPE_OUT_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_CYAN_DARK;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_CYAN_DARK;
	prvButton.pressedTextColor = GUI_CYAN_DARK;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = 0;
	prvButton.text[0] = "Output";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* GPIO1 Input type Button */
	prvButton.object.id = guiConfigGPIO1_TYPE_IN_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_CYAN_DARK;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_CYAN_DARK;
	prvButton.pressedTextColor = GUI_CYAN_DARK;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = 0;
	prvButton.text[0] = "Input";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* GPIO1 PWM type Button */
	prvButton.object.id = guiConfigGPIO1_TYPE_PWM_BUTTON_ID;
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
	prvButton.enabledBackgroundColor = GUI_CYAN_DARK;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_CYAN_DARK;
	prvButton.pressedTextColor = GUI_CYAN_DARK;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = 0;
	prvButton.text[0] = "PWM";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
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
	prvContainer.buttons[1] = GUI_GetButtonFromId(guiConfigGPIO1_TYPE_BUTTON_ID);
	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(guiConfigGPIO_LABEL_TEXT_BOX_ID);
	GUI_AddContainer(&prvContainer);

	/* GPIO0 type popout container */
	prvContainer.object.id = guiConfigPOPOUT_GPIO0_TYPE_CONTAINER_ID;
	prvContainer.object.xPos = 500;
	prvContainer.object.yPos = 100;
	prvContainer.object.width = 149;
	prvContainer.object.height = 120;
	prvContainer.object.layer = GUILayer_1;
	prvContainer.object.displayState = GUIDisplayState_Hidden;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 2;
	prvContainer.object.borderColor = GUI_WHITE;
	prvContainer.contentHideState = GUIHideState_HideAll;
	prvContainer.buttons[0] = GUI_GetButtonFromId(guiConfigGPIO0_TYPE_OUT_BUTTON_ID);
	prvContainer.buttons[1] = GUI_GetButtonFromId(guiConfigGPIO0_TYPE_IN_BUTTON_ID);
	prvContainer.buttons[2] = GUI_GetButtonFromId(guiConfigGPIO0_TYPE_PWM_BUTTON_ID);
	GUI_AddContainer(&prvContainer);

	/* GPIO1 type popout container */
	prvContainer.object.id = guiConfigPOPOUT_GPIO1_TYPE_CONTAINER_ID;
	prvContainer.object.xPos = 500;
	prvContainer.object.yPos = 150;
	prvContainer.object.width = 149;
	prvContainer.object.height = 120;
	prvContainer.object.layer = GUILayer_1;
	prvContainer.object.displayState = GUIDisplayState_Hidden;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 2;
	prvContainer.object.borderColor = GUI_WHITE;
	prvContainer.contentHideState = GUIHideState_HideAll;
	prvContainer.buttons[0] = GUI_GetButtonFromId(guiConfigGPIO1_TYPE_OUT_BUTTON_ID);
	prvContainer.buttons[1] = GUI_GetButtonFromId(guiConfigGPIO1_TYPE_IN_BUTTON_ID);
	prvContainer.buttons[2] = GUI_GetButtonFromId(guiConfigGPIO1_TYPE_PWM_BUTTON_ID);
	GUI_AddContainer(&prvContainer);

	/* GPIO0 main container */
	prvContainer.object.id = guiConfigMAIN_GPIO0_CONTAINER_ID;
	prvContainer.object.xPos = 25;
	prvContainer.object.yPos = 75;
	prvContainer.object.width = 600;
	prvContainer.object.height = 175;
	prvContainer.object.layer = GUILayer_0;
	prvContainer.object.containerPage = guiConfigMAIN_CONTAINER_GPIO_PAGE;
	prvContainer.object.displayState = GUIDisplayState_Hidden;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Right | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 2;
	prvContainer.object.borderColor = GUI_WHITE;
	prvContainer.backgroundColor = GUI_CYAN_LIGHT;
	prvContainer.contentHideState = GUIHideState_HideAll;
	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(guiConfigGPIO0_VALUE_TEXT_BOX_ID);
	GUI_AddContainer(&prvContainer);

	/* GPIO1 main container */
	prvContainer.object.id = guiConfigMAIN_GPIO1_CONTAINER_ID;
	prvContainer.object.xPos = 25;
	prvContainer.object.yPos = 275;
	prvContainer.object.width = 600;
	prvContainer.object.height = 175;
	prvContainer.object.layer = GUILayer_0;
	prvContainer.object.containerPage = guiConfigMAIN_CONTAINER_GPIO_PAGE;
	prvContainer.object.displayState = GUIDisplayState_Hidden;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Right | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 2;
	prvContainer.object.borderColor = GUI_WHITE;
	prvContainer.backgroundColor = GUI_CYAN_DARK;
	prvContainer.contentHideState = GUIHideState_HideAll;
	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(guiConfigGPIO1_VALUE_TEXT_BOX_ID);
	GUI_AddContainer(&prvContainer);
}

/* Interrupt Handlers --------------------------------------------------------*/
