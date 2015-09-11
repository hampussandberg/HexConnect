/**
 ******************************************************************************
 * @file    led.c
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-08-15
 * @brief   Manage the LED
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
#include "led.h"

#include <stdbool.h>

/** Private defines ----------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
/** Private functions --------------------------------------------------------*/
/** Functions ----------------------------------------------------------------*/
/**
 * @brief   Initializes the LED
 * @param   None
 * @retval  None
 */
void LED_Init(void)
{
  /* Enable clock for GPIOB */
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* Initialize as output push-pull */
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.Pin    = LED_PIN;
  GPIO_InitStructure.Speed  = GPIO_SPEED_LOW;
  GPIO_InitStructure.Mode   = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pull   = GPIO_NOPULL;
  HAL_GPIO_Init(LED_PORT, &GPIO_InitStructure);

  HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);


  /* Init the timer */
  __HAL_RCC_TIM2_CLK_ENABLE();
  HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM2_IRQn);

  /* Compute the prescaler value to have TIMx counter clock equal to 1000 Hz */
  uint32_t prescalerValue = (uint32_t)(SystemCoreClock / 1000) - 1;

  LedTimerHandle.Instance                = TIM2;
  LedTimerHandle.Init.Period             = 1000 - 1;
  LedTimerHandle.Init.Prescaler          = prescalerValue;
  LedTimerHandle.Init.ClockDivision      = 0;
  LedTimerHandle.Init.CounterMode        = TIM_COUNTERMODE_UP;
  LedTimerHandle.Init.RepetitionCounter  = 0;

  if (HAL_TIM_Base_Init(&LedTimerHandle) != HAL_OK)
  {
    /* Initialization Error */
  }

  /* Start the TIM Base generation in interrupt mode */
  if (HAL_TIM_Base_Start_IT(&LedTimerHandle) != HAL_OK)
  {
    /* Starting Error */
  }
}

/**
 * @brief
 * @param   Period:
 * @retval  None
 */
void LED_SetBlinkPeriod(uint32_t Period)
{
  if (LedTimerHandle.Init.Period != Period - 1)
  {
    LedTimerHandle.Init.Period = Period - 1;
    if (HAL_TIM_Base_Init(&LedTimerHandle) != HAL_OK)
    {
      /* Initialization Error */
    }
  }
}

/**
 * @brief   Turn on the LED
 * @param   None
 * @retval  None
 */
void LED_On()
{
  HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
}

/**
 * @brief   Turn off the LED
 * @param   None
 * @retval  None
 */
void LED_Off()
{
  HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
}

/**
 * @brief   Toggle the LED
 * @param   None
 * @retval  None
 */
void LED_Toggle()
{
  HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
}

/* Interrupt Handlers --------------------------------------------------------*/
