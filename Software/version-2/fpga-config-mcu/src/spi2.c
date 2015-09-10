/**
 *******************************************************************************
 * @file    spi2.c
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-09-10
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
#include "spi2.h"

/** Private defines ----------------------------------------------------------*/
#define SPI_INSTANCE            (SPI2)
#define SPI_CLK_ENABLE()        __SPI2_CLK_ENABLE()
#define SPI_PORT                (GPIOB)
#define SPI_GPIO_CLK_ENABLE()   __GPIOB_CLK_ENABLE()
#define SPI_SCK_PIN             (GPIO_PIN_13)
#define SPI_MISO_PIN            (GPIO_PIN_14)
#define SPI_MOSI_PIN            (GPIO_PIN_15)
#define SPI_SD_CARD_CS_PIN      (GPIO_PIN_12)
#define SPI_FPGA_CONFIG_CS_PIN  (GPIO_PIN_0)


/** Private typedefs ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
static SPI_HandleTypeDef SPI_Handle = {
  .Instance                 = SPI_INSTANCE,
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

static bool prvADeviceIsActive = false;

/** Private function prototypes ----------------------------------------------*/
static inline void prvSPI_SD_CARD_CS_LOW();
static inline void prvSPI_SD_CARD_CS_HIGH();
static inline void prvSPI_FPGA_CONFIG_CS_LOW();
static inline void prvSPI_FPGA_CONFIG_CS_HIGH();

/** Functions ----------------------------------------------------------------*/
/**
 * @brief   Initializes the GPIOs used for the SPI2
 * @param   None
 * @retval  None
 */
ErrorStatus SPI2_InitGpio()
{
  /* Init GPIO */
  SPI_GPIO_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.Pin    = SPI_SCK_PIN | SPI_MISO_PIN | SPI_MOSI_PIN;
  GPIO_InitStructure.Mode   = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull   = GPIO_NOPULL;
  GPIO_InitStructure.Speed  = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(SPI_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.Pin    = SPI_SD_CARD_CS_PIN | SPI_FPGA_CONFIG_CS_PIN;
  GPIO_InitStructure.Mode   = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(SPI_PORT, &GPIO_InitStructure);
  /* Deselect both */
  prvSPI_SD_CARD_CS_HIGH();
  prvSPI_FPGA_CONFIG_CS_HIGH();

  return SUCCESS;
}

/**
 * @brief   Initializes the SPI2 for use with FPGA config
 * @param   None
 * @retval  None
 */
ErrorStatus SPI2_InitForFpgaConfig()
{
  /* Init SPI */
  SPI_Handle.Init.FirstBit = SPI_FIRSTBIT_LSB;
  SPI_CLK_ENABLE();
  HAL_SPI_Init(&SPI_Handle);

  return SUCCESS;
}

/**
 * @brief   Initializes the SPI2 for use with SD Card
 * @param   None
 * @retval  None
 */
ErrorStatus SPI2_InitForSdCard()
{
  /* Init SPI */
  SPI_Handle.Init.FirstBit = SPI_FIRSTBIT_MSB;
  SPI_CLK_ENABLE();
  HAL_SPI_Init(&SPI_Handle);

  return SUCCESS;
}

/**
 * @brief
 * @param   None
 * @retval  None
 */
ErrorStatus SPI2_DeInit()
{
  if (HAL_SPI_DeInit(&SPI_Handle) != HAL_OK)
    return ERROR;
  else
    return SUCCESS;
}

/**
 * @brief
 * @param   None
 * @retval  None
 */
void SPI2_WriteBuffer(uint8_t* pBuffer, uint32_t NumByteToWrite)
{
  HAL_SPI_Transmit(&SPI_Handle, pBuffer, NumByteToWrite, 100);  /* TODO: Check timeout */
}

/**
 * @brief
 * @param   None
 * @retval  None
 */
void SPI2_SelectDevice(SPI2_Device Device)
{
  if (Device == SPI2_Device_SdCard)
    prvSPI_SD_CARD_CS_LOW();
  else if (Device == SPI2_Device_Fpga)
    prvSPI_FPGA_CONFIG_CS_LOW();
}

/**
 * @brief
 * @param   None
 * @retval  None
 */
void SPI2_DeselectDevice(SPI2_Device Device)
{
  if (Device == SPI2_Device_SdCard)
    prvSPI_SD_CARD_CS_HIGH();
  else if (Device == SPI2_Device_Fpga)
    prvSPI_FPGA_CONFIG_CS_HIGH();
}

/** Private functions .-------------------------------------------------------*/
/**
 * @brief   Pull the CS pin LOW for the SD Card
 * @param   None
 * @retval  None
 */
static inline void prvSPI_SD_CARD_CS_LOW()
{
  if (!prvADeviceIsActive)
  {
    HAL_GPIO_WritePin(SPI_PORT, SPI_SD_CARD_CS_PIN, GPIO_PIN_RESET);
    prvADeviceIsActive = true;
  }
}

/**
 * @brief   Pull the CS pin HIGH for the SD Card
 * @param   None
 * @retval  None
 */
static inline void prvSPI_SD_CARD_CS_HIGH()
{
  HAL_GPIO_WritePin(SPI_PORT, SPI_SD_CARD_CS_PIN, GPIO_PIN_SET);
  prvADeviceIsActive = false;
}

/**
 * @brief   Pull the CS pin LOW for the FPGA Config
 * @param   None
 * @retval  None
 */
static inline void prvSPI_FPGA_CONFIG_CS_LOW()
{
  if (!prvADeviceIsActive)
  {
    HAL_GPIO_WritePin(SPI_PORT, SPI_FPGA_CONFIG_CS_PIN, GPIO_PIN_RESET);
    prvADeviceIsActive = true;
  }
}

/**
 * @brief   Pull the CS pin HIGH for the FPGA Config
 * @param   None
 * @retval  None
 */
static inline void prvSPI_FPGA_CONFIG_CS_HIGH()
{
  HAL_GPIO_WritePin(SPI_PORT, SPI_FPGA_CONFIG_CS_PIN, GPIO_PIN_SET);
  prvADeviceIsActive = false;
}

/** Interrupt Handlers -------------------------------------------------------*/
