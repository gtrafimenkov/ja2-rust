#include "TileEngine/ShadeTableUtil.h"

#include <stdio.h>
#include <string.h>

#include "SGP/Debug.h"
#include "SGP/FileMan.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/Video.h"
#include "TileEngine/Lighting.h"
#include "TileEngine/WorldDat.h"
#include "platform.h"

#define SHADE_TABLE_DIR "ShadeTables"

CHAR8 TileSurfaceFilenames[NUMBEROFTILETYPES][32];
BOOLEAN gfForceBuildShadeTables = FALSE;

#ifdef JA2TESTVERSION
extern uint32_t uiNumTablesSaved;
extern uint32_t uiNumTablesLoaded;
#endif

void DetermineRGBDistributionSettings() {
  STRING512 DataDir;
  char ExecDir[256];
  STRING512 ShadeTableDir;
  uint32_t uiRBitMask, uiGBitMask, uiBBitMask;
  uint32_t uiPrevRBitMask, uiPrevGBitMask, uiPrevBBitMask;
  uint32_t uiNumBytesRead;
  HWFILE hfile;
  BOOLEAN fSaveRGBDist = FALSE;
  BOOLEAN fCleanShadeTable = FALSE;
  BOOLEAN fLoadedPrevRGBDist = FALSE;

  // First, determine if we have a file saved.  If not, then this is the first time, and
  // all shade tables will have to be built and saved to disk.  This can be time consuming, adding
  // up to 3-4 seconds to the time of a map load.
  Plat_GetExecutableDirectory(ExecDir, sizeof(ExecDir));
  sprintf(ShadeTableDir, "%s\\Data\\%s", ExecDir, SHADE_TABLE_DIR);

  // Check to make sure we have a ShadeTable directory.  If we don't create one!
  if (!Plat_SetCurrentDirectory(ShadeTableDir)) {
    if (!Plat_CreateDirectory(ShadeTableDir)) {
      AssertMsg(0, "ShadeTable directory doesn't exist, and couldn't create one!");
    }
    if (!Plat_SetCurrentDirectory(ShadeTableDir)) {
      AssertMsg(0, "Couldn't access the newly created ShadeTable directory.");
    }
    fSaveRGBDist = TRUE;
  }

  if (!fSaveRGBDist) {  // Load the previous RGBDist and determine if it is the same one
    if (!FileMan_Exists("RGBDist.dat") ||
        FileMan_Exists("ResetShadeTables.txt")) {  // Can't find the RGBDist.dat file.  The
                                                   // directory exists, but the file doesn't, which
      // means the user deleted the file manually.  Now, set it up to create a new one.
      fSaveRGBDist = TRUE;
      fCleanShadeTable = TRUE;
    } else {
      hfile = FileMan_Open("RGBDist.dat", FILE_ACCESS_READ, FALSE);
      if (!hfile) {
        AssertMsg(0, "Couldn't open RGBDist.dat, even though it exists!");
      }
      FileMan_Read(hfile, &uiPrevRBitMask, sizeof(uint32_t), &uiNumBytesRead);
      FileMan_Read(hfile, &uiPrevGBitMask, sizeof(uint32_t), &uiNumBytesRead);
      FileMan_Read(hfile, &uiPrevBBitMask, sizeof(uint32_t), &uiNumBytesRead);
      fLoadedPrevRGBDist = TRUE;
      FileMan_Close(hfile);
    }
  }

  if (!GetPrimaryRGBDistributionMasks(&uiRBitMask, &uiGBitMask, &uiBBitMask)) {
    AssertMsg(0, "Failed to extract the current RGB distribution masks.");
  }
  if (fLoadedPrevRGBDist) {
    if (uiRBitMask != uiPrevRBitMask || uiGBitMask != uiPrevGBitMask ||
        uiBBitMask != uiPrevBBitMask) {  // The user has changed modes since the last time he has
                                         // played JA2.  This essentially can only happen if:
      // 1)  The video card has been changed since the last run of JA2.
      // 2)  Certain video cards have different RGB distributions in different operating systems
      // such as 		the Millenium card using Windows NT or Windows 95 3)  The user has
      // physically modified the RGBDist.dat file.
      fSaveRGBDist = TRUE;
      fCleanShadeTable = TRUE;
    }
  }
  if (fCleanShadeTable) {  // This means that we are going to remove all of the current shade
                           // tables, if any exist, and
    // start fresh.
    Plat_EraseDirectory(ShadeTableDir);
  }
  if (fSaveRGBDist) {  // The RGB distribution is going to be saved in a tiny file for future
                       // reference.  As long as the
    // RGB distribution never changes, the shade table will grow until eventually, all tilesets are
    // loaded, shadetables generated and saved in this directory.
    hfile = FileMan_Open("RGBDist.dat", FILE_ACCESS_WRITE | FILE_CREATE_ALWAYS, FALSE);
    if (!hfile) {
      AssertMsg(0, "Couldn't create RGBDist.dat for writing!");
    }
    FileMan_Write(hfile, &uiRBitMask, sizeof(uint32_t), &uiNumBytesRead);
    FileMan_Write(hfile, &uiGBitMask, sizeof(uint32_t), &uiNumBytesRead);
    FileMan_Write(hfile, &uiBBitMask, sizeof(uint32_t), &uiNumBytesRead);
    FileMan_Close(hfile);
  }

  // We're done, so restore the executable directory to JA2\Data.
  sprintf(DataDir, "%s\\Data", ExecDir);
  Plat_SetCurrentDirectory(DataDir);
}

