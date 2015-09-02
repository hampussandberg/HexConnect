/**
 *******************************************************************************
 * @file    spi_flash.c
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-08-15
 * @brief   Driver for a S25FL116K0XMFI011 16Mbit SPI FLASH
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
#include "spi_flash.h"

#include <string.h>

/** Private defines ----------------------------------------------------------*/
#define FLASH_SPI               (SPI4)
#define FLASH_SPI_CLK_ENABLE()  __SPI4_CLK_ENABLE()
#define FLASH_PORT              (GPIOE)
#define FLASH_GPIO_CLK_ENABLE() __GPIOE_CLK_ENABLE()
#define FLASH_CS_PIN            (GPIO_PIN_4)
#define FLASH_SCK_PIN           (GPIO_PIN_2)
#define FLASH_MISO_PIN          (GPIO_PIN_5)
#define FLASH_MOSI_PIN          (GPIO_PIN_6)

/** SPI FLASH Commands */
#define SPI_FLASH_CMD_RDSR      (0x05)  /* Read Status Register */
#define SPI_FLASH_CMD_WRSR      (0x01)  /* Write Status Register */
#define SPI_FLASH_CMD_EWSR      (0x50)  /* Write Enable Status */

#define SPI_FLASH_CMD_WRDI      (0x04)  /* Write Disable */
#define SPI_FLASH_CMD_WREN      (0x06)  /* Write Enable */
#define SPI_FLASH_CMD_READ      (0x03)  /* Read Data Bytes */
#define SPI_FLASH_CMD_WRITE     (0x02)  /* Byte Program / Page program */
#define SPI_FLASH_CMD_SECTOR_ERASE  (0x20)  /* 4 KB Sector Erase instruction */
#define SPI_FLASH_CMD_BLOCK_ERASE   (0xD8)  /* 64 KB Block Erase instruction */
#define SPI_FLASH_CMD_CHIP_ERASE    (0xC7)  /* Chip Erase instruction */
#define SPI_FLASH_CMD_RDID      (0x9F)  /* JEDEC ID Read */

#define SPI_FLASH_DUMMY_BYTE    (0xFF)
#define SPI_FLASH_WIP_FLAG      (0x01)  /* Write In Progress (WIP) flag */


#define SPI_FLASH_S25FL116K0XMFI011_ID  (0x014015)  /* Device ID for the S25FL116K0XMFI011 */
#define SPI_FLASH_SIZE_BYTES            (0x200000)  /* 16Mbit = 2MByte*/
#define SPI_FLASH_LAST_ADDRESS          (SPI_FLASH_SIZE_BYTES-1)

#define SPI_FLASH_SECTOR_CLEAN_CHECK_SIZE    (128)
#define SPI_FLASH_PAGE_SIZE   (256)

/** Private typedefs ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
static SPI_HandleTypeDef SPI_Handle = {
  .Instance                 = FLASH_SPI,
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
static inline void prvSPI_FLASH_CS_LOW();
static inline void prvSPI_FLASH_CS_HIGH();
static void prvSPI_FLASH_WriteByte(uint32_t WriteAddress, uint8_t Byte);
static void prvSPI_FLASH_WriteBytes(uint8_t* pBuffer, uint32_t WriteAddress, uint32_t NumByteToWrite);
static void prvSPI_FLASH_WriteUpToOnePage(uint8_t* pBuffer, uint32_t WriteAddress, uint32_t NumByteToWrite);
static void prvSPI_FLASH_WriteDisable();
static void prvSPI_FLASH_WriteEnable();
static uint8_t prvSPI_FLASH_SendReceiveByte(uint8_t Byte);
static void prvSPI_FLASH_WaitForWriteEnd();

/** Functions ----------------------------------------------------------------*/
/**
 * @brief  Initializes the SPI FLASH
 * @param  None
 * @retval  None
 */
