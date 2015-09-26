/**
 *******************************************************************************
 * @file    simple_gui_config.h
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
#ifndef SIMPLE_GUI_CONFIG_H_
#define SIMPLE_GUI_CONFIG_H_

/** Includes -----------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "stm32f4xx.h"

/* Include the LCD driver here: */
#include "lcd.h"

/* Include the fonts here */
#include "fonts.h"

/** Defines and typedefs -----------------------------------------------------*/

/* Activate GUI items by un-commenting the specific line below */
#define GUI_SCROLLABLE_TEXT_BOX_ENABLED
#define GUI_ALERT_BOX_ENABLED
#define GUI_BUTTON_GRID_BOX_ENABLED
#define GUI_BUTTON_LIST_ENABLED
#define GUI_INFO_BOX_ENABLED

/* Set how many characters we can have at max for one row */
#define GUI_MAX_CHARACTERS_PER_ROW  (200)

/* */
#define GUI_MAX_CHARACTERS_IN_SCROLLABLE_TEXT_BOX  (1000)

/* LCD layer mapping to GUI layers */
#define LCD_LAYER_INVALID   (0xFF)
#define GUI_NUM_OF_LAYERS  (3)
#define GUI_MAX_OBJECTS_PER_LAYER  (256)
/** ========================================================================= */

/** Colors -------------------------------------------------------------------*/
/* Define the color mode to use here by uncommenting the appropriate line */
#define GUI_COLOR_MODE    ARGB8888
//#define GUI_COLOR_MODE    RGB565

#if (GUI_COLOR_MODE == ARGB8888)
typedef uint32_t guiColor;
#define COLOR_BLUE            0xFF0000FF
#define COLOR_BLUE_TRANSP     0x7F0000FF
#define COLOR_RED             0xFFFF0000
#define COLOR_RED_TRANSPARENT 0x7FFF0000
#define COLOR_GREEN           0xFF00FF00
#define COLOR_YELLOW          0xFFFFFF00
#define COLOR_CYAN            0xFF00FFFF
#define COLOR_MAGENTA         0xFFFF00FF
#define COLOR_TURQUOISE       0xFF539090
#define COLOR_GRAY            0xFF7F7F7F
#define COLOR_WHITE           0xFFFFFFFF
#define COLOR_BLACK           0xFF000000
#define COLOR_TRANSPARENT     0x00000000
#define COLOR_ERROR           COLOR_MAGENTA
#define COLOR_BACKGROUND      COLOR_BLACK

#define COLOR_APP_CH1         0xFF1D5AAC  /* Blue */
#define COLOR_APP_CH1_DARK    0xFF0E2D55  /* Dark Blue */
#define COLOR_APP_CH2         0xFFA10E1E  /* Red */
#define COLOR_APP_CH2_DARK    0xFF5C1718  /* Dark Red */
#define COLOR_APP_CH3         0xFF218041  /* Green */
#define COLOR_APP_CH3_DARK    0xFF155129  /* Dark Green */
#define COLOR_APP_CH4         0xFF009898  /* Turquoise */
#define COLOR_APP_CH4_DARK    0xFF0B5655  /* Dark Turquoise */
#define COLOR_APP_CH5         0xFFC06102  /* Orange */
#define COLOR_APP_CH5_DARK    0xFF7F4001  /* Dark Orange */
#define COLOR_APP_CH6         0xFF829D05  /* Lime */
#define COLOR_APP_CH6_DARK    0xFF435505  /* Dark Lime */

#define COLOR_APP_SYSTEM      0xFF575757  /* Gray */
#define COLOR_APP_SYSTEM_DARK 0xFF2D2D2D  /* Dark Gray */

#define COLOR_APP_RED         0xFF992727
#define COLOR_APP_GREEN       0xFF218041

