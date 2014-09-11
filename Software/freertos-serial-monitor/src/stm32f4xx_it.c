/**
 ******************************************************************************
 * @file	stm32f4xx_it.c
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-06-02
 * @brief
 ******************************************************************************
	Copyright (c) 2014 Hampus Sandberg.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation, either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"

#include "lcd_ra8875.h"
#include "ft5206.h"

#include "uart1_task.h"
#include "uart2_task.h"

/* Private defines -----------------------------------------------------------*/
/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void prvGetRegistersFromStack(uint32_t *pulFaultStackAddress);

/* Functions -----------------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
void prvGetRegistersFromStack(uint32_t *pulFaultStackAddress)
{
	/*
	 * These are volatile to try and prevent the compiler/linker optimising them
	 * away as the variables never actually get used.  If the debugger won't show the
	 * values of the variables, make them global my moving their declaration outside
	 * of this function.
	 */
	volatile uint32_t r0;
	volatile uint32_t r1;
	volatile uint32_t r2;
	volatile uint32_t r3;
	volatile uint32_t r12;
	volatile uint32_t lr; /* Link register. */
	volatile uint32_t pc; /* Program counter. */
	volatile uint32_t psr;/* Program status register. */

    r0 = pulFaultStackAddress[0];
    r1 = pulFaultStackAddress[1];
    r2 = pulFaultStackAddress[2];
    r3 = pulFaultStackAddress[3];

    r12 = pulFaultStackAddress[4];
    lr = pulFaultStackAddress[5];
    pc = pulFaultStackAddress[6];
    psr = pulFaultStackAddress[7];

    /* When the following line is hit, the variables contain the register values. */
    while (1);

    /* These lines help prevent getting warnings from compiler about unused variables */
    r0 = r1 = r2 = r3 = r12 = lr = pc = psr = 0;
    r0++;
}

/* Cortex-M4 Processor Exceptions Handlers  ----------------------------------*/
/**
 * @brief	This function handles NMI exception
 * @param	None
 * @retval	None
 */
void NMI_Handler(void)
{

}

/**
 * @brief	This function handles Hard Fault exception
 * @param	None
 * @retval	None
 */
void HardFault_Handler(void)
{
    __asm volatile
    (
        " tst lr, #4                                                \n"
        " ite eq                                                    \n"
        " mrseq r0, msp                                             \n"
        " mrsne r0, psp                                             \n"
        " ldr r1, [r0, #24]                                         \n"
        " ldr r2, handler2_address_const                            \n"
        " bx r2                                                     \n"
        " handler2_address_const: .word prvGetRegistersFromStack    \n"
    );
}

/**
 * @brief	This function handles Memory Manage exception
 * @param	None
 * @retval	None
 */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {

  }
}

/**
 * @brief	This function handles Bus Fault exception
 * @param	None
 * @retval	None
 */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {

  }
}

/**
 * @brief	This function handles Usage Fault exception
 * @param	None
 * @retval	None
 */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {

  }
}

/**
 * @brief	This function handles Debug Monitor exception
 * @param	None
 * @retval	None
 */
void DebugMon_Handler(void)
{

}

/* STM32F4xx Peripherals Interrupt Handlers   --------------------------------*/
void EXTI9_5_IRQHandler(void)
{
	/* Check CTP_INT Interrupt */
	if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_6) != RESET)
	{
		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_6);
		CTP_INT_Callback();
	}
}

void EXTI15_10_IRQHandler(void)
{
	/* Check LCD_WAIT Interrupt */
	if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_11) != RESET)
	{
		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_11);
		LCD_WAIT_Callback();
	}
	/* Check LCD_INT Interrupt */
	if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_12) != RESET)
	{
		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_12);
		LCD_INT_Callback();
	}
}

/* HAL Callback functions ----------------------------------------------------*/
/**
  * @brief  Tx Transfer completed callback
  * @param  UartHandle: UART handle.
  * @retval None
  */
void HAL_USART_TxCpltCallback(USART_HandleTypeDef *UsartHandle)
{
	if (UsartHandle->Instance == USART1)
	{
		uart1TxCpltCallback();
	}
	else if (UsartHandle->Instance == USART2)
	{
		uart2TxCpltCallback();
	}
}

/**
  * @brief  Rx Transfer completed callback
  * @param  UartHandle: UART handle
  * @retval None
  */
void HAL_USART_RxCpltCallback(USART_HandleTypeDef *UsartHandle)
{
	if (UsartHandle->Instance == USART1)
	{
		uart1RxCpltCallback();
	}
	else if (UsartHandle->Instance == USART2)
	{
		uart2RxCpltCallback();
	}
}

/**
  * @brief  UART error callback
  * @param  UartHandle: UART handle
  * @retval None
  */
 void HAL_USART_ErrorCallback(USART_HandleTypeDef *UsartHandle)
{
	if (UsartHandle->Instance == USART1)
	{
		uart1ErrorCallback();
	}
	else if (UsartHandle->Instance == USART2)
	{
		uart2ErrorCallback();
	}
}
