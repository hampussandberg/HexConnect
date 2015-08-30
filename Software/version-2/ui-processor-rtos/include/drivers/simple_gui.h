/**
 *******************************************************************************
 * @file    simple_gui.h
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
 
/** Define to prevent recursive inclusion ------------------------------------*/
#ifndef SIMPLE_GUI_H_
#define SIMPLE_GUI_H_

/** Includes -----------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "simple_gui_config.h"

#include "color.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/** Defines ------------------------------------------------------------------*/
#define GUI_INVALID_ID    (0)
#define GUI_INTERNAL_ID    (1)

/** Typedefs -----------------------------------------------------------------*/
/*
 * GUILayer - used to determine the layer an object is on
 */
typedef enum
{
  GUILayer_1       = LCD_LAYER_1,
  GUILayer_2       = LCD_LAYER_2,
  GUILayer_3       = LCD_LAYER_3,
  GUILayer_Invalid   = LCD_LAYER_INVALID,
} GUILayer;

/*
 * GUIDisplayState - used to determine if an object should be visible or not
 */
typedef enum
{
  GUIDisplayState_Hidden,      /* Object is hidden */
  GUIDisplayState_NotHidden,    /* Object is not hidden */
  GUIDisplayState_ContentHidden,  /* Object has content that is hidden but is
                             visible itself */
  GUIDisplayState_NoState,    /* Used to indicate errors */
} GUIDisplayState;

/*
 * GUIBorder - used to set how the border of an object should look like
 */
typedef enum
{
  GUIBorder_NoBorder   = 0x00,    /* No border at all */
  GUIBorder_Left     = 0x01,    /* Border on the left */
  GUIBorder_Right   = 0x02,    /* Border on the right */
  GUIBorder_Top     = 0x04,    /* Border on the top */
  GUIBorder_Bottom   = 0x08,    /* Border on the bottom */
  GUIBorder_All    = 0x0F,    /* Border on all sides */
} GUIBorder;

/*
 * GUIButtonState - mainly used to switch between the different colors of a
 * GUIButton
 */
typedef enum
{
  GUIButtonState_NoState,      /* Used to indicate errors */
  GUIButtonState_State1,      /* Button is in state1 and can be used,
                    COLORS = state1 colors */
  GUIButtonState_State2,      /* Button is in state2 and can be used,
                    COLORS = state2 colors */
  GUIButtonState_TouchDown,    /* Button is pressed down,
                    COLORS = pressed */
  GUIButtonState_DisabledTouch,  /* Button has disabled touch,
                    COLORS = last used colors */
} GUIButtonState;

/*
 * GUITouchEvent - used when there is an touch event happening to separate
 * between "touch up" and "touch down" events
 */
typedef enum
{
  GUITouchEvent_None,
  GUITouchEvent_Down,
  GUITouchEvent_Up,
} GUITouchEvent;

/*
 * GUIStatus - used by the gui so that the programmer can see what errors
 * occured in the gui function call
 */
typedef enum
{
  GUIStatus_Error,
  GUIStatus_Success,
  GUIStatus_InvalidId,
  GUIStatus_EndReached,
  GUIStatus_LayerNotActive,
} GUIStatus;

/*
 * GUIDrawDirection - used to determine which direction to draw in, top left
 * is origin
 */
typedef enum
{
  GUIDrawDirection_Horizontal = LCD_DrawDirection_Horizontal,
  GUIDrawDirection_Vertical   = LCD_DrawDirection_Vertical,
} GUIDrawDirection;

/*
 * GUIContainerPage - used to set on what page/pages an object should be
 * visible in a container
 */
typedef enum
{
  GUIContainerPage_None   = 0x00,
  GUIContainerPage_1     = 0x01,
  GUIContainerPage_2     = 0x02,
  GUIContainerPage_3     = 0x04,
  GUIContainerPage_4     = 0x08,
  GUIContainerPage_5     = 0x10,
  GUIContainerPage_6     = 0x20,
  GUIContainerPage_7     = 0x40,
  GUIContainerPage_8     = 0x80,
  GUIContainerPage_All   = 0xFF,
} GUIContainerPage;

