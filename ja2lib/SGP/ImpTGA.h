#ifndef _impTGA_h
#define _impTGA_h

#include "SGP/HImage.h"
#include "SGP/Types.h"

BOOLEAN LoadTGAFileToImage(const char* filePath, struct Image* hImage, UINT16 fContents);

#endif
