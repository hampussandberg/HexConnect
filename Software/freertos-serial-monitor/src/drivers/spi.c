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
//#define SCK_Pin_1	(GPIO_Pin_5)
//#define MISO_Pin_1	(GPIO_Pin_6)
//#define MOSI_Pin_1	(GPIO_Pin_7)
//
//#define SCK_Pin_2	(GPIO_Pin_13)
//#define MISO_Pin_2	(GPIO_Pin_14)
//#define MOSI_Pin_2	(GPIO_Pin_15)

/* Private variables ---------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Functions -----------------------------------------------------------------*/
/**
 * @brief	Initializes the SPI
 * @param	None
 * @retval	None
 */
void SPI_Device_InitDefault(SPI_Device* SPIDevice)
{
	/* Initialize SPIx */
	SPI_InitTypeDef SPI_InitStructure;
	SPI_InitStructure.Mode 				= SPI_MODE_MASTER;
	SPI_InitStructure.Direction 		= SPI_DIRECTION_2LINES;
	SPI_InitStructure.DataSize 			= SPI_DATASIZE_8BIT;
	SPI_InitStructure.CLKPolarity 		= SPI_POLARITY_LOW;
	SPI_InitStructure.CLKPhase 			= SPI_PHASE_1EDGE;
	SPI_InitStructure.NSS 				= SPI_NSS_SOFT;
	SPI_InitStructure.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
	SPI_InitStructure.FirstBit 			= SPI_FIRSTBIT_MSB;
	SPI_InitStructure.TIMode			= SPI_TIMODE_DISABLED;
	SPI_InitStructure.CRCCalculation	= SPI_CRCCALCULATION_DISABLED;
	SPI_InitStructure.CRCPolynomial 	= 0; /* CRC is disabled, so don't care */

	SPI_InitWithStructure(SPIDevice, &SPI_InitStructure);
}

/**
 * @brief	Initializes the SPI
 * @param	SPI_InitStructure: Struct with the parameters for the SPI peripheral
 * @retval	None
 */
void SPI_InitWithStructure(SPI_Device* SPIDevice, SPI_InitTypeDef* SPI_InitStructure)
{
	/* Make sure we haven't initialized this before */
	if (!SPIDevice->initialized)
	{
		SPIDevice->receivedByte = 0;

		/*
		 * Create the binary semaphores:
		 * The semaphore is created in the 'empty' state, meaning
		 * the semaphore must first be given before it can be taken (obtained)
		 * using the xSemaphoreTake() function.
		 */
		SPIDevice->xTxSemaphore = xSemaphoreCreateBinary();
		SPIDevice->xRxSemaphore = xSemaphoreCreateBinary();
		/* We can start sending immediately so give the TX semaphore */
		xSemaphoreGive(SPIDevice->xTxSemaphore);
		/* For RX we have to wait, so give the semaphore and then take it back */
		xSemaphoreGive(SPIDevice->xRxSemaphore);
		xSemaphoreTake(SPIDevice->xRxSemaphore, portMAX_DELAY);

//		if (SPIDevice->SPI_Channel == 1)
//		{
//			/* Enable GPIOx clock */
//			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
//			RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
//
//			/* NVIC Configuration */
//			NVIC_InitTypeDef NVIC_InitStructure;
//			NVIC_InitStructure.NVIC_IRQChannel 						= SPI1_IRQn;
//			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority 	= configLIBRARY_LOWEST_INTERRUPT_PRIORITY - 1;
//			NVIC_InitStructure.NVIC_IRQChannelSubPriority 			= 0;
//			NVIC_InitStructure.NVIC_IRQChannelCmd 					= ENABLE;
//			NVIC_Init(&NVIC_InitStructure);
//
//			/* Configure SPIx-SCK, SPIx-MOSI alternate function push-pull */
//			GPIO_InitTypeDef GPIO_InitStructure;
//			GPIO_InitStructure.GPIO_Pin   			= SCK_Pin_1 | MOSI_Pin_1;
//			GPIO_InitStructure.GPIO_Speed 			= GPIO_Speed_50MHz;
//			GPIO_InitStructure.GPIO_Mode  			= GPIO_Mode_AF_PP;
//			GPIO_Init(GPIOA, &GPIO_InitStructure);
//
//			/* Configure SPIx-MISO input floating */
//			GPIO_InitStructure.GPIO_Pin   			= MISO_Pin_1;
//			GPIO_InitStructure.GPIO_Speed 			= GPIO_Speed_50MHz;
//			GPIO_InitStructure.GPIO_Mode  			= GPIO_Mode_IN_FLOATING;
//			GPIO_Init(GPIOA, &GPIO_InitStructure);
//
//
//			/* Enable SPIx Peripheral clock */
//			RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
//		}
//		else if (SPIDevice->SPI_Channel == 2)
//		{
//			/* Enable GPIOx clock */
//			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
//			RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
//
//			/* NVIC Configuration */
//			NVIC_InitTypeDef NVIC_InitStructure;
//			NVIC_InitStructure.NVIC_IRQChannel 						= SPI2_IRQn;
//			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority 	= configLIBRARY_LOWEST_INTERRUPT_PRIORITY - 1;
//			NVIC_InitStructure.NVIC_IRQChannelSubPriority 			= 0;
//			NVIC_InitStructure.NVIC_IRQChannelCmd 					= ENABLE;
//			NVIC_Init(&NVIC_InitStructure);
//
//			/* Configure SPIx-SCK, SPIx-MOSI alternate function push-pull */
//			GPIO_InitTypeDef GPIO_InitStructure;
//			GPIO_InitStructure.GPIO_Pin   			= SCK_Pin_2 | MOSI_Pin_2;
//			GPIO_InitStructure.GPIO_Speed 			= GPIO_Speed_50MHz;
//			GPIO_InitStructure.GPIO_Mode  			= GPIO_Mode_AF_PP;
//			GPIO_Init(GPIOB, &GPIO_InitStructure);
//
//			/* Configure SPIx-MISO input floating */
//			GPIO_InitStructure.GPIO_Pin   			= MISO_Pin_2;
//			GPIO_InitStructure.GPIO_Speed 			= GPIO_Speed_50MHz;
//			GPIO_InitStructure.GPIO_Mode  			= GPIO_Mode_IN_FLOATING;
//			GPIO_Init(GPIOB, &GPIO_InitStructure);
//
//			/* Enable SPIx Peripheral clock */
//			RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
//		}
//
//		/* Initialize SPIx */
//		SPI_Init(SPIDevice->SPIx, SPI_InitStructure);
//
//		/*
//		 * Enable SPI_I2S_IT_RXNE interrupt
//		 * SPI_I2S_IT_TXE interrupt will only be enabled when a write should happen
//		 * */
//		SPI_I2S_ITConfig(SPIDevice->SPIx, SPI_I2S_IT_RXNE, ENABLE);
//
//		/* Enable SPIx */
//		SPI_Cmd(SPIDevice->SPIx, ENABLE);
		SPIDevice->initialized = true;
	}
	else
	{
		/* TODO: What to do? */
	}
}

