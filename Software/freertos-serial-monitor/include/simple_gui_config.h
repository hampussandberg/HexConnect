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

/* CAN1 */
#define guiConfigCAN1_TOP_BUTTON_ID				0
#define guiConfigCAN1_ENABLE_BUTTON_ID			1
#define guiConfigCAN1_BIT_RATE_BUTTON_ID		2
#define guiConfigCAN1_TERMINATION_BUTTON_ID		3

#define guiConfigCAN1_BIT10K_BUTTON_ID			4
#define guiConfigCAN1_BIT20K_BUTTON_ID			5
#define guiConfigCAN1_BIT50K_BUTTON_ID			6
#define guiConfigCAN1_BIT100K_BUTTON_ID			7
#define guiConfigCAN1_BIT125K_BUTTON_ID			8
#define guiConfigCAN1_BIT250K_BUTTON_ID			9
#define guiConfigCAN1_BIT500K_BUTTON_ID			10
#define guiConfigCAN1_BIT1M_BUTTON_ID			11


/* CAN2 */
#define guiConfigCAN2_TOP_BUTTON_ID				12
#define guiConfigCAN2_ENABLE_BUTTON_ID			13
#define guiConfigCAN2_BIT_RATE_BUTTON_ID		14
#define guiConfigCAN2_TERMINATION_BUTTON_ID		15

#define guiConfigCAN2_BIT10K_BUTTON_ID			16
#define guiConfigCAN2_BIT20K_BUTTON_ID			17
#define guiConfigCAN2_BIT50K_BUTTON_ID			18
#define guiConfigCAN2_BIT100K_BUTTON_ID			19
#define guiConfigCAN2_BIT125K_BUTTON_ID			20
#define guiConfigCAN2_BIT250K_BUTTON_ID			21
#define guiConfigCAN2_BIT500K_BUTTON_ID			22
#define guiConfigCAN2_BIT1M_BUTTON_ID			23


/* UART1 */
#define guiConfigUART1_TOP_BUTTON_ID			24
#define guiConfigUART1_ENABLE_BUTTON_ID			25
#define guiConfigUART1_BAUD_RATE_BUTTON_ID		26
#define guiConfigUART1_VOLTAGE_LEVEL_BUTTON_ID	27
#define guiConfigUART1_FORMAT_BUTTON_ID			28
#define guiConfigUART1_CLEAR_BUTTON_ID			29
#define guiConfigUART1_DEBUG_BUTTON_ID			30

#define guiConfigUART1_BAUD4800_BUTTON_ID		31
#define guiConfigUART1_BAUD7200_BUTTON_ID		32
#define guiConfigUART1_BAUD9600_BUTTON_ID		33
#define guiConfigUART1_BAUD19K2_BUTTON_ID		34
#define guiConfigUART1_BAUD28K8_BUTTON_ID		35
#define guiConfigUART1_BAUD38K4_BUTTON_ID		36
#define guiConfigUART1_BAUD57K6_BUTTON_ID		37
#define guiConfigUART1_BAUD115K_BUTTON_ID		38


/* UART2 */
#define guiConfigUART2_TOP_BUTTON_ID			39
#define guiConfigUART2_ENABLE_BUTTON_ID			40
#define guiConfigUART2_BAUD_RATE_BUTTON_ID		41
#define guiConfigUART2_VOLTAGE_LEVEL_BUTTON_ID	42
#define guiConfigUART2_FORMAT_BUTTON_ID			43
#define guiConfigUART2_CLEAR_BUTTON_ID			44
#define guiConfigUART2_DEBUG_BUTTON_ID			45

#define guiConfigUART2_BAUD4800_BUTTON_ID		46
#define guiConfigUART2_BAUD7200_BUTTON_ID		47
#define guiConfigUART2_BAUD9600_BUTTON_ID		48
#define guiConfigUART2_BAUD19K2_BUTTON_ID		49
#define guiConfigUART2_BAUD28K8_BUTTON_ID		50
#define guiConfigUART2_BAUD38K4_BUTTON_ID		51
#define guiConfigUART2_BAUD57K6_BUTTON_ID		52
#define guiConfigUART2_BAUD115K_BUTTON_ID		53