#elif (GUI_COLOR_MODE == RGB565)
typedef uint16_t guiColor;
#define GUI_BLUE      0x237F
#define GUI_DARK_BLUE    0x0258
#define GUI_RED        0xF926
#define GUI_DARK_RED    0xA821
#define GUI_GREEN      0x362A
#define GUI_DARK_GREEN    0x1BC6
#define GUI_YELLOW      0xFEE6
#define GUI_DARK_YELLOW    0xC560
#define GUI_PURPLE      0xA8D6
#define GUI_DARK_PURPLE    0x788F
#define GUI_GRAY      0xB596
#define GUI_MAGENTA      0xF81F
#define GUI_CYAN_LIGHT    0x1F3C
#define GUI_CYAN      0x1699
#define GUI_CYAN_DARK    0x45F7
#define GUI_CYAN_VERY_DARK  0x34B2
#define GUI_SYSTEM_BLUE    0x11CE
#define GUI_SYSTEM_BLUE_DARK 0x096B
#define GUI_WHITE      LCD_COLOR_WHITE
#define GUI_BLACK      LCD_COLOR_BLACK

#endif

/*
 * Object IDs:
 *     0-99:    Reserved
 *     100-199:  Buttons
 *     200-299:  Labels
 *     300-399:  Static Text Boxes
 *     400-499:  Scrollable Text Boxes
 *     500-599:  Alert boxes
 *     600-699:  Button grid boxes
 *     700-799:  Button lists
 *     800-899:  Info boxes
 *     900-999:  Containers
 */
#define guiConfigBUTTON_ID_OFFSET               100
#define guiConfigLABEL_ID_OFFSET                200
#define guiConfigSTATIC_TEXT_BOX_ID_OFFSET      300
#define guiConfigSCROLLABLE_TEXT_BOX_ID_OFFSET  400
#define guiConfigALERT_BOX_ID_OFFSET            500
#define guiConfigBUTTON_GRID_BOX_ID_OFFSET      600
#define guiConfigBUTTON_LIST_ID_OFFSET          700
#define guiConfigINFO_BOX_ID_OFFSET             800
#define guiConfigCONTAINER_ID_OFFSET            900

/** ========================================================================= */

/* Buttons */
typedef enum
{
  /* Test */
  GUIButtonId_Channel1Top = guiConfigBUTTON_ID_OFFSET,
  GUIButtonId_Channel2Top,
  GUIButtonId_Channel3Top,
  GUIButtonId_Channel4Top,
  GUIButtonId_Channel5Top,
  GUIButtonId_Channel6Top,
  GUIButtonId_System,
  GUIButtonId_RightTest,
  GUIButtonId_ContainerButton,

  /* The last item will represent how many items there are in total */
  GUIButtonId_NumberOfButtons,
} GUIButtonId;
#define guiConfigNUMBER_OF_BUTTONS (GUIButtonId_NumberOfButtons - guiConfigBUTTON_ID_OFFSET)

/** ========================================================================= */

/* Labels */
typedef enum
{
  /* Test */
  GUILabelId_Channel1Top = guiConfigLABEL_ID_OFFSET,
  GUILabelId_Channel2Top,
  GUILabelId_Channel3Top,
  GUILabelId_Channel4Top,
  GUILabelId_Channel5Top,
  GUILabelId_Channel6Top,
  GUILabelId_Clock,

  /* The last item will represent how many items there are in total */
  GUIButtonId_NumberOfLabels,
} GUILabelId;
#define guiConfigNUMBER_OF_LABELS (GUIButtonId_NumberOfLabels - guiConfigLABEL_ID_OFFSET)

/** ========================================================================= */

/* Static Text Boxes */
typedef enum
{
  GUIStaticTextBoxId_TestAlertBox = guiConfigSTATIC_TEXT_BOX_ID_OFFSET,

  /* The last item will represent how many items there are in total */
  GUIStaticTextBoxId_NumberOfStaticTextBoxes,
} GUIStaticTextBoxId;
#define guiConfigNUMBER_OF_STATIC_TEXT_BOXES (GUIStaticTextBoxId_NumberOfStaticTextBoxes - guiConfigSTATIC_TEXT_BOX_ID_OFFSET)

/** ========================================================================= */

#ifdef GUI_SCROLLABLE_TEXT_BOX_ENABLED
/* Scrollable Text Boxes */
typedef enum
{
  GUIScrollableTextBoxId_Test = guiConfigSCROLLABLE_TEXT_BOX_ID_OFFSET,

  /* The last item will represent how many items there are in total */
  GUIScrollableTextBoxId_NumberOfScrollableTextBoxes,
} GUIScrollableTextBoxId;
#define guiConfigNUMBER_OF_SCROLLABLE_TEXT_BOXES (GUIScrollableTextBoxId_NumberOfScrollableTextBoxes - guiConfigSCROLLABLE_TEXT_BOX_ID_OFFSET)
#endif

