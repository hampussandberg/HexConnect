/**
 *******************************************************************************
 * @file    simple_gui.c
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-08-29
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
#include "simple_gui.h"


/** Private defines ----------------------------------------------------------*/
#define DIRTY_ZONE_COUNT    (1) /* Set to 1 to test problems with dirty zones */
#define X_DIRTY_ZONE_SIZE   (LCD_PIXEL_WIDTH / DIRTY_ZONE_COUNT)
#define Y_DIRTY_ZONE_SIZE   (LCD_PIXEL_HEIGHT / DIRTY_ZONE_COUNT)

#define DIRTY_ZONE_SEMAPHORE_TIMEOUT  50

/** Private typedefs ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
static GUIButton prvButton_list[guiConfigNUMBER_OF_BUTTONS];
static GUILabel prvLabel_list[guiConfigNUMBER_OF_LABELS];
static GUIStaticTextBox prvStaticTextBox_list[guiConfigNUMBER_OF_STATIC_TEXT_BOXES];

#ifdef GUI_SCROLLABLE_TEXT_BOX_ENABLED
static GUIScrollableTextBox prvScrollableTextBox_list[guiConfigNUMBER_OF_SCROLLABLE_TEXT_BOXES];
#endif

#ifdef GUI_ALERT_BOX_ENABLED
static GUIAlertBox prvAlertBox_list[guiConfigNUMBER_OF_ALERT_BOXES];
#endif

#ifdef GUI_BUTTON_GRID_BOX_ENABLED
static GUIButtonGridBox prvButtonGridBox_list[guiConfigNUMBER_OF_BUTTON_GRID_BOXES];
#endif

#ifdef GUI_BUTTON_LIST_ENABLED
static GUIButtonList prvButtonList_list[guiConfigNUMBER_OF_BUTTON_LISTS];
#endif

#ifdef GUI_INFO_BOX_ENABLED
static GUIInfoBox prvInfoBox_list[guiConfigNUMBER_OF_INFO_BOXES];
#endif

static GUIContainer prvContainer_list[guiConfigNUMBER_OF_CONTAINERS];

static GUILayer prvCurrentlyActiveLayer = GUILayer_Invalid;
static uint32_t prvObjectsOnLayer[GUI_NUM_OF_LAYERS];

static bool prvDirtyZones[DIRTY_ZONE_COUNT][DIRTY_ZONE_COUNT];
static bool prvNoDirtyZones = false;

static char prvTempString[GUI_MAX_CHARACTERS_PER_ROW + 1] = {0};

/* Semaphore for handling dirty zones */
static SemaphoreHandle_t xSemaphoreDirtyZones;

/** Private function prototypes ----------------------------------------------*/
static void prvErrorHandler(char* ErrorString);
static void prvMarkDirtyZonesWithObject(GUIObject* Object);

static inline bool prvPointIsInsideObject(uint16_t XPos, uint16_t YPos, GUIObject* Object);
static bool prvOjectIsInsideObject(GUIObject SmallObject, GUIObject BigObject);

/** Functions ----------------------------------------------------------------*/
/**
 * @brief    Initializes the GUI by setting the items in the lists to appropriate values
 * @param    None
 * @retval  None
 */
void GUI_Init()
{
  /* Semaphore for handling dirty zones */
  xSemaphoreDirtyZones = xSemaphoreCreateMutex();

  /* Buttons */
  for (uint32_t i = 0; i < guiConfigNUMBER_OF_BUTTONS; i++)
    memset(&prvButton_list[i], 0, sizeof(GUIButton));
  /* Labels */
  for (uint32_t i = 0; i < guiConfigNUMBER_OF_LABELS; i++)
    memset(&prvLabel_list[i], 0, sizeof(GUILabel));
  /* Static text boxes */
  for (uint32_t i = 0; i < guiConfigNUMBER_OF_STATIC_TEXT_BOXES; i++)
    memset(&prvStaticTextBox_list[i], 0, sizeof(GUIStaticTextBox));
#ifdef GUI_ALERT_BOX_ENABLED
  /* Alert boxes */
  for (uint32_t i = 0; i < guiConfigNUMBER_OF_ALERT_BOXES; i++)
    memset(&prvAlertBox_list[i], 0, sizeof(GUIAlertBox));
#endif
#ifdef GUI_BUTTON_GRID_BOX_ENABLED
  /* Button grid boxes */
  for (uint32_t i = 0; i < guiConfigNUMBER_OF_BUTTON_GRID_BOXES; i++)
    memset(&prvButtonGridBox_list[i], 0, sizeof(GUIButtonGridBox));
#endif
  /* Containers */
  for (uint32_t i = 0; i < guiConfigNUMBER_OF_CONTAINERS; i++)
    memset(&prvContainer_list[i], 0, sizeof(GUIContainer));

  /* Clear buffer and all layers */
  LCD_ClearScreenBuffer(0x0000);
  LCD_ClearLayer(COLOR_TRANSPARENT, GUILayer_1);
  LCD_ClearLayer(COLOR_TRANSPARENT, GUILayer_2);
  LCD_ClearLayer(COLOR_TRANSPARENT, GUILayer_3);

  /* Set layer 1 as active */
  GUI_SetActiveLayer(GUILayer_1);

  /* Reset number of objects on each layer to 0 */
  for (uint32_t i = 0; i < GUI_NUM_OF_LAYERS; i++)
    prvObjectsOnLayer[i] = 0;
}

/**
 * @brief    Draw the border on an object
 * @param    Object: Pointer to the object to draw for
 * @retval  None
 */
void GUI_DrawBorderRaw(GUIObject* Object)
{
  /* Draw the border */
  if (Object->border & GUIBorder_Left)
  {
    LCD_DrawFilledRectangleOnLayer(Object->borderColor,   /* Color */
        Object->xPos,        /* X position */
        Object->yPos,        /* Y position */
        Object->borderThickness,  /* Width */
        Object->height,        /* Height */
        Object->layer);        /* Layer */
  }
  if (Object->border & GUIBorder_Right)
  {
    LCD_DrawFilledRectangleOnLayer(Object->borderColor,   /* Color */
        Object->xPos + Object->width - Object->borderThickness,  /* X position */
        Object->yPos,        /* Y position */
        Object->borderThickness,  /* Width */
        Object->height,        /* Height */
        Object->layer);        /* Layer */
  }
  if (Object->border & GUIBorder_Top)
  {
    LCD_DrawFilledRectangleOnLayer(Object->borderColor,   /* Color */
        Object->xPos,        /* X position */
        Object->yPos,        /* Y position */
        Object->width,        /* Width */
        Object->borderThickness,  /* Height */
        Object->layer);        /* Layer */
  }
  if (Object->border & GUIBorder_Bottom)
  {
    LCD_DrawFilledRectangleOnLayer(Object->borderColor,   /* Color */
        Object->xPos,        /* X position */
        Object->yPos + Object->height - Object->borderThickness,  /* Y position */
        Object->width,        /* Width */
        Object->borderThickness,  /* Height */
        Object->layer);        /* Layer */
  }
}

/**
 * @brief  Draw all layers to the display buffer. This will take into account the active layer,
 *       as there's no point in drawing the above layers that are not active.
 * @param  None
 * @retval  None
 */
void GUI_DrawAllLayersToDisplayBuffer()
{
  /* Start by clearing the buffer */
  LCD_ClearScreenBuffer(0x0000);
  /* Always draw layer 1 */
  LCD_DrawLayerToBuffer(GUILayer_1);
  if (prvObjectsOnLayer[GUILayer_2] != 0)
    LCD_DrawLayerToBuffer(GUILayer_2);
  if (prvObjectsOnLayer[GUILayer_3] != 0)
    LCD_DrawLayerToBuffer(GUILayer_3);

  /* Try to take the dirty zone semaphore so that the zones are not changed while refreshing */
  if (xSemaphoreTake(xSemaphoreDirtyZones, DIRTY_ZONE_SEMAPHORE_TIMEOUT) == pdTRUE)
  {
    /* Now that we have drawn all layers to buffer no zone is dirty */
    for (uint32_t x = 0; x < DIRTY_ZONE_COUNT; x++)
      for (uint32_t y = 0; y < DIRTY_ZONE_COUNT; y++)
        prvDirtyZones[x][y] = false;
    prvNoDirtyZones = true;

    /* Give back the semaphore */
    xSemaphoreGive(xSemaphoreDirtyZones);
  }
}

/**
 * @brief  Refresh the display that the user sees
 * @param  None
 * @retval  None
 */
void GUI_RefreshDisplay()
{
  LCD_RefreshActiveDisplay();
}

/**
 * @brief  Draw all layers and refresh the display at the same time. Simpler than calling both function above one after the other.
 * @param  None
 * @retval  None
 */
void GUI_DrawAllLayersAndRefreshDisplay()
{
  GUI_DrawAllLayersToDisplayBuffer();
  GUI_RefreshDisplay();
}

/**
 * @brief  Refresh only the zones that are dirty
 * @param  None
 * @retval  None
 */
void GUI_DrawAndRefreshDirtyZones()
{
  /* Try to take the dirty zone semaphore so that the zones are not changed while refreshing */
  if (xSemaphoreDirtyZones != NULL && xSemaphoreTake(xSemaphoreDirtyZones, DIRTY_ZONE_SEMAPHORE_TIMEOUT) == pdTRUE)
  {
    /* If there are no dirty zones we don't have to check this */
    if (prvNoDirtyZones == false)
    {
      uint32_t xIndex, yIndex;
      bool dirtyZonesFound = false;

      /* Check all available dirty zones, DIRTY_ZONE_COUNT^2 in total */
      for (xIndex = 0; xIndex < DIRTY_ZONE_COUNT; xIndex++)
      {
        for (yIndex = 0; yIndex < DIRTY_ZONE_COUNT; yIndex++)
        {
          if (prvDirtyZones[xIndex][yIndex] == true)
          {
            uint16_t xPos = xIndex*X_DIRTY_ZONE_SIZE;
            uint16_t yPos = yIndex*Y_DIRTY_ZONE_SIZE;
            uint16_t width = X_DIRTY_ZONE_SIZE;
            uint16_t height = Y_DIRTY_ZONE_SIZE;

            /* Always draw the first layer */
            LCD_DrawPartOfLayerToBuffer(GUILayer_1,  xPos, yPos, width, height);
            /* Draw the dirty zone in layers that have objects in them to the buffer */
            if (prvObjectsOnLayer[GUILayer_2] != 0)
              LCD_DrawPartOfLayerToBuffer(GUILayer_2,  xPos, yPos, width, height);
            if (prvObjectsOnLayer[GUILayer_3] != 0)
              LCD_DrawPartOfLayerToBuffer(GUILayer_3,  xPos, yPos, width, height);

            /* Mark the zone as clean */
            prvDirtyZones[xIndex][yIndex] = false;
            /* Mark that a dirty zone has been found so that we can refresh the display later */
            dirtyZonesFound = true;
  /* DEBUG */
  #if 0
//            LCD_RefreshActiveDisplay();
  #endif
          }
        }
      }

      /* Now that we have updated all dirty zones we can mark all as clean */
      prvNoDirtyZones = true;
      /* Give back the semaphore */
      xSemaphoreGive(xSemaphoreDirtyZones);

      /* Refresh the display if dirty zones were found */
      if (dirtyZonesFound == true)
      {
        LCD_RefreshActiveDisplay();
      }
    }
    else
    {
      /* Give back the semaphore */
      xSemaphoreGive(xSemaphoreDirtyZones);
    }
  }
}

/**
  * @brief  Fill a layer with a color
  * @param  Color: Color to clear the layer with
  * @param  Layer: The layer to fill, can be any value of GUILayer
  * @retval None
  */
void GUI_ClearLayer(guiColor Color, GUILayer Layer)
{
  LCD_ClearLayer(Color, Layer);
}

/**
  * @brief  Set which layer should be active. This affects which elements on the screen that react to
  *     to touches from the user.
  * @param  NewActiveLayer: The new layer to set as active, can be any value of GUILayer
  * @retval GUIStatus_Success if everything went OK
  * @retval  GUIStatus_Error if something went wrong
  */
GUIStatus GUI_SetActiveLayer(GUILayer NewActiveLayer)
{
  /* Make sure the new active layer is different from the old and that it's valid */
  if (prvCurrentlyActiveLayer != NewActiveLayer && (NewActiveLayer == GUILayer_1 || NewActiveLayer == GUILayer_2 || NewActiveLayer == GUILayer_3))
  {
    prvCurrentlyActiveLayer = NewActiveLayer;
    return GUIStatus_Success;
  }
  else
  {
    prvErrorHandler("GUI_SetActiveLayer-Error with NewActiveLayer");
    return GUIStatus_Error;
  }
}

/**
  * @brief  Increments the number of objects on given layer safely
  * @param  Layer: The layer to increment for, can be any value of GUILayer
  * @retval GUIStatus_Success if everything went OK
  * @retval  GUIStatus_Error if something went wrong
  */
GUIStatus GUI_IncrementObjectsOnLayer(GUILayer Layer)
{
  if (Layer == GUILayer_1 || Layer == GUILayer_2 || Layer == GUILayer_3)
  {
    if (prvObjectsOnLayer[Layer] < GUI_MAX_OBJECTS_PER_LAYER)
    {
      /* If we had zero objects in this layer and it is above the currently active layer we should activate this new layer */
      if (prvObjectsOnLayer[Layer] == 0 && Layer > prvCurrentlyActiveLayer)
      {
        GUI_SetActiveLayer(Layer);
      }
      prvObjectsOnLayer[Layer]++;
      return GUIStatus_Success;
    }
    else
    {
      prvErrorHandler("GUI_IncrementObjectsOnLayer-Reached max objects per layer");
      return GUIStatus_Error;
    }
  }
  else
  {
    prvErrorHandler("GUI_IncrementObjectsOnLayer-Invalid Layer");
    return GUIStatus_Error;
  }
}

/**
  * @brief  Decrements the number of objects on given layer safely
  * @param  Layer: The layer to decrement for, can be any value of GUILayer
  * @retval GUIStatus_Success if everything went OK
  * @retval  GUIStatus_Error if something went wrong
  */
GUIStatus GUI_DecrementObjectsOnLayer(GUILayer Layer)
{
  if (Layer == GUILayer_1 || Layer == GUILayer_2 || Layer == GUILayer_3)
  {
    if (prvObjectsOnLayer[Layer] != 0)
    {
      prvObjectsOnLayer[Layer]--;

      /* If we now have zero objects in this layer we should set the active layer to a lower one */
      if (prvObjectsOnLayer[Layer] == 0 && Layer != GUILayer_1)
      {
        GUI_SetActiveLayer(Layer - 1);
      }
      return GUIStatus_Success;
    }
    else
    {
      prvErrorHandler("GUI_DecrementObjectsOnLayer-No objects to decrement");
      return GUIStatus_Error;
    }
  }
  else
  {
    prvErrorHandler("GUI_DecrementObjectsOnLayer-Invalid Layer");
    return GUIStatus_Error;
  }
}

/**
 * @brief   This function should be called by the touch driver that is used by the application.
 *          It will go through active elements on the screen and do the appropriate action.
 * @param   GUITouchEvent: The event that happened, can be any value of GUITouchEvent
 * @param   XPos: X-position for event
 * @param   XPos: Y-position for event
 * @retval  None
 */
void GUI_TouchAtPosition(GUITouchEvent Event, uint16_t XPos, uint16_t YPos)
{
  /* Go trough all the buttons */
  for (uint32_t index = 0; index < guiConfigNUMBER_OF_BUTTONS; index++)
  {
    GUIButton* button = &prvButton_list[index];

    /* Check if the button is not hidden and enabled and if it's hit */
    if (button->object.layer == prvCurrentlyActiveLayer &&
        button->object.displayState == GUIDisplayState_NotHidden &&
        button->buttonState != GUIButtonState_NoState &&
        button->buttonState != GUIButtonState_DisabledTouch &&
        prvPointIsInsideObject(XPos, YPos, &button->object))
    {
      GUButton_TouchAtPosition(button, Event, XPos, YPos);
      /* Return as only one object can be touched */
      /* TODO: Support multi-touch? */
      return;
    }
  }
  /* No button found */
  GUButton_TouchAtPosition(0, GUITouchEvent_None, 0, 0);

#ifdef GUI_ALERT_BOX_ENABLED
  /* Go trough all the alert boxes */
  for (uint32_t index = 0; index < guiConfigNUMBER_OF_ALERT_BOXES; index++)
  {
    GUIAlertBox* alertBox = &prvAlertBox_list[index];

    /* Check if the alert box is not hidden and enabled and if it's hit */
    if (alertBox->object.layer == prvCurrentlyActiveLayer &&
        alertBox->object.displayState == GUIDisplayState_NotHidden &&
        prvPointIsInsideObject(XPos, YPos, &alertBox->object))
    {
      GUAlertBox_TouchAtPosition(alertBox, Event, XPos, YPos);
      /* Return as only one object can be touched */
      /* TODO: Support multi-touch? */
      return;
    }
  }
  /* No alert box found */
  GUAlertBox_TouchAtPosition(0, GUITouchEvent_None, 0, 0);
#endif

#ifdef GUI_BUTTON_GRID_BOX_ENABLED
  /* Go trough all the button grid boxes */
  for (uint32_t index = 0; index < guiConfigNUMBER_OF_BUTTON_GRID_BOXES; index++)
  {
    GUIButtonGridBox* buttonGridBox = &prvButtonGridBox_list[index];

    /* Check if the button grid box is not hidden and enabled and if it's hit */
    if (buttonGridBox->object.layer == prvCurrentlyActiveLayer &&
        buttonGridBox->object.displayState == GUIDisplayState_NotHidden &&
        prvPointIsInsideObject(XPos, YPos, &buttonGridBox->object))
    {
      GUButtonGridBox_TouchAtPosition(buttonGridBox, Event, XPos, YPos);
      /* Return as only one object can be touched */
      /* TODO: Support multi-touch? */
      return;
    }
  }
  /* No button grid box found */
  GUButtonGridBox_TouchAtPosition(0, GUITouchEvent_None, 0, 0);
#endif

#ifdef GUI_BUTTON_LIST_ENABLED
  /* Go trough all the button lists */
  for (uint32_t index = 0; index < guiConfigNUMBER_OF_BUTTON_LISTS; index++)
  {
    GUIButtonList* buttonList = &prvButtonList_list[index];

    /* Check if the button list is not hidden and enabled and if it's hit */
    if (buttonList->object.layer == prvCurrentlyActiveLayer &&
        buttonList->object.displayState == GUIDisplayState_NotHidden &&
        prvPointIsInsideObject(XPos, YPos, &buttonList->object))
    {
      GUButtonList_TouchAtPosition(buttonList, Event, XPos, YPos);
      /* Return as only one object can be touched */
      /* TODO: Support multi-touch? */
      return;
    }
  }
  /* No button list found */
  GUButtonList_TouchAtPosition(0, GUITouchEvent_None, 0, 0);
#endif

#ifdef GUI_INFO_BOX_ENABLED
  /* Go trough all the info boxes */
  for (uint32_t index = 0; index < guiConfigNUMBER_OF_INFO_BOXES; index++)
  {
    GUIInfoBox* infoBox = &prvInfoBox_list[index];

    /* Check if the alert box is not hidden and enabled and if it's hit */
    if (infoBox->object.layer == prvCurrentlyActiveLayer &&
      infoBox->object.displayState == GUIDisplayState_NotHidden &&
      prvPointIsInsideObject(XPos, YPos, &infoBox->object))
    {
      GUInfoBox_TouchAtPosition(infoBox, Event, XPos, YPos);
      /* Return as only one object can be touched */
      /* TODO: Support multi-touch? */
      return;
    }
  }
  /* No info box found */
  GUInfoBox_TouchAtPosition(0, GUITouchEvent_None, 0, 0);
#endif

}


/* Object --------------------------------------------------------------------*/
/**
 * @brief  Clears a label by drawing a transparent rectangle in it's place instead
 * @param  LabelId: The Id for the label
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 */
void GUIObject_ClearRaw(GUIObject* Object)
{
  /* Draw the background rectangle, COLOR_BACKGROUND if layer 1 otherwise transparent */
  guiColor color;
  if (Object->layer == GUILayer_1)
    color = COLOR_BACKGROUND;
  else
    color = COLOR_TRANSPARENT;
  LCD_DrawFilledRectangleOnLayer(color,
                  Object->xPos, Object->yPos,
                  Object->width, Object->height,
                  Object->layer);
  /* Mark dirty zones */
  prvMarkDirtyZonesWithObject(Object);

  /* Set it as hidden now that we have cleared it away */
  Object->displayState = GUIDisplayState_Hidden;
}

