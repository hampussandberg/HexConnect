/**
 ******************************************************************************
 * @file	simple_gui.h
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-06-14
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
#ifndef SIMPLE_GUI_H_
#define SIMPLE_GUI_H_

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "stm32f4xx_hal.h"
#include "simple_gui_config.h"

#include "color.h"
#include "buzzer.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/* Defines -------------------------------------------------------------------*/
#define IS_GUI_TEXT_FORMAT(X)	(((X) == GUITextFormat_ASCII) || \
								 ((X) == GUITextFormat_HexWithSpaces) || \
								 ((X) == GUITextFormat_HexWithoutSpaces))

/* Typedefs ------------------------------------------------------------------*/
typedef enum
{
	GUIErrorStatus_Error,
	GUIErrorStatus_Success,
	GUIErrorStatus_InvalidId,
} GUIErrorStatus;

typedef enum
{
	GUIButtonState_NoState,
	GUIButtonState_Enabled,
	GUIButtonState_Disabled,
	GUIButtonState_TouchUp,
	GUIButtonState_TouchDown,
	GUIButtonState_DisabledTouch,
} GUIButtonState;

typedef enum
{
	GUITouchEvent_Down,
	GUITouchEvent_Up,
} GUITouchEvent;

typedef enum
{
	GUILayer_0,
	GUILayer_1,
} GUILayer;

typedef enum
{
	GUIDisplayState_Hidden,
	GUIDisplayState_NotHidden,
	GUIDisplayState_ContentHidden,
	GUIDisplayState_NoState,
} GUIDisplayState;

typedef enum
{
	GUIBorder_NoBorder = 0x00,
	GUIBorder_Left = 0x01,
	GUIBorder_Right = 0x02,
	GUIBorder_Top = 0x04,
	GUIBorder_Bottom = 0x08,
} GUIBorder;

typedef enum
{
	GUIHideState_HideAll,
	GUIHideState_KeepBorders,
} GUIHideState;

typedef enum
{
	GUITextFormat_ASCII,
	GUITextFormat_HexWithSpaces,
	GUITextFormat_HexWithoutSpaces,
} GUITextFormat;

typedef enum
{
	GUITextFormatChangeStyle_LockStart, /* Lock the start address */
	GUITextFormatChangeStyle_LockEnd,	/* Lock the end address */
} GUITextFormatChangeStyle;

typedef enum
{
	GUIContainerPage_None = 0x0000,
	GUIContainerPage_1 = 0x0001,
	GUIContainerPage_2 = 0x0002,
	GUIContainerPage_3 = 0x0004,
	GUIContainerPage_4 = 0x0008,
	GUIContainerPage_5 = 0x0010,
	GUIContainerPage_6 = 0x0020,
	GUIContainerPage_7 = 0x0040,
	GUIContainerPage_8 = 0x0080,
	GUIContainerPage_9 = 0x0100,
	GUIContainerPage_10 = 0x0200,
	GUIContainerPage_11 = 0x0400,
	GUIContainerPage_12 = 0x0800,
	GUIContainerPage_All = 0xFFFF,
} GUIContainerPage;

typedef struct
{
	uint32_t left;
	uint32_t right;
	uint32_t top;
	uint32_t bottom;
} GUIPadding;

/*
 * @name	GUIObject
 * @brief	-	The basic object i Simple GUI. All other elements have a GUIObject in them.
 * 			-	The GUIObject manages the position and size of the object and it's border.
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
	uint16_t borderColor;

	/* Which page in the container the object should be on */
	GUIContainerPage containerPage;
} GUIObject;

/*
 * @name	GUIButton
 * @brief	- 	A button with a callback function which are called when running the function
 * 				GUI_CheckAllActiveButtonsForTouchEventAt() with appropriate arguments.
 * 			- 	A button can have either one or two rows of text. No check is done to make
 * 				sure the text will fit inside the button so the user has to make sure the
 * 				button is big enough.
 * 			- 	The maximum length of the text is determined by the text variable you send when
 * 				calling the GUI_AddButton function. You have to make sure you don't send any bigger
 * 				text than this using the GUI_SetButtonTextForRow function as that will probably
 * 				corrupt data! Think of it as static from when compiling.
 */
typedef struct
{
	/* Basic information about the object */
	GUIObject object;

	/* Colors */
	uint16_t enabledTextColor;
	uint16_t enabledBackgroundColor;
	uint16_t disabledTextColor;
	uint16_t disabledBackgroundColor;
	uint16_t pressedTextColor;
	uint16_t pressedBackgroundColor;

	/* The state which the button is in */
	GUIButtonState state;

	/* Pointer to a callback function called when a touch event has happened */
	void (*touchCallback)(GUITouchEvent, uint32_t);

	/* Text - Two rows of text can be displayed and it must have at least one row */
	uint8_t* text[2];
	LCDFontEnlarge textSize[2];

	uint32_t numOfChar[2];		/* These three are calculated automatically in GUI_AddButton */
	uint32_t textWidth[2];		/* --------------------------------------------------------- */
	uint32_t textHeight[2];		/* --------------------------------------------------------- */
} GUIButton;

