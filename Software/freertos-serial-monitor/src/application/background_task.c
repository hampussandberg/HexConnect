/**
 ******************************************************************************
 * @file	background_task.c
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-08-22
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
#include "background_task.h"

#include "i2c2.h"
#include "spi_flash.h"

/* Private defines -----------------------------------------------------------*/
#define MCP9808_TEMP_SENSOR_ADDRESS		(0x1F)

/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#define BUFFER_SIZE		16
uint8_t rxData[BUFFER_SIZE] = {0x00};

/* Private function prototypes -----------------------------------------------*/
static void prvHardwareInit();

/* Functions -----------------------------------------------------------------*/
/**
 * @brief	Text
 * @param	None
 * @retval	None
 */
void backgroundTask(void *pvParameters)
{
	prvHardwareInit();

	/* The parameter in vTaskDelayUntil is the absolute time
	 * in ticks at which you want to be woken calculated as
	 * an increment from the time you were last woken. */
	TickType_t xNextWakeTime;
	/* Initialize xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	SPI_FLASH_Init();

	SPI_FLASH_EraseSector(0x000000);
	uint8_t txData[4] = {0xAA, 0xBB, 0xCC, 0xDD};
	SPI_FLASH_WriteBuffer(txData, 0x000001, 4);
	SPI_FLASH_ReadBuffer(rxData, 0x000000, BUFFER_SIZE);

	while (1)
	{
		/* LED on for 25 ms */
		HAL_GPIO_WritePin(GPIOC, backgroundLED_0, GPIO_PIN_RESET);
		vTaskDelayUntil(&xNextWakeTime, 25 / portTICK_PERIOD_MS);

		/* LED off for 1000 ms */
		HAL_GPIO_WritePin(GPIOC, backgroundLED_0, GPIO_PIN_SET);
		vTaskDelayUntil(&xNextWakeTime, 1000 / portTICK_PERIOD_MS);
	}
}

/* Private functions .--------------------------------------------------------*/
/**
 * @brief	Initializes the hardware
 * @param	None
 * @retval	None
 */
static void prvHardwareInit()
{
	/* Set up the LED outputs */
	__GPIOC_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin  	= backgroundLED_0 | backgroundLED_1 | backgroundLED_2;
	GPIO_InitStructure.Mode  	= GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull		= GPIO_NOPULL;
	GPIO_InitStructure.Speed 	= GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

	HAL_GPIO_WritePin(GPIOC, backgroundLED_0, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, backgroundLED_1, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, backgroundLED_2, GPIO_PIN_SET);

	/* I2C */
	I2C2_Init();

	/* Get the Manufacturer ID from the MCP9808 */
	uint8_t manufacturIdRegister = 0x06;
	I2C2_Transmit(MCP9808_TEMP_SENSOR_ADDRESS, &manufacturIdRegister, 1);
	uint8_t storage[2] = {0x00, 0x00};
	I2C2_Receive(MCP9808_TEMP_SENSOR_ADDRESS, storage, 2);
	uint16_t manufacturerId = (storage[0] << 8) | storage[1];
	if (manufacturerId != 0x0054)
	{
		/* Something is wrong */
	}
}

/* Interrupt Handlers --------------------------------------------------------*/
