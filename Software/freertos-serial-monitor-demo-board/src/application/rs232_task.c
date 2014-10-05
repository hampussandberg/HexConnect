/**
 ******************************************************************************
 * @file	rs232_task.c
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-09-06
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
#include "rs232_task.h"

#include "relay.h"
#include "spi_flash_memory_map.h"
#include "spi_flash.h"

#include <string.h>
#include <stdbool.h>

/* Private defines -----------------------------------------------------------*/
#define UART_CHANNEL	(UART4)

#define UART_TX_PIN		(GPIO_PIN_0)
#define UART_RX_PIN		(GPIO_PIN_1)
#define UART_PORT		(GPIOA)

#define RX_BUFFER_SIZE	(256)

/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static RelayDevice switchRelay = {
		.gpioPort = GPIOE,
		.gpioPin = GPIO_PIN_6,
		.startState = RelayState_Off,
		.msBetweenStateChange = 1000};

/* Default UART handle */
static UART_HandleTypeDef UART_Handle = {
		.Instance			= UART_CHANNEL,
		.Init.BaudRate		= UARTBaudRate_115200,
		.Init.WordLength 	= UART_WORDLENGTH_8B,
		.Init.StopBits		= UART_STOPBITS_1,
		.Init.Parity		= UART_PARITY_NONE,
		.Init.Mode			= UART_MODE_TX_RX,
		.Init.HwFlowCtl		= UART_HWCONTROL_NONE,
};

/* Default settings that can be overwritten if valid settings are read from the SPI FLASH */
static UARTSettings prvCurrentSettings = {
		.connection 					= UARTConnection_Disconnected,
		.baudRate						= UARTBaudRate_115200,
		.parity							= UARTParity_None,
		.power							= UARTPower_5V,
		.mode							= UARTMode_TX_RX,
		.writeFormat					= GUIWriteFormat_ASCII,
		.numOfCharactersPerByte			= 1,
		.displayedDataStartAddress 		= FLASH_ADR_RS232_DATA,
		.lastDisplayDataStartAddress	= FLASH_ADR_RS232_DATA,
		.displayedDataEndAddress		= FLASH_ADR_RS232_DATA,
		.lastDisplayDataEndAddress		= FLASH_ADR_RS232_DATA,
		.readAddress					= FLASH_ADR_RS232_DATA,
		.writeAddress					= FLASH_ADR_RS232_DATA,
		.numOfCharactersDisplayed		= 0,
		.amountOfDataSaved				= 0,
		.scrolling						= false,
};

static SemaphoreHandle_t xSemaphore;
static SemaphoreHandle_t xSettingsSemaphore;

static uint8_t prvReceivedByte;

static uint8_t prvRxBuffer1[RX_BUFFER_SIZE];
static uint32_t prvRxBuffer1CurrentIndex = 0;
static uint32_t prvRxBuffer1Count = 0;
static BUFFERState prvRxBuffer1State = BUFFERState_Writing;
static TimerHandle_t prvBuffer1ClearTimer;

static uint8_t prvRxBuffer2[RX_BUFFER_SIZE];
static uint32_t prvRxBuffer2CurrentIndex = 0;
static uint32_t prvRxBuffer2Count = 0;
static BUFFERState prvRxBuffer2State = BUFFERState_Writing;
static TimerHandle_t prvBuffer2ClearTimer;

static bool prvDoneInitializing = false;

/* Private function prototypes -----------------------------------------------*/
static void prvHardwareInit();
static void prvEnableRs232Interface();
static void prvDisableRs232Interface();
static void prvReadSettingsFromSpiFlash();

static void prvBuffer1ClearTimerCallback();
static void prvBuffer2ClearTimerCallback();

/* Functions -----------------------------------------------------------------*/
/**
 * @brief	The main task for the RS232 channel
 * @param	pvParameters:
 * @retval	None
 */
