/**
 ******************************************************************************
 * @file	simple_gui_config.h
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-06-14
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SIMPLE_GUI_CONFIG_H_
#define SIMPLE_GUI_CONFIG_H_

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "stm32f4xx_hal.h"

/* Include the LCD driver here: */
#include "lcd_ra8875.h"

/* Defines -------------------------------------------------------------------*/

/* Colors */
#define GUI_BLUE			0x237F
#define GUI_DARK_BLUE		0x0258
#define GUI_RED				0xF926
#define GUI_DARK_RED		0xA821
#define GUI_GREEN			0x362A
#define GUI_DARK_GREEN		0x1BC6
#define GUI_YELLOW			0xFEE6
#define GUI_DARK_YELLOW		0xC560
#define GUI_PURPLE			0xA8D6
#define GUI_DARK_PURPLE		0x788F
#define GUI_GRAY			0xB596
#define GUI_MAGENTA			0xF81F
#define GUI_CYAN_LIGHT		0x1F3C
#define GUI_CYAN			0x1699
#define GUI_CYAN_DARK		0x45F7
#define GUI_CYAN_VERY_DARK	0x34B2
#define GUI_SYSTEM_BLUE		0x11CE
#define GUI_WHITE			LCD_COLOR_WHITE
#define GUI_BLACK			LCD_COLOR_BLACK

/* Num of characters */
#define GUI_MAIN_MAX_COLUMN_CHARACTERS	(81)
#define GUI_MAIN_MAX_ROW_CHARACTERS		(24)
#define GUI_MAIN_MAX_NUM_OF_CHARACTERS	(GUI_MAIN_MAX_COLUMN_CHARACTERS * (GUI_MAIN_MAX_ROW_CHARACTERS - 1))


/*
 * Object IDs:
 * 		0-199:		Buttons
 * 		200-299:	Text box
 * 		300-399:	Containers
 */
#define guiConfigBUTTON_ID_OFFSET		0
#define guiConfigTEXT_BOX_ID_OFFSET		200
#define guiConfigCONTAINER_ID_OFFSET	300

#define guiConfigINVALID_ID				1000

