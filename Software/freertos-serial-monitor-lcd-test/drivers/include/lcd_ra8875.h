/**
 ******************************************************************************
 * @file	lcd_ra8875.h
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-06-02
 * @brief
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef LCD_RA8875_H_
#define LCD_RA8875_H_

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
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
#define LCD_COLOR_PURPLE	(LCD_COLOR_RED | LCD_COLOR_BLUE)

/* Typedefs ------------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/
void LCD_Init();

void LCD_SetActiveWindow(uint16_t XLeft, uint16_t XRight, uint16_t YTop, uint16_t YBottom);

void LCD_SetBackgroundColor(uint16_t Color);
void LCD_SetBackgroundColorRGB(uint8_t Red, uint8_t Green, uint8_t Blue);
void LCD_SetBackgroundColorRGB565(RGB565_TypeDef* RGB);
void LCD_SetForegroundColor(uint16_t Color);

void LCD_SetTextWritePosition(uint16_t XPos, uint16_t YPos);
void LCD_WriteString(uint8_t *string);

void LCD_TestBackground(uint16_t Delay);
void LCD_TestBackgroundFade(uint16_t Delay);
void LCD_TestText(uint16_t Delay);
void LCD_TestWriteAllCharacters();


#endif /* LCD_RA8875_H_ */
