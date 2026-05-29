#ifndef HAN_PICTURE_JPEG_APP0_JFIF_H
#define HAN_PICTURE_JPEG_APP0_JFIF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\..\..\HAN_PictureJpegDef.h"
#include "..\..\..\HAN_PictureJpegSegment.h"

BOOL UpdateSegmentInfoWindow_APP0_ReadJFIF(PCPICTUREJPEGSEGMENT pSegment, HWND hListView);

#ifdef __cplusplus
}
#endif

#endif
