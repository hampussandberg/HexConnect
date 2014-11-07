/**
 ******************************************************************************
 * @file	can2_task.c
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-09-06
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
#include "can2_task.h"

#include "relay.h"
#include "spi_flash_memory_map.h"
#include "spi_flash.h"

#include <string.h>
#include <stdbool.h>

/* Private defines -----------------------------------------------------------*/
#define CANx						CAN2
#define CANx_CLK_ENABLE()			__CAN2_CLK_ENABLE()
#define CANx_GPIO_CLK_ENABLE()		__GPIOB_CLK_ENABLE()

#define CANx_FORCE_RESET()			__CAN2_FORCE_RESET()
#define CANx_RELEASE_RESET()		__CAN2_RELEASE_RESET()

#define CANx_TX_PIN					GPIO_PIN_5
#define CANx_TX_GPIO_PORT			GPIOB
#define CANx_TX_AF					GPIO_AF9_CAN2
#define CANx_RX_PIN					GPIO_PIN_6
#define CANx_RX_GPIO_PORT			GPIOB
#define CANx_RX_AF					GPIO_AF9_CAN2

#define CANx_RX_IRQn				CAN2_RX0_IRQn
#define CANx_RX_IRQHandler			CAN2_RX0_IRQHandler


#define RX_BUFFER_SIZE	(256)

/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static RelayDevice switchRelay = {
		.gpioPort = GPIOE,
		.gpioPin = GPIO_PIN_4,
		.startState = RelayState_Off,
		.msBetweenStateChange = 1000
};
static RelayDevice terminationRelay = {
		.gpioPort = GPIOE,
		.gpioPin = GPIO_PIN_5,
		.startState = RelayState_Off,
		.msBetweenStateChange = 1000
};

static CanTxMsgTypeDef TxMessage;
static CanRxMsgTypeDef RxMessage;

static CAN_HandleTypeDef CAN_Handle = {
		.Instance			= CANx,
		.pTxMsg 			= &TxMessage,
		.pRxMsg 			= &RxMessage,
		.Init.Prescaler 	= CANPrescaler_125k,
		.Init.Mode 			= CAN_MODE_NORMAL,
		.Init.SJW 			= CANSJW_125k,
		.Init.BS1 			= CANBS1_125k,
		.Init.BS2 			= CANBS2_125k,
		.Init.TTCM 			= DISABLE,
		.Init.ABOM 			= DISABLE,
		.Init.AWUM 			= DISABLE,
		.Init.NART 			= DISABLE,
		.Init.RFLM 			= DISABLE,
		.Init.TXFP 			= DISABLE,
};

static CAN_FilterConfTypeDef CAN_Filter = {
		.FilterIdHigh 			= 0x0000,
		.FilterIdLow 			= 0x0000,
		.FilterMaskIdHigh 		= 0x0000,
		.FilterMaskIdLow		= 0x0000,
		.FilterFIFOAssignment 	= 0,
		.FilterNumber 			= 14,
		.FilterMode 			= CAN_FILTERMODE_IDMASK,
		.FilterScale 			= CAN_FILTERSCALE_32BIT,
		.FilterActivation 		= ENABLE,
		.BankNumber 			= 14,	/* This defines the start bank for the CAN2 interface (Slave) in the range 0 to 27. */
};

static CANSettings prvCurrentSettings = {
		.connection 					= CANConnection_Disconnected,
		.termination					= CANTermination_Disconnected,
		.identifier						= CANIdentifier_Standard,
		.bitRate						= CANBitRate_125k,
		.displayedDataStartAddress 		= FLASH_ADR_CAN2_DATA,
		.lastDisplayDataStartAddress	= FLASH_ADR_CAN2_DATA,
		.displayedDataEndAddress		= FLASH_ADR_CAN2_DATA,
		.lastDisplayDataEndAddress		= FLASH_ADR_CAN2_DATA,
		.readAddress					= FLASH_ADR_CAN2_DATA,
		.writeAddress					= FLASH_ADR_CAN2_DATA,
		.numOfCharactersDisplayed		= 0,
		.numOfMessagesSaved				= 0,
};

static SemaphoreHandle_t xSemaphore;
static SemaphoreHandle_t xSettingsSemaphore;


static CANMessage prvRxBuffer1[RX_BUFFER_SIZE];
static uint32_t prvRxBuffer1CurrentIndex = 0;
static uint32_t prvRxBuffer1Count = 0;
static CANBufferState prvRxBuffer1State = CANBufferState_Writing;
static TimerHandle_t prvBuffer1ClearTimer;

