#include <string.h>
#include <Windows.h>
#include <CommCtrl.h>

#include "HAN_PictureJpegMPF.h"
#include "..\..\IFD\HAN_PictureJpegIFD.h"

static void UpdateSegmentInfoWindow_APP1_PrintMPFImageTitle(PPICTUREJPEGSEGMENTIFDDATA ifdData, void* pParam);
static BOOL UpdateSegmentInfoWindow_APP1_ReadMPFDecodeIFDMeta(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP2_MPFReadB000(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP2_MPFReadB001(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP2_MPFReadB002(PPICTUREJPEGSEGMENTREADEXIF pReadExif);

BOOL UpdateSegmentInfoWindow_APP2_ReadMPF(PCPICTUREJPEGSEGMENT pSegment, HWND hListView)
{
    BOOL bRet = FALSE;
    uint8_t pHeader[4] = "MPF";
    HANCHAR pText[4];
    LVCOLUMN lvValue = { .mask = LVCF_TEXT, };
    PICTUREJPEGSEGMENTPRINTIFD printIFD = {
        .PrintIFDTitleCallback = UpdateSegmentInfoWindow_IFD_PrintIFDRootTitle,
        .PrintIFDDataCallback = UpdateSegmentInfoWindow_IFD_PrintIFDStdData,
        .pPrintParam = hListView,
    };

    if (!memcmp(pHeader, pSegment->pData, sizeof(pHeader)))
    {
        HAN_snprintf(pText, ArrLen(pText), HANPSTR_PRINT_PCHAR_FORMAT, pHeader);
        lvValue.pszText = pText;
        ListView_SetColumn(hListView, PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE, &lvValue);
        UpdateSegmentInfoWindow_IFD_ExifReadTIFF(
            &(pSegment->pData[sizeof(pHeader)]),
            pSegment->nLength - sizeof(pHeader),
            &printIFD,
            UpdateSegmentInfoWindow_APP1_ReadMPFDecodeIFDMeta
        );
        bRet = TRUE;
    }

    return bRet;
}

static void UpdateSegmentInfoWindow_APP1_PrintMPFImageTitle(PPICTUREJPEGSEGMENTIFDDATA ifdData, void* pParam)
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
    ListView_InsertItem(hInfo, &lvItem);

    ifdData->nMetaId = 0;
}
static BOOL UpdateSegmentInfoWindow_APP1_ReadMPFDecodeIFDMeta(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    BOOL bRet = TRUE;

    switch (pReadExif->ifdData.ifdMeta.cTag) {
        case 0xB000: { UpdateSegmentInfoWindow_APP2_MPFReadB000(pReadExif); } break;
        case 0xB001: { UpdateSegmentInfoWindow_APP2_MPFReadB001(pReadExif); } break;
        case 0xB002: { UpdateSegmentInfoWindow_APP2_MPFReadB002(pReadExif); } break;

        default: { bRet = FALSE; } break;
    }

    return bRet;
}
static void UpdateSegmentInfoWindow_APP2_MPFReadB000(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    const uint8_t* pData = pReadExif->ifdData.pIFDData;
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP2_FieldName(PICTURE_JPEG_APP2_SEGMENT_FIELD_MPF_VERSION));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%c%c%c%c"), pData[0], pData[1], pData[2], pData[3]);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP2_MPFReadB001(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t nNum = pReadExif->ifdData.ifdMeta.cValue.pDataU32[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP2_FieldName(PICTURE_JPEG_APP2_SEGMENT_FIELD_MPF_NUMBER_OF_IMAGES));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), nNum);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP2_MPFReadB002(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    PICTUREJPEGSEGMENTREADEXIF readExif = *pReadExif;
    const uint8_t* pData = readExif.ifdData.pIFDData;
    uint32_t nNum = readExif.ifdData.ifdMeta.nDataCnt / 16;
    uint32_t nOffset;
    uint32_t cMask;
    uint32_t nImageLength;
    uint32_t nImagePos;
    uint16_t nEntryNumber;

    readExif.printIFD.PrintIFDTitleCallback = UpdateSegmentInfoWindow_APP1_PrintMPFImageTitle;
    for (uint32_t iLoop = 0; iLoop < nNum; iLoop++)
    {
        nOffset = iLoop * 16;

        HAN_snprintf(readExif.ifdData.pIFDTitle, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("图像%u"), iLoop);
        readExif.printIFD.PrintIFDTitleCallback(&(readExif.ifdData), readExif.printIFD.pPrintParam);

        cMask = readExif.ifdData.ReadBytes->Read4Bytes(&pData[nOffset]);
        HAN_snprintf(readExif.ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP2_FieldName(PICTURE_JPEG_APP2_SEGMENT_FIELD_MPF_MP_IMAGE_FLAGS));
        GetJpeg_APP2_MPImageFlagsName(cMask, readExif.ifdData.pValue);
        UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(&readExif);
        nOffset += sizeof(cMask);
        
        HAN_snprintf(readExif.ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP2_FieldName(PICTURE_JPEG_APP2_SEGMENT_FIELD_MPF_MP_IMAGE_FORMAT));
        HAN_snprintf(readExif.ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%s"), GetJpeg_APP2_MPImageFormatName(cMask));
        UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(&readExif);

        HAN_snprintf(readExif.ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP2_FieldName(PICTURE_JPEG_APP2_SEGMENT_FIELD_MPF_MP_IMAGE_TYPE));
        HAN_snprintf(readExif.ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%s"), GetJpeg_APP2_MPImageTypeName(cMask));
        UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(&readExif);

        nImageLength = readExif.ifdData.ReadBytes->Read4Bytes(&pData[nOffset]);
        HAN_snprintf(readExif.ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP2_FieldName(PICTURE_JPEG_APP2_SEGMENT_FIELD_MPF_MP_IMAGE_LENGTH));
        HAN_snprintf(readExif.ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), nImageLength);
        UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(&readExif);
        nOffset += sizeof(nImageLength);

        nImagePos = readExif.ifdData.ReadBytes->Read4Bytes(&pData[nOffset]);
        HAN_snprintf(readExif.ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP2_FieldName(PICTURE_JPEG_APP2_SEGMENT_FIELD_MPF_MP_IMAGE_START));
        HAN_snprintf(readExif.ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("0x%08X"), nImagePos);
        UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(&readExif);
        nOffset += sizeof(nImagePos);

        nEntryNumber = readExif.ifdData.ReadBytes->Read4Bytes(&pData[nOffset]);
        HAN_snprintf(readExif.ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP2_FieldName(PICTURE_JPEG_APP2_SEGMENT_FIELD_MPF_MP_IMAGE_DEPENDENT_IMAGE1_ENTRY_NUMBER));
        HAN_snprintf(readExif.ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), nEntryNumber);
        UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(&readExif);
        nOffset += sizeof(nEntryNumber);

        nEntryNumber = readExif.ifdData.ReadBytes->Read4Bytes(&pData[nOffset]);
        HAN_snprintf(readExif.ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP2_FieldName(PICTURE_JPEG_APP2_SEGMENT_FIELD_MPF_MP_IMAGE_DEPENDENT_IMAGE2_ENTRY_NUMBER));
        HAN_snprintf(readExif.ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), nEntryNumber);
        UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(&readExif);
        nOffset += sizeof(nEntryNumber);
    }
}
