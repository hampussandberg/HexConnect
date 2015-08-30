/**
 *******************************************************************************
 * @file    lcd.c
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

/** Includes -----------------------------------------------------------------*/
#include "lcd.h"
#include "sdram.h"
#include "color.h"

/** Private defines ----------------------------------------------------------*/
/* Pixel Clock - 30 MHz works better */
#define PIXEL_CLOCK_30M
//#define PIXEL_CLOCK_50M
#if defined(PIXEL_CLOCK_30M) && defined(PIXEL_CLOCK_50M)
#error "Please choose one frequency only!"
#endif

#ifdef PIXEL_CLOCK_30M
/* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
/* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAI_N = 240 Mhz */
/* PLLLCDCLK = PLLSAI_VCO Output/PLLSAI_R = 240/4 = 60 Mhz */
/* LTDC clock frequency = PLLLCDCLK / RCC_PLLSAIDivR = 60/2 = 30 Mhz */
#define PLLSAIN_VALUE     240
#define PLLSAIR_VALUE     4
#define PLLSAIDivR_VALUE  RCC_PLLSAIDIVR_2
#endif
#ifdef PIXEL_CLOCK_50M
/* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
/* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAI_N = 200 Mhz */
/* PLLLCDCLK = PLLSAI_VCO Output/PLLSAI_R = 200/2 = 100 Mhz */
/* LTDC clock frequency = PLLLCDCLK / RCC_PLLSAIDivR = 100/2 = 50 Mhz */
#define PLLSAIN_VALUE     200
#define PLLSAIR_VALUE     2
#define PLLSAIDivR_VALUE  RCC_PLLSAIDIVR_2
#endif



/* Layer addresses */
#define MEMORY_SIZE                 (SDRAM_SIZE)
#define LCD_ACTIVE_SCREEN_ADDRESS   (SDRAM_BANK_ADDR)
#define LCD_SCREEN_BYTES            (LCD_PIXELS * 2)  /* RBG565, 16 bits */
#define LCD_DISPLAY_BUFFER_ADDRESS  (LCD_ACTIVE_SCREEN_ADDRESS + LCD_SCREEN_BYTES)
#define LCD_LAYER_BYTES             (LCD_PIXELS * 4)  /* ARGB8888, 32 bits */
#define LCD_LAYER_1_ADDRESS         (LCD_DISPLAY_BUFFER_ADDRESS + LCD_SCREEN_BYTES)
#define LCD_LAYER_2_ADDRESS         (LCD_LAYER_1_ADDRESS + LCD_LAYER_BYTES)
#define LCD_LAYER_3_ADDRESS         (LCD_LAYER_2_ADDRESS + LCD_LAYER_BYTES)
#define LCD_LAST_LAYER_ADDRESS      (LCD_LAYER_3_ADDRESS + LCD_LAYER_BYTES)

#if (LCD_LAST_LAYER_ADDRESS > SDRAM_BANK_ADDR + MEMORY_SIZE)
#error "Not enough RAM"
#endif

/* TODO: Check timeout */
#define DMA2D_TIMEOUT    (100)

#define ABS(X)  ((X) > 0 ? (X) : -(X))

/** Private typedefs ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
static SemaphoreHandle_t xSemaphoreDma2d;

#if defined(DUAL_BUFFER_MODE)
static uint32_t prvCurrentScreenAddress = SDRAM_BANK_ADDR;
static uint32_t prvCurrentBufferAddress = SDRAM_BANK_ADDR+LCD_SCREEN_BYTES;
#endif

/** Private function prototypes ----------------------------------------------*/
static void prvGPIOConfig();
static void prvErrorHandler(char* ErrorString);
static void prvTransferComplete(DMA2D_HandleTypeDef *hdma2d);
static void prvTransferError(DMA2D_HandleTypeDef *hdma2d);

/** Functions ----------------------------------------------------------------*/
/**
 * @brief   Initializes the LCD
 * @param   None
 * @retval  None
 */
void LCD_Init()
{
  /* Binary semaphore for synchronisation between dma interrupt and tasks */
  xSemaphoreDma2d = xSemaphoreCreateBinary();
  /* The semaphore is created in the 'empty' state, meaning the semaphore must first be given before it can be taken */
  xSemaphoreGive(xSemaphoreDma2d);

  /* Enable the LTDC Clock */
  __HAL_RCC_LTDC_CLK_ENABLE();

  /* Enable the DMA2D Clock */
  __HAL_RCC_DMA2D_CLK_ENABLE();

  /* Configure the LCD pins */
  prvGPIOConfig();

  /* Init the SDRAM if it's not already done */
  if (!SDRAM_Initialized())
    SDRAM_Init();

  /* LTDC Configuration *********************************************************/
  /* Polarity configuration */
  /* Initialize the horizontal synchronization polarity as active low */
  LTDCHandle.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  /* Initialize the vertical synchronization polarity as active low */
  LTDCHandle.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  /* Initialize the data enable polarity as active low */
  LTDCHandle.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  /* Initialize the pixel clock polarity as input pixel clock */
  LTDCHandle.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

  /* Timing configuration */
  /* Configure horizontal synchronization width */
  LTDCHandle.Init.HorizontalSync = LCD_HSYNC_WIDTH;
  /* Configure vertical synchronization height */
  LTDCHandle.Init.VerticalSync = LCD_VSYNC_WIDTH;
  /* Configure accumulated horizontal back porch */
  LTDCHandle.Init.AccumulatedHBP = LTDCHandle.Init.HorizontalSync + LCD_HBACKPORCH;
  /* Configure accumulated vertical back porch */
  LTDCHandle.Init.AccumulatedVBP = LTDCHandle.Init.VerticalSync + LCD_VBACKPORCH;
  /* Configure accumulated active width */
  LTDCHandle.Init.AccumulatedActiveW = LTDCHandle.Init.AccumulatedHBP + LCD_PIXEL_WIDTH;
  /* Configure accumulated active height */
  LTDCHandle.Init.AccumulatedActiveH = LTDCHandle.Init.AccumulatedVBP + LCD_PIXEL_HEIGHT;
  /* Configure total width */
  LTDCHandle.Init.TotalWidth = LTDCHandle.Init.AccumulatedActiveW + LCD_HFRONTPORCH;
  /* Configure total height */
  LTDCHandle.Init.TotalHeigh = LTDCHandle.Init.AccumulatedActiveH + LCD_VFRONTPORCH;

  /* Configure PLLSAI prescalers for LCD */
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
  PeriphClkInitStruct.PeriphClockSelection =  RCC_PERIPHCLK_LTDC;
  PeriphClkInitStruct.PLLSAI.PLLSAIN =        PLLSAIN_VALUE;
  PeriphClkInitStruct.PLLSAI.PLLSAIR =        PLLSAIR_VALUE;
  PeriphClkInitStruct.PLLSAIDivR =            PLLSAIDivR_VALUE;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

  /* Configure R,G,B component values for LCD background color */
  LTDCHandle.Init.Backcolor.Blue  = 0;
  LTDCHandle.Init.Backcolor.Green = 0;
  LTDCHandle.Init.Backcolor.Red   = 0;

  /* Set the instance */
  LTDCHandle.Instance = LTDC;

  /* Configure the LTDC */
  if (HAL_LTDC_Init(&LTDCHandle) != HAL_OK)
  {
    /* Initialization Error */
    prvErrorHandler("HAL_LTDC_Init error");
  }

  /* NVIC configuration for DMA2D transfer complete interrupt */
  HAL_NVIC_SetPriority(DMA2D_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY, 0);
  HAL_NVIC_EnableIRQ(DMA2D_IRQn);

  /* Set the common DMA2D Handle parameters */
  DMA2DHandle.Instance          = DMA2D;
  DMA2DHandle.XferCpltCallback  = prvTransferComplete;
  DMA2DHandle.XferErrorCallback = prvTransferError;

#if defined(DUAL_BUFFER_MODE)
  /* NVIC configuration for LTDC interrupt */
  HAL_NVIC_SetPriority(LTDC_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY, 0);
  HAL_NVIC_EnableIRQ(LTDC_IRQn);
#endif
}

