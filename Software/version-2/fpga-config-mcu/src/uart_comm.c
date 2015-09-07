/**
 *******************************************************************************
 * @file    uart_comm.c
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-09-07
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

/**
 * Examples:
 * Remember to set the write address before issuing a write to flash command
 *
 * Set write address to 0x00000000: AA BB CC 10 04 00 00 00 00 C9
 * Set write address to 0x11223344: AA BB CC 10 04 11 22 33 44 8D
 * Get current write address: AA BB CC 11 CC
 * Erase full flash: AA BB CC 20 FD
 * Erase first sector at 0x00000000: AA BB CC 21 04 00 00 00 00 F8
 * Write "Hello World" to flash: AA BB CC 30 0B 48 65 6C 6C 6F 20 57 6F 72 6C 64 C6
 * Write "Test Write!" to flash: AA BB CC 30 0B 54 65 73 74 20 57 72 69 74 65 21 8C
 * Read 11 bytes from flash at address 0x00000000: AA BB CC 40 05 00 00 00 00 0B 93
 * Read 256 bytes from flash at address 0x00000000: AA BB CC 40 05 00 00 00 00 FF 67
 */

/** Includes -----------------------------------------------------------------*/
#include "uart_comm.h"
#include "uart1.h"
#include "spi_flash.h"

/** Private defines ----------------------------------------------------------*/
#define UART_COMM_HEADER_1      (0xAA)
#define UART_COMM_HEADER_2      (0xBB)
#define UART_COMM_HEADER_3      (0xCC)
#define UART_COMM_ACK           (0xDD)
#define UART_COMM_NACK          (0xEE)
#define UART_COMM_UNKNOWN_COMMAND (0xDE)
#define UART_COMM_BUFFER_SIZE   (257)

/* Data = 4 bytes write address to save */
#define UART_COMM_COMMAND_SET_FLASH_WRITE_ADDRESS   (0x10)
/* Data = None, returns the 4 byte write address MSByte first */
#define UART_COMM_COMMAND_GET_FLASH_WRITE_ADDRESS   (0x11)
/* Data = None */
#define UART_COMM_COMMAND_ERASE_FULL_FLASH          (0x20)
/* Data = 4 bytes address for sector to eras */
#define UART_COMM_COMMAND_ERASE_SECTOR_IN_FLASH     (0x21)
/* Data = The data to write 0 to 256 bytes */
#define UART_COMM_COMMAND_WRITE_DATA_TO_FLASH       (0x30)
/* Data = 4 bytes read address, 1 byte num of bytes to read, returns the data read */
#define UART_COMM_COMMAND_READ_DATA_FROM_FLASH      (0x40)

/** Private typedefs ---------------------------------------------------------*/
typedef enum
{
  UART_CommStateHeader1,
  UART_CommStateHeader2,
  UART_CommStateHeader3,
  UART_CommStateCommand,
  UART_CommStateDataCount,
  UART_CommStateData,
  UART_CommStateChecksum,
} UART_CommState;

/** Private variables --------------------------------------------------------*/
UART_CommState prvCurrentState = UART_CommStateHeader1;
uint8_t prvChecksum = 0;
uint32_t prvDataBytesToRead = 0;
uint8_t prvDataBuffer[UART_COMM_BUFFER_SIZE] = {0};
uint32_t prvDataBytesRead = 0;
uint8_t prvCurrentCommand = 0;
uint32_t prvCurrentFlashWriteAddress = 0;

/** Private function prototypes ----------------------------------------------*/
/** Functions ----------------------------------------------------------------*/
/**
 * @brief
 * @param   None
 * @retval  None
 */
