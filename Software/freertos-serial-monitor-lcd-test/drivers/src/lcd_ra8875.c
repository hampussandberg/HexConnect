/**
 ******************************************************************************
 * @file	lcd_ra8875.c
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-06-02
 * @brief
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
} LCD_TypeDef;

/* Private variables ---------------------------------------------------------*/
LCD_TypeDef LCD;

/* Private function prototypes -----------------------------------------------*/
static void prvLCD_GPIOConfig();
static void prvLCD_FSMCConfig();
static void prvLCD_PLLInit();

static inline void prvLCD_CmdWrite(uint16_t Command);
static inline void prvLCD_DataWrite(uint16_t Data);
static inline void prvLCD_WriteCommandWithData(uint16_t Command, uint16_t Data);
static inline uint16_t prvLCD_StatusRead();

static void prvLCD_CheckBusy();


/* Functions -----------------------------------------------------------------*/
/**
 * @brief	Initializes the LCD
 * @param	None
 * @retval	None
 */
void LCD_Init()
{
	LCD.LCD_REG = (uint16_t*) 0x60080000;
	LCD.LCD_RAM = (uint16_t*) 0x60000000;

	prvLCD_GPIOConfig();
	prvLCD_FSMCConfig();

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

	LCD_SetActiveWindow(0, 799, 0, 479);

	/* PWM setting */
	prvLCD_WriteCommandWithData(LCD_P1CR, 0x80);
	/* open PWM */
	prvLCD_WriteCommandWithData(LCD_P1CR, 0x81);
	/* Full Brightness */
	prvLCD_WriteCommandWithData(LCD_P1DCR, 0xFF);

	/* Display on*/
	prvLCD_WriteCommandWithData(LCD_PWRR, 0x80);
}

/**
 * @brief	Set the working window area
 * @param	None
 * @retval	None
 */
void LCD_SetActiveWindow(uint16_t XLeft, uint16_t XRight, uint16_t YTop, uint16_t YBottom)
{
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
}

/**
 * @brief	Background color settings
 * @param	None
 * @retval	None
 */
void LCD_SetBackgroundColor(uint16_t Color)
{
	prvLCD_WriteCommandWithData(LCD_BGCR0, (uint16_t)(Color >> 11));	/* Red */
	prvLCD_WriteCommandWithData(LCD_BGCR1, (uint16_t)(Color >> 5));		/* Green */
	prvLCD_WriteCommandWithData(LCD_BGCR2, (uint16_t)(Color));			/* Blue */
}

/**
 * @brief	Background color settings RGB
 * @param	None
 * @retval	None
 */
void LCD_SetBackgroundColorRGB(uint8_t Red, uint8_t Green, uint8_t Blue)
{
	prvLCD_WriteCommandWithData(LCD_BGCR0, (uint16_t)(Red & 0x1F));		/* Red */
	prvLCD_WriteCommandWithData(LCD_BGCR1, (uint16_t)(Green & 0x3F));	/* Green */
	prvLCD_WriteCommandWithData(LCD_BGCR2, (uint16_t)(Blue & 0x1F));	/* Blue */
}

/**
 * @brief	Background color settings RGB565
 * @param	None
 * @retval	None
 */
void LCD_SetBackgroundColorRGB565(RGB565_TypeDef* RGB)
{
	prvLCD_WriteCommandWithData(LCD_BGCR0, (uint16_t)(RGB->red & 0x1F));	/* Red */
	prvLCD_WriteCommandWithData(LCD_BGCR1, (uint16_t)(RGB->green & 0x3F));	/* Green */
	prvLCD_WriteCommandWithData(LCD_BGCR2, (uint16_t)(RGB->blue & 0x1F));	/* Blue */
}

/**
 * @brief	Foreground color settings
 * @param	None
 * @retval	None
 */
void LCD_SetForegroundColor(uint16_t Color)
{
	prvLCD_WriteCommandWithData(LCD_FGCR0, (uint16_t)(Color >> 11));	/* Red */
	prvLCD_WriteCommandWithData(LCD_FGCR1, (uint16_t)(Color >> 5));		/* Green */
	prvLCD_WriteCommandWithData(LCD_FGCR2, (uint16_t)(Color));			/* Blue */
}

/**
 * @brief	Text write position
 * @param	None
 * @retval	None
 */
void LCD_SetTextWritePosition(uint16_t XPos, uint16_t YPos)
{
	uint16_t temp;
	temp = XPos;
	prvLCD_WriteCommandWithData(LCD_F_CURXL, temp);
	temp = XPos >> 8;
	prvLCD_WriteCommandWithData(LCD_F_CURXH, temp);

	temp = YPos;
	prvLCD_WriteCommandWithData(LCD_F_CURYL, temp);
	temp = YPos >> 8;
	prvLCD_WriteCommandWithData(LCD_F_CURYH, temp);
}

/**
 * @brief	Write LCD_WriteString
 * @param	None
 * @retval	None
 */
