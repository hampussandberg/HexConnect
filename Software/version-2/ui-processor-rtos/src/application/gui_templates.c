/**
 *******************************************************************************
 * @file    gui_templates.c
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

/** Includes -----------------------------------------------------------------*/
#include "gui_templates.h"

/** Private typedefs ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
/** Private function prototypes ----------------------------------------------*/
/** Functions ----------------------------------------------------------------*/

/** Button Lists -------------------------------------------------------------*/
/* N/A ======================================================= */
const GUIButtonList NA_SidebarTemplate = {
  .object.id              = GUI_INVALID_ID,
  .object.xPos            = SIDEBAR_XPOS,
  .object.yPos            = SIDEBAR_YPOS,
  .object.width           = SIDEBAR_WIDTH,
  .object.height          = SIDEBAR_HEIGHT,
  .object.displayState    = GUIDisplayState_Hidden,
  .object.border          = SIDEBAR_BORDER,
  .object.borderThickness = 2,
  .object.borderColor     = SIDEBAR_BORDER_COLOR,
  .object.layer           = GUILayer_1,

  .backgroundColor                = COLOR_ERROR,
  .titleBackgroundColor           = COLOR_ERROR,
  .titleTextColor[0]              = COLOR_ERROR,
  .titleTextColor[1]              = COLOR_ERROR,
  .buttonsState1TextColor         = COLOR_ERROR,
  .buttonsState1BackgroundColor   = COLOR_ERROR,
  .buttonsState2TextColor         = COLOR_ERROR,
  .buttonsState2BackgroundColor   = COLOR_ERROR,
  .buttonsPressedTextColor        = COLOR_ERROR,
  .buttonsPressedBackgroundColor  = COLOR_ERROR,

  .buttonsBorder  = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom,
  .previousBorder = GUIBorder_Left | GUIBorder_Right | GUIBorder_Top | GUIBorder_Bottom,
  .nextBorder     = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom,

  .actionButtonPressed = 0,

  .title[0][0]  = "N/A (1/2)",
  .title[1][0]  = "N/A (2/2)",
  .titleEnabled = true,
  .titleFont    = SIDERBAR_TITLE_FONT,

    /* First page */
  .buttonText[NA_REFRESH_ID_EXTRA_INDEX][0] = "Refresh ID",

    /* Second Page */
  .buttonText[NA_ID_INDEX][0]           = "ID",
  .buttonText[NA_ID_INDEX][1]           = "0.000 V (No 0)",  /* TODO: */
  .buttonIsStaticText[NA_ID_INDEX]      = true,

  .buttonText[NA_REFRESH_ID_INDEX][0]   = "Refresh ID",

  .buttonText[NA_MODULE_POWER_INDEX][0] = "Module Power:",
  .buttonText[NA_MODULE_POWER_INDEX][1] = "X",

  .buttonFont           = SIDERBAR_BUTTON_FONT,
  .numOfPages           = NA_NUM_OF_PAGES,
  .numOfButtonsPerPage  = SIDERBAR_NUM_OF_BUTTONS_PER_PAGE,
};

/* SETUP ======================================================= */
const GUIButtonList SETUP_SidebarTemplate = {
  .object.id              = GUI_INVALID_ID,
  .object.xPos            = SIDEBAR_XPOS,
  .object.yPos            = SIDEBAR_YPOS,
  .object.width           = SIDEBAR_WIDTH,
  .object.height          = SIDEBAR_HEIGHT,
  .object.displayState    = GUIDisplayState_Hidden,
  .object.border          = SIDEBAR_BORDER,
  .object.borderThickness = 2,
  .object.borderColor     = SIDEBAR_BORDER_COLOR,
  .object.layer           = GUILayer_1,

  .backgroundColor                = COLOR_ERROR,
  .titleBackgroundColor           = COLOR_ERROR,
  .titleTextColor[0]              = COLOR_ERROR,
  .titleTextColor[1]              = COLOR_ERROR,
  .buttonsState1TextColor         = COLOR_ERROR,
  .buttonsState1BackgroundColor   = COLOR_ERROR,
  .buttonsState2TextColor         = COLOR_ERROR,
  .buttonsState2BackgroundColor   = COLOR_ERROR,
  .buttonsPressedTextColor        = COLOR_ERROR,
  .buttonsPressedBackgroundColor  = COLOR_ERROR,

  .buttonsBorder  = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom,
  .previousBorder = GUIBorder_Left | GUIBorder_Right | GUIBorder_Top | GUIBorder_Bottom,
  .nextBorder     = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom,

  .actionButtonPressed = 0,

  .title[0][0]  = "Setup! (1/2)",
  .title[1][0]  = "Setup! (2/2)",
  .titleEnabled = true,
  .titleFont    = SIDERBAR_TITLE_FONT,

    /* Second Page */
  .buttonText[SETUP_ID_INDEX][0]            = "ID",
  .buttonText[SETUP_ID_INDEX][1]            = "0.000 V (No 0)",  /* TODO: */
  .buttonIsStaticText[SETUP_ID_INDEX]       = true,

  .buttonText[SETUP_REFRESH_ID_INDEX][0]    = "Refresh ID",

  .buttonText[SETUP_MODULE_POWER_INDEX][0]  = "Module Power:",
  .buttonText[SETUP_MODULE_POWER_INDEX][1]  = "X",

  .buttonFont           = SIDERBAR_BUTTON_FONT,
  .numOfPages           = SETUP_NUM_OF_PAGES,
  .numOfButtonsPerPage  = SIDERBAR_NUM_OF_BUTTONS_PER_PAGE,
};

