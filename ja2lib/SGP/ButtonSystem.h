//*****************************************************************************************************
//	Button System.h
//
//	by Kris Morness (originally created by Bret Rowden)
//*****************************************************************************************************

#ifndef __BUTTON_SYSTEM_H
#define __BUTTON_SYSTEM_H

#include "SGP/ButtonSoundControl.h"
#include "SGP/MouseSystem.h"
#include "SGP/Types.h"

struct GRect;
struct VObject;
struct _GUI_BUTTON;

// Moved here from Button System.c by DB 99/01/07
// Names of the default generic button image files.
#define DEFAULT_GENERIC_BUTTON_OFF "GENBUTN.STI"
#define DEFAULT_GENERIC_BUTTON_ON "GENBUTN2.STI"
#define DEFAULT_GENERIC_BUTTON_OFF_HI "GENBUTN3.STI"
#define DEFAULT_GENERIC_BUTTON_ON_HI "GENBUTN4.STI"

#define BUTTON_TEXT_LEFT -1
#define BUTTON_TEXT_CENTER 0
#define BUTTON_TEXT_RIGHT 1

#define TEXT_LJUSTIFIED BUTTON_TEXT_LEFT
#define TEXT_CJUSTIFIED BUTTON_TEXT_CENTER
#define TEXT_RJUSTIFIED BUTTON_TEXT_RIGHT

// Some GUI_BUTTON system defines
#define BUTTON_USE_DEFAULT -1
#define BUTTON_NO_FILENAME NULL
#define BUTTON_NO_CALLBACK NULL
#define BUTTON_NO_IMAGE -1
#define BUTTON_NO_SLOT -1

#define BUTTON_INIT 1
#define BUTTON_WAS_CLICKED 2

// effects how the button is rendered.
#define BUTTON_TYPES (BUTTON_QUICK | BUTTON_GENERIC | BUTTON_HOT_SPOT | BUTTON_CHECKBOX)
// effects how the button is processed
#define BUTTON_TYPE_MASK \
  (BUTTON_NO_TOGGLE | BUTTON_ALLOW_DISABLED_CALLBACK | BUTTON_CHECKBOX | BUTTON_IGNORE_CLICKS)

// button flags
#define BUTTON_TOGGLE 0x00000000
#define BUTTON_QUICK 0x00000000
#define BUTTON_ENABLED 0x00000001
#define BUTTON_CLICKED_ON 0x00000002
#define BUTTON_NO_TOGGLE 0x00000004
#define BUTTON_CLICK_CALLBACK 0x00000008
#define BUTTON_MOVE_CALLBACK 0x00000010
#define BUTTON_GENERIC 0x00000020
#define BUTTON_HOT_SPOT 0x00000040
#define BUTTON_SELFDELETE_IMAGE 0x00000080
#define BUTTON_DELETION_PENDING 0x00000100
#define BUTTON_ALLOW_DISABLED_CALLBACK 0x00000200
#define BUTTON_DIRTY 0x00000400
#define BUTTON_SAVEBACKGROUND 0x00000800
#define BUTTON_CHECKBOX 0x00001000
#define BUTTON_NEWTOGGLE 0x00002000
#define BUTTON_FORCE_UNDIRTY \
  0x00004000  // no matter what happens this buttons does NOT get marked dirty
#define BUTTON_IGNORE_CLICKS 0x00008000  // Ignore any clicks on this button
#define BUTTON_DISABLED_CALLBACK 0x80000000

#define BUTTON_SOUND_NONE 0x00
#define BUTTON_SOUND_CLICKED_ON 0x01
#define BUTTON_SOUND_CLICKED_OFF 0x02
#define BUTTON_SOUND_MOVED_ONTO 0x04
#define BUTTON_SOUND_MOVED_OFF_OF 0x08
#define BUTTON_SOUND_DISABLED_CLICK 0x10
#define BUTTON_SOUND_DISABLED_MOVED_ONTO 0x20
#define BUTTON_SOUND_DISABLED_MOVED_OFF_OF 0x40
#define BUTTON_SOUND_ALREADY_PLAYED 0X80