/**
  * @brief  Initializes the LCD Layers.
  * @param  None
  * @retval None
  */
void LCD_LayerInit()
{
  LTDC_LayerCfgTypeDef LTCD_LayerCfg;

  /* Windowing configuration */
  LTCD_LayerCfg.WindowX0 = 0;
  LTCD_LayerCfg.WindowX1 = LCD_PIXEL_WIDTH;
  LTCD_LayerCfg.WindowY0 = 0;
  LTCD_LayerCfg.WindowY1 = LCD_PIXEL_HEIGHT;

  /* Pixel Format configuration*/
  LTCD_LayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;

#if defined(DUAL_BUFFER_MODE)
  /* Start Address configuration */
  LTCD_LayerCfg.FBStartAdress = prvCurrentScreenAddress;
#else
  /* Start Address configuration */
  LTCD_LayerCfg.FBStartAdress = LCD_ACTIVE_SCREEN_ADDRESS;
#endif

  /* Alpha constant (255 totally opaque) */
  LTCD_LayerCfg.Alpha = 255;

  /* Default Color configuration (configure A,R,G,B component values) */
  LTCD_LayerCfg.Alpha0 = 0;
  LTCD_LayerCfg.Backcolor.Blue = 0;
  LTCD_LayerCfg.Backcolor.Green = 0;
  LTCD_LayerCfg.Backcolor.Red = 0;

  /* Configure blending factors */
  LTCD_LayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
  LTCD_LayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;

  /* Configure the number of lines and number of pixels per line */
  LTCD_LayerCfg.ImageWidth = LCD_PIXEL_WIDTH;
  LTCD_LayerCfg.ImageHeight = LCD_PIXEL_HEIGHT;

  /* Initialize LTDC layer 1 */
  if (HAL_LTDC_ConfigLayer(&LTDCHandle, &LTCD_LayerCfg, 0) != HAL_OK)
  {
    /* Initialization Error */
    prvErrorHandler("HAL_LTDC_ConfigLayer error");
  }

  /* Set transparency for layer */
//  HAL_LTDC_SetAlpha(&LTDCHandle, 0xFF, 0);

  /* Dithering activation */
  HAL_LTDC_EnableDither(&LTDCHandle);

  /* Disable the other layer */
  __HAL_LTDC_LAYER_DISABLE(&LTDCHandle, 1);

  /* Clear all layers and buffer to transparent */
  LCD_ClearScreenBuffer(0x0000);
  LCD_ClearLayer(0x00000000, LCD_LAYER_1);
  LCD_ClearLayer(0x00000000, LCD_LAYER_2);
  LCD_ClearLayer(0x00000000, LCD_LAYER_3);

#if defined(DUAL_BUFFER_MODE)
  /* Refresh the display */
  LCD_SetBufferAsActiveScreen();
  /* Clear the old buffer */
  LCD_ClearScreenBuffer(0x0000);
#else
  /* Refresh the display */
  LCD_RefreshActiveDisplay();
#endif
}

/**
  * @brief  Refresh the displayed data on the display by moving the buffer to the active display
  * @param  None
  * @retval None
  */
void LCD_RefreshActiveDisplay()
{
  /* Try to take the binary semaphore */
  if (xSemaphoreTake(xSemaphoreDma2d, 100) == pdTRUE)
  {
    /* Configure the DMA2D Mode, Color Mode and line output offset */
    DMA2DHandle.Init.Mode         = DMA2D_M2M;
    DMA2DHandle.Init.ColorMode    = DMA2D_RGB565;
    DMA2DHandle.Init.OutputOffset = 0x0;

    /* Init the DMA2D */
    HAL_StatusTypeDef status;
    status = HAL_DMA2D_Init(&DMA2DHandle);
    if (status != HAL_OK)
    {
       prvErrorHandler("");
       return;
    }

#if defined(DUAL_BUFFER_MODE)
    /* Start the transfer in interrupt mode */
    HAL_DMA2D_Start_IT(&DMA2DHandle, prvCurrentBufferAddress, prvCurrentScreenAddress, LCD_PIXEL_WIDTH, LCD_PIXEL_HEIGHT);
#else
    /* Start the transfer in interrupt mode */
    HAL_DMA2D_Start_IT(&DMA2DHandle, LCD_DISPLAY_BUFFER_ADDRESS, LCD_ACTIVE_SCREEN_ADDRESS, LCD_PIXEL_WIDTH, LCD_PIXEL_HEIGHT);
#endif
  }
}

/**
  * @brief  This will draw a layer to the buffer and blend it
  * @param  Layer: Layer to draw, can be any value of LCD_LAYER
  * @retval None
  * @note   This is a time consuming task as the whole screen is drawn.
  */
void LCD_DrawLayerToBuffer(LCD_LAYER Layer)
{
  if (Layer < LCD_LAYER_NUM_OF_LAYERS)
  {
    /* Try to take the binary semaphore */
    if (xSemaphoreTake(xSemaphoreDma2d, 100) == pdTRUE)
    {
      /* Configure the DMA2D Mode, Color Mode and line output offset */
      DMA2DHandle.Init.Mode         = DMA2D_M2M_BLEND;
      DMA2DHandle.Init.ColorMode    = DMA2D_RGB565;
      DMA2DHandle.Init.OutputOffset = 0x0;

      /* Configure the foreground -> The layer */
      DMA2DHandle.LayerCfg[1].AlphaMode       = DMA2D_NO_MODIF_ALPHA;
      DMA2DHandle.LayerCfg[1].InputAlpha      = 0x00;
      DMA2DHandle.LayerCfg[1].InputColorMode  = CM_ARGB8888;
      DMA2DHandle.LayerCfg[1].InputOffset     = 0;

      /* Configure the background -> Display buffer */
      DMA2DHandle.LayerCfg[0].AlphaMode       = DMA2D_NO_MODIF_ALPHA;
      DMA2DHandle.LayerCfg[0].InputAlpha      = 0x00;
      DMA2DHandle.LayerCfg[0].InputColorMode  = CM_RGB565;
      DMA2DHandle.LayerCfg[0].InputOffset     = 0;

      /* Configure source address */
      uint32_t sourceMemoryAddress;
      if (Layer == LCD_LAYER_1)
        sourceMemoryAddress = LCD_LAYER_1_ADDRESS;
      else if (Layer == LCD_LAYER_2)
        sourceMemoryAddress = LCD_LAYER_2_ADDRESS;
      else if (Layer == LCD_LAYER_3)
        sourceMemoryAddress = LCD_LAYER_3_ADDRESS;
      else
        return;


      /* Init the DMA2D */
      HAL_StatusTypeDef status;
      status = HAL_DMA2D_Init(&DMA2DHandle);
      if (status != HAL_OK)
      {
         prvErrorHandler("");
         return;
      }
      /* Config the foreground layer */
      status = HAL_DMA2D_ConfigLayer(&DMA2DHandle, 1);
      if (status != HAL_OK)
      {
         prvErrorHandler("");
         return;
      }
      /* Config the background layer */
      status = HAL_DMA2D_ConfigLayer(&DMA2DHandle, 0);
      if (status != HAL_OK)
      {
         prvErrorHandler("");
         return;
      }

#if defined(DUAL_BUFFER_MODE)
      /* Start the transfer in interrupt mode */
      HAL_DMA2D_BlendingStart_IT(&DMA2DHandle, sourceMemoryAddress, prvCurrentBufferAddress, prvCurrentBufferAddress, LCD_PIXEL_WIDTH, LCD_PIXEL_HEIGHT);
#else
      /* Start the transfer in interrupt mode */
      HAL_DMA2D_BlendingStart_IT(&DMA2DHandle, sourceMemoryAddress, LCD_DISPLAY_BUFFER_ADDRESS, LCD_DISPLAY_BUFFER_ADDRESS, LCD_PIXEL_WIDTH, LCD_PIXEL_HEIGHT);
#endif
    }
  }
}

