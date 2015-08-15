/**
 *******************************************************************************
 * @file  can1_task.c
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
#include "can1_task.h"

#include "relay.h"
#include "spi_flash_memory_map.h"
#include "spi_flash.h"

#include <string.h>
#include <stdbool.h>

/* Private defines -----------------------------------------------------------*/
#define CAN1_GPIO_CLK_ENABLE()		__GPIOB_CLK_ENABLE()
#define CAN1_TX_PIN					GPIO_PIN_9
#define CAN1_TX_GPIO_PORT			GPIOB
#define CAN1_TX_AF					GPIO_AF9_CAN1
#define CAN1_RX_PIN					GPIO_PIN_8
#define CAN1_RX_GPIO_PORT			GPIOB
#define CAN1_RX_AF					GPIO_AF9_CAN1

#define RX_BUFFER_SIZE	(256)

/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static RelayDevice switchRelay = {
		.gpioPort = GPIOE,
		.gpioPin = GPIO_PIN_2,
		.startState = RelayState_Off,
		.msBetweenStateChange = 1000
};
static RelayDevice terminationRelay = {
		.gpioPort = GPIOE,
		.gpioPin = GPIO_PIN_3,
		.startState = RelayState_Off,
		.msBetweenStateChange = 1000
};

static CanTxMsgTypeDef TxMessage;
static CanRxMsgTypeDef RxMessage;

static CAN_HandleTypeDef CAN_Handle = {
		.Instance			= CAN1,
		.pTxMsg 			= &TxMessage,
		.pRxMsg 			= &RxMessage,
		.Init.Prescaler 	= CANPrescaler_125k,
		.Init.Mode 			= CAN_MODE_SILENT,
		.Init.SJW 			= CANSJW_125k,
		.Init.BS1 			= CANBS1_125k,
		.Init.BS2 			= CANBS2_125k,
		.Init.TTCM 			= DISABLE,
		.Init.ABOM 			= ENABLE,	/* Enable the automatic bus-off management */
		.Init.AWUM 			= ENABLE,	/* Enable the automatic wake-up mode */
		.Init.NART 			= DISABLE,
		.Init.RFLM 			= DISABLE,
		.Init.TXFP 			= DISABLE,
};

static CAN_FilterConfTypeDef CAN_Filter = {
		.FilterIdHigh 			= 0x0000,
		.FilterIdLow 			= 0x0000,
		.FilterMaskIdHigh 		= 0x0000,
		.FilterMaskIdLow		= 0x0000,
		.FilterFIFOAssignment 	= CAN_FILTER_FIFO0,
		.FilterNumber 			= 0,
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
		.displayedDataStartAddress 		= FLASH_ADR_CAN1_DATA,
		.lastDisplayDataStartAddress	= FLASH_ADR_CAN1_DATA,
		.displayedDataEndAddress		= FLASH_ADR_CAN1_DATA,
		.lastDisplayDataEndAddress		= FLASH_ADR_CAN1_DATA,
		.readAddress					= FLASH_ADR_CAN1_DATA,
		.writeAddress					= FLASH_ADR_CAN1_DATA,
		.numOfCharactersDisplayed		= 0,
		.numOfMessagesSaved				= 0,
};

