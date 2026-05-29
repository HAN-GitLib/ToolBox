#include <string.h>
#include <Windows.h>
#include <CommCtrl.h>

#include "HAN_PictureJpegICCProfile.h"
#include "..\..\..\HAN_PictureJpegLib.h"
#include "..\..\..\..\ICCProfile\HAN_PictureICCProfile.h"

static BOOL UpdateSegmentInfoWindow_APP2_ReadICCProfileDecodeICCProfile(const uint8_t* pData, uint16_t nLen, HWND hListView);
static void UpdateSegmentInfoWindow_APP13_PrintICCProfileData(HANPSTR pName, HANPSTR pValue, HWND hListView);
static void UpdateSegmentInfoWindow_APP13_PrintChars(HANPSTR pText, const uint8_t pStr[4]);
static HANFLOAT UpdateSegmentInfoWindow_APP2_ReadICCProfileDecodes15Fixed16(uint32_t cCode);

BOOL UpdateSegmentInfoWindow_APP2_ReadICCProfile(PCPICTUREJPEGSEGMENT pSegment, HWND hListView)
{
    BOOL bRet = FALSE;
    uint8_t pHeader[12] = "ICC_PROFILE";
    HANPCSTR pSegmentName = TEXT("ICC配置文件");
    HANCHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    HANSIZE nHeaderSize = sizeof(pHeader);
    LVCOLUMN lvValue = { .mask = LVCF_TEXT, };

    if (!memcmp(pHeader, pSegment->pData, nHeaderSize))
    {
        HAN_snprintf( pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%s（序号%u，总数%u）"), pSegmentName, pSegment->pData[nHeaderSize], pSegment->pData[nHeaderSize + 1]);
        lvValue.pszText = pText;
        ListView_SetColumn(hListView, PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE, &lvValue);

        UpdateSegmentInfoWindow_APP2_ReadICCProfileDecodeICCProfile(
            &(pSegment->pData)[nHeaderSize + 2],
            pSegment->nLength - (uint16_t)(nHeaderSize + 2),
            hListView
        );

        bRet = TRUE;
    }

    return bRet;
}

static BOOL UpdateSegmentInfoWindow_APP2_ReadICCProfileDecodeICCProfile(const uint8_t* pData, uint16_t nLen, HWND hListView)
{
    BOOL bRet = FALSE;
    HANCHAR pName[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    HANCHAR pValue[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    size_t nICCSize;
    HANDLE hHeap = GetProcessHeap();
    PICCPROFILEDATA pICCData;
    PICCPROFILEDATA pICCTempData;

    nICCSize = 0;
    pICCData = HANWinHeapAlloc(hHeap, NULL, sizeof(ICCPROFILEDATA));
    if (NULL != pICCData) { nICCSize = DecodeICCProfileHeader(pData, nLen, &(pICCData->pfHeader)); }
    if (0 < nICCSize) 
    {
        pICCTempData = HANWinHeapAlloc(hHeap, pICCData, nICCSize);
        if (NULL != pICCTempData)
        {
            pICCData = pICCTempData;
            DecodeICCProfileData(pData, nLen, pICCData);
            UpdateSegmentInfoWindow_APP13_PrintChars(pValue, pICCData->pfHeader.pCMMType);
            UpdateSegmentInfoWindow_APP13_PrintICCProfileData(TEXT("颜色管理模块类型"), pValue, hListView);
            HAN_snprintf(pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%08X"), pICCData->pfHeader.nVersion);
            UpdateSegmentInfoWindow_APP13_PrintICCProfileData(TEXT("版本号"), pValue, hListView);
            UpdateSegmentInfoWindow_APP13_PrintChars(pValue, pICCData->pfHeader.pClass);
            UpdateSegmentInfoWindow_APP13_PrintICCProfileData(TEXT("配置文件类别"), pValue, hListView);
            UpdateSegmentInfoWindow_APP13_PrintChars(pValue, pICCData->pfHeader.pColorSpace);
            UpdateSegmentInfoWindow_APP13_PrintICCProfileData(TEXT("设备色彩空间"), pValue, hListView);
            UpdateSegmentInfoWindow_APP13_PrintChars(pValue, pICCData->pfHeader.pPCS);
            UpdateSegmentInfoWindow_APP13_PrintICCProfileData(TEXT("PCS"), pValue, hListView);
            HAN_snprintf(pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u/%u/%u - %02u:%02u:%02u"),
                pICCData->pfHeader.dateTime.nYear, pICCData->pfHeader.dateTime.nMonth, pICCData->pfHeader.dateTime.nDay,
                pICCData->pfHeader.dateTime.nHour, pICCData->pfHeader.dateTime.nMinute, pICCData->pfHeader.dateTime.nSecond
            );
            UpdateSegmentInfoWindow_APP13_PrintICCProfileData(TEXT("创建时间"), pValue, hListView);
            UpdateSegmentInfoWindow_APP13_PrintChars(pValue, pICCData->pfHeader.pSignature);
            UpdateSegmentInfoWindow_APP13_PrintICCProfileData(TEXT("文件签名"), pValue, hListView);
            UpdateSegmentInfoWindow_APP13_PrintChars(pValue, pICCData->pfHeader.pPrimaryPlatform);
            UpdateSegmentInfoWindow_APP13_PrintICCProfileData(TEXT("目标平台"), pValue, hListView);
            HAN_snprintf(pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%08X"), pICCData->pfHeader.cFlags);
            UpdateSegmentInfoWindow_APP13_PrintICCProfileData(TEXT("颜色管理模块标志"), pValue, hListView);
            UpdateSegmentInfoWindow_APP13_PrintChars(pValue, pICCData->pfHeader.pDeviceManufacturer);
            UpdateSegmentInfoWindow_APP13_PrintICCProfileData(TEXT("设备制造商"), pValue, hListView);
            UpdateSegmentInfoWindow_APP13_PrintChars(pValue, pICCData->pfHeader.pDeviceModel);
            UpdateSegmentInfoWindow_APP13_PrintICCProfileData(TEXT("设备型号"), pValue, hListView);
            HAN_snprintf(pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%08X, %08X"), pICCData->pfHeader.pDeviceAttributes[0], pICCData->pfHeader.pDeviceAttributes[1]);
            UpdateSegmentInfoWindow_APP13_PrintICCProfileData(TEXT("设备属性"), pValue, hListView);
            HAN_snprintf(pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), pICCData->pfHeader.cRenderingIntent);
            UpdateSegmentInfoWindow_APP13_PrintICCProfileData(TEXT("渲染意图"), pValue, hListView);
            HAN_snprintf(pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%g, %g, %g"),
                UpdateSegmentInfoWindow_APP2_ReadICCProfileDecodes15Fixed16(pICCData->pfHeader.pPCSIlluminant[0]),
                UpdateSegmentInfoWindow_APP2_ReadICCProfileDecodes15Fixed16(pICCData->pfHeader.pPCSIlluminant[1]),
                UpdateSegmentInfoWindow_APP2_ReadICCProfileDecodes15Fixed16(pICCData->pfHeader.pPCSIlluminant[2])
            );
            UpdateSegmentInfoWindow_APP13_PrintICCProfileData(TEXT("光源XYZ值"), pValue, hListView);
            UpdateSegmentInfoWindow_APP13_PrintChars(pValue, pICCData->pfHeader.pCreator);
            UpdateSegmentInfoWindow_APP13_PrintICCProfileData(TEXT("创建者"), pValue, hListView);

            pValue[0] = TEXT('\0');
            UpdateSegmentInfoWindow_APP13_PrintICCProfileData(TEXT("条目"), pValue, hListView);
            for (uint32_t iLoop = 0; iLoop < pICCData->nTagNum; iLoop++)
            {
                UpdateSegmentInfoWindow_APP13_PrintChars(pName, pICCData->pTag[iLoop].pTagId);
                PictureJpegPrintHexData(pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, pICCData->pTag[iLoop].pData, pICCData->pTag[iLoop].nLen);
                UpdateSegmentInfoWindow_APP13_PrintICCProfileData(pName, pValue, hListView);
            }

            bRet = TRUE;
        }
    }

    if (NULL != pICCData) { HANWinHeapFree(hHeap, 0, pICCData); }

    return bRet;
}
static void UpdateSegmentInfoWindow_APP13_PrintICCProfileData(HANPSTR pName, HANPSTR pValue, HWND hListView)
{
    LVITEM lvItem = {
        .mask = LVIF_TEXT,
        .iItem = ListView_GetItemCount(hListView),
    };

    lvItem.iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD;
    lvItem.pszText = pName;
    ListView_InsertItem(hListView, &lvItem);

    lvItem.iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE;
    lvItem.pszText = pValue;
    ListView_SetItem(hListView, &lvItem);
}
static void UpdateSegmentInfoWindow_APP13_PrintChars(HANPSTR pText, const uint8_t pStr[4])
{
    HAN_snprintf(pText, 5, TEXT("%c%c%c%c"), pStr[0], pStr[1], pStr[2], pStr[3]);
}
static HANFLOAT UpdateSegmentInfoWindow_APP2_ReadICCProfileDecodes15Fixed16(uint32_t cCode)
{
    HANFLOAT nRet;
    uint16_t nInt;
    uint16_t nDec;

    if (0x80000000 == cCode) { nRet = -32768.0; }
    else
    {
        nInt = (uint16_t)(cCode >> 16);
        nDec = (uint16_t)cCode;
        nRet = (HANFLOAT)nInt + ((HANFLOAT)nDec / (HANFLOAT)65536);
    }

    return nRet;
}
