/**
 *******************************************************************************
 * @file  gui_uart1.h
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
#ifndef GUI_UART1_H_
#define GUI_UART1_H_

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "stm32f4xx_hal.h"

#include "uart1_task.h"
#include "lcd_task.h"
#include "simple_gui.h"
#include "simple_gui_config.h"

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/
void guiUart1ManageMainTextBox(bool ShouldRefresh);
void guiUart1EnableButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiUart1VoltageLevelButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiUart1FormatButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiUart1DebugButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiUart1TopButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiUart1BaudRateButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiUart1ParityButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiUart1BaudRateSelectionCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiUart1ParitySelectionCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiUart1SidebarForwardBackwardsButtonsCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiUart1UpdateGuiElementsReadFromSettings();
void guiUart1InitGuiElements();


#endif /* GUI_UART1_H_ */
