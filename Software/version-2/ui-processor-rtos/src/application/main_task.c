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

#include "i2c_eeprom.h"
#include "spi_comm.h"
#include "gui_clock.h"
#include "buzzer.h"

/** Private defines ----------------------------------------------------------*/
/** Private typedefs ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
static APP_ActiveSidebar prvCurrentlyActiveSidebar = APP_ActiveSidebar_None;
static uint8_t prvCurrentlyActiveChannel = 1;
static const uint8_t prvChannelNumberFromActiveSidebar[8] = {1, 2, 3, 4, 5, 6, 0, 0};

/* Buzzer */
static APP_BuzzerSound prvCurrentBuzzerSound = APP_BuzzerSound_Off;
static uint8_t prvVolumeForBuzzerSetting[4] = {0, 1, 4, 6};
#define EEPROM_BUZZER_SETTING_ADDRESS   (0x004C)


/* Power Source */
static APP_PowerSource prvCurrentPowerSource = APP_PowerSource_ExternalModule; /* TODO: one for each channel */
/* Direction data */
static APP_ChannelDirection prvCurrentChannelDirection[2] = {APP_ChannelDirection_Off, APP_ChannelDirection_Off}; /* TODO: one for each channel */
static char prvCurrentDirectionString[16] = {0};  /* Longest string is "A: Off - B: Off" -> 15 characters */
/* Parity */
static APP_Parity prvCurrentParity = APP_Parity_None;   /* TODO: one for each channel */
static char* prvCurrentParityString; /* TODO: one for each channel */

static bool prvChannelIsEnabled[6]              = {false, false, false, false, false, false};
static bool prvChannelSplitscreenEnabled[6]     = {false, false, false, false, false, false};
static bool prvModulePowerEnabled[6]            = {false, false, false, false, false, false};
static bool prvChannelCanTerminationEnabled[6]  = {false, false, false, false, false, false};
static bool prvSetupChannel[6]                  = {false, false, false, false, false, false};

/* Channel ID & Type */
static uint8_t prvChannelID[6]                  = {0, 0, 0, 0, 0, 0};
static APP_ChannelType prvChannelType[6]        = {APP_ChannelType_NA};
static char* prvNameForChannelType[6]           = {"N/A", "Setup!", "UART", "GPIO", "CAN", "RS-232"};
static char* prvChannelIdString[]               = {"0 (None)", "1 (GPIO)", "2 (Invalid)", "3 (CAN)", "4 (Invalid)", "5 (RS-232)"};
static char* prvChannelNumberString[6]          = {"1", "2", "3", "4", "5", "6"};
#define EEPROM_CHANNEL_ID_START_ADDRESS         (0x0040)
#define EEPROM_CHANNEL_TYPE_START_ADDRESS       (0x0046)

static uint16_t prvSidebarActivePage[8]     = {0, 0, 0, 0, 0, 0, 0, 0};
static bool prvEnablePromptEnabled          = true;

static GUIButton prvButton;
static GUILabel prvLabel;
//static GUIScrollableTextBox prvScrollableTextBox;
static GUIButtonList prvTempButtonList;
static GUIAlertBox prvTempAlertBox;
static GUIButtonGridBox prvTempButtonGridBox;
static GUIInfoBox prvTempInfoBox;

//static uint32_t prvCurrentLogAddress = 0;

static bool prvLcdTaskIsDone = false;

/* GUI Clock Refresh Timer */
static xTimerHandle prvGUIClockRefreshTimer;

/** Private function prototypes ----------------------------------------------*/
static void prvHardwareInit();

static void setActiveColorsForAlertBox(uint32_t Id);
static void setActiveColorsForButtonGridBox(uint32_t Id);

static void updateParityString();
static void updateDirectionString();

static void prvSetActiveSidebar(APP_ActiveSidebar NewActiveChannel, bool ForceSet);
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

static void manangeModuleCaptureButtonPress(uint32_t CaptureButtonIndex, uint32_t PowerButtonIndex);
static void manangeModulePowerButtonPress(uint32_t ButtonIndex);

/* Alert box callbacks */
static void confirmIdAlertBoxCallback(GUIAlertBoxCallbackButton CallbackButton);
static void refreshIdsAlertBoxCallback(GUIAlertBoxCallbackButton CallbackButton);
static void clearAllMemoryAlertBoxCallback(GUIAlertBoxCallbackButton CallbackButton);

/* Button grid callbacks */
static void buttonInParityBoxPressed(uint32_t Row, uint32_t Column);
static void buttonInDirectionBoxPressed(uint32_t Row, uint32_t Column);
static void buttonInModuleModeBoxPressed(uint32_t Row, uint32_t Column);

/* GUI init functions */
static uint32_t prvColorForChannel(uint8_t Channel);
static void prvInitTopForChannel(uint8_t Channel);
static void prvInitTopAndSystemItems();
static void prvTopAndSystemButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
static void prvInitSidebarItems();

/* Channel ID and Type */
static void prvInitChannelIdsAndTypes();
static void prvInitChannelTypeForChannel(uint8_t Channel);
static void prvSaveChannelIdToEeprom(uint8_t Channel);
static void prvSaveChannelTypeToEeprom(uint8_t Channel);

/** Functions ----------------------------------------------------------------*/
/**
  * @brief  Text
  * @param  None
  * @retval None
  */
