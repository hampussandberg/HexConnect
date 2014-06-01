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

/* ----- LCD definitions --------------------------------------------------- */
__IO uint16_t *LCD_REG = 0x60080000;
__IO uint16_t *LCD_RAM = 0x60000000;

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
void LCD_GPIOConfig()
{
	/* Enable clock for GPIOD, GPIOE and FSMC */
	__GPIOD_CLK_ENABLE();
	__GPIOE_CLK_ENABLE();
	__FSMC_CLK_ENABLE();

	/* The pins should be set to maximum speed and as alternate-function */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Mode 		= GPIO_MODE_AF_PP;
	GPIO_InitStructure.Speed 		= GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull  		= GPIO_NOPULL;
	GPIO_InitStructure.Alternate 	= GPIO_AF12_FSMC;


	/* PD7: FSMC_NE1 -> CS */
	GPIO_InitStructure.Pin 	= GPIO_PIN_7;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
	/* PD4: FSMC_NOE -> RD */
	GPIO_InitStructure.Pin 	= GPIO_PIN_4;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
	/* PD5: FSMC_NWE -> RW */
	GPIO_InitStructure.Pin 	= GPIO_PIN_5;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
	/* PD13: FSMC_A18 -> RS */
	GPIO_InitStructure.Pin 	= GPIO_PIN_13;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);


	/* PD14: FSMC_D0 */
	GPIO_InitStructure.Pin 	= GPIO_PIN_14;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
	/* PD15: FSMC_D1 */
	GPIO_InitStructure.Pin 	= GPIO_PIN_15;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
	/* PD0: FSMC_D2 */
	GPIO_InitStructure.Pin 	= GPIO_PIN_0;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
	/* PD1: FSMC_D3 */
	GPIO_InitStructure.Pin 	= GPIO_PIN_1;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
	/* PE7: FSMC_D4 */
	GPIO_InitStructure.Pin 	= GPIO_PIN_7;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);
	/* PE8: FSMC_D5 */
	GPIO_InitStructure.Pin 	= GPIO_PIN_8;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);
	/* PE9: FSMC_D6 */
	GPIO_InitStructure.Pin 	= GPIO_PIN_9;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);
	/* PE10: FSMC_D7 */
	GPIO_InitStructure.Pin 	= GPIO_PIN_10;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);
	/* PE11: FSMC_D8 */
	GPIO_InitStructure.Pin 	= GPIO_PIN_11;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);
	/* PE12: FSMC_D9 */
	GPIO_InitStructure.Pin 	= GPIO_PIN_12;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);
	/* PE13: FSMC_D10 */
	GPIO_InitStructure.Pin 	= GPIO_PIN_13;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);
	/* PE14: FSMC_D11 */
	GPIO_InitStructure.Pin 	= GPIO_PIN_14;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);
	/* PE15: FSMC_D12 */
	GPIO_InitStructure.Pin 	= GPIO_PIN_15;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);
	/* PD8: FSMC_D13 */
	GPIO_InitStructure.Pin 	= GPIO_PIN_13;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
	/* PD9: FSMC_D14 */
	GPIO_InitStructure.Pin 	= GPIO_PIN_9;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
	/* PD10: FSMC_D15 */
	GPIO_InitStructure.Pin 	= GPIO_PIN_10;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
}

