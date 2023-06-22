#ifndef __JASCREENS_H_
#define __JASCREENS_H_

#include "SGP/Types.h"

extern uint32_t EditScreenInit(void);
extern uint32_t EditScreenHandle(void);
extern uint32_t EditScreenShutdown(void);

extern uint32_t LoadSaveScreenInit(void);
extern uint32_t LoadSaveScreenHandle(void);
extern uint32_t LoadSaveScreenShutdown(void);

extern uint32_t SavingScreenInitialize(void);
extern uint32_t SavingScreenHandle(void);
extern uint32_t SavingScreenShutdown(void);

extern uint32_t LoadingScreenInitialize(void);
extern uint32_t LoadingScreenHandle(void);
extern uint32_t LoadingScreenShutdown(void);

extern uint32_t ErrorScreenInitialize(void);
extern uint32_t ErrorScreenHandle(void);
extern uint32_t ErrorScreenShutdown(void);

extern uint32_t InitScreenInitialize(void);
extern uint32_t InitScreenHandle(void);
extern uint32_t InitScreenShutdown(void);

extern uint32_t MainGameScreenInit(void);
extern uint32_t MainGameScreenHandle(void);
extern uint32_t MainGameScreenShutdown(void);

extern uint32_t QuestDebugScreenInit(void);
extern uint32_t QuestDebugScreenHandle(void);
extern uint32_t QuestDebugScreenShutdown(void);

uint32_t AniEditScreenInit(void);
uint32_t AniEditScreenHandle(void);
uint32_t AniEditScreenShutdown(void);

uint32_t PalEditScreenInit(void);
uint32_t PalEditScreenHandle(void);
uint32_t PalEditScreenShutdown(void);

uint32_t DebugScreenInit(void);
uint32_t DebugScreenHandle(void);
uint32_t DebugScreenShutdown(void);

extern uint32_t MapScreenInit(void);
extern uint32_t MapScreenHandle(void);
extern uint32_t MapScreenShutdown(void);

uint32_t LaptopScreenInit(void);
uint32_t LaptopScreenHandle(void);
uint32_t LaptopScreenShutdown(void);

uint32_t FadeScreenInit(void);
uint32_t FadeScreenHandle(void);
uint32_t FadeScreenShutdown(void);

uint32_t MessageBoxScreenInit(void);
uint32_t MessageBoxScreenHandle(void);
uint32_t MessageBoxScreenShutdown(void);

uint32_t MainMenuScreenInit(void);
uint32_t MainMenuScreenHandle(void);
uint32_t MainMenuScreenShutdown(void);

uint32_t AutoResolveScreenInit(void);
uint32_t AutoResolveScreenHandle(void);
uint32_t AutoResolveScreenShutdown(void);

uint32_t SaveLoadScreenShutdown(void);
uint32_t SaveLoadScreenHandle(void);
uint32_t SaveLoadScreenInit(void);

uint32_t ShopKeeperScreenInit(void);
uint32_t ShopKeeperScreenHandle(void);
uint32_t ShopKeeperScreenShutdown(void);

uint32_t SexScreenInit(void);
uint32_t SexScreenHandle(void);
uint32_t SexScreenShutdown(void);

uint32_t GameInitOptionsScreenInit(void);
uint32_t GameInitOptionsScreenHandle(void);
uint32_t GameInitOptionsScreenShutdown(void);

uint32_t DemoExitScreenInit(void);
uint32_t DemoExitScreenHandle(void);
uint32_t DemoExitScreenShutdown(void);

extern uint32_t IntroScreenShutdown(void);
extern uint32_t IntroScreenHandle(void);
extern uint32_t IntroScreenInit(void);

extern uint32_t CreditScreenInit(void);
extern uint32_t CreditScreenHandle(void);
extern uint32_t CreditScreenShutdown(void);

// External functions
extern void DisplayFrameRate();

void HandleTitleScreenAnimation();

// External Globals
extern char gubFilename[200];
extern uint32_t guiCurrentScreen;

typedef void (*RENDER_HOOK)(void);

void SetRenderHook(RENDER_HOOK pRenderOverride);
void SetCurrentScreen(uint32_t uiNewScreen);
void SetDebugRenderHook(RENDER_HOOK pDebugRenderOverride, int8_t ubPage);

void DisableFPSOverlay(BOOLEAN fEnable);

void EnterTacticalScreen();

#endif
