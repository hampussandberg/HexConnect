/**
 ******************************************************************************
 * @file	can1_task.c
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
#include "can1_task.h"

#include "relay.h"
#include "spi_flash_memory_map.h"
#include "spi_flash.h"

#include <string.h>

/* Private defines -----------------------------------------------------------*/
#define CANx						CAN1
#define CANx_CLK_ENABLE()			__CAN1_CLK_ENABLE()
#define CANx_GPIO_CLK_ENABLE()		__GPIOB_CLK_ENABLE()

#define CANx_FORCE_RESET()			__CAN1_FORCE_RESET()
#define CANx_RELEASE_RESET()		__CAN1_RELEASE_RESET()

#define CANx_TX_PIN					GPIO_PIN_9
#define CANx_TX_GPIO_PORT			GPIOB
#define CANx_TX_AF					GPIO_AF9_CAN1
#define CANx_RX_PIN					GPIO_PIN_8
#define CANx_RX_GPIO_PORT			GPIOB
#define CANx_RX_AF					GPIO_AF9_CAN1

#define CANx_RX0_IRQn				CAN1_RX0_IRQn
#define CANx_RX0_IRQHandler			CAN1_RX0_IRQHandler

#define CANx_TX_IRQn				CAN1_TX_IRQn
#define CANx_TX_IRQHandler			CAN1_TX_IRQHandler

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
		.FilterNumber 			= 0,
		.FilterMode 			= CAN_FILTERMODE_IDMASK,
		.FilterScale 			= CAN_FILTERSCALE_32BIT,
		.FilterActivation 		= ENABLE,
		.BankNumber 			= 14,	/* This defines the start bank for the CAN2 interface (Slave) in the range 0 to 27. */
};

static CANSettings prvCurrentSettings = {
		.connection 			= CANConnection_Disconnected,
		.termination			= CANTermination_Disconnected,
		.identifier				= CANIdentifier_Standard,
		.bitRate				= CANBitRate_125k,
};

static uint8_t* prvCanStatusMessages[4] = {
		"CAN1: HAL_OK",
		"CAN1: HAL_ERROR",
		"CAN1: HAL_BUSY",
		"CAN1: HAL_TIMEOUT",
};

static SemaphoreHandle_t xSemaphore;
static SemaphoreHandle_t xSettingsSemaphore;

/* Private function prototypes -----------------------------------------------*/
static void prvHardwareInit();
static ErrorStatus prvEnableCan1Interface();
static ErrorStatus prvDisableCan1Interface();

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

	/* Initialize hardware */
	prvHardwareInit();

	vTaskDelay(2000);

	can1SetTermination(CANTermination_Connected);
	can1SetConnection(CANConnection_Connected);

	/* The parameter in vTaskDelayUntil is the absolute time
	 * in ticks at which you want to be woken calculated as
	 * an increment from the time you were last woken. */
	TickType_t xNextWakeTime;
	/* Initialize xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	uint8_t count = 0;

	while (1)
	{
		/* Transmit debug data */

		/* Set the data to be transmitted */
		uint8_t data[4] = {0xAA, 0x55, count, count*2};
		can1Transmit(0x321, data, CANDataLength_4, 50);

		vTaskDelayUntil(&xNextWakeTime, 428 / portTICK_PERIOD_MS);

		/* Set the data to be transmitted */
		uint8_t data2[2] = {0xDD, count % 2};
		can1Transmit(0x512, data2, CANDataLength_2, 50);

		count++;

		vTaskDelayUntil(&xNextWakeTime, 527 / portTICK_PERIOD_MS);
	}
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
 * @brief	Enables the CAN1 interface with the current settings
 * @param	None
 * @retval	None
 */
static ErrorStatus prvEnableCan1Interface()
{
	/*##-1- Enable peripheral Clocks ###########################################*/
	/* CAN1 Peripheral clock enable */
	CANx_CLK_ENABLE();

	/*##-2- Configure the NVIC #################################################*/
	HAL_NVIC_SetPriority(CANx_RX0_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY, 0);
	HAL_NVIC_EnableIRQ(CANx_RX0_IRQn);

	HAL_NVIC_SetPriority(CANx_TX_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY, 0);
	HAL_NVIC_EnableIRQ(CANx_TX_IRQn);

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
	CANx_FORCE_RESET();
	CANx_RELEASE_RESET();

	/*##-2- Disable the NVIC for CAN reception #################################*/
	HAL_NVIC_DisableIRQ(CANx_RX0_IRQn);
	HAL_NVIC_DisableIRQ(CANx_TX_IRQn);

	return SUCCESS;
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
	if ((CAN_Handle.pRxMsg->StdId == 0x321) && (CAN_Handle.pRxMsg->IDE == CAN_ID_STD) && (CAN_Handle.pRxMsg->DLC == 2))
	{
		uint8_t ubKeyNumber = CAN_Handle.pRxMsg->Data[0];
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