/*
 * GUIContainerContentPositioning - used to determine if the content of a
 * container should have their position defined as relative to the container
 * or absolute to the screen.
 */
typedef enum
{
  GUIContainerContentPositioning_Absolute,
  GUIContainerContentPositioning_Relative,
} GUIContainerContentPositioning;

typedef enum
{
  GUITextAlignment_Center,
  GUITextAlignment_Left,
  GUITextAlignment_Right,
} GUITextAlignment;

/*
 * GUIPadding - used to control the amount of blank space between an objects
 * edge and it's internal components.
 */
typedef struct
{
  uint16_t leftRight;
  uint16_t topBottom;
} GUIPadding;

/** ========================================================================= */

/*
 * @name  GUIObject
 * @brief  -  The basic object i Simple GUI. All other elements have a
 *         GUIObject in them.
 *       -  The GUIObject manages the position and size of the object
 *         and it's border.
 */
typedef struct
{
  /* Unique ID set in simple_gui_config.h for each GUI object */
  uint32_t id;

  /* Position and size */
  uint16_t xPos;
  uint16_t yPos;
  uint16_t width;
  uint16_t height;

  /* Layer where the object is */
  GUILayer layer;

  /* The display state of the object */
  GUIDisplayState displayState;

  /* Border */
  GUIBorder border;
  uint32_t borderThickness;
  guiColor borderColor;

  /* Which page in the container the object should be on */
  GUIContainerPage containerPage;
} GUIObject;

/** ========================================================================= */
/*
 * @name  GUIButton
 * @brief  -   A button with a callback function which are called when running
 *         the function GUI_CheckAllActiveButtonsForTouchEventAt() with
 *         appropriate arguments.
 *       -   A button can have either one or two rows of text. No check is
 *         done to make sure the text will fit inside the button so the
 *         user has to make sure the button is big enough.
 *       -   The maximum length of the text is determined by the text
 *         variable you send when calling the GUI_AddButton function. You
 *         have to make sure you don't send any bigger text than this
 *         using the GUI_SetButtonTextForRow function as that will
 *         probably corrupt data! Think of it as static from when
 *         compiling.
 */
typedef struct
{
  /* Basic information about the object */
  GUIObject object;

  /* Colors */
  guiColor state1TextColor;
  guiColor state1BackgroundColor;
  guiColor state2TextColor;
  guiColor state2BackgroundColor;
  guiColor pressedTextColor;
  guiColor pressedBackgroundColor;

  /* The state of the button */
  GUIButtonState buttonState;
  GUIButtonState lastButtonState;

  /* Pointer to a callback function called when a touch event has happened */
  void (*touchCallback)(GUITouchEvent, uint32_t);

  /* Two rows of text can be displayed and it must have at least one row */
  char* text[2];
  FONT* font;

  /* Internal stuff - Do not touch! */
  uint32_t numOfChar[2];
  uint32_t textWidth[2];
  uint32_t textHeight[2];
} GUIButton;

/** ========================================================================= */

/*
 * @name  GUILabel
 * @brief  -   A rectangle which can display two rows of text with different
 *         color for each row.
 *       -  Texts, colors, fonts in labels can be changed
 */
typedef struct
{
  /* Basic information about the object */
  GUIObject object;

  /* Colors */
  guiColor backgroundColor;
  guiColor textColor[2];

  /* Two rows of text can be displayed and it must have at least one row */
  char* text[2];
  FONT* font;

  /* Internal stuff - Do not touch! */
  uint32_t numOfChar[2];
  uint32_t textWidth[2];
  uint32_t textHeight[2];
} GUILabel;

/** ========================================================================= */

/*
 * @name  GUIStaticTextBox
 * @brief  -  Used to display a simple static text in a box, i.e. something
 *         that doesn't need to updated that often and is more aimed
 *         towards informational text
 *       -  The position and dimensions can be set using the object
 *       -  Background and text color can be set independently
 *       -  A font should be set for the text
 *       -  Text can be aligned vertically to left, center or right using
 *         textAlignment
 *       -  The text string will automatically switch rows when needed,
 *         this is done by looking for space characters and cutting the
 *         string there
 *       -  If there is a word in the text string that is longer than text
 *         box an error will be generated
 *       -  If the text string is longer than what will fit in the whole
 *         text box, the end will be cut off
 */