/* UART ======================================================= */
const GUIButtonList UART_SidebarTemplate = {
  .object.id              = GUI_INVALID_ID,
  .object.xPos            = SIDEBAR_XPOS,
  .object.yPos            = SIDEBAR_YPOS,
  .object.width           = SIDEBAR_WIDTH,
  .object.height          = SIDEBAR_HEIGHT,
  .object.displayState    = GUIDisplayState_Hidden,
  .object.border          = SIDEBAR_BORDER,
  .object.borderThickness = 2,
  .object.borderColor     = SIDEBAR_BORDER_COLOR,
  .object.layer           = GUILayer_1,

  .backgroundColor                = COLOR_ERROR,
  .titleBackgroundColor           = COLOR_ERROR,
  .titleTextColor[0]              = COLOR_ERROR,
  .titleTextColor[1]              = COLOR_ERROR,
  .buttonsState1TextColor         = COLOR_ERROR,
  .buttonsState1BackgroundColor   = COLOR_ERROR,
  .buttonsState2TextColor         = COLOR_ERROR,
  .buttonsState2BackgroundColor   = COLOR_ERROR,
  .buttonsPressedTextColor        = COLOR_ERROR,
  .buttonsPressedBackgroundColor  = COLOR_ERROR,

  .buttonsBorder  = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom,
  .previousBorder = GUIBorder_Left | GUIBorder_Right | GUIBorder_Top | GUIBorder_Bottom,
  .nextBorder     = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom,

  .actionButtonPressed = 0,

  .title[0][0]  = "UART (1/4)",
  .title[1][0]  = "UART (2/4)",
  .title[2][0]  = "UART (3/4)",
  .title[3][0]  = "UART (4/4)",
  .titleEnabled = true,
  .titleFont    = SIDERBAR_TITLE_FONT,

  /* First page */
  .buttonText[UART_CAPTURE_INDEX][0]        = "Start Capture",

  .buttonText[UART_BAUD_RATE_INDEX][0]      = "< Baud Rate:",
  .buttonText[UART_BAUD_RATE_INDEX][1]      = "X",

  .buttonText[UART_PARITY_INDEX][0]         = "< Parity:",
  .buttonText[UART_PARITY_INDEX][1]         = "X",

  .buttonText[UART_DATA_BITS_INDEX][0]      = "< Data Bits:",
  .buttonText[UART_DATA_BITS_INDEX][1]      = "X",

  .buttonText[UART_DISPLAY_FORMAT_INDEX][0] = "< Display Format:",
  .buttonText[UART_DISPLAY_FORMAT_INDEX][1] = "X",

  .buttonText[UART_CLEAR_INDEX][0]          = "Clear",

  /* Second Page */
  .buttonText[UART_TIMEBASE_INDEX][0]       = "< Timebase:",
  .buttonText[UART_TIMEBASE_INDEX][1]       = "X",

  .buttonText[UART_SPLITSCREEN_INDEX][0]    = "Splitscreen:",
  .buttonText[UART_SPLITSCREEN_INDEX][1]    = "X",

  .buttonText[UART_OUTPUT_RELAY_INDEX][0]   = "< Output Relay:",
  .buttonText[UART_OUTPUT_RELAY_INDEX][1]   = "Active on capture",

  .buttonText[UART_CHANNEL_MODE_INDEX][0]         = "< Channel Mode",
  .buttonText[UART_CHANNEL_MODE_INDEX][1]         = "X",

  .buttonText[UART_MODULE_PINOUT_INDEX][0]         = "< Module Pinout",

  .buttonText[UART_MODULE_MODE_INDEX][0]    = "< Module Mode:",
  .buttonText[UART_MODULE_MODE_INDEX][1]    = "UART",

  /* Fourth page */
  .buttonText[UART_ID_INDEX][0]       = "ID:",
  .buttonText[UART_ID_INDEX][1]       = "0.155 V (No 1)",  /* TODO: */
  .buttonIsStaticText[UART_ID_INDEX]  = true,

  .buttonText[UART_REFRESH_ID_INDEX][0] = "Refresh ID",

  .buttonText[UART_MODULE_POWER_INDEX][0] = "Module Power:",
  .buttonText[UART_MODULE_POWER_INDEX][1] = "X",

  .buttonText[UART_AVAILABLE_STORAGE_INDEX][0] = "Available Storage:",
  .buttonText[UART_AVAILABLE_STORAGE_INDEX][1] = "X",
  .buttonIsStaticText[UART_AVAILABLE_STORAGE_INDEX] = true,

  .buttonFont           = SIDERBAR_BUTTON_FONT,
  .numOfPages           = UART_NUM_OF_PAGES,
  .numOfButtonsPerPage  = SIDERBAR_NUM_OF_BUTTONS_PER_PAGE,
};

