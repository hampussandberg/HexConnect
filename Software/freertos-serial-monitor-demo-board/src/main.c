/*
 * FreeRTOS Coding Standard and Style Guide:
 * http://www.freertos.org/FreeRTOS-Coding-Standard-and-Style-Guide.html
 */

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* STM32 Library includes. */
#include "stm32f4xx_hal.h"

#include <stdio.h>

#include "background_task.h"
#include "lcd_task.h"
#include "can1_task.h"
#include "can2_task.h"
#include "uart1_task.h"
#include "uart2_task.h"
#include "rs232_task.h"
#include "gpio0_task.h"
#include "gpio1_task.h"
#include "adc_task.h"

/* Priorities at which the tasks are created. */
#define mainBACKGROUND_TASK_PRIORITY		(tskIDLE_PRIORITY)
#define mainLCD_TASK_PRIORITY				(tskIDLE_PRIORITY + 1)
#define mainCAN1_TASK_PRIORITY				(tskIDLE_PRIORITY + 2)
#define mainCAN2_TASK_PRIORITY				(tskIDLE_PRIORITY + 2)
#define mainUART1_TASK_PRIORITY				(tskIDLE_PRIORITY + 2)
#define mainUART2_TASK_PRIORITY				(tskIDLE_PRIORITY + 2)
#define mainRS232_TASK_PRIORITY				(tskIDLE_PRIORITY + 2)
#define mainGPIO0_TASK_PRIORITY				(tskIDLE_PRIORITY + 2)
#define mainGPIO1_TASK_PRIORITY				(tskIDLE_PRIORITY + 2)
#define mainADC_TASK_PRIORITY				(tskIDLE_PRIORITY + 2)

/* ----- Main -------------------------------------------------------------- */
int main(int argc, char* argv[])
{
	/*
	 * The NVIC priority group is set to NVIC_PRIORITYGROUP_4 in HAL_Init
	 * called in _initialize_hardware.c
	 * At this point everything is ready to go!
	 */

	/* Create the tasks */
#if 1
	xTaskCreate(backgroundTask,					/* Pointer to the task entry function */
				"Background",					/* Name for the task */
				configMINIMAL_STACK_SIZE,		/* The size of the stack */
				NULL,							/* Pointer to parameters for the task */
				mainBACKGROUND_TASK_PRIORITY,	/* The priority for the task */
				NULL);							/* Handle for the created task */
#endif
#if 0
	xTaskCreate(lcdTask,						/* Pointer to the task entry function */
				"LCD",							/* Name for the task */
				configMINIMAL_STACK_SIZE * 2,	/* The size of the stack */
				NULL,							/* Pointer to parameters for the task */
				mainLCD_TASK_PRIORITY,			/* The priority for the task */
				NULL);							/* Handle for the created task */
#endif
#if 1
	xTaskCreate(can1Task,						/* Pointer to the task entry function */
				"CAN1",							/* Name for the task */
				configMINIMAL_STACK_SIZE,		/* The size of the stack */
				NULL,							/* Pointer to parameters for the task */
				mainCAN1_TASK_PRIORITY,			/* The priority for the task */
				NULL);							/* Handle for the created task */
#endif
#if 1
	xTaskCreate(can2Task,						/* Pointer to the task entry function */
				"CAN2",							/* Name for the task */
				configMINIMAL_STACK_SIZE,		/* The size of the stack */
				NULL,							/* Pointer to parameters for the task */
				mainCAN2_TASK_PRIORITY,			/* The priority for the task */
				NULL);							/* Handle for the created task */
#endif
#if 1
	xTaskCreate(uart1Task,						/* Pointer to the task entry function */
				"UART1",						/* Name for the task */
				configMINIMAL_STACK_SIZE,		/* The size of the stack */
				NULL,							/* Pointer to parameters for the task */
				mainUART1_TASK_PRIORITY,		/* The priority for the task */
				NULL);							/* Handle for the created task */
#endif
#if 1
	xTaskCreate(uart2Task,						/* Pointer to the task entry function */
				"UART2",						/* Name for the task */
				configMINIMAL_STACK_SIZE,		/* The size of the stack */
				NULL,							/* Pointer to parameters for the task */
				mainUART2_TASK_PRIORITY,		/* The priority for the task */
				NULL);							/* Handle for the created task */
#endif
#if 1
	xTaskCreate(rs232Task,						/* Pointer to the task entry function */
				"RS232",						/* Name for the task */
				configMINIMAL_STACK_SIZE,		/* The size of the stack */
				NULL,							/* Pointer to parameters for the task */
				mainRS232_TASK_PRIORITY,		/* The priority for the task */
				NULL);							/* Handle for the created task */
#endif
#if 1
	xTaskCreate(gpio0Task,						/* Pointer to the task entry function */
				"GPIO0",						/* Name for the task */
				configMINIMAL_STACK_SIZE,		/* The size of the stack */
				NULL,							/* Pointer to parameters for the task */
				mainGPIO0_TASK_PRIORITY,		/* The priority for the task */
				NULL);							/* Handle for the created task */
#endif
#if 1
	xTaskCreate(gpio1Task,						/* Pointer to the task entry function */
				"GPIO1",						/* Name for the task */
				configMINIMAL_STACK_SIZE,		/* The size of the stack */
				NULL,							/* Pointer to parameters for the task */
				mainGPIO1_TASK_PRIORITY,		/* The priority for the task */
				NULL);							/* Handle for the created task */
#endif
#if 1
	xTaskCreate(adcTask,						/* Pointer to the task entry function */
				"ADC",							/* Name for the task */
				configMINIMAL_STACK_SIZE,		/* The size of the stack */
				NULL,							/* Pointer to parameters for the task */
				mainADC_TASK_PRIORITY,			/* The priority for the task */
				NULL);							/* Handle for the created task */
#endif

	/* Start the scheduler */
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following line
	will never be reached.  If the following line does execute, then there was
	insufficient FreeRTOS heap memory available for the idle and/or timer tasks
	to be created.  See the memory management section on the FreeRTOS web site
	for more details. */
	while (1);
}

/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook(void)
{
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
	while (1);
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook(xTaskHandle pxTask, signed char *pcTaskName)
{
	(void) pcTaskName;
	(void) pxTask;

	/* Run time stack overflow checking is performed if
	configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	while (1);
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook(void)
{
	volatile size_t xFreeStackSpace;

	/* This function is called on each cycle of the idle task.  In this case it
	does nothing useful, other than report the amount of FreeRTOS heap that
	remains unallocated. */
	xFreeStackSpace = xPortGetFreeHeapSize();

	if (xFreeStackSpace > 100)
	{
		/* By now, the kernel has allocated everything it is going to, so
		if there is a lot of heap remaining unallocated then
		the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
		reduced accordingly. */
	}
}
