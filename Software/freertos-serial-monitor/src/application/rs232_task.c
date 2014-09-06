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

/* Private defines -----------------------------------------------------------*/
#define UART_CHANNEL	(UART4)

#define UART_TX_PIN		(GPIO_PIN_0)
#define UART_RX_PIN		(GPIO_PIN_1)
#define UART_PORT		(GPIOA)

/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static RelayDevice switchRelay = {
		.gpioPort = GPIOE,
		.gpioPin = GPIO_PIN_6,
		.startState = RelayState_On,
		.msBetweenStateChange = 1000};

/* Default UART handle */
static UART_HandleTypeDef UART_Handle = {
		.Instance 			= UART_CHANNEL,
		.Init.BaudRate 		= 115200,
		.Init.WordLength 	= UART_WORDLENGTH_8B,
		.Init.StopBits		= UART_STOPBITS_1,
		.Init.Parity		= UART_PARITY_NONE,
		.Init.Mode			= UART_MODE_TX_RX,
		.Init.HwFlowCtl 	= UART_HWCONTROL_NONE,
		.Init.OverSampling	= UART_OVERSAMPLING_8};	/* TODO: ??? */

/* Private function prototypes -----------------------------------------------*/
static void prvHardwareInit();

/* Functions -----------------------------------------------------------------*/
/**
 * @brief	The main task for the RS232 channel
 * @param	pvParameters:
 * @retval	None
 */
void rs232Task(void *pvParameters)
{
	prvHardwareInit();

	/* The parameter in vTaskDelayUntil is the absolute time
	 * in ticks at which you want to be woken calculated as
	 * an increment from the time you were last woken. */
	TickType_t xNextWakeTime;
	/* Initialize xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	uint8_t data[5] = {0x1F, 0x2E, 0x3D, 0x4C, 0x5B};
	while (1)
	{
		vTaskDelayUntil(&xNextWakeTime, 100 / portTICK_PERIOD_MS);
		rs232Transmit(data, 5);
	}
}

/**
 * @brief	Set whether or not the output should be connected to the connector
 * @param	Connection: Can be any value of RS232Connection
 * @retval	None
 */
void rs232SetConnection(RS232Connection Connection)
{
	if (Connection == RS232Connection_Connected)
		RELAY_SetState(&switchRelay, RelayState_On);
	else if (Connection == RS232Connection_Disconnected)
		RELAY_SetState(&switchRelay, RelayState_Off);
}

/**
 * @brief	Transmit data
 * @param	Data: Pointer to the buffer to send
 * @param	Size: Size of the buffer
 * @retval	None
 */
void rs232Transmit(uint8_t* Data, uint16_t Size)
{
	/* TODO: Check timeout! */
	HAL_UART_Transmit(&UART_Handle, Data, Size, 500);
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

	/* TODO: Configure these USART pins as alternate function pull-up. ??? */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin  		= UART_TX_PIN;
	GPIO_InitStructure.Mode  		= GPIO_MODE_AF_PP;
	GPIO_InitStructure.Alternate 	= GPIO_AF8_UART4;
	GPIO_InitStructure.Pull			= GPIO_NOPULL;
	GPIO_InitStructure.Speed 		= GPIO_SPEED_HIGH;
	HAL_GPIO_Init(UART_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.Pin  		= UART_RX_PIN;
	GPIO_InitStructure.Mode  		= GPIO_MODE_INPUT;		/* TODO: ??? */
	GPIO_InitStructure.Pull			= GPIO_NOPULL;
	GPIO_InitStructure.Speed 		= GPIO_SPEED_HIGH;
	HAL_GPIO_Init(UART_PORT, &GPIO_InitStructure);

	/* Init UART2 */
	__UART4_CLK_ENABLE();
	HAL_UART_Init(&UART_Handle);
}

/* Interrupt Handlers --------------------------------------------------------*/