/* Buttons */
typedef enum
{
	/* CAN1 */
	GUIButtonId_Can1Top = guiConfigBUTTON_ID_OFFSET,
	GUIButtonId_Can1Enable,
	GUIButtonId_Can1BitRate,
	GUIButtonId_Can1Termination,
	GUIButtonId_Can1BitRate10k,
	GUIButtonId_Can1BitRate20k,
	GUIButtonId_Can1BitRate50k,
	GUIButtonId_Can1BitRate100k,
	GUIButtonId_Can1BitRate125k,
	GUIButtonId_Can1BitRate250k,
	GUIButtonId_Can1BitRate500k,
	GUIButtonId_Can1BitRate1M,

	/* CAN2 */
	GUIButtonId_Can2Top ,
	GUIButtonId_Can2Enable,
	GUIButtonId_Can2BitRate,
	GUIButtonId_Can2Termination,
	GUIButtonId_Can2BitRate10k,
	GUIButtonId_Can2BitRate20k,
	GUIButtonId_Can2BitRate50k,
	GUIButtonId_Can2BitRate100k,
	GUIButtonId_Can2BitRate125k,
	GUIButtonId_Can2BitRate250k,
	GUIButtonId_Can2BitRate500k,
	GUIButtonId_Can2BitRate1M,

	/* UART1 */
	GUIButtonId_Uart1Top,
	GUIButtonId_Uart1Enable,
	GUIButtonId_Uart1BaudRate,
	GUIButtonId_Uart1Parity,
	GUIButtonId_Uart1VoltageLevel,
	GUIButtonId_Uart1Format,
	GUIButtonId_Uart1Clear,
	GUIButtonId_Uart1Debug,
	GUIButtonId_Uart1SidebarBackwards,
	GUIButtonId_Uart1SidebarForwards,

	GUIButtonId_Uart1BaudRate4800,
	GUIButtonId_Uart1BaudRate7200,
	GUIButtonId_Uart1BaudRate9600,
	GUIButtonId_Uart1BaudRate19k2,
	GUIButtonId_Uart1BaudRate28k8,
	GUIButtonId_Uart1BaudRate38k4,
	GUIButtonId_Uart1BaudRate57k6,
	GUIButtonId_Uart1BaudRate115k,

	GUIButtonId_Uart1ParityNone,
	GUIButtonId_Uart1ParityOdd,
	GUIButtonId_Uart1ParityEven,

	/* UART2 */
	GUIButtonId_Uart2Top,
	GUIButtonId_Uart2Enable,
	GUIButtonId_Uart2BaudRate,
	GUIButtonId_Uart2Parity,
	GUIButtonId_Uart2VoltageLevel,
	GUIButtonId_Uart2Format,
	GUIButtonId_Uart2Clear,
	GUIButtonId_Uart2Debug,
	GUIButtonId_Uart2SidebarBackwards,
	GUIButtonId_Uart2SidebarForwards,

	GUIButtonId_Uart2BaudRate4800,
	GUIButtonId_Uart2BaudRate7200,
	GUIButtonId_Uart2BaudRate9600,
	GUIButtonId_Uart2BaudRate19k2,
	GUIButtonId_Uart2BaudRate28k8,
	GUIButtonId_Uart2BaudRate38k4,
	GUIButtonId_Uart2BaudRate57k6,
	GUIButtonId_Uart2BaudRate115k,

	GUIButtonId_Uart2ParityNone,
	GUIButtonId_Uart2ParityOdd,
	GUIButtonId_Uart2ParityEven,


	/* RS232 */
	GUIButtonId_Rs232Top,
	GUIButtonId_Rs232Enable,
	GUIButtonId_Rs232BaudRate,
	GUIButtonId_Rs232Parity,
	GUIButtonId_Rs232Format,
	GUIButtonId_Rs232Clear,
	GUIButtonId_Rs232Debug,
	GUIButtonId_Rs232SidebarBackwards,
	GUIButtonId_Rs232SidebarForwards,

	GUIButtonId_Rs232BaudRate4800,
	GUIButtonId_Rs232BaudRate7200,
	GUIButtonId_Rs232BaudRate9600,
	GUIButtonId_Rs232BaudRate19k2,
	GUIButtonId_Rs232BaudRate28k8,
	GUIButtonId_Rs232BaudRate38k4,
	GUIButtonId_Rs232BaudRate57k6,
	GUIButtonId_Rs232BaudRate115k,

	GUIButtonId_Rs232ParityNone,
	GUIButtonId_Rs232ParityOdd,
	GUIButtonId_Rs232ParityEven,

	/* GPIO */
	GUIButtonId_GpioTop,

	GUIButtonId_Gpio0Type,
	GUIButtonId_Gpio0TypeOut,
	GUIButtonId_Gpio0TypeIn,
	GUIButtonId_Gpio0TypePwm,
	GUIButtonId_Gpio0Enable,
	GUIButtonId_Gpio0OutHigh,
	GUIButtonId_Gpio0OutLow,
	GUIButtonId_Gpio0OutToggle,
	GUIButtonId_Gpio0PwmDutyUp,
	GUIButtonId_Gpio0PwmDutyDown,
	GUIButtonId_Gpio0PwmFreqUp,
	GUIButtonId_Gpio0PwmFreqDown,

	GUIButtonId_Gpio1Type,
	GUIButtonId_Gpio1TypeOut,
	GUIButtonId_Gpio1TypeIn,
	GUIButtonId_Gpio1TypePwm,
	GUIButtonId_Gpio1Enable,
	GUIButtonId_Gpio1OutHigh,
	GUIButtonId_Gpio1OutLow,
	GUIButtonId_Gpio1OutToggle,
	GUIButtonId_Gpio1PwmDutyUp,
	GUIButtonId_Gpio1PwmDutyDown,
	GUIButtonId_Gpio1PwmFreqUp,
	GUIButtonId_Gpio1PwmFreqDown,

	/* ADC */
	GUIButtonId_AdcTop,
	GUIButtonId_AdcEnable,

	/* SYSTEM */
	GUIButtonId_System,
	GUIButtonId_Settings,
	GUIButtonId_Storage,
	GUIButtonId_Debug,
	GUIButtonId_SaveSettings,

	/* The last item will represent how many buttons there are in total */
	GUIButtonId_NumberOfButtons,
} GUIButtonId;

#define guiConfigNUMBER_OF_BUTTONS (GUIButtonId_NumberOfButtons - guiConfigBUTTON_ID_OFFSET)