typedef struct
{
  /* Basic information about the object */
  GUIObject object;

  /* Colors */
  guiColor backgroundColor;
  guiColor textColor;

  /* Pointer to the text and a font. Vertical text alignment */
  char* text;
  FONT* font;
  GUITextAlignment textAlignment;
} GUIStaticTextBox;

#ifdef GUI_SCROLLABLE_TEXT_BOX_ENABLED
/** ========================================================================= */
/*
 * @name  GUIScrollableTextBox
 * @brief  -
 */
typedef struct
{
  /* Basic information about the object */
  GUIObject object;

  /* Colors */
  guiColor backgroundColor;
  guiColor textColor;

  /* Pointer to where the data for all the characters can be found */
  char* dataBufferStart;
  /* Pointer to a function that returns the amount of data available */
  uint32_t (*numOfCharsAvailable)();
  /* Address to the framebuffer used by the text box */
  uint32_t frameBufferStartAddress;

  /* Font used in the text box - Must be fixed font */
  FONT* fixedWidthFont;

  /* Padding */
  GUIPadding padding;

  /* Internal stuff - Do not touch! */
  char* drawnDataStart;
  char* drawnDataEnd;
  int32_t scrollOffset;
  uint32_t visibleFrameStartAddress;
  uint32_t visibleFrameEndAddress;
  uint32_t maxNumOfRowsInVisbleBox;
  uint32_t maxNumOfCharsInRow;
  uint32_t maxNumOfCharsInVisbleBox;
} GUIScrollableTextBox;
#endif

#ifdef GUI_ALERT_BOX_ENABLED
/** ========================================================================= */
/*
 * GUIAlertBoxCallbackButton - used by the callback function to tell which
 * button was pressed.
 */
typedef enum
{
  GUIAlertBoxCallbackButton_Close,
  GUIAlertBoxCallbackButton_Left,
  GUIAlertBoxCallbackButton_Right,
} GUIAlertBoxCallbackButton;

/*
 * @name  GUIAlertBox
 * @brief  -  A rectangular box with a title, close button, info text and two
 *         buttons.
 *       -  When one of the buttons are pressed the corresponding callback
 *         function will be called.
 *       -  The position and dimensions can be set using the object.
 *       -  There are various color choices for flexibility.
 *       -  The left and right button can have two rows of text.
 *       -  The font is used for all elements of the alert box.
 *       -  titleHeight sets the height of the title bar and also the
 *         height of the close button.
 *       -  leftRightButtonHeight sets the height of the left and right
 *         button.
 *       -  padding controls the padding of the info text and left/right
 *         button.
 *       -  Dimension for the info text is calculated automatically based
 *         on the other dimension. This means you have to for example
 *         increase the object width/height if you can't fit all of the
 *         info text you want.
 *       -  The text in the info is center-aligned.
 *       -  The GUI objects should not be touched by the user, but are also
 *         reset in case they were. (Better safe than sorry).
 */
typedef struct
{
  /* Basic information about the object */
  GUIObject object;

  /* Colors */
  guiColor backgroundColor;
  guiColor titleBackgroundColor;
  guiColor titleTextColor;
  guiColor infoTextColor;
  guiColor buttonTextColor;
  guiColor leftButtonColor;
  guiColor rightButtonColor;

  /*
   * Pointer to callback functions that are called when a button has been
   * pressed
   */
  void (*actionButtonPressed)(GUIAlertBoxCallbackButton);

  /* Title, info text and button texts */
  char* title;
  char* infoText;
  char* leftButtonText[2];
  char* rightButtonText[2];
  FONT* font;

  /* Dimensions and Padding */
  uint16_t titleHeight;
  uint16_t leftRightButtonHeight;
  GUIPadding padding;

  /* Internal stuff - Do not touch! */
  GUILabel titleLabel;
  GUIStaticTextBox infoTextBox;
  GUIButton closeButton;
  GUIButton leftButton;
  GUIButton rightButton;
} GUIAlertBox;
#endif

