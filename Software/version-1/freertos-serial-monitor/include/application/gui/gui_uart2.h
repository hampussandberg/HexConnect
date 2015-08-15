/**
 *******************************************************************************
 * @file  gui_uart2.h
 * @author  Hampus Sandberg
 * @version 0.1
 * @date  2015-08-15
 * @brief
 *******************************************************************************
  Copyright (c) 2015 Hampus Sandberg.

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef GUI_UART2_H_
#define GUI_UART2_H_

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "stm32f4xx_hal.h"

#include "uart2_task.h"
#include "lcd_task.h"
#include "simple_gui.h"
#include "simple_gui_config.h"

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/
void guiUart2ManageMainTextBox(bool ShouldRefresh);
void guiUart2EnableButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiUart2VoltageLevelButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiUart2FormatButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiUart2DebugButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiUart2TopButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiUart2BaudRateButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiUart2ParityButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiUart2BaudRateSelectionCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiUart2ParitySelectionCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiUart2SidebarForwardBackwardsButtonsCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiUart2UpdateGuiElementsReadFromSettings();
void guiUart2InitGuiElements();


#endif /* GUI_UART2_H_ */
