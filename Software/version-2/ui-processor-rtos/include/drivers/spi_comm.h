/**
 *******************************************************************************
 * @file    spi_comm.h
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-08-16
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

/** Define to prevent recursive inclusion ------------------------------------*/
#ifndef SPI_COMM_H_
#define SPI_COMM_H_

/** Includes -----------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include <stdbool.h>

/** Defines ------------------------------------------------------------------*/
#define SPI_COMM_COMMAND_CHANNEL_POWER            (0x10)
#define SPI_COMM_COMMAND_CHANNEL_OUTPUT           (0x11)
#define SPI_COMM_COMMAND_CHANNEL_ID               (0x12)
#define SPI_COMM_COMMAND_CHANNEL_DIRECTION        (0x13)
#define SPI_COMM_COMMAND_CAN_CHANNEL_TERMINATION  (0x30)

/** Typedefs -----------------------------------------------------------------*/
typedef enum
{
  SPI_COMM_Channel_1 = 1,
  SPI_COMM_Channel_2 = 2,
  SPI_COMM_Channel_3 = 3,
  SPI_COMM_Channel_4 = 4,
  SPI_COMM_Channel_5 = 5,
  SPI_COMM_Channel_6 = 6,
  SPI_COMM_Channel_All = 7,
} SPI_COMM_Channel;

/** Function prototypes ------------------------------------------------------*/
ErrorStatus SPI_COMM_Init();
bool SPI_COMM_Initialized();
uint32_t SPI_COMM_ReadID();
void SPI_COMM_SendCommand(uint8_t Command, uint8_t* pData, uint32_t DataCount);
void SPI_COMM_SendGetCommand(uint8_t Command, uint8_t* pTxData, uint8_t* pRxData, uint32_t DataCount);
void SPI_COMM_GetData(uint8_t* pDataBuffer, uint32_t DataCount);

ErrorStatus SPI_COMM_GetPowerForAllChannels(uint8_t* pCurrentPower);
void SPI_COMM_EnablePowerForChannel(SPI_COMM_Channel Channel);
void SPI_COMM_DisablePowerForChannel(SPI_COMM_Channel Channel);

ErrorStatus SPI_COMM_GetOutputForAllChannels(uint8_t* pCurrentOutput);
void SPI_COMM_EnableOutputForChannel(SPI_COMM_Channel Channel);
void SPI_COMM_DisableOutputForChannel(SPI_COMM_Channel Channel);

ErrorStatus SPI_COMM_GetIdForChannel(uint8_t Channel, uint8_t* pCurrentId);
void SPI_COMM_EnableIdUpdateForChannel(SPI_COMM_Channel Channel);
void SPI_COMM_DisableIdUpdateForChannel(SPI_COMM_Channel Channel);

ErrorStatus SPI_COMM_GetTerminationForAllChannels(uint8_t* pCurrentTermination);
void SPI_COMM_EnableTerminationForChannel(SPI_COMM_Channel Channel);
void SPI_COMM_DisableTerminationForChannel(SPI_COMM_Channel Channel);

#endif /* SPI_COMM_H_ */
