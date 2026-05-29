#include <math.h>
#include <string.h>
#include <Windows.h>
#include <CommCtrl.h>

#include "HAN_PictureJpegExif.h"
#include "..\..\IFD\HAN_PictureJpegIFD.h"
#include "..\..\MakerNote\HAN_PictureJpegMakerNote.h"

#define PICTURE_JPEG_APP1_EXIF_IFD_TITLE_EXIF               TEXT("拍摄信息")
#define PICTURE_JPEG_APP1_EXIF_IFD_TITLE_GPS                TEXT("地理位置信息")
#define PICTURE_JPEG_APP1_EXIF_IFD_TITLE_INTEROPERABILITY   TEXT("IOP")

static void UpdateSegmentInfoWindow_APP1_PrintUnknownIFDTag(PPICTUREJPEGSEGMENTIFDDATA ifdData);

static void UpdateSegmentInfoWindow_APP1_ReadExifIFDBlockExif(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ReadExifIFDBlockGPS(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ReadExifIFDBlockInteroperability(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
#if 1 // IFD 根目录
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0100(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t nWidth = pReadExif->ifdData.ifdMeta.cValue.pDataU32[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_IMAGE_WIDTH));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), nWidth);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0101(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t nHeight = pReadExif->ifdData.ifdMeta.cValue.pDataU32[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_IMAGE_HEIGHT));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), nHeight);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0102(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t nDataCnt = pReadExif->ifdData.ifdMeta.nDataCnt;
    HANPSTR pValue = pReadExif->ifdData.pValue;
    uint16_t pBits[16] = { 0 };
    HANSIZE nOffset;

    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_BITS_PER_SAMPLE));
    nOffset = 0;
    for (HANSIZE iLoop = 0; iLoop < nDataCnt; iLoop++)
    {
        pBits[iLoop] = pReadExif->ifdData.ReadBytes->Read2Bytes(&(pReadExif->ifdData.pIFDData)[nOffset]);
        nOffset += 2;
    }
    nOffset = 0;
    HAN_snprintf(pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), pBits[0]);
    pValue[HAN_PICTURE_JPEG_TEXT_BUF_SIZE - 1] = TEXT('\0');
    for (HANSIZE iLoop = 1; iLoop < nDataCnt; iLoop++)
    {
        nOffset += HAN_strlen(&pValue[nOffset]);
        HAN_snprintf(&pValue[nOffset], HAN_PICTURE_JPEG_TEXT_BUF_SIZE - nOffset, TEXT(", %u"), pBits[iLoop]);
    }
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0103(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint16_t cCompression = pReadExif->ifdData.ifdMeta.cValue.pDataU16[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_COMPRESSION));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), cCompression, GetJpeg_APP1_CompressionName(cCompression));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0106(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint16_t cPI = pReadExif->ifdData.ifdMeta.cValue.pDataU16[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_PHOTOMETRIC_INTERPRETATION));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), cPI, GetJpeg_APP1_PhotometricInterpretationName(cPI));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot010E(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_IMAGE_DESCRIPTION));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, pReadExif->ifdData.pIFDData);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot010F(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_CAMERA_MAKE));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, pReadExif->ifdData.pIFDData);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0110(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    pReadExif->ifdData.exInfo.pCameraModel = pReadExif->ifdData.pIFDData;
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_CAMERA_MODEL));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, pReadExif->ifdData.pIFDData);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0112(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint16_t cOrientation = pReadExif->ifdData.ifdMeta.cValue.pDataU16[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_ORIENTATION));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), cOrientation, GetJpeg_APP1_OrientationName(cOrientation));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0115(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint16_t nNum = pReadExif->ifdData.ifdMeta.cValue.pDataU16[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_SAMPLES_PER_PIXEL));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), nNum);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot011A(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t nDisplay = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[0]);
    uint32_t nPrint = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[4]);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_RESOLUTION_H));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u / %u"), nDisplay, nPrint);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot011B(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t nDisplay = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[0]);
    uint32_t nPrint = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[4]);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_RESOLUTION_V));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u / %u"), nDisplay, nPrint);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0128(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint16_t cUnit = pReadExif->ifdData.ifdMeta.cValue.pDataU16[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_RESOLUTION_UNIT));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), cUnit, GetJpeg_APP1_ResolutionUnitName(cUnit));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0131(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_SOFTWARE));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, pReadExif->ifdData.pIFDData);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0132(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_DATETIME));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, pReadExif->ifdData.pIFDData);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot013E(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t nNum1 = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[0]);
    uint32_t nNum2 = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[4]);
    uint32_t nNum3 = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[8]);
    uint32_t nNum4 = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[12]);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_WHITE_POINT));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("白 (%g, %g)"),
        (HANDOUBLE)nNum1 / (HANDOUBLE)nNum2,
        (HANDOUBLE)nNum3 / (HANDOUBLE)nNum4
    );
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot013F(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t nNum1 = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[0]);
    uint32_t nNum2 = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[4]);
    uint32_t nNum3 = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[8]);
    uint32_t nNum4 = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[12]);
    uint32_t nNum5 = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[16]);
    uint32_t nNum6 = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[20]);
    uint32_t nNum7 = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[24]);
    uint32_t nNum8 = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[28]);
    uint32_t nNum9 = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[32]);
    uint32_t nNum10 = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[36]);
    uint32_t nNum11 = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[40]);
    uint32_t nNum12 = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[44]);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_PRIMARY_CHROMATICITIES));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("红 (%g, %g)"),
        (HANDOUBLE)nNum1 / (HANDOUBLE)nNum2,
        (HANDOUBLE)nNum3 / (HANDOUBLE)nNum4
    );
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
    pReadExif->ifdData.pName[0] = TEXT('\0');
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("绿 (%g, %g)"),
        (HANDOUBLE)nNum5 / (HANDOUBLE)nNum6,
        (HANDOUBLE)nNum7 / (HANDOUBLE)nNum8
    );
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("蓝 (%g, %g)"),
        (HANDOUBLE)nNum9 / (HANDOUBLE)nNum10,
        (HANDOUBLE)nNum11 / (HANDOUBLE)nNum12
    );
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0201(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_THUMB_POS));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%08X"), pReadExif->ifdData.ifdMeta.cValue.pDataU32[0]);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0202(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_THUMB_SIZE));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), pReadExif->ifdData.ifdMeta.cValue.pDataU32[0]);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0211(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t nNum1 = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[0]);
    uint32_t nNum2 = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[4]);
    uint32_t nNum3 = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[8]);
    uint32_t nNum4 = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[12]);
    uint32_t nNum5 = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[16]);
    uint32_t nNum6 = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[20]);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_YCBCR_COEFFICIENTS));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%g, %g, %g"),
        (HANDOUBLE)nNum1 / (HANDOUBLE)nNum2,
        (HANDOUBLE)nNum3 / (HANDOUBLE)nNum4,
        (HANDOUBLE)nNum5 / (HANDOUBLE)nNum6
    );
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0213(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint16_t cPosition = pReadExif->ifdData.ifdMeta.cValue.pDataU16[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_YCBCR_POSITION));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), cPosition, GetJpeg_APP1_YCbCrositionName(cPosition));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot8298(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_COPY_RIGHT));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, pReadExif->ifdData.pIFDData);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRootC4A5(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    HANSIZE nLen;
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_PRINT_IM));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, pReadExif->ifdData.pIFDData);
    nLen = HAN_strlen(pReadExif->ifdData.pValue);
    HAN_snprintf(&(pReadExif->ifdData.pValue)[nLen], HAN_PICTURE_JPEG_TEXT_BUF_SIZE - nLen, HANPSTR_PRINT_PCHAR_FORMAT, &(pReadExif->ifdData.pIFDData[nLen + 1]));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static BOOL UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    BOOL bRet = TRUE;

    switch (pReadExif->ifdData.ifdMeta.cTag) {
        case 0x0100: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0100(pReadExif); } break;
        case 0x0101: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0101(pReadExif); } break;
        case 0x0102: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0102(pReadExif); } break;
        case 0x0103: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0103(pReadExif); } break;
        case 0x0106: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0106(pReadExif); } break;
        case 0x010E: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot010E(pReadExif); } break;
        case 0x010F: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot010F(pReadExif); } break;
        case 0x0110: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0110(pReadExif); } break;
        case 0x0112: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0112(pReadExif); } break;
        case 0x0115: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0115(pReadExif); } break;
        case 0x011A: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot011A(pReadExif); } break;
        case 0x011B: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot011B(pReadExif); } break;
        case 0x0128: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0128(pReadExif); } break;
        case 0x0131: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0131(pReadExif); } break;
        case 0x0132: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0132(pReadExif); } break;
        case 0x013E: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot013E(pReadExif); } break;
        case 0x013F: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot013F(pReadExif); } break;
        case 0x0201: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0201(pReadExif); } break;
        case 0x0202: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0202(pReadExif); } break;
        case 0x0211: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0211(pReadExif); } break;
        case 0x0213: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot0213(pReadExif); } break;
        case 0x8769: { UpdateSegmentInfoWindow_APP1_ReadExifIFDBlockExif(pReadExif); } break;
        case 0x8825: { UpdateSegmentInfoWindow_APP1_ReadExifIFDBlockGPS(pReadExif); } break;
        case 0x8298: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot8298(pReadExif); } break;
        case 0xC4A5: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRootC4A5(pReadExif); } break;

        default: {
            UpdateSegmentInfoWindow_APP1_PrintUnknownIFDTag(&(pReadExif->ifdData));
            bRet = FALSE;
        } break;
    }

    return bRet;
}
#endif
#if 1 // Exif 目录（8769）
static void UpdateSegmentInfoWindow_APP1_PrintExifTitle(PPICTUREJPEGSEGMENTIFDDATA ifdData, void* pParam)
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
    if (0 == ifdData->nIFDBlockCnt) { HAN_strncpy(ifdData->pIFDTitle, PICTURE_JPEG_APP1_EXIF_IFD_TITLE_EXIF, HAN_PICTURE_JPEG_TEXT_BUF_SIZE); }
    else
    {
        HAN_snprintf(
            ifdData->pIFDTitle,
            HAN_PICTURE_JPEG_TEXT_BUF_SIZE,
            PICTURE_JPEG_APP1_EXIF_IFD_TITLE_EXIF TEXT(" - ") TEXT(HANSIZE_PRINT_FORMAT),
            ifdData->nIFDBlockCnt);
    }
    ListView_InsertItem(hInfo, &lvItem);

    ifdData->nMetaId = 0;
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif829A(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t nX = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[0]);
    uint32_t nY = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[4]);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_EXPOSURE_TIME));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u/%u秒"), nX, nY);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif829D(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t nX = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[0]);
    uint32_t nY = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[4]);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_F_NUMBER));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("f/%g"), (HANDOUBLE)nX / (HANDOUBLE)nY);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif8822(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint16_t cProgram = pReadExif->ifdData.ifdMeta.cValue.pDataU16[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_EXPOSURE_PROGRAM));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), cProgram, GetJpeg_APP1_ExposureProgramName(cProgram));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif8827(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint16_t* pSpeed = &(pReadExif->ifdData.ifdMeta.cValue.pDataU16[0]);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_ISO_SPEED));
    if (1 == pReadExif->ifdData.ifdMeta.nDataCnt) { HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), pSpeed[0]); }
    else { HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u / %u"), pSpeed[0], pSpeed[1]); }
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif8830(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint16_t cType = pReadExif->ifdData.ifdMeta.cValue.pDataU16[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_SENSITIVITY_TYPE));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), cType, GetJpeg_APP1_SensitivityTypeName(cType));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif8832(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t cIndex = pReadExif->ifdData.ifdMeta.cValue.pDataU32[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_REI));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), cIndex);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9000(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    CHAR pMainVersion[3];
    CHAR pSubVersion[3];
    uint8_t nMainVersion;
    memcpy(pMainVersion, &(pReadExif->ifdData.pIFDData[0]), 2);
    memcpy(pSubVersion, &(pReadExif->ifdData.pIFDData[2]), 2);
    pMainVersion[2] = '\0';
    pSubVersion[2] = '\0';
    nMainVersion = (uint8_t)strtoul(pMainVersion, NULL, 10);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_EXIF_VERSION));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u.") HANPSTR_PRINT_PCHAR_FORMAT, nMainVersion, pSubVersion);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9003(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_DATETIME_ORI));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, pReadExif->ifdData.pIFDData);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9004(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_DATETIME_DIGIT));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, pReadExif->ifdData.pIFDData);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9101(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_COMPONENT_CONFIGURATION));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%s,%s,%s,%s"),
        GetJpeg_APP1_ComponentConfigurationName(pReadExif->ifdData.pIFDData[0]),
        GetJpeg_APP1_ComponentConfigurationName(pReadExif->ifdData.pIFDData[1]),
        GetJpeg_APP1_ComponentConfigurationName(pReadExif->ifdData.pIFDData[2]),
        GetJpeg_APP1_ComponentConfigurationName(pReadExif->ifdData.pIFDData[3])
    );
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9102(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t nX = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[0]);
    uint32_t nY = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[4]);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_COMPRESSED_BITS_PER_PIXEL));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u/%u"), nX, nY);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9201(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    PICTUREJPEGIFDTYPERATIONAL rValue = {
        .rational64u = {
            [0] = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[0]),
            [1] = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[4]),
        }
    };
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_SHUTTER_SPEED_VALUE));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%g APEX"), (HANDOUBLE)(rValue.rational64s[0]) / (HANDOUBLE)(rValue.rational64s[1]));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9202(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t nX = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[0]);
    uint32_t nY = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[4]);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_APERTURE_VALUE));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%g APEX"), (HANDOUBLE)nX / (HANDOUBLE)nY);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9203(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    PICTUREJPEGIFDTYPERATIONAL rValue = {
        .rational64u = {
            [0] = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[0]),
            [1] = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[4]),
        }
    };
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_BRIGHTNESS));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%g APEX"), (HANDOUBLE)(rValue.rational64s[0]) / (HANDOUBLE)(rValue.rational64s[1]));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9204(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    int32_t nSignedX;
    int32_t nSignedY;
    uint32_t nUnsignedX = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[0]);
    uint32_t nUnsignedY = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[4]);
    memcpy(&nSignedX, &nUnsignedX, 4);
    memcpy(&nSignedY, &nUnsignedY, 4);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_EXPOSURE_BIAS));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%+g APEX"), (HANDOUBLE)nSignedX / (HANDOUBLE)nSignedY);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9205(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t nX = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[0]);
    uint32_t nY = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[4]);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_MAX_APERTURE));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("f/%g"), pow(2, (HANDOUBLE)nX / (HANDOUBLE)nY / (HANDOUBLE)2));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9207(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint16_t cMeteringMode = pReadExif->ifdData.ifdMeta.cValue.pDataU16[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_METERING_MODE));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), cMeteringMode, GetJpeg_APP1_MeteringModeName(cMeteringMode));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9208(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint16_t cLightSource = pReadExif->ifdData.ifdMeta.cValue.pDataU16[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_LIGHT_SOURCE));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), cLightSource, GetJpeg_APP1_LightSourceName(cLightSource));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9209(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint16_t cFlash = pReadExif->ifdData.ifdMeta.cValue.pDataU16[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_FLASH_FIRED));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%s"), GetJpeg_APP1_FlashFiredName(cFlash));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_FLASH_RETURN));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%s"), GetJpeg_APP1_FlashReturnName(cFlash));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_FLASH_MODE));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%s"), GetJpeg_APP1_FlashModeName(cFlash));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_FLASH_FUNCTION));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%s"), GetJpeg_APP1_FlashFunctionName(cFlash));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_FLASH_RED_EYE_MODE));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%s"), GetJpeg_APP1_FlashRedEyeModeName(cFlash));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif920A(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t nX = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[0]);
    uint32_t nY = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[4]);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_FOCAL_LENGTH));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%g毫米"), (HANDOUBLE)nX / (HANDOUBLE)nY);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9286(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_USER_COMMENT));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, pReadExif->ifdData.pIFDData);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9290(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_SUB_SEC_TIME));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, pReadExif->ifdData.pIFDData);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9291(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_SUB_SEC_TIME_ORI));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, pReadExif->ifdData.pIFDData);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9292(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_SUB_SEC_TIME_DIGIT));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, pReadExif->ifdData.pIFDData);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA000(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_FLASH_PIX_VERSION));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%s"), GetJpeg_APP1_FlashPixVersionName(pReadExif->ifdData.pIFDData));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA001(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint16_t cColorSpace = pReadExif->ifdData.ifdMeta.cValue.pDataU16[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_COLOR_SPACE));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), cColorSpace, GetJpeg_APP1_ColorSpaceName(cColorSpace));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA002(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t nWidth;
    if (3 == pReadExif->ifdData.ifdMeta.cDataType) { nWidth = pReadExif->ifdData.ifdMeta.cValue.pDataU16[0]; }
    else if (4 == pReadExif->ifdData.ifdMeta.cDataType) { nWidth = pReadExif->ifdData.ifdMeta.cValue.pDataU32[0]; }
    else { nWidth = 0; }
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_WIDTH));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u像素"), nWidth);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA003(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t nHeight;
    if (3 == pReadExif->ifdData.ifdMeta.cDataType) { nHeight = pReadExif->ifdData.ifdMeta.cValue.pDataU16[0]; }
    else if (4 == pReadExif->ifdData.ifdMeta.cDataType) { nHeight = pReadExif->ifdData.ifdMeta.cValue.pDataU32[0]; }
    else { nHeight = 0; }
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_HEIGHT));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u像素"), nHeight);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA20E(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t nX = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[0]);
    uint32_t nY = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[4]);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_FOCAL_PLANE_X_RESOLUTION));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%g"), (HANDOUBLE)nX / (HANDOUBLE)nY);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA20F(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t nX = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[0]);
    uint32_t nY = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[4]);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_FOCAL_PLANE_Y_RESOLUTION));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%g"), (HANDOUBLE)nX / (HANDOUBLE)nY);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA210(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint16_t cUnit = pReadExif->ifdData.ifdMeta.cValue.pDataU16[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_FOCAL_PLANE_RESOLUTION_UNIT));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), cUnit, GetJpeg_APP1_FocalPlaneResolutionUnitName(cUnit));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA217(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint16_t cSensingMethod = pReadExif->ifdData.ifdMeta.cValue.pDataU16[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_SENSING_METHOD));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), cSensingMethod, GetJpeg_APP1_SensingMethodName(cSensingMethod));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA300(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint8_t cSource = pReadExif->ifdData.ifdMeta.cValue.pDataU8[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_FILE_SOURCE));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), cSource, GetJpeg_APP1_FileSourceName(cSource));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA301(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint8_t cSource = pReadExif->ifdData.ifdMeta.cValue.pDataU8[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_SCENE_TYPE));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), cSource, GetJpeg_APP1_SceneTypeName(cSource));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA302(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    const uint8_t* pData = pReadExif->ifdData.pIFDData;
    uint16_t nArrayWidth = pReadExif->ifdData.ReadBytes->Read2Bytes(&pData[0]);
    uint16_t nArrayHeight = pReadExif->ifdData.ReadBytes->Read2Bytes(&pData[2]);
    HANSIZE nOffset = 4;

    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_CFA_PATTERN));
    for (uint16_t nRow = 0; nRow < nArrayHeight; nRow++)
    {
        pReadExif->ifdData.pValue[0] = TEXT('\0');
        for (uint16_t nCol = 0; nCol < nArrayWidth; nCol++)
        {
            HAN_strcat(pReadExif->ifdData.pValue, GetJpeg_APP1_CFAPatternName(pData[nOffset]));
            HAN_strcat(pReadExif->ifdData.pValue, TEXT(" "));
            nOffset++;
        }
        UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
        pReadExif->ifdData.pName[0] = TEXT('\0');
    }
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA401(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint16_t cRender = pReadExif->ifdData.ifdMeta.cValue.pDataU16[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_CUSTOM_RENDERED));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), cRender, GetJpeg_APP1_CustonRenderedName(cRender));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA402(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint16_t cMode = pReadExif->ifdData.ifdMeta.cValue.pDataU16[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_EXPOSURE_MODE));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), cMode, GetJpeg_APP1_ExposureModeName(cMode));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA403(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint16_t cWhiteBalance = pReadExif->ifdData.ifdMeta.cValue.pDataU16[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_WHITE_BALANCE));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), cWhiteBalance, GetJpeg_APP1_WhiteBalanceName(cWhiteBalance));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA404(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t nX = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[0]);
    uint32_t nY = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[4]);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_DIGITAL_ZOOM_RATIO));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%g"), (HANDOUBLE)nX / (HANDOUBLE)nY);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA405(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint16_t cFocalLength = pReadExif->ifdData.ifdMeta.cValue.pDataU16[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_FOCAL_LENGTH_IN_35MM_FILM));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), cFocalLength);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA406(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint16_t cType = pReadExif->ifdData.ifdMeta.cValue.pDataU16[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_SCENE_CAPTURE_TYPE));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), cType, GetJpeg_APP1_SceneCaptureTypeName(cType));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA407(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint16_t cGain = pReadExif->ifdData.ifdMeta.cValue.pDataU16[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_GAIN_CONTROL));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), cGain, GetJpeg_APP1_GainControlName(cGain));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA408(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint16_t cContrast = pReadExif->ifdData.ifdMeta.cValue.pDataU16[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_CONTRAST));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), cContrast, GetJpeg_APP1_ContrastName(cContrast));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA409(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint16_t cSaturation = pReadExif->ifdData.ifdMeta.cValue.pDataU16[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_SATURATION));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), cSaturation, GetJpeg_APP1_SaturationName(cSaturation));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA40A(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint16_t cSharpness = pReadExif->ifdData.ifdMeta.cValue.pDataU16[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_SHARPNESS));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), cSharpness, GetJpeg_APP1_SharpnessName(cSharpness));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA40C(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint16_t cDis = pReadExif->ifdData.ifdMeta.cValue.pDataU16[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_SUBJECT_DISTANCE_RANGE));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u（%s）"), cDis, GetJpeg_APP1_SubjectDistanceRangeName(cDis));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA432(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    const uint32_t* cLens = (const uint32_t*)(pReadExif->ifdData.pIFDData);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_MIN_FOCAL));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%g"), (HANDOUBLE)cLens[0] / (HANDOUBLE)cLens[1]);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_MAX_FOCAL));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%g"), (HANDOUBLE)cLens[2] / (HANDOUBLE)cLens[3]);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_MIN_FOCAL_MAX_APERTURE));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("f/%g"), (HANDOUBLE)cLens[4] / (HANDOUBLE)cLens[5]);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_MAX_FOCAL_MAX_APERTURE));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("f/%g"), (HANDOUBLE)cLens[6] / (HANDOUBLE)cLens[7]);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA434(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_LENS_MODEL));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, pReadExif->ifdData.pIFDData);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA500(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t nX = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[0]);
    uint32_t nY = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[4]);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_GAMMA));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%g"), (HANDOUBLE)nX / (HANDOUBLE)nY);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static BOOL UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    BOOL bRet = TRUE;

    switch (pReadExif->ifdData.ifdMeta.cTag) {
        case 0x829A: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif829A(pReadExif); } break;
        case 0x829D: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif829D(pReadExif); } break;
        case 0x8822: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif8822(pReadExif); } break;
        case 0x8827: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif8827(pReadExif); } break;
        case 0x8830: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif8830(pReadExif); } break;
        case 0x8832: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif8832(pReadExif); } break;
        case 0x9000: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9000(pReadExif); } break;
        case 0x9003: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9003(pReadExif); } break;
        case 0x9004: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9004(pReadExif); } break;
        case 0x9101: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9101(pReadExif); } break;
        case 0x9102: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9102(pReadExif); } break;
        case 0x9201: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9201(pReadExif); } break;
        case 0x9202: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9202(pReadExif); } break;
        case 0x9203: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9203(pReadExif); } break;
        case 0x9204: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9204(pReadExif); } break;
        case 0x9205: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9205(pReadExif); } break;
        case 0x9207: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9207(pReadExif); } break;
        case 0x9208: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9208(pReadExif); } break;
        case 0x9209: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9209(pReadExif); } break;
        case 0x920A: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif920A(pReadExif); } break;
        case 0x927C: { UpdateSegmentInfoWindow_APP1_ReadExifIFDBlockMakerNote(pReadExif); } break;
        case 0x9286: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9286(pReadExif); } break;
        case 0x9290: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9290(pReadExif); } break;
        case 0x9291: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9291(pReadExif); } break;
        case 0x9292: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif9292(pReadExif); } break;
        case 0xA000: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA000(pReadExif); } break;
        case 0xA001: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA001(pReadExif); } break;
        case 0xA002: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA002(pReadExif); } break;
        case 0xA003: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA003(pReadExif); } break;
        case 0xA005: { UpdateSegmentInfoWindow_APP1_ReadExifIFDBlockInteroperability(pReadExif); } break;
        case 0xA20E: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA20E(pReadExif); } break;
        case 0xA20F: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA20F(pReadExif); } break;
        case 0xA210: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA210(pReadExif); } break;
        case 0xA217: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA217(pReadExif); } break;
        case 0xA300: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA300(pReadExif); } break;
        case 0xA301: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA301(pReadExif); } break;
        case 0xA302: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA302(pReadExif); } break;
        case 0xA401: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA401(pReadExif); } break;
        case 0xA402: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA402(pReadExif); } break;
        case 0xA403: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA403(pReadExif); } break;
        case 0xA404: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA404(pReadExif); } break;
        case 0xA405: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA405(pReadExif); } break;
        case 0xA406: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA406(pReadExif); } break;
        case 0xA407: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA407(pReadExif); } break;
        case 0xA408: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA408(pReadExif); } break;
        case 0xA409: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA409(pReadExif); } break;
        case 0xA40A: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA40A(pReadExif); } break;
        case 0xA40C: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA40C(pReadExif); } break;
        case 0xA432: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA432(pReadExif); } break;
        case 0xA434: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA434(pReadExif); } break;
        case 0xA500: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeExifA500(pReadExif); } break;

        default: {
            UpdateSegmentInfoWindow_APP1_PrintUnknownIFDTag(&(pReadExif->ifdData));
            bRet = FALSE;
        } break;
    }

    return bRet;
}
static void UpdateSegmentInfoWindow_APP1_ReadExifIFDBlockExif(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    HANSIZE nOffset;
    PICTUREJPEGSEGMENTREADEXIF readExif;

    nOffset = pReadExif->ifdData.ifdMeta.cValue.pDataU32[0];
    readExif = *pReadExif;
    readExif.printIFD.PrintIFDTitleCallback = UpdateSegmentInfoWindow_APP1_PrintExifTitle;
    readExif.printIFD.PrintIFDDataCallback = UpdateSegmentInfoWindow_IFD_PrintIFDStdData;
    readExif.ifdData.pIFDData = &(readExif.ifdData.pTIFFData[nOffset]);
    readExif.DecodeIFDMetaCallback = UpdateSegmentInfoWindow_APP1_ReadExifDecodeExif;
    readExif.ifdData.cIFDID = pReadExif->ifdData.ifdMeta.cTag;

    PictureJpegReadIFD(&readExif);
}
#endif
#if 1 // GPS 目录（8825）
static void UpdateSegmentInfoWindow_APP1_PrintGPSTitle(PPICTUREJPEGSEGMENTIFDDATA ifdData, void* pParam)
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
    if (0 == ifdData->nIFDBlockCnt) { HAN_strncpy(ifdData->pIFDTitle, PICTURE_JPEG_APP1_EXIF_IFD_TITLE_GPS, HAN_PICTURE_JPEG_TEXT_BUF_SIZE); }
    else
    {
        HAN_snprintf(
            ifdData->pIFDTitle,
            HAN_PICTURE_JPEG_TEXT_BUF_SIZE,
            PICTURE_JPEG_APP1_EXIF_IFD_TITLE_GPS TEXT(" - ") TEXT(HANSIZE_PRINT_FORMAT),
            ifdData->nIFDBlockCnt);
    }
    ListView_InsertItem(hInfo, &lvItem);

    ifdData->nMetaId = 0;
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeGPS0000(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_GPS_VERSION_ID));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u.%u.%u.%u"),
        pReadExif->ifdData.pIFDData[0],
        pReadExif->ifdData.pIFDData[1],
        pReadExif->ifdData.pIFDData[2],
        pReadExif->ifdData.pIFDData[3]
    );
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static BOOL UpdateSegmentInfoWindow_APP1_ReadExifDecodeGPS(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    BOOL bRet = TRUE;

    switch (pReadExif->ifdData.ifdMeta.cTag) {
        case 0x0000: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeGPS0000(pReadExif); } break;

        default: {
            UpdateSegmentInfoWindow_APP1_PrintUnknownIFDTag(&(pReadExif->ifdData));
            bRet = FALSE;
        } break;
    }

    return bRet;
}
static void UpdateSegmentInfoWindow_APP1_ReadExifIFDBlockGPS(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    HANSIZE nOffset;
    PICTUREJPEGSEGMENTREADEXIF readExif;

    nOffset = pReadExif->ifdData.ifdMeta.cValue.pDataU32[0];
    readExif = *pReadExif;
    readExif.printIFD.PrintIFDTitleCallback = UpdateSegmentInfoWindow_APP1_PrintGPSTitle;
    readExif.printIFD.PrintIFDDataCallback = UpdateSegmentInfoWindow_IFD_PrintIFDStdData;
    readExif.ifdData.pIFDData = &(readExif.ifdData.pTIFFData[nOffset]);
    readExif.ifdData.cIFDID = pReadExif->ifdData.ifdMeta.cTag;
    readExif.DecodeIFDMetaCallback = UpdateSegmentInfoWindow_APP1_ReadExifDecodeGPS;

    PictureJpegReadIFD(&readExif);
}
#endif
#if 1 // Maker Note（927C）
/* 参考 Maker Note */
#endif
#if 1 // IOP（A005）
static void UpdateSegmentInfoWindow_APP1_PrintInteroperabilityTitle(PPICTUREJPEGSEGMENTIFDDATA ifdData, void* pParam)
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
    if (0 == ifdData->nIFDBlockCnt) { HAN_strncpy(ifdData->pIFDTitle, PICTURE_JPEG_APP1_EXIF_IFD_TITLE_INTEROPERABILITY, HAN_PICTURE_JPEG_TEXT_BUF_SIZE); }
    else
    {
        HAN_snprintf(
            ifdData->pIFDTitle,
            HAN_PICTURE_JPEG_TEXT_BUF_SIZE,
            PICTURE_JPEG_APP1_EXIF_IFD_TITLE_INTEROPERABILITY TEXT(" - ") TEXT(HANSIZE_PRINT_FORMAT),
            ifdData->nIFDBlockCnt);
    }
    ListView_InsertItem(hInfo, &lvItem);

    ifdData->nMetaId = 0;
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeInteroperability0001(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_INTEROPERABILITY_INDEX));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, pReadExif->ifdData.pIFDData);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeInteroperability0002(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    CHAR pMainVersion[3] = { 0 };
    CHAR pSubVersion[3] = { 0 };
    uint8_t nMainVersion;
    memcpy(pMainVersion, &(pReadExif->ifdData.pIFDData[0]), 2);
    memcpy(pSubVersion, &(pReadExif->ifdData.pIFDData[2]), 2);
    nMainVersion = (uint8_t)strtoul(pMainVersion, NULL, 10);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_INTEROPERABILITY_VERSION));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u.") HANPSTR_PRINT_PCHAR_FORMAT, nMainVersion, pSubVersion);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeInteroperability1001(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint16_t nWidth = pReadExif->ifdData.ifdMeta.cValue.pDataU16[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_RELATED_IMAGE_WIDTH));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), nWidth);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ReadExifDecodeInteroperability1002(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint16_t nHeight = pReadExif->ifdData.ifdMeta.cValue.pDataU16[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpeg_APP1_FieldName(PICTURE_JPEG_APP1_SEGMENT_FIELD_RELATED_IMAGE_HEIGHT));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), nHeight);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static BOOL UpdateSegmentInfoWindow_APP1_ReadExifDecodeInteroperability(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    BOOL bRet = TRUE;

    switch (pReadExif->ifdData.ifdMeta.cTag) {
        case 0x0001: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeInteroperability0001(pReadExif); } break;
        case 0x0002: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeInteroperability0002(pReadExif); } break;
        case 0x1001: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeInteroperability1001(pReadExif); } break;
        case 0x1002: { UpdateSegmentInfoWindow_APP1_ReadExifDecodeInteroperability1002(pReadExif); } break;

        default: {
            UpdateSegmentInfoWindow_APP1_PrintUnknownIFDTag(&(pReadExif->ifdData));
            bRet = FALSE;
        } break;
    }

    return bRet;
}
static void UpdateSegmentInfoWindow_APP1_ReadExifIFDBlockInteroperability(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    HANSIZE nOffset;
    PICTUREJPEGSEGMENTREADEXIF readExif;

    nOffset = pReadExif->ifdData.ifdMeta.cValue.pDataU32[0];
    readExif = *pReadExif;
    readExif.printIFD.PrintIFDTitleCallback = UpdateSegmentInfoWindow_APP1_PrintInteroperabilityTitle;
    readExif.printIFD.PrintIFDDataCallback = UpdateSegmentInfoWindow_IFD_PrintIFDStdData;
    readExif.ifdData.pIFDData = &(readExif.ifdData.pTIFFData[nOffset]);
    readExif.ifdData.cIFDID = pReadExif->ifdData.ifdMeta.cTag;
    readExif.DecodeIFDMetaCallback = UpdateSegmentInfoWindow_APP1_ReadExifDecodeInteroperability;

    PictureJpegReadIFD(&readExif);
}
#endif

BOOL UpdateSegmentInfoWindow_APP1_ReadExif(PCPICTUREJPEGSEGMENT pSegment, HWND hListView)
{
    BOOL bRet = FALSE;
    uint8_t pHeader[6] = { 'E', 'x', 'i', 'f', '\0', '\0' };
    HANCHAR pText[6];
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
            &(pSegment->pData)[sizeof(pHeader)],
            pSegment->nLength - sizeof(pHeader),
            &printIFD,
            UpdateSegmentInfoWindow_APP1_ReadExifDecodeIFDRoot
        );
        bRet = TRUE;
    }

    return bRet;
}

static void UpdateSegmentInfoWindow_APP1_PrintUnknownIFDTag(PPICTUREJPEGSEGMENTIFDDATA ifdData)
{
    printf("Unknown IFD tag: %04X, type: %u, cnt: %u, IFD Id: %04X\n",
        ifdData->ifdMeta.cTag,
        ifdData->ifdMeta.cDataType,
        ifdData->ifdMeta.nDataCnt,
        ifdData->cIFDID
    );
}
