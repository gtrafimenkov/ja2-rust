	#include "Laptop/LaptopAll.h"
#ifdef PRECOMPILEDHEADERS
#else
	#include "Laptop/Laptop.h"
	#include "Laptop/AimSort.h"
	#include "Laptop/Aim.h"
	#include "SGP/WCheck.h"
	#include "Utils/Utilities.h"
	#include "Utils/WordWrap.h"
	#include "Tactical/SoldierProfile.h"
	#include "stdlib.h"
	#include "Strategic/GameClock.h"
	#include "Utils/Text.h"
	#include "Utils/MultiLanguageGraphicUtils.h"
#endif



//#define		

#define		AIM_SORT_FONT_TITLE									FONT14ARIAL
#define		AIM_SORT_FONT_SORT_TEXT							FONT10ARIAL

#define		AIM_SORT_COLOR_SORT_TEXT						AIM_FONT_MCOLOR_WHITE
#define		AIM_SORT_SORT_BY_COLOR							146
#define		AIM_SORT_LINK_TEXT_COLOR						146

#define		AIM_SORT_GAP_BN_ICONS				60
#define		AIM_SORT_CHECKBOX_SIZE			10
#define		AIM_SORT_ON									0
#define		AIM_SORT_OFF								1

#define		AIM_SORT_SORT_BY_X					IMAGE_OFFSET_X + 155
#define		AIM_SORT_SORT_BY_Y					IMAGE_OFFSET_Y + 96
#define		AIM_SORT_SORT_BY_WIDTH			190
#define		AIM_SORT_SORT_BY_HEIGHT			81

#define		AIM_SORT_TO_MUGSHOTS_X			IMAGE_OFFSET_X + 89
#define		AIM_SORT_TO_MUGSHOTS_Y			IMAGE_OFFSET_Y + 184
#define		AIM_SORT_TO_MUGSHOTS_SIZE		54

#define		AIM_SORT_TO_STATS_X					AIM_SORT_TO_MUGSHOTS_X
#define		AIM_SORT_TO_STATS_Y					AIM_SORT_TO_MUGSHOTS_Y + AIM_SORT_GAP_BN_ICONS
#define		AIM_SORT_TO_STATS_SIZE			AIM_SORT_TO_MUGSHOTS_SIZE	

#define		AIM_SORT_TO_ALUMNI_X				AIM_SORT_TO_MUGSHOTS_X
#define		AIM_SORT_TO_ALUMNI_Y				AIM_SORT_TO_STATS_Y + AIM_SORT_GAP_BN_ICONS
#define		AIM_SORT_TO_ALUMNI_SIZE			AIM_SORT_TO_MUGSHOTS_SIZE

#define		AIM_SORT_AIM_MEMBER_X				AIM_SORT_SORT_BY_X
#define		AIM_SORT_AIM_MEMBER_Y				105 + LAPTOP_SCREEN_WEB_DELTA_Y
#define		AIM_SORT_AIM_MEMBER_WIDTH		AIM_SORT_SORT_BY_WIDTH

#define		AIM_SORT_SORT_BY_TEXT_X			AIM_SORT_SORT_BY_X + 9
#define		AIM_SORT_SORT_BY_TEXT_Y			AIM_SORT_SORT_BY_Y + 8

#define		AIM_SORT_PRICE_TEXT_X				AIM_SORT_SORT_BY_X + 22
#define		AIM_SORT_PRICE_TEXT_Y				AIM_SORT_SORT_BY_Y + 36

#define		AIM_SORT_EXP_TEXT_X					AIM_SORT_PRICE_TEXT_X
#define		AIM_SORT_EXP_TEXT_Y					AIM_SORT_PRICE_TEXT_Y + 13
						
#define		AIM_SORT_MARKMNSHP_TEXT_X		AIM_SORT_PRICE_TEXT_X
#define		AIM_SORT_MARKMNSHP_TEXT_Y		AIM_SORT_EXP_TEXT_Y + 13

#define		AIM_SORT_MEDICAL_X					AIM_SORT_SORT_BY_X + 125
#define		AIM_SORT_MEDICAL_Y					AIM_SORT_PRICE_TEXT_Y
						
#define		AIM_SORT_EXPLOSIVES_X				AIM_SORT_MEDICAL_X
#define		AIM_SORT_EXPLOSIVES_Y				AIM_SORT_EXP_TEXT_Y

#define		AIM_SORT_MECHANICAL_X				AIM_SORT_MEDICAL_X
#define		AIM_SORT_MECHANICAL_Y				AIM_SORT_MARKMNSHP_TEXT_Y