/**
  * @brief  Refresh the displayed data on the display by moving the buffer to the active display
  * @param  Layer: Layer to draw, can be any value of LCD_LAYER
  * @param  XPos: X-coordinate
  * @param  YPos: Y-coordinate
  * @param  Width: Width of the rectangle
  * @param  Height: Height of the rectangle
  * @retval None
  */
void LCD_DrawPartOfLayerToBuffer(LCD_LAYER Layer, uint16_t XPos, uint16_t YPos, uint16_t Width, uint16_t Height)
{
  if (Layer < LCD_LAYER_NUM_OF_LAYERS)
  {
    /* Try to take the binary semaphore */
    if (xSemaphoreTake(xSemaphoreDma2d, 100) == pdTRUE)
    {
      /* Configure the DMA2D Mode, Color Mode and line output offset */
      DMA2DHandle.Init.Mode         = DMA2D_M2M_BLEND;
      DMA2DHandle.Init.ColorMode    = DMA2D_RGB565;
      DMA2DHandle.Init.OutputOffset = LCD_PIXEL_WIDTH - Width;

      /* Configure the foreground -> The layer */
      DMA2DHandle.LayerCfg[1].AlphaMode       = DMA2D_NO_MODIF_ALPHA;
      DMA2DHandle.LayerCfg[1].InputAlpha      = 0x00;
      DMA2DHandle.LayerCfg[1].InputColorMode  = CM_ARGB8888;
      DMA2DHandle.LayerCfg[1].InputOffset     = LCD_PIXEL_WIDTH - Width;

      /* Configure the background -> Display buffer */
      DMA2DHandle.LayerCfg[0].AlphaMode       = DMA2D_NO_MODIF_ALPHA;
      DMA2DHandle.LayerCfg[0].InputAlpha      = 0x00;
      DMA2DHandle.LayerCfg[0].InputColorMode  = CM_RGB565;
      DMA2DHandle.LayerCfg[0].InputOffset     = LCD_PIXEL_WIDTH - Width;

      /* Configure source address */
      uint32_t sourceMemoryAddress;
      if (Layer == LCD_LAYER_1)
        sourceMemoryAddress = LCD_LAYER_1_ADDRESS + 4*(XPos + YPos*LCD_PIXEL_WIDTH);
      else if (Layer == LCD_LAYER_2)
        sourceMemoryAddress = LCD_LAYER_2_ADDRESS + 4*(XPos + YPos*LCD_PIXEL_WIDTH);
      else if (Layer == LCD_LAYER_3)
        sourceMemoryAddress = LCD_LAYER_3_ADDRESS + 4*(XPos + YPos*LCD_PIXEL_WIDTH);
      else
        return;

      /* Init the DMA2D */
      HAL_StatusTypeDef status;
      status = HAL_DMA2D_Init(&DMA2DHandle);
      if (status != HAL_OK)
      {
         prvErrorHandler("");
         return;
      }
      /* Config the foreground layer */
      status = HAL_DMA2D_ConfigLayer(&DMA2DHandle, 1);
      if (status != HAL_OK)
      {
         prvErrorHandler("");
         return;
      }
      /* Config the background layer */
      status = HAL_DMA2D_ConfigLayer(&DMA2DHandle, 0);
      if (status != HAL_OK)
      {
         prvErrorHandler("");
         return;
      }

#if defined(DUAL_BUFFER_MODE)
      /* Start the transfer in interrupt mode */
      HAL_DMA2D_BlendingStart_IT(&DMA2DHandle,
                                 sourceMemoryAddress,
                                 prvCurrentBufferAddress + 2*(XPos + YPos*LCD_PIXEL_WIDTH),
                                 prvCurrentBufferAddress + 2*(XPos + YPos*LCD_PIXEL_WIDTH),
                                 Width,
                                 Height);
#else
      /* Start the transfer in interrupt mode */
      HAL_DMA2D_BlendingStart_IT(&DMA2DHandle,
                                 sourceMemoryAddress,
                                 LCD_DISPLAY_BUFFER_ADDRESS + 2*(XPos + YPos*LCD_PIXEL_WIDTH),
                                 LCD_DISPLAY_BUFFER_ADDRESS + 2*(XPos + YPos*LCD_PIXEL_WIDTH),
                                 Width,
                                 Height);
#endif
    }
  }
}

/**
  * @brief  Clear the screen buffer of content
  * @param  Color: Color to clear the layer with, format RGB565, 16 bits
  * @retval None
  */
void LCD_ClearScreenBuffer(uint16_t Color)
{
  /* Try to take the binary semaphore */
  if (xSemaphoreTake(xSemaphoreDma2d, 100) == pdTRUE)
  {
    /* Configure the DMA2D Mode, Color Mode and line output offset */
    DMA2DHandle.Init.Mode         = DMA2D_R2M;
    DMA2DHandle.Init.ColorMode    = DMA2D_RGB565;
    DMA2DHandle.Init.OutputOffset = 0x0;

    /* We need to convert the color to ARGB8888 to be compatible with the HAL */
    uint32_t argb8888Color = COLOR_RGB565ToARGB8888(Color);

    /* Init the DMA2D */
    HAL_StatusTypeDef status;
    status = HAL_DMA2D_Init(&DMA2DHandle);
    if (status != HAL_OK)
    {
       prvErrorHandler("");
       return;
    }

#if defined(DUAL_BUFFER_MODE)
      /* Start the transfer in interrupt mode */
      HAL_DMA2D_Start_IT(&DMA2DHandle, argb8888Color, prvCurrentBufferAddress, LCD_PIXEL_WIDTH, LCD_PIXEL_HEIGHT);
#else
      /* Start the transfer in interrupt mode */
      HAL_DMA2D_Start_IT(&DMA2DHandle, argb8888Color, LCD_DISPLAY_BUFFER_ADDRESS, LCD_PIXEL_WIDTH, LCD_PIXEL_HEIGHT);
#endif
  }
}

/**
  * @brief  Clear the screen buffer of content
  * @param  Color: Color to clear the buffer with, format ARGB8888, 32 bits
  * @param  Width:
  * @param  Height:
  * @param  BufferStartAddress:
  * @retval None
  */
void LCD_ClearBuffer(uint32_t Color, uint16_t Width, uint16_t Height, uint32_t BufferStartAddress)
{
  /* Try to take the binary semaphore */
  if (xSemaphoreTake(xSemaphoreDma2d, 100) == pdTRUE)
  {
    /* Configure the DMA2D Mode, Color Mode and line output offset */
    DMA2DHandle.Init.Mode         = DMA2D_R2M;
    DMA2DHandle.Init.ColorMode    = DMA2D_RGB565;
    DMA2DHandle.Init.OutputOffset = LCD_PIXEL_WIDTH - Width;

    /* Init the DMA2D */
    HAL_StatusTypeDef status;
    status = HAL_DMA2D_Init(&DMA2DHandle);
    if (status != HAL_OK)
    {
       prvErrorHandler("");
       return;
    }

    /* Start the transfer in interrupt mode */
    HAL_DMA2D_Start_IT(&DMA2DHandle, Color, BufferStartAddress, Width, Height);
  }
}

/**
  * @brief  Clear a layer of content
  * @param  Color: Color to clear the layer with, format ARGB8888, 32 bits
  * @param  Layer: The layer to clear, can be any value of LCD_LAYER
  * @retval None
  */
