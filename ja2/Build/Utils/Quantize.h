#ifndef __QUANTIZE_H_
#define __QUANTIZE_H_

#include "SGP/Types.h"

// copied from wingdi.h
typedef struct tagRGBQUAD {
  BYTE rgbBlue;
  BYTE rgbGreen;
  BYTE rgbRed;
  BYTE rgbReserved;
} RGBQUAD;

typedef struct _NODE {
  BOOLEAN bIsLeaf;          // TRUE if node has no children
  u32 nPixelCount;          // Number of pixels represented by this leaf
  u32 nRedSum;              // Sum of red components
  u32 nGreenSum;            // Sum of green components
  u32 nBlueSum;             // Sum of blue components
  struct _NODE* pChild[8];  // Pointers to child nodes
  struct _NODE* pNext;      // Pointer to next reducible node
} NODE;

class CQuantizer {
 protected:
  NODE* m_pTree;
  u32 m_nLeafCount;
  NODE* m_pReducibleNodes[9];
  u32 m_nMaxColors;
  u32 m_nColorBits;

 public:
  CQuantizer(u32 nMaxColors, u32 nColorBits);
  virtual ~CQuantizer();
  BOOLEAN ProcessImage(u8* pData, int iWidth, int iHeight);
  u32 GetColorCount();
  void GetColorTable(RGBQUAD* prgb);

 protected:
  int GetLeftShiftCount(u32 dwVal);
  int GetRightShiftCount(u32 dwVal);
  void AddColor(NODE** ppNode, u8 r, u8 g, u8 b, u32 nColorBits, u32 nLevel, u32* pLeafCount,
                NODE** pReducibleNodes);
  NODE* CreateNode(u32 nLevel, u32 nColorBits, u32* pLeafCount, NODE** pReducibleNodes);
  void ReduceTree(u32 nColorBits, u32* pLeafCount, NODE** pReducibleNodes);
  void DeleteTree(NODE** ppNode);
  void GetPaletteColors(NODE* pTree, RGBQUAD* prgb, u32* pIndex);
};

#endif
