/**
 ******************************************************************************
 * @file	adc_task.c
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-09-28
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
#include "adc_task.h"

#include "relay.h"
#include "spi_flash_memory_map.h"
#include "spi_flash.h"
#include "max1301.h"

#include <string.h>
#include <stdbool.h>

/* Private defines -----------------------------------------------------------*/
#define ADC_SPI					(SPI1)
#define ADC_SPI_CLK_ENABLE()	(__SPI1_CLK_ENABLE())
#define ADC_PORT				(GPIOA)
#define ADC_GPIO_CLK_ENABLE()	(__GPIOA_CLK_ENABLE())
#define ADC_CS_PIN				(GPIO_PIN_4)
#define ADC_SCK_PIN				(GPIO_PIN_5)
#define ADC_MISO_PIN			(GPIO_PIN_6)
#define ADC_MOSI_PIN			(GPIO_PIN_7)

/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static SPI_HandleTypeDef SPI_Handle = {
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

static bool prvDoneInitializing = false;

/* Private function prototypes -----------------------------------------------*/
static void prvHardwareInit();
static void prvReadSettingsFromSpiFlash();

static inline void prvADC_CS_LOW();
static inline void prvADC_CS_HIGH();
static uint8_t prvADC_SendReceiveByte(uint8_t Byte);


/* Functions -----------------------------------------------------------------*/
/**
 * @brief	The main task for the UART1 channel
 * @param	pvParameters:
 * @retval	None
 */
void adcTask(void *pvParameters)
{
	/* Initialize hardware */
	prvHardwareInit();

	/* Wait to make sure the SPI FLASH is initialized */
//	while (SPI_FLASH_Initialized() == false)
//	{
//		vTaskDelay(100 / portTICK_PERIOD_MS);
//	}

	/* Try to read the settings from SPI FLASH */
//	prvReadSettingsFromSpiFlash();


	/* The parameter in vTaskDelayUntil is the absolute time
	 * in ticks at which you want to be woken calculated as
	 * an increment from the time you were last woken. */
	TickType_t xNextWakeTime;
	/* Initialize xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	prvDoneInitializing = true;
	while (1)
	{
		vTaskDelayUntil(&xNextWakeTime, 250 / portTICK_PERIOD_MS);

		for (uint32_t i = 0; i < 128; i++)
			prvADC_SendReceiveByte(0xAA);
	}

	/* Something has gone wrong */
	error:
		while (1);
}

/**
 * @brief	Check if the channel is done initializing
 * @param	None
 * @retval	true if it's done
 * @retval	false if not done
 */
bool adcIsDoneInitializing()
{
	return prvDoneInitializing;
}


/* Private functions .--------------------------------------------------------*/
/**
 * @brief	Initializes the hardware
 * @param	None
 * @retval	None
 */
static void prvHardwareInit()
{
//	MAX1301_Init();

	/* Init GPIO */
	ADC_GPIO_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin  		= ADC_SCK_PIN | ADC_MISO_PIN | ADC_MOSI_PIN;
	GPIO_InitStructure.Mode  		= GPIO_MODE_AF_PP;
	GPIO_InitStructure.Alternate	= GPIO_AF5_SPI1;
	GPIO_InitStructure.Pull			= GPIO_NOPULL;
	GPIO_InitStructure.Speed 		= GPIO_SPEED_HIGH;
	HAL_GPIO_Init(ADC_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.Pin  		= ADC_CS_PIN;
	GPIO_InitStructure.Mode  		= GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(ADC_PORT, &GPIO_InitStructure);
	/* Deselect the ADC */
	prvADC_CS_HIGH();

	/* Init SPI */
	ADC_SPI_CLK_ENABLE();
	HAL_SPI_Init(&SPI_Handle);
}


/**
 * @brief	Read settings from SPI FLASH
 * @param	None
 * @retval	None
 */
static void prvReadSettingsFromSpiFlash()
{
//	/* Read to a temporary settings variable */
//	UARTSettings settings;
//	SPI_FLASH_ReadBufferDMA((uint8_t*)&settings, FLASH_ADR_UART1_SETTINGS, sizeof(UARTSettings));

}

/* Private functions .--------------------------------------------------------*/
/**
 * @brief	Pull the CS pin LOW
 * @param	None
 * @retval	None
 */
static inline void prvADC_CS_LOW()
{
	HAL_GPIO_WritePin(ADC_PORT, ADC_CS_PIN, GPIO_PIN_RESET);
}

/**
 * @brief	Pull the CS pin HIGH
 * @param	None
 * @retval	None
 */
static inline void prvADC_CS_HIGH()
{
	HAL_GPIO_WritePin(ADC_PORT, ADC_CS_PIN, GPIO_PIN_SET);
}

/**
  * @brief  Send one byte to the ADC and return the byte received back
  * @param  Byte: The byte to send
  * @retval The byte received from the ADC
  */
static uint8_t prvADC_SendReceiveByte(uint8_t Byte)
{
	/* TODO: Do this RAW instead of using HAL??? A lot of overhead in HAL */
	uint8_t rxByte;
	HAL_SPI_TransmitReceive(&SPI_Handle, &Byte, &rxByte, 1, 10);	/* TODO: Check timeout */
	return rxByte;
}


/* Interrupt Handlers --------------------------------------------------------*/
/* HAL Callback functions ----------------------------------------------------*/