ErrorStatus SPI_FLASH_Init()
{
  /* Make sure we only initialize it once */
  if (!prvInitialized)
  {
    /* Mutex semaphore for mutual exclusion to the SPI Flash device */
    xSemaphore = xSemaphoreCreateMutex();

    /* Init GPIO */
    FLASH_GPIO_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Pin        = FLASH_SCK_PIN | FLASH_MISO_PIN | FLASH_MOSI_PIN;
    GPIO_InitStructure.Mode       = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Alternate  = GPIO_AF5_SPI4;
    GPIO_InitStructure.Pull       = GPIO_NOPULL;
    GPIO_InitStructure.Speed      = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(FLASH_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin        = FLASH_CS_PIN;
    GPIO_InitStructure.Mode       = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(FLASH_PORT, &GPIO_InitStructure);
    /* Deselect the FLASH */
    prvSPI_FLASH_CS_HIGH();

    /* Init SPI */
    FLASH_SPI_CLK_ENABLE();
    HAL_SPI_Init(&SPI_Handle);

    /* Read FLASH identification */
    prvDeviceId = SPI_FLASH_ReadID();

    if (prvDeviceId == SPI_FLASH_S25FL116K0XMFI011_ID)
    {
      /* Select the FLASH */
      prvSPI_FLASH_CS_LOW();
      /* Send "Write Enable Status" instruction */
      prvSPI_FLASH_SendReceiveByte(SPI_FLASH_CMD_EWSR);
      /* Deselect the FLASH */
      prvSPI_FLASH_CS_HIGH();

      /* Select the FLASH */
      prvSPI_FLASH_CS_LOW();
      /* Send "Write Status Register" instruction and set all bits to 0 */
      prvSPI_FLASH_SendReceiveByte(SPI_FLASH_CMD_WRSR);
      prvSPI_FLASH_SendReceiveByte(0x00);
      prvSPI_FLASH_SendReceiveByte(0x00);
      /* Deselect the FLASH */
      prvSPI_FLASH_CS_HIGH();

      prvInitialized = true;

      return SUCCESS;
    }
  }
  return ERROR;
}

/**
  * @brief  Reads FLASH identification
  * @param  None
  * @retval FLASH identification
  */
uint32_t SPI_FLASH_ReadID()
{
  uint8_t byte[3];

  /* Try to take the semaphore in case some other process is using the device */
  if (xSemaphoreTake(xSemaphore, 100) == pdTRUE)
  {
    /* Select the FLASH */
    prvSPI_FLASH_CS_LOW();

    /* Send "JEDEC ID Read" instruction */
    prvSPI_FLASH_SendReceiveByte(SPI_FLASH_CMD_RDID);

    /* Read three bytes from the FLASH */
    byte[0] = prvSPI_FLASH_SendReceiveByte(SPI_FLASH_DUMMY_BYTE);
    byte[1] = prvSPI_FLASH_SendReceiveByte(SPI_FLASH_DUMMY_BYTE);
    byte[2] = prvSPI_FLASH_SendReceiveByte(SPI_FLASH_DUMMY_BYTE);

    /* Deselect the FLASH */
    prvSPI_FLASH_CS_HIGH();

    /* Give back the semaphore */
    xSemaphoreGive(xSemaphore);
  }
  return (byte[0] << 16) | (byte[1] << 8) | byte[2];
}

/**
  * @brief  Write a buffer to the FLASH
  * @note   Addresses to be written must be in the erased state
  * @param  pBuff: pointer to the buffer with data to write
  * @param  WriteAddress: start of FLASH's internal address to write to
  * @param  NumByteToWrite: number of bytes to write to the FLASH
  * @retval None
  */
void SPI_FLASH_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddress, uint32_t NumByteToWrite)
{
  /* Check address */
  if (WriteAddress <= SPI_FLASH_LAST_ADDRESS)
  {
    /* Try to take the semaphore in case some other process is using the device */
    if (xSemaphoreTake(xSemaphore, 100) == pdTRUE)
    {
      prvSPI_FLASH_WriteBytes(pBuffer, WriteAddress, NumByteToWrite);

      /* Give back the semaphore */
      xSemaphoreGive(xSemaphore);
    }
  }
}

/**
  * @brief  Write one byte to the FLASH.
  * @note   Addresses to be written must be in the erased state
  * @param  WriteAddress: FLASH's internal address to write to.
  * @param  Byte: the data to be written.
  * @retval None
  */
void SPI_FLASH_WriteByte(uint32_t WriteAddress, uint8_t Byte)
{
  /* Check address */
  if (WriteAddress <= SPI_FLASH_LAST_ADDRESS)
  {
    /* Try to take the semaphore in case some other process is using the device */
    if (xSemaphoreTake(xSemaphore, 100) == pdTRUE)
    {
      prvSPI_FLASH_WriteByte(WriteAddress, Byte);

      /* Give back the semaphore */
      xSemaphoreGive(xSemaphore);
    }
  }
}

