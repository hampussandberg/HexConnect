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
/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static bool prvDoneInitializing = false;

/* Private function prototypes -----------------------------------------------*/
static void prvHardwareInit();
static void prvReadSettingsFromSpiFlash();

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
	while (SPI_FLASH_Initialized() == false)
	{
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}

	/* Try to read the settings from SPI FLASH */
	prvReadSettingsFromSpiFlash();


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
	MAX1301_Init();
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


/* Interrupt Handlers --------------------------------------------------------*/
/* HAL Callback functions ----------------------------------------------------*/
