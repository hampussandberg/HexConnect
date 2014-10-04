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
static void prvUpdateOutButtonTextToMatchRealOutput(uint32_t channel);
static void prvEnableChannel(uint32_t channel);
static void prvDisableChannel(uint32_t channel);
static void prvUpdateDutyValuesInGui(uint32_t channel);
static void prvUpdatePullValuesInGui(uint32_t channel);

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
			GUI_SetStaticTextInTextBox(GUITextBoxId_Gpio0Value, "High");
			lastState = newState;
		}
		else if (newState != lastState && newState == GPIO_PIN_RESET)
		{
			GUI_SetStaticTextInTextBox(GUITextBoxId_Gpio0Value, "Low");
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
			GUI_SetStaticTextInTextBox(GUITextBoxId_Gpio1Value, "High");
			lastState = newState;
		}
		else if (newState != lastState && newState == GPIO_PIN_RESET)
		{
			GUI_SetStaticTextInTextBox(GUITextBoxId_Gpio1Value, "Low");
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
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(GUIContainerId_SidebarGpio);
		lcdChangeDisplayStateOfSidebar(GUIContainerId_SidebarGpio);

		if (displayState == GUIDisplayState_Hidden)
		{
			GUI_DrawContainer(GUIContainerId_Gpio0MainContent);
			GUI_DrawContainer(GUIContainerId_Gpio1MainContent);
		}
		else if (displayState == GUIDisplayState_NotHidden)
		{
			GUI_HideContainer(GUIContainerId_Gpio0MainContent);
			GUI_HideContainer(GUIContainerId_Gpio1MainContent);
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
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(GUIContainerId_PopoutGpio0Type);

		if (displayState == GUIDisplayState_Hidden)
		{
			GUI_SetActiveLayer(GUILayer_1);
			GUI_SetLayerForButton(GUIButtonId_Gpio0Type, GUILayer_1);
			GUI_SetButtonState(GUIButtonId_Gpio0Type, GUIButtonState_Enabled);
			GUI_DrawContainer(GUIContainerId_PopoutGpio0Type);
		}
		else if (displayState == GUIDisplayState_NotHidden)
		{
			GUI_HideContainer(GUIContainerId_PopoutGpio0Type);
			GUI_SetActiveLayer(GUILayer_0);
			GUI_SetLayerForButton(GUIButtonId_Gpio0Type, GUILayer_0);
			GUI_SetButtonState(GUIButtonId_Gpio0Type, GUIButtonState_Disabled);

			/* Refresh the main container */
			GUI_DrawContainer(GUIContainerId_MainContent);
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
		GUIDisplayState displayState = GUI_GetDisplayStateForContainer(GUIContainerId_PopoutGpio1Type);

		if (displayState == GUIDisplayState_Hidden)
		{
			GUI_SetActiveLayer(GUILayer_1);
			GUI_SetLayerForButton(GUIButtonId_Gpio1Type, GUILayer_1);
			GUI_SetButtonState(GUIButtonId_Gpio1Type, GUIButtonState_Enabled);
			GUI_DrawContainer(GUIContainerId_PopoutGpio1Type);
		}
		else if (displayState == GUIDisplayState_NotHidden)
		{
			GUI_HideContainer(GUIContainerId_PopoutGpio1Type);
			GUI_SetActiveLayer(GUILayer_0);
			GUI_SetLayerForButton(GUIButtonId_Gpio1Type, GUILayer_0);
			GUI_SetButtonState(GUIButtonId_Gpio1Type, GUIButtonState_Disabled);

			/* Refresh the main container */
			GUI_DrawContainer(GUIContainerId_MainContent);
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
		GUI_HideContainer(GUIContainerId_PopoutGpio0Type);
		GUI_SetActiveLayer(GUILayer_0);
		GUI_SetLayerForButton(GUIButtonId_Gpio0Type, GUILayer_0);
		GUI_SetButtonState(GUIButtonId_Gpio0Type, GUIButtonState_Disabled);

		/* Make sure to disable before changing type */
		prvDisableChannel(0);

		/* Do different things depending on which button was pressed */
		switch (ButtonId)
		{
			case GUIButtonId_Gpio0TypeOut:
				GUI_SetButtonTextForRow(GUIButtonId_Gpio0Type, "Output", 1);
				GUI_SetStaticTextInTextBox(GUITextBoxId_Gpio0Type, "Output ->");
				GUI_ChangePageOfContainer(GUIContainerId_Gpio0MainContent, guiConfigGPIO_OUTPUT_PAGE);
				gpio0SetDirection(GPIODirection_Output);
				break;
			case GUIButtonId_Gpio0TypeIn:
				GUI_SetButtonTextForRow(GUIButtonId_Gpio0Type, "Input", 1);
				GUI_SetStaticTextInTextBox(GUITextBoxId_Gpio0Type, "Input <-");
				GUI_ChangePageOfContainer(GUIContainerId_Gpio0MainContent, guiConfigGPIO_INPUT_PAGE);
				gpio0SetDirection(GPIODirection_Input);
				break;
			case GUIButtonId_Gpio0TypePwm:
				GUI_SetButtonTextForRow(GUIButtonId_Gpio0Type, "PWM", 1);
				GUI_SetStaticTextInTextBox(GUITextBoxId_Gpio0Type, "PWM |_|->");
				GUI_ChangePageOfContainer(GUIContainerId_Gpio0MainContent, guiConfigGPIO_PWM_PAGE);
				gpio0SetDirection(GPIODirection_OutputPWM);
				break;
			default:
				break;
		}

		/* Refresh the main container */
		GUI_DrawContainer(GUIContainerId_MainContent);
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
		GUI_HideContainer(GUIContainerId_PopoutGpio1Type);
		GUI_SetActiveLayer(GUILayer_0);
		GUI_SetLayerForButton(GUIButtonId_Gpio1Type, GUILayer_0);
		GUI_SetButtonState(GUIButtonId_Gpio1Type, GUIButtonState_Disabled);

		/* Make sure to disable before changing type */
		prvDisableChannel(1);

		/* Do different things depending on which button was pressed */
		switch (ButtonId)
		{
			case GUIButtonId_Gpio1TypeOut:
				GUI_SetButtonTextForRow(GUIButtonId_Gpio1Type, "Output", 1);
				GUI_SetStaticTextInTextBox(GUITextBoxId_Gpio1Type, "Output ->");
				GUI_ChangePageOfContainer(GUIContainerId_Gpio1MainContent, guiConfigGPIO_OUTPUT_PAGE);
				gpio1SetDirection(GPIODirection_Output);
				break;
			case GUIButtonId_Gpio1TypeIn:
				GUI_SetButtonTextForRow(GUIButtonId_Gpio1Type, "Input", 1);
				GUI_SetStaticTextInTextBox(GUITextBoxId_Gpio1Type, "Input <-");
				GUI_ChangePageOfContainer(GUIContainerId_Gpio1MainContent, guiConfigGPIO_INPUT_PAGE);
				gpio1SetDirection(GPIODirection_Input);
				break;
			case GUIButtonId_Gpio1TypePwm:
				GUI_SetButtonTextForRow(GUIButtonId_Gpio1Type, "PWM", 1);
				GUI_SetStaticTextInTextBox(GUITextBoxId_Gpio1Type, "PWM |_|->");
				GUI_ChangePageOfContainer(GUIContainerId_Gpio1MainContent, guiConfigGPIO_PWM_PAGE);
				gpio1SetDirection(GPIODirection_OutputPWM);
				break;
			default:
				break;
		}

		/* Refresh the main container */
		GUI_DrawContainer(GUIContainerId_MainContent);
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
			case GUIButtonId_Gpio0Enable:
				if (gpio0IsEnabled())
					prvDisableChannel(0);
				else
					prvEnableChannel(0);
				break;
			case GUIButtonId_Gpio1Enable:
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
			GUI_SetButtonState(GUIButtonId_GpioTop, GUIButtonState_Enabled);
		else
			GUI_SetButtonState(GUIButtonId_GpioTop, GUIButtonState_Disabled);

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
			case GUIButtonId_Gpio0OutHigh:
				gpio0WritePin(GPIO_PIN_SET);
				channel = 0;
				break;
			case GUIButtonId_Gpio0OutToggle:
				gpio0TogglePin();
				channel = 0;
				break;
			case GUIButtonId_Gpio0OutLow:
				gpio0WritePin(GPIO_PIN_RESET);
				channel = 0;
				break;

			case GUIButtonId_Gpio1OutHigh:
				gpio1WritePin(GPIO_PIN_SET);
				channel = 1;
				break;
			case GUIButtonId_Gpio1OutToggle:
				gpio1TogglePin();
				channel = 1;
				break;
			case GUIButtonId_Gpio1OutLow:
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
			case GUIButtonId_Gpio0PwmDutyUp:
				currentDutyCh0 += 1.0;
				if (currentDutyCh0 > 100.0)
					currentDutyCh0 = 100.0;
				gpio0SetPwmDuty(currentDutyCh0);
				/* Update the text box */
				prvUpdateDutyValuesInGui(0);
				break;

			/* CH0 Down */
			case GUIButtonId_Gpio0PwmDutyDown:
				currentDutyCh0 -= 1.0;
				if (currentDutyCh0 < 0)
					currentDutyCh0 = 0.0;
				gpio0SetPwmDuty(currentDutyCh0);
				/* Update the text box */
				prvUpdateDutyValuesInGui(0);
				break;

			/* CH1 Up */
			case GUIButtonId_Gpio1PwmDutyUp:
				currentDutyCh1 += 1.0;
				if (currentDutyCh1 > 100.0)
					currentDutyCh1 = 100.0;
				gpio1SetPwmDuty(currentDutyCh1);
				/* Update the text box */
				prvUpdateDutyValuesInGui(1);
				break;

			/* CH1 Down */
			case GUIButtonId_Gpio1PwmDutyDown:
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
	prvTextBox.object.id = GUITextBoxId_GpioLabel;
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
	prvTextBox.object.id = GUITextBoxId_Gpio0Label;
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
	prvTextBox.object.id = GUITextBoxId_Gpio0Type;
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
	prvTextBox.object.id = GUITextBoxId_Gpio1Label;
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
	prvTextBox.object.id = GUITextBoxId_Gpio1Type;
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
	prvTextBox.object.id = GUITextBoxId_Gpio0ValueLabel;
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
	prvTextBox.object.id = GUITextBoxId_Gpio0Value;
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
	prvTextBox.object.id = GUITextBoxId_Gpio0DutyCycleValue;
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
	prvTextBox.object.id = GUITextBoxId_Gpio0FrequencyValue;
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
	prvTextBox.object.id = GUITextBoxId_Gpio0DutyCycleLabel;
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
	prvTextBox.object.id = GUITextBoxId_Gpio0FrequencyValue;
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
	prvTextBox.object.id = GUITextBoxId_Gpio1ValueLabel;
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
	prvTextBox.object.id = GUITextBoxId_Gpio1Value;
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
	prvTextBox.object.id = GUITextBoxId_Gpio1DutyCycleValue;
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
	prvTextBox.object.id = GUITextBoxId_Gpio1FrequencyValue;
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
	prvTextBox.object.id = GUITextBoxId_Gpio1DutyCycleLabel;
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
	prvTextBox.object.id = GUITextBoxId_Gpio1FrequencyLabel;
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
	prvButton.object.id = GUIButtonId_GpioTop;
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
	prvButton.object.id = GUIButtonId_Gpio0Type;
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
	prvButton.object.id = GUIButtonId_Gpio1Type;
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
	prvButton.object.id = GUIButtonId_Gpio0TypeOut;
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
	prvButton.object.id = GUIButtonId_Gpio0TypeIn;
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
	prvButton.object.id = GUIButtonId_Gpio0TypePwm;
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
	prvButton.object.id = GUIButtonId_Gpio1TypeOut;
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
	prvButton.object.id = GUIButtonId_Gpio1TypeIn;
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
	prvButton.object.id = GUIButtonId_Gpio1TypePwm;
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
	prvButton.object.id = GUIButtonId_Gpio0Enable;
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

	/* GPIO0 Out high Button */
	prvButton.object.id = GUIButtonId_Gpio0OutHigh;
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
	prvButton.object.id = GUIButtonId_Gpio0OutToggle;
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
	prvButton.object.id = GUIButtonId_Gpio0OutLow;
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
	prvButton.object.id = GUIButtonId_Gpio0PwmDutyUp;
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
	prvButton.object.id = GUIButtonId_Gpio0PwmDutyDown;
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
	prvButton.object.id = GUIButtonId_Gpio0PwmFreqUp;
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
	prvButton.object.id = GUIButtonId_Gpio0PwmFreqDown;
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
	prvButton.object.id = GUIButtonId_Gpio1Enable;
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

	/* GPIO1 Out high Button */
	prvButton.object.id = GUIButtonId_Gpio1OutHigh;
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
	prvButton.object.id = GUIButtonId_Gpio1OutToggle;
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
	prvButton.object.id = GUIButtonId_Gpio1OutLow;
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
	prvButton.object.id = GUIButtonId_Gpio1PwmDutyUp;
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
	prvButton.object.id = GUIButtonId_Gpio1PwmDutyDown;
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
	prvButton.object.id = GUIButtonId_Gpio1PwmFreqUp;
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
	prvButton.object.id = GUIButtonId_Gpio1PwmFreqDown;
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
	prvContainer.object.id = GUIContainerId_SidebarGpio;
	prvContainer.object.xPos = 650;
	prvContainer.object.yPos = 50;
	prvContainer.object.width = 150;
	prvContainer.object.height = 405;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 1;
	prvContainer.object.borderColor = GUI_WHITE;
	prvContainer.activePage = GUIContainerPage_1;
	prvContainer.contentHideState = GUIHideState_KeepBorders;
	prvContainer.buttons[0] = GUI_GetButtonFromId(GUIButtonId_Gpio0Type);
	prvContainer.buttons[1] = GUI_GetButtonFromId(GUIButtonId_Gpio1Type);
	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(GUITextBoxId_GpioLabel);
	GUI_AddContainer(&prvContainer);

	/* GPIO0 type popout container */
	prvContainer.object.id = GUIContainerId_PopoutGpio0Type;
	prvContainer.object.xPos = 500;
	prvContainer.object.yPos = 100;
	prvContainer.object.width = 149;
	prvContainer.object.height = 120;
	prvContainer.object.layer = GUILayer_1;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 2;
	prvContainer.object.borderColor = GUI_WHITE;
	prvContainer.contentHideState = GUIHideState_HideAll;
	prvContainer.buttons[0] = GUI_GetButtonFromId(GUIButtonId_Gpio0TypeOut);
	prvContainer.buttons[1] = GUI_GetButtonFromId(GUIButtonId_Gpio0TypeIn);
	prvContainer.buttons[2] = GUI_GetButtonFromId(GUIButtonId_Gpio0TypePwm);
	GUI_AddContainer(&prvContainer);

	/* GPIO1 type popout container */
	prvContainer.object.id = GUIContainerId_PopoutGpio1Type;
	prvContainer.object.xPos = 500;
	prvContainer.object.yPos = 150;
	prvContainer.object.width = 149;
	prvContainer.object.height = 120;
	prvContainer.object.layer = GUILayer_1;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 2;
	prvContainer.object.borderColor = GUI_WHITE;
	prvContainer.contentHideState = GUIHideState_HideAll;
	prvContainer.buttons[0] = GUI_GetButtonFromId(GUIButtonId_Gpio1TypeOut);
	prvContainer.buttons[1] = GUI_GetButtonFromId(GUIButtonId_Gpio1TypeIn);
	prvContainer.buttons[2] = GUI_GetButtonFromId(GUIButtonId_Gpio1TypePwm);
	GUI_AddContainer(&prvContainer);

	/* GPIO0 main container */
	prvContainer.object.id = GUIContainerId_Gpio0MainContent;
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
	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(GUITextBoxId_Gpio0ValueLabel);
	prvContainer.textBoxes[1] = GUI_GetTextBoxFromId(GUITextBoxId_Gpio0Value);
	prvContainer.textBoxes[2] = GUI_GetTextBoxFromId(GUITextBoxId_Gpio0Label);
	prvContainer.textBoxes[3] = GUI_GetTextBoxFromId(GUITextBoxId_Gpio0Type);
	prvContainer.textBoxes[4] = GUI_GetTextBoxFromId(GUITextBoxId_Gpio0DutyCycleValue);
	prvContainer.textBoxes[5] = GUI_GetTextBoxFromId(GUITextBoxId_Gpio0FrequencyValue);
	prvContainer.textBoxes[6] = GUI_GetTextBoxFromId(GUITextBoxId_Gpio0DutyCycleLabel);
	prvContainer.textBoxes[7] = GUI_GetTextBoxFromId(GUITextBoxId_Gpio0FrequencyValue);

	prvContainer.buttons[0] = GUI_GetButtonFromId(GUIButtonId_Gpio0Enable);
	prvContainer.buttons[1] = GUI_GetButtonFromId(GUIButtonId_Gpio0OutHigh);
	prvContainer.buttons[2] = GUI_GetButtonFromId(GUIButtonId_Gpio0OutLow);
	prvContainer.buttons[3] = GUI_GetButtonFromId(GUIButtonId_Gpio0OutToggle);
	prvContainer.buttons[4] = GUI_GetButtonFromId(GUIButtonId_Gpio0PwmDutyUp);
	prvContainer.buttons[5] = GUI_GetButtonFromId(GUIButtonId_Gpio0PwmDutyDown);
	prvContainer.buttons[6] = GUI_GetButtonFromId(GUIButtonId_Gpio0PwmFreqUp);
	prvContainer.buttons[7] = GUI_GetButtonFromId(GUIButtonId_Gpio0PwmFreqDown);
	GUI_AddContainer(&prvContainer);


	/* GPIO1 main container */
	prvContainer.object.id = GUIContainerId_Gpio1MainContent;
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
	prvContainer.textBoxes[0] = GUI_GetTextBoxFromId(GUITextBoxId_Gpio1ValueLabel);
	prvContainer.textBoxes[1] = GUI_GetTextBoxFromId(GUITextBoxId_Gpio1Value);
	prvContainer.textBoxes[2] = GUI_GetTextBoxFromId(GUITextBoxId_Gpio1Label);
	prvContainer.textBoxes[3] = GUI_GetTextBoxFromId(GUITextBoxId_Gpio1Type);
	prvContainer.textBoxes[4] = GUI_GetTextBoxFromId(GUITextBoxId_Gpio1DutyCycleValue);
	prvContainer.textBoxes[5] = GUI_GetTextBoxFromId(GUITextBoxId_Gpio1FrequencyValue);
	prvContainer.textBoxes[6] = GUI_GetTextBoxFromId(GUITextBoxId_Gpio1DutyCycleLabel);
	prvContainer.textBoxes[7] = GUI_GetTextBoxFromId(GUITextBoxId_Gpio1FrequencyLabel);

	prvContainer.buttons[0] = GUI_GetButtonFromId(GUIButtonId_Gpio1Enable);
	prvContainer.buttons[1] = GUI_GetButtonFromId(GUIButtonId_Gpio1OutHigh);
	prvContainer.buttons[2] = GUI_GetButtonFromId(GUIButtonId_Gpio1OutLow);
	prvContainer.buttons[3] = GUI_GetButtonFromId(GUIButtonId_Gpio1OutToggle);
	prvContainer.buttons[4] = GUI_GetButtonFromId(GUIButtonId_Gpio1PwmDutyUp);
	prvContainer.buttons[5] = GUI_GetButtonFromId(GUIButtonId_Gpio1PwmDutyDown);
	prvContainer.buttons[6] = GUI_GetButtonFromId(GUIButtonId_Gpio1PwmFreqUp);
	prvContainer.buttons[7] = GUI_GetButtonFromId(GUIButtonId_Gpio1PwmFreqDown);
	GUI_AddContainer(&prvContainer);
}


/* Private functions .--------------------------------------------------------*/
/**
 * @brief	Sets the correct text for the out buttons to be the same as the real output
 * @param	None
 * @retval	None
 */
static void prvUpdateOutButtonTextToMatchRealOutput(uint32_t channel)
{
	/* Read the pin and update the button to display the current state */
	if (channel == 0 && gpio0IsEnabled())
	{
		GPIO_PinState state = gpio0ReadPin();
		if (state == GPIO_PIN_SET)
		{
			GUI_SetButtonTextForRow(GUIButtonId_Gpio0OutHigh, ">High<", 0);
			GUI_SetButtonTextForRow(GUIButtonId_Gpio0OutLow, "Low", 0);
		}
		else if (state == GPIO_PIN_RESET)
		{
			GUI_SetButtonTextForRow(GUIButtonId_Gpio0OutHigh, "High", 0);
			GUI_SetButtonTextForRow(GUIButtonId_Gpio0OutLow, ">Low<", 0);
		}
	}
	else if (channel == 1 && gpio1IsEnabled())
	{
		GPIO_PinState state = gpio1ReadPin();
		if (state == GPIO_PIN_SET)
		{
			GUI_SetButtonTextForRow(GUIButtonId_Gpio1OutHigh, ">High<", 0);
			GUI_SetButtonTextForRow(GUIButtonId_Gpio1OutLow, "Low", 0);
		}
		else if (state == GPIO_PIN_RESET)
		{
			GUI_SetButtonTextForRow(GUIButtonId_Gpio1OutHigh, "High", 0);
			GUI_SetButtonTextForRow(GUIButtonId_Gpio1OutLow, ">Low<", 0);
		}
	}
}

/**
 * @brief	Enables the channel and fixes all the GUI related stuff
 * @param	None
 * @retval	None
 */
static void prvEnableChannel(uint32_t channel)
{
	if (channel == 0)
	{
		/* Enable the channel */
		gpio0Enable();
		GUI_SetButtonTextForRow(GUIButtonId_Gpio0Enable, "Disable", 0);

		/* Update the button text if it's set to output */
		if (gpio0GetDirection() == GPIODirection_Output)
			prvUpdateOutButtonTextToMatchRealOutput(0);
	}
	else if (channel == 1)
	{
		/* Enable the channel */
		gpio1Enable();
		GUI_SetButtonTextForRow(GUIButtonId_Gpio1Enable, "Disable", 0);

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
static void prvDisableChannel(uint32_t channel)
{
	if (channel == 0)
	{
		/* Disable the channel */
		gpio0Disable();
		GUI_SetButtonTextForRow(GUIButtonId_Gpio0Enable, "Enable", 0);

		/* Update the button text if it's set to output */
		if (gpio0GetDirection() == GPIODirection_Output)
		{
			GUI_SetButtonTextForRow(GUIButtonId_Gpio0OutHigh, "High", 0);
			GUI_SetButtonTextForRow(GUIButtonId_Gpio0OutLow, "Low", 0);
		}
	}
	else if (channel == 1)
	{
		/* Disable the channel */
		gpio1Disable();
		GUI_SetButtonTextForRow(GUIButtonId_Gpio1Enable, "Enable", 0);

		/* Update the button text if it's set to output */
		if (gpio0GetDirection() == GPIODirection_Output)
		{
			GUI_SetButtonTextForRow(GUIButtonId_Gpio1OutHigh, "High", 0);
			GUI_SetButtonTextForRow(GUIButtonId_Gpio1OutLow, "Low", 0);
		}
	}
}

/**
 * @brief	Update the GUI stuff for the duty cycle values
 * @param	None
 * @retval	None
 */
static void prvUpdateDutyValuesInGui(uint32_t channel)
{
	if (channel == 0)
	{
		/* Update the text box */
		float currentDutyCh0 = gpio0GetPwmDuty();
		GUI_ClearAndResetTextBox(GUITextBoxId_Gpio0DutyCycleValue);
		GUI_SetYWritePositionToCenter(GUITextBoxId_Gpio0DutyCycleValue);
		GUI_WriteNumberInTextBox(GUITextBoxId_Gpio0DutyCycleValue, (int32_t)currentDutyCh0);
		GUI_WriteStringInTextBox(GUITextBoxId_Gpio0DutyCycleValue, " %");
	}
	else if (channel == 1)
	{
		/* Update the text box */
		float currentDutyCh1 = gpio1GetPwmDuty();
		GUI_ClearAndResetTextBox(GUITextBoxId_Gpio1DutyCycleValue);
		GUI_SetYWritePositionToCenter(GUITextBoxId_Gpio1DutyCycleValue);
		GUI_WriteNumberInTextBox(GUITextBoxId_Gpio1DutyCycleValue, (int32_t)currentDutyCh1);
		GUI_WriteStringInTextBox(GUITextBoxId_Gpio1DutyCycleValue, " %");
	}
}

/* Interrupt Handlers --------------------------------------------------------*/