/* Button --------------------------------------------------------------------*/
/**
 * @brief  Get a pointer to the button corresponding to the id
 * @param  ButtonId: The Id for the button
 * @retval  Pointer the item or 0 if no item was found
 */
GUIButton* GUIButton_GetFromId(uint32_t ButtonId)
{
  uint32_t index = ButtonId - guiConfigBUTTON_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_BUTTONS && prvButton_list[index].object.id != GUI_INVALID_ID)
    return &prvButton_list[index];
  else
  {
    prvErrorHandler("GUIButton_GetFromId-Invalid ID");
    return 0;
  }
}

/**
 * @brief  Init a button and add it to the button list
 * @param  Button: Pointer to a GUIButton struct which data should be copied from
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 * @retval  GUIStatus_Error: If the item was hidden or something else went wrong
 */
GUIStatus GUIButton_Init(GUIButton* Button)
{
  uint32_t index = Button->object.id - guiConfigBUTTON_ID_OFFSET;
  GUIStatus status = GUIStatus_Success;

  /* Make sure we don't try to create more items than there's room for in the list */
  if (index < guiConfigNUMBER_OF_BUTTONS)
  {
    /* Copy the item to the list */
    memcpy(&prvButton_list[index], Button, sizeof(GUIButton));

    /* Get a pointer to the item */
    GUIButton* button = &prvButton_list[index];

    /* Init the item */
    GUIButton_InitRaw(button);
  }
  else
  {
    prvErrorHandler("GUIButton_Init-Invalid ID");
    status = GUIStatus_InvalidId;
  }

  /* Set all the data in the item we received as a parameter to 0 so that it can be reused easily */
//  GUIButton_Reset(Button);

  return status;
}

/**
 * @brief  Init a GUIButton object. This does not make use of the IDs so use with caution.
 * @param  Button: Pointer to a GUIButton struct where data can be found
 */
void GUIButton_InitRaw(GUIButton* Button)
{
  /* Calculate some parameters */
  /* Two rows of text */
  if (Button->text[0] != 0 && Button->text[1] != 0)
  {
    Button->numOfChar[0] = strlen(Button->text[0]);
    Button->textWidth[0] = FONTS_GetTotalWidthForString(Button->text[0], Button->font);
    Button->textHeight[0] = Button->font->Height;

    Button->numOfChar[1] = strlen(Button->text[1]);
    Button->textWidth[1] = FONTS_GetTotalWidthForString(Button->text[1], Button->font);
    Button->textHeight[1] = Button->font->Height;
  }
  /* One row of text */
  else if (Button->text[0] != 0)
  {
    Button->numOfChar[0] = strlen(Button->text[0]);
    Button->textWidth[0] = FONTS_GetTotalWidthForString(Button->text[0], Button->font);
    Button->textHeight[0] = Button->font->Height;
  }
  else
  {
    prvErrorHandler("GUIButton_InitRaw-No text on rows");
  }

  /* Set the last state to the current state */
  Button->lastButtonState = Button->buttonState;

  /* If it's set to not hidden we should draw the button */
  if (Button->object.displayState == GUIDisplayState_NotHidden)
    GUIButton_DrawRaw(Button, true);
}

/**
 * @brief  Resets a button by setting all it's memory to 0
 * @param  AlertBox: Pointer to a GUIAlertBox struct which data should be reset for
 * @retval  None
 */
void GUIButton_Reset(GUIButton* Button)
{
  memset(Button, 0, sizeof(GUIButton));
}


/**
 * @brief  Draw a specific button in the button_list
 * @param  ButtonId: The Id for the button
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 */
GUIStatus GUIButton_Draw(uint32_t ButtonId)
{
  uint32_t index = ButtonId - guiConfigBUTTON_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_BUTTONS &&
    prvButton_list[index].object.id != GUI_INVALID_ID &&
    prvButton_list[index].object.displayState != GUIDisplayState_NotHidden)
  {
    /* Get a pointer to the current item */
    GUIButton* button = &prvButton_list[index];

    /* Draw the item */
    GUIButton_DrawRaw(button, true);

    /* Increment number of objects on this layer if it's not already drawn before */
    if (button->object.displayState != GUIDisplayState_NotHidden)
      GUI_IncrementObjectsOnLayer(button->object.layer);

    return GUIStatus_Success;
  }
  else
  {
    prvErrorHandler("GUIButton_Draw-Invalid ID");
    return GUIStatus_InvalidId;
  }
}

/**
 * @brief  Draw a GUIButton object. This does not make use of the IDs so use with caution.
 * @param  Button: Pointer to a GUIButton struct where data can be found
 * @param  MarkDirtyZones: If set to true it will mark the dirty zones this object hits
 */
void GUIButton_DrawRaw(GUIButton* Button, bool MarkDirtyZones)
{
  /* Set state colors */
  guiColor backgroundColor, textColor;
  switch (Button->buttonState) {
    case GUIButtonState_State1:
      /* Enabled state */
      backgroundColor = Button->state1BackgroundColor;
      textColor = Button->state1TextColor;
      break;
    case GUIButtonState_State2:
      /* Disabled state */
      backgroundColor = Button->state2BackgroundColor;
      textColor = Button->state2TextColor;
      break;
    case GUIButtonState_TouchDown:
      /* Disabled state */
      backgroundColor = Button->pressedBackgroundColor;
      textColor = Button->pressedTextColor;
      break;
    default:
      return;
      break;
  }

  /* Draw the background rectangle */
  LCD_DrawFilledRectangleOnLayer(backgroundColor,
                                 Button->object.xPos, Button->object.yPos,
                                 Button->object.width, Button->object.height,
                                 Button->object.layer);

  /* Draw the text */
  if (Button->text[0] != 0 && Button->text[1] != 0)
  {
    /* Two rows of text */
    uint16_t xPos = Button->object.xPos + (Button->object.width - Button->textWidth[0]) / 2;
    uint16_t yPos = Button->object.yPos + Button->object.height/2 - Button->textHeight[0] - 2;
    LCD_DrawStringOnLayer(textColor, xPos, yPos, Button->text[0], Button->font, Button->object.layer);

    xPos = Button->object.xPos + (Button->object.width - Button->textWidth[1]) / 2;
    yPos = Button->object.yPos + Button->object.height/2 - 2;
    LCD_DrawStringOnLayer(textColor, xPos, yPos, Button->text[1], Button->font, Button->object.layer);
  }
  else if (Button->text[0] != 0)
  {
    /* One row of text */
    uint16_t xPos = Button->object.xPos + (Button->object.width - Button->textWidth[0]) / 2;
    uint16_t yPos = Button->object.yPos + (Button->object.height - Button->textHeight[0]) / 2 - 2;
    LCD_DrawStringOnLayer(textColor, xPos, yPos, Button->text[0], Button->font, Button->object.layer);
  }

  /* Draw the border */
  GUI_DrawBorderRaw(&Button->object);

  /* Mark dirty zones */
  if (MarkDirtyZones)
    prvMarkDirtyZonesWithObject(&Button->object);

  Button->object.displayState = GUIDisplayState_NotHidden;
}

/**
 * @brief   Clears a button by drawing a transparent rectangle in it's place instead
 * @param   ButtonId: The Id for the button
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 */
GUIStatus GUIButton_Clear(uint32_t ButtonId)
{
  uint32_t index = ButtonId - guiConfigBUTTON_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_BUTTONS && prvButton_list[index].object.id != GUI_INVALID_ID)
  {
    /* Get a pointer to the current item */
    GUIButton* button = &prvButton_list[index];

    /* Clear the object */
    GUIObject_ClearRaw(&button->object);

    /* Decrement number of objects on this layer */
    GUI_DecrementObjectsOnLayer(button->object.layer);

    return GUIStatus_Success;
  }
  else
  {
    prvErrorHandler("GUIButton_Clear-Invalid ID");
    return GUIStatus_InvalidId;
  }
}

/**
 * @brief   Set the button state of a button
 * @param   ButtonId: The Id for the button
 * @param   NewState: The new button state
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 */
GUIStatus GUIButton_SetButtonState(uint32_t ButtonId, GUIButtonState NewState)
{
  uint32_t index = ButtonId - guiConfigBUTTON_ID_OFFSET;
  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_BUTTONS && prvButton_list[index].object.id != GUI_INVALID_ID)
  {
    /* Save the current state as the new last state */
    prvButton_list[index].lastButtonState = prvButton_list[index].buttonState;
    /* Update the current state */
    prvButton_list[index].buttonState = NewState;
    /* Draw the button */
    GUIButton_DrawRaw(&prvButton_list[index], true);
    return GUIStatus_Success;
  }
  else
  {
    prvErrorHandler("GUIButton_SetButtonState-Invalid ID");
    return GUIStatus_InvalidId;
  }
}

/**
 * @brief   Get the button state of a button
 * @param   ButtonId: The Id for the button
 * @retval  The state of the button or GUIButtonState_NoState if the Id was invalid
 */
GUIButtonState GUIButton_GetButtonState(uint32_t ButtonId)
{
  uint32_t index = ButtonId - guiConfigBUTTON_ID_OFFSET;
  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_BUTTONS && prvButton_list[index].object.id != GUI_INVALID_ID)
  {
    return prvButton_list[index].buttonState;
  }
  else
  {
    return GUIButtonState_NoState;
  }
}

/**
 * @brief   Get the display state of a button
 * @param   ButtonId: The Id for the button
 * @retval  The display state if valid ID, otherwise GUIDisplayState_NoState
 */
GUIDisplayState GUIButton_GetDisplayState(uint32_t ButtonId)
{
  uint32_t index = ButtonId - guiConfigBUTTON_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_BUTTONS && prvButton_list[index].object.id != GUI_INVALID_ID)
    return prvButton_list[index].object.displayState;
  else
  {
    prvErrorHandler("GUIButton_GetDisplayState-Invalid ID");
    return GUIDisplayState_NoState;
  }
}

/**
 * @brief   Set a new layer for a button
 * @param   ButtonId: The Id for the button
 * @param   NewLayer: The new layer to use for the button
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_Errro: The new layer is the same as the old
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 */
GUIStatus GUIButton_SetLayer(uint32_t ButtonId, GUILayer NewLayer)
{
  uint32_t index = ButtonId - guiConfigBUTTON_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_BUTTONS && prvButton_list[index].object.id != GUI_INVALID_ID)
  {
    /* Check if the new layer is different from the old layer */
    if (prvButton_list[index].object.layer != NewLayer)
    {
      /* Start by clearing the button from it's current layer */
      GUIButton_Clear(ButtonId);
      /* Change the layer of the button */
      prvButton_list[index].object.layer = NewLayer;
      /* Draw the button on the new layer */
      GUIButton_Draw(ButtonId);

      return GUIStatus_Success;
    }
    else
      return GUIStatus_Error;
  }
  else
  {
    prvErrorHandler("GUIButton_SetLayer-Invalid ID");
    return GUIStatus_InvalidId;
  }
}

/**
 * @brief   Get the layer for a button
 * @param   ButtonId: The Id for the button
 * @retval  The layer if valid Id, otherwise GUILayer_Invalid
 */
GUILayer GUIButton_GetLayer(uint32_t ButtonId)
{
  uint32_t index = ButtonId - guiConfigBUTTON_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_BUTTONS && prvButton_list[index].object.id != GUI_INVALID_ID)
    return prvButton_list[index].object.layer;
  else
  {
    prvErrorHandler("GUIButton_GetLayer-Invalid ID");
    return GUILayer_Invalid;
  }
}


/**
 * @brief   Respond to a touch of a button. Will also reset the state inside if GUITouchEvent_None is received
 *          as a parameter for Event. This is to make sure button states are reset correctly
 * @param   Button: The button the event applies to
 * @param   Event: The event that happened, can be any value of GUITouchEvent
 * @param   XPos: X-position for event
 * @param   YPos: Y-position for event
 * @retval  None
 */
void GUButton_TouchAtPosition(GUIButton* Button, GUITouchEvent Event, uint16_t XPos, uint16_t YPos)
{
  static GUIButton* lastActiveButton = 0;

  if (Event == GUITouchEvent_None)
  {
    /* If no button hit was found, check if a button is still in touch down state and if so change it's state back */
    if (lastActiveButton != 0)
    {
      GUIButton_SetButtonState(lastActiveButton->object.id, lastActiveButton->lastButtonState);
      lastActiveButton = 0;
    }
  }
  else if (Event == GUITouchEvent_Up)
  {
    /* Reset the button to it's previous state now that touch up has happened */
    GUIButton_SetButtonState(Button->object.id, Button->lastButtonState);
    lastActiveButton = 0;
    if (Button->touchCallback != 0)
    {
      Button->touchCallback(Event, Button->object.id);
      /* TODO: */
//          if (prvBeepIsOn)
//            BUZZER_BeepNumOfTimes(1);
    }
  }
  else if (Event == GUITouchEvent_Down)
  {
    /*
     * Check if the new button we have hit is different from the last time,
     * if so change back the state of the old button and activate the new one
     * and save a reference to it
     */
    if (lastActiveButton == 0 || lastActiveButton->object.id != Button->object.id)
    {
      /*
       * If we had a last active button it means the user has moved away from the button while
       * still holding down on the screen. We therefore have to reset the state of that button
       */
      if (lastActiveButton != 0)
        GUIButton_SetButtonState(lastActiveButton->object.id, lastActiveButton->lastButtonState);

      /* Save the new button and change it's state to touch down */
      lastActiveButton = Button;
      GUIButton_SetButtonState(Button->object.id, GUIButtonState_TouchDown);
    }
    /* Otherwise just call the callback as the user is still touching the same button */
    if (Button->touchCallback != 0)
      Button->touchCallback(Event, Button->object.id);
  }
}

/**
 * @brief   Check if a button is located at the position where a touch up event occurred
 * @param   GUITouchEvent: The event that happened, can be any value of GUITouchEvent
 * @param   XPos: X-position for event
 * @param   XPos: Y-position for event
 * @retval  None
 */
void GUIButton_CheckAllActiveForTouchEventAt(GUITouchEvent Event, uint16_t XPos, uint16_t YPos)
{
  static GUIButton* lastActiveButton = 0;
  static GUIButtonState lastState = GUIButtonState_NoState;

  for (uint32_t index = 0; index < guiConfigNUMBER_OF_BUTTONS; index++)
  {
    GUIButton* button = &prvButton_list[index];
    /* Check if the button is not hidden and enabled and if it's hit */
    if (button->object.displayState == GUIDisplayState_NotHidden && button->buttonState != GUIButtonState_NoState &&
      button->object.layer == prvCurrentlyActiveLayer && button->buttonState != GUIButtonState_DisabledTouch &&
      prvPointIsInsideObject(XPos, YPos, &button->object))
    {
      if (Event == GUITouchEvent_Up)
      {
        GUIButton_SetButtonState(index + guiConfigBUTTON_ID_OFFSET, lastState);
        lastActiveButton = 0;
        lastState = GUIButtonState_NoState;
        if (button->touchCallback != 0)
        {
          button->touchCallback(Event, index + guiConfigBUTTON_ID_OFFSET);
          /* TODO: */
//          if (prvBeepIsOn)
//            BUZZER_BeepNumOfTimes(1);
        }
      }
      else if (Event == GUITouchEvent_Down)
      {
        /*
         * Check if the new button we have hit is different from the last time,
         * if so change back the state of the old button and activate the new one
         * and save a reference to it
         */
        if (lastActiveButton == 0 || lastActiveButton->object.id != button->object.id)
        {
          /*
           * If we had a last active button it means the user has moved away from the button while
           * still holding down on the screen. We therefore have to reset the state of that button
           */
          if (lastActiveButton != 0)
            GUIButton_SetButtonState(lastActiveButton->object.id, lastState);

          /* Save the new button and change it's state */
          lastState = button->buttonState;
          lastActiveButton = &prvButton_list[index];
          GUIButton_SetButtonState(index + guiConfigBUTTON_ID_OFFSET, GUIButtonState_TouchDown);
        }
        /* Otherwise just call the callback as the user is still touching the same button */
        if (button->touchCallback != 0)
          button->touchCallback(Event, index + guiConfigBUTTON_ID_OFFSET);
      }
      /* Only one button can be active on an event so return when we have found one */
      return;
    }
  }

  /* If no button hit was found, check if a button is still in touch down state and if so change it's state */
  if (lastActiveButton != 0)
  {
    GUIButton_SetButtonState(lastActiveButton->object.id, lastState);
    lastActiveButton = 0;
    lastState = GUIButtonState_NoState;
  }
}

/** Label --------------------------------------------------------------------*/
/**
 * @brief   Get a pointer to the label corresponding to the id
 * @param   LabelId: The Id for the button
 * @retval  Pointer the item or 0 if no item was found
 */
GUILabel* GUILabel_GetFromId(uint32_t LabelId)
{
  uint32_t index = LabelId - guiConfigLABEL_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_LABELS && prvLabel_list[index].object.id != GUI_INVALID_ID)
    return &prvLabel_list[index];
  else
  {
    prvErrorHandler("GUILabel_GetFromId-Invalid ID");
    return 0;
  }
}

/**
 * @brief   Init a Label and add it to the label list
 * @param   Label: Pointer to a GUILabel struct which data should be copied from
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 * @retval  GUIStatus_Error: If the item was hidden or something else went wrong
 */
GUIStatus GUILabel_Init(GUILabel* Label)
{
  uint32_t index = Label->object.id - guiConfigLABEL_ID_OFFSET;
  GUIStatus status = GUIStatus_Success;

  /* Make sure we don't try to create more items than there's room for in the list */
  if (index < guiConfigNUMBER_OF_LABELS)
  {
    /* Copy the item to the list */
    memcpy(&prvLabel_list[index], Label, sizeof(GUILabel));

    /* Get a pointer to the item */
    GUILabel* label = &prvLabel_list[index];

    /* Init the item */
    GUILabel_InitRaw(label);
  }
  else
  {
    prvErrorHandler("GUILabel_Init-Invalid ID");
    status = GUIStatus_InvalidId;
  }

  /* Set all the data in the item we received as a parameter to 0 so that it can be reused easily */
//  GUILabel_Reset(Label);

  return status;
}

/**
 * @brief   Init a GUILabel object. This does not make use of the IDs so use with caution.
 * @param   Label: Pointer to a GUILabel struct where data can be found
 * @retval  None
 */
void GUILabel_InitRaw(GUILabel* Label)
{
  /* Calculate some parameters */
  /* Two rows of text */
  if (Label->text[0] != 0 && Label->text[1] != 0)
  {
    Label->numOfChar[0] = strlen(Label->text[0]);
    Label->textWidth[0] = FONTS_GetTotalWidthForString(Label->text[0], Label->font);
    Label->textHeight[0] = Label->font->Height;

    Label->numOfChar[1] = strlen(Label->text[1]);
    Label->textWidth[1] = FONTS_GetTotalWidthForString(Label->text[1], Label->font);
    Label->textHeight[1] = Label->font->Height;
  }
  /* One row of text */
  else if (Label->text[0] != 0)
  {
    Label->numOfChar[0] = strlen(Label->text[0]);
    Label->textWidth[0] = FONTS_GetTotalWidthForString(Label->text[0], Label->font);
    Label->textHeight[0] = Label->font->Height;
  }
  else
  {
    prvErrorHandler("GUILabel_InitRaw-No text on rows");
  }

  /* If it's set to not hidden we should draw the item */
  if (Label->object.displayState == GUIDisplayState_NotHidden)
    GUILabel_DrawRaw(Label, true);
}

/**
 * @brief   Resets a label by setting all it's memory to 0
 * @param   Label: Pointer to a GUILabel struct which data should be reset for
 * @retval  None
 */
void GUILabel_Reset(GUILabel* Label)
{
  memset(Label, 0, sizeof(GUILabel));
}

/**
 * @brief   Draw a specific label in the label_list
 * @param   LabelId: The Id for the label
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 */
GUIStatus GUILabel_Draw(uint32_t LabelId)
{
  uint32_t index = LabelId - guiConfigLABEL_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_LABELS && prvLabel_list[index].object.id != GUI_INVALID_ID)
  {
    /* Get a pointer to the current item */
    GUILabel* label = &prvLabel_list[index];

    /* Increment number of objects on this layer if it's not already drawn before */
    if (label->object.displayState != GUIDisplayState_NotHidden)
      GUI_IncrementObjectsOnLayer(label->object.layer);

    /* Draw the item */
    GUILabel_DrawRaw(label, true);

    return GUIStatus_Success;
  }
  else
  {
    prvErrorHandler("GUILabel_Draw-Invalid ID");
    return GUIStatus_InvalidId;
  }
}

/**
 * @brief   Draw a GUILabel object. This does not make use of the IDs so use with caution.
 * @param   Label: Pointer to a GUILabel struct where data can be found
 * @param   MarkDirtyZones: If set to true it will mark the dirty zones this object hits
 * @retval  None
 */
