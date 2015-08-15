/**
 *******************************************************************************
 * @file    buzzer.c
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

/* Includes ------------------------------------------------------------------*/
#include "buzzer.h"

#include <stdbool.h>

/* Private defines -----------------------------------------------------------*/
#define BUZZER_PORT               (GPIOA)
#define BUZZER_PIN                (GPIO_PIN_0)
#define BUZZER_AF_GPIO            (GPIO_AF2_TIM5)
#define BUZZER_GPIO_CLK_ENABLE    __GPIOA_CLK_ENABLE()

#define BUZZER_TIMER              (TIM5)
#define BUZZER_TIMER_CLK_ENABLE   __TIM5_CLK_ENABLE()
#define BUZZER_TIMER_GET_CLOCK()  (HAL_RCC_GetPCLK1Freq())
#define BUZZER_TIMER_CHANNEL      (TIM_CHANNEL_1)
#define BUZZER_CCR_REGISTER       CCR1
#define BUZZER_TIMER_CLOCK        (90000000)  /* 90 MHz, see _initialize_hardware.c */
#define BUZZER_PERIOD             (1023)      /* 1024 step PWM */

#define BUZZER_NORMAL_VOLUME      ((BUZZER_PERIOD+1) / 10 - 1)

/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static BUZZERSettings prvCurrentSettings = {
  .volume             = 1,
  .frequency          = 440,
  .delayBetweenBeeps  = 40,
};

static TIM_HandleTypeDef prvTimerHandle = {0};

static TimerHandle_t prvBuzzerBeepTimer = 0;
static bool prvBuzzerIsOn = false;

/* Private function prototypes -----------------------------------------------*/
static uint32_t prvGetPrescalerForFrequency(uint32_t Frequency);
static void prvBuzzerBeepTimerCallback();

/* Functions -----------------------------------------------------------------*/
/**
 * @brief   Initialize the buzzer
 * @param   None
 * @retval  None
 */
void BUZZER_Init()
{
  /* Create software timers */
  if (prvBuzzerBeepTimer == 0)
    prvBuzzerBeepTimer = xTimerCreate("BuzzerBeep", prvCurrentSettings.delayBetweenBeeps, pdFALSE, 0, prvBuzzerBeepTimerCallback);

  /* Enable TIMER Clock */
  BUZZER_TIMER_CLK_ENABLE;

  /* Enable GPIO Clock */
  BUZZER_GPIO_CLK_ENABLE;

  /* Configure the GPIO as alternate function */
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.Pin        = BUZZER_PIN;
  GPIO_InitStructure.Mode       = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Alternate  = BUZZER_AF_GPIO;
  GPIO_InitStructure.Pull       = GPIO_NOPULL;
  GPIO_InitStructure.Speed      = GPIO_SPEED_LOW;
  HAL_GPIO_Init(BUZZER_PORT, &GPIO_InitStructure);

  /* Timer init */
  prvTimerHandle.Instance           = BUZZER_TIMER;
  prvTimerHandle.Init.Period        = BUZZER_PERIOD;
  prvTimerHandle.Init.Prescaler     = prvGetPrescalerForFrequency(prvCurrentSettings.frequency);
  prvTimerHandle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  prvTimerHandle.Init.CounterMode   = TIM_COUNTERMODE_UP;
  HAL_TIM_PWM_Init(&prvTimerHandle);

  /* Output compare init */
  TIM_OC_InitTypeDef timerOutputCompare;
  timerOutputCompare.OCMode       = TIM_OCMODE_PWM1;
  timerOutputCompare.Pulse        = BUZZERVolume_Off;
  timerOutputCompare.OCPolarity   = TIM_OCPOLARITY_HIGH;
  timerOutputCompare.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  timerOutputCompare.OCFastMode   = TIM_OCFAST_DISABLE;
  timerOutputCompare.OCIdleState  = TIM_OCIDLESTATE_SET;
  timerOutputCompare.OCNIdleState = TIM_OCNIDLESTATE_SET;
  HAL_TIM_PWM_ConfigChannel(&prvTimerHandle, &timerOutputCompare, BUZZER_TIMER_CHANNEL);

  /* Start the PWM */
  HAL_TIM_PWM_Start(&prvTimerHandle, BUZZER_TIMER_CHANNEL);
}

/**
 * @brief   Deintialize the buzzer
 * @param   None
 * @retval  None
 */
void BUZZER_DeInit()
{
  HAL_GPIO_DeInit(BUZZER_PORT, BUZZER_PIN);
  HAL_TIM_PWM_Stop(&prvTimerHandle, BUZZER_TIMER_CHANNEL);
  HAL_TIM_PWM_DeInit(&prvTimerHandle);
}

/**
 * @brief   Set the volume of the buzzer
 * @param	  Volume: The volume to set, 0-100
 * @retval  None
 */
void BUZZER_SetVolume(uint32_t Volume)
{
  if (Volume <= 100)
  {
    BUZZER_TIMER->BUZZER_CCR_REGISTER = (uint16_t)(Volume/200.0 * (BUZZER_PERIOD));
    prvCurrentSettings.volume = Volume;
  }
}

/**
 * @brief   Turn off the buzzer
 * @param   None
 * @retval  None
 */
void BUZZER_Off()
{
  BUZZER_TIMER->BUZZER_CCR_REGISTER = 0;
}

/**
 * @brief   Set the frequency of the buzzer
 * @param   Frequency: The frequency the use
 * @retval  None
 */
void BUZZER_SetFrequency(uint32_t Frequency)
{
  prvCurrentSettings.frequency = Frequency;
  BUZZER_DeInit();
  BUZZER_Init();
}

/**
 * @brief   Beep the buzzer a specified number of times
 * @param   NumOfBeeps: Number of beeps
 * @retval  None
 */
void BUZZER_BeepNumOfTimes(uint32_t NumOfBeeps)
{
  if (NumOfBeeps != 0)
  {
    prvCurrentSettings.numOfBeeps = 2*NumOfBeeps;
    /* Start the timer if it's not already started */
    if (xTimerIsTimerActive(prvBuzzerBeepTimer) == pdFALSE)
      xTimerStart(prvBuzzerBeepTimer, 100);
  }
}

/* Private functions .--------------------------------------------------------*/
/**
 * @brief   Calculate the prescaler value for the frequency. No checks are done
 * @param   Frequency: Frequency to use in calculation
 * @retval  The prescaler value
 */
static uint32_t prvGetPrescalerForFrequency(uint32_t Frequency)
{
  uint32_t prescaler = (BUZZER_TIMER_CLOCK / (Frequency * (BUZZER_PERIOD + 1))) - 1;
  return prescaler;
}

/**
 * @brief   Callback for the beep timer
 * @param   None
 * @retval  None
 */
static void prvBuzzerBeepTimerCallback()
{
  /* Only beep if we should beep */
  if (prvCurrentSettings.numOfBeeps != 0)
  {
    prvCurrentSettings.numOfBeeps--;
    /* Check if we were beeping last time or not */
    if (prvBuzzerIsOn)
    {
      prvBuzzerIsOn = false;
      BUZZER_Off();
    }
    else
    {
      prvBuzzerIsOn = true;
      BUZZER_SetVolume(prvCurrentSettings.volume);
    }
    xTimerStart(prvBuzzerBeepTimer, 100);
  }
  /* Otherwise turn it off */
  else
  {
    prvBuzzerIsOn = false;
    BUZZER_Off();
  }
}

/* Interrupt Handlers --------------------------------------------------------*/
