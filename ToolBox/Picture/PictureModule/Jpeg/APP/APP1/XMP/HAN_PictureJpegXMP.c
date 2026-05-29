#include <string.h>
#include <Windows.h>
#include <CommCtrl.h>

#include "HAN_PictureJpegXMP.h"
#include "..\..\..\HAN_PictureJpegSegment.h"

typedef enum {
    PICTURE_JPEG_APP1_XML_NAMESPACE_EXIF,
    PICTURE_JPEG_APP1_XML_NAMESPACE_TIFF,
    PICTURE_JPEG_APP1_XML_NAMESPACE_PDF,
    PICTURE_JPEG_APP1_XML_NAMESPACE_PHOTOSHOP,
    PICTURE_JPEG_APP1_XML_NAMESPACE_XAP,
    PICTURE_JPEG_APP1_XML_NAMESPACE_STREF,
    PICTURE_JPEG_APP1_XML_NAMESPACE_DC,
    PICTURE_JPEG_APP1_XML_NAMESPACE_CNT,
} PICTUREJPEGAPP1XMLNAMESPACE;

typedef struct tagPICTUREJPEGAPP1XMLESCAPEINFO {
    CHAR            cChar;
    PCHAR           pEscapeStr;
    HANSIZE         nLen;
} PICTUREJPEGAPP1XMLESCAPEINFO;

typedef struct tagPICTUREJPEGAPP1XMLELEMENT {
    PCHAR           pTag;
    HANSIZE         nTagLen;
    PCHAR           pText;
    HANSIZE         nTextLen;
} PICTUREJPEGAPP1XMLELEMENT, * PPICTUREJPEGAPP1XMLELEMENT;

typedef struct tagPICTUREJPEGAPP1XMLNAMESPACEINFO {
    PCCH            pNameSpace;
    HANPCSTR        pTitle;
    void            (*TagToFiled)(PPICTUREJPEGSEGMENTREADXML readXML);
    void            (*TextToValue)(PPICTUREJPEGSEGMENTREADXML readXML);
} PICTUREJPEGAPP1XMLNAMESPACEINFO;

typedef struct tagPICTUREJPEGAPP1XMLTAGINFO {
    PCCH            pTag;
    HANPSTR         pField;
} PICTUREJPEGAPP1XMLTAGINFO;

static void UpdateSegmentInfoWindow_APP1_PrintAdobeXMPNameSpace(PPICTUREJPEGSEGMENTADOBEXMPELEMENT xmpElement, void* pParam);
static void UpdateSegmentInfoWindow_APP1_PrintAdobeXMPElement(PPICTUREJPEGSEGMENTADOBEXMPELEMENT xmpElement, void* pParam);

static inline HANINT UpdateSegmentInfoWindow_GetIdLines(HWND hListView, HANINT nId);
static inline void UpdateSegmentInfoWindow_SetIdLines(HWND hListView, HANINT nId, HANINT nLines);
static inline void UpdateSegmentInfoWindow_IdLinesInc(HWND hListView, HANINT nId);

static void UpdateSegmentInfoWindow_APP1_XMPReadXMLMain(const uint8_t* pData, HANSIZE nLen, PPICTUREJPEGSEGMENTPRINTXML printXML);

static BOOL UpdateSegmentInfoWindow_APP1_XMPReadXMLReadTag(const uint8_t* pSrc, HANPSIZE pSrcLen, PCHAR pDest, HANPSIZE pDestLen);
static BOOL UpdateSegmentInfoWindow_APP1_XMPReadXMLReadText(const uint8_t* pSrc, HANPSIZE pSrcLen, PCHAR pDest, HANPSIZE pDestLen);

static HANSIZE UpdateSegmentInfoWindow_APP1_XMPReadXMLCopyEscapeStr(const uint8_t* pData, HANSIZE nLen, PCHAR pEscapeChar);

