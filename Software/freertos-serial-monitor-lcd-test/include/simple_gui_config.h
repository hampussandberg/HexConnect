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
#define guiConfigNUMBER_OF_BUTTONS		9
#define guiConfigCAN1_INDEX				0
#define guiConfigCAN2_INDEX				1
#define guiConfigUART1_INDEX			2
#define guiConfigUART2_INDEX			3
#define guiConfigRS232_INDEX			4
#define guiConfigI2C_INDEX				5
#define guiConfigADC_INDEX				6
#define guiConfigGPIO_INDEX				7
#define guiConfigSETTINGS_INDEX			8

/* Typedefs ------------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/

#endif /* SIMPLE_GUI_CONFIG_H_ */