/* GPIO ======================================================= */
const GUIButtonList GPIO_SidebarTemplate = {
  .object.id              = GUI_INVALID_ID,
  .object.xPos            = SIDEBAR_XPOS,
  .object.yPos            = SIDEBAR_YPOS,
  .object.width           = SIDEBAR_WIDTH,
  .object.height          = SIDEBAR_HEIGHT,
  .object.displayState    = GUIDisplayState_Hidden,
  .object.border          = SIDEBAR_BORDER,
  .object.borderThickness = 2,
  .object.borderColor     = SIDEBAR_BORDER_COLOR,
  .object.layer           = GUILayer_1,

  .backgroundColor                = COLOR_ERROR,
  .titleBackgroundColor           = COLOR_ERROR,
  .titleTextColor[0]              = COLOR_ERROR,
  .titleTextColor[1]              = COLOR_ERROR,
  .buttonsState1TextColor         = COLOR_ERROR,
  .buttonsState1BackgroundColor   = COLOR_ERROR,
  .buttonsState2TextColor         = COLOR_ERROR,
  .buttonsState2BackgroundColor   = COLOR_ERROR,
  .buttonsPressedTextColor        = COLOR_ERROR,
  .buttonsPressedBackgroundColor  = COLOR_ERROR,

  .buttonsBorder  = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom,
  .previousBorder = GUIBorder_Left | GUIBorder_Right | GUIBorder_Top | GUIBorder_Bottom,
  .nextBorder     = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom,

  .actionButtonPressed = 0,

  .title[0][0]  = "GPIO (1/3)",
  .title[1][0]  = "GPIO (2/3)",
  .title[2][0]  = "GPIO (3/3)",
  .titleEnabled = true,
  .titleFont    = SIDERBAR_TITLE_FONT,

  /* First page */
  .buttonText[GPIO_OUTPUT_RELAY_INDEX][0]   = "Start Capture",

  .buttonText[GPIO_CHANNEL_MODE_INDEX][0]   = "< Channel Mode:",
  .buttonText[GPIO_CHANNEL_MODE_INDEX][1]   = "A: Out - B: PWM",

  /* Second Page */
  .buttonText[GPIO_MODULE_PINOUT_INDEX][0]  = "< Module Pinout",

  .buttonText[GPIO_MODULE_MODE_INDEX][0]    = "< Module Mode:",
  .buttonText[GPIO_MODULE_MODE_INDEX][1]    = "GPIO",

  /* Third page */
  .buttonText[GPIO_ID_INDEX][0]             = "ID:",
  .buttonText[GPIO_ID_INDEX][1]             = "0.155 V (No 1)",  /* TODO: */
  .buttonIsStaticText[GPIO_ID_INDEX]        = true,

  .buttonText[GPIO_REFRESH_ID_INDEX][0]     = "Refresh ID",

  .buttonText[GPIO_MODULE_POWER_INDEX][0]   = "Module Power:",
  .buttonText[GPIO_MODULE_POWER_INDEX][1]   = "X",

  .buttonText[GPIO_AVAILABLE_STORAGE_INDEX][0] = "Available Storage",
  .buttonText[GPIO_AVAILABLE_STORAGE_INDEX][1] = "1024 kB",  /* TODO: */
  .buttonIsStaticText[GPIO_AVAILABLE_STORAGE_INDEX] = true,

  .buttonFont           = SIDERBAR_BUTTON_FONT,
  .numOfPages           = GPIO_NUM_OF_PAGES,
  .numOfButtonsPerPage  = SIDERBAR_NUM_OF_BUTTONS_PER_PAGE,
};

