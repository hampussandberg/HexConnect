/**
 *******************************************************************************
 * @file    i2c_eeprom.c
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-08-16
 * @brief   24LC256T, 256kbit I2C EEPROM
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
#include "i2c_eeprom.h"

#include "i2c3.h"

/** Private defines ----------------------------------------------------------*/
#define I2C_EEPROM_MS_BETWEEN_WRITES  5
#define I2C_EEPROM_BUS_ADDRESS        0x50

#define I2C_EEPROM_WP_PORT              (GPIOC)
#define I2C_EEPROM_WP_GPIO_CLK_ENABLE() __GPIOC_CLK_ENABLE()
#define I2C_EEPROM_WP_PIN               (GPIO_PIN_8)

#define I2C_EEPROM_SIZE_BYTES         (0x8000)  /* 256kbit = 32kByte*/
#define I2C_EEPROM_LAST_ADDRESS       (I2C_EEPROM_SIZE_BYTES-1)

/** Private typedefs ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
static SemaphoreHandle_t xSemaphore;
static bool prvInitialized = false;
static bool prvWriteProtected = false;
portTickType xNextWriteTime;

/** Private function prototypes ----------------------------------------------*/

/** Functions ----------------------------------------------------------------*/
/**
 * @brief  Initializes the EEPROM
 * @param  None
 * @retval None
 */
ErrorStatus I2C_EEPROM_Init()
{
  /* Make sure we only initialize it once */
  if (!prvInitialized)
  {
    /* I2C */
    I2C3_Init();

    /* EEPROM WP configuration */
    I2C_EEPROM_WP_GPIO_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Pin        = I2C_EEPROM_WP_PIN;
    GPIO_InitStructure.Mode       = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull       = GPIO_NOPULL;
    GPIO_InitStructure.Speed      = GPIO_SPEED_LOW;
    HAL_GPIO_Init(I2C_EEPROM_WP_PORT, &GPIO_InitStructure);
    I2C_EEPROM_DisableWriteProtection();

    /* Mutex semaphore for mutual exclusion to the EEPROM device */
    xSemaphore = xSemaphoreCreateMutex();

    /* Initialize xNextWriteTime - this only needs to be done once. */
    xNextWriteTime = xTaskGetTickCount();

    prvInitialized = true;

    return SUCCESS;
  }
  else
    return ERROR;
}

/**
  * @brief  Return the status of if the EEPROM is intialized or not
  * @param  None
  * @retval true: The device is initialized
  * @retval false: The device is not initialized
  */
bool I2C_EEPROM_Initialized()
{
  return prvInitialized;
}

/**
 * @brief  Enable the write protection on the EEPROM
 * @param  None
 * @retval None
 */
void I2C_EEPROM_EnableWriteProtection()
{
  HAL_GPIO_WritePin(I2C_EEPROM_WP_PORT, I2C_EEPROM_WP_PIN, GPIO_PIN_SET);
  prvWriteProtected = true;
}

/**
 * @brief  Disable the write protection on the EEPROM
 * @param  None
 * @retval None
 */
void I2C_EEPROM_DisableWriteProtection()
{
  HAL_GPIO_WritePin(I2C_EEPROM_WP_PORT, I2C_EEPROM_WP_PIN, GPIO_PIN_RESET);
  prvWriteProtected = false;
}

/**
  * @brief  Write one byte to the EEPROM.
  * @param  WriteAddress: EEPROM's internal address to write to.
  * @param  Byte: the data to be written.
  * @retval None
  */
void I2C_EEPROM_WriteByte(uint32_t WriteAddress, uint8_t Byte)
{
  if (WriteAddress <= I2C_EEPROM_LAST_ADDRESS && !prvWriteProtected)
  {
    /* Try to take the semaphore in case some other process is using the device */
    if (xSemaphoreTake(xSemaphore, 100) == pdTRUE)
    {
      /* Wait until some time has passed since the last write */
      vTaskDelayUntil(&xNextWriteTime, I2C_EEPROM_MS_BETWEEN_WRITES / portTICK_PERIOD_MS);

      /* Transmit the address + data byte*/
      uint8_t tempData[3] = {(WriteAddress >> 8) & 0x7F, WriteAddress & 0xFF, Byte};
      I2C3_Transmit(I2C_EEPROM_BUS_ADDRESS, tempData, 3);

      /* Give back the semaphore */
      xSemaphoreGive(xSemaphore);
    }
  }
}

