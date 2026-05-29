#ifndef HAN_PICTURE_JPEG_APP1_EXIF_H
#define HAN_PICTURE_JPEG_APP1_EXIF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\..\..\HAN_PictureJpegDef.h"
#include "..\..\..\HAN_PictureJpegSegment.h"

BOOL UpdateSegmentInfoWindow_APP1_ReadExif(PCPICTUREJPEGSEGMENT pSegment, HWND hListView);

#ifdef __cplusplus
}
#endif

#endif