#define BUTTON_SOUND_ALL_EVENTS 0xff

// Internal use!
#define GUI_SND_CLK_ON BUTTON_SOUND_CLICKED_ON
#define GUI_SND_CLK_OFF BUTTON_SOUND_CLICKED_OFF
#define GUI_SND_MOV_ON BUTTON_SOUND_MOVED_ONTO
#define GUI_SND_MOV_OFF BUTTON_SOUND_MOVED_OFF_OF
#define GUI_SND_DCLK BUTTON_SOUND_DISABLED_CLICK
#define GUI_SND_DMOV BUTTON_SOUND_DISABLED_MOVED_ONTO

extern uint32_t FRAME_BUFFER;

// GUI_BUTTON callback function type
typedef void (*GUI_CALLBACK)(struct _GUI_BUTTON *, int32_t);

// GUI_BUTTON structure definitions.
typedef struct _GUI_BUTTON {
  int32_t IDNum;               // ID Number, contains it's own button number
  uint32_t ImageNum;           // Image number to use (see DOCs for details)
  struct MOUSE_REGION Area;    // Mouse System's mouse region to use for this button
  GUI_CALLBACK ClickCallback;  // Button Callback when button is clicked
  GUI_CALLBACK MoveCallback;   // Button Callback when mouse moved on this region
  int16_t Cursor;              // Cursor to use for this button
  uint32_t uiFlags;            // Button state flags etc.( 32-bit )
  uint32_t uiOldFlags;         // Old flags from previous render loop
  int16_t XLoc;                // Coordinates where button is on the screen
  int16_t YLoc;
  uintptr_t UserData[4];  // Place holder for user data etc.
  int16_t Group;          // Group this button belongs to (see DOCs)
  int8_t bDefaultStatus;
  // Button disabled style
  int8_t bDisabledStyle;
  // For buttons with text
  wchar_t *string;      // the string
  uint16_t usFont;      // font for text
  BOOLEAN fMultiColor;  // font is a multi-color font
  int16_t sForeColor;   // text colors if there is text
  int16_t sShadowColor;
  int16_t sForeColorDown;  // text colors when button is down (optional)
  int16_t sShadowColorDown;
  int16_t sForeColorHilited;  // text colors when button is down (optional)
  int16_t sShadowColorHilited;
  int8_t bJustification;  // BUTTON_TEXT_LEFT, BUTTON_TEXT_CENTER, BUTTON_TEXT_RIGHT
  int8_t bTextXOffset;
  int8_t bTextYOffset;
  int8_t bTextXSubOffSet;
  int8_t bTextYSubOffSet;
  BOOLEAN fShiftText;
  int16_t sWrappedWidth;
  // For buttons with icons (don't confuse this with quickbuttons which have up to 5 states )
  int32_t iIconID;
  int16_t usIconIndex;
  int8_t bIconXOffset;  //-1 means horizontally centered
  int8_t bIconYOffset;  //-1 means vertically centered
  BOOLEAN fShiftImage;  // if true, icon is shifted +1,+1 when button state is down.

  uint8_t ubToggleButtonOldState;  // Varibles for new toggle buttons that work
  uint8_t ubToggleButtonActivated;

  int32_t BackRect;  // Handle to a Background Rectangle
  uint8_t ubSoundSchemeID;
} GUI_BUTTON;

#define MAX_BUTTONS 400

extern GUI_BUTTON *ButtonList[MAX_BUTTONS];  // Button System's Main Button List

#define GetButtonPtr(x) (((x >= 0) && (x < MAX_BUTTONS)) ? ButtonList[x] : NULL)

// Struct definition for the QuickButton pictures.
typedef struct {
  struct VObject *vobj;  // The Image itself
  int32_t Grayed;        // Index to use for a "Grayed-out" button
  int32_t OffNormal;     // Index to use when button is OFF
  int32_t OffHilite;     // Index to use when button is OFF w/ hilite on it
  int32_t OnNormal;      // Index to use when button is ON
  int32_t OnHilite;      // Index to use when button is ON w/ hilite on it
  uint32_t MaxWidth;     // Width of largest image in use
  uint32_t MaxHeight;    // Height of largest image in use
  uint32_t fFlags;       // Special image flags
} BUTTON_PICS;

