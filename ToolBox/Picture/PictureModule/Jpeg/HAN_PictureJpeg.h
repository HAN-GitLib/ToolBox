#ifndef HAN_PICTURE_JPEG_H
#define HAN_PICTURE_JPEG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <Windows.h>

#include "..\..\HAN_PictureDef.h"
#include "HAN_PictureJpegDef.h"

#define HAN_PICTURE_JPEG_CLASS                      TEXT("HAN_PictureJpeg")

BOOL CheckJpegType(const uint8_t* pData, HANSIZE nLen);

void RegisterHANPictureJpeg(HINSTANCE hInst);

void SavePictureJpeg(HANPCSTR pFileName, PPICTURESAVEPARAM pSaveParam);

#ifdef __cplusplus
}
#endif

#endif
