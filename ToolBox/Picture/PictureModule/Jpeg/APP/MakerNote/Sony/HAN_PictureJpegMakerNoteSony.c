#include <string.h>

#include "..\HAN_PictureJpegMakerNote_Def.h"
#include "HAN_PictureJpegMakerNoteSony.h"
#include "HAN_PictureJpegMakerNoteSonyTag.h"
#include "HAN_PictureJpegMakerNoteSonyModelInfo.h"
#include "HAN_PictureJpegMakerNoteSonyLensType.h"

#define PICTURE_JPEG_APP1_EXIF_MAKER_NOTE_MAKER_NAME_SONY       TEXT("Ë÷Äá")

#define PICTURE_JPEG_APP1_EXIF_MAKER_NOTE_SONY_DSC_HEADER       "SONY DSC "

typedef enum {
    PICTURE_JPEG_APP1_EXIF_MAKER_NOTE_SONY_DSC,
    PICTURE_JPEG_APP1_EXIF_MAKER_NOTE_SONY_CNT,
} PICTUREJPEGAPP1EXIFMAKERNOTESONY;

typedef struct tagPICTUREJPEGAPP1EXIFMAKERNOTESONYINFO {
    HANSIZE                                 nHeaderLen;
    const void*                             pHeader;
    void                                    (*ReadIFD)(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
} PICTUREJPEGAPP1EXIFMAKERNOTESONYINF;

static void UpdateSegmentInfoWindow_APP1_ReadExifIFDBlockMakerNote_SonyDsc(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static BOOL UpdateSegmentInfoWindow_APP1_ReadExifDecodeMakerNote_Sony(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSony0102(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSony0104(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSony0105(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSony0112(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSony0115(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSony0116(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSony2000(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSony2001(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSony2002(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSony2003(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSony2004(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSony2005(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSony2006(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSony2007(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSony2008(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSony2009(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSony200A(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSony200B(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSony900B(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSonyB021(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSonyB022(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSonyB023(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSonyB024(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSonyB025(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSonyB026(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSonyB027(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSonyB029(PPICTUREJPEGSEGMENTREADEXIF pReadExif);
static void UpdateSegmentInfoWindow_APP1_ExifReadSonyB02A(PPICTUREJPEGSEGMENTREADEXIF pReadExif);

static const PICTUREJPEGAPP1EXIFMAKERNOTESONYINF sg_pIFDSonyInfo[PICTURE_JPEG_APP1_EXIF_MAKER_NOTE_SONY_CNT] = {
    [PICTURE_JPEG_APP1_EXIF_MAKER_NOTE_SONY_DSC] = {
        .nHeaderLen = sizeof(PICTURE_JPEG_APP1_EXIF_MAKER_NOTE_SONY_DSC_HEADER),
        .pHeader = PICTURE_JPEG_APP1_EXIF_MAKER_NOTE_SONY_DSC_HEADER,
        .ReadIFD = UpdateSegmentInfoWindow_APP1_ReadExifIFDBlockMakerNote_SonyDsc,
    },
};

BOOL UpdateSegmentInfoWindow_APP1_ReadExifIFDBlockMakerNote_Sony(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    BOOL bRet = FALSE;
    const uint8_t* pData = pReadExif->ifdData.pIFDData;
    PPICTUREJPEGAPP1EXIFMAKERNOTEPRINTTITLEPARAM pPrintParam = (PPICTUREJPEGAPP1EXIFMAKERNOTEPRINTTITLEPARAM)(pReadExif->printIFD.pPrintParam);

    pPrintParam->pMaker = PICTURE_JPEG_APP1_EXIF_MAKER_NOTE_MAKER_NAME_SONY;

    for (PICTUREJPEGAPP1EXIFMAKERNOTESONY iLoop = 0; iLoop < PICTURE_JPEG_APP1_EXIF_MAKER_NOTE_SONY_CNT; iLoop++)
    {
        if (!memcmp(pData, sg_pIFDSonyInfo[iLoop].pHeader, sg_pIFDSonyInfo[iLoop].nHeaderLen))
        {
            sg_pIFDSonyInfo[iLoop].ReadIFD(pReadExif);
            bRet = TRUE;
        }
    }

    return bRet;
}

static void UpdateSegmentInfoWindow_APP1_ReadExifIFDBlockMakerNote_SonyDsc(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    HANCHAR pTag[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    PPICTUREJPEGAPP1EXIFMAKERNOTEPRINTTITLEPARAM pPrintParam = (PPICTUREJPEGAPP1EXIFMAKERNOTEPRINTTITLEPARAM)(pReadExif->printIFD.pPrintParam);

    HAN_snprintf(pTag, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, PICTURE_JPEG_APP1_EXIF_MAKER_NOTE_SONY_DSC_HEADER);
    pPrintParam->pTag = pTag;

    pReadExif->DecodeIFDMetaCallback = UpdateSegmentInfoWindow_APP1_ReadExifDecodeMakerNote_Sony;
    pReadExif->ifdData.pIFDData = &(pReadExif->ifdData.pIFDData)[12];
    PictureJpegReadIFD(pReadExif);
}
static BOOL UpdateSegmentInfoWindow_APP1_ReadExifDecodeMakerNote_Sony(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    BOOL bRet = TRUE;

    switch (pReadExif->ifdData.ifdMeta.cTag) {
        case 0x0010: { UpdateSegmentInfoWindow_APP1_ExifReadSony0010(pReadExif); } break;
        case 0x0102: { UpdateSegmentInfoWindow_APP1_ExifReadSony0102(pReadExif); } break;
        case 0x0104: { UpdateSegmentInfoWindow_APP1_ExifReadSony0104(pReadExif); } break;
        case 0x0105: { UpdateSegmentInfoWindow_APP1_ExifReadSony0105(pReadExif); } break;
        case 0x0112: { UpdateSegmentInfoWindow_APP1_ExifReadSony0112(pReadExif); } break;
        case 0x0115: { UpdateSegmentInfoWindow_APP1_ExifReadSony0115(pReadExif); } break;
        case 0x0116: { UpdateSegmentInfoWindow_APP1_ExifReadSony0116(pReadExif); } break;
        case 0x2000: { UpdateSegmentInfoWindow_APP1_ExifReadSony2000(pReadExif); } break;
        case 0x2001: { UpdateSegmentInfoWindow_APP1_ExifReadSony2001(pReadExif); } break;
        case 0x2002: { UpdateSegmentInfoWindow_APP1_ExifReadSony2002(pReadExif); } break;
        case 0x2003: { UpdateSegmentInfoWindow_APP1_ExifReadSony2003(pReadExif); } break;
        case 0x2004: { UpdateSegmentInfoWindow_APP1_ExifReadSony2004(pReadExif); } break;
        case 0x2005: { UpdateSegmentInfoWindow_APP1_ExifReadSony2005(pReadExif); } break;
        case 0x2006: { UpdateSegmentInfoWindow_APP1_ExifReadSony2006(pReadExif); } break;
        case 0x2007: { UpdateSegmentInfoWindow_APP1_ExifReadSony2007(pReadExif); } break;
        case 0x2008: { UpdateSegmentInfoWindow_APP1_ExifReadSony2008(pReadExif); } break;
        case 0x2009: { UpdateSegmentInfoWindow_APP1_ExifReadSony2009(pReadExif); } break;
        case 0x200A: { UpdateSegmentInfoWindow_APP1_ExifReadSony200A(pReadExif); } break;
        case 0x200B: { UpdateSegmentInfoWindow_APP1_ExifReadSony200B(pReadExif); } break;
        case 0x900B: { UpdateSegmentInfoWindow_APP1_ExifReadSony900B(pReadExif); } break;
        case 0xB021: { UpdateSegmentInfoWindow_APP1_ExifReadSonyB021(pReadExif); } break;
        case 0xB022: { UpdateSegmentInfoWindow_APP1_ExifReadSonyB022(pReadExif); } break;
        case 0xB023: { UpdateSegmentInfoWindow_APP1_ExifReadSonyB023(pReadExif); } break;
        case 0xB024: { UpdateSegmentInfoWindow_APP1_ExifReadSonyB024(pReadExif); } break;
        case 0xB025: { UpdateSegmentInfoWindow_APP1_ExifReadSonyB025(pReadExif); } break;
        case 0xB026: { UpdateSegmentInfoWindow_APP1_ExifReadSonyB026(pReadExif); } break;
        case 0xB027: { UpdateSegmentInfoWindow_APP1_ExifReadSonyB027(pReadExif); } break;
        case 0xB029: { UpdateSegmentInfoWindow_APP1_ExifReadSonyB029(pReadExif); } break;
        case 0xB02A: { UpdateSegmentInfoWindow_APP1_ExifReadSonyB02A(pReadExif); } break;

        default: { bRet = FALSE; } break;
    }

    return bRet;
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSony0102(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t cQuality = pReadExif->ifdData.ifdMeta.cValue.pDataU32[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_QUALITY));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u£¨%s£©"), cQuality, GetJpegMakerNoteSonyTagQualityName(cQuality));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSony0104(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    int32_t nSignedX;
    int32_t nSignedY;
    uint32_t nUnsignedX = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[0]);
    uint32_t nUnsignedY = pReadExif->ifdData.ReadBytes->Read4Bytes(&pReadExif->ifdData.pIFDData[4]);
    memcpy(&nSignedX, &nUnsignedX, 4);
    memcpy(&nSignedY, &nUnsignedY, 4);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_FLASH_EXPOSURE_COMP));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%+g EV"), (HANDOUBLE)nSignedX / (HANDOUBLE)nSignedY);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSony0105(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t cTeleconverter = pReadExif->ifdData.ifdMeta.cValue.pDataU32[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_TELECONVERTER));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u£¨%s£©"), cTeleconverter, GetJpegMakerNoteSonyTagTeleconverterName(cTeleconverter));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSony0112(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t cTuneValue = pReadExif->ifdData.ifdMeta.cValue.pDataU32[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_WHITE_BALANCE_FINE_TUNE));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), cTuneValue);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSony0115(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t cWhiteBalance = pReadExif->ifdData.ifdMeta.cValue.pDataU32[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_WHITE_BALANCE));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%X£¨%s£©"), cWhiteBalance, GetJpegMakerNoteSonyTagWhiteBalanceName(cWhiteBalance));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSony0116(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    const uint8_t* pData = pReadExif->ifdData.pIFDData;
    uint8_t cValueU8;
    uint16_t cValueU16;
    
    cValueU16 = pReadExif->ifdData.ReadBytes->Read2Bytes(&pData[0]);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_BATTERY_UNKNOWN));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%04X"), cValueU16);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);

    cValueU8 = pData[2];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_BATTERY_TEMPERATURE));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%.1lf¡æ"), (HANDOUBLE)(cValueU8 - 32) / (HANDOUBLE)1.8);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);

    cValueU8 = pData[4];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_BATTERY_LEVEL));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u%%"), cValueU8);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);

    cValueU16 = pReadExif->ifdData.ReadBytes->Read2Bytes(&pData[6]);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_BATTERY_VOLTAGE1));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%.2lf V"), (HANDOUBLE)cValueU16 / (HANDOUBLE)256 * (HANDOUBLE)2);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);

    cValueU16 = pReadExif->ifdData.ReadBytes->Read2Bytes(&pData[8]);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_BATTERY_VOLTAGE2));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%.2lf V"), (HANDOUBLE)cValueU16 / (HANDOUBLE)256 * (HANDOUBLE)2);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);

    cValueU8 = pData[17];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_IMAGE_STABILIZATION_0116));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u£¨%s£©"), cValueU8, GetJpegMakerNoteSonyTagImageStabilizationName(cValueU8));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);

    cValueU8 = pData[20];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_MODE_DIAL_POSITION));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u£¨%s£©"), cValueU8, GetJpegMakerNoteSonyTagModeDialPositionName(cValueU8));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);

    cValueU8 = pData[22];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_MEM_CARD_CFG));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u£¨%s£©"), cValueU8, GetJpegMakerNoteSonyTagMemoryCardConfigurationName(cValueU8));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);

    cValueU8 = (pData[24] >> 4) & 0x03;
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_CAMERA_ORIENTATION));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u£¨%s£©"), cValueU8, GetJpegMakerNoteSonyTagCameraOrientationName(cValueU8));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSony2000(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    /* Î´Öª */
    (void)pReadExif;
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSony2001(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    /* Ô¤ÀÀÍ¼ */
    (void)pReadExif;
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSony2002(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t cRating = pReadExif->ifdData.ifdMeta.cValue.pDataU32[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_RATING));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), cRating);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSony2003(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    /* ¿Õ°××Ö·û´® */
    (void)pReadExif;
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSony2004(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    int32_t cContrast = pReadExif->ifdData.ifdMeta.cValue.pDataS32[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_CONTRAST));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%d"), cContrast);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSony2005(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    int32_t cSaturation = pReadExif->ifdData.ifdMeta.cValue.pDataS32[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_SATURATION));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%d"), cSaturation);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSony2006(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    int32_t cSharpness = pReadExif->ifdData.ifdMeta.cValue.pDataS32[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_SHARPNESS));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%d"), cSharpness);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSony2007(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    int32_t cBrightness = pReadExif->ifdData.ifdMeta.cValue.pDataS32[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_BRIGHTNESS));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%d"), cBrightness);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSony2008(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t cLongExposureNoiseReduction = pReadExif->ifdData.ifdMeta.cValue.pDataU32[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_LONG_EXPOSURE_NOISE_REDUCTION));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u£¨%s£©"),
        cLongExposureNoiseReduction, GetJpegMakerNoteSonyTagLongExposureNoiseReductionName(cLongExposureNoiseReduction)
    );
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSony2009(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint16_t cHighISONoiseReduction = pReadExif->ifdData.ifdMeta.cValue.pDataU16[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_HIGH_ISO_NOISE_REDUCTION));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u£¨%s£©"),
        cHighISONoiseReduction, GetJpegMakerNoteSonyTagHighISONoiseReductionName(cHighISONoiseReduction)
    );
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSony200A(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint16_t cHDR0 = pReadExif->ifdData.ReadBytes->Read2Bytes(&pReadExif->ifdData.pIFDData[0]);;
    uint16_t cHDR1 = pReadExif->ifdData.ReadBytes->Read2Bytes(&pReadExif->ifdData.pIFDData[2]);;
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_HDR));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%s, %s"),
        GetJpegMakerNoteSonyTagHDRValue0Name(cHDR0), GetJpegMakerNoteSonyTagHDRValue1Name(cHDR1)
    );
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSony200B(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t cMultiFrameNoiseReduction = pReadExif->ifdData.ifdMeta.cValue.pDataU32[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_MULTI_FRAME_NOISE_REDUCTION));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u£¨%s£©"),
        cMultiFrameNoiseReduction, GetJpegMakerNoteSonyTagMultiFrameNoiseReductionName(cMultiFrameNoiseReduction)
    );
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSony900B(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    const uint8_t* pFace = pReadExif->ifdData.pIFDData;
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_FACE_DETECTION));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u£¨%s£©"), pFace[189], GetJpegMakerNoteSonyTagFaceDetectionName(pFace[189]));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_FACE_DETECTED));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u£¨%s£©"), pFace[2], GetJpegMakerNoteSonyTagFaceDetectedName(pFace[2]));
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSonyB021(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t cColorTemperature = pReadExif->ifdData.ifdMeta.cValue.pDataU32[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_COLOR_TEMPERATURE));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), cColorTemperature);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSonyB022(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t cColorCompensationFilter = pReadExif->ifdData.ifdMeta.cValue.pDataU32[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_COLOR_COMPENSATION_FILTER));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), cColorCompensationFilter);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSonyB023(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t cSceneMode = pReadExif->ifdData.ifdMeta.cValue.pDataU32[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_SCENE_MODE));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), cSceneMode);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSonyB024(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t cZoneMatching = pReadExif->ifdData.ifdMeta.cValue.pDataU32[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_ZONE_MATCHING));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), cZoneMatching);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSonyB025(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t cDynamicRangeOptimizer = pReadExif->ifdData.ifdMeta.cValue.pDataU32[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_DYNAMIC_RANGE_OPTIMIZER));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), cDynamicRangeOptimizer);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSonyB026(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t cImageStabilization = pReadExif->ifdData.ifdMeta.cValue.pDataU32[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_IMAGE_STABILIZATION_B026));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), cImageStabilization);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSonyB027(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t cLensType = pReadExif->ifdData.ifdMeta.cValue.pDataU32[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_LENS_TYPE));
    UpdateSegmentInfoWindow_APP1_ExifReadSonyGetLensTypeName(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, cLensType);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSonyB029(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    uint32_t cColorMode = pReadExif->ifdData.ifdMeta.cValue.pDataU32[0];
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_COLOR_MODE));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), cColorMode);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
static void UpdateSegmentInfoWindow_APP1_ExifReadSonyB02A(PPICTUREJPEGSEGMENTREADEXIF pReadExif)
{
    const uint8_t* pLensSpec = pReadExif->ifdData.pIFDData;
    HAN_snprintf(pReadExif->ifdData.pName, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, GetJpegMakerNoteSonyTagName(PICTURE_JPEG_MAKER_NOTE_SONY_TAG_LENS_SPEC_B02A));
    HAN_snprintf(pReadExif->ifdData.pValue, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, (const PSTR)pLensSpec);
    UpdateSegmentInfoWindow_APP1_PrintIFDStdDataCallback(pReadExif);
}