#define MAX_BUTTON_PICS 256

extern BUTTON_PICS ButtonPictures[MAX_BUTTON_PICS];

void ShutdownButtonImageManager(void);
BOOLEAN InitButtonSystem(void);
void ShutdownButtonSystem(void);

int16_t FindFreeIconSlot(void);
int32_t FindFreeButtonSlot(void);
int16_t FindFreeGenericSlot(void);
int16_t FindFreeIconSlot(void);
int32_t GetNextButtonNumber(void);

// Now used by Wizardry -- DB
void SetButtonFastHelpText(int32_t iButton, wchar_t *Text);

void SetBtnHelpEndCallback(int32_t iButton, MOUSE_HELPTEXT_DONE_CALLBACK CallbackFxn);
// void DisplayFastHelp(GUI_BUTTON *b);
void RenderButtonsFastHelp(void);

#define RenderButtonsFastHelp() RenderFastHelp()

BOOLEAN SetButtonSavedRect(int32_t iButton);
void FreeButtonSavedRect(int32_t iButton);

int16_t LoadGenericButtonIcon(char *filename);
BOOLEAN UnloadGenericButtonIcon(int16_t GenImg);
int32_t LoadButtonImage(char *filename, int32_t Grayed, int32_t OffNormal, int32_t OffHilite,
                        int32_t OnNormal, int32_t OnHilite);
int32_t UseLoadedButtonImage(int32_t LoadedImg, int32_t Grayed, int32_t OffNormal,
                             int32_t OffHilite, int32_t OnNormal, int32_t OnHilite);
int32_t UseVObjAsButtonImage(struct VObject *hVObject, int32_t Grayed, int32_t OffNormal,
                             int32_t OffHilite, int32_t OnNormal, int32_t OnHilite);
void UnloadButtonImage(int32_t Index);
int16_t LoadGenericButtonImages(char *GrayName, char *OffNormName, char *OffHiliteName,
                                char *OnNormName, char *OnHiliteName, char *BkGrndName,
                                int16_t Index, int16_t OffsetX, int16_t OffsetY);
BOOLEAN UnloadGenericButtonImage(int16_t GenImg);

BOOLEAN EnableButton(int32_t iButtonID);
BOOLEAN DisableButton(int32_t iButtonID);
void RemoveButton(int32_t iButtonID);
void HideButton(int32_t iButtonID);
void ShowButton(int32_t iButton);

void RenderButtons(void);
BOOLEAN DrawButton(int32_t iButtonID);
void DrawButtonFromPtr(GUI_BUTTON *b);

// Base button types
void DrawGenericButton(GUI_BUTTON *b);
void DrawQuickButton(GUI_BUTTON *b);
void DrawCheckBoxButton(GUI_BUTTON *b);
// Additional layers on buttons that can exist in any combination on generic or quick buttons
// To do so, use the new specify functions below.
void DrawIconOnButton(GUI_BUTTON *b);
void DrawTextOnButton(GUI_BUTTON *b);

extern BOOLEAN gfIgnoreShutdownAssertions;
extern BOOLEAN gfRenderHilights;
#define EnableHilightsAndHelpText() gfRenderHilights = TRUE;
#define DisableHilightsAndHelpText() gfRenderHilights = FALSE;

// Providing you have allocated your own image, this is a somewhat simplified function.
int32_t QuickCreateButton(uint32_t Image, int16_t xloc, int16_t yloc, int32_t Type,
                          int16_t Priority, GUI_CALLBACK MoveCallback, GUI_CALLBACK ClickCallback);

