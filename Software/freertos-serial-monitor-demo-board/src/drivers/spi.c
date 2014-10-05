/**
 ******************************************************************************
 * @file	spi.c
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-08-23
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
#include "spi.h"

/* Private defines -----------------------------------------------------------*/
#define ADC_SPI					(SPI1)
#define ADC_SPI_CLK_ENABLE()	(__SPI1_CLK_ENABLE())
#define ADC_PORT				(GPIOA)
#define ADC_GPIO_CLK_ENABLE()	(__GPIOA_CLK_ENABLE())
#define ADC_CS_PIN				(GPIO_PIN_4)
#define ADC_SCK_PIN				(GPIO_PIN_5)
#define ADC_MISO_PIN			(GPIO_PIN_6)
#define ADC_MOSI_PIN			(GPIO_PIN_7)


#define FLASH_SPI				(SPI2)
#define FLASH_SPI_CLK_ENABLE()	(__SPI2_CLK_ENABLE())
#define FLASH_PORT				(GPIOB)
#define FLASH_GPIO_CLK_ENABLE()	(__GPIOB_CLK_ENABLE())
#define FLASH_CS_PIN			(GPIO_PIN_12)
#define FLASH_SCK_PIN			(GPIO_PIN_13)
#define FLASH_MISO_PIN			(GPIO_PIN_14)
#define FLASH_MOSI_PIN			(GPIO_PIN_15)

/* Private variables ---------------------------------------------------------*/
static SPI_HandleTypeDef SPI_Handle_ADC_Thermocouple = {
		.Instance 				= ADC_SPI,
		.Init.Mode 				= SPI_MODE_MASTER,
		.Init.Direction 		= SPI_DIRECTION_2LINES,
		.Init.DataSize 			= SPI_DATASIZE_8BIT,
		.Init.CLKPolarity 		= SPI_POLARITY_LOW,
		.Init.CLKPhase 			= SPI_PHASE_1EDGE,
		.Init.NSS 				= SPI_NSS_SOFT,
		.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2,
		.Init.FirstBit 			= SPI_FIRSTBIT_MSB,
		.Init.TIMode			= SPI_TIMODE_DISABLED,
		.Init.CRCCalculation	= SPI_CRCCALCULATION_DISABLED,
		.Init.CRCPolynomial 	= 1,
};

static SPI_HandleTypeDef SPI_Handle_FLASH = {
		.Instance 				= FLASH_SPI,
		.Init.Mode 				= SPI_MODE_MASTER,
		.Init.Direction 		= SPI_DIRECTION_2LINES,
		.Init.DataSize 			= SPI_DATASIZE_8BIT,
		.Init.CLKPolarity 		= SPI_POLARITY_LOW,
		.Init.CLKPhase 			= SPI_PHASE_1EDGE,
		.Init.NSS 				= SPI_NSS_SOFT,
		.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2,
		.Init.FirstBit 			= SPI_FIRSTBIT_MSB,
		.Init.TIMode			= SPI_TIMODE_DISABLED,
		.Init.CRCCalculation	= SPI_CRCCALCULATION_DISABLED,
		.Init.CRCPolynomial 	= 1,
};


static SemaphoreHandle_t xSemaphoreSpi1;
static bool prvInitializedSpi1 = false;

static SemaphoreHandle_t xSemaphoreSpi2;
static bool prvInitializedSpi2 = false;

/* Private functions ---------------------------------------------------------*/
/* Functions -----------------------------------------------------------------*/
/**
 * @brief	Initializes the SPI
 * @param
 * @retval	None
 */
void SPI_Init(SPIChannel Channel)
{
	/* Make sure we haven't initialized this before */
	if (!prvInitializedSpi1 && (Channel == SPIChannel_ADC || Channel == SPIChannel_Thermocouple))
	{
		/* Mutex semaphore for mutual exclusion to the SPI device */
		xSemaphoreSpi1 = xSemaphoreCreateBinary();

		/* Init GPIO */
		ADC_GPIO_CLK_ENABLE();
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_InitStructure.Pin  		= ADC_SCK_PIN | ADC_MISO_PIN | ADC_MOSI_PIN | ADC_CS_PIN;
		GPIO_InitStructure.Mode  		= GPIO_MODE_AF_PP;
		GPIO_InitStructure.Alternate	= GPIO_AF5_SPI2;
		GPIO_InitStructure.Pull			= GPIO_NOPULL;
		GPIO_InitStructure.Speed 		= GPIO_SPEED_HIGH;
		HAL_GPIO_Init(ADC_PORT, &GPIO_InitStructure);

		GPIO_InitStructure.Pin  		= ADC_CS_PIN;
		GPIO_InitStructure.Mode  		= GPIO_MODE_OUTPUT_PP;
		HAL_GPIO_Init(ADC_PORT, &GPIO_InitStructure);
		/* Deselect the ADC */
		SPI_DeselectChannel(SPIChannel_ADC);

		/* Init SPI */
		ADC_SPI_CLK_ENABLE();
		HAL_SPI_Init(&SPI_Handle_ADC_Thermocouple);


		prvInitializedSpi1 = true;
	}
	else if (prvInitializedSpi2 && (Channel == SPIChannel_FLASH))
	{
		/* Mutex semaphore for mutual exclusion to the SPI device */
		xSemaphoreSpi2 = xSemaphoreCreateBinary();


		prvInitializedSpi2 = true;
	}
}

/**
  * @brief  Send one byte to the SPI ADC and return the byte received back
  * @param  Byte: The byte to send
  * @retval The byte received from the SPI ADC
  */
uint8_t SPI_SendReceiveByte(SPIChannel Channel, uint8_t Byte)
{
	if (Channel == SPIChannel_ADC || Channel == SPIChannel_Thermocouple)
	{
		/* TODO: Do this RAW instead of using HAL??? A lot of overhead in HAL */
		uint8_t rxByte;
		HAL_SPI_TransmitReceive(&SPI_Handle_ADC_Thermocouple, &Byte, &rxByte, 1, 10);	/* TODO: Check timeout */
		return rxByte;
	}

	return 0;
}

/**
 * @brief	Select the SPI channel
 * @param	None
 * @retval	None
 */
void SPI_SelectChannel(SPIChannel Channel)
{
	if (Channel == SPIChannel_ADC)
	{
		HAL_GPIO_WritePin(ADC_PORT, ADC_CS_PIN, GPIO_PIN_RESET);
	}
	else if (Channel == SPIChannel_Thermocouple)
	{
		HAL_GPIO_WritePin(ADC_PORT, ADC_CS_PIN, GPIO_PIN_SET);
	}
	else if (Channel == SPIChannel_FLASH)
	{

	}
}

/**
 * @brief	Deselect the SPI channel
 * @param	None
 * @retval	None
 */
void SPI_DeselectChannel(SPIChannel Channel)
{
	if (Channel == SPIChannel_ADC)
	{
		HAL_GPIO_WritePin(ADC_PORT, ADC_CS_PIN, GPIO_PIN_SET);
	}
	else if (Channel == SPIChannel_Thermocouple)
	{
		HAL_GPIO_WritePin(ADC_PORT, ADC_CS_PIN, GPIO_PIN_RESET);
	}
	else if (Channel == SPIChannel_FLASH)
	{

	}
}

/* Interrupt Handlers --------------------------------------------------------*/
