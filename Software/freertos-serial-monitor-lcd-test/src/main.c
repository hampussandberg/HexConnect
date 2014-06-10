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

#include "lcd_ra8875.h"

/* Priorities at which the tasks are created. */
#define mainBLINK_TASK_PRIORITY				(tskIDLE_PRIORITY)
#define mainLCD_TASK_PRIORITY				(tskIDLE_PRIORITY + 1)

/* ----- LED definitions --------------------------------------------------- */
/* LEDs on STM32F4 Discovery Board */
#define mainLED_3	GPIO_PIN_13
#define mainLED_4	GPIO_PIN_12
#define mainLED_5	GPIO_PIN_14
#define mainLED_6	GPIO_PIN_15

#define mainLED_7	GPIO_PIN_9

/* ----- GUI definitions --------------------------------------------------- */
//#define GUI_BLUE	LCD_COLOR_BLUE //0x1AD5
//#define GUI_RED		LCD_COLOR_RED //0x9924
//#define GUI_GREEN	LCD_COLOR_GREEN //0x2408
//#define GUI_YELLOW	LCD_COLOR_YELLOW //0x8BE4
//#define GUI_PURPLE	LCD_COLOR_PURPLE //0x6154
//#define GUI_GRAY	LCD_COLOR_WHITE //0x7BEF
//#define GUI_MAGENTA	LCD_COLOR_MAGENTA //0x8951
//#define GUI_CYAN	LCD_COLOR_CYAN //0x13D1

/* DARK */
//#define GUI_BLUE	0x1AD5
//#define GUI_RED		0x9924
//#define GUI_GREEN	0x2408
//#define GUI_YELLOW	0x8BE4
//#define GUI_PURPLE	0x6154
//#define GUI_GRAY	0x7BEF
//#define GUI_MAGENTA	0x8951
//#define GUI_CYAN	0x13D1

#define GUI_BLUE	0x237F
#define GUI_RED		0xF926
#define GUI_GREEN	0x362A
#define GUI_YELLOW	0xFEE6
#define GUI_PURPLE	0xA8D6
#define GUI_GRAY	0xB596
#define GUI_MAGENTA	0xF81F
#define GUI_CYAN	0x07FF


/* ----- Task definitions -------------------------------------------------- */
static void prvBlinkTask(void *pvParameters);
static void prvLcdTask(void *pvParameters);

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
	xTaskCreate(prvBlinkTask,					/* Pointer to the task entry function */
				"Blink",						/* Name for the task */
				configMINIMAL_STACK_SIZE,		/* The size of the stack */
				NULL,							/* Pointer to parameters for the task */
				mainBLINK_TASK_PRIORITY,		/* The priority for the task */
				NULL);							/* Handle for the created task */
#endif
#if 1
	xTaskCreate(prvLcdTask,						/* Pointer to the task entry function */
				"LCD",							/* Name for the task */
				configMINIMAL_STACK_SIZE,		/* The size of the stack */
				NULL,							/* Pointer to parameters for the task */
				mainLCD_TASK_PRIORITY,			/* The priority for the task */
				NULL);							/* Handle for the created task */
#endif

	/* Start the scheduler */
	vTaskStartScheduler();

	/*
	 * If all is well, the scheduler will now be running, and the following line
	 * will never be reached.  If the following line does execute, then there was
	 * insufficient FreeRTOS heap memory available for the idle and/or timer tasks
	 * to be created.  See the memory management section on the FreeRTOS web site
	 * for more details.
	 */
	while (1);
}

/*-----------------------------------------------------------*/
static void prvBlinkTask(void *pvParameters)
{
	/* Set up the LED outputs */
	// Enable clock for GPIOA
	__GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin  	= mainLED_7;
	GPIO_InitStructure.Mode  	= GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull		= GPIO_NOPULL;
	GPIO_InitStructure.Speed 	= GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	HAL_GPIO_WritePin(GPIOA, mainLED_7, GPIO_PIN_RESET);

	/*
	 * The parameter in vTaskDelayUntil is the absolute time
	 * in ticks at which you want to be woken calculated as
	 * an increment from the time you were last woken.
	 */
	TickType_t xNextWakeTime;
	/* Initialize xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	while (1)
	{
		/* LED on for 25 ms */
		HAL_GPIO_WritePin(GPIOA, mainLED_7, GPIO_PIN_SET);
		vTaskDelayUntil(&xNextWakeTime, 25 / portTICK_PERIOD_MS);

		/* LED off for 1000 ms */
		HAL_GPIO_WritePin(GPIOA, mainLED_7, GPIO_PIN_RESET);
		vTaskDelayUntil(&xNextWakeTime, 1000 / portTICK_PERIOD_MS);
	}
}