void UART_COMM_HandleReceivedByte(uint8_t Byte)
{
  /* Header byte 1 ========================================================== */
  if (prvCurrentState == UART_CommStateHeader1 && Byte == UART_COMM_HEADER_1)
  {
    prvCurrentState = UART_CommStateHeader2;
    /* Start calculating the checksum */
    prvChecksum = Byte;
  }
  /* Header byte 2 ========================================================== */
  else if (prvCurrentState == UART_CommStateHeader2)
  {
    if (Byte == UART_COMM_HEADER_2)
    {
      prvCurrentState = UART_CommStateHeader3;
      /* Calculate the checksum */
      prvChecksum ^= Byte;
    }
    else
      prvCurrentState = UART_CommStateHeader1;
  }
  /* Header byte 3 ========================================================== */
  else if (prvCurrentState == UART_CommStateHeader3)
  {
    if (Byte == UART_COMM_HEADER_3)
    {
      prvCurrentState = UART_CommStateCommand;
      /* Calculate the checksum */
      prvChecksum ^= Byte;
      /* Reset the command */
      prvCurrentCommand = 0;
    }
    else
      prvCurrentState = UART_CommStateHeader1;
  }
  /* Command ================================================================ */
  else if (prvCurrentState == UART_CommStateCommand)
  {
    if (Byte == UART_COMM_COMMAND_SET_FLASH_WRITE_ADDRESS ||
        Byte == UART_COMM_COMMAND_ERASE_SECTOR_IN_FLASH ||
        Byte == UART_COMM_COMMAND_WRITE_DATA_TO_FLASH ||
        Byte == UART_COMM_COMMAND_READ_DATA_FROM_FLASH)
    {
      prvCurrentState = UART_CommStateDataCount;
    }
    else if (Byte == UART_COMM_COMMAND_ERASE_FULL_FLASH ||
             Byte == UART_COMM_COMMAND_GET_FLASH_WRITE_ADDRESS)
    {
      prvCurrentState = UART_CommStateChecksum;
    }
    else
    {
      prvCurrentState = UART_CommStateHeader1;
      /* Command not recognized */
      UART1_SendByte(UART_COMM_UNKNOWN_COMMAND);
      return;
    }
    /* Save the command */
    prvCurrentCommand = Byte;
    /* Calculate the checksum */
    prvChecksum ^= Byte;
  }
  /* Data count ============================================================= */
  else if (prvCurrentState == UART_CommStateDataCount)
  {
    prvDataBytesToRead = Byte;
    prvCurrentState = UART_CommStateData;
    prvDataBytesRead = 0;
    /* Calculate the checksum */
    prvChecksum ^= Byte;
  }
  /* Data =================================================================== */
  else if (prvCurrentState == UART_CommStateData)
  {
    /* Add the data to the buffer */
    prvDataBuffer[prvDataBytesRead] = Byte;
    /* Increment the number of bytes read */
    prvDataBytesRead++;
    /* Calculate checksum */
    prvChecksum ^= Byte;
    /* Decrement number of bytes left to read */
    prvDataBytesToRead--;
    if (prvDataBytesToRead == 0)
    {
      prvCurrentState = UART_CommStateChecksum;
    }
  }
  /* Checksum =============================================================== */
  else if (prvCurrentState == UART_CommStateChecksum)
  {
    if (Byte == prvChecksum)
    {
      /* Full FLASH erase command */
      if (prvCurrentCommand == UART_COMM_COMMAND_ERASE_FULL_FLASH)
      {
        SPI_FLASH_EraseChip();
      }
      /* Set write address for FLASH */
      else if (prvCurrentCommand == UART_COMM_COMMAND_SET_FLASH_WRITE_ADDRESS)
      {
        prvCurrentFlashWriteAddress =
            ((uint32_t)prvDataBuffer[0] << 24) |
            ((uint32_t)prvDataBuffer[1] << 16) |
            ((uint32_t)prvDataBuffer[2] << 8) |
            (uint32_t)prvDataBuffer[3];
      }
      /* Get current write address */
      else if (prvCurrentCommand == UART_COMM_COMMAND_GET_FLASH_WRITE_ADDRESS)
      {
        prvDataBuffer[0] = (prvCurrentFlashWriteAddress >> 24) & 0xFF;
        prvDataBuffer[1] = (prvCurrentFlashWriteAddress >> 16) & 0xFF;
        prvDataBuffer[2] = (prvCurrentFlashWriteAddress >> 8) & 0xFF;
        prvDataBuffer[3] = (prvCurrentFlashWriteAddress) & 0xFF;
        prvDataBuffer[4] = UART_COMM_ACK;
        UART1_SendBuffer(prvDataBuffer, 5);
        goto change_state;
      }
      /* Sector Erase */
      else if (prvCurrentCommand == UART_COMM_COMMAND_ERASE_SECTOR_IN_FLASH)
      {
        uint32_t sectorAddress =
            ((uint32_t)prvDataBuffer[0] << 24) |
            ((uint32_t)prvDataBuffer[1] << 16) |
            ((uint32_t)prvDataBuffer[2] << 8) |
            (uint32_t)prvDataBuffer[3];
        SPI_FLASH_EraseSector(sectorAddress);
      }
      /* Write to flash */
      else if (prvCurrentCommand == UART_COMM_COMMAND_WRITE_DATA_TO_FLASH)
      {
        SPI_FLASH_WriteBuffer(prvDataBuffer, prvCurrentFlashWriteAddress, prvDataBytesRead);
        /* Auto-increment the write address */
        prvCurrentFlashWriteAddress += prvDataBytesRead;
      }
      /* Read from flash */
      else if (prvCurrentCommand == UART_COMM_COMMAND_READ_DATA_FROM_FLASH)
      {
        /* Calculate the read address */
        uint32_t readAddress =
            ((uint32_t)prvDataBuffer[0] << 24) |
            ((uint32_t)prvDataBuffer[1] << 16) |
            ((uint32_t)prvDataBuffer[2] << 8) |
            (uint32_t)prvDataBuffer[3];
        uint8_t dataSize = prvDataBuffer[4];
        /* Read the data */
        SPI_FLASH_ReadBuffer(prvDataBuffer, readAddress, dataSize);
        /* Send the data back + ACK */
        prvDataBuffer[dataSize] = UART_COMM_ACK;
        UART1_SendBuffer(prvDataBuffer, dataSize + 1);
        goto change_state;
      }
      else
      {
        /* Command not recognized */
        UART1_SendByte(UART_COMM_UNKNOWN_COMMAND);
        goto change_state;
      }
      /* Checksum OK */
      UART1_SendByte(UART_COMM_ACK);
    }
    else
    {
      /* Checksum wrong */
      UART1_SendByte(UART_COMM_NACK);
    }
change_state:
    prvCurrentState = UART_CommStateHeader1;
  }
  /* Just in case =========================================================== */
  else
  {
    prvCurrentState = UART_CommStateHeader1;
  }
}

/** Private functions .-------------------------------------------------------*/
/** Interrupt Handlers -------------------------------------------------------*/
