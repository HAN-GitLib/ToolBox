#include <string.h>
#include <math.h>
#include <Windows.h>
#include <CommCtrl.h>

#include "HAN_PictureJpegAdobe.h"

typedef enum {
    ADOBE_COLOR_TRANSFORM_1,
    ADOBE_COLOR_TRANSFORM_2,
    ADOBE_COLOR_TRANSFORM_DEFAULT,
    ADOBE_COLOR_TRANSFORM_CNT,
} ADOBECOLORTRANSFORM;

static HANPSTR GetJpeg_APP14_AdobeColorTransformName(uint8_t nTransform);

static const HANPSTR sg_pColorTransformName[ADOBE_COLOR_TRANSFORM_CNT] = {
    [ADOBE_COLOR_TRANSFORM_1] = TEXT("YCbCr"),
    [ADOBE_COLOR_TRANSFORM_2] = TEXT("YCCK"),
    [ADOBE_COLOR_TRANSFORM_DEFAULT] = TEXT("未知"),
};

BOOL UpdateSegmentInfoWindow_APP14_ReadAdobe(PCPICTUREJPEGSEGMENT pSegment, HWND hListView)
{
    BOOL bRet = FALSE;
    CHAR pHeader[6] = "Adobe";
    HANCHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    const uint8_t* pData = pSegment->pData;
    LVCOLUMN lvTitle = { .mask = LVCF_TEXT, };
    LVITEM lvName = {
        .mask = LVIF_TEXT,
        .iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD,
    };
    LVITEM lvValue = {
        .mask = LVIF_TEXT,
        .iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE,
        .pszText = pText,
    };


    if (!memcmp(pHeader, pData, 5))
    {
        HAN_snprintf(pText, ArrLen(pText), HANPSTR_PRINT_PCHAR_FORMAT, pHeader);
        lvTitle.pszText = pText;
        ListView_SetColumn(hListView, PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE, &lvTitle);

        lvName.iItem = 0;
        lvValue.iItem = 0;
        lvName.pszText = TEXT("DCT编码版本");
        HAN_snprintf(pText, ArrLen(pText), TEXT("%u"), ReadJpegData2ByteMSB(&pData[5]));
        ListView_InsertItem(hListView, &lvName);
        ListView_SetItem(hListView, &lvValue);
        lvName.iItem = 1;
        lvValue.iItem = 1;
        lvName.pszText = TEXT("标志0");
        HAN_snprintf(pText, ArrLen(pText), TEXT("%04X"), ReadJpegData2ByteMSB(&pData[7]));
        ListView_InsertItem(hListView, &lvName);
        ListView_SetItem(hListView, &lvValue);
        lvName.iItem = 2;
        lvValue.iItem = 2;
        lvName.pszText = TEXT("标志1");
        HAN_snprintf(pText, ArrLen(pText), TEXT("%04X"), ReadJpegData2ByteMSB(&pData[9]));
        ListView_InsertItem(hListView, &lvName);
        ListView_SetItem(hListView, &lvValue);
        lvName.iItem = 3;
        lvValue.iItem = 3;
        lvName.pszText = TEXT("色彩变换");
        HAN_snprintf(pText, ArrLen(pText), TEXT("%u（%s）"), pData[11], GetJpeg_APP14_AdobeColorTransformName(pData[11]));
        ListView_InsertItem(hListView, &lvName);
        ListView_SetItem(hListView, &lvValue);

        bRet = TRUE;
    }

    return bRet;
}

static HANPSTR GetJpeg_APP14_AdobeColorTransformName(uint8_t nTransform)
{
    HANPSTR pRet;

    switch(nTransform) {
        case 1: { pRet = sg_pColorTransformName[ADOBE_COLOR_TRANSFORM_1]; } break;
        case 2: { pRet = sg_pColorTransformName[ADOBE_COLOR_TRANSFORM_2]; } break;
        default: { pRet = sg_pColorTransformName[ADOBE_COLOR_TRANSFORM_DEFAULT]; } break;
    }

    return pRet;
}
