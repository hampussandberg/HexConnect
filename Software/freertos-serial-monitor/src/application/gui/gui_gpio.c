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

static bool prvRefreshMainContent = false;

/* Private function prototypes -----------------------------------------------*/
void prvUpdateOutButtonTextToMatchRealOutput(uint32_t channel);
void prvEnableChannel(uint32_t channel);
void prvDisableChannel(uint32_t channel);
void prvUpdateDutyValuesInGui(uint32_t channel);

/* Functions -----------------------------------------------------------------*/
/* GPIO GUI Elements ========================================================*/
/**
 * @brief	Manages how data is displayed in the main text box when the source is GPIO
 * @param	None
 * @retval	None
 */
void guiGpioManageMainTextBox()
{
	if (gpio0IsEnabled() && gpio0GetDirection() == GPIODirection_Input)
	{
		static GPIO_PinState lastState = 3;
		GPIO_PinState newState = gpio0ReadPin();
		if (newState != lastState && newState == GPIO_PIN_SET)
		{
			GUI_SetStaticTextInTextBox(guiConfigGPIO0_VALUE_TEXT_BOX_ID, "High");
			lastState = newState;
		}
		else if (newState != lastState && newState == GPIO_PIN_RESET)
		{
			GUI_SetStaticTextInTextBox(guiConfigGPIO0_VALUE_TEXT_BOX_ID, "Low");
			lastState = newState;
		}
	}
	else if (gpio0IsEnabled() && gpio0GetDirection() == GPIODirection_OutputPWM)
	{
		if (prvRefreshMainContent)
			prvUpdateDutyValuesInGui(0);
	}

	if (gpio1IsEnabled() && gpio1GetDirection() == GPIODirection_Input)
	{
		static GPIO_PinState lastState = 3;
		GPIO_PinState newState = gpio1ReadPin();
		if (newState != lastState && newState == GPIO_PIN_SET)
		{
			GUI_SetStaticTextInTextBox(guiConfigGPIO1_VALUE_TEXT_BOX_ID, "High");
			lastState = newState;
		}
		else if (newState != lastState && newState == GPIO_PIN_RESET)
		{
			GUI_SetStaticTextInTextBox(guiConfigGPIO1_VALUE_TEXT_BOX_ID, "Low");
			lastState = newState;
		}
	}
	else if (gpio1IsEnabled() && gpio1GetDirection() == GPIODirection_OutputPWM)
	{
		if (prvRefreshMainContent)
			prvUpdateDutyValuesInGui(1);
	}

	prvRefreshMainContent = false;
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
			prvRefreshMainContent = true;
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
			prvRefreshMainContent = true;
		}
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiGpio0TypeSelectionCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		/* Hide the pop out */
		GUI_HideContainer(guiConfigPOPOUT_GPIO0_TYPE_CONTAINER_ID);
		GUI_SetActiveLayer(GUILayer_0);
		GUI_SetLayerForButton(guiConfigGPIO0_TYPE_BUTTON_ID, GUILayer_0);
		GUI_SetButtonState(guiConfigGPIO0_TYPE_BUTTON_ID, GUIButtonState_Disabled);

		/* Make sure to disable before changing type */
		prvDisableChannel(0);

		/* Do different things depending on which button was pressed */
		switch (ButtonId)
		{
			case guiConfigGPIO0_TYPE_OUT_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigGPIO0_TYPE_BUTTON_ID, "Output", 1);
				GUI_SetStaticTextInTextBox(guiConfigGPIO0_TYPE_TEXT_BOX_ID, "Output ->");
				GUI_ChangePageOfContainer(guiConfigMAIN_GPIO0_CONTAINER_ID, guiConfigGPIO_OUTPUT_PAGE);
				gpio0SetDirection(GPIODirection_Output);
				break;
			case guiConfigGPIO0_TYPE_IN_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigGPIO0_TYPE_BUTTON_ID, "Input", 1);
				GUI_SetStaticTextInTextBox(guiConfigGPIO0_TYPE_TEXT_BOX_ID, "Input <-");
				GUI_ChangePageOfContainer(guiConfigMAIN_GPIO0_CONTAINER_ID, guiConfigGPIO_INPUT_PAGE);
				gpio0SetDirection(GPIODirection_Input);
				break;
			case guiConfigGPIO0_TYPE_PWM_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigGPIO0_TYPE_BUTTON_ID, "PWM", 1);
				GUI_SetStaticTextInTextBox(guiConfigGPIO0_TYPE_TEXT_BOX_ID, "PWM |_|->");
				GUI_ChangePageOfContainer(guiConfigMAIN_GPIO0_CONTAINER_ID, guiConfigGPIO_PWM_PAGE);
				gpio0SetDirection(GPIODirection_OutputPWM);
				break;
			default:
				break;
		}

		/* Refresh the main container */
		GUI_DrawContainer(guiConfigMAIN_CONTENT_CONTAINER_ID);
		prvRefreshMainContent = true;
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiGpio1TypeSelectionCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		/* Hide the pop out */
		GUI_HideContainer(guiConfigPOPOUT_GPIO1_TYPE_CONTAINER_ID);
		GUI_SetActiveLayer(GUILayer_0);
		GUI_SetLayerForButton(guiConfigGPIO1_TYPE_BUTTON_ID, GUILayer_0);
		GUI_SetButtonState(guiConfigGPIO1_TYPE_BUTTON_ID, GUIButtonState_Disabled);

		/* Make sure to disable before changing type */
		prvDisableChannel(1);

		/* Do different things depending on which button was pressed */
		switch (ButtonId)
		{
			case guiConfigGPIO1_TYPE_OUT_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigGPIO1_TYPE_BUTTON_ID, "Output", 1);
				GUI_SetStaticTextInTextBox(guiConfigGPIO1_TYPE_TEXT_BOX_ID, "Output ->");
				GUI_ChangePageOfContainer(guiConfigMAIN_GPIO1_CONTAINER_ID, guiConfigGPIO_OUTPUT_PAGE);
				gpio1SetDirection(GPIODirection_Output);
				break;
			case guiConfigGPIO1_TYPE_IN_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigGPIO1_TYPE_BUTTON_ID, "Input", 1);
				GUI_SetStaticTextInTextBox(guiConfigGPIO1_TYPE_TEXT_BOX_ID, "Input <-");
				GUI_ChangePageOfContainer(guiConfigMAIN_GPIO1_CONTAINER_ID, guiConfigGPIO_INPUT_PAGE);
				gpio1SetDirection(GPIODirection_Input);
				break;
			case guiConfigGPIO1_TYPE_PWM_BUTTON_ID:
				GUI_SetButtonTextForRow(guiConfigGPIO1_TYPE_BUTTON_ID, "PWM", 1);
				GUI_SetStaticTextInTextBox(guiConfigGPIO1_TYPE_TEXT_BOX_ID, "PWM |_|->");
				GUI_ChangePageOfContainer(guiConfigMAIN_GPIO1_CONTAINER_ID, guiConfigGPIO_PWM_PAGE);
				gpio1SetDirection(GPIODirection_OutputPWM);
				break;
			default:
				break;
		}

		/* Refresh the main container */
		GUI_DrawContainer(guiConfigMAIN_CONTENT_CONTAINER_ID);
		prvRefreshMainContent = true;
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiGpioEnableCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		/* Do different things depending on which button was pressed */
		switch (ButtonId)
		{
			case guiConfigGPIO0_ENABLE_BUTTON_ID:
				if (gpio0IsEnabled())
					prvDisableChannel(0);
				else
					prvEnableChannel(0);
				break;
			case guiConfigGPIO1_ENABLE_BUTTON_ID:
				if (gpio1IsEnabled())
					prvDisableChannel(1);
				else
					prvEnableChannel(1);
				break;
			default:
				break;
		}

		/* Update the state of the top button */
		if (gpio0IsEnabled() || gpio1IsEnabled())
			GUI_SetButtonState(guiConfigGPIO_TOP_BUTTON_ID, GUIButtonState_Enabled);
		else
			GUI_SetButtonState(guiConfigGPIO_TOP_BUTTON_ID, GUIButtonState_Disabled);

		prvRefreshMainContent = true;
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiGpioOutPinCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		uint32_t channel = 2;
		/* Do different things depending on which button was pressed */
		switch (ButtonId)
		{
			case guiConfigGPIO0_OUT_HIGH_BUTTON_ID:
				gpio0WritePin(GPIO_PIN_SET);
				channel = 0;
				break;
			case guiConfigGPIO0_OUT_TOGGLE_BUTTON_ID:
				gpio0TogglePin();
				channel = 0;
				break;
			case guiConfigGPIO0_OUT_LOW_BUTTON_ID:
				gpio0WritePin(GPIO_PIN_RESET);
				channel = 0;
				break;

			case guiConfigGPIO1_OUT_HIGH_BUTTON_ID:
				gpio1WritePin(GPIO_PIN_SET);
				channel = 1;
				break;
			case guiConfigGPIO1_OUT_TOGGLE_BUTTON_ID:
				gpio1TogglePin();
				channel = 1;
				break;
			case guiConfigGPIO1_OUT_LOW_BUTTON_ID:
				gpio1WritePin(GPIO_PIN_RESET);
				channel = 1;
				break;
			default:
				break;
		}
		/* Update button text */
		prvUpdateOutButtonTextToMatchRealOutput(channel);
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiGpioDutyCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	static TickType_t lastDutyUpdate = 0;

	if ((Event == GUITouchEvent_Up || Event == GUITouchEvent_Down) && xTaskGetTickCount() - lastDutyUpdate > 100)
	{
		lastDutyUpdate = xTaskGetTickCount();

		float currentDutyCh0 = gpio0GetPwmDuty();
		float currentDutyCh1 = gpio1GetPwmDuty();
		/* Do different things depending on which button was pressed */
		switch (ButtonId)
		{
			/* CH0 Up */
			case guiConfigGPIO0_PWM_DUTY_UP_BUTTON_ID:
				currentDutyCh0 += 1.0;
				if (currentDutyCh0 > 100.0)
					currentDutyCh0 = 100.0;
				gpio0SetPwmDuty(currentDutyCh0);
				/* Update the text box */
				prvUpdateDutyValuesInGui(0);
				break;

			/* CH0 Down */
			case guiConfigGPIO0_PWM_DUTY_DOWN_BUTTON_ID:
				currentDutyCh0 -= 1.0;
				if (currentDutyCh0 < 0)
					currentDutyCh0 = 0.0;
				gpio0SetPwmDuty(currentDutyCh0);
				/* Update the text box */
				prvUpdateDutyValuesInGui(0);
				break;

			/* CH1 Up */
			case guiConfigGPIO1_PWM_DUTY_UP_BUTTON_ID:
				currentDutyCh1 += 1.0;
				if (currentDutyCh1 > 100.0)
					currentDutyCh1 = 100.0;
				gpio1SetPwmDuty(currentDutyCh1);
				/* Update the text box */
				prvUpdateDutyValuesInGui(1);
				break;

			/* CH1 Down */
			case guiConfigGPIO1_PWM_DUTY_DOWN_BUTTON_ID:
				currentDutyCh1 -= 1.0;
				if (currentDutyCh1 < 0)
					currentDutyCh1 = 0.0;
				gpio1SetPwmDuty(currentDutyCh1);
				/* Update the text box */
				prvUpdateDutyValuesInGui(1);
				break;

			default:
				break;
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
	prvTextBox.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvTextBox.object.borderThickness = 1;
	prvTextBox.object.borderColor = GUI_WHITE;
	prvTextBox.object.containerPage = GUIContainerPage_All;
	prvTextBox.textColor = GUI_CYAN_LIGHT;
	prvTextBox.backgroundColor = GUI_WHITE;
	prvTextBox.staticText = "GPIO";
	prvTextBox.textSize = LCDFontEnlarge_2x;
	GUI_AddTextBox(&prvTextBox);

	/* GPIO0 Label text box */
	prvTextBox.object.id = guiConfigGPIO0_LABEL_TEXT_BOX_ID;
	prvTextBox.object.xPos = 30;
	prvTextBox.object.yPos = 80;
	prvTextBox.object.width = 100;
	prvTextBox.object.height = 50;
	prvTextBox.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left | GUIBorder_Right;
	prvTextBox.object.borderThickness = 1;
	prvTextBox.object.borderColor = GUI_WHITE;
	prvTextBox.object.containerPage = GUIContainerPage_All;
	prvTextBox.textColor = GUI_CYAN_LIGHT;
	prvTextBox.backgroundColor = GUI_WHITE;
	prvTextBox.staticText = "GPIO0";
	prvTextBox.textSize = LCDFontEnlarge_2x;
	GUI_AddTextBox(&prvTextBox);

	/* GPIO0 type text box */
	prvTextBox.object.id = guiConfigGPIO0_TYPE_TEXT_BOX_ID;
	prvTextBox.object.xPos = 30;
	prvTextBox.object.yPos = 135;
	prvTextBox.object.width = 100;
	prvTextBox.object.height = 30;
	prvTextBox.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left | GUIBorder_Right;
	prvTextBox.object.borderThickness = 1;
	prvTextBox.object.borderColor = GUI_WHITE;
	prvTextBox.object.containerPage = GUIContainerPage_All;
	prvTextBox.textColor = GUI_CYAN_LIGHT;
	prvTextBox.backgroundColor = GUI_WHITE;
	prvTextBox.staticText = "Output ->";
	prvTextBox.textSize = LCDFontEnlarge_1x;
	GUI_AddTextBox(&prvTextBox);

	/* GPIO1 Label text box */
	prvTextBox.object.id = guiConfigGPIO1_LABEL_TEXT_BOX_ID;
	prvTextBox.object.xPos = 30;
	prvTextBox.object.yPos = 280;
	prvTextBox.object.width = 100;
	prvTextBox.object.height = 50;
	prvTextBox.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left | GUIBorder_Right;
	prvTextBox.object.borderThickness = 1;
	prvTextBox.object.borderColor = GUI_WHITE;
	prvTextBox.object.containerPage = GUIContainerPage_All;
	prvTextBox.textColor = GUI_CYAN_DARK;
	prvTextBox.backgroundColor = GUI_WHITE;
	prvTextBox.staticText = "GPIO1";
	prvTextBox.textSize = LCDFontEnlarge_2x;
	GUI_AddTextBox(&prvTextBox);

	/* GPIO1 type text box */
	prvTextBox.object.id = guiConfigGPIO1_TYPE_TEXT_BOX_ID;
	prvTextBox.object.xPos = 30;
	prvTextBox.object.yPos = 335;
	prvTextBox.object.width = 100;
	prvTextBox.object.height = 30;
	prvTextBox.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left | GUIBorder_Right;
	prvTextBox.object.borderThickness = 1;
	prvTextBox.object.borderColor = GUI_WHITE;
	prvTextBox.object.containerPage = GUIContainerPage_All;
	prvTextBox.textColor = GUI_CYAN_DARK;
	prvTextBox.backgroundColor = GUI_WHITE;
	prvTextBox.staticText = "Output ->";
	prvTextBox.textSize = LCDFontEnlarge_1x;
	GUI_AddTextBox(&prvTextBox);



	/* GPIO0 Value label text box */
	prvTextBox.object.id = guiConfigGPIO0_VALUE_LABEL_TEXT_BOX_ID;
	prvTextBox.object.xPos = 400;
	prvTextBox.object.yPos = 80;
	prvTextBox.object.width = 150;
	prvTextBox.object.height = 50;
	prvTextBox.object.containerPage = guiConfigGPIO_INPUT_PAGE;
	prvTextBox.textColor = GUI_WHITE;
	prvTextBox.backgroundColor = GUI_CYAN_LIGHT;
	prvTextBox.staticText = "Current value:";
	prvTextBox.textSize = LCDFontEnlarge_1x;
	GUI_AddTextBox(&prvTextBox);

	/* GPIO0 Value text box */
	prvTextBox.object.id = guiConfigGPIO0_VALUE_TEXT_BOX_ID;
	prvTextBox.object.xPos = 400;
	prvTextBox.object.yPos = 135;
	prvTextBox.object.width = 150;
	prvTextBox.object.height = 100;
	prvTextBox.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left | GUIBorder_Right;
	prvTextBox.object.borderThickness = 1;
	prvTextBox.object.borderColor = GUI_WHITE;
	prvTextBox.object.containerPage = guiConfigGPIO_INPUT_PAGE;
	prvTextBox.textColor = GUI_CYAN_LIGHT;
	prvTextBox.backgroundColor = GUI_WHITE;
	prvTextBox.staticText = "Unknown";
	prvTextBox.textSize = LCDFontEnlarge_2x;
	GUI_AddTextBox(&prvTextBox);

	/* GPIO0 Duty value text box */
	prvTextBox.object.id = guiConfigGPIO0_DUTY_VALUE_TEXT_BOX_ID;
	prvTextBox.object.xPos = 410;
	prvTextBox.object.yPos = 135;
	prvTextBox.object.width = 150;
	prvTextBox.object.height = 50;
	prvTextBox.object.containerPage = guiConfigGPIO_PWM_PAGE;
	prvTextBox.textColor = GUI_WHITE;
	prvTextBox.backgroundColor = GUI_CYAN_LIGHT;
	prvTextBox.textSize = LCDFontEnlarge_2x;
	GUI_AddTextBox(&prvTextBox);

	/* GPIO0 Frequency value text box */
	prvTextBox.object.id = guiConfigGPIO0_FREQ_VALUE_TEXT_BOX_ID;
	prvTextBox.object.xPos = 410;
	prvTextBox.object.yPos = 190;
	prvTextBox.object.width = 150;
	prvTextBox.object.height = 50;
	prvTextBox.object.containerPage = guiConfigGPIO_PWM_PAGE;
	prvTextBox.textColor = GUI_WHITE;
	prvTextBox.backgroundColor = GUI_CYAN_LIGHT;
	prvTextBox.textSize = LCDFontEnlarge_2x;
	GUI_AddTextBox(&prvTextBox);

	/* GPIO0 Duty label text box */
	prvTextBox.object.id = guiConfigGPIO0_DUTY_TEXT_BOX_ID;
	prvTextBox.object.xPos = 200;
	prvTextBox.object.yPos = 135;
	prvTextBox.object.width = 150;
	prvTextBox.object.height = 50;
	prvTextBox.object.containerPage = guiConfigGPIO_PWM_PAGE;
	prvTextBox.textColor = GUI_WHITE;
	prvTextBox.backgroundColor = GUI_CYAN_LIGHT;
	prvTextBox.staticText = "Duty:";
	prvTextBox.textSize = LCDFontEnlarge_2x;
	GUI_AddTextBox(&prvTextBox);

	/* GPIO0 Frequency label text box */
	prvTextBox.object.id = guiConfigGPIO0_FREQ_TEXT_BOX_ID;
	prvTextBox.object.xPos = 200;
	prvTextBox.object.yPos = 190;
	prvTextBox.object.width = 150;
	prvTextBox.object.height = 50;
	prvTextBox.object.containerPage = guiConfigGPIO_PWM_PAGE;
	prvTextBox.textColor = GUI_WHITE;
	prvTextBox.backgroundColor = GUI_CYAN_LIGHT;
	prvTextBox.staticText = "Freq:";
	prvTextBox.textSize = LCDFontEnlarge_2x;
	GUI_AddTextBox(&prvTextBox);



	/* GPIO1 Value label text box */
	prvTextBox.object.id = guiConfigGPIO1_VALUE_LABEL_TEXT_BOX_ID;
	prvTextBox.object.xPos = 400;
	prvTextBox.object.yPos = 280;
	prvTextBox.object.width = 150;
	prvTextBox.object.height = 50;
	prvTextBox.object.containerPage = guiConfigGPIO_INPUT_PAGE;
	prvTextBox.textColor = GUI_WHITE;
	prvTextBox.backgroundColor = GUI_CYAN_DARK;
	prvTextBox.staticText = "Current value:";
	prvTextBox.textSize = LCDFontEnlarge_1x;
	GUI_AddTextBox(&prvTextBox);

	/* GPIO1 Value text box */
	prvTextBox.object.id = guiConfigGPIO1_VALUE_TEXT_BOX_ID;
	prvTextBox.object.xPos = 400;
	prvTextBox.object.yPos = 335;
	prvTextBox.object.width = 150;
	prvTextBox.object.height = 100;
	prvTextBox.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left | GUIBorder_Right;
	prvTextBox.object.borderThickness = 1;
	prvTextBox.object.borderColor = GUI_WHITE;
	prvTextBox.object.containerPage = guiConfigGPIO_INPUT_PAGE;
	prvTextBox.textColor = GUI_CYAN_DARK;
	prvTextBox.backgroundColor = GUI_WHITE;
	prvTextBox.staticText = "Unknown";
	prvTextBox.textSize = LCDFontEnlarge_2x;
	GUI_AddTextBox(&prvTextBox);

	/* GPIO1 Duty value text box */
	prvTextBox.object.id = guiConfigGPIO1_DUTY_VALUE_TEXT_BOX_ID;
	prvTextBox.object.xPos = 410;
	prvTextBox.object.yPos = 335;
	prvTextBox.object.width = 150;
	prvTextBox.object.height = 50;
	prvTextBox.object.containerPage = guiConfigGPIO_PWM_PAGE;
	prvTextBox.textColor = GUI_WHITE;
	prvTextBox.backgroundColor = GUI_CYAN_DARK;
	prvTextBox.textSize = LCDFontEnlarge_2x;
	GUI_AddTextBox(&prvTextBox);

	/* GPIO1 Frequency value text box */
	prvTextBox.object.id = guiConfigGPIO1_FREQ_VALUE_TEXT_BOX_ID;
	prvTextBox.object.xPos = 410;
	prvTextBox.object.yPos = 390;
	prvTextBox.object.width = 150;
	prvTextBox.object.height = 50;
	prvTextBox.object.containerPage = guiConfigGPIO_PWM_PAGE;
	prvTextBox.textColor = GUI_WHITE;
	prvTextBox.backgroundColor = GUI_CYAN_DARK;
	prvTextBox.textSize = LCDFontEnlarge_2x;
	GUI_AddTextBox(&prvTextBox);

	/* GPIO1 Duty label text box */
	prvTextBox.object.id = guiConfigGPIO1_DUTY_TEXT_BOX_ID;
	prvTextBox.object.xPos = 200;
	prvTextBox.object.yPos = 335;
	prvTextBox.object.width = 150;
	prvTextBox.object.height = 50;
	prvTextBox.object.containerPage = guiConfigGPIO_PWM_PAGE;
	prvTextBox.textColor = GUI_WHITE;
	prvTextBox.backgroundColor = GUI_CYAN_DARK;
	prvTextBox.staticText = "Duty:";
	prvTextBox.textSize = LCDFontEnlarge_2x;
	GUI_AddTextBox(&prvTextBox);

	/* GPIO1 Frequency label text box */
	prvTextBox.object.id = guiConfigGPIO1_FREQ_TEXT_BOX_ID;
	prvTextBox.object.xPos = 200;
	prvTextBox.object.yPos = 390;
	prvTextBox.object.width = 150;
	prvTextBox.object.height = 50;
	prvTextBox.object.containerPage = guiConfigGPIO_PWM_PAGE;
	prvTextBox.textColor = GUI_WHITE;
	prvTextBox.backgroundColor = GUI_CYAN_DARK;
	prvTextBox.staticText = "Freq:";
	prvTextBox.textSize = LCDFontEnlarge_2x;
	GUI_AddTextBox(&prvTextBox);

	/* Buttons -------------------------------------------------------------------*/
	/* GPIO Top Button */
	prvButton.object.id = guiConfigGPIO_TOP_BUTTON_ID;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 0;
	prvButton.object.width = 100;
	prvButton.object.height = 50;
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
	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.object.containerPage = GUIContainerPage_1;
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
//	prvButton.text[1] = "Input";
//	prvButton.text[1] = "PWM";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* GPIO1 Type Button */
	prvButton.object.id = guiConfigGPIO1_TYPE_BUTTON_ID;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 150;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.object.containerPage = GUIContainerPage_1;
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
//	prvButton.text[1] = "Input";
//	prvButton.text[1] = "PWM";
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
	prvButton.touchCallback = guiGpio0TypeSelectionCallback;
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
	prvButton.touchCallback = guiGpio0TypeSelectionCallback;
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
	prvButton.touchCallback = guiGpio0TypeSelectionCallback;
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
	prvButton.touchCallback = guiGpio1TypeSelectionCallback;
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
	prvButton.touchCallback = guiGpio1TypeSelectionCallback;
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
	prvButton.touchCallback = guiGpio1TypeSelectionCallback;
	prvButton.text[0] = "PWM";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);


	/* GPIO0 Enable Button */
	prvButton.object.id = guiConfigGPIO0_ENABLE_BUTTON_ID;
	prvButton.object.xPos = 200;
	prvButton.object.yPos = 80;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.containerPage = guiConfigGPIO_OUTPUT_PAGE | guiConfigGPIO_INPUT_PAGE | guiConfigGPIO_PWM_PAGE;
	prvButton.enabledTextColor = GUI_CYAN_LIGHT;
	prvButton.enabledBackgroundColor = GUI_CYAN_LIGHT;
	prvButton.disabledTextColor = GUI_CYAN_LIGHT;
	prvButton.disabledBackgroundColor = GUI_WHITE;
	prvButton.pressedTextColor = GUI_CYAN_LIGHT;
	prvButton.pressedBackgroundColor = GUI_CYAN;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiGpioEnableCallback;
	prvButton.text[0] = "Enable";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* GPIO0 Pull up Button */
	prvButton.object.id = guiConfigGPIO0_PULLUP_BUTTON_ID;
	prvButton.object.xPos = 200;
	prvButton.object.yPos = 135;
	prvButton.object.width = 150;
	prvButton.object.height = 30;
	prvButton.object.containerPage = guiConfigGPIO_INPUT_PAGE;
	prvButton.enabledTextColor = GUI_CYAN_LIGHT;
	prvButton.enabledBackgroundColor = GUI_CYAN_LIGHT;
	prvButton.disabledTextColor = GUI_CYAN_LIGHT;
	prvButton.disabledBackgroundColor = GUI_WHITE;
	prvButton.pressedTextColor = GUI_CYAN_LIGHT;
	prvButton.pressedBackgroundColor = GUI_CYAN;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = 0;
	prvButton.text[0] = "Pull Up";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* GPIO0 No pull Button */
	prvButton.object.id = guiConfigGPIO0_NOPULL_BUTTON_ID;
	prvButton.object.xPos = 200;
	prvButton.object.yPos = 170;
	prvButton.object.width = 150;
	prvButton.object.height = 30;
	prvButton.object.containerPage = guiConfigGPIO_INPUT_PAGE;
	prvButton.enabledTextColor = GUI_CYAN_LIGHT;
	prvButton.enabledBackgroundColor = GUI_CYAN_LIGHT;
	prvButton.disabledTextColor = GUI_CYAN_LIGHT;
	prvButton.disabledBackgroundColor = GUI_WHITE;
	prvButton.pressedTextColor = GUI_CYAN_LIGHT;
	prvButton.pressedBackgroundColor = GUI_CYAN;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = 0;
	prvButton.text[0] = "> No Pull";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* GPIO0 Pull down Button */
	prvButton.object.id = guiConfigGPIO0_PULLDOWN_BUTTON_ID;
	prvButton.object.xPos = 200;
	prvButton.object.yPos = 205;
	prvButton.object.width = 150;
	prvButton.object.height = 30;
	prvButton.object.containerPage = guiConfigGPIO_INPUT_PAGE;
	prvButton.enabledTextColor = GUI_CYAN_LIGHT;
	prvButton.enabledBackgroundColor = GUI_CYAN_LIGHT;
	prvButton.disabledTextColor = GUI_CYAN_LIGHT;
	prvButton.disabledBackgroundColor = GUI_WHITE;
	prvButton.pressedTextColor = GUI_CYAN_LIGHT;
	prvButton.pressedBackgroundColor = GUI_CYAN;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = 0;
	prvButton.text[0] = "Pull Down";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* GPIO0 Out high Button */
	prvButton.object.id = guiConfigGPIO0_OUT_HIGH_BUTTON_ID;
	prvButton.object.xPos = 400;
	prvButton.object.yPos = 80;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.containerPage = guiConfigGPIO_OUTPUT_PAGE;
	prvButton.enabledTextColor = GUI_CYAN_LIGHT;
	prvButton.enabledBackgroundColor = GUI_CYAN_LIGHT;
	prvButton.disabledTextColor = GUI_CYAN_LIGHT;
	prvButton.disabledBackgroundColor = GUI_WHITE;
	prvButton.pressedTextColor = GUI_CYAN_LIGHT;
	prvButton.pressedBackgroundColor = GUI_CYAN;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiGpioOutPinCallback;
	prvButton.text[0] = "High";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* GPIO0 Out toggle Button */
	prvButton.object.id = guiConfigGPIO0_OUT_TOGGLE_BUTTON_ID;
	prvButton.object.xPos = 400;
	prvButton.object.yPos = 135;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.containerPage = guiConfigGPIO_OUTPUT_PAGE;
	prvButton.enabledTextColor = GUI_CYAN_LIGHT;
	prvButton.enabledBackgroundColor = GUI_CYAN_LIGHT;
	prvButton.disabledTextColor = GUI_CYAN_LIGHT;
	prvButton.disabledBackgroundColor = GUI_WHITE;
	prvButton.pressedTextColor = GUI_CYAN_LIGHT;
	prvButton.pressedBackgroundColor = GUI_CYAN;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiGpioOutPinCallback;
	prvButton.text[0] = "Toggle";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* GPIO0 Out low Button */
	prvButton.object.id = guiConfigGPIO0_OUT_LOW_BUTTON_ID;
	prvButton.object.xPos = 400;
	prvButton.object.yPos = 190;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.containerPage = guiConfigGPIO_OUTPUT_PAGE;
	prvButton.enabledTextColor = GUI_CYAN_LIGHT;
	prvButton.enabledBackgroundColor = GUI_CYAN_LIGHT;
	prvButton.disabledTextColor = GUI_CYAN_LIGHT;
	prvButton.disabledBackgroundColor = GUI_WHITE;
	prvButton.pressedTextColor = GUI_CYAN_LIGHT;
	prvButton.pressedBackgroundColor = GUI_CYAN;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiGpioOutPinCallback;
	prvButton.text[0] = "Low";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* GPIO0 Duty Up Button */
	prvButton.object.id = guiConfigGPIO0_PWM_DUTY_UP_BUTTON_ID;
	prvButton.object.xPos = 570;
	prvButton.object.yPos = 135;
	prvButton.object.width = 50;
	prvButton.object.height = 50;
	prvButton.object.containerPage = guiConfigGPIO_PWM_PAGE;
	prvButton.enabledTextColor = GUI_CYAN_LIGHT;
	prvButton.enabledBackgroundColor = GUI_CYAN_LIGHT;
	prvButton.disabledTextColor = GUI_CYAN_LIGHT;
	prvButton.disabledBackgroundColor = GUI_WHITE;
	prvButton.pressedTextColor = GUI_CYAN_LIGHT;
	prvButton.pressedBackgroundColor = GUI_CYAN;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiGpioDutyCallback;
	prvButton.text[0] = "+";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* GPIO0 Duty Down Button */
	prvButton.object.id = guiConfigGPIO0_PWM_DUTY_DOWN_BUTTON_ID;
	prvButton.object.xPos = 350;
	prvButton.object.yPos = 135;
	prvButton.object.width = 50;
	prvButton.object.height = 50;
	prvButton.object.containerPage = guiConfigGPIO_PWM_PAGE;
	prvButton.enabledTextColor = GUI_CYAN_LIGHT;
	prvButton.enabledBackgroundColor = GUI_CYAN_LIGHT;
	prvButton.disabledTextColor = GUI_CYAN_LIGHT;
	prvButton.disabledBackgroundColor = GUI_WHITE;
	prvButton.pressedTextColor = GUI_CYAN_LIGHT;
	prvButton.pressedBackgroundColor = GUI_CYAN;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiGpioDutyCallback;
	prvButton.text[0] = "-";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* GPIO0 Frequency Up Button */
	prvButton.object.id = guiConfigGPIO0_PWM_FREQ_UP_BUTTON_ID;
	prvButton.object.xPos = 570;
	prvButton.object.yPos = 190;
	prvButton.object.width = 50;
	prvButton.object.height = 50;
	prvButton.object.containerPage = guiConfigGPIO_PWM_PAGE;
	prvButton.enabledTextColor = GUI_CYAN_LIGHT;
	prvButton.enabledBackgroundColor = GUI_CYAN_LIGHT;
	prvButton.disabledTextColor = GUI_CYAN_LIGHT;
	prvButton.disabledBackgroundColor = GUI_WHITE;
	prvButton.pressedTextColor = GUI_CYAN_LIGHT;
	prvButton.pressedBackgroundColor = GUI_CYAN;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = 0;
	prvButton.text[0] = "+";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* GPIO0 Frequency Down Button */
	prvButton.object.id = guiConfigGPIO0_PWM_FREQ_DOWN_BUTTON_ID;
	prvButton.object.xPos = 350;
	prvButton.object.yPos = 190;
	prvButton.object.width = 50;
	prvButton.object.height = 50;
	prvButton.object.containerPage = guiConfigGPIO_PWM_PAGE;
	prvButton.enabledTextColor = GUI_CYAN_LIGHT;
	prvButton.enabledBackgroundColor = GUI_CYAN_LIGHT;
	prvButton.disabledTextColor = GUI_CYAN_LIGHT;
	prvButton.disabledBackgroundColor = GUI_WHITE;
	prvButton.pressedTextColor = GUI_CYAN_LIGHT;
	prvButton.pressedBackgroundColor = GUI_CYAN;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = 0;
	prvButton.text[0] = "-";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);



	/* GPIO1 Enable Button */
	prvButton.object.id = guiConfigGPIO1_ENABLE_BUTTON_ID;
	prvButton.object.xPos = 200;
	prvButton.object.yPos = 280;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.containerPage = guiConfigGPIO_OUTPUT_PAGE | guiConfigGPIO_INPUT_PAGE | guiConfigGPIO_PWM_PAGE;
	prvButton.enabledTextColor = GUI_CYAN_DARK;
	prvButton.enabledBackgroundColor = GUI_CYAN_DARK;
	prvButton.disabledTextColor = GUI_CYAN_DARK;
	prvButton.disabledBackgroundColor = GUI_WHITE;
	prvButton.pressedTextColor = GUI_CYAN_DARK;
	prvButton.pressedBackgroundColor = GUI_CYAN_VERY_DARK;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiGpioEnableCallback;
	prvButton.text[0] = "Enable";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* GPIO1 Pull up Button */
	prvButton.object.id = guiConfigGPIO1_PULLUP_BUTTON_ID;
	prvButton.object.xPos = 200;
	prvButton.object.yPos = 335;
	prvButton.object.width = 150;
	prvButton.object.height = 30;
	prvButton.object.containerPage = guiConfigGPIO_INPUT_PAGE;
	prvButton.enabledTextColor = GUI_CYAN_DARK;
	prvButton.enabledBackgroundColor = GUI_CYAN_DARK;
	prvButton.disabledTextColor = GUI_CYAN_DARK;
	prvButton.disabledBackgroundColor = GUI_WHITE;
	prvButton.pressedTextColor = GUI_CYAN_DARK;
	prvButton.pressedBackgroundColor = GUI_CYAN_VERY_DARK;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = 0;
	prvButton.text[0] = "Pull Up";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* GPIO1 No pull Button */
	prvButton.object.id = guiConfigGPIO1_NOPULL_BUTTON_ID;
	prvButton.object.xPos = 200;
	prvButton.object.yPos = 370;
	prvButton.object.width = 150;
	prvButton.object.height = 30;
	prvButton.object.containerPage = guiConfigGPIO_INPUT_PAGE;
	prvButton.enabledTextColor = GUI_CYAN_DARK;
	prvButton.enabledBackgroundColor = GUI_CYAN_DARK;
	prvButton.disabledTextColor = GUI_CYAN_DARK;
	prvButton.disabledBackgroundColor = GUI_WHITE;
	prvButton.pressedTextColor = GUI_CYAN_DARK;
	prvButton.pressedBackgroundColor = GUI_CYAN_VERY_DARK;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = 0;
	prvButton.text[0] = "> No Pull";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* GPIO1 Pull down Button */
	prvButton.object.id = guiConfigGPIO1_PULLDOWN_BUTTON_ID;
	prvButton.object.xPos = 200;
	prvButton.object.yPos = 405;
	prvButton.object.width = 150;
	prvButton.object.height = 30;
	prvButton.object.containerPage = guiConfigGPIO_INPUT_PAGE;
	prvButton.enabledTextColor = GUI_CYAN_DARK;
	prvButton.enabledBackgroundColor = GUI_CYAN_DARK;
	prvButton.disabledTextColor = GUI_CYAN_DARK;
	prvButton.disabledBackgroundColor = GUI_WHITE;
	prvButton.pressedTextColor = GUI_CYAN_DARK;
	prvButton.pressedBackgroundColor = GUI_CYAN_VERY_DARK;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = 0;
	prvButton.text[0] = "Pull Down";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUI_AddButton(&prvButton);

	/* GPIO1 Out high Button */
	prvButton.object.id = guiConfigGPIO1_OUT_HIGH_BUTTON_ID;
	prvButton.object.xPos = 400;
	prvButton.object.yPos = 280;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.containerPage = guiConfigGPIO_OUTPUT_PAGE;
	prvButton.enabledTextColor = GUI_CYAN_DARK;
	prvButton.enabledBackgroundColor = GUI_CYAN_DARK;
	prvButton.disabledTextColor = GUI_CYAN_DARK;
	prvButton.disabledBackgroundColor = GUI_WHITE;
	prvButton.pressedTextColor = GUI_CYAN_DARK;
	prvButton.pressedBackgroundColor = GUI_CYAN_VERY_DARK;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiGpioOutPinCallback;
	prvButton.text[0] = "High";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* GPIO1 Out toggle Button */
	prvButton.object.id = guiConfigGPIO1_OUT_TOGGLE_BUTTON_ID;
	prvButton.object.xPos = 400;
	prvButton.object.yPos = 335;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.containerPage = guiConfigGPIO_OUTPUT_PAGE;
	prvButton.enabledTextColor = GUI_CYAN_DARK;
	prvButton.enabledBackgroundColor = GUI_CYAN_DARK;
	prvButton.disabledTextColor = GUI_CYAN_DARK;
	prvButton.disabledBackgroundColor = GUI_WHITE;
	prvButton.pressedTextColor = GUI_CYAN_DARK;
	prvButton.pressedBackgroundColor = GUI_CYAN_VERY_DARK;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiGpioOutPinCallback;
	prvButton.text[0] = "Toggle";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* GPIO1 Out low Button */
	prvButton.object.id = guiConfigGPIO1_OUT_LOW_BUTTON_ID;
	prvButton.object.xPos = 400;
	prvButton.object.yPos = 390;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.containerPage = guiConfigGPIO_OUTPUT_PAGE;
	prvButton.enabledTextColor = GUI_CYAN_DARK;
	prvButton.enabledBackgroundColor = GUI_CYAN_DARK;
	prvButton.disabledTextColor = GUI_CYAN_DARK;
	prvButton.disabledBackgroundColor = GUI_WHITE;
	prvButton.pressedTextColor = GUI_CYAN_DARK;
	prvButton.pressedBackgroundColor = GUI_CYAN_VERY_DARK;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiGpioOutPinCallback;
	prvButton.text[0] = "Low";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* GPIO1 Duty Up Button */
	prvButton.object.id = guiConfigGPIO1_PWM_DUTY_UP_BUTTON_ID;
	prvButton.object.xPos = 570;
	prvButton.object.yPos = 335;
	prvButton.object.width = 50;
	prvButton.object.height = 50;
	prvButton.object.containerPage = guiConfigGPIO_PWM_PAGE;
	prvButton.enabledTextColor = GUI_CYAN_DARK;
	prvButton.enabledBackgroundColor = GUI_CYAN_DARK;
	prvButton.disabledTextColor = GUI_CYAN_DARK;
	prvButton.disabledBackgroundColor = GUI_WHITE;
	prvButton.pressedTextColor = GUI_CYAN_DARK;
	prvButton.pressedBackgroundColor = GUI_CYAN_VERY_DARK;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiGpioDutyCallback;
	prvButton.text[0] = "+";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* GPIO1 Duty Down Button */
	prvButton.object.id = guiConfigGPIO1_PWM_DUTY_DOWN_BUTTON_ID;
	prvButton.object.xPos = 350;
	prvButton.object.yPos = 335;
	prvButton.object.width = 50;
	prvButton.object.height = 50;
	prvButton.object.containerPage = guiConfigGPIO_PWM_PAGE;
	prvButton.enabledTextColor = GUI_CYAN_DARK;
	prvButton.enabledBackgroundColor = GUI_CYAN_DARK;
	prvButton.disabledTextColor = GUI_CYAN_DARK;
	prvButton.disabledBackgroundColor = GUI_WHITE;
	prvButton.pressedTextColor = GUI_CYAN_DARK;
	prvButton.pressedBackgroundColor = GUI_CYAN_VERY_DARK;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiGpioDutyCallback;
	prvButton.text[0] = "-";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* GPIO1 Frequency Up Button */
	prvButton.object.id = guiConfigGPIO1_PWM_FREQ_UP_BUTTON_ID;
	prvButton.object.xPos = 570;
	prvButton.object.yPos = 390;
	prvButton.object.width = 50;
	prvButton.object.height = 50;
	prvButton.object.containerPage = guiConfigGPIO_PWM_PAGE;
	prvButton.enabledTextColor = GUI_CYAN_DARK;
	prvButton.enabledBackgroundColor = GUI_CYAN_DARK;
	prvButton.disabledTextColor = GUI_CYAN_DARK;
	prvButton.disabledBackgroundColor = GUI_WHITE;
	prvButton.pressedTextColor = GUI_CYAN_DARK;
	prvButton.pressedBackgroundColor = GUI_CYAN_VERY_DARK;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = 0;
	prvButton.text[0] = "+";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* GPIO1 Frequency Down Button */
	prvButton.object.id = guiConfigGPIO1_PWM_FREQ_DOWN_BUTTON_ID;
	prvButton.object.xPos = 350;
	prvButton.object.yPos = 390;
	prvButton.object.width = 50;
	prvButton.object.height = 50;
	prvButton.object.containerPage = guiConfigGPIO_PWM_PAGE;
	prvButton.enabledTextColor = GUI_CYAN_DARK;
	prvButton.enabledBackgroundColor = GUI_CYAN_DARK;
	prvButton.disabledTextColor = GUI_CYAN_DARK;
	prvButton.disabledBackgroundColor = GUI_WHITE;
	prvButton.pressedTextColor = GUI_CYAN_DARK;
	prvButton.pressedBackgroundColor = GUI_CYAN_VERY_DARK;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = 0;
	prvButton.text[0] = "-";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUI_AddButton(&prvButton);

	/* Containers ----------------------------------------------------------------*/
	/* Sidebar GPIO container */
	prvContainer.object.id = guiConfigSIDEBAR_GPIO_CONTAINER_ID;
	prvContainer.object.xPos = 650;
	prvContainer.object.yPos = 50;
	prvContainer.object.width = 150;
	prvContainer.object.height = 405;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 1;
	prvContainer.object.borderColor = GUI_WHITE;
	prvContainer.activePage = GUIContainerPage_1;
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
	prvContainer.object.height = 170;
	prvContainer.object.containerPage = guiConfigMAIN_CONTAINER_GPIO_PAGE;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Right | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 2;
	prvContainer.object.borderColor = GUI_WHITE;
	prvContainer.activePage = guiConfigGPIO_OUTPUT_PAGE;
	prvContainer.backgroundColor = GUI_CYAN_LIGHT;
	prvContainer.contentHideState = GUIHideState_HideAll;
	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(guiConfigGPIO0_VALUE_LABEL_TEXT_BOX_ID);
	prvContainer.textBoxes[1] = GUI_GetTextBoxFromId(guiConfigGPIO0_VALUE_TEXT_BOX_ID);
	prvContainer.textBoxes[2] = GUI_GetTextBoxFromId(guiConfigGPIO0_LABEL_TEXT_BOX_ID);
	prvContainer.textBoxes[3] = GUI_GetTextBoxFromId(guiConfigGPIO0_TYPE_TEXT_BOX_ID);
	prvContainer.textBoxes[4] = GUI_GetTextBoxFromId(guiConfigGPIO0_DUTY_VALUE_TEXT_BOX_ID);
	prvContainer.textBoxes[5] = GUI_GetTextBoxFromId(guiConfigGPIO0_FREQ_VALUE_TEXT_BOX_ID);
	prvContainer.textBoxes[6] = GUI_GetTextBoxFromId(guiConfigGPIO0_DUTY_TEXT_BOX_ID);
	prvContainer.textBoxes[7] = GUI_GetTextBoxFromId(guiConfigGPIO0_FREQ_TEXT_BOX_ID);

	prvContainer.buttons[0] = GUI_GetButtonFromId(guiConfigGPIO0_ENABLE_BUTTON_ID);
	prvContainer.buttons[1] = GUI_GetButtonFromId(guiConfigGPIO0_PULLUP_BUTTON_ID);
	prvContainer.buttons[2] = GUI_GetButtonFromId(guiConfigGPIO0_NOPULL_BUTTON_ID);
	prvContainer.buttons[3] = GUI_GetButtonFromId(guiConfigGPIO0_PULLDOWN_BUTTON_ID);
	prvContainer.buttons[4] = GUI_GetButtonFromId(guiConfigGPIO0_OUT_HIGH_BUTTON_ID);
	prvContainer.buttons[5] = GUI_GetButtonFromId(guiConfigGPIO0_OUT_LOW_BUTTON_ID);
	prvContainer.buttons[6] = GUI_GetButtonFromId(guiConfigGPIO0_OUT_TOGGLE_BUTTON_ID);
	prvContainer.buttons[7] = GUI_GetButtonFromId(guiConfigGPIO0_PWM_DUTY_UP_BUTTON_ID);
	prvContainer.buttons[8] = GUI_GetButtonFromId(guiConfigGPIO0_PWM_DUTY_DOWN_BUTTON_ID);
	prvContainer.buttons[9] = GUI_GetButtonFromId(guiConfigGPIO0_PWM_FREQ_UP_BUTTON_ID);
	prvContainer.buttons[10] = GUI_GetButtonFromId(guiConfigGPIO0_PWM_FREQ_DOWN_BUTTON_ID);
	GUI_AddContainer(&prvContainer);


	/* GPIO1 main container */
	prvContainer.object.id = guiConfigMAIN_GPIO1_CONTAINER_ID;
	prvContainer.object.xPos = 25;
	prvContainer.object.yPos = 275;
	prvContainer.object.width = 600;
	prvContainer.object.height = 170;
	prvContainer.object.containerPage = guiConfigMAIN_CONTAINER_GPIO_PAGE;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Right | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 2;
	prvContainer.object.borderColor = GUI_WHITE;
	prvContainer.activePage = guiConfigGPIO_OUTPUT_PAGE;
	prvContainer.backgroundColor = GUI_CYAN_DARK;
	prvContainer.contentHideState = GUIHideState_HideAll;
	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(guiConfigGPIO1_VALUE_LABEL_TEXT_BOX_ID);
	prvContainer.textBoxes[1] = GUI_GetTextBoxFromId(guiConfigGPIO1_VALUE_TEXT_BOX_ID);
	prvContainer.textBoxes[2] = GUI_GetTextBoxFromId(guiConfigGPIO1_LABEL_TEXT_BOX_ID);
	prvContainer.textBoxes[3] = GUI_GetTextBoxFromId(guiConfigGPIO1_TYPE_TEXT_BOX_ID);
	prvContainer.textBoxes[4] = GUI_GetTextBoxFromId(guiConfigGPIO1_DUTY_VALUE_TEXT_BOX_ID);
	prvContainer.textBoxes[5] = GUI_GetTextBoxFromId(guiConfigGPIO1_FREQ_VALUE_TEXT_BOX_ID);
	prvContainer.textBoxes[6] = GUI_GetTextBoxFromId(guiConfigGPIO1_DUTY_TEXT_BOX_ID);
	prvContainer.textBoxes[7] = GUI_GetTextBoxFromId(guiConfigGPIO1_FREQ_TEXT_BOX_ID);

	prvContainer.buttons[0] = GUI_GetButtonFromId(guiConfigGPIO1_ENABLE_BUTTON_ID);
	prvContainer.buttons[1] = GUI_GetButtonFromId(guiConfigGPIO1_PULLUP_BUTTON_ID);
	prvContainer.buttons[2] = GUI_GetButtonFromId(guiConfigGPIO1_NOPULL_BUTTON_ID);
	prvContainer.buttons[3] = GUI_GetButtonFromId(guiConfigGPIO1_PULLDOWN_BUTTON_ID);
	prvContainer.buttons[4] = GUI_GetButtonFromId(guiConfigGPIO1_OUT_HIGH_BUTTON_ID);
	prvContainer.buttons[5] = GUI_GetButtonFromId(guiConfigGPIO1_OUT_LOW_BUTTON_ID);
	prvContainer.buttons[6] = GUI_GetButtonFromId(guiConfigGPIO1_OUT_TOGGLE_BUTTON_ID);
	prvContainer.buttons[7] = GUI_GetButtonFromId(guiConfigGPIO1_PWM_DUTY_UP_BUTTON_ID);
	prvContainer.buttons[8] = GUI_GetButtonFromId(guiConfigGPIO1_PWM_DUTY_DOWN_BUTTON_ID);
	prvContainer.buttons[9] = GUI_GetButtonFromId(guiConfigGPIO1_PWM_FREQ_UP_BUTTON_ID);
	prvContainer.buttons[10] = GUI_GetButtonFromId(guiConfigGPIO1_PWM_FREQ_DOWN_BUTTON_ID);
	GUI_AddContainer(&prvContainer);
}


