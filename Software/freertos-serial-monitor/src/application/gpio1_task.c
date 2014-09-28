/**
 ******************************************************************************
 * @file	gpio1_task.c
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
#include "gpio1_task.h"

/* Private defines -----------------------------------------------------------*/
#define GPIO1_PORT				(GPIOB)
#define GPIO1_PIN				(GPIO_PIN_1)
#define GPIO1_DIRECTION_PORT	(GPIOC)
#define GPIO1_DIRECTION_PIN		(GPIO_PIN_5)


/* TODO: Use timer 8 instead */
#define PWM_TIMER				(TIM3)
#define PWM_TIMER_CLK_ENABLE	(__TIM3_CLK_ENABLE())
#define PWM_AF_GPIO				(GPIO_AF2_TIM3)
#define PWM_TIMER_CHANNEL		(TIM_CHANNEL_4)
#define PWM_TIMER_CLOCK			(42000000)	/* 42 MHz, see datasheet page 31 */
#define PWM_PERIOD				(255)		/* 256 step PWM */
#define PWM_PRESCALER			(6)			/* Divide by 7 */
#define PWM_FREQ				(PWM_TIMER_CLOCK/((PWM_PRESCALER+1) * (PWM_PERIOD+1)))	/* Is not valid in PWM mode */

/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Default settings that can be overwritten if valid settings are read from the SPI FLASH */
static GPIOSettings prvCurrentSettings = {
		.direction		= GPIODirection_Output,
		.pull			= GPIOPull_NoPull,
};

static bool prvIsEnabled = false;
static float prvCurrentDuty = 50.0;

/* Private function prototypes -----------------------------------------------*/
static void prvHardwareInit();
static void prvActivatePwmFunctionality();
static void prvDeactivatePwmFunctionality();

/* Functions -----------------------------------------------------------------*/
/**
 * @brief	The main task for the GPIO1 channel
 * @param	pvParameters:
 * @retval	None
 */
void gpio1Task(void *pvParameters)
{
	prvHardwareInit();

	gpio1SetDirection(GPIODirection_Output);

	/* The parameter in vTaskDelayUntil is the absolute time
	 * in ticks at which you want to be woken calculated as
	 * an increment from the time you were last woken. */
	TickType_t xNextWakeTime;
	/* Initialize xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	while (1)
	{
		vTaskDelayUntil(&xNextWakeTime, 1000 / portTICK_PERIOD_MS);
	}
}

/**
 * @brief	Sets the direction of the GPIO
 * @param	Direction: The direction to use, can be any value of GPIODirection
 * @retval	None
 */
void gpio1SetDirection(GPIODirection Direction)
{
	prvCurrentSettings.direction = Direction;
}

/**
 * @brief	Get the curren direction of the channel
 * @param	None
 * @retval	The direction as a GPIODirection
 */
GPIODirection gpio1GetDirection()
{
	return prvCurrentSettings.direction;
}

/**
 * @brief	Read the value of the pin
 * @param	None
 * @retval	None
 */
GPIO_PinState gpio1ReadPin()
{
	return HAL_GPIO_ReadPin(GPIO1_PORT, GPIO1_PIN);
}

/**
 * @brief	Write a value to the pin
 * @param	None
 * @retval	None
 */
void gpio1WritePin(GPIO_PinState PinState)
{
	/* TODO: Make sure it's set as an output, otherwise return error */
	HAL_GPIO_WritePin(GPIO1_PORT, GPIO1_PIN, PinState);
}

/**
 * @brief	Toggle the value of the pin
 * @param	None
 * @retval	None
 */
void gpio1TogglePin()
{
	/* TODO: Make sure it's set as an output, otherwise return error */
	HAL_GPIO_TogglePin(GPIO1_PORT, GPIO1_PIN);
}

/**
 * @brief	Set the duty of the PWM
 * @param	Duty: The duty as a percentage (0.0 - 100.0%)
 * @retval	None
 */
void gpio1SetPwmDuty(float Duty)
{
	if (Duty >= 0.0 && Duty <= 100.0)
	{
		PWM_TIMER->CCR4 = (uint16_t)(Duty/100.0 * PWM_PERIOD);

		prvCurrentDuty = Duty;
	}
}

/**
 * @brief	Get the duty of the PWM
 * @param	None
 * @retval	The duty as a percentage (0.0 - 100.0%)
 */
float gpio1GetPwmDuty()
{
	return prvCurrentDuty;
}

/**
 * @brief	Enable the GPIO with the currently set settings
 * @param	None
 * @retval	None
 */
void gpio1Enable()
{
	/* If it's PWM we have to activate the timer and such */
	if (prvCurrentSettings.direction == GPIODirection_OutputPWM)
	{
		prvActivatePwmFunctionality();
	}
	else
	{
		/* Init the GPIO */
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_InitStructure.Pin  	= GPIO1_PIN;
		GPIO_InitStructure.Speed 	= GPIO_SPEED_HIGH;
		if (prvCurrentSettings.direction == GPIODirection_Output)
		{
			GPIO_InitStructure.Pull		= GPIOPull_NoPull;
			GPIO_InitStructure.Mode  	= GPIO_MODE_OUTPUT_PP;
		}
		else
		{
			GPIO_InitStructure.Pull		= prvCurrentSettings.pull;
			GPIO_InitStructure.Mode  	= GPIO_MODE_INPUT;
		}
		HAL_GPIO_Init(GPIO1_PORT, &GPIO_InitStructure);
	}

	/* Set the direction pin */
	if (prvCurrentSettings.direction == GPIODirection_Input)
		HAL_GPIO_WritePin(GPIO1_DIRECTION_PORT, GPIO1_DIRECTION_PIN, GPIO_PIN_SET);
	else if (prvCurrentSettings.direction == GPIODirection_Output || GPIODirection_OutputPWM)
		HAL_GPIO_WritePin(GPIO1_DIRECTION_PORT, GPIO1_DIRECTION_PIN, GPIO_PIN_RESET);

	prvIsEnabled = true;
}

/**
 * @brief	Disable the GPIO
 * @param	None
 * @retval	None
 */
void gpio1Disable()
{
	/* If it's PWM we have to deactivate the timer and such */
	if (prvCurrentSettings.direction == GPIODirection_OutputPWM)
		prvDeactivatePwmFunctionality();

	/* Deinit the GPIO */
	HAL_GPIO_DeInit(GPIO1_PORT, GPIO1_PIN);

	/* Set the direction pin as input */
	HAL_GPIO_WritePin(GPIO1_DIRECTION_PORT, GPIO1_DIRECTION_PIN, GPIO_PIN_SET);

	prvIsEnabled = false;
}

/**
 * @brief	Check if the GPIO is enabled or not
 * @param	None
 * @retval	true if it is
 * @retval	false if it is not
 */
bool gpio1IsEnabled()
{
	return prvIsEnabled;
}

/* Private functions .--------------------------------------------------------*/
/**
 * @brief	Initializes the hardware
 * @param	None
 * @retval	None
 */
static void prvHardwareInit()
{
	__GPIOB_CLK_ENABLE();
	__GPIOC_CLK_ENABLE();

	/* Direction pin */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin  	= GPIO1_DIRECTION_PIN;
	GPIO_InitStructure.Mode  	= GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull		= GPIO_NOPULL;
	GPIO_InitStructure.Speed 	= GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIO1_DIRECTION_PORT, &GPIO_InitStructure);

	/* Set as input as default */
	gpio1SetDirection(GPIODirection_Input);
}

