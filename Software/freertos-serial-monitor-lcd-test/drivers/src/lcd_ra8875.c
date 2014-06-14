/**
 ******************************************************************************
 * @file	lcd_ra8875.c
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

/* Includes ------------------------------------------------------------------*/
#include "lcd_ra8875.h"
#include "lcd_ra8875_registers.h"

/* Private defines -----------------------------------------------------------*/
/* Private typedefs ----------------------------------------------------------*/
typedef struct
{
	__IO uint16_t *LCD_REG;
	__IO uint16_t *LCD_RAM;
	SemaphoreHandle_t xWaitSemaphore;	/* Semaphore for the wait signal */
} LCD_TypeDef;

/* Private variables ---------------------------------------------------------*/
LCD_TypeDef LCD;
SemaphoreHandle_t xLCDSemaphore;

/* Private function prototypes -----------------------------------------------*/
static void prvLCD_GPIOConfig();
static void prvLCD_FSMCConfig();
static void prvLCD_InterruptConfig();
static void prvLCD_PLLInit();

static inline void prvLCD_CmdWrite(uint16_t Command);
static inline void prvLCD_DataWrite(uint16_t Data);
static inline void prvLCD_WriteCommandWithData(uint16_t Command, uint16_t Data);
static inline uint16_t prvLCD_StatusRead();
static inline uint16_t prvLCD_DataRead();

static void prvLCD_CheckBusy();
static void prvLCD_CheckBTEBusy();


/* Functions -----------------------------------------------------------------*/
/**
 * @brief	Initializes the LCD
 * @param	None
 * @retval	None
 */
void LCD_Init()
{
	xLCDSemaphore = xSemaphoreCreateMutex();

	/* Try to take the semaphore - should not be a problem here */
	xSemaphoreTake(xLCDSemaphore, portMAX_DELAY);

	LCD.LCD_REG = (uint16_t*) 0x60080000;
	LCD.LCD_RAM = (uint16_t*) 0x60000000;

	/*
	 * Create the binary semaphores:
	 * The semaphore is created in the 'empty' state, meaning
	 * the semaphore must first be given before it can be taken (obtained)
	 * using the xSemaphoreTake() function.
	 */
	LCD.xWaitSemaphore = xSemaphoreCreateBinary();
	/* Give the semaphore because the LCD should be when we start */
	xSemaphoreGive(LCD.xWaitSemaphore);

	prvLCD_GPIOConfig();
	prvLCD_FSMCConfig();
	prvLCD_InterruptConfig();

	/* Software reset the LCD */
	prvLCD_WriteCommandWithData(LCD_PWRR, 0x01);
	vTaskDelay(10 / portTICK_PERIOD_MS);
	prvLCD_WriteCommandWithData(LCD_PWRR, 0x00);

	prvLCD_PLLInit();

	/* 16 bit, 65k */
	prvLCD_WriteCommandWithData(LCD_SYSR, 0x0F);

	/* PDAT is fetched at PCLK falling edge, PCLK period = 2 times of System Clock period */
	prvLCD_WriteCommandWithData(LCD_PCSR, 0x81);

	vTaskDelay(1 / portTICK_PERIOD_MS);

	/* Horizontal set */
	/* Horizontal display width(pixels) = (HDWR + 1)*8 */
	prvLCD_WriteCommandWithData(LCD_HDWR, 0x63);
	/* Horizontal Non-Display Period Fine Tuning(HNDFT) [3:0] */
	prvLCD_WriteCommandWithData(LCD_HNDFTR, 0x00);
	/* Horizontal Non-Display Period (pixels) = (HNDR + 1)*8 */
	prvLCD_WriteCommandWithData(LCD_HNDR, 0x03);
	/* HSYNC Start Position(PCLK) = (HSTR + 1)*8 */
	prvLCD_WriteCommandWithData(LCD_HSTR, 0x03);
	/* HSYNC Width [4:0]   HSYNC Pulse width(PCLK) = (HPWR + 1)*8 */
	prvLCD_WriteCommandWithData(LCD_HPWR, 0x0B);

	/* Vertical set */
	/* Vertical pixels = VDHR + 1 */
	prvLCD_WriteCommandWithData(LCD_VDHR0, 0xDF);
	prvLCD_WriteCommandWithData(LCD_VDHR1, 0x01);
	/* Vertical Non-Display area = (VNDR + 1) */
	prvLCD_WriteCommandWithData(LCD_VNDR0, 0x20);
	prvLCD_WriteCommandWithData(LCD_VNDR1, 0x00);
	/* VSYNC Start Position(PCLK) = (VSTR + 1) */
	prvLCD_WriteCommandWithData(LCD_VSTR0, 0x16);
	prvLCD_WriteCommandWithData(LCD_VSTR1, 0x00);
	/* VSYNC Pulse Width(PCLK) = (VPWR + 1) */
	prvLCD_WriteCommandWithData(LCD_VPWR, 0x01);

	/* Give back the semaphore */
	xSemaphoreGive(xLCDSemaphore);


	/* Set the whole screen as the active window */
	LCD_SetActiveWindow(0, 799, 0, 479);

	/* Full Brightness */
	LCD_SetBrightness(0xFF);

	/* Display on */
	LCD_DisplayOn();
}

