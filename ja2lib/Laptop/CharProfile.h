#ifndef __CHARPROFILE_H
#define __CHARPROFILE_H

#include "SGP/Types.h"

void GameInitCharProfile();
void EnterCharProfile();
void ExitCharProfile();
void HandleCharProfile();
void RenderCharProfile();
void ResetCharacterStats(void);
void InitIMPSubPageList(void);

extern BOOLEAN fButtonPendingFlag;
extern BOOLEAN fReDrawCharProfile;
extern int32_t iCurrentImpPage;

// attributes
extern int32_t iStrength;
extern int32_t iDexterity;
extern int32_t iAgility;
extern int32_t iWisdom;
extern int32_t iLeadership;
extern int32_t iHealth;

// skills
extern int32_t iMarksmanship;
extern int32_t iMedical;
extern int32_t iExplosives;
extern int32_t iMechanical;

// sex?
extern BOOLEAN fCharacterIsMale;

// name?
extern wchar_t pFullName[];
extern wchar_t pNickName[];

// skills
extern int32_t iSkillA;
extern int32_t iSkillB;

// persoanlity
extern int32_t iPersonality;

// attitude
extern int32_t iAttitude;

// addtivies
extern int32_t iAddStrength;
extern int32_t iAddDexterity;
extern int32_t iAddAgility;
extern int32_t iAddWisdom;
extern int32_t iAddHealth;
extern int32_t iAddLeadership;

extern int32_t iAddMarksmanship;
extern int32_t iAddMedical;
extern int32_t iAddExplosives;
extern int32_t iAddMechanical;

// pop up strings
extern STR16 pImpPopUpStrings[];

// extern BOOLEAN fIMPCompletedFlag;

enum {
  IMP_HOME_PAGE,
  IMP_BEGIN,
  IMP_FINISH,
  IMP_MAIN_PAGE,
  IMP_PERSONALITY,
  IMP_PERSONALITY_QUIZ,
  IMP_PERSONALITY_FINISH,
  IMP_ATTRIBUTE_ENTRANCE,
  IMP_ATTRIBUTE_PAGE,
  IMP_ATTRIBUTE_FINISH,
  IMP_PORTRAIT,
  IMP_VOICE,
  IMP_ABOUT_US,
  IMP_CONFIRM,

  IMP_NUM_PAGES,
};

#define COST_OF_PROFILE 3000

#endif
