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
#define guiConfigPAGE_ID_OFFSET			200

#define guiConfigINVALID_ID				1000

/*
 * Object IDs:
 * 		0-99:		Buttons
 * 		100-199:	Text box
 * 		200-299:	Images
 */
#define guiConfigCAN1_BUTTON_ID			0
#define guiConfigCAN2_BUTTON_ID			1
#define guiConfigUART1_BUTTON_ID		2
#define guiConfigUART2_BUTTON_ID		3
#define guiConfigRS232_BUTTON_ID		4
#define guiConfigI2C_BUTTON_ID			5
#define guiConfigADC_BUTTON_ID			6
#define guiConfigGPIO_BUTTON_ID			7
#define guiConfigSETTINGS_BUTTON_ID		8
#define guiConfigSTORAGE_BUTTON_ID		9
#define guiConfigNUMBER_OF_BUTTONS			10

#define guiConfigMAIN_TEXT_BOX_ID		100
#define guiConfigCLOCK_TEXT_BOX_ID		101
#define guiConfigTEMP_TEXT_BOX_ID		102
#define guiConfigDEBUG_TEXT_BOX_ID		103
#define guiConfigNUMBER_OF_TEXT_BOXES		4


#define guiConfigCAN1_PAGE_ID			200
#define guiConfigCAN2_PAGE_ID			201
#define guiConfigUART1_PAGE_ID			202
#define guiConfigUART2_PAGE_ID			203
#define guiConfigRS232_PAGE_ID			204
#define guiConfigGPIO_PAGE_ID			205
#define guiConfigADC_PAGE_ID			206
#define guiConfigNUMBER_OF_PAGES			7

/* Typedefs ------------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/

#endif /* SIMPLE_GUI_CONFIG_H_ */
