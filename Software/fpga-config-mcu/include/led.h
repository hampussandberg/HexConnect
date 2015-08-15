/**
 ******************************************************************************
 * @file    led.h
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-08-15
 * @brief   Manage the LED
 ******************************************************************************
  Copyright (c) 2015 Hampus Sandberg.

  TODO: License
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef LED_H_
#define LED_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/* Defines -------------------------------------------------------------------*/
#define LED_PORT (GPIOB)
#define LED_PIN (GPIO_Pin_9)

/* Typedefs ------------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/
void LED_Init();
void LED_On();
void LED_Off();
void LED_Toggle();

#endif /* LED_H_ */
