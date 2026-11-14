#include "HAN_VideoMP4Decoder_H264.h"
#include "..\..\..\HAN_VideoMP4Def.h"
#include "..\..\..\..\H264\HAN_VideoH264.h"

#define VIDEO_MP4_H264_BOX_NAME_avc1        TEXT("H.264解码器入口")
#define VIDEO_MP4_H264_BOX_NAME_pasp        TEXT("像素宽高比")
#define VIDEO_MP4_H264_BOX_NAME_avcC        TEXT("解码器配置信息")
#define VIDEO_MP4_H264_BOX_NAME_btrt        TEXT("比特率信息")

#define VIDEO_MP4_H264_BOX_OFFSET_avc1      78

typedef struct tagVIDEOMP4DECODERH264 {
    VIDEOH264PICTUREINFO    H264;
    struct {
        HANSIZE             nNum;
        PVIDEOH264SPS       pSPS;
    } sps;
    struct {
        HANSIZE             nNum;
        PVIDEOH264PPS       pPPS;
    } pps;
    HANSIZE                 nSize;
    uint8_t                 pBuf[];
} VIDEOMP4DECODERH264, * PVIDEOMP4DECODERH264;

typedef struct tagVIDEOMP4DECODERH264SIZE {
    HANSIZE                 nSPSSize;
    HANSIZE                 nPPSSize;
    HANSIZE                 nSliceGroupMapSize;
    HANSIZE                 nMacroBlockSize;
} VIDEOMP4DECODERH264SIZE, * PVIDEOMP4DECODERH264SIZE;

typedef struct tagVIDEOMP4DECODERH264INIT {
    PVIDEOMP4DECODERH264    pDecoder;
    HANSIZE                 nSize;
} VIDEOMP4DECODERH264INIT, * PVIDEOMP4DECODERH264INIT;

static inline uint16_t ReadMP4BoxDecoderH2642ByteMSB(const uint8_t pData[2]);
static inline uint32_t ReadMP4BoxDecoderH2644ByteMSB(const uint8_t pData[4]);

static void ReadMP4BoxCallback_GetH264Size(PCVIDEOMP4BOX pBox, void* pParam);
static void ReadMP4BoxCallback_InitH264Decoder(PCVIDEOMP4BOX pBox, void* pParam);

static void UpdateH264Size_avcC(PVIDEOMP4BOXINFO_avcC pavcC, PVIDEOMP4DECODERH264SIZE pSize);

static HANSIZE UpdateH264Decoder_avcC(PVIDEOMP4BOXINFO_avcC pavcC, PVIDEOMP4DECODERH264INIT pInitDecoder);

static void DecodeBoxData_avc1(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_avc1 pavc1);
static void DecodeBoxData_avcC(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_avcC pavcC);

static void DecodeBoxData_avcC_SPS(const uint8_t* pData, HANSIZE nLen, PVIDEOH264SPS pSPS);
static void DecodeBoxData_avcC_PPS(const uint8_t* pData, HANSIZE nLen, PVIDEOH264PPS pPPS);

static void UpdateBoxInfoWindow_pasp(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam);
static void UpdateBoxInfoWindow_avc1(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam);
static void UpdateBoxInfoWindow_avcC(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam);
static void UpdateBoxInfoWindow_btrt(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam);

static HANINT UpdateH264InfoWindow_SPS(PCVIDEOH264SPS pSPS, HWND hListView, HANINT nStartId);
static HANINT UpdateH264InfoWindow_VUIParam(PCVIDEOH264PARAMETER_vui_parameters pVUI, HWND hListView, HANINT nStartId);
static HANINT UpdateH264InfoWindow_HRDParam(PCVIDEOH264PARAMETER_hrd_parameters pHRD, HANPCSTR pHeader, HWND hListView, HANINT nStartId);

static HANINT UpdateH264InfoWindow_PPS(PCVIDEOH264PPS pPPS, HWND hListView, HANINT nStartId);

static HANINT UpdateH264InfoWindow_Slice(PCVIDEOH264SLICE pSlice, HWND hListView, HANINT nStartId);
static HANINT UpdateH264InfoWindow_RefPicListMVCModification(PCVIDEOH264PARAMETER_ref_pic_list_mvc pRefPicListMVC, uint32_t sliceType, HWND hListView, HANINT nStartId);
static HANINT UpdateH264InfoWindow_RefPicListModification(PCVIDEOH264PARAMETER_ref_pic_list pRefPicList, uint32_t sliceType, HWND hListView, HANINT nStartId);
static HANINT UpdateH264InfoWindow_DecRefPicMarking(PCVIDEOH264PARAMETER_dec_ref_pic_marking pDecRefPicMarking, uint8_t idrPicFlag, HWND hListView, HANINT nStartId);

BOOL GetVideoDecoderType_H264(const uint8_t* pData, HANSIZE nLen)
{
    (void)nLen;
    BOOL bRet;

    if (VIDEO_MP4_BOX_TYPE_avc1 == GetMP4BoxType(&pData[4])) { bRet = TRUE; }
    else { bRet = FALSE; }

    return bRet;
}

HANSIZE GetVideoDecoderSize_H264(const uint8_t* pData, HANSIZE nLen, PVIDEOMP4DECODERMEDIAINFO pMediaInfo)
{
    (void)pMediaInfo;
    HANSIZE nRet;
    VIDEOMP4DECODERH264SIZE sizeDecoder;
    HANSIZE nBoxSize;

    memset(&sizeDecoder, 0, sizeof(sizeDecoder));
    nBoxSize = ReadMP4Boxes(pData, nLen, ReadMP4BoxCallback_GetH264Size, &sizeDecoder);
    if (0 < nBoxSize)
    {
        nRet = sizeof(VIDEOMP4DECODERH264);
        nRet += sizeDecoder.nSPSSize;
        nRet += sizeDecoder.nPPSSize;
        nRet += sizeDecoder.nSliceGroupMapSize;
        nRet += sizeDecoder.nMacroBlockSize;
    }
    else { nRet = 0; }
    
    return nRet;
}

HANSIZE InitVideoDecoder_H264(const uint8_t* pData, HANSIZE nLen, uint8_t* pDecoder, HANSIZE nDecoderSize)
{
    VIDEOMP4DECODERH264INIT initDecoder = {
        .pDecoder = (PVIDEOMP4DECODERH264)pDecoder,
        .nSize = nDecoderSize,
    };
    
    initDecoder.pDecoder->nSize = 0;
    (void)ReadMP4Boxes(pData, nLen, ReadMP4BoxCallback_InitH264Decoder, &initDecoder);

    return initDecoder.pDecoder->nSize;
}

HANPSTR GetBoxName_H264(VIDEOMP4BOXTYPE boxType)
{
    HANPSTR pRet;

    switch (boxType) {
        case VIDEO_MP4_BOX_TYPE_avc1: { pRet = VIDEO_MP4_H264_BOX_NAME_avc1; } break;
        case VIDEO_MP4_BOX_TYPE_pasp: { pRet = VIDEO_MP4_H264_BOX_NAME_pasp; } break;
        case VIDEO_MP4_BOX_TYPE_avcC: { pRet = VIDEO_MP4_H264_BOX_NAME_avcC; } break;
        case VIDEO_MP4_BOX_TYPE_btrt: { pRet = VIDEO_MP4_H264_BOX_NAME_btrt; } break;
        default: { pRet = NULL; } break;
    }

    return pRet;
}

