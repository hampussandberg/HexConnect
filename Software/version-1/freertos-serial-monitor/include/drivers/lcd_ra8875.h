/**
 *******************************************************************************
 * @file  lcd_ra8875.h
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef LCD_RA8875_H_
#define LCD_RA8875_H_

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "stm32f4xx_hal.h"

#include "color.h"

/* Defines -------------------------------------------------------------------*/
#define LCD_COLOR_BROWN		(0x40C0)
#define LCD_COLOR_BLACK		(0x0000)
#define LCD_COLOR_WHITE		(0xFFFF)
#define LCD_COLOR_RED		(0xF800)
#define LCD_COLOR_GREEN		(0x07E0)
#define LCD_COLOR_BLUE		(0x001F)
#define LCD_COLOR_YELLOW	(LCD_COLOR_RED | LCD_COLOR_GREEN)
#define LCD_COLOR_CYAN		(LCD_COLOR_GREEN | LCD_COLOR_BLUE)
#define LCD_COLOR_MAGENTA	(LCD_COLOR_RED | LCD_COLOR_BLUE)
#define LCD_COLOR_PURPLE	(LCD_COLOR_RED | LCD_COLOR_BLUE)

#define LCD_SQUARE			1
#define LCD_LINE			2

/* Typedefs ------------------------------------------------------------------*/
typedef enum
{
	LCDFontEnlarge_1x = 1,
	LCDFontEnlarge_2x,
	LCDFontEnlarge_3x,
	LCDFontEnlarge_4x,
} LCDFontEnlarge;

typedef enum
{
	LCDTransparency_NotTransparent = 0,
	LCDTransparency_Transparent,
} LCDTransparency;

typedef enum
{
	LCDDrawType_Ellipse = 0,
	LCDDrawType_Circle,
	LCDDrawType_Square,
	LCDDrawType_Line,
	LCDDrawType_Triangle,
} LCDDrawType;

typedef enum
{
	LCDFill_NoFill = 0,
	LCDFill_Fill,
} LCDFill;

typedef enum
{
	LCDTextFormat_ASCII,
	LCDTextFormat_HexWithSpaces,
	LCDTextFormat_HexWithoutSpaces,
} LCDTextFormat;

typedef struct
{
	const uint16_t *data;
	uint16_t width;
	uint16_t height;
} LCDImage;

typedef struct
{
	uint16_t xLeft;
	uint16_t xRight;
	uint16_t yTop;
	uint16_t yBottom;
} LCDActiveWindow;

/* Function prototypes -------------------------------------------------------*/
void LCD_Init();

void LCD_ClearFullWindow();
void LCD_ClearActiveWindow(uint16_t XLeft, uint16_t XRight, uint16_t YTop, uint16_t YBottom);
void LCD_SetBrightness(uint8_t Brightness);
uint8_t LCD_GetBrightness();
void LCD_DisplayOn();

/* Color */
void LCD_SetBackgroundColor(uint16_t Color);
void LCD_SetBackgroundColorRGB(uint8_t Red, uint8_t Green, uint8_t Blue);
void LCD_SetBackgroundColorRGB565(RGB565_TypeDef* RGB);
void LCD_SetForegroundColor(uint16_t Color);
void LCD_SetForegroundColorRGB(uint8_t Red, uint8_t Green, uint8_t Blue);
void LCD_SetForegroundColorRGB565(RGB565_TypeDef* RGB);

/* Text */
void LCD_SetTextWritePosition(uint16_t XPos, uint16_t YPos);
void LCD_WriteString(uint8_t *String, LCDTransparency TransparentBackground, LCDFontEnlarge Enlargement);
void LCD_WriteStringInActiveWindowAtPosition(uint8_t *String, LCDTransparency TransparentBackground,
											 LCDFontEnlarge Enlargement, LCDActiveWindow Window,
											 uint16_t* XPos, uint16_t* YPos);
void LCD_WriteBufferInActiveWindowAtPosition(uint8_t *pBuffer, uint32_t Size, LCDTransparency TransparentBackground,
											 LCDFontEnlarge Enlargement, LCDActiveWindow Window,
											 uint16_t* XPos, uint16_t* YPos);
void LCD_WriteBufferInActiveWindowAtPositionWithFormat(uint8_t *pBuffer, uint32_t Size, LCDTransparency TransparentBackground,
											 	 	  LCDFontEnlarge Enlargement, LCDActiveWindow Window,
											 	 	  uint16_t* XPos, uint16_t* YPos, LCDTextFormat Format);

/* Drawing */
void LCD_DrawEllipse(uint16_t XPos, uint16_t YPos, uint16_t LongAxis, uint16_t ShortAxis, uint8_t Filled);
void LCD_DrawCircle(uint16_t XPos, uint16_t YPos, uint16_t Radius, uint8_t Filled);
void LCD_DrawSquareOrLine(uint16_t XStart, uint16_t XEnd, uint16_t YStart, uint16_t YEnd, LCDDrawType Type, LCDFill Filled);

/* BTE - Block Transfer Engine */
void LCD_BTESize(uint16_t Width, uint16_t Height);
void LCD_BTESourceDestinationPoints(uint16_t SourceX, uint16_t SourceY, uint16_t DestinationX, uint16_t DestinationY);
void LCD_BTEDisplayImageOfSizeAt(const LCDImage* Image, uint16_t XPos, uint16_t YPos);

void LCD_TestBackground(uint16_t Delay);
void LCD_TestBackgroundFade(uint16_t Delay);
void LCD_TestText(uint16_t Delay);
void LCD_TestWriteAllCharacters();
void LCD_TestDrawing(uint16_t Delay);
void LCD_TestBTE(const LCDImage* Image, uint16_t XPos, uint16_t YPos);

/* Interrupt callbacks */
void LCD_INT_Callback();
void LCD_WAIT_Callback();


#endif /* LCD_RA8875_H_ */
