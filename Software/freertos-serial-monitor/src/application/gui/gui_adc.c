/**
 ******************************************************************************
 * @file	gui_adc.c
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
#include "gui_adc.h"

/* Private defines -----------------------------------------------------------*/
/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static GUITextBox prvTextBox = {0};
static GUIButton prvButton = {0};
static GUIContainer prvContainer = {0};

/* Private function prototypes -----------------------------------------------*/
/* Functions -----------------------------------------------------------------*/
/* ADC GUI Elements ========================================================*/
/**
 * @brief	Manages how data is displayed in the main text box when the source is ADC
 * @param	None
 * @retval	None
 */
void guiAdcManageMainTextBox()
{

}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiAdcTopButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(guiConfigSIDEBAR_ADC_CONTAINER_ID);
		lcdChangeDisplayStateOfSidebar(guiConfigSIDEBAR_ADC_CONTAINER_ID);
	}
}

/**
 * @brief
 * @param	None
 * @retval	None
 */
void guiAdcInitGuiElements()
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
	prvButton.touchCallback = guiAdcTopButtonCallback;
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