HANSIZE GetSubBoxOffset_H264(VIDEOMP4BOXTYPE boxType)
{
    HANSIZE nRet;

    switch (boxType) {
        case VIDEO_MP4_BOX_TYPE_avc1: { nRet = VIDEO_MP4_H264_BOX_OFFSET_avc1; } break;
        default: { nRet = HAN_VIDEO_MP4_INVALID_SIZE; } break;
    }

    return nRet;
}

UPDATEBOXINFOWINDOWCALLBACK GetUpdateBoxInfoWindowCallback_H264(VIDEOMP4BOXTYPE boxType)
{
    UPDATEBOXINFOWINDOWCALLBACK pRet;

    switch (boxType) {
        case VIDEO_MP4_BOX_TYPE_avc1: { pRet = UpdateBoxInfoWindow_avc1; } break;
        case VIDEO_MP4_BOX_TYPE_pasp: { pRet = UpdateBoxInfoWindow_pasp; } break;
        case VIDEO_MP4_BOX_TYPE_avcC: { pRet = UpdateBoxInfoWindow_avcC; } break;
        case VIDEO_MP4_BOX_TYPE_btrt: { pRet = UpdateBoxInfoWindow_btrt; } break;
        default: { pRet = NULL; } break;
    }

    return pRet;
}

void UpdateBoxInfoWindowSample_H264(const uint8_t* pData, HANSIZE nLen, HWND hWindow, uint8_t* pDecoder)
{
    PVIDEOMP4DECODERH264 pH264 = (PVIDEOMP4DECODERH264)pDecoder;
    VIDEOH264PARAMLIST paramList = {
        .spsList = {
            .nCnt = pH264->sps.nNum,
            .pSPS = pH264->sps.pSPS,
        },
        .ppsList = {
            .nCnt = pH264->pps.nNum,
            .pPPS = pH264->pps.pPPS,
        },
    };
    const uint8_t* pNalData;
    VIDEOH264_nal_unit nalUnit;
    uint32_t nNalSize;
    uint32_t nNalOffset = 0;
    uint32_t nDecodedSliceLen = 0;
    VIDEOH264SLICE slice;
    HANINT nId = ListView_GetItemCount(hWindow);
    
    do {
        pNalData = &pData[nNalOffset];
        nNalSize = ReadMP4BoxDecoderH2644ByteMSB(pNalData);
        GetNalUnit(pNalData[4], &nalUnit);
        if (true == H264DecodeBy_slice_layer_without_partitioning(&nalUnit))
        {
            nDecodedSliceLen = DecodeH264Parameter_slice_header(&pNalData[4], nNalSize, &slice, &paramList);
            break;
        }
        nNalOffset += nNalSize + 4;
    } while (nNalOffset < nLen);

    if (0 < nDecodedSliceLen)
    {
        nId = UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hWindow);
        nId = UpdateH264InfoWindow_Slice(&slice, hWindow, nId);
    }
}

void DecodeMedia_H264(const uint8_t* pData, HANSIZE nLen, uint8_t* pDecoder)
{
    PVIDEOMP4DECODERH264 pH264 = (PVIDEOMP4DECODERH264)pDecoder;
    const uint8_t* pNalData;
    uint32_t nNalSize;
    uint32_t nNalOffset;
    VIDEOH264PARAMLIST paramList;
    VIDEOH264_nal_unit nalUnit;

    paramList.spsList.nCnt = pH264->sps.nNum;
    paramList.spsList.pSPS = pH264->sps.pSPS;
    paramList.ppsList.nCnt = pH264->pps.nNum;
    paramList.ppsList.pPPS = pH264->pps.pPPS;

    nNalOffset = 0;
    do {
        pNalData = &pData[nNalOffset];
        nNalSize = ReadMP4BoxDecoderH2644ByteMSB(pNalData);
        GetNalUnit(pNalData[4], &nalUnit);
        if (true == H264DecodeBy_slice_layer_without_partitioning(&nalUnit))
        {
            DecodeH264_slice_layer_without_partitioning(&pNalData[4], nNalSize, &(pH264->H264), &paramList);
            break;
        }
        nNalOffset += nNalSize + 4;
    } while (nNalOffset < nLen);
}

static inline uint16_t ReadMP4BoxDecoderH2642ByteMSB(const uint8_t pData[2])
{
    return (((uint16_t)pData[0] << 8) + (uint16_t)pData[1]);
}
static inline uint32_t ReadMP4BoxDecoderH2644ByteMSB(const uint8_t pData[4])
{
    return (((uint32_t)pData[0] << 24) + ((uint32_t)pData[1] << 16) + ((uint32_t)pData[2] << 8) + (uint32_t)pData[3]);
}

static void ReadMP4BoxCallback_GetH264Size(PCVIDEOMP4BOX pBox, void* pParam)
{
    PVIDEOMP4DECODERH264SIZE pSize = (PVIDEOMP4DECODERH264SIZE)pParam;
    VIDEOMP4BOXINFO_avcC avcCInfo;

    switch (pBox->eType) {
        case VIDEO_MP4_BOX_TYPE_avc1: {
            (void)ReadMP4Boxes(&pBox->pData[VIDEO_MP4_H264_BOX_OFFSET_avc1], pBox->nSize - VIDEO_MP4_H264_BOX_OFFSET_avc1, ReadMP4BoxCallback_GetH264Size, pSize);
        } break;
        case VIDEO_MP4_BOX_TYPE_avcC: {
            DecodeBoxData_avcC(pBox, &avcCInfo);
            pSize->nSPSSize += sizeof(VIDEOH264SPS) * avcCInfo.sps.nNum;
            pSize->nPPSSize += sizeof(VIDEOH264PPS) * avcCInfo.pps.nNum;
            UpdateH264Size_avcC(&avcCInfo, pSize);
        } break;
        default: { } break;
    }
}
static void ReadMP4BoxCallback_InitH264Decoder(PCVIDEOMP4BOX pBox, void* pParam)
{
    PVIDEOMP4DECODERH264INIT pInitDecoder = (PVIDEOMP4DECODERH264INIT)pParam;
    VIDEOMP4BOXINFO_avcC avcCInfo;

    switch (pBox->eType) {
        case VIDEO_MP4_BOX_TYPE_avc1: {
            (void)ReadMP4Boxes(&pBox->pData[VIDEO_MP4_H264_BOX_OFFSET_avc1], pBox->nSize - VIDEO_MP4_H264_BOX_OFFSET_avc1, ReadMP4BoxCallback_InitH264Decoder, pInitDecoder);
        } break;
        case VIDEO_MP4_BOX_TYPE_avcC: {
            DecodeBoxData_avcC(pBox, &avcCInfo);
            pInitDecoder->pDecoder->nSize += UpdateH264Decoder_avcC(&avcCInfo, pInitDecoder);
        } break;
        default: { } break;
    }
}

