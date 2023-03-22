#include "TileEngine/ShadeTableUtil.h"

#include <stdio.h>
#include <string.h>

#include "SGP/Debug.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VObjectInternal.h"
#include "SGP/Video.h"
#include "TileEngine/Lighting.h"
#include "TileEngine/WorldDat.h"
#include "platform.h"
#include "rust_fileman.h"

#define SHADE_TABLE_DIR "ShadeTables"

char TileSurfaceFilenames[NUMBEROFTILETYPES][32];
BOOLEAN gfForceBuildShadeTables = FALSE;

#ifdef JA2TESTVERSION
extern uint32_t uiNumTablesSaved;
extern uint32_t uiNumTablesLoaded;
#endif

void DetermineRGBDistributionSettings() {
  char DataDir[600];
  struct Str512 ExecDir;
  char ShadeTableDir[600];
  uint32_t uiRBitMask, uiGBitMask, uiBBitMask;
  uint32_t uiPrevRBitMask, uiPrevGBitMask, uiPrevBBitMask;
  uint32_t uiNumBytesRead;
  FileID hfile = FILE_ID_ERR;
  BOOLEAN fSaveRGBDist = FALSE;
  BOOLEAN fCleanShadeTable = FALSE;
  BOOLEAN fLoadedPrevRGBDist = FALSE;

  // First, determine if we have a file saved.  If not, then this is the first time, and
  // all shade tables will have to be built and saved to disk.  This can be time consuming, adding
  // up to 3-4 seconds to the time of a map load.
  if (!Plat_GetExecutableDirectory(&ExecDir)) {
    return;
  }
  snprintf(ShadeTableDir, ARR_SIZE(ShadeTableDir), "%s\\Data\\%s", ExecDir.buf, SHADE_TABLE_DIR);

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
    if (!File_Exists("RGBDist.dat") ||
        File_Exists("ResetShadeTables.txt")) {  // Can't find the RGBDist.dat file.  The
                                                // directory exists, but the file doesn't, which
      // means the user deleted the file manually.  Now, set it up to create a new one.
      fSaveRGBDist = TRUE;
      fCleanShadeTable = TRUE;
    } else {
      hfile = File_OpenForReading("RGBDist.dat");
      if (!hfile) {
        AssertMsg(0, "Couldn't open RGBDist.dat, even though it exists!");
      }
      File_Read(hfile, &uiPrevRBitMask, sizeof(uint32_t), &uiNumBytesRead);
      File_Read(hfile, &uiPrevGBitMask, sizeof(uint32_t), &uiNumBytesRead);
      File_Read(hfile, &uiPrevBBitMask, sizeof(uint32_t), &uiNumBytesRead);
      fLoadedPrevRGBDist = TRUE;
      File_Close(hfile);
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
    Plat_RemoveFilesInDirectory(ShadeTableDir);
  }
  if (fSaveRGBDist) {
    // The RGB distribution is going to be saved in a tiny file for future
    // reference.  As long as the
    // RGB distribution never changes, the shade table will grow until eventually, all tilesets are
    // loaded, shadetables generated and saved in this directory.
    hfile = File_OpenForWriting("RGBDist.dat");
    if (!hfile) {
      AssertMsg(0, "Couldn't create RGBDist.dat for writing!");
    }
    File_Write(hfile, &uiRBitMask, sizeof(uint32_t), &uiNumBytesRead);
    File_Write(hfile, &uiGBitMask, sizeof(uint32_t), &uiNumBytesRead);
    File_Write(hfile, &uiBBitMask, sizeof(uint32_t), &uiNumBytesRead);
    File_Close(hfile);
  }

  // We're done, so restore the executable directory to JA2\Data.
  snprintf(DataDir, ARR_SIZE(DataDir), "%s\\Data", ExecDir.buf);
  Plat_SetCurrentDirectory(DataDir);
}

BOOLEAN LoadShadeTable(struct VObject* pObj, uint32_t uiTileTypeIndex) {
  FileID hfile = FILE_ID_ERR;
  int32_t i;
  uint32_t uiNumBytesRead;
  char ShadeFileName[100];
  char* ptr;
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

  hfile = File_OpenForReading(ShadeFileName);
  if (!hfile) {  // File doesn't exist, so generate it
    File_Close(hfile);
    return FALSE;
  }

  // MISSING:  Compare time stamps.

  for (i = 0; i < 16; i++) {
    pObj->pShades[i] = (uint16_t*)MemAlloc(512);
    Assert(pObj->pShades[i]);
    File_Read(hfile, pObj->pShades[i], 512, &uiNumBytesRead);
  }

  // The file exists, now make sure the
  File_Close(hfile);
#ifdef JA2TESTVERSION
  uiNumTablesLoaded++;
#endif
  return TRUE;
}

BOOLEAN SaveShadeTable(struct VObject* pObj, uint32_t uiTileTypeIndex) {
  FileID hfile = FILE_ID_ERR;
  int32_t i;
  uint32_t uiNumBytesWritten;
  char ShadeFileName[100];
  char* ptr;
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

  hfile = File_OpenForWriting(ShadeFileName);
  if (!hfile) {
    File_Close(hfile);
    AssertMsg(0, String("Can't create %s", ShadeFileName));
    return FALSE;
  }
  for (i = 0; i < 16; i++) {
    File_Write(hfile, pObj->pShades[i], 512, &uiNumBytesWritten);
  }

  File_Close(hfile);
  return TRUE;
}

BOOLEAN DeleteShadeTableDir() { return (Plat_RemoveDirectory(SHADE_TABLE_DIR)); }
