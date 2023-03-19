#include "Tactical/Gap.h"

#include <stdio.h>
#include <string.h>

#include "SGP/Debug.h"
#include "SGP/MemMan.h"
#include "SGP/SoundMan.h"
#include "SGP/Types.h"
#include "Utils/SoundControl.h"
#include "Utils/TimerControl.h"
#include "rust_fileman.h"

#if 0
static void AILCALLBACK timer_func( uint32_t user )
{
	AudioGapList	*pGapList;

	pGapList = (AudioGapList*)user;

  pGapList->gap_monitor_timer += GAP_TIMER_INTERVAL;

  if ( pGapList->audio_gap_active )
  {
   if ( (pGapList->gap_monitor_timer / 1000) > pGapList->end[ pGapList->gap_monitor_current] )
    {
      pGapList->audio_gap_active = 0;
      pGapList->gap_monitor_current++;
    }
  }
  else
	{
    if ( pGapList->gap_monitor_current < pGapList->count )
		{
      if ( ( pGapList->gap_monitor_timer / 1000) >= pGapList->start[ pGapList->gap_monitor_current ])
      {
				pGapList->audio_gap_active = 1;
      }
		}
	}
}
#endif

void AudioGapListInit(char *zSoundFile, AudioGapList *pGapList) {
  // This procedure will load in the appropriate .gap file, corresponding
  // to the .wav file in szSoundEffects indexed by uiSampleNum
  // The procedure will then allocate and load in the AUDIO_GAP information,
  // while counting the number of elements loaded

  //	FILE *pFile;
  FileID pFile = FILE_ID_ERR;
  char *pSourceFileName;
  char *pDestFileName;
  char sFileName[256];
  uint8_t counter = 0;
  AUDIO_GAP *pCurrentGap, *pPreviousGap;
  uint32_t Start;
  uint32_t uiNumBytesRead;

  uint32_t End;

  pSourceFileName = zSoundFile;
  pDestFileName = sFileName;
  // Initialize GapList
  pGapList->size = 0;
  pGapList->current_time = 0;
  pGapList->pHead = 0;
  pGapList->pCurrent = 0;
  pGapList->audio_gap_active = FALSE;
  pPreviousGap = pCurrentGap = 0;
  // DebugMsg(TOPIC_JA2, DBG_LEVEL_3,String("File is %s", szSoundEffects[uiSampleNum]));
  // Get filename
  strcpy(pDestFileName, pSourceFileName);
  // strip .wav and change to .gap

  while (pDestFileName[counter] != '.') {
    counter++;
  }

  pDestFileName[counter + 1] = 'g';
  pDestFileName[counter + 2] = 'a';
  pDestFileName[counter + 3] = 'p';
  pDestFileName[counter + 4] = '\0';

  pFile = File_OpenForReading(pDestFileName);
  if (pFile) {
    counter = 0;
    // gap file exists
    // now read in the AUDIO_GAPs

    while (File_Read(pFile, &Start, sizeof(uint32_t), &uiNumBytesRead) && uiNumBytesRead != 0) {
      // can read the first element, there exists a second
      File_Read(pFile, &End, sizeof(uint32_t), &uiNumBytesRead);

      // allocate space for AUDIO_GAP
      pCurrentGap = (AUDIO_GAP *)MemAlloc(sizeof(AUDIO_GAP));
      if (pPreviousGap != 0)
        pPreviousGap->pNext = pCurrentGap;
      else {
        // Start of list
        pGapList->pCurrent = pCurrentGap;
        pGapList->pHead = pCurrentGap;
      }

      pGapList->size++;
      pCurrentGap->pNext = 0;
      pCurrentGap->uiStart = Start;
      pCurrentGap->uiEnd = End;
      DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("Gap Start %d and Ends %d", Start, End));

      // Increment pointer
      pPreviousGap = pCurrentGap;
    }

    pGapList->audio_gap_active = FALSE;
    pGapList->current_time = 0;

    // fclose(pFile);
    File_Close(pFile);
  }
  DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
           String("Gap List Started From File %s and has %d gaps", pDestFileName, pGapList->size));
}

void AudioGapListDone(AudioGapList *pGapList) {
  // This procedure will go through the  AudioGapList and free space/nullify pointers
  // for any allocated elements

  AUDIO_GAP *pCurrent, *pNext;
  if (pGapList->pHead != 0) {
    pCurrent = pGapList->pHead;
    pNext = pCurrent->pNext;
    // There are elements in the list
    while (pNext != 0) {
      // kill pCurrent
      MemFree(pCurrent);
      pCurrent = pNext;
      pNext = pNext->pNext;
    }
    // now kill the last element
    MemFree(pCurrent);
    pCurrent = 0;
  }
  pGapList->pHead = 0;
  pGapList->pCurrent = 0;
  pGapList->size = 0;
  DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("Audio Gap List Deleted"));
}

void PollAudioGap(uint32_t uiSampleNum, AudioGapList *pGapList) {
  // This procedure will access the AudioGapList pertaining to the .wav about
  // to be played and sets the audio_gap_active flag. This is done by
  // going to the current AUDIO_GAP element in the AudioGapList, comparing to see if
  //	the current time is between the uiStart and uiEnd. If so, set flag..if not and
  // the uiStart of the next element is not greater than current time, set current to next and
  // repeat
  // ...if next elements uiStart is larger than current_time, or no more elements..
  // set flag FALSE

  uint32_t time;
  AUDIO_GAP *pCurrent;

  if (!pGapList) {
    // no gap list, return
    return;
  }

  if (pGapList->size > 0) {
    time = SoundGetPosition(uiSampleNum);
    //  DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Sound Sample Time is %d", time) );
  } else {
    pGapList->audio_gap_active = (FALSE);
    return;
  }

  // set current ot head of gap list for this sound
  pCurrent = pGapList->pHead;

  // check to see if we have fallen behind
  if ((time > pCurrent->uiEnd)) {
    // fallen behind
    // catchup
    while (time > pCurrent->uiEnd) {
      pCurrent = pCurrent->pNext;
      if (!pCurrent) {
        pGapList->audio_gap_active = (FALSE);
        return;
      }
    }
  }

  // check to see if time is within the next AUDIO_GAPs start time
  if ((time > pCurrent->uiStart) && (time < pCurrent->uiEnd)) {
    if ((time > pCurrent->uiStart) && (time < pCurrent->uiEnd)) {
      // we are within the time frame
      DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("Gap Started at %d", time));
      pGapList->audio_gap_active = (TRUE);

    } else if ((time > pCurrent->uiEnd) && (pGapList->audio_gap_active == TRUE)) {
      // reset if already set
      pGapList->audio_gap_active = (FALSE);
      DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("Gap Ended at %d", time));
    }
  } else {
    pGapList->audio_gap_active = (FALSE);
  }
}

uint32_t PlayJA2GapSample(char *zSoundFile, uint32_t usRate, uint32_t ubVolume, uint32_t ubLoops,
                          uint32_t uiPan, AudioGapList *pData) {
  SOUNDPARMS spParms;

  memset(&spParms, 0xff, sizeof(SOUNDPARMS));

  spParms.uiSpeed = usRate;
  spParms.uiVolume = (uint32_t)((ubVolume / (float)HIGHVOLUME) * GetSpeechVolume());
  spParms.uiLoop = ubLoops;
  spParms.uiPan = uiPan;
  spParms.uiPriority = GROUP_PLAYER;

  // Setup Gap Detection, if it is not null
  if (pData != NULL) AudioGapListInit(zSoundFile, pData);

  return (SoundPlayStreamedFile(zSoundFile, &spParms));
}