/* Private functions .--------------------------------------------------------*/
/**
 * @brief	Sets the correct text for the out buttons to be the same as the real output
 * @param	None
 * @retval	None
 */
void prvUpdateOutButtonTextToMatchRealOutput(uint32_t channel)
{
	/* Read the pin and update the button to display the current state */
	if (channel == 0 && gpio0IsEnabled())
	{
		GPIO_PinState state = gpio0ReadPin();
		if (state == GPIO_PIN_SET)
		{
			GUI_SetButtonTextForRow(guiConfigGPIO0_OUT_HIGH_BUTTON_ID, ">High<", 0);
			GUI_SetButtonTextForRow(guiConfigGPIO0_OUT_LOW_BUTTON_ID, "Low", 0);
		}
		else if (state == GPIO_PIN_RESET)
		{
			GUI_SetButtonTextForRow(guiConfigGPIO0_OUT_HIGH_BUTTON_ID, "High", 0);
			GUI_SetButtonTextForRow(guiConfigGPIO0_OUT_LOW_BUTTON_ID, ">Low<", 0);
		}
	}
	else if (channel == 1 && gpio1IsEnabled())
	{
		GPIO_PinState state = gpio1ReadPin();
		if (state == GPIO_PIN_SET)
		{
			GUI_SetButtonTextForRow(guiConfigGPIO1_OUT_HIGH_BUTTON_ID, ">High<", 0);
			GUI_SetButtonTextForRow(guiConfigGPIO1_OUT_LOW_BUTTON_ID, "Low", 0);
		}
		else if (state == GPIO_PIN_RESET)
		{
			GUI_SetButtonTextForRow(guiConfigGPIO1_OUT_HIGH_BUTTON_ID, "High", 0);
			GUI_SetButtonTextForRow(guiConfigGPIO1_OUT_LOW_BUTTON_ID, ">Low<", 0);
		}
	}
}