void LCD_ClearLayer(uint32_t Color, LCD_LAYER Layer)
{
  if (Layer < LCD_LAYER_NUM_OF_LAYERS)
  {
    /* Try to take the binary semaphore */
    if (xSemaphoreTake(xSemaphoreDma2d, 100) == pdTRUE)
    {
      /* Configure the DMA2D Mode, Color Mode and line output offset */
      DMA2DHandle.Init.Mode         = DMA2D_R2M;
      DMA2DHandle.Init.ColorMode    = DMA2D_ARGB8888;
      DMA2DHandle.Init.OutputOffset = 0x0;

      /* Configure destination address */
      uint32_t destinationMemoryAddress;
      if (Layer == LCD_LAYER_1)
        destinationMemoryAddress = LCD_LAYER_1_ADDRESS;
      else if (Layer == LCD_LAYER_2)
        destinationMemoryAddress = LCD_LAYER_2_ADDRESS;
      else if (Layer == LCD_LAYER_3)
        destinationMemoryAddress = LCD_LAYER_3_ADDRESS;
      else
        return;

      /* Init the DMA2D */
      HAL_StatusTypeDef status;
      status = HAL_DMA2D_Init(&DMA2DHandle);
      if (status != HAL_OK)
      {
         prvErrorHandler("");
         return;
      }

      /* Start the transfer in interrupt mode */
      HAL_DMA2D_Start_IT(&DMA2DHandle, Color, destinationMemoryAddress, LCD_PIXEL_WIDTH, LCD_PIXEL_HEIGHT);
    }
  }
}

#if defined(DUAL_BUFFER_MODE)
/**
  * @brief  Sets the current buffer as the active screen and set the old screen
  *         to the new buffer that can be drawn on. It then configures a line
  *         interrupt so that it can set the new screen address at a good point
  *         in time. This will make sure the screen does not flicker.
  * @param  None
  * @retval None
  */
void LCD_SetBufferAsActiveScreen()
{
  /* Try to take the binary semaphore to make sure no transfer is happening right now */
  if (xSemaphoreTake(xSemaphoreDma2d, 100) == pdTRUE)
  {
    /* Switch the address variables */
    uint32_t temp = prvCurrentScreenAddress;
    prvCurrentScreenAddress = prvCurrentBufferAddress;
    prvCurrentBufferAddress = temp;

    /* Program the line interrupt to trigger at the first line */
    HAL_LTDC_ProgramLineEvent(&LTDCHandle, 0);
  }
}

/**
 * @brief   Callback for LTDC Line Event
 * @param   hltdc
 * @retval  None
 */