void LCD_FSMCConfig()
{
	/*-- FSMC Configuration ---------------------------------*/
	/* The FSMC NOR Flash/SRAM bank is suitable for MCU parallel color LCD interfaces */
	FSMC_NORSRAM_InitTypeDef  FSMC_NORSRAM_InitStructure;
	FSMC_NORSRAM_TimingTypeDef FSMC_NORSRAM_TimingInitStructure;

	/* FSMC_NORSRAM_BANK1 timing configuration, see reference manual rev 7 p. 1528 for min/max values */
	FSMC_NORSRAM_TimingInitStructure.AddressSetupTime 		= 4;
	FSMC_NORSRAM_TimingInitStructure.AddressHoldTime 		= 1;
	FSMC_NORSRAM_TimingInitStructure.DataSetupTime 			= 3;
	FSMC_NORSRAM_TimingInitStructure.BusTurnAroundDuration 	= 0;
	FSMC_NORSRAM_TimingInitStructure.CLKDivision 			= 2;
	FSMC_NORSRAM_TimingInitStructure.DataLatency 			= 2;
	FSMC_NORSRAM_TimingInitStructure.AccessMode 			= FSMC_ACCESS_MODE_B;

	/* FSMC_NORSRAM_BANK1 configured as follows:
	* - Data/Address MUX = Disable
	* - Memory Type = SRAM
	* - Data Width = 16bit
	* - Write Operation = Enable
	* - Asynchronous Wait = Disable
	* - Extended Mode = Disable
	*/
	FSMC_NORSRAM_InitStructure.NSBank 						= FSMC_NORSRAM_BANK1;
	FSMC_NORSRAM_InitStructure.DataAddressMux 				= FSMC_DATA_ADDRESS_MUX_DISABLE;
	FSMC_NORSRAM_InitStructure.MemoryType 					= FSMC_MEMORY_TYPE_SRAM;			// NOR???
	FSMC_NORSRAM_InitStructure.MemoryDataWidth 				= FSMC_NORSRAM_MEM_BUS_WIDTH_16;
	FSMC_NORSRAM_InitStructure.BurstAccessMode 				= FSMC_BURST_ACCESS_MODE_DISABLE;
	FSMC_NORSRAM_InitStructure.WaitSignalPolarity 			= FSMC_WAIT_SIGNAL_POLARITY_LOW;
	FSMC_NORSRAM_InitStructure.WrapMode 					= FSMC_WRAP_MODE_DISABLE;
	FSMC_NORSRAM_InitStructure.WaitSignalActive 			= FSMC_WAIT_TIMING_BEFORE_WS;
	FSMC_NORSRAM_InitStructure.WriteOperation 				= FSMC_WRITE_OPERATION_ENABLE;
	FSMC_NORSRAM_InitStructure.WaitSignal 					= FSMC_WAIT_SIGNAL_DISABLE;			// ENABLE, LCD_WAIT???
	FSMC_NORSRAM_InitStructure.AsynchronousWait 			= FSMC_ASYNCHRONOUS_WAIT_DISABLE;
	FSMC_NORSRAM_InitStructure.ExtendedMode 				= FSMC_EXTENDED_MODE_DISABLE;
	FSMC_NORSRAM_InitStructure.WriteBurst 					= FSMC_WRITE_BURST_DISABLE;

	/* FSMC NORSRAM bank control configuration */
	FSMC_NORSRAM_Init(FSMC_NORSRAM_DEVICE, &FSMC_NORSRAM_InitStructure);
	/* FSMC NORSRAM bank timing configuration */
	FSMC_NORSRAM_Timing_Init(FSMC_NORSRAM_DEVICE, &FSMC_NORSRAM_TimingInitStructure, 1);
}


static void prvLcdTask(void *pvParameters)
{
	LCD_GPIOConfig();
	LCD_FSMCConfig();

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
		vTaskDelayUntil(&xNextWakeTime, 25 / portTICK_PERIOD_MS);
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

#ifdef DEBUG
/*
 * The fault handler implementation calls a function called
 * prvGetRegistersFromStack().
 */
void HardFault_Handler(void)
{
    __asm volatile
    (
        " tst lr, #4                                                \n"
        " ite eq                                                    \n"
        " mrseq r0, msp                                             \n"
        " mrsne r0, psp                                             \n"
        " ldr r1, [r0, #24]                                         \n"
        " ldr r2, handler2_address_const                            \n"
        " bx r2                                                     \n"
        " handler2_address_const: .word prvGetRegistersFromStack    \n"
    );
}

void prvGetRegistersFromStack(uint32_t *pulFaultStackAddress)
{
	/*
	 * These are volatile to try and prevent the compiler/linker optimising them
	 * away as the variables never actually get used.  If the debugger won't show the
	 * values of the variables, make them global my moving their declaration outside
	 * of this function.
	 */
	volatile uint32_t r0;
	volatile uint32_t r1;
	volatile uint32_t r2;
	volatile uint32_t r3;
	volatile uint32_t r12;
	volatile uint32_t lr; /* Link register. */
	volatile uint32_t pc; /* Program counter. */
	volatile uint32_t psr;/* Program status register. */

    r0 = pulFaultStackAddress[0];
    r1 = pulFaultStackAddress[1];
    r2 = pulFaultStackAddress[2];
    r3 = pulFaultStackAddress[3];

    r12 = pulFaultStackAddress[4];
    lr = pulFaultStackAddress[5];
    pc = pulFaultStackAddress[6];
    psr = pulFaultStackAddress[7];

    /* When the following line is hit, the variables contain the register values. */
    while (1);

    /* These lines help prevent getting warnings from compiler about unused variables */
    r0 = r1 = r2 = r3 = r12 = lr = pc = psr = 0;
    r0++;
}

#endif // #ifdef DEBUG
