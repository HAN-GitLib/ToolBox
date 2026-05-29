#ifndef HAN_PICTURE_JPEG_APP1_XMP_H
#define HAN_PICTURE_JPEG_APP1_XMP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\..\..\HAN_PictureJpegDef.h"
#include "..\..\..\HAN_PictureJpegSegment.h"

typedef enum {
    PICTURE_JPEG_APPn_SEGMENT_FIELD_ADOBE_XMP_TAG_CREATE_DATE,
    PICTURE_JPEG_APPn_SEGMENT_FIELD_ADOBE_XMP_TAG_MODIFY_DATE,
    PICTURE_JPEG_APPn_SEGMENT_FIELD_ADOBE_XMP_TAG_META_DATA_DATE,
    PICTURE_JPEG_APPn_SEGMENT_FIELD_ADOBE_XMP_TAG_CREATOR_TOOL,
    PICTURE_JPEG_APPn_SEGMENT_FIELD_ADOBE_XMP_TAG_CNT,
} PICTUREJPEGAPPnSEGMENTFIELDADOBEXMP;

typedef struct tagPICTUREJPEGSEGMENTADOBEXMPELEMENT {
    CHAR                                pNameSpace[HAN_PICTURE_JPEG_TEXT_BUF_SIZE]; // 打印命名空间时，存放了新名字。打印元素时，存放了元素所属名字
    CHAR                                pTag[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];       // 去掉了命名空间的标签名
    CHAR                                pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];      // 标签对应的文本内容
    HANCHAR                             pTitle[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];     // 要打印到窗口上的标题
    HANCHAR                             pField[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];     // 要打印到窗口上的域名
    HANCHAR                             pValue[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];     // 要打印到窗口上的值
} PICTUREJPEGSEGMENTADOBEXMPELEMENT, * PPICTUREJPEGSEGMENTADOBEXMPELEMENT;

typedef struct tagPICTUREJPEGSEGMENTPRINTXML {
    void                                (*PrintXMLTitleCallback)(PPICTUREJPEGSEGMENTADOBEXMPELEMENT xmlElement, void* pParam);
    void                                (*PrintXMLDataCallback)(PPICTUREJPEGSEGMENTADOBEXMPELEMENT xmlElement, void* pParam);
    void*                               pPrintParam;
} PICTUREJPEGSEGMENTPRINTXML, * PPICTUREJPEGSEGMENTPRINTXML;

typedef struct tagPICTUREJPEGSEGMENTREADXML {
    PICTUREJPEGSEGMENTADOBEXMPELEMENT   xmlElement;
    PPICTUREJPEGSEGMENTPRINTXML         printXML;
} PICTUREJPEGSEGMENTREADXML, * PPICTUREJPEGSEGMENTREADXML;

BOOL UpdateSegmentInfoWindow_APP1_ReadAdobeXMP(PCPICTUREJPEGSEGMENT pSegment, HWND hListView);

#ifdef __cplusplus
}
#endif

#endif
