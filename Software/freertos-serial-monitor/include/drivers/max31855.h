/**
 ******************************************************************************
 * @file	max31855.h
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MAX31855_H_
#define MAX31855_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "gpio.h"
#include "spi.h"

/* Defines -------------------------------------------------------------------*/
typedef enum
{
	ThermocoupleType_K,
	ThermocoupleType_J,
	ThermocoupleType_N,
	ThermocoupleType_S,
	ThermocoupleType_T,
	ThermocoupleType_E,
	ThermocoupleType_R,
} MAX31855_ThermocoupleType;

typedef struct
{
	MAX31855_ThermocoupleType thermocoupleType;	/* The thermocouple type */
	SPI_Device* spiDevice;						/* SPI device used for communication */
	GPIO_Device* chipSelectGpioDevice; 			/* Gpio for the chip select signal */
	bool initialized;							/* Set to true if the initialization has been done successfully */
} MAX31855_Device;

/* Typedefs ------------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/
void MAX31855_Init(MAX31855_Device* Device);


#endif /* MAX31855_H_ */
