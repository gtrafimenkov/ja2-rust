// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __SECTOR_SUMMARY_H
#define __SECTOR_SUMMARY_H

#include "BuildDefines.h"
#include "SGP/Types.h"

// This function can be very time consuming as it loads every map file with a valid
// coordinate name, analyses it, and builds a new global summary file.
void CreateGlobalSummary();

void CreateSummaryWindow();
void ReleaseSummaryWindow();
void DestroySummaryWindow();
void RenderSummaryWindow();
void LoadWorldInfo();

void UpdateSectorSummary(wchar_t* gszFilename, BOOLEAN fUpdate);

void SaveGlobalSummary();
void LoadGlobalSummary();

extern BOOLEAN gfGlobalSummaryExists;

extern BOOLEAN gfSummaryWindowActive;

extern BOOLEAN gSectorExists[16][16];

extern uint16_t gusNumEntriesWithOutdatedOrNoSummaryInfo;

extern BOOLEAN gfUpdateSummaryInfo;

extern uint8_t GetCurrentSummaryVersion();

#endif