void GUILabel_DrawRaw(GUILabel* Label, bool MarkDirtyZones)
{
  /* Draw the background rectangle */
  LCD_DrawFilledRectangleOnLayer(Label->backgroundColor,
      Label->object.xPos, Label->object.yPos,
      Label->object.width, Label->object.height,
      Label->object.layer);

  /* Draw the text */
  if (Label->text[0] != 0 && Label->text[1] != 0)
  {
    /* Two rows of text */
    uint16_t xPos = Label->object.xPos + (Label->object.width - Label->textWidth[0]) / 2;
    uint16_t yPos = Label->object.yPos + Label->object.height/2 - Label->textHeight[0];
    LCD_DrawStringOnLayer(Label->textColor[0], xPos, yPos, Label->text[0], Label->font, Label->object.layer);

    xPos = Label->object.xPos + (Label->object.width - Label->textWidth[1]) / 2;
    yPos = Label->object.yPos + Label->object.height/2;
    LCD_DrawStringOnLayer(Label->textColor[1], xPos, yPos, Label->text[1], Label->font, Label->object.layer);
  }
  else if (Label->text[0] != 0)
  {
    /* One row of text */
    uint16_t xPos = Label->object.xPos + (Label->object.width - Label->textWidth[0]) / 2;
    uint16_t yPos = Label->object.yPos + (Label->object.height - Label->textHeight[0]) / 2;
    LCD_DrawStringOnLayer(Label->textColor[0], xPos, yPos, Label->text[0], Label->font, Label->object.layer);
  }

  /* Draw the border */
  GUI_DrawBorderRaw(&Label->object);

  /* Mark dirty zones */
  if (MarkDirtyZones)
    prvMarkDirtyZonesWithObject(&Label->object);

  Label->object.displayState = GUIDisplayState_NotHidden;
}

/**
 * @brief   Clears a label by drawing a transparent rectangle in it's place instead
 * @param   LabelId: The Id for the label
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 */
GUIStatus GUILabel_Clear(uint32_t LabelId)
{
  uint32_t index = LabelId - guiConfigLABEL_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_LABELS && prvLabel_list[index].object.id != GUI_INVALID_ID)
  {
    /* Get a pointer to the current item */
    GUILabel* label = &prvLabel_list[index];

    /* Clear the object */
    GUIObject_ClearRaw(&label->object);

    /* Decrement number of objects on this layer */
    GUI_DecrementObjectsOnLayer(label->object.layer);

    return GUIStatus_Success;
  }
  else
  {
    prvErrorHandler("GUILabel_Clear-Invalid ID");
    return GUIStatus_InvalidId;
  }
}

/** Static Text Box ----------------------------------------------------------*/
/**
 * @brief   Get a pointer to the static text box corresponding to the id
 * @param   StaticTextBoxId: The Id for the item
 * @retval  Pointer the item or 0 if no item was found
 */
GUIStaticTextBox* GUIStaticTextBox_GetFromId(uint32_t StaticTextBoxId)
{
  uint32_t index = StaticTextBoxId - guiConfigSTATIC_TEXT_BOX_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_STATIC_TEXT_BOXES && prvStaticTextBox_list[index].object.id != GUI_INVALID_ID)
    return &prvStaticTextBox_list[index];
  else
  {
    prvErrorHandler("GUIStaticTextBox_GetFromId-Invalid ID");
    return 0;
  }
}

/**
 * @brief   Init a Static text box and add it to the list
 * @param   StaticTextBox: Pointer to a GUIStaticTextBox struct which data should be copied from
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 * @retval  GUIStatus_Error: If the item was hidden or something else went wrong
 */
GUIStatus GUIStaticTextBox_Init(GUIStaticTextBox* StaticTextBox)
{
  uint32_t index = StaticTextBox->object.id - guiConfigSTATIC_TEXT_BOX_ID_OFFSET;
  GUIStatus status = GUIStatus_Success;

  /* Make sure we don't try to create more items than there's room for in the list */
  if (index < guiConfigNUMBER_OF_STATIC_TEXT_BOXES)
  {
    /* Copy the item to the list */
    memcpy(&prvStaticTextBox_list[index], StaticTextBox, sizeof(GUIStaticTextBox));

    /* Get a pointer to the item */
    GUIStaticTextBox* staticTextBox = &prvStaticTextBox_list[index];

    /* Init the item */
    GUIStaticTextBox_InitRaw(staticTextBox);
  }
  else
  {
    prvErrorHandler("GUIStaticTextBox_Init-Invalid ID");
    status = GUIStatus_InvalidId;
  }

  /* Set all the data in the item we received as a parameter to 0 so that it can be reused easily */
//  GUIStaticTextBox_Reset(StaticTextBox);

  return status;
}

/**
 * @brief   Init a GUIStaticTextBox object. This does not make use of the IDs so use with caution.
 * @param   StaticTextBox: Pointer to a GUIStaticTextBox struct where data can be found
 * @retval  None
 */
void GUIStaticTextBox_InitRaw(GUIStaticTextBox* StaticTextBox)
{
  /* If it's set to not hidden we should draw the item */
  if (StaticTextBox->object.displayState == GUIDisplayState_NotHidden)
    GUIStaticTextBox_DrawRaw(StaticTextBox, true);
}

/**
 * @brief   Resets a static text box by setting all it's memory to 0
 * @param   StaticTextBox: Pointer to a GUIStaticTextBox struct which data should be reset for
 * @retval  None
 */
void GUIStaticTextBox_Reset(GUIStaticTextBox* StaticTextBox)
{
  memset(StaticTextBox, 0, sizeof(GUIStaticTextBox));
}

/**
 * @brief   Draw a specific static text box in the list
 * @param   StaticTextBoxId: The Id for the static text box
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 */
GUIStatus GUIStaticTextBox_Draw(uint32_t StaticTextBoxId)
{
  uint32_t index = StaticTextBoxId - guiConfigSTATIC_TEXT_BOX_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_STATIC_TEXT_BOXES &&  prvStaticTextBox_list[index].object.id != GUI_INVALID_ID)
  {
    /* Get a pointer to the current item */
    GUIStaticTextBox* staticTextBox = &prvStaticTextBox_list[index];

    /* Increment number of objects on this layer if it's not already drawn before */
    if (staticTextBox->object.displayState != GUIDisplayState_NotHidden)
      GUI_IncrementObjectsOnLayer(staticTextBox->object.layer);

    /* Draw the item */
    GUIStaticTextBox_DrawRaw(staticTextBox, true);

    return GUIStatus_Success;
  }
  else
  {
    prvErrorHandler("GUIStaticTextBox_Draw-Invalid ID");
    return GUIStatus_InvalidId;
  }
}

/**
 * @brief   Draw a GUIStaticTextBox object. This does not make use of the IDs so use with caution.
 * @param   StaticTextBox: Pointer to a GUIStaticTextBox struct where data can be found
 * @param   MarkDirtyZones: If set to true it will mark the dirty zones this object hits
 * @retval  None
 */
void GUIStaticTextBox_DrawRaw(GUIStaticTextBox* StaticTextBox, bool MarkDirtyZones)
{
  /* Draw the background rectangle */
  LCD_DrawFilledRectangleOnLayer(StaticTextBox->backgroundColor,
      StaticTextBox->object.xPos, StaticTextBox->object.yPos,
      StaticTextBox->object.width, StaticTextBox->object.height,
      StaticTextBox->object.layer);

  /* Draw the text string */
  uint32_t index = 0;
  bool keepLooping = true;
  char* currentCharacter = StaticTextBox->text;
  uint16_t xTemp = StaticTextBox->object.xPos;
  uint16_t yTemp = StaticTextBox->object.yPos;
  uint16_t xMax = StaticTextBox->object.xPos + StaticTextBox->object.width - 1;
  uint16_t yMax = StaticTextBox->object.yPos + StaticTextBox->object.height - 1 - StaticTextBox->font->Height;
  while (keepLooping)
  {
    /* Check if we have reached the x-limit or the text string has ended */
    if (xTemp + FONTS_GetWidthForCharacter(*currentCharacter, StaticTextBox->font) > xMax || *currentCharacter == 0)
    {
      /*
       * Cut the string in a row between two words:
       * If we are not at the end of the text string we only want to move down one row between two words.
       * We therefore need to check the temporary string for the nearest space character ' ' backwards
       * and set this as our end for this row's string.
       * If the current character is 0 we should not do this.
       */
      while (prvTempString[index] != ' ' && *currentCharacter != 0)
      {
        /* Reverse the index and the current character */
        index--;
        currentCharacter--;
        /* Check for error, this means that there is a word in the text string longer then the width of the text box */
        if (index == 0)
          prvErrorHandler("GUIStaticTextBox_DrawRaw-Long word");
      }

      /* Terminate the temporary string */
      prvTempString[index] = 0;

      /* Calculate some dimensions */
      uint16_t stringWidth = FONTS_GetTotalWidthForString(prvTempString, StaticTextBox->font);
      uint16_t xPos = 0;
      if (StaticTextBox->textAlignment == GUITextAlignment_Center)
        xPos = StaticTextBox->object.xPos + StaticTextBox->object.width / 2 - stringWidth / 2;
      else if (StaticTextBox->textAlignment == GUITextAlignment_Left)
        xPos = StaticTextBox->object.xPos;
      else if (StaticTextBox->textAlignment == GUITextAlignment_Right)
        xPos = StaticTextBox->object.xPos + StaticTextBox->object.width - stringWidth;
      else
        prvErrorHandler("GUIStaticTextBox_DrawRaw-Invalid text alignment");

      /* Draw the temporary string */
      LCD_DrawStringOnLayer(StaticTextBox->textColor, xPos, yTemp, prvTempString, StaticTextBox->font, StaticTextBox->object.layer);
      /* Check if end of text string */
      if (*currentCharacter == 0)
      {
        keepLooping = false;
        break;
      }
      /* Reset the index for the temporary string */
      index = 0;

      /* Move on step forward to get rid of the space character for the next row */
      currentCharacter++;

      /* If moving one row down causes a jump outside, stop drawing the text string */
      if (yTemp + StaticTextBox->font->Height > yMax)
        break;
      /* Otherwise move one row down and reset x-position */
      else
      {
        yTemp += StaticTextBox->font->Height;
        xTemp = StaticTextBox->object.xPos;
      }
    }

    /* Check for a newline character */
    if (*currentCharacter == 10)
    {
      /* Move forward to get the next valid character after this newline */
      currentCharacter++;

      /* Terminate the temporary string */
      prvTempString[index] = 0;

      /* Calculate some dimensions */
      uint16_t stringWidth = FONTS_GetTotalWidthForString(prvTempString, StaticTextBox->font);
      uint16_t xPos = 0;
      if (StaticTextBox->textAlignment == GUITextAlignment_Center)
        xPos = StaticTextBox->object.xPos + StaticTextBox->object.width / 2 - stringWidth / 2;
      else if (StaticTextBox->textAlignment == GUITextAlignment_Left)
        xPos = StaticTextBox->object.xPos;
      else if (StaticTextBox->textAlignment == GUITextAlignment_Right)
        xPos = StaticTextBox->object.xPos + StaticTextBox->object.width - stringWidth;
      else
        prvErrorHandler("GUIStaticTextBox_DrawRaw-Invalid text alignment");

      /* Draw the temporary string */
      LCD_DrawStringOnLayer(StaticTextBox->textColor, xPos, yTemp, prvTempString, StaticTextBox->font, StaticTextBox->object.layer);
      /* Check if end of text string */
      if (*currentCharacter == 0)
      {
        keepLooping = false;
        break;
      }
      /* Reset the index for the temporary string */
      index = 0;

      /* If moving one row down causes a jump outside, stop drawing the text string */
      if (yTemp + StaticTextBox->font->Height > yMax)
        break;
      /* Otherwise move one row down and reset x-position */
      else
      {
        yTemp += StaticTextBox->font->Height;
        xTemp = StaticTextBox->object.xPos;
      }
    }

    /* Copy the character to the temporary string */
    prvTempString[index] = *currentCharacter;
    index++;
    /* Make sure the index doesn't overflow */
    if (index == GUI_MAX_CHARACTERS_PER_ROW)
    {
      prvErrorHandler("GUIStaticTextBox_DrawRaw-Index overflow");
    }
    /* Increment the temporary x-position */
    xTemp += FONTS_GetWidthForCharacter(*currentCharacter, StaticTextBox->font);
    /* Point on the next character */
    currentCharacter++;
  }

  /* Draw the border */
  GUI_DrawBorderRaw(&StaticTextBox->object);

  /* Mark dirty zones */
  if (MarkDirtyZones)
    prvMarkDirtyZonesWithObject(&StaticTextBox->object);

  StaticTextBox->object.displayState = GUIDisplayState_NotHidden;
}

/**
 * @brief   Clears a static text box by drawing a transparent rectangle in it's place instead
 * @param   StaticTextBoxId: The Id for the item
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 */
GUIStatus GUIStaticTextBox_Clear(uint32_t StaticTextBoxId)
{
  uint32_t index = StaticTextBoxId - guiConfigSTATIC_TEXT_BOX_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_STATIC_TEXT_BOXES && prvStaticTextBox_list[index].object.id != GUI_INVALID_ID)
  {
    /* Get a pointer to the current item */
    GUIStaticTextBox* staticTextBox = &prvStaticTextBox_list[index];

    /* Clear the object */
    GUIObject_ClearRaw(&staticTextBox->object);

    /* Decrement number of objects on this layer */
    GUI_DecrementObjectsOnLayer(staticTextBox->object.layer);

    return GUIStatus_Success;
  }
  else
  {
    prvErrorHandler("GUIStaticTextBox_Clear-Invalid ID");
    return GUIStatus_InvalidId;
  }
}

#ifdef GUI_SCROLLABLE_TEXT_BOX_ENABLED
/** Scrollable Text Box ------------------------------------------------------*/
/**
 * @brief   Get a pointer to the scrollable text box corresponding to the id
 * @param   ScrollableTextBoxId: The Id for the item
 * @retval  Pointer the item or 0 if no item was found
 */
GUIScrollableTextBox* GUIScrollableTextBox_GetFromId(uint32_t ScrollableTextBoxId)
{
  uint32_t index = ScrollableTextBoxId - guiConfigSCROLLABLE_TEXT_BOX_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_SCROLLABLE_TEXT_BOXES && prvScrollableTextBox_list[index].object.id != GUI_INVALID_ID)
    return &prvScrollableTextBox_list[index];
  else
  {
    prvErrorHandler("GUIScrollableTextBox_GetFromId-Invalid ID");
    return 0;
  }
}

/**
 * @brief   Init a scrollable text box and add it to the list
 * @param   ScrollableTextBox: Pointer to a GUIScrollableTextBox struct which data should be copied from
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 * @retval  GUIStatus_Error: If the item was hidden or something else went wrong
 */
GUIStatus GUIScrollableTextBox_Init(GUIScrollableTextBox* ScrollableTextBox)
{
  uint32_t index = ScrollableTextBox->object.id - guiConfigSCROLLABLE_TEXT_BOX_ID_OFFSET;
  GUIStatus status = GUIStatus_Success;

  /* Make sure we don't try to create more items than there's room for in the list */
  if (index < guiConfigNUMBER_OF_SCROLLABLE_TEXT_BOXES)
  {
    /* Copy the item to the list */
    memcpy(&prvScrollableTextBox_list[index], ScrollableTextBox, sizeof(GUIScrollableTextBox));

    /* Get a pointer to the item */
    GUIScrollableTextBox* scrollableTextBox = &prvScrollableTextBox_list[index];

    /* Init the item */
    GUIScrollableTextBox_InitRaw(scrollableTextBox);
  }
  else
  {
    prvErrorHandler("GUIScrollableTextBox_Init-Invalid ID");
    status = GUIStatus_InvalidId;
  }

  /* Set all the data in the item we received as a parameter to 0 so that it can be reused easily */
//  GUIScrollableTextBox_Reset(ScrollableTextBox);

  return status;
}

/**
 * @brief   Init a GUIScrollableTextBox object. This does not make use of the IDs so use with caution.
 * @param   ScrollableTextBox: Pointer to a GUIScrollableTextBox struct where data can be found
 * @retval  None
 */
void GUIScrollableTextBox_InitRaw(GUIScrollableTextBox* ScrollableTextBox)
{
  /* Check if it's a fixed font */
  if (ScrollableTextBox->fixedWidthFont->fixedWidth == false)
  {
    prvErrorHandler("GUIScrollableTextBox_InitRaw-Font is not fixed width");
  }

  ScrollableTextBox->scrollOffset = 0;

  /* Calculate some stuff */
  ScrollableTextBox->maxNumOfRowsInVisbleBox = (ScrollableTextBox->object.height - 2*ScrollableTextBox->padding.topBottom) / ScrollableTextBox->fixedWidthFont->Height;
  ScrollableTextBox->maxNumOfCharsInRow = (ScrollableTextBox->object.width - 2*ScrollableTextBox->padding.leftRight) / ScrollableTextBox->fixedWidthFont->SpaceWidth;
  ScrollableTextBox->maxNumOfCharsInVisbleBox = ScrollableTextBox->maxNumOfRowsInVisbleBox * ScrollableTextBox->maxNumOfCharsInRow;

  /* Reset the data buffer pointer and check if there is data available */
  ScrollableTextBox->drawnDataStart = ScrollableTextBox->dataBufferStart;
  if (ScrollableTextBox->numOfCharsAvailable())
  {
    ScrollableTextBox->drawnDataEnd = ScrollableTextBox->drawnDataStart + ScrollableTextBox->numOfCharsAvailable();
    GUIScrollableTextBox_DrawDataOnFrameBufferRaw(ScrollableTextBox);
  }
  else
    ScrollableTextBox->drawnDataEnd = ScrollableTextBox->dataBufferStart;

  /* Check if the circular buffer is big enough - Two extra rows are needed besides the ones visble */
  if (ScrollableTextBox->maxNumOfCharsInVisbleBox + 2*ScrollableTextBox->maxNumOfCharsInRow > GUI_MAX_CHARACTERS_IN_SCROLLABLE_TEXT_BOX)
  {
    prvErrorHandler("GUIScrollableTextBox_InitRaw-Circular buffer is too small");
  }

  /* Reset the framebuffer pointers */
  ScrollableTextBox->visibleFrameStartAddress = ScrollableTextBox->frameBufferStartAddress;
  ScrollableTextBox->visibleFrameEndAddress = ScrollableTextBox->visibleFrameStartAddress + ScrollableTextBox->object.width*ScrollableTextBox->object.height*4;

  /* If it's set to not hidden we should draw the item */
  if (ScrollableTextBox->object.displayState == GUIDisplayState_NotHidden)
    GUIScrollableTextBox_DrawRaw(ScrollableTextBox, true);
}

/**
 * @brief   Resets a scrollabe text box by setting all it's memory to 0
 * @param   ScrollableTextBox: Pointer to a GUIScrollableTextBox struct which data should be reset for
 * @retval  None
 */
void GUIScrollableTextBox_Reset(GUIScrollableTextBox* ScrollableTextBox)
{
  memset(ScrollableTextBox, 0, sizeof(GUIScrollableTextBox));
}

/**
 * @brief   Draw a specific scrollable text box in the list
 * @param   ScrollableTextBoxId: The Id for the scrollable text box
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 */
GUIStatus GUIScrollableTextBox_Draw(uint32_t ScrollableTextBoxId)
{
  uint32_t index = ScrollableTextBoxId - guiConfigSCROLLABLE_TEXT_BOX_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_SCROLLABLE_TEXT_BOXES &&  prvScrollableTextBox_list[index].object.id != GUI_INVALID_ID)
  {
    /* Get a pointer to the current item */
    GUIScrollableTextBox* scrollableTextBox = &prvScrollableTextBox_list[index];

    /* Increment number of objects on this layer if it's not already drawn before */
    if (scrollableTextBox->object.displayState != GUIDisplayState_NotHidden)
      GUI_IncrementObjectsOnLayer(scrollableTextBox->object.layer);

    /* Draw the item */
    GUIScrollableTextBox_DrawRaw(scrollableTextBox, true);

    return GUIStatus_Success;
  }
  else
  {
    prvErrorHandler("GUIScrollableTextBox_Draw-Invalid ID");
    return GUIStatus_InvalidId;
  }
}

/**
 * @brief   Draw a GUIScrollableTextBox object. This does not make use of the IDs so use with caution.
 * @param   ScrollableTextBox: Pointer to a GUIScrollableTextBox struct where data can be found
 * @param   MarkDirtyZones: If set to true it will mark the dirty zones this object hits
 * @retval  None
 */
