/**
 ******************************************************************************
 * @file	millis.h
 * @author	Hampus Sandberg
 * @version	0.1
 * @date  2015-08-15
 * @brief	Manage a millisecond counter
 ******************************************************************************
  Copyright (c) 2015 Hampus Sandberg.

  TODO: License
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MILLIS_H_
#define MILLIS_H_

/* Includes ------------------------------------------------------------------*/
#if defined(STM32F40_41xxx)
#include "stm32f4xx.h"
#elif defined(STM32F10X_MD) || defined(STM32F10X_MD_VL)
#include "stm32f10x.h"
#endif

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/
void MILLIS_Init(void);
uint32_t millis(void);
void millisDelay(uint32_t);

#endif /* MILLIS_H_ */
