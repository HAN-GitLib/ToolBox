#include <string.h>
#include <Windows.h>
#include <CommCtrl.h>

#include "HAN_PictureJpegMakerNote.h"
#include "HAN_PictureJpegMakerNote_Def.h"

#include "Sony\HAN_PictureJpegMakerNoteSony.h"

#define PICTURE_JPEG_APP1_EXIF_IFD_TITLE_MAKER_NOTE         TEXT("厂商信息")

typedef enum {
    PICTURE_JPEG_APP1_EXIF_MAKER_NOTE_SONY,
    PICTURE_JPEG_APP1_EXIF_MAKER_NOTE_CNT,
} PICTUREJPEGAPP1EXIFMAKERNOTEID;

typedef struct tagPICTUREJPEGAPP1EXIFMAKERNOTEINFO {
    BOOL                        (*ReadIFD)(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
} PICTUREJPEGAPP1EXIFMAKERNOTEINFO;

static void UpdateSegmentInfoWindow_APP1_PrintMakerNoteTitle(PPICTUREJPEGSEGMENTIFDDATA ifdData, void* pParam);
static void UpdateSegmentInfoWindow_APP1_PrintMakerNoteData(PPICTUREJPEGSEGMENTIFDDATA ifdData, void* pParam);

static const PICTUREJPEGAPP1EXIFMAKERNOTEINFO sg_pMakerNoteInfo[PICTURE_JPEG_APP1_EXIF_MAKER_NOTE_CNT] = {
    [PICTURE_JPEG_APP1_EXIF_MAKER_NOTE_SONY] = {
        .ReadIFD = UpdateSegmentInfoWindow_APP1_ReadExifIFDBlockMakerNote_Sony,
    },
};

void UpdateSegmentInfoWindow_APP1_ReadExifIFDBlockMakerNote(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    BOOL bOk = FALSE;
    HANSIZE nOffset;
    PICTUREJPEGSEGMENTREADEXIF readExif;
    PICTUREJPEGAPP1EXIFMAKERNOTEPRINTTITLEPARAM printParam = {
        .hInfo = pReadExif->printIFD.pPrintParam,
        .pMaker = NULL,
        .pTag = NULL,
    };

    nOffset = pReadExif->ifdData.ifdMeta.cValue.pDataU32[0];
    readExif = *pReadExif;
    readExif.printIFD.PrintIFDTitleCallback = UpdateSegmentInfoWindow_APP1_PrintMakerNoteTitle;
    readExif.printIFD.PrintIFDDataCallback = UpdateSegmentInfoWindow_APP1_PrintMakerNoteData;
    readExif.printIFD.pPrintParam = &printParam;
    readExif.ifdData.pIFDData = &(readExif.ifdData.pTIFFData[nOffset]);
    readExif.ifdData.cIFDID = pReadExif->ifdData.ifdMeta.cTag;

    for (PICTUREJPEGAPP1EXIFMAKERNOTEID iLoop = 0; iLoop < PICTURE_JPEG_APP1_EXIF_MAKER_NOTE_CNT; iLoop++)
    {
        if (TRUE == sg_pMakerNoteInfo[iLoop].ReadIFD(&readExif))
        {
            bOk = TRUE;
            break;
        }
    }

    if (FALSE == bOk)
    {
        HAN_strcpy(readExif.ifdData.pIFDTitle, PICTURE_JPEG_APP1_EXIF_IFD_TITLE_MAKER_NOTE);
        printParam.pMaker = TEXT("无法识别");
        readExif.printIFD.PrintIFDTitleCallback(&(readExif.ifdData), readExif.printIFD.pPrintParam);
    }
}

static void UpdateSegmentInfoWindow_APP1_PrintMakerNoteTitle(PPICTUREJPEGSEGMENTIFDDATA ifdData, void* pParam)
{
    // 新的 IFD，推荐操作：打印标题，元数据计数器清0
    PPICTUREJPEGAPP1EXIFMAKERNOTEPRINTTITLEPARAM pPrintParam = (PPICTUREJPEGAPP1EXIFMAKERNOTEPRINTTITLEPARAM)pParam;
    LVITEM lvItem = {
        .mask = LVIF_TEXT,
        .iItem = ListView_GetItemCount(pPrintParam->hInfo),
        .iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD,
        .pszText = TEXT(""),
    };
    
    if (0 < lvItem.iItem) { ListView_InsertItem(pPrintParam->hInfo, &lvItem); }

    lvItem.iItem++;
    lvItem.pszText = ifdData->pIFDTitle;
    if (0 == ifdData->nIFDBlockCnt) { HAN_strncpy(ifdData->pIFDTitle, PICTURE_JPEG_APP1_EXIF_IFD_TITLE_MAKER_NOTE, HAN_PICTURE_JPEG_TEXT_BUF_SIZE); }
    else
    {
        HAN_snprintf(
            ifdData->pIFDTitle,
            HAN_PICTURE_JPEG_TEXT_BUF_SIZE,
            PICTURE_JPEG_APP1_EXIF_IFD_TITLE_MAKER_NOTE TEXT(" - ") TEXT(HANSIZE_PRINT_FORMAT),
            ifdData->nIFDBlockCnt);
    }
    ListView_InsertItem(pPrintParam->hInfo, &lvItem);

    ifdData->nMetaId = 0;

    if (NULL != pPrintParam->pMaker)
    {
        HAN_strncpy(ifdData->pName, TEXT("厂商"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
        HAN_strncpy(ifdData->pValue, pPrintParam->pMaker, HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
        UpdateSegmentInfoWindow_APP1_PrintMakerNoteData(ifdData, pParam);
    }
    if (NULL != pPrintParam->pTag)
    {
        HAN_strncpy(ifdData->pName, TEXT("标签"), HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
        HAN_strncpy(ifdData->pValue, pPrintParam->pTag, HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
        UpdateSegmentInfoWindow_APP1_PrintMakerNoteData(ifdData, pParam);
    }
}
static void UpdateSegmentInfoWindow_APP1_PrintMakerNoteData(PPICTUREJPEGSEGMENTIFDDATA ifdData, void* pParam)
{
    // 新的元数据，推荐操作：打印内容，计数器累加。如果需要忽略某些 Tag，则可以什么都不做，包括计数器累加
    PPICTUREJPEGAPP1EXIFMAKERNOTEPRINTTITLEPARAM pPrintParam = (PPICTUREJPEGAPP1EXIFMAKERNOTEPRINTTITLEPARAM)pParam;
    LVFINDINFO lvFind = { .flags = LVFI_STRING, .psz = ifdData->pIFDTitle, };
    HANINT nListId = ListView_FindItem(pPrintParam->hInfo, -1, &lvFind) + 1 + (HANINT)(ifdData->nMetaId);
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

    ListView_InsertItem(pPrintParam->hInfo, &lvName);
    ListView_SetItem(pPrintParam->hInfo, &lvValue);
    (ifdData->nMetaId)++;
}
