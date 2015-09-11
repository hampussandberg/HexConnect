/**
 *******************************************************************************
 * @file    gui_templates.h
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
#ifndef GUI_TEMPLATES_H_
#define GUI_TEMPLATES_H_

/** Includes -----------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "simple_gui.h"
//#include "static_text.h"

/** Defines ------------------------------------------------------------------*/
#define COMMON_TITLE_HEIGHT   (30)

/* Common sidebar properties ================================== */
#define SIDEBAR_XPOS                      (660)
#define SIDEBAR_YPOS                      (40)
#define SIDEBAR_WIDTH                     (140)
#define SIDEBAR_HEIGHT                    (400)
#define SIDERBAR_NUM_OF_BUTTONS_PER_PAGE  (6)
#define SIDEBAR_BORDER_COLOR              (COLOR_WHITE)
#define SIDERBAR_TITLE_FONT               &font_18pt_variableWidth
#define SIDERBAR_BUTTON_FONT              &font_15pt_variableWidth

/* Common alert box properties ================================== */
#define ALERT_BOX_XPOS            (215)
#define ALERT_BOX_YPOS            (140)
#define ALERT_BOX_TITLE_HEIGHT    (COMMON_TITLE_HEIGHT)
#define ALERT_BOX_BUTTON_HEIGHT   (50)
#define ALERT_BOX_PADDING         (10)

/* Common button grid properties ================================ */
#define BUTTON_GRID_XPOS              (215)
#define BUTTON_GRID_YPOS              (150)
#define BUTTON_GRID_BOX_TITLE_HEIGHT  (COMMON_TITLE_HEIGHT)

/* N/A ======================================================== */
#define NA_REFRESH_ID_EXTRA_INDEX (0)    /* Page 1 */
#define NA_ID_INDEX               (6)    /* Page 2 */
#define NA_REFRESH_ID_INDEX       (7)
#define NA_MODULE_POWER_INDEX     (8)
#define NA_NUM_OF_PAGES           (2)

/* SETUP ====================================================== */
#define SETUP_ID_INDEX            (6)    /* Page 2 */
#define SETUP_REFRESH_ID_INDEX    (7)
#define SETUP_MODULE_POWER_INDEX  (8)
#define SETUP_NUM_OF_PAGES        (2)

/* UART ======================================================= */
#define UART_CAPTURE_INDEX            (0)   /* Page 1 */
#define UART_BAUD_RATE_INDEX          (1)
#define UART_PARITY_INDEX             (2)
#define UART_DATA_BITS_INDEX          (3)
#define UART_DISPLAY_FORMAT_INDEX     (4)
#define UART_CLEAR_INDEX              (5)
#define UART_TIMEBASE_INDEX           (6)   /* Page 2 */
#define UART_SPLITSCREEN_INDEX        (7)
#define UART_OUTPUT_RELAY_INDEX       (8)
#define UART_CHANNEL_MODE_INDEX       (9)
#define UART_MODULE_PINOUT_INDEX      (10)
#define UART_MODULE_MODE_INDEX        (11)
#define UART_ID_INDEX                 (18)  /* Page 4 */
#define UART_REFRESH_ID_INDEX         (19)
#define UART_MODULE_POWER_INDEX       (20)
#define UART_AVAILABLE_STORAGE_INDEX  (21)
#define UART_NUM_OF_PAGES             (4)

/* GPIO ======================================================= */
#define GPIO_OUTPUT_RELAY_INDEX       (0)   /* Page 1 */
#define GPIO_CHANNEL_MODE_INDEX       (1)
#define GPIO_MODULE_PINOUT_INDEX      (6)   /* Page 2 */
#define GPIO_MODULE_MODE_INDEX        (7)
#define GPIO_ID_INDEX                 (12)  /* Page 3 */
#define GPIO_REFRESH_ID_INDEX         (13)
#define GPIO_MODULE_POWER_INDEX       (14)
#define GPIO_AVAILABLE_STORAGE_INDEX  (15)
#define GPIO_NUM_OF_PAGES             (3)

/* CAN ======================================================== */
#define CAN_CAPTURE_INDEX           (0)   /* Page 1 */
#define CAN_BIT_RATE_INDEX          (1)
#define CAN_TERMINATION_INDEX       (2)
#define CAN_FILTERS_INDEX           (3)
#define CAN_DISPLAY_MODE_INDEX      (4)
#define CAN_CLEAR_INDEX             (5)
#define CAN_TIMEBASE_INDEX          (6)   /* Page 2 */
#define CAN_OUTPUT_RELAY_INDEX      (7)
#define CAN_MODULE_PINOUT_INDEX     (8)
#define CAN_ID_INDEX                (18)  /* Page 4 */
#define CAN_REFRESH_ID_INDEX        (19)
#define CAN_MODULE_POWER_INDEX      (20)
#define CAN_AVAILABLE_STORAGE_INDEX (21)
#define CAN_NUM_OF_PAGES            (4)

