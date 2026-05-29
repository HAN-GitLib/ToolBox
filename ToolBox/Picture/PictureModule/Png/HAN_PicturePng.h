#ifndef HAN_PICTURE_PNG_H
#define HAN_PICTURE_PNG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <Windows.h>

#include "..\..\HAN_PictureDef.h"
#include "HAN_PicturePngDef.h"

#define HAN_PICTURE_PNG_CLASS                       TEXT("HAN_PicturePng")

BOOL CheckPngType(const uint8_t* pData, HANSIZE nLen);

void RegisterHANPicturePng(HINSTANCE hInst);

void SavePicturePng(HANPCSTR pFileName, PPICTURESAVEPARAM pSaveParam);

#ifdef __cplusplus
}
#endif

#endif
