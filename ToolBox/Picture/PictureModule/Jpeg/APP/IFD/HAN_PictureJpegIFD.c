#include <math.h>
#include <string.h>
#include <Windows.h>
#include <CommCtrl.h>

#include "HAN_PictureJpegIFD.h"
#include "..\..\HAN_PictureJpegSegment.h"

typedef enum {
    PICTURE_JPEG_SEGMENT_READ_BYTE_ORDER_LSB,
    PICTURE_JPEG_SEGMENT_READ_BYTE_ORDER_MSB,
    PICTURE_JPEG_SEGMENT_READ_BYTE_ORDER_CNT,
} PICTUREJPEGSEGMENTREADBYTEORDER;

typedef struct tagPICTUREJPEGSEGMENTIFDPRINTPARAM {
    HWND                            hSegmentInfo;
} PICTUREJPEGSEGMENTIFDPRINTPARAM, * PPICTUREJPEGSEGMENTIFDPRINTPARAM;

static uint16_t ReadIFDData2ByteLSB(const uint8_t pData[2]);
static uint32_t ReadIFDData4ByteLSB(const uint8_t pData[4]);
static uint64_t ReadIFDData8ByteLSB(const uint8_t pData[8]);
static uint16_t ReadIFDData2ByteMSB(const uint8_t pData[2]);
static uint32_t ReadIFDData4ByteMSB(const uint8_t pData[4]);
static uint64_t ReadIFDData8ByteMSB(const uint8_t pData[8]);

static HANSIZE GetIFDDataLen(PPICTUREJPEGIFDMETA pIFD);
static HANSIZE GetIFDValue(const uint8_t* pSrc, PPICTUREJPEGIFDMETA pIFD, PCPICTUREJPEGREADBYTES ReadBytesCallback);

static const PICTUREJPEGREADBYTES sg_pJpegReadBytes[PICTURE_JPEG_SEGMENT_READ_BYTE_ORDER_CNT] = {
    [PICTURE_JPEG_SEGMENT_READ_BYTE_ORDER_LSB] = {
        .Read2Bytes = ReadIFDData2ByteLSB,
        .Read4Bytes = ReadIFDData4ByteLSB,
        .Read8Bytes = ReadIFDData8ByteLSB,
    },
    [PICTURE_JPEG_SEGMENT_READ_BYTE_ORDER_MSB] = {
        .Read2Bytes = ReadIFDData2ByteMSB,
        .Read4Bytes = ReadIFDData4ByteMSB,
        .Read8Bytes = ReadIFDData8ByteMSB,
    },
};

void UpdateSegmentInfoWindow_IFD_ExifReadTIFF(
    const uint8_t* pData,
    HANSIZE nLen,
    PPICTUREJPEGSEGMENTPRINTIFD pPrintIFD,
    DECODEIFDMETACALLBACK DecodeIFDMeataCallback)
{
    BOOL bOk = TRUE;
    PCPICTUREJPEGREADBYTES ReadBytesCallback;

    if ((0x49 == pData[0]) && (0x49 == pData[1])) { ReadBytesCallback = &sg_pJpegReadBytes[PICTURE_JPEG_SEGMENT_READ_BYTE_ORDER_LSB]; }
    else if ((0x4D == pData[0]) && (0x4D == pData[1])) { ReadBytesCallback = &sg_pJpegReadBytes[PICTURE_JPEG_SEGMENT_READ_BYTE_ORDER_MSB]; }
    else { bOk = FALSE; }

    if (TRUE == bOk) { UpdateSegmentInfoWindow_IFD_ExifReadIFDMain(pData, nLen, ReadBytesCallback, pPrintIFD, DecodeIFDMeataCallback); }
}

void UpdateSegmentInfoWindow_IFD_ExifReadIFDMain(
    const uint8_t* pData,
    HANSIZE nLen,
    PCPICTUREJPEGREADBYTES ReadBytesCallback,
    PPICTUREJPEGSEGMENTPRINTIFD pPrintIFD,
    DECODEIFDMETACALLBACK DecodeIFDMeataCallback)
{
    PICTUREJPEGSEGMENTREADEXIF readExif;
    HANSIZE nOffset;

    nOffset = ReadBytesCallback->Read4Bytes(&pData[4]);
    readExif.printIFD = *pPrintIFD;
    readExif.ifdData.ReadBytes = ReadBytesCallback;
    readExif.ifdData.pTIFFData = pData;
    readExif.ifdData.nFileLen = nLen;
    readExif.ifdData.pIFDData = &pData[nOffset];
    readExif.ifdData.nIFDLen = nLen - nOffset;
    readExif.ifdData.cIFDID = 0;
    readExif.DecodeIFDMetaCallback = DecodeIFDMeataCallback;
    PictureJpegReadIFD(&readExif);
}

