#include <string.h>
#include <math.h>
#include <Windows.h>
#include <CommCtrl.h>

#include "HAN_PictureJpegPhotoshop.h"
#include "HAN_PictureJpegPhotoshop8BIM.h"
#include "..\..\IPTC\HAN_PictureJpegIPTC.h"
#include "..\..\..\HAN_PictureJpegLib.h"

typedef struct tagPICTUREJPEGAPP13PHOTOSHOP {
    uint16_t        nSrcId;
    CHAR            pName[260];
    uint32_t        nLen;
    const uint8_t*  pData;
} PICTUREJPEGAPP13PHOTOSHOP, * PPICTUREJPEGAPP13PHOTOSHOP;

typedef struct tagPICTUREJPEGAPP13PHOTOSHOPPRINTPARAM {
    HWND            hListView;
    HANCHAR         pTitle[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    HANCHAR         pName[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    HANCHAR         pValue[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
} PICTUREJPEGAPP13PHOTOSHOPPRINTPARAM, * PPICTUREJPEGAPP13PHOTOSHOPPRINTPARAM;

static void UpdateSegmentInfoWindow_APP13_ReadPhotoshopPrintUnknown8BIM(PPICTUREJPEGAPP13PHOTOSHOP pPsData);

static void UpdateSegmentInfoWindow_APP13_PrintUnknownDataFormat(PPICTUREJPEGAPP13PHOTOSHOP pPsData, PPICTUREJPEGAPP13PHOTOSHOPPRINTPARAM pPrintData);
static void UpdateSegmentInfoWindow_APP13_PrintPhotoshopTitle(PPICTUREJPEGAPP13PHOTOSHOPPRINTPARAM pParam);
static void UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(PPICTUREJPEGAPP13PHOTOSHOPPRINTPARAM pParam);
static HANPSTR UpdateSegmentInfoWindow_APP13_PrintBitTrueFalse(uint32_t cFlag);
static HANSIZE UpdateSegmentInfoWindow_APP13_ReadPhotoshopRead8BIM(const uint8_t* pData, HANSIZE nLen, HWND hListView);

static const HANPSTR sg_pTrueFalse[2] = {
    [0] = TEXT("False"),
    [1] = TEXT("True"),
};

BOOL UpdateSegmentInfoWindow_APP13_ReadPhotoshop(PCPICTUREJPEGSEGMENT pSegment, HWND hListView)
{
    BOOL bRet = FALSE;
    PCCH pHeader = "Photoshop 3.0";
    HANCHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    LVCOLUMN lvValue = { .mask = LVCF_TEXT, };
    const uint8_t* pData = pSegment->pData;
    HANSIZE nLen = pSegment->nLength;
    HANSIZE nOffset = strlen(pHeader) + 1;
    HANSIZE nReadLen;

    if (!memcmp(pHeader, pData, nOffset))
    {
        HAN_snprintf(pText, ArrLen(pText), HANPSTR_PRINT_PCHAR_FORMAT, pHeader);
        lvValue.pszText = pText;
        ListView_SetColumn(hListView, PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE, &lvValue);

        while (nOffset < nLen)
        {
            nReadLen = UpdateSegmentInfoWindow_APP13_ReadPhotoshopRead8BIM(&pData[nOffset], nLen - nOffset, hListView);
            if (0 < nReadLen) { nOffset += nReadLen; }
            else { nOffset++; }
        }

        bRet = TRUE;
    }

    return bRet;
}

static void UpdateSegmentInfoWindow_APP13_ReadPhotoshopPrintUnknown8BIM(PPICTUREJPEGAPP13PHOTOSHOP pPsData)
{
    printf("Unknown 8BIM: %04X, len: %u\n", pPsData->nSrcId, pPsData->nLen);
}

static void UpdateSegmentInfoWindow_APP13_PrintUnknownDataFormat(PPICTUREJPEGAPP13PHOTOSHOP pPsData, PPICTUREJPEGAPP13PHOTOSHOPPRINTPARAM pPrintData)
{
    HANPSTR pName = pPrintData->pName;
    HANPSTR pValue = pPrintData->pValue;
    HANSIZE nValueLen;

    HAN_strncpy(pName, TEXT("未知数据格式"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("长度: %u, 数据: "), pPsData->nLen);
    pValue[HAN_PICTURE_JPEG_TEXT_BUF_SIZE - 1] = TEXT('\0');
    nValueLen = HAN_strlen(pValue);
    PictureJpegPrintHexData(&pValue[nValueLen], HAN_PICTURE_JPEG_TEXT_BUF_SIZE - nValueLen, pPsData->pData, pPsData->nLen);
}
static void UpdateSegmentInfoWindow_APP13_PrintPhotoshopTitle(PPICTUREJPEGAPP13PHOTOSHOPPRINTPARAM pParam)
{
    HWND hInfo = pParam->hListView;
    LVITEM lvItem = {
        .mask = LVIF_TEXT,
        .iItem = ListView_GetItemCount(hInfo),
        .iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD,
    };

    if (0 < lvItem.iItem)
    {
        lvItem.pszText = TEXT(""),
        ListView_InsertItem(hInfo, &lvItem);
        lvItem.iItem++;
    }
    lvItem.pszText = pParam->pTitle;
    ListView_InsertItem(hInfo, &lvItem);
}
static void UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(PPICTUREJPEGAPP13PHOTOSHOPPRINTPARAM pParam)
{
    HWND hInfo = pParam->hListView;
    LVITEM lvItem = {
        .mask = LVIF_TEXT,
        .iItem = ListView_GetItemCount(hInfo),
    };

    lvItem.iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD;
    lvItem.pszText = pParam->pName;
    ListView_InsertItem(hInfo, &lvItem);

    lvItem.iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE;
    lvItem.pszText = pParam->pValue;
    ListView_SetItem(hInfo, &lvItem);
}
static HANPSTR UpdateSegmentInfoWindow_APP13_PrintBitTrueFalse(uint32_t cFlag)
{
    HANPSTR pRet;

    if (0 == cFlag) { pRet = sg_pTrueFalse[0]; }
    else { pRet = sg_pTrueFalse[1]; }

    return pRet;
}
static void UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM03ED(PPICTUREJPEGAPP13PHOTOSHOP pPsData, HWND hListView)
{
    const uint8_t* pData = pPsData->pData;
    PICTUREJPEGAPP13PHOTOSHOPPRINTPARAM printData = {
        .hListView = hListView,
    };
    uint32_t pResolution[2];
    uint16_t pUnit[2];
    
    HAN_strncpy(printData.pTitle, TEXT("分辨率信息"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopTitle(&printData);
    
    pResolution[0] = ReadJpegData2ByteMSB(&pData[0]);
    pUnit[0] = ReadJpegData2ByteMSB(&pData[6]);
    pResolution[1] = ReadJpegData2ByteMSB(&pData[8]);
    pUnit[1] = ReadJpegData2ByteMSB(&pData[14]);
    
    HAN_strncpy(printData.pName, TEXT("水平分辨率"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), pResolution[0]);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
    HAN_strncpy(printData.pName, TEXT("水平分辨率单位"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), pUnit[0], GetJpeg_APP13_PhotoshopResolutionUnitName(pUnit[0]));
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
    HAN_strncpy(printData.pName, TEXT("垂直分辨率"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), pResolution[1]);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
    HAN_strncpy(printData.pName, TEXT("垂直分辨率单位"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), pUnit[1], GetJpeg_APP13_PhotoshopResolutionUnitName(pUnit[1]));
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
}
static void UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM03F3(PPICTUREJPEGAPP13PHOTOSHOP pPsData, HWND hListView)
{
    const uint8_t* pData = pPsData->pData;
    PICTUREJPEGAPP13PHOTOSHOPPRINTPARAM printData = {
        .hListView = hListView,
    };
    
    HAN_strncpy(printData.pTitle, TEXT("打印标志"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopTitle(&printData);

    HAN_strncpy(printData.pName, TEXT("标签"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%s"), UpdateSegmentInfoWindow_APP13_PrintBitTrueFalse(pData[0]));
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);

    HAN_strncpy(printData.pName, TEXT("角落裁剪"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%s"), UpdateSegmentInfoWindow_APP13_PrintBitTrueFalse(pData[1]));
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);

    HAN_strncpy(printData.pName, TEXT("颜色条"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%s"), UpdateSegmentInfoWindow_APP13_PrintBitTrueFalse(pData[2]));
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);

    HAN_strncpy(printData.pName, TEXT("套准标记"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%s"), UpdateSegmentInfoWindow_APP13_PrintBitTrueFalse(pData[3]));
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);

    HAN_strncpy(printData.pName, TEXT("负片打印"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%s"), UpdateSegmentInfoWindow_APP13_PrintBitTrueFalse(pData[4]));
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);

    HAN_strncpy(printData.pName, TEXT("乳剂朝下"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%s"), UpdateSegmentInfoWindow_APP13_PrintBitTrueFalse(pData[5]));
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);

    HAN_strncpy(printData.pName, TEXT("插值"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%s"), UpdateSegmentInfoWindow_APP13_PrintBitTrueFalse(pData[6]));
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);

    HAN_strncpy(printData.pName, TEXT("描述"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%s"), UpdateSegmentInfoWindow_APP13_PrintBitTrueFalse(pData[7]));
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);

    HAN_strncpy(printData.pName, TEXT("打印标志"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%s"), UpdateSegmentInfoWindow_APP13_PrintBitTrueFalse(pData[8]));
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
}
static void UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM03F5(PPICTUREJPEGAPP13PHOTOSHOP pPsData, HWND hListView)
{
    PICTUREJPEGAPP13PHOTOSHOPPRINTPARAM printData = {
        .hListView = hListView,
    };
    
    HAN_strncpy(printData.pTitle, TEXT("颜色半色调信息"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopTitle(&printData);
    
    UpdateSegmentInfoWindow_APP13_PrintUnknownDataFormat(pPsData, &printData);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
}
static void UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM03F8(PPICTUREJPEGAPP13PHOTOSHOP pPsData, HWND hListView)
{
    PICTUREJPEGAPP13PHOTOSHOPPRINTPARAM printData = {
        .hListView = hListView,
    };
    
    HAN_strncpy(printData.pTitle, TEXT("色彩传递函数"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopTitle(&printData);
    
    UpdateSegmentInfoWindow_APP13_PrintUnknownDataFormat(pPsData, &printData);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
}
static void UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM0404(PPICTUREJPEGAPP13PHOTOSHOP pPsData, HWND hListView)
{
    const uint8_t* pData = pPsData->pData;
    HANSIZE nPsLen = pPsData->nLen;
    HANSIZE nOffset = 0;
    HANSIZE nIPTCLen;
    PICTUREJPEGAPP13PHOTOSHOPPRINTPARAM printIPTC = {
        .hListView = hListView,
    };
    PICTUREJPEGIPTCDATA iptcData;
    PICTUREJPEGIPTCDECODEDATA decodeIPTC = {
        .cCodePage = CP_ACP,
        .pIPTCData = &iptcData,
        .pName = printIPTC.pName,
        .nNameSize = HAN_PICTURE_JPEG_TEXT_BUF_SIZE,
        .pValue = printIPTC.pValue,
        .nValueSize = HAN_PICTURE_JPEG_TEXT_BUF_SIZE,
    };
    
    HAN_strncpy(printIPTC.pTitle, TEXT("IPTC"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopTitle(&printIPTC);

    while (nOffset < pPsData->nLen)
    {
        nIPTCLen = PictureJpegReadIPTC(&pData[nOffset], nPsLen - nOffset, 0x1C, &iptcData);
        if (0 < nIPTCLen)
        {
            if (TRUE == PictureJpegDecodeIPTC(&decodeIPTC)) { UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printIPTC); }
            nOffset += nIPTCLen;
        }
        else { nOffset++; }
    }
}
static void UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM0406(PPICTUREJPEGAPP13PHOTOSHOP pPsData, HWND hListView)
{
    PICTUREJPEGAPP13PHOTOSHOPPRINTPARAM printData = {
        .hListView = hListView,
    };
    
    HAN_strncpy(printData.pTitle, TEXT("JPEG质量"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopTitle(&printData);
    
    UpdateSegmentInfoWindow_APP13_PrintUnknownDataFormat(pPsData, &printData);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
}
static void UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM0408(PPICTUREJPEGAPP13PHOTOSHOP pPsData, HWND hListView)
{
    const uint8_t* pData = pPsData->pData;
    PICTUREJPEGAPP13PHOTOSHOPPRINTPARAM printData = {
        .hListView = hListView,
    };
    
    HAN_strncpy(printData.pTitle, TEXT("网格和参考线信息"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopTitle(&printData);
    
    HAN_strncpy(printData.pName, TEXT("版本"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), ReadJpegData4ByteMSB(&pData[0]));
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
    HAN_strncpy(printData.pName, TEXT("水平网格"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), ReadJpegData4ByteMSB(&pData[4]));
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
    HAN_strncpy(printData.pName, TEXT("垂直网格"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), ReadJpegData4ByteMSB(&pData[8]));
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
    HAN_strncpy(printData.pName, TEXT("网格资源数量"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), ReadJpegData4ByteMSB(&pData[12]));
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
}
static void UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM040A(PPICTUREJPEGAPP13PHOTOSHOP pPsData, HWND hListView)
{
    const uint8_t* pData = pPsData->pData;
    PICTUREJPEGAPP13PHOTOSHOPPRINTPARAM printData = {
        .hListView = hListView,
    };
    
    HAN_strncpy(printData.pTitle, TEXT("版权标志"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopTitle(&printData);
    
    HAN_strncpy(printData.pName, TEXT("版权"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), *pData, GetJpeg_APP13_PhotoshopCopyRightFlagName(*pData));
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
}
static void UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM040C(PPICTUREJPEGAPP13PHOTOSHOP pPsData, HWND hListView)
{
    const uint8_t* pData = pPsData->pData;
    uint32_t nFormat;
    HANSIZE nOffset;
    PICTUREJPEGAPP13PHOTOSHOPPRINTPARAM printData = {
        .hListView = hListView,
    };
    
    HAN_strncpy(printData.pTitle, TEXT("Photoshop缩略图"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopTitle(&printData);
    
    nOffset = 0;
    HAN_strncpy(printData.pName, TEXT("格式"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    nFormat = ReadJpegData4ByteMSB(&pData[nOffset]); nOffset += 4;
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), nFormat, GetJpeg_APP13_PhotoshopThumbnailFormatName(nFormat));
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
    HAN_strncpy(printData.pName, TEXT("缩略图宽度"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), ReadJpegData4ByteMSB(&pData[nOffset])); nOffset += 4;
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
    HAN_strncpy(printData.pName, TEXT("缩略图高度"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), ReadJpegData4ByteMSB(&pData[nOffset])); nOffset += 4;
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
    HAN_strncpy(printData.pName, TEXT("每行字节数"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u Bytes"), ReadJpegData4ByteMSB(&pData[nOffset])); nOffset += 4;
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
    HAN_strncpy(printData.pName, TEXT("总大小"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u Bytes"), ReadJpegData4ByteMSB(&pData[nOffset])); nOffset += 4;
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
    HAN_strncpy(printData.pName, TEXT("压缩后大小"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u Bytes"), ReadJpegData4ByteMSB(&pData[nOffset])); nOffset += 4;
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
    HAN_strncpy(printData.pName, TEXT("像素位数"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), ReadJpegData2ByteMSB(&pData[nOffset])); nOffset += 2;
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
    HAN_strncpy(printData.pName, TEXT("平面数"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), ReadJpegData2ByteMSB(&pData[nOffset])); nOffset += 2;
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
    HAN_strncpy(printData.pName, TEXT("缩略图数据"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    PictureJpegPrintHexData(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, &pData[nOffset], pPsData->nLen - (uint32_t)nOffset);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
}
static void UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM040D(PPICTUREJPEGAPP13PHOTOSHOP pPsData, HWND hListView)
{
    PICTUREJPEGAPP13PHOTOSHOPPRINTPARAM printData = {
        .hListView = hListView,
    };
    
    HAN_strncpy(printData.pTitle, TEXT("全局光照角度"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopTitle(&printData);
    
    HAN_strncpy(printData.pName, TEXT("角度"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u°"), ReadJpegData4ByteMSB(pPsData->pData));
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
}
static void UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM0414(PPICTUREJPEGAPP13PHOTOSHOP pPsData, HWND hListView)
{
    PICTUREJPEGAPP13PHOTOSHOPPRINTPARAM printData = {
        .hListView = hListView,
    };
    
    HAN_strncpy(printData.pTitle, TEXT("切片ID基准值"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopTitle(&printData);
    
    HAN_strncpy(printData.pName, TEXT("基准值"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), ReadJpegData4ByteMSB(pPsData->pData));
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
}
static void UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM0419(PPICTUREJPEGAPP13PHOTOSHOP pPsData, HWND hListView)
{
    PICTUREJPEGAPP13PHOTOSHOPPRINTPARAM printData = {
        .hListView = hListView,
    };
    
    HAN_strncpy(printData.pTitle, TEXT("全局光照高度"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopTitle(&printData);
    
    HAN_strncpy(printData.pName, TEXT("高度"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u°"), ReadJpegData4ByteMSB(pPsData->pData));
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
}
static void UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM041A(PPICTUREJPEGAPP13PHOTOSHOP pPsData, HWND hListView)
{
    PICTUREJPEGAPP13PHOTOSHOPPRINTPARAM printData = {
        .hListView = hListView,
    };
    
    HAN_strncpy(printData.pTitle, TEXT("切片信息"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopTitle(&printData);
    
    UpdateSegmentInfoWindow_APP13_PrintUnknownDataFormat(pPsData, &printData);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
}
static void UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM041E(PPICTUREJPEGAPP13PHOTOSHOP pPsData, HWND hListView)
{
    PICTUREJPEGAPP13PHOTOSHOPPRINTPARAM printData = {
        .hListView = hListView,
    };
    
    HAN_strncpy(printData.pTitle, TEXT("URL列表"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopTitle(&printData);
    
    UpdateSegmentInfoWindow_APP13_PrintUnknownDataFormat(pPsData, &printData);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
}
static void UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM0421(PPICTUREJPEGAPP13PHOTOSHOP pPsData, HWND hListView)
{
    const uint8_t* pData = pPsData->pData;
    HANSIZE nOffset;
    uint8_t bMerge;
    uint32_t nNameLen;
    WCHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    PICTUREJPEGAPP13PHOTOSHOPPRINTPARAM printData = {
        .hListView = hListView,
    };
    
    HAN_strncpy(printData.pTitle, TEXT("版本信息"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopTitle(&printData);
    
    nOffset = 0;
    HAN_strncpy(printData.pName, TEXT("版本"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), ReadJpegData4ByteMSB(&pData[nOffset])); nOffset += 4;
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
    HAN_strncpy(printData.pName, TEXT("是否包括真实的合并数据"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    bMerge = pData[nOffset]; nOffset += 1;
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%s"), UpdateSegmentInfoWindow_APP13_PrintBitTrueFalse((uint32_t)bMerge));
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
    HAN_strncpy(printData.pName, TEXT("读取器名称"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    nNameLen = ReadJpegData4ByteMSB(&pData[nOffset]); nOffset += 4;
    if (0x00 == pData[nOffset])
    {
        for (uint32_t iLoop = 0; iLoop < nNameLen; iLoop++)
        {
            pText[iLoop] = ReadJpegData2ByteMSB(&pData[nOffset]);
            nOffset += 2;
        }
        pText[nNameLen] = TEXT('\0');
        WStrToHANStr(CP_ACP, 0, pText, nNameLen + 1, printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, NULL, NULL);
    }
    else { HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, &pData[nOffset]); }
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
    HAN_strncpy(printData.pName, TEXT("写入器名称"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    nNameLen = ReadJpegData4ByteMSB(&pData[nOffset]); nOffset += 4;
    if (0x00 == pData[nOffset])
    {
        for (uint32_t iLoop = 0; iLoop < nNameLen; iLoop++)
        {
            pText[iLoop] = ReadJpegData2ByteMSB(&pData[nOffset]);
            nOffset += 2;
        }
        pText[nNameLen] = TEXT('\0');
        WStrToHANStr(CP_ACP, 0, pText, nNameLen + 1, printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, NULL, NULL);
    }
    else { HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, &pData[nOffset]); }
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
    HAN_strncpy(printData.pName, TEXT("文件版本"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), ReadJpegData4ByteMSB(&pData[nOffset])); nOffset += 4;
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
}
static void UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM0425(PPICTUREJPEGAPP13PHOTOSHOP pPsData, HWND hListView)
{
    PICTUREJPEGAPP13PHOTOSHOPPRINTPARAM printData = {
        .hListView = hListView,
    };
    
    HAN_strncpy(printData.pTitle, TEXT("IPTC摘要"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopTitle(&printData);
    
    UpdateSegmentInfoWindow_APP13_PrintUnknownDataFormat(pPsData, &printData);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
}
static void UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM0426(PPICTUREJPEGAPP13PHOTOSHOP pPsData, HWND hListView)
{
    const uint8_t* pData = pPsData->pData;
    PICTUREJPEGAPP13PHOTOSHOPPRINTPARAM printData = {
        .hListView = hListView,
    };
    uint16_t nStyle;
    uint32_t cData;
    HANFLOAT pLocation[2];
    HANFLOAT nScale;
    
    HAN_strncpy(printData.pTitle, TEXT("打印比例信息"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopTitle(&printData);

    nStyle = ReadJpegData2ByteMSB(&pData[0]);
    cData = ReadJpegData4ByteMSB(&pData[2]); memcpy(&pLocation[0], &cData, 4);
    cData = ReadJpegData4ByteMSB(&pData[6]); memcpy(&pLocation[1], &cData, 4);
    cData = ReadJpegData4ByteMSB(&pData[10]); memcpy(&nScale, &cData, 4);

    HAN_strncpy(printData.pName, TEXT("风格"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), nStyle, GetJpeg_APP13_PhotoshopPrintScaleInfoPrintStyleName(nStyle));
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
    HAN_strncpy(printData.pName, TEXT("水平位置"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%g"), pLocation[0]);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
    HAN_strncpy(printData.pName, TEXT("垂直位置"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%g"), pLocation[1]);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
    HAN_strncpy(printData.pName, TEXT("缩放比例"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%g"), nScale);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
}
static void UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM0428(PPICTUREJPEGAPP13PHOTOSHOP pPsData, HWND hListView)
{
    const uint8_t* pData = pPsData->pData;
    PICTUREJPEGAPP13PHOTOSHOPPRINTPARAM printData = {
        .hListView = hListView,
    };
    uint8_t pDouble[sizeof(HANDOUBLE)];
    HANDOUBLE kRatio;
    
    HAN_strncpy(printData.pTitle, TEXT("像素信息"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopTitle(&printData);

    HAN_strncpy(printData.pName, TEXT("版本"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), ReadJpegData4ByteMSB(pData));
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
    for (HANSIZE iLoop = 0; iLoop < sizeof(HANDOUBLE); iLoop++)
    {
        pDouble[iLoop] = pData[4 + sizeof(HANDOUBLE) - iLoop - 1];
    }
    memcpy(&kRatio, pDouble, sizeof(HANDOUBLE));
    HAN_strncpy(printData.pName, TEXT("x/y 比率"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%g"), kRatio);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
}
static void UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM043A(PPICTUREJPEGAPP13PHOTOSHOP pPsData, HWND hListView)
{
    PICTUREJPEGAPP13PHOTOSHOPPRINTPARAM printData = {
        .hListView = hListView,
    };
    
    HAN_strncpy(printData.pTitle, TEXT("打印信息"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopTitle(&printData);
    
    UpdateSegmentInfoWindow_APP13_PrintUnknownDataFormat(pPsData, &printData);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
}
static void UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM043B(PPICTUREJPEGAPP13PHOTOSHOP pPsData, HWND hListView)
{
    PICTUREJPEGAPP13PHOTOSHOPPRINTPARAM printData = {
        .hListView = hListView,
    };
    
    HAN_strncpy(printData.pTitle, TEXT("打印风格"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopTitle(&printData);
    
    UpdateSegmentInfoWindow_APP13_PrintUnknownDataFormat(pPsData, &printData);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
}
static void UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM2710(PPICTUREJPEGAPP13PHOTOSHOP pPsData, HWND hListView)
{
    const uint8_t* pData = pPsData->pData;
    PICTUREJPEGAPP13PHOTOSHOPPRINTPARAM printData = {
        .hListView = hListView,
    };
    
    HAN_strncpy(printData.pTitle, TEXT("打印标志信息"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopTitle(&printData);
    
    HAN_strncpy(printData.pName, TEXT("版本"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), ReadJpegData2ByteMSB(&pData[0]));
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
    HAN_strncpy(printData.pName, TEXT("中心裁剪标记"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), ReadJpegData2ByteMSB(&pData[2]));
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
    HAN_strncpy(printData.pName, TEXT("出血尺寸宽度"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), ReadJpegData2ByteMSB(&pData[6]));
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
    HAN_strncpy(printData.pName, TEXT("出血缩放比例"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    HAN_snprintf(printData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), ReadJpegData2ByteMSB(&pData[8]));
    UpdateSegmentInfoWindow_APP13_PrintPhotoshopData(&printData);
}
static void UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM(PPICTUREJPEGAPP13PHOTOSHOP pPsData, HWND hListView)
{
    switch (pPsData->nSrcId) {
        case 0x03ED: { UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM03ED(pPsData, hListView); } break;
        case 0x03F3: { UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM03F3(pPsData, hListView); } break;
        case 0x03F5: { UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM03F5(pPsData, hListView); } break;
        case 0x03F8: { UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM03F8(pPsData, hListView); } break;
        case 0x0404: { UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM0404(pPsData, hListView); } break;
        case 0x0406: { UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM0406(pPsData, hListView); } break;
        case 0x0408: { UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM0408(pPsData, hListView); } break;
        case 0x040A: { UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM040A(pPsData, hListView); } break;
        case 0x040C: { UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM040C(pPsData, hListView); } break;
        case 0x040D: { UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM040D(pPsData, hListView); } break;
        case 0x0414: { UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM0414(pPsData, hListView); } break;
        case 0x0419: { UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM0419(pPsData, hListView); } break;
        case 0x041A: { UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM041A(pPsData, hListView); } break;
        case 0x041E: { UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM041E(pPsData, hListView); } break;
        case 0x0421: { UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM0421(pPsData, hListView); } break;
        case 0x0425: { UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM0425(pPsData, hListView); } break;
        case 0x0426: { UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM0426(pPsData, hListView); } break;
        case 0x0428: { UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM0428(pPsData, hListView); } break;
        case 0x043A: { UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM043A(pPsData, hListView); } break;
        case 0x043B: { UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM043B(pPsData, hListView); } break;
        case 0x2710: { UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM2710(pPsData, hListView); } break;

        default: { UpdateSegmentInfoWindow_APP13_ReadPhotoshopPrintUnknown8BIM(pPsData); } break;
    }
}
static HANSIZE UpdateSegmentInfoWindow_APP13_ReadPhotoshopRead8BIM(const uint8_t* pData, HANSIZE nLen, HWND hListView)
{
    HANSIZE nRet = 0;
    HANSIZE nOffset;
    PICTUREJPEGAPP13PHOTOSHOP psData;
    HANSIZE nNameLen;

    if (!memcmp(pData, "8BIM", 4))
    {
        nOffset = 4;
        psData.nSrcId = ReadJpegData2ByteMSB(&pData[nOffset]); nOffset += 2;
        nNameLen = pData[nOffset];
        memcpy(psData.pName, &pData[nOffset + 1], nNameLen);
        psData.pName[nNameLen] = '\0';
        nNameLen++;
        if (0 != (nNameLen & 0x01)) { nNameLen++; }
        nOffset += nNameLen;
        psData.nLen = ReadJpegData4ByteMSB(&pData[nOffset]); nOffset += 4;
        psData.pData = &pData[nOffset];
        nRet = nOffset + psData.nLen;
        if (nLen < nRet) { nRet = 0; }
        else
        {
            UpdateSegmentInfoWindow_APP13_ReadPhotoshopDecode8BIM(&psData, hListView);
        }
    }

    return nRet;
}
