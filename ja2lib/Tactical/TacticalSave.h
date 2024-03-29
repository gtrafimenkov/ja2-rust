#ifndef __TACTICAL_SAVE_H_
#define __TACTICAL_SAVE_H_

#include "SGP/Types.h"
#include "Tactical/RottingCorpses.h"
#include "Tactical/SoldierProfileType.h"
#include "Tactical/WorldItems.h"

#define MAPS_DIR "Temp"

// Defines used for the bUseMercGridNoPlacement contained in the the merc profile struct
enum {
  PROFILE_NOT_SET,          // initially set to this
  PROFILE_DONT_USE_GRIDNO,  // if the merc is switching sectors, etc
  PROFILE_USE_GRIDNO,       // if we are to use the GridNo variable in the profile struct
};

// Add
BOOLEAN AddMapModification(u8 sMapX, u8 sMapY, i8 bMapZ);

// Load the Map modifications from the saved game file
BOOLEAN LoadMapTempFilesFromSavedGameFile(FileID hFile);

// Save the Map Temp files to the saved game file
BOOLEAN SaveMapTempFilesToSavedGameFile(FileID hFile);

// delete temp file
BOOLEAN DeleteTempItemMapFile(u8 sMapX, u8 sMapY, i8 bMapZ);

// Retrieves the number of items in the sectors temp item file
BOOLEAN GetNumberOfWorldItemsFromTempItemFile(u8 sMapX, u8 sMapY, i8 bMapZ,
                                              UINT32 *puiNumberOfItems, BOOLEAN fIfEmptyCreate);

// Saves the Current Sectors, ( world Items, rotting corpses, ... )  to the temporary file used to
// store the sectors items
BOOLEAN SaveCurrentSectorsInformationToTempItemFile();

// Loads the Currents Sectors information ( world Items, rotting corpses, ... ) from the temporary
// file used to store the sectores items
BOOLEAN LoadCurrentSectorsInformationFromTempItemsFile();

// Loads a World Item array from that sectors temp item file
BOOLEAN LoadWorldItemsFromTempItemFile(u8 sMapX, u8 sMapY, INT8 bMapZ, WORLDITEM *pData);

//  Adds an array of Item Objects to the specified location on a unloaded map.
//  If you want to overwrite all the items in the array set fReplaceEntireFile to TRUE.
BOOLEAN AddItemsToUnLoadedSector(u8 sMapX, u8 sMapY, INT8 bMapZ, INT16 sGridNo,
                                 UINT32 uiNumberOfItems, struct OBJECTTYPE *pObject, UINT8 ubLevel,
                                 UINT16 usFlags, INT8 bRenderZHeightAboveLevel, INT8 bVisible,
                                 BOOLEAN fReplaceEntireFile);

BOOLEAN AddWorldItemsToUnLoadedSector(u8 sMapX, u8 sMapY, INT8 bMapZ, INT16 sGridNo,
                                      UINT32 uiNumberOfItems, WORLDITEM *pWorldItem,
                                      BOOLEAN fOverWrite);

// Deletes all the Temp files in the Maps\Temp directory
BOOLEAN InitTacticalSave(BOOLEAN fCreateTempDir);

// Gets the number of ACTIVE ( Not the TOTAL number ) of World Items from the sectors temp file
BOOLEAN GetNumberOfActiveWorldItemsFromTempFile(u8 sMapX, u8 sMapY, i8 bMapZ,
                                                UINT32 *pNumberOfData);

// Call this function to set the new sector a NPC will travel to
void ChangeNpcToDifferentSector(UINT8 ubNpcId, u8 sSectorX, u8 sSectorY, INT8 bSectorZ);

// Adds a rotting corpse definition to the end of a sectors rotting corpse temp file
BOOLEAN AddRottingCorpseToUnloadedSectorsRottingCorpseFile(
    u8 sMapX, u8 sMapY, i8 bMapZ, ROTTING_CORPSE_DEFINITION *pRottingCorpseDef);

// Flags used for the AddDeadSoldierToUnLoadedSector() function
#define ADD_DEAD_SOLDIER_USE_GRIDNO \
  0x00000001  // just place the items and corpse on the gridno location
#define ADD_DEAD_SOLDIER_TO_SWEETSPOT 0x00000002  // Finds the closet free gridno

#define ADD_DEAD_SOLDIER__USE_JFK_HEADSHOT_CORPSE 0x00000040  // Will ue the JFK headshot

// Pass in the sector to add the dead soldier to.
// The gridno if you are passing in either of the flags ADD_DEAD_SOLDIER_USE_GRIDNO, or the
// ADD_DEAD_SOLDIER_TO_SWEETSPOT
//
// This function DOES NOT remove the soldier from the soldier struct.  YOU must do it.
BOOLEAN AddDeadSoldierToUnLoadedSector(u8 sMapX, u8 sMapY, UINT8 bMapZ,
                                       struct SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT32 uiFlags);

BOOLEAN GetSectorFlagStatus(u8 sMapX, u8 sMapY, UINT8 bMapZ, UINT32 uiFlagToSet);
BOOLEAN SetSectorFlag(u8 sMapX, u8 sMapY, UINT8 bMapZ, UINT32 uiFlagToSet);
BOOLEAN ReSetUnderGroundSectorFlag(u8 sSectorX, u8 sSectorY, UINT8 ubSectorZ, UINT32 uiFlagToSet);
BOOLEAN ReSetSectorFlag(u8 sMapX, u8 sMapY, UINT8 bMapZ, UINT32 uiFlagToSet);

// Saves the NPC temp Quote file to the saved game file
BOOLEAN LoadTempNpcQuoteArrayToSaveGameFile(FileID hFile);

// Loads the NPC temp Quote file from the saved game file
BOOLEAN SaveTempNpcQuoteArrayToSaveGameFile(FileID hFile);

UINT32 MercChecksum(struct SOLDIERTYPE *pSoldier);
UINT32 ProfileChecksum(MERCPROFILESTRUCT *pProfile);
BOOLEAN JA2EncryptedFileRead(FileID hFile, PTR pDest, UINT32 uiBytesToRead, UINT32 *puiBytesRead);
BOOLEAN JA2EncryptedFileWrite(FileID hFile, PTR pDest, UINT32 uiBytesToWrite,
                              UINT32 *puiBytesWritten);

BOOLEAN NewJA2EncryptedFileRead(FileID hFile, PTR pDest, UINT32 uiBytesToRead,
                                UINT32 *puiBytesRead);
BOOLEAN NewJA2EncryptedFileWrite(FileID hFile, PTR pDest, UINT32 uiBytesToWrite,
                                 UINT32 *puiBytesWritten);

// If hacker's mess with our save/temp files, this is our final line of defence.
void InitExitGameDialogBecauseFileHackDetected();

void HandleAllReachAbleItemsInTheSector(u8 sSectorX, u8 sSectorY, INT8 bSectorZ);

void GetMapTempFileName(UINT32 uiType, STR pMapName, u8 sMapX, u8 sMapY, i8 bMapZ);

UINT32 GetNumberOfVisibleWorldItemsFromSectorStructureForSector(u8 sMapX, u8 sMapY, INT8 bMapZ);
void SetNumberOfVisibleWorldItemsInSectorStructureForSector(u8 sMapX, u8 sMapY, i8 bMapZ,
                                                            UINT32 uiNumberOfItems);

#define NEW_ROTATION_ARRAY_SIZE 49
#define BASE_NUMBER_OF_ROTATION_ARRAYS 19

#endif
