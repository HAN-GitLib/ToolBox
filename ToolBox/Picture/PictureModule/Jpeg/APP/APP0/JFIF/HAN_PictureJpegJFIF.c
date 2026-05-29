#include <string.h>
#include <Windows.h>
#include <CommCtrl.h>

#include "HAN_PictureJpegJFIF.h"
#include "..\..\IFD\HAN_PictureJpegIFD.h"

BOOL UpdateSegmentInfoWindow_APP0_ReadJFIF(PCPICTUREJPEGSEGMENT pSegment, HWND hListView)
{
    BOOL bRet = FALSE;
    uint8_t pHeader[5] = { 'J', 'F', 'I', 'F', '\0', };
    HANCHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    const uint8_t* pData = pSegment->pData;
    LVCOLUMN lvValue = { .mask = LVCF_TEXT, };
    LVITEM lvItem = { .mask = LVIF_TEXT, };
    
    if (!memcmp(pHeader, pData, sizeof(pHeader)))
    {
        HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, pHeader);
        lvValue.pszText = pText;
        ListView_SetColumn(hListView, PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE, &lvValue);

        lvItem.iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD;
        for (PICTUREJPEGSEGMENTFIELD_APP0 iLoop = 0; iLoop < PICTURE_JPEG_APP0_SEGMENT_FIELD_CNT; iLoop++)
        {
            lvItem.iItem = iLoop;
            lvItem.pszText = GetJpeg_APP0_FieldName(iLoop);
            ListView_InsertItem(hListView, &lvItem);
        }
    
        lvItem.iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE;
        lvItem.pszText = pText;
        /* °æ±¾ºÅ */
        lvItem.iItem = PICTURE_JPEG_APP0_SEGMENT_FIELD_VERSION;
        HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u.%02u"), pData[5], pData[6]);
        ListView_SetItem(hListView, &lvItem);
        /* ÃÜ¶Èµ¥Î» */
        lvItem.iItem = PICTURE_JPEG_APP0_SEGMENT_FIELD_DENSITY_UNIT;
        HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u£¨%s£©"), pData[7], GetJpeg_APP0_DensityUnitName(pData[7]));
        ListView_SetItem(hListView, &lvItem);
        /* XÏñËØÃÜ¶È */
        lvItem.iItem = PICTURE_JPEG_APP0_SEGMENT_FIELD_X_DENSITY;
        HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), ReadJpegData2ByteMSB(&pData[8]));
        ListView_SetItem(hListView, &lvItem);
        /* YÏñËØÃÜ¶È */
        lvItem.iItem = PICTURE_JPEG_APP0_SEGMENT_FIELD_Y_DENSITY;
        HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), ReadJpegData2ByteMSB(&pData[10]));
        ListView_SetItem(hListView, &lvItem);
        /* ËõÂÔÍ¼XÏñËØ */
        lvItem.iItem = PICTURE_JPEG_APP0_SEGMENT_FIELD_THUMB_X_PIXEL;
        HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), pData[12]);
        ListView_SetItem(hListView, &lvItem);
        /* ËõÂÔÍ¼YÏñËØ */
        lvItem.iItem = PICTURE_JPEG_APP0_SEGMENT_FIELD_THUMB_Y_PIXEL;
        HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), pData[13]);
        ListView_SetItem(hListView, &lvItem);

        bRet = TRUE;
    }

    return bRet;
}