/* CAN ======================================================== */
const GUIButtonList CAN_SidebarTemplate = {
  .object.id              = GUI_INVALID_ID,
  .object.xPos            = SIDEBAR_XPOS,
  .object.yPos            = SIDEBAR_YPOS,
  .object.width           = SIDEBAR_WIDTH,
  .object.height          = SIDEBAR_HEIGHT,
  .object.displayState    = GUIDisplayState_Hidden,
  .object.border          = SIDEBAR_BORDER,
  .object.borderThickness = 2,
  .object.borderColor     = SIDEBAR_BORDER_COLOR,
  .object.layer           = GUILayer_1,

  .backgroundColor                = COLOR_ERROR,
  .titleBackgroundColor           = COLOR_ERROR,
  .titleTextColor[0]              = COLOR_ERROR,
  .titleTextColor[1]              = COLOR_ERROR,
  .buttonsState1TextColor         = COLOR_ERROR,
  .buttonsState1BackgroundColor   = COLOR_ERROR,
  .buttonsState2TextColor         = COLOR_ERROR,
  .buttonsState2BackgroundColor   = COLOR_ERROR,
  .buttonsPressedTextColor        = COLOR_ERROR,
  .buttonsPressedBackgroundColor  = COLOR_ERROR,

  .buttonsBorder  = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom,
  .previousBorder = GUIBorder_Left | GUIBorder_Right | GUIBorder_Top | GUIBorder_Bottom,
  .nextBorder     = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom,

  .actionButtonPressed = 0,

  .title[0][0]  = "CAN (1/4)",
  .title[1][0]  = "CAN (2/4)",
  .title[2][0]  = "CAN (3/4)",
  .title[3][0]  = "CAN (4/4)",
  .titleEnabled = true,
  .titleFont    = SIDERBAR_TITLE_FONT,

  /* First page */
  .buttonText[CAN_CAPTURE_INDEX][0]       = "Start Capture",

  .buttonText[CAN_BIT_RATE_INDEX][0]      = "< Bit Rate:",
  .buttonText[CAN_BIT_RATE_INDEX][1]      = "X",

  .buttonText[CAN_TERMINATION_INDEX][0]   = "< Termination:",
  .buttonText[CAN_TERMINATION_INDEX][1]   = "X",

  .buttonText[CAN_FILTERS_INDEX][0]       = "< Filters:",
  .buttonText[CAN_FILTERS_INDEX][1]       = "X/8 Active",

  .buttonText[CAN_DISPLAY_MODE_INDEX][0]  = "< Display Mode:",
  .buttonText[CAN_DISPLAY_MODE_INDEX][1]  = "X",

  .buttonText[CAN_CLEAR_INDEX][0]         = "Clear",

  /* Second Page */
  .buttonText[CAN_TIMEBASE_INDEX][0]      = "< Timebase:",
  .buttonText[CAN_TIMEBASE_INDEX][1]      = "X",

  .buttonText[CAN_OUTPUT_RELAY_INDEX][0]  = "< Output Relay:",
  .buttonText[CAN_OUTPUT_RELAY_INDEX][1]  = "Active on capture",

  .buttonText[CAN_MODULE_PINOUT_INDEX][0]  = "< Module Pinout",

  /* Fourth page */
  .buttonText[CAN_ID_INDEX][0]            = "ID:",
  .buttonText[CAN_ID_INDEX][1]            = "0.361 V (No 3)",  /* TODO: */
  .buttonIsStaticText[CAN_ID_INDEX]       = true,

  .buttonText[CAN_REFRESH_ID_INDEX][0]    = "Refresh ID",

  .buttonText[CAN_MODULE_POWER_INDEX][0]  = "Module Power:",
  .buttonText[CAN_MODULE_POWER_INDEX][1]  = "X",

  .buttonText[CAN_AVAILABLE_STORAGE_INDEX][0] = "Available Storage:",
  .buttonText[CAN_AVAILABLE_STORAGE_INDEX][1] = "X",
  .buttonIsStaticText[CAN_AVAILABLE_STORAGE_INDEX] = true,

  .buttonFont           = SIDERBAR_BUTTON_FONT,
  .numOfPages           = CAN_NUM_OF_PAGES,
  .numOfButtonsPerPage  = SIDERBAR_NUM_OF_BUTTONS_PER_PAGE,
};