/**
 * @brief	Set the working window area
 * @param	None
 * @retval	None
 */
void LCD_SetActiveWindow(uint16_t XLeft, uint16_t XRight, uint16_t YTop, uint16_t YBottom)
{
	/* Try to take the semaphore */
	xSemaphoreTake(xLCDSemaphore, portMAX_DELAY);

	uint16_t temp;
	/* setting active window X */
	temp = XLeft;
	prvLCD_WriteCommandWithData(LCD_HSAW0, temp);
	temp = XLeft >> 8;
	prvLCD_WriteCommandWithData(LCD_HSAW1, temp);

	temp = XRight;
	prvLCD_WriteCommandWithData(LCD_HEAW0, temp);
	temp = XRight >> 8;
	prvLCD_WriteCommandWithData(LCD_HEAW1, temp);

	/* setting active window Y */
	temp = YTop;
	prvLCD_WriteCommandWithData(LCD_VSAW0, temp);
	temp = YTop >> 8;
	prvLCD_WriteCommandWithData(LCD_VSAW1, temp);

	temp = YBottom;
	prvLCD_WriteCommandWithData(LCD_VEAW0, temp);
	temp = YBottom >> 8;
	prvLCD_WriteCommandWithData(LCD_VEAW1, temp);

	/* Give back the semaphore */
	xSemaphoreGive(xLCDSemaphore);
}

/**
 * @brief	Clear the active window
 * @param	None
 * @retval	None
 */
void LCD_ClearActiveWindow()
{
	/* Try to take the semaphore */
	xSemaphoreTake(xLCDSemaphore, portMAX_DELAY);

	prvLCD_WriteCommandWithData(LCD_MCLR, 0xC0);

	/* Give back the semaphore */
	xSemaphoreGive(xLCDSemaphore);
}

/**
 * @brief	Clear the full window
 * @param	None
 * @retval	None
 */
void LCD_ClearFullWindow()
{
	/* Try to take the semaphore */
	xSemaphoreTake(xLCDSemaphore, portMAX_DELAY);

	prvLCD_WriteCommandWithData(LCD_MCLR, 0x80);

	/* Give back the semaphore */
	xSemaphoreGive(xLCDSemaphore);
}

/**
 * @brief	Sets the brightness of the backlight
 * @param	Brightness: A value between 0 and 255 where 255 is fully on
 * @retval	None
 */
void LCD_SetBrightness(uint8_t Brightness)
{
	/* Try to take the semaphore */
	xSemaphoreTake(xLCDSemaphore, portMAX_DELAY);

	if (Brightness == 0)
	{
		/* Disable PWM1 */
		prvLCD_WriteCommandWithData(LCD_P1CR, 0x01);
	}
	else
	{
		prvLCD_WriteCommandWithData(LCD_P1CR, 0x81);
		prvLCD_WriteCommandWithData(LCD_P1DCR, Brightness);
	}

	/* Give back the semaphore */
	xSemaphoreGive(xLCDSemaphore);
}

/**
 * @brief	Turn the display on
 * @param	None
 * @retval	None
 */
void LCD_DisplayOn()
{
	/* Try to take the semaphore */
	xSemaphoreTake(xLCDSemaphore, portMAX_DELAY);

	/* Display on*/
	prvLCD_WriteCommandWithData(LCD_PWRR, 0x80);

	/* Give back the semaphore */
	xSemaphoreGive(xLCDSemaphore);
}

/* Color ---------------------------------------------------------------------*/
/**
 * @brief	Background color settings
 * @param	None
 * @retval	None
 */
void LCD_SetBackgroundColor(uint16_t Color)
{
	/* Try to take the semaphore */
	xSemaphoreTake(xLCDSemaphore, portMAX_DELAY);

	prvLCD_WriteCommandWithData(LCD_BGCR0, (uint16_t)(Color >> 11));	/* Red */
	prvLCD_WriteCommandWithData(LCD_BGCR1, (uint16_t)(Color >> 5));		/* Green */
	prvLCD_WriteCommandWithData(LCD_BGCR2, (uint16_t)(Color));			/* Blue */

	/* Give back the semaphore */
	xSemaphoreGive(xLCDSemaphore);
}

/**
 * @brief	Background color settings RGB
 * @param	None
 * @retval	None
 */
void LCD_SetBackgroundColorRGB(uint8_t Red, uint8_t Green, uint8_t Blue)
{
	/* Try to take the semaphore */
	xSemaphoreTake(xLCDSemaphore, portMAX_DELAY);

	prvLCD_WriteCommandWithData(LCD_BGCR0, (uint16_t)(Red & 0x1F));		/* Red */
	prvLCD_WriteCommandWithData(LCD_BGCR1, (uint16_t)(Green & 0x3F));	/* Green */
	prvLCD_WriteCommandWithData(LCD_BGCR2, (uint16_t)(Blue & 0x1F));	/* Blue */

	/* Give back the semaphore */
	xSemaphoreGive(xLCDSemaphore);
}