/**
  * @brief  Write one byte to the FLASH. Can be called from an ISR
  * @note   Addresses to be written must be in the erased state
  * @param  WriteAddress: FLASH's internal address to write to.
  * @param  Byte: the data to be written.
  * @retval None
  */
void SPI_FLASH_WriteByteFromISR(uint32_t WriteAddress, uint8_t Byte)
{
  /* Check address */
  if (WriteAddress <= SPI_FLASH_LAST_ADDRESS)
  {
    /* Try to take the semaphore in case some other process is using the device */
    if (xSemaphoreTakeFromISR(xSemaphore, NULL) == pdTRUE)
    {
      prvSPI_FLASH_WriteByte(WriteAddress, Byte);

      /* Give back the semaphore */
      xSemaphoreGiveFromISR(xSemaphore, NULL);
    }
  }
}

/**
  * @brief  Reads a block of data from the FLASH.
  * @param  pBuff: pointer to the buffer that receives the data read from the FLASH.
  * @param  ReadAddress: FLASH's internal address to read from.
  * @param  NumByteToRead: number of bytes to read from the FLASH.
  * @retval None
  */
void SPI_FLASH_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddress, uint32_t NumByteToRead)
{
  /* Check address */
  if (ReadAddress + NumByteToRead - 1 <= SPI_FLASH_LAST_ADDRESS)
  {
    /* Try to take the semaphore in case some other process is using the device */
    if (NumByteToRead != 0 && xSemaphoreTake(xSemaphore, 100) == pdTRUE)
    {
      /* Select the FLASH */
      prvSPI_FLASH_CS_LOW();

      /* Send "Read from Memory " instruction */
      prvSPI_FLASH_SendReceiveByte(SPI_FLASH_CMD_READ);

      /* Send ReadAddr high nibble address byte to read from */
      prvSPI_FLASH_SendReceiveByte((ReadAddress & 0xFF0000) >> 16);
      /* Send ReadAddr medium nibble address byte to read from */
      prvSPI_FLASH_SendReceiveByte((ReadAddress& 0xFF00) >> 8);
      /* Send ReadAddr low nibble address byte to read from */
      prvSPI_FLASH_SendReceiveByte(ReadAddress & 0xFF);

      /* While there is data to be read */
      while (NumByteToRead)
      {
        /* Read a byte from the FLASH and point to the next location */
        *pBuffer++ = prvSPI_FLASH_SendReceiveByte(SPI_FLASH_DUMMY_BYTE);
        /* Decrement NumByteToRead */
        NumByteToRead--;
      }

      /* Deselect the FLASH */
      prvSPI_FLASH_CS_HIGH();

      /* Give back the semaphore */
      xSemaphoreGive(xSemaphore);
    }
  }
}

/**
  * @brief  Erases the specified FLASH sector
  * @param  SectorAddr: address of the sector to erase
  * @retval None
  */
ErrorStatus SPI_FLASH_EraseSector(uint32_t SectorAddress)
{
  /* Check address */
  if (SectorAddress <= SPI_FLASH_LAST_ADDRESS)
  {
    /* Try to take the semaphore in case some other process is using the device */
    if (xSemaphoreTake(xSemaphore, 10000) == pdTRUE)
    {
      /* Enable the write access to the FLASH */
      prvSPI_FLASH_WriteEnable();

      /* Sector Erase */
      /* Select the FLASH: Chip Select low */
      prvSPI_FLASH_CS_LOW();

      /* Send Sector Erase instruction */
      prvSPI_FLASH_SendReceiveByte(SPI_FLASH_CMD_SECTOR_ERASE);

      /* Send SectorAddr high nibble address byte */
      prvSPI_FLASH_SendReceiveByte((SectorAddress & 0xFF0000) >> 16);
      /* Send SectorAddr medium nibble address byte */
      prvSPI_FLASH_SendReceiveByte((SectorAddress & 0xFF00) >> 8);
      /* Send SectorAddr low nibble address byte */
      prvSPI_FLASH_SendReceiveByte(SectorAddress & 0xFF);
      /* Deselect the FLASH: Chip Select high */
      prvSPI_FLASH_CS_HIGH();

      /* Wait till the end of Flash writing */
      prvSPI_FLASH_WaitForWriteEnd();

      /* Give back the semaphore */
      xSemaphoreGive(xSemaphore);

      return SUCCESS;
    }
    else
      return ERROR;
  }
  else
    return ERROR;
}