static CANMessage prvRxBuffer2[RX_BUFFER_SIZE];
static uint32_t prvRxBuffer2CurrentIndex = 0;
static uint32_t prvRxBuffer2Count = 0;
static CANBufferState prvRxBuffer2State = CANBufferState_Writing;
static TimerHandle_t prvBuffer2ClearTimer;

static bool prvDoneInitializing = false;

/* Private function prototypes -----------------------------------------------*/
static void prvHardwareInit();
static ErrorStatus prvEnableCan2Interface();
static ErrorStatus prvDisableCan2Interface();
static ErrorStatus prvReadSettingsFromSpiFlash();

static void prvBuffer1ClearTimerCallback();
static void prvBuffer2ClearTimerCallback();

/* Functions -----------------------------------------------------------------*/
/**
 * @brief	The main task for the CAN2 channel
 * @param	pvParameters:
 * @retval	None
 */
void can2Task(void *pvParameters)
{
	/* Mutex semaphore to manage when it's ok to send and receive new data */
	xSemaphore = xSemaphoreCreateMutex();

	/* Mutex semaphore for accessing the settings for this channel */
	xSettingsSemaphore = xSemaphoreCreateMutex();

	/* Create software timers */
	prvBuffer1ClearTimer = xTimerCreate("Buf1ClearCan2", 10, pdFALSE, 0, prvBuffer1ClearTimerCallback);
	prvBuffer2ClearTimer = xTimerCreate("Buf2ClearCan2", 10, pdFALSE, 0, prvBuffer2ClearTimerCallback);

	/* Initialize hardware */
	prvHardwareInit();

	/* Wait to make sure the SPI FLASH is initialized */
	while (SPI_FLASH_Initialized() == false)
	{
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}

	/* Try to read the settings from SPI FLASH */
	prvReadSettingsFromSpiFlash();

	/*
	 * TODO: Figure out a good way to allow saved data in SPI FLASH to be read next time we wake up so that we
	 * don't have to do a clear every time we start up the device.
	 */
	can2Clear();

	/* The parameter in vTaskDelayUntil is the absolute time
	 * in ticks at which you want to be woken calculated as
	 * an increment from the time you were last woken. */
	TickType_t xNextWakeTime;
	/* Initialize xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	prvDoneInitializing = true;
	while (1)
	{
		vTaskDelayUntil(&xNextWakeTime, 1000 / portTICK_PERIOD_MS);
		/* Transmit debug data */
//		if (prvCurrentSettings.connection == CANConnection_Connected)
//		{
//			/* Set the data to be transmitted */
//			CAN_Handle.pTxMsg->Data[0] = 2;
//			CAN_Handle.pTxMsg->Data[1] = 0xAD;
//
//			/* Start the Transmission process */
//			if (HAL_CAN_Transmit(&CAN_Handle, 10) != HAL_OK)
//			{
//				/* Transmission Error */
//			}
//		}
	}
}

/**
 * @brief	Check if the channel is done initializing
 * @param	None
 * @retval	true if it's done
 * @retval	false if not done
 */
bool can2IsDoneInitializing()
{
	return prvDoneInitializing;
}

/**
 * @brief	Restart the CAN
 * @param	None
 * @retval	None
 */
void can2Restart()
{
	prvDisableCan2Interface();
	prvEnableCan2Interface();
}

/**
 * @brief	Set whether or not to use termination on the output
 * @param	Termination: Can be any value of CANTermination
 * @retval	SUCCES: Everything went ok
 * @retval	ERROR: Something went wrong
 */
ErrorStatus can2SetTermination(CANTermination Termination)
{
	RelayStatus status;
	if (Termination == CANTermination_Connected)
		status = RELAY_SetState(&terminationRelay, RelayState_On);
	else if (Termination == CANTermination_Disconnected)
		status = RELAY_SetState(&terminationRelay, RelayState_Off);

	if (status == RelayStatus_Ok)
		return SUCCESS;
	else
		return ERROR;
}

/**
 * @brief	Set whether or not the output should be connected to the connector
 * @param	Connection: Can be any value of CANConnection
 * @retval	SUCCES: Everything went ok
 * @retval	ERROR: Something went wrong
 */
