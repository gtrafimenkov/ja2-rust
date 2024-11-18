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

typedef void (*SLIDER_CHANGE_CALLBACK)(int32_t);

/*

ubStyle
usPosX
usPosY
usWidth
usNumberOfIncrements
sPriority
SliderChangeCallback
        void SliderChangeCallBack( int32_t iNewValue )
*/

int32_t AddSlider(uint8_t ubStyle, uint16_t usCursor, uint16_t usPosX, uint16_t usPosY, uint16_t usWidth,
                uint16_t usNumberOfIncrements, int8_t sPriority,
                SLIDER_CHANGE_CALLBACK SliderChangeCallback, uint32_t uiFlags);

BOOLEAN InitSlider();

void ShutDownSlider();

void RenderAllSliderBars();

void RemoveSliderBar(uint32_t uiSliderID);

void SetSliderValue(uint32_t uiSliderID, uint32_t uiNewValue);

#endif
