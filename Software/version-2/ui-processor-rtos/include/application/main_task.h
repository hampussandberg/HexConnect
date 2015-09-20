/**
 *******************************************************************************
 * @file    main_task.h
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-08-29
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

/** Define to prevent recursive inclusion ------------------------------------*/
#ifndef MAIN_TASK_H_
#define MAIN_TASK_H_

/** Includes -----------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "gui_templates.h"

/** Defines ------------------------------------------------------------------*/
/** Typedefs -----------------------------------------------------------------*/
typedef enum
{
  APP_ActiveSidebar_1 = 0,
  APP_ActiveSidebar_2,
  APP_ActiveSidebar_3,
  APP_ActiveSidebar_4,
  APP_ActiveSidebar_5,
  APP_ActiveSidebar_6,
  APP_ActiveSidebar_System,
  APP_ActiveSidebar_None,
} APP_ActiveSidebar;
#define IS_APP_ACTIVE_SIDEBAR(X)  (X == APP_ActiveSidebar_1 || X == APP_ActiveSidebar_2 || X == APP_ActiveSidebar_3 || \
                                   X == APP_ActiveSidebar_4 || X == APP_ActiveSidebar_5 || X == APP_ActiveSidebar_6 || \
                                   X == APP_ActiveSidebar_System  || X == APP_ActiveSidebar_None)
#define ACTIVE_SIDEBAR_IS_FOR_A_CHANNEL(X)  (X == APP_ActiveSidebar_1 || X == APP_ActiveSidebar_2 || X == APP_ActiveSidebar_3 || \
                                             X == APP_ActiveSidebar_4 || X == APP_ActiveSidebar_5 || X == APP_ActiveSidebar_6)

typedef enum
{
  APP_ModuleIdType_NA     = 0x00,
  APP_ModuleIdType_GPIO   = 0x01,
  APP_ModuleIdType_CAN    = 0x03,
  APP_ModuleIdType_RS_232 = 0x05,
} APP_ModuleIdType;
#define IS_VALID_MODULE_ID(X) (X == APP_ModuleIdType_NA || X == APP_ModuleIdType_GPIO || \
                               X == APP_ModuleIdType_CAN || X == APP_ModuleIdType_RS_232)

typedef enum
{
  APP_ChannelType_NA      = 0,
  APP_ChannelType_SETUP   = 1,
  APP_ChannelType_UART    = 2,
  APP_ChannelType_GPIO    = 3,
  APP_ChannelType_CAN     = 4,
  APP_ChannelType_RS_232  = 5,
} APP_ChannelType;
#define IS_APP_CHANNEL_TYPE(X)  ( X == APP_ChannelType_NA || X == APP_ChannelType_SETUP || X == APP_ChannelType_UART || \
                                  X == APP_ChannelType_GPIO || X == APP_ChannelType_CAN || X == APP_ChannelType_RS_232)

typedef enum
{
  APP_BuzzerSound_Off,
  APP_BuzzerSound_Low,
  APP_BuzzerSound_Medium,
  APP_BuzzerSound_High,
} APP_BuzzerSound;

typedef enum
{
  APP_PowerSource_ExternalModule,
  APP_PowerSource_USB,
  APP_PowerSource_Battery,
} APP_PowerSource;

typedef enum
{
  APP_ChannelDirection_Off,
  APP_ChannelDirection_TX,
  APP_ChannelDirection_RX,
} APP_ChannelDirection;

typedef enum
{
  APP_Parity_None,
  APP_Parity_Odd,
  APP_Parity_Even,
} APP_Parity;

/** Function prototypes ------------------------------------------------------*/
void mainTask(void *pvParameters);
void MAIN_TASK_NotifyLcdTaskIsDone();


#endif /* MAIN_TASK_H_ */
