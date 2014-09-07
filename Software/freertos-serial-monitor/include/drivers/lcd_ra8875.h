/**
 ******************************************************************************
 * @file	lcd_ra8875.h
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-06-02
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
	ENLARGE_1X = 1,
	ENLARGE_2X,
	ENLARGE_3X,
	ENLARGE_4X
} LCD_FontEnlargement_TypeDef;

typedef enum
{
	NOT_TRANSPARENT = 0,
	TRANSPARENT
} LCD_Transparency_TypeDef;

typedef enum
{
	ELLIPSE = 0,
	CIRCLE,
	SQUARE,
	LINE,
	TRIANGLE
} LCD_DrawType_TypeDef;

typedef enum
{
	NOT_FILLED = 0,
	FILLED
} LCD_Fill_TypeDef;

typedef struct
{
	const uint16_t *data;
	uint16_t width;
	uint16_t height;
} LCD_Image_TypeDef;

typedef struct
{
	uint16_t xLeft;
	uint16_t xRight;
	uint16_t yTop;
	uint16_t yBottom;
} LCD_ActiveWindow_TypeDef;

/* Function prototypes -------------------------------------------------------*/
void LCD_Init();

void LCD_ClearFullWindow();
void LCD_ClearActiveWindow(uint16_t XLeft, uint16_t XRight, uint16_t YTop, uint16_t YBottom);
void LCD_SetBrightness(uint8_t Brightness);
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
void LCD_WriteString(uint8_t *String, LCD_Transparency_TypeDef TransparentBackground, LCD_FontEnlargement_TypeDef Enlargement);
void LCD_WriteStringInActiveWindowAtPosition(uint8_t *String, LCD_Transparency_TypeDef TransparentBackground,
											 LCD_FontEnlargement_TypeDef Enlargement, LCD_ActiveWindow_TypeDef Window,
											 uint16_t* XPos, uint16_t* YPos);

/* Drawing */
void LCD_DrawEllipse(uint16_t XPos, uint16_t YPos, uint16_t LongAxis, uint16_t ShortAxis, uint8_t Filled);
void LCD_DrawCircle(uint16_t XPos, uint16_t YPos, uint16_t Radius, uint8_t Filled);
void LCD_DrawSquareOrLine(uint16_t XStart, uint16_t XEnd, uint16_t YStart, uint16_t YEnd, LCD_DrawType_TypeDef Type, LCD_Fill_TypeDef Filled);

/* BTE - Block Transfer Engine */
void LCD_BTESize(uint16_t Width, uint16_t Height);
void LCD_BTESourceDestinationPoints(uint16_t SourceX, uint16_t SourceY, uint16_t DestinationX, uint16_t DestinationY);
void LCD_BTEDisplayImageOfSizeAt(const LCD_Image_TypeDef* Image, uint16_t XPos, uint16_t YPos);

void LCD_TestBackground(uint16_t Delay);
void LCD_TestBackgroundFade(uint16_t Delay);
void LCD_TestText(uint16_t Delay);
void LCD_TestWriteAllCharacters();
void LCD_TestDrawing(uint16_t Delay);
void LCD_TestBTE(const LCD_Image_TypeDef* Image, uint16_t XPos, uint16_t YPos);

/* Interrupt callbacks */
void LCD_INT_Callback();
void LCD_WAIT_Callback();


#endif /* LCD_RA8875_H_ */
