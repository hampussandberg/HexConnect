/**
 *******************************************************************************
 * @file    background_task.c
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
#include "background_task.h"

#include <string.h>

#include "buzzer.h"
#include "spi_flash.h"
#include "spi_comm.h"
#include "i2c_eeprom.h"
#include "sdram.h"
#include "lcd.h"

#include "images.h"

/** Private defines ----------------------------------------------------------*/
/** Private typedefs ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
/** Private function prototypes ----------------------------------------------*/
static void prvHardwareInit();

/** Functions ----------------------------------------------------------------*/
/**
 * @brief  Text
 * @param  None
 * @retval  None
 */
void backgroundTask(void *pvParameters)
{
  prvHardwareInit();

  /* The parameter in vTaskDelayUntil is the absolute time
   * in ticks at which you want to be woken calculated as
   * an increment from the time you were last woken. */
  TickType_t xNextWakeTime;
  /* Initialize xNextWakeTime - this only needs to be done once. */
  xNextWakeTime = xTaskGetTickCount();


//  BUZZER_Init();
//
//  SPI_FLASH_Init();
////  SPI_FLASH_WriteByte(0x000000, 0xDA);
//  uint8_t data = 0x00;
//  SPI_FLASH_ReadBuffer(&data, 0x000000, 1);
//  if (data != 0xDA)
//    BUZZER_BeepNumOfTimes(20);
//  else
//    BUZZER_BeepNumOfTimes(5);
//
//  SPI_COMM_Init();
//
//  vTaskDelayUntil(&xNextWakeTime, 1000 / portTICK_PERIOD_MS);
//
//  I2C_EEPROM_Init();
////  I2C_EEPROM_WriteByte(0x00, 0xEC);
//  data = 0x00;
//  data = I2C_EEPROM_ReadByte(0x00);
//  if (data != 0xEC)
//    BUZZER_BeepNumOfTimes(20);
//  else
//    BUZZER_BeepNumOfTimes(5);

  SDRAM_Init();
  SDRAM_EraseAll(SDRAM_END);

  LCD_Init();
  LCD_LayerInit();

  LCD_DrawFilledRectangleOnLayer(0xFFFFFFFF, 100, 100, 100, 100, LCD_LAYER_1);
  LCD_DrawFilledRectangleOnLayer(0xFFFFFFFF, 300, 100, 100, 100, LCD_LAYER_1);
  LCD_DrawFilledRectangleOnLayer(0xFF00FF00, 350, 150, 100, 100, LCD_LAYER_2);
  LCD_DrawStringOnLayer(0xFFFFFFFF, 10, 10, "Hello World!", &font_24pt_variableWidth, LCD_LAYER_1);
  LCD_DrawStraightLineOnLayer(0xFFFF00FF, 10, 300, 200, LCD_DrawDirection_Horizontal, LCD_LAYER_2);
  LCD_DrawLineOnLayer(0xFF00FFFF, 20, 310, 200, 320, LCD_LAYER_1);
  LCD_DrawRectangleOnLayer(0xFF00FF00, 10, 300, 50, 30, LCD_LAYER_2);
  LCD_DrawCircleOnLayer(0xFFFF0000, 500, 300, 20, LCD_LAYER_2);
  LCD_DrawFilledCircleOnLayer(0xFFFFFF00, 500, 340, 15, LCD_LAYER_1);
  LCD_DrawARGB8888ImageOnLayer(500, 10, &splash_screen, LCD_LAYER_3);

  LCD_DrawLayerToBuffer(LCD_LAYER_1);
  LCD_DrawLayerToBuffer(LCD_LAYER_2);
  LCD_DrawLayerToBuffer(LCD_LAYER_3);
  LCD_RefreshActiveDisplay();

  uint32_t color = 0;

  while (1)
  {
    /* Toggle the LED every 500ms */
    HAL_GPIO_TogglePin(GPIOD, backgroundLED_0);
    vTaskDelayUntil(&xNextWakeTime, 500 / portTICK_PERIOD_MS);

//    if (color == 0)
//    {
//      SDRAM_FillAll(SDRAM_END, 0x001F);
//      color = 1;
//    }
//    else if (color == 1)
//    {
//      SDRAM_FillAll(SDRAM_END, 0x07E0);
//      color = 2;
//    }
//    else if (color == 2)
//    {
//      SDRAM_FillAll(SDRAM_END, 0xF800);
//      color = 0;
//    }
  }
}

/** Private functions .-------------------------------------------------------*/
/**
 * @brief   Initializes the hardware
 * @param   None
 * @retval  None
 */
static void prvHardwareInit()
{
  /* Set up the LED outputs */
  __GPIOD_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.Pin    = backgroundLED_0 | backgroundLED_1 | backgroundLED_2;
  GPIO_InitStructure.Mode    = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pull    = GPIO_NOPULL;
  GPIO_InitStructure.Speed   = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

  HAL_GPIO_WritePin(GPIOD, backgroundLED_0, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOD, backgroundLED_1, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOD, backgroundLED_2, GPIO_PIN_SET);


  /* BL ADJ */
  __GPIOA_CLK_ENABLE();
  GPIO_InitStructure.Pin    = GPIO_PIN_7;
  GPIO_InitStructure.Mode   = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pull   = GPIO_NOPULL;
  GPIO_InitStructure.Speed  = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);

  /* LCD-DISP-ENABLE */
  __GPIOC_CLK_ENABLE();
  GPIO_InitStructure.Pin    = GPIO_PIN_4;
  GPIO_InitStructure.Mode   = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pull   = GPIO_NOPULL;
  GPIO_InitStructure.Speed  = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);
}

/** Interrupt Handlers -------------------------------------------------------*/