/**
 * @brief	Writes and receives data from the SPI
 * @param	Data: data to be written to the SPI
 * @retval	The received data
 */
uint8_t SPI_WriteRead(SPI_Device* SPIDevice, uint8_t Data)
{
//	/* Enable SPI_MASTER TXE interrupt */
//	SPI_I2S_ITConfig(SPIDevice->SPIx, SPI_I2S_IT_TXE, ENABLE);
//	/* Try to take the TX Semaphore */
//	xSemaphoreTake(SPIDevice->xTxSemaphore, portMAX_DELAY);
//
//	/* Send byte through the SPIx peripheral */
//	SPIDevice->SPIx->DR = Data;
//
//	/* Try to take the RX Semaphore */
//	xSemaphoreTake(SPIDevice->xRxSemaphore, portMAX_DELAY);
//
//	/* Return the byte read from the SPI bus */
//	return SPIDevice->receivedByte;
}

/* Interrupt Handlers --------------------------------------------------------*/
void SPI_Interrupt(SPI_Device* SPIDevice)
{
//	/* Transmit buffer empty interrupt */
//	if (SPI_I2S_GetITStatus(SPIDevice->SPIx, SPI_I2S_IT_TXE) != RESET)
//	{
//		/* Release the semaphore */
//		xSemaphoreGiveFromISR(SPIDevice->xTxSemaphore, NULL);
//
//		/* Disable SPI_MASTER TXE interrupt */
//		SPI_I2S_ITConfig(SPIDevice->SPIx, SPI_I2S_IT_TXE, DISABLE);
//	}
//	/* Receive buffer not empty interrupt */
//	else if (SPI_I2S_GetITStatus(SPIDevice->SPIx, SPI_I2S_IT_RXNE) != RESET)
//	{
//		/* Release the semaphore */
//		xSemaphoreGiveFromISR(SPIDevice->xRxSemaphore, NULL);
//
//		/* Read the byte received in order to clear the interrupt flag */
//		SPIDevice->receivedByte = SPI_I2S_ReceiveData(SPIDevice->SPIx);
//	}
}
