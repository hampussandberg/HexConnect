/**
 ******************************************************************************
 * @file	gui_can1.h
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef GUI_CAN1_H_
#define GUI_CAN1_H_

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "stm32f4xx_hal.h"

#include "can1_task.h"
#include "lcd_task.h"
#include "simple_gui.h"
#include "simple_gui_config.h"

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/
void guiCan1ManageMainTextBox(bool ShouldRefresh);
void guiCan1EnableButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiCan1TerminationButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiCan1TopButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiCan1BitRateButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiCan1BitRateSelectionCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiCan1UpdateGuiElementsReadFromSettings();
void guiCan1InitGuiElements();


#endif /* GUI_CAN1_H_ */
