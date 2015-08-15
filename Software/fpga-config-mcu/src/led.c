/**
 ******************************************************************************
 * @file    led.c
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-08-15
 * @brief   Manage the LED
 ******************************************************************************
  Copyright (c) 2015 Hampus Sandberg.

  TODO: License
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "led.h"

#include <stdbool.h>

/* Private defines -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
bool prvLedIsOn = false;

/* Private functions ---------------------------------------------------------*/
/* Functions -----------------------------------------------------------------*/
/**
 * @brief   Initializes the LED
 * @param   None
 * @retval  None
 */
void LED_Init(void)
{
  /* Enable clock for GPIOB */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

  /* Initialize as output push-pull */
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = LED_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(LED_PORT, &GPIO_InitStructure);

  GPIO_SetBits(LED_PORT, LED_PIN);
  prvLedIsOn = false;
}

/**
 * @brief   Turn on the LED
 * @param   None
 * @retval  None
 */
void LED_On()
{
  GPIO_ResetBits(LED_PORT, LED_PIN);
  prvLedIsOn = true;
}

/**
 * @brief   Turn off the LED
 * @param   None
 * @retval  None
 */
void LED_Off()
{
  GPIO_SetBits(LED_PORT, LED_PIN);
  prvLedIsOn = false;
}

/**
 * @brief   Toggle the LED
 * @param   None
 * @retval  None
 */
void LED_Toggle()
{
  if (prvLedIsOn)
    LED_Off();
  else
    LED_On();
}

/* Interrupt Handlers --------------------------------------------------------*/
