/**
 *******************************************************************************
 * @file    main_task.c
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
#include "main_task.h"
#include <string.h>
#include <stdbool.h>

#include "spi_comm.h"

/** Private defines ----------------------------------------------------------*/
/** Private typedefs ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
static APP_ActiveSidebar prvCurrentlyActiveSidebar = APP_ActiveSidebar_None;
static prvChannelNumberFromActiveSidebar[8] = {1, 2, 3, 4, 5, 6, 0, 0};

static APP_ChannelType prvChannelType[6] = {
    APP_ChannelType_UART, APP_ChannelType_RS_232, APP_ChannelType_GPIO,
    APP_ChannelType_SETUP, APP_ChannelType_CAN, APP_ChannelType_NA
};

/* Buzzer */
static APP_BuzzerSound prvCurrentBuzzerSound = APP_BuzzerSound_Off;
/* Power Source */
static APP_PowerSource prvCurrentPowerSource = APP_PowerSource_ExternalModule; /* TODO: one for each channel */
/* Direction data */
static APP_ChannelDirection prvCurrentChannelDirection[2] = {APP_ChannelDirection_Off, APP_ChannelDirection_Off}; /* TODO: one for each channel */
static char prvCurrentDirectionString[16] = {0};  /* Longest string is "A: Off - B: Off" -> 15 characters */
/* Parity */
static APP_Parity prvCurrentParity = APP_Parity_None;   /* TODO: one for each channel */
static char* prvCurrentParityString; /* TODO: one for each channel */

static bool prvChannelIsEnabled[6]        = {false, false, false, false, false, false};
static bool prvChannelSplitscreenEnabled[6]   = {false, false, false, false, false, false};
static bool prvModulePowerEnabled[6]      = {true, true, true, true, true, true};

static uint16_t prvSidebarActivePage[8]     = {0, 0, 0, 0, 0, 0, 0, 0};
static bool prvEnablePromptEnabled        = true;

static uint16_t prvIdAsMilliVolt[6]       = {155, 567, 155, 155, 361, 0};
static char* prvIdAsString[6][15];

static GUIButton prvButton;
static GUILabel prvLabel;
static GUIScrollableTextBox prvScrollableTextBox;
static GUIButtonList prvTempButtonList;
static GUIAlertBox prvTempAlertBox;
static GUIButtonGridBox prvTempButtonGridBox;
static GUIInfoBox prvTempInfoBox;

//static uint32_t prvCurrentLogAddress = 0;

static bool prvLcdTaskIsDone = false;

/** Private function prototypes ----------------------------------------------*/
static void prvHardwareInit();

static void refreshIds();

static void setActiveColorsForAlertBox(uint32_t Id);
static void setActiveColorsForButtonGridBox(uint32_t Id);

static void updateParityString();
static void updateDirectionString();

static void setActiveSidebar(APP_ActiveSidebar NewActiveChannel);
static void setNASidebarItems();
static void setSETUPSidebarItems();
static void setUARTSidebarItems();
static void setGPIOSidebarItems();
static void setCANSidebarItems();
static void setRS232SidebarItems();
static void setSystemSidebarItems();

static void buttonInSidebarPressed(uint32_t ButtonListId, uint32_t ButtonIndex);
static void buttonPressedInSystemSidebar(uint32_t ButtonIndex);
static void buttonPressedInNASidebar(uint32_t ButtonIndex);
static void buttonPressedInSetupSidebar(uint32_t ButtonIndex);
static void buttonPressedInUARTSidebar(uint32_t ButtonIndex);
static void buttonPressedInGPIOSidebar(uint32_t ButtonIndex);
static void buttonPressedInCANSidebar(uint32_t ButtonIndex);
static void buttonPressedInRS232Sidebar(uint32_t ButtonIndex);

/* Alert box callbacks */
static void refreshIdsAlertBoxCallback(GUIAlertBoxCallbackButton CallbackButton);
static void clearAllMemoryAlertBoxCallback(GUIAlertBoxCallbackButton CallbackButton);

/* Button grid callbacks */
static void buttonInParityBoxPressed(uint32_t Row, uint32_t Column);
static void buttonInDirectionBoxPressed(uint32_t Row, uint32_t Column);

/* GUI init functions */
static void prvInitTopAndSystemItems();
static void prvTopAndSystemButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvInitSidebarItems();

/** Functions ----------------------------------------------------------------*/
/**
  * @brief  Text
  * @param  None
  * @retval None
  */
void mainTask(void *pvParameters)
{
  /* Initialize the hardware */
  prvHardwareInit();

  /* Init the SPI Communication with the FPGA Data Processor */
  SPI_COMM_Init();

  /* The parameter in vTaskDelayUntil is the absolute time
   * in ticks at which you want to be woken calculated as
   * an increment from the time you were last woken. */
  TickType_t xNextWakeTime;
  /* Initialize xNextWakeTime - this only needs to be done once. */
  xNextWakeTime = xTaskGetTickCount();

  /* Wait until LCD task is done with init */
  while (!prvLcdTaskIsDone)
  {
    vTaskDelayUntil(&xNextWakeTime, 100 / portTICK_PERIOD_MS);
  }

  /* # Init GUI */
  GUI_Init();

  /* # Fill the first layer black */
  GUI_ClearLayer(COLOR_BLACK, GUILayer_1);

  /* Refresh IDs */
//  refreshIds();


//  writeToLog("Performing memory test...");
//  writeToLog("PASSED\n");
//
//  writeToLog("Loading splash screen...");
//  writeToLog("DONE\n");
//
//  writeToLog("Reading settings from EEPROM...");
//  writeToLog("OK\n");
//
//  writeToLog("Testing communication with data processor...");
//  writeToLog("OK\n");

//  writeToLog("Reading module IDs...");
//  writeToLog("OK\n");
//
//  writeToLog("Test test test test test test test test test test test test test test test");
//  writeToLog("Test test test test test test test test test test test test test test test");
//  writeToLog("Test test test test test test test test test test test test test test test");
//  writeToLog("OK\n");
//
//  writeToLog("Ready!");



//  /* Add scrollable text box */
//  prvScrollableTextBox.object.id        = GUIScrollableTextBoxId_Test;
//  prvScrollableTextBox.object.xPos      = 10;
//  prvScrollableTextBox.object.yPos      = 10;
//  prvScrollableTextBox.object.width       = 200;
//  prvScrollableTextBox.object.height      = 100;
//  prvScrollableTextBox.object.border      = GUIBorder_All;
//  prvScrollableTextBox.object.borderThickness = 2;
//  prvScrollableTextBox.object.borderColor   = COLOR_WHITE;
//  prvScrollableTextBox.object.layer     = GUILayer_2;
//  prvScrollableTextBox.backgroundColor    = 0xBE000000;
//  prvScrollableTextBox.textColor        = COLOR_WHITE;
//
//  prvScrollableTextBox.dataBufferStart    = (char*)LOG_DATA_ADDRESS;
//  prvScrollableTextBox.numOfCharsAvailable  = charactersWrittenInLog;
//  prvScrollableTextBox.frameBufferStartAddress = LOG_FRAME_BUFFER_ADDRESS;
//
//  prvScrollableTextBox.fixedWidthFont     = &font8x16_fixedWidth_bold;
//  prvScrollableTextBox.padding.leftRight    = 5;
//  prvScrollableTextBox.padding.topBottom    = 5;
//  GUIScrollableTextBox_Init(&prvScrollableTextBox);
//  GUIScrollableTextBox_Draw(GUIScrollableTextBoxId_Test);

//  LCD_ILI9341_DrawARGB8888ImageOnLayer(0, 0, &splash_screen, LCD_ILI9341_LAYER_1);
//  GUI_DrawAllLayersAndRefreshDisplay();


//  GUIScrollableTextBox_ScrollBuffer(GUIScrollableTextBoxId_Test, 2);
//  GUI_DrawAllLayersAndRefreshDisplay();
//  GUIScrollableTextBox_ScrollBuffer(GUIScrollableTextBoxId_Test, 2);
//  GUI_DrawAllLayersAndRefreshDisplay();
//  GUIScrollableTextBox_ScrollBuffer(GUIScrollableTextBoxId_Test, 14);
//  GUI_DrawAllLayersAndRefreshDisplay();


  /** Top Labels and Buttons */
  prvInitTopAndSystemItems();

  /** Sidebar */
  prvInitSidebarItems();

  /* Add a Parity Button Grid Box and set it for channel 1 to start with */
  GUIButtonGridBox_Reset(&prvTempButtonGridBox);
  memcpy(&prvTempButtonGridBox, &ParityButtonGridBoxTemplate, sizeof(GUIButtonGridBox));
  prvTempButtonGridBox.object.id                      = GUIButtonGridBoxId_ParitySelection;
  prvTempButtonGridBox.backgroundColor                = COLOR_APP_CH1;
  prvTempButtonGridBox.titleBackgroundColor           = COLOR_WHITE;
  prvTempButtonGridBox.titleTextColor                 = COLOR_APP_CH1;
  prvTempButtonGridBox.labelsBackgroundColor          = COLOR_APP_CH1;
  prvTempButtonGridBox.labelsTextColor                = COLOR_WHITE;
  prvTempButtonGridBox.buttonsState1TextColor         = COLOR_APP_CH1;
  prvTempButtonGridBox.buttonsState1BackgroundColor   = COLOR_WHITE;
  prvTempButtonGridBox.buttonsState2TextColor         = COLOR_WHITE;
  prvTempButtonGridBox.buttonsState2BackgroundColor   = COLOR_APP_CH1_DARK;
  prvTempButtonGridBox.buttonsPressedTextColor        = COLOR_WHITE;
  prvTempButtonGridBox.buttonsPressedBackgroundColor  = COLOR_APP_CH1;
  prvTempButtonGridBox.actionButtonPressed            = buttonInParityBoxPressed;
  GUIButtonGridBox_Init(&prvTempButtonGridBox);
  /* Set the None button in the grid to state 2 */  /* TODO */
  GUIButtonGridBox_SetButtonState(GUIButtonGridBoxId_ParitySelection, 1, 0, GUIButtonState_State2, false);

  /* Add a Direction Button Grid Box and set it for channel 1 to start with */
  GUIButtonGridBox_Reset(&prvTempButtonGridBox);
  memcpy(&prvTempButtonGridBox, &DirectionButtonGridBoxTemplate, sizeof(GUIButtonGridBox));
  prvTempButtonGridBox.object.id                      = GUIButtonGridBoxId_DirectionSelection;
  prvTempButtonGridBox.backgroundColor                = COLOR_APP_CH1;
  prvTempButtonGridBox.titleBackgroundColor           = COLOR_WHITE;
  prvTempButtonGridBox.titleTextColor                 = COLOR_APP_CH1;
  prvTempButtonGridBox.labelsBackgroundColor          = COLOR_APP_CH1;
  prvTempButtonGridBox.labelsTextColor                = COLOR_WHITE;
  prvTempButtonGridBox.buttonsState1TextColor         = COLOR_APP_CH1;
  prvTempButtonGridBox.buttonsState1BackgroundColor   = COLOR_WHITE;
  prvTempButtonGridBox.buttonsState2TextColor         = COLOR_WHITE;
  prvTempButtonGridBox.buttonsState2BackgroundColor   = COLOR_APP_CH1_DARK;
  prvTempButtonGridBox.buttonsPressedTextColor        = COLOR_WHITE;
  prvTempButtonGridBox.buttonsPressedBackgroundColor  = COLOR_APP_CH1;
  prvTempButtonGridBox.actionButtonPressed            = buttonInDirectionBoxPressed;
  GUIButtonGridBox_Init(&prvTempButtonGridBox);
  /* Set the Off buttons in the grid to state 2 */  /* TODO */
  GUIButtonGridBox_SetButtonState(GUIButtonGridBoxId_DirectionSelection, 0, 2, GUIButtonState_State2, false);
  GUIButtonGridBox_SetButtonState(GUIButtonGridBoxId_DirectionSelection, 1, 2, GUIButtonState_State2, false);

  /* System ================================================================================ */
  /* Add the Version Info GUIInfoBox */
  memcpy(&prvTempInfoBox, &VersionInfoBoxTemplate, sizeof(GUIInfoBox));
  GUIInfoBox_Init(&prvTempInfoBox);

  /* Add the Refresh IDs alert box */
  memcpy(&prvTempAlertBox, &RefreshIdsAlertBoxTemplate, sizeof(GUIAlertBox));
  prvTempAlertBox.object.id       = GUIAlertBoxId_RefreshIds;
  prvTempAlertBox.actionButtonPressed = refreshIdsAlertBoxCallback;
  GUIAlertBox_Init(&prvTempAlertBox);

  /* Add the Clear All Memory alert box */
  memcpy(&prvTempAlertBox, &ClearAllMemoryAlertBoxTemplate, sizeof(GUIAlertBox));
  prvTempAlertBox.object.id       = GUIAlertBoxId_ClearAllMemory;
  prvTempAlertBox.actionButtonPressed = clearAllMemoryAlertBoxCallback;
  GUIAlertBox_Init(&prvTempAlertBox);

  GUI_DrawAllLayersAndRefreshDisplay();

  while (1)
  {
//    GUI_TouchAtPosition(GUITouchEvent_Down, 381, 397);
//    vTaskDelayUntil(&xNextWakeTime, 1000 / portTICK_PERIOD_MS);
//    GUI_TouchAtPosition(GUITouchEvent_Up, 381, 397);
//    vTaskDelayUntil(&xNextWakeTime, 1000 / portTICK_PERIOD_MS);
//    GUI_TouchAtPosition(GUITouchEvent_Down, 381, 347);
//    vTaskDelayUntil(&xNextWakeTime, 1000 / portTICK_PERIOD_MS);
//    GUI_TouchAtPosition(GUITouchEvent_Up, 381, 347);

//    setActiveSidebar(APP_ActiveSidebar_1);
//    vTaskDelayUntil(&xNextWakeTime, 5000 / portTICK_PERIOD_MS);
//    setActiveSidebar(APP_ActiveSidebar_2);
//    vTaskDelayUntil(&xNextWakeTime, 5000 / portTICK_PERIOD_MS);
//    setActiveSidebar(APP_ActiveSidebar_3);
//    vTaskDelayUntil(&xNextWakeTime, 5000 / portTICK_PERIOD_MS);
//    setActiveSidebar(APP_ActiveSidebar_4);
//    vTaskDelayUntil(&xNextWakeTime, 5000 / portTICK_PERIOD_MS);
//    setActiveSidebar(APP_ActiveSidebar_5);
//    vTaskDelayUntil(&xNextWakeTime, 5000 / portTICK_PERIOD_MS);
//    setActiveSidebar(APP_ActiveSidebar_6);
    vTaskDelayUntil(&xNextWakeTime, 5000 / portTICK_PERIOD_MS);
  }
}