void GUIScrollableTextBox_DrawRaw(GUIScrollableTextBox* ScrollableTextBox, bool MarkDirtyZones)
{
  /* Draw the background rectangle */
  LCD_DrawFilledRectangleOnLayer(ScrollableTextBox->backgroundColor,
      ScrollableTextBox->object.xPos, ScrollableTextBox->object.yPos,
      ScrollableTextBox->object.width, ScrollableTextBox->object.height,
      ScrollableTextBox->object.layer);


  /* Draw the visible frame to the layer */
  LCD_DrawARGB8888BufferOnLayer(ScrollableTextBox->object.xPos, ScrollableTextBox->object.yPos,
                  ScrollableTextBox->object.width, ScrollableTextBox->object.height,
                  ScrollableTextBox->visibleFrameStartAddress, ScrollableTextBox->object.layer);

  /* Draw the border */
  GUI_DrawBorderRaw(&ScrollableTextBox->object);

  /* Mark dirty zones */
  if (MarkDirtyZones)
    prvMarkDirtyZonesWithObject(&ScrollableTextBox->object);

  ScrollableTextBox->object.displayState = GUIDisplayState_NotHidden;
}

/**
 * @brief   TODO:
 * @param   ScrollableTextBox: Pointer to a GUIScrollableTextBox struct where data can be found
 * @retval  None
 */
void GUIScrollableTextBox_DrawDataOnFrameBufferRaw(GUIScrollableTextBox* ScrollableTextBox)
{
  /* Draw the characters */
  char* currentCharacter = ScrollableTextBox->drawnDataStart;
  uint16_t xTemp = ScrollableTextBox->padding.leftRight;
  uint16_t yTemp = ScrollableTextBox->padding.topBottom;
  uint16_t xMax = ScrollableTextBox->object.width - 1 - ScrollableTextBox->padding.leftRight;
  /* Y-max is two rows more than the visible part */
  uint16_t yMax = ScrollableTextBox->object.height - 1 + ScrollableTextBox->fixedWidthFont->Height - ScrollableTextBox->padding.topBottom;

  /* Draw until we've reached the end of the data that should be drawn */
  while (currentCharacter != ScrollableTextBox->drawnDataEnd)
  {
    /* Check for a newline character or if we have reached the x-limit */
    if (*currentCharacter == 10 || xTemp + FONTS_GetWidthForCharacter(*currentCharacter, ScrollableTextBox->fixedWidthFont) > xMax)
    {
      /* If moving one row down causes a jump outside, stop drawing */
      if (yTemp + ScrollableTextBox->fixedWidthFont->Height > yMax)
        break;
      /* Otherwise move one row down and reset x-position */
      else
      {
        yTemp += ScrollableTextBox->fixedWidthFont->Height;
        xTemp = ScrollableTextBox->padding.leftRight;
      }
    }

    /* Check for space character */
    if (*currentCharacter == ' ')
    {
      xTemp += ScrollableTextBox->fixedWidthFont->SpaceWidth;
    }
    /* Otherwise draw the character (if it's not the newline character) */
    else if (*currentCharacter != 10)
    {
      LCD_DrawCharacterOnBuffer(ScrollableTextBox->textColor, xTemp, yTemp, *currentCharacter, ScrollableTextBox->fixedWidthFont, ScrollableTextBox->frameBufferStartAddress);
      xTemp += FONTS_GetWidthForCharacter(*currentCharacter, ScrollableTextBox->fixedWidthFont);
    }
    currentCharacter++;
  }
}

/**
 * @brief   TODO:
 * @param   ScrollableTextBoxId: The Id for the scrollable text box
 * @param   RowsToScroll:
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 */
GUIStatus GUIScrollableTextBox_ScrollBuffer(uint32_t ScrollableTextBoxId, int32_t PixelsToScroll)
{
  uint32_t index = ScrollableTextBoxId - guiConfigSCROLLABLE_TEXT_BOX_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_SCROLLABLE_TEXT_BOXES &&  prvScrollableTextBox_list[index].object.id != GUI_INVALID_ID)
  {
    /* Get a pointer to the current item */
    GUIScrollableTextBox* scrollableTextBox = &prvScrollableTextBox_list[index];

    if (PixelsToScroll > 0)
    {
      /* Check if we have scrolled more than a row */
      if (scrollableTextBox->scrollOffset + PixelsToScroll > scrollableTextBox->fixedWidthFont->Height)
      {
        /* Clear the Frame Buffer */
        LCD_ClearBuffer(COLOR_TRANSPARENT, scrollableTextBox->object.width, scrollableTextBox->object.height, scrollableTextBox->frameBufferStartAddress);

        /* Load new row from data */
        uint32_t numOfRowsToMove = (scrollableTextBox->scrollOffset + PixelsToScroll) / scrollableTextBox->fixedWidthFont->Height;
        scrollableTextBox->drawnDataStart += scrollableTextBox->maxNumOfCharsInRow * numOfRowsToMove;
        GUIScrollableTextBox_DrawDataOnFrameBufferRaw(scrollableTextBox);
        scrollableTextBox->scrollOffset -= 16;
      }
      scrollableTextBox->visibleFrameStartAddress += LCD_PIXEL_WIDTH * 4 * PixelsToScroll;
      GUIScrollableTextBox_DrawRaw(scrollableTextBox, true);
      scrollableTextBox->scrollOffset += PixelsToScroll;
    }
    else if (PixelsToScroll < 0)
    {

    }

    return GUIStatus_Success;
  }
  else
  {
    prvErrorHandler("GUIScrollableTextBox_ScrollRows-Invalid ID");
    return GUIStatus_InvalidId;
  }
}


#endif

#ifdef GUI_ALERT_BOX_ENABLED
/** Alert Box ----------------------------------------------------------------*/
/**
 * @brief   Get a pointer to the alert box corresponding to the id
 * @param   AlertBoxId: The Id for the item
 * @retval  Pointer the item or 0 if no item was found
 */
GUIAlertBox* GUIAlertBox_GetFromId(uint32_t AlertBoxId)
{
  uint32_t index = AlertBoxId - guiConfigALERT_BOX_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_ALERT_BOXES && prvAlertBox_list[index].object.id != GUI_INVALID_ID)
    return &prvAlertBox_list[index];
  else
  {
    prvErrorHandler("GUIAlertBox_GetFromId-Invalid ID");
    return 0;
  }
}

/**
 * @brief   Init an alert box and add it to the list
 * @param   AlertBox: Pointer to a GUIAlertBox struct which data should be copied from
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 * @retval  GUIStatus_Error: If the item was hidden or something else went wrong
 */
GUIStatus GUIAlertBox_Init(GUIAlertBox* AlertBox)
{
  uint32_t index = AlertBox->object.id - guiConfigALERT_BOX_ID_OFFSET;
  GUIStatus status = GUIStatus_Success;

  /* Make sure we don't try to create more items than there's room for in the list */
  if (index < guiConfigNUMBER_OF_ALERT_BOXES)
  {
    /* Copy the item to the list */
    memcpy(&prvAlertBox_list[index], AlertBox, sizeof(GUIAlertBox));

    /* Get a pointer to the item */
    GUIAlertBox* alertBox = &prvAlertBox_list[index];

    /* Configure the title label */
    GUILabel_Reset(&alertBox->titleLabel);
    alertBox->titleLabel.object.id         = GUI_INTERNAL_ID;
    alertBox->titleLabel.object.xPos       = alertBox->object.xPos;
    alertBox->titleLabel.object.yPos       = alertBox->object.yPos;
    alertBox->titleLabel.object.width       = alertBox->object.width;
    alertBox->titleLabel.object.height       = alertBox->titleHeight;
    alertBox->titleLabel.object.displayState   = GUIDisplayState_Hidden;
    alertBox->titleLabel.object.border       = GUIBorder_NoBorder;
    alertBox->titleLabel.object.layer      = alertBox->object.layer;
    alertBox->titleLabel.backgroundColor    = alertBox->titleBackgroundColor;
    alertBox->titleLabel.textColor[0]      = alertBox->titleTextColor;
    alertBox->titleLabel.text[0]        = alertBox->title;
    alertBox->titleLabel.font          = alertBox->font;
    GUILabel_InitRaw(&alertBox->titleLabel);

    /* Configure the close button */
    GUIButton_Reset(&alertBox->closeButton);
    alertBox->closeButton.object.id         = GUI_INTERNAL_ID;
    alertBox->closeButton.object.xPos         = alertBox->object.xPos;
    alertBox->closeButton.object.yPos         = alertBox->object.yPos;
    alertBox->closeButton.object.width         = 50;  /* TODO: Adjustable? */
    alertBox->closeButton.object.height       = alertBox->titleHeight;
    alertBox->closeButton.object.displayState     = GUIDisplayState_Hidden;
    alertBox->closeButton.object.border       = GUIBorder_Left | GUIBorder_Top;
    alertBox->closeButton.object.borderThickness  = alertBox->object.borderThickness;
    alertBox->closeButton.object.borderColor    = alertBox->object.borderColor;
    alertBox->closeButton.object.layer        = alertBox->object.layer;
    alertBox->closeButton.state1TextColor       = alertBox->titleBackgroundColor;
    alertBox->closeButton.state1BackgroundColor   = alertBox->titleTextColor;
    alertBox->closeButton.pressedTextColor       = alertBox->titleTextColor;
    alertBox->closeButton.pressedBackgroundColor   = COLOR_WHITE;
    alertBox->closeButton.buttonState        = GUIButtonState_State1;
    alertBox->closeButton.touchCallback       = 0;
    alertBox->closeButton.text[0]          = "x";
    alertBox->closeButton.font            = alertBox->font;
    GUIButton_InitRaw(&alertBox->closeButton);

    /* Calculate info text dimensions */
    uint16_t infoTextWidth = alertBox->object.width - 2*alertBox->padding.leftRight;
    uint16_t infoTextXPos = alertBox->object.xPos + alertBox->padding.leftRight;
    uint16_t infoTextYPos = alertBox->object.yPos + alertBox->titleHeight + alertBox->padding.topBottom;
    uint16_t infoTextHeight = alertBox->object.height - alertBox->titleHeight - 3*alertBox->padding.topBottom - alertBox->leftRightButtonHeight;

    /* Configure the info text box */
    GUIStaticTextBox_Reset(&alertBox->infoTextBox);
    alertBox->infoTextBox.object.id       = GUI_INTERNAL_ID;
    alertBox->infoTextBox.object.xPos       = infoTextXPos;
    alertBox->infoTextBox.object.yPos       = infoTextYPos;
    alertBox->infoTextBox.object.width       = infoTextWidth;
    alertBox->infoTextBox.object.height     = infoTextHeight;
    alertBox->infoTextBox.object.displayState   = GUIDisplayState_Hidden;
    alertBox->infoTextBox.object.border     = GUIBorder_NoBorder;
    alertBox->infoTextBox.object.layer      = alertBox->object.layer;
    alertBox->infoTextBox.backgroundColor    = alertBox->backgroundColor;
    alertBox->infoTextBox.textColor        = alertBox->infoTextColor;
    alertBox->infoTextBox.text          = alertBox->infoText;
    alertBox->infoTextBox.font          = alertBox->font;
    alertBox->infoTextBox.textAlignment      = GUITextAlignment_Center;
    GUIStaticTextBox_InitRaw(&alertBox->infoTextBox);

    /* Calculate button dimensions */
    /* padding - button - padding - button - padding */
    uint16_t buttonsWidth = (alertBox->object.width - 3*alertBox->padding.leftRight) / 2;
    uint16_t leftButtonXPos = alertBox->object.xPos + alertBox->padding.leftRight;
    uint16_t rightButtonXPos = alertBox->object.xPos + 2*alertBox->padding.leftRight + buttonsWidth;
    uint16_t buttonsYPos = alertBox->object.yPos + alertBox->object.height - 1 - alertBox->padding.topBottom - alertBox->leftRightButtonHeight;

    /* Configure the left button */
    GUIButton_Reset(&alertBox->leftButton);
    alertBox->leftButton.object.id         = GUI_INTERNAL_ID;
    alertBox->leftButton.object.xPos       = leftButtonXPos;
    alertBox->leftButton.object.yPos       = buttonsYPos;
    alertBox->leftButton.object.width       = buttonsWidth;
    alertBox->leftButton.object.height       = alertBox->leftRightButtonHeight;
    alertBox->leftButton.object.displayState   = GUIDisplayState_Hidden;
    alertBox->leftButton.object.border       = GUIBorder_All;
    alertBox->leftButton.object.borderThickness  = alertBox->object.borderThickness;
    alertBox->leftButton.object.borderColor    = alertBox->object.borderColor;
    alertBox->leftButton.object.layer      = alertBox->object.layer;
    alertBox->leftButton.state1TextColor     = alertBox->buttonTextColor;
    alertBox->leftButton.state1BackgroundColor   = alertBox->leftButtonColor;
    alertBox->leftButton.pressedTextColor     = alertBox->leftButtonColor;
    alertBox->leftButton.pressedBackgroundColor = COLOR_WHITE;
    alertBox->leftButton.buttonState      = GUIButtonState_State1;
    alertBox->leftButton.touchCallback       = 0;
    alertBox->leftButton.text[0]        = alertBox->leftButtonText[0];
    alertBox->leftButton.text[1]        = alertBox->leftButtonText[1];
    alertBox->leftButton.font          = alertBox->font;
    GUIButton_InitRaw(&alertBox->leftButton);

    /* Configure the right button */
    GUIButton_Reset(&alertBox->rightButton);
    alertBox->rightButton.object.id       = GUI_INTERNAL_ID;
    alertBox->rightButton.object.xPos       = rightButtonXPos;
    alertBox->rightButton.object.yPos       = buttonsYPos;
    alertBox->rightButton.object.width       = buttonsWidth;
    alertBox->rightButton.object.height     = alertBox->leftRightButtonHeight;
    alertBox->rightButton.object.displayState   = GUIDisplayState_Hidden;
    alertBox->rightButton.object.border     = GUIBorder_All;
    alertBox->rightButton.object.borderThickness = alertBox->object.borderThickness;
    alertBox->rightButton.object.borderColor  = alertBox->object.borderColor;
    alertBox->rightButton.object.layer      = alertBox->object.layer;
    alertBox->rightButton.state1TextColor     = alertBox->buttonTextColor;
    alertBox->rightButton.state1BackgroundColor = alertBox->rightButtonColor;
    alertBox->rightButton.pressedTextColor     = alertBox->rightButtonColor;
    alertBox->rightButton.pressedBackgroundColor = COLOR_WHITE;
    alertBox->rightButton.buttonState      = GUIButtonState_State1;
    alertBox->rightButton.touchCallback     = 0;
    alertBox->rightButton.text[0]        = alertBox->rightButtonText[0];
    alertBox->rightButton.text[1]        = alertBox->rightButtonText[1];
    alertBox->rightButton.font          = alertBox->font;
    GUIButton_InitRaw(&alertBox->rightButton);

    /* If it's set to not hidden we should draw the alert box */
    if (alertBox->object.displayState == GUIDisplayState_NotHidden)
      status = GUIAlertBox_Draw(alertBox->object.id);
  }
  else
  {
    prvErrorHandler("GUIAlertBox_Init-Invalid ID");
    status = GUIStatus_InvalidId;
  }

  /* Set all the data in the item we received as a parameter to 0 so that it can be reused easily */
//  GUIAlertBox_Reset(AlertBox);

  return status;
}

/**
 * @brief   Init a GUIAlertBox with color data
 * @param   AlertBox: Pointer to a GUIAlertBox where the new color data is
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_Error: If something went wrong
 */
GUIStatus GUIAlertBox_InitColors(GUIAlertBox* AlertBox)
{
  /* Make sure the pointer is valid */
  if (AlertBox != 0)
  {
    /* Configure the title label */
    AlertBox->titleLabel.backgroundColor    = AlertBox->titleBackgroundColor;
    AlertBox->titleLabel.textColor[0]      = AlertBox->titleTextColor;

    /* Configure the close button */
    AlertBox->closeButton.state1TextColor     = AlertBox->titleBackgroundColor;
    AlertBox->closeButton.state1BackgroundColor = AlertBox->titleTextColor;
    AlertBox->closeButton.pressedTextColor     = AlertBox->titleTextColor;

    /* Configure the info text box */
    AlertBox->infoTextBox.backgroundColor    = AlertBox->backgroundColor;
    AlertBox->infoTextBox.textColor        = AlertBox->infoTextColor;

    /* Configure the left button */
    AlertBox->leftButton.state1TextColor     = AlertBox->buttonTextColor;
    AlertBox->leftButton.state1BackgroundColor   = AlertBox->leftButtonColor;
    AlertBox->leftButton.pressedTextColor     = AlertBox->leftButtonColor;

    /* Configure the right button */
    AlertBox->rightButton.state1TextColor     = AlertBox->buttonTextColor;
    AlertBox->rightButton.state1BackgroundColor = AlertBox->rightButtonColor;
    AlertBox->rightButton.pressedTextColor     = AlertBox->rightButtonColor;

    return GUIStatus_Success;
  }
  else
  {
    prvErrorHandler("GUIAlertBox_InitColors-Pointer is empty");
    return GUIStatus_Error;
  }
}

/**
 * @brief   Resets an alert box by setting all it's memory to 0
 * @param   AlertBox: Pointer to a GUIAlertBox struct which data should be reset for
 * @retval  None
 */
void GUIAlertBox_Reset(GUIAlertBox* AlertBox)
{
  memset(AlertBox, 0, sizeof(GUIAlertBox));
}

/**
 * @brief  Draw a specific alert box
 * @param  AlertBoxId: The Id for the alert box
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 */
GUIStatus GUIAlertBox_Draw(uint32_t AlertBoxId)
{
  uint32_t index = AlertBoxId - guiConfigALERT_BOX_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_ALERT_BOXES && prvAlertBox_list[index].object.id != GUI_INVALID_ID)
  {
    /* Get a pointer to the current item */
    GUIAlertBox* alertBox = &prvAlertBox_list[index];

    /* Increment number of objects on this layer if it's not already drawn before */
    if (alertBox->object.displayState != GUIDisplayState_NotHidden)
      GUI_IncrementObjectsOnLayer(alertBox->object.layer);

    /* Draw the item */
    GUIAlertBox_DrawRaw(alertBox, true);

    return GUIStatus_Success;
  }
  else
  {
    prvErrorHandler("GUIAlertBox_Draw-Invalid ID");
    return GUIStatus_InvalidId;
  }
}

/**
 * @brief   Draw a GUIAlertBox object. This does not make use of the IDs so use with caution.
 * @param   AlertBox: Pointer to a GUIAlertBox struct where data can be found
 * @param   MarkDirtyZones: If set to true it will mark the dirty zones this object hits
 * @retval  None
 */
void GUIAlertBox_DrawRaw(GUIAlertBox* AlertBox, bool MarkDirtyZones)
{
  /* Draw the background rectangle */
  LCD_DrawFilledRectangleOnLayer(AlertBox->backgroundColor,
      AlertBox->object.xPos, AlertBox->object.yPos,
      AlertBox->object.width, AlertBox->object.height,
      AlertBox->object.layer);

  /* Draw the title bar and title text */
  GUILabel_DrawRaw(&AlertBox->titleLabel, false);
  /* Draw the close button */
  GUIButton_DrawRaw(&AlertBox->closeButton, false);
  /* Draw the info text */
  GUIStaticTextBox_DrawRaw(&AlertBox->infoTextBox, false);
  /* Draw the left button */
  GUIButton_DrawRaw(&AlertBox->leftButton, false);
  /* Draw the right button */
  GUIButton_DrawRaw(&AlertBox->rightButton, false);

  /* Draw the border */
  GUI_DrawBorderRaw(&AlertBox->object);

  /* Mark dirty zones */
  prvMarkDirtyZonesWithObject(&AlertBox->object);

  AlertBox->object.displayState = GUIDisplayState_NotHidden;
}

/**
 * @brief   Clears an alert box by drawing a transparent rectangle in it's place instead
 * @param   AlertBoxId: The Id for the item
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 */
GUIStatus GUIAlertBox_Clear(uint32_t AlertBoxId)
{
  uint32_t index = AlertBoxId - guiConfigALERT_BOX_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_ALERT_BOXES && prvAlertBox_list[index].object.id != GUI_INVALID_ID)
  {
    /* Get a pointer to the current item */
    GUIAlertBox* alertBox = &prvAlertBox_list[index];

    /* Clear the object */
    GUIObject_ClearRaw(&alertBox->object);

    /* Decrement number of objects on this layer */
    GUI_DecrementObjectsOnLayer(alertBox->object.layer);

    return GUIStatus_Success;
  }
  else
  {
    prvErrorHandler("GUIAlertBox_Clear-Invalid ID");
    return GUIStatus_InvalidId;
  }
}

/**
 * @brief   Respond to a touch of an alert box
 * @param   AlertBox: The alert box the event applies to
 * @param   Event: The event that happened, can be any value of GUITouchEvent
 * @param   XPos: X-position for event
 * @param   XPos: Y-position for event
 * @retval  None
 */
