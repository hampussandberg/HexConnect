/**
 ******************************************************************************
 * @file	max1301.h
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MAX1301_H_
#define MAX1301_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
typedef enum
{
	MAX1301Mode_SingleEnded = 0x00,
	MAX1301Mode_Differential = 0x08,
} MAX1301Mode;

typedef enum
{
	MAX1301DiffChannel_0 = 0x00,
	MAX1301DiffChannel_1 = 0x02,
} MAX1301DiffChannel;

typedef enum
{
	MAX1301SingleEndedChannel_0 = 0x00,
	MAX1301SingleEndedChannel_1,
	MAX1301SingleEndedChannel_2,
	MAX1301SingleEndedChannel_3,
} MAX1301SingleEndedChannel;

typedef enum
{
	MAX1301Range_3x = 0x01,		/* Full scale range (FSR): 3 x VREF */
	MAX1301Range_6x = 0x03,		/* Full scale range (FSR): 6 x VREF */
	MAX1301Range_12x = 0x07,	/* Full scale range (FSR): 12 x VREF */
} MAX1301Range;

typedef struct
{
	MAX1301Mode mode;
	MAX1301DiffChannel channel;
	MAX1301Range range;
} MAX1301Configuration;

/* Function prototypes -------------------------------------------------------*/


#endif /* MAX1301_H_ */
