#ifndef _IMP_PERSOANLITY_QUIZ_H
#define _IMP_PERSOANLITY_QUIZ_H

#include "SGP/Types.h"

void EnterIMPPersonalityQuiz(void);
void RenderIMPPersonalityQuiz(void);
void ExitIMPPersonalityQuiz(void);
void HandleIMPPersonalityQuiz(void);

void BltAnswerIndents(int32_t iNumberOfIndents);

extern int32_t giCurrentPersonalityQuizQuestion;
extern int32_t iCurrentAnswer;

#endif