void GUAlertBox_TouchAtPosition(GUIAlertBox* AlertBox, GUITouchEvent Event, uint16_t XPos, uint16_t YPos)
{
  static GUIButton* lastActiveButton = 0;
  GUIButton* buttonTouched = 0;

  /* If no alert box hit was found, check if a button is still in touch down state and if so change it's state */
  if (Event == GUITouchEvent_None)
  {
    if (lastActiveButton != 0)
    {
      lastActiveButton->buttonState = lastActiveButton->lastButtonState;
      GUIButton_DrawRaw(lastActiveButton, true);
      lastActiveButton = 0;
    }
    return;
  }

  /* Check close button hit */
  if (prvPointIsInsideObject(XPos, YPos, &AlertBox->closeButton.object))
    buttonTouched = &AlertBox->closeButton;
  /* Check left button hit */
  else if (prvPointIsInsideObject(XPos, YPos, &AlertBox->leftButton.object))
    buttonTouched = &AlertBox->leftButton;
  /* Check right button hit */
  else if (prvPointIsInsideObject(XPos, YPos, &AlertBox->rightButton.object))
    buttonTouched = &AlertBox->rightButton;
  /* If no button hit was found, check if a button is still in touch down state and if so change it's state */
  else
  {
    if (lastActiveButton != 0)
    {
      lastActiveButton->buttonState = lastActiveButton->lastButtonState;
      GUIButton_DrawRaw(lastActiveButton, true);
      lastActiveButton = 0;
    }
    return;
  }

  if (Event == GUITouchEvent_Up)
  {
    /* Change the state of the button touched and reset lastActiveButton and lastState */
    buttonTouched->buttonState = buttonTouched->lastButtonState;
    GUIButton_DrawRaw(buttonTouched, true);
    lastActiveButton = 0;

    /* If it's the close button, clear the alert box */
    if (buttonTouched == &AlertBox->closeButton)
    {
      GUIAlertBox_Clear(AlertBox->object.id);
    }
    /* Call the callback function if there is one */
    if (AlertBox->actionButtonPressed != 0)
    {
      if (buttonTouched == &AlertBox->closeButton)
        AlertBox->actionButtonPressed(GUIAlertBoxCallbackButton_Close);
      else if (buttonTouched == &AlertBox->leftButton)
        AlertBox->actionButtonPressed(GUIAlertBoxCallbackButton_Left);
      else if (buttonTouched == &AlertBox->rightButton)
        AlertBox->actionButtonPressed(GUIAlertBoxCallbackButton_Right);
      /* TODO: */
//          if (prvBeepIsOn)
//            BUZZER_BeepNumOfTimes(1);
    }
  }
  else if (Event == GUITouchEvent_Down)
  {
    /* If we have not saved a last active button or if the last active is different from the new one */
    if (lastActiveButton == 0 || lastActiveButton != buttonTouched)
    {
      /* Reset the last button if there is one */
      if (lastActiveButton != 0)
      {
        lastActiveButton->buttonState = lastActiveButton->lastButtonState;
        GUIButton_DrawRaw(lastActiveButton, true);
      }

      /* Save the new button as last active and change it's state */
      lastActiveButton = buttonTouched;
      lastActiveButton->buttonState = GUIButtonState_TouchDown;
      GUIButton_DrawRaw(lastActiveButton, true);
    }
  }
}

#endif

#ifdef GUI_BUTTON_GRID_BOX_ENABLED
/** Button Grid Box ----------------------------------------------------------*/
/**
 * @brief   Get a pointer to the button grid box corresponding to the id
 * @param   ButtonGridBoxId: The Id for the item
 * @retval  Pointer the item or 0 if no item was found
 */
GUIButtonGridBox* GUIButtonGridBox_GetFromId(uint32_t ButtonGridBoxId)
{
  uint32_t index = ButtonGridBoxId - guiConfigBUTTON_GRID_BOX_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_BUTTON_GRID_BOXES && prvButtonGridBox_list[index].object.id != GUI_INVALID_ID)
    return &prvButtonGridBox_list[index];
  else
  {
    prvErrorHandler("GUIButtonGridBox_GetFromId-Invalid ID");
    return 0;
  }
}

/**
 * @brief   Init a button grid box and add it to the list
 * @param   ButtonGridBox: Pointer to a GUIButtonGridBox struct which data should be copied from
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 * @retval  GUIStatus_Error: If the item was hidden or something else went wrong
 */
GUIStatus GUIButtonGridBox_Init(GUIButtonGridBox* ButtonGridBox)
{
  uint32_t index = ButtonGridBox->object.id - guiConfigBUTTON_GRID_BOX_ID_OFFSET;
  GUIStatus status = GUIStatus_Success;

  /* Make sure we don't try to create more items than there's room for in the list */
  if (index < guiConfigNUMBER_OF_BUTTON_GRID_BOXES)
  {
    /* Make sure that the number of rows and columns does not exceed the limits */
    if (ButtonGridBox->numOfRows <= GUI_BUTTON_GRID_MAX_ROWS && ButtonGridBox->numOfColumns <= GUI_BUTTON_GRID_MAX_COLUMNS)
    {
      /* Copy the item to the list */
      memcpy(&prvButtonGridBox_list[index], ButtonGridBox, sizeof(GUIButtonGridBox));

      /* Get a pointer to the item */
      GUIButtonGridBox* buttonGridBox = &prvButtonGridBox_list[index];

      /* Configure the title label */
      GUILabel_Reset(&buttonGridBox->titleLabel);
      buttonGridBox->titleLabel.object.id           = GUI_INTERNAL_ID;
      buttonGridBox->titleLabel.object.xPos         = buttonGridBox->object.xPos;
      buttonGridBox->titleLabel.object.yPos         = buttonGridBox->object.yPos;
      buttonGridBox->titleLabel.object.width        = buttonGridBox->object.width;
      buttonGridBox->titleLabel.object.height       = buttonGridBox->titleHeight;
      buttonGridBox->titleLabel.object.displayState = GUIDisplayState_Hidden;
      buttonGridBox->titleLabel.object.border       = GUIBorder_NoBorder;
      buttonGridBox->titleLabel.object.layer        = buttonGridBox->object.layer;
      buttonGridBox->titleLabel.backgroundColor     = buttonGridBox->titleBackgroundColor;
      buttonGridBox->titleLabel.textColor[0]        = buttonGridBox->titleTextColor;
      buttonGridBox->titleLabel.text[0]             = buttonGridBox->title;
      buttonGridBox->titleLabel.font                = buttonGridBox->font;
      GUILabel_InitRaw(&buttonGridBox->titleLabel);

      /* Configure the close button */
      GUIButton_Reset(&buttonGridBox->closeButton);
      buttonGridBox->closeButton.object.id              = GUI_INTERNAL_ID;
      buttonGridBox->closeButton.object.xPos            = buttonGridBox->object.xPos;
      buttonGridBox->closeButton.object.yPos            = buttonGridBox->object.yPos;
      buttonGridBox->closeButton.object.width           = 50;  /* TODO: Adjustable? */
      buttonGridBox->closeButton.object.height          = buttonGridBox->titleHeight;
      buttonGridBox->closeButton.object.displayState    = GUIDisplayState_Hidden;
      buttonGridBox->closeButton.object.border          = GUIBorder_Left | GUIBorder_Top;
      buttonGridBox->closeButton.object.borderThickness = buttonGridBox->object.borderThickness;
      buttonGridBox->closeButton.object.borderColor     = buttonGridBox->object.borderColor;
      buttonGridBox->closeButton.object.layer           = buttonGridBox->object.layer;
      buttonGridBox->closeButton.state1TextColor        = buttonGridBox->titleBackgroundColor;
      buttonGridBox->closeButton.state1BackgroundColor  = buttonGridBox->titleTextColor;
      buttonGridBox->closeButton.pressedTextColor       = buttonGridBox->titleTextColor;
      buttonGridBox->closeButton.pressedBackgroundColor = COLOR_WHITE;
      buttonGridBox->closeButton.buttonState            = GUIButtonState_State1;
      buttonGridBox->closeButton.touchCallback          = 0;
      buttonGridBox->closeButton.text[0]                = "x";
      buttonGridBox->closeButton.font                   = buttonGridBox->font;
      GUIButton_InitRaw(&buttonGridBox->closeButton);

      /* Calculate grid dimensions */
      uint32_t gridItemWidth;
      uint32_t gridItemHeight;
      uint32_t gridXStart = buttonGridBox->object.xPos + buttonGridBox->padding.leftRight;
      uint32_t gridYStart = buttonGridBox->object.yPos + buttonGridBox->titleHeight + buttonGridBox->padding.topBottom;
      uint32_t numOfColumnsTotal = buttonGridBox->numOfColumns;
      if (buttonGridBox->labelColumnEnabled == true)
        numOfColumnsTotal++;
      gridItemWidth = (buttonGridBox->object.width - (numOfColumnsTotal+1)*buttonGridBox->padding.leftRight) / numOfColumnsTotal;
      gridItemHeight = (buttonGridBox->object.height - (buttonGridBox->numOfRows+1)*buttonGridBox->padding.topBottom - buttonGridBox->titleHeight) / buttonGridBox->numOfRows;

      /* Configure the label column if it's enabled */
      if (buttonGridBox->labelColumnEnabled == true)
      {
        for (uint32_t row = 0; row < buttonGridBox->numOfRows; row++)
        {
          GUILabel_Reset(&buttonGridBox->label[row]);
          buttonGridBox->label[row].object.id     = GUI_INTERNAL_ID;
          buttonGridBox->label[row].object.xPos     = gridXStart;
          buttonGridBox->label[row].object.yPos     = gridYStart + (row)*(gridItemHeight + buttonGridBox->padding.topBottom);
          buttonGridBox->label[row].object.width     = gridItemWidth;
          buttonGridBox->label[row].object.height   = gridItemHeight;
          buttonGridBox->label[row].object.displayState = GUIDisplayState_Hidden;
          buttonGridBox->label[row].object.border   = GUIBorder_NoBorder;
          buttonGridBox->label[row].object.layer    = buttonGridBox->object.layer;
          buttonGridBox->label[row].backgroundColor  = buttonGridBox->labelsBackgroundColor;
          buttonGridBox->label[row].textColor[0]    = buttonGridBox->labelsTextColor;
          buttonGridBox->label[row].text[0]      = buttonGridBox->labelText[row];
          buttonGridBox->label[row].font        = buttonGridBox->font;
          GUILabel_InitRaw(&buttonGridBox->label[row]);
        }
        /* Increment the X-start position by one grid unit now that we have initialized one column of labels */
        gridXStart += gridItemWidth + buttonGridBox->padding.leftRight;
      }

      /* Configure the grid of buttons */
      for (uint32_t row = 0; row < buttonGridBox->numOfRows; row++)
      {
        for (uint32_t column = 0; column < buttonGridBox->numOfColumns; column++)
        {
          /* Check if the button should be used or not, i.e. the text is not 0 */
          if (buttonGridBox->buttonText[row][column] != 0)
          {
            GUIButton_Reset(&buttonGridBox->button[row][column]);
            buttonGridBox->button[row][column].object.id       = GUI_INTERNAL_ID;
            buttonGridBox->button[row][column].object.xPos       = gridXStart + column*(gridItemWidth + buttonGridBox->padding.leftRight);
            buttonGridBox->button[row][column].object.yPos       = gridYStart + row*(gridItemHeight + buttonGridBox->padding.topBottom);
            buttonGridBox->button[row][column].object.width     = gridItemWidth;
            buttonGridBox->button[row][column].object.height     = gridItemHeight;
            buttonGridBox->button[row][column].object.displayState   = GUIDisplayState_Hidden;
            buttonGridBox->button[row][column].object.border     = GUIBorder_All;
            buttonGridBox->button[row][column].object.borderThickness = buttonGridBox->object.borderThickness;
            buttonGridBox->button[row][column].object.borderColor  = buttonGridBox->object.borderColor;
            buttonGridBox->button[row][column].object.layer      = buttonGridBox->object.layer;
            buttonGridBox->button[row][column].state1TextColor     = buttonGridBox->buttonsState1TextColor;
            buttonGridBox->button[row][column].state1BackgroundColor = buttonGridBox->buttonsState1BackgroundColor;
            buttonGridBox->button[row][column].state2TextColor     = buttonGridBox->buttonsState2TextColor;
            buttonGridBox->button[row][column].state2BackgroundColor = buttonGridBox->buttonsState2BackgroundColor;
            buttonGridBox->button[row][column].pressedTextColor   = buttonGridBox->buttonsPressedTextColor;
            buttonGridBox->button[row][column].pressedBackgroundColor = buttonGridBox->buttonsPressedBackgroundColor;
            buttonGridBox->button[row][column].buttonState      = GUIButtonState_State1;
            buttonGridBox->button[row][column].touchCallback     = 0;
            buttonGridBox->button[row][column].text[0]        = buttonGridBox->buttonText[row][column];
            buttonGridBox->button[row][column].font          = buttonGridBox->font;
            GUIButton_InitRaw(&buttonGridBox->button[row][column]);
          }
        }
      }

      /* If it's set to not hidden we should draw the label */
      if (buttonGridBox->object.displayState == GUIDisplayState_NotHidden)
        status = GUIButtonGridBox_Draw(buttonGridBox->object.id);
    }
    else
    {
      prvErrorHandler("GUIButtonGridBox_Init-Too many rows or columns");
      status = GUIStatus_Error;
    }
  }
  else
  {
    prvErrorHandler("GUIButtonGridBox_Init-Invalid ID");
    status = GUIStatus_InvalidId;
  }

  /* Set all the data in the item we received as a parameter to 0 so that it can be reused easily */
//  GUIButtonGridBox_Reset(ButtonGridBox);

  return status;
}

/**
 * @brief   Init a GUIButtonGridBox with color data. Can be used if you want to have the same ButtonGridBox but
 *          with other colors.
 * @param   ButtonGridBox: Pointer to a GUIButtonGridBox where the new color data is
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_Error: If something went wrong
 */
GUIStatus GUIButtonGridBox_InitColors(GUIButtonGridBox* ButtonGridBox)
{
  /* Make sure the pointer is valid */
  if (ButtonGridBox != 0)
  {
    /* Configure the title label */
    ButtonGridBox->titleLabel.backgroundColor = ButtonGridBox->titleBackgroundColor;
    ButtonGridBox->titleLabel.textColor[0]    = ButtonGridBox->titleTextColor;

    /* Configure the close button */
    ButtonGridBox->closeButton.state1TextColor        = ButtonGridBox->titleBackgroundColor;
    ButtonGridBox->closeButton.state1BackgroundColor  = ButtonGridBox->titleTextColor;
    ButtonGridBox->closeButton.pressedTextColor       = ButtonGridBox->titleTextColor;

    /* Configure the label column if it's enabled */
    if (ButtonGridBox->labelColumnEnabled == true)
    {
      for (uint32_t row = 0; row < ButtonGridBox->numOfRows; row++)
      {
        ButtonGridBox->label[row].backgroundColor = ButtonGridBox->labelsBackgroundColor;
        ButtonGridBox->label[row].textColor[0]    = ButtonGridBox->labelsTextColor;
      }
    }

    /* Configure the grid of buttons */
    for (uint32_t row = 0; row < ButtonGridBox->numOfRows; row++)
    {
      for (uint32_t column = 0; column < ButtonGridBox->numOfColumns; column++)
      {
        /* Check if the button should be used or not, i.e. the text is not 0 */
        if (ButtonGridBox->buttonText[row][column] != 0)
        {
          ButtonGridBox->button[row][column].state1TextColor     = ButtonGridBox->buttonsState1TextColor;
          ButtonGridBox->button[row][column].state1BackgroundColor = ButtonGridBox->buttonsState1BackgroundColor;
          ButtonGridBox->button[row][column].state2TextColor     = ButtonGridBox->buttonsState2TextColor;
          ButtonGridBox->button[row][column].state2BackgroundColor = ButtonGridBox->buttonsState2BackgroundColor;
          ButtonGridBox->button[row][column].pressedTextColor   = ButtonGridBox->buttonsPressedTextColor;
          ButtonGridBox->button[row][column].pressedBackgroundColor = ButtonGridBox->buttonsPressedBackgroundColor;
        }
      }
    }

    return GUIStatus_Success;
  }
  else
  {
    prvErrorHandler("GUIButtonGridBox_InitColors-Pointer is empty");
    return GUIStatus_Error;
  }
}

/**
 * @brief   Resets a button grid box by setting all it's memory to 0
 * @param   GUIButtonGridBox: Pointer to a GUIButtonGridBox struct which data should be reset for
 * @retval  None
 */
void GUIButtonGridBox_Reset(GUIButtonGridBox* ButtonGridBox)
{
  memset(ButtonGridBox, 0, sizeof(GUIButtonGridBox));
}

/**
 * @brief   Draw a specific button grid box
 * @param   ButtonGridBoxId: The Id for the item
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 */
GUIStatus GUIButtonGridBox_Draw(uint32_t ButtonGridBoxId)
{
  uint32_t index = ButtonGridBoxId - guiConfigBUTTON_GRID_BOX_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_BUTTON_GRID_BOXES && prvButtonGridBox_list[index].object.id != GUI_INVALID_ID)
  {
    /* Get a pointer to the current item */
    GUIButtonGridBox* buttonGridBox = &prvButtonGridBox_list[index];

    /* Increment number of objects on this layer if it's not already drawn before */
    if (buttonGridBox->object.displayState != GUIDisplayState_NotHidden)
      GUI_IncrementObjectsOnLayer(buttonGridBox->object.layer);

    /* Draw the item */
    GUIButtonGridBox_DrawRaw(buttonGridBox, true);

    return GUIStatus_Success;
  }
  else
  {
    prvErrorHandler("GUIButtonGridBox_Draw-Invalid ID");
    return GUIStatus_InvalidId;
  }
}

/**
 * @brief   Draw a GUIButtonGridBox object. This does not make use of the IDs so use with caution.
 * @param   ButtonGridBox: Pointer to a GUIButtonGridBox struct where data can be found
 * @param   MarkDirtyZones: If set to true it will mark the dirty zones this object hits
 * @retval  None
 */
void GUIButtonGridBox_DrawRaw(GUIButtonGridBox* ButtonGridBox, bool MarkDirtyZones)
{
  /* Draw the background rectangle */
  LCD_DrawFilledRectangleOnLayer(ButtonGridBox->backgroundColor,
      ButtonGridBox->object.xPos, ButtonGridBox->object.yPos,
      ButtonGridBox->object.width, ButtonGridBox->object.height,
      ButtonGridBox->object.layer);

  /* Draw the title bar and title text */
  GUILabel_DrawRaw(&ButtonGridBox->titleLabel, false);
  /* Draw the close button */
  GUIButton_DrawRaw(&ButtonGridBox->closeButton, false);
  /* Draw the label column if it's enabled */
  if (ButtonGridBox->labelColumnEnabled)
  {
    for (uint32_t row = 0; row < ButtonGridBox->numOfRows; row++)
      GUILabel_DrawRaw(&ButtonGridBox->label[row], false);
  }
  /* Draw the button grid */
  for (uint32_t row = 0; row < ButtonGridBox->numOfRows; row++)
  {
    for (uint32_t column = 0; column < ButtonGridBox->numOfColumns; column++)
    {
      /* Check if the button should be used or not, i.e. the text is not 0 */
      if (ButtonGridBox->buttonText[row][column] != 0)
        GUIButton_DrawRaw(&ButtonGridBox->button[row][column], false);
    }
  }
  /* Draw the border */
  GUI_DrawBorderRaw(&ButtonGridBox->object);

  /* Mark dirty zones */
  prvMarkDirtyZonesWithObject(&ButtonGridBox->object);

  ButtonGridBox->object.displayState = GUIDisplayState_NotHidden;
}

/**
 * @brief   Clears a button grid box by drawing a transparent rectangle in it's place instead
 * @param   ButtonGridBoxId: The Id for the item
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 */
GUIStatus GUIButtonGridBox_Clear(uint32_t ButtonGridBoxId)
{
  uint32_t index = ButtonGridBoxId - guiConfigBUTTON_GRID_BOX_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_BUTTON_GRID_BOXES && prvButtonGridBox_list[index].object.id != GUI_INVALID_ID)
  {
    /* Get a pointer to the current item */
    GUIButtonGridBox* buttonGridBox = &prvButtonGridBox_list[index];

    /* Clear the object */
    GUIObject_ClearRaw(&buttonGridBox->object);

    /* Decrement number of objects on this layer */
    GUI_DecrementObjectsOnLayer(buttonGridBox->object.layer);

    return GUIStatus_Success;
  }
  else
  {
    prvErrorHandler("GUIButtonGridBox_Clear-Invalid ID");
    return GUIStatus_InvalidId;
  }
}

