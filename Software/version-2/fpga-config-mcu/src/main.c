/**
 ******************************************************************************
 * @file    main.c
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-08-15
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

#include <stdio.h>
#include <stdlib.h>

/* STM32 Library */
#include "stm32f10x.h"

#include "millis.h"
#include "led.h"
#include "fpga_config.h"


/** ----- Main ---------------------------------------------------------------*/
int main()
{
  MILLIS_Init();
  LED_Init();
  FPGA_CONFIG_Init();

//  FPGA_CONFIG_Start();

  /* Main loop */
  while (1)
  {
      millisDelay(1000);
      LED_Toggle();
  }
}
