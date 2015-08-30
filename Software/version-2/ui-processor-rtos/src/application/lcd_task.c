/**
 *******************************************************************************
 * @file    lcd_task.c
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-08-15
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
#include "lcd_task.h"

#include <string.h>
#include <stdbool.h>

#include "main_task.h"
#include "lcd.h"
#include "ft5206.h"
#include "simple_gui.h"

/** Private defines ----------------------------------------------------------*/
/** Private typedefs ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
static xTimerHandle prvRefreshTimer;

static bool prvRefreshDisplay = true;

/** Private function prototypes ----------------------------------------------*/
static void prvHardwareInit();
static void prvSplashScreen();
static void prvRefreshTimerCallback();

/** Functions ----------------------------------------------------------------*/
/**
  * @brief  Text
  * @param  None
  * @retval None
  */
void lcdTask(void *pvParameters)
{
  /* Initialize the hardware */
  prvHardwareInit();

  /* Display splash screen */
  prvSplashScreen();

  /* Create the LCDEventMessage queue */
  xLCDEventQueue = xQueueCreate(10, sizeof(LCDEventMessage));
  if (xLCDEventQueue == 0)
  {
    /* TODO */
    // Queue was not created and must not be used.
  }

  MAIN_TASK_NotifyLcdTaskIsDone();

  prvRefreshTimer = xTimerCreate("RefreshTimer", 50 / portTICK_PERIOD_MS, pdTRUE, 0, prvRefreshTimerCallback);
  if (prvRefreshTimer != NULL)
    xTimerStart(prvRefreshTimer, portMAX_DELAY);

  LCDEventMessage receivedMessage;

  while (1)
  {
    /* Wait for a message to be received or the timeout to happen */
    if (xQueueReceive(xLCDEventQueue, &receivedMessage, 50) == pdTRUE)
    {
      /* Item successfully removed from the queue */
      switch (receivedMessage.event)
      {
        /* New touch data received */
        case LCDEvent_TouchEvent:
          if (receivedMessage.data[3] == FT5206Point_1)
          {
            if (receivedMessage.data[2] == FT5206Event_PutUp)
              GUI_TouchAtPosition(GUITouchEvent_Up, receivedMessage.data[0], receivedMessage.data[1]);
            else if (receivedMessage.data[2] == FT5206Event_Contact || receivedMessage.data[2] == FT5206Event_PutDown)
              GUI_TouchAtPosition(GUITouchEvent_Down, receivedMessage.data[0], receivedMessage.data[1]);

            HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);

#if 0
            /* DEBUG */
            if (GUI_GetDisplayStateForTextBox(GUITextBoxId_Debug) == GUIDisplayState_NotHidden)
            {
              GUITextBox_SetWritePosition(GUITextBoxId_Debug, 5, 5);
              GUITextBox_Clear(GUITextBoxId_Debug);
              GUITextBox_WriteString(GUITextBoxId_Debug, "X:");
              GUITextBox_WriteNumber(GUITextBoxId_Debug, receivedMessage.data[0]);
              GUITextBox_WriteString(GUITextBoxId_Debug, ", Y:");
              GUITextBox_WriteNumber(GUITextBoxId_Debug, receivedMessage.data[1]);
              GUITextBox_WriteString(GUITextBoxId_Debug, ", EVENT:");
              GUITextBox_WriteNumber(GUITextBoxId_Debug, receivedMessage.data[2]);
            }
#endif

#if 0
            /* Draw a dot on debug */
//            LCD_DrawPixelOnLayer(0xFF00FF00, receivedMessage.data[0], receivedMessage.data[1], LCD_LAYER_1);
            LCD_DrawFilledCircleOnLayer(0xFFFF0000, receivedMessage.data[0], receivedMessage.data[1], 10, LCD_LAYER_1);
//            prvRefreshDisplay = true;
#endif
          }
          break;
        default:
          break;
      }
    }
    else
    {
      /* Timeout has occured i.e. no message available */
//      HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_2);
      /* Do something else */
    }
  }
}

/** Private functions .-------------------------------------------------------*/
/**
  * @brief  Initializes the hardware
  * @param  None
  * @retval None
  */
static void prvHardwareInit()
{
  /* LCD */
  LCD_Init();
  LCD_LayerInit();

  /* Capacitive Touch */
  FT5206_Init();

//  FT5206_TestMode();
//  GPIO_InitTypeDef GPIO_InitStructure;
//  GPIO_InitStructure.Pin    = GPIO_PIN_7;
//  GPIO_InitStructure.Mode   = GPIO_MODE_OUTPUT_PP;
//  GPIO_InitStructure.Pull   = GPIO_NOPULL;
//  GPIO_InitStructure.Speed  = GPIO_SPEED_LOW;
//  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
//  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
}

/**
  * @brief  Displays the splash screen
  * @param  None
  * @retval None
  */
static void prvSplashScreen()
{

}

/**
  * @brief
  * @param  None
  * @retval None
  */
static void prvRefreshTimerCallback()
{
  GUI_DrawAndRefreshDirtyZones();

//  if (prvRefreshDisplay)
//  {
//    LCD_ClearScreenBuffer(0x0000);
//    LCD_DrawLayerToBuffer(LCD_LAYER_1);
//  //  LCD_DrawLayerToBuffer(LCD_LAYER_2);
//  //  LCD_DrawLayerToBuffer(LCD_LAYER_3);
//
//#if defined(DUAL_BUFFER_MODE)
//    LCD_SetBufferAsActiveScreen();
//#else
//    LCD_RefreshActiveDisplay();
//#endif
//    prvRefreshDisplay = false;
//  }
}

/** Interrupt Handlers -------------------------------------------------------*/
