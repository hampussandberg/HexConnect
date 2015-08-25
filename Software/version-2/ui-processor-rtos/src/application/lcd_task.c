/**
 *******************************************************************************
 * @file  lcd_task.c
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

/** Includes -----------------------------------------------------------------*/
#include "lcd_task.h"

#include <string.h>

/** Private defines ----------------------------------------------------------*/
/** Private typedefs ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
/** Private function prototypes ----------------------------------------------*/
static void prvHardwareInit();

/** Functions ----------------------------------------------------------------*/
/**
  * @brief  Text
  * @param  None
  * @retval None
  */
void lcdTask(void *pvParameters)
{
  prvHardwareInit();

  /* The parameter in vTaskDelayUntil is the absolute time
   * in ticks at which you want to be woken calculated as
   * an increment from the time you were last woken. */
  TickType_t xNextWakeTime;
  /* Initialize xNextWakeTime - this only needs to be done once. */
  xNextWakeTime = xTaskGetTickCount();

  while (1)
  {
    vTaskDelayUntil(&xNextWakeTime, 500 / portTICK_PERIOD_MS);
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

}

/** Interrupt Handlers -------------------------------------------------------*/