/** ========================================================================= */

#ifdef GUI_ALERT_BOX_ENABLED
/* Alert Boxes */
typedef enum
{
  GUIAlertBoxId_RefreshIds = guiConfigALERT_BOX_ID_OFFSET,
  GUIAlertBoxId_ClearAllMemory,
  GUIAlertBoxId_ConfirmId,

  /* The last item will represent how many items there are in total */
  GUIAlertBoxId_NumberOfAlertBoxes,
} GUIAlertBoxId;
#define guiConfigNUMBER_OF_ALERT_BOXES (GUIAlertBoxId_NumberOfAlertBoxes - guiConfigALERT_BOX_ID_OFFSET)
#endif

/** ========================================================================= */

#ifdef GUI_BUTTON_GRID_BOX_ENABLED
/* Button grid boxes */
/*
 * This is the maximum number of rows and columns a GUIButtonGridBox can have.
 * Please change these to match your need as it takes up a lot of memory if
 * the values are too high.
 */
#define GUI_BUTTON_GRID_MAX_ROWS  (3)
#define GUI_BUTTON_GRID_MAX_COLUMNS  (4)
typedef enum
{
  GUIButtonGridBoxId_ParitySelection = guiConfigBUTTON_GRID_BOX_ID_OFFSET,
  GUIButtonGridBoxId_DirectionSelection,
  GUIButtonGridBoxId_ModuleModeSelection,

  /* The last item will represent how many items there are in total */
  GUIButtonGridBoxId_NumberOfButtonGridBoxes,
} GUIButtonGridBoxId;
#define guiConfigNUMBER_OF_BUTTON_GRID_BOXES (GUIButtonGridBoxId_NumberOfButtonGridBoxes - guiConfigBUTTON_GRID_BOX_ID_OFFSET)
#endif

/** ========================================================================= */

#ifdef GUI_BUTTON_LIST_ENABLED
/* Button list */
/* This is the maximum number of pages in a button list */
#define GUI_BUTTON_LIST_MAX_PAGES  (4)
/* This is the maximum number of buttons in a button list */
#define GUI_BUTTON_LIST_MAX_BUTTONS_PER_PAGE  (6)
#define GUI_BUTTON_LIST_MAX_BUTTONS  (GUI_BUTTON_LIST_MAX_PAGES * GUI_BUTTON_LIST_MAX_BUTTONS_PER_PAGE)
typedef enum
{
  GUIButtonListId_Sidebar = guiConfigBUTTON_LIST_ID_OFFSET,

  /* The last item will represent how many items there are in total */
  GUIButtonListId_NumberOfButtonLists,
} GUIButtonListId;
#define guiConfigNUMBER_OF_BUTTON_LISTS (GUIButtonListId_NumberOfButtonLists - guiConfigBUTTON_LIST_ID_OFFSET)
#endif

/** ========================================================================= */

#ifdef GUI_INFO_BOX_ENABLED
/* Info Boxes */
typedef enum
{
  GUIInfoBoxId_VersionInfo = guiConfigINFO_BOX_ID_OFFSET,

  /* The last item will represent how many items there are in total */
  GUIInfoBoxId_NumberOfInfoBoxes,
} GUIInfoBoxId;
#define guiConfigNUMBER_OF_INFO_BOXES (GUIInfoBoxId_NumberOfInfoBoxes - guiConfigINFO_BOX_ID_OFFSET)
#endif

/** ========================================================================= */

/* Containers */
typedef enum
{
  GUIContainerId_TestContainer = guiConfigCONTAINER_ID_OFFSET,

  /* The last item will represent how many items there are in total */
  GUIContainerId_NumberOfContainers,
} GUIContainerId;
#define guiConfigNUMBER_OF_CONTAINERS (GUIContainerId_NumberOfContainers - guiConfigCONTAINER_ID_OFFSET)

/* Function prototypes -------------------------------------------------------*/

#endif /* SIMPLE_GUI_CONFIG_H_ */
