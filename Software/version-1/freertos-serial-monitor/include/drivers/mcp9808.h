/**
 *******************************************************************************
 * @file  mcp9808.h
 * @author  Hampus Sandberg
 * @version 0.1
 * @date  2015-08-15
 * @brief
 *******************************************************************************
  Copyright (c) 2015 Hampus Sandberg.

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MCP9808_H_
#define MCP9808_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"

#include "i2c2.h"

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
typedef enum
{
	MCP9808Resolution_0_5 = 0,		/* Tconv = 30ms typical */
	MCP9808Resolution_0_25 = 1,		/* Tconv = 65ms typical */
	MCP9808Resolution_0_125 = 2,	/* Tconv = 130ms typical */
	MCP9808Resolution_0_0625 = 3,	/* Tconv = 250ms typical */
} MCP9808Resolution;

/* Function prototypes -------------------------------------------------------*/
void MCP9808_Init();
float MCP9808_GetTemperature();
void MCP9808_SetResolution(MCP9808Resolution Resolution);


#endif /* MCP9808_H_ */