/**
 * @brief	Enables the channel and fixes all the GUI related stuff
 * @param	None
 * @retval	None
 */
void prvEnableChannel(uint32_t channel)
{
	if (channel == 0)
	{
		/* Enable the channel */
		gpio0Enable();
		GUI_SetButtonTextForRow(guiConfigGPIO0_ENABLE_BUTTON_ID, "Disable", 0);

		/* Update the button text if it's set to output */
		if (gpio0GetDirection() == GPIODirection_Output)
			prvUpdateOutButtonTextToMatchRealOutput(0);
	}
	else if (channel == 1)
	{
		/* Enable the channel */
		gpio1Enable();
		GUI_SetButtonTextForRow(guiConfigGPIO1_ENABLE_BUTTON_ID, "Disable", 0);

		/* Update the button text if it's set to output */
		if (gpio1GetDirection() == GPIODirection_Output)
			prvUpdateOutButtonTextToMatchRealOutput(1);
	}
}

/**
 * @brief	Disables the channel and fixes all the GUI related stuff
 * @param	None
 * @retval	None
 */
void prvDisableChannel(uint32_t channel)
{
	if (channel == 0)
	{
		/* Disable the channel */
		gpio0Disable();
		GUI_SetButtonTextForRow(guiConfigGPIO0_ENABLE_BUTTON_ID, "Enable", 0);

		/* Update the button text if it's set to output */
		if (gpio0GetDirection() == GPIODirection_Output)
		{
			GUI_SetButtonTextForRow(guiConfigGPIO0_OUT_HIGH_BUTTON_ID, "High", 0);
			GUI_SetButtonTextForRow(guiConfigGPIO0_OUT_LOW_BUTTON_ID, "Low", 0);
		}
	}
	else if (channel == 1)
	{
		/* Disable the channel */
		gpio1Disable();
		GUI_SetButtonTextForRow(guiConfigGPIO1_ENABLE_BUTTON_ID, "Enable", 0);

		/* Update the button text if it's set to output */
		if (gpio0GetDirection() == GPIODirection_Output)
		{
			GUI_SetButtonTextForRow(guiConfigGPIO1_OUT_HIGH_BUTTON_ID, "High", 0);
			GUI_SetButtonTextForRow(guiConfigGPIO1_OUT_LOW_BUTTON_ID, "Low", 0);
		}
	}
}