void LCD_WriteString(uint8_t *LCD_WriteString)
{
	prvLCD_WriteCommandWithData(LCD_MWCR0, 0x80); /* Set the character mode */
	prvLCD_CmdWrite(LCD_MRWC);
	while (*LCD_WriteString != '\0')
	{
		prvLCD_DataWrite(*LCD_WriteString);
		++LCD_WriteString;
		prvLCD_CheckBusy();
	}
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
  	prvLCD_CmdWrite(Command);
  	prvLCD_DataWrite(Data);
}

/**
 * @brief	Read  status
 * @param	None
 * @retval	None
 */
static inline uint16_t prvLCD_StatusRead()
{
	return *LCD.LCD_REG;
}

/**
 * @brief	Check busy
 * @param	None
 * @retval	None
 */
static void prvLCD_CheckBusy()
{
	uint16_t temp;
	do
	{
		temp = prvLCD_StatusRead();
	} while ((temp & 0x80) == 0x80);
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
	prvLCD_WriteCommandWithData(LCD_MCLR, 0x80);//Began to clear the screen (display window)
	vTaskDelay(Delay / portTICK_PERIOD_MS);

	///display green
	LCD_SetBackgroundColor(LCD_COLOR_GREEN);//Background color setting
	prvLCD_WriteCommandWithData(LCD_MCLR, 0x80);//Began to clear the screen (display window)
	vTaskDelay(Delay / portTICK_PERIOD_MS);

	///display blue
	LCD_SetBackgroundColor(LCD_COLOR_BLUE);//Background color setting
	prvLCD_WriteCommandWithData(LCD_MCLR, 0x80);//Began to clear the screen (display window)
	vTaskDelay(Delay / portTICK_PERIOD_MS);

	///display white
	LCD_SetBackgroundColor(LCD_COLOR_WHITE);//Background color setting
	prvLCD_WriteCommandWithData(LCD_MCLR, 0x80);//Began to clear the screen (display window)
	vTaskDelay(Delay / portTICK_PERIOD_MS);

	///display cyan
	LCD_SetBackgroundColor(LCD_COLOR_CYAN);//Background color setting
	prvLCD_WriteCommandWithData(LCD_MCLR, 0x80);//Began to clear the screen (display window)
	vTaskDelay(Delay / portTICK_PERIOD_MS);

	///display yellow
	LCD_SetBackgroundColor(LCD_COLOR_YELLOW);//Background color setting
	prvLCD_WriteCommandWithData(LCD_MCLR, 0x80);//Began to clear the screen (display window)
	vTaskDelay(Delay / portTICK_PERIOD_MS);

	///display purple
	LCD_SetBackgroundColor(LCD_COLOR_PURPLE);//Background color setting
	prvLCD_WriteCommandWithData(LCD_MCLR, 0x80);//Began to clear the screen (display window)
	vTaskDelay(Delay / portTICK_PERIOD_MS);

	///display brown
//	LCD_SetBackgroundColor(LCD_COLOR_BROWN);//Background color setting
//	prvLCD_WriteCommandWithData(LCD_MCLR, 0x80);//Began to clear the screen (display window)
//	vTaskDelay(Delay / portTICK_PERIOD_MS);

	///display black
	LCD_SetBackgroundColor(LCD_COLOR_BLACK);//Background color setting
	prvLCD_WriteCommandWithData(LCD_MCLR, 0x80);//Began to clear the screen (display window)
	vTaskDelay(Delay / portTICK_PERIOD_MS);
}

/**
 * @brief	Test background color of LCD by fading
 * @param	Delay: Time for the delay used in the test
 * @retval	None
 */
void LCD_TestBackgroundFade(uint16_t Delay)
{
	static HSB_TypeDef hsb = {0, 1, 1};
	RGB565_TypeDef rgb;
	COLOR_HSBtoRGB565(&hsb, &rgb);

	LCD_SetBackgroundColorRGB565(&rgb);
	prvLCD_WriteCommandWithData(LCD_MCLR, 0x80);
	vTaskDelay(Delay / portTICK_PERIOD_MS);

	hsb.hue++;
	if (hsb.hue > 360)
		hsb.hue = 0;
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
	prvLCD_WriteCommandWithData(LCD_MCLR, 0x80);//Began to clear the screen (display window)
    LCD_SetForegroundColor(LCD_COLOR_WHITE);//Set the foreground color
    prvLCD_WriteCommandWithData(LCD_FWTSR, 0x01);//Set the characters mode 16x16 / spacing 1
    prvLCD_WriteCommandWithData(LCD_MWCR0, 0x80);//Set the character mode
    prvLCD_WriteCommandWithData(LCD_FNCR0, 0x10);//Select the internal CGROM  ISO/IEC 8859-1.

    LCD_SetTextWritePosition(300, 232);//Text written to the position
    LCD_WriteString(" abcdefghijklmnopqrstuvxyz");

    LCD_SetForegroundColor(LCD_COLOR_GREEN);//Set the foreground color
    LCD_SetTextWritePosition(300, 232+32);//Text written to the position
    LCD_WriteString("ABCDEFGHIJKLMNOPQRSTUVXYZ");

    LCD_SetForegroundColor(LCD_COLOR_BLUE);//Set the foreground color
    LCD_SetTextWritePosition(300, 232+64);//Text written to the position
    LCD_WriteString("0123456789 !\"#Û%&/()=?`«");

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

/* Interrupt Handlers --------------------------------------------------------*/
