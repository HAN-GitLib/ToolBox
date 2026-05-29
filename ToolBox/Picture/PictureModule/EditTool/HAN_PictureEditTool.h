#ifndef HAN_PICTURE_EDIT_TOOL_H
#define HAN_PICTURE_EDIT_TOOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Windows.h>

#include "..\..\HAN_PictureDef.h"
#include "HAN_PictureEditToolDef.h"

#define HAN_PICTURE_EDIT_TOOL_CLASS                 TEXT("HAN_PictureEditTool")

void RegisterHANPictureEditTool(HINSTANCE hInst, HBRUSH hBackground);

BOOL PictureEditToolSetPaintId(HWND hEditTool, HANSIZE nId, BOOL bRedraw);

HANSIZE PictureEditToolGetPaintId(HWND hEditTool);

#ifdef __cplusplus
}
#endif

#endif