ErrorStatus can2SetConnection(CANConnection Connection)
{
	RelayStatus relayStatus = RelayStatus_NotEnoughTimePassed;
	if (Connection == CANConnection_Connected)
		relayStatus = RELAY_SetState(&switchRelay, RelayState_On);
	else if (Connection == CANConnection_Disconnected)
		relayStatus = RELAY_SetState(&switchRelay, RelayState_Off);

	/* Make sure the relay was set or reset correctly */
	if (relayStatus == RelayStatus_Ok)
	{
		ErrorStatus errorStatus = ERROR;
		if (Connection == CANConnection_Connected)
			errorStatus = prvEnableCan2Interface();
		else
			errorStatus = prvDisableCan2Interface();

		/* Make sure the interface was enabled correctly */
		if (errorStatus == SUCCESS)
		{
			prvCurrentSettings.connection = Connection;
			return SUCCESS;
		}
		else
		{
			RELAY_SetState(&switchRelay, RelayState_Off);
			goto error;
		}
	}

error:
	return ERROR;
}

/**
 * @brief	Get the current settings of the CAN2 channel
 * @param	None
 * @retval	A pointer to the current settings
 */
CANSettings* can2GetSettings()
{
	return &prvCurrentSettings;
}

/**
 * @brief	Update with the new settings stored in prvCurrentSettings
 * @param	None
 * @retval	SUCCESS: Everything went ok
 * @retval	ERROR: Something went wrong
 */
ErrorStatus can2UpdateWithNewSettings()
{
	/* Set the values in the USART handle */
	switch (prvCurrentSettings.bitRate) {
		case CANBitRate_10k:
			CAN_Handle.Init.Prescaler = CANPrescaler_10k;
			break;
		case CANBitRate_20k:
			CAN_Handle.Init.Prescaler = CANPrescaler_20k;
			break;
		case CANBitRate_50k:
			CAN_Handle.Init.Prescaler = CANPrescaler_50k;
			break;
		case CANBitRate_100k:
			CAN_Handle.Init.Prescaler = CANPrescaler_100k;
			break;
		case CANBitRate_125k:
			CAN_Handle.Init.Prescaler = CANPrescaler_125k;
			break;
		case CANBitRate_250k:
			CAN_Handle.Init.Prescaler = CANPrescaler_250k;
			break;
		case CANBitRate_500k:
			CAN_Handle.Init.Prescaler = CANPrescaler_500k;
			break;
		case CANBitRate_1M:
			CAN_Handle.Init.Prescaler = CANPrescaler_1M;
			break;
		default:
			break;
	}

	return SUCCESS;
}

/**
 * @brief	Get the settings semaphore
 * @param	None
 * @retval	A pointer to the settings semaphore
 */
SemaphoreHandle_t* can2GetSettingsSemaphore()
{
	return &xSettingsSemaphore;
}

/**
 * @brief	Returns the address which the data will be written to next
 * @param	None
 * @retval	The address
 */
uint32_t can2GetCurrentWriteAddress()
{
	return prvCurrentSettings.writeAddress;
}

/**
 * @brief	Clear the channel
 * @param	None
 * @retval	SUCCESS: Everything went ok
 * @retval	ERROR: Something went wrong
 */
ErrorStatus can2Clear()
{
	/* Try to take the settings semaphore */
	if (xSettingsSemaphore != 0 && xSemaphoreTake(xSettingsSemaphore, 100) == pdTRUE)
	{
		prvCurrentSettings.displayedDataStartAddress = FLASH_ADR_CAN2_DATA;
		prvCurrentSettings.lastDisplayDataStartAddress = FLASH_ADR_CAN2_DATA;
		prvCurrentSettings.displayedDataEndAddress = FLASH_ADR_CAN2_DATA;
		prvCurrentSettings.lastDisplayDataEndAddress = FLASH_ADR_CAN2_DATA;
		prvCurrentSettings.readAddress = FLASH_ADR_CAN2_DATA;
		prvCurrentSettings.writeAddress = FLASH_ADR_CAN2_DATA;
		prvCurrentSettings.numOfCharactersDisplayed = 0;
		prvCurrentSettings.numOfMessagesSaved = 0;

		/* Clear the FLASH */
		can2ClearFlash();

		/* Give back the semaphore now that we are done */
		xSemaphoreGive(xSettingsSemaphore);

		return SUCCESS;
	}
	else
	{
		return ERROR;
	}
}

/**
 * @brief	Clear the FLASH memory by first checking if it's clean or not -> avoids clear when not needed
 * @param	None
 * @retval	None
 */
