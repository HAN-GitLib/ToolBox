#ifndef HAN_PICTURE_JPEG_APP2_ICC_PROFILE_H
#define HAN_PICTURE_JPEG_APP2_ICC_PROFILE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\..\..\HAN_PictureJpegDef.h"
#include "..\..\..\HAN_PictureJpegSegment.h"

BOOL UpdateSegmentInfoWindow_APP2_ReadICCProfile(PCPICTUREJPEGSEGMENT pSegment, HWND hListView);

#ifdef __cplusplus
}
#endif

#endif