static void UpdateSegmentInfoWindow_APP1_XMPReadXMLDecodeTag(PCCH pTag, PPICTUREJPEGSEGMENTREADXML readXML);
static void UpdateSegmentInfoWindow_APP1_XMPReadXMLDecodeRdfDescription(PCCH pDescription, PPICTUREJPEGSEGMENTREADXML readXML);

static inline PICTUREJPEGAPP1XMLNAMESPACE UpdateSegmentInfoWindow_APP1_XMPReadXMLGetNameSpaceId(PCCH pNameSpace);

static void UpdateSegmentInfoWindow_APP1_XMPReadXMLDefaultTagToField(PPICTUREJPEGSEGMENTREADXML readXML);
static void UpdateSegmentInfoWindow_APP1_XMPReadXMLExifTagToField(PPICTUREJPEGSEGMENTREADXML readXML);
static void UpdateSegmentInfoWindow_APP1_XMPReadXMLTIFFTagToField(PPICTUREJPEGSEGMENTREADXML readXML);
static void UpdateSegmentInfoWindow_APP1_XMPReadXMLXmpTagToField(PPICTUREJPEGSEGMENTREADXML readXML);

static void UpdateSegmentInfoWindow_APP1_XMPReadXMLDefaultTextToValue(PPICTUREJPEGSEGMENTREADXML readXML);
static void UpdateSegmentInfoWindow_APP1_XMPReadXMLExifTextToValue(PPICTUREJPEGSEGMENTREADXML readXML);
static void UpdateSegmentInfoWindow_APP1_XMPReadXMLTIFFTextToValue(PPICTUREJPEGSEGMENTREADXML readXML);

static void UpdateSegmentInfoWindow_APP1_XMPReadXMLPrintNumber(HANPSTR pDest, HANSIZE nDestLen, PCCH pSrc, HANPCSTR pUnit);

