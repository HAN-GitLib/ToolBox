#ifndef HAN_PICTURE_BITMAP_PAINT_H
#define HAN_PICTURE_BITMAP_PAINT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\..\HAN_PictureDef.h"
#include "HAN_PictureBitmapDef.h"

#define HAN_PICTURE_BITMAP_PAINT_CLASS              TEXT("HAN_PictureBitmapPaint")

#define PICTURE_BITMAP_ALPHA_BACKGROUND_W0          (7)
#define PICTURE_BITMAP_ALPHA_BACKGROUND_W2          (1)
#define PICTURE_BITMAP_ALPHA_BACKGROUND_W           (PICTURE_BITMAP_ALPHA_BACKGROUND_W0 + PICTURE_BITMAP_ALPHA_BACKGROUND_W2)

void RegisterHANPictureBitmapPaint(HINSTANCE hInst);

#ifdef __cplusplus
}
#endif

#endif
