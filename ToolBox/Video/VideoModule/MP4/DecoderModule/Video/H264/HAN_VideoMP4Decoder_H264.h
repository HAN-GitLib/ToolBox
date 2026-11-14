#ifndef HAN_VIDEO_MP4_DECODER_H264_H
#define HAN_VIDEO_MP4_DECODER_H264_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\..\HAN_VideoMP4DecoderDef.h"

BOOL GetVideoDecoderType_H264(const uint8_t* pData, HANSIZE nLen);

HANSIZE GetVideoDecoderSize_H264(const uint8_t* pData, HANSIZE nLen, PVIDEOMP4DECODERMEDIAINFO pMediaInfo);

HANSIZE InitVideoDecoder_H264(const uint8_t* pData, HANSIZE nLen, uint8_t* pDecoder, HANSIZE nDecoderSize);

HANPSTR GetBoxName_H264(VIDEOMP4BOXTYPE boxType);

HANSIZE GetSubBoxOffset_H264(VIDEOMP4BOXTYPE boxType);

UPDATEBOXINFOWINDOWCALLBACK GetUpdateBoxInfoWindowCallback_H264(VIDEOMP4BOXTYPE boxType);

void UpdateBoxInfoWindowSample_H264(const uint8_t* pData, HANSIZE nLen, HWND hWindow, uint8_t* pDecoder);

void DecodeMedia_H264(const uint8_t* pData, HANSIZE nLen, uint8_t* pDecoder);

#ifdef __cplusplus
}
#endif

#endif
