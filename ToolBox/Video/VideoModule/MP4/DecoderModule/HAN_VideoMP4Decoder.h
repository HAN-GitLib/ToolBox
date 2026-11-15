#ifndef HAN_VIDEO_MP4_DECODER_H
#define HAN_VIDEO_MP4_DECODER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\HAN_VideoMP4Def.h"
#include "..\HAN_VideoMP4Box.h"
#include "HAN_VideoMP4DecoderDef.h"

VIDEOMP4DECODERTYPE GetVideoDecoderType(const uint8_t* pData, HANSIZE nLen);

HANSIZE GetVideoDecoderSize(const uint8_t* pData, HANSIZE nLen, VIDEOMP4DECODERTYPE decoderType, PVIDEOMP4DECODERMEDIAINFO pMediaInfo);

HANSIZE InitVideoDecoder(const uint8_t* pData, HANSIZE nLen, VIDEOMP4DECODERTYPE decoderType, void* pDecoder, HANSIZE nSize);

HANPSTR GetDecoderBoxName(VIDEOMP4DECODERTYPE decoderType, VIDEOMP4BOXTYPE boxType);

HANSIZE GetSubBoxOffset(VIDEOMP4DECODERTYPE decoderType, VIDEOMP4BOXTYPE boxType);

UPDATEBOXINFOWINDOWCALLBACK GetUpdateBoxInfoWindowCallback(VIDEOMP4DECODERTYPE decoderType, VIDEOMP4BOXTYPE boxType);

void UpdateBoxInfoWindowDecoderSample(const uint8_t* pData, HANSIZE nLen, VIDEOMP4DECODERTYPE decoderType, HWND hWindow, uint8_t* pDecoder);

void DecodeMedia(const uint8_t* pData, HANSIZE nLen, VIDEOMP4DECODERTYPE decoderType, uint8_t* pDecoder);

#ifdef __cplusplus
}
#endif

#endif