void mainTask(void *pvParameters)
{
  /* The parameter in vTaskDelayUntil is the absolute time
   * in ticks at which you want to be woken calculated as
   * an increment from the time you were last woken. */
  TickType_t xNextWakeTime;
  /* Initialize xNextWakeTime - this only needs to be done once. */
  xNextWakeTime = xTaskGetTickCount();


  /** Initialize the hardware */
  prvHardwareInit();

  /** Initialize the clock */
  GUI_CLOCK_InitClock();

  /** Init the SPI Communication with the FPGA Data Processor */
  SPI_COMM_Init();

  /** Init the I2C EEPROM */
  I2C_EEPROM_Init();

  /** Wait until LCD task is done with init */
  while (!prvLcdTaskIsDone)
  {
    vTaskDelayUntil(&xNextWakeTime, 100 / portTICK_PERIOD_MS);
  }

  /** Wait until FPGA is done, TODO: Timeout */
  while (SPI_COMM_GetStatus() != 0x01)
    vTaskDelayUntil(&xNextWakeTime, 100 / portTICK_PERIOD_MS);

//  /* Read Channel Power */
//  uint8_t currentPower = 0;
//  if (SPI_COMM_GetPowerForAllChannels(&currentPower) == SUCCESS)
//  {
//    prvModulePowerEnabled[0] = currentPower & 0x1;
//    prvModulePowerEnabled[1] = currentPower & 0x2;
//    prvModulePowerEnabled[2] = currentPower & 0x4;
//    prvModulePowerEnabled[3] = currentPower & 0x8;
//    prvModulePowerEnabled[4] = currentPower & 0x10;
//    prvModulePowerEnabled[5] = currentPower & 0x20;
//  }

  /* Read Channel Output */
  uint8_t currentOutput = 0;
  if (SPI_COMM_GetOutputForAllChannels(&currentOutput) == SUCCESS)
  {
    prvChannelIsEnabled[0] = currentOutput & 0x1;
    prvChannelIsEnabled[1] = currentOutput & 0x2;
    prvChannelIsEnabled[2] = currentOutput & 0x4;
    prvChannelIsEnabled[3] = currentOutput & 0x8;
    prvChannelIsEnabled[4] = currentOutput & 0x10;
    prvChannelIsEnabled[5] = currentOutput & 0x20;
  }

  /* Read Channel Termination */
  uint8_t currentTermination = 0;
  if (SPI_COMM_GetTerminationForAllChannels(&currentTermination) == SUCCESS)
  {
    prvChannelCanTerminationEnabled[0] = currentTermination & 0x1;
    prvChannelCanTerminationEnabled[1] = currentTermination & 0x2;
    prvChannelCanTerminationEnabled[2] = currentTermination & 0x4;
    prvChannelCanTerminationEnabled[3] = currentTermination & 0x8;
    prvChannelCanTerminationEnabled[4] = currentTermination & 0x10;
    prvChannelCanTerminationEnabled[5] = currentTermination & 0x20;
  }

  /** Init the channel IDs and Types */
  prvInitChannelIdsAndTypes();

  /** # Init GUI */
  GUI_Init();

  /** # Fill the first layer black */
  GUI_ClearLayer(COLOR_BLACK, GUILayer_1);


  /** Add a Parity Button Grid Box and set it for channel 1 to start with */
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

  /** Add a Direction Button Grid Box and set it for channel 1 to start with */
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

  /** Add a Module Mode Button Grid Box and set it for channel 1 to start with */
  GUIButtonGridBox_Reset(&prvTempButtonGridBox);
  memcpy(&prvTempButtonGridBox, &ModuleModeButtonGridBoxTemplate, sizeof(GUIButtonGridBox));
  prvTempButtonGridBox.object.id                      = GUIButtonGridBoxId_ModuleModeSelection;
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
  prvTempButtonGridBox.actionButtonPressed            = buttonInModuleModeBoxPressed;
  GUIButtonGridBox_Init(&prvTempButtonGridBox);


  /** Add the Confirm ID alert box */
  memcpy(&prvTempAlertBox, &ConfirmIdAlertBoxTemplate, sizeof(GUIAlertBox));
  prvTempAlertBox.object.id           = GUIAlertBoxId_ConfirmId;
  prvTempAlertBox.actionButtonPressed = confirmIdAlertBoxCallback;
  GUIAlertBox_Init(&prvTempAlertBox);

  /** System ================================================================ */
  /** Add the Version Info GUIInfoBox */
  memcpy(&prvTempInfoBox, &VersionInfoBoxTemplate, sizeof(GUIInfoBox));
  GUIInfoBox_Init(&prvTempInfoBox);

  /** Add the Refresh IDs alert box */
  memcpy(&prvTempAlertBox, &RefreshIdsAlertBoxTemplate, sizeof(GUIAlertBox));
  prvTempAlertBox.object.id           = GUIAlertBoxId_RefreshIds;
  prvTempAlertBox.actionButtonPressed = refreshIdsAlertBoxCallback;
  GUIAlertBox_Init(&prvTempAlertBox);

  /** Add the Clear All Memory alert box */
  memcpy(&prvTempAlertBox, &ClearAllMemoryAlertBoxTemplate, sizeof(GUIAlertBox));
  prvTempAlertBox.object.id           = GUIAlertBoxId_ClearAllMemory;
  prvTempAlertBox.actionButtonPressed = clearAllMemoryAlertBoxCallback;
  GUIAlertBox_Init(&prvTempAlertBox);


  /** Top Labels and Buttons */
  prvInitTopAndSystemItems();

  /** Sidebar */
  prvInitSidebarItems();

  /** Initialize the buzzer */
  BUZZER_Init();
  uint8_t temp = I2C_EEPROM_ReadByte(EEPROM_BUZZER_SETTING_ADDRESS);
  if (IS_BUZZER_SOUND(temp))
    prvCurrentBuzzerSound = temp;
  else
    I2C_EEPROM_WriteByte(EEPROM_BUZZER_SETTING_ADDRESS, (uint8_t)prvCurrentBuzzerSound);
  BUZZER_SetVolume(prvVolumeForBuzzerSetting[prvCurrentBuzzerSound]);

  /** Enable refresh now that we are ready */
  GUI_EnableRefresh();


  while (1)
  {
    for (uint32_t i = 0; i < 256; i++)
    {
//      SPI_COMM_SendCommand((uint8_t)i, 0, 0);
      vTaskDelayUntil(&xNextWakeTime, 1000 / portTICK_PERIOD_MS);
    }

//    vTaskDelayUntil(&xNextWakeTime, 5000 / portTICK_PERIOD_MS);
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
    pTempButtonGridBox->backgroundColor               = COLOR_APP_CH1;
    pTempButtonGridBox->titleTextColor                = COLOR_APP_CH1;
    pTempButtonGridBox->labelsBackgroundColor         = COLOR_APP_CH1;
    pTempButtonGridBox->buttonsState1TextColor        = COLOR_APP_CH1;
    pTempButtonGridBox->buttonsState2BackgroundColor  = COLOR_APP_CH1_DARK;
    pTempButtonGridBox->buttonsPressedBackgroundColor = COLOR_APP_CH1;
  }
  /* Channel 2 */
  else if (prvCurrentlyActiveSidebar == APP_ActiveSidebar_2)
  {
    pTempButtonGridBox->backgroundColor               = COLOR_APP_CH2;
    pTempButtonGridBox->titleTextColor                = COLOR_APP_CH2;
    pTempButtonGridBox->labelsBackgroundColor         = COLOR_APP_CH2;
    pTempButtonGridBox->buttonsState1TextColor        = COLOR_APP_CH2;
    pTempButtonGridBox->buttonsState2BackgroundColor  = COLOR_APP_CH2_DARK;
    pTempButtonGridBox->buttonsPressedBackgroundColor = COLOR_APP_CH2;
  }
  /* Channel 3 */
  else if (prvCurrentlyActiveSidebar == APP_ActiveSidebar_3)
  {
    pTempButtonGridBox->backgroundColor               = COLOR_APP_CH3;
    pTempButtonGridBox->titleTextColor                = COLOR_APP_CH3;
    pTempButtonGridBox->labelsBackgroundColor         = COLOR_APP_CH3;
    pTempButtonGridBox->buttonsState1TextColor        = COLOR_APP_CH3;
    pTempButtonGridBox->buttonsState2BackgroundColor  = COLOR_APP_CH3_DARK;
    pTempButtonGridBox->buttonsPressedBackgroundColor = COLOR_APP_CH3;
  }
  /* Channel 4 */
  else if (prvCurrentlyActiveSidebar == APP_ActiveSidebar_4)
  {
    pTempButtonGridBox->backgroundColor               = COLOR_APP_CH4;
    pTempButtonGridBox->titleTextColor                = COLOR_APP_CH4;
    pTempButtonGridBox->labelsBackgroundColor         = COLOR_APP_CH4;
    pTempButtonGridBox->buttonsState1TextColor        = COLOR_APP_CH4;
    pTempButtonGridBox->buttonsState2BackgroundColor  = COLOR_APP_CH4_DARK;
    pTempButtonGridBox->buttonsPressedBackgroundColor = COLOR_APP_CH4;
  }
  /* Channel 5 */
  else if (prvCurrentlyActiveSidebar == APP_ActiveSidebar_5)
  {
    pTempButtonGridBox->backgroundColor               = COLOR_APP_CH5;
    pTempButtonGridBox->titleTextColor                = COLOR_APP_CH5;
    pTempButtonGridBox->labelsBackgroundColor         = COLOR_APP_CH5;
    pTempButtonGridBox->buttonsState1TextColor        = COLOR_APP_CH5;
    pTempButtonGridBox->buttonsState2BackgroundColor  = COLOR_APP_CH5_DARK;
    pTempButtonGridBox->buttonsPressedBackgroundColor = COLOR_APP_CH5;
  }
  /* Channel 6 */
  else if (prvCurrentlyActiveSidebar == APP_ActiveSidebar_6)
  {
    pTempButtonGridBox->backgroundColor               = COLOR_APP_CH6;
    pTempButtonGridBox->titleTextColor                = COLOR_APP_CH6;
    pTempButtonGridBox->labelsBackgroundColor         = COLOR_APP_CH6;
    pTempButtonGridBox->buttonsState1TextColor        = COLOR_APP_CH6;
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
static void prvSetActiveSidebar(APP_ActiveSidebar NewActiveChannel, bool ForceSet)
{
  if (IS_APP_ACTIVE_SIDEBAR(NewActiveChannel) && (ForceSet == true || NewActiveChannel != prvCurrentlyActiveSidebar))
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
      prvCurrentlyActiveChannel = 1;
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
      prvCurrentlyActiveChannel = 2;
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
      prvCurrentlyActiveChannel = 3;
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
      prvCurrentlyActiveChannel = 4;
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
      prvCurrentlyActiveChannel = 5;
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
      prvCurrentlyActiveChannel = 6;
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

  /* Module ID */
  prvTempButtonList.buttonText[NA_ID_INDEX][1] = prvChannelIdString[prvChannelID[prvCurrentlyActiveSidebar]];
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

  /* Module ID */
  prvTempButtonList.buttonText[SETUP_ID_INDEX][1] = prvChannelIdString[prvChannelID[prvCurrentlyActiveSidebar]];

  /* Confirm ID alert box */
  setActiveColorsForAlertBox(GUIAlertBoxId_ConfirmId);
  GUIAlertBox_Draw(GUIAlertBoxId_ConfirmId);
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void setUARTSidebarItems()
{
  /* Channel status */
  if (prvChannelIsEnabled[prvCurrentlyActiveSidebar] == true)
    prvTempButtonList.buttonText[UART_CAPTURE_INDEX][0] = "Stop Capture";
  else
    prvTempButtonList.buttonText[UART_CAPTURE_INDEX][0] = "Start Capture";

  /* Parity */
  updateParityString();
  prvTempButtonList.buttonText[UART_PARITY_INDEX][1] = prvCurrentParityString;

  /* Splitscreen */
  if (prvModulePowerEnabled[prvCurrentlyActiveSidebar] == true)
    prvTempButtonList.buttonText[UART_SPLITSCREEN_INDEX][1] = "Enabled";
  else
    prvTempButtonList.buttonText[UART_SPLITSCREEN_INDEX][1] = "Disabled";

  /* Direction */
  updateDirectionString();
  prvTempButtonList.buttonText[UART_CHANNEL_MODE_INDEX][1] = prvCurrentDirectionString;

  /* Module Power */
  if (prvModulePowerEnabled[prvCurrentlyActiveSidebar] == true)
    prvTempButtonList.buttonText[UART_MODULE_POWER_INDEX][1] = "Enabled";
  else
    prvTempButtonList.buttonText[UART_MODULE_POWER_INDEX][1] = "Disabled";

  /* Module ID */
  prvTempButtonList.buttonText[UART_ID_INDEX][1] = prvChannelIdString[prvChannelID[prvCurrentlyActiveSidebar]];

  /* Module Mode */
  prvTempButtonList.buttonText[UART_MODULE_MODE_INDEX][1] = "UART";
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void setGPIOSidebarItems()
{
  /* Module Power */
  if (prvModulePowerEnabled[prvCurrentlyActiveSidebar] == true)
    prvTempButtonList.buttonText[GPIO_MODULE_POWER_INDEX][1] = "Enabled";
  else
    prvTempButtonList.buttonText[GPIO_MODULE_POWER_INDEX][1] = "Disabled";

  /* Module ID */
  prvTempButtonList.buttonText[GPIO_ID_INDEX][1] = prvChannelIdString[prvChannelID[prvCurrentlyActiveSidebar]];

  /* Module Mode */
  prvTempButtonList.buttonText[UART_MODULE_MODE_INDEX][1] = "GPIO";
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void setCANSidebarItems()
{
  /* Channel status */
  if (prvChannelIsEnabled[prvCurrentlyActiveSidebar] == true)
    prvTempButtonList.buttonText[CAN_CAPTURE_INDEX][0] = "Stop Capture";
  else
    prvTempButtonList.buttonText[CAN_CAPTURE_INDEX][0] = "Start Capture";

  /* Module Power */
  if (prvModulePowerEnabled[prvCurrentlyActiveSidebar] == true)
    prvTempButtonList.buttonText[CAN_MODULE_POWER_INDEX][1] = "Enabled";
  else
    prvTempButtonList.buttonText[CAN_MODULE_POWER_INDEX][1] = "Disabled";

  /* Module ID */
  prvTempButtonList.buttonText[CAN_ID_INDEX][1] = prvChannelIdString[prvChannelID[prvCurrentlyActiveSidebar]];
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void setRS232SidebarItems()
{
  /* Channel status */
  if (prvChannelIsEnabled[prvCurrentlyActiveSidebar] == true)
    prvTempButtonList.buttonText[RS_232_CAPTURE_INDEX][0] = "Stop Capture";
  else
    prvTempButtonList.buttonText[RS_232_CAPTURE_INDEX][0] = "Start Capture";

  /* Parity */
  updateParityString();
  prvTempButtonList.buttonText[UART_PARITY_INDEX][1] = prvCurrentParityString;

  /* Splitscreen */
  if (prvChannelIsEnabled[prvCurrentlyActiveSidebar] == true)
    prvTempButtonList.buttonText[RS_232_SPLITSCREEN_INDEX][1] = "Enabled";
  else
    prvTempButtonList.buttonText[RS_232_SPLITSCREEN_INDEX][1] = "Disabled";

  /* Direction */
  updateDirectionString();
  prvTempButtonList.buttonText[UART_CHANNEL_MODE_INDEX][1] = prvCurrentDirectionString;

  /* Module Power */
  if (prvModulePowerEnabled[prvCurrentlyActiveSidebar] == true)
    prvTempButtonList.buttonText[RS_232_MODULE_POWER_INDEX][1] = "Enabled";
  else
    prvTempButtonList.buttonText[RS_232_MODULE_POWER_INDEX][1] = "Disabled";

  /* Module ID */
  prvTempButtonList.buttonText[RS_232_ID_INDEX][1] = prvChannelIdString[prvChannelID[prvCurrentlyActiveSidebar]];
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

    /* Save to EEPROM */
    I2C_EEPROM_WriteByte(EEPROM_BUZZER_SETTING_ADDRESS, (uint8_t)prvCurrentBuzzerSound);

    /* Set the volume of the buzzer */
    BUZZER_SetVolume(prvVolumeForBuzzerSetting[prvCurrentBuzzerSound]);
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
  if (ButtonIndex == NA_REFRESH_ID_INDEX)
  {

  }
  /* Module Power */
  else if (ButtonIndex == NA_MODULE_POWER_INDEX)
  {
    manangeModulePowerButtonPress(ButtonIndex);
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
    manangeModulePowerButtonPress(ButtonIndex);
  }
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void buttonPressedInUARTSidebar(uint32_t ButtonIndex)
{
  /* Start/Stop Capture */
  if (ButtonIndex == UART_CAPTURE_INDEX)
  {
    manangeModuleCaptureButtonPress(ButtonIndex, UART_MODULE_POWER_INDEX);
  }
  /* Baud rate button grid box */
  else if (ButtonIndex == UART_BAUD_RATE_INDEX)
  {

  }
  /* Parity button grid box */
  else if (ButtonIndex == UART_PARITY_INDEX)
  {
    /* TODO: Read the current setting and set the state of the parity box */
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
    setActiveColorsForButtonGridBox(GUIButtonGridBoxId_ModuleModeSelection);
    GUIButtonGridBox_SetButtonState(GUIButtonGridBoxId_ModuleModeSelection, 0, MODULE_MODE_BUTTON_GRID_BOX_GPIO_COLUMN, GUIButtonState_State1, false);
    GUIButtonGridBox_SetButtonState(GUIButtonGridBoxId_ModuleModeSelection, 0, MODULE_MODE_BUTTON_GRID_BOX_UART_COLUMN, GUIButtonState_State2, false);
    GUIButtonGridBox_Draw(GUIButtonGridBoxId_ModuleModeSelection);
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
    manangeModulePowerButtonPress(ButtonIndex);
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
    setActiveColorsForButtonGridBox(GUIButtonGridBoxId_ModuleModeSelection);
    GUIButtonGridBox_SetButtonState(GUIButtonGridBoxId_ModuleModeSelection, 0, MODULE_MODE_BUTTON_GRID_BOX_GPIO_COLUMN, GUIButtonState_State2, false);
    GUIButtonGridBox_SetButtonState(GUIButtonGridBoxId_ModuleModeSelection, 0, MODULE_MODE_BUTTON_GRID_BOX_UART_COLUMN, GUIButtonState_State1, false);
    GUIButtonGridBox_Draw(GUIButtonGridBoxId_ModuleModeSelection);
  }
  /* Refresh ID */
  else if (ButtonIndex == GPIO_REFRESH_ID_INDEX)
  {

  }
  /* Module Power */
  else if (ButtonIndex == GPIO_MODULE_POWER_INDEX)
  {
    manangeModulePowerButtonPress(ButtonIndex);
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
    manangeModuleCaptureButtonPress(ButtonIndex, CAN_MODULE_POWER_INDEX);
  }
  /* Bit rate button grid box */
  else if (ButtonIndex == CAN_BIT_RATE_INDEX)
  {

  }
  /* Termination */
  else if (ButtonIndex == CAN_TERMINATION_INDEX)
  {
    /* TODO: Read back and make sure that the termination is actually enabled, don't assume */
    if (prvChannelCanTerminationEnabled[prvCurrentlyActiveSidebar] == true)
    {
      SPI_COMM_DisableTerminationForChannel(prvChannelNumberFromActiveSidebar[prvCurrentlyActiveSidebar]);
      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, CAN_TERMINATION_INDEX, 0, "Disabled");
      prvChannelCanTerminationEnabled[prvCurrentlyActiveSidebar] = false;
    }
    else
    {
      SPI_COMM_EnableTerminationForChannel(prvChannelNumberFromActiveSidebar[prvCurrentlyActiveSidebar]);
      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, CAN_TERMINATION_INDEX, 0, "Enabled");
      prvChannelCanTerminationEnabled[prvCurrentlyActiveSidebar] = true;
    }
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
    manangeModulePowerButtonPress(ButtonIndex);
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
    manangeModuleCaptureButtonPress(ButtonIndex, RS_232_MODULE_POWER_INDEX);
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
    manangeModulePowerButtonPress(ButtonIndex);
  }
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void manangeModuleCaptureButtonPress(uint32_t CaptureButtonIndex, uint32_t PowerButtonIndex)
{
  uint8_t temp = 0;
  if (prvChannelIsEnabled[prvCurrentlyActiveChannel - 1] == true)
  {
    SPI_COMM_DisableOutputForChannel(prvCurrentlyActiveChannel);
    /* Check to make sure the output was actually disabled */
    if (SPI_COMM_GetOutputForAllChannels(&temp) == SUCCESS && (temp & (1 << (prvCurrentlyActiveChannel - 1))) == 0)
    {
      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, CaptureButtonIndex, "Start Capture", 0);
      prvChannelIsEnabled[prvCurrentlyActiveChannel - 1] = false;
    }
  }
  else
  {
    /* If the power is not enabled, enable it first */
    if (prvModulePowerEnabled[prvCurrentlyActiveChannel - 1] == false)
    {
      SPI_COMM_EnablePowerForChannel(prvCurrentlyActiveChannel);
      /* Check to make sure the power was actually enabled */
      if (SPI_COMM_GetPowerForAllChannels(&temp) == SUCCESS && (temp & (1 << (prvCurrentlyActiveChannel - 1))) != 0)
      {
        GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, PowerButtonIndex, 0, "Enabled");
        prvModulePowerEnabled[prvCurrentlyActiveChannel - 1] = true;
      }
      else
         return;
    }

    SPI_COMM_EnableOutputForChannel(prvCurrentlyActiveChannel);
    /* Check to make sure the output was actually enabled */
    if (SPI_COMM_GetOutputForAllChannels(&temp) == SUCCESS && (temp & (1 << (prvCurrentlyActiveChannel - 1))) != 0)
    {
      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, CaptureButtonIndex, "Stop Capture", 0);
      prvChannelIsEnabled[prvCurrentlyActiveChannel - 1] = true;
    }
  }
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void manangeModulePowerButtonPress(uint32_t ButtonIndex)
{
  uint8_t temp = 0;
  if (prvModulePowerEnabled[prvCurrentlyActiveChannel - 1] == true)
  {
    SPI_COMM_DisablePowerForChannel(prvCurrentlyActiveChannel);
    /* Check to make sure the power was actually disabled */
    if (SPI_COMM_GetPowerForAllChannels(&temp) == SUCCESS && (temp & (1 << (prvCurrentlyActiveChannel - 1))) == 0)
    {
      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, ButtonIndex, 0, "Disabled");
      prvModulePowerEnabled[prvCurrentlyActiveChannel - 1] = false;
    }
  }
  else
  {
    SPI_COMM_EnablePowerForChannel(prvCurrentlyActiveChannel);
    /* Check to make sure the power was actually enabled */
    if (SPI_COMM_GetPowerForAllChannels(&temp) == SUCCESS && (temp & (1 << (prvCurrentlyActiveChannel - 1))) != 0)
    {
      GUIButtonList_SetTextForButton(GUIButtonListId_Sidebar, ButtonIndex, 0, "Enabled");
      prvModulePowerEnabled[prvCurrentlyActiveChannel - 1] = true;
    }
  }
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void confirmIdAlertBoxCallback(GUIAlertBoxCallbackButton CallbackButton)
{
  if (CallbackButton == GUIAlertBoxCallbackButton_Left)
  {
    /* If the left button was pressed the ID is confirmed */
    prvSetupChannel[prvCurrentlyActiveChannel-1] = false;
    /* Save the confirmed ID to EEPROM */
    prvSaveChannelIdToEeprom(prvCurrentlyActiveChannel);
    /* Update the channel type */
    prvInitChannelTypeForChannel(prvCurrentlyActiveChannel);
    /* Update the top button */
    GUIButton_SetText(GUIButtonId_Channel1Top + prvCurrentlyActiveChannel - 1,
                      prvNameForChannelType[prvChannelType[prvCurrentlyActiveChannel-1]],
                      0);
    /* Update the sidebar that is currently visible */
    prvSetActiveSidebar(prvCurrentlyActiveSidebar, true);

    /* Clear the alert box */
    GUIAlertBox_Clear(GUIAlertBoxId_ConfirmId);
  }
  else if (CallbackButton == GUIAlertBoxCallbackButton_Right)
  {
    /* TODO: User said no, do something! */

    /* Clear the alert box */
    GUIAlertBox_Clear(GUIAlertBoxId_ConfirmId);
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
//      return;
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
static void buttonInModuleModeBoxPressed(uint32_t Row, uint32_t Column)
{
  /* Sanity check */
  if (ACTIVE_SIDEBAR_IS_FOR_A_CHANNEL(prvCurrentlyActiveSidebar))
  {
    /* Change the module mode if it is a new one */
    if (prvChannelType[prvCurrentlyActiveChannel-1] == APP_ChannelType_UART && Column != MODULE_MODE_BUTTON_GRID_BOX_UART_COLUMN)
    {
      /* Switch the state of the buttons */
      GUIButtonGridBox_SetButtonState(GUIButtonGridBoxId_ModuleModeSelection, 0, MODULE_MODE_BUTTON_GRID_BOX_GPIO_COLUMN, GUIButtonState_State2, true);
      GUIButtonGridBox_SetButtonState(GUIButtonGridBoxId_ModuleModeSelection, 0, MODULE_MODE_BUTTON_GRID_BOX_UART_COLUMN, GUIButtonState_State1, true);
      /* Change the channel type to GPIO */
      prvChannelType[prvCurrentlyActiveChannel-1] = APP_ChannelType_GPIO;
    }
    else if (prvChannelType[prvCurrentlyActiveChannel-1] == APP_ChannelType_GPIO && Column != MODULE_MODE_BUTTON_GRID_BOX_GPIO_COLUMN)
    {
      /* Switch the state of the buttons */
      GUIButtonGridBox_SetButtonState(GUIButtonGridBoxId_ModuleModeSelection, 0, MODULE_MODE_BUTTON_GRID_BOX_GPIO_COLUMN, GUIButtonState_State1, true);
      GUIButtonGridBox_SetButtonState(GUIButtonGridBoxId_ModuleModeSelection, 0, MODULE_MODE_BUTTON_GRID_BOX_UART_COLUMN, GUIButtonState_State2, true);
      /* Change the channel type to UART */
      prvChannelType[prvCurrentlyActiveChannel-1] = APP_ChannelType_UART;
    }
    else
    {
      goto clear;
    }

    /* Update the EEPROM */
    prvSaveChannelTypeToEeprom(prvCurrentlyActiveChannel);

    /* Update the sidebar that is currently visible to reflect the change in channel */
    prvSetActiveSidebar(prvCurrentlyActiveSidebar, true);

    /* Update the top button */
    GUIButton_SetText(GUIButtonId_Channel1Top + prvCurrentlyActiveChannel - 1,
                      prvNameForChannelType[prvChannelType[prvCurrentlyActiveChannel-1]],
                      0);

clear:
    /* Clear the button grid box */
    GUIButtonGridBox_Clear(GUIButtonGridBoxId_ModuleModeSelection);
  }
}

/**
 * @brief   Get the color for a specific channel
 * @param   Channel: The channel to use
 * @retval  The color for the channel or COLOR_ERROR if the channel is invalid
 */
static uint32_t prvColorForChannel(uint8_t Channel)
{
  if (Channel == 1)
    return COLOR_APP_CH1;
  else if (Channel == 2)
    return COLOR_APP_CH2;
  else if (Channel == 3)
    return COLOR_APP_CH3;
  else if (Channel == 4)
    return COLOR_APP_CH4;
  else if (Channel == 5)
    return COLOR_APP_CH5;
  else if (Channel == 6)
    return COLOR_APP_CH6;
  else
    return COLOR_ERROR;
}

/**
 * @brief   Init the top item for a channel
 * @param   Channel: The channel to init
 * @retval  None
 */
static void prvInitTopForChannel(uint8_t Channel)
{
  if (Channel > 0 && Channel < 7)
  {
    /* Label */
    prvLabel.object.id                = GUILabelId_Channel1Top + Channel-1;
    prvLabel.object.xPos              = (Channel-1)*110;
    prvLabel.object.yPos              = 0;
    prvLabel.object.width             = 30;
    prvLabel.object.height            = 40;
    if (Channel == 1)
      prvLabel.object.border            = GUIBorder_Bottom;
    else
      prvLabel.object.border            = GUIBorder_Bottom | GUIBorder_Left;
    prvLabel.object.borderThickness   = 1;
    prvLabel.object.borderColor       = COLOR_WHITE;
    prvLabel.object.layer             = GUILayer_1;
    prvLabel.object.displayState      = GUIDisplayState_NotHidden;
    prvLabel.backgroundColor          = prvColorForChannel(Channel);
    prvLabel.textColor[0]             = COLOR_BLACK;
    prvLabel.text[0]                  = prvChannelNumberString[Channel-1];
    prvLabel.font                     = &font_18pt_variableWidth;
//    prvLabel.font                     = &font_18pt_bold_variableWidth;
    GUILabel_Init(&prvLabel);

    /* Button */
    prvButton.object.id               = GUIButtonId_Channel1Top + Channel-1;
    prvButton.object.xPos             = 30 + (Channel-1)*110;
    prvButton.object.yPos             = 0;
    prvButton.object.width            = 80;
    prvButton.object.height           = 40;
    prvButton.object.border           = GUIBorder_Bottom | GUIBorder_Right;
    prvButton.object.borderThickness  = 1;
    prvButton.object.borderColor      = COLOR_WHITE;
    prvButton.object.layer            = GUILayer_1;
    prvButton.object.displayState     = GUIDisplayState_NotHidden;
    prvButton.state1TextColor         = prvColorForChannel(Channel);
    prvButton.state1BackgroundColor   = COLOR_BLACK;
    prvButton.state2TextColor         = prvButton.state1TextColor;
    prvButton.state2BackgroundColor   = prvButton.state1BackgroundColor;
    prvButton.pressedTextColor        = prvButton.state1TextColor;
    prvButton.pressedBackgroundColor  = COLOR_WHITE;
    prvButton.buttonState             = GUIButtonState_State1;
    prvButton.touchCallback           = prvTopAndSystemButtonCallback;
    if (IS_VALID_CHANNEL_TYPE(prvChannelType[Channel-1]))
      prvButton.text[0]               = prvNameForChannelType[prvChannelType[Channel-1]];
    else
      prvButton.text[0]               = "ERROR";
    prvButton.font                    = &font_18pt_variableWidth;
//    prvButton.font                    = &font_18pt_bold_variableWidth;
    GUIButton_Init(&prvButton);
  }
}

/**
 * @brief   Init the top and system items
 * @param   None
 * @retval  None
 */
static void prvInitTopAndSystemItems()
{
  /* Init all the channel tops */
  for (uint32_t channel = 1; channel < 7; channel++)
  {
    prvInitTopForChannel(channel);
  }

  /* Clock Label */
  GUI_CLOCK_InitLabel();
  /* Create the GUI Clock refresh timer */
  prvGUIClockRefreshTimer = xTimerCreate("GUI Clock RefreshTimer", 500 / portTICK_PERIOD_MS, pdTRUE, 0, GUI_CLOCK_UpdateTime);
  if (prvGUIClockRefreshTimer != NULL)
    xTimerStart(prvGUIClockRefreshTimer, portMAX_DELAY);

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
//  prvButton.font                    = &font_18pt_bold_variableWidth;
  GUIButton_Init(&prvButton);
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void prvTopAndSystemButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
  static APP_ActiveSidebar lastActiveSidebar = APP_ActiveSidebar_None;

  /* If a button was pressed we should change the sidebar */
  if (Event == GUITouchEvent_Up)
  {
    switch (ButtonId)
    {
      case GUIButtonId_Channel1Top:
        prvSetActiveSidebar(APP_ActiveSidebar_1, false);
        break;
      case GUIButtonId_Channel2Top:
        prvSetActiveSidebar(APP_ActiveSidebar_2, false);
        break;
      case GUIButtonId_Channel3Top:
        prvSetActiveSidebar(APP_ActiveSidebar_3, false);
        break;
      case GUIButtonId_Channel4Top:
        prvSetActiveSidebar(APP_ActiveSidebar_4, false);
        break;
      case GUIButtonId_Channel5Top:
        prvSetActiveSidebar(APP_ActiveSidebar_5, false);
        break;
      case GUIButtonId_Channel6Top:
        prvSetActiveSidebar(APP_ActiveSidebar_6, false);
        break;
      case GUIButtonId_System:
        /* For the system button we want it to restore to the last active sidebar if it's pressed again */
        if (prvCurrentlyActiveSidebar == APP_ActiveSidebar_System && lastActiveSidebar != APP_ActiveSidebar_None)
        {
          prvSetActiveSidebar(lastActiveSidebar, false);
          lastActiveSidebar = APP_ActiveSidebar_None;
        }
        else
        {
          lastActiveSidebar = prvCurrentlyActiveSidebar;
          prvSetActiveSidebar(APP_ActiveSidebar_System, false);
        }
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
  prvSetActiveSidebar(APP_ActiveSidebar_1, false);
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void prvInitChannelIdsAndTypes()
{
  /** 1. Read IDs from FPGA */
  /* Enable power to all channels to read the ID */
  SPI_COMM_EnablePowerForChannel(SPI_COMM_Channel_All);
  /* Enable ID update */
  SPI_COMM_EnableIdUpdateForChannel(SPI_COMM_Channel_All);
  /* Wait a bit */
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  /* Disable Id update */
  SPI_COMM_DisableIdUpdateForChannel(SPI_COMM_Channel_All);
  /* Disable power to all channels */
  SPI_COMM_DisablePowerForChannel(SPI_COMM_Channel_All);
  /* Read the IDs */
  uint8_t tempData;
  for (uint32_t channel = 1; channel < 7; channel++)
  {
    if (SPI_COMM_GetIdForChannel(channel, &tempData) == SUCCESS)
      prvChannelID[channel-1] = tempData;
    /* TODO: What happens here if ERROR */
  }

  /* Init one channel at a time */
  for (uint32_t channel = 1; channel < 7; channel++)
  {
    /** 2. Read ID from EEPROM and compare with the one from the FPGA */
    uint8_t storedChannelId = I2C_EEPROM_ReadByte(EEPROM_CHANNEL_ID_START_ADDRESS + channel - 1);
    /* If it's not valid we should store a valid id */
    if (!IS_VALID_MODULE_ID(storedChannelId))
      prvSaveChannelIdToEeprom(channel);
    /* If the stored id is different from the one read from the FPGA we should setup the channel */
    else if (storedChannelId != prvChannelID[channel-1])
    {
      /* TODO: Read the ID again from data-proc to make sure? */
      prvSetupChannel[channel - 1] = true;
    }

    /** 3. Init the channel type */
    prvInitChannelTypeForChannel(channel);

  }
  /** 5. Done */
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void prvInitChannelTypeForChannel(uint8_t Channel)
{
  if (Channel > 0 && Channel < 7)
  {
    /** 1. Setup the default type based on the ID or the setup variable */
    if (prvSetupChannel[Channel-1] == true)
      prvChannelType[Channel-1]       = APP_ChannelType_SETUP;
    else if (prvChannelID[Channel-1] == APP_ModuleIdType_GPIO)
      prvChannelType[Channel-1]       = APP_ChannelType_GPIO;
    else if (prvChannelID[Channel-1] == APP_ModuleIdType_CAN)
      prvChannelType[Channel-1]       = APP_ChannelType_CAN;
    else if (prvChannelID[Channel-1] == APP_ModuleIdType_RS_232)
      prvChannelType[Channel-1]       = APP_ChannelType_RS_232;
    else
      prvChannelType[Channel-1]       = APP_ChannelType_NA;


    /** 2. Read Type from EEPROM and compare with the one in the array */
    uint8_t storedChannelType = I2C_EEPROM_ReadByte(EEPROM_CHANNEL_TYPE_START_ADDRESS + Channel - 1);
    /* If it's not valid we should store a valid type */
    if (!IS_VALID_CHANNEL_TYPE(storedChannelType))
      prvSaveChannelTypeToEeprom(Channel);
    /* If the stored type is different from the one in the array we should check what to use  */
    else if (storedChannelType != prvChannelType[Channel-1])
    {
      /* A GPIO Module can be of type UART as well */
      if (prvChannelID[Channel-1] == APP_ModuleIdType_GPIO && storedChannelType == APP_ChannelType_UART)
        prvChannelType[Channel-1] = storedChannelType;

      /* Write back to EEPROM the new type */
      prvSaveChannelTypeToEeprom(Channel);
    }
  }
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void prvSaveChannelIdToEeprom(uint8_t Channel)
{
  if (Channel > 0 && Channel < 7)
  {
    I2C_EEPROM_WriteByte(EEPROM_CHANNEL_ID_START_ADDRESS + Channel - 1, prvChannelID[Channel-1]);
  }
}

/**
 * @brief
 * @param
 * @retval  None
 */
static void prvSaveChannelTypeToEeprom(uint8_t Channel)
{
  if (Channel > 0 && Channel < 7)
  {
    I2C_EEPROM_WriteByte(EEPROM_CHANNEL_TYPE_START_ADDRESS + Channel - 1, prvChannelType[Channel-1]);
  }
}

/** Interrupt Handlers -------------------------------------------------------*/
