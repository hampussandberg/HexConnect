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
#include "stm32f1xx_hal.h"

#include "led.h"
#include "fpga_config.h"
#include "spi_flash.h"
#include "spi2.h"
#include "uart1.h"
#include "uart_comm.h"


/** ----- Main ---------------------------------------------------------------*/
int main()
{
  LED_Init();
  LED_SetBlinkPeriod(1000);
  FPGA_CONFIG_Init();
  SPI_FLASH_Init();
  UART1_Init();

  SPI2_InitGpio();

  LED_SetBlinkPeriod(50);

  /* Try to configure the FPGA with the first bit file */
  if (FPGA_CONFIG_Start(1) != SUCCESS)
  {
    /* No valid bit file at first position */
  }

  LED_SetBlinkPeriod(1000);

  /* Main loop */
  while (1)
  {
    /* If there is data available we should handle it */
    while (UART1_BytesAvailable())
    {
      UART_COMM_HandleReceivedByte(UART1_GetByteFromBuffer());
    }
  }
}
