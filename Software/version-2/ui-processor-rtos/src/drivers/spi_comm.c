/**
 *******************************************************************************
 * @file    spi_comm.c
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-08-16
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
#include "spi_comm.h"

#include <string.h>

/** Private defines ----------------------------------------------------------*/
#define COMM_SPI               (SPI5)
#define COMM_SPI_CLK_ENABLE()  __SPI5_CLK_ENABLE()
#define COMM_PORT              (GPIOF)
#define COMM_GPIO_CLK_ENABLE() __GPIOF_CLK_ENABLE()
#define COMM_CS_PIN            (GPIO_PIN_6)
#define COMM_SCK_PIN           (GPIO_PIN_7)
#define COMM_MISO_PIN          (GPIO_PIN_8)
#define COMM_MOSI_PIN          (GPIO_PIN_9)

/** SPI COMM Commands */
#define SPI_COMM_LAST_ADDRESS   (0x0)

/** Private typedefs ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
static SPI_HandleTypeDef SPI_Handle = {
  .Instance                 = COMM_SPI,
  .Init.Mode                = SPI_MODE_MASTER,
  .Init.Direction           = SPI_DIRECTION_2LINES,
  .Init.DataSize            = SPI_DATASIZE_8BIT,
  .Init.CLKPolarity         = SPI_POLARITY_LOW,
  .Init.CLKPhase            = SPI_PHASE_1EDGE,
  .Init.NSS                 = SPI_NSS_SOFT,
  .Init.BaudRatePrescaler   = SPI_BAUDRATEPRESCALER_2,
  .Init.FirstBit            = SPI_FIRSTBIT_MSB,
  .Init.TIMode              = SPI_TIMODE_DISABLED,
  .Init.CRCCalculation      = SPI_CRCCALCULATION_DISABLED,
  .Init.CRCPolynomial       = 1,
};

static uint32_t prvDeviceId = 0;
static SemaphoreHandle_t xSemaphore;
static bool prvInitialized = false;

/** Private function prototypes ----------------------------------------------*/
static inline void prvSPI_COMM_CS_LOW();
static inline void prvSPI_COMM_CS_HIGH();
static void prvSPI_COMM_WriteByte(uint32_t WriteAddress, uint8_t Byte);
static void prvSPI_COMM_WriteBytes(uint8_t* pBuffer, uint32_t WriteAddress, uint32_t NumByteToWrite);
static void prvSPI_COMM_WriteDisable();
static void prvSPI_COMM_WriteEnable();
static uint8_t prvSPI_COMM_SendReceiveByte(uint8_t Byte);
static void prvSPI_COMM_WaitForWriteEnd();

/** Functions ----------------------------------------------------------------*/
/**
 * @brief  Initializes the SPI COMM
 * @param  None
 * @retval  None
 */
ErrorStatus SPI_COMM_Init()
{
  /* Make sure we only initialize it once */
  if (!prvInitialized)
  {
    /* Mutex semaphore for mutual exclusion to the SPI Comm device */
    xSemaphore = xSemaphoreCreateMutex();

    /* Init GPIO */
    COMM_GPIO_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Pin        = COMM_SCK_PIN | COMM_MISO_PIN | COMM_MOSI_PIN;
    GPIO_InitStructure.Mode       = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Alternate  = GPIO_AF5_SPI5;
    GPIO_InitStructure.Pull       = GPIO_NOPULL;
    GPIO_InitStructure.Speed      = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(COMM_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin        = COMM_CS_PIN;
    GPIO_InitStructure.Mode       = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(COMM_PORT, &GPIO_InitStructure);
    /* Deselect the COMM */
    prvSPI_COMM_CS_HIGH();

    /* Init SPI */
    COMM_SPI_CLK_ENABLE();
    HAL_SPI_Init(&SPI_Handle);

    /* Read COMM identification */
    prvDeviceId = SPI_COMM_ReadID();
  }
  return ERROR;
}

/**
  * @brief  Reads COMM identification
  * @param  None
  * @retval COMM identification
  */
uint32_t SPI_COMM_ReadID()
{
  uint8_t byte[3];

//  /* Try to take the semaphore in case some other process is using the device */
//  if (xSemaphoreTake(xSemaphore, 100) == pdTRUE)
//  {
//    /* Select the COMM */
//    prvSPI_COMM_CS_LOW();
//
//    /* TODO: Read ID */
//
//    /* Deselect the COMM */
//    prvSPI_COMM_CS_HIGH();
//
//    /* Give back the semaphore */
//    xSemaphoreGive(xSemaphore);
//  }
  return (byte[0] << 16) | (byte[1] << 8) | byte[2];
}

/**
  * @brief  Write a buffer to the COMM
  * @note   Addresses to be written must be in the erased state
  * @param  pBuff: pointer to the buffer with data to write
  * @param  WriteAddress: start of COMM's internal address to write to
  * @param  NumByteToWrite: number of bytes to write to the COMM
  * @retval None
  */
void SPI_COMM_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddress, uint32_t NumByteToWrite)
{
//  /* Check address */
//  if (WriteAddress <= SPI_COMM_LAST_ADDRESS)
//  {
//    /* Try to take the semaphore in case some other process is using the device */
//    if (xSemaphoreTake(xSemaphore, 100) == pdTRUE)
//    {
//      prvSPI_COMM_WriteBytes(pBuffer, WriteAddress, NumByteToWrite);
//
//      /* Give back the semaphore */
//      xSemaphoreGive(xSemaphore);
//    }
//  }
}