/**
  * @brief  Erases the entire FLASH
  * @param  None
  * @retval None
  */
void SPI_FLASH_EraseChip()
{
  /* Try to take the semaphore in case some other process is using the device */
  if (xSemaphoreTake(xSemaphore, 100) == pdTRUE)
  {
    /* Enable the write access to the FLASH */
    prvSPI_FLASH_WriteEnable();

    /* Bulk Erase */
    /* Select the FLASH */
    prvSPI_FLASH_CS_LOW();
    /* Send Bulk Erase instruction  */
    prvSPI_FLASH_SendReceiveByte(SPI_FLASH_CMD_CHIP_ERASE);
    /* Deselect the FLASH */
    prvSPI_FLASH_CS_HIGH();

    /* Wait till the end of Flash writing */
    prvSPI_FLASH_WaitForWriteEnd();

    /* Give back the semaphore */
    xSemaphoreGive(xSemaphore);
  }
}

/**
  * @brief  Return the status of if the SPI FLASH is intialized or not
  * @param  None
  * @retval true: The device is initialized
  * @retval false: The device is not initialized
  */
bool SPI_FLASH_Initialized()
{
  return prvInitialized;
}

/** Private functions .-------------------------------------------------------*/
/**
 * @brief  Pull the CS pin LOW
 * @param  None
 * @retval  None
 */
static inline void prvSPI_FLASH_CS_LOW()
{
  HAL_GPIO_WritePin(FLASH_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);
}

/**
 * @brief  Pull the CS pin HIGH
 * @param  None
 * @retval  None
 */
static inline void prvSPI_FLASH_CS_HIGH()
{
  HAL_GPIO_WritePin(FLASH_PORT, FLASH_CS_PIN, GPIO_PIN_SET);
}

/**
  * @brief  Write one byte to the FLASH.
  * @note   Addresses to be written must be in the erased state
  * @param  WriteAddress: FLASH's internal address to write to.
  * @param  Byte: the data to be written.
  * @retval None
  */
static void prvSPI_FLASH_WriteByte(uint32_t WriteAddress, uint8_t Byte)
{
  /* Enable the write access to the FLASH */
  prvSPI_FLASH_WriteEnable();

  /* Select the FLASH */
  prvSPI_FLASH_CS_LOW();
  /* Send "Byte Program" instruction */
  prvSPI_FLASH_SendReceiveByte(SPI_FLASH_CMD_WRITE);
  /* Send WriteAddress high, medium and low nibble address byte to write to */
  prvSPI_FLASH_SendReceiveByte((WriteAddress & 0xFF0000) >> 16);
  prvSPI_FLASH_SendReceiveByte((WriteAddress & 0xFF00) >> 8);
  prvSPI_FLASH_SendReceiveByte(WriteAddress & 0xFF);
  /* Send the byte */
  prvSPI_FLASH_SendReceiveByte(Byte);
  /* Deselect the FLASH */
  prvSPI_FLASH_CS_HIGH();
  /* Wait till the end of Flash writing */
  prvSPI_FLASH_WaitForWriteEnd();
}

/**
  * @brief  Writes more than one byte to the FLASH.
  * @note   The address must be even and the number of bytes must be a multiple
  *         of two.
  * @note   Addresses to be written must be in the erased state
  * @param  pBuffer: pointer to the buffer containing the data to be written
  *         to the FLASH.
  * @param  WriteAddress: FLASH's internal address to write to, must be even.
  * @param  NumByteToWrite: number of bytes to write to the FLASH, must be even.
  * @retval None
  */
static void prvSPI_FLASH_WriteBytes(uint8_t* pBuffer, uint32_t WriteAddress, uint32_t NumByteToWrite)
{
  uint32_t numOfWholePages = NumByteToWrite / SPI_FLASH_PAGE_SIZE;
  uint32_t bytesLeft = NumByteToWrite % SPI_FLASH_PAGE_SIZE;

  while (numOfWholePages)
  {
    prvSPI_FLASH_WriteUpToOnePage(pBuffer, WriteAddress, SPI_FLASH_PAGE_SIZE);
    numOfWholePages--;
    pBuffer += SPI_FLASH_PAGE_SIZE;
    WriteAddress += SPI_FLASH_PAGE_SIZE;
  }

  if (bytesLeft)
    prvSPI_FLASH_WriteUpToOnePage(pBuffer, WriteAddress, bytesLeft);

  /* Disable the write access to the FLASH */
  prvSPI_FLASH_WriteDisable();
}

