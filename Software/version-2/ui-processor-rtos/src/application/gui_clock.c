/**
 *******************************************************************************
 * @file    gui_clock.c
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-09-20
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
#include "gui_clock.h"
#include <stdbool.h>

#include "simple_gui.h"

/** Private defines ----------------------------------------------------------*/
#define RTC_ASYNCH_PREDIV  0x7F   /* LSE as RTC clock */
#define RTC_SYNCH_PREDIV   0x00FF /* LSE as RTC clock */

#define BACKUP_CHECK_VALUE    0x32F1

/** Private typedefs ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
static GUILabel prvLabel;
static RTC_HandleTypeDef RTC_Handle;
static RTC_TimeTypeDef prvCurrentTime;
static RTC_DateTypeDef prvCurrentDate;
static char prvCurrentTimeAsString[10] = "HH:MM:SS";
static bool prvForceCalenderConfig = false;

static bool prvInitialized = false;

/** Private function prototypes ----------------------------------------------*/
static void prvEnableClock();
static void prvCalendarConfig();

/** Functions ----------------------------------------------------------------*/
/**
 * @brief   Initializes the GUI Clock Hardware
 * @param   None
 * @retval  None
 */
void GUI_CLOCK_InitClock()
{
  prvEnableClock();

  /*##-1- Configure the RTC peripheral #######################################*/
  /* Configure RTC prescaler and RTC data registers */
  /* RTC configured as follows:
      - Hour Format    = Format 24
      - Asynch Prediv  = Value according to source clock
      - Synch Prediv   = Value according to source clock
      - OutPut         = Output Disable
      - OutPutPolarity = High Polarity
      - OutPutType     = Open Drain
  */
  RTC_Handle.Instance             = RTC;
  RTC_Handle.Init.HourFormat      = RTC_HOURFORMAT_24;
  RTC_Handle.Init.AsynchPrediv    = RTC_ASYNCH_PREDIV;
  RTC_Handle.Init.SynchPrediv     = RTC_SYNCH_PREDIV;
  RTC_Handle.Init.OutPut          = RTC_OUTPUT_DISABLE;
  RTC_Handle.Init.OutPutPolarity  = RTC_OUTPUT_POLARITY_HIGH;
  RTC_Handle.Init.OutPutType      = RTC_OUTPUT_TYPE_OPENDRAIN;

  if (HAL_RTC_Init(&RTC_Handle) != HAL_OK)
  {
    /* Initialization Error */
  }

  /*##-2- Check if Data stored in BackUp register1: No Need to reconfigure RTC#*/
  /* Read the Back Up Register 1 Data */
  if (HAL_RTCEx_BKUPRead(&RTC_Handle, RTC_BKP_DR1) != BACKUP_CHECK_VALUE || prvForceCalenderConfig == true)
  {
    /* Configure RTC Calendar */
    prvCalendarConfig();
  }
}

/**
 * @brief   Initializes the GUI Clock Label
 * @param   None
 * @retval  None
 */
void GUI_CLOCK_InitLabel()
{
  /* Clock Label */
  prvLabel.object.id              = GUILabelId_Clock;
  prvLabel.object.xPos            = 660;
  prvLabel.object.yPos            = 0;
  prvLabel.object.width           = 140;
  prvLabel.object.height          = 40;
  prvLabel.object.border          = GUIBorder_Left | GUIBorder_Bottom;
  prvLabel.object.borderThickness = 1;
  prvLabel.object.borderColor     = COLOR_WHITE;
  prvLabel.object.layer           = GUILayer_1;
  prvLabel.object.displayState    = GUIDisplayState_NotHidden;
  prvLabel.backgroundColor        = COLOR_BLACK;
  prvLabel.textColor[0]           = COLOR_WHITE;
  prvLabel.text[0]                = prvCurrentTimeAsString;
  prvLabel.font                   = &font_18pt_variableWidth;
//  prvLabel.font                   = &font_18pt_bold_variableWidth;
  GUILabel_Init(&prvLabel);

  prvInitialized = true;
}

/**
 * @brief   Display the current time and date
 * @param   None
 * @retval  None
 */
