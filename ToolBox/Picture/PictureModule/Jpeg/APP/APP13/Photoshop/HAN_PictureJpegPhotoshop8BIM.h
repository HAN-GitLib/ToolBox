#ifndef HAN_PICTURE_JPEG_APP13_PHOTOSHOP_8BIM_H
#define HAN_PICTURE_JPEG_APP13_PHOTOSHOP_8BIM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\..\..\HAN_PictureJpegDef.h"

HANPSTR GetJpeg_APP13_PhotoshopResolutionUnitName(uint16_t cUnit);
HANPSTR GetJpeg_APP13_PhotoshopCopyRightFlagName(uint8_t cFlag);
HANPSTR GetJpeg_APP13_PhotoshopPrintScaleInfoPrintStyleName(uint16_t cStyle);
HANPSTR GetJpeg_APP13_PhotoshopThumbnailFormatName(uint32_t nFormat);

#ifdef __cplusplus
}
#endif

#endif