// A hybrid of QuickCreateButton.  Takes a lot less parameters, but makes more assumptions.  It self
// manages the loading, and deleting of the image.  The size of the image determines the size of the
// button.  It also uses the default move callback which emulates Win95.  Finally, it sets the
// priority to normal.  The function you choose also determines the type of button (toggle,
// notoggle, or newtoggle)
int32_t CreateEasyNoToggleButton(int32_t x, int32_t y, char *filename, GUI_CALLBACK ClickCallback);
int32_t CreateEasyToggleButton(int32_t x, int32_t y, char *filename, GUI_CALLBACK ClickCallback);
int32_t CreateEasyNewToggleButton(int32_t x, int32_t y, char *filename, GUI_CALLBACK ClickCallback);
// Same as above, but accepts specify toggle type
int32_t CreateEasyButton(int32_t x, int32_t y, char *filename, int32_t Type,
                         GUI_CALLBACK ClickCallback);
// Same as above, but accepts priority specification.
int32_t CreateSimpleButton(int32_t x, int32_t y, char *filename, int32_t Type, int16_t Priority,
                           GUI_CALLBACK ClickCallback);

int32_t CreateCheckBoxButton(int16_t x, int16_t y, char *filename, int16_t Priority,
                             GUI_CALLBACK ClickCallback);
int32_t CreateIconButton(int16_t Icon, int16_t IconIndex, int16_t GenImg, int16_t xloc,
                         int16_t yloc, int16_t w, int16_t h, int32_t Type, int16_t Priority,
                         GUI_CALLBACK MoveCallback, GUI_CALLBACK ClickCallback);
int32_t CreateHotSpot(int16_t xloc, int16_t yloc, int16_t Width, int16_t Height, int16_t Priority,
                      GUI_CALLBACK MoveCallback, GUI_CALLBACK ClickCallback);

int32_t CreateTextButton(wchar_t *string, uint32_t uiFont, int16_t sForeColor, int16_t sShadowColor,
                         int16_t GenImg, int16_t xloc, int16_t yloc, int16_t w, int16_t h,
                         int32_t Type, int16_t Priority, GUI_CALLBACK MoveCallback,
                         GUI_CALLBACK ClickCallback);
int32_t CreateIconAndTextButton(int32_t Image, wchar_t *string, uint32_t uiFont, int16_t sForeColor,
                                int16_t sShadowColor, int16_t sForeColorDown,
                                int16_t sShadowColorDown, int8_t bJustification, int16_t xloc,
                                int16_t yloc, int32_t Type, int16_t Priority,
                                GUI_CALLBACK MoveCallback, GUI_CALLBACK ClickCallback);

// New functions
void SpecifyButtonText(int32_t iButtonID, wchar_t *string);
void SpecifyButtonFont(int32_t iButtonID, uint32_t uiFont);
void SpecifyButtonMultiColorFont(int32_t iButtonID, BOOLEAN fMultiColor);
void SpecifyButtonUpTextColors(int32_t iButtonID, int16_t sForeColor, int16_t sShadowColor);
void SpecifyButtonDownTextColors(int32_t iButtonID, int16_t sForeColorDown,
                                 int16_t sShadowColorDown);
void SpecifyButtonHilitedTextColors(int32_t iButtonID, int16_t sForeColorHilited,
                                    int16_t sShadowColorHilited);
void SpecifyButtonTextJustification(int32_t iButtonID, int8_t bJustification);
void SpecifyGeneralButtonTextAttributes(int32_t iButtonID, wchar_t *string, int32_t uiFont,
                                        int16_t sForeColor, int16_t sShadowColor);
void SpecifyFullButtonTextAttributes(int32_t iButtonID, wchar_t *string, int32_t uiFont,
                                     int16_t sForeColor, int16_t sShadowColor,
                                     int16_t sForeColorDown, int16_t sShadowColorDown,
                                     int8_t bJustification);
void SpecifyGeneralButtonTextAttributes(int32_t iButtonID, wchar_t *string, int32_t uiFont,
                                        int16_t sForeColor, int16_t sShadowColor);
void SpecifyButtonTextOffsets(int32_t iButtonID, int8_t bTextXOffset, int8_t bTextYOffset,
                              BOOLEAN fShiftText);
void SpecifyButtonTextSubOffsets(int32_t iButtonID, int8_t bTextXOffset, int8_t bTextYOffset,
                                 BOOLEAN fShiftText);
