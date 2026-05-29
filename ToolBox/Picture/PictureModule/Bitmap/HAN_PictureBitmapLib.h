#ifndef HAN_PICTURE_BITMAP_LIB_H
#define HAN_PICTURE_BITMAP_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

#include "..\..\HAN_PictureDef.h"
#include "..\..\HAN_PictureLib.h"

typedef struct tagPICTUREBITMAPINFO {
    BITMAPINFOHEADER    bmiHeader;
    union {
        const uint8_t*  pData;
        const uint32_t* pBitFileds;
        PCPICTURERGBA   pColors;
        RGBQUAD*        pRGBQuad;
    } colorTable;
    union {
        const uint8_t*  pData;
        const uint16_t* p16;
        PCPICTURERGB    pRGB;
        PCPICTURERGBA   pRGBA;
    } pixelData;
} PICTUREBITMAPINFO, * PPICTUREBITMAPINFO;
typedef const PICTUREBITMAPINFO* PCPICTUREBITMAPINFO;

void GetBitmapPixelSizeName(WORD nSize, HANPSTR pText, HANSIZE nTextSize);

void CopyBitmapToPictureBitmapInfo(PPICTUREBITMAPINFO pInfo, const BITMAP* pBitmap);

BOOL DecodeBitmapColor(HANPPICTURE pPicture, PCPICTUREBITMAPINFO pInfo, BOOL* pAlpha);
BOOL DecodeBitmapMask(HANPPICTURE pPicture, PCPICTUREBITMAPINFO pInfo);

#ifdef __cplusplus
}
#endif

#endif
