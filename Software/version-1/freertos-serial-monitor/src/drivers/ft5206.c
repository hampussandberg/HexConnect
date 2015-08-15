 /**
 *******************************************************************************
 * @file  ft5206.c
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
#include "ft5206.h"

#include "i2c2.h"

/* Private defines -----------------------------------------------------------*/
#define FT5206_REGISTER_DEVICE_MODE 	(0x00)
#define FT5206_REGISTER_GEST_ID 		(0x01)
#define FT5206_REGISTER_TD_STATUS 		(0x02)

#define FT5206_REGISTER_TOUCH1_XH 		(0x03)
#define FT5206_REGISTER_TOUCH1_XL 		(0x04)
#define FT5206_REGISTER_TOUCH1_YH 		(0x05)
#define FT5206_REGISTER_TOUCH1_YL 		(0x06)

#define FT5206_REGISTER_TOUCH2_XH 		(0x09)
#define FT5206_REGISTER_TOUCH2_XL 		(0x0A)
#define FT5206_REGISTER_TOUCH2_YH 		(0x0B)
#define FT5206_REGISTER_TOUCH2_YL 		(0x0C)

#define FT5206_REGISTER_TOUCH3_XH 		(0x0F)
#define FT5206_REGISTER_TOUCH3_XL 		(0x10)
#define FT5206_REGISTER_TOUCH3_YH 		(0x11)
#define FT5206_REGISTER_TOUCH3_YL 		(0x12)

#define FT5206_REGISTER_TOUCH4_XH 		(0x15)
#define FT5206_REGISTER_TOUCH5_XH 		(0x1B)

#define FT5206_REGISTER_ID_G_PERIODACTIVE (0x88)	/* The period of active status, value 3 to 14 (defualt 12) */
#define FT5206_REGISTER_ID_G_CIPHER		(0xA3)	/* Chip vendor ID */
#define FT5206_REGISTER_ID_G_MODE		(0xA4)	/* Interrupt mode */
#define FT5206_REGISTER_ID_G_FT5201ID	(0xA8)	/* CTPM vendor ID */

#define FT5206_ADDRESS			(0x38)
#define FT5206_CHIP_VENDOR_ID	(0x55)
#define FT5206_CTPM_VENDOR_ID	(0x86)

/* Pins */
#define FT5206_WAKE_PORT		(GPIOD)
#define FT5206_WAKE_PIN			(GPIO_PIN_3)
#define FT5206_WAKE_CLK_ENABLE	(__GPIOD_CLK_ENABLE())

#define FT5206_INT_PORT			(GPIOD)
#define FT5206_INT_PIN			(GPIO_PIN_6)
#define FT5206_INT_CLK_ENABLE	(__GPIOD_CLK_ENABLE())

#define FT5206_RESET_PORT		(GPIOB)
#define FT5206_RESET_PIN		(GPIO_PIN_7)
#define FT5206_RESET_CLK_ENABLE	(__GPIOB_CLK_ENABLE())

/* Uncomment below if multiple touch points should be detected */
//#define MULTIPLE_TOUCH_POINTS

/* Private typedefs ----------------------------------------------------------*/
typedef enum
{
	FT5206DeviceMode_NormalOperatingMode = 0,
	FT5206DeviceMode_SystemInfoMode = 1,
	FT5206DeviceMode_TestMode = 4,
} FT5206DeviceMode;

/* Private variables ---------------------------------------------------------*/
static const uint8_t prvBaseRegisterForPoint[5] = {
		FT5206_REGISTER_TOUCH1_XH,
		FT5206_REGISTER_TOUCH2_XH,
		FT5206_REGISTER_TOUCH3_XH,
		FT5206_REGISTER_TOUCH4_XH,
		FT5206_REGISTER_TOUCH5_XH
};

/* Private function prototypes -----------------------------------------------*/

/* Functions -----------------------------------------------------------------*/
/**
 * @brief	Initializes the FT5206 Capacitive touch controller
 * @param	None
 * @retval	None
 */
