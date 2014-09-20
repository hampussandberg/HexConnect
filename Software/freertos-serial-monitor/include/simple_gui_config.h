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
#define guiConfigBUTTON_ID_OFFSET		0
#define guiConfigTEXT_BOX_ID_OFFSET		100
#define guiConfigCONTAINER_ID_OFFSET	200

#define guiConfigINVALID_ID				1000

/*
 * Object IDs:
 * 		0-99:		Buttons
 * 		100-199:	Text box
 * 		200-299:	Containers
 */

/* Buttons */

/* CAN1 */
#define guiConfigCAN1_TOP_BUTTON_ID			0
#define guiConfigCAN1_ENABLE_BUTTON_ID		1
#define guiConfigCAN1_BIT_RATE_BUTTON_ID	2
#define guiConfigCAN1_TERMINATION_BUTTON_ID	3


/* CAN2 */
#define guiConfigCAN2_TOP_BUTTON_ID			4
#define guiConfigCAN2_ENABLE_BUTTON_ID		5
#define guiConfigCAN2_BIT_RATE_BUTTON_ID	6
#define guiConfigCAN2_TERMINATION_BUTTON_ID	7


/* UART1 */
#define guiConfigUART1_TOP_BUTTON_ID			8
#define guiConfigUART1_ENABLE_BUTTON_ID			9
#define guiConfigUART1_BAUD_RATE_BUTTON_ID		10
#define guiConfigUART1_VOLTAGE_LEVEL_BUTTON_ID	11
#define guiConfigUART1_FORMAT_BUTTON_ID			12
#define guiConfigUART1_CLEAR_BUTTON_ID			13
#define guiConfigUART1_DEBUG_BUTTON_ID			14

#define guiConfigUART1_BAUD4800_BUTTON_ID		15
#define guiConfigUART1_BAUD7200_BUTTON_ID		16
#define guiConfigUART1_BAUD9600_BUTTON_ID		17
#define guiConfigUART1_BAUD19K2_BUTTON_ID		18
#define guiConfigUART1_BAUD28K8_BUTTON_ID		19
#define guiConfigUART1_BAUD38K4_BUTTON_ID		20
#define guiConfigUART1_BAUD57K6_BUTTON_ID		21
#define guiConfigUART1_BAUD115K_BUTTON_ID		22


/* UART2 */
#define guiConfigUART2_TOP_BUTTON_ID			23
#define guiConfigUART2_ENABLE_BUTTON_ID			24
#define guiConfigUART2_BAUD_RATE_BUTTON_ID		25
#define guiConfigUART2_VOLTAGE_LEVEL_BUTTON_ID	26
#define guiConfigUART2_FORMAT_BUTTON_ID			27
#define guiConfigUART2_CLEAR_BUTTON_ID			28
#define guiConfigUART2_DEBUG_BUTTON_ID			29

#define guiConfigUART2_BAUD4800_BUTTON_ID		30
#define guiConfigUART2_BAUD7200_BUTTON_ID		31
#define guiConfigUART2_BAUD9600_BUTTON_ID		32
#define guiConfigUART2_BAUD19K2_BUTTON_ID		33
#define guiConfigUART2_BAUD28K8_BUTTON_ID		34
#define guiConfigUART2_BAUD38K4_BUTTON_ID		35
#define guiConfigUART2_BAUD57K6_BUTTON_ID		36
#define guiConfigUART2_BAUD115K_BUTTON_ID		37


/* RS232 */
#define guiConfigRS232_TOP_BUTTON_ID			38
#define guiConfigRS232_ENABLE_BUTTON_ID			39
#define guiConfigRS232_BAUD_RATE_BUTTON_ID		40
#define guiConfigRS232_FORMAT_BUTTON_ID			41
#define guiConfigRS232_CLEAR_BUTTON_ID			42
#define guiConfigRS232_DEBUG_BUTTON_ID			43

