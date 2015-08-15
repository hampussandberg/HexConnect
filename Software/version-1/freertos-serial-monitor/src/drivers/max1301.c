/**
 ******************************************************************************
 * @file	max1301.c
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
#include "max1301.h"

/* Private defines -----------------------------------------------------------*/
#define ADC_SPI					(SPI1)
#define ADC_SPI_CLK_ENABLE()	(__SPI1_CLK_ENABLE())
#define ADC_PORT				(GPIOA)
#define ADC_GPIO_CLK_ENABLE()	(__GPIOA_CLK_ENABLE())
#define ADC_CS_PIN				(GPIO_PIN_4)
#define ADC_SCK_PIN				(GPIO_PIN_5)
#define ADC_MISO_PIN			(GPIO_PIN_6)
#define ADC_MOSI_PIN			(GPIO_PIN_7)

#define START_BIT	(0x80)


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
		.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64,
		.Init.FirstBit 			= SPI_FIRSTBIT_MSB,
		.Init.TIMode			= SPI_TIMODE_DISABLED,
		.Init.CRCCalculation	= SPI_CRCCALCULATION_DISABLED,
		.Init.CRCPolynomial 	= 1,
};

static MAX1301Configuration prvCurrentConfig[2] = {
		{
				.mode = MAX1301Mode_SingleEnded,
				.channel = MAX1301SingleEndedChannel_0,
				.range = MAX1301Range_3x,
		},
		{
				.mode = MAX1301Mode_SingleEnded,
				.channel = MAX1301SingleEndedChannel_1,
				.range = MAX1301Range_3x,
		},
};

/* Private function prototypes -----------------------------------------------*/
static inline void prvADC_CS_LOW();
static inline void prvADC_CS_HIGH();

static uint8_t prvADC_SendReceiveByte(uint8_t Byte);

/* Functions -----------------------------------------------------------------*/
/**
 * @brief	Text
 * @param	None
 * @retval	None
 */
void MAX1301_Init()
{
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

	/* Configure the ADC channels */

	/* Channel 0 */
	prvADC_CS_LOW();
	/* Config byte */
	prvADC_SendReceiveByte( START_BIT |
							prvCurrentConfig[0].channel |
							prvCurrentConfig[0].mode |
							prvCurrentConfig[0].range);
	/* Mode control byte - External Clock */
	prvADC_SendReceiveByte(0b10001000);
	prvADC_CS_HIGH();

	/* Channel 1 */
	prvADC_CS_LOW();
	/* Config byte */
	prvADC_SendReceiveByte( START_BIT |
							prvCurrentConfig[1].channel |
							prvCurrentConfig[1].mode |
							prvCurrentConfig[1].range);
	/* Mode control byte - External Clock */
	prvADC_SendReceiveByte(0b10001000);
	prvADC_CS_HIGH();
}

/**
 * @brief	Sample the selected channel
 * @param	Channel: The channel to sample, can be any value of MAX1301DiffChannel
 * @retval	The sampled value
 */
int16_t MAX1301_GetDataFromDiffChannel(MAX1301DiffChannel Channel)
{
	uint8_t byte0, byte1;
	prvADC_CS_LOW();
	prvADC_SendReceiveByte(START_BIT | Channel);	/* Conversion start byte */
	prvADC_SendReceiveByte(0x00);					/* Dummy data */
	byte0 = prvADC_SendReceiveByte(0x00);			/* Dummy data */
	byte1 = prvADC_SendReceiveByte(0x00);			/* Dummy data */
	prvADC_CS_HIGH();

	int16_t result = (byte0 << 8) | byte1;
	return result;
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