void FT5206_Init()
{
	/* I2C */
	I2C2_Init();

	/* GPIO */
	FT5206_WAKE_CLK_ENABLE;
	FT5206_INT_CLK_ENABLE;
	FT5206_RESET_CLK_ENABLE;

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin  		= FT5206_WAKE_PIN;
	GPIO_InitStructure.Mode  		= GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull			= GPIO_NOPULL;
	GPIO_InitStructure.Speed 		= GPIO_SPEED_HIGH;
	HAL_GPIO_Init(FT5206_WAKE_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.Pin  		= FT5206_RESET_PIN;
	HAL_GPIO_Init(FT5206_RESET_PORT, &GPIO_InitStructure);

	HAL_GPIO_WritePin(FT5206_WAKE_PORT, FT5206_WAKE_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(FT5206_RESET_PORT, FT5206_RESET_PIN, GPIO_PIN_SET);

	/* Interrupt */
	GPIO_InitStructure.Mode  		= GPIO_MODE_IT_FALLING;
	GPIO_InitStructure.Pin  		= FT5206_INT_PIN;
	HAL_GPIO_Init(FT5206_INT_PORT, &GPIO_InitStructure);
	/* Configure priority and enable interrupt */
	HAL_NVIC_SetPriority(EXTI9_5_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY, 0);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

	/* Try to get chip id */
	uint8_t data = FT5206_REGISTER_ID_G_CIPHER;
	I2C2_Transmit(FT5206_ADDRESS, &data, 1);
	uint8_t chipId = 0;
	I2C2_Receive(FT5206_ADDRESS, &chipId, 1);
	if (chipId != FT5206_CHIP_VENDOR_ID)
	{
		/* TODO: Do something */
	}

	/* Set interrupt mode to triggering and update rate */
	uint8_t temp[2] = {FT5206_REGISTER_ID_G_MODE, FT5206InterruptMode_Trigger};
	I2C2_Transmit(FT5206_ADDRESS, temp, 2);
	uint8_t temp2[2] = {FT5206_REGISTER_ID_G_PERIODACTIVE, 12};	/* 3 Å 33.56 Hz */
	I2C2_Transmit(FT5206_ADDRESS, temp2, 2);
}

/**
 * @brief
 * @param	None
 * @retval	None
 */
uint32_t FT5206_GetNumOfTouchPoints()
{
	uint8_t data = FT5206_REGISTER_TD_STATUS;
	I2C2_Transmit(FT5206_ADDRESS, &data, 1);
	uint8_t storage = 0;
	I2C2_Receive(FT5206_ADDRESS, &storage, 1);

	return storage;
}

/**
 * @brief
 * @param	pEvent: Pointer to
 * @param	pCoordinate: Pointer to
 * @param	Point: The point to get data from, can be any value of FT5206Point
 * @retval	None
 */
void FT5206_GetTouchDataForPoint(FT5206Event* pEvent, FT5206TouchCoordinate* pCoordinate, FT5206Point Point)
{
	uint8_t reg = prvBaseRegisterForPoint[Point-1];
	I2C2_Transmit(FT5206_ADDRESS, &reg, 1);
	uint8_t storage[4] = {0x00};
	I2C2_Receive(FT5206_ADDRESS, storage, 4);

	*pEvent = (storage[0] & 0xC0) >> 6;
	pCoordinate->x = ((storage[0] & 0x0F) << 8) | storage[1];
	pCoordinate->y = ((storage[2] & 0x0F) << 8) | storage[3];
}

/* Interrupt Handlers --------------------------------------------------------*/
void CTP_INT_Callback()
{
#if defined(MULTIPLE_TOUCH_POINTS)
	uint8_t numOfPoints = 0;
	uint8_t data = FT5206_REGISTER_TD_STATUS;
	I2C2_TransmitFromISR(FT5206_ADDRESS, &data, 1);
	I2C2_ReceiveFromISR(FT5206_ADDRESS, &numOfPoints, 1);

	uint32_t i;
	for (i = 0; i < numOfPoints; i++)
	{
		uint8_t reg = prvBaseRegisterForPoint[i];
		I2C2_TransmitFromISR(FT5206_ADDRESS, &reg, 1);
		uint8_t storage[4] = {0x00};
		I2C2_ReceiveFromISR(FT5206_ADDRESS, storage, 4);

		LCDEventMessage message;
		message.event = LCDEvent_TouchEvent;
		message.data[0] = ((storage[0] & 0x0F) << 8) | storage[1];
		message.data[1] = ((storage[2] & 0x0F) << 8) | storage[3];
		message.data[2] = (storage[0] & 0xC0) >> 6;
		message.data[3] = i+1;
		xQueueSendToBackFromISR(xLCDEventQueue, &message, NULL);
	}
#else
	uint8_t reg = prvBaseRegisterForPoint[FT5206Point_1 - 1];
	I2C2_TransmitFromISR(FT5206_ADDRESS, &reg, 1);
	uint8_t storage[4] = {0x00};
	I2C2_ReceiveFromISR(FT5206_ADDRESS, storage, 4);

	LCDEventMessage message;
	message.event = LCDEvent_TouchEvent;
	message.data[0] = ((storage[0] & 0x0F) << 8) | storage[1];
	message.data[1] = ((storage[2] & 0x0F) << 8) | storage[3];
	message.data[2] = (storage[0] & 0xC0) >> 6;
	message.data[3] = FT5206Point_1;
	xQueueSendToBackFromISR(xLCDEventQueue, &message, NULL);
#endif
}