#define		AIM_SORT_ASC_DESC_WIDTH			100
#define		AIM_SORT_ASCEND_TEXT_X			AIM_SORT_SORT_BY_X + 154 - AIM_SORT_ASC_DESC_WIDTH - 4 + 18
#define		AIM_SORT_ASCEND_TEXT_Y			128 + LAPTOP_SCREEN_WEB_DELTA_Y

#define		AIM_SORT_DESCEND_TEXT_X			AIM_SORT_ASCEND_TEXT_X
#define		AIM_SORT_DESCEND_TEXT_Y			141 + LAPTOP_SCREEN_WEB_DELTA_Y


#define		AIM_SORT_MUGSHOT_TEXT_X			266
#define		AIM_SORT_MUGSHOT_TEXT_Y			230 + LAPTOP_SCREEN_WEB_DELTA_Y

#define		AIM_SORT_MERC_STATS_TEXT_X	AIM_SORT_MUGSHOT_TEXT_X
#define		AIM_SORT_MERC_STATS_TEXT_Y	293 + LAPTOP_SCREEN_WEB_DELTA_Y

#define		AIM_SORT_ALUMNI_TEXT_X			AIM_SORT_MUGSHOT_TEXT_X
#define		AIM_SORT_ALUMNI_TEXT_Y			351 + LAPTOP_SCREEN_WEB_DELTA_Y

#define		AIM_SORT_FIRST_SORT_CLOUMN_GAP	22



UINT16		AimSortCheckBoxLoc[]={
							(AIM_SORT_SORT_BY_X + 9), (AIM_SORT_SORT_BY_Y + 34),
							(AIM_SORT_SORT_BY_X + 9), (AIM_SORT_SORT_BY_Y + 47),
							(AIM_SORT_SORT_BY_X + 9), (AIM_SORT_SORT_BY_Y + 60),
							(AIM_SORT_SORT_BY_X + 111), (AIM_SORT_SORT_BY_Y + 34),
							(AIM_SORT_SORT_BY_X + 111), (AIM_SORT_SORT_BY_Y + 47),
							(AIM_SORT_SORT_BY_X + 111), (AIM_SORT_SORT_BY_Y + 60),
							(AIM_SORT_SORT_BY_X + 172), (AIM_SORT_SORT_BY_Y + 4),
							(AIM_SORT_SORT_BY_X + 172), (AIM_SORT_SORT_BY_Y + 17)};

UINT8			gubCurrentSortMode;
UINT8			gubOldSortMode;
UINT8			gubCurrentListMode;
UINT8			gubOldListMode;

// Mouse stuff
//Clicking on To Mugshot
MOUSE_REGION    gSelectedToMugShotRegion;
void SelectToMugShotRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );

//Clicking on ToStats
MOUSE_REGION    gSelectedToStatsRegion;
void SelectToStatsRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );

//Clicking on ToStats
MOUSE_REGION    gSelectedToArchiveRegion;
void SelectToArchiveRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );

//Clicking on Price Check Box
MOUSE_REGION    gSelectedPriceBoxRegion;
void SelectPriceBoxRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );
//Clicking on Explosive Check Box
MOUSE_REGION    gSelectedExpBoxRegion;
void SelectExpBoxRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );
//Clicking on Markmanship Check Box
MOUSE_REGION    gSelectedMarkBoxRegion;
void SelectMarkBoxRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );
//Clicking on Medical Check box
MOUSE_REGION    gSelectedMedicalBoxRegion;
void SelectMedicalBoxRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );
//Clicking on Explosive Check Box
MOUSE_REGION    gSelectedExplosiveBoxRegion;
void SelectExplosiveBoxRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );
//Clicking on Mechanical Check Box
MOUSE_REGION    gSelectedMechanicalBoxRegion;
void SelectMechanicalBoxRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );
//Clicking on Ascending Check Box
MOUSE_REGION    gSelectedAscendBoxRegion;
void SelectAscendBoxRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );
//Clicking on Descending Check Box
MOUSE_REGION    gSelectedDescendBoxRegion;
void SelectDescendBoxRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );



void DrawSelectLight(UINT8 ubMode, UINT8 ubImage);
INT32 QsortCompare( const void *pNum1, const void *pNum2);
INT32 CompareValue(const INT32 Num1, const INT32 Num2);
BOOLEAN SortMercArray(void);


UINT32		guiSortByBox;
UINT32		guiToAlumni;
UINT32		guiToMugShots;
UINT32		guiToStats;
UINT32		guiSelectLight;