/* Text boxes */
typedef enum
{
	GUITextBoxId_Main = guiConfigTEXT_BOX_ID_OFFSET,
	GUITextBoxId_Clock,
	GUITextBoxId_Temperature,
	GUITextBoxId_Debug,

	GUITextBoxId_Can1Label,
	GUITextBoxId_Can2Label,
	GUITextBoxId_Uart1Label,
	GUITextBoxId_Uart2Label,
	GUITextBoxId_Rs232Label,
	GUITextBoxId_GpioLabel,
	GUITextBoxId_AdcLabel,
	GUITextBoxId_SystemLabel,

	GUITextBoxId_Can2Main,
	GUITextBoxId_Uart1Main,
	GUITextBoxId_Uart2Main,
	GUITextBoxId_Rs232Main,

	/* GPIO0 */
	GUITextBoxId_Gpio0Value,
	GUITextBoxId_Gpio0Label,
	GUITextBoxId_Gpio0Type,
	GUITextBoxId_Gpio0ValueLabel,
	GUITextBoxId_Gpio0DutyCycleLabel,
	GUITextBoxId_Gpio0DutyCycleValue,
	GUITextBoxId_Gpio0FrequencyLabel,
	GUITextBoxId_Gpio0FrequencyValue,

	/* GPIO1 */
	GUITextBoxId_Gpio1Value,
	GUITextBoxId_Gpio1Label,
	GUITextBoxId_Gpio1Type,
	GUITextBoxId_Gpio1ValueLabel,
	GUITextBoxId_Gpio1DutyCycleLabel,
	GUITextBoxId_Gpio1DutyCycleValue,
	GUITextBoxId_Gpio1FrequencyLabel,
	GUITextBoxId_Gpio1FrequencyValue,

	/* ADC */
	GUITextBoxId_Adc0Value,
	GUITextBoxId_Adc1Value,

	/* The last item will represent how many text boxes there are in total */
	GUITextBoxId_NumberOfTextBoxes,
} GUITextBoxId;

#define guiConfigNUMBER_OF_TEXT_BOXES (GUITextBoxId_NumberOfTextBoxes - guiConfigTEXT_BOX_ID_OFFSET)


/* Containers */
typedef enum
{
	GUIContainerId_MainContent = guiConfigCONTAINER_ID_OFFSET,
	GUIContainerId_Status,
	GUIContainerId_Debug,

	/* Sidebar containers */
	GUIContainerId_SidebarEmpty,
	GUIContainerId_SidebarSystem,
	GUIContainerId_SidebarCan1,
	GUIContainerId_SidebarCan2,
	GUIContainerId_SidebarUart1,
	GUIContainerId_SidebarUart2,
	GUIContainerId_SidebarRs232,
	GUIContainerId_SidebarGpio,
	GUIContainerId_SidebarAdc,

	/* Popout containers */
	GUIContainerId_PopoutCan1BitRate,
	GUIContainerId_PopoutCan2BitRate,
	GUIContainerId_PopoutUart1BaudRate,
	GUIContainerId_PopoutUart1Parity,
	GUIContainerId_PopoutUart2BaudRate,
	GUIContainerId_PopoutUart2Parity,
	GUIContainerId_PopoutRs232BaudRate,
	GUIContainerId_PopoutRs232Parity,
	GUIContainerId_PopoutGpio0Type,
	GUIContainerId_PopoutGpio1Type,

	GUIContainerId_Gpio0MainContent,
	GUIContainerId_Gpio1MainContent,
	GUIContainerId_AdcMainContent,

	/* The last item will represent how many containers there are in total */
	GUITextBoxId_NumberContainers,
} GUIContainerId;

#define guiConfigNUMBER_OF_CONTAINERS (GUITextBoxId_NumberContainers - guiConfigCONTAINER_ID_OFFSET)


/* Main container pages */
#define guiConfigMAIN_CONTAINER_EMPTY_PAGE				GUIContainerPage_1
#define guiConfigMAIN_CONTAINER_CAN1_PAGE				GUIContainerPage_2
#define guiConfigMAIN_CONTAINER_CAN2_PAGE				GUIContainerPage_3
#define guiConfigMAIN_CONTAINER_UART1_PAGE				GUIContainerPage_4
#define guiConfigMAIN_CONTAINER_UART2_PAGE				GUIContainerPage_5
#define guiConfigMAIN_CONTAINER_RS232_PAGE				GUIContainerPage_6
#define guiConfigMAIN_CONTAINER_GPIO_PAGE				GUIContainerPage_7
#define guiConfigMAIN_CONTAINER_ADC_PAGE				GUIContainerPage_8

/* GPIO container pages */
#define guiConfigGPIO_OUTPUT_PAGE						GUIContainerPage_1
#define guiConfigGPIO_INPUT_PAGE						GUIContainerPage_2
#define guiConfigGPIO_PWM_PAGE							GUIContainerPage_3


/* Typedefs ------------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/

#endif /* SIMPLE_GUI_CONFIG_H_ */
