#ifndef __IMP_COMPILE_H
#define __IMP_COMPILE_H

#include "SGP/Types.h"

#define PLAYER_GENERATED_CHARACTER_ID 51
#define ATTITUDE_LIST_SIZE 20
#define NUMBER_OF_PLAYER_PORTRAITS 16

void AddAnAttitudeToAttitudeList(int8_t bAttitude);
void CreatePlayerAttitude(void);
BOOLEAN DoesCharacterHaveAnAttitude(void);
void CreateACharacterFromPlayerEnteredStats(void);
void CreatePlayerSkills(void);
void CreatePlayersPersonalitySkillsAndAttitude(void);
void AddAPersonalityToPersonalityList(int8_t bPersonlity);
void CreatePlayerPersonality(void);
BOOLEAN DoesCharacterHaveAPersoanlity(void);
void AddSkillToSkillList(int8_t bSkill);
void ResetSkillsAttributesAndPersonality(void);
void ResetIncrementCharacterAttributes(void);
void HandleMercStatsForChangesInFace(void);

extern STR8 pPlayerSelectedFaceFileNames[NUMBER_OF_PLAYER_PORTRAITS];
extern STR8 pPlayerSelectedBigFaceFileNames[NUMBER_OF_PLAYER_PORTRAITS];
#endif
