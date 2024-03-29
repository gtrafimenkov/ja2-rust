#ifndef __TYPES_
#define __TYPES_

#include <stdbool.h>
#include <stdint.h>
#include <wchar.h>

#include "BuildDefines.h"
#include "LeanTypes.h"

// *** SIR-TECH TYPE DEFINITIONS ***

// These two types are defined by VC6 and were causing redefinition
// problems, but JA2 is compiled with VC5

// HEY WIZARDRY DUDES, JA2 ISN'T THE ONLY PROGRAM WE COMPILE! :-)

typedef uint32_t UINT32;
typedef int32_t INT32;

// integers
typedef unsigned char UINT8;
typedef signed char INT8;
typedef uint16_t UINT16;
typedef int16_t INT16;
// floats
typedef float FLOAT;
typedef double DOUBLE;
// strings
typedef char CHAR8;
typedef wchar_t CHAR16;
typedef char* STR;
typedef char* STR8;
typedef wchar_t* STR16;
// flags (individual bits used)
typedef unsigned char FLAGS8;
typedef uint16_t FLAGS16;
typedef unsigned long FLAGS32;
// other
typedef unsigned char BOOLEAN;
typedef void* PTR;
typedef uint16_t HNDL;
typedef UINT8 BYTE;
typedef CHAR8 STRING512[512];
typedef UINT32 FileID;

#define SGPFILENAME_LEN 100
typedef CHAR8 SGPFILENAME[SGPFILENAME_LEN];

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
  INT32 iX;
  INT32 iY;
} SGPPoint;

typedef struct {
  INT32 Min;
  INT32 Max;
} SGPRange;

typedef FLOAT VECTOR2[2];  // 2d vector (2x1 matrix)
typedef FLOAT VECTOR3[3];  // 3d vector (3x1 matrix)
typedef FLOAT VECTOR4[4];  // 4d vector (4x1 matrix)

typedef INT32 IVECTOR2[2];  // 2d vector (2x1 matrix)
typedef INT32 IVECTOR3[3];  // 3d vector (3x1 matrix)
typedef INT32 IVECTOR4[4];  // 4d vector (4x1 matrix)

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

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

// This definition mimics what is found in WINDOWS.H ( for Direct Draw compatiblity )
typedef UINT32 COLORVAL;

#endif