void PictureJpegReadIFD(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    PCPICTUREJPEGREADBYTES ReadBytesCallback = pReadExif->ifdData.ReadBytes;
    const uint8_t* pData = pReadExif->ifdData.pIFDData;
    HANSIZE nOffset;
    uint16_t nEntryCnt;
    BOOL bLoop = TRUE;

    pReadExif->ifdData.nIFDBlockCnt = 0;
    while (TRUE == bLoop)
    {
        nOffset = 0;
        pReadExif->printIFD.PrintIFDTitleCallback(&(pReadExif->ifdData), pReadExif->printIFD.pPrintParam);
        nEntryCnt = ReadBytesCallback->Read2Bytes(&pData[nOffset]); nOffset += 2;
        for (HANSIZE iLoop = 0; iLoop < nEntryCnt; iLoop++)
        {
            pReadExif->ifdData.ifdMeta.cTag = ReadBytesCallback->Read2Bytes(&pData[nOffset]); nOffset += 2;
            pReadExif->ifdData.ifdMeta.cDataType = ReadBytesCallback->Read2Bytes(&pData[nOffset]); nOffset += 2;
            pReadExif->ifdData.ifdMeta.nDataCnt = ReadBytesCallback->Read4Bytes(&pData[nOffset]); nOffset += 4;
            if (GetIFDValue(&pData[nOffset], &(pReadExif->ifdData.ifdMeta), ReadBytesCallback) <= 4)
            {
                pReadExif->ifdData.pIFDData = pReadExif->ifdData.ifdMeta.cValue.pDataU8;
            }
            else
            {
                pReadExif->ifdData.pIFDData = &pReadExif->ifdData.pTIFFData[pReadExif->ifdData.ifdMeta.cValue.nOffset];
            }
            nOffset += 4;

            pReadExif->DecodeIFDMetaCallback(pReadExif);
        }
        nOffset = ReadBytesCallback->Read4Bytes(&pData[nOffset]);
        if ((0 < nOffset) && (nOffset < pReadExif->ifdData.nFileLen))
        {
            pData = &(pReadExif->ifdData.pTIFFData[nOffset]);
            pReadExif->ifdData.nIFDBlockCnt++;
        }
        else
        {
            bLoop = FALSE;
        }
    }
}

void UpdateSegmentInfoWindow_IFD_PrintIFDRootTitle(PPICTUREJPEGSEGMENTIFDDATA ifdData, void* pParam)
{
    // 新的 IFD，推荐操作：打印标题，元数据计数器清0
    HWND hInfo = (HWND)pParam;
    LVITEM lvItem = {
        .mask = LVIF_TEXT,
        .iItem = ListView_GetItemCount(hInfo),
        .iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD,
        .pszText = TEXT(""),
    };
    
    if (0 < lvItem.iItem) { ListView_InsertItem(hInfo, &lvItem); }

    lvItem.iItem++;
    lvItem.pszText = ifdData->pIFDTitle;
    HAN_snprintf(ifdData->pIFDTitle, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("IFD" HANSIZE_PRINT_FORMAT), ifdData->nIFDBlockCnt);
    ListView_InsertItem(hInfo, &lvItem);

    ifdData->nMetaId = 0;
}
void UpdateSegmentInfoWindow_IFD_PrintIFDStdData(PPICTUREJPEGSEGMENTIFDDATA ifdData, void* pParam)
{
    // 新的元数据，推荐操作：打印内容，计数器累加。如果需要忽略某些 Tag，则可以什么都不做，包括计数器累加
    HWND hInfo = (HWND)pParam;
    LVFINDINFO lvFind = { .flags = LVFI_STRING, .psz = ifdData->pIFDTitle, };
    HANINT nListId = ListView_FindItem(hInfo, -1, &lvFind) + 1 + (HANINT)(ifdData->nMetaId);
    LVITEM lvName = {
        .mask = LVIF_TEXT,
        .iItem = nListId,
        .iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD,
        .pszText = ifdData->pName,
    };
    LVITEM lvValue = {
        .mask = LVIF_TEXT,
        .iItem = nListId,
        .iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE,
        .pszText = ifdData->pValue,
    };

    ListView_InsertItem(hInfo, &lvName);
    ListView_SetItem(hInfo, &lvValue);
    (ifdData->nMetaId)++;
}