/* RS-232 ===================================================== */
const GUIButtonList RS_232_SidebarTemplate = {
  .object.id              = GUI_INVALID_ID,
  .object.xPos            = SIDEBAR_XPOS,
  .object.yPos            = SIDEBAR_YPOS,
  .object.width           = SIDEBAR_WIDTH,
  .object.height          = SIDEBAR_HEIGHT,
  .object.displayState    = GUIDisplayState_Hidden,
  .object.border          = SIDEBAR_BORDER,
  .object.borderThickness = 2,
  .object.borderColor     = SIDEBAR_BORDER_COLOR,
  .object.layer           = GUILayer_1,

  .backgroundColor                = COLOR_ERROR,
  .titleBackgroundColor           = COLOR_ERROR,
  .titleTextColor[0]              = COLOR_ERROR,
  .titleTextColor[1]              = COLOR_ERROR,
  .buttonsState1TextColor         = COLOR_ERROR,
  .buttonsState1BackgroundColor   = COLOR_ERROR,
  .buttonsState2TextColor         = COLOR_ERROR,
  .buttonsState2BackgroundColor   = COLOR_ERROR,
  .buttonsPressedTextColor        = COLOR_ERROR,
  .buttonsPressedBackgroundColor  = COLOR_ERROR,

  .buttonsBorder  = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom,
  .previousBorder = GUIBorder_Left | GUIBorder_Right | GUIBorder_Top | GUIBorder_Bottom,
  .nextBorder     = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom,

  .actionButtonPressed = 0,

  .title[0][0]  = "RS-232 (1/4)",
  .title[1][0]  = "RS-232 (2/4)",
  .title[2][0]  = "RS-232 (3/4)",
  .title[3][0]  = "RS-232 (4/4)",
  .titleEnabled = true,
  .titleFont    = SIDERBAR_TITLE_FONT,

  /* First page */
  .buttonText[RS_232_CAPTURE_INDEX][0] = "Start Capture",

  .buttonText[RS_232_BAUD_RATE_INDEX][0] = "< Baud Rate:",
  .buttonText[RS_232_BAUD_RATE_INDEX][1] = "X",

  .buttonText[RS_232_PARITY_INDEX][0] = "< Parity:",
  .buttonText[RS_232_PARITY_INDEX][1] = "X",

  .buttonText[RS_232_DATA_BITS_INDEX][0] = "< Data Bits:",
  .buttonText[RS_232_DATA_BITS_INDEX][1] = "X",

  .buttonText[RS_232_DISPLAY_FORMAT_INDEX][0] = "< Display Format:",
  .buttonText[RS_232_DISPLAY_FORMAT_INDEX][1] = "X",

  .buttonText[RS_232_CLEAR_INDEX][0] = "Clear",

  /* Second Page */
  .buttonText[RS_232_TIMEBASE_INDEX][0] = "< Timebase:",
  .buttonText[RS_232_TIMEBASE_INDEX][1] = "X",

  .buttonText[RS_232_SPLITSCREEN_INDEX][0] = "Splitscreen:",
  .buttonText[RS_232_SPLITSCREEN_INDEX][1] = "X",

  .buttonText[RS_232_OUTPUT_RELAY_INDEX][0] = "< Output Relay:",
  .buttonText[RS_232_OUTPUT_RELAY_INDEX][1] = "Active on capture",

  .buttonText[RS_232_CHANNEL_MODE_INDEX][0] = "< Channel Mode",
  .buttonText[RS_232_CHANNEL_MODE_INDEX][1] = "X",

  .buttonText[RS_232_MODULE_PINOUT_INDEX][0] = "< Module Pinout",

  /* Fourth page */
  .buttonText[RS_232_ID_INDEX][0] = "ID:",
  .buttonText[RS_232_ID_INDEX][1] = "0.567 V (No 5)",  /* TODO: */
  .buttonIsStaticText[RS_232_ID_INDEX] = true,

  .buttonText[RS_232_REFRESH_ID_INDEX][0] = "Refresh ID",

  .buttonText[RS_232_MODULE_POWER_INDEX][0] = "Module Power:",
  .buttonText[RS_232_MODULE_POWER_INDEX][1] = "X",

  .buttonText[RS_232_AVAILABLE_STORAGE_INDEX][0] = "Available Storage:",
  .buttonText[RS_232_AVAILABLE_STORAGE_INDEX][1] = "X",
  .buttonIsStaticText[RS_232_AVAILABLE_STORAGE_INDEX] = true,

  .buttonFont           = SIDERBAR_BUTTON_FONT,
  .numOfPages           = RS_232_NUM_OF_PAGES,
  .numOfButtonsPerPage  = SIDERBAR_NUM_OF_BUTTONS_PER_PAGE,
};