/* RS232 */
#define guiConfigRS232_TOP_BUTTON_ID			54
#define guiConfigRS232_ENABLE_BUTTON_ID			55
#define guiConfigRS232_BAUD_RATE_BUTTON_ID		56
#define guiConfigRS232_FORMAT_BUTTON_ID			57
#define guiConfigRS232_CLEAR_BUTTON_ID			58
#define guiConfigRS232_DEBUG_BUTTON_ID			59

#define guiConfigRS232_BAUD4800_BUTTON_ID		60
#define guiConfigRS232_BAUD7200_BUTTON_ID		61
#define guiConfigRS232_BAUD9600_BUTTON_ID		62
#define guiConfigRS232_BAUD19K2_BUTTON_ID		63
#define guiConfigRS232_BAUD28K8_BUTTON_ID		64
#define guiConfigRS232_BAUD38K4_BUTTON_ID		65
#define guiConfigRS232_BAUD57K6_BUTTON_ID		66
#define guiConfigRS232_BAUD115K_BUTTON_ID		67


/* GPIO */
#define guiConfigGPIO_TOP_BUTTON_ID				68
#define guiConfigGPIO0_TYPE_BUTTON_ID			69
#define guiConfigGPIO1_TYPE_BUTTON_ID			70

#define guiConfigGPIO0_TYPE_OUT_BUTTON_ID		71
#define guiConfigGPIO0_TYPE_IN_BUTTON_ID		72
#define guiConfigGPIO0_TYPE_PWM_BUTTON_ID		73
#define guiConfigGPIO1_TYPE_OUT_BUTTON_ID		74
#define guiConfigGPIO1_TYPE_IN_BUTTON_ID		75
#define guiConfigGPIO1_TYPE_PWM_BUTTON_ID		76

#define guiConfigGPIO0_ENABLE_BUTTON_ID			77
#define guiConfigGPIO0_OUT_HIGH_BUTTON_ID		78
#define guiConfigGPIO0_OUT_LOW_BUTTON_ID		79
#define guiConfigGPIO0_OUT_TOGGLE_BUTTON_ID		80
#define guiConfigGPIO0_PWM_DUTY_UP_BUTTON_ID	81
#define guiConfigGPIO0_PWM_DUTY_DOWN_BUTTON_ID	82
#define guiConfigGPIO0_PWM_FREQ_UP_BUTTON_ID	83
#define guiConfigGPIO0_PWM_FREQ_DOWN_BUTTON_ID	84

#define guiConfigGPIO1_ENABLE_BUTTON_ID			85
#define guiConfigGPIO1_OUT_HIGH_BUTTON_ID		86
#define guiConfigGPIO1_OUT_LOW_BUTTON_ID		87
#define guiConfigGPIO1_OUT_TOGGLE_BUTTON_ID		88
#define guiConfigGPIO1_PWM_DUTY_UP_BUTTON_ID	89
#define guiConfigGPIO1_PWM_DUTY_DOWN_BUTTON_ID	90
#define guiConfigGPIO1_PWM_FREQ_UP_BUTTON_ID	91
#define guiConfigGPIO1_PWM_FREQ_DOWN_BUTTON_ID	92


/* ADC */
#define guiConfigADC_TOP_BUTTON_ID				93
#define guiConfigADC_ENABLE_BUTTON_ID			94


/* SYSTEM */
#define guiConfigSETTINGS_BUTTON_ID				95
#define guiConfigSTORAGE_BUTTON_ID				96
#define guiConfigDEBUG_BUTTON_ID				97
#define guiConfigSYSTEM_BUTTON_ID				98
#define guiConfigSAVE_SETTINGS_BUTTON_ID		99
#define guiConfigNUMBER_OF_BUTTONS					100


