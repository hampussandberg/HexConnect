/**
 ******************************************************************************
 * @file	color.h
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-06-02
 * @brief
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef COLOR_H_
#define COLOR_H_

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "stm32f4xx_hal.h"

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
typedef struct
{
	uint16_t hue;
	float saturation;
	float brightness;
} HSB_TypeDef;

typedef struct
{
	uint8_t red;	/* 5-bits used for red */
	uint8_t green;	/* 6-bits used for green */
	uint8_t blue;	/* 5-bits used for blue */
} RGB565_TypeDef;

/* Function prototypes -------------------------------------------------------*/
void COLOR_HSBtoRGB565(HSB_TypeDef* HSB, RGB565_TypeDef* RGB);

#endif /* COLOR_H_ */
