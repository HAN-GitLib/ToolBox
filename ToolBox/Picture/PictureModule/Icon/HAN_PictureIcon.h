#ifndef HAN_PICTURE_ICON_H
#define HAN_PICTURE_ICON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

#include "..\..\HAN_PictureDef.h"
#include "HAN_PictureIconDef.h"

#define HAN_PICTURE_ICON_CLASS              TEXT("HAN_PictureIcon")

BOOL CheckPathHaveIcons(HINSTANCE hInst, HANPCSTR pPath);

void RegisterHANPictureIcon(HINSTANCE hInst);

#ifdef __cplusplus
}
#endif

#endif