static const PICTUREJPEGAPP1XMLESCAPEINFO sg_pEscapeInfo[5] = {
    {
        .cChar = '<',
        .pEscapeStr = "&lt;",
        .nLen = sizeof("&lt;") - 1,
    },
    {
        .cChar = '>',
        .pEscapeStr = "&gt;",
        .nLen = sizeof("&gt;") - 1,
    },
    {
        .cChar = '&',
        .pEscapeStr = "&amp;",
        .nLen = sizeof("&amp;") - 1,
    },
    {
        .cChar = '\'',
        .pEscapeStr = "&apos;",
        .nLen = sizeof("&apos;") - 1,
    },
    {
        .cChar = '\"',
        .pEscapeStr = "&quot;",
        .nLen = sizeof("&quot;") - 1,
    },
};
static const PICTUREJPEGAPP1XMLNAMESPACEINFO sg_pXmlNameSpaceInfo[PICTURE_JPEG_APP1_XML_NAMESPACE_CNT] = {
    [PICTURE_JPEG_APP1_XML_NAMESPACE_EXIF] = {
        .pNameSpace = "exif",
        .pTitle = TEXT("Exif属性"),
        .TagToFiled = UpdateSegmentInfoWindow_APP1_XMPReadXMLExifTagToField,
        .TextToValue = UpdateSegmentInfoWindow_APP1_XMPReadXMLExifTextToValue,
    },
    [PICTURE_JPEG_APP1_XML_NAMESPACE_TIFF] = {
        .pNameSpace = "tiff",
        .pTitle = TEXT("TIFF属性"),
        .TagToFiled = UpdateSegmentInfoWindow_APP1_XMPReadXMLTIFFTagToField,
        .TextToValue = UpdateSegmentInfoWindow_APP1_XMPReadXMLTIFFTextToValue,
    },
    [PICTURE_JPEG_APP1_XML_NAMESPACE_PDF] = {
        .pNameSpace = "pdf",
        .pTitle = TEXT("pdf属性"),
        .TagToFiled = UpdateSegmentInfoWindow_APP1_XMPReadXMLDefaultTagToField,
        .TextToValue = UpdateSegmentInfoWindow_APP1_XMPReadXMLDefaultTextToValue,
    },
    [PICTURE_JPEG_APP1_XML_NAMESPACE_PHOTOSHOP] = {
        .pNameSpace = "photoshop",
        .pTitle = TEXT("Photoshop属性"),
        .TagToFiled = UpdateSegmentInfoWindow_APP1_XMPReadXMLDefaultTagToField,
        .TextToValue = UpdateSegmentInfoWindow_APP1_XMPReadXMLDefaultTextToValue,
    },
    [PICTURE_JPEG_APP1_XML_NAMESPACE_XAP] = {
        .pNameSpace = "xap",
        .pTitle = TEXT("XMP基本属性"),
        .TagToFiled = UpdateSegmentInfoWindow_APP1_XMPReadXMLXmpTagToField,
        .TextToValue = UpdateSegmentInfoWindow_APP1_XMPReadXMLDefaultTextToValue,
    },
    [PICTURE_JPEG_APP1_XML_NAMESPACE_STREF] = {
        .pNameSpace = "stRef",
        .pTitle = TEXT("XMP扩展属性"),
        .TagToFiled = UpdateSegmentInfoWindow_APP1_XMPReadXMLDefaultTagToField,
        .TextToValue = UpdateSegmentInfoWindow_APP1_XMPReadXMLDefaultTextToValue,
    },
    [PICTURE_JPEG_APP1_XML_NAMESPACE_DC] = {
        .pNameSpace = "dc",
        .pTitle = TEXT("都柏林核心属性"),
        .TagToFiled = UpdateSegmentInfoWindow_APP1_XMPReadXMLDefaultTagToField,
        .TextToValue = UpdateSegmentInfoWindow_APP1_XMPReadXMLDefaultTextToValue,
    },
};
static const PICTUREJPEGAPP1XMLTAGINFO sg_pXmlTagInfo[PICTURE_JPEG_APPn_SEGMENT_FIELD_ADOBE_XMP_TAG_CNT] = {
    [PICTURE_JPEG_APPn_SEGMENT_FIELD_ADOBE_XMP_TAG_CREATE_DATE] = {
        .pTag = "CreateDate",
        .pField = TEXT("创建时间"),
    },
    [PICTURE_JPEG_APPn_SEGMENT_FIELD_ADOBE_XMP_TAG_MODIFY_DATE] = {
        .pTag = "ModifyDate",
        .pField = TEXT("图片修改时间"),
    },
    [PICTURE_JPEG_APPn_SEGMENT_FIELD_ADOBE_XMP_TAG_META_DATA_DATE] = {
        .pTag = "MetadataDate",
        .pField = TEXT("元数据修改时间"),
    },
    [PICTURE_JPEG_APPn_SEGMENT_FIELD_ADOBE_XMP_TAG_CREATOR_TOOL] = {
        .pTag = "CreatorTool",
        .pField = TEXT("创建工具"),
    },
};

BOOL UpdateSegmentInfoWindow_APP1_ReadAdobeXMP(PCPICTUREJPEGSEGMENT pSegment, HWND hListView)
{
    BOOL bRet = FALSE;
    PCHAR pHeader = "http://ns.adobe.com/xap/1.0/";
    LVCOLUMN lvValue = { .mask = LVCF_TEXT, };
    HANSIZE nOffset = strlen(pHeader) + 1;
    PICTUREJPEGSEGMENTPRINTXML printXML = {
        .PrintXMLTitleCallback = UpdateSegmentInfoWindow_APP1_PrintAdobeXMPNameSpace,
        .PrintXMLDataCallback = UpdateSegmentInfoWindow_APP1_PrintAdobeXMPElement,
        .pPrintParam = hListView,
    };
    
    if (!_stricmp(pHeader, (PCCH)(pSegment->pData)))
    {
        lvValue.pszText = TEXT("Adobe XMP元数据");
        ListView_SetColumn(hListView, PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE, &lvValue);
        UpdateSegmentInfoWindow_APP1_XMPReadXMLMain(&(pSegment->pData[nOffset]), pSegment->nLength - nOffset, &printXML);
        bRet = TRUE;
    }

    return bRet;
}

