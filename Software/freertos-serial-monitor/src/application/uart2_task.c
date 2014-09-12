/**
 ******************************************************************************
 * @file	uart2_task.c
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
#include "uart2_task.h"

#include "relay.h"
#include "spi_flash.h"

#include <string.h>

/* Private defines -----------------------------------------------------------*/
#define UART_CHANNEL	(USART2)

#define UART_TX_PIN		(GPIO_PIN_2)
#define UART_RX_PIN		(GPIO_PIN_3)
#define UART_PORT		(GPIOA)

/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static RelayDevice switchRelay = {
		.gpioPort = GPIOC,
		.gpioPin = GPIO_PIN_1,
		.startState = RelayState_Off,
		.msBetweenStateChange = 500};
static RelayDevice powerRelay = {
		.gpioPort = GPIOE,
		.gpioPin = GPIO_PIN_1,
		.startState = RelayState_Off,
		.msBetweenStateChange = 500};

/* Default UART handle */
static UART_HandleTypeDef UART_Handle = {
		.Instance			= UART_CHANNEL,
		.Init.BaudRate		= UART2BaudRate_115200,
		.Init.WordLength 	= UART_WORDLENGTH_8B,
		.Init.StopBits		= UART_STOPBITS_1,
		.Init.Parity		= UART_PARITY_NONE,
		.Init.Mode			= UART_MODE_TX_RX,
		.Init.HwFlowCtl		= UART_HWCONTROL_NONE,
};

static UART2Settings prvCurrentSettings;
static SemaphoreHandle_t xSemaphore;

static uint8_t prvRxBuffer[8];
static uint8_t prvTxBuffer[128];
static uint32_t prvSizeOfDataInTxBuffer;

uint32_t prvFlashWriteAddress = FLASH_ADR_UART2_DATA;

/* Private function prototypes -----------------------------------------------*/
static void prvHardwareInit();
static void prvEnableUart2Interface();
static void prvDisableUart2Interface();

/* Functions -----------------------------------------------------------------*/
/**
 * @brief	The main task for the UART2 channel
 * @param	pvParameters:
 * @retval	None
 */
void uart2Task(void *pvParameters)
{
	/* Mutex semaphore to manage when it's ok to send and receive new data */
	xSemaphore = xSemaphoreCreateMutex();

	prvHardwareInit();

	/* TODO: Read these from FLASH instead */
	prvCurrentSettings.baudRate = UART_Handle.Init.BaudRate;
	prvCurrentSettings.mode = UART_Handle.Init.Mode;

	/* The parameter in vTaskDelayUntil is the absolute time
	 * in ticks at which you want to be woken calculated as
	 * an increment from the time you were last woken. */
	TickType_t xNextWakeTime;
	/* Initialize xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	vTaskDelay(1000 / portTICK_PERIOD_MS);
	SPI_FLASH_EraseSector(FLASH_ADR_UART2_DATA);

	uint8_t* data = "UART2 Debug! ";
	while (1)
	{
		vTaskDelayUntil(&xNextWakeTime, 1000 / portTICK_PERIOD_MS);

		/* Transmit debug data if that mode is active */
		if (prvCurrentSettings.connection == UART2Connection_Connected && prvCurrentSettings.mode == UART2Mode_DebugTX)
			uart2Transmit(data, strlen(data));
	}
}

/**
 * @brief	Set the power of the UART2
 * @param	Power: The power to set, UART2Power_3V3 or UART2Power_5V
 * @retval	SUCCES: Everything went ok
 * @retval	ERROR: Something went wrong
 */
ErrorStatus uart2SetPower(UART2Power Power)
{
	RelayStatus status = RelayStatus_NotEnoughTimePassed;;
	if (Power == UART2Power_3V3)
		status = RELAY_SetState(&powerRelay, RelayState_On);
	else if (Power == UART2Power_5V)
		status = RELAY_SetState(&powerRelay, RelayState_Off);

	if (status == RelayStatus_Ok)
	{
		prvCurrentSettings.power = Power;
		return SUCCESS;
	}
	else
		return ERROR;
}

/**
 * @brief	Set whether or not the output should be connected to the connector
 * @param	Connection: Can be any value of UART2Connection
 * @retval	SUCCES: Everything went ok
 * @retval	ERROR: Something went wrong
 */
ErrorStatus uart2SetConnection(UART2Connection Connection)
{
	RelayStatus status = RelayStatus_NotEnoughTimePassed;
	if (Connection == UART2Connection_Connected)
		status = RELAY_SetState(&switchRelay, RelayState_On);
	else if (Connection == UART2Connection_Disconnected)
		status = RELAY_SetState(&switchRelay, RelayState_Off);

	if (status == RelayStatus_Ok)
	{
		prvCurrentSettings.connection = Connection;
		if (Connection == UART2Connection_Connected)
			prvEnableUart2Interface();
		else
			prvDisableUart2Interface();
		return SUCCESS;
	}
	else
		return ERROR;
}

