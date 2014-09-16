/**
 ******************************************************************************
 * @file	can_common.h
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-09-16
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
#ifndef CAN_COMMON_H_
#define CAN_COMMON_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
typedef enum
{
	CANConnection_Disconnected,
	CANConnection_Connected,
} CANConnection;

typedef enum
{
	CANPrescaler_10k 	= 300,
	CANPrescaler_20k 	= 150,
	CANPrescaler_50k 	= 60,
	CANPrescaler_100k 	= 30,
	CANPrescaler_125k 	= 24,
	CANPrescaler_250k 	= 12,
	CANPrescaler_500k 	= 6,
	CANPrescaler_1M 	= 3,
} CANPrescaler;

typedef enum
{
	CANSJW_10k	= CAN_SJW_1TQ,
	CANSJW_20k	= CAN_SJW_1TQ,
	CANSJW_50k	= CAN_SJW_1TQ,
	CANSJW_100k	= CAN_SJW_1TQ,
	CANSJW_125k	= CAN_SJW_1TQ,
	CANSJW_250k	= CAN_SJW_1TQ,
	CANSJW_500k	= CAN_SJW_1TQ,
	CANSJW_1M 	= CAN_SJW_1TQ,
} CANSJW;

typedef enum
{
	CANBS1_10k 	= CAN_BS1_11TQ,
	CANBS1_20k 	= CAN_BS1_11TQ,
	CANBS1_50k 	= CAN_BS1_11TQ,
	CANBS1_100k	= CAN_BS1_11TQ,
	CANBS1_125k	= CAN_BS1_11TQ,
	CANBS1_250k	= CAN_BS1_11TQ,
	CANBS1_500k	= CAN_BS1_11TQ,
	CANBS1_1M 	= CAN_BS1_11TQ,
} CANBS1;

typedef enum
{
	CANBS2_10k 	= CAN_BS2_2TQ,
	CANBS2_20k 	= CAN_BS2_2TQ,
	CANBS2_50k 	= CAN_BS2_2TQ,
	CANBS2_100k	= CAN_BS2_2TQ,
	CANBS2_125k	= CAN_BS2_2TQ,
	CANBS2_250k	= CAN_BS2_2TQ,
	CANBS2_500k	= CAN_BS2_2TQ,
	CANBS2_1M 	= CAN_BS2_2TQ,
} CANBS2;

typedef enum
{
	CANTermination_Disconnected,
	CANTermination_Connected,
} CANTermination;

typedef enum
{
	CANIdentifier_Standard = CAN_ID_STD,
	CANIdentifier_Extended = CAN_ID_EXT,
} CANIdentifier;

typedef enum
{
	CANDataLength_0 = 0,
	CANDataLength_1,
	CANDataLength_2,
	CANDataLength_3,
	CANDataLength_4,
	CANDataLength_5,
	CANDataLength_6,
	CANDataLength_7,
	CANDataLength_8,
} CANDataLength;

typedef struct
{
	CANConnection connection;
	CANTermination termination;
	CANIdentifier identifier;
} CANSettings;

/* Function prototypes -------------------------------------------------------*/


#endif /* CAN_COMMON_H_ */
