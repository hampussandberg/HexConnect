 /**
 *******************************************************************************
 * @file    ft5206.c
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

/** Includes -----------------------------------------------------------------*/
#include "ft5206.h"

#include "i2c3.h"

/** Private defines ----------------------------------------------------------*/
#define FT5206_REGISTER_DEVICE_MODE   (0x00)
#define FT5206_REGISTER_GEST_ID       (0x01)
#define FT5206_REGISTER_TD_STATUS     (0x02)

#define FT5206_REGISTER_TOUCH1_XH     (0x03)
#define FT5206_REGISTER_TOUCH1_XL     (0x04)
#define FT5206_REGISTER_TOUCH1_YH     (0x05)
#define FT5206_REGISTER_TOUCH1_YL     (0x06)

#define FT5206_REGISTER_TOUCH2_XH     (0x09)
#define FT5206_REGISTER_TOUCH2_XL     (0x0A)
#define FT5206_REGISTER_TOUCH2_YH     (0x0B)
#define FT5206_REGISTER_TOUCH2_YL     (0x0C)

#define FT5206_REGISTER_TOUCH3_XH     (0x0F)
#define FT5206_REGISTER_TOUCH3_XL     (0x10)
#define FT5206_REGISTER_TOUCH3_YH     (0x11)
#define FT5206_REGISTER_TOUCH3_YL     (0x12)

#define FT5206_REGISTER_TOUCH4_XH     (0x15)
#define FT5206_REGISTER_TOUCH5_XH     (0x1B)

/* The period of active status, value 3 to 14 (default: 12) */
#define FT5206_REGISTER_ID_G_PERIODACTIVE (0x88)
/* Chip vendor ID */
#define FT5206_REGISTER_ID_G_CIPHER       (0xA3)
/* Interrupt mode */
#define FT5206_REGISTER_ID_G_MODE         (0xA4)
/* CTPM vendor ID */
#define FT5206_REGISTER_ID_G_FT5201ID     (0xA8)

#define FT5206_ADDRESS          (0x38)
#define FT5206_CHIP_VENDOR_ID   (0x55)
#define FT5206_CTPM_VENDOR_ID   (0x86)

/* Interrupt pin */
#define FT5206_INT_PORT           (GPIOA)
#define FT5206_INT_PIN            (GPIO_PIN_1)
#define FT5206_INT_CLK_ENABLE()   __GPIOA_CLK_ENABLE()
#define FT5206_INT_IRQ            EXTI1_IRQn

/* Reset pin */
#define FT5206_RESET_PORT         (GPIOA)
#define FT5206_RESET_PIN          (GPIO_PIN_2)
#define FT5206_RESET_CLK_ENABLE() __GPIOA_CLK_ENABLE()

/* Uncomment below if multiple touch points should be detected */
//#define MULTIPLE_TOUCH_POINTS

/** Private typedefs ---------------------------------------------------------*/
typedef enum
{
  FT5206DeviceMode_NormalOperatingMode = 0,
  FT5206DeviceMode_SystemInfoMode = 1,
  FT5206DeviceMode_TestMode = 4,
} FT5206DeviceMode;

/** Private variables --------------------------------------------------------*/
static const uint8_t prvBaseRegisterForPoint[5] = {
  FT5206_REGISTER_TOUCH1_XH,
  FT5206_REGISTER_TOUCH2_XH,
  FT5206_REGISTER_TOUCH3_XH,
  FT5206_REGISTER_TOUCH4_XH,
  FT5206_REGISTER_TOUCH5_XH
};

/** Private function prototypes ----------------------------------------------*/

/** Functions ----------------------------------------------------------------*/
/**
 * @brief  Initializes the FT5206 Capacitive touch controller
 * @param  None
 * @retval  None
 */