#ifdef GUI_BUTTON_GRID_BOX_ENABLED
/** ========================================================================= */
/*
 * @name  GUIButtonGridBox
 * @brief  -  A rectangular box with a title, close button, optional label
 *         column and a grid of buttons.
 *       -  When a button is pressed actionButtonPressed will be called
 *         with the row and column for the pressed button as parameters.
 *       -  The position and dimensions can be set using the object
 *       -  There are various color choices for flexibility.
 *       -  The buttons in the grid can have one row of text.
 *       -  If a certain position in the grid should be empty, just don't
 *         set that button's text to anything, or set it to 0.
 *       -  The font is used for all elements of the button grid box.
 *       -  titleHeight sets the height of the title bar and also the
 *         height of the close button.
 *       -  Dimensions for the button grid and optional label column are
 *         calculated automatically according to the dimension of the box
 *         and it's padding.
 */
typedef struct
{
  /* Basic information about the object */
  GUIObject object;

  /* Colors */
  guiColor backgroundColor;
  guiColor titleBackgroundColor;
  guiColor titleTextColor;
  guiColor labelsBackgroundColor;
  guiColor labelsTextColor;
  guiColor buttonsState1TextColor;
  guiColor buttonsState1BackgroundColor;
  guiColor buttonsState2TextColor;
  guiColor buttonsState2BackgroundColor;
  guiColor buttonsPressedTextColor;
  guiColor buttonsPressedBackgroundColor;

  /*
   * Pointer to callback functions that are called when a button has been
   * pressed
   * Parameters: Row, Column
   */
  void (*actionButtonPressed)(uint32_t, uint32_t);

  /* Title, info text and button texts */
  char* title;
  char* labelText[GUI_BUTTON_GRID_MAX_ROWS];
  /* A button with the text set to 0 will not be used */
  char* buttonText[GUI_BUTTON_GRID_MAX_ROWS][GUI_BUTTON_GRID_MAX_COLUMNS];
  FONT* font;

  /* Dimensions and Padding */
  uint16_t titleHeight;
  bool labelColumnEnabled;
  uint32_t numOfRows;
  uint32_t numOfColumns;
  GUIPadding padding;

  /*
   * Internal stuff - Do not touch!
   * The memory for these objects are allocated static to make things more
   * deterministic.
   * This of course increase the total memory usage.
   * TODO: Switch to dynamic memory allocation?
   */
  GUILabel titleLabel;
  GUIButton closeButton;
  GUILabel label[GUI_BUTTON_GRID_MAX_ROWS];
  GUIButton button[GUI_BUTTON_GRID_MAX_ROWS][GUI_BUTTON_GRID_MAX_COLUMNS];
} GUIButtonGridBox;
#endif

#ifdef GUI_BUTTON_LIST_ENABLED
/** ========================================================================= */
/*
 * @name  GUIButtonList
 * @brief  -  TODO
 */
typedef struct
{
  /* Basic information about the object */
  GUIObject object;

  /* Colors */
  guiColor backgroundColor;
  guiColor titleBackgroundColor;
  guiColor titleTextColor[2];
  guiColor buttonsState1TextColor;
  guiColor buttonsState1BackgroundColor;
  guiColor buttonsState2TextColor;
  guiColor buttonsState2BackgroundColor;
  guiColor buttonsPressedTextColor;
  guiColor buttonsPressedBackgroundColor;

  /* Borders */
  GUIBorder buttonsBorder;
  GUIBorder previousBorder;
  GUIBorder nextBorder;

  /*
   * Pointer to callback functions that are called when a button has been
   * pressed
   * Parameters: uint32_t ButtonListId, uint32_t ButtonIndex
   */
  void (*actionButtonPressed)(uint32_t, uint32_t);

  /* Title text, one for each page */
  char* title[GUI_BUTTON_LIST_MAX_PAGES][2];
  bool titleEnabled;
  FONT* titleFont;
  /* A button with the text set to 0 will be considered empty in the list */
  char* buttonText[GUI_BUTTON_LIST_MAX_BUTTONS][2];
  bool buttonIsStaticText[GUI_BUTTON_LIST_MAX_BUTTONS];
  FONT* buttonFont;

  /* Pages and item per page */
  uint16_t numOfPages;
  uint16_t numOfButtonsPerPage;

  /*
   * Internal stuff - Do not touch!
   * The memory for these objects are allocated static to make things more
   * deterministic.
   * This of course increase the total memory usage.
   * TODO: Switch to dynamic memory allocation?
   */
  GUILabel titleLabel;
  GUIButton button[GUI_BUTTON_LIST_MAX_BUTTONS_PER_PAGE];
  GUIButton previousPageButton;
  GUIButton nextPageButton;
  uint16_t activePage;
  uint16_t listItemHeight;
} GUIButtonList;
#endif