/**
 * @brief	Background color settings RGB565
 * @param	None
 * @retval	None
 */
void LCD_SetBackgroundColorRGB565(RGB565_TypeDef* RGB)
{
	/* Try to take the semaphore */
	xSemaphoreTake(xLCDSemaphore, portMAX_DELAY);

	prvLCD_WriteCommandWithData(LCD_BGCR0, (uint16_t)(RGB->red & 0x1F));	/* Red */
	prvLCD_WriteCommandWithData(LCD_BGCR1, (uint16_t)(RGB->green & 0x3F));	/* Green */
	prvLCD_WriteCommandWithData(LCD_BGCR2, (uint16_t)(RGB->blue & 0x1F));	/* Blue */

	/* Give back the semaphore */
	xSemaphoreGive(xLCDSemaphore);
}

/**
 * @brief	Foreground color settings
 * @param	None
 * @retval	None
 */
void LCD_SetForegroundColor(uint16_t Color)
{
	/* Try to take the semaphore */
	xSemaphoreTake(xLCDSemaphore, portMAX_DELAY);

	prvLCD_WriteCommandWithData(LCD_FGCR0, (uint16_t)(Color >> 11));	/* Red */
	prvLCD_WriteCommandWithData(LCD_FGCR1, (uint16_t)(Color >> 5));		/* Green */
	prvLCD_WriteCommandWithData(LCD_FGCR2, (uint16_t)(Color));			/* Blue */

	/* Give back the semaphore */
	xSemaphoreGive(xLCDSemaphore);
}

/**
 * @brief	Foreground color settings RGB
 * @param	None
 * @retval	None
 */
void LCD_SetForegroundColorRGB(uint8_t Red, uint8_t Green, uint8_t Blue)
{
	/* Try to take the semaphore */
	xSemaphoreTake(xLCDSemaphore, portMAX_DELAY);

	prvLCD_WriteCommandWithData(LCD_FGCR0, (uint16_t)(Red & 0x1F));		/* Red */
	prvLCD_WriteCommandWithData(LCD_FGCR1, (uint16_t)(Green & 0x3F));	/* Green */
	prvLCD_WriteCommandWithData(LCD_FGCR2, (uint16_t)(Blue & 0x1F));	/* Blue */

	/* Give back the semaphore */
	xSemaphoreGive(xLCDSemaphore);
}

/**
 * @brief	Foreground color settings RGB565
 * @param	None
 * @retval	None
 */
void LCD_SetForegroundColorRGB565(RGB565_TypeDef* RGB)
{
	/* Try to take the semaphore */
	xSemaphoreTake(xLCDSemaphore, portMAX_DELAY);

	prvLCD_WriteCommandWithData(LCD_FGCR0, (uint16_t)(RGB->red & 0x1F));	/* Red */
	prvLCD_WriteCommandWithData(LCD_FGCR1, (uint16_t)(RGB->green & 0x3F));	/* Green */
	prvLCD_WriteCommandWithData(LCD_FGCR2, (uint16_t)(RGB->blue & 0x1F));	/* Blue */

	/* Give back the semaphore */
	xSemaphoreGive(xLCDSemaphore);
}

/* Text ----------------------------------------------------------------------*/
/**
 * @brief	Text write position
 * @param	None
 * @retval	None
 */
void LCD_SetTextWritePosition(uint16_t XPos, uint16_t YPos)
{
	/* Try to take the semaphore */
	xSemaphoreTake(xLCDSemaphore, portMAX_DELAY);

	uint16_t temp;
	temp = XPos;
	prvLCD_WriteCommandWithData(LCD_F_CURXL, temp);
	temp = XPos >> 8;
	prvLCD_WriteCommandWithData(LCD_F_CURXH, temp);

	temp = YPos;
	prvLCD_WriteCommandWithData(LCD_F_CURYL, temp);
	temp = YPos >> 8;
	prvLCD_WriteCommandWithData(LCD_F_CURYH, temp);

	/* Give back the semaphore */
	xSemaphoreGive(xLCDSemaphore);
}

/**
 * @brief	Write LCD_WriteString
 * @param	String: The string to write
 * @param	TransparentBackground: TRANSPARENT if the background should be transparent, NOT_TRANSPARENT otherwise
 * @param	Enlargement: Enlarge the font by ENLARGE_1X, ENLARGE_2X, ENLARGE_3X or ENLARGE_4X times
 * @retval	None
 */