static void UpdateSegmentInfoWindow_APP1_PrintAdobeXMPNameSpace(PPICTUREJPEGSEGMENTADOBEXMPELEMENT xmpElement, void* pParam)
{
    HWND hInfo = (HWND)pParam;
    LVITEM lvItem = {
        .mask = LVIF_TEXT,
        .iItem = ListView_GetItemCount(hInfo),
        .pszText = TEXT(""),
    };
    
    if (0 < lvItem.iItem) { ListView_InsertItem(hInfo, &lvItem); }
    
    lvItem.mask |= LVIF_PARAM;
    lvItem.iItem++;
    lvItem.iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD,
    lvItem.pszText = xmpElement->pTitle;
    lvItem.lParam = 0;
    ListView_InsertItem(hInfo, &lvItem);
}
static void UpdateSegmentInfoWindow_APP1_PrintAdobeXMPElement(PPICTUREJPEGSEGMENTADOBEXMPELEMENT xmpElement, void* pParam)
{
    HWND hInfo = (HWND)pParam;
    HANPSTR pBlankField = TEXT("");
    HANINT nTitleStartId;
    HANINT nTitleEndId;
    HANINT nTitleLines;
    HANINT nFieldStartId;
    HANINT nFieldLines;
    LVFINDINFO lvFindTitle = {
        .flags = LVFI_STRING,
        .psz = xmpElement->pTitle,
    };
    LVFINDINFO lvFindField = {
        .flags = LVFI_STRING,
        .psz = xmpElement->pField,
    };
    LVITEM lvField = {
        .mask = LVIF_TEXT | LVIF_PARAM,
        .iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD,
        .pszText = xmpElement->pField,
        .lParam = 0,
    };
    LVITEM lvData = {
        .mask = LVIF_TEXT,
        .iSubItem = PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE,
        .pszText = xmpElement->pValue,
    };

    nTitleStartId = ListView_FindItem(hInfo, -1, &lvFindTitle);
    if (0 <= nTitleStartId)
    {
        nTitleLines = UpdateSegmentInfoWindow_GetIdLines(hInfo, nTitleStartId);
        nTitleEndId = nTitleStartId + nTitleLines + 1;
        nFieldStartId = ListView_FindItem(hInfo, nTitleStartId, &lvFindField);
        if ((0 <= nFieldStartId) && (nFieldStartId < nTitleEndId))
        {
            nFieldLines = UpdateSegmentInfoWindow_GetIdLines(hInfo, nFieldStartId);
            lvField.iItem = nFieldStartId + nFieldLines;
            lvData.iItem = nFieldStartId + nFieldLines;
            lvField.pszText = pBlankField;
        }
        else
        {
            nFieldStartId = nTitleEndId;
            lvField.iItem = nTitleEndId;
            lvData.iItem = nTitleEndId;
        }
        
        ListView_InsertItem(hInfo, &lvField);
        ListView_SetItem(hInfo, &lvData);

        UpdateSegmentInfoWindow_IdLinesInc(hInfo, nTitleStartId);
        UpdateSegmentInfoWindow_IdLinesInc(hInfo, nFieldStartId);
    }
}

static inline HANINT UpdateSegmentInfoWindow_GetIdLines(HWND hListView, HANINT nId)
{
    LVITEM lvItem = {
        .mask = LVIF_PARAM,
        .iItem = nId,
    };

    ListView_GetItem(hListView, &lvItem);
    
    return (HANINT)(lvItem.lParam);
}
static inline void UpdateSegmentInfoWindow_SetIdLines(HWND hListView, HANINT nId, HANINT nLines)
{
    LVITEM lvItem = {
        .mask = LVIF_PARAM,
        .iItem = nId,
        .lParam = (LPARAM)nLines,
    };

    ListView_SetItem(hListView, &lvItem);
}
static inline void UpdateSegmentInfoWindow_IdLinesInc(HWND hListView, HANINT nId)
{
    HANINT nLines = UpdateSegmentInfoWindow_GetIdLines(hListView, nId);
    UpdateSegmentInfoWindow_SetIdLines(hListView, nId, nLines + 1);
}