/**
  * @brief  Write one byte to the COMM.
  * @note   Addresses to be written must be in the erased state
  * @param  WriteAddress: COMM's internal address to write to.
  * @param  Byte: the data to be written.
  * @retval None
  */
void SPI_COMM_WriteByte(uint32_t WriteAddress, uint8_t Byte)
{
//  /* Check address */
//  if (WriteAddress <= SPI_COMM_LAST_ADDRESS)
//  {
//    /* Try to take the semaphore in case some other process is using the device */
//    if (xSemaphoreTake(xSemaphore, 100) == pdTRUE)
//    {
//      prvSPI_COMM_WriteByte(WriteAddress, Byte);
//
//      /* Give back the semaphore */
//      xSemaphoreGive(xSemaphore);
//    }
//  }
}

/**
  * @brief  Write one byte to the COMM. Can be called from an ISR
  * @note   Addresses to be written must be in the erased state
  * @param  WriteAddress: COMM's internal address to write to.
  * @param  Byte: the data to be written.
  * @retval None
  */
void SPI_COMM_WriteByteFromISR(uint32_t WriteAddress, uint8_t Byte)
{
//  /* Check address */
//  if (WriteAddress <= SPI_COMM_LAST_ADDRESS)
//  {
//    /* Try to take the semaphore in case some other process is using the device */
//    if (xSemaphoreTakeFromISR(xSemaphore, NULL) == pdTRUE)
//    {
//      prvSPI_COMM_WriteByte(WriteAddress, Byte);
//
//      /* Give back the semaphore */
//      xSemaphoreGiveFromISR(xSemaphore, NULL);
//    }
//  }
}

/**
  * @brief  Reads a block of data from the COMM.
  * @param  pBuff: pointer to the buffer that receives the data read from the COMM.
  * @param  ReadAddress: COMM's internal address to read from.
  * @param  NumByteToRead: number of bytes to read from the COMM.
  * @retval None
  */
void SPI_COMM_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddress, uint32_t NumByteToRead)
{
//  /* Check address */
//  if (ReadAddress + NumByteToRead - 1 <= SPI_COMM_LAST_ADDRESS)
//  {
//    /* Try to take the semaphore in case some other process is using the device */
//    if (NumByteToRead != 0 && xSemaphoreTake(xSemaphore, 100) == pdTRUE)
//    {
//      /* Select the COMM */
//      prvSPI_COMM_CS_LOW();
//
//      /* TODO: Read data */
//
//      /* Deselect the COMM */
//      prvSPI_COMM_CS_HIGH();
//
//      /* Give back the semaphore */
//      xSemaphoreGive(xSemaphore);
//    }
//  }
}

/**
  * @brief  Erases the specified COMM sector
  * @param  SectorAddr: address of the sector to erase
  * @retval None
  */