/**
 * @brief	Activate the PWM functionality for the channel
 * @param	None
 * @retval	None
 */
static void prvActivatePwmFunctionality()
{
//	TIM_HandleTypeDef htim8;
//	TIM_OC_InitTypeDef sConfigOC;
//	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;
//	TIM_MasterConfigTypeDef sMasterConfig;
//
//	htim8.Instance = TIM8;
//	htim8.Init.Prescaler = 2;
//	htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
//	htim8.Init.Period = 255;
//	htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
//	htim8.Init.RepetitionCounter = 0;
//	HAL_TIM_PWM_Init(&htim8);
//
//	sConfigOC.OCMode = TIM_OCMODE_PWM1;
//	sConfigOC.Pulse = 126;
//	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
//	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
//	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
//	HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_3);
//
//	sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
//	sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
//	sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
//	sBreakDeadTimeConfig.DeadTime = 0;
//	sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
//	sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
//	sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
//	HAL_TIMEx_ConfigBreakDeadTime(&htim8, &sBreakDeadTimeConfig);
//
//	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
//	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
//	HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig);


	/* Enable TIMER Clock */
	PWM_TIMER_CLK_ENABLE;

	/* Configure the GPIO as alternate function */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin  		= GPIO1_PIN;
	GPIO_InitStructure.Mode  		= GPIO_MODE_AF_PP;
	GPIO_InitStructure.Alternate	= PWM_AF_GPIO;
	GPIO_InitStructure.Pull			= GPIO_NOPULL;
	GPIO_InitStructure.Speed 		= GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIO1_PORT, &GPIO_InitStructure);

	/* Timer init */
	TIM_HandleTypeDef timerHandle;
	timerHandle.Instance 			= PWM_TIMER;
	timerHandle.Init.Period			= PWM_PERIOD;
	timerHandle.Init.Prescaler		= PWM_PRESCALER;
	timerHandle.Init.ClockDivision	= TIM_CLOCKDIVISION_DIV1;
	timerHandle.Init.CounterMode	= TIM_COUNTERMODE_UP;
	timerHandle.Init.RepetitionCounter 	= 0;
	HAL_TIM_PWM_Init(&timerHandle);

	/* Output compare init */
	TIM_OC_InitTypeDef timerOutputCompare;
	timerOutputCompare.OCMode 		= TIM_OCMODE_PWM1;
	timerOutputCompare.Pulse		= PWM_PERIOD / 2;
	timerOutputCompare.OCNPolarity	= TIM_OCNPOLARITY_HIGH;
	timerOutputCompare.OCFastMode	= TIM_OCFAST_DISABLE;
	timerOutputCompare.OCNIdleState	= TIM_OCNIDLESTATE_RESET;
	HAL_TIM_PWM_ConfigChannel(&timerHandle, &timerOutputCompare, PWM_TIMER_CHANNEL);

	/* Start the PWM */
	HAL_TIM_PWM_Start(&timerHandle, PWM_TIMER_CHANNEL);
}

/**
 * @brief	Deactivate the PWM functionality for the channel
 * @param	None
 * @retval	None
 */
static void prvDeactivatePwmFunctionality()
{
	HAL_GPIO_DeInit(GPIO1_PORT, GPIO1_PIN);
	TIM_HandleTypeDef timerHandle;
	timerHandle.Instance = PWM_TIMER;
	HAL_TIM_PWM_Stop(&timerHandle, PWM_TIMER_CHANNEL);
}

/* Interrupt Handlers --------------------------------------------------------*/