/* System ===================================================== */
const GUIButtonList SYSTEM_SidebarTemplate = {
  .object.id              = GUI_INVALID_ID,
  .object.xPos            = SIDEBAR_XPOS,
  .object.yPos            = SIDEBAR_YPOS,
  .object.width           = SIDEBAR_WIDTH,
  .object.height          = SIDEBAR_HEIGHT,
  .object.displayState    = GUIDisplayState_Hidden,
  .object.border          = SIDEBAR_BORDER,
  .object.borderThickness = 2,
  .object.borderColor     = SIDEBAR_BORDER_COLOR,
  .object.layer           = GUILayer_1,

  .backgroundColor                = COLOR_ERROR,
  .titleBackgroundColor           = COLOR_ERROR,
  .titleTextColor[0]              = COLOR_ERROR,
  .titleTextColor[1]              = COLOR_ERROR,
  .buttonsState1TextColor         = COLOR_ERROR,
  .buttonsState1BackgroundColor   = COLOR_ERROR,
  .buttonsState2TextColor         = COLOR_ERROR,
  .buttonsState2BackgroundColor   = COLOR_ERROR,
  .buttonsPressedTextColor        = COLOR_ERROR,
  .buttonsPressedBackgroundColor  = COLOR_ERROR,

  .buttonsBorder  = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom,
  .previousBorder = GUIBorder_Left | GUIBorder_Right | GUIBorder_Top | GUIBorder_Bottom,
  .nextBorder     = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom,

  .actionButtonPressed = 0,

  .title[0][0]  = "System (1/3)",
  .title[1][0]  = "System (2/3)",
  .title[2][0]  = "System (3/3)",
  .titleEnabled = true,
  .titleFont    = SIDERBAR_TITLE_FONT,

  /* First page */
  .buttonText[SYSTEM_TIME_INDEX][0] = "< Time",

  .buttonText[SYSTEM_VERSION_INFO_INDEX][0] = "< Version Info",

  .buttonText[SYSTEM_REFRESH_MODULE_IDS_INDEX][0] = "Refresh Module",
  .buttonText[SYSTEM_REFRESH_MODULE_IDS_INDEX][1] = "IDs",

  .buttonText[SYSTEM_CLEAR_DATA_MEMORY_INDEX][0] = "Clear All Data",
  .buttonText[SYSTEM_CLEAR_DATA_MEMORY_INDEX][1] = "Memory",

  .buttonText[SYSTEM_TOUCH_BUZZER_INDEX][0] = "Touch Buzzer:",
  .buttonText[SYSTEM_TOUCH_BUZZER_INDEX][1] = "X",

  .buttonText[SYSTEM_LCD_BRIGHTNESS_INDEX][0] = "< LCD Brightness:",
  .buttonText[SYSTEM_LCD_BRIGHTNESS_INDEX][1] = "X",

  /* Second Page */
  .buttonText[SYSTEM_ENABLE_PROMPT_INDEX][0] = "Enable Prompt:",
  .buttonText[SYSTEM_ENABLE_PROMPT_INDEX][1] = "X",

  .buttonText[SYSTEM_LCD_OFF_TIMER_INDEX][0] = "< LCD Off Timer:",
  .buttonText[SYSTEM_LCD_OFF_TIMER_INDEX][1] = "X",

  /* Third page */
  .buttonText[SYSTEM_POWER_SOURCE_INDEX][0] = "Power Source:",
  .buttonText[SYSTEM_POWER_SOURCE_INDEX][1] = "X",
  .buttonIsStaticText[SYSTEM_POWER_SOURCE_INDEX] = true,

  .buttonFont           = SIDERBAR_BUTTON_FONT,
  .numOfPages           = SYSTEM_NUM_OF_PAGES,
  .numOfButtonsPerPage  = SIDERBAR_NUM_OF_BUTTONS_PER_PAGE,
};

/** Alert Boxes --------------------------------------------------------------*/
const GUIAlertBox EnableAlertBoxTemplate = {
  .object.id              = GUI_INVALID_ID,
  .object.xPos            = ENABLE_ALERT_BOX_XPOS,
  .object.yPos            = ENABLE_ALERT_BOX_YPOS,
  .object.width           = ENABLE_ALERT_BOX_WIDTH,
  .object.height          = ENABLE_ALERT_BOX_HEIGHT,
  .object.border          = GUIBorder_All,
  .object.borderThickness = 2,
  .object.borderColor     = COLOR_WHITE,
  .object.layer           = GUILayer_2,

  .backgroundColor      = COLOR_ERROR,
  .titleBackgroundColor = COLOR_ERROR,
  .titleTextColor       = COLOR_ERROR,
  .infoTextColor        = COLOR_ERROR,
  .buttonTextColor      = COLOR_WHITE,
  .leftButtonColor      = COLOR_APP_GREEN,
  .rightButtonColor     = COLOR_APP_RED,

  .actionButtonPressed = 0,

  .title                  = "Enable Channel",
  .infoText               = "Are you sure you want to enable the channel?",
  .leftButtonText[0]      = "Yes",
  .rightButtonText[0]     = "No",
  .font                   = ENABLE_ALERT_BOX_FONT,
  .titleHeight            = ENABLE_ALERT_BOX_TITLE_HEIGHT,
  .leftRightButtonHeight  = ENABLE_ALERT_BOX_BUTTON_HEIGHT,
  .padding.leftRight      = ENABLE_ALERT_BOX_PADDING_LEFT_RIGHT,
  .padding.topBottom      = ENABLE_ALERT_BOX_PADDING_TOP_BOTTOM,
};