static uint8_t* prvCanStatusMessages[4] = {
		"CAN1: HAL_OK",
		"CAN1: HAL_ERROR",
		"CAN1: HAL_BUSY",
		"CAN1: HAL_TIMEOUT",
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
static ErrorStatus prvEnableCan1Interface();
static ErrorStatus prvDisableCan1Interface();
static ErrorStatus prvReadSettingsFromSpiFlash();

static void prvBuffer1ClearTimerCallback();
static void prvBuffer2ClearTimerCallback();

/* Functions -----------------------------------------------------------------*/
/**
 * @brief	The main task for the CAN1 channel
 * @param	pvParameters:
 * @retval	None
 */
void can1Task(void *pvParameters)
{
	/* Mutex semaphore to manage when it's ok to send and receive new data */
	xSemaphore = xSemaphoreCreateMutex();

	/* Mutex semaphore for accessing the settings for this channel */
	xSettingsSemaphore = xSemaphoreCreateMutex();

	/* Create software timers */
	prvBuffer1ClearTimer = xTimerCreate("Buf1ClearCan1", 10, pdFALSE, 0, prvBuffer1ClearTimerCallback);
	prvBuffer2ClearTimer = xTimerCreate("Buf2ClearCan1", 10, pdFALSE, 0, prvBuffer2ClearTimerCallback);

	/* Initialize hardware */
	prvHardwareInit();

	/* Wait to make sure the SPI FLASH is initialized */
	while (SPI_FLASH_Initialized() == false)
	{
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}

	/* Try to read the settings from SPI FLASH */
	prvReadSettingsFromSpiFlash();

	can1Clear();

	/* The parameter in vTaskDelayUntil is the absolute time
	 * in ticks at which you want to be woken calculated as
	 * an increment from the time you were last woken. */
	TickType_t xNextWakeTime;
	/* Initialize xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	uint8_t count = 0;

	prvDoneInitializing = true;
	while (1)
	{
		vTaskDelayUntil(&xNextWakeTime, 1000 / portTICK_PERIOD_MS);
//		/* Transmit debug data */
//		if (prvCurrentSettings.connection == CANConnection_Connected)
//		{
//			/* Set the data to be transmitted */
//			uint8_t data[2] = {0xAA, count};
//			can1Transmit(0x321, data, CANDataLength_2, 50);
//			count++;
//
//			if (count % 10 == 0)
//			{
//				uint8_t data2[5] = {0x72, 0x21, 0xDE, 0x03, 0xFA};
//				can1Transmit(0x321, data2, CANDataLength_5, 50);
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
bool can1IsDoneInitializing()
{
	return prvDoneInitializing;
}


/**
 * @brief	Restart the CAN
 * @param	None
 * @retval	None
 */
void can1Restart()
{
	prvDisableCan1Interface();
	prvEnableCan1Interface();
}

/**
 * @brief	Set whether or not to use termination on the output
 * @param	Termination: Can be any value of CANTermination
 * @retval	SUCCES: Everything went ok
 * @retval	ERROR: Something went wrong
 */
ErrorStatus can1SetTermination(CANTermination Termination)
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
ErrorStatus can1SetConnection(CANConnection Connection)
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
			errorStatus = prvEnableCan1Interface();
		else
			errorStatus = prvDisableCan1Interface();

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
 * @brief	Get the current settings of the CAN1 channel
 * @param	None
 * @retval	A pointer to the current settings
 */
CANSettings* can1GetSettings()
{
	return &prvCurrentSettings;
}

/**
 * @brief	Update with the new settings stored in prvCurrentSettings
 * @param	None
 * @retval	SUCCESS: Everything went ok
 * @retval	ERROR: Something went wrong
 */
ErrorStatus can1UpdateWithNewSettings()
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
SemaphoreHandle_t* can1GetSettingsSemaphore()
{
	return &xSettingsSemaphore;
}

/**
 * @brief	Returns the address which the data will be written to next
 * @param	None
 * @retval	The address
 */
uint32_t can1GetCurrentWriteAddress()
{
	return prvCurrentSettings.writeAddress;
}

/**
 * @brief	Clear the channel
 * @param	None
 * @retval	SUCCESS: Everything went ok
 * @retval	ERROR: Something went wrong
 */
ErrorStatus can1Clear()
{
	/* Try to take the settings semaphore */
	if (xSettingsSemaphore != 0 && xSemaphoreTake(xSettingsSemaphore, 1000) == pdTRUE)
	{
		prvCurrentSettings.displayedDataStartAddress = FLASH_ADR_CAN1_DATA;
		prvCurrentSettings.lastDisplayDataStartAddress = FLASH_ADR_CAN1_DATA;
		prvCurrentSettings.displayedDataEndAddress = FLASH_ADR_CAN1_DATA;
		prvCurrentSettings.lastDisplayDataEndAddress = FLASH_ADR_CAN1_DATA;
		prvCurrentSettings.readAddress = FLASH_ADR_CAN1_DATA;
		prvCurrentSettings.writeAddress = FLASH_ADR_CAN1_DATA;
		prvCurrentSettings.numOfCharactersDisplayed = 0;
		prvCurrentSettings.numOfMessagesSaved = 0;

		/* Clear the FLASH */
		can1ClearFlash();

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
void can1ClearFlash()
{
	/* Check if the sectors associated with this channel are clean or not and erase them if necassary */
	for (uint32_t i = 0; i < 15; i++)
	{
		if (!SPI_FLASH_SectorIsClean(FLASH_ADR_CAN1_DATA + i * 0x10000))
			SPI_FLASH_EraseSector(FLASH_ADR_CAN1_DATA + i * 0x10000);
	}
}

/**
 * @brief	Set the settings of the CAN1 channel
 * @param	MessageId: The message to transmit
 * @param	pData: Pointer to the data to transmit
 * @param	DataLength: Length of the data to transmit, can be any value of CANDataLength
 * @param	Timeout: Timeout when we should stop trying to send if the CAN is busy
 * @retval	None
 */
ErrorStatus can1Transmit(uint32_t MessageId, uint8_t* pData, CANDataLength DataLength, uint32_t Timeout)
{
	/* Save the message ID */
	if (prvCurrentSettings.identifier == CANIdentifier_Standard && MessageId <= 0x7FF)
	{
		CAN_Handle.pTxMsg->StdId = MessageId;
		CAN_Handle.pTxMsg->ExtId = 0x0;
		CAN_Handle.pTxMsg->IDE = CAN_ID_STD;
	}
	else if (prvCurrentSettings.identifier == CANIdentifier_Extended && MessageId <= 0x1FFFFFFF)
	{
		CAN_Handle.pTxMsg->StdId = 0x0;
		CAN_Handle.pTxMsg->ExtId = MessageId;
		CAN_Handle.pTxMsg->IDE = CAN_ID_EXT;
	}
	else
		goto error;

	/* Data frame */
	CAN_Handle.pTxMsg->RTR = CAN_RTR_DATA;

	/* Data length */
	CAN_Handle.pTxMsg->DLC = DataLength;

	/* Save the data */
	for (uint32_t i = 0; i < DataLength; i++)
	{
		CAN_Handle.pTxMsg->Data[i] = pData[i];
	}

	/* Save the current time and keep trying to transmit until the timeout happens */
	TickType_t currentTime = xTaskGetTickCount();
	uint32_t RetryCount = 0;
	while (HAL_CAN_Transmit_IT(&CAN_Handle) != HAL_OK)
	{
		vTaskDelayUntil(&currentTime, 1 / portTICK_PERIOD_MS);
		RetryCount++;
		if (RetryCount >= Timeout)
			goto error;
	}

	/* Everything went OK */
	return SUCCESS;

error:
	/* Something went wrong */
	return ERROR;
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
	CAN1_GPIO_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStruct;

	/* CAN2 TX GPIO pin configuration */
	GPIO_InitStruct.Pin 		= CAN1_TX_PIN;
	GPIO_InitStruct.Mode 		= GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed 		= GPIO_SPEED_FAST;
	GPIO_InitStruct.Pull 		= GPIO_PULLUP;
	GPIO_InitStruct.Alternate 	= CAN1_TX_AF;
	HAL_GPIO_Init(CAN1_TX_GPIO_PORT, &GPIO_InitStruct);

	/* CAN2 RX GPIO pin configuration */
	GPIO_InitStruct.Pin 		= CAN1_RX_PIN;
	GPIO_InitStruct.Mode 		= GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed 		= GPIO_SPEED_FAST;
	GPIO_InitStruct.Pull		= GPIO_PULLUP;
	GPIO_InitStruct.Alternate 	= CAN1_RX_AF;
	HAL_GPIO_Init(CAN1_RX_GPIO_PORT, &GPIO_InitStruct);
}

/**
 * @brief	Enables the CAN1 interface with the current settings
 * @param	None
 * @retval	None
 */
static ErrorStatus prvEnableCan1Interface()
{
	/*##-1- Enable peripheral Clocks ###########################################*/
	/* CAN1 Peripheral clock enable */
	__CAN1_CLK_ENABLE();

	/*##-2- Configure the NVIC #################################################*/
	HAL_NVIC_SetPriority(CAN1_RX0_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY, 0);
	HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);

	HAL_NVIC_SetPriority(CAN1_TX_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY, 0);
	HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);

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

	/*##-6- Start the Reception process and enable reception interrupt #########*/
	if (HAL_CAN_Receive_IT(&CAN_Handle, CAN_FIFO0) != HAL_OK)
	{
		/* Reception Error */
		goto error;
	}

	return SUCCESS;

error:
	/* Something went wrong so disable */
	prvDisableCan1Interface();
	return ERROR;
}

/**
 * @brief	Disables the CAN1 interface
 * @param	None
 * @retval	None
 */
static ErrorStatus prvDisableCan1Interface()
{
	/*##-1- Reset peripherals ##################################################*/
	__CAN1_FORCE_RESET();
	__CAN1_RELEASE_RESET();

	/*##-2- Disable the NVIC for CAN reception #################################*/
	HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
	HAL_NVIC_DisableIRQ(CAN1_TX_IRQn);

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
	if (SPI_FLASH_ReadBufferDMA((uint8_t*)&settings, FLASH_ADR_CAN1_SETTINGS, sizeof(CANSettings), 2000) == SUCCESS)
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
				can1UpdateWithNewSettings();
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

	/* Reset the buffer */
	prvRxBuffer2CurrentIndex = 0;
	prvRxBuffer2Count = 0;
	prvRxBuffer2State = CANBufferState_Writing;
}

/* Interrupt Handlers --------------------------------------------------------*/
/**
* @brief  This function handles CAN1 RX FIFO 0 interrupt request.
* @param  None
* @retval None
*/
void CAN1_RX0_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&CAN_Handle);
}

/**
* @brief  This function handles CAN1 RX FIFO 1 interrupt request.
* @param  None
* @retval None
*/
void CAN1_RX1_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&CAN_Handle);
}

/**
* @brief  This function handles CAN1 TX interrupt request.
* @param  None
* @retval None
*/
void CAN1_TX_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&CAN_Handle);
}

/* HAL Callback functions ----------------------------------------------------*/
/**
  * @brief  TX Transfer completed callback
  * @param  None
  * @retval None
  */
void can1TxCpltCallback()
{
	/* TODO: Do something */
}

/**
  * @brief  RX Transfer completed callback
  * @param  None
  * @retval None
  */
void can1RxCpltCallback()
{
//	if ((CAN_Handle.pRxMsg->StdId == 0x321) && (CAN_Handle.pRxMsg->IDE == CAN_ID_STD) && (CAN_Handle.pRxMsg->DLC == 2))
//	{
//		uint8_t ubKeyNumber = CAN_Handle.pRxMsg->Data[0];
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
		/* TODO: Do something */
	}
}

/**
  * @brief  Error callback
  * @param  None
  * @retval None
  */
void can1ErrorCallback()
{
	/* TODO: Do something */
}
