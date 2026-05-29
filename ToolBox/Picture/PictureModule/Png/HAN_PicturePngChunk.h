#ifndef HAN_PICTURE_PNG_CHUNK_H
#define HAN_PICTURE_PNG_CHUNK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\..\HAN_PictureDef.h"

#define PICTURE_PNG_CHUNK_INFO_LIST_ITEM_MAX        256

typedef enum {
    PICTURE_PNG_CHUNK_TYPE_IHDR,
    PICTURE_PNG_CHUNK_TYPE_gAMA,
    PICTURE_PNG_CHUNK_TYPE_pHYs,
    PICTURE_PNG_CHUNK_TYPE_bKGD,
    PICTURE_PNG_CHUNK_TYPE_tIME,
    PICTURE_PNG_CHUNK_TYPE_iCCP,
    PICTURE_PNG_CHUNK_TYPE_cHRM,
    PICTURE_PNG_CHUNK_TYPE_PLTE,
    PICTURE_PNG_CHUNK_TYPE_tRNS,
    PICTURE_PNG_CHUNK_TYPE_IDAT,
    PICTURE_PNG_CHUNK_TYPE_tEXT,
    PICTURE_PNG_CHUNK_TYPE_IEND,
    PICTURE_PNG_CHUNK_TYPE_CNT,
} PICTUREPNGCHUNKTYPE;

typedef enum {
    PICTURE_PNG_IHDR_CHUNK_FIELD_RESOLUTION,
    PICTURE_PNG_IHDR_CHUNK_FIELD_BIT_DEPTH,
    PICTURE_PNG_IHDR_CHUNK_FIELD_COLOR_TYPE,
    PICTURE_PNG_IHDR_CHUNK_FIELD_COMPRESSION,
    PICTURE_PNG_IHDR_CHUNK_FIELD_FILTER,
    PICTURE_PNG_IHDR_CHUNK_FIELD_INTERLACE,
    PICTURE_PNG_IHDR_CHUNK_FIELD_CNT,
} PICTUREPNGCHUNKFIELD_IHDR;
typedef enum {
    PICTURE_PNG_gAMA_CHUNK_FIELD_VALUE,
    PICTURE_PNG_gAMA_CHUNK_FIELD_CNT,
} PICTUREPNGCHUNKFIELD_gAMA;
typedef enum {
    PICTURE_PNG_pHYs_CHUNK_FIELD_UNIT,
    PICTURE_PNG_pHYs_CHUNK_FIELD_RESOLUTION_H,
    PICTURE_PNG_pHYs_CHUNK_FIELD_RESOLUTION_V,
    PICTURE_PNG_pHYs_CHUNK_FIELD_PHY_SIZE,
    PICTURE_PNG_pHYs_CHUNK_FIELD_CNT,
} PICTUREPNGCHUNKFIELD_pHYs;
typedef enum {
    PICTURE_PNG_bKGD_CHUNK_FIELD_BACKGROUND,
    PICTURE_PNG_bKGD_CHUNK_FIELD_CNT,
} PICTUREPNGCHUNKFIELD_bKGD;
typedef enum {
    PICTURE_PNG_tIME_CHUNK_FIELD_UTC,
    PICTURE_PNG_tIME_CHUNK_FIELD_CNT,
} PICTUREPNGCHUNKFIELD_tIME;
typedef enum {
    PICTURE_PNG_iCCP_CHUNK_FIELD_FILE_NAME,
    PICTURE_PNG_iCCP_CHUNK_FIELD_COMPRESSION,
    PICTURE_PNG_iCCP_CHUNK_FIELD_DATA,
    PICTURE_PNG_iCCP_CHUNK_FIELD_CNT,
} PICTUREPNGCHUNKFIELD_iCCP;
typedef enum {
    PICTURE_PNG_cHRM_CHUNK_FIELD_WHITE_POSITION,
    PICTURE_PNG_cHRM_CHUNK_FIELD_RED_POSITION,
    PICTURE_PNG_cHRM_CHUNK_FIELD_GREEN_POSITION,
    PICTURE_PNG_cHRM_CHUNK_FIELD_BLUE_POSITION,
    PICTURE_PNG_cHRM_CHUNK_FIELD_CNT,
} PICTUREPNGCHUNKFIELD_cHRM;
typedef enum {
    PICTURE_PNG_IDAT_CHUNK_FIELD_LEN,
    PICTURE_PNG_IDAT_CHUNK_FIELD_DATA,
    PICTURE_PNG_IDAT_CHUNK_FIELD_CNT,
} PICTUREPNGCHUNKFIELD_IDAT;
typedef enum {
    PICTURE_PNG_tEXT_CHUNK_FIELD_TEXT,
    PICTURE_PNG_tEXT_CHUNK_FIELD_CNT,
} PICTUREPNGCHUNKFIELD_tEXT;