void SpecifyButtonTextWrappedWidth(int32_t iButtonID, int16_t sWrappedWidth);

void SpecifyButtonSoundScheme(int32_t iButtonID, int8_t bSoundScheme);
void PlayButtonSound(int32_t iButtonID, int32_t iSoundType);

void AllowDisabledButtonFastHelp(int32_t iButtonID, BOOLEAN fAllow);

enum {
  DEFAULT_STATUS_NONE,
  DEFAULT_STATUS_DARKBORDER,      // shades the borders 2 pixels deep
  DEFAULT_STATUS_DOTTEDINTERIOR,  // draws the familiar dotted line in the interior portion of the
                                  // button.
  DEFAULT_STATUS_WINDOWS95,       // both DARKBORDER and DOTTEDINTERIOR
};
void GiveButtonDefaultStatus(int32_t iButtonID, int32_t iDefaultStatus);
void RemoveButtonDefaultStatus(int32_t iButtonID);

enum  // for use with SpecifyDisabledButtonStyle
{
  DISABLED_STYLE_NONE,     // for dummy buttons, panels, etc.  Always displays normal state.
  DISABLED_STYLE_DEFAULT,  // if button has text then shade, else hatch
  DISABLED_STYLE_HATCHED,  // always hatches the disabled button
  DISABLED_STYLE_SHADED    // always shades the disabled button 25% darker
};
void SpecifyDisabledButtonStyle(int32_t iButtonID, int8_t bStyle);

void RemoveTextFromButton(int32_t iButtonID);
void RemoveIconFromButton(int32_t iButtonID);

// Note:  Text is always on top
// If fShiftImage is true, then the image will shift down one pixel and right one pixel
// just like the text does.
BOOLEAN SpecifyButtonIcon(int32_t iButtonID, int32_t iVideoObjectID, uint16_t usVideoObjectIndex,
                          int8_t bXOffset, int8_t bYOffset, BOOLEAN fShiftImage);

void SetButtonPosition(int32_t iButtonID, int16_t x, int16_t y);
void ResizeButton(int32_t iButtonID, int16_t w, int16_t h);

void QuickButtonCallbackMMove(struct MOUSE_REGION *reg, int32_t reason);
void QuickButtonCallbackMButn(struct MOUSE_REGION *reg, int32_t reason);

BOOLEAN SetButtonCursor(int32_t iBtnId, uint16_t crsr);
void MSYS_SetBtnUserData(int32_t iButtonNum, int32_t index, int32_t userdata);
int32_t MSYS_GetBtnUserData(GUI_BUTTON *b, int32_t index);
void MarkAButtonDirty(int32_t iButtonNum);  // will mark only selected button dirty
void MarkButtonsDirty(
    void);  // Function to mark buttons dirty ( all will redraw at next RenderButtons )
void PausedMarkButtonsDirty(void);  // mark buttons dirty for button render the frame after the next
void UnMarkButtonDirty(int32_t iButtonIndex);  // unmark button
void UnmarkButtonsDirty(void);                 // unmark ALL the buttoms on the screen dirty
void ForceButtonUnDirty(
    int32_t iButtonIndex);  // forces button undirty no matter the reason, only lasts one frame

// DB 98-05-05
BOOLEAN GetButtonArea(int32_t iButtonID, struct GRect *pRect);
// DB 99-01-13
int32_t GetButtonWidth(int32_t iButtonID);
int32_t GetButtonHeight(int32_t iButtonID);

// DB 99-08-27
int32_t GetButtonX(int32_t iButtonID);
int32_t GetButtonY(int32_t iButtonID);

void BtnGenericMouseMoveButtonCallback(GUI_BUTTON *btn, int32_t reason);
#define DEFAULT_MOVE_CALLBACK BtnGenericMouseMoveButtonCallback

void DrawCheckBoxButtonOn(int32_t iButtonID);

void DrawCheckBoxButtonOff(int32_t iButtonID);

extern uint16_t GetWidthOfButtonPic(uint16_t usButtonPicID, int32_t iSlot);

#endif