/*
 * @name	GUITextBox
 * @brief	- 	A box that can display text of arbitrary length. When the text cursor reaches
 * 				the end of the text box size it will wrap around to the start again meaning it will
 * 				write over whatever text was written there.
 * 			- 	You can also create a static text box by setting the staticText when you add it.
 * 				This will make sure the text is centered in the box and written every time you call
 * 				the GUI_DrawTextBox. This can be useful if you want to display a label. You should
 * 				not call any GUI_Write function for a static text box.
 */
typedef struct
{
	/* Basic information about the object */
	GUIObject object;

	/* Colors */
	uint16_t textColor;
	uint16_t backgroundColor;

	/* Text */
	uint8_t* staticText;
	LCDFontEnlarge textSize;

	/* Padding */
	GUIPadding padding;

	uint32_t staticTextNumOfChar;	/* These values are calculated automatically in GUI_AddTextBox */
	uint32_t staticTextWidth;		/* ----------------------------------------------------------- */
	uint32_t staticTextHeight;		/* ----------------------------------------------------------- */
	uint32_t maxCharactersPerRow;	/* ----------------------------------------------------------- */
	uint32_t maxRows;				/* ----------------------------------------------------------- */
	uint32_t maxNumOfCharacters;	/* ----------------------------------------------------------- */
	uint32_t effectiveWidth;		/* ----------------------------------------------------------- */
	uint32_t effectiveHeight;		/* ----------------------------------------------------------- */

	/*
	 * Pointer to an array where all the text displayed in the text box is stored. The memory for this will
	 * be allocated when calling the GUI_AddTextBox and will only contain the amount that can be displayed.
	 */
	uint8_t* textBuffer;		/* Circular buffer */
	uint32_t bufferCount;		/* Number of valid characters in the buffer */
	GUITextFormat textFormat;	/* Format of the text, can be any value of GUIWriteFormat */

	/* Function pointer to where data can be read from. Arguments: buffer, start address, number of bytes to read */
	void (*dataReadFunction)(uint8_t*, uint32_t, uint32_t);
	uint32_t readStartAddress;
	uint32_t readEndAddress;
	uint32_t readMinAddress;
	uint32_t readMaxAddress;
	uint32_t readLastValidByteAddress;
	bool isScrolling;

	/* Position where the next character will be written. Referenced from the objects origin (xPos, yPos) */
	uint16_t xWritePos;
	uint16_t yWritePos;

	/* Pointer to a callback function called when a touch event has happened */
	void (*touchCallback)(GUITouchEvent, uint16_t, uint16_t);
} GUITextBox;

/*
 * @name	GUITable
 * @brief
 */
typedef struct
{
	/* Basic information about the object */
	GUIObject object;

	/* Colors */
	uint16_t textColor;
	uint16_t backgroundColor;

	/* Padding */
	GUIPadding padding;

	/* Rows and columns */
	uint32_t maxNumOfRows;

} GUITable;

/*
 * @name	GUIContainer
 * @brief	- 	A collection of other GUI items to more easily hide/show groups of items.
 * 			- 	When a container is drawn it will draw all of it's containing elements as well.
 * 				The same happens when it is hidden.
 */
typedef struct GUIContainer GUIContainer;	/* We need to typedef here because a GUIContainer can contain other GUIContainers */
struct GUIContainer
{
	/* Basic information about the object */
	GUIObject object;

	GUIHideState contentHideState;

	/* Colors */
	uint16_t backgroundColor;

	/* Store a pointer to all the object on the page for easy access and small footprint */
	GUIButton* buttons[guiConfigNUMBER_OF_BUTTONS];
	GUITextBox* textBoxes[guiConfigNUMBER_OF_TEXT_BOXES];
	GUIContainer* containers[guiConfigNUMBER_OF_CONTAINERS];

	/* The active page of the container, starts at GUIContainerPage_None */
	GUIContainerPage activePage;
	GUIContainerPage lastPage;

	/* Pointer to a callback function called when a touch event has happened */
	void (*touchCallback)(GUITouchEvent, uint16_t, uint16_t);
};

/* Function prototypes -------------------------------------------------------*/
void GUI_Init();
void GUI_DrawBorder(GUIObject Object);
void GUI_RedrawLayer(GUILayer Layer);
void GUI_SetActiveLayer(GUILayer Layer);
GUILayer GUI_GetActiveLayer();
void GUI_SetBeepOn();
void GUI_SetBeepOff();
bool GUI_BeepIsOn();

/* Button functions */
GUIButton* GUIButton_GetFromId(uint32_t ButtonId);
GUIErrorStatus GUIButton_Add(GUIButton* Button);
GUIErrorStatus GUIButton_Hide(uint32_t ButtonId);
GUIErrorStatus GUIButton_Draw(uint32_t ButtonId);
void GUIButton_DrawAll();
GUIErrorStatus GUIButton_SetState(uint32_t ButtonId, GUIButtonState State);
GUIErrorStatus GUIButton_SetTextForRow(uint32_t ButtonId, uint8_t* Text, uint32_t Row);
GUIDisplayState GUIButton_GetDisplayState(uint32_t ButtonId);
GUIDisplayState GUIButton_SetLayer(uint32_t ButtonId, GUILayer Layer);
void GUIButton_CheckAllActiveForTouchEventAt(GUITouchEvent Event, uint16_t XPos, uint16_t YPos);