ErrorStatus SPI_COMM_EraseSector(uint32_t SectorAddress)
{
//  /* Check address */
//  if (SectorAddress <= SPI_COMM_LAST_ADDRESS)
//  {
//    /* Try to take the semaphore in case some other process is using the device */
//    if (xSemaphoreTake(xSemaphore, 10000) == pdTRUE)
//    {
//      /* Enable the write access to the COMM */
//      prvSPI_COMM_WriteEnable();
//
//      /* Select the COMM: Chip Select low */
//      prvSPI_COMM_CS_LOW();
//
//      /* TODO: Erase data */
//
//      /* Give back the semaphore */
//      xSemaphoreGive(xSemaphore);
//
//      return SUCCESS;
//    }
//    else
//      return ERROR;
//  }
//  else
//    return ERROR;
}

/**
  * @brief  Erases the entire COMM
  * @param  None
  * @retval None
  */
void SPI_COMM_EraseChip()
{
//  /* Try to take the semaphore in case some other process is using the device */
//  if (xSemaphoreTake(xSemaphore, 100) == pdTRUE)
//  {
//    /* Enable the write access to the COMM */
//    prvSPI_COMM_WriteEnable();
//
//    /* Select the COMM */
//    prvSPI_COMM_CS_LOW();
//
//    /* TODO: Erase data */
//
//    /* Deselect the COMM */
//    prvSPI_COMM_CS_HIGH();
//
//    /* Wait till the end of Flash writing */
//    prvSPI_COMM_WaitForWriteEnd();
//
//    /* Give back the semaphore */
//    xSemaphoreGive(xSemaphore);
//  }
}

/**
  * @brief  Return the status of if the SPI COMM is intialized or not
  * @param  None
  * @retval true: The device is initialized
  * @retval false: The device is not initialized
  */
bool SPI_COMM_Initialized()
{
  return prvInitialized;
}

/** Private functions .-------------------------------------------------------*/
/**
 * @brief  Pull the CS pin LOW
 * @param  None
 * @retval  None
 */
static inline void prvSPI_COMM_CS_LOW()
{
  HAL_GPIO_WritePin(COMM_PORT, COMM_CS_PIN, GPIO_PIN_RESET);
}

/**
 * @brief  Pull the CS pin HIGH
 * @param  None
 * @retval  None
 */
static inline void prvSPI_COMM_CS_HIGH()
{
  HAL_GPIO_WritePin(COMM_PORT, COMM_CS_PIN, GPIO_PIN_SET);
}

/**
  * @brief  Write one byte to the COMM.
  * @note   Addresses to be written must be in the erased state
  * @param  WriteAddress: COMM's internal address to write to.
  * @param  Byte: the data to be written.
  * @retval None
  */
static void prvSPI_COMM_WriteByte(uint32_t WriteAddress, uint8_t Byte)
{
//  /* Enable the write access to the COMM */
//  prvSPI_COMM_WriteEnable();
//
//  /* Select the COMM */
//  prvSPI_COMM_CS_LOW();
//  /* Send "Byte Program" instruction */
//  prvSPI_COMM_SendReceiveByte(SPI_COMM_CMD_WRITE);
//  /* Send WriteAddress high, medium and low nibble address byte to write to */
//  prvSPI_COMM_SendReceiveByte((WriteAddress & 0xFF0000) >> 16);
//  prvSPI_COMM_SendReceiveByte((WriteAddress & 0xFF00) >> 8);
//  prvSPI_COMM_SendReceiveByte(WriteAddress & 0xFF);
//  /* Send the byte */
//  prvSPI_COMM_SendReceiveByte(Byte);
//  /* Deselect the COMM */
//  prvSPI_COMM_CS_HIGH();
//  /* Wait till the end of Flash writing */
//  prvSPI_COMM_WaitForWriteEnd();
}

/**
  * @brief  Writes more than one byte to the COMM.
  * @note   The address must be even and the number of bytes must be a multiple
  *         of two.
  * @note   Addresses to be written must be in the erased state
  * @param  pBuffer: pointer to the buffer containing the data to be written
  *         to the COMM.
  * @param  WriteAddress: COMM's internal address to write to, must be even.
  * @param  NumByteToWrite: number of bytes to write to the COMM, must be even.
  * @retval None
  */
