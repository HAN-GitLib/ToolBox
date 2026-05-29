#ifndef HAN_PICTURE_JPEG_LIB_H
#define HAN_PICTURE_JPEG_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAN_PictureJpegDef.h"

void PictureJpegPrintHexData(HANPSTR pText, HANSIZE nTextLen, const uint8_t* pData, HANSIZE nLen);

#ifdef __cplusplus
}
#endif

#endif
