#include "HAN_VideoMP4Decoder_MPEG4Audio.h"

#define VIDEO_MP4_MPEG4AUDIO_BOX_NAME_mp4a      TEXT("MPEG-4 Audio编码器入口")
#define VIDEO_MP4_MPEG4AUDIO_BOX_NAME_esds      TEXT("元素流描述符")

#define VIDEO_MP4_MPEG4AUDIO_BOX_OFFSET_mp4a    28

static inline uint16_t ReadMP4BoxDecoderMPEG4Audio2ByteMSB(const uint8_t pData[2]);
static inline uint32_t ReadMP4BoxDecoderMPEG4Audio4ByteMSB(const uint8_t pData[4]);

static void DecodeBoxData_mp4a(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_mp4a pmp4a);

static void UpdateBoxInfoWindow_mp4a(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam);

BOOL GetVideoDecoderType_MPEG4Audio(const uint8_t* pData, HANSIZE nLen)
{
    (void)nLen;
    BOOL bRet;

    if (VIDEO_MP4_BOX_TYPE_mp4a == GetMP4BoxType(&pData[4])) { bRet = TRUE; }
    else { bRet = FALSE; }

    return bRet;
}

HANSIZE GetVideoDecoderSize_MPEG4Audio(const uint8_t* pData, HANSIZE nLen, PVIDEOMP4DECODERMEDIAINFO pMediaInfo)
{
    (void)pData;
    (void)nLen;
    (void)pMediaInfo;

    return 0;
}

HANSIZE InitVideoDecoder_MPEG4Audio(const uint8_t* pData, HANSIZE nLen, uint8_t* pDecoder, HANSIZE nDecoderSize)
{
    (void)pData;
    (void)nLen;
    (void)pDecoder;
    (void)nDecoderSize;

    return 0;
}

HANPSTR GetBoxName_MPEG4Audio(VIDEOMP4BOXTYPE boxType)
{
    HANPSTR pRet;

    switch (boxType) {
        case VIDEO_MP4_BOX_TYPE_mp4a: { pRet = VIDEO_MP4_MPEG4AUDIO_BOX_NAME_mp4a; } break;
        default: { pRet = NULL; } break;
    }

    return pRet;
}

HANSIZE GetSubBoxOffset_MPEG4Audio(VIDEOMP4BOXTYPE boxType)
{
    HANSIZE nRet;

    switch (boxType) {
        case VIDEO_MP4_BOX_TYPE_mp4a: { nRet = VIDEO_MP4_MPEG4AUDIO_BOX_OFFSET_mp4a; } break;
        default: { nRet = HAN_VIDEO_MP4_INVALID_SIZE; } break;
    }

    return nRet;
}

UPDATEBOXINFOWINDOWCALLBACK GetUpdateBoxInfoWindowCallback_MPEG4Audio(VIDEOMP4BOXTYPE boxType)
{
    UPDATEBOXINFOWINDOWCALLBACK pRet;

    switch (boxType) {
        case VIDEO_MP4_BOX_TYPE_mp4a: { pRet = UpdateBoxInfoWindow_mp4a; } break;
        default: { pRet = NULL; } break;
    }

    return pRet;
}

void UpdateBoxInfoWindowDecoderSample_MPEG4Audio(const uint8_t* pData, HANSIZE nLen, HWND hWindow, uint8_t* pDecoder)
{
    (void)pData;
    (void)nLen;
    (void)hWindow;
    (void)pDecoder;
}

void DecodeMedia_MPEG4Audio(const uint8_t* pData, HANSIZE nLen, uint8_t* pDecoder)
{
    (void)pData;
    (void)nLen;
    (void)pDecoder;
}

static inline uint16_t ReadMP4BoxDecoderMPEG4Audio2ByteMSB(const uint8_t pData[2])
{
    return (((uint16_t)pData[0] << 8) + (uint16_t)pData[1]);
}
static inline uint32_t ReadMP4BoxDecoderMPEG4Audio4ByteMSB(const uint8_t pData[4])
{
    return (((uint32_t)pData[0] << 24) + ((uint32_t)pData[1] << 16) + ((uint32_t)pData[2] << 8) + (uint32_t)pData[3]);
}