void can2ClearFlash()
{
	/* Check if the four sectors associated with this channel are clean or not and erase them if necassary */
	if (!SPI_FLASH_SectorIsClean(FLASH_ADR_CAN2_DATA))
		SPI_FLASH_EraseSector(FLASH_ADR_CAN2_DATA);
	if (!SPI_FLASH_SectorIsClean(FLASH_ADR_CAN2_DATA + 0x10000))
		SPI_FLASH_EraseSector(FLASH_ADR_CAN2_DATA + 0x10000);
	if (!SPI_FLASH_SectorIsClean(FLASH_ADR_CAN2_DATA + 0x20000))
		SPI_FLASH_EraseSector(FLASH_ADR_CAN2_DATA + 0x20000);
	if (!SPI_FLASH_SectorIsClean(FLASH_ADR_CAN2_DATA + 0x30000))
		SPI_FLASH_EraseSector(FLASH_ADR_CAN2_DATA + 0x30000);
}

/* Private functions .--------------------------------------------------------*/
/**
 * @brief	Initializes the hardware
 * @param	None
 * @retval	None
 */
static void prvHardwareInit()
{
	/* Init relays */
	RELAY_Init(&switchRelay);
	RELAY_Init(&terminationRelay);

	/* Configure peripheral GPIO */
	CANx_GPIO_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStruct;

	/* CAN2 TX GPIO pin configuration */
	GPIO_InitStruct.Pin 		= CANx_TX_PIN;
	GPIO_InitStruct.Mode 		= GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed 		= GPIO_SPEED_FAST;
	GPIO_InitStruct.Pull 		= GPIO_PULLUP;
	GPIO_InitStruct.Alternate 	= CANx_TX_AF;
	HAL_GPIO_Init(CANx_TX_GPIO_PORT, &GPIO_InitStruct);

	/* CAN2 RX GPIO pin configuration */
	GPIO_InitStruct.Pin 		= CANx_RX_PIN;
	GPIO_InitStruct.Mode 		= GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed 		= GPIO_SPEED_FAST;
	GPIO_InitStruct.Pull		= GPIO_PULLUP;
	GPIO_InitStruct.Alternate 	= CANx_RX_AF;
	HAL_GPIO_Init(CANx_RX_GPIO_PORT, &GPIO_InitStruct);
}

/**
 * @brief	Enables the CAN2 interface with the current settings
 * @param	None
 * @retval	None
 */
static ErrorStatus prvEnableCan2Interface()
{
	/*##-1- Enable peripheral Clocks ###########################################*/
	/* CAN2 Peripheral clock enable */
	CANx_CLK_ENABLE();

	/*##-2- Configure the NVIC #################################################*/
	/* NVIC configuration for CAN2 Reception complete interrupt */
	HAL_NVIC_SetPriority(CANx_RX_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY, 0);
	HAL_NVIC_EnableIRQ(CANx_RX_IRQn);

	/*##-3- Configure the CAN peripheral #######################################*/
	if (HAL_CAN_Init(&CAN_Handle) != HAL_OK)
	{
		/* Initialization Error */
		goto error;
	}

	/*##-4- Configure the CAN Filter ###########################################*/
	if (HAL_CAN_ConfigFilter(&CAN_Handle, &CAN_Filter) != HAL_OK)
	{
		/* Filter configuration Error */
		goto error;
	}

	/*##-5- Configure Transmission process #####################################*/
	CAN_Handle.pTxMsg->StdId = 0x321;
	CAN_Handle.pTxMsg->ExtId = 0x01;
	CAN_Handle.pTxMsg->RTR = CAN_RTR_DATA;
	CAN_Handle.pTxMsg->IDE = CAN_ID_STD;
	CAN_Handle.pTxMsg->DLC = 2;

	/*##-6- Start the Reception process and enable reception interrupt #########*/
	if (HAL_CAN_Receive_IT(&CAN_Handle, CAN_FIFO0) != HAL_OK)
	{
		/* Reception Error */
		goto error;
	}

	return SUCCESS;

error:
	/* Something went wrong so disable */
	prvDisableCan2Interface();
	return ERROR;
}

/**
 * @brief	Disables the CAN2 interface
 * @param	None
 * @retval	None
 */
static ErrorStatus prvDisableCan2Interface()
{
	/*##-1- Reset peripherals ##################################################*/
	CANx_FORCE_RESET();
	CANx_RELEASE_RESET();

	/*##-3- Disable the NVIC for CAN reception #################################*/
	HAL_NVIC_DisableIRQ(CANx_RX_IRQn);

	return SUCCESS;
}

/**
 * @brief	Read settings from SPI FLASH
 * @param	None
 * @retval	None
 */