void GUI_CLOCK_UpdateTime()
{
  if (prvInitialized == true)
  {
    /* Get the RTC current Time and Date (has to read both after each other */
    HAL_RTC_GetTime(&RTC_Handle, &prvCurrentTime, RTC_FORMAT_BCD);
    HAL_RTC_GetDate(&RTC_Handle, &prvCurrentDate, RTC_FORMAT_BCD);

    prvCurrentTimeAsString[0] = '0' + (prvCurrentTime.Hours >> 4);
    prvCurrentTimeAsString[1] = '0' + (prvCurrentTime.Hours & 0x0F);

    prvCurrentTimeAsString[3] = '0' + (prvCurrentTime.Minutes >> 4);
    prvCurrentTimeAsString[4] = '0' + (prvCurrentTime.Minutes & 0x0F);

    prvCurrentTimeAsString[6] = '0' + (prvCurrentTime.Seconds >> 4);
    prvCurrentTimeAsString[7] = '0' + (prvCurrentTime.Seconds & 0x0F);

    /* Update the label */
    GUILabel_SetText(GUILabelId_Clock, prvCurrentTimeAsString, 0);

    /* Display time Format : hh:mm:ss */
  //  sprintf((char *)showtime, "%02d:%02d:%02d", RTC_TimeStructure.Hours, RTC_TimeStructure.Minutes, RTC_TimeStructure.Seconds);
    /* Display date Format : mm-dd-yy */
  //  sprintf((char *)showdate, "%02d-%02d-%02d", RTC_DateStructure.Month, RTC_DateStructure.Date, 2000 + RTC_DateStructure.Year);
  }
}

/** Private functions .-------------------------------------------------------*/
/**
 * @brief   Configure the current time and date.
 * @param   None
 * @retval  None
 */
static void prvEnableClock()
{
  RCC_OscInitTypeDef        RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

  /*##-1- Enables the PWR Clock and Enables access to the backup domain ######*/
  /* To change the source clock of the RTC feature (LSE, LSI), You have to:
     - Enable the power clock using __HAL_RCC_PWR_CLK_ENABLE()
     - Enable write access using HAL_PWR_EnableBkUpAccess() function before to
       configure the RTC clock source (to be done once after reset).
     - Reset the Back up Domain using __HAL_RCC_BACKUPRESET_FORCE() and
       __HAL_RCC_BACKUPRESET_RELEASE().
     - Configure the needed RTc clock source */
  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();

  /*##-2- Configure LSE as RTC clock source ###################################*/
  RCC_OscInitStruct.OscillatorType  = RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.PLL.PLLState    = RCC_PLL_NONE;
  RCC_OscInitStruct.LSEState        = RCC_LSE_ON;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Config error */
  }

  PeriphClkInitStruct.PeriphClockSelection  = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection     = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    /* Config error */
  }

  /*##-3- Enable RTC peripheral Clocks #######################################*/
  /* Enable RTC Clock */
  __HAL_RCC_RTC_ENABLE();
}

/**
 * @brief   Configure the current time and date.
 * @param   None
 * @retval  None
 */
static void prvCalendarConfig()
{
  RTC_DateTypeDef RTC_DateStructure;
  RTC_TimeTypeDef RTC_TimeStructure;

  /*##-1- Configure the Date #################################################*/
  /* Set Date: Sunday September 26th 2015 */
  RTC_DateStructure.Year     = 0x15;
  RTC_DateStructure.Month    = RTC_MONTH_SEPTEMBER;
  RTC_DateStructure.Date     = 0x26;
  RTC_DateStructure.WeekDay  = RTC_WEEKDAY_FRIDAY;

  if (HAL_RTC_SetDate(&RTC_Handle, &RTC_DateStructure, RTC_FORMAT_BCD) != HAL_OK)
  {
    /* Initialization Error */
  }

  /*##-2- Configure the Time #################################################*/
  /* Set Time: 13:06:00 */
  RTC_TimeStructure.Hours           = 0x13;
  RTC_TimeStructure.Minutes         = 0x06;
  RTC_TimeStructure.Seconds         = 0x00;
  RTC_TimeStructure.TimeFormat      = RTC_HOURFORMAT_24;
  RTC_TimeStructure.DayLightSaving  = RTC_DAYLIGHTSAVING_NONE ;
  RTC_TimeStructure.StoreOperation  = RTC_STOREOPERATION_RESET;

  if (HAL_RTC_SetTime(&RTC_Handle, &RTC_TimeStructure, RTC_FORMAT_BCD) != HAL_OK)
  {
    /* Initialization Error */
  }

  /*##-3- Writes a data in a RTC Backup data Register1 #######################*/
  HAL_RTCEx_BKUPWrite(&RTC_Handle, RTC_BKP_DR1, BACKUP_CHECK_VALUE);
}

/** Interrupt Handlers -------------------------------------------------------*/
