#ifndef HAN_VIDEO_MP4_H
#define HAN_VIDEO_MP4_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <Windows.h>

#include "..\..\HAN_VideoDef.h"
#include "HAN_VideoMP4Def.h"

#define HAN_VIDEO_MP4_CLASS                         TEXT("HAN_VideoMP4")

BOOL CheckMP4Type(const uint8_t* pData, HANSIZE nLen);

void RegisterHANVideoMP4(HINSTANCE hInst);

#ifdef __cplusplus
}
#endif

#endif
