/**
 *******************************************************************************
 * @file    lcd.h
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-08-25
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

/** Define to prevent recursive inclusion ------------------------------------*/
#ifndef LCD_H_
#define LCD_H_

/** Includes -----------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "semphr.h"

#include "fonts.h"
#include "images.h"

/** Defines ------------------------------------------------------------------*/
/*
 * Uncomment the line below to use dual buffer mode.
 * In this mode there are two buffers that the lcd driver switches between.
 * While one buffer is used by the lcd to display the current content, the other
 * buffer is used to draw new content. When the new content should be displayed
 * the driver changes the address where the lcd is getting it's data from to the
 * new buffer. The old buffer is then cleaned and new content can be added to it
 */
//#define DUAL_BUFFER_MODE

/* LCD Layer */
#define LCD_BACKGROUND_LAYER     0x0000
#define LCD_FOREGROUND_LAYER     0x0001

/* LCD Direction */
#define LCD_DIR_HORIZONTAL       0x0000
#define LCD_DIR_VERTICAL         0x0001

/* LCD Size (Width and Height) */
#define YL070MY01
//#define ER_TFT070_4

#if defined(ER_TFT070_4)
#define LCD_PIXEL_WIDTH   (800)
#define LCD_PIXEL_HEIGHT  (480)
#define LCD_PIXELS        (LCD_PIXEL_WIDTH*LCD_PIXEL_HEIGHT)
#define LCD_HSYNC_WIDTH   (48)
#define LCD_VSYNC_WIDTH   (3)
#define LCD_HBACKPORCH    (88)
#define LCD_VBACKPORCH    (32)
#define LCD_HFRONTPORCH   (40)
#define LCD_VFRONTPORCH   (13)
#endif
#if defined(YL070MY01)
#define LCD_PIXEL_WIDTH   (800)
#define LCD_PIXEL_HEIGHT  (480)
#define LCD_PIXELS        (LCD_PIXEL_WIDTH*LCD_PIXEL_HEIGHT)
#define LCD_HSYNC_WIDTH   (30)
#define LCD_VSYNC_WIDTH   (13)
#define LCD_HBACKPORCH    (16)
#define LCD_VBACKPORCH    (10)
#define LCD_HFRONTPORCH   (210)
#define LCD_VFRONTPORCH   (22)
#endif

/** Typedefs -----------------------------------------------------------------*/
typedef enum
{
  LCD_DrawDirection_Horizontal,
  LCD_DrawDirection_Vertical,
} LCD_DrawDirection;

typedef enum
{
  LCD_LAYER_1 = 0,
  LCD_LAYER_2,
  LCD_LAYER_3,

  LCD_LAYER_NUM_OF_LAYERS,
} LCD_LAYER;
#define IS_VALID_LAYER(X)  (X == LCD_LAYER_1 || X == LCD_LAYER_2 || X == LCD_LAYER_3)

/** Global variables ---------------------------------------------------------*/
LTDC_HandleTypeDef LTDCHandle;
DMA2D_HandleTypeDef DMA2DHandle;

/** Function prototypes ------------------------------------------------------*/
void LCD_Init();
void LCD_LayerInit();

void LCD_RefreshActiveDisplay();
void LCD_DrawLayerToBuffer(LCD_LAYER Layer);
void LCD_DrawPartOfLayerToBuffer(LCD_LAYER Layer, uint16_t XPos, uint16_t YPos, uint16_t Width, uint16_t Height);
void LCD_ClearScreenBuffer(uint16_t Color);
void LCD_ClearBuffer(uint32_t Color, uint16_t Width, uint16_t Height, uint32_t BufferStartAddress);
void LCD_ClearLayer(uint32_t Color, LCD_LAYER Layer);

#if defined(DUAL_BUFFER_MODE)
void LCD_SetBufferAsActiveScreen();
void HAL_LTDC_LineEvenCallback(LTDC_HandleTypeDef *hltdc);
#endif

void LCD_DrawCharacterOnLayer(uint32_t Color, uint16_t XPos, uint16_t YPos, char Character, FONT* Font, LCD_LAYER Layer);
void LCD_DrawCharacterOnBuffer(uint32_t Color, uint16_t XPos, uint16_t YPos, char Character, FONT* Font, uint32_t BufferStartAddress);
void LCD_DrawStringOnLayer(uint32_t Color, uint16_t XPos, uint16_t YPos, char* String, FONT* Font, LCD_LAYER Layer);

void LCD_DrawPixelOnLayer(uint32_t Color, uint16_t XPos, uint16_t YPos, LCD_LAYER Layer);
void LCD_DrawStraightLineOnLayer(uint32_t Color, uint16_t XPos, uint16_t YPos, uint16_t Length, LCD_DrawDirection DrawDirection, LCD_LAYER Layer);
void LCD_DrawLineOnLayer(uint32_t Color, uint16_t XPos1, uint16_t YPos1, uint16_t XPos2, uint16_t YPos2, LCD_LAYER Layer);
void LCD_DrawAntiAliasedLineOnLayer(uint32_t Color, uint16_t XPos1, uint16_t YPos1, uint16_t XPos2, uint16_t YPos2, LCD_LAYER Layer);
void LCD_DrawRectangleOnLayer(uint32_t Color, uint16_t XPos, uint16_t YPos, uint16_t Width, uint16_t Height, LCD_LAYER Layer);
void LCD_DrawFilledRectangleOnLayer(uint32_t Color, uint16_t XPos, uint16_t YPos, uint16_t Width, uint16_t Height, LCD_LAYER Layer);
void LCD_DrawCircleOnLayer(uint32_t Color, uint16_t XPos, uint16_t YPos, uint16_t Radius, LCD_LAYER Layer);
void LCD_DrawFilledCircleOnLayer(uint32_t Color, uint16_t XPos, uint16_t YPos, uint16_t Radius, LCD_LAYER Layer);

void LCD_DrawAlphaImageOnLayer(uint16_t XPos, uint16_t YPos, uint32_t Color, ALPHA_IMAGE* Image, LCD_LAYER Layer);
void LCD_DrawARGB8888ImageOnLayer(uint16_t XPos, uint16_t YPos, ARGB8888_IMAGE* Image, LCD_LAYER Layer);

void LCD_DrawARGB8888BufferOnLayer(uint16_t XPos, uint16_t YPos, uint16_t Width, uint16_t Height, uint32_t BufferStartAddress, LCD_LAYER Layer);


#endif /* LCD_H_ */