/**
  * @brief
  * @param  None
  * @retval None
  */
static void prvSPI_FLASH_WriteUpToOnePage(uint8_t* pBuffer, uint32_t WriteAddress, uint32_t NumByteToWrite)
{
  if (NumByteToWrite <= SPI_FLASH_PAGE_SIZE)
  {
    /* Enable the write access to the FLASH */
    prvSPI_FLASH_WriteEnable();

    /* Select the FLASH */
    prvSPI_FLASH_CS_LOW();

    /* Send write command */
    prvSPI_FLASH_SendReceiveByte(SPI_FLASH_CMD_WRITE);
    /* Send WriteAddress high, medium and low nibble address byte to write to */
    prvSPI_FLASH_SendReceiveByte((WriteAddress & 0xFF0000) >> 16);
    prvSPI_FLASH_SendReceiveByte((WriteAddress & 0xFF00) >> 8);
    prvSPI_FLASH_SendReceiveByte(WriteAddress & 0xFF);

    /* While there is data to be written to the FLASH */
    while (NumByteToWrite)
    {
      /* Send one byte */
      prvSPI_FLASH_SendReceiveByte(*pBuffer++);
      /* Update NumByteToWrite */
      NumByteToWrite--;
    }

    /* Deselect the FLASH */
    prvSPI_FLASH_CS_HIGH();
    /* Wait till the end of Flash writing */
    prvSPI_FLASH_WaitForWriteEnd();
  }
}

/**
  * @brief  Disables the write access to the FLASH
  * @param  None
  * @retval None
  */
static void prvSPI_FLASH_WriteDisable()
{
  /* Select the FLASH */
  prvSPI_FLASH_CS_LOW();

  /* Send "Write Disable" instruction */
  prvSPI_FLASH_SendReceiveByte(SPI_FLASH_CMD_WRDI);

  /* Deselect the FLASH */
  prvSPI_FLASH_CS_HIGH();
}

/**
  * @brief  Enables the write access to the FLASH
  * @param  None
  * @retval None
  */
static void prvSPI_FLASH_WriteEnable()
{
  /* Select the FLASH */
  prvSPI_FLASH_CS_LOW();

  /* Send "Write Enable" instruction */
  prvSPI_FLASH_SendReceiveByte(SPI_FLASH_CMD_WREN);

  /* Deselect the FLASH */
  prvSPI_FLASH_CS_HIGH();
}

/**
  * @brief  Send one byte to the SPI FLASH and return the byte received back
  * @param  Byte: The byte to send
  * @retval The byte received from the SPI FLASH
  */
static uint8_t prvSPI_FLASH_SendReceiveByte(uint8_t Byte)
{
  /* TODO: Do this RAW instead of using HAL??? A lot of overhead in HAL */
  uint8_t rxByte;
  HAL_SPI_TransmitReceive(&SPI_Handle, &Byte, &rxByte, 1, 10);  /* TODO: Check timeout */
  return rxByte;
}

/**
  * @brief  Polls the status of the Write In Progress (WIP) flag in the FLASH's
  *         status register and loop until write operation has completed.
  * @param  None
  * @retval None
  */
static void prvSPI_FLASH_WaitForWriteEnd()
{
  uint8_t flashStatus = 0;

  /* Select the FLASH */
  prvSPI_FLASH_CS_LOW();

  /* Send "Read Status Register" instruction */
  prvSPI_FLASH_SendReceiveByte(SPI_FLASH_CMD_RDSR);

  /* Loop as long as the memory is busy with a write cycle */
  do
  {
    /* TODO: Handle blocking */
    /* Send a dummy byte to generate the clock needed by the FLASH
    and put the value of the status register in FLASH_Status variable */
    flashStatus = prvSPI_FLASH_SendReceiveByte(SPI_FLASH_DUMMY_BYTE);
  } while ((flashStatus & SPI_FLASH_WIP_FLAG) == SET); /* Write in progress */

  /* Deselect the FLASH */
  prvSPI_FLASH_CS_HIGH();
}

/** Interrupt Handlers -------------------------------------------------------*/