/*-----------------------------------------------------------*/
void guiTest()
{
	LCD_SetBackgroundColor(LCD_COLOR_BLACK);
	LCD_SetActiveWindow(0, 799, 0, 479);
	LCD_ClearFullWindow();

	/* Square filled */
	LCD_SetForegroundColor(LCD_COLOR_BLACK);
	LCD_DrawSquareOrLine(0, 99, 0, 49, SQUARE, FILLED);
	LCD_SetForegroundColor(GUI_BLUE);
	LCD_SetTextWritePosition(20, 6);
	LCD_WriteString("CAN1", TRANSPARENT, ENLARGE_2X);

	LCD_SetForegroundColor(GUI_RED);
	LCD_DrawSquareOrLine(100, 199, 0, 49, SQUARE, FILLED);
	LCD_SetForegroundColor(LCD_COLOR_WHITE);
	LCD_SetTextWritePosition(120, 6);
	LCD_WriteString("CAN2", TRANSPARENT, ENLARGE_2X);

	LCD_SetForegroundColor(LCD_COLOR_BLACK);
	LCD_DrawSquareOrLine(200, 299, 0, 49, SQUARE, FILLED);
	LCD_SetForegroundColor(GUI_GREEN);
	LCD_SetTextWritePosition(212, 6);
	LCD_WriteString("UART1", TRANSPARENT, ENLARGE_2X);

	LCD_SetForegroundColor(LCD_COLOR_BLACK);
	LCD_DrawSquareOrLine(300, 399, 0, 49, SQUARE, FILLED);
	LCD_SetForegroundColor(GUI_YELLOW);
	LCD_SetTextWritePosition(312, 6);
	LCD_WriteString("UART2", TRANSPARENT, ENLARGE_2X);

	LCD_SetForegroundColor(LCD_COLOR_WHITE);
	LCD_DrawSquareOrLine(400, 499, 0, 49, SQUARE, FILLED);
	LCD_SetForegroundColor(GUI_PURPLE);
	LCD_SetTextWritePosition(412, 6);
	LCD_WriteString("RS232", TRANSPARENT, ENLARGE_2X);

	LCD_SetForegroundColor(LCD_COLOR_BLACK);
	LCD_DrawSquareOrLine(500, 599, 0, 49, SQUARE, FILLED);
	LCD_SetForegroundColor(GUI_GRAY);
	LCD_SetTextWritePosition(524, 6);
	LCD_WriteString("I2C", TRANSPARENT, ENLARGE_2X);

//	LCD_SetForegroundColor(GUI_MAGENTA);
//	LCD_DrawSquareOrLine(600, 649, 0, 24, SQUARE, FILLED);
//	LCD_SetForegroundColor(GUI_CYAN);
//	LCD_DrawSquareOrLine(600, 649, 25, 49, SQUARE, FILLED);

	LCD_SetForegroundColor(LCD_COLOR_WHITE);
	LCD_DrawSquareOrLine(0, 799, 49, 50, SQUARE, FILLED);
	LCD_DrawSquareOrLine(99, 100, 0, 49, SQUARE, FILLED);
	LCD_DrawSquareOrLine(199, 200, 0, 49, SQUARE, FILLED);
	LCD_DrawSquareOrLine(299, 300, 0, 49, SQUARE, FILLED);
	LCD_DrawSquareOrLine(399, 400, 0, 49, SQUARE, FILLED);
	LCD_DrawSquareOrLine(499, 500, 0, 49, SQUARE, FILLED);
	LCD_DrawSquareOrLine(599, 600, 0, 49, SQUARE, FILLED);
	LCD_DrawSquareOrLine(650, 651, 0, 479, SQUARE, FILLED);

	vTaskDelay(1000 / portTICK_PERIOD_MS);
}

static void prvLcdTask(void *pvParameters)
{
	LCD_Init();

	/*
	 * The parameter in vTaskDelayUntil is the absolute time
	 * in ticks at which you want to be woken calculated as
	 * an increment from the time you were last woken.
	 */
	TickType_t xNextWakeTime;
	/* Initialize xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	while (1)
	{
//		LCD_TestBackground(1000);
//		LCD_TestBackgroundFade(50);
//		LCD_TestText(1000);
//		LCD_TestDrawing(100);
//		vTaskDelayUntil(&xNextWakeTime, 4000 / portTICK_PERIOD_MS);
		guiTest();
		vTaskDelayUntil(&xNextWakeTime, 4000 / portTICK_PERIOD_MS);
	}
}

/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook(void)
{
	/*
	 * Called if a call to pvPortMalloc() fails because there is insufficient
	 * free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	 * internally by FreeRTOS API functions that create tasks, queues, software
	 * timers, and semaphores.  The size of the FreeRTOS heap is set by the
	 * configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h.
	 */
	while (1);
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook(xTaskHandle pxTask, signed char *pcTaskName)
{
	(void) pcTaskName;
	(void) pxTask;

	/*
	 * Run time stack overflow checking is performed if
	 * configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	 * function is called if a stack overflow is detected.
	 */
	while (1);
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook(void)
{
	volatile size_t xFreeStackSpace;

	/*
	 * This function is called on each cycle of the idle task.  In this case it
	 * does nothing useful, other than report the amount of FreeRTOS heap that
	 * remains unallocated.
	 */
	xFreeStackSpace = xPortGetFreeHeapSize();

	if (xFreeStackSpace > 100)
	{
		/*
		 * By now, the kernel has allocated everything it is going to, so
		 * if there is a lot of heap remaining unallocated then
		 * the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
		 * reduced accordingly.
		 */
	}
}