/* Text boxes */
#define guiConfigMAIN_TEXT_BOX_ID				200
#define guiConfigCLOCK_TEXT_BOX_ID				201
#define guiConfigTEMP_TEXT_BOX_ID				202
#define guiConfigDEBUG_TEXT_BOX_ID				203
#define guiConfigCAN1_LABEL_TEXT_BOX_ID			204
#define guiConfigCAN2_LABEL_TEXT_BOX_ID			205
#define guiConfigUART1_LABEL_TEXT_BOX_ID		206
#define guiConfigUART2_LABEL_TEXT_BOX_ID		207
#define guiConfigRS232_LABEL_TEXT_BOX_ID		208
#define guiConfigGPIO_LABEL_TEXT_BOX_ID			209
#define guiConfigADC_LABEL_TEXT_BOX_ID			210

#define guiConfigCAN2_MAIN_TEXT_BOX_ID			211
#define guiConfigUART1_MAIN_TEXT_BOX_ID			212
#define guiConfigUART2_MAIN_TEXT_BOX_ID			213
#define guiConfigRS232_MAIN_TEXT_BOX_ID			214

#define guiConfigGPIO0_VALUE_TEXT_BOX_ID		215
#define guiConfigGPIO0_LABEL_TEXT_BOX_ID		216
#define guiConfigGPIO0_TYPE_TEXT_BOX_ID			217
#define guiConfigGPIO0_VALUE_LABEL_TEXT_BOX_ID	218
#define guiConfigGPIO0_DUTY_TEXT_BOX_ID			219
#define guiConfigGPIO0_DUTY_VALUE_TEXT_BOX_ID	220
#define guiConfigGPIO0_FREQ_TEXT_BOX_ID			221
#define guiConfigGPIO0_FREQ_VALUE_TEXT_BOX_ID	222

#define guiConfigGPIO1_VALUE_TEXT_BOX_ID		223
#define guiConfigGPIO1_LABEL_TEXT_BOX_ID		224
#define guiConfigGPIO1_TYPE_TEXT_BOX_ID			225
#define guiConfigGPIO1_VALUE_LABEL_TEXT_BOX_ID	226
#define guiConfigGPIO1_DUTY_TEXT_BOX_ID			227
#define guiConfigGPIO1_DUTY_VALUE_TEXT_BOX_ID	228
#define guiConfigGPIO1_FREQ_TEXT_BOX_ID			229
#define guiConfigGPIO1_FREQ_VALUE_TEXT_BOX_ID	230
#define guiConfigNUMBER_OF_TEXT_BOXES				31


/* Containers */
#define guiConfigMAIN_CONTENT_CONTAINER_ID				300
#define guiConfigSTATUS_CONTAINER_ID					301
#define guiConfigDEBUG_CONTAINER_ID						302
#define guiConfigSIDEBAR_SYSTEM_CONTAINER_ID			303
#define guiConfigSIDEBAR_EMPTY_CONTAINER_ID				304
#define guiConfigSIDEBAR_CAN1_CONTAINER_ID				305
#define guiConfigSIDEBAR_CAN2_CONTAINER_ID				306
#define guiConfigSIDEBAR_UART1_CONTAINER_ID				307
#define guiConfigSIDEBAR_UART2_CONTAINER_ID				308
#define guiConfigSIDEBAR_RS232_CONTAINER_ID				309
#define guiConfigSIDEBAR_GPIO_CONTAINER_ID				310
#define guiConfigSIDEBAR_ADC_CONTAINER_ID				311

#define guiConfigPOPOUT_CAN1_BIT_RATE_CONTAINER_ID		312
#define guiConfigPOPOUT_CAN2_BIT_RATE_CONTAINER_ID		313
#define guiConfigPOPOUT_UART1_BAUD_RATE_CONTAINER_ID	314
#define guiConfigPOPOUT_UART2_BAUD_RATE_CONTAINER_ID	315
#define guiConfigPOPOUT_RS232_BAUD_RATE_CONTAINER_ID	316
#define guiConfigPOPOUT_GPIO0_TYPE_CONTAINER_ID			317
#define guiConfigPOPOUT_GPIO1_TYPE_CONTAINER_ID			318

#define guiConfigMAIN_GPIO0_CONTAINER_ID				319
#define guiConfigMAIN_GPIO1_CONTAINER_ID				320
#define guiConfigNUMBER_OF_CONTAINERS						21


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