void rs232Task(void *pvParameters)
{
	/* Mutex semaphore to manage when it's ok to send and receive new data */
	xSemaphore = xSemaphoreCreateMutex();

	/* Mutex semaphore for accessing the settings for this channel */
	xSettingsSemaphore = xSemaphoreCreateMutex();

	/* Create software timers */
	prvBuffer1ClearTimer = xTimerCreate("Buf1Clear4", 10, pdFALSE, 0, prvBuffer1ClearTimerCallback);
	prvBuffer2ClearTimer = xTimerCreate("Buf2Clear5", 10, pdFALSE, 0, prvBuffer2ClearTimerCallback);

	/* Initialize hardware */
	prvHardwareInit();

	/* Wait to make sure the SPI FLASH is initialized */
	while (SPI_FLASH_Initialized() == false)
	{
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}

	/* Try to read the settings from SPI FLASH */
	prvReadSettingsFromSpiFlash();

	/*
	 * TODO: Figure out a good way to allow saved data in SPI FLASH to be read next time we wake up so that we
	 * don't have to do a clear every time we start up the device.
	 */
	rs232Clear();

	uint8_t* data = "RS232 Debug! ";

	/* The parameter in vTaskDelayUntil is the absolute time
	 * in ticks at which you want to be woken calculated as
	 * an increment from the time you were last woken. */
	TickType_t xNextWakeTime;
	/* Initialize xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	prvDoneInitializing = true;
	while (1)
	{
		vTaskDelayUntil(&xNextWakeTime, 500 / portTICK_PERIOD_MS);

		/* Transmit debug data if that mode is active */
		if (prvCurrentSettings.connection == UARTConnection_Connected && prvCurrentSettings.mode == UARTMode_DebugTX)
			rs232Transmit(data, strlen(data));
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
bool rs232IsDoneInitializing()
{
	return prvDoneInitializing;
}

/**
 * @brief	Restart the UART1
 * @param	None
 * @retval	None
 */
void rs232Restart()
{
	prvDisableRs232Interface();
	prvEnableRs232Interface();
}

/**
 * @brief	Set whether or not the output should be connected to the connector
 * @param	Connection: Can be any value of UARTConnection
 * @retval	SUCCES: Everything went ok
 * @retval	ERROR: Something went wrong
 */
ErrorStatus rs232SetConnection(UARTConnection Connection)
{
	RelayStatus status = RelayStatus_NotEnoughTimePassed;
	if (Connection == UARTConnection_Connected)
		status = RELAY_SetState(&switchRelay, RelayState_On);
	else if (Connection == UARTConnection_Disconnected)
		status = RELAY_SetState(&switchRelay, RelayState_Off);

	if (status == RelayStatus_Ok)
	{
		prvCurrentSettings.connection = Connection;
		if (Connection == UARTConnection_Connected)
			prvEnableRs232Interface();
		else
			prvDisableRs232Interface();
		return SUCCESS;
	}
	else
		return ERROR;
}

/**
 * @brief	Get the current settings of the RS232 channel
 * @param	None
 * @retval	A pointer to the current settings
 */
UARTSettings* rs232GetSettings()
{
	return &prvCurrentSettings;
}

/**
 * @brief	Update with the new settings stored in prvCurrentSettings
 * @param	None
 * @retval	SUCCESS: Everything went ok
 * @retval	ERROR: Something went wrong
 */
ErrorStatus rs232UpdateWithNewSettings()
{
	/* Set the values in the USART handle */
	UART_Handle.Init.BaudRate = prvCurrentSettings.baudRate;
	UART_Handle.Init.Parity = prvCurrentSettings.parity;

	if (prvCurrentSettings.mode == UARTMode_DebugTX)
		UART_Handle.Init.Mode = UARTMode_TX_RX;
	else
		UART_Handle.Init.Mode = prvCurrentSettings.mode;

	return SUCCESS;
}

/**
 * @brief	Get the settings semaphore
 * @param	None
 * @retval	A pointer to the settings semaphore
 */
SemaphoreHandle_t* rs232GetSettingsSemaphore()
{
	return &xSettingsSemaphore;
}

/**
 * @brief	Clear the channel
 * @param	None
 * @retval	SUCCESS: Everything went ok
 * @retval	ERROR: Something went wrong
 */
ErrorStatus rs232Clear()
{
	/* Try to take the settings semaphore */
	if (xSettingsSemaphore != 0 && xSemaphoreTake(xSettingsSemaphore, 100) == pdTRUE)
	{
		prvCurrentSettings.displayedDataStartAddress = FLASH_ADR_RS232_DATA;
		prvCurrentSettings.lastDisplayDataStartAddress = FLASH_ADR_RS232_DATA;
		prvCurrentSettings.displayedDataEndAddress = FLASH_ADR_RS232_DATA;
		prvCurrentSettings.lastDisplayDataEndAddress = FLASH_ADR_RS232_DATA;
		prvCurrentSettings.readAddress = FLASH_ADR_RS232_DATA;
		prvCurrentSettings.writeAddress = FLASH_ADR_RS232_DATA;
		prvCurrentSettings.numOfCharactersDisplayed = 0;
		prvCurrentSettings.amountOfDataSaved = 0;
		prvCurrentSettings.scrolling = false;

		/* TODO: Check which of the sectors should be erased, it can be more than one! */
		SPI_FLASH_EraseSector(FLASH_ADR_RS232_DATA);

		/* Give back the semaphore now that we are done */
		xSemaphoreGive(xSettingsSemaphore);

		return SUCCESS;
	}
	else
	{
		return ERROR;
	}
}

/**
 * @brief	Returns the address which the RS232 data will be written to next
 * @param	None
 * @retval	The address
 */
uint32_t rs232GetCurrentWriteAddress()
{
	return prvCurrentSettings.writeAddress;
}

/**
 * @brief	Transmit data
 * @param	Data: Pointer to the buffer to send
 * @param	Size: Size of the buffer
 * @retval	None
 */
void rs232Transmit(uint8_t* Data, uint32_t Size)
{
	/* Make sure the UART is available */
	if (Size != 0 && xSemaphoreTake(xSemaphore, 100) == pdTRUE)
	{
		HAL_UART_Transmit_IT(&UART_Handle, Data, Size);
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
	/* Init relays */
	RELAY_Init(&switchRelay);

	/* Init GPIO */
	__GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin  		= UART_TX_PIN | UART_RX_PIN;
	GPIO_InitStructure.Mode  		= GPIO_MODE_AF_PP;
	GPIO_InitStructure.Alternate 	= GPIO_AF8_UART4;
	GPIO_InitStructure.Pull			= GPIO_NOPULL;
	GPIO_InitStructure.Speed 		= GPIO_SPEED_HIGH;
	HAL_GPIO_Init(UART_PORT, &GPIO_InitStructure);
}

/**
 * @brief	Enables the RS232 interface with the current settings
 * @param	None
 * @retval	None
 */
static void prvEnableRs232Interface()
{
	/* Enable UART clock */
	__UART4_CLK_ENABLE();

	/* Configure priority and enable interrupt */
	HAL_NVIC_SetPriority(UART4_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY, 0);
	HAL_NVIC_EnableIRQ(UART4_IRQn);

	/* Enable the UART */
	HAL_UART_Init(&UART_Handle);

	/* If we are in RX mode we should start receiving data */
	if (UART_Handle.Init.Mode == UARTMode_RX || UART_Handle.Init.Mode == UARTMode_TX_RX)
		HAL_UART_Receive_IT(&UART_Handle, &prvReceivedByte, 1);
}

/**
 * @brief	Disables the RS232 interface
 * @param	None
 * @retval	None
 */
static void prvDisableRs232Interface()
{
	HAL_NVIC_DisableIRQ(UART4_IRQn);
	HAL_UART_DeInit(&UART_Handle);
	__UART4_CLK_DISABLE();
	xSemaphoreGive(xSemaphore);

	prvRxBuffer1CurrentIndex = 0;
	prvRxBuffer1Count = 0;
	prvRxBuffer1State = BUFFERState_Writing;
	prvRxBuffer2CurrentIndex = 0;
	prvRxBuffer2Count = 0;
	prvRxBuffer2State = BUFFERState_Writing;
}

/**
 * @brief	Read settings from SPI FLASH
 * @param	None
 * @retval	None
 */
static void prvReadSettingsFromSpiFlash()
{
	/* Read to a temporary settings variable */
	UARTSettings settings;
	SPI_FLASH_ReadBufferDMA((uint8_t*)&settings, FLASH_ADR_RS232_SETTINGS, sizeof(UARTSettings));

	/* Check to make sure the data is reasonable */
	if (IS_UART_CONNECTION(settings.connection) &&
		IS_UART_BAUDRATE(settings.baudRate) &&
		IS_UART_POWER(settings.power) &&
		IS_UART_MODE_APP(settings.mode) &&
		IS_GUI_WRITE_FORMAT(settings.writeFormat))
	{
		/* Try to take the settings semaphore */
		if (xSettingsSemaphore != 0 && xSemaphoreTake(xSettingsSemaphore, 100) == pdTRUE)
		{
			/* Copy to the real settings variable */
			memcpy(&prvCurrentSettings, &settings, sizeof(UARTSettings));
			prvCurrentSettings.power = UARTPower_5V;
			prvCurrentSettings.mode = UARTMode_TX_RX;
			/* Give back the semaphore now that we are done */
			xSemaphoreGive(xSettingsSemaphore);
		}
	}
}

/**
 * @brief	Callback for the buffer 1 software timer
 * @param	None
 * @retval	None
 */
static void prvBuffer1ClearTimerCallback()
{
	/* Set the buffer to reading state */
	prvRxBuffer1State = BUFFERState_Reading;

	/* Write the data to FLASH */
	for (uint32_t i = 0; i < prvRxBuffer1Count; i++)
		SPI_FLASH_WriteByte(prvCurrentSettings.writeAddress++, prvRxBuffer1[i]);
	/* TODO: Something strange with the FLASH page write so doing one byte at a time now */
//	/* Write all the data in the buffer to SPI FLASH */
//	SPI_FLASH_WriteBuffer(prvRxBuffer1, prvCurrentSettings.writeAddress, prvRxBuffer1Count);
//	/* Update the write address */
//	prvCurrentSettings.writeAddress += prvRxBuffer1Count;

	/* Save how many bytes we saved */
	prvCurrentSettings.amountOfDataSaved += prvRxBuffer1Count;

	/* Reset the buffer */
	prvRxBuffer1CurrentIndex = 0;
	prvRxBuffer1Count = 0;
	prvRxBuffer1State = BUFFERState_Writing;
}

/**
 * @brief	Callback for the buffer 2 software timer
 * @param	None
 * @retval	None
 */
static void prvBuffer2ClearTimerCallback()
{
	/* Set the buffer to reading state */
	prvRxBuffer2State = BUFFERState_Reading;

	/* Write the data to FLASH */
	for (uint32_t i = 0; i < prvRxBuffer2Count; i++)
		SPI_FLASH_WriteByte(prvCurrentSettings.writeAddress++, prvRxBuffer2[i]);
	/* TODO: Something strange with the FLASH page write so doing one byte at a time now */
//	/* Write all the data in the buffer to SPI FLASH */
//	SPI_FLASH_WriteBuffer(prvRxBuffer2, prvCurrentSettings.writeAddress, prvRxBuffer2Count);
//	/* Update the write address */
//	prvCurrentSettings.writeAddress += prvRxBuffer2Count;

	/* Save how many bytes we saved */
	prvCurrentSettings.amountOfDataSaved += prvRxBuffer2Count;

	/* Reset the buffer */
	prvRxBuffer2CurrentIndex = 0;
	prvRxBuffer2Count = 0;
	prvRxBuffer2State = BUFFERState_Writing;
}

/* Interrupt Handlers --------------------------------------------------------*/
/**
  * @brief  This function handles UART1 interrupt request
  * @param  None
  * @retval None
  */
void UART4_IRQHandler(void)
{
	HAL_UART_IRQHandler(&UART_Handle);
}

/* HAL Callback functions ----------------------------------------------------*/
/**
  * @brief  Tx Transfer completed callback
  * @param  None
  * @retval None
  */
void rs232TxCpltCallback()
{
	/* Give back the semaphore now that we are done */
	xSemaphoreGiveFromISR(xSemaphore, NULL);
}

/**
  * @brief  Rx Transfer completed callback
  * @param  None
  * @retval None
  */
void rs232RxCpltCallback()
{
	if (prvRxBuffer1State != BUFFERState_Reading && prvRxBuffer1Count < RX_BUFFER_SIZE)
	{
		prvRxBuffer1State = BUFFERState_Writing;
		prvRxBuffer1[prvRxBuffer1CurrentIndex++] = prvReceivedByte;
		prvRxBuffer1Count++;
		/* Start the timer which will clear the buffer if it's not already started */
		if (xTimerIsTimerActive(prvBuffer1ClearTimer) == pdFALSE)
			xTimerStartFromISR(prvBuffer1ClearTimer, NULL);
	}
	else if (prvRxBuffer2State != BUFFERState_Reading && prvRxBuffer2Count < RX_BUFFER_SIZE)
	{
		prvRxBuffer2State = BUFFERState_Writing;
		prvRxBuffer2[prvRxBuffer2CurrentIndex++] = prvReceivedByte;
		prvRxBuffer2Count++;
		/* Start the timer which will clear the buffer if it's not already started */
		if (xTimerIsTimerActive(prvBuffer2ClearTimer) == pdFALSE)
			xTimerStartFromISR(prvBuffer2ClearTimer, NULL);
	}
	else
	{
		/* No buffer available, something has gone wrong */
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_3);
	}

	/* Continue receiving data */
	HAL_UART_Receive_IT(&UART_Handle, &prvReceivedByte, 1);
	/* Give back the semaphore now that we are done */
	xSemaphoreGiveFromISR(xSemaphore, NULL);
}

/**
  * @brief  UART error callback
  * @param  None
  * @retval None
  */
 void rs232ErrorCallback()
{
	/* Give back the semaphore now that we are done */
	xSemaphoreGiveFromISR(xSemaphore, NULL);
	/* TODO: Indicate error somehow ??? */
}