static void prvSPI_COMM_WriteBytes(uint8_t* pBuffer, uint32_t WriteAddress, uint32_t NumByteToWrite)
{
//  /* Enable the write access to the COMM */
//  prvSPI_COMM_WriteEnable();
//
//  /* Select the COMM */
//  prvSPI_COMM_CS_LOW();
//
//  /* Send write command */
//  prvSPI_COMM_SendReceiveByte(SPI_COMM_CMD_WRITE);
//  /* Send WriteAddress high, medium and low nibble address byte to write to */
//  prvSPI_COMM_SendReceiveByte((WriteAddress & 0xFF0000) >> 16);
//  prvSPI_COMM_SendReceiveByte((WriteAddress & 0xFF00) >> 8);
//  prvSPI_COMM_SendReceiveByte(WriteAddress & 0xFF);
//
//  /* While there is data to be written to the COMM */
//  while (NumByteToWrite)
//  {
//    /* Send one byte */
//    prvSPI_COMM_SendReceiveByte(*pBuffer++);
//    /* Update NumByteToWrite */
//    NumByteToWrite--;
//  }
//
//  /* Deselect the COMM */
//  prvSPI_COMM_CS_HIGH();
//  /* Wait till the end of Flash writing */
//  prvSPI_COMM_WaitForWriteEnd();
//
//  /* Disable the write access to the COMM */
//  prvSPI_COMM_WriteDisable();
}

/**
  * @brief  Disables the write access to the COMM
  * @param  None
  * @retval None
  */
static void prvSPI_COMM_WriteDisable()
{
//  /* Select the COMM */
//  prvSPI_COMM_CS_LOW();
//
//  /* Send "Write Disable" instruction */
//  prvSPI_COMM_SendReceiveByte(SPI_COMM_CMD_WRDI);
//
//  /* Deselect the COMM */
//  prvSPI_COMM_CS_HIGH();
}

/**
  * @brief  Enables the write access to the COMM
  * @param  None
  * @retval None
  */
static void prvSPI_COMM_WriteEnable()
{
//  /* Select the COMM */
//  prvSPI_COMM_CS_LOW();
//
//  /* Send "Write Enable" instruction */
//  prvSPI_COMM_SendReceiveByte(SPI_COMM_CMD_WREN);
//
//  /* Deselect the COMM */
//  prvSPI_COMM_CS_HIGH();
}

/**
  * @brief  Send one byte to the SPI COMM and return the byte received back
  * @param  Byte: The byte to send
  * @retval The byte received from the SPI COMM
  */
static uint8_t prvSPI_COMM_SendReceiveByte(uint8_t Byte)
{
  /* TODO: Do this RAW instead of using HAL??? A lot of overhead in HAL */
  uint8_t rxByte;
  HAL_SPI_TransmitReceive(&SPI_Handle, &Byte, &rxByte, 1, 10);  /* TODO: Check timeout */
  return rxByte;
}

/**
  * @brief  Polls the status of the Write In Progress (WIP) flag in the COMM's
  *         status register and loop until write operation has completed.
  * @param  None
  * @retval None
  */
static void prvSPI_COMM_WaitForWriteEnd()
{
//  uint8_t flashStatus = 0;
//
//  /* Select the COMM */
//  prvSPI_COMM_CS_LOW();
//
//  /* Send "Read Status Register" instruction */
//  prvSPI_COMM_SendReceiveByte(SPI_COMM_CMD_RDSR);
//
//  /* Loop as long as the memory is busy with a write cycle */
//  do
//  {
//    /* TODO: Handle blocking */
//    /* Send a dummy byte to generate the clock needed by the COMM
//    and put the value of the status register in COMM_Status variable */
//    flashStatus = prvSPI_COMM_SendReceiveByte(SPI_COMM_DUMMY_BYTE);
//  } while ((flashStatus & SPI_COMM_WIP_FLAG) == SET); /* Write in progress */
//
//  /* Deselect the COMM */
//  prvSPI_COMM_CS_HIGH();
}

/** Interrupt Handlers -------------------------------------------------------*/
/**
  * @brief  TxRx Transfer completed callback.
  * @param  hspi: SPI handle
  * @retval None
  */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{

}

/**
  * @brief  SPI error callbacks
  * @param  hspi: SPI handle
  * @retval None
  */
 void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
   /* TODO: Manage errors */
}