static void UpdateH264Size_avcC(PVIDEOMP4BOXINFO_avcC pavcC, PVIDEOMP4DECODERH264SIZE pSize)
{
    uint8_t nSPSCnt = pavcC->sps.nNum;
    const uint8_t* pData;
    uint16_t nLen;
    VIDEOH264SPS sps;
    HANSIZE nOffset;
    HANSIZE nMacroBlockSize;

    nOffset = 0;
    pData = pavcC->sps.pList;
    for (uint8_t iLoop = 0; iLoop < nSPSCnt; iLoop++)
    {
        nLen = ReadMP4BoxDecoderH2642ByteMSB(&pData[nOffset]);
        DecodeBoxData_avcC_SPS(&pData[nOffset + 2], nLen, &sps);
        pSize->nSliceGroupMapSize += (HANSIZE)GetMbToSliceGroupMapSize(&sps) + (HANSIZE)GetmapUnitToSliceGroupMapSize(&sps);
        nMacroBlockSize = sizeof(VIDEOH264MACROBLOCK) * sps.PicWidthInMbs * sps.FrameHeightInMbs;
        if (pSize->nMacroBlockSize < nMacroBlockSize) { pSize->nMacroBlockSize = nMacroBlockSize; }
        nOffset += nLen + 2;
    }
}

static HANSIZE UpdateH264Decoder_avcC(PVIDEOMP4BOXINFO_avcC pavcC, PVIDEOMP4DECODERH264INIT pInitDecoder)
{
    PVIDEOMP4DECODERH264 pDecoder = pInitDecoder->pDecoder;
    HANSIZE nSize = pInitDecoder->nSize;
    uint8_t* pBuf = pDecoder->pBuf;
    uint8_t nSPSCnt = pavcC->sps.nNum;
    uint8_t nPPSCnt = pavcC->pps.nNum;
    const uint8_t* pData;
    PVIDEOH264SPS pSPS;
    PVIDEOH264PPS pPPS;
    HANSIZE nLen;
    HANSIZE nDataOffset;
    HANSIZE nDecoderSize;
    HANSIZE nMacroBlockTempSize;
    HANSIZE nMacroBlockSize = 0;
    BOOL bOk = TRUE;

    nDecoderSize = 0;
    /* 更新 SPS */
    nDataOffset = 0;
    pSPS = (PVIDEOH264SPS)(&pBuf[nDecoderSize]);
    pData = pavcC->sps.pList;
    nDecoderSize += sizeof(VIDEOH264SPS) * nSPSCnt;
    if (nDecoderSize <= nSize)
    {
        for (uint8_t iLoop = 0; iLoop < nSPSCnt; iLoop++)
        {
            nLen = ReadMP4BoxDecoderH2642ByteMSB(&pData[nDataOffset]);
            DecodeBoxData_avcC_SPS(&pData[nDataOffset + 2], nLen, &(pSPS[iLoop]));
            nMacroBlockTempSize = sizeof(VIDEOH264MACROBLOCK) * pSPS[iLoop].PicWidthInMbs * pSPS[iLoop].FrameHeightInMbs;
            if (nMacroBlockSize < nMacroBlockTempSize) { nMacroBlockSize = nMacroBlockTempSize; }
            nDataOffset += nLen + 2;
        }
        pDecoder->sps.pSPS = pSPS;
        pDecoder->sps.nNum = nSPSCnt;
    }
    else { bOk = FALSE; }
    /* 更新 mapUnitToSliceGroupMap 和 MbToSliceGroupMap */
    for (uint8_t iLoop = 0; iLoop < nSPSCnt; iLoop++)
    {
        nLen = GetmapUnitToSliceGroupMapSize(&pSPS[iLoop]);
        pSPS[iLoop].mapUnitToSliceGroupMap = (PVIDEOH264SLICEGROUPMAP)(&pBuf[nDecoderSize]);
        nDecoderSize += nLen;
        nLen = GetMbToSliceGroupMapSize(&pSPS[iLoop]);
        pSPS[iLoop].MbToSliceGroupMap = (PVIDEOH264SLICEGROUPMAP)(&pBuf[nDecoderSize]);
        nDecoderSize += nLen;
        if (nSize < nDecoderSize)
        {
            bOk = FALSE;
            break;
        }
    }
    /* 更新 PPS */
    nDataOffset = 0;
    pPPS = (PVIDEOH264PPS)(&pBuf[nDecoderSize]);
    pData = pavcC->pps.pList;
    nDecoderSize += sizeof(VIDEOH264PPS) * nPPSCnt;
    if (nDecoderSize <= nSize)
    {
        for (uint8_t iLoop = 0; iLoop < nPPSCnt; iLoop++)
        {
            nLen = ReadMP4BoxDecoderH2642ByteMSB(&pData[nDataOffset]);
            DecodeBoxData_avcC_PPS(&pData[nDataOffset + 2], nLen, &(pPPS[iLoop]));
            nDataOffset += nLen + 2;
        }
        pDecoder->pps.pPPS = pPPS;
        pDecoder->pps.nNum = nPPSCnt;
    }
    else { bOk = FALSE; }
    /* 更新宏块 */
    pDecoder->H264.pMacroBlock = (PVIDEOH264MACROBLOCK)(&pBuf[nDecoderSize]);
    nDecoderSize += nMacroBlockSize;
    if (nSize < nDecoderSize) { bOk = FALSE; }
    /* 更新缓存大小 */
    if (TRUE == bOk) { nDecoderSize += sizeof(VIDEOMP4DECODERH264); }
    else { nDecoderSize = 0; }

    return nDecoderSize;
}