static uint16_t ReadIFDData2ByteLSB(const uint8_t pData[2])
{
    uint16_t nRet;
    memcpy(&nRet, pData, sizeof(nRet));
    return nRet;
}
static uint32_t ReadIFDData4ByteLSB(const uint8_t pData[4])
{
    uint32_t nRet;
    memcpy(&nRet, pData, sizeof(nRet));
    return nRet;
}
static uint64_t ReadIFDData8ByteLSB(const uint8_t pData[8])
{
    uint64_t nRet;
    memcpy(&nRet, pData, sizeof(nRet));
    return nRet;
}
static uint16_t ReadIFDData2ByteMSB(const uint8_t pData[2])
{
    return (((uint16_t)pData[0] << 8) + (uint16_t)pData[1]);
}
static uint32_t ReadIFDData4ByteMSB(const uint8_t pData[4])
{
    return (((uint32_t)pData[0] << 24) + ((uint32_t)pData[1] << 16) + ((uint32_t)pData[2] << 8) + (uint32_t)pData[3]);
}
static uint64_t ReadIFDData8ByteMSB(const uint8_t pData[8])
{
    return (
        ((uint64_t)pData[0] << 56) + ((uint64_t)pData[1] << 48) + ((uint64_t)pData[2] << 40) + ((uint64_t)pData[3] << 32)
         + ((uint64_t)pData[4] << 24) + ((uint64_t)pData[5] << 16) + ((uint64_t)pData[6] << 8) + (uint64_t)pData[7]
    );
}

static HANSIZE GetIFDDataLen(PPICTUREJPEGIFDMETA pIFD)
{
    HANSIZE nRet;

    switch (pIFD->cDataType) {
        case 1: { nRet = pIFD->nDataCnt; } break;
        case 2: { nRet = pIFD->nDataCnt; } break;
        case 3: { nRet = 2 * pIFD->nDataCnt; } break;
        case 4: { nRet = 4 * pIFD->nDataCnt; } break;
        case 5: { nRet = 8 * pIFD->nDataCnt; } break;
        case 7: { nRet = pIFD->nDataCnt; } break;
        case 9: { nRet = 4 * pIFD->nDataCnt; } break;
        case 10: { nRet = 8 * pIFD->nDataCnt; } break;
        default: { nRet = 0; } break;
    }

    return nRet;
}
static HANSIZE GetIFDValue(const uint8_t* pSrc, PPICTUREJPEGIFDMETA pIFD, PCPICTUREJPEGREADBYTES ReadBytesCallback)
{
    HANSIZE nRet = GetIFDDataLen(pIFD);
    
    if (nRet <= 4)
    {
        switch (pIFD->cDataType) {
            case 1: { memcpy(&(pIFD->cValue.pDataU8), pSrc, nRet); } break;
            case 2: { memcpy(&(pIFD->cValue.pDataU8), pSrc, nRet); } break;
            case 3: {
                for (HANSIZE iLoop = 0; iLoop < pIFD->nDataCnt; iLoop++)
                {
                    pIFD->cValue.pDataU16[iLoop] = ReadBytesCallback->Read2Bytes(&pSrc[iLoop * 2]);
                }
            } break;
            case 4: { pIFD->cValue.pDataU32[0] = ReadBytesCallback->Read4Bytes(pSrc); } break;
            case 7: { memcpy(&(pIFD->cValue.pDataU8), pSrc, nRet); } break;
            case 9: { pIFD->cValue.pDataU32[0] = ReadBytesCallback->Read4Bytes(pSrc); } break;
            default: { } break;
        }
    }
    else
    {
        pIFD->cValue.nOffset = ReadBytesCallback->Read4Bytes(pSrc);
    }

    return nRet;
}