const GUIAlertBox RefreshIdsAlertBoxTemplate = {
  .object.id              = GUIAlertBoxId_RefreshIds,
  .object.xPos            = REFRESH_IDS_ALERT_BOX_XPOS,
  .object.yPos            = REFRESH_IDS_ALERT_BOX_YPOS,
  .object.width           = REFRESH_IDS_ALERT_BOX_WIDTH,
  .object.height          = REFRESH_IDS_ALERT_BOX_HEIGHT,
  .object.border          = GUIBorder_All,
  .object.borderThickness = 2,
  .object.borderColor     = COLOR_WHITE,
  .object.layer           = GUILayer_2,

  .backgroundColor      = COLOR_APP_SYSTEM,
  .titleBackgroundColor = COLOR_WHITE,
  .titleTextColor       = COLOR_APP_SYSTEM,
  .infoTextColor        = COLOR_WHITE,
  .buttonTextColor      = COLOR_WHITE,
  .leftButtonColor      = COLOR_APP_GREEN,
  .rightButtonColor     = COLOR_APP_RED,

  .actionButtonPressed = 0,

  .title                  = "Refresh IDs",
  .infoText               = "Refreshing IDs will clear all channel settings. Proceed?",
  .leftButtonText[0]      = "Yes",
  .rightButtonText[0]     = "No",
  .font                   = REFRESH_IDS_ALERT_BOX_FONT,
  .titleHeight            = REFRESH_IDS_ALERT_BOX_TITLE_HEIGHT,
  .leftRightButtonHeight  = REFRESH_IDS_ALERT_BOX_BUTTON_HEIGHT,
  .padding.leftRight      = REFRESH_IDS_ALERT_BOX_PADDING_LEFT_RIGHT,
  .padding.topBottom      = REFRESH_IDS_ALERT_BOX_PADDING_TOP_BOTTOM,
};

const GUIAlertBox ClearAllMemoryAlertBoxTemplate = {
  .object.id              = GUIAlertBoxId_ClearAllMemory,
  .object.xPos            = CLEAR_ALL_MEMORY_ALERT_BOX_XPOS,
  .object.yPos            = CLEAR_ALL_MEMORY_ALERT_BOX_YPOS,
  .object.width           = CLEAR_ALL_MEMORY_ALERT_BOX_WIDTH,
  .object.height          = CLEAR_ALL_MEMORY_ALERT_BOX_HEIGHT,
  .object.border          = GUIBorder_All,
  .object.borderThickness = 2,
  .object.borderColor     = COLOR_WHITE,
  .object.layer           = GUILayer_2,

  .backgroundColor      = COLOR_APP_SYSTEM,
  .titleBackgroundColor = COLOR_WHITE,
  .titleTextColor       = COLOR_APP_SYSTEM,
  .infoTextColor        = COLOR_WHITE,
  .buttonTextColor      = COLOR_WHITE,
  .leftButtonColor      = COLOR_APP_GREEN,
  .rightButtonColor     = COLOR_APP_RED,

  .actionButtonPressed = 0,

  .title                  = "Clear Memory",
  .infoText               = "Are you sure you want to clear all data memory?",
  .leftButtonText[0]      = "Yes",
  .rightButtonText[0]     = "No",
  .font                   = CLEAR_ALL_MEMORY_ALERT_BOX_FONT,
  .titleHeight            = CLEAR_ALL_MEMORY_ALERT_BOX_TITLE_HEIGHT,
  .leftRightButtonHeight  = CLEAR_ALL_MEMORY_ALERT_BOX_BUTTON_HEIGHT,
  .padding.leftRight      = CLEAR_ALL_MEMORY_ALERT_BOX_PADDING_LEFT_RIGHT,
  .padding.topBottom      = CLEAR_ALL_MEMORY_ALERT_BOX_PADDING_TOP_BOTTOM,
};

/** Button Grid Boxes --------------------------------------------------------*/
const GUIButtonGridBox ParityButtonGridBoxTemplate = {
  .object.id              = GUI_INVALID_ID,
  .object.xPos            = PARITY_BUTTON_GRID_BOX_XPOS,
  .object.yPos            = PARITY_BUTTON_GRID_BOX_YPOS,
  .object.width           = PARITY_BUTTON_GRID_BOX_WIDTH,
  .object.height          = PARITY_BUTTON_GRID_BOX_HEIGHT,
  .object.border          = GUIBorder_All,
  .object.borderThickness = 2,
  .object.borderColor     = PARITY_BUTTON_GRID_BOX_BORDER_COLOR,
  .object.layer           = GUILayer_2,

  .backgroundColor                = COLOR_ERROR,
  .titleBackgroundColor           = COLOR_ERROR,
  .titleTextColor                 = COLOR_ERROR,
  .labelsBackgroundColor          = COLOR_ERROR,
  .labelsTextColor                = COLOR_ERROR,
  .buttonsState1TextColor         = COLOR_ERROR,
  .buttonsState1BackgroundColor   = COLOR_ERROR,
  .buttonsState2TextColor         = COLOR_ERROR,
  .buttonsState2BackgroundColor   = COLOR_ERROR,
  .buttonsPressedTextColor        = COLOR_ERROR,
  .buttonsPressedBackgroundColor  = COLOR_ERROR,

  .actionButtonPressed = 0,

  .title              = "Parity",
  .buttonText[0][0]   = "Odd",
  .buttonText[0][1]   = "Even",
  .buttonText[1][0]   = "None",
  .font               = PARITY_BUTTON_GRID_BOX_FONT,

  .titleHeight        = PARITY_BUTTON_GRID_BOX_TITLE_HEIGHT,
  .labelColumnEnabled = false,
  .numOfRows          = 2,
  .numOfColumns       = 2,
  .padding.leftRight  = PARITY_BUTTON_GRID_BOX_PADDING_LEFT_RIGHT,
  .padding.topBottom  = PARITY_BUTTON_GRID_BOX_PADDING_TOP_BOTTOM,
};