/**
 * @brief	Update the GUI stuff for the duty cycle values
 * @param	None
 * @retval	None
 */
void prvUpdateDutyValuesInGui(uint32_t channel)
{
	if (channel == 0)
	{
		/* Update the text box */
		float currentDutyCh0 = gpio0GetPwmDuty();
		GUI_ClearAndResetTextBox(guiConfigGPIO0_DUTY_VALUE_TEXT_BOX_ID);
		GUI_SetYWritePositionToCenter(guiConfigGPIO0_DUTY_VALUE_TEXT_BOX_ID);
		GUI_WriteNumberInTextBox(guiConfigGPIO0_DUTY_VALUE_TEXT_BOX_ID, (int32_t)currentDutyCh0);
		GUI_WriteStringInTextBox(guiConfigGPIO0_DUTY_VALUE_TEXT_BOX_ID, " %");
	}
	else if (channel == 1)
	{
		/* Update the text box */
		float currentDutyCh1 = gpio1GetPwmDuty();
		GUI_ClearAndResetTextBox(guiConfigGPIO1_DUTY_VALUE_TEXT_BOX_ID);
		GUI_SetYWritePositionToCenter(guiConfigGPIO1_DUTY_VALUE_TEXT_BOX_ID);
		GUI_WriteNumberInTextBox(guiConfigGPIO1_DUTY_VALUE_TEXT_BOX_ID, (int32_t)currentDutyCh1);
		GUI_WriteStringInTextBox(guiConfigGPIO1_DUTY_VALUE_TEXT_BOX_ID, " %");
	}
}

/* Interrupt Handlers --------------------------------------------------------*/
