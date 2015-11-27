/**
 ******************************************************************************
 * @file    fpga_config.c
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-08-16
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

/** Includes -----------------------------------------------------------------*/
#include "fpga_config.h"
#include "spi_flash.h"
#include "spi2.h"

/** Private defines ----------------------------------------------------------*/
#define CONF_DONE_PORT  (GPIOB)
#define CONF_DONE_PIN   (GPIO_PIN_10)

#define NSTATUS_PORT    (GPIOB)
#define NSTATUS_PIN     (GPIO_PIN_11)

#define FPGA_INTERRUPT_PORT   (GPIOB)
#define FPGA_INTERRUPT_PIN    (GPIO_PIN_1)

#define NCONFIG_PORT   (GPIOA)
#define NCONFIG_PIN    (GPIO_PIN_8)


#define FPGA_CONFIG_BIT_FILE_OFFSET           (0x00060000)  /* 393216 */
#define FPGA_CONFIG_BIT_FILE_NUM_OF_BLOCKS    (6)
#define FPGA_CONFIG_BIT_FILE_MAX_SIZE         (368011)

#define VALID_BITFILE_NUMBER(X) (X > 0 || X <= 5)

/** Private variables --------------------------------------------------------*/
/** Private functions --------------------------------------------------------*/
/** Functions ----------------------------------------------------------------*/
/**
 * @brief   Initializes the FPGA Config
 * @param   None
 * @retval  None
 */
void FPGA_CONFIG_Init(void)
{
  /* Enable clock for GPIOA & GPIOB */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStructure;

  /* Initialize CONF_DONE */
  GPIO_InitStructure.Pin    = CONF_DONE_PIN;
  GPIO_InitStructure.Speed  = GPIO_SPEED_LOW;
  GPIO_InitStructure.Mode   = GPIO_MODE_INPUT;
  GPIO_InitStructure.Pull   = GPIO_NOPULL;
  HAL_GPIO_Init(CONF_DONE_PORT, &GPIO_InitStructure);

  /* Initialize nSTATUS */
  GPIO_InitStructure.Pin    = NSTATUS_PIN;
  GPIO_InitStructure.Speed  = GPIO_SPEED_LOW;
  GPIO_InitStructure.Mode   = GPIO_MODE_INPUT;
  GPIO_InitStructure.Pull   = GPIO_NOPULL;
  HAL_GPIO_Init(NSTATUS_PORT, &GPIO_InitStructure);

  /* Initialize FPGA_INTERRUPT */
  GPIO_InitStructure.Pin    = FPGA_INTERRUPT_PIN;
  GPIO_InitStructure.Speed  = GPIO_SPEED_LOW;
  GPIO_InitStructure.Mode   = GPIO_MODE_INPUT;
  GPIO_InitStructure.Pull   = GPIO_NOPULL;
  HAL_GPIO_Init(FPGA_INTERRUPT_PORT, &GPIO_InitStructure);

  /* Initialize nCONFIG */
  GPIO_InitStructure.Pin    = NCONFIG_PIN;
  GPIO_InitStructure.Speed  = GPIO_SPEED_LOW;
  GPIO_InitStructure.Mode   = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pull   = GPIO_NOPULL;
  HAL_GPIO_Init(NCONFIG_PORT, &GPIO_InitStructure);
  HAL_GPIO_WritePin(NCONFIG_PORT, NCONFIG_PIN, GPIO_PIN_SET);
}

/**
 * @brief
 * @param   None
 * @retval  None
 */
uint32_t FPGA_CONFIG_SizeOfBitFile(uint8_t BitFileNumber)
{
  uint32_t headerAddress = BitFileNumber * FPGA_CONFIG_BIT_FILE_OFFSET;
  uint8_t dataBuffer[4] = {0};
  SPI_FLASH_ReadBuffer(dataBuffer, headerAddress, 4);
  uint32_t bitFileSize =  (dataBuffer[0] << 24) | (dataBuffer[1] << 16) |
                          (dataBuffer[2] << 8) | (dataBuffer[3]);
  return bitFileSize;
}