void LCD_WriteString(uint8_t *String, LCD_Transparency_TypeDef TransparentBackground, LCD_FontEnlargement_TypeDef Enlargement)
{
	/* Try to take the semaphore */
	xSemaphoreTake(xLCDSemaphore, portMAX_DELAY);

	/* Set to text mode with invisible cursor */
	prvLCD_WriteCommandWithData(LCD_MWCR0, 0x80);

	/* Set background transparency and font size */
	uint8_t fontControlValue = 0;
	if (TransparentBackground)
		fontControlValue |= 0x40;
	if (Enlargement == ENLARGE_2X)
		fontControlValue |= 0x05;
	else if (Enlargement == ENLARGE_3X)
		fontControlValue |= 0x0A;
	else if (Enlargement == ENLARGE_4X)
		fontControlValue |= 0x0F;
	prvLCD_WriteCommandWithData(LCD_FNCR1, fontControlValue);

	/* Write to memory */
	prvLCD_CmdWrite(LCD_MRWC);
	while (*String != '\0')
	{
		prvLCD_DataWrite(*String);
		++String;
		prvLCD_CheckBusy();
	}

	/* Give back the semaphore */
	xSemaphoreGive(xLCDSemaphore);
}

/* Drawing -------------------------------------------------------------------*/
/**
 * @brief	Draw an ellipse
 * @param	XPos:
 * @param	YPos:
 * @param	LongAxis:
 * @param	ShortAxis:
 * @param	Filled:
 * @retval	None
 */
void LCD_DrawEllipse(uint16_t XPos, uint16_t YPos, uint16_t LongAxis, uint16_t ShortAxis, uint8_t Filled)
{
	/* Try to take the semaphore */
	xSemaphoreTake(xLCDSemaphore, portMAX_DELAY);

	uint16_t temp;
	temp = XPos;
	prvLCD_WriteCommandWithData(LCD_DEHR0, temp);
	temp = XPos >> 8;
	prvLCD_WriteCommandWithData(LCD_DEHR1, temp);

	temp = YPos;
	prvLCD_WriteCommandWithData(LCD_DEVR0, temp);
	temp = YPos >> 8;
	prvLCD_WriteCommandWithData(LCD_DEVR1, temp);

	temp = LongAxis;
	prvLCD_WriteCommandWithData(LCD_ELL_A0, temp);
	temp = LongAxis >> 8;
	prvLCD_WriteCommandWithData(LCD_ELL_A1, temp);

	temp = ShortAxis;
	prvLCD_WriteCommandWithData(LCD_ELL_B0, temp);
	temp = ShortAxis >> 8;
	prvLCD_WriteCommandWithData(LCD_ELL_B1, temp);

	if (Filled)
		prvLCD_WriteCommandWithData(LCD_ELLCR, 0xC0);
	else
		prvLCD_WriteCommandWithData(LCD_ELLCR, 0x80);

	/* Give back the semaphore */
	xSemaphoreGive(xLCDSemaphore);
}

/**
 * @brief	Draw a circle
 * @param	XPos:
 * @param	YPos:
 * @param	Radius:
 * @param	Filled:
 * @retval	None
 */
void LCD_DrawCircle(uint16_t XPos, uint16_t YPos, uint16_t Radius, uint8_t Filled)
{
	/* Try to take the semaphore */
	xSemaphoreTake(xLCDSemaphore, portMAX_DELAY);

	uint16_t temp;

	/* Draw Circle Center Horizontal */
	temp = XPos;
	prvLCD_WriteCommandWithData(LCD_DCHR0, temp);
	temp = XPos >> 8;
	prvLCD_WriteCommandWithData(LCD_DCHR1, temp);

	/* Draw Circle Center Vertical */
	temp = YPos;
	prvLCD_WriteCommandWithData(LCD_DCVR0, temp);
	temp = YPos >> 8;
	prvLCD_WriteCommandWithData(LCD_DCVR1, temp);

	/* Draw Circle Radius */
	temp = Radius;
	prvLCD_WriteCommandWithData(LCD_DCRR, temp);

	if (Filled)
		prvLCD_WriteCommandWithData(LCD_DCR, 0x60);
	else
		prvLCD_WriteCommandWithData(LCD_DCR, 0x40);

	/* Give back the semaphore */
	xSemaphoreGive(xLCDSemaphore);
}

/**
 * @brief	Draw a square or a line
 * @param	XStart:
 * @param	XEnd:
 * @param	YStart:
 * @param	YEnd:
 * @param	Type: Can be SQUARE or LINE
 * @param	Filled: Can be FILLED or NOT_FILLED
 * @retval	None
 */