void MAIN_TASK_NotifyLcdTaskIsDone()
{
  prvLcdTaskIsDone = true;
}

/** Private functions .-------------------------------------------------------*/
/**
  * @brief  Initializes the hardware
  * @param  None
  * @retval None
  */
static void prvHardwareInit()
{

}

/**
 * @brief
 * @param
 * @retval  None
 */
static void refreshIds()
{
  /* Get new ID voltages and save in prvIdAsMilliVolt[n] */

  /* Get the ID as a number and save in prvChannelType[n] */
//  volatile uint16_t IdNumber = 32 * (prvIdAsMilliVolt[0]) / 3300;

  /* Convert the millivolt and ID to a string */
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void setActiveColorsForAlertBox(uint32_t Id)
{
  GUIAlertBox* pTempAlertBox = GUIAlertBox_GetFromId(Id);
  /* Channel 1 */
  if (prvCurrentlyActiveSidebar == APP_ActiveSidebar_1)
  {
    pTempAlertBox->backgroundColor    = COLOR_APP_CH1;
    pTempAlertBox->titleTextColor   = COLOR_APP_CH1;
  }
  /* Channel 2 */
  else if (prvCurrentlyActiveSidebar == APP_ActiveSidebar_2)
  {
    pTempAlertBox->backgroundColor    = COLOR_APP_CH2;
    pTempAlertBox->titleTextColor   = COLOR_APP_CH2;
  }
  /* Channel 3 */
  else if (prvCurrentlyActiveSidebar == APP_ActiveSidebar_3)
  {
    pTempAlertBox->backgroundColor    = COLOR_APP_CH3;
    pTempAlertBox->titleTextColor   = COLOR_APP_CH3;
  }
  /* Channel 4 */
  else if (prvCurrentlyActiveSidebar == APP_ActiveSidebar_4)
  {
    pTempAlertBox->backgroundColor    = COLOR_APP_CH4;
    pTempAlertBox->titleTextColor   = COLOR_APP_CH4;
  }
  /* Channel 5 */
  else if (prvCurrentlyActiveSidebar == APP_ActiveSidebar_5)
  {
    pTempAlertBox->backgroundColor    = COLOR_APP_CH5;
    pTempAlertBox->titleTextColor   = COLOR_APP_CH5;
  }
  /* Channel 6 */
  else if (prvCurrentlyActiveSidebar == APP_ActiveSidebar_6)
  {
    pTempAlertBox->backgroundColor    = COLOR_APP_CH6;
    pTempAlertBox->titleTextColor   = COLOR_APP_CH6;
  }

  /* Init with the new colors */
  GUIAlertBox_InitColors(pTempAlertBox);
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void setActiveColorsForButtonGridBox(uint32_t Id)
{
  GUIButtonGridBox* pTempButtonGridBox = GUIButtonGridBox_GetFromId(Id);
  /* Channel 1 */
  if (prvCurrentlyActiveSidebar == APP_ActiveSidebar_1)
  {
    pTempButtonGridBox->backgroundColor         = COLOR_APP_CH1;
    pTempButtonGridBox->titleTextColor          = COLOR_APP_CH1;
    pTempButtonGridBox->labelsBackgroundColor       = COLOR_APP_CH1;
    pTempButtonGridBox->buttonsState1TextColor      = COLOR_APP_CH1;
    pTempButtonGridBox->buttonsState2BackgroundColor  = COLOR_APP_CH1_DARK;
    pTempButtonGridBox->buttonsPressedBackgroundColor = COLOR_APP_CH1;
  }
  /* Channel 2 */
  else if (prvCurrentlyActiveSidebar == APP_ActiveSidebar_2)
  {
    pTempButtonGridBox->backgroundColor         = COLOR_APP_CH2;
    pTempButtonGridBox->titleTextColor          = COLOR_APP_CH2;
    pTempButtonGridBox->labelsBackgroundColor       = COLOR_APP_CH2;
    pTempButtonGridBox->buttonsState1TextColor      = COLOR_APP_CH2;
    pTempButtonGridBox->buttonsState2BackgroundColor  = COLOR_APP_CH2_DARK;
    pTempButtonGridBox->buttonsPressedBackgroundColor = COLOR_APP_CH2;
  }
  /* Channel 3 */
  else if (prvCurrentlyActiveSidebar == APP_ActiveSidebar_3)
  {
    pTempButtonGridBox->backgroundColor         = COLOR_APP_CH3;
    pTempButtonGridBox->titleTextColor          = COLOR_APP_CH3;
    pTempButtonGridBox->labelsBackgroundColor       = COLOR_APP_CH3;
    pTempButtonGridBox->buttonsState1TextColor      = COLOR_APP_CH3;
    pTempButtonGridBox->buttonsState2BackgroundColor  = COLOR_APP_CH3_DARK;
    pTempButtonGridBox->buttonsPressedBackgroundColor = COLOR_APP_CH3;
  }
  /* Channel 4 */
  else if (prvCurrentlyActiveSidebar == APP_ActiveSidebar_4)
  {
    pTempButtonGridBox->backgroundColor         = COLOR_APP_CH4;
    pTempButtonGridBox->titleTextColor          = COLOR_APP_CH4;
    pTempButtonGridBox->labelsBackgroundColor       = COLOR_APP_CH4;
    pTempButtonGridBox->buttonsState1TextColor      = COLOR_APP_CH4;
    pTempButtonGridBox->buttonsState2BackgroundColor  = COLOR_APP_CH4_DARK;
    pTempButtonGridBox->buttonsPressedBackgroundColor = COLOR_APP_CH4;
  }
  /* Channel 5 */
  else if (prvCurrentlyActiveSidebar == APP_ActiveSidebar_5)
  {
    pTempButtonGridBox->backgroundColor         = COLOR_APP_CH5;
    pTempButtonGridBox->titleTextColor          = COLOR_APP_CH5;
    pTempButtonGridBox->labelsBackgroundColor       = COLOR_APP_CH5;
    pTempButtonGridBox->buttonsState1TextColor      = COLOR_APP_CH5;
    pTempButtonGridBox->buttonsState2BackgroundColor  = COLOR_APP_CH5_DARK;
    pTempButtonGridBox->buttonsPressedBackgroundColor = COLOR_APP_CH5;
  }
  /* Channel 6 */
  else if (prvCurrentlyActiveSidebar == APP_ActiveSidebar_6)
  {
    pTempButtonGridBox->backgroundColor         = COLOR_APP_CH6;
    pTempButtonGridBox->titleTextColor          = COLOR_APP_CH6;
    pTempButtonGridBox->labelsBackgroundColor       = COLOR_APP_CH6;
    pTempButtonGridBox->buttonsState1TextColor      = COLOR_APP_CH6;
    pTempButtonGridBox->buttonsState2BackgroundColor  = COLOR_APP_CH6_DARK;
    pTempButtonGridBox->buttonsPressedBackgroundColor = COLOR_APP_CH6;
  }

  /* Init with the new colors */
  GUIButtonGridBox_InitColors(pTempButtonGridBox);
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void updateParityString()
{
  /* Update the parity string */
  if (prvCurrentParity == APP_Parity_Odd)
    prvCurrentParityString = "Odd";
  else if (prvCurrentParity == APP_Parity_Even)
    prvCurrentParityString = "Even";
  else if (prvCurrentParity == APP_Parity_None)
    prvCurrentParityString = "None";
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void updateDirectionString()
{
  /* Temporary storage */
  char* chAString;
  char* chBString;

  /* Channel A */
  if (prvCurrentChannelDirection[0] == APP_ChannelDirection_Off)
    chAString = "Off";
  else if (prvCurrentChannelDirection[0] == APP_ChannelDirection_TX)
    chAString = "TX";
  else if (prvCurrentChannelDirection[0] == APP_ChannelDirection_RX)
    chAString = "RX";
  else
    return;

  /* Channel B */
  if (prvCurrentChannelDirection[1] == APP_ChannelDirection_Off)
    chBString = "Off";
  else if (prvCurrentChannelDirection[1] == APP_ChannelDirection_TX)
    chBString = "TX";
  else if (prvCurrentChannelDirection[1] == APP_ChannelDirection_RX)
    chBString = "RX";
  else
    return;

  /* Info: http://www.cplusplus.com/reference/cstdio/sprintf/ */
  sprintf(prvCurrentDirectionString, "A: %s - B: %s", chAString, chBString);
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void setActiveSidebar(APP_ActiveSidebar NewActiveChannel)
{
  if (IS_APP_ACTIVE_SIDEBAR(NewActiveChannel) && NewActiveChannel != prvCurrentlyActiveSidebar)
  {
    /* Store the active page of the currently active channel's sidebar */
    uint16_t activePage = GUIButtonList_GetActivePage(GUIButtonListId_Sidebar);
    if (activePage != 0xFFF)
      prvSidebarActivePage[prvCurrentlyActiveSidebar] = activePage;

    prvCurrentlyActiveSidebar = NewActiveChannel;

    /* Check if it's a channel sidebar */
    if (ACTIVE_SIDEBAR_IS_FOR_A_CHANNEL(prvCurrentlyActiveSidebar))
    {
      /* Copy the template sidebar for this channel */
      if (prvChannelType[prvCurrentlyActiveSidebar] == APP_ChannelType_SETUP)
        memcpy(&prvTempButtonList, &SETUP_SidebarTemplate, sizeof(GUIButtonList));
      else if (prvChannelType[prvCurrentlyActiveSidebar] == APP_ChannelType_UART)
        memcpy(&prvTempButtonList, &UART_SidebarTemplate, sizeof(GUIButtonList));
      else if (prvChannelType[prvCurrentlyActiveSidebar] == APP_ChannelType_GPIO)
        memcpy(&prvTempButtonList, &GPIO_SidebarTemplate, sizeof(GUIButtonList));
      else if (prvChannelType[prvCurrentlyActiveSidebar] == APP_ChannelType_CAN)
        memcpy(&prvTempButtonList, &CAN_SidebarTemplate, sizeof(GUIButtonList));
      else if (prvChannelType[prvCurrentlyActiveSidebar] == APP_ChannelType_RS_232)
        memcpy(&prvTempButtonList, &RS_232_SidebarTemplate, sizeof(GUIButtonList));
      else
        memcpy(&prvTempButtonList, &NA_SidebarTemplate, sizeof(GUIButtonList));
    }
    else if (prvCurrentlyActiveSidebar == APP_ActiveSidebar_System)
      memcpy(&prvTempButtonList, &SYSTEM_SidebarTemplate, sizeof(GUIButtonList));
    else
      return; /* TODO: */

    /* Channel 1 */
    if (prvCurrentlyActiveSidebar == APP_ActiveSidebar_1)
    {
      /* Sidebar */
      prvTempButtonList.object.id                     = GUIButtonListId_Sidebar;
      prvTempButtonList.backgroundColor               = COLOR_APP_CH1_DARK;
      prvTempButtonList.titleBackgroundColor          = COLOR_WHITE;
      prvTempButtonList.titleTextColor[0]             = COLOR_APP_CH1;
      prvTempButtonList.titleTextColor[1]             = COLOR_APP_RED;
      prvTempButtonList.buttonsState1TextColor        = COLOR_WHITE;
      prvTempButtonList.buttonsState1BackgroundColor  = COLOR_APP_CH1;
      prvTempButtonList.buttonsState2TextColor        = COLOR_WHITE;
      prvTempButtonList.buttonsState2BackgroundColor  = COLOR_APP_CH1_DARK;
      prvTempButtonList.buttonsPressedTextColor       = COLOR_APP_CH1;
      prvTempButtonList.buttonsPressedBackgroundColor = COLOR_WHITE;
      prvTempButtonList.actionButtonPressed           = buttonInSidebarPressed;
    }
    /* Channel 2 */
    else if (prvCurrentlyActiveSidebar == APP_ActiveSidebar_2)
    {
      /* Sidebar */
      prvTempButtonList.object.id                     = GUIButtonListId_Sidebar;
      prvTempButtonList.backgroundColor               = COLOR_APP_CH2_DARK;
      prvTempButtonList.titleBackgroundColor          = COLOR_WHITE;
      prvTempButtonList.titleTextColor[0]             = COLOR_APP_CH2;
      prvTempButtonList.titleTextColor[1]             = COLOR_APP_RED;
      prvTempButtonList.buttonsState1TextColor        = COLOR_WHITE;
      prvTempButtonList.buttonsState1BackgroundColor  = COLOR_APP_CH2;
      prvTempButtonList.buttonsState2TextColor        = COLOR_WHITE;
      prvTempButtonList.buttonsState2BackgroundColor  = COLOR_APP_CH2_DARK;
      prvTempButtonList.buttonsPressedTextColor       = COLOR_APP_CH2;
      prvTempButtonList.buttonsPressedBackgroundColor = COLOR_WHITE;
      prvTempButtonList.actionButtonPressed           = buttonInSidebarPressed;
    }
    /* Channel 3 */
    else if (prvCurrentlyActiveSidebar == APP_ActiveSidebar_3)
    {
      /* Sidebar */
      prvTempButtonList.object.id                     = GUIButtonListId_Sidebar;
      prvTempButtonList.backgroundColor               = COLOR_APP_CH3_DARK;
      prvTempButtonList.titleBackgroundColor          = COLOR_WHITE;
      prvTempButtonList.titleTextColor[0]             = COLOR_APP_CH3;
      prvTempButtonList.titleTextColor[1]             = COLOR_APP_RED;
      prvTempButtonList.buttonsState1TextColor        = COLOR_WHITE;
      prvTempButtonList.buttonsState1BackgroundColor  = COLOR_APP_CH3;
      prvTempButtonList.buttonsState2TextColor        = COLOR_WHITE;
      prvTempButtonList.buttonsState2BackgroundColor  = COLOR_APP_CH3_DARK;
      prvTempButtonList.buttonsPressedTextColor       = COLOR_APP_CH3;
      prvTempButtonList.buttonsPressedBackgroundColor = COLOR_WHITE;
      prvTempButtonList.actionButtonPressed           = buttonInSidebarPressed;
    }
    /* Channel 4 */
    else if (prvCurrentlyActiveSidebar == APP_ActiveSidebar_4)
    {
      /* Sidebar */
      prvTempButtonList.object.id                     = GUIButtonListId_Sidebar;
      prvTempButtonList.backgroundColor               = COLOR_APP_CH4_DARK;
      prvTempButtonList.titleBackgroundColor          = COLOR_WHITE;
      prvTempButtonList.titleTextColor[0]             = COLOR_APP_CH4;
      prvTempButtonList.titleTextColor[1]             = COLOR_APP_RED;
      prvTempButtonList.buttonsState1TextColor        = COLOR_WHITE;
      prvTempButtonList.buttonsState1BackgroundColor  = COLOR_APP_CH4;
      prvTempButtonList.buttonsState2TextColor        = COLOR_WHITE;
      prvTempButtonList.buttonsState2BackgroundColor  = COLOR_APP_CH4_DARK;
      prvTempButtonList.buttonsPressedTextColor       = COLOR_APP_CH4;
      prvTempButtonList.buttonsPressedBackgroundColor = COLOR_WHITE;
      prvTempButtonList.actionButtonPressed           = buttonInSidebarPressed;
    }
    /* Channel 5 */
    else if (prvCurrentlyActiveSidebar == APP_ActiveSidebar_5)
    {
      /* Sidebar */
      prvTempButtonList.object.id                     = GUIButtonListId_Sidebar;
      prvTempButtonList.backgroundColor               = COLOR_APP_CH5_DARK;
      prvTempButtonList.titleBackgroundColor          = COLOR_WHITE;
      prvTempButtonList.titleTextColor[0]             = COLOR_APP_CH5;
      prvTempButtonList.titleTextColor[1]             = COLOR_APP_RED;
      prvTempButtonList.buttonsState1TextColor        = COLOR_WHITE;
      prvTempButtonList.buttonsState1BackgroundColor  = COLOR_APP_CH5;
      prvTempButtonList.buttonsState2TextColor        = COLOR_WHITE;
      prvTempButtonList.buttonsState2BackgroundColor  = COLOR_APP_CH5_DARK;
      prvTempButtonList.buttonsPressedTextColor       = COLOR_APP_CH5;
      prvTempButtonList.buttonsPressedBackgroundColor = COLOR_WHITE;
      prvTempButtonList.actionButtonPressed           = buttonInSidebarPressed;
    }
    /* Channel 6 */
    else if (prvCurrentlyActiveSidebar == APP_ActiveSidebar_6)
    {
      /* Sidebar */
      prvTempButtonList.object.id                     = GUIButtonListId_Sidebar;
      prvTempButtonList.backgroundColor               = COLOR_APP_CH6_DARK;
      prvTempButtonList.titleBackgroundColor          = COLOR_WHITE;
      prvTempButtonList.titleTextColor[0]             = COLOR_APP_CH6;
      prvTempButtonList.titleTextColor[1]             = COLOR_APP_RED;
      prvTempButtonList.buttonsState1TextColor        = COLOR_WHITE;
      prvTempButtonList.buttonsState1BackgroundColor  = COLOR_APP_CH6;
      prvTempButtonList.buttonsState2TextColor        = COLOR_WHITE;
      prvTempButtonList.buttonsState2BackgroundColor  = COLOR_APP_CH6_DARK;
      prvTempButtonList.buttonsPressedTextColor       = COLOR_APP_CH6;
      prvTempButtonList.buttonsPressedBackgroundColor = COLOR_WHITE;
      prvTempButtonList.actionButtonPressed           = buttonInSidebarPressed;
    }
    /* System */
    else if (prvCurrentlyActiveSidebar == APP_ActiveSidebar_System)
    {
      /* Sidebar */
      prvTempButtonList.object.id                     = GUIButtonListId_Sidebar;
      prvTempButtonList.backgroundColor               = COLOR_APP_SYSTEM_DARK;
      prvTempButtonList.titleBackgroundColor          = COLOR_WHITE;
      prvTempButtonList.titleTextColor[0]             = COLOR_APP_SYSTEM;
      prvTempButtonList.titleTextColor[1]             = COLOR_APP_RED;
      prvTempButtonList.buttonsState1TextColor        = COLOR_WHITE;
      prvTempButtonList.buttonsState1BackgroundColor  = COLOR_APP_SYSTEM;
      prvTempButtonList.buttonsState2TextColor        = COLOR_WHITE;
      prvTempButtonList.buttonsState2BackgroundColor  = COLOR_APP_SYSTEM_DARK;
      prvTempButtonList.buttonsPressedTextColor       = COLOR_APP_SYSTEM;
      prvTempButtonList.buttonsPressedBackgroundColor = COLOR_WHITE;
      prvTempButtonList.actionButtonPressed           = buttonInSidebarPressed;
    }

    /* TODO: Read the module ID */

    /* Check if the currently active sidebar is for a channel */
    if (ACTIVE_SIDEBAR_IS_FOR_A_CHANNEL(prvCurrentlyActiveSidebar))
    {
      if (prvChannelType[prvCurrentlyActiveSidebar] == APP_ChannelType_NA)
        setNASidebarItems();
      else if (prvChannelType[prvCurrentlyActiveSidebar] == APP_ChannelType_SETUP)
        setSETUPSidebarItems();
      else if (prvChannelType[prvCurrentlyActiveSidebar] == APP_ChannelType_UART)
        setUARTSidebarItems();
      else if (prvChannelType[prvCurrentlyActiveSidebar] == APP_ChannelType_GPIO)
        setGPIOSidebarItems();
      else if (prvChannelType[prvCurrentlyActiveSidebar] == APP_ChannelType_CAN)
        setCANSidebarItems();
      else if (prvChannelType[prvCurrentlyActiveSidebar] == APP_ChannelType_RS_232)
        setRS232SidebarItems();
    }
    /* Check if the currently active sidebar is for the system */
    else if (prvCurrentlyActiveSidebar == APP_ActiveSidebar_System)
      setSystemSidebarItems();

    /* Init the sidebar */
    GUIButtonList_Init(&prvTempButtonList);
    /* Set the active page to what is was before */
    if (prvTempButtonList.activePage != prvSidebarActivePage[prvCurrentlyActiveSidebar])
      GUIButtonList_SetActivePage(GUIButtonListId_Sidebar, prvSidebarActivePage[prvCurrentlyActiveSidebar]);

    /* Clear the sidebar if was displayed before */
    if (GUIButtonList_GetDisplayState(GUIButtonListId_Sidebar) == GUIDisplayState_NotHidden)
      GUIButtonList_Clear(GUIButtonListId_Sidebar);
    /* Then draw the new sidebar */
    GUIButtonList_Draw(GUIButtonListId_Sidebar);

    /* Update the display */
    GUI_DrawAllLayersAndRefreshDisplay();
  }
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void setNASidebarItems()
{
  /* Module Power */
  if (prvModulePowerEnabled[prvCurrentlyActiveSidebar])
    prvTempButtonList.buttonText[NA_MODULE_POWER_INDEX][1] = "Enabled";
  else
    prvTempButtonList.buttonText[NA_MODULE_POWER_INDEX][1] = "Disabled";
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void setSETUPSidebarItems()
{
  /* Module Power */
  if (prvModulePowerEnabled[prvCurrentlyActiveSidebar])
    prvTempButtonList.buttonText[SETUP_MODULE_POWER_INDEX][1] = "Enabled";
  else
    prvTempButtonList.buttonText[SETUP_MODULE_POWER_INDEX][1] = "Disabled";
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void setUARTSidebarItems()
{
  /* Channel status */
  if (prvChannelIsEnabled[prvCurrentlyActiveSidebar])
    prvTempButtonList.buttonText[UART_CAPTURE_INDEX][0] = "Start Capture";
  else
    prvTempButtonList.buttonText[UART_CAPTURE_INDEX][0] = "Stop Capture";

  /* Parity */
  updateParityString();
  prvTempButtonList.buttonText[UART_PARITY_INDEX][1] = prvCurrentParityString;

  /* Splitscreen */
  if (prvModulePowerEnabled[prvCurrentlyActiveSidebar])
    prvTempButtonList.buttonText[UART_SPLITSCREEN_INDEX][1] = "Enabled";
  else
    prvTempButtonList.buttonText[UART_SPLITSCREEN_INDEX][1] = "Disabled";

  /* Direction */
  updateDirectionString();
  prvTempButtonList.buttonText[UART_CHANNEL_MODE_INDEX][1] = prvCurrentDirectionString;

  /* Module Power */
  if (prvModulePowerEnabled[prvCurrentlyActiveSidebar])
    prvTempButtonList.buttonText[UART_MODULE_POWER_INDEX][1] = "Enabled";
  else
    prvTempButtonList.buttonText[UART_MODULE_POWER_INDEX][1] = "Disabled";
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void setGPIOSidebarItems()
{
  /* Module Power */
  if (prvModulePowerEnabled[prvCurrentlyActiveSidebar])
    prvTempButtonList.buttonText[GPIO_MODULE_POWER_INDEX][1] = "Enabled";
  else
    prvTempButtonList.buttonText[GPIO_MODULE_POWER_INDEX][1] = "Disabled";
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void setCANSidebarItems()
{
  /* Channel status */
  if (prvChannelIsEnabled[prvCurrentlyActiveSidebar])
    prvTempButtonList.buttonText[CAN_CAPTURE_INDEX][1] = "Enabled";
  else
    prvTempButtonList.buttonText[CAN_CAPTURE_INDEX][1] = "Disabled";

  /* Module Power */
  if (prvModulePowerEnabled[prvCurrentlyActiveSidebar])
    prvTempButtonList.buttonText[CAN_MODULE_POWER_INDEX][1] = "Enabled";
  else
    prvTempButtonList.buttonText[CAN_MODULE_POWER_INDEX][1] = "Disabled";
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void setRS232SidebarItems()
{
  /* Channel status */
  if (prvChannelIsEnabled[prvCurrentlyActiveSidebar])
    prvTempButtonList.buttonText[RS_232_CAPTURE_INDEX][1] = "Enabled";
  else
    prvTempButtonList.buttonText[RS_232_CAPTURE_INDEX][1] = "Disabled";

  /* Parity */
  updateParityString();
  prvTempButtonList.buttonText[UART_PARITY_INDEX][1] = prvCurrentParityString;

  /* Splitscreen */
  if (prvChannelIsEnabled[prvCurrentlyActiveSidebar])
    prvTempButtonList.buttonText[RS_232_SPLITSCREEN_INDEX][1] = "Enabled";
  else
    prvTempButtonList.buttonText[RS_232_SPLITSCREEN_INDEX][1] = "Disabled";

  /* Direction */
  updateDirectionString();
  prvTempButtonList.buttonText[UART_CHANNEL_MODE_INDEX][1] = prvCurrentDirectionString;

  /* Module Power */
  if (prvModulePowerEnabled[prvCurrentlyActiveSidebar])
    prvTempButtonList.buttonText[RS_232_MODULE_POWER_INDEX][1] = "Enabled";
  else
    prvTempButtonList.buttonText[RS_232_MODULE_POWER_INDEX][1] = "Disabled";
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void setSystemSidebarItems()
{
  /* Touch Buzzer */
  if (prvCurrentBuzzerSound == APP_BuzzerSound_Off)
    prvTempButtonList.buttonText[SYSTEM_TOUCH_BUZZER_INDEX][1] = "Off";
  else if (prvCurrentBuzzerSound == APP_BuzzerSound_Low)
    prvTempButtonList.buttonText[SYSTEM_TOUCH_BUZZER_INDEX][1] = "Low";
  else if (prvCurrentBuzzerSound == APP_BuzzerSound_Medium)
    prvTempButtonList.buttonText[SYSTEM_TOUCH_BUZZER_INDEX][1] = "Medium";
  else if (prvCurrentBuzzerSound == APP_BuzzerSound_High)
    prvTempButtonList.buttonText[SYSTEM_TOUCH_BUZZER_INDEX][1] = "High";

  /* Enable prompt enabled */
  if (prvEnablePromptEnabled)
    prvTempButtonList.buttonText[SYSTEM_ENABLE_PROMPT_INDEX][1] = "Enabled";
  else
    prvTempButtonList.buttonText[SYSTEM_ENABLE_PROMPT_INDEX][1] = "Disabled";

  /* Power Source */
  if (prvCurrentPowerSource == APP_PowerSource_ExternalModule)
    prvTempButtonList.buttonText[SYSTEM_POWER_SOURCE_INDEX][1] = "External Module";
  else if (prvCurrentPowerSource == APP_PowerSource_USB)
    prvTempButtonList.buttonText[SYSTEM_POWER_SOURCE_INDEX][1] = "USB";
  else if (prvCurrentPowerSource == APP_PowerSource_Battery)
    prvTempButtonList.buttonText[SYSTEM_POWER_SOURCE_INDEX][1] = "Battery";
}

/**
 * @brief
 * @param ButtonListId:
 * @param ButtonListId:
 * @retval  None
 */
static void buttonInSidebarPressed(uint32_t ButtonListId, uint32_t ButtonIndex)
{
  /* Check if the system sidebar is active */
  if (prvCurrentlyActiveSidebar == APP_ActiveSidebar_System)
    buttonPressedInSystemSidebar(ButtonIndex);
  /* Check if it's the empty sidebar */
  else if (prvCurrentlyActiveSidebar == APP_ActiveSidebar_None)
  {

  }
  /* Otherwise it's a channel sidebar that is active */
  else
  {
    /* Depending on what type the current channel is, the ButtonIndexes means different things */
    if (prvChannelType[prvCurrentlyActiveSidebar] == APP_ChannelType_NA)
      buttonPressedInNASidebar(ButtonIndex);
    else if (prvChannelType[prvCurrentlyActiveSidebar] == APP_ChannelType_SETUP)
      buttonPressedInSetupSidebar(ButtonIndex);
    else if (prvChannelType[prvCurrentlyActiveSidebar] == APP_ChannelType_UART)
      buttonPressedInUARTSidebar(ButtonIndex);
    else if (prvChannelType[prvCurrentlyActiveSidebar] == APP_ChannelType_GPIO)
      buttonPressedInGPIOSidebar(ButtonIndex);
    else if (prvChannelType[prvCurrentlyActiveSidebar] == APP_ChannelType_CAN)
      buttonPressedInCANSidebar(ButtonIndex);
    else if (prvChannelType[prvCurrentlyActiveSidebar] == APP_ChannelType_RS_232)
      buttonPressedInRS232Sidebar(ButtonIndex);
  }
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void buttonPressedInSystemSidebar(uint32_t ButtonIndex)
{
  /* Time */
  if (ButtonIndex == SYSTEM_TIME_INDEX)
  {

  }
  /* Version info */
  else if (ButtonIndex == SYSTEM_VERSION_INFO_INDEX)
  {
    GUIInfoBox_Draw(GUIInfoBoxId_VersionInfo);
  }
  /* Refresh Module IDs */
  else if (ButtonIndex == SYSTEM_REFRESH_MODULE_IDS_INDEX)
  {
    GUIAlertBox_Draw(GUIAlertBoxId_RefreshIds);
  }
  /* Clear all data memory */
  else if (ButtonIndex == SYSTEM_CLEAR_DATA_MEMORY_INDEX)
  {
    GUIAlertBox_Draw(GUIAlertBoxId_ClearAllMemory);
  }
  /* Touch Buzzer */
  else if (ButtonIndex == SYSTEM_TOUCH_BUZZER_INDEX)
  {
    /* Increment the setting variable */
    if (prvCurrentBuzzerSound == APP_BuzzerSound_High)
      prvCurrentBuzzerSound = APP_BuzzerSound_Off;
    else
      prvCurrentBuzzerSound++;
    /* Update the text of the button */
    if (prvCurrentBuzzerSound == APP_BuzzerSound_Off)
      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, SYSTEM_TOUCH_BUZZER_INDEX, 0, "Off");
    else if (prvCurrentBuzzerSound == APP_BuzzerSound_Low)
      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, SYSTEM_TOUCH_BUZZER_INDEX, 0, "Low");
    else if (prvCurrentBuzzerSound == APP_BuzzerSound_Medium)
      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, SYSTEM_TOUCH_BUZZER_INDEX, 0, "Medium");
    else if (prvCurrentBuzzerSound == APP_BuzzerSound_High)
      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, SYSTEM_TOUCH_BUZZER_INDEX, 0, "High");

    /* TODO: Do something */
  }
  /* LCD Brightness */
  else if (ButtonIndex == SYSTEM_LCD_BRIGHTNESS_INDEX)
  {

  }
  /* Enable prompt */
  else if (ButtonIndex == SYSTEM_ENABLE_PROMPT_INDEX)
  {
    prvEnablePromptEnabled = !prvEnablePromptEnabled;
    if (prvEnablePromptEnabled)
      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, SYSTEM_ENABLE_PROMPT_INDEX, 0, "Enabled");
    else
      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, SYSTEM_ENABLE_PROMPT_INDEX, 0, "Disabled");
    /* TODO: Do something */
  }
  /* LCD off timer */
  else if (ButtonIndex == SYSTEM_LCD_OFF_TIMER_INDEX)
  {

  }
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void buttonPressedInNASidebar(uint32_t ButtonIndex)
{
  /* Refresh ID */
  if (ButtonIndex == NA_REFRESH_ID_INDEX || ButtonIndex == NA_REFRESH_ID_EXTRA_INDEX)
  {

  }
  /* Module Power */
  else if (ButtonIndex == NA_MODULE_POWER_INDEX)
  {
    prvModulePowerEnabled[prvCurrentlyActiveSidebar] = !prvModulePowerEnabled[prvCurrentlyActiveSidebar];
    if (prvModulePowerEnabled[prvCurrentlyActiveSidebar])
      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, NA_MODULE_POWER_INDEX, 0, "Enabled");
    else
      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, NA_MODULE_POWER_INDEX, 0, "Disabled");
    /* TODO: Do something */
  }
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void buttonPressedInSetupSidebar(uint32_t ButtonIndex)
{
  /* Refresh ID */
  if (ButtonIndex == SETUP_REFRESH_ID_INDEX)
  {

  }
  /* Module Power */
  else if (ButtonIndex == SETUP_MODULE_POWER_INDEX)
  {
    prvModulePowerEnabled[prvCurrentlyActiveSidebar] = !prvModulePowerEnabled[prvCurrentlyActiveSidebar];
    if (prvModulePowerEnabled[prvCurrentlyActiveSidebar])
      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, SETUP_MODULE_POWER_INDEX, 0, "Enabled");
    else
      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, SETUP_MODULE_POWER_INDEX, 0, "Disabled");
    /* TODO: Do something */
  }
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void buttonPressedInUARTSidebar(uint32_t ButtonIndex)
{
  /* Enable/disable channel alert box */
  if (ButtonIndex == UART_CAPTURE_INDEX)
  {
//    setActiveColorsForAlertBox(GUIAlertBoxId_EnableChannel);
//    if (prvChannelIsEnabled[prvCurrentlyActiveSidebar])
//    {
//      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, UART_CAPTURE_INDEX, 0, "Disabled");
//      prvChannelIsEnabled[prvCurrentlyActiveSidebar] = false;
//    }
//    else if (prvEnablePromptEnabled)
//    {
//      GUIAlertBox_Draw(GUIAlertBoxId_EnableChannel);
//    }
//    else
//    {
//      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, UART_CAPTURE_INDEX, 0, "Enabled");
//      prvChannelIsEnabled[prvCurrentlyActiveSidebar] = true;
//    }
  }
  /* Baud rate button grid box */
  else if (ButtonIndex == UART_BAUD_RATE_INDEX)
  {

  }
  /* Parity button grid box */
  else if (ButtonIndex == UART_PARITY_INDEX)
  {
    setActiveColorsForButtonGridBox(GUIButtonGridBoxId_ParitySelection);
    GUIButtonGridBox_Draw(GUIButtonGridBoxId_ParitySelection);
  }
  /* Data bits button grid box */
  else if (ButtonIndex == UART_DATA_BITS_INDEX)
  {

  }
  /* Display format button grid box */
  else if (ButtonIndex == UART_DISPLAY_FORMAT_INDEX)
  {

  }
  /* Clear */
  else if (ButtonIndex == UART_CLEAR_INDEX)
  {

  }
  /* Module mode button grid box */
  else if (ButtonIndex == UART_MODULE_MODE_INDEX)
  {

  }
  /* Timebase button grid box */
  else if (ButtonIndex == UART_TIMEBASE_INDEX)
  {

  }
  /* Splitscreen */
  else if (ButtonIndex == UART_SPLITSCREEN_INDEX)
  {
    prvChannelSplitscreenEnabled[prvCurrentlyActiveSidebar] = !prvChannelSplitscreenEnabled[prvCurrentlyActiveSidebar];
    if (prvChannelSplitscreenEnabled[prvCurrentlyActiveSidebar])
      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, UART_SPLITSCREEN_INDEX, 0, "Enabled");
    else
      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, UART_SPLITSCREEN_INDEX, 0, "Disabled");
  }
  /* Direction button grid box */
  else if (ButtonIndex == UART_CHANNEL_MODE_INDEX)
  {
    setActiveColorsForButtonGridBox(GUIButtonGridBoxId_DirectionSelection);
    GUIButtonGridBox_Draw(GUIButtonGridBoxId_DirectionSelection);
  }
  /* Refresh ID */
  else if (ButtonIndex == UART_REFRESH_ID_INDEX)
  {

  }
  /* Module Power */
  else if (ButtonIndex == UART_MODULE_POWER_INDEX)
  {
    /* TODO: Read back and make sure that the power is actually enabled, don't assume */
    prvModulePowerEnabled[prvCurrentlyActiveSidebar] = !prvModulePowerEnabled[prvCurrentlyActiveSidebar];
    if (prvModulePowerEnabled[prvCurrentlyActiveSidebar])
    {
      SPI_COMM_EnablePowerForChannel(prvChannelNumberFromActiveSidebar[prvCurrentlyActiveSidebar]);
      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, UART_MODULE_POWER_INDEX, 0, "Enabled");
    }
    else
    {
      SPI_COMM_DisablePowerForChannel(prvChannelNumberFromActiveSidebar[prvCurrentlyActiveSidebar]);
      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, UART_MODULE_POWER_INDEX, 0, "Disabled");
    }
  }
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void buttonPressedInGPIOSidebar(uint32_t ButtonIndex)
{
  /* Module mode button grid box */
  if (ButtonIndex == GPIO_MODULE_MODE_INDEX)
  {

  }
  /* Refresh ID */
  else if (ButtonIndex == GPIO_REFRESH_ID_INDEX)
  {

  }
  /* Module Power */
  else if (ButtonIndex == GPIO_MODULE_POWER_INDEX)
  {
    prvModulePowerEnabled[prvCurrentlyActiveSidebar] = !prvModulePowerEnabled[prvCurrentlyActiveSidebar];
    if (prvModulePowerEnabled[prvCurrentlyActiveSidebar])
      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, GPIO_MODULE_POWER_INDEX, 0, "Enabled");
    else
      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, GPIO_MODULE_POWER_INDEX, 0, "Disabled");
    /* TODO: Do something */
  }
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void buttonPressedInCANSidebar(uint32_t ButtonIndex)
{
  /* Enable/disable channel alert box */
  if (ButtonIndex == CAN_CAPTURE_INDEX)
  {
//    setActiveColorsForAlertBox(GUIAlertBoxId_EnableChannel);
//    if (prvChannelIsEnabled[prvCurrentlyActiveSidebar])
//    {
//      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, CAN_CAPTURE_INDEX, 0, "Disabled");
//      prvChannelIsEnabled[prvCurrentlyActiveSidebar] = false;
//    }
//    else if (prvEnablePromptEnabled)
//    {
//      GUIAlertBox_Draw(GUIAlertBoxId_EnableChannel);
//    }
//    else
//    {
//      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, CAN_CAPTURE_INDEX, 0, "Enabled");
//      prvChannelIsEnabled[prvCurrentlyActiveSidebar] = true;
//    }
  }
  /* Bit rate button grid box */
  else if (ButtonIndex == CAN_BIT_RATE_INDEX)
  {

  }
  /* Termination */
  else if (ButtonIndex == CAN_TERMINATION_INDEX)
  {

  }
  /* Filters */
  else if (ButtonIndex == CAN_FILTERS_INDEX)
  {

  }
  /* Display mode button grid box */
  else if (ButtonIndex == CAN_DISPLAY_MODE_INDEX)
  {

  }
  /* Clear */
  else if (ButtonIndex == CAN_CLEAR_INDEX)
  {

  }
  /* Timebase button grid box */
  else if (ButtonIndex == CAN_TIMEBASE_INDEX)
  {

  }
  /* Refresh ID */
  else if (ButtonIndex == CAN_REFRESH_ID_INDEX)
  {

  }
  /* Module Power */
  else if (ButtonIndex == CAN_MODULE_POWER_INDEX)
  {
    prvModulePowerEnabled[prvCurrentlyActiveSidebar] = !prvModulePowerEnabled[prvCurrentlyActiveSidebar];
    if (prvModulePowerEnabled[prvCurrentlyActiveSidebar])
      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, CAN_MODULE_POWER_INDEX, 0, "Enabled");
    else
      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, CAN_MODULE_POWER_INDEX, 0, "Disabled");
    /* TODO: Do something */
  }
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void buttonPressedInRS232Sidebar(uint32_t ButtonIndex)
{
  /* Enable/disable channel alert box */
  if (ButtonIndex == RS_232_CAPTURE_INDEX)
  {
//    setActiveColorsForAlertBox(GUIAlertBoxId_EnableChannel);
//    if (prvChannelIsEnabled[prvCurrentlyActiveSidebar])
//    {
//      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, RS_232_CAPTURE_INDEX, 0, "Disabled");
//      prvChannelIsEnabled[prvCurrentlyActiveSidebar] = false;
//    }
//    else if (prvEnablePromptEnabled)
//    {
//      GUIAlertBox_Draw(GUIAlertBoxId_EnableChannel);
//    }
//    else
//    {
//      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, RS_232_CAPTURE_INDEX, 0, "Enabled");
//      prvChannelIsEnabled[prvCurrentlyActiveSidebar] = true;
//    }
  }
  /* Baud rate button grid box */
  else if (ButtonIndex == RS_232_BAUD_RATE_INDEX)
  {

  }
  /* Parity button grid box */
  else if (ButtonIndex == RS_232_PARITY_INDEX)
  {
    setActiveColorsForButtonGridBox(GUIButtonGridBoxId_ParitySelection);
    GUIButtonGridBox_Draw(GUIButtonGridBoxId_ParitySelection);
  }
  /* Data bits button grid box */
  else if (ButtonIndex == RS_232_DATA_BITS_INDEX)
  {

  }
  /* Display format button grid box */
  else if (ButtonIndex == RS_232_DISPLAY_FORMAT_INDEX)
  {

  }
  /* Clear */
  else if (ButtonIndex == RS_232_CLEAR_INDEX)
  {

  }
  /* Timebase button grid box */
  else if (ButtonIndex == RS_232_TIMEBASE_INDEX)
  {

  }
  /* Splitscreen */
  else if (ButtonIndex == RS_232_SPLITSCREEN_INDEX)
  {
    prvChannelSplitscreenEnabled[prvCurrentlyActiveSidebar] = !prvChannelSplitscreenEnabled[prvCurrentlyActiveSidebar];
    if (prvChannelSplitscreenEnabled[prvCurrentlyActiveSidebar])
      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, RS_232_SPLITSCREEN_INDEX, 0, "Enabled");
    else
      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, RS_232_SPLITSCREEN_INDEX, 0, "Disabled");
  }
  /* Direction button grid box */
  else if (ButtonIndex == RS_232_CHANNEL_MODE_INDEX)
  {
    setActiveColorsForButtonGridBox(GUIButtonGridBoxId_DirectionSelection);
    GUIButtonGridBox_Draw(GUIButtonGridBoxId_DirectionSelection);
  }
  /* Refresh ID */
  else if (ButtonIndex == RS_232_REFRESH_ID_INDEX)
  {

  }
  /* Module Power */
  else if (ButtonIndex == RS_232_MODULE_POWER_INDEX)
  {
    prvModulePowerEnabled[prvCurrentlyActiveSidebar] = !prvModulePowerEnabled[prvCurrentlyActiveSidebar];
    if (prvModulePowerEnabled[prvCurrentlyActiveSidebar])
      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, RS_232_MODULE_POWER_INDEX, 0, "Enabled");
    else
      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, RS_232_MODULE_POWER_INDEX, 0, "Disabled");
    /* TODO: Do something */
  }
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void refreshIdsAlertBoxCallback(GUIAlertBoxCallbackButton CallbackButton)
{
  /* If the left button was pressed we should refresh all IDs */
  if (CallbackButton == GUIAlertBoxCallbackButton_Left)
  {
    /* TODO: Do something */

    /* Clear the alert box */
    GUIAlertBox_Clear(GUIAlertBoxId_RefreshIds);
  }
  else if (CallbackButton == GUIAlertBoxCallbackButton_Right)
  {
    /* Clear the alert box */
    GUIAlertBox_Clear(GUIAlertBoxId_RefreshIds);
  }
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void clearAllMemoryAlertBoxCallback(GUIAlertBoxCallbackButton CallbackButton)
{
  /* If the left button was pressed we should clear all memory */
  if (CallbackButton == GUIAlertBoxCallbackButton_Left)
  {
    /* TODO: Do something */

    /* Clear the alert box */
    GUIAlertBox_Clear(GUIAlertBoxId_ClearAllMemory);
  }
  else if (CallbackButton == GUIAlertBoxCallbackButton_Right)
  {
    /* Clear the alert box */
    GUIAlertBox_Clear(GUIAlertBoxId_ClearAllMemory);
  }
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void buttonInParityBoxPressed(uint32_t Row, uint32_t Column)
{
  /* Sanity check */
  if (ACTIVE_SIDEBAR_IS_FOR_A_CHANNEL(prvCurrentlyActiveSidebar))
  {
    /* Reset the Button State of the previous */
    if (prvCurrentParity == APP_Parity_Odd)
      GUIButtonGridBox_SetButtonState(GUIButtonGridBoxId_ParitySelection, 0, 0, GUIButtonState_State1, true);
    else if (prvCurrentParity == APP_Parity_Even)
      GUIButtonGridBox_SetButtonState(GUIButtonGridBoxId_ParitySelection, 0, 1, GUIButtonState_State1, true);
    else if (prvCurrentParity == APP_Parity_None)
      GUIButtonGridBox_SetButtonState(GUIButtonGridBoxId_ParitySelection, 1, 0, GUIButtonState_State1, true);

    /* Check if it's a UART or RS-232 module */
    uint32_t buttonIndex = 0;
    if (prvChannelType[prvCurrentlyActiveSidebar] == APP_ChannelType_UART)
      buttonIndex = UART_PARITY_INDEX;
    else if (prvChannelType[prvCurrentlyActiveSidebar] == APP_ChannelType_RS_232)
      buttonIndex = RS_232_PARITY_INDEX;
    else
    {
      /* TODO: Error */
      return;
    }

    /* Update the curren parity */
    if (Row == 0 && Column == 0)
      prvCurrentParity = APP_Parity_Odd;
    else if (Row == 0 && Column == 1)
      prvCurrentParity = APP_Parity_Even;
    else if (Row == 1 && Column == 0)
      prvCurrentParity = APP_Parity_None;

    /* Update the parity string in the sidebar */
    updateParityString();
    GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, buttonIndex, 0, prvCurrentParityString);

    /* Clear the button grid box */
    GUIButtonGridBox_Clear(GUIButtonGridBoxId_ParitySelection);

    /* Change the button state of the selected item */
    GUIButtonGridBox_SetButtonState(GUIButtonGridBoxId_ParitySelection, Row, Column, GUIButtonState_State2, true);

    /* TODO: Do something with this new information (prvCurrentParity) */
  }
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void buttonInDirectionBoxPressed(uint32_t Row, uint32_t Column)
{
  /* Sanity check */
  if (ACTIVE_SIDEBAR_IS_FOR_A_CHANNEL(prvCurrentlyActiveSidebar))
  {
    /* Check if it's a UART or RS-232 module */
    uint32_t buttonIndex = 0;
    if (prvChannelType[prvCurrentlyActiveSidebar] == APP_ChannelType_UART)
      buttonIndex = UART_PARITY_INDEX;
    else if (prvChannelType[prvCurrentlyActiveSidebar] == APP_ChannelType_RS_232)
      buttonIndex = RS_232_PARITY_INDEX;
    else
    {
      /* TODO: Error */
      return;
    }

    /* Sanity check */
    if (Row == 0 || Row == 1)
    {
      /* Reset the previously selected direction */
      if (prvCurrentChannelDirection[Row] == APP_ChannelDirection_TX)
        GUIButtonGridBox_SetButtonState(GUIButtonGridBoxId_DirectionSelection, Row, 0, GUIButtonState_State1, true);
      else if (prvCurrentChannelDirection[Row] == APP_ChannelDirection_RX)
        GUIButtonGridBox_SetButtonState(GUIButtonGridBoxId_DirectionSelection, Row, 1, GUIButtonState_State1, true);
      else if (prvCurrentChannelDirection[Row] == APP_ChannelDirection_Off)
        GUIButtonGridBox_SetButtonState(GUIButtonGridBoxId_DirectionSelection, Row, 2, GUIButtonState_State1, true);

      /* TX */
      if (Column == 0)
        prvCurrentChannelDirection[Row] = APP_ChannelDirection_TX;
      /* RX */
      else if (Column == 1)
        prvCurrentChannelDirection[Row] = APP_ChannelDirection_RX;
      /* Off */
      else if (Column == 2)
        prvCurrentChannelDirection[Row] = APP_ChannelDirection_Off;
    }

    /* Clear the button grid box */
    GUIButtonGridBox_Clear(GUIButtonGridBoxId_DirectionSelection);

    /* Change the button state of the selected item */
    GUIButtonGridBox_SetButtonState(GUIButtonGridBoxId_DirectionSelection, Row, Column, GUIButtonState_State2, false);

    /* Update the sidebar item */
    updateDirectionString();
    GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, buttonIndex, 0, prvCurrentDirectionString);

    /* TODO: Do something with this new information (prvChannelDirection[Row]) */
  }
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void prvInitTopAndSystemItems()
{
  const uint16_t xDiff = 110;

  /* Channel 1 */
  prvLabel.object.id              = GUILabelId_Channel1Top;
  prvLabel.object.xPos            = 0;
  prvLabel.object.yPos            = 0;
  prvLabel.object.width           = 30;
  prvLabel.object.height          = 40;
  prvLabel.object.border          = GUIBorder_Bottom;
  prvLabel.object.borderThickness = 1;
  prvLabel.object.borderColor     = COLOR_WHITE;
  prvLabel.object.layer           = GUILayer_1;
  prvLabel.object.displayState    = GUIDisplayState_NotHidden;
  prvLabel.backgroundColor        = COLOR_APP_CH1;
  prvLabel.textColor[0]           = COLOR_BLACK;
  prvLabel.text[0]                = "1";
  prvLabel.font                   = &font_18pt_variableWidth;
  GUILabel_Init(&prvLabel);
  prvButton.object.id               = GUIButtonId_Channel1Top;
  prvButton.object.xPos             = 30;
  prvButton.object.yPos             = 0;
  prvButton.object.width            = 80;
  prvButton.object.height           = 40;
  prvButton.object.border           = GUIBorder_Bottom | GUIBorder_Right;
  prvButton.object.borderThickness  = 1;
  prvButton.object.borderColor      = COLOR_WHITE;
  prvButton.object.layer            = GUILayer_1;
  prvButton.object.displayState     = GUIDisplayState_NotHidden;
  prvButton.state1TextColor         = COLOR_APP_CH1;
  prvButton.state1BackgroundColor   = COLOR_BLACK;
  prvButton.state2TextColor         = prvButton.state1TextColor;
  prvButton.state2BackgroundColor   = prvButton.state1BackgroundColor;
  prvButton.pressedTextColor        = prvButton.state1TextColor;
  prvButton.pressedBackgroundColor  = COLOR_WHITE;
  prvButton.buttonState             = GUIButtonState_State1;
  prvButton.touchCallback           = prvTopAndSystemButtonCallback;
  prvButton.text[0]                 = "UART";
  prvButton.font                    = &font_18pt_variableWidth;
  GUIButton_Init(&prvButton);

  /* Channel 2 */
  prvLabel.object.id              = GUILabelId_Channel2Top;
  prvLabel.object.xPos            += xDiff;
  prvLabel.object.yPos            = 0;
  prvLabel.object.width           = 30;
  prvLabel.object.height          = 40;
  prvLabel.object.border          = GUIBorder_Left | GUIBorder_Bottom;
  prvLabel.object.borderThickness = 1;
  prvLabel.object.borderColor     = COLOR_WHITE;
  prvLabel.object.layer           = GUILayer_1;
  prvLabel.object.displayState    = GUIDisplayState_NotHidden;
  prvLabel.backgroundColor        = COLOR_APP_CH2;
  prvLabel.textColor[0]           = COLOR_BLACK;
  prvLabel.text[0]                = "2";
  prvLabel.font                   = &font_18pt_variableWidth;
  GUILabel_Init(&prvLabel);
  prvButton.object.id               = GUIButtonId_Channel2Top;
  prvButton.object.xPos             += xDiff;
  prvButton.object.yPos             = 0;
  prvButton.object.width            = 80;
  prvButton.object.height           = 40;
  prvButton.object.border           = GUIBorder_Bottom | GUIBorder_Right;
  prvButton.object.borderThickness  = 1;
  prvButton.object.borderColor      = COLOR_WHITE;
  prvButton.object.layer            = GUILayer_1;
  prvButton.object.displayState     = GUIDisplayState_NotHidden;
  prvButton.state1TextColor         = COLOR_APP_CH2;
  prvButton.state1BackgroundColor   = COLOR_BLACK;
  prvButton.state2TextColor         = prvButton.state1TextColor;
  prvButton.state2BackgroundColor   = prvButton.state1BackgroundColor;
  prvButton.pressedTextColor        = prvButton.state1TextColor;
  prvButton.pressedBackgroundColor  = COLOR_WHITE;
  prvButton.buttonState             = GUIButtonState_State1;
  prvButton.touchCallback           = prvTopAndSystemButtonCallback;
  prvButton.text[0]                 = "RS-232";
  prvButton.font                    = &font_18pt_variableWidth;
  GUIButton_Init(&prvButton);

  /* Channel 3 */
  prvLabel.object.id              = GUILabelId_Channel3Top;
  prvLabel.object.xPos            += xDiff;
  prvLabel.object.yPos            = 0;
  prvLabel.object.width           = 30;
  prvLabel.object.height          = 40;
  prvLabel.object.border          = GUIBorder_Left | GUIBorder_Bottom;
  prvLabel.object.borderThickness = 1;
  prvLabel.object.borderColor     = COLOR_WHITE;
  prvLabel.object.layer           = GUILayer_1;
  prvLabel.object.displayState    = GUIDisplayState_NotHidden;
  prvLabel.backgroundColor        = COLOR_APP_CH3;
  prvLabel.textColor[0]           = COLOR_BLACK;
  prvLabel.text[0]                = "3";
  prvLabel.font                   = &font_18pt_variableWidth;
  GUILabel_Init(&prvLabel);
  prvButton.object.id               = GUIButtonId_Channel3Top;
  prvButton.object.xPos             += xDiff;
  prvButton.object.yPos             = 0;
  prvButton.object.width            = 80;
  prvButton.object.height           = 40;
  prvButton.object.border           = GUIBorder_Bottom | GUIBorder_Right;
  prvButton.object.borderThickness  = 1;
  prvButton.object.borderColor      = COLOR_WHITE;
  prvButton.object.layer            = GUILayer_1;
  prvButton.object.displayState     = GUIDisplayState_NotHidden;
  prvButton.state1TextColor         = COLOR_APP_CH3;
  prvButton.state1BackgroundColor   = COLOR_BLACK;
  prvButton.state2TextColor         = prvButton.state1TextColor;
  prvButton.state2BackgroundColor   = prvButton.state1BackgroundColor;
  prvButton.pressedTextColor        = prvButton.state1TextColor;
  prvButton.pressedBackgroundColor  = COLOR_WHITE;
  prvButton.buttonState             = GUIButtonState_State1;
  prvButton.touchCallback           = prvTopAndSystemButtonCallback;
  prvButton.text[0]                 = "GPIO";
  prvButton.font                    = &font_18pt_variableWidth;
  GUIButton_Init(&prvButton);

  /* Channel 4 */
  prvLabel.object.id              = GUILabelId_Channel4Top;
  prvLabel.object.xPos            += xDiff;
  prvLabel.object.yPos            = 0;
  prvLabel.object.width           = 30;
  prvLabel.object.height          = 40;
  prvLabel.object.border          = GUIBorder_Left | GUIBorder_Bottom;
  prvLabel.object.borderThickness = 1;
  prvLabel.object.borderColor     = COLOR_WHITE;
  prvLabel.object.layer           = GUILayer_1;
  prvLabel.object.displayState    = GUIDisplayState_NotHidden;
  prvLabel.backgroundColor        = COLOR_APP_CH4;
  prvLabel.textColor[0]           = COLOR_BLACK;
  prvLabel.text[0]                = "4";
  prvLabel.font                   = &font_18pt_variableWidth;
  GUILabel_Init(&prvLabel);
  prvButton.object.id               = GUIButtonId_Channel4Top;
  prvButton.object.xPos             += xDiff;
  prvButton.object.yPos             = 0;
  prvButton.object.width            = 80;
  prvButton.object.height           = 40;
  prvButton.object.border           = GUIBorder_Bottom | GUIBorder_Right;
  prvButton.object.borderThickness  = 1;
  prvButton.object.borderColor      = COLOR_WHITE;
  prvButton.object.layer            = GUILayer_1;
  prvButton.object.displayState     = GUIDisplayState_NotHidden;
  prvButton.state1TextColor         = COLOR_APP_CH4;
  prvButton.state1BackgroundColor   = COLOR_BLACK;
  prvButton.state2TextColor         = prvButton.state1TextColor;
  prvButton.state2BackgroundColor   = prvButton.state1BackgroundColor;
  prvButton.pressedTextColor        = prvButton.state1TextColor;
  prvButton.pressedBackgroundColor  = COLOR_WHITE;
  prvButton.buttonState             = GUIButtonState_State1;
  prvButton.touchCallback           = prvTopAndSystemButtonCallback;
  prvButton.text[0]                 = "Setup!";
  prvButton.font                    = &font_18pt_variableWidth;
  GUIButton_Init(&prvButton);

  /* Channel 5 */
  prvLabel.object.id              = GUILabelId_Channel5Top;
  prvLabel.object.xPos            += xDiff;
  prvLabel.object.yPos            = 0;
  prvLabel.object.width           = 30;
  prvLabel.object.height          = 40;
  prvLabel.object.border          = GUIBorder_Left | GUIBorder_Bottom;
  prvLabel.object.borderThickness = 1;
  prvLabel.object.borderColor     = COLOR_WHITE;
  prvLabel.object.layer           = GUILayer_1;
  prvLabel.object.displayState    = GUIDisplayState_NotHidden;
  prvLabel.backgroundColor        = COLOR_APP_CH5;
  prvLabel.textColor[0]           = COLOR_BLACK;
  prvLabel.text[0]                = "5";
  prvLabel.font                   = &font_18pt_variableWidth;
  GUILabel_Init(&prvLabel);
  prvButton.object.id               = GUIButtonId_Channel5Top;
  prvButton.object.xPos             += xDiff;
  prvButton.object.yPos             = 0;
  prvButton.object.width            = 80;
  prvButton.object.height           = 40;
  prvButton.object.border           = GUIBorder_Bottom | GUIBorder_Right;
  prvButton.object.borderThickness  = 1;
  prvButton.object.borderColor      = COLOR_WHITE;
  prvButton.object.layer            = GUILayer_1;
  prvButton.object.displayState     = GUIDisplayState_NotHidden;
  prvButton.state1TextColor         = COLOR_APP_CH5;
  prvButton.state1BackgroundColor   = COLOR_BLACK;
  prvButton.state2TextColor         = prvButton.state1TextColor;
  prvButton.state2BackgroundColor   = prvButton.state1BackgroundColor;
  prvButton.pressedTextColor        = prvButton.state1TextColor;
  prvButton.pressedBackgroundColor  = COLOR_WHITE;
  prvButton.buttonState             = GUIButtonState_State1;
  prvButton.touchCallback           = prvTopAndSystemButtonCallback;
  prvButton.text[0]                 = "CAN";
  prvButton.font                    = &font_18pt_variableWidth;
  GUIButton_Init(&prvButton);

  /* Channel 6 */
  prvLabel.object.id              = GUILabelId_Channel6Top;
  prvLabel.object.xPos            += xDiff;
  prvLabel.object.yPos            = 0;
  prvLabel.object.width           = 30;
  prvLabel.object.height          = 40;
  prvLabel.object.border          = GUIBorder_Left | GUIBorder_Bottom;
  prvLabel.object.borderThickness = 1;
  prvLabel.object.borderColor     = COLOR_WHITE;
  prvLabel.object.layer           = GUILayer_1;
  prvLabel.object.displayState    = GUIDisplayState_NotHidden;
  prvLabel.backgroundColor        = COLOR_APP_CH6;
  prvLabel.textColor[0]           = COLOR_BLACK;
  prvLabel.text[0]                = "6";
  prvLabel.font                   = &font_18pt_variableWidth;
  GUILabel_Init(&prvLabel);
  prvButton.object.id               = GUIButtonId_Channel6Top;
  prvButton.object.xPos             += xDiff;
  prvButton.object.yPos             = 0;
  prvButton.object.width            = 80;
  prvButton.object.height           = 40;
  prvButton.object.border           = GUIBorder_Bottom | GUIBorder_Right;
  prvButton.object.borderThickness  = 1;
  prvButton.object.borderColor      = COLOR_WHITE;
  prvButton.object.layer            = GUILayer_1;
  prvButton.object.displayState     = GUIDisplayState_NotHidden;
  prvButton.state1TextColor         = COLOR_APP_CH6;
  prvButton.state1BackgroundColor   = COLOR_BLACK;
  prvButton.state2TextColor         = prvButton.state1TextColor;
  prvButton.state2BackgroundColor   = prvButton.state1BackgroundColor;
  prvButton.pressedTextColor        = prvButton.state1TextColor;
  prvButton.pressedBackgroundColor  = COLOR_WHITE;
  prvButton.buttonState             = GUIButtonState_State1;
  prvButton.touchCallback           = prvTopAndSystemButtonCallback;
  prvButton.text[0]                 = "N/A";
  prvButton.font                    = &font_18pt_variableWidth;
  GUIButton_Init(&prvButton);

  /* System */
  prvButton.object.id               = GUIButtonId_System;
  prvButton.object.xPos             = 660;
  prvButton.object.yPos             = 440;
  prvButton.object.width            = 140;
  prvButton.object.height           = 40;
  prvButton.object.border           = GUIBorder_Left | GUIBorder_Top;
  prvButton.object.borderThickness  = 1;
  prvButton.object.borderColor      = COLOR_WHITE;
  prvButton.object.layer            = GUILayer_1;
  prvButton.object.displayState     = GUIDisplayState_NotHidden;
  prvButton.state1TextColor         = COLOR_WHITE;
  prvButton.state1BackgroundColor   = COLOR_APP_SYSTEM;
  prvButton.state2TextColor         = prvButton.state1TextColor;
  prvButton.state2BackgroundColor   = prvButton.state1BackgroundColor;
  prvButton.pressedTextColor        = COLOR_APP_SYSTEM;
  prvButton.pressedBackgroundColor  = COLOR_WHITE;
  prvButton.buttonState             = GUIButtonState_State1;
  prvButton.touchCallback           = prvTopAndSystemButtonCallback;
  prvButton.text[0]                 = "System";
  prvButton.font                    = &font_18pt_variableWidth;
  GUIButton_Init(&prvButton);
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void prvTopAndSystemButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
  /* If a button was pressed we should change the sidebar */
  if (Event == GUITouchEvent_Up)
  {
    switch (ButtonId)
    {
      case GUIButtonId_Channel1Top:
        setActiveSidebar(APP_ActiveSidebar_1);
        break;
      case GUIButtonId_Channel2Top:
        setActiveSidebar(APP_ActiveSidebar_2);
        break;
      case GUIButtonId_Channel3Top:
        setActiveSidebar(APP_ActiveSidebar_3);
        break;
      case GUIButtonId_Channel4Top:
        setActiveSidebar(APP_ActiveSidebar_4);
        break;
      case GUIButtonId_Channel5Top:
        setActiveSidebar(APP_ActiveSidebar_5);
        break;
      case GUIButtonId_Channel6Top:
        setActiveSidebar(APP_ActiveSidebar_6);
        break;
      case GUIButtonId_System:
        setActiveSidebar(APP_ActiveSidebar_System);
        break;
      default:
        break;
    }
  }
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void prvInitSidebarItems()
{
  /* Add the sidebar as a NA sidebar to start with */
  memcpy(&prvTempButtonList, &NA_SidebarTemplate, sizeof(GUIButtonList));
  prvTempButtonList.object.id = GUIButtonListId_Sidebar;
  GUIButtonList_Init(&prvTempButtonList);

  /* Set the active channel to channel 1 */
  /* TODO: Read last active channel from eeprom/flash */
  setActiveSidebar(APP_ActiveSidebar_1);
}

/** Interrupt Handlers -------------------------------------------------------*/
