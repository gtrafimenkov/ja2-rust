#include "Utils/DebugControl.h"

#include <stdio.h>

#include "SGP/Types.h"

#ifdef _ANIMSUBSYSTEM_DEBUG

void AnimDbgMessage(char *strMessage) {
  FILE *OutFile;

  if ((OutFile = fopen("AnimDebug.txt", "a+t")) != NULL) {
    fprintf(OutFile, "%s\n", strMessage);
    fclose(OutFile);
  }
}

#endif

#ifdef _PHYSICSSUBSYSTEM_DEBUG

void PhysicsDbgMessage(char *strMessage) {
  FILE *OutFile;

  if ((OutFile = fopen("PhysicsDebug.txt", "a+t")) != NULL) {
    fprintf(OutFile, "%s\n", strMessage);
    fclose(OutFile);
  }
}

#endif

#ifdef _AISUBSYSTEM_DEBUG

void AiDbgMessage(char *strMessage) {
  FILE *OutFile;

  if ((OutFile = fopen("AiDebug.txt", "a+t")) != NULL) {
    fprintf(OutFile, "%s\n", strMessage);
    fclose(OutFile);
  }
}

#endif

void LiveMessage(char *strMessage) {
  FILE *OutFile;

  if ((OutFile = fopen("Log.txt", "a+t")) != NULL) {
    fprintf(OutFile, "%s\n", strMessage);
    fclose(OutFile);
  }
}