void LCD_DrawSquareOrLine(uint16_t XStart, uint16_t XEnd, uint16_t YStart, uint16_t YEnd, LCD_DrawType_TypeDef Type, LCD_Fill_TypeDef Filled)
{
	/* Try to take the semaphore */
	xSemaphoreTake(xLCDSemaphore, portMAX_DELAY);

	uint16_t temp;

	/* Horizontal start */
	temp = XStart;
	prvLCD_WriteCommandWithData(LCD_DLHSR0, temp);
	temp = XStart >> 8;
	prvLCD_WriteCommandWithData(LCD_DLHSR1, temp);

	/* Horizontal end */
	temp = XEnd;
	prvLCD_WriteCommandWithData(LCD_DLHER0, temp);
	temp = XEnd>>8;
	prvLCD_WriteCommandWithData(LCD_DLHER1, temp);

	/* Vertical start */
	temp = YStart;
	prvLCD_WriteCommandWithData(LCD_DLVSR0, temp);
	temp = YStart >> 8;
	prvLCD_WriteCommandWithData(LCD_DLVSR1, temp);

	/* Vertical end */
	temp = YEnd;
	prvLCD_WriteCommandWithData(LCD_DLVER0, temp);
	temp = YEnd >> 8;
	prvLCD_WriteCommandWithData(LCD_DLVER1, temp);

	if (Type == SQUARE)
	{
		if (Filled == FILLED)
			prvLCD_WriteCommandWithData(LCD_DCR, 0xB0);
		else
			prvLCD_WriteCommandWithData(LCD_DCR, 0x90);
	}
	else if (Type == LINE)
	{
		prvLCD_WriteCommandWithData(LCD_DCR, 0x80);
	}

	/* Give back the semaphore */
	xSemaphoreGive(xLCDSemaphore);
}

/* BTE - Block Transfer Engine -----------------------------------------------*/
/**
 * @brief	BTE area size settings
 * @param	Width: The width
 * @param	Height: The height
 * @retval	None
 */
void LCD_BTESize(uint16_t Width, uint16_t Height)
{
	/* Try to take the semaphore */
	xSemaphoreTake(xLCDSemaphore, portMAX_DELAY);

	uint16_t temp;
	temp = Width;
	/* BTE Width */
	prvLCD_WriteCommandWithData(LCD_BEWR0, temp);
	temp = Width >> 8;
	prvLCD_WriteCommandWithData(LCD_BEWR1, temp);

	temp = Height;
	/* BTE Height */
	prvLCD_WriteCommandWithData(LCD_BEHR0, temp);
	temp = Height >> 8;
	prvLCD_WriteCommandWithData(LCD_BEHR1, temp);

	/* Give back the semaphore */
	xSemaphoreGive(xLCDSemaphore);
}

/**
 * @brief	Set points for source and destination for BTE
 * @param	SourceX: X-coordinate for the source
 * @param	SourceY: Y-coordinate for the source
 * @param	DestinationX: X-coordinate for the destination
 * @param	DestinationY: Y-coordinate for the destination
 * @retval	None
 */
void LCD_BTESourceDestinationPoints(uint16_t SourceX, uint16_t SourceY, uint16_t DestinationX, uint16_t DestinationY)
{
	/* Try to take the semaphore */
	xSemaphoreTake(xLCDSemaphore, portMAX_DELAY);

	uint16_t temp, temp1;

	/* Horizontal Source Point of BTE */
	temp = SourceX;
	prvLCD_WriteCommandWithData(LCD_HSBE0, temp);
	temp = SourceX >> 8;
	prvLCD_WriteCommandWithData(LCD_HSBE1, temp);

	/* Vertical Source Point of BTE */
	temp = SourceY;
	prvLCD_WriteCommandWithData(LCD_VSBE0, temp);
	temp = SourceY >> 8;
	prvLCD_CmdWrite(LCD_VSBE1);
	temp1 = prvLCD_DataRead();
	temp1 &= 0x80;	/* Get Source layer */
    temp = temp | temp1;
    prvLCD_WriteCommandWithData(LCD_VSBE1, temp);

	/* Horizontal Destination Point of BTE */
	temp = DestinationX;
	prvLCD_WriteCommandWithData(LCD_HDBE0, temp);
	temp = DestinationX >> 8;
	prvLCD_WriteCommandWithData(LCD_HDBE1, temp);

    /* Vertical Destination Point of BTE */
	temp = DestinationY;
	prvLCD_WriteCommandWithData(LCD_VDBE0, temp);
	temp = DestinationY >> 8;
	prvLCD_CmdWrite(LCD_VDBE1);
	temp1 = prvLCD_DataRead();
	temp1 &= 0x80;	/* Get Source layer */
	temp = temp | temp1;
	prvLCD_WriteCommandWithData(LCD_VDBE1, temp);

	/* Give back the semaphore */
	xSemaphoreGive(xLCDSemaphore);
}

/**
 * @brief	Display an image (or any pixel-data) at a specific location
 * @param	Image: Pointer to the image to display. See LCDImage_TypeDef for how it should look like.
 * @param	XPos: The x position where the image should be displayed
 * @param	YPos: The y position where the image should be displayed
 * @retval	None
 * @note	No checks are done to make sure the image will fit on the screen. This should be done by setting
 * 			the correct active window and making sure the width and height are valid values
 */
