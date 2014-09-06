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
		.startState = RelayState_On,
		.msBetweenStateChange = 1000};
static RelayDevice powerRelay = {
		.gpioPort = GPIOE,
		.gpioPin = GPIO_PIN_1,
		.startState = RelayState_Off,
		.msBetweenStateChange = 1000};

/* Default UART handle */
static USART_HandleTypeDef USART_Handle = {
		.Instance 			= UART_CHANNEL,
		.Init.BaudRate 		= 115200,
		.Init.WordLength 	= USART_WORDLENGTH_8B,
		.Init.StopBits		= USART_STOPBITS_1,
		.Init.Parity		= USART_PARITY_NONE,
		.Init.Mode			= USART_MODE_TX_RX,
		.Init.CLKPolarity 	= USART_POLARITY_LOW,
		.Init.CLKPhase		= USART_PHASE_1EDGE,
		.Init.CLKLastBit	= USART_LASTBIT_DISABLE};

/* Private function prototypes -----------------------------------------------*/
static void prvHardwareInit();

/* Functions -----------------------------------------------------------------*/
/**
 * @brief	The main task for the UART2 channel
 * @param	pvParameters:
 * @retval	None
 */
void uart2Task(void *pvParameters)
{
	prvHardwareInit();

	/* The parameter in vTaskDelayUntil is the absolute time
	 * in ticks at which you want to be woken calculated as
	 * an increment from the time you were last woken. */
	TickType_t xNextWakeTime;
	/* Initialize xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	uint8_t data[5] = {0x11, 0x22, 0x33, 0x44, 0x55};
	while (1)
	{
		vTaskDelayUntil(&xNextWakeTime, 100 / portTICK_PERIOD_MS);
		uart2Transmit(data, 5);
	}
}

/**
 * @brief	Set the power of the UART2
 * @param	Power: The power to set, UART2Power_3V3 or UART2Power_5V
 * @retval	None
 */
void uart2SetPower(UART2Power Power)
{
	if (Power == UART2Power_3V3)
		RELAY_SetState(&powerRelay, RelayState_On);
	else if (Power == UART2Power_5V)
		RELAY_SetState(&powerRelay, RelayState_Off);
}

/**
 * @brief	Set whether or not the output should be connected to the connector
 * @param	Connection: Can be any value of UART2Connection
 * @retval	None
 */
void uart2SetConnection(UART2Connection Connection)
{
	if (Connection == UART2Connection_Connected)
		RELAY_SetState(&switchRelay, RelayState_On);
	else if (Connection == UART2Connection_Disconnected)
		RELAY_SetState(&switchRelay, RelayState_Off);
}

/**
 * @brief	Transmit data
 * @param	Data: Pointer to the buffer to send
 * @param	Size: Size of the buffer
 * @retval	None
 */
void uart2Transmit(uint8_t* Data, uint16_t Size)
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

	/* Init GPIO */
	__GPIOA_CLK_ENABLE();

	/* TODO: Configure these USART pins as alternate function pull-up. ??? */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin  		= UART_TX_PIN;
	GPIO_InitStructure.Mode  		= GPIO_MODE_AF_PP;
	GPIO_InitStructure.Alternate 	= GPIO_AF7_USART2;
	GPIO_InitStructure.Pull			= GPIO_NOPULL;
	GPIO_InitStructure.Speed 		= GPIO_SPEED_HIGH;
	HAL_GPIO_Init(UART_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.Pin  		= UART_RX_PIN;
	GPIO_InitStructure.Mode  		= GPIO_MODE_INPUT;		/* TODO: ??? */
	GPIO_InitStructure.Pull			= GPIO_NOPULL;
	GPIO_InitStructure.Speed 		= GPIO_SPEED_HIGH;
	HAL_GPIO_Init(UART_PORT, &GPIO_InitStructure);

	/* Init UART2 */
	__USART2_CLK_ENABLE();
	HAL_USART_Init(&USART_Handle);
}

/* Interrupt Handlers --------------------------------------------------------*/