void GameInitAimSort()
{
	gubCurrentSortMode=0;
	gubOldSortMode=0;
	gubCurrentListMode=AIM_DESCEND;
	gubOldListMode=AIM_DESCEND;
}

BOOLEAN EnterAimSort()
{
  VOBJECT_DESC    VObjectDesc;
	UINT8						ubCurNumber=0;
	UINT16						ubWidth;
	UINT8	i;


	//Everytime into Aim Sort, reset array.
	for(i=0; i<MAX_NUMBER_MERCS; i++)
	{
		AimMercArray[i] = i;
	}

	InitAimDefaults();

	// load the SortBy box graphic and add it
	VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\SortBy.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiSortByBox));

	// load the ToAlumni graphic and add it
	VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	GetMLGFilename( VObjectDesc.ImageFile, MLG_TOALUMNI );
	CHECKF(AddVideoObject(&VObjectDesc, &guiToAlumni));

	// load the ToMugShots graphic and add it
	VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	GetMLGFilename( VObjectDesc.ImageFile, MLG_TOMUGSHOTS );
	CHECKF(AddVideoObject(&VObjectDesc, &guiToMugShots));

	// load the ToStats graphic and add it
	VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	GetMLGFilename( VObjectDesc.ImageFile, MLG_TOSTATS );
	CHECKF(AddVideoObject(&VObjectDesc, &guiToStats));

	// load the SelectLight graphic and add it
	VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\SelectLight.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiSelectLight));


	//** Mouse Regions **

	//Mouse region for the ToMugShotRegion
	MSYS_DefineRegion( &gSelectedToMugShotRegion, AIM_SORT_TO_MUGSHOTS_X, AIM_SORT_TO_MUGSHOTS_Y, (AIM_SORT_TO_MUGSHOTS_X + AIM_SORT_TO_MUGSHOTS_SIZE), (AIM_SORT_TO_MUGSHOTS_Y + AIM_SORT_TO_MUGSHOTS_SIZE), MSYS_PRIORITY_HIGH,
							 CURSOR_WWW, MSYS_NO_CALLBACK, SelectToMugShotRegionCallBack ); 
  MSYS_AddRegion(&gSelectedToMugShotRegion); 

	//Mouse region for the ToStatsRegion
	MSYS_DefineRegion( &gSelectedToStatsRegion, AIM_SORT_TO_STATS_X, AIM_SORT_TO_STATS_Y, (AIM_SORT_TO_STATS_X + AIM_SORT_TO_STATS_SIZE), (AIM_SORT_TO_STATS_Y + AIM_SORT_TO_STATS_SIZE), MSYS_PRIORITY_HIGH,
							 CURSOR_WWW, MSYS_NO_CALLBACK, SelectToStatsRegionCallBack ); 
  MSYS_AddRegion(&gSelectedToStatsRegion); 

	//Mouse region for the ToArhciveRegion
	MSYS_DefineRegion( &gSelectedToArchiveRegion, AIM_SORT_TO_ALUMNI_X, AIM_SORT_TO_ALUMNI_Y, (AIM_SORT_TO_ALUMNI_X + AIM_SORT_TO_ALUMNI_SIZE), (AIM_SORT_TO_ALUMNI_Y + AIM_SORT_TO_ALUMNI_SIZE), MSYS_PRIORITY_HIGH,
							 CURSOR_WWW, MSYS_NO_CALLBACK, SelectToArchiveRegionCallBack ); 
  MSYS_AddRegion(&gSelectedToArchiveRegion); 


	
	//CURSOR_WWW MSYS_NO_CURSOR
	ubCurNumber = 0;
	//Mouse region for the Price Check Box
	ubWidth = StringPixLength( AimSortText[PRICE], AIM_SORT_FONT_SORT_TEXT) + AimSortCheckBoxLoc[ ubCurNumber ] + (AIM_SORT_PRICE_TEXT_X - AimSortCheckBoxLoc[ ubCurNumber ]) - 3;
	MSYS_DefineRegion( &gSelectedPriceBoxRegion, AimSortCheckBoxLoc[ ubCurNumber ] , AimSortCheckBoxLoc[ubCurNumber + 1] , (UINT16)ubWidth, (UINT16)(AimSortCheckBoxLoc[ ubCurNumber + 1 ] + AIM_SORT_CHECKBOX_SIZE), MSYS_PRIORITY_HIGH,
							 MSYS_NO_CURSOR, MSYS_NO_CALLBACK, SelectPriceBoxRegionCallBack ); 
  MSYS_AddRegion(&gSelectedPriceBoxRegion); 

	ubCurNumber+=2;
	ubWidth = StringPixLength( AimSortText[EXPERIENCE], AIM_SORT_FONT_SORT_TEXT) + AimSortCheckBoxLoc[ ubCurNumber ] + (AIM_SORT_PRICE_TEXT_X - AimSortCheckBoxLoc[ ubCurNumber ]) - 3;
	//Mouse region for the Experience Check Box
	MSYS_DefineRegion( &gSelectedExpBoxRegion, AimSortCheckBoxLoc[ ubCurNumber ] , AimSortCheckBoxLoc[ubCurNumber + 1] , (UINT16)ubWidth, (UINT16)(AimSortCheckBoxLoc[ ubCurNumber + 1 ] + AIM_SORT_CHECKBOX_SIZE), MSYS_PRIORITY_HIGH,
							 MSYS_NO_CURSOR, MSYS_NO_CALLBACK, SelectExpBoxRegionCallBack ); 
  MSYS_AddRegion(&gSelectedExpBoxRegion); 
	
	ubCurNumber+=2;
	ubWidth = StringPixLength( AimSortText[AIMMARKSMANSHIP], AIM_SORT_FONT_SORT_TEXT) + AimSortCheckBoxLoc[ ubCurNumber ] + (AIM_SORT_PRICE_TEXT_X - AimSortCheckBoxLoc[ ubCurNumber ]) - 3;
	//Mouse region for the Markmanship Check Box
	MSYS_DefineRegion( &gSelectedMarkBoxRegion, AimSortCheckBoxLoc[ ubCurNumber ] , AimSortCheckBoxLoc[ubCurNumber + 1] , (UINT16)ubWidth, (UINT16)(AimSortCheckBoxLoc[ ubCurNumber + 1 ] + AIM_SORT_CHECKBOX_SIZE), MSYS_PRIORITY_HIGH,
							 MSYS_NO_CURSOR, MSYS_NO_CALLBACK, SelectMarkBoxRegionCallBack ); 
  MSYS_AddRegion(&gSelectedMarkBoxRegion); 
	
	ubCurNumber+=2;
	ubWidth = StringPixLength( AimSortText[AIMMEDICAL], AIM_SORT_FONT_SORT_TEXT) + AimSortCheckBoxLoc[ ubCurNumber ] + (AIM_SORT_MEDICAL_X - AimSortCheckBoxLoc[ ubCurNumber ]) - 3;
	//Mouse region for the Medical  Check Box
	MSYS_DefineRegion( &gSelectedMedicalBoxRegion, AimSortCheckBoxLoc[ ubCurNumber ] , AimSortCheckBoxLoc[ubCurNumber + 1] , (UINT16)ubWidth, (UINT16)(AimSortCheckBoxLoc[ ubCurNumber + 1 ] + AIM_SORT_CHECKBOX_SIZE), MSYS_PRIORITY_HIGH,
							 MSYS_NO_CURSOR, MSYS_NO_CALLBACK, SelectMedicalBoxRegionCallBack ); 
  MSYS_AddRegion(&gSelectedMedicalBoxRegion); 

	ubCurNumber+=2;
	ubWidth = StringPixLength( AimSortText[EXPLOSIVES], AIM_SORT_FONT_SORT_TEXT) + AimSortCheckBoxLoc[ ubCurNumber ] + (AIM_SORT_MEDICAL_X - AimSortCheckBoxLoc[ ubCurNumber ]) - 3;
	//Mouse region for the Explosive  Check Box
	MSYS_DefineRegion( &gSelectedExplosiveBoxRegion, AimSortCheckBoxLoc[ ubCurNumber ] , AimSortCheckBoxLoc[ubCurNumber + 1] , (UINT16)ubWidth, (UINT16)(AimSortCheckBoxLoc[ ubCurNumber + 1 ] + AIM_SORT_CHECKBOX_SIZE), MSYS_PRIORITY_HIGH,
							 MSYS_NO_CURSOR, MSYS_NO_CALLBACK, SelectExplosiveBoxRegionCallBack ); 
  MSYS_AddRegion(&gSelectedExplosiveBoxRegion); 

	ubCurNumber+=2;
	ubWidth = StringPixLength( AimSortText[AIMMECHANICAL], AIM_SORT_FONT_SORT_TEXT) + AimSortCheckBoxLoc[ ubCurNumber ] + (AIM_SORT_MEDICAL_X - AimSortCheckBoxLoc[ ubCurNumber ]) - 3;
	//Mouse region for the Mechanical Check Box
	MSYS_DefineRegion( &gSelectedMechanicalBoxRegion, AimSortCheckBoxLoc[ ubCurNumber ] , AimSortCheckBoxLoc[ubCurNumber + 1] , (UINT16)ubWidth, (UINT16)(AimSortCheckBoxLoc[ ubCurNumber + 1 ] + AIM_SORT_CHECKBOX_SIZE), MSYS_PRIORITY_HIGH,
							 MSYS_NO_CURSOR, MSYS_NO_CALLBACK, SelectMechanicalBoxRegionCallBack ); 
  MSYS_AddRegion(&gSelectedMechanicalBoxRegion); 


	ubCurNumber+=2;

	ubWidth = AimSortCheckBoxLoc[ ubCurNumber ] - StringPixLength( AimSortText[ASCENDING], AIM_SORT_FONT_SORT_TEXT) - 6;
	//Mouse region for the Ascend Check Box
	MSYS_DefineRegion( &gSelectedAscendBoxRegion, ubWidth , AimSortCheckBoxLoc[ubCurNumber + 1] , (UINT16)(AimSortCheckBoxLoc[ ubCurNumber ] + AIM_SORT_CHECKBOX_SIZE), (UINT16)(AimSortCheckBoxLoc[ ubCurNumber + 1 ] + AIM_SORT_CHECKBOX_SIZE), MSYS_PRIORITY_HIGH,
							 MSYS_NO_CURSOR, MSYS_NO_CALLBACK, SelectAscendBoxRegionCallBack ); 
  MSYS_AddRegion(&gSelectedAscendBoxRegion); 

	ubCurNumber+=2;
	ubWidth = AimSortCheckBoxLoc[ ubCurNumber ] - StringPixLength( AimSortText[DESCENDING], AIM_SORT_FONT_SORT_TEXT) - 6;

	//Mouse region for the Descend Check Box
	MSYS_DefineRegion( &gSelectedDescendBoxRegion, ubWidth, AimSortCheckBoxLoc[ubCurNumber + 1] , (UINT16)(AimSortCheckBoxLoc[ ubCurNumber ] + AIM_SORT_CHECKBOX_SIZE), (UINT16)(AimSortCheckBoxLoc[ ubCurNumber + 1 ] + AIM_SORT_CHECKBOX_SIZE), MSYS_PRIORITY_HIGH,
							 MSYS_NO_CURSOR, MSYS_NO_CALLBACK, SelectDescendBoxRegionCallBack ); 
  MSYS_AddRegion(&gSelectedDescendBoxRegion); 


	InitAimMenuBar();

	
	RenderAimSort();

	return( TRUE );
}