/* Text box functions */
GUITextBox* GUITextBox_GetFromId(uint32_t TextBoxId);
GUIErrorStatus GUITextBox_Add(GUITextBox* TextBox);
GUIErrorStatus GUITextBox_Hide(uint32_t TextBoxId);
GUIErrorStatus GUITextBox_Draw(uint32_t TextBoxId);
void GUITextBox_DrawAll();
GUIErrorStatus GUITextBox_Clear(uint32_t TextBoxId);
GUIErrorStatus GUITextBox_ClearAndResetWritePosition(uint32_t TextBoxId);

GUIErrorStatus GUITextBox_WriteString(uint32_t TextBoxId, uint8_t* String);
GUIErrorStatus GUITextBox_WriteBuffer(uint32_t TextBoxId, uint8_t* pBuffer, uint32_t Size);
GUIErrorStatus GUITextBox_WriteBufferWithFormat(uint32_t TextBoxId, uint8_t* pBuffer, uint32_t Size, GUITextFormat Format);
GUIErrorStatus GUITextBox_FormatDataForTextBox(uint32_t TextBoxId, const uint8_t* pSourceData, const uint32_t SourceSize,
											uint8_t* pFormattedData, uint32_t* pFormattedSize);

GUIErrorStatus GUITextBox_WriteNumber(uint32_t TextBoxId, int32_t Number);
GUIErrorStatus GUITextBox_SetStaticText(uint32_t TextBoxId, uint8_t* String);
GUIErrorStatus GUITextBox_NewLine(uint32_t TextBoxId);

GUIErrorStatus GUITextBox_AppendDataFromMemory(uint32_t TextBoxId, uint32_t NewEndAddress);
GUIErrorStatus GUITextBox_RefreshCurrentDataFromMemory(uint32_t TextBoxId);
GUIErrorStatus GUITextBox_ChangeTextFormat(uint32_t TextBoxId, GUITextFormat NewFormat, GUITextFormatChangeStyle ChangeStyle);
GUIErrorStatus GUITextBox_MoveDisplayedDataNumOfRows(uint32_t TextBoxId, int32_t NumOfRows);
GUIErrorStatus GUITextBox_ClearDisplayedData(uint32_t TextBoxId);

uint32_t GUITextBox_GetNumOfCharactersDisplayed(uint32_t TextBoxId);
uint32_t GUITextBox_GetMaxNumOfCharacters(uint32_t TextBoxId);
uint32_t GUITextBox_GetMaxCharactersPerRow(uint32_t TextBoxId);
uint32_t GUITextBox_GetMaxRows(uint32_t TextBoxId);
uint32_t GUITextBox_GetReadEndAddress(uint32_t TextBoxId);
GUIErrorStatus GUITextBox_SetAddressesTo(uint32_t TextBoxId, uint32_t NewAddress);
GUIErrorStatus GUITextBox_SetLastValidByteAddress(uint32_t TextBoxId, uint32_t NewAddress);
GUIErrorStatus GUITextBox_SetWritePosition(uint32_t TextBoxId, uint16_t XPos, uint16_t YPos);
GUIErrorStatus GUITextBox_SetYWritePositionToCenter(uint32_t TextBoxId);
GUIErrorStatus GUITextBox_GetWritePosition(uint32_t TextBoxId, uint16_t* XPos, uint16_t* YPos);
GUIDisplayState GUITextBox_GetDisplayState(uint32_t TextBoxId);
bool GUITextBox_IsScrolling(uint32_t TextBoxId);

void GUITextBox_CheckAllActiveForTouchEventAt(GUITouchEvent Event, uint16_t XPos, uint16_t YPos);


/* Container functions */
GUIContainer* GUIContainer_GetFromId(uint32_t ContainerId);
GUIErrorStatus GUIContainer_Add(GUIContainer* Container);
GUIErrorStatus GUIContainer_HideContent(uint32_t ContainerId);
GUIErrorStatus GUIContainer_Hide(uint32_t ContainerId);
GUIErrorStatus GUIContainer_Draw(uint32_t ContainerId);
GUIErrorStatus GUIContainer_ChangePage(uint32_t ContainerId, GUIContainerPage NewPage);

GUIContainerPage GUIContainer_GetActivePage(uint32_t ContainerId);
GUIContainerPage GUIContainer_GetLastPage(uint32_t ContainerId);
GUIDisplayState GUIContainer_GetDisplayState(uint32_t ContainerId);

GUIErrorStatus GUIContainer_IncrementPage(uint32_t ContainerId);
GUIErrorStatus GUIContainer_DecrementPage(uint32_t ContainerId);
void GUIContainer_CheckAllActiveForTouchEventAt(GUITouchEvent Event, uint16_t XPos, uint16_t YPos);


#endif /* SIMPLE_GUI_H_ */