#ifdef GUI_INFO_BOX_ENABLED
/** ========================================================================= */
/*
 * @name  GUIInfoBox
 * @brief  -
 */
typedef struct
{
  /* Basic information about the object */
  GUIObject object;

  /* Colors */
  guiColor backgroundColor;
  guiColor titleBackgroundColor;
  guiColor titleTextColor;
  guiColor infoTextColor;

  /* Title and info text */
  char* title;
  char* infoText;
  FONT* font;
  GUITextAlignment textAlignment;

  /* Dimensions and Padding */
  uint16_t titleHeight;
  GUIPadding padding;

  /* Internal stuff - Do not touch! */
  GUILabel titleLabel;
  GUIStaticTextBox infoTextBox;
  GUIButton closeButton;
} GUIInfoBox;
#endif

/** ========================================================================= */
/*
 * @name  GUIContainer
 * @brief  -   A collection of other GUI items to more easily hide/show groups
 *         of items.
 *       -   When a container is drawn it will draw all of it's containing
 *         elements as well. The same happens when it is hidden.
 */
/* We need to typedef here because a container can contain other containers */
typedef struct GUIContainer GUIContainer;
struct GUIContainer
{
  /* Basic information about the object */
  GUIObject object;

  /* Colors */
  guiColor backgroundColor;

  /* Store a pointer to all objects on the page */
  GUIButton* buttons[guiConfigNUMBER_OF_BUTTONS];
  GUILabel* labels[guiConfigNUMBER_OF_LABELS];
  GUIContainer* containers[guiConfigNUMBER_OF_CONTAINERS];

  /* The active page of the container, starts at GUIContainerPage_None */
  GUIContainerPage activePage;
  GUIContainerPage lastPage;

  /* The type of positioning to use for the content of the container */
  GUIContainerContentPositioning contentPositioning;

  /* Pointer to a callback function called when a touch event has happened */
  void (*touchCallback)(GUITouchEvent, uint16_t, uint16_t);
};

/** ========================================================================= */

/** Function prototypes ------------------------------------------------------*/
void GUI_Init();
void GUI_DrawBorderRaw(GUIObject* Object);
void GUI_DrawAllLayersToDisplayBuffer();
void GUI_RefreshDisplay();
void GUI_DrawAllLayersAndRefreshDisplay();
void GUI_DrawAndRefreshDirtyZones();

void GUI_ClearLayer(guiColor Color, GUILayer Layer);
GUIStatus GUI_SetActiveLayer(GUILayer NewActiveLayer);
GUIStatus GUI_IncrementObjectsOnLayer(GUILayer Layer);
GUIStatus GUI_DecrementObjectsOnLayer(GUILayer Layer);

void GUI_TouchAtPosition(GUITouchEvent Event, uint16_t XPos, uint16_t YPos);

/** Object functions =========================================================*/
void GUIObject_ClearRaw(GUIObject* Object);

/** Button functions =========================================================*/
GUIButton* GUIButton_GetFromId(uint32_t ButtonId);
GUIStatus GUIButton_Init(GUIButton* Button);
void GUIButton_InitRaw(GUIButton* Button);
void GUIButton_Reset(GUIButton* Button);
GUIStatus GUIButton_Draw(uint32_t ButtonId);
void GUIButton_DrawRaw(GUIButton* Button, bool MarkDirtyZones);
GUIStatus GUIButton_Clear(uint32_t ButtonId);
GUIStatus GUIButton_SetButtonState(uint32_t ButtonId, GUIButtonState NewState);
GUIButtonState GUIButton_GetButtonState(uint32_t ButtonId);
GUIDisplayState GUIButton_GetDisplayState(uint32_t ButtonId);
GUIStatus GUIButton_SetLayer(uint32_t ButtonId, GUILayer NewLayer);
GUILayer GUIButton_GetLayer(uint32_t ButtonId);
void GUButton_TouchAtPosition(GUIButton* Button, GUITouchEvent Event, uint16_t XPos, uint16_t YPos);