static void UpdateSegmentInfoWindow_APP1_XMPReadXMLMain(const uint8_t* pData, HANSIZE nLen, PPICTUREJPEGSEGMENTPRINTXML printXML)
{
    HANSIZE nOffset;
    CHAR pTag[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    CHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    PICTUREJPEGSEGMENTREADXML readXML;
    HANSIZE nReadLen;
    HANSIZE nWriteLen;
    BOOL bReadOk;
    PICTUREJPEGAPP1XMLNAMESPACE eNameSpaceId;

    readXML.printXML = printXML;
    readXML.xmlElement.pNameSpace[0] = TEXT('\0');

    nOffset = 0;
    while (nOffset < nLen)
    {
        bReadOk = TRUE;

        while ((nOffset < nLen) && isspace(pData[nOffset])) { nOffset++; }

        if (nOffset < nLen)
        {
            if ('<' == pData[nOffset])
            {
                nReadLen = nLen - nOffset;
                nWriteLen = HAN_PICTURE_JPEG_TEXT_BUF_SIZE;
                bReadOk = UpdateSegmentInfoWindow_APP1_XMPReadXMLReadTag(&pData[nOffset], &nReadLen, pTag, &nWriteLen);
                if (TRUE == bReadOk) { UpdateSegmentInfoWindow_APP1_XMPReadXMLDecodeTag(pTag, &readXML); }
                nOffset += nReadLen;
            }
            else
            {
                nReadLen = nLen - nOffset;
                nWriteLen = HAN_PICTURE_JPEG_TEXT_BUF_SIZE;
                bReadOk = UpdateSegmentInfoWindow_APP1_XMPReadXMLReadText(&pData[nOffset], &nReadLen, pText, &nWriteLen);
                nOffset += nReadLen;
                if (TRUE == bReadOk)
                {
                    if ('\0' != readXML.xmlElement.pTag[0])
                    {
                        eNameSpaceId = UpdateSegmentInfoWindow_APP1_XMPReadXMLGetNameSpaceId(readXML.xmlElement.pNameSpace);
                        if (eNameSpaceId < PICTURE_JPEG_APP1_XML_NAMESPACE_CNT)
                        {
                            strcpy(readXML.xmlElement.pText, pText);
                            sg_pXmlNameSpaceInfo[eNameSpaceId].TextToValue(&readXML);
                            printXML->PrintXMLDataCallback(&(readXML.xmlElement), printXML->pPrintParam);
                        }
                    }
                }
            }
        }

        if (FALSE == bReadOk) { break; }
    }
}

static BOOL UpdateSegmentInfoWindow_APP1_XMPReadXMLReadTag(const uint8_t* pSrc, HANPSIZE pSrcLen, PCHAR pDest, HANPSIZE pDestLen)
{
    BOOL bRet = FALSE;
    BOOL bLoop;
    HANSIZE nSrcLen = *pSrcLen;
    HANSIZE nDestLen = *pDestLen;
    HANSIZE nOffset;
    HANSIZE nCopyLen;
    HANSIZE nEscapeLen;

    if ('<' == pSrc[0])
    {
        nCopyLen = 0;
        nOffset = 1;
        bLoop = TRUE;
        while (TRUE == bLoop)
        {
            if ((nOffset < nSrcLen) && (nCopyLen < nDestLen))
            {
                if ('>' == pSrc[nOffset])
                {
                    pDest[nCopyLen] = '\0';
                    *pSrcLen = nOffset + 1;
                    *pDestLen = nCopyLen + 1;
                    bLoop = FALSE;
                    bRet = TRUE;
                }
                else if ('&' == pSrc[nOffset])
                {
                    nEscapeLen = UpdateSegmentInfoWindow_APP1_XMPReadXMLCopyEscapeStr(&pSrc[nOffset], nSrcLen - nOffset, &pDest[nCopyLen]);
                    if (0 == nEscapeLen)
                    {
                        bLoop = FALSE;
                        bRet = FALSE;
                    }
                    else
                    {
                        nOffset += nEscapeLen;
                        nCopyLen++;
                    }
                }
                else
                {
                    pDest[nCopyLen] = pSrc[nOffset];
                    nCopyLen++;
                    nOffset++;
                }
            }
            else { bLoop = FALSE; }
        }
    }

    return bRet;
}
static BOOL UpdateSegmentInfoWindow_APP1_XMPReadXMLReadText(const uint8_t* pSrc, HANPSIZE pSrcLen, PCHAR pDest, HANPSIZE pDestLen)
{
    BOOL bRet = FALSE;
    BOOL bLoop = TRUE;
    HANSIZE nSrcLen = *pSrcLen;
    HANSIZE nDestLen = *pDestLen;
    HANSIZE nOffset;
    HANSIZE nCopyLen;
    HANSIZE nEscapeLen;

    nCopyLen = 0;
    nOffset = 0;
    while (TRUE == bLoop)
    {
        if ((nOffset < nSrcLen) && (nCopyLen < nDestLen))
        {
            if ('<' == pSrc[nOffset])
            {
                pDest[nCopyLen] = '\0';
                *pSrcLen = nOffset;
                *pDestLen = nCopyLen;
                bLoop = FALSE;
                bRet = TRUE;
            }
            else if ('&' == pSrc[nOffset])
            {
                nEscapeLen = UpdateSegmentInfoWindow_APP1_XMPReadXMLCopyEscapeStr(&pSrc[nOffset], nSrcLen - nOffset, &pDest[nCopyLen]);
                if (0 == nEscapeLen)
                {
                    bLoop = FALSE;
                    bRet = FALSE;
                }
                else
                {
                    nOffset += nEscapeLen;
                    nCopyLen++;
                }
            }
            else
            {
                pDest[nCopyLen] = pSrc[nOffset];
                nCopyLen++;
                nOffset++;
            }
        }
        else { bLoop = FALSE; }
    }

    return bRet;
}

static HANSIZE UpdateSegmentInfoWindow_APP1_XMPReadXMLCopyEscapeStr(const uint8_t* pData, HANSIZE nLen, PCHAR pEscapeChar)
{
    HANSIZE nRet = 0;
    HANSIZE nEscapeLen;

    for (HANSIZE iLoop = 0; iLoop < ArrLen(sg_pEscapeInfo); iLoop++)
    {
        nEscapeLen = sg_pEscapeInfo[iLoop].nLen;
        if (nEscapeLen < nLen)
        {
            if (!memcmp(pData, sg_pEscapeInfo[iLoop].pEscapeStr, nEscapeLen))
            {
                *pEscapeChar = sg_pEscapeInfo[iLoop].cChar;
                nRet = nEscapeLen;
                break;
            }
        }
    }

    return nRet;
}

static void UpdateSegmentInfoWindow_APP1_XMPReadXMLDecodeTag(PCCH pTag, PPICTUREJPEGSEGMENTREADXML readXML)
{
    CHAR pNameSpace[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    PCCH pStart = NULL;
    PCCH pEnd;
    HANSIZE nNSLen;
    PICTUREJPEGAPP1XMLNAMESPACE eNameSpaceId;

    /* 截取命名空间 */
    pEnd = strstr(pTag, ":");
    if (NULL != pEnd)
    {
        nNSLen = (uintptr_t)pEnd - (uintptr_t)pTag;
        memcpy(pNameSpace, pTag, nNSLen);
        pNameSpace[nNSLen] = '\0';
        pStart = pEnd + 1;
    }
    
    if (!strncmp("rdf:Description", pTag, 15))
    {
        UpdateSegmentInfoWindow_APP1_XMPReadXMLDecodeRdfDescription(&pTag[15], readXML);
    }
    else if (!strncmp("/rdf:Description", pTag, 16))
    {
        readXML->xmlElement.pNameSpace[0] = TEXT('\0');
    }
    else if ((!strncmp("rdf:", pTag, 4)) || (!strncmp("/rdf:", pTag, 5)))
    {
        /* rdf 属于程序内部处理，对打印没有影响，这里不做任何处理 */
    }
    else
    {
        if (NULL != pStart)
        {
            eNameSpaceId = UpdateSegmentInfoWindow_APP1_XMPReadXMLGetNameSpaceId(pNameSpace);
            if (eNameSpaceId < PICTURE_JPEG_APP1_XML_NAMESPACE_CNT)
            {
                strcpy(readXML->xmlElement.pTag, pStart);
                sg_pXmlNameSpaceInfo[eNameSpaceId].TagToFiled(readXML);
            }
            else
            {
                readXML->xmlElement.pTag[0] = '\0';
                readXML->xmlElement.pField[0] = TEXT('\0');
            }
        }
    }
}
static void UpdateSegmentInfoWindow_APP1_XMPReadXMLDecodeRdfDescription(PCCH pDescription, PPICTUREJPEGSEGMENTREADXML readXML)
{
    CHAR pNameSpace[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    PCCH pXmlNS;
    PCCH pStart;
    PCCH pEnd;
    HANSIZE nNSLen;
    PICTUREJPEGAPP1XMLNAMESPACE eNameSpaceId;

    pXmlNS = strstr(pDescription, "xmlns:");
    if (NULL != pXmlNS)
    {
        pStart = &pXmlNS[6];
        pEnd = strstr(pStart, "=");
        if (NULL != pEnd)
        {
            nNSLen = (uintptr_t)pEnd - (uintptr_t)pStart;
            if (nNSLen < HAN_PICTURE_JPEG_TEXT_BUF_SIZE)
            {
                memcpy(pNameSpace, pStart, nNSLen);
                pNameSpace[nNSLen] = '\0';
                eNameSpaceId = UpdateSegmentInfoWindow_APP1_XMPReadXMLGetNameSpaceId(pNameSpace);
                if (eNameSpaceId < PICTURE_JPEG_APP1_XML_NAMESPACE_CNT)
                {
                    strcpy(readXML->xmlElement.pNameSpace, sg_pXmlNameSpaceInfo[eNameSpaceId].pNameSpace);
                    HAN_strcpy(readXML->xmlElement.pTitle, sg_pXmlNameSpaceInfo[eNameSpaceId].pTitle);
                    readXML->printXML->PrintXMLTitleCallback(&(readXML->xmlElement), readXML->printXML->pPrintParam);
                }
            }
        }
    }
}

static inline PICTUREJPEGAPP1XMLNAMESPACE UpdateSegmentInfoWindow_APP1_XMPReadXMLGetNameSpaceId(PCCH pNameSpace)
{
    PICTUREJPEGAPP1XMLNAMESPACE eRet;

    for (eRet = 0; eRet < PICTURE_JPEG_APP1_XML_NAMESPACE_CNT; eRet++)
    {
        if (!_stricmp(pNameSpace, sg_pXmlNameSpaceInfo[eRet].pNameSpace))
        {
            break;
        }
    }

    return eRet;
}

static void UpdateSegmentInfoWindow_APP1_XMPReadXMLDefaultTagToField(PPICTUREJPEGSEGMENTREADXML readXML)
{
    HAN_snprintf(readXML->xmlElement.pField, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, readXML->xmlElement.pTag);
}
static void UpdateSegmentInfoWindow_APP1_XMPReadXMLExifTagToField(PPICTUREJPEGSEGMENTREADXML readXML)
{
    HANPCSTR pName = GetJpeg_APP1_FieldNameByExifName(readXML->xmlElement.pTag);

    if (NULL != pName)
    {
        HAN_strncpy(readXML->xmlElement.pField, pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    }
    else
    {
        HAN_snprintf(readXML->xmlElement.pField, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, readXML->xmlElement.pTag);
    }
}
static void UpdateSegmentInfoWindow_APP1_XMPReadXMLTIFFTagToField(PPICTUREJPEGSEGMENTREADXML readXML)
{
    HANPCSTR pName = GetJpeg_APP1_FieldNameByExifName(readXML->xmlElement.pTag);

    if (NULL != pName)
    {
        HAN_strncpy(readXML->xmlElement.pField, pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    }
    else
    {
        HAN_snprintf(readXML->xmlElement.pField, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, readXML->xmlElement.pTag);
    }
}
static void UpdateSegmentInfoWindow_APP1_XMPReadXMLXmpTagToField(PPICTUREJPEGSEGMENTREADXML readXML)
{
    BOOL bFind = FALSE;

    for (PICTUREJPEGAPPnSEGMENTFIELDADOBEXMP iLoop = 0; iLoop < PICTURE_JPEG_APPn_SEGMENT_FIELD_ADOBE_XMP_TAG_CNT; iLoop++)
    {
        if (!strcmp(sg_pXmlTagInfo[iLoop].pTag, readXML->xmlElement.pTag))
        {
            HAN_strncpy(readXML->xmlElement.pField, sg_pXmlTagInfo[iLoop].pField, HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
            bFind = TRUE;
            break;
        }
    }
    if (FALSE == bFind)
    {
        HAN_snprintf(readXML->xmlElement.pField, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, readXML->xmlElement.pTag);
    }
}

static void UpdateSegmentInfoWindow_APP1_XMPReadXMLDefaultTextToValue(PPICTUREJPEGSEGMENTREADXML readXML)
{
    HAN_snprintf(readXML->xmlElement.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, readXML->xmlElement.pText);
}
static void UpdateSegmentInfoWindow_APP1_XMPReadXMLExifTextToValue(PPICTUREJPEGSEGMENTREADXML readXML)
{
    PCCH pText = readXML->xmlElement.pText;
    HANPSTR pValue = readXML->xmlElement.pValue;

    if (!_stricmp("ExposureTime", readXML->xmlElement.pTag))
    {
        UpdateSegmentInfoWindow_APP1_XMPReadXMLPrintNumber(pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, pText, TEXT("秒"));
    }
    else if (!_stricmp("FNumber", readXML->xmlElement.pTag))
    {
        HAN_strcpy(pValue, TEXT("f/"));
        UpdateSegmentInfoWindow_APP1_XMPReadXMLPrintNumber(&pValue[2], HAN_PICTURE_JPEG_TEXT_BUF_SIZE - 2, pText, TEXT(""));
    }
    else
    {
        HAN_snprintf(pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, pText);
    }
}
static void UpdateSegmentInfoWindow_APP1_XMPReadXMLTIFFTextToValue(PPICTUREJPEGSEGMENTREADXML readXML)
{
    HAN_snprintf(readXML->xmlElement.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, readXML->xmlElement.pText);
}

static void UpdateSegmentInfoWindow_APP1_XMPReadXMLPrintNumber(HANPSTR pDest, HANSIZE nDestLen, PCCH pSrc, HANPCSTR pUnit)
{
    HANDOUBLE n1;
    HANDOUBLE n2;
    PCHAR p2;
    CHAR nOpt;

    n1 = strtod(pSrc, &p2);
    while (isspace(*p2)) { p2 = &p2[1]; }
    nOpt = *p2;
    if (TEXT('\0') != nOpt)
    {
        p2 = &p2[1];
        while (isspace(*p2)) { p2 = &p2[1]; }
        n2 = strtod(p2, NULL);
        switch (nOpt) {
            case '+': { n1 += n2; } break;
            case '-': { n1 -= n2; } break;
            case '*': { n1 *= n2; } break;
            case '/': { if (0 != n2) { n1 /= n2; } } break;
            default: { } break;
        }
    }
    HAN_snprintf(pDest, nDestLen, TEXT("%g%s"), n1, pUnit);
}