static void DecodeBoxData_mp4a(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_mp4a pmp4a)
{
    const uint8_t* pData = pBox->pData;
    HANSIZE nOffset = 6;
    uint32_t nSampleRate;
    
    pmp4a->pRawData = pData;
    pmp4a->nId = ReadMP4BoxDecoderMPEG4Audio2ByteMSB(&pData[nOffset]); nOffset += 2;
    pmp4a->nVersion = ReadMP4BoxDecoderMPEG4Audio2ByteMSB(&pData[nOffset]); nOffset += 2;
    pmp4a->nRevision = ReadMP4BoxDecoderMPEG4Audio2ByteMSB(&pData[nOffset]); nOffset += 2;
    pmp4a->nVendor = ReadMP4BoxDecoderMPEG4Audio4ByteMSB(&pData[nOffset]); nOffset += 4;
    pmp4a->nChannelCnt = ReadMP4BoxDecoderMPEG4Audio2ByteMSB(&pData[nOffset]); nOffset += 2;
    pmp4a->nSampleSize = ReadMP4BoxDecoderMPEG4Audio2ByteMSB(&pData[nOffset]); nOffset += 2;
    pmp4a->nCompressionId = ReadMP4BoxDecoderMPEG4Audio2ByteMSB(&pData[nOffset]); nOffset += 2;
    pmp4a->nPacketSize = ReadMP4BoxDecoderMPEG4Audio2ByteMSB(&pData[nOffset]); nOffset += 2;
    nSampleRate = ReadMP4BoxDecoderMPEG4Audio4ByteMSB(&pData[nOffset]); nOffset += 4;
    pmp4a->nSampleRate = (HANDOUBLE)nSampleRate / (HANDOUBLE)0x10000;
}

static void UpdateBoxInfoWindow_mp4a(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam)
{
    (void)pParam;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_mp4a mp4aInfo;

    DecodeBoxData_mp4a(&(pBoxTree->mp4Box), &mp4aInfo);
    /* ID */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), mp4aInfo.nId);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mp4a_FieldName(VIDEO_MP4_mp4a_BOX_FIELD_ID), pText, nId, hListView);
    /* 版本 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), mp4aInfo.nVersion);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mp4a_FieldName(VIDEO_MP4_mp4a_BOX_FIELD_VERSION), pText, nId, hListView);
    /* 修订 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), mp4aInfo.nRevision);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mp4a_FieldName(VIDEO_MP4_mp4a_BOX_FIELD_REVISION), pText, nId, hListView);
    /* 供应商 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), mp4aInfo.nVendor);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mp4a_FieldName(VIDEO_MP4_mp4a_BOX_FIELD_VENDOR), pText, nId, hListView);
    /* 声道数 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u（%s）"), mp4aInfo.nChannelCnt, GetMP4_mp4a_ChannelCountName(mp4aInfo.nChannelCnt));
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mp4a_FieldName(VIDEO_MP4_mp4a_BOX_FIELD_CHANNEL_COUNT), pText, nId, hListView);
    /* 样本大小 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), mp4aInfo.nSampleSize);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mp4a_FieldName(VIDEO_MP4_mp4a_BOX_FIELD_SAMPLE_SIZE), pText, nId, hListView);
    /* 压缩ID */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), mp4aInfo.nCompressionId);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mp4a_FieldName(VIDEO_MP4_mp4a_BOX_FIELD_COMPRESSION_ID), pText, nId, hListView);
    /* 数据包大小 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), mp4aInfo.nPacketSize);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mp4a_FieldName(VIDEO_MP4_mp4a_BOX_FIELD_PACKET_SIZE), pText, nId, hListView);
    /* 采样率 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%g Hz"), mp4aInfo.nSampleRate);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mp4a_FieldName(VIDEO_MP4_mp4a_BOX_FIELD_SAMPLE_RATE), pText, nId, hListView);
}
