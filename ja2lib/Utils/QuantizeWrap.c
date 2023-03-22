#include "Utils/QuantizeWrap.h"

#include <string.h>

#include "SGP/HImage.h"
#include "SGP/PaletteEntry.h"
#include "SGP/Types.h"
#include "TileEngine/PhysMath.h"

///////////////////////////////////////////////////////////////////////////////////
// Quantize.h
///////////////////////////////////////////////////////////////////////////////////

// copied from wingdi.h
typedef struct tagRGBQUAD {
  uint8_t rgbBlue;
  uint8_t rgbGreen;
  uint8_t rgbRed;
  uint8_t rgbReserved;
} RGBQUAD;

typedef struct _NODE {
  BOOLEAN bIsLeaf;          // TRUE if node has no children
  uint32_t nPixelCount;     // Number of pixels represented by this leaf
  uint32_t nRedSum;         // Sum of red components
  uint32_t nGreenSum;       // Sum of green components
  uint32_t nBlueSum;        // Sum of blue components
  struct _NODE* pChild[8];  // Pointers to child nodes
  struct _NODE* pNext;      // Pointer to next reducible node
} NODE;

struct CQuantizer {
  NODE* m_pTree;
  uint32_t m_nLeafCount;
  NODE* m_pReducibleNodes[9];
  uint32_t m_nMaxColors;
  uint32_t m_nColorBits;
};

static void CQuantizer_CQuantizer(struct CQuantizer* cq, uint32_t nMaxColors, uint32_t nColorBits);
static BOOLEAN CQuantizer_ProcessImage(struct CQuantizer* cq, uint8_t* pData, int iWidth,
                                       int iHeight);
static uint32_t CQuantizer_GetColorCount(struct CQuantizer* cq);
static void CQuantizer_GetColorTable(struct CQuantizer* cq, RGBQUAD* prgb);

static void CQuantizer_AddColor(NODE** ppNode, uint8_t r, uint8_t g, uint8_t b, uint32_t nColorBits,
                                uint32_t nLevel, uint32_t* pLeafCount, NODE** pReducibleNodes);
static NODE* CQuantizer_CreateNode(uint32_t nLevel, uint32_t nColorBits, uint32_t* pLeafCount,
                                   NODE** pReducibleNodes);
static void CQuantizer_ReduceTree(uint32_t nColorBits, uint32_t* pLeafCount,
                                  NODE** pReducibleNodes);
static void CQuantizer_GetPaletteColors(NODE* pTree, RGBQUAD* prgb, uint32_t* pIndex);

///////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////

static void CQuantizer_CQuantizer(struct CQuantizer* cq, uint32_t nMaxColors, uint32_t nColorBits) {
  cq->m_pTree = NULL;
  cq->m_nLeafCount = 0;
  for (int i = 0; i <= (int)nColorBits; i++) cq->m_pReducibleNodes[i] = NULL;
  cq->m_nMaxColors = nMaxColors;
  cq->m_nColorBits = nColorBits;
}

static BOOLEAN CQuantizer_ProcessImage(struct CQuantizer* cq, uint8_t* pData, int iWidth,
                                       int iHeight) {
  uint8_t* pbBits;
  uint8_t r, g, b;
  int i, j;

  pbBits = (uint8_t*)pData;
  for (i = 0; i < iHeight; i++) {
    for (j = 0; j < iWidth; j++) {
      b = *pbBits++;
      g = *pbBits++;
      r = *pbBits++;
      CQuantizer_AddColor(&cq->m_pTree, r, g, b, cq->m_nColorBits, 0, &cq->m_nLeafCount,
                          cq->m_pReducibleNodes);
      while (cq->m_nLeafCount > cq->m_nMaxColors)
        CQuantizer_ReduceTree(cq->m_nColorBits, &cq->m_nLeafCount, cq->m_pReducibleNodes);
    }
    // Padding
    // pbBits ++;
  }
  return TRUE;
}