static void DecodeBoxData_avc1(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_avc1 pavc1)
{
    const uint8_t* pData = pBox->pData;
    HANSIZE nOffset = 0;
    
    nOffset += 6;
    pavc1->pRawData = pData;
    pavc1->nDataRefIndex = ReadMP4BoxDecoderH2642ByteMSB(&pData[nOffset]); nOffset += 2;
    pavc1->nVersion = ReadMP4BoxDecoderH2642ByteMSB(&pData[nOffset]); nOffset += 2;
    pavc1->nRevision = ReadMP4BoxDecoderH2642ByteMSB(&pData[nOffset]); nOffset += 2;
    pavc1->cVendor = ReadMP4BoxDecoderH2644ByteMSB(&pData[nOffset]); nOffset += 4;
    pavc1->nTemporalQuality = ReadMP4BoxDecoderH2644ByteMSB(&pData[nOffset]); nOffset += 4;
    pavc1->nSpatialQuality = ReadMP4BoxDecoderH2644ByteMSB(&pData[nOffset]); nOffset += 4;
    pavc1->nWidth = ReadMP4BoxDecoderH2642ByteMSB(&pData[nOffset]); nOffset += 2;
    pavc1->nHeight = ReadMP4BoxDecoderH2642ByteMSB(&pData[nOffset]); nOffset += 2;
    pavc1->nHorizResolution.u32 = ReadMP4BoxDecoderH2644ByteMSB(&pData[nOffset]); nOffset += 4;
    pavc1->nHorizResolution.nPhy = (HANDOUBLE)(pavc1->nHorizResolution.u32) / (HANDOUBLE)0x10000;
    pavc1->nVertResolution.u32 = ReadMP4BoxDecoderH2644ByteMSB(&pData[nOffset]); nOffset += 4;
    pavc1->nVertResolution.nPhy = (HANDOUBLE)(pavc1->nVertResolution.u32) / (HANDOUBLE)0x10000;
    pavc1->nDataSize = ReadMP4BoxDecoderH2644ByteMSB(&pData[nOffset]); nOffset += 4;
    pavc1->nFrameCount = ReadMP4BoxDecoderH2642ByteMSB(&pData[nOffset]); nOffset += 2;
    memcpy(pavc1->pCompressorName, &pData[nOffset], 32); nOffset += 32;
    pavc1->nDepth = ReadMP4BoxDecoderH2642ByteMSB(&pData[nOffset]); nOffset += 2;
    pavc1->nColorTable = ReadMP4BoxDecoderH2642ByteMSB(&pData[nOffset]); nOffset += 2;
}
static void DecodeBoxData_avcC(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_avcC pavcC)
{
    const uint8_t* pData = pBox->pData;
    HANSIZE nOffset = 0;
    uint16_t nUnitLen;
    
    pavcC->pRawData = pData;
    pavcC->nConfigurationVersion = pData[nOffset]; nOffset += 1;
    pavcC->nAvcProfileIndication = pData[nOffset]; nOffset += 1;
    pavcC->nProfileCompatibility = pData[nOffset]; nOffset += 1;
    pavcC->nAvcLevelIndication = pData[nOffset]; nOffset += 1;
    pavcC->nNALULengthSize = (pData[nOffset] & 0x03) + 1; nOffset += 1;
    pavcC->sps.nNum = (pData[nOffset] & 0x1F); nOffset += 1;
    pavcC->sps.pList = &pData[nOffset];
    pavcC->sps.nSize = 0;
    for (uint8_t iLoop = 0; iLoop < pavcC->sps.nNum; iLoop++)
    {
        nUnitLen = ReadMP4BoxDecoderH2642ByteMSB(&pData[nOffset]);
        nOffset += (HANSIZE)2 + nUnitLen;
        pavcC->sps.nSize += (HANSIZE)2 + nUnitLen;
    }
    pavcC->pps.nNum = pData[nOffset]; nOffset += 1;
    pavcC->pps.pList = &pData[nOffset];
    for (uint8_t iLoop = 0; iLoop < pavcC->pps.nNum; iLoop++)
    {
        nUnitLen = ReadMP4BoxDecoderH2642ByteMSB(&pData[nOffset]);
        nOffset += (HANSIZE)2 + nUnitLen;
        pavcC->pps.nSize += (HANSIZE)2 + nUnitLen;
    }
}

static void DecodeBoxData_avcC_SPS(const uint8_t* pData, HANSIZE nLen, PVIDEOH264SPS pSPS)
{
    DecodeH264Parameter_seq_parameter_set(pData, (uint32_t)nLen, pSPS);
    DecodeH264BinToVariableSPS(pSPS);
}
static void DecodeBoxData_avcC_PPS(const uint8_t* pData, HANSIZE nLen, PVIDEOH264PPS pPPS)
{
    DecodeH264Parameter_pic_parameter_set(pData, (uint32_t)nLen, pPPS, NULL);
    DecodeH264BinToVariablePPS(pPPS);
}

