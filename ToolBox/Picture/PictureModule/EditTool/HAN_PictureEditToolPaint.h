#ifndef HAN_PICTURE_EDIT_TOOL_PAINT_H
#define HAN_PICTURE_EDIT_TOOL_PAINT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Windows.h>

#include "..\..\HAN_PictureDef.h"
#include "HAN_PictureEditToolDef.h"

#define HAN_PICTURE_EDIT_TOOL_PAINT_CLASS           TEXT("HAN_PictureEditToolPaint")

void RegisterHANPictureEditToolPaint(HINSTANCE hInst, HBRUSH hBackground);

void PictureEditToolPaintSetPaintId(HWND hEditToolPaint, HANSIZE nId);
HANSIZE PictureEditToolPaintGetPaintId(HWND hEditToolPaint);

#ifdef __cplusplus
}
#endif

#endif