/**
 * @brief   Set the button state of a button in a button grid box
 * @param   ButtonGridBoxId: The Id for the button grid box
 * @param   Row: The row for the button to set
 * @param   Column: The column for the button to set
 * @param   NewState: The new button state
 * @param   Redraw: Set to true if the button should be redrawn after the state has been changed
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 */
GUIStatus GUIButtonGridBox_SetButtonState(uint32_t ButtonGridBoxId, uint32_t Row, uint32_t Column, GUIButtonState NewState, bool Redraw)
{
  uint32_t index = ButtonGridBoxId - guiConfigBUTTON_GRID_BOX_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_BUTTON_GRID_BOXES && prvButtonGridBox_list[index].object.id != GUI_INVALID_ID)
  {
    /* Make sure that the row and column does not exceed the limits */
    if (Row < GUI_BUTTON_GRID_MAX_ROWS && Column < GUI_BUTTON_GRID_MAX_COLUMNS)
    {
      /* Get a pointer to the current button */
      GUIButtonGridBox* buttonGridBox = &prvButtonGridBox_list[index];

      /* Check if the button is enabled and that the new state is different from the current state */
      if (buttonGridBox->buttonText[Row][Column] != 0 && buttonGridBox->button[Row][Column].buttonState != NewState)
      {
        /* Set the button state */
        buttonGridBox->button[Row][Column].buttonState = NewState;
        if (Redraw)
          GUIButton_DrawRaw(&buttonGridBox->button[Row][Column], true);
      }
      else
      {
        /* TODO: */
//        prvErrorHandler("GUIButtonGridBox_SetButtonState-Button is not enabled or state is the same");
        return GUIStatus_Error;
      }

      return GUIStatus_Success;
    }
    else
    {
      prvErrorHandler("GUIButtonGridBox_SetButtonState-Invalid row or column");
      return GUIStatus_Error;
    }
  }
  else
  {
    prvErrorHandler("GUIButtonGridBox_SetButtonState-Invalid ID");
    return GUIStatus_InvalidId;
  }
}

/**
 * @brief   Get the button state of a button in a button grid box
 * @param   ButtonGridBoxId: The Id for the button grid box
 * @param   Row: The row for the button to set
 * @param   Column: The column for the button to set
 * @retval  The state of the button or GUIButtonState_NoState if the Id was invalid
 */
GUIButtonState GUIButtonGridBox_GetButtonState(uint32_t ButtonGridBoxId, uint32_t Row, uint32_t Column)
{
  uint32_t index = ButtonGridBoxId - guiConfigBUTTON_GRID_BOX_ID_OFFSET;
  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_BUTTON_GRID_BOXES && prvButtonGridBox_list[index].object.id != GUI_INVALID_ID)
  {
    /* Make sure that the row and column does not exceed the limits */
    if (Row < GUI_BUTTON_GRID_MAX_ROWS && Column < GUI_BUTTON_GRID_MAX_COLUMNS)
    {
      /* Get a pointer to the current button */
      GUIButtonGridBox* buttonGridBox = &prvButtonGridBox_list[index];

      /* Check if the button is enabled */
      if (buttonGridBox->buttonText[Row][Column] != 0)
      {
        return buttonGridBox->button[Row][Column].buttonState;
      }
      else
      {
        prvErrorHandler("GUIButtonGridBox_GetButtonState-Button is not enabled");
        return GUIButtonState_NoState;
      }
    }
    else
    {
      prvErrorHandler("GUIButtonGridBox_GetButtonState-Invalid row or column");
      return GUIButtonState_NoState;
    }
  }
  else
  {
    prvErrorHandler("GUIButtonGridBox_GetButtonState-Invalid ID");
    return GUIButtonState_NoState;
  }
}

/**
 * @brief   Respond to a touch of an button grid box
 * @param   ButtonGridBox: The button grid box the event applies to
 * @param   Event: The event that happened, can be any value of GUITouchEvent
 * @param   XPos: X-position for event
 * @param   XPos: Y-position for event
 * @retval  None
 */
void GUButtonGridBox_TouchAtPosition(GUIButtonGridBox* ButtonGridBox, GUITouchEvent Event, uint16_t XPos, uint16_t YPos)
{
  static GUIButton* lastActiveButton = 0;
  GUIButton* buttonTouched = 0;

  /* If no alert box hit was found, check if a button is still in touch down state and if so change it's state */
  if (Event == GUITouchEvent_None)
  {
    if (lastActiveButton != 0)
    {
      lastActiveButton->buttonState = lastActiveButton->lastButtonState;
      GUIButton_DrawRaw(lastActiveButton, true);
      lastActiveButton = 0;
    }
    return;
  }

  uint32_t row;
  uint32_t column;

  /* Check close button hit */
  if (prvPointIsInsideObject(XPos, YPos, &ButtonGridBox->closeButton.object))
    buttonTouched = &ButtonGridBox->closeButton;
  /* Check the buttons in the grid */
  else
  {
    for (row = 0; row < ButtonGridBox->numOfRows; row++)
    {
      for (column = 0; column < ButtonGridBox->numOfColumns; column++)
      {
        /* Check if a button exist at the row and column and if it's hit */
        if (ButtonGridBox->buttonText[row][column] != 0 && prvPointIsInsideObject(XPos, YPos, &ButtonGridBox->button[row][column].object))
        {
          buttonTouched = &ButtonGridBox->button[row][column];
          break;
        }
      }
      /* Break if a button has been found */
      if (buttonTouched != 0)
        break;
    }
  }

  /* If no button hit was found, check if a button is still in touch down state and if so change it's state */
  if (buttonTouched == 0)
  {
    if (lastActiveButton != 0)
    {
      lastActiveButton->buttonState = lastActiveButton->lastButtonState;
      GUIButton_DrawRaw(lastActiveButton, true);
      lastActiveButton = 0;
    }
    return;
  }

  if (Event == GUITouchEvent_Up)
  {
    /* Change the state of the button touched and reset lastActiveButton and lastState */
    buttonTouched->buttonState = buttonTouched->lastButtonState;
    GUIButton_DrawRaw(buttonTouched, true);
    lastActiveButton = 0;

    /* If it's the close button, clear the button grid box */
    if (buttonTouched == &ButtonGridBox->closeButton)
    {
      GUIButtonGridBox_Clear(ButtonGridBox->object.id);
    }
    /* Otherwise a button in the grid was hit so call the callback function with the button row and column */
    else if (ButtonGridBox->actionButtonPressed != 0)
    {
      ButtonGridBox->actionButtonPressed(row, column);
      /* TODO: */
//          if (prvBeepIsOn)
//            BUZZER_BeepNumOfTimes(1);
    }
  }
  else if (Event == GUITouchEvent_Down)
  {
    /* If we have not saved a last active button or if the last active is different from the new one */
    if (lastActiveButton == 0 || lastActiveButton != buttonTouched)
    {
      /* Reset the last button if there is one */
      if (lastActiveButton != 0)
      {
        lastActiveButton->buttonState = lastActiveButton->lastButtonState;
        GUIButton_DrawRaw(lastActiveButton, true);
      }

      /* Save the new button as last active and change it's state */
      lastActiveButton = buttonTouched;
      buttonTouched->lastButtonState = buttonTouched->buttonState;
      buttonTouched->buttonState = GUIButtonState_TouchDown;
      GUIButton_DrawRaw(buttonTouched, true);
    }
  }
}

#endif

#ifdef GUI_BUTTON_LIST_ENABLED
/** Button List --------------------------------------------------------------*/
/**
 * @brief   Get a pointer to the button list corresponding to the id
 * @param   ButtonListId: The Id for the item
 * @retval  Pointer the item or 0 if no item was found
 */
GUIButtonList* GUIButtonList_GetFromId(uint32_t ButtonListId)
{
  uint32_t index = ButtonListId - guiConfigBUTTON_LIST_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_BUTTON_LISTS && prvButtonList_list[index].object.id != GUI_INVALID_ID)
    return &prvButtonList_list[index];
  else
  {
    prvErrorHandler("GUIButtonList_GetFromId-Invalid ID");
    return 0;
  }
}

/**
 * @brief   Init a button list and add it to the list
 * @param   ButtonList: Pointer to a GUIButtonList struct which data should be copied from
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 * @retval  GUIStatus_Error: If the item was hidden or something else went wrong
 */
GUIStatus GUIButtonList_Init(GUIButtonList* ButtonList)
{
  uint32_t index = ButtonList->object.id - guiConfigBUTTON_LIST_ID_OFFSET;
  GUIStatus status = GUIStatus_Success;

  /* Make sure we don't try to create more items than there's room for in the list */
  if (index < guiConfigNUMBER_OF_BUTTON_LISTS)
  {
    /* Make sure that the number of pages and button per page does not exceed the limits */
    if (ButtonList->numOfPages <= GUI_BUTTON_LIST_MAX_PAGES && ButtonList->numOfButtonsPerPage <= GUI_BUTTON_LIST_MAX_BUTTONS_PER_PAGE)
    {
      /* Copy the item to the list */
      memcpy(&prvButtonList_list[index], ButtonList, sizeof(GUIButtonList));

      /* Get a pointer to the item */
      GUIButtonList* buttonList = &prvButtonList_list[index];

      /* Set the first page as active */
      buttonList->activePage = 0;

      /* Calculate some dimensions */
      if (ButtonList->numOfPages == 1 && ButtonList->titleEnabled == false)
        buttonList->listItemHeight = buttonList->object.height / (ButtonList->numOfButtonsPerPage);
      else if (ButtonList->numOfPages == 1)
        buttonList->listItemHeight = buttonList->object.height / (ButtonList->numOfButtonsPerPage + 1);
      else
        buttonList->listItemHeight = buttonList->object.height / (ButtonList->numOfButtonsPerPage + 2);

      /* Check if the division above left no remainder */
      if (buttonList->object.height % buttonList->listItemHeight != 0)
      {
        prvErrorHandler("GUIButtonList_Init-Dimensions are not valid, please double check");
        return GUIStatus_Error;
      }

      /* Check if the listItemHeight is high enough to fit the fonts */
      if (2*buttonList->titleFont->Height > buttonList->listItemHeight || 2*buttonList->buttonFont->Height > buttonList->listItemHeight)
      {
        prvErrorHandler("GUIButtonList_Init-Font does not fit");
        return GUIStatus_Error;
      }

      /* Init the elements for the currently active page */
      status = GUIButtonList_InitWithDataForActivePageRaw(buttonList);
    }
    else
    {
      prvErrorHandler("GUIButtonList_Init-Too many pages or buttons per page");
      status = GUIStatus_Error;
    }
  }
  else
  {
    prvErrorHandler("GUIButtonList_Init-Invalid ID");
    status = GUIStatus_InvalidId;
  }

  /* Set all the data in the item we received as a parameter to 0 so that it can be reused easily */
//  GUIButtonList_Reset(ButtonList);

  return status;
}

/**
 * @brief   Init a GUIButtonList with color data
 * @param   ButtonGridBox: Pointer to a GUIButtonList where the new color data is
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_Error: If something went wrong
 */
GUIStatus GUIButtonList_InitColors(GUIButtonList* ButtonList)
{
  /* Make sure the pointer is valid */
  if (ButtonList != 0)
  {

    /* Title label */
    if (ButtonList->titleEnabled)
    {
      ButtonList->titleLabel.backgroundColor    = ButtonList->titleBackgroundColor;
      ButtonList->titleLabel.textColor[0]      = ButtonList->titleTextColor[0];
      ButtonList->titleLabel.textColor[1]      = ButtonList->titleTextColor[1];
    }

    /* Buttons */
    uint32_t dataOffset = ButtonList->activePage * ButtonList->numOfButtonsPerPage;
    for (uint32_t index = 0; index < ButtonList->numOfButtonsPerPage; index++)
    {
      /* Only init the rest if there is a button at this index */
      if (ButtonList->buttonText[index + dataOffset][0] != 0)
      {
        ButtonList->button[index].state1TextColor     = ButtonList->buttonsState1TextColor;
        ButtonList->button[index].state1BackgroundColor = ButtonList->buttonsState1BackgroundColor;
        ButtonList->button[index].state2TextColor     = ButtonList->buttonsState2TextColor;
        ButtonList->button[index].state2BackgroundColor = ButtonList->buttonsState2BackgroundColor;
        ButtonList->button[index].pressedTextColor     = ButtonList->buttonsPressedTextColor;
        ButtonList->button[index].pressedBackgroundColor = ButtonList->buttonsPressedBackgroundColor;
      }
    }

    /* Check if we should init previous and next buttons */
    if (ButtonList->numOfPages > 1)
    {
      /* Previous page button */
      ButtonList->previousPageButton.state1TextColor     = ButtonList->buttonsState1TextColor;
      ButtonList->previousPageButton.state1BackgroundColor = ButtonList->buttonsState1BackgroundColor;
      ButtonList->previousPageButton.state2TextColor     = ButtonList->buttonsState2TextColor;
      ButtonList->previousPageButton.state2BackgroundColor = ButtonList->buttonsState2BackgroundColor;
      ButtonList->previousPageButton.pressedTextColor   = ButtonList->buttonsPressedTextColor;
      ButtonList->previousPageButton.pressedBackgroundColor = ButtonList->buttonsPressedBackgroundColor;

      /* Previous page button */
      ButtonList->nextPageButton.state1TextColor     = ButtonList->buttonsState1TextColor;
      ButtonList->nextPageButton.state1BackgroundColor = ButtonList->buttonsState1BackgroundColor;
      ButtonList->nextPageButton.state2TextColor     = ButtonList->buttonsState2TextColor;
      ButtonList->nextPageButton.state2BackgroundColor = ButtonList->buttonsState2BackgroundColor;
      ButtonList->nextPageButton.pressedTextColor   = ButtonList->buttonsPressedTextColor;
      ButtonList->nextPageButton.pressedBackgroundColor = ButtonList->buttonsPressedBackgroundColor;
    }

    return GUIStatus_Success;
  }
  else
  {
    prvErrorHandler("GUIButtonList_InitColors-Pointer is empty");
    return GUIStatus_Error;
  }
}

/**
 * @brief   Init a GUIButtonList object. This does not make use of the IDs so use with caution.
 * @param   ButtonList: Pointer to a GUIButtonList struct where data can be found
 * @retval  TODO:
 */
GUIStatus GUIButtonList_InitWithDataForActivePageRaw(GUIButtonList* ButtonList)
{
  /* Configure the elements */
  uint32_t currentYPos = ButtonList->object.yPos;

  /* Title label */
  if (ButtonList->titleEnabled)
  {
    GUILabel_Reset(&ButtonList->titleLabel);
    ButtonList->titleLabel.object.id              = GUI_INTERNAL_ID;
    ButtonList->titleLabel.object.xPos            = ButtonList->object.xPos;
    ButtonList->titleLabel.object.yPos            = currentYPos;
    ButtonList->titleLabel.object.width           = ButtonList->object.width;
    ButtonList->titleLabel.object.height          = ButtonList->listItemHeight;
    ButtonList->titleLabel.object.displayState    = GUIDisplayState_Hidden;
    ButtonList->titleLabel.object.border          = GUIBorder_NoBorder;
    ButtonList->titleLabel.object.borderThickness = ButtonList->object.borderThickness / 2;
    ButtonList->titleLabel.object.borderColor     = ButtonList->object.borderColor;
    ButtonList->titleLabel.object.layer           = ButtonList->object.layer;
    ButtonList->titleLabel.backgroundColor        = ButtonList->titleBackgroundColor;
    ButtonList->titleLabel.textColor[0]           = ButtonList->titleTextColor[0];
    ButtonList->titleLabel.textColor[1]           = ButtonList->titleTextColor[1];
    ButtonList->titleLabel.text[0]                = ButtonList->title[ButtonList->activePage][0];
    ButtonList->titleLabel.text[1]                = ButtonList->title[ButtonList->activePage][1];
    ButtonList->titleLabel.font                   = ButtonList->titleFont;
    GUILabel_InitRaw(&ButtonList->titleLabel);
    /* Increment y-position */
    currentYPos += ButtonList->listItemHeight;
  }

  /* Buttons */
  uint32_t dataOffset = ButtonList->activePage * ButtonList->numOfButtonsPerPage;
  for (uint32_t index = 0; index < ButtonList->numOfButtonsPerPage; index++)
  {
    GUIButton_Reset(&ButtonList->button[index]);
    ButtonList->button[index].object.id               = GUI_INTERNAL_ID;
    ButtonList->button[index].object.xPos             = ButtonList->object.xPos;
    ButtonList->button[index].object.yPos             = currentYPos;
    ButtonList->button[index].object.width            = ButtonList->object.width;
    ButtonList->button[index].object.height           = ButtonList->listItemHeight;
    ButtonList->button[index].object.displayState     = GUIDisplayState_Hidden;
    ButtonList->button[index].object.border           = ButtonList->buttonsBorder;
    ButtonList->button[index].object.borderThickness  = ButtonList->object.borderThickness / 2;
    ButtonList->button[index].object.borderColor      = ButtonList->object.borderColor;
    ButtonList->button[index].object.layer            = ButtonList->object.layer;

    /* Clear the text in case there was text on the page earlier */
    ButtonList->button[index].text[0]        = 0;
    ButtonList->button[index].text[1]        = 0;

    /* Only init the rest if there is a button at this index, otherwise the information above is enough */
    if (ButtonList->buttonText[index + dataOffset][0] != 0)
    {
      ButtonList->button[index].state1TextColor         = ButtonList->buttonsState1TextColor;
      ButtonList->button[index].state1BackgroundColor   = ButtonList->buttonsState1BackgroundColor;
      ButtonList->button[index].state2TextColor         = ButtonList->buttonsState2TextColor;
      ButtonList->button[index].state2BackgroundColor   = ButtonList->buttonsState2BackgroundColor;
      ButtonList->button[index].pressedTextColor        = ButtonList->buttonsPressedTextColor;
      ButtonList->button[index].pressedBackgroundColor  = ButtonList->buttonsPressedBackgroundColor;
      ButtonList->button[index].buttonState             = GUIButtonState_State1;
      ButtonList->button[index].touchCallback           = 0;
      ButtonList->button[index].text[0]                 = ButtonList->buttonText[index + dataOffset][0];
      ButtonList->button[index].text[1]                 = ButtonList->buttonText[index + dataOffset][1];
      ButtonList->button[index].font                    = ButtonList->buttonFont;
      GUIButton_InitRaw(&ButtonList->button[index]);
    }

    /* Increment y-position */
    currentYPos += ButtonList->listItemHeight;
  }

  /* Check if we should init previous and next buttons */
  if (ButtonList->numOfPages > 1)
  {
    /* Previous page button */
    GUIButton_Reset(&ButtonList->previousPageButton);
    ButtonList->previousPageButton.object.id              = GUI_INTERNAL_ID;
    ButtonList->previousPageButton.object.xPos            = ButtonList->object.xPos;
    ButtonList->previousPageButton.object.yPos            = currentYPos;
    ButtonList->previousPageButton.object.width           = ButtonList->object.width / 2;
    ButtonList->previousPageButton.object.height          = ButtonList->listItemHeight;
    ButtonList->previousPageButton.object.displayState    = GUIDisplayState_Hidden;
    ButtonList->previousPageButton.object.border          = ButtonList->previousBorder;
    ButtonList->previousPageButton.object.borderThickness = ButtonList->object.borderThickness / 2;
    ButtonList->previousPageButton.object.borderColor     = ButtonList->object.borderColor;
    ButtonList->previousPageButton.object.layer           = ButtonList->object.layer;
    ButtonList->previousPageButton.state1TextColor        = ButtonList->buttonsState1TextColor;
    ButtonList->previousPageButton.state1BackgroundColor  = ButtonList->buttonsState1BackgroundColor;
    ButtonList->previousPageButton.state2TextColor        = ButtonList->buttonsState2TextColor;
    ButtonList->previousPageButton.state2BackgroundColor  = ButtonList->buttonsState2BackgroundColor;
    ButtonList->previousPageButton.pressedTextColor       = ButtonList->buttonsPressedTextColor;
    ButtonList->previousPageButton.pressedBackgroundColor = ButtonList->buttonsPressedBackgroundColor;
    if (ButtonList->activePage == 0)
      ButtonList->previousPageButton.buttonState          = GUIButtonState_State2;
    else
      ButtonList->previousPageButton.buttonState          = GUIButtonState_State1;
    ButtonList->previousPageButton.touchCallback          = 0;
    ButtonList->previousPageButton.text[0]                = "<";
    ButtonList->previousPageButton.font                   = ButtonList->buttonFont;
    GUIButton_InitRaw(&ButtonList->previousPageButton);

    /* Next page button */
    GUIButton_Reset(&ButtonList->nextPageButton);
    ButtonList->nextPageButton.object.id              = GUI_INTERNAL_ID;
    ButtonList->nextPageButton.object.xPos            = ButtonList->object.xPos + ButtonList->object.width / 2;
    ButtonList->nextPageButton.object.yPos            = currentYPos;
    ButtonList->nextPageButton.object.width           = ButtonList->object.width / 2;
    ButtonList->nextPageButton.object.height          = ButtonList->listItemHeight;
    ButtonList->nextPageButton.object.displayState    = GUIDisplayState_Hidden;
    ButtonList->nextPageButton.object.border          = ButtonList->nextBorder;
    ButtonList->nextPageButton.object.borderThickness = ButtonList->object.borderThickness / 2;
    ButtonList->nextPageButton.object.borderColor     = ButtonList->object.borderColor;
    ButtonList->nextPageButton.object.layer           = ButtonList->object.layer;
    ButtonList->nextPageButton.state1TextColor        = ButtonList->buttonsState1TextColor;
    ButtonList->nextPageButton.state1BackgroundColor  = ButtonList->buttonsState1BackgroundColor;
    ButtonList->nextPageButton.state2TextColor        = ButtonList->buttonsState2TextColor;
    ButtonList->nextPageButton.state2BackgroundColor  = ButtonList->buttonsState2BackgroundColor;
    ButtonList->nextPageButton.pressedTextColor       = ButtonList->buttonsPressedTextColor;
    ButtonList->nextPageButton.pressedBackgroundColor = ButtonList->buttonsPressedBackgroundColor;
    if (ButtonList->activePage == ButtonList->numOfPages - 1)
      ButtonList->nextPageButton.buttonState          = GUIButtonState_State2;
    else
      ButtonList->nextPageButton.buttonState          = GUIButtonState_State1;
    ButtonList->nextPageButton.touchCallback          = 0;
    ButtonList->nextPageButton.text[0]                = ">";
    ButtonList->nextPageButton.font                   = ButtonList->buttonFont;
    GUIButton_InitRaw(&ButtonList->nextPageButton);
  }

  /* If it's set to not hidden we should draw the item */
  if (ButtonList->object.displayState == GUIDisplayState_NotHidden)
    return GUIButtonList_Draw(ButtonList->object.id);

  return GUIStatus_Success;
}


