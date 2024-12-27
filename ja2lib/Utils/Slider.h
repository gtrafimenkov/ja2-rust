#ifndef _SLIDER__H_
#define _SLIDER__H_

#include "SGP/Types.h"

#define SLIDER_VERTICAL 0x00000001
#define SLIDER_HORIZONTAL 0x00000002

// defines for the different styles of sliders
enum {
  SLIDER_DEFAULT_STYLE,

  SLIDER_VERTICAL_STEEL,

  NUM_SLIDER_STYLES,
};

typedef void (*SLIDER_CHANGE_CALLBACK)(INT32);

/*

ubStyle
usPosX
usPosY
usWidth
usNumberOfIncrements
sPriority
SliderChangeCallback
        void SliderChangeCallBack( INT32 iNewValue )
*/

INT32 AddSlider(uint8_t ubStyle, uint16_t usCursor, uint16_t usPosX, uint16_t usPosY,
                uint16_t usWidth, uint16_t usNumberOfIncrements, INT8 sPriority,
                SLIDER_CHANGE_CALLBACK SliderChangeCallback, uint32_t uiFlags);

BOOLEAN InitSlider();

void ShutDownSlider();

void RenderAllSliderBars();

void RemoveSliderBar(uint32_t uiSliderID);

void SetSliderValue(uint32_t uiSliderID, uint32_t uiNewValue);

#endif
