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

#include <string.h>

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
		.connection 			= CANConnection_Disconnected,
		.termination			= CANTermination_Disconnected,
		.identifier				= CANIdentifier_Standard,
};

/* Private function prototypes -----------------------------------------------*/
static void prvHardwareInit();
static ErrorStatus prvEnableCan2Interface();
static ErrorStatus prvDisableCan2Interface();

/* Functions -----------------------------------------------------------------*/
/**
 * @brief	The main task for the CAN2 channel
 * @param	pvParameters:
 * @retval	None
 */
void can2Task(void *pvParameters)
{
	prvHardwareInit();

	/* The parameter in vTaskDelayUntil is the absolute time
	 * in ticks at which you want to be woken calculated as
	 * an increment from the time you were last woken. */
	TickType_t xNextWakeTime;
	/* Initialize xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

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

	if (relayStatus == RelayStatus_Ok)
	{
		ErrorStatus errorStatus = ERROR;
		if (Connection == CANConnection_Connected)
			errorStatus = prvEnableCan2Interface();
		else
			errorStatus = prvDisableCan2Interface();

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
 * @brief	Set the settings of the CAN2 channel
 * @param	Settings: New settings to use
 * @retval	SUCCES: Everything went ok
 * @retval	ERROR: Something went wrong
 */
ErrorStatus can2SetSettings(CANSettings* Settings)
{
	mempcpy(&prvCurrentSettings, Settings, sizeof(CANSettings));

	return SUCCESS;
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
	if ((CAN_Handle.pRxMsg->StdId == 0x321) && (CAN_Handle.pRxMsg->IDE == CAN_ID_STD) && (CAN_Handle.pRxMsg->DLC == 2))
	{
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_3);
		volatile uint8_t test1 = CAN_Handle.pRxMsg->Data[0];
		volatile uint8_t test2 = CAN_Handle.pRxMsg->Data[1];
	}

	/* Receive */
	if (HAL_CAN_Receive_IT(&CAN_Handle, CAN_FIFO0) != HAL_OK)
	{
		/* Reception Error */
//		Error_Handler();
	}
}
