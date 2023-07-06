#include "SGP/CursorControl.h"

#include "SGP/CursorFileData.h"
#include "SGP/Debug.h"
#include "SGP/VObject.h"
#include "SGP/VObjectInternal.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/VideoInternal.h"
#include "SGP/WCheck.h"
#include "platform.h"
#include "rust_images.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Cursor Database
//
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOLEAN gfCursorDatabaseInit = FALSE;

struct CursorFileData *gpCursorFileDatabase;
CursorData *gpCursorDatabase;
INT16 gsGlobalCursorYOffset = 0;
INT16 gsCurMouseOffsetX = 0;
INT16 gsCurMouseOffsetY = 0;
UINT16 gsCurMouseHeight = 0;
UINT16 gsCurMouseWidth = 0;
UINT16 gusNumDataFiles = 0;
UINT32 guiExternVo;
UINT16 gusExternVoSubIndex;
UINT32 guiExtern2Vo;
UINT16 gusExtern2VoSubIndex;
UINT32 guiOldSetCursor = 0;
UINT32 guiDelayTimer = 0;

MOUSEBLT_HOOK gMouseBltOverride = NULL;

BOOLEAN BltToMouseCursorFromVObjectWithOutline(struct VObject *hVObject,
                                               UINT16 usVideoObjectSubIndex, UINT16 usXPos,
                                               UINT16 usYPos) {
  BOOLEAN ReturnValue;
  struct Subimage *pTrav;
  INT16 sXPos, sYPos;

  // Adjust for offsets
  pTrav = &(hVObject->subimages[usVideoObjectSubIndex]);

  sXPos = 0;
  sYPos = 0;

  // Remove offsets...
  sXPos -= pTrav->x_offset;
  sYPos -= pTrav->y_offset;

  // Center!
  sXPos += ((gsCurMouseWidth - pTrav->width) / 2);
  sYPos += ((gsCurMouseHeight - pTrav->height) / 2);

  ReturnValue = BltVideoObjectOutline(vsMouseBuffer, hVObject, usVideoObjectSubIndex, sXPos, sYPos,
                                      Get16BPPColor(FROMRGB(0, 255, 0)), TRUE);

  return ReturnValue;
}

