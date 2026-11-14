#ifndef HAN_VIDEO_MP4_DECODER_MPEG4AUDIO_H
#define HAN_VIDEO_MP4_DECODER_MPEG4AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\..\HAN_VideoMP4DecoderDef.h"

BOOL GetVideoDecoderType_MPEG4Audio(const uint8_t* pData, HANSIZE nLen);

HANSIZE GetVideoDecoderSize_MPEG4Audio(const uint8_t* pData, HANSIZE nLen, PVIDEOMP4DECODERMEDIAINFO pMediaInfo);

HANSIZE InitVideoDecoder_MPEG4Audio(const uint8_t* pData, HANSIZE nLen, uint8_t* pDecoder, HANSIZE nDecoderSize);

HANPSTR GetBoxName_MPEG4Audio(VIDEOMP4BOXTYPE boxType);

HANSIZE GetSubBoxOffset_MPEG4Audio(VIDEOMP4BOXTYPE boxType);

UPDATEBOXINFOWINDOWCALLBACK GetUpdateBoxInfoWindowCallback_MPEG4Audio(VIDEOMP4BOXTYPE boxType);

void UpdateBoxInfoWindowSample_MPEG4Audio(const uint8_t* pData, HANSIZE nLen, HWND hWindow, uint8_t* pDecoder);

void DecodeMedia_MPEG4Audio(const uint8_t* pData, HANSIZE nLen, uint8_t* pDecoder);

#ifdef __cplusplus
}
#endif

#endif