const GUIButtonGridBox DirectionButtonGridBoxTemplate = {
  .object.id              = GUI_INVALID_ID,
  .object.xPos            = DIRECTION_BUTTON_GRID_BOX_XPOS,
  .object.yPos            = DIRECTION_BUTTON_GRID_BOX_YPOS,
  .object.width           = DIRECTION_BUTTON_GRID_BOX_WIDTH,
  .object.height          = DIRECTION_BUTTON_GRID_BOX_HEIGHT,
  .object.border          = GUIBorder_All,
  .object.borderThickness = 2,
  .object.borderColor     = DIRECTION_BUTTON_GRID_BOX_BORDER_COLOR,
  .object.layer           = GUILayer_2,

  .backgroundColor                = COLOR_ERROR,
  .titleBackgroundColor           = COLOR_ERROR,
  .titleTextColor                 = COLOR_ERROR,
  .labelsBackgroundColor          = COLOR_ERROR,
  .labelsTextColor                = COLOR_ERROR,
  .buttonsState1TextColor         = COLOR_ERROR,
  .buttonsState1BackgroundColor   = COLOR_ERROR,
  .buttonsState2TextColor         = COLOR_ERROR,
  .buttonsState2BackgroundColor   = COLOR_ERROR,
  .buttonsPressedTextColor        = COLOR_ERROR,
  .buttonsPressedBackgroundColor  = COLOR_ERROR,

  .actionButtonPressed = 0,

  .title              = "Direction",
  .labelText[0]       = "Ch A:",
  .labelText[1]       = "Ch B:",
  .buttonText[0][0]   = "TX",
  .buttonText[0][1]   = "RX",
  .buttonText[0][2]   = "Off",
  .buttonText[1][0]   = "TX",
  .buttonText[1][1]   = "RX",
  .buttonText[1][2]   = "Off",
  .font               = DIRECTION_BUTTON_GRID_BOX_FONT,

  .titleHeight        = DIRECTION_BUTTON_GRID_BOX_TITLE_HEIGHT,
  .labelColumnEnabled = true,
  .numOfRows          = 2,
  .numOfColumns       = 3,
  .padding.leftRight  = DIRECTION_BUTTON_GRID_BOX_PADDING_LEFT_RIGHT,
  .padding.topBottom  = DIRECTION_BUTTON_GRID_BOX_PADDING_TOP_BOTTOM,
};

/** Info Boxes ---------------------------------------------------------------*/
const GUIInfoBox VersionInfoBoxTemplate = {
  .object.id              = GUIInfoBoxId_VersionInfo,
  .object.xPos            = VERSION_INFO_BOX_XPOS,
  .object.yPos            = VERSION_INFO_BOX_YPOS,
  .object.width           = VERSION_INFO_BOX_WIDTH,
  .object.height          = VERSION_INFO_BOX_HEIGHT,
  .object.border          = GUIBorder_All,
  .object.borderThickness = 2,
  .object.borderColor     = VERSION_INFO_BOX_BORDER_COLOR,
  .object.layer           = GUILayer_2,

  .backgroundColor      = COLOR_APP_SYSTEM,
  .titleBackgroundColor = COLOR_WHITE,
  .titleTextColor       = COLOR_APP_SYSTEM,
  .infoTextColor        = COLOR_WHITE,

  .title    = "Version Info",
  .infoText = "Hardware:\n"
              "DataProcessor: 1.0\n"
              "UI Processor: 1.0\n"
              "Software:\n"
              "Data Processor: 0.1\n"
              "UI Processor: 0.1",      /* TODO */
  .font               = VERSION_INFO_BOX_FONT,
  .textAlignment      = GUITextAlignment_Left,
  .titleHeight        = VERSION_INFO_BOX_TITLE_HEIGHT,
  .padding.leftRight  = VERSION_INFO_BOX_PADDING_LEFT_RIGHT,
  .padding.topBottom  = VERSION_INFO_BOX_PADDING_TOP_BOTTOM,
};