static ErrorStatus prvReadSettingsFromSpiFlash()
{
	/* Read to a temporary settings variable */
	CANSettings settings = {0};
	if (SPI_FLASH_ReadBufferDMA((uint8_t*)&settings, FLASH_ADR_CAN2_SETTINGS, sizeof(CANSettings), 2000) == SUCCESS)
	{
		/* Check to make sure the data is reasonable */
		if (IS_CAN_CONNECTION(settings.connection) &&
			IS_CAN_TERMINATION(settings.termination) &&
			IS_CAN_BIT_RATE(settings.bitRate))
		{
			/* Try to take the settings semaphore */
			if (xSettingsSemaphore != 0 && xSemaphoreTake(xSettingsSemaphore, 100) == pdTRUE)
			{
				/* Copy to the real settings variable */
				memcpy(&prvCurrentSettings, &settings, sizeof(CANSettings));
				prvCurrentSettings.connection = CANConnection_Disconnected;
				prvCurrentSettings.termination = CANTermination_Disconnected;
				can2UpdateWithNewSettings();
				/* Give back the semaphore now that we are done */
				xSemaphoreGive(xSettingsSemaphore);
				return SUCCESS;
			}
			else
			{
				/* Something went wrong as we couldn't take the semaphore */
				return ERROR;
			}
		}
		else
			return ERROR;
	}
	else
		return ERROR;
}

/**
 * @brief	Callback for the buffer 1 software timer
 * @param	None
 * @retval	None
 */
static void prvBuffer1ClearTimerCallback()
{
	/* Set the buffer to reading state */
	prvRxBuffer1State = CANBufferState_Reading;

	/* Write the data to FLASH */
	for (uint32_t i = 0; i < prvRxBuffer1Count; i++)
	{
		uint8_t* pData = (uint8_t*)&prvRxBuffer1[i];

		/* ID - 4 bytes */
		SPI_FLASH_WriteByte(prvCurrentSettings.writeAddress++, *(pData++));
		SPI_FLASH_WriteByte(prvCurrentSettings.writeAddress++, *(pData++));
		SPI_FLASH_WriteByte(prvCurrentSettings.writeAddress++, *(pData++));
		SPI_FLASH_WriteByte(prvCurrentSettings.writeAddress++, *(pData++));

		/* DLC - 1 byte */
		SPI_FLASH_WriteByte(prvCurrentSettings.writeAddress++, *(pData));

		/* Data - 0-8 bytes */
		uint8_t dlc = *pData++;
		for (uint32_t n = 0; n < dlc; n++)
		{
			SPI_FLASH_WriteByte(prvCurrentSettings.writeAddress++, *(pData++));
		}

		/* Update how many message we have saved */
		prvCurrentSettings.numOfMessagesSaved++;
	}
	/* TODO: Something strange with the FLASH page write so doing one byte at a time now */
//	/* Write all the data in the buffer to SPI FLASH */
//	SPI_FLASH_WriteBuffer(prvRxBuffer1, prvCurrentSettings.writeAddress, prvRxBuffer1Count);
//	/* Update the write address */
//	prvCurrentSettings.writeAddress += prvRxBuffer1Count;

	/* Reset the buffer */
	prvRxBuffer1CurrentIndex = 0;
	prvRxBuffer1Count = 0;
	prvRxBuffer1State = CANBufferState_Writing;
}

/**
 * @brief	Callback for the buffer 2 software timer
 * @param	None
 * @retval	None
 */
static void prvBuffer2ClearTimerCallback()
{
	/* Set the buffer to reading state */
	prvRxBuffer2State = CANBufferState_Reading;

	/* Write the data to FLASH */
	for (uint32_t i = 0; i < prvRxBuffer2Count; i++)
	{
		uint8_t* pData = (uint8_t*)&prvRxBuffer2[i];

		/* ID - 4 bytes */
		SPI_FLASH_WriteByte(prvCurrentSettings.writeAddress++, *(pData++));
		SPI_FLASH_WriteByte(prvCurrentSettings.writeAddress++, *(pData++));
		SPI_FLASH_WriteByte(prvCurrentSettings.writeAddress++, *(pData++));
		SPI_FLASH_WriteByte(prvCurrentSettings.writeAddress++, *(pData++));

		/* DLC - 1 byte */
		SPI_FLASH_WriteByte(prvCurrentSettings.writeAddress++, *(pData));

		/* Data - 0-8 bytes */
		uint8_t dlc = *pData++;
		for (uint32_t n = 0; n < dlc; n++)
		{
			SPI_FLASH_WriteByte(prvCurrentSettings.writeAddress++, *(pData++));
		}

		/* Update how many message we have saved */
		prvCurrentSettings.numOfMessagesSaved++;
	}
	/* TODO: Something strange with the FLASH page write so doing one byte at a time now */
//	/* Write all the data in the buffer to SPI FLASH */
//	SPI_FLASH_WriteBuffer(prvRxBuffer2, prvCurrentSettings.writeAddress, prvRxBuffer2Count);
//	/* Update the write address */
//	prvCurrentSettings.writeAddress += prvRxBuffer2Count;

	/* Reset the buffer */
	prvRxBuffer2CurrentIndex = 0;
	prvRxBuffer2Count = 0;
	prvRxBuffer2State = CANBufferState_Writing;
}



