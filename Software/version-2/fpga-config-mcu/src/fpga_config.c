/**
 ******************************************************************************
 * @file    fpga_config.c
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-08-16
 * @brief
 ******************************************************************************
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
 ******************************************************************************
 */

/** Includes -----------------------------------------------------------------*/
#include "fpga_config.h"

/** Private defines ----------------------------------------------------------*/
#define CONF_DONE_PORT  (GPIOB)
#define CONF_DONE_PIN   (GPIO_PIN_10)

#define NSTATUS_PORT    (GPIOB)
#define NSTATUS_PIN     (GPIO_PIN_11)

#define FPGA_INTERRUPT_PORT   (GPIOB)
#define FPGA_INTERRUPT_PIN    (GPIO_PIN_1)

#define NCONFIG_PORT   (GPIOA)
#define NCONFIG_PIN    (GPIO_PIN_8)

/** Private variables --------------------------------------------------------*/
/** Private functions --------------------------------------------------------*/
/** Functions ----------------------------------------------------------------*/
/**
 * @brief   Initializes the FPGA Config
 * @param   None
 * @retval  None
 */
void FPGA_CONFIG_Init(void)
{
  /* Enable clock for GPIOA & GPIOB */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStructure;

  /* Initialize CONF_DONE */
  GPIO_InitStructure.Pin    = CONF_DONE_PIN;
  GPIO_InitStructure.Speed  = GPIO_SPEED_LOW;
  GPIO_InitStructure.Mode   = GPIO_MODE_INPUT;
  HAL_GPIO_Init(CONF_DONE_PORT, &GPIO_InitStructure);

  /* Initialize nSTATUS */
  GPIO_InitStructure.Pin    = NSTATUS_PIN;
  GPIO_InitStructure.Speed  = GPIO_SPEED_LOW;
  GPIO_InitStructure.Mode   = GPIO_MODE_INPUT;
  HAL_GPIO_Init(NSTATUS_PORT, &GPIO_InitStructure);

  /* Initialize FPGA_INTERRUPT */
  GPIO_InitStructure.Pin    = FPGA_INTERRUPT_PIN;
  GPIO_InitStructure.Speed  = GPIO_SPEED_LOW;
  GPIO_InitStructure.Mode   = GPIO_MODE_INPUT;
  HAL_GPIO_Init(FPGA_INTERRUPT_PORT, &GPIO_InitStructure);

  /* Initialize nCONFIG */
  GPIO_InitStructure.Pin    = NCONFIG_PIN;
  GPIO_InitStructure.Speed  = GPIO_SPEED_LOW;
  GPIO_InitStructure.Mode   = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(NCONFIG_PORT, &GPIO_InitStructure);
  HAL_GPIO_WritePin(NCONFIG_PORT, NCONFIG_PIN, GPIO_PIN_SET);
}

void FPGA_CONFIG_Start()
{
  /**
   * To begin the configuration, the external host device must generate a
   * low-to-high transition on the nCONFIG pin
   */
  HAL_GPIO_WritePin(NCONFIG_PORT, NCONFIG_PIN, GPIO_PIN_RESET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(NCONFIG_PORT, NCONFIG_PIN, GPIO_PIN_SET);

  /**
   * When nSTATUS is pulled high, the external host device must place the
   * configuration data one bit at a time on DATA[0]
   */
  while (HAL_GPIO_ReadPin(NSTATUS_PORT, NSTATUS_PIN) != GPIO_PIN_SET)
  {
    HAL_Delay(10);
  }
}

/** Interrupt Handlers -------------------------------------------------------*/