/**
 * @brief	Get the current settings of the UART2 channel
 * @param	None
 * @retval	A UART2Settings with all the settings
 */
UART2Settings uart2GetSettings()
{
	return prvCurrentSettings;
}

/**
 * @brief	Set the settings of the UART2 channel
 * @param	Settings: New settings to use
 * @retval	SUCCESS: Everything went ok
 * @retval	ERROR: Something went wrong
 */
ErrorStatus uart2SetSettings(UART2Settings* Settings)
{
	mempcpy(&prvCurrentSettings, Settings, sizeof(UART2Settings));

	/* Set the values in the UART handle */
	UART_Handle.Init.BaudRate = prvCurrentSettings.baudRate;
	if (prvCurrentSettings.mode == UART2Mode_DebugTX)
		UART_Handle.Init.Mode = UART2Mode_TX_RX;
	else
		UART_Handle.Init.Mode = prvCurrentSettings.mode;

	return SUCCESS;
}

/**
 * @brief	Transmit data
 * @param	Data: Pointer to the buffer to send
 * @param	Size: Size of the buffer
 * @retval	None
 */
void uart2Transmit(uint8_t* Data, uint16_t Size)
{
	/* Make sure the UART is available */
	if (Size != 0 && xSemaphoreTake(xSemaphore, 100) == pdTRUE)
	{
		/* Copy data to the internal buffer and start sending */
		memcpy(prvTxBuffer, Data, Size);
		prvSizeOfDataInTxBuffer = Size;

		HAL_UART_Transmit_IT(&UART_Handle, prvTxBuffer, prvSizeOfDataInTxBuffer);
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
	RELAY_Init(&powerRelay);

	/* Init GPIO */
	__GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin  		= UART_TX_PIN | UART_RX_PIN;
	GPIO_InitStructure.Mode  		= GPIO_MODE_AF_PP;
	GPIO_InitStructure.Alternate 	= GPIO_AF7_USART2;
	GPIO_InitStructure.Pull			= GPIO_PULLUP;
	GPIO_InitStructure.Speed 		= GPIO_SPEED_HIGH;
	HAL_GPIO_Init(UART_PORT, &GPIO_InitStructure);
}

/**
 * @brief	Enables the UART2 interface with the current settings
 * @param	None
 * @retval	None
 */
static void prvEnableUart2Interface()
{
	/* Enable UART clock */
	__USART2_CLK_ENABLE();

	/* Configure priority and enable interrupt */
	HAL_NVIC_SetPriority(USART2_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY, 0);
	HAL_NVIC_EnableIRQ(USART2_IRQn);

	/* Enable the UART */
	HAL_UART_Init(&UART_Handle);

	/* If we are in RX mode we should start receiving data */
	if (UART_Handle.Init.Mode == UART2Mode_RX || UART_Handle.Init.Mode == UART2Mode_TX_RX)
		HAL_UART_Receive_IT(&UART_Handle, prvRxBuffer, 1);
}

/**
 * @brief	Disables the UART2 interface
 * @param	None
 * @retval	None
 */
static void prvDisableUart2Interface()
{
	HAL_NVIC_DisableIRQ(USART2_IRQn);
	HAL_UART_DeInit(&UART_Handle);
	__USART2_CLK_DISABLE();

	/* Give back the sempahore in case it was taken */
	xSemaphoreGive(xSemaphore);
}

/* Interrupt Handlers --------------------------------------------------------*/
/**
  * @brief  This function handles UART2 interrupt request
  * @param  None
  * @retval None
  */
void USART2_IRQHandler(void)
{
	HAL_UART_IRQHandler(&UART_Handle);
}

/* HAL Callback functions ----------------------------------------------------*/
/**
  * @brief  Tx Transfer completed callback
  * @param  None
  * @retval None
  */
void uart2TxCpltCallback()
{
	/* Give back the semaphore now that we are done */
	xSemaphoreGiveFromISR(xSemaphore, NULL);
}

/**
  * @brief  Rx Transfer completed callback
  * @param  None
  * @retval None
  */
void uart2RxCpltCallback()
{
	/* TODO: Do something with the data received */
	/* Continue receiving data */
	HAL_UART_Receive_IT(&UART_Handle, prvRxBuffer, 1);
	/* Give back the semaphore now that we are done */
	xSemaphoreGiveFromISR(xSemaphore, NULL);
}

/**
  * @brief  UART error callback
  * @param  None
  * @retval None
  */
 void uart2ErrorCallback()
{
	/* Give back the semaphore now that we are done */
	 xSemaphoreGiveFromISR(xSemaphore, NULL);
	/* TODO: Indicate error somehow ??? */
}