/* RS-232 ===================================================== */
#define RS_232_CAPTURE_INDEX            (0)   /* Page 1 */
#define RS_232_BAUD_RATE_INDEX          (1)
#define RS_232_PARITY_INDEX             (2)
#define RS_232_DATA_BITS_INDEX          (3)
#define RS_232_DISPLAY_FORMAT_INDEX     (4)
#define RS_232_CLEAR_INDEX              (5)
#define RS_232_TIMEBASE_INDEX           (6)   /* Page 2 */
#define RS_232_SPLITSCREEN_INDEX        (7)
#define RS_232_OUTPUT_RELAY_INDEX       (8)
#define RS_232_CHANNEL_MODE_INDEX       (9)
#define RS_232_MODULE_PINOUT_INDEX      (10)
#define RS_232_ID_INDEX                 (12)  /* Page 4 */
#define RS_232_REFRESH_ID_INDEX         (13)
#define RS_232_MODULE_POWER_INDEX       (14)
#define RS_232_AVAILABLE_STORAGE_INDEX  (15)
#define RS_232_NUM_OF_PAGES             (4)

/* SYSTEM ===================================================== */
#define SYSTEM_TIME_INDEX               (0)    /* Page 1 */
#define SYSTEM_VERSION_INFO_INDEX       (1)
#define SYSTEM_REFRESH_MODULE_IDS_INDEX (2)
#define SYSTEM_CLEAR_DATA_MEMORY_INDEX  (3)
#define SYSTEM_TOUCH_BUZZER_INDEX       (4)
#define SYSTEM_LCD_BRIGHTNESS_INDEX     (5)
#define SYSTEM_ENABLE_PROMPT_INDEX      (6)    /* Page 2 */
#define SYSTEM_LCD_OFF_TIMER_INDEX      (7)
#define SYSTEM_POWER_SOURCE_INDEX       (12)  /* Page 3 */
#define SYSTEM_NUM_OF_PAGES             (3)

/* Enable - Alert Box =========================================== */
#define ENABLE_ALERT_BOX_XPOS               (ALERT_BOX_XPOS)
#define ENABLE_ALERT_BOX_YPOS               (ALERT_BOX_YPOS)
#define ENABLE_ALERT_BOX_WIDTH              (230)
#define ENABLE_ALERT_BOX_HEIGHT             (150)
#define ENABLE_ALERT_BOX_FONT               &font_15pt_variableWidth
#define ENABLE_ALERT_BOX_TITLE_HEIGHT       (ALERT_BOX_TITLE_HEIGHT)
#define ENABLE_ALERT_BOX_BUTTON_HEIGHT      (ALERT_BOX_BUTTON_HEIGHT)
#define ENABLE_ALERT_BOX_PADDING_LEFT_RIGHT (ALERT_BOX_PADDING)
#define ENABLE_ALERT_BOX_PADDING_TOP_BOTTOM (ALERT_BOX_PADDING)

/* Refresh IDs - Alert Box =========================================== */
#define REFRESH_IDS_ALERT_BOX_XPOS                (ALERT_BOX_XPOS)
#define REFRESH_IDS_ALERT_BOX_YPOS                (ALERT_BOX_YPOS)
#define REFRESH_IDS_ALERT_BOX_WIDTH               (230)
#define REFRESH_IDS_ALERT_BOX_HEIGHT              (150)
#define REFRESH_IDS_ALERT_BOX_FONT                &font_15pt_variableWidth
#define REFRESH_IDS_ALERT_BOX_TITLE_HEIGHT        (ALERT_BOX_TITLE_HEIGHT)
#define REFRESH_IDS_ALERT_BOX_BUTTON_HEIGHT       (ALERT_BOX_BUTTON_HEIGHT)
#define REFRESH_IDS_ALERT_BOX_PADDING_LEFT_RIGHT  (ALERT_BOX_PADDING)
#define REFRESH_IDS_ALERT_BOX_PADDING_TOP_BOTTOM  (ALERT_BOX_PADDING)

