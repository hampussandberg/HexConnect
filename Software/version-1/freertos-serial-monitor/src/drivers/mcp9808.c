/**
 *******************************************************************************
 * @file  mcp9808.c
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

/* Includes ------------------------------------------------------------------*/
#include "mcp9808.h"

/* Private defines -----------------------------------------------------------*/
#define MCP9808_TEMP_SENSOR_ADDRESS		(0x1F)
#define MCP9808_MANUFACTURER_ID			(0x0054)

#define MCP9808_REGISTER_CONFIG			(0x01)
#define MCP9808_REGISTER_T_UPPER		(0x02)
#define MCP9808_REGISTER_T_LOWER		(0x03)
#define MCP9808_REGISTER_T_CRIT			(0x04)
#define MCP9808_REGISTER_T_A			(0x05)
#define MCP9808_REGISTER_MANUFAC_ID		(0x06)
#define MCP9808_REGISTER_DEVICE_ID		(0x07)
#define MCP9808_REGISTER_RESOLUTION		(0x08)

/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
float prvLastReadTemperature = 0.0;

/* Private function prototypes -----------------------------------------------*/

/* Functions -----------------------------------------------------------------*/
/**
 * @brief	Initializes the MCP9808 temperature sensor
 * @param	None
 * @retval	None
 */
void MCP9808_Init()
{
	/* I2C Init */
	I2C2_Init();

	/* Get the Manufacturer ID from the MCP9808 */
	uint8_t reg = MCP9808_REGISTER_MANUFAC_ID;
	I2C2_Transmit(MCP9808_TEMP_SENSOR_ADDRESS, &reg, 1);
	uint8_t storage[2] = {0x00, 0x00};
	I2C2_Receive(MCP9808_TEMP_SENSOR_ADDRESS, storage, 2);
	uint16_t manufacturerId = (storage[0] << 8) | storage[1];
	if (manufacturerId != MCP9808_MANUFACTURER_ID)
	{
		/* Something is wrong */
	}
}

/**
 * @brief	Get the current temperature from the sensor
 * @param	None
 * @retval	The temperature as a float
 */
float MCP9808_GetTemperature()
{
	/* Get the temperature from the MCP9808 */
	uint8_t reg = MCP9808_REGISTER_T_A;
	I2C2_Transmit(MCP9808_TEMP_SENSOR_ADDRESS, &reg, 1);
	uint8_t storage[2] = {0x00};
	I2C2_Receive(MCP9808_TEMP_SENSOR_ADDRESS, storage, 2);

	/* Check flags */
	if ((storage[0] & 0x80) == 0x80)	/* T_critical */
	{

	}
	if ((storage[0] & 0x40) == 0x40)	/* T_upper */
	{

	}
	if ((storage[0] & 0x20) == 0x20)	/* T_lower */
	{

	}

	/* Clear flag bits */
	storage[0] = storage[0] & 0x1F;

	/* Convert the result */
	uint8_t integerPart = (storage[0] << 4) | (storage[1] >> 4);
	uint8_t fractionPart = storage[1] & 0xF;
	uint16_t temperature = (integerPart << 4) | fractionPart;
	prvLastReadTemperature = (float)temperature * 0.0625;

	if ((storage[0] & 0x10) == 0x10)
		prvLastReadTemperature -= 1.0;

	/* TODO: Calibrate */
	return prvLastReadTemperature;
}

/**
 * @brief	Set the resolution of the sensor
 * @param	Resolution: The resolution to set, can be any value of MCP9808Resolution
 * @retval	None
 */
void MCP9808_SetResolution(MCP9808Resolution Resolution)
{
	/* Set the resolution register */
	uint8_t data[2] = {MCP9808_REGISTER_RESOLUTION, Resolution};
	I2C2_Transmit(MCP9808_TEMP_SENSOR_ADDRESS, data, 2);
}

/* Interrupt Handlers --------------------------------------------------------*/