BOOLEAN LoadShadeTable(struct VObject* pObj, uint32_t uiTileTypeIndex) {
  HWFILE hfile;
  INT32 i;
  uint32_t uiNumBytesRead;
  CHAR8 ShadeFileName[100];
  STR8 ptr;
  // ASSUMPTIONS:
  // We are assuming that the uiTileTypeIndex is referring to the correct file
  // stored in the TileSurfaceFilenames[].  If it isn't, then that is a huge problem
  // and should be fixed.  Also assumes that the directory is set to Data\ShadeTables.
  strcpy(ShadeFileName, TileSurfaceFilenames[uiTileTypeIndex]);
  ptr = strstr(ShadeFileName, ".");
  if (!ptr) {
    return FALSE;
  }
  ptr++;
  sprintf(ptr, "sha");

  hfile = FileMan_Open(ShadeFileName, FILE_ACCESS_READ, FALSE);
  if (!hfile) {  // File doesn't exist, so generate it
    FileMan_Close(hfile);
    return FALSE;
  }

  // MISSING:  Compare time stamps.

  for (i = 0; i < 16; i++) {
    pObj->pShades[i] = (uint16_t*)MemAlloc(512);
    Assert(pObj->pShades[i]);
    FileMan_Read(hfile, pObj->pShades[i], 512, &uiNumBytesRead);
  }

  // The file exists, now make sure the
  FileMan_Close(hfile);
#ifdef JA2TESTVERSION
  uiNumTablesLoaded++;
#endif
  return TRUE;
}

BOOLEAN SaveShadeTable(struct VObject* pObj, uint32_t uiTileTypeIndex) {
  HWFILE hfile;
  INT32 i;
  uint32_t uiNumBytesWritten;
  CHAR8 ShadeFileName[100];
  STR8 ptr;
#ifdef JA2TESTVERSION
  uiNumTablesSaved++;
#endif
  // ASSUMPTIONS:
  // We are assuming that the uiTileTypeIndex is referring to the correct file
  // stored in the TileSurfaceFilenames[].  If it isn't, then that is a huge problem
  // and should be fixed.  Also assumes that the directory is set to Data\ShadeTables.
  strcpy(ShadeFileName, TileSurfaceFilenames[uiTileTypeIndex]);
  ptr = strstr(ShadeFileName, ".");
  if (!ptr) {
    return FALSE;
  }
  ptr++;
  sprintf(ptr, "sha");

  hfile = FileMan_Open(ShadeFileName, FILE_ACCESS_WRITE | FILE_CREATE_ALWAYS, FALSE);
  if (!hfile) {
    FileMan_Close(hfile);
    AssertMsg(0, String("Can't create %s", ShadeFileName));
    return FALSE;
  }
  for (i = 0; i < 16; i++) {
    FileMan_Write(hfile, pObj->pShades[i], 512, &uiNumBytesWritten);
  }

  FileMan_Close(hfile);
  return TRUE;
}

BOOLEAN DeleteShadeTableDir() { return (Plat_RemoveDirectory(SHADE_TABLE_DIR, TRUE)); }
