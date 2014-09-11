/**
 ******************************************************************************
 * @file	uart1_task.c
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
#include "uart1_task.h"

#include "relay.h"

#include <string.h>

/* Private defines -----------------------------------------------------------*/
#define UART_CHANNEL	(USART1)

#define UART_TX_PIN		(GPIO_PIN_9)
#define UART_RX_PIN		(GPIO_PIN_10)
#define UART_PORT		(GPIOA)

/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static RelayDevice switchRelay = {
		.gpioPort = GPIOC,
		.gpioPin = GPIO_PIN_0,
		.startState = RelayState_Off,
		.msBetweenStateChange = 500};
static RelayDevice powerRelay = {
		.gpioPort = GPIOE,
		.gpioPin = GPIO_PIN_0,
		.startState = RelayState_Off,
		.msBetweenStateChange = 500};

/* Default UART handle */
static USART_HandleTypeDef USART_Handle = {
		.Instance 			= UART_CHANNEL,
		.Init.BaudRate 		= UART1BaudRate_115200,
		.Init.WordLength 	= USART_WORDLENGTH_8B,
		.Init.StopBits		= USART_STOPBITS_1,
		.Init.Parity		= USART_PARITY_NONE,
		.Init.Mode			= USART_MODE_TX_RX,
		.Init.CLKPolarity 	= USART_POLARITY_LOW,
		.Init.CLKPhase		= USART_PHASE_1EDGE,
		.Init.CLKLastBit	= USART_LASTBIT_DISABLE};

static UART1Settings prvCurrentSettings;

/* Private function prototypes -----------------------------------------------*/
static void prvHardwareInit();
static void prvEnableUart1Interface();
static void prvDisableUart1Interface();

/* Functions -----------------------------------------------------------------*/
/**
 * @brief	The main task for the UART1 channel
 * @param	pvParameters:
 * @retval	None
 */
void uart1Task(void *pvParameters)
{
	prvHardwareInit();

	/* TODO: Read these from FLASH instead */
	prvCurrentSettings.baudRate = USART_Handle.Init.BaudRate;
	prvCurrentSettings.mode = USART_Handle.Init.Mode;

	/* The parameter in vTaskDelayUntil is the absolute time
	 * in ticks at which you want to be woken calculated as
	 * an increment from the time you were last woken. */
	TickType_t xNextWakeTime;
	/* Initialize xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	uint8_t* data = "UART1 Debug! ";
	while (1)
	{
		vTaskDelayUntil(&xNextWakeTime, 100 / portTICK_PERIOD_MS);

		/* Transmit debug data if that mode is active */
		if (prvCurrentSettings.connection == UART1Connection_Connected && prvCurrentSettings.mode == UART1Mode_DebugTX)
			uart1Transmit(data, strlen(data));
	}
}

/**
 * @brief	Set the power of the UART1
 * @param	Power: The power to set, UART1Power_3V3 or UART1Power_5V
 * @retval	SUCCES: Everything went ok
 * @retval	ERROR: Something went wrong
 */
ErrorStatus uart1SetPower(UART1Power Power)
{
	RelayStatus status = RelayStatus_NotEnoughTimePassed;;
	if (Power == UART1Power_3V3)
		status = RELAY_SetState(&powerRelay, RelayState_On);
	else if (Power == UART1Power_5V)
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
 * @param	Connection: Can be any value of UART1Connection
 * @retval	SUCCES: Everything went ok
 * @retval	ERROR: Something went wrong
 */
ErrorStatus uart1SetConnection(UART1Connection Connection)
{
	RelayStatus status = RelayStatus_NotEnoughTimePassed;
	if (Connection == UART1Connection_Connected)
		status = RELAY_SetState(&switchRelay, RelayState_On);
	else if (Connection == UART1Connection_Disconnected)
		status = RELAY_SetState(&switchRelay, RelayState_Off);

	if (status == RelayStatus_Ok)
	{
		prvCurrentSettings.connection = Connection;
		if (Connection == UART1Connection_Connected)
			prvEnableUart1Interface();
		else
			prvDisableUart1Interface();
		return SUCCESS;
	}
	else
		return ERROR;
}

/**
 * @brief	Get the current settings of the UART1 channel
 * @param	None
 * @retval	A UART1Settings with all the settings
 */
UART1Settings uart1GetSettings()
{
	return prvCurrentSettings;
}

/**
 * @brief	Set the settings of the UART1 channel
 * @param	Settings: New settings to use
 * @retval	SUCCESS: Everything went ok
 * @retval	ERROR: Something went wrong
 */
ErrorStatus uart1SetSettings(UART1Settings* Settings)
{
	mempcpy(&prvCurrentSettings, Settings, sizeof(UART1Settings));

	/* Set the values in the USART handle */
	USART_Handle.Init.BaudRate = prvCurrentSettings.baudRate;
	if (prvCurrentSettings.mode == UART1Mode_DebugTX)
		USART_Handle.Init.Mode = UART1Mode_TX_RX;
	else
		USART_Handle.Init.Mode = prvCurrentSettings.mode;

	return SUCCESS;
}

/**
 * @brief	Transmit data
 * @param	Data: Pointer to the buffer to send
 * @param	Size: Size of the buffer
 * @retval	None
 */
void uart1Transmit(uint8_t* Data, uint16_t Size)
{
	/* TODO: Check timeout! */
	HAL_USART_Transmit(&USART_Handle, Data, Size, 500);
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
}

/**
 * @brief	Enables the UART1 interface with the current settings
 * @param	None
 * @retval	None
 */
static void prvEnableUart1Interface()
{
	/* Init GPIO */
	__GPIOA_CLK_ENABLE();

	/* TODO: Configure these USART pins as alternate function pull-up. ??? */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin  		= UART_TX_PIN;
	GPIO_InitStructure.Mode  		= GPIO_MODE_AF_PP;
	GPIO_InitStructure.Alternate 	= GPIO_AF7_USART1;
	GPIO_InitStructure.Pull			= GPIO_NOPULL;
	GPIO_InitStructure.Speed 		= GPIO_SPEED_HIGH;
	HAL_GPIO_Init(UART_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.Pin  		= UART_RX_PIN;
	GPIO_InitStructure.Mode  		= GPIO_MODE_INPUT;		/* TODO: ??? */
	GPIO_InitStructure.Pull			= GPIO_NOPULL;
	GPIO_InitStructure.Speed 		= GPIO_SPEED_HIGH;
	HAL_GPIO_Init(UART_PORT, &GPIO_InitStructure);

	/* Init UART channel */
	__USART1_CLK_ENABLE();
	HAL_USART_Init(&USART_Handle);
}

/**
 * @brief	Disables the UART1 interface
 * @param	None
 * @retval	None
 */
static void prvDisableUart1Interface()
{
	HAL_USART_DeInit(&USART_Handle);
	__USART1_CLK_DISABLE();
}

/* Interrupt Handlers --------------------------------------------------------*/