void HAL_LTDC_LineEvenCallback(LTDC_HandleTypeDef *hltdc)
{
  /* Update the LCD peripheral */
  HAL_LTDC_SetAddress(&LTDCHandle, prvCurrentScreenAddress, 0);

  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
  /* Give back the binary semaphore */
  xSemaphoreGiveFromISR(xSemaphoreDma2d, &xHigherPriorityTaskWoken);

  /* If xHigherPriorityTaskWoken was set to true you we should yield */
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
#endif

/* Draw functions ------------------------------------------------------------*/
/**
  * @brief  Draw a character
  * @param  Color: Color to use, format ARGB8888, 32 bits
  * @param  XPos: X-coordinate
  * @param  YPos: Y-coordinate
  * @param  Character: Character to draw
  * @param  Font: Pointer to the font to use
  * @param  Layer: Layer to draw on
  * @retval  None
  */
void LCD_DrawCharacterOnLayer(uint32_t Color, uint16_t XPos, uint16_t YPos, char Character, FONT* Font, LCD_LAYER Layer)
{
  if (IS_VALID_LAYER(Layer))
  {
    if (Layer == LCD_LAYER_1)
      LCD_DrawCharacterOnBuffer(Color, XPos, YPos, Character, Font, LCD_LAYER_1_ADDRESS);
    else if (Layer == LCD_LAYER_2)
      LCD_DrawCharacterOnBuffer(Color, XPos, YPos, Character, Font, LCD_LAYER_2_ADDRESS);
    else if (Layer == LCD_LAYER_3)
      LCD_DrawCharacterOnBuffer(Color, XPos, YPos, Character, Font, LCD_LAYER_3_ADDRESS);
  }
}

/**
  * @brief  Draw a character
  * @param  Color: Color to use, format ARGB8888, 32 bits
  * @param  XPos: X-coordinate
  * @param  YPos: Y-coordinate
  * @param  Character: Character to draw
  * @param  Font: Pointer to the font to use
  * @param  BufferStartAddress: Buffer to draw on
  * @retval None
  */
void LCD_DrawCharacterOnBuffer(uint32_t Color, uint16_t XPos, uint16_t YPos, char Character, FONT* Font, uint32_t BufferStartAddress)
{
  /* Try to take the binary semaphore */
  if (xSemaphoreTake(xSemaphoreDma2d, 100) == pdTRUE)
  {
    /* Get the information about the character */
    uint32_t characterAddress;
    uint8_t characterWidth;
    uint8_t characterHeight = Font->Height;
    FONTS_GetAddressAndWidthForCharacter(&characterAddress, &characterWidth, Character, Font);

    /* Configure the DMA2D Mode, Color Mode and line output offset */
    DMA2DHandle.Init.Mode         = DMA2D_M2M_BLEND;
    DMA2DHandle.Init.ColorMode    = DMA2D_ARGB8888;
    DMA2DHandle.Init.OutputOffset = LCD_PIXEL_WIDTH - characterWidth;

    /* Configure the foreground -> The character */
    DMA2DHandle.LayerCfg[1].AlphaMode       = DMA2D_COMBINE_ALPHA;
    DMA2DHandle.LayerCfg[1].InputAlpha      = Color;
    DMA2DHandle.LayerCfg[1].InputColorMode  = CM_A8;
    DMA2DHandle.LayerCfg[1].InputOffset     = 0;

    /* Configure the background -> Display buffer */
    DMA2DHandle.LayerCfg[0].AlphaMode       = DMA2D_NO_MODIF_ALPHA;
    DMA2DHandle.LayerCfg[0].InputAlpha      = 0x00;
    DMA2DHandle.LayerCfg[0].InputColorMode  = CM_ARGB8888;
    DMA2DHandle.LayerCfg[0].InputOffset     = LCD_PIXEL_WIDTH - characterWidth;

    /* Init the DMA2D */
    HAL_StatusTypeDef status;
    status = HAL_DMA2D_Init(&DMA2DHandle);
    if (status != HAL_OK)
    {
       prvErrorHandler("");
       return;
    }
    /* Config the foreground layer */
    status = HAL_DMA2D_ConfigLayer(&DMA2DHandle, 1);
    if (status != HAL_OK)
    {
       prvErrorHandler("");
       return;
    }
    /* Config the background layer */
    status = HAL_DMA2D_ConfigLayer(&DMA2DHandle, 0);
    if (status != HAL_OK)
    {
       prvErrorHandler("");
       return;
    }

    /* Start the transfer in interrupt mode */
    HAL_DMA2D_BlendingStart_IT(&DMA2DHandle,
                               characterAddress,
                               BufferStartAddress + 4*(XPos + YPos*LCD_PIXEL_WIDTH),
                               BufferStartAddress + 4*(XPos + YPos*LCD_PIXEL_WIDTH),
                               characterWidth,
                               characterHeight);
  }
}

/**
  * @brief  Draw a string to the screen, will only draw if there is room for a character.
  *     It will not jump to a new row when the end has been reached, this has to be
  *     managed in some other way.
  * @param  Color: Color to use, format ARGB8888, 32 bits
  * @param  XPos: X-coordinate
  * @param  YPos: Y-coordinate
  * @param  String: Pointer to a string to draw
  * @param  Font: Pointer to the font to use
  * @param  Layer: Layer to draw on
  * @retval  None
  */
void LCD_DrawStringOnLayer(uint32_t Color, uint16_t XPos, uint16_t YPos, char* String, FONT* Font, LCD_LAYER Layer)
{
  uint16_t xTemp = XPos;
  uint16_t yTemp = YPos;

  while (*String != 0 && xTemp + FONTS_GetWidthForCharacter(*String, Font) <= LCD_PIXEL_WIDTH)
  {
    /* If it's not space draw the character */
    if (*String != ' ')
    {
      /* Display one character on LCD */
      LCD_DrawCharacterOnLayer(Color, xTemp, yTemp, *String, Font, Layer);
      /* Move the x position forward one character */
      xTemp += FONTS_GetWidthForCharacter(*String, Font);
    }
    /* If the character is a space just move forward but don't draw anything */
    else
    {
      xTemp += Font->SpaceWidth;
    }
    /* Point on the next character */
    String++;
  }
}

/**
 * @brief  Draw a pixel on a layer
 * @param  Color: Color to use, format ARGB8888, 32 bits
 * @param  XPos: x-coordinate
 * @param  YPos: y-coordinate
 * @param  Layer: Layer to draw on
 * @retval  None
 */
void LCD_DrawPixelOnLayer(uint32_t Color, uint16_t XPos, uint16_t YPos, LCD_LAYER Layer)
{
  /* Check bounds */
  if (XPos > LCD_PIXEL_WIDTH-1 || YPos > LCD_PIXEL_HEIGHT-1)
    return;
  if (Layer == LCD_LAYER_1)
    *(__IO uint32_t*) (LCD_LAYER_1_ADDRESS + 4*(XPos + LCD_PIXEL_WIDTH*YPos)) = Color;
  else if (Layer == LCD_LAYER_2)
    *(__IO uint32_t*) (LCD_LAYER_2_ADDRESS + 4*(XPos + LCD_PIXEL_WIDTH*YPos)) = Color;
  else if (Layer == LCD_LAYER_3)
    *(__IO uint32_t*) (LCD_LAYER_3_ADDRESS + 4*(XPos + LCD_PIXEL_WIDTH*YPos)) = Color;
}

/**
 * @brief  Draw a horizontal or vertical line on a layer
 * @param  Color: Color to use, format ARGB8888, 32 bits
 * @param  XPos: x-coordinate
 * @param  YPos: y-coordinate
 * @param  Length: length of the line
 * @param  Direction: direction of the line, can be any value of LCD_DIRECTION
 * @param  Layer: Layer to draw on
 * @retval  None
 */
void LCD_DrawStraightLineOnLayer(uint32_t Color, uint16_t XPos, uint16_t YPos, uint16_t Length, LCD_DrawDirection DrawDirection, LCD_LAYER Layer)
{
  if (IS_VALID_LAYER(Layer))
  {
    /* Try to take the binary semaphore */
    if (xSemaphoreTake(xSemaphoreDma2d, 100) == pdTRUE)
    {
      /* Configure the DMA2D Mode, Color Mode */
      DMA2DHandle.Init.Mode         = DMA2D_R2M;
      DMA2DHandle.Init.ColorMode    = DMA2D_ARGB8888;

      /* Configure destination address */
      uint32_t destinationMemoryAddress;
      if (Layer == LCD_LAYER_1)
        destinationMemoryAddress = LCD_LAYER_1_ADDRESS + 4*(XPos + YPos*LCD_PIXEL_WIDTH);
      else if (Layer == LCD_LAYER_2)
        destinationMemoryAddress = LCD_LAYER_2_ADDRESS + 4*(XPos + YPos*LCD_PIXEL_WIDTH);
      else if (Layer == LCD_LAYER_3)
        destinationMemoryAddress = LCD_LAYER_3_ADDRESS + 4*(XPos + YPos*LCD_PIXEL_WIDTH);
      else
        return;

      uint32_t transferWidth;
      uint32_t transferHeight;
      if (DrawDirection == LCD_DrawDirection_Horizontal)
      {
        DMA2DHandle.Init.OutputOffset = 0;
        transferHeight = 1;
        transferWidth = Length;
      }
      else
      {
        DMA2DHandle.Init.OutputOffset = LCD_PIXEL_WIDTH - 1;
        transferHeight = Length;
        transferWidth = 1;
      }

      /* Init the DMA2D */
      HAL_StatusTypeDef status;
      status = HAL_DMA2D_Init(&DMA2DHandle);
      if (status != HAL_OK)
      {
         prvErrorHandler("");
         return;
      }

      /* Start the transfer in interrupt mode */
      HAL_DMA2D_Start_IT(&DMA2DHandle, Color, destinationMemoryAddress, transferWidth, transferHeight);
    }
  }
}

/**
 * @brief  Draw a line between two points on a layer
 * @param  Color: Color to use, format ARGB8888, 32 bits
 * @param  XPos1: x-coordinate 1
 * @param  YPos1: y-coordinate 1
 * @param  XPos2: x-coordinate 2
 * @param  YPos2: y-coordinate 2
 * @param  Layer: Layer to draw on
 * @retval  None
 * TODO: Anti-aliasing line drawing
 */
void LCD_DrawLineOnLayer(uint32_t Color, uint16_t XPos1, uint16_t YPos1, uint16_t XPos2, uint16_t YPos2, LCD_LAYER Layer)
{
  if (IS_VALID_LAYER(Layer))
  {
    int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0,
    yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0,
    curpixel = 0;

    /* The difference between the x's */
    deltax = ABS(XPos2 - XPos1);
    /* The difference between the y's */
    deltay = ABS(YPos2 - YPos1);
    /* Start x off at the first pixel */
    x = XPos1;
    /* Start y off at the first pixel */
    y = YPos1;

    /* The x-values are increasing */
    if (XPos2 >= XPos1)
    {
      xinc1 = 1;
      xinc2 = 1;
    }
    /* The x-values are decreasing */
    else
    {
      xinc1 = -1;
      xinc2 = -1;
    }

    /* The y-values are increasing */
    if (YPos2 >= YPos1)
    {
      yinc1 = 1;
      yinc2 = 1;
    }
    /* The y-values are decreasing */
    else
    {
      yinc1 = -1;
      yinc2 = -1;
    }

    /* There is at least one x-value for every y-value */
    if (deltax >= deltay)
    {
      /* Don't change the x when numerator >= denominator */
      xinc1 = 0;
      /* Don't change the y for every iteration */
      yinc2 = 0;
      den = deltax;
      num = deltax / 2;
      numadd = deltay;
      /* There are more x-values than y-values */
      numpixels = deltax;
    }
    /* There is at least one y-value for every x-value */
    else
    {
      /* Don't change the x for every iteration */
      xinc2 = 0;
      /* Don't change the y when numerator >= denominator */
      yinc1 = 0;
      den = deltay;
      num = deltay / 2;
      numadd = deltax;
      /* There are more y-values than x-values */
      numpixels = deltay;
    }

    for (curpixel = 0; curpixel <= numpixels; curpixel++)
    {
      /* Draw the current pixel */
      LCD_DrawPixelOnLayer(Color, x, y, Layer);
      /* Increase the numerator by the top of the fraction */
      num += numadd;
      /* Check if numerator >= denominator */
      if (num >= den)
      {
        /* Calculate the new numerator value */
        num -= den;
        /* Change the x as appropriate */
        x += xinc1;
        /* Change the y as appropriate */
        y += yinc1;
      }
      /* Change the x as appropriate */
      x += xinc2;
      /* Change the y as appropriate */
      y += yinc2;
    }
  }
}

/**
 * @brief  Draw a rectangle on a layer
 * @param  Color: Color to use, format ARGB8888, 32 bits
 * @param  XPos: x-coordinate
 * @param  YPos: y-coordinate
 * @param  Width: width of the rectangle
 * @param  Height: height of the rectangle
 * @param  Layer: Layer to draw on
 * @retval  None
 */
void LCD_DrawRectangleOnLayer(uint32_t Color, uint16_t XPos, uint16_t YPos, uint16_t Width, uint16_t Height, LCD_LAYER Layer)
{
  /* Draw horizontal lines */
  LCD_DrawStraightLineOnLayer(Color, XPos, YPos, Width, LCD_DrawDirection_Horizontal, Layer);
  LCD_DrawStraightLineOnLayer(Color, XPos, (YPos+ Height), Width, LCD_DrawDirection_Horizontal, Layer);

  /* Draw vertical lines */
  LCD_DrawStraightLineOnLayer(Color, XPos, YPos, Height, LCD_DrawDirection_Vertical, Layer);
  LCD_DrawStraightLineOnLayer(Color, (XPos + Width), YPos, Height, LCD_DrawDirection_Vertical, Layer);
}

/**
  * @brief  Draw a filled rectangle on a layer
  * @param  Color: Color to use, format ARGB8888, 32 bits
  * @param  XPos: X-coordinate
  * @param  YPos: Y-coordinate
  * @param  Width: Width of the rectangle
  * @param  Height: Height of the rectangle
  * @param  Layer: Layer to draw on
  * @retval  None
  */
void LCD_DrawFilledRectangleOnLayer(uint32_t Color, uint16_t XPos, uint16_t YPos, uint16_t Width, uint16_t Height, LCD_LAYER Layer)
{
  /* Check for error */
  if (Width == 0 || Height == 0)
  {
    prvErrorHandler("LCD_DrawFilledRectangleOnLayer-Dimensions Wrong");
    return;
  }
  if ((IS_VALID_LAYER(Layer)) && (XPos + Width <= LCD_PIXEL_WIDTH) && (YPos + Height <= LCD_PIXEL_HEIGHT))
  {
    /* Try to take the binary semaphore */
    if (xSemaphoreTake(xSemaphoreDma2d, 100) == pdTRUE)
    {
      /* Configure the DMA2D Mode, Color Mode and line output offset */
      DMA2DHandle.Init.Mode         = DMA2D_R2M;
      DMA2DHandle.Init.ColorMode    = DMA2D_ARGB8888;
      DMA2DHandle.Init.OutputOffset = LCD_PIXEL_WIDTH - Width;

      /* Configure destination address */
      uint32_t destinationMemoryAddress;
      if (Layer == LCD_LAYER_1)
        destinationMemoryAddress = LCD_LAYER_1_ADDRESS + 4*(XPos + YPos*LCD_PIXEL_WIDTH);
      else if (Layer == LCD_LAYER_2)
        destinationMemoryAddress = LCD_LAYER_2_ADDRESS + 4*(XPos + YPos*LCD_PIXEL_WIDTH);
      else if (Layer == LCD_LAYER_3)
        destinationMemoryAddress = LCD_LAYER_3_ADDRESS + 4*(XPos + YPos*LCD_PIXEL_WIDTH);
      else
        return;

      /* Init the DMA2D */
      HAL_StatusTypeDef status;
      status = HAL_DMA2D_Init(&DMA2DHandle);
      if (status != HAL_OK)
      {
         prvErrorHandler("");
         return;
      }

      /* Start the transfer in interrupt mode */
      HAL_DMA2D_Start_IT(&DMA2DHandle, Color, destinationMemoryAddress, Width, Height);
    }
  }
}

/**
 * @brief  Draw a circle on a layer
 * @param  Color: Color to use, format ARGB8888, 32 bits
 * @param  XPos: x-coordinate
 * @param  YPos: y-coordinate
 * @param  Radius: radius of the circle
 * @param  Layer: Layer to draw on
 * @retval  None
 */
void LCD_DrawCircleOnLayer(uint32_t Color, uint16_t XPos, uint16_t YPos, uint16_t Radius, LCD_LAYER Layer)
{
  if (IS_VALID_LAYER(Layer))
  {
    /* Get the address for the layer */
    uint32_t layerAddres;
    if (Layer == LCD_LAYER_1)
      layerAddres = LCD_LAYER_1_ADDRESS;
    else if (Layer == LCD_LAYER_2)
      layerAddres = LCD_LAYER_2_ADDRESS;
    else if (Layer == LCD_LAYER_3)
      layerAddres = LCD_LAYER_3_ADDRESS;
    else
      return;

    /* Adjust to make the XPos and YPos in the upper left corner */
    XPos += Radius;
    YPos += Radius;

    int x = -Radius;
    int y = 0;
    int err = 2-2*Radius;
    int e2;

    do
    {
      *(__IO uint32_t*) (layerAddres + 4*((XPos - x) + LCD_PIXEL_WIDTH * (YPos + y))) = Color;
      *(__IO uint32_t*) (layerAddres + 4*((XPos + x) + LCD_PIXEL_WIDTH * (YPos + y))) = Color;
      *(__IO uint32_t*) (layerAddres + 4*((XPos + x) + LCD_PIXEL_WIDTH * (YPos - y))) = Color;
      *(__IO uint32_t*) (layerAddres + 4*((XPos - x) + LCD_PIXEL_WIDTH * (YPos - y))) = Color;

      e2 = err;
      if (e2 <= y)
      {
        err += ++y*2+1;
        if (-x == y && e2 <= x)
          e2 = 0;
      }
      if (e2 > x)
        err += ++x*2+1;
    } while (x <= 0);
  }
}

/**
 * @brief  Draw a filled circle on a layer
 * @param  Color: Color to use, format ARGB8888, 32 bits
 * @param  XPos: x-coordinate
 * @param  YPos: y-coordinate
 * @param  Radius: radius of the circle
 * @param  Layer: Layer to draw on
 * @retval  None
 */
void LCD_DrawFilledCircleOnLayer(uint32_t Color, uint16_t XPos, uint16_t YPos, uint16_t Radius, LCD_LAYER Layer)
{
  if (IS_VALID_LAYER(Layer))
  {
    /* Adjust to make the XPos and YPos in the upper left corner */
    uint16_t XPosAdjusted = XPos + Radius;
    uint16_t YPosAdjusted = YPos + Radius;

    int32_t decisionVariable;
    uint32_t curXValue;
    uint32_t curYValue;

    decisionVariable = 3 - (Radius << 1);

    curXValue = 0;
    curYValue = Radius;

    while (curXValue <= curYValue)
    {
      if (curYValue > 0)
      {
        LCD_DrawStraightLineOnLayer(
            Color,
            XPosAdjusted - curXValue,   /* XPos */
            YPosAdjusted - curYValue,   /* YPos */
            2*curYValue,         /* Length */
            LCD_DrawDirection_Vertical,
            Layer);
        LCD_DrawStraightLineOnLayer(
            Color,
            XPosAdjusted + curXValue,  /* XPos */
            YPosAdjusted - curYValue,  /* YPos */
            2*curYValue,        /* Length */
            LCD_DrawDirection_Vertical,
            Layer);
      }

      if (curXValue > 0)
      {
        LCD_DrawStraightLineOnLayer(
            Color,
            XPosAdjusted - curYValue,  /* XPos */
            YPosAdjusted - curXValue,  /* YPos */
            2*curXValue,        /* Length */
            LCD_DrawDirection_Vertical,
            Layer);
        LCD_DrawStraightLineOnLayer(
            Color,
            XPosAdjusted + curYValue,  /* XPos */
            YPosAdjusted - curXValue,  /* YPos */
            2*curXValue,        /* Length */
            LCD_DrawDirection_Vertical,
            Layer);
      }

      if (decisionVariable < 0)
      {
        decisionVariable += (curXValue << 2) + 6;
      }
      else
      {
        decisionVariable += ((curXValue - curYValue) << 2) + 10;
        curYValue--;
      }
      curXValue++;
    }

    LCD_DrawCircleOnLayer(Color, XPos, YPos, Radius, Layer);
  }
}

/**
 * @brief
 * @param  Color: Color to use, format ARGB8888, 32 bits
 * @param  XPos: x-coordinate
 * @param  YPos: y-coordinate
 * @param  Image: pointer to the ALPHA_IMAGE
 * @param  Layer: Layer to draw on
 * @retval  None
 */
void LCD_DrawAlphaImageOnLayer(uint16_t XPos, uint16_t YPos, uint32_t Color, ALPHA_IMAGE* Image, LCD_LAYER Layer)
{
  if (IS_VALID_LAYER(Layer))
  {
    /* Sanity check */
    if (Image == 0 || Image->Width == 0 || Image->Height == 0)
    {
      prvErrorHandler("LCD_DrawARGB8888ImageOnLayer-Image is invalid");
      return;
    }
    else if (XPos + Image->Width > LCD_PIXEL_WIDTH || YPos + Image->Height > LCD_PIXEL_HEIGHT)
    {
      prvErrorHandler("LCD_DrawAlphaImageOnLayer-Image does not fit on display");
      return;
    }

    /* Try to take the binary semaphore */
    if (xSemaphoreTake(xSemaphoreDma2d, 100) == pdTRUE)
    {
      /* Configure the DMA2D Mode, Color Mode and line output offset */
      DMA2DHandle.Init.Mode         = DMA2D_M2M_BLEND;
      DMA2DHandle.Init.ColorMode    = DMA2D_ARGB8888;
      DMA2DHandle.Init.OutputOffset = LCD_PIXEL_WIDTH - Image->Width;

      /* Configure the foreground -> Image */
      DMA2DHandle.LayerCfg[1].AlphaMode       = DMA2D_NO_MODIF_ALPHA;
      DMA2DHandle.LayerCfg[1].InputAlpha      = Color;
      DMA2DHandle.LayerCfg[1].InputColorMode  = CM_A8;
      DMA2DHandle.LayerCfg[1].InputOffset     = 0;

      /* Configure the background -> Layer */
      DMA2DHandle.LayerCfg[0].AlphaMode       = DMA2D_NO_MODIF_ALPHA;
      DMA2DHandle.LayerCfg[0].InputAlpha      = 0x00;
      DMA2DHandle.LayerCfg[0].InputColorMode  = CM_ARGB8888;
      DMA2DHandle.LayerCfg[0].InputOffset     = LCD_PIXEL_WIDTH - Image->Width;

      /* Configure source address */
      uint32_t outputMemoryAddress;
      if (Layer == LCD_LAYER_1)
        outputMemoryAddress = LCD_LAYER_1_ADDRESS + 4*(XPos + YPos*LCD_PIXEL_WIDTH);
      else if (Layer == LCD_LAYER_2)
        outputMemoryAddress = LCD_LAYER_2_ADDRESS + 4*(XPos + YPos*LCD_PIXEL_WIDTH);
      else if (Layer == LCD_LAYER_3)
        outputMemoryAddress = LCD_LAYER_3_ADDRESS + 4*(XPos + YPos*LCD_PIXEL_WIDTH);
      else
        return;

      /* Init the DMA2D */
      HAL_StatusTypeDef status;
      status = HAL_DMA2D_Init(&DMA2DHandle);
      if (status != HAL_OK)
      {
         prvErrorHandler("");
         return;
      }
      /* Config the foreground layer */
      status = HAL_DMA2D_ConfigLayer(&DMA2DHandle, 1);
      if (status != HAL_OK)
      {
         prvErrorHandler("");
         return;
      }
      /* Config the background layer */
      status = HAL_DMA2D_ConfigLayer(&DMA2DHandle, 0);
      if (status != HAL_OK)
      {
         prvErrorHandler("");
         return;
      }

      /* Start the transfer in interrupt mode */
      HAL_DMA2D_BlendingStart_IT(&DMA2DHandle, (uint32_t)Image->DataTable, outputMemoryAddress, outputMemoryAddress, Image->Width, Image->Height);
    }
  }
}

/**
 * @brief
 * @param  XPos: x-coordinate
 * @param  YPos: y-coordinate
 * @param  Image: pointer to the ARGB8888_IMAGE
 * @param  Layer: Layer to draw on
 * @retval  None
 */
void LCD_DrawARGB8888ImageOnLayer(uint16_t XPos, uint16_t YPos, ARGB8888_IMAGE* Image, LCD_LAYER Layer)
{
  if (IS_VALID_LAYER(Layer))
  {
    /* Sanity check */
    if (Image == 0 || Image->Width == 0 || Image->Height == 0)
    {
      prvErrorHandler("LCD_DrawARGB8888ImageOnLayer-Image is invalid");
      return;
    }
    else if (XPos + Image->Width > LCD_PIXEL_WIDTH || YPos + Image->Height > LCD_PIXEL_HEIGHT)
    {
      prvErrorHandler("LCD_DrawARGB8888ImageOnLayer-Image does not fit on display");
      return;
    }

    /* Try to take the binary semaphore */
    if (xSemaphoreTake(xSemaphoreDma2d, 100) == pdTRUE)
    {
      /* Configure the DMA2D Mode, Color Mode and line output offset */
      DMA2DHandle.Init.Mode         = DMA2D_M2M_BLEND;
      DMA2DHandle.Init.ColorMode    = DMA2D_ARGB8888;
      DMA2DHandle.Init.OutputOffset = LCD_PIXEL_WIDTH - Image->Width;

      /* Configure the foreground -> Image */
      DMA2DHandle.LayerCfg[1].AlphaMode       = DMA2D_NO_MODIF_ALPHA;
      DMA2DHandle.LayerCfg[1].InputAlpha      = 0x00;
      DMA2DHandle.LayerCfg[1].InputColorMode  = CM_ARGB8888;
      DMA2DHandle.LayerCfg[1].InputOffset     = 0;

      /* Configure the background -> Layer */
      DMA2DHandle.LayerCfg[0].AlphaMode       = DMA2D_NO_MODIF_ALPHA;
      DMA2DHandle.LayerCfg[0].InputAlpha      = 0x00;
      DMA2DHandle.LayerCfg[0].InputColorMode  = CM_ARGB8888;
      DMA2DHandle.LayerCfg[0].InputOffset     = LCD_PIXEL_WIDTH - Image->Width;

      /* Configure source address */
      uint32_t outputMemoryAddress;
      if (Layer == LCD_LAYER_1)
        outputMemoryAddress = LCD_LAYER_1_ADDRESS + 4*(XPos + YPos*LCD_PIXEL_WIDTH);
      else if (Layer == LCD_LAYER_2)
        outputMemoryAddress = LCD_LAYER_2_ADDRESS + 4*(XPos + YPos*LCD_PIXEL_WIDTH);
      else if (Layer == LCD_LAYER_3)
        outputMemoryAddress = LCD_LAYER_3_ADDRESS + 4*(XPos + YPos*LCD_PIXEL_WIDTH);
      else
        return;

      /* Init the DMA2D */
      HAL_StatusTypeDef status;
      status = HAL_DMA2D_Init(&DMA2DHandle);
      if (status != HAL_OK)
      {
         prvErrorHandler("");
         return;
      }
      /* Config the foreground layer */
      status = HAL_DMA2D_ConfigLayer(&DMA2DHandle, 1);
      if (status != HAL_OK)
      {
         prvErrorHandler("");
         return;
      }
      /* Config the background layer */
      status = HAL_DMA2D_ConfigLayer(&DMA2DHandle, 0);
      if (status != HAL_OK)
      {
         prvErrorHandler("");
         return;
      }

      /* Start the transfer in interrupt mode */
      HAL_DMA2D_BlendingStart_IT(&DMA2DHandle, (uint32_t)Image->DataTable, outputMemoryAddress, outputMemoryAddress, Image->Width, Image->Height);
    }
  }
}

/**
 * @brief
 * @param  XPos: x-coordinate
 * @param  YPos: y-coordinate
 * @param  Width:
 * @param  Height:
 * @param  BufferStartAddress: Address to the buffer
 * @param  Layer: Layer to draw on
 * @retval  None
 */
void LCD_DrawARGB8888BufferOnLayer(uint16_t XPos, uint16_t YPos, uint16_t Width, uint16_t Height, uint32_t BufferStartAddress, LCD_LAYER Layer)
{
  if (IS_VALID_LAYER(Layer))
  {
    /* Sanity check */
    if (BufferStartAddress == 0 || Width == 0 || Height == 0)
    {
      prvErrorHandler("LCD_DrawARGB8888BufferOnLayer-Image is invalid");
      return;
    }
    else if (XPos + Width > LCD_PIXEL_WIDTH || YPos + Height > LCD_PIXEL_HEIGHT)
    {
      prvErrorHandler("LCD_DrawARGB8888BufferOnLayer-Image does not fit on display");
      return;
    }

    /* Try to take the binary semaphore */
    if (xSemaphoreTake(xSemaphoreDma2d, 100) == pdTRUE)
    {
      /* Configure the DMA2D Mode, Color Mode and line output offset */
      DMA2DHandle.Init.Mode         = DMA2D_M2M_BLEND;
      DMA2DHandle.Init.ColorMode    = DMA2D_ARGB8888;
      DMA2DHandle.Init.OutputOffset = LCD_PIXEL_WIDTH - Width;

      /* Configure the foreground -> Image */
      DMA2DHandle.LayerCfg[1].AlphaMode       = DMA2D_NO_MODIF_ALPHA;
      DMA2DHandle.LayerCfg[1].InputAlpha      = 0x00;
      DMA2DHandle.LayerCfg[1].InputColorMode  = CM_ARGB8888;
      DMA2DHandle.LayerCfg[1].InputOffset     = LCD_PIXEL_WIDTH - Width;

      /* Configure the background -> Layer */
      DMA2DHandle.LayerCfg[0].AlphaMode       = DMA2D_NO_MODIF_ALPHA;
      DMA2DHandle.LayerCfg[0].InputAlpha      = 0x00;
      DMA2DHandle.LayerCfg[0].InputColorMode  = CM_ARGB8888;
      DMA2DHandle.LayerCfg[0].InputOffset     = LCD_PIXEL_WIDTH - Width;

      /* Configure source address */
      uint32_t outputMemoryAddress;
      if (Layer == LCD_LAYER_1)
        outputMemoryAddress = LCD_LAYER_1_ADDRESS + 4*(XPos + YPos*LCD_PIXEL_WIDTH);
      else if (Layer == LCD_LAYER_2)
        outputMemoryAddress = LCD_LAYER_2_ADDRESS + 4*(XPos + YPos*LCD_PIXEL_WIDTH);
      else if (Layer == LCD_LAYER_3)
        outputMemoryAddress = LCD_LAYER_3_ADDRESS + 4*(XPos + YPos*LCD_PIXEL_WIDTH);
      else
        return;

      /* Init the DMA2D */
      HAL_StatusTypeDef status;
      status = HAL_DMA2D_Init(&DMA2DHandle);
      if (status != HAL_OK)
      {
         prvErrorHandler("");
         return;
      }
      /* Config the foreground layer */
      status = HAL_DMA2D_ConfigLayer(&DMA2DHandle, 1);
      if (status != HAL_OK)
      {
         prvErrorHandler("");
         return;
      }
      /* Config the background layer */
      status = HAL_DMA2D_ConfigLayer(&DMA2DHandle, 0);
      if (status != HAL_OK)
      {
         prvErrorHandler("");
         return;
      }

      /* Start the transfer in interrupt mode */
      HAL_DMA2D_BlendingStart_IT(&DMA2DHandle, BufferStartAddress, outputMemoryAddress, outputMemoryAddress, Width, Height);
    }
  }
}

/** Private functions .-------------------------------------------------------*/

/**
  * @brief  GPIO config for LTDC.
  * @param  None
  * @retval None
  */
static void prvGPIOConfig()
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIO clocks */
  __GPIOA_CLK_ENABLE();
  __GPIOB_CLK_ENABLE();
  __GPIOC_CLK_ENABLE();
  __GPIOD_CLK_ENABLE();
  __GPIOF_CLK_ENABLE();
  __GPIOG_CLK_ENABLE();

  /* GPIOs Configuration */
  /*
  +------------------------+-----------------------+----------------------------+
  +                       LCD pins assignment                                   +
  +------------------------+-----------------------+----------------------------+
  |  LCD_TFT R2 <-> PC.10  |  LCD_TFT G2 <-> PA.06 |  LCD_TFT B2 <-> PD.06      |
  |  LCD_TFT R3 <-> PB.00  |  LCD_TFT G3 <-> PG.10 |  LCD_TFT B3 <-> PG.11      |
  |  LCD_TFT R4 <-> PA.11  |  LCD_TFT G4 <-> PB.10 |  LCD_TFT B4 <-> PG.12      |
  |  LCD_TFT R5 <-> PA.12  |  LCD_TFT G5 <-> PB.11 |  LCD_TFT B5 <-> PA.03      |
  |  LCD_TFT R6 <-> PB.01  |  LCD_TFT G6 <-> PC.07 |  LCD_TFT B6 <-> PB.08      |
  |  LCD_TFT R7 <-> PG.06  |  LCD_TFT G7 <-> PD.03 |  LCD_TFT B7 <-> PB.09      |
  -------------------------------------------------------------------------------
      |  LCD_TFT HSYNC <-> PC.06  | LCDTFT VSYNC <->  PA.04 |
      |  LCD_TFT CLK   <-> PG.07  | LCD_TFT DE   <->  PF.10 |
       -----------------------------------------------------
  */

  /* Common GPIO configuration */
  GPIO_InitStructure.Mode       = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull       = GPIO_NOPULL;
  GPIO_InitStructure.Speed      = GPIO_SPEED_HIGH;  /* TODO: Use fast? */
  GPIO_InitStructure.Alternate  = GPIO_AF14_LTDC;

  /* GPIOA configuration */
  GPIO_InitStructure.Pin  = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_6 |
                            GPIO_PIN_11 | GPIO_PIN_12;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* GPIOB configuration */
  GPIO_InitStructure.Pin  = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
                            GPIO_PIN_11;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* GPIOC configuration */
  GPIO_InitStructure.Pin  = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_10;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* GPIOD configuration */
  GPIO_InitStructure.Pin  = GPIO_PIN_3 | GPIO_PIN_6;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

  /* GPIOF configuration */
  GPIO_InitStructure.Pin  = GPIO_PIN_10;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStructure);

  /* GPIOG configuration */
  GPIO_InitStructure.Pin  = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_10 |
                            GPIO_PIN_11 | GPIO_PIN_12;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);


  /* Two pins use another alternate function value */
  GPIO_InitStructure.Alternate  = GPIO_AF9_LTDC;

  /* GPIOB configuration */
  GPIO_InitStructure.Pin  = GPIO_PIN_0 | GPIO_PIN_1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/**
 * @brief  Error handler to handle various errors
 * @param  None
 * @retval  None
 */
static void prvErrorHandler(char* ErrorString)
{
  /* TODO: Handle the error string */
#ifdef DEBUG
  while (1);
#endif
}

/** Interrupt Callbacks ------------------------------------------------------*/
/**
 * @brief   Callback for DMA2D transfer complete
 * @param   None
 * @retval  None
 */
static void prvTransferComplete(DMA2D_HandleTypeDef *hdma2d)
{
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

  /* Give back the binary semaphore */
  xSemaphoreGiveFromISR(xSemaphoreDma2d, &xHigherPriorityTaskWoken);

  /* If xHigherPriorityTaskWoken was set to true you we should yield */
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * @brief   Callback for DMA2D transfer error
 * @param   None
 * @retval  None
 */
static void prvTransferError(DMA2D_HandleTypeDef *hdma2d)
{
  /* TODO: Handle the error */
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

  /* Give back the binary semaphore */
  xSemaphoreGiveFromISR(xSemaphoreDma2d, &xHigherPriorityTaskWoken);

  /* If xHigherPriorityTaskWoken was set to true you we should yield */
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