void LCD_BTEDisplayImageOfSizeAt(const LCD_Image_TypeDef* Image, uint16_t XPos, uint16_t YPos)
{
	/* Try to take the semaphore */
	xSemaphoreTake(xLCDSemaphore, portMAX_DELAY);

	LCD_BTESize(Image->width, Image->height);							/* Set size */
	prvLCD_WriteCommandWithData(LCD_BECR1, 0xC0); 		/* Write BTE operation - Use source data (i.e. data we send) */
	LCD_BTESourceDestinationPoints(0, 0, XPos, YPos);	/* Set destination coordinates */
	prvLCD_WriteCommandWithData(LCD_BECR0, 0x80);		/* Enable BTE in block mode for source and destination */
	prvLCD_CheckBusy();

	/* Write data to memory */
	prvLCD_CmdWrite(LCD_MRWC);
	for (uint32_t i = 0; i < Image->width*Image->height; i++)
	{
		prvLCD_DataWrite(Image->data[i]);
		prvLCD_CheckBusy();
	}
	prvLCD_CheckBTEBusy();

	/* Give back the semaphore */
	xSemaphoreGive(xLCDSemaphore);
}

/* Private functions ---------------------------------------------------------*/
/**
 * @brief	Initializes the GPIO used for the LCD
 * @param	None
 * @retval	None
 */
static void prvLCD_GPIOConfig()
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

/**
 * @brief	Initializes the FSMC
 * @param	None
 * @retval	None
 */
static void prvLCD_FSMCConfig()
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
	FSMC_NORSRAM_Timing_Init(FSMC_NORSRAM_DEVICE, &FSMC_NORSRAM_TimingInitStructure, 0);
	/* Enable FSMC_NORSRAM_BANK1 */
	FSMC_NORSRAMCmd(FSMC_NORSRAM_BANK1, ENABLE);
}

/**
 * @brief	Initializes the LCD interrupt
 * @param	None
 * @retval	None
 */
