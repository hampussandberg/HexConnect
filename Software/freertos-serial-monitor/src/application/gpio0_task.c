/**
 ******************************************************************************
 * @file	gpio0_task.c
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
#include "gpio0_task.h"

#include "stm32f4xx_hal_tim.h"

/* Private defines -----------------------------------------------------------*/
#define GPIO0_PORT				(GPIOB)
#define GPIO0_PIN				(GPIO_PIN_0)
#define GPIO0_DIRECTION_PORT	(GPIOC)
#define GPIO0_DIRECTION_PIN		(GPIO_PIN_4)

#define PWM_TIMER				(TIM3)
#define PWM_TIMER_CLK_ENABLE	(__TIM3_CLK_ENABLE())
#define PWM_AF_GPIO				(GPIO_AF2_TIM3)
#define PWM_TIMER_CHANNEL		(TIM_CHANNEL_3)
#define PWM_TIMER_CLOCK			(42000000)	/* 42 MHz, see datasheet page 31 */
#define PWM_PERIOD				(255)		/* 256 step PWM */
#define PWM_PRESCALER			(6)			/* Divide by 7 */
#define PWM_FREQ				(PWM_TIMER_CLOCK/((PWM_PRESCALER+1) * (PWM_PERIOD+1)))	/* Is not valid in PWM mode */

/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void prvHardwareInit();
static void prvActivatePwmFunctionality();
static void prvDeactivatePwmFunctionality();

/* Functions -----------------------------------------------------------------*/
/**
 * @brief	The main task for the GPIO0 channel
 * @param	pvParameters:
 * @retval	None
 */
void gpio0Task(void *pvParameters)
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
	}
}

/**
 * @brief	Sets the direction of the GPIO
 * @param	Direction: The direction to use, can be any value of GPIO0Direction
 * @retval	None
 */
void gpio0SetDirection(GPIO0Direction Direction)
{
	/* Reset GPIO first */
	HAL_GPIO_DeInit(GPIO0_PORT, GPIO0_PIN);

	/* If it's PWM we have to activate the timer and such */
	if (Direction == GPIO0Direction_OutputPWM)
	{
		prvActivatePwmFunctionality();
	}
	else
	{
		/* Deactivate the PWM in case it was active */
		prvDeactivatePwmFunctionality();

		/* Init with new direction */
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_InitStructure.Pin  	= GPIO0_PIN;
		GPIO_InitStructure.Pull		= GPIO_NOPULL;
		GPIO_InitStructure.Speed 	= GPIO_SPEED_HIGH;
		if (Direction == GPIO0Direction_Output)
			GPIO_InitStructure.Mode  	= GPIO_MODE_OUTPUT_PP;
		else
			GPIO_InitStructure.Mode  	= GPIO_MODE_INPUT;
		HAL_GPIO_Init(GPIO0_PORT, &GPIO_InitStructure);
	}

	/* Set the direction pin */
	if (Direction == GPIO0Direction_Input)
		HAL_GPIO_WritePin(GPIO0_DIRECTION_PORT, GPIO0_DIRECTION_PIN, GPIO_PIN_SET);
	else if (Direction == GPIO0Direction_Output || GPIO0Direction_OutputPWM)
		HAL_GPIO_WritePin(GPIO0_DIRECTION_PORT, GPIO0_DIRECTION_PIN, GPIO_PIN_RESET);
}

/**
 * @brief	Read the value of the pin
 * @param	None
 * @retval	None
 */
GPIO_PinState gpio0ReadPin()
{
	return HAL_GPIO_ReadPin(GPIO0_PORT, GPIO0_PIN);
}

/**
 * @brief	Write a value to the pin
 * @param	None
 * @retval	None
 */
void gpio0WritePin(GPIO_PinState PinState)
{
	/* TODO: Make sure it's set as an output, otherwise return error */
	HAL_GPIO_WritePin(GPIO0_PORT, GPIO0_PIN, PinState);
}

/**
 * @brief	Toggle the value of the pin
 * @param	None
 * @retval	None
 */
void gpio0TogglePin()
{
	/* TODO: Make sure it's set as an output, otherwise return error */
	HAL_GPIO_TogglePin(GPIO0_PORT, GPIO0_PIN);
}

/**
 * @brief	Set the duty of the PWM
 * @param	Duty: The duty as a percentage (0.0 - 100.0%)
 * @retval	None
 */
void gpio0SetPwmDuty(float Duty)
{
	if (Duty >= 0.0 && Duty <= 100.0)
	{
		PWM_TIMER->CCR3 = (uint16_t)(Duty/100.0 * PWM_PERIOD);
	}
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
	GPIO_InitStructure.Pin  	= GPIO0_DIRECTION_PIN;
	GPIO_InitStructure.Mode  	= GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull		= GPIO_NOPULL;
	GPIO_InitStructure.Speed 	= GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIO0_DIRECTION_PORT, &GPIO_InitStructure);

	/* Set as input as default */
	gpio0SetDirection(GPIO0Direction_Input);
}

/**
 * @brief	Activate the PWM functionality for the channel
 * @param	None
 * @retval	None
 */
static void prvActivatePwmFunctionality()
{
	/* Enable TIMER Clock */
	PWM_TIMER_CLK_ENABLE;

	/* Configure the GPIO as alternate function */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin  		= GPIO0_PIN;
	GPIO_InitStructure.Mode  		= GPIO_MODE_AF_PP;
	GPIO_InitStructure.Alternate	= PWM_AF_GPIO;
	GPIO_InitStructure.Pull			= GPIO_NOPULL;
	GPIO_InitStructure.Speed 		= GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIO0_PORT, &GPIO_InitStructure);

	/* Timer init */
	TIM_HandleTypeDef timerHandle;
	timerHandle.Instance 			= PWM_TIMER;
	timerHandle.Init.Period			= PWM_PERIOD;
	timerHandle.Init.Prescaler		= PWM_PRESCALER;
	timerHandle.Init.ClockDivision	= TIM_CLOCKDIVISION_DIV1;
	timerHandle.Init.CounterMode	= TIM_COUNTERMODE_UP;
	HAL_TIM_PWM_Init(&timerHandle);

	/* Output compare init */
	TIM_OC_InitTypeDef timerOutputCompare;
	timerOutputCompare.OCMode 		= TIM_OCMODE_PWM1;
	timerOutputCompare.Pulse		= PWM_PERIOD / 2;
	timerOutputCompare.OCPolarity	= TIM_OCPOLARITY_HIGH;
	timerOutputCompare.OCNPolarity	= TIM_OCNPOLARITY_HIGH;
	timerOutputCompare.OCFastMode	= TIM_OCFAST_DISABLE;
	timerOutputCompare.OCIdleState	= TIM_OCIDLESTATE_SET;
	timerOutputCompare.OCNIdleState	= TIM_OCNIDLESTATE_SET;
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
	TIM_HandleTypeDef timerHandle;
	timerHandle.Instance = PWM_TIMER;
	HAL_TIM_PWM_Stop(&timerHandle, PWM_TIMER_CHANNEL);
}

/* Interrupt Handlers --------------------------------------------------------*/
