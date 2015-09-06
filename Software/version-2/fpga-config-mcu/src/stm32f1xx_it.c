/**
 *******************************************************************************
 * @file    stm32f1xx_it.c
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-09-05
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
#include "stm32f1xx_it.h"

#include "uart1.h"

/** Private defines ----------------------------------------------------------*/
/** Private typedefs ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
/** Private function prototypes ----------------------------------------------*/
/** Functions ----------------------------------------------------------------*/

/** Private functions --------------------------------------------------------*/

/** Cortex-M3 Processor Exceptions Handlers  ---------------------------------*/
/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  HAL_IncTick();
}

/** STM32F4xx Peripherals Interrupt Handlers   -------------------------------*/
/**
  * @brief  This function handles UART interrupt request.
  * @param  None
  * @retval None
  */
void USART1_IRQHandler(void)
{
  /* Check if it's a RX interrupt */
  uint32_t tmp_flag = 0, tmp_it_source = 0;
  tmp_flag = __HAL_UART_GET_FLAG(&UART_Handle, UART_FLAG_RXNE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(&UART_Handle, UART_IT_RXNE);
  if ((tmp_flag != RESET) && (tmp_it_source != RESET))
  {
    UART1_DataReceivedHandler();
  }
  /* Otherwise call the HAL IRQ handler */
  else
    HAL_UART_IRQHandler(&UART_Handle);
}

/** HAL Callback functions ---------------------------------------------------*/