void FT5206_Init()
{
  /* I2C */
  I2C3_Init();

  /* Reset pin configuration */
  FT5206_RESET_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.Pin    = FT5206_RESET_PIN;
  GPIO_InitStructure.Mode   = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pull   = GPIO_NOPULL;
  GPIO_InitStructure.Speed  = GPIO_SPEED_LOW;
  HAL_GPIO_Init(FT5206_RESET_PORT, &GPIO_InitStructure);
  HAL_GPIO_WritePin(FT5206_RESET_PORT, FT5206_RESET_PIN, GPIO_PIN_SET);

  /* Interrupt pin configuration */
  FT5206_INT_CLK_ENABLE();
  GPIO_InitStructure.Pin      = FT5206_INT_PIN;
  GPIO_InitStructure.Mode     = GPIO_MODE_IT_FALLING;
  GPIO_InitStructure.Pull     = GPIO_NOPULL;
  GPIO_InitStructure.Speed    = GPIO_SPEED_LOW;
  HAL_GPIO_Init(FT5206_INT_PORT, &GPIO_InitStructure);
  /* Configure priority and enable interrupt */
  HAL_NVIC_SetPriority(FT5206_INT_IRQ, configLIBRARY_LOWEST_INTERRUPT_PRIORITY, 0);
  HAL_NVIC_EnableIRQ(FT5206_INT_IRQ);

  /* Try to get chip id */
  uint8_t data = FT5206_REGISTER_ID_G_CIPHER;
  I2C3_Transmit(FT5206_ADDRESS, &data, 1);
  uint8_t chipId = 0;
  I2C3_Receive(FT5206_ADDRESS, &chipId, 1);
  if (chipId != FT5206_CHIP_VENDOR_ID)
  {
    /* TODO: Do something */
  }

  /* Select normal operating mode */
  uint8_t mode[2] = {FT5206_REGISTER_DEVICE_MODE, 0x00};
  I2C3_Transmit(FT5206_ADDRESS, mode, 2);

  /* Set interrupt mode to triggering and update rate */
  uint8_t temp[2] = {FT5206_REGISTER_ID_G_MODE, FT5206InterruptMode_Trigger};
  I2C3_Transmit(FT5206_ADDRESS, temp, 2);
  uint8_t temp2[2] = {FT5206_REGISTER_ID_G_PERIODACTIVE, 8};
  I2C3_Transmit(FT5206_ADDRESS, temp2, 2);
}

/**
 * @brief
 * @param  None
 * @retval None
 */
uint32_t FT5206_GetNumOfTouchPoints()
{
  uint8_t data = FT5206_REGISTER_TD_STATUS;
  I2C3_Transmit(FT5206_ADDRESS, &data, 1);
  uint8_t storage = 0;
  I2C3_Receive(FT5206_ADDRESS, &storage, 1);

  return storage;
}

/**
 * @brief
 * @param  pEvent: Pointer to
 * @param  pCoordinate: Pointer to
 * @param  Point: The point to get data from, can be any value of FT5206Point
 * @retval None
 */
void FT5206_GetTouchDataForPoint(FT5206Event* pEvent, FT5206TouchCoordinate* pCoordinate, FT5206Point Point)
{
  uint8_t reg = prvBaseRegisterForPoint[Point-1];
  I2C3_Transmit(FT5206_ADDRESS, &reg, 1);
  uint8_t storage[4] = {0x00};
  I2C3_Receive(FT5206_ADDRESS, storage, 4);

  *pEvent = (storage[0] & 0xC0) >> 6;
  pCoordinate->x = ((storage[0] & 0x0F) << 8) | storage[1];
  pCoordinate->y = ((storage[2] & 0x0F) << 8) | storage[3];
}


