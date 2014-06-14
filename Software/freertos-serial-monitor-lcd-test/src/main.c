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
#include "simple_gui.h"

#include "test_image.c"
#include "golden_gate_bridge_image.c"

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
#define GUI_DARK_BLUE	0x11CE


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
void guiTestInit()
{
	LCD_SetBackgroundColor(LCD_COLOR_BLACK);
	LCD_SetActiveWindow(0, 799, 0, 479);
	LCD_ClearFullWindow();


	GUIButton_TypeDef button;
	button.disabledBackgroundColor = LCD_COLOR_BLACK;
	button.pressedBackgroundColor = LCD_COLOR_WHITE;
	button.enabledTextColor = LCD_COLOR_WHITE;
	button.state = DISABLED;

	button.yPos = 0;
	button.height = 49;

	/* CAN1 Button */
	button.enabledBackgroundColor = button.disabledTextColor = button.pressedTextColor = GUI_BLUE;
	button.text = "CAN1";
	button.textSize = ENLARGE_2X;
	button.xPos = 0;
	button.width = 99;
	GUI_AddButton(&button);

	/* CAN2 Button */
	button.enabledBackgroundColor = button.disabledTextColor = button.pressedTextColor = GUI_RED;
	button.text = "CAN2";
	button.textSize = ENLARGE_2X;
	button.xPos = 101;
	button.width = 98;
	GUI_AddButton(&button);

	/* UART1 Button */
	button.enabledBackgroundColor = button.disabledTextColor = button.pressedTextColor = GUI_GREEN;
	button.text = "UART1";
	button.textSize = ENLARGE_2X;
	button.xPos = 201;
	GUI_AddButton(&button);

	/* UART2 Button */
	button.enabledBackgroundColor = button.disabledTextColor = button.pressedTextColor = GUI_YELLOW;
	button.text = "UART2";
	button.textSize = ENLARGE_2X;
	button.xPos = 301;
	GUI_AddButton(&button);

	/* RS232 Button */
	button.enabledBackgroundColor = button.disabledTextColor = button.pressedTextColor = GUI_PURPLE;
	button.text = "RS232";
	button.textSize = ENLARGE_2X;
	button.xPos = 401;
	GUI_AddButton(&button);

	/* I2C Button */
	button.enabledBackgroundColor = button.disabledTextColor = button.pressedTextColor = GUI_GRAY;
	button.text = "I2C";
	button.textSize = ENLARGE_2X;
	button.xPos = 501;
	GUI_AddButton(&button);

	/* ADC Button */
	button.enabledBackgroundColor = button.disabledTextColor = button.pressedTextColor = GUI_MAGENTA;
	button.text = "ADC";
	button.textSize = ENLARGE_1X;
	button.xPos = 601;
	button.yPos = 0;
	button.width = 49;
	button.height = 25;
	GUI_AddButton(&button);

	/* GPIO Button */
	button.enabledBackgroundColor = button.disabledTextColor = button.pressedTextColor = GUI_CYAN;
	button.text = "GPIO";
	button.textSize = ENLARGE_1X;
	button.yPos = 25;
	button.height = 24;
	GUI_AddButton(&button);

	/* SETTINGS Button */
	button.disabledBackgroundColor = button.enabledBackgroundColor = button.pressedTextColor = GUI_DARK_BLUE;
	button.disabledTextColor = LCD_COLOR_WHITE;
	button.text = "SETTINGS";
	button.textSize = ENLARGE_2X;
	button.xPos = 652;
	button.yPos = 430;
	button.width = 148;
	button.height = 50;
	GUI_AddButton(&button);

	GUI_DrawAllButtons();

	LCD_SetForegroundColor(LCD_COLOR_WHITE);
	LCD_SetTextWritePosition(740, 3);
	LCD_WriteString("18:38", TRANSPARENT, ENLARGE_1X);
	LCD_SetTextWritePosition(740, 28);
	LCD_WriteString("22 C", TRANSPARENT, ENLARGE_1X);

	LCD_SetForegroundColor(LCD_COLOR_WHITE);
	LCD_DrawSquareOrLine(0, 799, 49, 50, SQUARE, FILLED);
	LCD_DrawSquareOrLine(99, 100, 0, 49, SQUARE, FILLED);
	LCD_DrawSquareOrLine(199, 200, 0, 49, SQUARE, FILLED);
	LCD_DrawSquareOrLine(299, 300, 0, 49, SQUARE, FILLED);
	LCD_DrawSquareOrLine(399, 400, 0, 49, SQUARE, FILLED);
	LCD_DrawSquareOrLine(499, 500, 0, 49, SQUARE, FILLED);
	LCD_DrawSquareOrLine(599, 600, 0, 49, SQUARE, FILLED);
	LCD_DrawSquareOrLine(650, 651, 0, 479, SQUARE, FILLED);
}

void guiTest()
{
	static uint32_t index = 0;
	static uint32_t state = ENABLED;

	GUI_SetButtonState(index, state);

	index++;
	if (index >= guiConfigNUMBER_OF_BUTTONS)
	{
		index = 0;
		state++;
		if (state > PRESSED)
			state = 0;
	}

	vTaskDelay(1000 / portTICK_PERIOD_MS);
}

const LCD_Image_TypeDef testImage = {image_data_test, 30, 29};
const LCD_Image_TypeDef goldengatebridge = {image_data_goldengatebridge, 800, 480};

static void prvLcdTask(void *pvParameters)
{
	LCD_Init();

	guiTestInit();

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
//		LCD_TestBTE(&testImage, 30, 30);
//		LCD_TestBTE(&goldengatebridge, 0, 0);

//		vTaskDelayUntil(&xNextWakeTime, 4000 / portTICK_PERIOD_MS);
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
