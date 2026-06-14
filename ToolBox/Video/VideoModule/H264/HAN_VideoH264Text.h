#ifndef HAN_VIDEO_H264_TEXT_H
#define HAN_VIDEO_H264_TEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "HAN_VideoH264Def.h"

HANPSTR GetH264_SPS_Name(void);
HANPSTR GetH264_SPS_FieldName(VIDEOH264FIELDSPS eName);
HANPSTR GetH264_SPS_ProfileIdcName(VIDEOH264PROFILETYPE eProfileType);
HANPSTR GetH264_SPS_ChromaFormatName(uint8_t nChroma);

HANPSTR GetH264_VUIParam_Name(void);
HANPSTR GetH264_VUIParam_FieldName(VIDEOH264FIELDVUIPARAM eName);

#ifdef __cplusplus
}
#endif

#endif