void FT5206_TestMode()
{
  uint8_t reg = 0;
  uint8_t storage = 0;
  uint8_t storage2[2] = {0, 0};

  /* Read Op, 0x8A-0x9F */
  uint8_t data[256] = {0};
  reg = 0x00;
  I2C3_Transmit(FT5206_ADDRESS, &reg, 1);
  I2C3_Receive(FT5206_ADDRESS, data, 256);

  /* Read Firmware ID */
  reg = 0xA6;
  I2C3_Transmit(FT5206_ADDRESS, &reg, 1);
  storage = 0;
  I2C3_Receive(FT5206_ADDRESS, &storage, 1);

  /* Read chip vendor ID */
  reg = 0xA3;
  I2C3_Transmit(FT5206_ADDRESS, &reg, 1);
  storage = 0;
  I2C3_Receive(FT5206_ADDRESS, &storage, 1);

  /* Read chip CTPM vendor ID */
  reg = 0xA8;
  I2C3_Transmit(FT5206_ADDRESS, &reg, 1);
  storage = 0;
  I2C3_Receive(FT5206_ADDRESS, &storage, 1);

  /* Get firmare library version */
  reg = 0xA1;
  I2C3_Transmit(FT5206_ADDRESS, &reg, 1);
  storage2[0] = 0;
  storage2[1] = 0;
  I2C3_Receive(FT5206_ADDRESS, storage2, 2);
  volatile uint16_t firm_lib_version = (storage2[0] << 8) | storage2[1];

  /* Select test mode */
  uint8_t mode[2] = {FT5206_REGISTER_DEVICE_MODE, 0b01000000};
  I2C3_Transmit(FT5206_ADDRESS, mode, 2);

  /* Read the device mode */
  reg = FT5206_REGISTER_DEVICE_MODE;
  I2C3_Transmit(FT5206_ADDRESS, &reg, 1);
  storage = 0;
  I2C3_Receive(FT5206_ADDRESS, &storage, 1);

  /* Get panel row number */
  reg = 0x03;
  I2C3_Transmit(FT5206_ADDRESS, &reg, 1);
  storage = 0;
  I2C3_Receive(FT5206_ADDRESS, &storage, 1);

  /* Get panel column number */
  reg = 0x04;
  I2C3_Transmit(FT5206_ADDRESS, &reg, 1);
  storage = 0;
  I2C3_Receive(FT5206_ADDRESS, &storage, 1);

  /* Get driver voltage of chip */
  reg = 0x05;
  I2C3_Transmit(FT5206_ADDRESS, &reg, 1);
  storage = 0;
  I2C3_Receive(FT5206_ADDRESS, &storage, 1);

  /* Get gain */
  reg = 0x07;
  I2C3_Transmit(FT5206_ADDRESS, &reg, 1);
  storage = 0;
  I2C3_Receive(FT5206_ADDRESS, &storage, 1);

  /* Get origin X */
  reg = 0x08;
  I2C3_Transmit(FT5206_ADDRESS, &reg, 1);
  storage2[0] = 0;
  storage2[1] = 0;
  I2C3_Receive(FT5206_ADDRESS, storage2, 2);
  volatile uint16_t origin_x = (storage2[0] << 8) | storage2[1];

  /* Get origin Y */
  reg = 0x0A;
  I2C3_Transmit(FT5206_ADDRESS, &reg, 1);
  storage2[0] = 0;
  storage2[1] = 0;
  I2C3_Receive(FT5206_ADDRESS, storage2, 2);
  volatile uint16_t origin_y = (storage2[0] << 8) | storage2[1];

  /* Get resolution width */
  reg = 0x0C;
  I2C3_Transmit(FT5206_ADDRESS, &reg, 1);
  storage2[0] = 0;
  storage2[1] = 0;
  I2C3_Receive(FT5206_ADDRESS, storage2, 2);
  volatile uint16_t res_width = (storage2[0] << 8) | storage2[1];

  /* Get resolution height */
  reg = 0x0E;
  I2C3_Transmit(FT5206_ADDRESS, &reg, 1);
  storage2[0] = 0;
  storage2[1] = 0;
  I2C3_Receive(FT5206_ADDRESS, storage2, 2);
  volatile uint16_t res_height = (storage2[0] << 8) | storage2[1];

  /* Get touch point number support */
  reg = 0x4C;
  I2C3_Transmit(FT5206_ADDRESS, &reg, 1);
  storage = 0;
  I2C3_Receive(FT5206_ADDRESS, &storage, 1);
}

/** Interrupt Handlers -------------------------------------------------------*/
void CTP_INT_Callback()
{
#if defined(MULTIPLE_TOUCH_POINTS)
  uint8_t numOfPoints = 0;
  uint8_t data = FT5206_REGISTER_TD_STATUS;
  I2C3_TransmitFromISR(FT5206_ADDRESS, &data, 1);
  I2C3_ReceiveFromISR(FT5206_ADDRESS, &numOfPoints, 1);

  uint32_t i;
  for (i = 0; i < numOfPoints; i++)
  {
    uint8_t reg = prvBaseRegisterForPoint[i];
    I2C3_TransmitFromISR(FT5206_ADDRESS, &reg, 1);
    uint8_t storage[4] = {0x00};
    I2C3_ReceiveFromISR(FT5206_ADDRESS, storage, 4);

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
  I2C3_TransmitFromISR(FT5206_ADDRESS, &reg, 1);
  uint8_t storage[4] = {0x00};
  I2C3_ReceiveFromISR(FT5206_ADDRESS, storage, 4);

  LCDEventMessage message;
  message.event = LCDEvent_TouchEvent;
  message.data[0] = ((storage[0] & 0x0F) << 8) | storage[1];
  message.data[1] = ((storage[2] & 0x0F) << 8) | storage[3];
  message.data[2] = (storage[0] & 0xC0) >> 6;
  message.data[3] = FT5206Point_1;
  xQueueSendToBackFromISR(xLCDEventQueue, &message, NULL);
#endif
}
