/**
 *******************************************************************************
 * @file    uart1.c
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-09-06
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
#include "uart1.h"

#include <string.h>

/** Private defines ----------------------------------------------------------*/
#define UART_PORT               (GPIOA)
#define UART_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOA_CLK_ENABLE()
#define UART_TX_PIN             (GPIO_PIN_9)
#define UART_RX_PIN             (GPIO_PIN_10)

#define UART_RX_BUFFER_SIZE     (128)

/** Private typedefs ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
static uint8_t prvRxBuffer[UART_RX_BUFFER_SIZE] = {0};
static uint8_t* prvRxBufferIn;
static uint8_t* prvRxBufferOut;
static uint32_t prvRxBufferCount;

static bool prvInitialized = false;

/** Private function prototypes ----------------------------------------------*/
/** Functions ----------------------------------------------------------------*/
/**
 * @brief   Initializes the UART
 * @param   None
 * @retval  None
 */
ErrorStatus UART1_Init()
{
  /* Make sure we only initialize it once */
  if (!prvInitialized)
  {
    /* Init GPIO */
    UART_GPIO_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Pin        = UART_TX_PIN;
    GPIO_InitStructure.Mode       = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Pull       = GPIO_NOPULL;
    GPIO_InitStructure.Speed      = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(UART_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin        = UART_RX_PIN;
    GPIO_InitStructure.Mode       = GPIO_MODE_AF_INPUT;
    HAL_GPIO_Init(UART_PORT, &GPIO_InitStructure);

    /* Init the RX Buffer */
    prvRxBufferIn = &prvRxBuffer[0];
    prvRxBufferOut = &prvRxBuffer[0];
    prvRxBufferCount = 0;

    /* Enable UART clock */
    __HAL_RCC_USART1_CLK_ENABLE();

    /* Init the UART */
    UART_Handle.Instance           = USART1;
    UART_Handle.Init.BaudRate      = 115200;
    UART_Handle.Init.WordLength    = UART_WORDLENGTH_8B;
    UART_Handle.Init.StopBits      = UART_STOPBITS_1;
    UART_Handle.Init.Parity        = UART_PARITY_NONE;
    UART_Handle.Init.Mode          = UART_MODE_TX_RX;
    UART_Handle.Init.HwFlowCtl     = UART_HWCONTROL_NONE;
    if (HAL_UART_Init(&UART_Handle) != HAL_OK)
      return ERROR;

    /* Enable the UART Data Register not empty Interrupt */
    __HAL_UART_ENABLE_IT(&UART_Handle, UART_IT_RXNE);

    /* NVIC for USART */
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 1);
    HAL_NVIC_EnableIRQ(USART1_IRQn);

    prvInitialized = true;

    return SUCCESS;
  }
  return ERROR;
}

/**
 * @brief   Get the number of bytes available in the RX buffer
 * @param   None
 * @retval  The number of bytes available
 */
uint32_t UART1_BytesAvailable()
{
  return prvRxBufferCount;
}

/**
 * @brief   Get an array of data from the buffer
 * @param   pStorage: Pointer to where data should be stored
 * @param   Size: The number of bytes to get
 * @retval  None
 */
void UART1_GetDataFromBuffer(uint8_t* pStorage, uint32_t Size)
{
  /* Sanity check */
  if (Size <= prvRxBufferCount)
  {
    while (Size != 0)
    {
      *pStorage = *prvRxBufferOut;
      pStorage++;
      /* Check for overflow */
      if (++prvRxBufferOut == &prvRxBuffer[UART_RX_BUFFER_SIZE])
        prvRxBufferOut = prvRxBuffer;
      Size--;
      prvRxBufferCount--;
    }
  }
}

/**
 * @brief   Get one byte from the RX buffer. User has to check data is available
 *          in the buffer first
 * @param   None
 * @retval  The byte or 0 if the buffer is empty.
 */
uint8_t UART1_GetByteFromBuffer()
{
  if (prvRxBufferCount != 0)
  {
    uint8_t temp = *prvRxBufferOut;
    /* Check for overflow */
    if (++prvRxBufferOut == &prvRxBuffer[UART_RX_BUFFER_SIZE])
      prvRxBufferOut = prvRxBuffer;
    prvRxBufferCount--;
    return temp;
  }
  else
    return 0;
}

/** Private functions .-------------------------------------------------------*/
/** Interrupt Handlers -------------------------------------------------------*/
/**
 * @brief
 * @param   None
 * @retval  None
 */
void UART1_DataReceivedHandler()
{
  uint8_t temp = (uint8_t)(UART_Handle.Instance->DR & (uint8_t)0x00FF);

  /* Only save to the buffer if there's room for it */
  if (prvRxBufferCount < UART_RX_BUFFER_SIZE)
  {
    *prvRxBufferIn = temp;
    /* Check for overflow */
    if (++prvRxBufferIn == &prvRxBuffer[UART_RX_BUFFER_SIZE])
      prvRxBufferIn = prvRxBuffer;
    /* Increment the count */
    prvRxBufferCount++;
  }
  /* TODO: Handle fault when buffer is full */
}

/**
 * @brief   Tx Transfer completed callback
 * @param   UartHandle: UART handle
 * @retval  None
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{

}

/**
 * @brief   UART error callbacks
 * @param   UartHandle: UART handle
 * @retval  None
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle)
{
//    Error_Handler();
}