void GUIButton_CheckAllActiveForTouchEventAt(GUITouchEvent Event, uint16_t XPos, uint16_t YPos);

/** Label Functions ==========================================================*/
GUILabel* GUILabel_GetFromId(uint32_t LabelId);
GUIStatus GUILabel_Init(GUILabel* Label);
void GUILabel_InitRaw(GUILabel* Label);
void GUILabel_Reset(GUILabel* Label);
GUIStatus GUILabel_Draw(uint32_t LabelId);
void GUILabel_DrawRaw(GUILabel* Label, bool MarkDirtyZones);
GUIStatus GUILabel_Clear(uint32_t LabelId);

/** Static Text Box Functions ================================================*/
GUIStaticTextBox* GUIStaticTextBox_GetFromId(uint32_t StaticTextBoxId);
GUIStatus GUIStaticTextBox_Init(GUIStaticTextBox* StaticTextBox);
void GUIStaticTextBox_InitRaw(GUIStaticTextBox* StaticTextBox);
void GUIStaticTextBox_Reset(GUIStaticTextBox* StaticTextBox);
GUIStatus GUIStaticTextBox_Draw(uint32_t StaticTextBoxId);
void GUIStaticTextBox_DrawRaw(GUIStaticTextBox* StaticTextBox, bool MarkDirtyZones);
GUIStatus GUIStaticTextBox_Clear(uint32_t StaticTextBoxId);

#ifdef GUI_SCROLLABLE_TEXT_BOX_ENABLED
/** Scrollable Text Box Functions ============================================*/
GUIScrollableTextBox* GUIScrollableTextBox_GetFromId(uint32_t ScrollableTextBoxId);
GUIStatus GUIScrollableTextBox_Init(GUIScrollableTextBox* ScrollableTextBox);
void GUIScrollableTextBox_InitRaw(GUIScrollableTextBox* ScrollableTextBox);
void GUIScrollableTextBox_Reset(GUIScrollableTextBox* ScrollableTextBox);
GUIStatus GUIScrollableTextBox_Draw(uint32_t ScrollableTextBoxId);
void GUIScrollableTextBox_DrawRaw(GUIScrollableTextBox* ScrollableTextBox, bool MarkDirtyZones);
void GUIScrollableTextBox_DrawDataOnFrameBufferRaw(GUIScrollableTextBox* ScrollableTextBox);
GUIStatus GUIScrollableTextBox_ScrollBuffer(uint32_t ScrollableTextBoxId, int32_t PixelsToScroll);
#endif

#ifdef GUI_ALERT_BOX_ENABLED
/** Alert Box Functions ======================================================*/
GUIAlertBox* GUIAlertBox_GetFromId(uint32_t AlertBoxId);
GUIStatus GUIAlertBox_Init(GUIAlertBox* AlertBox);
GUIStatus GUIAlertBox_InitColors(GUIAlertBox* AlertBox);
void GUIAlertBox_Reset(GUIAlertBox* AlertBox);
GUIStatus GUIAlertBox_Draw(uint32_t AlertBoxId);
void GUIAlertBox_DrawRaw(GUIAlertBox* AlertBox, bool MarkDirtyZones);
GUIStatus GUIAlertBox_Clear(uint32_t AlertBoxId);
void GUAlertBox_TouchAtPosition(GUIAlertBox* AlertBox, GUITouchEvent Event, uint16_t XPos, uint16_t YPos);
#endif

