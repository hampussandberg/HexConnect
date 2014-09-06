/**
 ******************************************************************************
 * @file	i2c2.c
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-09-07
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
#include "i2c2.h"

/* Private defines -----------------------------------------------------------*/
#define I2C_PERIPHERAL		(I2C2)

#define I2C_PORT			(GPIOB)
#define I2C_SCL_PIN			(GPIO_PIN_10)
#define I2C_SDA_PIN			(GPIO_PIN_11)

#define I2C_CLOCK_SPEED		(400000)

/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static I2C_HandleTypeDef I2C_Handle = {
		.Instance 				= I2C_PERIPHERAL,
		.Init.ClockSpeed 		= I2C_CLOCK_SPEED,
		.Init.DutyCycle 		= I2C_DUTYCYCLE_2,
		.Init.AddressingMode 	= I2C_ADDRESSINGMODE_7BIT,
		.Init.DualAddressMode 	= I2C_DUALADDRESS_DISABLED,
		.Init.GeneralCallMode	= I2C_GENERALCALL_DISABLED,
		.Init.NoStretchMode		= I2C_NOSTRETCH_DISABLED,
};

static SemaphoreHandle_t xSemaphore;
/* Private function prototypes -----------------------------------------------*/

/* Functions -----------------------------------------------------------------*/
/**
 * @brief	Initializes the I2C
 * @param	None
 * @retval	None
 */
void I2C2_Init()
{
	/* Mutex semaphore for mutual exclusion to the I2C2 device */
	xSemaphore = xSemaphoreCreateMutex();

	if (xSemaphoreTake(xSemaphore, 100) == pdTRUE)
	{
		/* I2C clock & GPIOB enable */
		__GPIOB_CLK_ENABLE();
		__I2C2_CLK_ENABLE();

		/* I2C SDA and SCL configuration */
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_InitStructure.Pin  		= I2C_SCL_PIN | I2C_SDA_PIN;
		GPIO_InitStructure.Mode  		= GPIO_MODE_AF_OD;
		GPIO_InitStructure.Alternate 	= GPIO_AF4_I2C2;
		GPIO_InitStructure.Pull			= GPIO_NOPULL;
		GPIO_InitStructure.Speed 		= GPIO_SPEED_HIGH;
		HAL_GPIO_Init(I2C_PORT, &GPIO_InitStructure);

	//	/* NVIC Configuration */
	//	NVIC_InitTypeDef NVIC_InitStructure;
	//	/* Event interrupt */
	//	NVIC_InitStructure.NVIC_IRQChannel = I2C1_EV_IRQn;
	//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configMAX_SYSCALL_INTERRUPT_PRIORITY;
	//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	//	NVIC_Init(&NVIC_InitStructure);
	//	/* Error interrupt */
	//	NVIC_InitStructure.NVIC_IRQChannel = I2C1_ER_IRQn;
	//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 14;
	//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	//	NVIC_Init(&NVIC_InitStructure);

		/* I2C Init */
		HAL_I2C_Init(&I2C_Handle);

		/* Enable the I2C1 interrupts */
	//	I2C_ITConfig(I2C_PERIPHERAL, I2C_IT_EVT, ENABLE);
	//	I2C_ITConfig(I2C_PERIPHERAL, I2C_IT_ERR, ENABLE);

		xSemaphoreGive(xSemaphore);
	}
}

/**
 * @brief	Transmits data as a master to a slave
 * @param	DevAddress: Address for the slave device
 * @param	Data: Pointer to the buffer of data to send
 * @param	Size: Size of the buffer
 * @retval	None
 */
void I2C2_Transmit(uint8_t DevAddress, uint8_t *Data, uint16_t Size)
{
	/* Try to take the semaphore in case some other process is using the device */
	if (xSemaphoreTake(xSemaphore, 100) == pdTRUE)
	{
		HAL_I2C_Master_Transmit(&I2C_Handle, (uint16_t)(DevAddress << 1), Data, Size, 500); /* TODO: Check timeout value */
		xSemaphoreGive(xSemaphore);
	}
}

/**
 * @brief	Transmits data as a master to a slave
 * @param	DevAddress: Address for the slave device
 * @param	Data: Pointer to a buffer where data will be stored
 * @param	Size: Size of the amount of data to receive
 * @retval	None
 */
void I2C2_Receive(uint8_t DevAddress, uint8_t *Data, uint16_t Size)
{
	/* Try to take the semaphore in case some other process is using the device */
	if (xSemaphoreTake(xSemaphore, 100) == pdTRUE)
	{
		HAL_I2C_Master_Receive(&I2C_Handle, (uint16_t)(DevAddress << 1), Data, Size, 500); /* TODO: Check timeout value */
		xSemaphoreGive(xSemaphore);
	}
}

/* Interrupt Handlers --------------------------------------------------------*/