static void CQuantizer_AddColor(NODE** ppNode, uint8_t r, uint8_t g, uint8_t b, uint32_t nColorBits,
                                uint32_t nLevel, uint32_t* pLeafCount, NODE** pReducibleNodes) {
  static uint8_t mask[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

  //
  // If the node doesn't exist, create it.
  //
  if (*ppNode == NULL)
    *ppNode = CQuantizer_CreateNode(nLevel, nColorBits, pLeafCount, pReducibleNodes);

  //
  // Update color information if it's a leaf node.
  //
  if ((*ppNode)->bIsLeaf) {
    (*ppNode)->nPixelCount++;
    (*ppNode)->nRedSum += r;
    (*ppNode)->nGreenSum += g;
    (*ppNode)->nBlueSum += b;
  }

  //
  // Recurse a level deeper if the node is not a leaf.
  //
  else {
    int shift = 7 - nLevel;
    int nIndex = (((r & mask[nLevel]) >> shift) << 2) | (((g & mask[nLevel]) >> shift) << 1) |
                 ((b & mask[nLevel]) >> shift);
    CQuantizer_AddColor(&((*ppNode)->pChild[nIndex]), r, g, b, nColorBits, nLevel + 1, pLeafCount,
                        pReducibleNodes);
  }
}

static NODE* CQuantizer_CreateNode(uint32_t nLevel, uint32_t nColorBits, uint32_t* pLeafCount,
                                   NODE** pReducibleNodes) {
  NODE* pNode;

  pNode = (NODE*)MemAlloc(sizeof(NODE));
  if (pNode == NULL) {
    return NULL;
  }
  memset(pNode, 0, sizeof(NODE));

  pNode->bIsLeaf = (nLevel == nColorBits) ? TRUE : FALSE;
  if (pNode->bIsLeaf)
    (*pLeafCount)++;
  else {
    pNode->pNext = pReducibleNodes[nLevel];
    pReducibleNodes[nLevel] = pNode;
  }
  return pNode;
}

static void CQuantizer_ReduceTree(uint32_t nColorBits, uint32_t* pLeafCount,
                                  NODE** pReducibleNodes) {
  //
  // Find the deepest level containing at least one reducible node.
  //
  int i;
  for (i = nColorBits - 1; (i > 0) && (pReducibleNodes[i] == NULL); i--);

  //
  // Reduce the node most recently added to the list at level i.
  //
  NODE* pNode = pReducibleNodes[i];
  pReducibleNodes[i] = pNode->pNext;

  uint32_t nRedSum = 0;
  uint32_t nGreenSum = 0;
  uint32_t nBlueSum = 0;
  uint32_t nChildren = 0;

  for (i = 0; i < 8; i++) {
    if (pNode->pChild[i] != NULL) {
      nRedSum += pNode->pChild[i]->nRedSum;
      nGreenSum += pNode->pChild[i]->nGreenSum;
      nBlueSum += pNode->pChild[i]->nBlueSum;
      pNode->nPixelCount += pNode->pChild[i]->nPixelCount;
      MemFree(pNode->pChild[i]);
      pNode->pChild[i] = NULL;
      nChildren++;
    }
  }

  pNode->bIsLeaf = TRUE;
  pNode->nRedSum = nRedSum;
  pNode->nGreenSum = nGreenSum;
  pNode->nBlueSum = nBlueSum;
  *pLeafCount -= (nChildren - 1);
}

static void CQuantizer_GetPaletteColors(NODE* pTree, RGBQUAD* prgb, uint32_t* pIndex) {
  if (pTree->bIsLeaf) {
    prgb[*pIndex].rgbRed = (uint8_t)((pTree->nRedSum) / (pTree->nPixelCount));
    prgb[*pIndex].rgbGreen = (uint8_t)((pTree->nGreenSum) / (pTree->nPixelCount));
    prgb[*pIndex].rgbBlue = (uint8_t)((pTree->nBlueSum) / (pTree->nPixelCount));
    prgb[*pIndex].rgbReserved = 0;
    (*pIndex)++;
  } else {
    for (int i = 0; i < 8; i++) {
      if (pTree->pChild[i] != NULL) CQuantizer_GetPaletteColors(pTree->pChild[i], prgb, pIndex);
    }
  }
}

static uint32_t CQuantizer_GetColorCount(struct CQuantizer* cq) { return cq->m_nLeafCount; }

static void CQuantizer_GetColorTable(struct CQuantizer* cq, RGBQUAD* prgb) {
  uint32_t nIndex = 0;
  CQuantizer_GetPaletteColors(cq->m_pTree, prgb, &nIndex);
}

///////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;

} RGBValues;

BOOLEAN QuantizeImage(uint8_t* pDest, uint8_t* pSrc, int16_t sWidth, int16_t sHeight,
                      struct SGPPaletteEntry* pPalette) {
  int16_t sNumColors;

  // FIRST CREATE PALETTE
  struct CQuantizer q;
  CQuantizer_CQuantizer(&q, 255, 6);

  CQuantizer_ProcessImage(&q, pSrc, sWidth, sHeight);

  sNumColors = CQuantizer_GetColorCount(&q);

  memset(pPalette, 0, sizeof(struct SGPPaletteEntry) * 256);

  CQuantizer_GetColorTable(&q, (RGBQUAD*)pPalette);

  // THEN MAP IMAGE TO PALETTE
  // OK, MAPIT!
  MapPalette(pDest, pSrc, sWidth, sHeight, sNumColors, pPalette);

  return (TRUE);
}

void MapPalette(uint8_t* pDest, uint8_t* pSrc, int16_t sWidth, int16_t sHeight, int16_t sNumColors,
                struct SGPPaletteEntry* pTable) {
  int32_t cX, cY, cnt, bBest;
  real dLowestDist;
  real dCubeDist;
  vector_3 vTableVal, vSrcVal, vDiffVal;
  uint8_t* pData;
  RGBValues* pRGBData;

  pRGBData = (RGBValues*)pSrc;

  for (cX = 0; cX < sWidth; cX++) {
    for (cY = 0; cY < sHeight; cY++) {
      // OK, FOR EACH PALETTE ENTRY, FIND CLOSEST
      bBest = 0;
      dLowestDist = (float)9999999;
      pData = &(pSrc[(cY * sWidth) + cX]);

      for (cnt = 0; cnt < sNumColors; cnt++) {
        vSrcVal.x = pRGBData[(cY * sWidth) + cX].r;
        vSrcVal.y = pRGBData[(cY * sWidth) + cX].g;
        vSrcVal.z = pRGBData[(cY * sWidth) + cX].b;

        vTableVal.x = pTable[cnt].peRed;
        vTableVal.y = pTable[cnt].peGreen;
        vTableVal.z = pTable[cnt].peBlue;

        // Get Dist
        vDiffVal = VSubtract(&vSrcVal, &vTableVal);

        // Get mag dist
        dCubeDist = VGetLength(&(vDiffVal));

        if (dCubeDist < dLowestDist) {
          dLowestDist = dCubeDist;
          bBest = cnt;
        }
      }

      // Now we have the lowest value
      // Set into dest
      pData = &(pDest[(cY * sWidth) + cX]);

      // Set!
      *pData = (uint8_t)bBest;
    }
  }
}