#ifdef GUI_BUTTON_GRID_BOX_ENABLED
/** Button Grid Box ==========================================================*/
GUIButtonGridBox* GUIButtonGridBox_GetFromId(uint32_t ButtonGridBoxId);
GUIStatus GUIButtonGridBox_Init(GUIButtonGridBox* ButtonGridBox);
GUIStatus GUIButtonGridBox_InitColors(GUIButtonGridBox* ButtonGridBox);
void GUIButtonGridBox_Reset(GUIButtonGridBox* ButtonGridBox);
GUIStatus GUIButtonGridBox_Draw(uint32_t ButtonGridBoxId);
void GUIButtonGridBox_DrawRaw(GUIButtonGridBox* ButtonGridBox, bool MarkDirtyZones);
GUIStatus GUIButtonGridBox_Clear(uint32_t ButtonGridBoxId);
GUIStatus GUIButtonGridBox_SetButtonState(uint32_t ButtonGridBoxId, uint32_t Row, uint32_t Column, GUIButtonState NewState, bool Redraw);
GUIButtonState GUIButtonGridBox_GetButtonState(uint32_t ButtonGridBoxId, uint32_t Row, uint32_t Column);
void GUButtonGridBox_TouchAtPosition(GUIButtonGridBox* ButtonGridBox, GUITouchEvent Event, uint16_t XPos, uint16_t YPos);
#endif

#ifdef GUI_BUTTON_LIST_ENABLED
/** Button List ==============================================================*/
GUIButtonList* GUIButtonList_GetFromId(uint32_t ButtonListId);
GUIStatus GUIButtonList_Init(GUIButtonList* ButtonList);
GUIStatus GUIButtonList_InitColors(GUIButtonList* ButtonList);
GUIStatus GUIButtonList_InitWithDataForActivePageRaw(GUIButtonList* ButtonList);
void GUIButtonList_Reset(GUIButtonList* ButtonList);
GUIStatus GUIButtonList_Draw(uint32_t ButtonListId);
void GUIButtonList_DrawRaw(GUIButtonList* ButtonList, bool MarkDirtyZones);
GUIStatus GUIButtonList_Clear(uint32_t ButtonListId);
GUIDisplayState GUIButtonList_GetDisplayState(uint32_t ButtonListId);
GUIStatus GUIButtonList_SetActivePage(uint32_t ButtonListId, uint16_t NewActivePage);
uint16_t GUIButtonList_GetActivePage(uint32_t ButtonListId);
GUIStatus GUIButtonList_SetTextForButton(uint32_t ButtonListId, uint32_t ButtonIndex, char* TextRow1, char* TextRow2);
void GUButtonList_TouchAtPosition(GUIButtonList* ButtonList, GUITouchEvent Event, uint16_t XPos, uint16_t YPos);
#endif

#ifdef GUI_INFO_BOX_ENABLED
/** Info Box =================================================================*/
GUIInfoBox* GUIInfoBox_GetFromId(uint32_t InfoBoxId);
GUIStatus GUIInfoBox_Init(GUIInfoBox* InfoBox);
GUIStatus GUIInfoBox_InitColors(GUIInfoBox* InfoBox);
void GUIInfoBox_Reset(GUIInfoBox* InfoBox);
GUIStatus GUIInfoBox_Draw(uint32_t InfoBoxId);
void GUIInfoBox_DrawRaw(GUIInfoBox* InfoBox, bool MarkDirtyZones);
GUIStatus GUIInfoBox_Clear(uint32_t InfoBoxId);
void GUInfoBox_TouchAtPosition(GUIInfoBox* InfoBox, GUITouchEvent Event, uint16_t XPos, uint16_t YPos);
#endif

/** Container functions ======================================================*/
GUIContainer* GUIContainer_GetFromId(uint32_t ContainerId);
GUIStatus GUIContainer_Init(GUIContainer* Container);
void GUIContainer_InitRaw(GUIContainer* Container);
void GUIContainer_Reset(GUIContainer* Container);
GUIStatus GUIContainer_Draw(uint32_t ContainerId);
void GUIContainer_DrawRaw(GUIContainer* Container, bool MarkDirtyZones);
GUIStatus GUIContainer_Clear(uint32_t ContainerId);
GUIStatus GUIContainer_ClearContent(uint32_t ContainerId);

#endif /* SIMPLE_GUI_H_ */
