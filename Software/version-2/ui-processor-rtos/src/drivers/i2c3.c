/**
 *******************************************************************************
 * @file  i2c3.c
 * @author  Hampus Sandberg
 * @version 0.1
 * @date  2015-08-16
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
#include "i2c3.h"

#include <stdbool.h>

/** Private defines ----------------------------------------------------------*/
#define I2C_PERIPHERAL  (I2C3)
#define I2C_AF_GPIO     GPIO_AF4_I2C3

#define I2C_SCL_PORT              (GPIOA)
#define I2C_SCL_GPIO_CLK_ENABLE() __GPIOA_CLK_ENABLE()
#define I2C_SCL_PIN               (GPIO_PIN_8)

#define I2C_SDA_PORT              (GPIOC)
#define I2C_SDA_GPIO_CLK_ENABLE() __GPIOC_CLK_ENABLE()
#define I2C_SDA_PIN               (GPIO_PIN_9)

#define I2C_CLOCK_SPEED           (400000)

/** Private typedefs ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
static I2C_HandleTypeDef I2C_Handle = {
  .Instance             = I2C_PERIPHERAL,
  .Init.ClockSpeed      = I2C_CLOCK_SPEED,
  .Init.DutyCycle       = I2C_DUTYCYCLE_2,
  .Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT,
  .Init.DualAddressMode = I2C_DUALADDRESS_DISABLED,
  .Init.GeneralCallMode = I2C_GENERALCALL_DISABLED,
  .Init.NoStretchMode   = I2C_NOSTRETCH_DISABLED,
};

static SemaphoreHandle_t xSemaphore;
static bool prvInitialized = false;

/** Private function prototypes ----------------------------------------------*/

/** Functions ----------------------------------------------------------------*/
/**
 * @brief  Initializes the I2C
 * @param  None
 * @retval  None
 */
void I2C3_Init()
{
  /* Make sure we only initialize it once */
  if (!prvInitialized)
  {
    /* Mutex semaphore for mutual exclusion to the I2C3 device */
    xSemaphore = xSemaphoreCreateMutex();

    if (xSemaphoreTake(xSemaphore, 100) == pdTRUE)
    {
      /* I2C clock enable */
      __I2C3_CLK_ENABLE();

      /* I2C SCL configuration */
      I2C_SCL_GPIO_CLK_ENABLE();
      GPIO_InitTypeDef GPIO_InitStructure;
      GPIO_InitStructure.Pin        = I2C_SCL_PIN;
      GPIO_InitStructure.Mode       = GPIO_MODE_AF_OD;
      GPIO_InitStructure.Alternate  = I2C_AF_GPIO;
      GPIO_InitStructure.Pull       = GPIO_NOPULL;
      GPIO_InitStructure.Speed      = GPIO_SPEED_LOW;
      HAL_GPIO_Init(I2C_SCL_PORT, &GPIO_InitStructure);

      /* I2C SDA configuration */
      I2C_SDA_GPIO_CLK_ENABLE();
      GPIO_InitStructure.Pin        = I2C_SDA_PIN;
      GPIO_InitStructure.Mode       = GPIO_MODE_AF_OD;
      GPIO_InitStructure.Alternate  = I2C_AF_GPIO;
      GPIO_InitStructure.Pull       = GPIO_NOPULL;
      GPIO_InitStructure.Speed      = GPIO_SPEED_LOW;
      HAL_GPIO_Init(I2C_SDA_PORT, &GPIO_InitStructure);

      /* I2C Init */
      HAL_I2C_Init(&I2C_Handle);

      xSemaphoreGive(xSemaphore);
    }

    prvInitialized = true;
  }
}

/**
 * @brief  Transmits data as a master to a slave
 * @param  DevAddress: Address for the slave device
 * @param  pBuffer: Pointer to the buffer of data to send
 * @param  Size: Size of the buffer
 * @retval None
 */
void I2C3_Transmit(uint8_t DevAddress, uint8_t* pBuffer, uint16_t Size)
{
  /* Try to take the semaphore in case some other process is using the device */
  if (xSemaphoreTake(xSemaphore, 100) == pdTRUE)
  {
    HAL_I2C_Master_Transmit(&I2C_Handle, (uint16_t)(DevAddress << 1), pBuffer, Size, 500); /* TODO: Check timeout value */
    xSemaphoreGive(xSemaphore);
  }
}

/**
  * @brief  Transmits data as a master to a slave, used in ISR
  * @param  DevAddress: Address for the slave device
  * @param  pBuffer: Pointer to the buffer of data to send
  * @param  Size: Size of the buffer
  * @retval None
 */
void I2C3_TransmitFromISR(uint8_t DevAddress, uint8_t* pBuffer, uint16_t Size)
{
  /* Try to take the semaphore in case some other process is using the device */
  if (xSemaphoreTakeFromISR(xSemaphore, NULL) == pdTRUE)
  {
    HAL_I2C_Master_Transmit(&I2C_Handle, (uint16_t)(DevAddress << 1), pBuffer, Size, 500); /* TODO: Check timeout value */
    xSemaphoreGiveFromISR(xSemaphore, NULL);
  }
}

/**
  * @brief  Transmits data as a master to a slave
  * @param  DevAddress: Address for the slave device
  * @param  Data: Pointer to a buffer where data will be stored
  * @param  Size: Size of the amount of data to receive
  * @retval None
 */
void I2C3_Receive(uint8_t DevAddress, uint8_t* pBuffer, uint16_t Size)
{
  /* Try to take the semaphore in case some other process is using the device */
  if (xSemaphoreTake(xSemaphore, 100) == pdTRUE)
  {
    HAL_I2C_Master_Receive(&I2C_Handle, (uint16_t)(DevAddress << 1), pBuffer, Size, 500); /* TODO: Check timeout value */
    xSemaphoreGive(xSemaphore);
  }
}

/**
 * @brief  Transmits data as a master to a slave, used in ISR
 * @param  DevAddress: Address for the slave device
 * @param  Data: Pointer to a buffer where data will be stored
 * @param  Size: Size of the amount of data to receive
 * @retval  None
 */
void I2C3_ReceiveFromISR(uint8_t DevAddress, uint8_t* pBuffer, uint16_t Size)
{
  /* Try to take the semaphore in case some other process is using the device */
  if (xSemaphoreTakeFromISR(xSemaphore, NULL) == pdTRUE)
  {
    HAL_I2C_Master_Receive(&I2C_Handle, (uint16_t)(DevAddress << 1), pBuffer, Size, 500); /* TODO: Check timeout value */
    xSemaphoreGiveFromISR(xSemaphore, NULL);
  }
}

/** Interrupt Handlers -------------------------------------------------------*/
