/**
 ******************************************************************************
 * @file  main.c
 * @author  Hampus Sandberg
 * @version 0.1
 * @date  2015-08-15
 * @brief
 ******************************************************************************
  Copyright (c) 2015 Hampus Sandberg.

  TODO: License
 ******************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>

/* STM32 Library */
#include "stm32f10x.h"

#include "millis.h"
#include "led.h"


/* ----- Main -------------------------------------------------------------- */
int main()
{
  MILLIS_Init();
  LED_Init();

  /* Main loop */
  while (1)
  {
      millisDelay(1000);
      LED_Toggle();
  }
}