/**
  * @brief  Write one byte to the EEPROM. Can be called from an ISR
  * @param  WriteAddress: EEPROM's internal address to write to.
  * @param  Byte: the data to be written.
  * @retval None
  */
void I2C_EEPROM_WriteByteFromISR(uint32_t WriteAddress, uint8_t Byte)
{
  if (WriteAddress <= I2C_EEPROM_LAST_ADDRESS && !prvWriteProtected)
  {
    /* Try to take the semaphore in case some other process is using the device */
    if (xSemaphoreTakeFromISR(xSemaphore, NULL) == pdTRUE)
    {
      /* Wait until some time has passed since the last write */
      vTaskDelayUntil(&xNextWriteTime, I2C_EEPROM_MS_BETWEEN_WRITES / portTICK_PERIOD_MS);

      /* Transmit the address + data byte*/
      uint8_t tempData[3] = {(WriteAddress >> 8) & 0x7F, WriteAddress & 0xFF, Byte};
      I2C3_Transmit(I2C_EEPROM_BUS_ADDRESS, tempData, 3);

      /* Give back the semaphore */
      xSemaphoreGive(xSemaphore);
    }
  }
}

/**
  * @brief  Read one byte from the EEPROM
  * @param  ReadAddress: Address where the read should be done
  * @retval The data at the address
  * @retval ERROR: The address is invalid
  */
uint8_t I2C_EEPROM_ReadByte(uint32_t ReadAddress)
{
  if (ReadAddress <= I2C_EEPROM_LAST_ADDRESS)
  {
    /* Try to take the semaphore in case some other process is using the device */
    if (xSemaphoreTake(xSemaphore, 100) == pdTRUE)
    {
      /* Wait until some time has passed since the last write */
      vTaskDelayUntil(&xNextWriteTime, I2C_EEPROM_MS_BETWEEN_WRITES / portTICK_PERIOD_MS);

      /* Transmit the address */
      uint8_t tempData[2] = {(ReadAddress >> 8) & 0x7F, ReadAddress & 0xFF};
      I2C3_Transmit(I2C_EEPROM_BUS_ADDRESS, tempData, 2);

      /* Read the data byte */
      uint8_t readData = 0;
      I2C3_Receive(I2C_EEPROM_BUS_ADDRESS, &readData, 1);

      /* Give back the semaphore */
      xSemaphoreGive(xSemaphore);

      return readData;
    }
    else
      return ERROR;
  }
  else
    return ERROR;
}

/**
  * @brief  Read one byte from the EEPROM. Can be called from an ISR
  * @param  ReadAddress: Address where the read should be done
  * @retval The data at the address
  * @retval ERROR: The address is invalid
  */
uint8_t I2C_EEPROM_ReadByteFromISR(uint32_t ReadAddress)
{
  if (ReadAddress <= I2C_EEPROM_LAST_ADDRESS)
  {
    /* Try to take the semaphore in case some other process is using the device */
    if (xSemaphoreTakeFromISR(xSemaphore, NULL) == pdTRUE)
    {
      /* Wait until some time has passed since the last write */
      vTaskDelayUntil(&xNextWriteTime, I2C_EEPROM_MS_BETWEEN_WRITES / portTICK_PERIOD_MS);

      /* Transmit the address */
      uint8_t tempData[2] = {(ReadAddress >> 8) & 0x7F, ReadAddress & 0xFF};
      I2C3_Transmit(I2C_EEPROM_BUS_ADDRESS, tempData, 2);

      /* Read the data byte */
      uint8_t readData = 0;
      I2C3_Receive(I2C_EEPROM_BUS_ADDRESS, &readData, 1);

      /* Give back the semaphore */
      xSemaphoreGive(xSemaphore);

      return readData;
    }
    else
      return ERROR;
  }
  else
    return ERROR;
}

/** Private functions .-------------------------------------------------------*/
/** Interrupt Handlers -------------------------------------------------------*/