/**
 * @brief   Resets a button list by setting all it's memory to 0
 * @param   ButtonList: Pointer to a GUIButtonList struct which data should be reset for
 * @retval  None
 */
void GUIButtonList_Reset(GUIButtonList* ButtonList)
{
  memset(ButtonList, 0, sizeof(GUIButtonList));
}

/**
 * @brief  Draw a specific button list
 * @param  ButtonListId: The Id for the item
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 */
GUIStatus GUIButtonList_Draw(uint32_t ButtonListId)
{
  uint32_t index = ButtonListId - guiConfigBUTTON_LIST_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_BUTTON_LISTS && prvButtonList_list[index].object.id != GUI_INVALID_ID)
  {
    /* Get a pointer to the current item */
    GUIButtonList* buttonList = &prvButtonList_list[index];

    /* Increment number of objects on this layer if it's not already drawn before */
    if (buttonList->object.displayState != GUIDisplayState_NotHidden)
      GUI_IncrementObjectsOnLayer(buttonList->object.layer);

    /* Draw the item */
    GUIButtonList_DrawRaw(buttonList, true);

    return GUIStatus_Success;
  }
  else
  {
    prvErrorHandler("GUIButtonList_Draw-Invalid ID");
    return GUIStatus_InvalidId;
  }
}

/**
 * @brief   Draw a GUIButtonList object. This does not make use of the IDs so use with caution.
 * @param   ButtonList: Pointer to a GUIButtonList struct where data can be found
 * @param   MarkDirtyZones: If set to true it will mark the dirty zones this object hits
 * @retval  None
 */
void GUIButtonList_DrawRaw(GUIButtonList* ButtonList, bool MarkDirtyZones)
{
  /* Draw the background rectangle */
  LCD_DrawFilledRectangleOnLayer(ButtonList->backgroundColor,
      ButtonList->object.xPos, ButtonList->object.yPos,
      ButtonList->object.width, ButtonList->object.height,
      ButtonList->object.layer);

  /* Draw the title if enabled */
  if (ButtonList->titleEnabled)
    GUILabel_DrawRaw(&ButtonList->titleLabel, false);

  /* Draw the list of buttons */
  for (uint32_t index = 0; index < ButtonList->numOfButtonsPerPage; index++)
  {
    /* Draw a button if there is one at this index */
    if (ButtonList->button[index].text[0] != 0)
    {
      GUIButton_DrawRaw(&ButtonList->button[index], false);
    }
    /* Otherwise draw an empty rectangle */
    else
    {
      /* Draw the background rectangle */
      LCD_DrawFilledRectangleOnLayer(ButtonList->backgroundColor,
          ButtonList->button[index].object.xPos, ButtonList->button[index].object.yPos,
          ButtonList->button[index].object.width, ButtonList->button[index].object.height,
          ButtonList->button[index].object.layer);
      /* Draw the border */
      GUI_DrawBorderRaw(&ButtonList->button[index].object);
    }
  }

  /* Draw the previous and next buttons if needed */
  if (ButtonList->numOfPages > 1)
  {
    GUIButton_DrawRaw(&ButtonList->previousPageButton, false);
    GUIButton_DrawRaw(&ButtonList->nextPageButton, false);
  }

  /* Draw the border */
  GUI_DrawBorderRaw(&ButtonList->object);

  /* Mark dirty zones */
  prvMarkDirtyZonesWithObject(&ButtonList->object);

  ButtonList->object.displayState = GUIDisplayState_NotHidden;
}

/**
 * @brief   Clears a button list by drawing a transparent rectangle in it's place instead
 * @param   ButtonListId: The Id for the item
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 */
GUIStatus GUIButtonList_Clear(uint32_t ButtonListId)
{
  uint32_t index = ButtonListId - guiConfigBUTTON_LIST_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_BUTTON_LISTS && prvButtonList_list[index].object.id != GUI_INVALID_ID)
  {
    /* Get a pointer to the current item */
    GUIButtonList* buttonList = &prvButtonList_list[index];

    /* Clear the object */
    GUIObject_ClearRaw(&buttonList->object);

    /* Decrement number of objects on this layer */
    GUI_DecrementObjectsOnLayer(buttonList->object.layer);

    return GUIStatus_Success;
  }
  else
  {
    prvErrorHandler("GUIButtonList_Clear-Invalid ID");
    return GUIStatus_InvalidId;
  }
}

/**
 * @brief   Get the display state of a button list
 * @param   ButtonListId: The Id for the item
 * @retval  The display state if valid ID, otherwise GUIDisplayState_NoState
 */
GUIDisplayState GUIButtonList_GetDisplayState(uint32_t ButtonListId)
{
  uint32_t index = ButtonListId - guiConfigBUTTON_LIST_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_BUTTON_LISTS && prvButtonList_list[index].object.id != GUI_INVALID_ID)
    return prvButtonList_list[index].object.displayState;
  else
  {
    prvErrorHandler("GUIButtonList_GetDisplayState-Invalid ID");
    return GUIDisplayState_NoState;
  }
}

/**
 * @brief   Set the active page of a button list
 * @param   ButtonListId: The Id for the item
 * @param   NewActivePage:
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 */
GUIStatus GUIButtonList_SetActivePage(uint32_t ButtonListId, uint16_t NewActivePage)
{
  uint32_t index = ButtonListId - guiConfigBUTTON_LIST_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_BUTTON_LISTS && prvButtonList_list[index].object.id != GUI_INVALID_ID)
  {
    /* Get a pointer to the current item */
    GUIButtonList* buttonList = &prvButtonList_list[index];

    /* Make sure the new page is valid */
    if (NewActivePage < buttonList->numOfPages)
    {
      /* Only do stuff if the new page is different from the currently active one */
      if (buttonList->activePage != NewActivePage)
      {
        /* Set the new active page */
        buttonList->activePage = NewActivePage;
        /* Init the elements for the currently active page */
        return GUIButtonList_InitWithDataForActivePageRaw(buttonList);
      }
      else
      {
        prvErrorHandler("GUIButtonList_SetActivePage-New page is same as before");
        return GUIStatus_Error;
      }
    }
    else
    {
      prvErrorHandler("GUIButtonList_SetActivePage-New page is invalid");
      return GUIStatus_Error;
    }
  }
  else
  {
    prvErrorHandler("GUIButtonList_SetActivePage-Invalid ID");
    return GUIStatus_InvalidId;
  }
}

/**
 * @brief   Get the active page of a button list
 * @param   ButtonListId: The Id for the item
 * @retval  The active page if the id was valid, otherwise 0xFFF
 */
uint16_t GUIButtonList_GetActivePage(uint32_t ButtonListId)
{
  uint32_t index = ButtonListId - guiConfigBUTTON_LIST_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_BUTTON_LISTS && prvButtonList_list[index].object.id != GUI_INVALID_ID)
  {
    /* Get a pointer to the current item */
    GUIButtonList* buttonList = &prvButtonList_list[index];

    return buttonList->activePage;
  }
  else
  {
    prvErrorHandler("GUIButtonList_GetActivePage-Invalid ID");
    return 0xFFFF;
  }
}

/**
 * @brief   Set the text for a button in a button list
 * @param   ButtonListId: The Id for the item
 * @param   ButtonIndex: Index for the button
 * @param   TextRow1: Text for row 1, leave as 0 to keep old value
 * @param   TextRow2: Text for row 2, leave as 0 to keep old value
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 */
GUIStatus GUIButtonList_SetTextForButton(uint32_t ButtonListId, uint32_t ButtonIndex, char* TextRow1, char* TextRow2)
{
  uint32_t index = ButtonListId - guiConfigBUTTON_LIST_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_BUTTON_LISTS && prvButtonList_list[index].object.id != GUI_INVALID_ID)
  {
    /* Get a pointer to the current item */
    GUIButtonList* buttonList = &prvButtonList_list[index];

    /* Make sure the index is valid */
    if (ButtonIndex < GUI_BUTTON_LIST_MAX_BUTTONS - 1)
    {
      /* Set the text if there is one to set */
      if (TextRow1 != 0)
        buttonList->buttonText[ButtonIndex][0] = TextRow1;
      if (TextRow2 != 0)
        buttonList->buttonText[ButtonIndex][1] = TextRow2;

      /* Init the elements for the currently active page */
      return GUIButtonList_InitWithDataForActivePageRaw(buttonList);
    }
    else
    {
      prvErrorHandler("GUIButtonList_SetTextForButton-Invalid button index");
      return GUIStatus_Error;
    }

  }
  else
  {
    prvErrorHandler("GUIButtonList_SetTextForButton-Invalid ID");
    return GUIStatus_InvalidId;
  }
}

/**
 * @brief   Respond to a touch of a button list
 * @param   ButtonList: The button list the event applies to
 * @param   Event: The event that happened, can be any value of GUITouchEvent
 * @param   XPos: X-position for event
 * @param   XPos: Y-position for event
 * @retval  None
 */
void GUButtonList_TouchAtPosition(GUIButtonList* ButtonList, GUITouchEvent Event, uint16_t XPos, uint16_t YPos)
{
  static GUIButton* lastActiveButton = 0;
  static GUIButtonList* lastActiveButtonList = 0;
  GUIButton* buttonTouched = 0;

  /* Save this button list as the last active if it's a valid one */
  if (ButtonList != 0)
    lastActiveButtonList = ButtonList;

  /* If no alert box hit was found, check if a button is still in touch down state and if so change it's state */
  if (Event == GUITouchEvent_None)
  {
    if (lastActiveButton != 0)
    {
      lastActiveButton->buttonState = lastActiveButton->lastButtonState;
      GUIButton_DrawRaw(lastActiveButton, false);
      /* Redraw the border */
      GUI_DrawBorderRaw(&lastActiveButtonList->object);
      /* Mark the button object as dirty */
      prvMarkDirtyZonesWithObject(&lastActiveButton->object);
      lastActiveButton = 0;
    }
    lastActiveButtonList = 0;
    return;
  }

  /* Check previous and next button if they exist */
  if (ButtonList->numOfPages > 1)
  {
    /* Check previous button hit if it's enabled, i.e. not in State 2 */
    if (ButtonList->previousPageButton.buttonState != GUIButtonState_State2 && prvPointIsInsideObject(XPos, YPos, &ButtonList->previousPageButton.object))
      buttonTouched = &ButtonList->previousPageButton;
    /* Check next button hit if it's enabled, i.e. not in State 2 */
    else if (ButtonList->nextPageButton.buttonState != GUIButtonState_State2 && prvPointIsInsideObject(XPos, YPos, &ButtonList->nextPageButton.object))
      buttonTouched = &ButtonList->nextPageButton;
  }
  /* Check the buttons in the list */
  uint32_t index;
  for (index = 0; index < ButtonList->numOfButtonsPerPage; index++)
  {
    /* Check if a button exist at the index, that it's not a static text and if it's hit */
    if (ButtonList->button[index].text[0] != 0 &&
      ButtonList->buttonIsStaticText[index + ButtonList->activePage*ButtonList->numOfButtonsPerPage] == false &&
      prvPointIsInsideObject(XPos, YPos, &ButtonList->button[index].object))
    {
      buttonTouched = &ButtonList->button[index];
      break;
    }
  }
  /* If no button hit was found, check if a button is still in touch down state and if so change it's state */
  if (buttonTouched == 0)
  {
    if (lastActiveButton != 0)
    {
      lastActiveButton = 0;
      lastActiveButton->buttonState = lastActiveButton->lastButtonState;
      GUIButton_DrawRaw(lastActiveButton, true);
      /* Redraw the border */
      GUI_DrawBorderRaw(&ButtonList->object);
    }
    return;
  }

  if (Event == GUITouchEvent_Up)
  {
    /* Change the state of the button touched and reset lastActiveButton and lastState */
    lastActiveButton = 0;
    buttonTouched->buttonState = buttonTouched->lastButtonState;
    GUIButton_DrawRaw(buttonTouched, true);
    /* Redraw the border */
    GUI_DrawBorderRaw(&ButtonList->object);


    /* If it's the previous button and we're not at the first page, move one page back */
    if (buttonTouched == &ButtonList->previousPageButton && ButtonList->activePage != 0)
    {
      GUIButtonList_SetActivePage(ButtonList->object.id, ButtonList->activePage - 1);
    }
    /* If it's the next button and we're not at the last page, move one page forward */
    else if (buttonTouched == &ButtonList->nextPageButton && ButtonList->activePage != ButtonList->numOfPages - 1)
    {
      GUIButtonList_SetActivePage(ButtonList->object.id, ButtonList->activePage + 1);
    }
    /* Otherwise a button in the list was hit so call the callback function with the button index */
    else if (ButtonList->actionButtonPressed != 0)
    {
      ButtonList->actionButtonPressed(ButtonList->object.id, index + ButtonList->activePage*ButtonList->numOfButtonsPerPage);
      /* TODO: */
//          if (prvBeepIsOn)
//            BUZZER_BeepNumOfTimes(1);
    }
  }
  else if (Event == GUITouchEvent_Down)
  {
    /* If we have not saved a last active button or if the last active is different from the new one */
    if (lastActiveButton == 0 || lastActiveButton != buttonTouched)
    {
      /* Reset the last button if there is one */
      if (lastActiveButton != 0)
      {
        lastActiveButton->buttonState = lastActiveButton->lastButtonState;
        GUIButton_DrawRaw(lastActiveButton, true);
      }

      /* Save the new button as last active and change it's state */
      lastActiveButton = buttonTouched;
      lastActiveButton->buttonState = GUIButtonState_TouchDown;
      GUIButton_DrawRaw(buttonTouched, true);
      /* Redraw the border */
      GUI_DrawBorderRaw(&ButtonList->object);
    }
  }
}

#endif

#ifdef GUI_INFO_BOX_ENABLED
/** Info Box -----------------------------------------------------------------*/
/**
 * @brief   Get a pointer to the info box corresponding to the id
 * @param   InfoBoxId: The Id for the item
 * @retval  Pointer the item or 0 if no item was found
 */
GUIInfoBox* GUIInfoBox_GetFromId(uint32_t InfoBoxId)
{
  uint32_t index = InfoBoxId - guiConfigINFO_BOX_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_INFO_BOXES && prvInfoBox_list[index].object.id != GUI_INVALID_ID)
    return &prvInfoBox_list[index];
  else
  {
    prvErrorHandler("GUIInfoBox_GetFromId-Invalid ID");
    return 0;
  }
}

/**
 * @brief   Init an info box and add it to the list
 * @param   InfoBox: Pointer to a GUIInfoBox struct which data should be copied from
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 * @retval  GUIStatus_Error: If the item was hidden or something else went wrong
 */
GUIStatus GUIInfoBox_Init(GUIInfoBox* InfoBox)
{
  uint32_t index = InfoBox->object.id - guiConfigINFO_BOX_ID_OFFSET;
  GUIStatus status = GUIStatus_Success;

  /* Make sure we don't try to create more items than there's room for in the list */
  if (index < guiConfigNUMBER_OF_INFO_BOXES)
  {
    /* Copy the item to the list */
    memcpy(&prvInfoBox_list[index], InfoBox, sizeof(GUIInfoBox));

    /* Get a pointer to the item */
    GUIInfoBox* infoBox = &prvInfoBox_list[index];

    /* Configure the title label */
    GUILabel_Reset(&infoBox->titleLabel);
    infoBox->titleLabel.object.id         = GUI_INTERNAL_ID;
    infoBox->titleLabel.object.xPos       = infoBox->object.xPos;
    infoBox->titleLabel.object.yPos       = infoBox->object.yPos;
    infoBox->titleLabel.object.width       = infoBox->object.width;
    infoBox->titleLabel.object.height       = infoBox->titleHeight;
    infoBox->titleLabel.object.displayState   = GUIDisplayState_Hidden;
    infoBox->titleLabel.object.border       = GUIBorder_NoBorder;
    infoBox->titleLabel.object.layer      = infoBox->object.layer;
    infoBox->titleLabel.backgroundColor      = infoBox->titleBackgroundColor;
    infoBox->titleLabel.textColor[0]      = infoBox->titleTextColor;
    infoBox->titleLabel.text[0]          = infoBox->title;
    infoBox->titleLabel.font          = infoBox->font;
    GUILabel_InitRaw(&infoBox->titleLabel);

    /* Configure the close button */
    GUIButton_Reset(&infoBox->closeButton);
    infoBox->closeButton.object.id           = GUI_INTERNAL_ID;
    infoBox->closeButton.object.xPos         = infoBox->object.xPos;
    infoBox->closeButton.object.yPos         = infoBox->object.yPos;
    infoBox->closeButton.object.width         = 50;  /* TODO: Adjustable? */
    infoBox->closeButton.object.height         = infoBox->titleHeight;
    infoBox->closeButton.object.displayState     = GUIDisplayState_Hidden;
    infoBox->closeButton.object.border         = GUIBorder_Left | GUIBorder_Top;
    infoBox->closeButton.object.borderThickness    = infoBox->object.borderThickness;
    infoBox->closeButton.object.borderColor      = infoBox->object.borderColor;
    infoBox->closeButton.object.layer        = infoBox->object.layer;
    infoBox->closeButton.state1TextColor       = infoBox->titleBackgroundColor;
    infoBox->closeButton.state1BackgroundColor     = infoBox->titleTextColor;
    infoBox->closeButton.pressedTextColor       = infoBox->titleTextColor;
    infoBox->closeButton.pressedBackgroundColor   = COLOR_WHITE;
    infoBox->closeButton.buttonState        = GUIButtonState_State1;
    infoBox->closeButton.touchCallback         = 0;
    infoBox->closeButton.text[0]          = "x";
    infoBox->closeButton.font            = infoBox->font;
    GUIButton_InitRaw(&infoBox->closeButton);

    /* Calculate info text dimensions */
    uint16_t infoTextWidth = infoBox->object.width - 2*infoBox->padding.leftRight;
    uint16_t infoTextXPos = infoBox->object.xPos + infoBox->padding.leftRight;
    uint16_t infoTextYPos = infoBox->object.yPos + infoBox->titleHeight + infoBox->padding.topBottom;
    uint16_t infoTextHeight = infoBox->object.height - infoBox->titleHeight - 3*infoBox->padding.topBottom;

    /* Configure the info text box */
    GUIStaticTextBox_Reset(&infoBox->infoTextBox);
    infoBox->infoTextBox.object.id         = GUI_INTERNAL_ID;
    infoBox->infoTextBox.object.xPos       = infoTextXPos;
    infoBox->infoTextBox.object.yPos       = infoTextYPos;
    infoBox->infoTextBox.object.width       = infoTextWidth;
    infoBox->infoTextBox.object.height       = infoTextHeight;
    infoBox->infoTextBox.object.displayState   = GUIDisplayState_Hidden;
    infoBox->infoTextBox.object.border       = GUIBorder_NoBorder;
    infoBox->infoTextBox.object.layer      = infoBox->object.layer;
    infoBox->infoTextBox.backgroundColor    = infoBox->backgroundColor;
    infoBox->infoTextBox.textColor        = infoBox->infoTextColor;
    infoBox->infoTextBox.text          = infoBox->infoText;
    infoBox->infoTextBox.font          = infoBox->font;
    infoBox->infoTextBox.textAlignment      = infoBox->textAlignment;
    GUIStaticTextBox_InitRaw(&infoBox->infoTextBox);


    /* If it's set to not hidden we should draw the alert box */
    if (infoBox->object.displayState == GUIDisplayState_NotHidden)
      status = GUIInfoBox_Draw(infoBox->object.id);
  }
  else
  {
    prvErrorHandler("GUIInfoBox_Init-Invalid ID");
    status = GUIStatus_InvalidId;
  }

  /* Set all the data in the item we received as a parameter to 0 so that it can be reused easily */
//  GUIInfoBox_Reset(AlertBox);

  return status;
}