#define guiConfigRS232_BAUD4800_BUTTON_ID		44
#define guiConfigRS232_BAUD7200_BUTTON_ID		45
#define guiConfigRS232_BAUD9600_BUTTON_ID		46
#define guiConfigRS232_BAUD19K2_BUTTON_ID		47
#define guiConfigRS232_BAUD28K8_BUTTON_ID		48
#define guiConfigRS232_BAUD38K4_BUTTON_ID		49
#define guiConfigRS232_BAUD57K6_BUTTON_ID		50
#define guiConfigRS232_BAUD115K_BUTTON_ID		51


/* GPIO */
#define guiConfigGPIO_TOP_BUTTON_ID				52
#define guiConfigGPIO0_TYPE_BUTTON_ID			53
#define guiConfigGPIO0_SETTINGS_BUTTON_ID		54
#define guiConfigGPIO1_TYPE_BUTTON_ID			55
#define guiConfigGPIO1_SETTINGS_BUTTON_ID		56


/* ADC */
#define guiConfigADC_TOP_BUTTON_ID				57
#define guiConfigADC_ENABLE_BUTTON_ID			58


/* SYSTEM */
#define guiConfigSETTINGS_BUTTON_ID				59
#define guiConfigSTORAGE_BUTTON_ID				60
#define guiConfigDEBUG_BUTTON_ID				61
#define guiConfigSYSTEM_BUTTON_ID				62
#define guiConfigNUMBER_OF_BUTTONS					63


/* Text boxes */
#define guiConfigMAIN_TEXT_BOX_ID			100
#define guiConfigCLOCK_TEXT_BOX_ID			101
#define guiConfigTEMP_TEXT_BOX_ID			102
#define guiConfigDEBUG_TEXT_BOX_ID			103
#define guiConfigCAN1_LABEL_TEXT_BOX_ID		104
#define guiConfigCAN2_LABEL_TEXT_BOX_ID		105
#define guiConfigUART1_LABEL_TEXT_BOX_ID	106
#define guiConfigUART2_LABEL_TEXT_BOX_ID	107
#define guiConfigRS232_LABEL_TEXT_BOX_ID	108
#define guiConfigGPIO_LABEL_TEXT_BOX_ID		109
#define guiConfigADC_LABEL_TEXT_BOX_ID		110
#define guiConfigNUMBER_OF_TEXT_BOXES			11


/* Containers */
#define guiConfigSTATUS_CONTAINER_ID					200
#define guiConfigDEBUG_CONTAINER_ID						201
#define guiConfigSIDEBAR_SYSTEM_CONTAINER_ID			202
#define guiConfigSIDEBAR_EMPTY_CONTAINER_ID				203
#define guiConfigSIDEBAR_CAN1_CONTAINER_ID				204
#define guiConfigSIDEBAR_CAN2_CONTAINER_ID				205
#define guiConfigSIDEBAR_UART1_CONTAINER_ID				206
#define guiConfigSIDEBAR_UART2_CONTAINER_ID				207
#define guiConfigSIDEBAR_RS232_CONTAINER_ID				208
#define guiConfigSIDEBAR_GPIO_CONTAINER_ID				209
#define guiConfigSIDEBAR_ADC_CONTAINER_ID				210
#define guiConfigPOPOUT_CAN1_BIT_RATE_CONTAINER_ID		211
#define guiConfigPOPOUT_CAN2_BIT_RATE_CONTAINER_ID		212
#define guiConfigPOPOUT_UART1_BAUD_RATE_CONTAINER_ID	213
#define guiConfigPOPOUT_UART2_BAUD_RATE_CONTAINER_ID	214
#define guiConfigPOPOUT_RS232_BAUD_RATE_CONTAINER_ID	215
#define guiConfigNUMBER_OF_CONTAINERS						16

/* Typedefs ------------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/

#endif /* SIMPLE_GUI_CONFIG_H_ */
