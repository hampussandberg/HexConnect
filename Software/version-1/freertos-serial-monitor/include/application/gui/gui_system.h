/**
 *******************************************************************************
 * @file  gui_system.h
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
#ifndef GUI_SYSTEM_H_
#define GUI_SYSTEM_H_

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "stm32f4xx_hal.h"

#include "can1_task.h"
#include "can2_task.h"
#include "uart1_task.h"
#include "uart2_task.h"
#include "rs232_task.h"
#include "lcd_task.h"
#include "simple_gui.h"
#include "simple_gui_config.h"

#include "spi_flash.h"

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/
void guiSystemInitGuiElements();
void guiDebugToggleCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiSystemButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiSaveSettingsButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiBeepButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiScreenBrightnessButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiScreenBrightnessCallback(GUITouchEvent Event, uint32_t ButtonId);


#endif /* GUI_SYSTEM_H_ */