/* Clear all memory - Alert Box ============================================ */
#define CLEAR_ALL_MEMORY_ALERT_BOX_XPOS               (ALERT_BOX_XPOS)
#define CLEAR_ALL_MEMORY_ALERT_BOX_YPOS               (ALERT_BOX_YPOS)
#define CLEAR_ALL_MEMORY_ALERT_BOX_WIDTH              (230)
#define CLEAR_ALL_MEMORY_ALERT_BOX_HEIGHT             (150)
#define CLEAR_ALL_MEMORY_ALERT_BOX_FONT               &font_15pt_variableWidth
#define CLEAR_ALL_MEMORY_ALERT_BOX_TITLE_HEIGHT       (ALERT_BOX_TITLE_HEIGHT)
#define CLEAR_ALL_MEMORY_ALERT_BOX_BUTTON_HEIGHT      (ALERT_BOX_BUTTON_HEIGHT)
#define CLEAR_ALL_MEMORY_ALERT_BOX_PADDING_LEFT_RIGHT (ALERT_BOX_PADDING)
#define CLEAR_ALL_MEMORY_ALERT_BOX_PADDING_TOP_BOTTOM (ALERT_BOX_PADDING)

/* Parity - Button Grid Box ================================================ */
#define PARITY_BUTTON_GRID_BOX_XPOS               (BUTTON_GRID_XPOS)
#define PARITY_BUTTON_GRID_BOX_YPOS               (BUTTON_GRID_YPOS)
#define PARITY_BUTTON_GRID_BOX_WIDTH              (230)
#define PARITY_BUTTON_GRID_BOX_HEIGHT             (150)
#define PARITY_BUTTON_GRID_BOX_BORDER_COLOR       (COLOR_WHITE)
#define PARITY_BUTTON_GRID_BOX_FONT               &font_15pt_variableWidth
#define PARITY_BUTTON_GRID_BOX_TITLE_HEIGHT       (BUTTON_GRID_BOX_TITLE_HEIGHT)
#define PARITY_BUTTON_GRID_BOX_PADDING_LEFT_RIGHT (10)
#define PARITY_BUTTON_GRID_BOX_PADDING_TOP_BOTTOM (10)

/* Direction - Button Grid Box ============================================= */
#define DIRECTION_BUTTON_GRID_BOX_XPOS                (BUTTON_GRID_XPOS)
#define DIRECTION_BUTTON_GRID_BOX_YPOS                (BUTTON_GRID_YPOS)
#define DIRECTION_BUTTON_GRID_BOX_WIDTH               (230)
#define DIRECTION_BUTTON_GRID_BOX_HEIGHT              (120)
#define DIRECTION_BUTTON_GRID_BOX_BORDER_COLOR        (COLOR_WHITE)
#define DIRECTION_BUTTON_GRID_BOX_FONT                &font_15pt_variableWidth
#define DIRECTION_BUTTON_GRID_BOX_TITLE_HEIGHT        (BUTTON_GRID_BOX_TITLE_HEIGHT)
#define DIRECTION_BUTTON_GRID_BOX_PADDING_LEFT_RIGHT  (10)
#define DIRECTION_BUTTON_GRID_BOX_PADDING_TOP_BOTTOM  (10)

/* Version Info - Info Box ================================================= */
#define VERSION_INFO_BOX_XPOS               (5)
#define VERSION_INFO_BOX_YPOS               (50)
#define VERSION_INFO_BOX_WIDTH              (230)
#define VERSION_INFO_BOX_HEIGHT             (165)
#define VERSION_INFO_BOX_BORDER_COLOR       (COLOR_WHITE)
#define VERSION_INFO_BOX_FONT               &font_15pt_variableWidth
#define VERSION_INFO_BOX_TITLE_HEIGHT       (25)
#define VERSION_INFO_BOX_PADDING_LEFT_RIGHT (10)
#define VERSION_INFO_BOX_PADDING_TOP_BOTTOM (10)

/** Typedefs -----------------------------------------------------------------*/
/** External variables -------------------------------------------------------*/
/* Button Lists */
extern const GUIButtonList NA_SidebarTemplate;
extern const GUIButtonList SETUP_SidebarTemplate;
extern const GUIButtonList UART_SidebarTemplate;
extern const GUIButtonList GPIO_SidebarTemplate;
extern const GUIButtonList CAN_SidebarTemplate;
extern const GUIButtonList RS_232_SidebarTemplate;
extern const GUIButtonList SYSTEM_SidebarTemplate;

/* Alert Boxes */
extern const GUIAlertBox EnableAlertBoxTemplate;
extern const GUIAlertBox RefreshIdsAlertBoxTemplate;
extern const GUIAlertBox ClearAllMemoryAlertBoxTemplate;

/* Button Grid Boxes */
extern const GUIButtonGridBox ParityButtonGridBoxTemplate;
extern const GUIButtonGridBox DirectionButtonGridBoxTemplate;

/* Info Boxes */
extern const GUIInfoBox VersionInfoBoxTemplate;

/** Function prototypes ------------------------------------------------------*/

#endif /* GUI_TEMPLATES_H_ */