void ExitAimSort()
{
	// Sort the merc array
	SortMercArray();
	RemoveAimDefaults();

	DeleteVideoObjectFromIndex(guiSortByBox);
	DeleteVideoObjectFromIndex(guiToAlumni);
	DeleteVideoObjectFromIndex(guiToMugShots);
	DeleteVideoObjectFromIndex(guiToStats);
	DeleteVideoObjectFromIndex(guiSelectLight);

  MSYS_RemoveRegion( &gSelectedToMugShotRegion);
  MSYS_RemoveRegion( &gSelectedToStatsRegion);
  MSYS_RemoveRegion( &gSelectedToArchiveRegion);

  MSYS_RemoveRegion( &gSelectedPriceBoxRegion);
  MSYS_RemoveRegion( &gSelectedExpBoxRegion);
  MSYS_RemoveRegion( &gSelectedMarkBoxRegion);
  MSYS_RemoveRegion( &gSelectedMedicalBoxRegion);
  MSYS_RemoveRegion( &gSelectedExplosiveBoxRegion);
  MSYS_RemoveRegion( &gSelectedMechanicalBoxRegion);
  MSYS_RemoveRegion( &gSelectedAscendBoxRegion);
  MSYS_RemoveRegion( &gSelectedDescendBoxRegion);
	ExitAimMenuBar();

}

