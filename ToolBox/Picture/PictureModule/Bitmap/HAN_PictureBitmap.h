#ifndef HAN_PICTURE_BITMAP_H
#define HAN_PICTURE_BITMAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <windows.h>

#include "..\..\HAN_PictureDef.h"
#include "..\..\HAN_PictureLib.h"

#define HAN_PICTURE_BITMAP_CLASS                    TEXT("HAN_PictureBitmap")

BOOL CheckBitmapType(const uint8_t* pData, HANSIZE nLen);

void RegisterHANPictureBitmap(HINSTANCE hInst);

void SavePictureBitmap(HANPCSTR pFileName, PPICTURESAVEPARAM pSaveParam);

#ifdef __cplusplus
}
#endif

#endif
