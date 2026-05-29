#ifndef HAN_PICTURE_LIB_H
#define HAN_PICTURE_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "HAN_PictureDef.h"

SIZE_T GetPictureInfoMemSize(HANSIZE nCnt);

SIZE_T GetPictureMemSize(PPICTURERESOLUTION pResolution);

void UpdatePictureMap(HANPPICTURE pPicture, PPICTURERESOLUTION pResolution);

PICTURERGBA BlendRGBA(PICTURERGBA rgbaFore, PICTURERGBA rgbaBack);

#ifdef __cplusplus
}
#endif

#endif
