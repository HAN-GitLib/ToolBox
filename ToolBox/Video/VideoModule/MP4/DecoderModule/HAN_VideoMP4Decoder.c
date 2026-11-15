#include "..\HAN_VideoMP4Def.h"
#include "HAN_VideoMP4DecoderDef.h"

#include "Video\H264\HAN_VideoMP4Decoder_H264.h"
#include "Audio\MPEG4Audio\HAN_VideoMP4Decoder_MPEG4Audio.h"

static const VIDEOMP4DECODERHANDLER sg_pDecoderInfo[VIDEO_MP4_DECODER_TYPE_CNT] = {
    [VIDEO_MP4_DECODER_TYPE_H264] = {
        .GetVideoDecoderType = GetVideoDecoderType_H264,
        .GetVideoDecoderSize = GetVideoDecoderSize_H264,
        .InitVideoDecoder = InitVideoDecoder_H264,
        .GetBoxName = GetBoxName_H264,
        .GetSubBoxOffset = GetSubBoxOffset_H264,
        .GetUpdateBoxInfoWindowCallback = GetUpdateBoxInfoWindowCallback_H264,
        .UpdateBoxInfoWindowDecoderSample = UpdateBoxInfoWindowDecoderSample_H264,
        .DecodeMedia = DecodeMedia_H264,
    },
    [VIDEO_MP4_DECODER_TYPE_MPEG4AUDIO] = {
        .GetVideoDecoderType = GetVideoDecoderType_MPEG4Audio,
        .GetVideoDecoderSize = GetVideoDecoderSize_MPEG4Audio,
        .InitVideoDecoder = InitVideoDecoder_MPEG4Audio,
        .GetBoxName = GetBoxName_MPEG4Audio,
        .GetSubBoxOffset = GetSubBoxOffset_MPEG4Audio,
        .GetUpdateBoxInfoWindowCallback = GetUpdateBoxInfoWindowCallback_MPEG4Audio,
        .UpdateBoxInfoWindowDecoderSample = UpdateBoxInfoWindowDecoderSample_MPEG4Audio,
        .DecodeMedia = DecodeMedia_MPEG4Audio,
    },
};

VIDEOMP4DECODERTYPE GetVideoDecoderType(const uint8_t* pData, HANSIZE nLen)
{
    VIDEOMP4DECODERTYPE decoderType = VIDEO_MP4_DECODER_TYPE_CNT;

    /* 确认解码器类型 */
    for (decoderType = 0; decoderType < VIDEO_MP4_DECODER_TYPE_CNT; decoderType++)
    {
        if (TRUE == sg_pDecoderInfo[decoderType].GetVideoDecoderType(pData, nLen)) { break; }
    }

    return decoderType;
}

HANSIZE GetVideoDecoderSize(const uint8_t* pData, HANSIZE nLen, VIDEOMP4DECODERTYPE decoderType, PVIDEOMP4DECODERMEDIAINFO pMediaInfo)
{
    HANSIZE nRet = 0;

    if ((0 <= decoderType) && (decoderType < VIDEO_MP4_DECODER_TYPE_CNT))
    {
        nRet = sg_pDecoderInfo[decoderType].GetVideoDecoderSize(pData, nLen, pMediaInfo);
    }

    return nRet;
}

HANSIZE InitVideoDecoder(const uint8_t* pData, HANSIZE nLen, VIDEOMP4DECODERTYPE decoderType, void* pDecoder, HANSIZE nSize)
{
    HANSIZE nRet = 0;

    if ((0 <= decoderType) && (decoderType < VIDEO_MP4_DECODER_TYPE_CNT))
    {
        nRet = sg_pDecoderInfo[decoderType].InitVideoDecoder(pData, nLen, pDecoder, nSize);
    }

    return nRet;
}

HANPSTR GetDecoderBoxName(VIDEOMP4DECODERTYPE decoderType, VIDEOMP4BOXTYPE boxType)
{
    HANPSTR pRet = 0;

    if ((0 <= decoderType) && (decoderType < VIDEO_MP4_DECODER_TYPE_CNT))
    {
        pRet = sg_pDecoderInfo[decoderType].GetBoxName(boxType);
    }

    return pRet;
}

HANSIZE GetSubBoxOffset(VIDEOMP4DECODERTYPE decoderType, VIDEOMP4BOXTYPE boxType)
{
    HANSIZE nRet = 0;

    if ((0 <= decoderType) && (decoderType < VIDEO_MP4_DECODER_TYPE_CNT))
    {
        nRet = sg_pDecoderInfo[decoderType].GetSubBoxOffset(boxType);
    }

    return nRet;
}

UPDATEBOXINFOWINDOWCALLBACK GetUpdateBoxInfoWindowCallback(VIDEOMP4DECODERTYPE decoderType, VIDEOMP4BOXTYPE boxType)
{
    UPDATEBOXINFOWINDOWCALLBACK pRet = NULL;

    if ((0 <= decoderType) && (decoderType < VIDEO_MP4_DECODER_TYPE_CNT))
    {
        pRet = sg_pDecoderInfo[decoderType].GetUpdateBoxInfoWindowCallback(boxType);
    }

    return pRet;
}

void UpdateBoxInfoWindowDecoderSample(const uint8_t* pData, HANSIZE nLen, VIDEOMP4DECODERTYPE decoderType, HWND hWindow, uint8_t* pDecoder)
{
    if ((0 <= decoderType) && (decoderType < VIDEO_MP4_DECODER_TYPE_CNT))
    {
        sg_pDecoderInfo[decoderType].UpdateBoxInfoWindowDecoderSample(pData, nLen, hWindow, pDecoder);
    }
}

void DecodeMedia(const uint8_t* pData, HANSIZE nLen, VIDEOMP4DECODERTYPE decoderType, uint8_t* pDecoder)
{
    if ((0 <= decoderType) && (decoderType < VIDEO_MP4_DECODER_TYPE_CNT))
    {
        sg_pDecoderInfo[decoderType].DecodeMedia(pData, nLen, pDecoder);
    }
}
