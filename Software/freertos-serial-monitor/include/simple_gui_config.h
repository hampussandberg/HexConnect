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

/* Inlude the LCD driver here: */
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
 * 		200-299:	Images
 */
#define guiConfigCAN1_BUTTON_ID				0
#define guiConfigCAN1_ENABLE_BUTTON_ID		1

#define guiConfigCAN2_BUTTON_ID				2
#define guiConfigCAN2_ENABLE_BUTTON_ID		3

#define guiConfigUART1_TOP_BUTTON_ID		4
#define guiConfigUART1_ENABLE_BUTTON_ID		5

#define guiConfigUART2_BUTTON_ID			6
#define guiConfigUART2_ENABLE_BUTTON_ID		7

#define guiConfigRS232_BUTTON_ID			8
#define guiConfigRS232_ENABLE_BUTTON_ID		9

#define guiConfigGPIO_BUTTON_ID				10
#define guiConfigGPIO_ENABLE_BUTTON_ID		11

#define guiConfigADC_BUTTON_ID				12
#define guiConfigADC_ENABLE_BUTTON_ID		13

#define guiConfigSETTINGS_BUTTON_ID			14
#define guiConfigSTORAGE_BUTTON_ID			15
#define guiConfigDEBUG_BUTTON_ID			16
#define guiConfigSYSTEM_BUTTON_ID			17
#define guiConfigNUMBER_OF_BUTTONS				18

#define guiConfigMAIN_TEXT_BOX_ID		100
#define guiConfigCLOCK_TEXT_BOX_ID		101
#define guiConfigTEMP_TEXT_BOX_ID		102
#define guiConfigDEBUG_TEXT_BOX_ID		103
#define guiConfigNUMBER_OF_TEXT_BOXES		4

#define guiConfigSTATUS_CONTAINER_ID			200
#define guiConfigDEBUG_CONTAINER_ID				201
#define guiConfigSIDEBAR_SYSTEM_CONTAINER_ID	202
#define guiConfigSIDEBAR_EMPTY_CONTAINER_ID		203
#define guiConfigSIDEBAR_CAN1_CONTAINER_ID		204
#define guiConfigSIDEBAR_CAN2_CONTAINER_ID		205
#define guiConfigSIDEBAR_UART1_CONTAINER_ID		206
#define guiConfigSIDEBAR_UART2_CONTAINER_ID		207
#define guiConfigSIDEBAR_RS232_CONTAINER_ID		208
#define guiConfigSIDEBAR_GPIO_CONTAINER_ID		209
#define guiConfigSIDEBAR_ADC_CONTAINER_ID		210
#define guiConfigNUMBER_OF_CONTAINERS				11

/* Typedefs ------------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/

#endif /* SIMPLE_GUI_CONFIG_H_ */