/**
 * @brief
 * @param   None
 * @retval  None
 */
ErrorStatus FPGA_CONFIG_EraseBitFile(uint8_t BitFileNumber)
{
  /* Make sure it's a valid bit file number */
  if (VALID_BITFILE_NUMBER(BitFileNumber))
  {
    uint32_t currentAddress = BitFileNumber * FPGA_CONFIG_BIT_FILE_OFFSET;
    uint32_t numOfBlocksLeftToErase = FPGA_CONFIG_BIT_FILE_NUM_OF_BLOCKS;

    while (numOfBlocksLeftToErase != 0)
    {
      SPI_FLASH_EraseBlock(currentAddress);
      currentAddress += SPI_FLASH_BYTES_IN_BLOCK;
      numOfBlocksLeftToErase--;
    }
    return SUCCESS;
  }
  else
    return ERROR;
}

/**
 * @brief
 * @param   None
 * @retval  None
 */
ErrorStatus FPGA_CONFIG_Start(uint8_t BitFileNumber)
{
  /* Make sure it's a valid bit file number */
  if (VALID_BITFILE_NUMBER(BitFileNumber))
  {
    uint32_t headerAddress = BitFileNumber * FPGA_CONFIG_BIT_FILE_OFFSET;
    uint32_t bitFileSize = FPGA_CONFIG_SizeOfBitFile(BitFileNumber);

    if (bitFileSize != 0 && bitFileSize <= FPGA_CONFIG_BIT_FILE_MAX_SIZE)
    {
      /**
       * To begin the configuration, the external host device must generate a
       * low-to-high transition on the nCONFIG pin
       */
      HAL_GPIO_WritePin(NCONFIG_PORT, NCONFIG_PIN, GPIO_PIN_RESET);
      HAL_Delay(10);
      HAL_GPIO_WritePin(NCONFIG_PORT, NCONFIG_PIN, GPIO_PIN_SET);

      /**
       * When nSTATUS is pulled high, the external host device must place the
       * configuration data one bit at a time on DATA[0]
       */
      while (HAL_GPIO_ReadPin(NSTATUS_PORT, NSTATUS_PIN) != GPIO_PIN_SET)
      {
        HAL_Delay(10);
      }

      /** DeInit and init the SPI2 */
      SPI2_DeInit();
      SPI2_InitForFpgaConfig();
      SPI2_SelectDevice(SPI2_Device_Fpga);

      /** Start transferring data from SPI Flash to FPGA */
      uint8_t dataBuffer[256] = {0};
      uint32_t bytesLeftToTransfer = bitFileSize;
      uint32_t currentReadAddress = headerAddress + 256;
      while (bytesLeftToTransfer)
      {
        if (bytesLeftToTransfer >= 256)
        {
          /* Get the data from flash */
          SPI_FLASH_ReadBuffer(dataBuffer, currentReadAddress, 256);
          currentReadAddress += 256;
          /* Write the data to the fpga */
          SPI2_WriteBuffer(dataBuffer, 256);
          bytesLeftToTransfer -= 256;
        }
        else
        {
          /* Get the data from flash */
          SPI_FLASH_ReadBuffer(dataBuffer, currentReadAddress, bytesLeftToTransfer);
          /* Write the data to the fpga */
          SPI2_WriteBuffer(dataBuffer, bytesLeftToTransfer);
          bytesLeftToTransfer = 0;
        }
      }

      if (HAL_GPIO_ReadPin(CONF_DONE_PORT, CONF_DONE_PIN) != GPIO_PIN_SET)
      {
        /* TODO: ERROR */
      }
      else
      {
        /* Two DCLK falling edges are required after CONF_DONE goes high to begin the initialization of the device */
        dataBuffer[0] = 0;
        SPI2_WriteBuffer(dataBuffer, 1);
      }

      SPI2_DeselectDevice(SPI2_Device_Fpga);
      SPI2_DeInit();

      return SUCCESS;
    }
    else
      return ERROR;
  }
  else
    return ERROR;
}

/** Interrupt Handlers -------------------------------------------------------*/
