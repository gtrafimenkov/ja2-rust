// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __TYPES_
#define __TYPES_

#include <stdbool.h>
#include <stdint.h>
#include <wchar.h>

#include "BuildDefines.h"
#include "LeanTypes.h"

// flags (individual bits used)
typedef unsigned char FLAGS8;
typedef uint16_t FLAGS16;
typedef unsigned long FLAGS32;
// other
typedef unsigned char BOOLEAN;
typedef uint16_t HNDL;
typedef uint8_t uint8_t;
typedef char STRING512[512];
typedef uint32_t FileID;

#define SGPFILENAME_LEN 100
typedef char SGPFILENAME[SGPFILENAME_LEN];

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define BAD_INDEX -1

#define NULL_HANDLE 65535

#define PI 3.1415926

#define ST_EPSILON 0.00001  // define a sir-tech epsilon value

#ifndef NULL
#define NULL 0
#endif

typedef struct {
  int32_t iX;
  int32_t iY;
} SGPPoint;

typedef struct {
  int32_t Min;
  int32_t Max;
} SGPRange;

typedef float VECTOR2[2];  // 2d vector (2x1 matrix)
typedef float VECTOR3[3];  // 3d vector (3x1 matrix)
typedef float VECTOR4[4];  // 4d vector (4x1 matrix)

typedef int32_t IVECTOR2[2];  // 2d vector (2x1 matrix)
typedef int32_t IVECTOR3[3];  // 3d vector (3x1 matrix)
typedef int32_t IVECTOR4[4];  // 4d vector (4x1 matrix)

typedef VECTOR3 MATRIX3[3];  // 3x3 matrix
typedef VECTOR4 MATRIX4[4];  // 4x4 matrix

typedef VECTOR3 ANGLE;  // angle return array
typedef VECTOR4 COLOR;  // rgba color array

#define ARR_SIZE(a) (sizeof(a) / sizeof(a[0]))

#if !defined(max)
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#if !defined(min)
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

// This definition mimics what is found in WINDOWS.H ( for Direct Draw compatiblity )
typedef uint32_t COLORVAL;

#endif