// THESE TWO PARAMETERS MUST POINT TO STATIC OR GLOBAL DATA, NOT AUTOMATIC VARIABLES
void InitCursorDatabase(struct CursorFileData *pCursorFileData, CursorData *pCursorData,
                        UINT16 suNumDataFiles) {
  // Set global values!

  gpCursorFileDatabase = pCursorFileData;
  gpCursorDatabase = pCursorData;
  gusNumDataFiles = suNumDataFiles;
  gfCursorDatabaseInit = TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Cursor Handlers
//
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOLEAN LoadCursorData(UINT32 uiCursorIndex) {
  // Load cursor data will load all data required for the cursor specified by this index
  CursorData *pCurData;
  CursorImage *pCurImage;
  UINT32 cnt;
  INT16 sMaxHeight = -1;
  INT16 sMaxWidth = -1;
  struct Subimage *pTrav;

  pCurData = &(gpCursorDatabase[uiCursorIndex]);

  for (cnt = 0; cnt < pCurData->usNumComposites; cnt++) {
    pCurImage = &(pCurData->Composites[cnt]);

    if (gpCursorFileDatabase[pCurImage->uiFileIndex].fLoaded == FALSE) {
      //
      // The file containing the video object hasn't been loaded yet. Let's load it now
      //
      // FIRST LOAD AS AN struct Image* SO WE CAN GET AUX DATA!
      struct Image *hImage;
      struct AuxObjectData *pAuxData;

      // ATE: First check if we are using an extern vo cursor...
      if (gpCursorFileDatabase[pCurImage->uiFileIndex].ubFlags & USE_EXTERN_VO_CURSOR) {
        // Let's check if we have NOT NULL here...
        if (gpCursorFileDatabase[pCurImage->uiFileIndex].hVObject == NULL) {
          // Something wrong here...
        }

      } else {
        hImage = CreateImage(gpCursorFileDatabase[pCurImage->uiFileIndex].ubFilename, true);
        if (hImage == NULL) {
          return (FALSE);
        }

        gpCursorFileDatabase[pCurImage->uiFileIndex].hVObject = CreateVObjectFromImage(hImage);
        if (!gpCursorFileDatabase[pCurImage->uiFileIndex].hVObject) {
          return (FALSE);
        }

        // Check for animated tile
        if (hImage->app_data_size > 0) {
          // Valid auxiliary data, so get # od frames from data
          pAuxData = (struct AuxObjectData *)hImage->app_data;

          if (pAuxData->fFlags & AUX_ANIMATED_TILE) {
            gpCursorFileDatabase[pCurImage->uiFileIndex].ubFlags |= ANIMATED_CURSOR;
            gpCursorFileDatabase[pCurImage->uiFileIndex].ubNumberOfFrames =
                pAuxData->ubNumberOfFrames;
          }
        }

        // the hImage is no longer needed
        DestroyImage(hImage);
      }

      gpCursorFileDatabase[pCurImage->uiFileIndex].fLoaded = TRUE;
    }

    // Get ETRLE Data for this video object
    pTrav =
        &(gpCursorFileDatabase[pCurImage->uiFileIndex].hVObject->subimages[pCurImage->uiSubIndex]);

    if (!pTrav) {
      return FALSE;
    }

    if (pTrav->height > sMaxHeight) {
      sMaxHeight = pTrav->height;
    }

    if (pTrav->width > sMaxWidth) {
      sMaxWidth = pTrav->width;
    }
  }

  pCurData->usHeight = sMaxHeight;
  pCurData->usWidth = sMaxWidth;

  if (pCurData->sOffsetX == CENTER_CURSOR) {
    pCurData->sOffsetX = (pCurData->usWidth / 2);
  }
  if (pCurData->sOffsetX == RIGHT_CURSOR) {
    pCurData->sOffsetX = pCurData->usWidth;
  }
  if (pCurData->sOffsetX == LEFT_CURSOR) {
    pCurData->sOffsetX = 0;
  }

  if (pCurData->sOffsetY == CENTER_CURSOR) {
    pCurData->sOffsetY = (pCurData->usHeight / 2);
  }
  if (pCurData->sOffsetY == BOTTOM_CURSOR) {
    pCurData->sOffsetY = pCurData->usHeight;
  }
  if (pCurData->sOffsetY == TOP_CURSOR) {
    pCurData->sOffsetY = 0;
  }

  gsCurMouseOffsetX = pCurData->sOffsetX;
  gsCurMouseOffsetY = pCurData->sOffsetY;
  gsCurMouseHeight = pCurData->usHeight;
  gsCurMouseWidth = pCurData->usWidth;

  // Adjust relative offsets
  for (cnt = 0; cnt < pCurData->usNumComposites; cnt++) {
    pCurImage = &(pCurData->Composites[cnt]);

    // Get ETRLE Data for this video object
    pTrav =
        &(gpCursorFileDatabase[pCurImage->uiFileIndex].hVObject->subimages[pCurImage->uiSubIndex]);

    if (!pTrav) {
      return FALSE;
    }

    if (pCurImage->usPosX == CENTER_SUBCURSOR) {
      pCurImage->usPosX = pCurData->sOffsetX - (pTrav->width / 2);
    }

    if (pCurImage->usPosY == CENTER_SUBCURSOR) {
      pCurImage->usPosY = pCurData->sOffsetY - (pTrav->height / 2);
    }
  }

  return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void UnLoadCursorData(UINT32 uiCursorIndex) {
  // This function will unload add data used for this cursor
  //
  // Ok, first we make sure that the video object file is indeed loaded. Once this is verified, we
  // will move on to the deletion
  //
  CursorData *pCurData;
  CursorImage *pCurImage;
  UINT32 cnt;

  pCurData = &(gpCursorDatabase[uiCursorIndex]);

  for (cnt = 0; cnt < pCurData->usNumComposites; cnt++) {
    pCurImage = &(pCurData->Composites[cnt]);

    if (gpCursorFileDatabase[pCurImage->uiFileIndex].fLoaded) {
      if (!(gpCursorFileDatabase[pCurImage->uiFileIndex].ubFlags & USE_EXTERN_VO_CURSOR)) {
        DeleteVideoObject(gpCursorFileDatabase[pCurImage->uiFileIndex].hVObject);
        gpCursorFileDatabase[pCurImage->uiFileIndex].hVObject = NULL;
      }
      gpCursorFileDatabase[pCurImage->uiFileIndex].fLoaded = FALSE;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void CursorDatabaseClear(void) {
  UINT32 uiIndex;

  for (uiIndex = 0; uiIndex < gusNumDataFiles; uiIndex++) {
    if (gpCursorFileDatabase[uiIndex].fLoaded == TRUE) {
      if (!(gpCursorFileDatabase[uiIndex].ubFlags & USE_EXTERN_VO_CURSOR)) {
        DeleteVideoObject(gpCursorFileDatabase[uiIndex].hVObject);
        gpCursorFileDatabase[uiIndex].hVObject = NULL;
      }

      gpCursorFileDatabase[uiIndex].fLoaded = FALSE;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

BOOLEAN SetCurrentCursor(UINT16 usVideoObjectSubIndex, UINT16 usOffsetX, UINT16 usOffsetY) {
  BOOLEAN ReturnValue;
  struct Subimage pETRLEPointer;

  //
  // Make sure we have a cursor store
  //

  if (gpCursorStore == NULL) {
    DebugMsg(TOPIC_VIDEO, DBG_ERROR, "ERROR : Cursor store is not loaded");
    return FALSE;
  }

  EraseMouseCursor();

  //
  // Get new cursor data
  //

  ReturnValue = BltVObject(vsMouseBuffer, gpCursorStore, usVideoObjectSubIndex, 0, 0);
  guiMouseBufferState = BUFFER_DIRTY;

  if (GetVideoObjectETRLEProperties(gpCursorStore, &pETRLEPointer, usVideoObjectSubIndex)) {
    gsMouseCursorXOffset = usOffsetX;
    gsMouseCursorYOffset = usOffsetY;
    gusMouseCursorWidth = pETRLEPointer.width + pETRLEPointer.x_offset;
    gusMouseCursorHeight = pETRLEPointer.height + pETRLEPointer.y_offset;

    DebugMsg(TOPIC_VIDEO, DBG_ERROR, "=================================================");
    DebugMsg(TOPIC_VIDEO, DBG_ERROR,
             String("Mouse Create with [ %d. %d ] [ %d, %d]", pETRLEPointer.x_offset,
                    pETRLEPointer.y_offset, pETRLEPointer.width, pETRLEPointer.height));
    DebugMsg(TOPIC_VIDEO, DBG_ERROR, "=================================================");
  } else {
    DebugMsg(TOPIC_VIDEO, DBG_ERROR, "Failed to get mouse info");
  }

  return ReturnValue;
}

BOOLEAN SetCurrentCursorFromDatabase(UINT32 uiCursorIndex) {
  BOOLEAN ReturnValue = TRUE;
  UINT16 usSubIndex;
  CursorData *pCurData;
  CursorImage *pCurImage;
  UINT32 cnt;
  INT16 sCenterValX, sCenterValY;
  struct VObject *hVObject;
  struct Subimage *pTrav;
  UINT16 usEffHeight, usEffWidth;

  if (gfCursorDatabaseInit) {
    // Enter mouse buffer mutex

    // If the current cursor is the first index, disable cursors

    if (uiCursorIndex == VIDEO_NO_CURSOR) {
      EraseMouseCursor();

      SetMouseCursorProperties(0, 0, 5, 5);
      DirtyCursor();

      // EnableCursor( FALSE );

    } else {
      // CHECK FOR EXTERN CURSOR
      if (uiCursorIndex == EXTERN_CURSOR || uiCursorIndex == EXTERN2_CURSOR) {
        INT16 sSubX, sSubY;
        struct VObject *hVObjectTemp;
        struct Subimage *pTravTemp;

        // Erase old cursor
        EraseMouseCursor();

        if (uiCursorIndex == EXTERN2_CURSOR) {
          // Get ETRLE values
          GetVideoObject(&hVObject, guiExtern2Vo);
          pTrav = &(hVObject->subimages[gusExtern2VoSubIndex]);
        } else {
          // Get ETRLE values
          GetVideoObject(&hVObject, guiExternVo);
          pTrav = &(hVObject->subimages[gusExternVoSubIndex]);
        }

        // Determine center
        sCenterValX = 0;
        sCenterValY = 0;

        // Effective height
        usEffHeight = pTrav->height + pTrav->y_offset;
        usEffWidth = pTrav->width + pTrav->x_offset;

        // ATE: Check for extern 2nd...
        if (uiCursorIndex == EXTERN2_CURSOR) {
          BltVideoObjectOutlineFromIndex(vsMouseBuffer, guiExtern2Vo, gusExtern2VoSubIndex, 0, 0, 0,
                                         FALSE);

          // Get ETRLE values
          GetVideoObject(&hVObjectTemp, guiExternVo);
          pTravTemp = &(hVObjectTemp->subimages[gusExternVoSubIndex]);

          sSubX = (pTrav->width - pTravTemp->width - pTravTemp->x_offset) / 2;
          sSubY = (pTrav->height - pTravTemp->height - pTravTemp->y_offset) / 2;

          BltVideoObjectOutlineFromIndex(vsMouseBuffer, guiExternVo, gusExternVoSubIndex, sSubX,
                                         sSubY, 0, FALSE);

        } else {
          BltVideoObjectOutlineFromIndex(vsMouseBuffer, guiExternVo, gusExternVoSubIndex, 0, 0, 0,
                                         FALSE);
        }

        // Hook into hook function
        if (gMouseBltOverride != NULL) {
          gMouseBltOverride();
        }

        SetMouseCursorProperties((INT16)(usEffWidth / 2), (INT16)(usEffHeight / 2),
                                 (UINT16)(usEffHeight), (UINT16)(usEffWidth));
        DirtyCursor();

      } else {
        pCurData = &(gpCursorDatabase[uiCursorIndex]);

        // First check if we are a differnet curosr...
        if (uiCursorIndex != guiOldSetCursor) {
          // OK, check if we are a delay cursor...
          if (pCurData->bFlags & DELAY_START_CURSOR) {
            guiDelayTimer = Plat_GetTickCount();
          }
        }

        guiOldSetCursor = uiCursorIndex;

        // Olny update if delay timer has elapsed...
        if (pCurData->bFlags & DELAY_START_CURSOR) {
          if ((Plat_GetTickCount() - guiDelayTimer) < 1000) {
            EraseMouseCursor();

            SetMouseCursorProperties(0, 0, 5, 5);
            DirtyCursor();

            return (TRUE);
          }
        }

        //
        // Call LoadCursorData to make sure that the video object is loaded
        //
        LoadCursorData(uiCursorIndex);

        // Erase old cursor
        EraseMouseCursor();
        // NOW ACCOMODATE COMPOSITE CURSORS
        pCurData = &(gpCursorDatabase[uiCursorIndex]);

        for (cnt = 0; cnt < pCurData->usNumComposites; cnt++) {
          // Check if we are a flashing cursor!
          if (pCurData->bFlags & CURSOR_TO_FLASH) {
            if (cnt <= 1) {
              if (pCurData->bFlashIndex != cnt) {
                continue;
              }
            }
          }
          // Check if we are a sub cursor!
          // IN this case, do all frames but
          // skip the 1st or second!

          if (pCurData->bFlags & CURSOR_TO_SUB_CONDITIONALLY) {
            if (pCurData->bFlags & CURSOR_TO_FLASH) {
              if (cnt <= 1) {
                if (pCurData->bFlashIndex != cnt) {
                  continue;
                }
              }
            } else if (pCurData->bFlags & CURSOR_TO_FLASH2) {
              if (cnt <= 2 && cnt > 0) {
                if (pCurData->bFlashIndex != cnt) {
                  continue;
                }
              }
            } else {
              if (cnt <= 1) {
                if (pCurData->bFlashIndex != cnt) {
                  continue;
                }
              }
            }
          }

          pCurImage = &(pCurData->Composites[cnt]);

          // Adjust sub-index if cursor is animated
          if (gpCursorFileDatabase[pCurImage->uiFileIndex].ubFlags & ANIMATED_CURSOR) {
            usSubIndex = (UINT16)pCurImage->uiCurrentFrame;
          } else {
            usSubIndex = pCurImage->uiSubIndex;
          }

          if (pCurImage->usPosX != HIDE_SUBCURSOR && pCurImage->usPosY != HIDE_SUBCURSOR) {
            // Blit cursor at position in mouse buffer
            if (gpCursorFileDatabase[pCurImage->uiFileIndex].ubFlags & USE_OUTLINE_BLITTER) {
              ReturnValue = BltToMouseCursorFromVObjectWithOutline(
                  gpCursorFileDatabase[pCurImage->uiFileIndex].hVObject, usSubIndex,
                  pCurImage->usPosX, pCurImage->usPosY);
            } else {
              ReturnValue =
                  BltVObject(vsMouseBuffer, gpCursorFileDatabase[pCurImage->uiFileIndex].hVObject,
                             usSubIndex, pCurImage->usPosX, pCurImage->usPosY);
            }
            if (!ReturnValue) {
              return (FALSE);
            }
          }
        }

        // Hook into hook function
        if (gMouseBltOverride != NULL) {
          gMouseBltOverride();
        }

        sCenterValX = pCurData->sOffsetX;
        sCenterValY = pCurData->sOffsetY;

        SetMouseCursorProperties(sCenterValX, (INT16)(sCenterValY + gsGlobalCursorYOffset),
                                 pCurData->usHeight, pCurData->usWidth);
        DirtyCursor();
      }
    }
  } else {
    if (uiCursorIndex == VIDEO_NO_CURSOR) {
      EraseMouseCursor();

      SetMouseCursorProperties(0, 0, 5, 5);
      DirtyCursor();

      // EnableCursor( FALSE );

    } else {
      SetCurrentCursor((UINT16)uiCursorIndex, 0, 0);
      ReturnValue = TRUE;
    }
  }

  return (ReturnValue);
}

void SetMouseBltHook(MOUSEBLT_HOOK pMouseBltOverride) { gMouseBltOverride = pMouseBltOverride; }

// Sets an external video object as cursor file data....
void SetExternVOData(UINT32 uiCursorIndex, struct VObject *hVObject, UINT16 usSubIndex) {
  CursorData *pCurData;
  CursorImage *pCurImage;
  UINT32 cnt;

  pCurData = &(gpCursorDatabase[uiCursorIndex]);

  for (cnt = 0; cnt < pCurData->usNumComposites; cnt++) {
    pCurImage = &(pCurData->Composites[cnt]);

    if (gpCursorFileDatabase[pCurImage->uiFileIndex].ubFlags & USE_EXTERN_VO_CURSOR) {
      // OK, set Video Object here....

      // If loaded, unload...
      UnLoadCursorData(uiCursorIndex);

      // Set extern vo
      gpCursorFileDatabase[pCurImage->uiFileIndex].hVObject = hVObject;
      pCurImage->uiSubIndex = usSubIndex;

      // Reload....
      LoadCursorData(uiCursorIndex);
    }
  }
}

void RemoveExternVOData(UINT32 uiCursorIndex) {
  CursorData *pCurData;
  CursorImage *pCurImage;
  UINT32 cnt;

  pCurData = &(gpCursorDatabase[uiCursorIndex]);

  for (cnt = 0; cnt < pCurData->usNumComposites; cnt++) {
    pCurImage = &(pCurData->Composites[cnt]);

    if (gpCursorFileDatabase[pCurImage->uiFileIndex].ubFlags & USE_EXTERN_VO_CURSOR) {
      gpCursorFileDatabase[pCurImage->uiFileIndex].hVObject = NULL;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