void HandleAimSort()
{

}

void RenderAimSort()
{
  HVOBJECT	hSortByHandle;
  HVOBJECT	hToAlumniHandle;
  HVOBJECT	hToMugShotHandle;
  HVOBJECT	hToStatsHandle;

	DrawAimDefaults();
	// SortBy
	GetVideoObject(&hSortByHandle, guiSortByBox);
  BltVideoObject(FRAME_BUFFER, hSortByHandle, 0,AIM_SORT_SORT_BY_X, AIM_SORT_SORT_BY_Y, VO_BLT_SRCTRANSPARENCY,NULL);

	// To MugShots
	GetVideoObject(&hToMugShotHandle, guiToMugShots);
  BltVideoObject(FRAME_BUFFER, hToMugShotHandle, 0,AIM_SORT_TO_MUGSHOTS_X, AIM_SORT_TO_MUGSHOTS_Y, VO_BLT_SRCTRANSPARENCY,NULL);

	// To stats
	GetVideoObject(&hToStatsHandle, guiToStats);
  BltVideoObject(FRAME_BUFFER, hToStatsHandle, 0,AIM_SORT_TO_STATS_X, AIM_SORT_TO_STATS_Y, VO_BLT_SRCTRANSPARENCY,NULL);

	// To Alumni
	GetVideoObject(&hToAlumniHandle, guiToAlumni);
  BltVideoObject(FRAME_BUFFER, hToAlumniHandle, 0,AIM_SORT_TO_ALUMNI_X, AIM_SORT_TO_ALUMNI_Y, VO_BLT_SRCTRANSPARENCY,NULL);


	// Draw the aim slogan under the symbol
	DisplayAimSlogan();

	//Display AIM Member text
	DrawTextToScreen(AimSortText[AIM_AIMMEMBERS], AIM_SORT_AIM_MEMBER_X, AIM_SORT_AIM_MEMBER_Y, AIM_SORT_AIM_MEMBER_WIDTH, AIM_MAINTITLE_FONT, AIM_MAINTITLE_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED	);

	//Display sort title
	DrawTextToScreen(AimSortText[SORT_BY], AIM_SORT_SORT_BY_TEXT_X, AIM_SORT_SORT_BY_TEXT_Y, 0, AIM_SORT_FONT_TITLE, AIM_SORT_SORT_BY_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED	);

	//Display all the sort by text
	DrawTextToScreen(AimSortText[PRICE], AIM_SORT_PRICE_TEXT_X, AIM_SORT_PRICE_TEXT_Y, 0, AIM_SORT_FONT_SORT_TEXT, AIM_SORT_COLOR_SORT_TEXT, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED	);
	DrawTextToScreen(AimSortText[EXPERIENCE], AIM_SORT_EXP_TEXT_X, AIM_SORT_EXP_TEXT_Y, 0, AIM_SORT_FONT_SORT_TEXT, AIM_SORT_COLOR_SORT_TEXT, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED	);
	DrawTextToScreen(AimSortText[AIMMARKSMANSHIP], AIM_SORT_MARKMNSHP_TEXT_X, AIM_SORT_MARKMNSHP_TEXT_Y, 0, AIM_SORT_FONT_SORT_TEXT, AIM_SORT_COLOR_SORT_TEXT, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED	);
	DrawTextToScreen(AimSortText[AIMMEDICAL], AIM_SORT_MEDICAL_X, AIM_SORT_MEDICAL_Y, 0, AIM_SORT_FONT_SORT_TEXT, AIM_SORT_COLOR_SORT_TEXT, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED	);
	DrawTextToScreen(AimSortText[EXPLOSIVES], AIM_SORT_EXPLOSIVES_X, AIM_SORT_EXPLOSIVES_Y, 0, AIM_SORT_FONT_SORT_TEXT, AIM_SORT_COLOR_SORT_TEXT, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED	);
	DrawTextToScreen(AimSortText[AIMMECHANICAL], AIM_SORT_MECHANICAL_X, AIM_SORT_MECHANICAL_Y, 0, AIM_SORT_FONT_SORT_TEXT, AIM_SORT_COLOR_SORT_TEXT, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED	);

	DrawTextToScreen(AimSortText[ASCENDING], AIM_SORT_ASCEND_TEXT_X, AIM_SORT_ASCEND_TEXT_Y, AIM_SORT_ASC_DESC_WIDTH, AIM_SORT_FONT_SORT_TEXT, AIM_SORT_COLOR_SORT_TEXT, FONT_MCOLOR_BLACK, FALSE, RIGHT_JUSTIFIED	);
	DrawTextToScreen(AimSortText[DESCENDING], AIM_SORT_DESCEND_TEXT_X, AIM_SORT_DESCEND_TEXT_Y, AIM_SORT_ASC_DESC_WIDTH, AIM_SORT_FONT_SORT_TEXT, AIM_SORT_COLOR_SORT_TEXT, FONT_MCOLOR_BLACK, FALSE, RIGHT_JUSTIFIED	);


	// Display text for the 3 icons
	DrawTextToScreen(AimSortText[MUGSHOT_INDEX], AIM_SORT_MUGSHOT_TEXT_X, AIM_SORT_MUGSHOT_TEXT_Y, 0, AIM_SORT_FONT_SORT_TEXT, AIM_SORT_LINK_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED	);
	DrawTextToScreen(AimSortText[MERCENARY_FILES], AIM_SORT_MERC_STATS_TEXT_X, AIM_SORT_MERC_STATS_TEXT_Y, 0, AIM_SORT_FONT_SORT_TEXT, AIM_SORT_LINK_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED	);
	DrawTextToScreen(AimSortText[ALUMNI_GALLERY], AIM_SORT_ALUMNI_TEXT_X, AIM_SORT_ALUMNI_TEXT_Y, 0, AIM_SORT_FONT_SORT_TEXT, AIM_SORT_LINK_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED	);

	DrawSelectLight(gubCurrentSortMode, AIM_SORT_ON);
	DrawSelectLight(gubCurrentListMode, AIM_SORT_ON);

	DisableAimButton();

  MarkButtonsDirty( );

	RenderWWWProgramTitleBar( );

  InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}




void SelectToMugShotRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason )
{ 
	if (iReason & MSYS_CALLBACK_REASON_INIT)
	{

	}
	else if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_AIM_MEMBERS_FACIAL_INDEX;
	}
	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
	} 
}


void SelectToStatsRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason )
{ 
	if (iReason & MSYS_CALLBACK_REASON_INIT)
	{

	}
	else if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_AIM_MEMBERS;
	}
	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
	} 
}



void SelectToArchiveRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason )
{ 
	if (iReason & MSYS_CALLBACK_REASON_INIT)
	{

	}
	else if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_AIM_MEMBERS_ARCHIVES;
	}
	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
	} 
}



void SelectPriceBoxRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason )
{ 
	if (iReason & MSYS_CALLBACK_REASON_INIT)
	{

	}
	else if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if(gubCurrentSortMode != 0)
		{
			gubCurrentSortMode = 0;
			DrawSelectLight(gubCurrentSortMode, AIM_SORT_ON);
			DrawSelectLight(gubOldSortMode, AIM_SORT_OFF);
			gubOldSortMode = gubCurrentSortMode;
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
	} 
}



void SelectExpBoxRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason )
{ 
	if (iReason & MSYS_CALLBACK_REASON_INIT)
	{

	}
	else if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if(gubCurrentSortMode != 1)
		{
			gubCurrentSortMode = 1;
			DrawSelectLight(gubCurrentSortMode, AIM_SORT_ON);
			DrawSelectLight(gubOldSortMode, AIM_SORT_OFF);
			gubOldSortMode = gubCurrentSortMode;
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
	} 
}


void SelectMarkBoxRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason )
{ 
	if (iReason & MSYS_CALLBACK_REASON_INIT)
	{

	}
	else if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if(gubCurrentSortMode != 2)
		{
			gubCurrentSortMode = 2;
			DrawSelectLight(gubCurrentSortMode, AIM_SORT_ON);
			DrawSelectLight(gubOldSortMode, AIM_SORT_OFF);
			gubOldSortMode = gubCurrentSortMode;
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
	} 
}



void SelectMedicalBoxRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason )
{ 
	if (iReason & MSYS_CALLBACK_REASON_INIT)
	{

	}
	else if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if(gubCurrentSortMode != 3)
		{
			gubCurrentSortMode = 3;
			DrawSelectLight(gubCurrentSortMode, AIM_SORT_ON);
			DrawSelectLight(gubOldSortMode, AIM_SORT_OFF);
			gubOldSortMode = gubCurrentSortMode;
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
	} 
}



void SelectExplosiveBoxRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason )
{ 
	if (iReason & MSYS_CALLBACK_REASON_INIT)
	{

	}
	else if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if(gubCurrentSortMode != 4)
		{
			gubCurrentSortMode = 4;
			DrawSelectLight(gubCurrentSortMode, AIM_SORT_ON);
			DrawSelectLight(gubOldSortMode, AIM_SORT_OFF);
			gubOldSortMode = gubCurrentSortMode;
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
	} 
}



void SelectMechanicalBoxRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason )
{ 
	if (iReason & MSYS_CALLBACK_REASON_INIT)
	{

	}
	else if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if(gubCurrentSortMode != 5)
		{
			gubCurrentSortMode = 5;
			DrawSelectLight(gubCurrentSortMode, AIM_SORT_ON);
			DrawSelectLight(gubOldSortMode, AIM_SORT_OFF);
			gubOldSortMode = gubCurrentSortMode;
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
	} 
}



void SelectAscendBoxRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason )
{ 
	if (iReason & MSYS_CALLBACK_REASON_INIT)
	{

	}
	else if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if(gubCurrentListMode != AIM_ASCEND)
		{
			gubCurrentListMode = AIM_ASCEND;
			DrawSelectLight(gubCurrentListMode, AIM_SORT_ON);
			DrawSelectLight(gubOldListMode, AIM_SORT_OFF);
			gubOldListMode = gubCurrentListMode;
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
	} 
}



void SelectDescendBoxRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason )
{ 
	if (iReason & MSYS_CALLBACK_REASON_INIT)
	{

	}
	else if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if(gubCurrentListMode != AIM_DESCEND )
		{
			gubCurrentListMode = AIM_DESCEND;
			DrawSelectLight(gubCurrentListMode, AIM_SORT_ON);
			DrawSelectLight(gubOldListMode, AIM_SORT_OFF);
			gubOldListMode = gubCurrentListMode;
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
	} 
}



void DrawSelectLight(UINT8 ubMode, UINT8 ubImage)
{
	HVOBJECT	hSelectLightHandle;

	ubMode *= 2;

	GetVideoObject(&hSelectLightHandle, guiSelectLight);
  BltVideoObject(FRAME_BUFFER, hSelectLightHandle, ubImage, (AimSortCheckBoxLoc[ubMode] ), ( AimSortCheckBoxLoc[ubMode + 1] ), VO_BLT_SRCTRANSPARENCY,NULL);

//  InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);

  InvalidateRegion(AimSortCheckBoxLoc[ubMode],AimSortCheckBoxLoc[ubMode+1], (AimSortCheckBoxLoc[ubMode] + AIM_SORT_CHECKBOX_SIZE), (AimSortCheckBoxLoc[ubMode+1] + AIM_SORT_CHECKBOX_SIZE) );
}





BOOLEAN SortMercArray(void)
{
	qsort( (LPVOID)AimMercArray, (size_t) MAX_NUMBER_MERCS, sizeof(UINT8), QsortCompare);

	return(TRUE);
}




INT32 QsortCompare( const void *pNum1, const void *pNum2)
{
	UINT8 Num1 = *(UINT8*)pNum1;
	UINT8 Num2 = *(UINT8*)pNum2;

	switch( gubCurrentSortMode )
	{
		//Price						INT16	uiWeeklySalary
		case 0:
			return( CompareValue((INT32)gMercProfiles[ Num1 ].uiWeeklySalary,  (INT32)gMercProfiles[Num2].uiWeeklySalary ) );
			break;
		//Experience			INT16	bExpLevel
		case 1:
			return( CompareValue((INT32)gMercProfiles[ Num1 ].bExpLevel,  (INT32)gMercProfiles[Num2].bExpLevel) );
			break;
		//Marksmanship		INT16	bMarksmanship
		case 2:
			return( CompareValue((INT32)gMercProfiles[ Num1 ].bMarksmanship,  (INT32)gMercProfiles[Num2].bMarksmanship ) );
			break;
		//Medical					INT16	bMedical
		case 3:
			return( CompareValue((INT32)gMercProfiles[ Num1 ].bMedical,  (INT32)gMercProfiles[Num2].bMedical ) );
			break;
		//Explosives			INT16	bExplosive
		case 4:
			return( CompareValue((INT32)gMercProfiles[ Num1 ].bExplosive,  (INT32)gMercProfiles[Num2].bExplosive ) );
			break;
		//Mechanical			INT16	bMechanical
		case 5:
			return( CompareValue((INT32)gMercProfiles[ Num1 ].bMechanical,  (INT32)gMercProfiles[Num2].bMechanical ) );
			break;

		default:
			Assert( 0 );
			return( 0 );
			break;
	}
}




INT32 CompareValue(const INT32 Num1, const INT32 Num2)
{
	// Ascending
	if( gubCurrentListMode == AIM_ASCEND)
	{
		if( Num1 < Num2)
			return(-1);
		else if( Num1 == Num2)
			return(0);
		else
			return(1);
	}

	// Descending
	else if( gubCurrentListMode == AIM_DESCEND )
	{
		if( Num1 > Num2)
			return(-1);
		else if( Num1 == Num2)
			return(0);
		else
			return(1);
	}

	return( 0 );
}