/* Interrupt Handlers --------------------------------------------------------*/
/**
* @brief  This function handles CAN2 RX0 interrupt request.
* @param  None
* @retval None
*/
void CAN2_RX0_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&CAN_Handle);
}

/**
* @brief  This function handles CAN2 RX1 interrupt request.
* @param  None
* @retval None
*/
void CAN2_RX1_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&CAN_Handle);
}

/**
* @brief  This function handles CAN2 TX interrupt request.
* @param  None
* @retval None
*/
void CAN2_TX_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&CAN_Handle);
}

/* HAL Callback functions ----------------------------------------------------*/
/**
  * @brief  Rx Transfer completed callback
  * @param  None
  * @retval None
  */
void can2RxCpltCallback()
{
//	if ((CAN_Handle.pRxMsg->StdId == 0x321) && (CAN_Handle.pRxMsg->IDE == CAN_ID_STD) && (CAN_Handle.pRxMsg->DLC == 2))
//	{
//		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_3);
//		volatile uint8_t test1 = CAN_Handle.pRxMsg->Data[0];
//		volatile uint8_t test2 = CAN_Handle.pRxMsg->Data[1];
//	}

	if (prvRxBuffer1State != CANBufferState_Reading && prvRxBuffer1Count < RX_BUFFER_SIZE)
	{
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_3);
		prvRxBuffer1State = CANBufferState_Writing;
		/* Save the message */
		prvRxBuffer1[prvRxBuffer1CurrentIndex].id = CAN_Handle.pRxMsg->StdId;
		prvRxBuffer1[prvRxBuffer1CurrentIndex].dlc = CAN_Handle.pRxMsg->DLC;
		for (uint32_t i = 0; i < CAN_Handle.pRxMsg->DLC; i++)
			prvRxBuffer1[prvRxBuffer1CurrentIndex].data[i] = CAN_Handle.pRxMsg->Data[i];

		/* Increment the counters */
		prvRxBuffer1CurrentIndex++;
		prvRxBuffer1Count++;

		/* Start the timer which will clear the buffer if it's not already started */
		if (xTimerIsTimerActive(prvBuffer1ClearTimer) == pdFALSE)
			xTimerStartFromISR(prvBuffer1ClearTimer, NULL);
	}
	else if (prvRxBuffer2State != CANBufferState_Reading && prvRxBuffer2Count < RX_BUFFER_SIZE)
	{
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_3);
		prvRxBuffer2State = CANBufferState_Writing;
		/* Save the message */
		prvRxBuffer2[prvRxBuffer1CurrentIndex].id = CAN_Handle.pRxMsg->StdId;
		prvRxBuffer2[prvRxBuffer1CurrentIndex].dlc = CAN_Handle.pRxMsg->DLC;
		for (uint32_t i = 0; i < CAN_Handle.pRxMsg->DLC; i++)
			prvRxBuffer2[prvRxBuffer1CurrentIndex].data[i] = CAN_Handle.pRxMsg->Data[i];

		/* Increment the counters */
		prvRxBuffer2CurrentIndex++;
		prvRxBuffer2Count++;

		/* Start the timer which will clear the buffer if it's not already started */
		if (xTimerIsTimerActive(prvBuffer2ClearTimer) == pdFALSE)
			xTimerStartFromISR(prvBuffer2ClearTimer, NULL);
	}
	else
	{
		/* No buffer available, something has gone wrong */
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_3);
	}




	/* Receive */
	if (HAL_CAN_Receive_IT(&CAN_Handle, CAN_FIFO0) != HAL_OK)
	{
		/* Reception Error */
//		Error_Handler();
	}
}