static void UpdateBoxInfoWindow_pasp(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam)
{
    (void)pParam;
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;

    /* 像素宽高比 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u:%u"), ReadMP4BoxDecoderH2644ByteMSB(&pData[0]), ReadMP4BoxDecoderH2644ByteMSB(&pData[4]));
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_pasp_FieldName(VIDEO_MP4_pasp_BOX_FIELD_PIXEL_ASPECT_RATIO), pText, nId, hListView);
}
static void UpdateBoxInfoWindow_avc1(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam)
{
    (void)pParam;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_avc1 avc1Info;

    DecodeBoxData_avc1(&(pBoxTree->mp4Box), &avc1Info);
    /* 数据引用索引 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nDataRefIndex);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_DATA_REF_INDEX), pText, nId, hListView);
    /* 版本 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nVersion);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_VERSION), pText, nId, hListView);
    /* 修订 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nRevision);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_REVISION), pText, nId, hListView);
    /* 修订 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.cVendor);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_VENDOR), pText, nId, hListView);
    /* 时间质量 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nTemporalQuality);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_TEMPORAL_QUALITY), pText, nId, hListView);
    /* 空间质量 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nSpatialQuality);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_SPATIAL_QUALITY), pText, nId, hListView);
    /* 宽度（像素） */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nWidth);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_WIDTH), pText, nId, hListView);
    /* 高度（像素） */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nHeight);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_HEIGHT), pText, nId, hListView);
    /* 水平分辨率 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%g"), avc1Info.nHorizResolution.nPhy);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_HORIZ_RESOLUTION), pText, nId, hListView);
    /* 垂直分辨率 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%g"), avc1Info.nVertResolution.nPhy);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_VERT_RESOLUTION), pText, nId, hListView);
    /* 数据大小 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nDataSize);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_DATA_SIZE), pText, nId, hListView);
    /* 每样本帧数 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nFrameCount);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_FRAME_COUNT), pText, nId, hListView);
    /* 压缩器名称 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, avc1Info.pCompressorName);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_COMPRESSOR_NAME), pText, nId, hListView);
    /* 颜色深度 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nDepth);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_DEPTH), pText, nId, hListView);
    /* 颜色表ID */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nColorTable);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(VIDEO_MP4_avc1_BOX_FIELD_COLOR_TABLE), pText, nId, hListView);
}
static void UpdateBoxInfoWindow_avcC(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam)
{
    (void)pParam;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_avcC avcCInfo;
    VIDEOH264SPS sps;
    VIDEOH264PPS pps;
    uint32_t nLen;
    uint32_t nOffset;

    DecodeBoxData_avcC(&(pBoxTree->mp4Box), &avcCInfo);
    /* 配置版本 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avcCInfo.nConfigurationVersion);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avcC_FieldName(VIDEO_MP4_avcC_BOX_FIELD_CONFIGURATION_VERSION), pText, nId, hListView);
    /* 配置文件标识 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u（%s）"), avcCInfo.nAvcProfileIndication, GetMP4_avcC_ProfileIndicationName(avcCInfo.nAvcProfileIndication));
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avcC_FieldName(VIDEO_MP4_avcC_BOX_FIELD_AVC_PROFILE_INDICATION), pText, nId, hListView);
    /* 配置文件兼容性 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avcCInfo.nProfileCompatibility);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avcC_FieldName(VIDEO_MP4_avcC_BOX_FIELD_PROFILE_COMPATIBILITY), pText, nId, hListView);
    /* 级别标识 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avcCInfo.nAvcLevelIndication);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avcC_FieldName(VIDEO_MP4_avcC_BOX_FIELD_AVC_LEVEL_INDICATION), pText, nId, hListView);
    /* NALU长度大小 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u字节"), avcCInfo.nNALULengthSize);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_avcC_FieldName(VIDEO_MP4_avcC_BOX_FIELD_AVC_NALU_LENGTH_SIZE), pText, nId, hListView);
    /* SPS 条目 */
    nId = UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
    nOffset = 0;
    for (uint8_t iLoop = 0; iLoop < avcCInfo.sps.nNum; iLoop++)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%s%u"), GetMP4_avcC_FieldName(VIDEO_MP4_avcC_BOX_FIELD_AVC_SPS), iLoop + 1);
        nId = UpdateBoxInfoWindow_InsertLine(pText, TEXT(""), nId, hListView);
        nLen = ReadMP4BoxDecoderH2642ByteMSB(&avcCInfo.sps.pList[nOffset]);
        if ((nLen + 2) <= avcCInfo.sps.nSize)
        {
            DecodeH264Parameter_seq_parameter_set(&(avcCInfo.sps.pList)[nOffset + 2], nLen, &sps);
            nId = UpdateH264InfoWindow_SPS(&sps, hListView, nId);
            nOffset += nLen + 2;
        }
        else { break; }
    }
    /* PPS 条目 */
    nId = UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
    nOffset = 0;
    for (uint8_t iLoop = 0; iLoop < avcCInfo.pps.nNum; iLoop++)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%s%u"), GetMP4_avcC_FieldName(VIDEO_MP4_avcC_BOX_FIELD_AVC_PPS), iLoop + 1);
        nId = UpdateBoxInfoWindow_InsertLine(pText, TEXT(""), nId, hListView);
        nLen = ReadMP4BoxDecoderH2642ByteMSB(&avcCInfo.pps.pList[nOffset]);
        if ((nLen + 2) <= avcCInfo.pps.nSize)
        {
            DecodeH264Parameter_pic_parameter_set(&(avcCInfo.pps.pList)[nOffset + 2], nLen, &pps, NULL);
            nId = UpdateH264InfoWindow_PPS(&pps, hListView, nId);
            nOffset += nLen + 2;
        }
        else { break; }
    }
}
static void UpdateBoxInfoWindow_btrt(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam)
{
    (void)pParam;
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;

    /* 解码缓存大小 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u 字节"), ReadMP4BoxDecoderH2644ByteMSB(&pData[0]));
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_btrt_FieldName(VIDEO_MP4_btrt_BOX_FIELD_BUFFER_SIZE), pText, nId, hListView);
    /* 最大比特率 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u bps"), ReadMP4BoxDecoderH2644ByteMSB(&pData[4]));
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_btrt_FieldName(VIDEO_MP4_btrt_BOX_FIELD_MAX_BIT_RATE), pText, nId, hListView);
    /* 平均比特率 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u bps"), ReadMP4BoxDecoderH2644ByteMSB(&pData[8]));
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_btrt_FieldName(VIDEO_MP4_btrt_BOX_FIELD_AVG_BIT_RATE), pText, nId, hListView);
}

static HANINT UpdateH264InfoWindow_SPS(PCVIDEOH264SPS pSPS, HWND hListView, HANINT nStartId)
{
    HANINT nRet = nStartId;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");

    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->nal_ref_idc);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("nal_ref_idc"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->nal_unit_type);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("nal_unit_type"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->profile_idc);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("profile_idc"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->constraint_set0_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("constraint_set0_flag"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->constraint_set1_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("constraint_set1_flag"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->constraint_set2_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("constraint_set2_flag"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->constraint_set3_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("constraint_set3_flag"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->constraint_set4_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("constraint_set4_flag"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->constraint_set5_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("constraint_set5_flag"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->level_idc);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("level_idc"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->seq_parameter_set_id);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("seq_parameter_set_id"), pText, nRet, hListView);
    if (TRUE == SPSProfileIdcHasExtendedData(pSPS->profile_idc))
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->chroma_format_idc);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("chroma_format_idc"), pText, nRet, hListView);
        if (3 == pSPS->chroma_format_idc)
        {
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->separate_colour_plane_flag);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("separate_colour_plane_flag"), pText, nRet, hListView);
        }
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->bit_depth_luma_minus8);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("bit_depth_luma_minus8"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->bit_depth_chroma_minus8);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("bit_depth_chroma_minus8"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->qpprime_y_zero_transform_bypass_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("qpprime_y_zero_transform_bypass_flag"), pText, nRet, hListView);
        if (0 != pSPS->seq_scaling_matrix_present_flag)
        {
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("scaling_list 未实现"), TEXT(""), nRet, hListView);
        }
    }
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->log2_max_frame_num_minus4);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("log2_max_frame_num_minus4"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->pic_order_cnt_type);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("pic_order_cnt_type"), pText, nRet, hListView);
    if (0 == pSPS->pic_order_cnt_type)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->log2_max_pic_order_cnt_lsb_minus4);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("log2_max_pic_order_cnt_lsb_minus4"), pText, nRet, hListView);
    }
    else if (1 == pSPS->pic_order_cnt_type)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->delta_pic_order_always_zero_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("delta_pic_order_always_zero_flag"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%d"), pSPS->offset_for_non_ref_pic);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("offset_for_non_ref_pic"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%d"), pSPS->offset_for_top_to_bottom_field);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("offset_for_top_to_bottom_field"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->num_ref_frames_in_pic_order_cnt_cycle);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("num_ref_frames_in_pic_order_cnt_cycle"), pText, nRet, hListView);
    }
    else { }
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->max_num_ref_frames);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("max_num_ref_frames"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->gaps_in_frame_num_value_allowed_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("gaps_in_frame_num_value_allowed_flag"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->pic_width_in_mbs_minus1);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("pic_width_in_mbs_minus1"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->pic_height_in_map_units_minus1);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("pic_height_in_map_units_minus1"), pText, nRet, hListView);
    if (0 == pSPS->frame_mbs_only_flag)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->mb_adaptive_frame_field_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("mb_adaptive_frame_field_flag"), pText, nRet, hListView);
    }
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->direct_8x8_inference_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("direct_8x8_inference_flag"), pText, nRet, hListView);
    if (0 != pSPS->frame_cropping_flag)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->frame_crop_left_offset);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("frame_crop_left_offset"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->frame_crop_right_offset);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("frame_crop_right_offset"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->frame_crop_top_offset);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("frame_crop_top_offset"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSPS->frame_crop_bottom_offset);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("frame_crop_bottom_offset"), pText, nRet, hListView);
    }
    if (0 != pSPS->vui_parameters_present_flag)
    {
        nRet = UpdateH264InfoWindow_VUIParam(&(pSPS->vui_parameters), hListView, nRet);
    }

    return nRet;
}
static HANINT UpdateH264InfoWindow_VUIParam(PCVIDEOH264PARAMETER_vui_parameters pVUI, HWND hListView, HANINT nStartId)
{
    HANINT nRet = nStartId;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");

    if (0 != pVUI->aspect_ratio_info_present_flag)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->aspect_ratio_idc);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("aspect_ratio_idc"), pText, nRet, hListView);
        if (VIDEO_H264_EXTENDED_SAR == pVUI->aspect_ratio_idc)
        {
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u:%u"), pVUI->sar_width, pVUI->sar_height);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("Extended_SAR"), pText, nRet, hListView);
        }
    }
    if (0 != pVUI->overscan_info_present_flag)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->overscan_appropriate_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("overscan_appropriate_flag"), pText, nRet, hListView);
    }
    if (0 != pVUI->video_signal_type_present_flag)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->video_format);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("video_format"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->video_full_range_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("video_full_range_flag"), pText, nRet, hListView);
        if (0 != pVUI->colour_description_present_flag)
        {
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->colour_primaries);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("colour_primaries"), pText, nRet, hListView);
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->transfer_characteristics);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("transfer_characteristics"), pText, nRet, hListView);
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->matrix_coefficients);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("matrix_coefficients"), pText, nRet, hListView);
        }
    }
    if (0 != pVUI->chroma_loc_info_present_flag)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->chroma_sample_loc_type_top_field);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("chroma_sample_loc_type_top_field"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->chroma_sample_loc_type_bottom_field);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("chroma_sample_loc_type_bottom_field"), pText, nRet, hListView);
    }
    if (0 != pVUI->timing_info_present_flag)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->num_units_in_tick);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("num_units_in_tick"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->time_scale);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("time_scale"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->fixed_frame_rate_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("fixed_frame_rate_flag"), pText, nRet, hListView);
    }
    if (0 != pVUI->nal_hrd_parameters_present_flag) { nRet = UpdateH264InfoWindow_HRDParam(&(pVUI->nal.hrd_parameters), TEXT("nal."), hListView, nRet); }
    if (0 != pVUI->vcl_hrd_parameters_present_flag) { nRet = UpdateH264InfoWindow_HRDParam(&(pVUI->vcl.hrd_parameters), TEXT("vcl."), hListView, nRet); }
    if ((0 != pVUI->nal_hrd_parameters_present_flag) || (0 != pVUI->vcl_hrd_parameters_present_flag))
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->low_delay_hrd_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("low_delay_hrd_flag"), pText, nRet, hListView);
    }
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->pic_struct_present_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("pic_struct_present_flag"), pText, nRet, hListView);
    if (0 != pVUI->bitstream_restriction_flag)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->motion_vectors_over_pic_boundaries_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("motion_vectors_over_pic_boundaries_flag"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->max_bytes_per_pic_denom);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("max_bytes_per_pic_denom"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->max_bits_per_mb_denom);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("max_bits_per_mb_denom"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->log2_max_mv_length_horizontal);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("log2_max_mv_length_horizontal"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->log2_max_mv_length_vertical);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("log2_max_mv_length_vertical"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->max_num_reorder_frames);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("max_num_reorder_frames"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pVUI->max_dec_frame_buffering);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("max_dec_frame_buffering"), pText, nRet, hListView);
    }

    return nRet;
}
static HANINT UpdateH264InfoWindow_HRDParam(PCVIDEOH264PARAMETER_hrd_parameters pHRD, HANPCSTR pHeader, HWND hListView, HANINT nStartId)
{
    HANINT nRet = nStartId;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANCHAR pField[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANPSTR pFieldName;
    HANSIZE nTextLen;
    HANSIZE nFieldSize;

    HAN_strncpy(pField, pHeader, HAN_VIDEO_MP4_TEXT_BUF_SIZE);
    pField[HAN_VIDEO_MP4_TEXT_BUF_SIZE - 1] = TEXT('\0');
    nTextLen = HAN_strlen(pField);
    pFieldName = &pField[nTextLen];
    nFieldSize = HAN_VIDEO_MP4_TEXT_BUF_SIZE - nTextLen;

    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pHRD->cpb_cnt_minus1);
    HAN_strncpy(pFieldName, TEXT("cpb_cnt_minus1"), nFieldSize);
    nRet = UpdateBoxInfoWindow_InsertLine(pField, pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pHRD->bit_rate_scale);
    HAN_strncpy(pFieldName, TEXT("bit_rate_scale"), nFieldSize);
    nRet = UpdateBoxInfoWindow_InsertLine(pField, pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pHRD->cpb_size_scale);
    HAN_strncpy(pFieldName, TEXT("cpb_size_scale"), nFieldSize);
    nRet = UpdateBoxInfoWindow_InsertLine(pField, pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pHRD->initial_cpb_removal_delay_length_minus1);
    HAN_strncpy(pFieldName, TEXT("initial_cpb_removal_delay_length_minus1"), nFieldSize);
    nRet = UpdateBoxInfoWindow_InsertLine(pField, pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pHRD->cpb_removal_delay_length_minus1);
    HAN_strncpy(pFieldName, TEXT("cpb_removal_delay_length_minus1"), nFieldSize);
    nRet = UpdateBoxInfoWindow_InsertLine(pField, pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pHRD->dpb_output_delay_length_minus1);
    HAN_strncpy(pFieldName, TEXT("dpb_output_delay_length_minus1"), nFieldSize);
    nRet = UpdateBoxInfoWindow_InsertLine(pField, pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pHRD->time_offset_length);
    HAN_strncpy(pFieldName, TEXT("time_offset_length"), nFieldSize);
    nRet = UpdateBoxInfoWindow_InsertLine(pField, pText, nRet, hListView);

    return nRet;
}

static HANINT UpdateH264InfoWindow_PPS(PCVIDEOH264PPS pPPS, HWND hListView, HANINT nStartId)
{
    HANINT nRet = nStartId;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");

    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->nal_ref_idc);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("nal_ref_idc"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->nal_unit_type);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("nal_unit_type"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->pic_parameter_set_id);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("pic_parameter_set_id"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->seq_parameter_set_id);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("seq_parameter_set_id"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->entropy_coding_mode_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("entropy_coding_mode_flag"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->bottom_field_pic_order_in_frame_present_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("bottom_field_pic_order_in_frame_present_flag"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->num_slice_groups_minus1);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("num_slice_groups_minus1"), pText, nRet, hListView);
    if (0 < pPPS->num_slice_groups_minus1)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->slice_group_map_type);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("slice_group_map_type"), pText, nRet, hListView);
        if (0 == pPPS->slice_group_map_type)
        {
            ;
        }
        else if (2 == pPPS->slice_group_map_type)
        {
            ;
        }
        else if ((3 == pPPS->slice_group_map_type) ||
                 (4 == pPPS->slice_group_map_type) ||
                 (5 == pPPS->slice_group_map_type))
        {
            ;
        }
        else if (6 == pPPS->slice_group_map_type)
        {
            ;
        }
        else { }
    }
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->num_ref_idx_l0_default_active_minus1);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("num_ref_idx_l0_default_active_minus1"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->num_ref_idx_l1_default_active_minus1);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("num_ref_idx_l1_default_active_minus1"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->weighted_pred_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("weighted_pred_flag"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->weighted_bipred_idc);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("weighted_bipred_idc"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%d"), pPPS->pic_init_qp_minus26);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("pic_init_qp_minus26"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%d"), pPPS->pic_init_qs_minus26);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("pic_init_qs_minus26"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%d"), pPPS->chroma_qp_index_offset);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("chroma_qp_index_offset"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->deblocking_filter_control_present_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("deblocking_filter_control_present_flag"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->constrained_intra_pred_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("constrained_intra_pred_flag"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->redundant_pic_cnt_present_flag);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("redundant_pic_cnt_present_flag"), pText, nRet, hListView);
    if (TRUE == pPPS->bMoreRbspData)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->transform_8x8_mode_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("transform_8x8_mode_flag"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->pic_scaling_matrix_present_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("pic_scaling_matrix_present_flag"), pText, nRet, hListView);
        if (0 != pPPS->pic_scaling_matrix_present_flag)
        {
            ;
        }
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pPPS->second_chroma_qp_index_offset);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("second_chroma_qp_index_offset"), pText, nRet, hListView);
    }

    return nRet;
}

static HANINT UpdateH264InfoWindow_Slice(PCVIDEOH264SLICE pSlice, HWND hListView, HANINT nStartId)
{
    HANINT nRet = nStartId;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    PCVIDEOH264SPS pSPS = pSlice->pSPS;
    PCVIDEOH264PPS pPPS = pSlice->pPPS;
    uint8_t idrPicFlag = GetIdrPicFlag(pSlice->slice_header.nalUnit.nal_unit_type);
    VIDEOH264SLICETYPE sliceType = DecodeH264NormalizeSliceType(pSlice->slice_header.slice_type);

    /* slice_header */
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("Slice Header"), TEXT(""), nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.nalUnit.nal_ref_idc);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("nal_ref_idc"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.nalUnit.nal_unit_type);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("nal_unit_type"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.first_mb_in_slice);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("first_mb_in_slice"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.slice_type);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("slice_type"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.pic_parameter_set_id);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("pic_parameter_set_id"), pText, nRet, hListView);
    if (1 == pSPS->separate_colour_plane_flag)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.colour_plane_id);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("colour_plane_id"), pText, nRet, hListView);
    }
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.frame_num);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("frame_num"), pText, nRet, hListView);
    if (0 == pSPS->frame_mbs_only_flag)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.field_pic_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("field_pic_flag"), pText, nRet, hListView);
        if (0 != pSlice->slice_header.field_pic_flag)
        {
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.bottom_field_flag);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("bottom_field_flag"), pText, nRet, hListView);
        }
    }
    if (0 != idrPicFlag)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.idr_pic_id);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("idr_pic_id"), pText, nRet, hListView);
    }
    if (0 == pSPS->pic_order_cnt_type)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.pic_order_cnt_lsb);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("pic_order_cnt_lsb"), pText, nRet, hListView);
        if ((0 != pPPS->bottom_field_pic_order_in_frame_present_flag) && (0 == pSlice->slice_header.field_pic_flag))
        {
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%d"), pSlice->slice_header.delta_pic_order_cnt_bottom);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("delta_pic_order_cnt_bottom"), pText, nRet, hListView);
        }
    }
    if ((1 == pSPS->pic_order_cnt_type) && (0 == pSPS->delta_pic_order_always_zero_flag))
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%d"), pSlice->slice_header.delta_pic_order_cnt[0]);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("delta_pic_order_cnt[0]"), pText, nRet, hListView);
        if ((0 != pPPS->bottom_field_pic_order_in_frame_present_flag) && (0 == pSlice->slice_header.field_pic_flag))
        {
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%d"), pSlice->slice_header.delta_pic_order_cnt[1]);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("delta_pic_order_cnt[1]"), pText, nRet, hListView);
        }
    }
    if (0 != pPPS->redundant_pic_cnt_present_flag)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.redundant_pic_cnt);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("redundant_pic_cnt"), pText, nRet, hListView);
    }
    if (VIDEO_H264_SLICE_TYPE_B == sliceType)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.direct_spatial_mv_pred_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("direct_spatial_mv_pred_flag"), pText, nRet, hListView);
    }
    if ((VIDEO_H264_SLICE_TYPE_P == sliceType) || (VIDEO_H264_SLICE_TYPE_SP == sliceType) || (VIDEO_H264_SLICE_TYPE_B == sliceType))
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.num_ref_idx_active_override_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("num_ref_idx_active_override_flag"), pText, nRet, hListView);
        if (0 != pSlice->slice_header.num_ref_idx_active_override_flag)
        {
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.num_ref_idx_l0_active_minus1);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("num_ref_idx_l0_active_minus1"), pText, nRet, hListView);
            if (VIDEO_H264_SLICE_TYPE_B == sliceType)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.num_ref_idx_l1_active_minus1);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("num_ref_idx_l1_active_minus1"), pText, nRet, hListView);
            }
        }
    }
    if ((20 == pSlice->slice_header.nalUnit.nal_unit_type) || (21 == pSlice->slice_header.nalUnit.nal_unit_type))
    {
        nRet = UpdateH264InfoWindow_RefPicListMVCModification(&(pSlice->slice_header.ref_pic_list_mvc), sliceType, hListView, nRet);
    }
    else
    {
        nRet = UpdateH264InfoWindow_RefPicListModification(&(pSlice->slice_header.ref_pic_list_mvc), sliceType, hListView, nRet);
    }
    if (((0 != pPPS->weighted_pred_flag) && ((VIDEO_H264_SLICE_TYPE_P == sliceType) || (VIDEO_H264_SLICE_TYPE_SP == sliceType))) ||
        ((1 == pPPS->weighted_bipred_idc) && (VIDEO_H264_SLICE_TYPE_B == sliceType)))
    {
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("pred_weight_table"), TEXT("未实现"), nRet, hListView);
    }
    if (0 != pSlice->slice_header.nalUnit.nal_ref_idc)
    {
        nRet = UpdateH264InfoWindow_DecRefPicMarking(&(pSlice->slice_header.dec_ref_pic_marking), idrPicFlag, hListView, nRet);
    }
    if ((0 != pPPS->entropy_coding_mode_flag) && (VIDEO_H264_SLICE_TYPE_I != sliceType) && (VIDEO_H264_SLICE_TYPE_SI != sliceType))
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.cabac_init_idc);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("cabac_init_idc"), pText, nRet, hListView);
    }
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%d"), pSlice->slice_header.slice_qp_delta);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("slice_qp_delta"), pText, nRet, hListView);
    if ((VIDEO_H264_SLICE_TYPE_SP == sliceType) || (VIDEO_H264_SLICE_TYPE_SI == sliceType))
    {
        if (VIDEO_H264_SLICE_TYPE_SP == sliceType)
        {
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.sp_for_switch_flag);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("sp_for_switch_flag"), pText, nRet, hListView);
        }
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%d"), pSlice->slice_header.slice_qs_delta);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("slice_qs_delta"), pText, nRet, hListView);
    }
    if (0 != pPPS->deblocking_filter_control_present_flag)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%d"), pSlice->slice_header.disable_deblocking_filter_idc);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("disable_deblocking_filter_idc"), pText, nRet, hListView);
        if (1 != pSlice->slice_header.disable_deblocking_filter_idc)
        {
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%d"), pSlice->slice_header.slice_alpha_c0_offset_div2);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("slice_alpha_c0_offset_div2"), pText, nRet, hListView);
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%d"), pSlice->slice_header.slice_beta_offset_div2);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("slice_beta_offset_div2"), pText, nRet, hListView);
        }
    }
    if ((0 < pPPS->num_slice_groups_minus1) && (3 <= pPPS->slice_group_map_type) && (pPPS->slice_group_map_type <= 5))
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_header.slice_group_change_cycle);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("slice_group_change_cycle"), pText, nRet, hListView);
    }
    /* slice_data */
    if (0 != pPPS->entropy_coding_mode_flag)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSlice->slice_data.cabac_alignment_one_bit.nLen);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("cabac_alignment_one_bit 位数"), pText, nRet, hListView);
    }

    return nRet;
}
static HANINT UpdateH264InfoWindow_RefPicListMVCModification(PCVIDEOH264PARAMETER_ref_pic_list_mvc pRefPicListMVC, uint32_t sliceType, HWND hListView, HANINT nStartId)
{
    HANINT nRet = nStartId;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");

    if ((2 != sliceType) && (4 != sliceType))
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicListMVC->ref_pic_list_modification_flag_l0);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("ref_pic_list_modification_flag_l0"), pText, nRet, hListView);
        if (0 != pRefPicListMVC->ref_pic_list_modification_flag_l0)
        {
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicListMVC->modification_of_pic_nums_idc);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("modification_of_pic_nums_idc"), pText, nRet, hListView);
            if (0 != pRefPicListMVC->bValid_abs_diff_pic_num_minus1)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicListMVC->abs_diff_pic_num_minus1);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("abs_diff_pic_num_minus1"), pText, nRet, hListView);
            }
            if (0 != pRefPicListMVC->bValid_long_term_pic_num)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicListMVC->long_term_pic_num);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("long_term_pic_num"), pText, nRet, hListView);
            }
            if (0 != pRefPicListMVC->bValid_abs_diff_view_idx_minus1)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicListMVC->abs_diff_view_idx_minus1);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("abs_diff_view_idx_minus1"), pText, nRet, hListView);
            }
        }
    }
    if (1 == sliceType)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicListMVC->ref_pic_list_modification_flag_l1);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("ref_pic_list_modification_flag_l1"), pText, nRet, hListView);
        if (0 != pRefPicListMVC->ref_pic_list_modification_flag_l1)
        {
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicListMVC->modification_of_pic_nums_idc);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("modification_of_pic_nums_idc"), pText, nRet, hListView);
            if (0 != pRefPicListMVC->bValid_abs_diff_pic_num_minus1)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicListMVC->abs_diff_pic_num_minus1);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("abs_diff_pic_num_minus1"), pText, nRet, hListView);
            }
            if (0 != pRefPicListMVC->bValid_long_term_pic_num)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicListMVC->long_term_pic_num);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("long_term_pic_num"), pText, nRet, hListView);
            }
            if (0 != pRefPicListMVC->bValid_abs_diff_view_idx_minus1)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicListMVC->abs_diff_view_idx_minus1);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("abs_diff_view_idx_minus1"), pText, nRet, hListView);
            }
        }
    }

    return nRet;
}
static HANINT UpdateH264InfoWindow_RefPicListModification(PCVIDEOH264PARAMETER_ref_pic_list pRefPicList, uint32_t sliceType, HWND hListView, HANINT nStartId)
{
    HANINT nRet = nStartId;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");

    if ((2 != sliceType) && (4 != sliceType))
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicList->ref_pic_list_modification_flag_l0);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("ref_pic_list_modification_flag_l0"), pText, nRet, hListView);
        if (0 != pRefPicList->ref_pic_list_modification_flag_l0)
        {
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicList->modification_of_pic_nums_idc);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("modification_of_pic_nums_idc"), pText, nRet, hListView);
            if (0 != pRefPicList->bValid_abs_diff_pic_num_minus1)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicList->abs_diff_pic_num_minus1);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("abs_diff_pic_num_minus1"), pText, nRet, hListView);
            }
            if (0 != pRefPicList->bValid_long_term_pic_num)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicList->long_term_pic_num);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("long_term_pic_num"), pText, nRet, hListView);
            }
        }
    }
    if (1 == sliceType)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicList->ref_pic_list_modification_flag_l1);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("ref_pic_list_modification_flag_l1"), pText, nRet, hListView);
        if (0 != pRefPicList->ref_pic_list_modification_flag_l1)
        {
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicList->modification_of_pic_nums_idc);
            nRet = UpdateBoxInfoWindow_InsertLine(TEXT("modification_of_pic_nums_idc"), pText, nRet, hListView);
            if (0 != pRefPicList->bValid_abs_diff_pic_num_minus1)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicList->abs_diff_pic_num_minus1);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("abs_diff_pic_num_minus1"), pText, nRet, hListView);
            }
            if (0 != pRefPicList->bValid_long_term_pic_num)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pRefPicList->long_term_pic_num);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("long_term_pic_num"), pText, nRet, hListView);
            }
        }
    }

    return nRet;
}
static HANINT UpdateH264InfoWindow_DecRefPicMarking(PCVIDEOH264PARAMETER_dec_ref_pic_marking pDecRefPicMarking, uint8_t idrPicFlag, HWND hListView, HANINT nStartId)
{
    HANINT nRet = nStartId;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");

    if (0 != idrPicFlag)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pDecRefPicMarking->no_output_of_prior_pics_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("no_output_of_prior_pics_flag"), pText, nRet, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pDecRefPicMarking->long_term_reference_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("long_term_reference_flag"), pText, nRet, hListView);
    }
    else
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pDecRefPicMarking->adaptive_ref_pic_marking_mode_flag);
        nRet = UpdateBoxInfoWindow_InsertLine(TEXT("adaptive_ref_pic_marking_mode_flag"), pText, nRet, hListView);
        if (0 != pDecRefPicMarking->adaptive_ref_pic_marking_mode_flag)
        {
            if (0 != pDecRefPicMarking->bValid_difference_of_pic_nums_minus1)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pDecRefPicMarking->difference_of_pic_nums_minus1);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("difference_of_pic_nums_minus1"), pText, nRet, hListView);
            }
            if (0 != pDecRefPicMarking->bValid_long_term_pic_num)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pDecRefPicMarking->long_term_pic_num);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("long_term_pic_num"), pText, nRet, hListView);
            }
            if (0 != pDecRefPicMarking->bValid_long_term_frame_idx)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pDecRefPicMarking->long_term_frame_idx);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("long_term_frame_idx"), pText, nRet, hListView);
            }
            if (0 != pDecRefPicMarking->bValid_max_long_term_frame_idx_plus1)
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pDecRefPicMarking->max_long_term_frame_idx_plus1);
                nRet = UpdateBoxInfoWindow_InsertLine(TEXT("max_long_term_frame_idx_plus1"), pText, nRet, hListView);
            }
        }
    }

    return nRet;
}