static void prvLCD_InterruptConfig()
{
	/* Enable clock for GPIOD */
	__GPIOD_CLK_ENABLE();

	/* LCD Interrupt signal */
	/* Configure PD12 as interrupt, falling edge, with pull-up */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin = GPIO_PIN_12;
	GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;	/* When LCD_INT goes low an interrupt has occured */
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* LCD wait signal */
	/* Configure PD11 as interrupt, rising edge, with pull-up */
	GPIO_InitStructure.Pin = GPIO_PIN_11;
	GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING;	/* When LCD_WAIT goes high the LCD is done */
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* Configure priority and enable interrupt */
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/**
 * @brief	Initializes the PLL
 * @param	None
 * @retval	None
 */
static void prvLCD_PLLInit()
{
	prvLCD_WriteCommandWithData(LCD_PLLC1, 0x0C);
	vTaskDelay(1 / portTICK_PERIOD_MS);

	prvLCD_WriteCommandWithData(LCD_PLLC2, 0x02);
	vTaskDelay(1 / portTICK_PERIOD_MS);
}

/**
 * @brief	Sends a command to the LCD
 * @param	None
 * @retval	None
 */
static inline void prvLCD_CmdWrite(uint16_t Command)
{
	*LCD.LCD_REG = Command;
}

/**
 * @brief	Sends data to the LCD
 * @param	None
 * @retval	None
 */
static inline void prvLCD_DataWrite(uint16_t Data)
{
	*LCD.LCD_RAM = Data;
}

/**
 * @brief	Write command and data
 * @param	None
 * @retval	None
 */
static inline void prvLCD_WriteCommandWithData(uint16_t Command, uint16_t Data)
{
	prvLCD_CheckBusy();
  	prvLCD_CmdWrite(Command);
  	prvLCD_DataWrite(Data);
}

/**
 * @brief	Read status
 * @param	None
 * @retval	None
 */
static inline uint16_t prvLCD_StatusRead()
{
	return *LCD.LCD_REG;
}


/**
 * @brief	Read data
 * @param	None
 * @retval	None
 */
static inline uint16_t prvLCD_DataRead()
{
	return *LCD.LCD_RAM;
}


/**
 * @brief	Check busy
 * @param	None
 * @retval	None
 */
static void prvLCD_CheckBusy()
{
//	/* Try to take the semaphore */
//	xSemaphoreTake(LCD.xWaitSemaphore, portMAX_DELAY);

	/* TODO: Check if it's better to just poll the wait pin X number of times */

	uint16_t temp;
	do
	{
		temp = prvLCD_StatusRead();
	} while ((temp & 0x80) == 0x80);
}

/**
 * @brief	Check BTE busy
 * @param	None
 * @retval	None
 */
static void prvLCD_CheckBTEBusy()
{
	volatile uint16_t temp;
	do
	{
		temp = prvLCD_StatusRead();
	} while ((temp & 0x40) == 0x40);
}

/* Test Functions ------------------------------------------------------------*/
/**
 * @brief	Test background color of LCD
 * @param	Delay: Time for the delay used in the test
 * @retval	None
 */
void LCD_TestBackground(uint16_t Delay)
{
	///display red
	LCD_SetBackgroundColor(LCD_COLOR_RED);//Background color setting
	LCD_ClearFullWindow();
	vTaskDelay(Delay / portTICK_PERIOD_MS);

	///display green
	LCD_SetBackgroundColor(LCD_COLOR_GREEN);//Background color setting
	LCD_ClearFullWindow();
	vTaskDelay(Delay / portTICK_PERIOD_MS);

	///display blue
	LCD_SetBackgroundColor(LCD_COLOR_BLUE);//Background color setting
	LCD_ClearFullWindow();
	vTaskDelay(Delay / portTICK_PERIOD_MS);

	///display white
	LCD_SetBackgroundColor(LCD_COLOR_WHITE);//Background color setting
	LCD_ClearFullWindow();
	vTaskDelay(Delay / portTICK_PERIOD_MS);

	///display cyan
	LCD_SetBackgroundColor(LCD_COLOR_CYAN);//Background color setting
	LCD_ClearFullWindow();
	vTaskDelay(Delay / portTICK_PERIOD_MS);

	///display yellow
	LCD_SetBackgroundColor(LCD_COLOR_YELLOW);//Background color setting
	LCD_ClearFullWindow();
	vTaskDelay(Delay / portTICK_PERIOD_MS);

	///display purple
	LCD_SetBackgroundColor(LCD_COLOR_PURPLE);//Background color setting
	LCD_ClearFullWindow();
	vTaskDelay(Delay / portTICK_PERIOD_MS);

	///display brown
//	LCD_SetBackgroundColor(LCD_COLOR_BROWN);//Background color setting
//	LCD_ClearFullWindow();
//	vTaskDelay(Delay / portTICK_PERIOD_MS);

	///display black
	LCD_SetBackgroundColor(LCD_COLOR_BLACK);//Background color setting
	LCD_ClearFullWindow();
	vTaskDelay(Delay / portTICK_PERIOD_MS);
}

/**
 * @brief	Test background color of LCD by fading
 * @param	Delay: Time for the delay used in the test
 * @retval	None
 */
void LCD_TestBackgroundFade(uint16_t Delay)
{
	for (uint32_t i = 0; i < 360*2; i++)
	{
		static HSB_TypeDef hsb = {0, 1, 1};
		RGB565_TypeDef rgb;
		COLOR_HSBtoRGB565(&hsb, &rgb);

		LCD_SetBackgroundColorRGB565(&rgb);
		LCD_ClearFullWindow();
		vTaskDelay(Delay / portTICK_PERIOD_MS);

		hsb.hue++;
		if (hsb.hue > 360)
			hsb.hue = 0;
	}
}

/**
 * @brief	Test text on LCD
 * @param	Delay: Time for the delay used in the test
 * @retval	None
 */
void LCD_TestText(uint16_t Delay)
{
	////////////RA8875 internal input character test
	LCD_SetBackgroundColor(LCD_COLOR_BLACK);//Background color setting
	LCD_ClearFullWindow();
    LCD_SetForegroundColor(LCD_COLOR_WHITE);//Set the foreground color
    prvLCD_WriteCommandWithData(LCD_FWTSR, 0x01);//Set the characters mode 16x16 / spacing 1
    prvLCD_WriteCommandWithData(LCD_MWCR0, 0x80);//Set the character mode
    prvLCD_WriteCommandWithData(LCD_FNCR0, 0x10);//Select the internal CGROM  ISO/IEC 8859-1.

    LCD_SetTextWritePosition(300, 232);//Text written to the position
    LCD_WriteString(" abcdefghijklmnopqrstuvxyz", TRANSPARENT, ENLARGE_1X);

    LCD_SetForegroundColor(LCD_COLOR_GREEN);//Set the foreground color
    LCD_SetTextWritePosition(300, 232+32);//Text written to the position
    LCD_WriteString("ABCDEFGHIJKLMNOPQRSTUVXYZ", TRANSPARENT, ENLARGE_1X);

    LCD_SetForegroundColor(LCD_COLOR_BLUE);//Set the foreground color
    LCD_SetTextWritePosition(300, 232+64);//Text written to the position
    LCD_WriteString("0123456789 !\"#Û%&/()=?`«", TRANSPARENT, ENLARGE_1X);

    LCD_SetForegroundColor(LCD_COLOR_PURPLE);//Set the foreground color
    LCD_SetTextWritePosition(0, 232+64);//Text written to the position
    LCD_TestWriteAllCharacters();

    vTaskDelay(Delay / portTICK_PERIOD_MS);
}

/**
 * @brief	Test writing all characters on LCD
 * @param	None
 * @retval	None
 */
void LCD_TestWriteAllCharacters()
{
	prvLCD_WriteCommandWithData(LCD_MWCR0, 0x80);//Set the character mode
	prvLCD_CmdWrite(0x02);
	for (uint16_t i = 0; i < 0xFF; i++)
	{
		prvLCD_DataWrite(i);
		prvLCD_CheckBusy();
	}
}

/**
 * @brief	Test drawing figures on the LCD
 * @param	Delay:
 * @retval	None
 */
void LCD_TestDrawing(uint16_t Delay)
{
	LCD_SetBackgroundColor(LCD_COLOR_BLACK);
	LCD_SetActiveWindow(0, 799, 0, 479);
	LCD_ClearFullWindow();
	prvLCD_CheckBusy();

	/* Ellipse */
	LCD_SetForegroundColor(LCD_COLOR_CYAN);
	LCD_DrawEllipse(50, 50, 25, 10, 0);
	vTaskDelay(Delay / portTICK_PERIOD_MS);

	/* Ellipse filled */
	LCD_SetForegroundColor(LCD_COLOR_PURPLE);
	LCD_DrawEllipse(150, 50, 10, 30, 1);
	vTaskDelay(Delay / portTICK_PERIOD_MS);

	/* Circle */
	LCD_SetForegroundColor(LCD_COLOR_YELLOW);
	LCD_DrawCircle(50, 150, 30, 0);
	vTaskDelay(Delay / portTICK_PERIOD_MS);

	/* Circle filled */
	LCD_SetForegroundColor(LCD_COLOR_BLUE);
	LCD_DrawCircle(150, 150, 5, 1);
	vTaskDelay(Delay / portTICK_PERIOD_MS);

	/* Square */
	LCD_SetForegroundColor(LCD_COLOR_GREEN);
	LCD_DrawSquareOrLine(10, 100, 200, 250, LCD_SQUARE, 0);
	vTaskDelay(Delay / portTICK_PERIOD_MS);

	/* Square filled */
	LCD_SetForegroundColor(LCD_COLOR_RED);
	LCD_DrawSquareOrLine(150, 200, 200, 220, LCD_SQUARE, 1);
	vTaskDelay(Delay / portTICK_PERIOD_MS);

	/* Line */
	LCD_SetForegroundColor(LCD_COLOR_WHITE);
	LCD_DrawSquareOrLine(10, 500, 300, 320, LCD_LINE, 0);
	vTaskDelay(Delay / portTICK_PERIOD_MS);
}

/**
 * @brief	Test the BTE (Block Transfer Engine)
 * @param	None
 * @retval	None
 */
void LCD_TestBTE(const LCD_Image_TypeDef* Image, uint16_t XPos, uint16_t YPos)
{
	/* Clear display */
	LCD_SetBackgroundColor(LCD_COLOR_BLACK);
	LCD_SetActiveWindow(0, 799, 0, 479);
	LCD_ClearFullWindow();
	prvLCD_CheckBusy();

#if 0
	/* BTE Color Fill */
	LCD_BTESize(25, 120);
	prvLCD_WriteCommandWithData(LCD_BECR1, 0xCC); /* Raster Settings */
	for (uint32_t i = 0; i < 32; i++)
	{
		LCD_SetForegroundColorRGB(i, 0, 0);
		LCD_BTESourceDestinationPoints(0, 0, i*25, 0); /* BTE starting position */
		prvLCD_WriteCommandWithData(LCD_BECR0, 0x80); /* BET open */
	    prvLCD_CheckBTEBusy();

		LCD_SetForegroundColorRGB(0, i*2, 0);
		LCD_BTESourceDestinationPoints(0, 0, i*25, 120); /* BTE starting position */
		prvLCD_WriteCommandWithData(LCD_BECR0, 0x80); /* BET open */
	    prvLCD_CheckBTEBusy();

		LCD_SetForegroundColorRGB(0, 0, i);
		LCD_BTESourceDestinationPoints(0, 0, i*25, 240); /* BTE starting position */
		prvLCD_WriteCommandWithData(LCD_BECR0, 0x80); /* BET open */
	    prvLCD_CheckBTEBusy();

		LCD_SetForegroundColorRGB(i, i*2, i);
		LCD_BTESourceDestinationPoints(0, 0, i*25, 360); /* BTE starting position */
		prvLCD_WriteCommandWithData(LCD_BECR0, 0x80); /* BET open */
	    prvLCD_CheckBTEBusy();
	}

	vTaskDelay(2000 / portTICK_PERIOD_MS);
#endif

	/* Clear display */
	LCD_SetBackgroundColor(LCD_COLOR_BLACK);
	LCD_SetActiveWindow(0, 799, 0, 479);
	LCD_ClearFullWindow();
	prvLCD_CheckBusy();

	/* BTE Image test */
	LCD_BTEDisplayImageOfSizeAt(Image, XPos, YPos);

	vTaskDelay(5000 / portTICK_PERIOD_MS);
}

/* Interrupt Handlers --------------------------------------------------------*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	/* LCD wait pin */
	if (GPIO_Pin == GPIO_PIN_11)
	{
		/* Give the semaphore as the LCD is done processing now */
		xSemaphoreGiveFromISR(LCD.xWaitSemaphore, NULL);
	}
	/* LCD Interrupt pin */
	else if (GPIO_Pin == GPIO_PIN_12)
	{
		/* Do something */
	}
}