/**
 * @brief   Init a GUIInfoBox with color data
 * @param   InfoBox: Pointer to a GUIInfoBox where the new color data is
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_Error: If something went wrong
 */
GUIStatus GUIInfoBox_InitColors(GUIInfoBox* InfoBox)
{
  /* Make sure the pointer is valid */
  if (InfoBox != 0)
  {
    /* Configure the title label */
    InfoBox->titleLabel.backgroundColor      = InfoBox->titleBackgroundColor;
    InfoBox->titleLabel.textColor[0]      = InfoBox->titleTextColor;

    /* Configure the close button */
    InfoBox->closeButton.state1TextColor     = InfoBox->titleBackgroundColor;
    InfoBox->closeButton.state1BackgroundColor   = InfoBox->titleTextColor;
    InfoBox->closeButton.pressedTextColor     = InfoBox->titleTextColor;

    /* Configure the info text box */
    InfoBox->infoTextBox.backgroundColor    = InfoBox->backgroundColor;
    InfoBox->infoTextBox.textColor        = InfoBox->infoTextColor;

    return GUIStatus_Success;
  }
  else
  {
    prvErrorHandler("GUIInfoBox_InitColors-Pointer is empty");
    return GUIStatus_Error;
  }
}

/**
 * @brief   Resets an GUIInfoBox by setting all it's memory to 0
 * @param   InfoBox: Pointer to a GUIInfoBox struct which data should be reset for
 * @retval  None
 */
void GUIInfoBox_Reset(GUIInfoBox* InfoBox)
{
  memset(InfoBox, 0, sizeof(GUIInfoBox));
}

/**
 * @brief   Draw a specific GUIInfoBox
 * @param   InfoBoxId: The Id for the GUIInfoBox
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 */
GUIStatus GUIInfoBox_Draw(uint32_t InfoBoxId)
{
  uint32_t index = InfoBoxId - guiConfigINFO_BOX_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_INFO_BOXES && prvInfoBox_list[index].object.id != GUI_INVALID_ID)
  {
    /* Get a pointer to the current item */
    GUIInfoBox* infoBox = &prvInfoBox_list[index];

    /* Increment number of objects on this layer if it's not already drawn before */
    if (infoBox->object.displayState != GUIDisplayState_NotHidden)
      GUI_IncrementObjectsOnLayer(infoBox->object.layer);

    /* Draw the item */
    GUIInfoBox_DrawRaw(infoBox, true);

    return GUIStatus_Success;
  }
  else
  {
    prvErrorHandler("GUIInfoBox_Draw-Invalid ID");
    return GUIStatus_InvalidId;
  }
}

/**
 * @brief   Draw a GUIInfoBox object. This does not make use of the IDs so use with caution.
 * @param   InfoBox: Pointer to a GUIInfoBox struct where data can be found
 * @param   MarkDirtyZones: If set to true it will mark the dirty zones this object hits
 * @retval  None
 */
void GUIInfoBox_DrawRaw(GUIInfoBox* InfoBox, bool MarkDirtyZones)
{
  /* Draw the background rectangle */
  LCD_DrawFilledRectangleOnLayer(InfoBox->backgroundColor,
      InfoBox->object.xPos, InfoBox->object.yPos,
      InfoBox->object.width, InfoBox->object.height,
      InfoBox->object.layer);

  /* Draw the title bar and title text */
  GUILabel_DrawRaw(&InfoBox->titleLabel, false);
  /* Draw the close button */
  GUIButton_DrawRaw(&InfoBox->closeButton, false);
  /* Draw the info text */
  GUIStaticTextBox_DrawRaw(&InfoBox->infoTextBox, false);

  /* Draw the border */
  GUI_DrawBorderRaw(&InfoBox->object);

  /* Mark dirty zones */
  prvMarkDirtyZonesWithObject(&InfoBox->object);

  InfoBox->object.displayState = GUIDisplayState_NotHidden;
}

/**
 * @brief   Clears GUIInfoBox by drawing a transparent rectangle in it's place instead
 * @param   InfoBoxId: The Id for the item
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 */
GUIStatus GUIInfoBox_Clear(uint32_t InfoBoxId)
{
  uint32_t index = InfoBoxId - guiConfigINFO_BOX_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_INFO_BOXES && prvInfoBox_list[index].object.id != GUI_INVALID_ID)
  {
    /* Get a pointer to the current item */
    GUIInfoBox* infoBox = &prvInfoBox_list[index];

    /* Clear the object */
    GUIObject_ClearRaw(&infoBox->object);

    /* Decrement number of objects on this layer */
    GUI_DecrementObjectsOnLayer(infoBox->object.layer);

    return GUIStatus_Success;
  }
  else
  {
    prvErrorHandler("GUIInfoBox_Clear-Invalid ID");
    return GUIStatus_InvalidId;
  }
}

/**
 * @brief   Respond to a touch of GUIInfoBox
 * @param   InfoBox: The GUIInfoBox the event applies to
 * @param   Event: The event that happened, can be any value of GUITouchEvent
 * @param   XPos: X-position for event
 * @param   XPos: Y-position for event
 * @retval  None
 */
void GUInfoBox_TouchAtPosition(GUIInfoBox* InfoBox, GUITouchEvent Event, uint16_t XPos, uint16_t YPos)
{
  static GUIButton* lastActiveButton = 0;
  GUIButton* buttonTouched = 0;

  /* If no GUIInfoBox hit was found, check if a button is still in touch down state and if so change it's state */
  if (Event == GUITouchEvent_None)
  {
    if (lastActiveButton != 0)
    {
      lastActiveButton->buttonState = lastActiveButton->lastButtonState;
      GUIButton_DrawRaw(lastActiveButton, true);
      lastActiveButton = 0;
    }
    return;
  }

  /* Check close button hit */
  if (prvPointIsInsideObject(XPos, YPos, &InfoBox->closeButton.object))
    buttonTouched = &InfoBox->closeButton;
  /* If no button hit was found, check if a button is still in touch down state and if so change it's state */
  else
  {
    if (lastActiveButton != 0)
    {
      lastActiveButton->buttonState = lastActiveButton->lastButtonState;
      GUIButton_DrawRaw(lastActiveButton, true);
      lastActiveButton = 0;
    }
    return;
  }

  if (Event == GUITouchEvent_Up)
  {
    /* Change the state of the button touched and reset lastActiveButton and lastState */
    buttonTouched->buttonState = buttonTouched->lastButtonState;
    GUIButton_DrawRaw(buttonTouched, true);
    lastActiveButton = 0;

    /* If it's the close button, clear the alert box */
    if (buttonTouched == &InfoBox->closeButton)
    {
      GUIInfoBox_Clear(InfoBox->object.id);
    }
      /* TODO: */
//          if (prvBeepIsOn)
//            BUZZER_BeepNumOfTimes(1);
  }
  else if (Event == GUITouchEvent_Down)
  {
    /* If we have not saved a last active button or if the last active is different from the new one */
    if (lastActiveButton == 0 || lastActiveButton != buttonTouched)
    {
      /* Reset the last button if there is one */
      if (lastActiveButton != 0)
      {
        lastActiveButton->buttonState = lastActiveButton->lastButtonState;
        GUIButton_DrawRaw(lastActiveButton, true);
      }

      /* Save the new button as last active and change it's state */
      lastActiveButton = buttonTouched;
      lastActiveButton->buttonState = GUIButtonState_TouchDown;
      GUIButton_DrawRaw(lastActiveButton, true);
    }
  }
}

#endif

/** Container ----------------------------------------------------------------*/
/**
 * @brief   Get a pointer to the container corresponding to the id
 * @param   ContainerId: The Id for the item
 * @retval  Pointer the item or 0 if no item was found
 */
GUIContainer* GUIContainer_GetFromId(uint32_t ContainerId)
{
  uint32_t index = ContainerId - guiConfigCONTAINER_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_CONTAINERS && prvContainer_list[index].object.id != GUI_INVALID_ID)
    return &prvContainer_list[index];
  else
  {
    prvErrorHandler("GUIContainer_GetFromId-Invalid ID");
    return 0;
  }
}

/**
 * @brief   Init a container and add it to the list
 * @param   Container: Pointer to the container to add
 * @retval  GUIErrorStatus_Success: If everything went OK
 * @retval  GUIErrorStatus_InvalidId: If the ID is invalid
 * @retval  GUIStatus_Error: If the item was hidden or something else went wrong
 */
GUIStatus GUIContainer_Init(GUIContainer* Container)
{
  uint32_t index = Container->object.id - guiConfigCONTAINER_ID_OFFSET;
  GUIStatus status;

  /* Make sure we don't try to create more items than there's room for in the list */
  if (index < guiConfigNUMBER_OF_CONTAINERS)
  {
    /* Copy the item to the list */
    memcpy(&prvContainer_list[index], Container, sizeof(GUIContainer));

    /* Get a pointer to the item */
    GUIContainer* container = &prvContainer_list[index];

    /* Init the item */
    GUIContainer_InitRaw(container);
    status = GUIStatus_Success;
  }
  else
  {
    prvErrorHandler("GUIContainer_Init-Invalid ID");
    status = GUIStatus_InvalidId;
  }

  /* Set all the data in the item we received as a parameter to 0 so that it can be reused easily */
//  GUIContainer_Reset(Container);

  return status;
}

/**
 * @brief   Init a GUIContainer object. This does not make use of the IDs so use with caution.
 * @param   Container: Pointer to a GUIContainer struct where data can be found
 * @retval  None
 */
void GUIContainer_InitRaw(GUIContainer* Container)
{
  /* Check if the content's position should be relative to the container */
  if (Container->contentPositioning == GUIContainerContentPositioning_Relative)
  {
    /* Adjust the position for the buttons */
    for (uint32_t i = 0; i < guiConfigNUMBER_OF_BUTTONS; i++)
    {
      Container->buttons[i]->object.xPos += Container->object.xPos;
      Container->buttons[i]->object.yPos += Container->object.yPos;
    }
  }

  /* If it's set to not hidden we should draw the container */
  if (Container->object.displayState == GUIDisplayState_NotHidden)
    GUIContainer_DrawRaw(Container, true);
}

/**
 * @brief   Resets a container by setting all it's memory to 0
 * @param   Label: Pointer to a GUIContainer struct which data should be reset for
 * @retval  None
 */
void GUIContainer_Reset(GUIContainer* Container)
{
  memset(Container, 0, sizeof(GUIContainer));
}

/**
 * @brief   Draw a specific container with the specified id
 * @param   ContainerId: The Id for the container
 * @retval  GUIErrorStatus_Success: If everything went OK
 * @retval  GUIErrorStatus_InvalidId: If the ID is invalid
 */
GUIStatus GUIContainer_Draw(uint32_t ContainerId)
{
  uint32_t index = ContainerId - guiConfigCONTAINER_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_CONTAINERS && prvContainer_list[index].object.id != GUI_INVALID_ID)
  {
    /* Get a pointer to the current item */
    GUIContainer* container = &prvContainer_list[index];

    /* Increment number of objects on this layer if it's not already drawn before */
    if (container->object.displayState != GUIDisplayState_NotHidden)
      GUI_IncrementObjectsOnLayer(container->object.layer);

    /* Draw the item */
    GUIContainer_DrawRaw(container, true);

    return GUIStatus_Success;
  }
  else
  {
    prvErrorHandler("GUIContainer_Draw-Invalid ID");
    return GUIStatus_InvalidId;
  }
}

/**
 * @brief   Draw a GUIContainer object. This does not make use of the IDs so use with caution.
 * @param   Container: Pointer to a GUIContainer struct where data can be found
 * @param   MarkDirtyZones: If set to true it will mark the dirty zones this object hits
 * @retval  None
 */
void GUIContainer_DrawRaw(GUIContainer* Container, bool MarkDirtyZones)
{
  /* Draw the background rectangle */
  LCD_DrawFilledRectangleOnLayer(Container->backgroundColor,
      Container->object.xPos, Container->object.yPos,
      Container->object.width, Container->object.height,
      Container->object.layer);

  /*
   * Conditions for a object to be drawn
   * - It has to exist, (!= 0)
   * - It has to be on the same layer as the container
   * - It has to be within the container, objects that are not will not be drawn
   * - The objects container page has to be the same as that currently active for the container
   */

  /* Draw the buttons */
  for (uint32_t i = 0; i < guiConfigNUMBER_OF_BUTTONS; i++)
  {
    GUIButton* button = Container->buttons[i];
    if (button != 0 && button->object.layer == Container->object.layer &&
      prvOjectIsInsideObject(button->object, Container->object) &&
      ((button->object.containerPage & Container->activePage) || (button->object.containerPage == Container->activePage)))
    {
      GUIButton_Draw(button->object.id);
    }
  }

//    /* Draw the text boxes */
//    for (uint32_t i = 0; i < guiConfigNUMBER_OF_TEXT_BOXES; i++)
//    {
//      if (container->textBoxes[i] != 0 && ((container->textBoxes[i]->object.containerPage & container->activePage) ||
//          (container->textBoxes[i]->object.containerPage == container->activePage)))
//        GUITextBox_Draw(container->textBoxes[i]->object.id);
//    }

//    /* Draw the containers */
//    for (uint32_t i = 0; i < guiConfigNUMBER_OF_CONTAINERS; i++)
//    {
//      if (container->containers[i] != 0 && ((container->containers[i]->object.containerPage & container->activePage) ||
//          (container->containers[i]->object.containerPage == container->activePage)))
//        GUIContainer_Draw(container->containers[i]->object.id);
//    }

  /* Draw the border */
  GUI_DrawBorderRaw(&Container->object);

  /* Mark dirty zones */
  prvMarkDirtyZonesWithObject(&Container->object);

  /* Set display state */
  Container->object.displayState = GUIDisplayState_NotHidden;
}

/**
 * @brief   Clear a container with all of it's content
 * @param   ContainerId: The Id for the container
 * @retval  GUIStatus_Success: If everything went OK
 * @retval  GUIStatus_InvalidId: If the ID is invalid
 */
GUIStatus GUIContainer_Clear(uint32_t ContainerId)
{
  uint32_t index = ContainerId - guiConfigCONTAINER_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_CONTAINERS && prvContainer_list[index].object.id != GUI_INVALID_ID)
  {
    GUIContainer* container = &prvContainer_list[index];

    /* Clear the object */
    GUIObject_ClearRaw(&container->object);

    /* Decrement number of objects on this layer */
    GUI_DecrementObjectsOnLayer(container->object.layer);

    /* Hide the buttons */
    for (uint32_t i = 0; i < guiConfigNUMBER_OF_BUTTONS; i++)
    {
      if (container->buttons[i] != 0)
        container->buttons[i]->object.displayState = GUIDisplayState_Hidden;
    }

//    /* Hide the text boxes */
//    for (uint32_t i = 0; i < guiConfigNUMBER_OF_TEXT_BOXES; i++)
//    {
//      if (container->textBoxes[i] != 0)
//        container->textBoxes[i]->object.displayState = GUIDisplayState_Hidden;
//    }

//    /* Hide the containers */
//    for (uint32_t i = 0; i < guiConfigNUMBER_OF_CONTAINERS; i++)
//    {
//      if (container->containers[i] != 0)
//        container->containers[i]->object.displayState = GUIDisplayState_Hidden;
//    }

    return GUIStatus_Success;
  }
  else
  {
    prvErrorHandler("GUIContainer_Clear-Invalid ID");
    return GUIStatus_InvalidId;
  }
}

/**
  * @brief   Clear only the content in a container
  * @param   ContainerId: The Id for the container
  * @retval  GUIStatus_Success: If everything went OK
  * @retval  GUIStatus_InvalidId: If the ID is invalid
  */
GUIStatus GUIContainer_ClearContent(uint32_t ContainerId)
{
  uint32_t index = ContainerId - guiConfigCONTAINER_ID_OFFSET;

  /* Make sure the index is valid and there is an object at that index */
  if (index < guiConfigNUMBER_OF_CONTAINERS && prvContainer_list[index].object.id != GUI_INVALID_ID)
  {
    GUIContainer* container = &prvContainer_list[index];

    /* Start by clearing the whole container */
    GUIContainer_Clear(ContainerId);

    /* Then draw the background rectangle */
    LCD_DrawFilledRectangleOnLayer(container->backgroundColor,
        container->object.xPos, container->object.yPos,
        container->object.width, container->object.height,
        container->object.layer);

    /* Draw the border */
    GUI_DrawBorderRaw(&container->object);

    /* Set display state */
    container->object.displayState = GUIDisplayState_ContentHidden;

    return GUIStatus_Success;
  }
  else
  {
    prvErrorHandler("GUIContainer_ClearContent-Invalid ID");
    return GUIStatus_InvalidId;
  }
}

/** Private functions --------------------------------------------------------*/
/**
  * @brief   Error handler to handle various errors
  * @param   None
  * @retval  None
  */
static void prvErrorHandler(char* ErrorString)
{
  /* TODO: Handle the error string */
#ifdef DEBUG
  while (1);
#endif
}

/**
  * @brief   Mark the zones that the Object intersects as dirty
  * @param   Object: The object to check
  * @retval  None
  */
static void prvMarkDirtyZonesWithObject(GUIObject* Object)
{
  /* Try to take the dirty zone semaphore so that the zones are not changed while refreshing */
  if (xSemaphoreTake(xSemaphoreDirtyZones, DIRTY_ZONE_SEMAPHORE_TIMEOUT) == pdTRUE)
  {
    uint32_t xLeft = (Object->xPos) / X_DIRTY_ZONE_SIZE;
    uint32_t xRight = (Object->xPos + Object->width - 1) / X_DIRTY_ZONE_SIZE;

    uint32_t yTop = (Object->yPos) / Y_DIRTY_ZONE_SIZE;
    uint32_t yBottom = (Object->yPos + Object->height - 1) / Y_DIRTY_ZONE_SIZE;

    /* Mark all the zones that the object is intersecting as dirty */
    uint32_t xIndex, yIndex;
    for (xIndex = xLeft; xIndex <= xRight; xIndex++)
      for (yIndex = yTop; yIndex <= yBottom; yIndex++)
        prvDirtyZones[xIndex][yIndex] = true;

    prvNoDirtyZones = false;

    /* Give back the semaphore */
    xSemaphoreGive(xSemaphoreDirtyZones);
  }
}


/**
  * @brief  Check if a point (XPos, YPos) is inside and object (Object)
  * @param  Object: The  object
  * @param  XPos: X-position for the point
  * @param  YPos: Y-position for the point
  * @retval true: The point is inside
  * @retval false: The point is not inside
  */
static inline bool prvPointIsInsideObject(uint16_t XPos, uint16_t YPos, GUIObject* Object)
{
  return (XPos >= Object->xPos && XPos <= Object->xPos + Object->width &&
      YPos >= Object->yPos && YPos <= Object->yPos + Object->height);
}

/**
  * @brief  Check if an object (FirstObject) is inside the borders of another object (SecondObject)
  * @param  SmallObject: The "small" object
  * @param  BigObject: The "large" object
  * @retval true: The object is inside
  * @retval false: The object is not inside
  */
static bool prvOjectIsInsideObject(GUIObject SmallObject, GUIObject BigObject)
{
  if (SmallObject.xPos >= BigObject.xPos && SmallObject.xPos + SmallObject.width <= BigObject.xPos + BigObject.width &&
    SmallObject.yPos >= BigObject.yPos && SmallObject.yPos + SmallObject.height <= BigObject.yPos + BigObject.height)
    return true;
  else
    return false;
}

/** Interrupt Handlers -------------------------------------------------------*/