typedef struct tagPICTUREPNGCHUNK {
    uint32_t                        nLength;
    CHAR                            pTypeCode[4];
    PICTUREPNGCHUNKTYPE             eType;
    const uint8_t*                  pData;
    uint32_t                        cCrc;
    BOOL                            bCrcOk;
} PICTUREPNGCHUNK, * PPICTUREPNGCHUNK;
typedef const PICTUREPNGCHUNK* PCPICTUREPNGCHUNK;

typedef struct tagPICTUREPNGCHUNKINFO {
    struct {
        BOOL                        bValid;
        PICTURERESOLUTION           pxResolution;
        uint8_t                     cBitDepth;
        uint8_t                     cColorType;
        uint8_t                     cCompression;
        uint8_t                     cFilter;
        uint8_t                     cInterlace;
    } IHDR;
    struct {
        BOOL                        bValid;
        HANDOUBLE                   nValue;
    } gAMA;
    struct {
        BOOL                        bValid;
        uint32_t                    pxResolutionH;
        uint32_t                    pxResolutionV;
        uint8_t                     cUnit;
    } pHYs;
    struct {
        BOOL                        bValid;
        uint16_t                    red;
        uint16_t                    green;
        uint16_t                    blue;
    } bKGD;
    struct {
        BOOL                        bValid;
        uint16_t                    nYear;
        uint8_t                     nMonth;
        uint8_t                     nDay;
        uint8_t                     nHour;
        uint8_t                     nMinute;
        uint8_t                     nSecond;
    } tIME;
    struct {
        BOOL                        bValid;
        PCHAR                       pFileName;
        uint8_t                     cCompression;
        const uint8_t*              pData;
        uint32_t                    nDataLen;
    } iCCP;
    struct {
        BOOL                        bValid;
        uint32_t                    whiteX;
        uint32_t                    whiteY;
        uint32_t                    redX;
        uint32_t                    redY;
        uint32_t                    greenX;
        uint32_t                    greenY;
        uint32_t                    blueX;
        uint32_t                    blueY;
    } cHRM;
    struct {
        BOOL                        bValid;
        const uint8_t*              pData;
        uint32_t                    nDataLen;
    } PLTE;
    struct {
        BOOL                        bValid;
        const uint8_t*              pData;
        uint32_t                    nDataLen;
    } tRNS;
    struct {
        BOOL                        bValid;
        const uint8_t*              pData;
        uint32_t                    nDataLen;
    } IDAT;
    struct {
        BOOL                        bValid;
    } IEND;
} PICTUREPNGCHUNKINFO, * PPICTUREPNGCHUNKINFO;
typedef const PICTUREPNGCHUNKINFO* PCPICTUREPNGCHUNKINFO;

HANPSTR GetPng_IHDR_Name(void);
HANPSTR GetPng_IHDR_FieldName(PICTUREPNGCHUNKFIELD_IHDR eName);
HANPSTR GetPng_IHDR_ColorTypeName(uint8_t cColorType);
HANPSTR GetPng_IHDR_CompressionName(uint8_t cCompression);
HANPSTR GetPng_IHDR_FilterName(uint8_t cFilter);
HANPSTR GetPng_IHDR_InterlaceName(uint8_t cInterlace);

HANPSTR GetPng_gAMA_Name(void);
HANPSTR GetPng_gAMA_FieldName(PICTUREPNGCHUNKFIELD_gAMA eName);

HANPSTR GetPng_pHYs_Name(void);
HANPSTR GetPng_pHYs_FieldName(PICTUREPNGCHUNKFIELD_pHYs eName);
HANPSTR GetPng_pHYs_UnitName(uint8_t cUnit);

HANPSTR GetPng_bKGD_Name(void);
HANPSTR GetPng_bKGD_FieldName(PICTUREPNGCHUNKFIELD_bKGD eName);

HANPSTR GetPng_tIME_Name(void);
HANPSTR GetPng_tIME_FieldName(PICTUREPNGCHUNKFIELD_tIME eName);

HANPSTR GetPng_iCCP_Name(void);
HANPSTR GetPng_iCCP_FieldName(PICTUREPNGCHUNKFIELD_iCCP eName);
HANPSTR GetPng_iCCP_CompressionName(uint8_t cCompression);

HANPSTR GetPng_cHRM_Name(void);
HANPSTR GetPng_cHRM_FieldName(PICTUREPNGCHUNKFIELD_cHRM eName);

HANPSTR GetPng_PLTE_Name(void);

HANPSTR GetPng_tRNS_Name(void);

HANPSTR GetPng_IDAT_Name(void);
HANPSTR GetPng_IDAT_FieldName(PICTUREPNGCHUNKFIELD_IDAT eName);

HANPSTR GetPng_tEXT_Name(void);

#ifdef __cplusplus
}
#endif

#endif
