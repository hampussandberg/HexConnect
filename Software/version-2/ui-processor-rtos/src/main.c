/**
 *******************************************************************************
 * @file  main.c
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

#include <stdio.h>
#include <stdlib.h>

/** Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/** STM32 Library includes. */
#include "stm32f4xx_hal.h"

/** Tasks */
#include "background_task.h"
#include "lcd_task.h"
#include "main_task.h"


/** Priorities at which the tasks are created. */
#define mainBACKGROUND_TASK_PRIORITY    (tskIDLE_PRIORITY)
#define mainLCD_TASK_PRIORITY           (tskIDLE_PRIORITY + 1)
#define mainMAIN_TASK_PRIORITY          (tskIDLE_PRIORITY + 2)

/** ----- Main -------------------------------------------------------------- */
int main()
{
  /*
  * The NVIC priority group is set to NVIC_PRIORITYGROUP_4 in HAL_Init
  * called in _initialize_hardware.c
  * At this point everything is ready to go!
  */

  /* Create the tasks */
#if 1
   xTaskCreate(backgroundTask,              /* Pointer to the task entry function */
              "Background",                 /* Name for the task */
              configMINIMAL_STACK_SIZE,     /* The size of the stack */
              NULL,                         /* Pointer to parameters for the task */
              mainBACKGROUND_TASK_PRIORITY, /* The priority for the task */
              NULL);                        /* Handle for the created task */
#endif

#if 1
  xTaskCreate(lcdTask,                      /* Pointer to the task entry function */
              "LCD",                        /* Name for the task */
              configMINIMAL_STACK_SIZE*2,     /* The size of the stack */
              NULL,                         /* Pointer to parameters for the task */
              mainLCD_TASK_PRIORITY,        /* The priority for the task */
              NULL);                        /* Handle for the created task */
#endif

#if 1
  xTaskCreate(mainTask,                      /* Pointer to the task entry function */
              "Main",                        /* Name for the task */
              configMINIMAL_STACK_SIZE,     /* The size of the stack */
              NULL,                         /* Pointer to parameters for the task */
              mainMAIN_TASK_PRIORITY,        /* The priority for the task */
              NULL);                        /* Handle for the created task */
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

/** ----- Debug ------------------------------------------------------------- */
void vApplicationMallocFailedHook(void)
{
  /* Called if a call to pvPortMalloc() fails because there is insufficient
  free memory available in the FreeRTOS heap.  pvPortMalloc() is called
  internally by FreeRTOS API functions that create tasks, queues, software
  timers, and semaphores.  The size of the FreeRTOS heap is set by the
  configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
  while (1);
}

void vApplicationStackOverflowHook(xTaskHandle pxTask, signed char *pcTaskName)
{
  (void) pcTaskName;
  (void) pxTask;

  /* Run time stack overflow checking is performed if
  configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
  function is called if a stack overflow is detected. */
  while (1);
}

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
