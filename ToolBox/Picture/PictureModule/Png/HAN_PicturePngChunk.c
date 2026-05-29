#include "HAN_PicturePngChunk.h"

#if 1 /******************** IHDR ********************/
typedef enum {
    PICTURE_PNG_IHDR_CHUNK_FIELD_COLOR_TYPE_0,
    PICTURE_PNG_IHDR_CHUNK_FIELD_COLOR_TYPE_2,
    PICTURE_PNG_IHDR_CHUNK_FIELD_COLOR_TYPE_3,
    PICTURE_PNG_IHDR_CHUNK_FIELD_COLOR_TYPE_4,
    PICTURE_PNG_IHDR_CHUNK_FIELD_COLOR_TYPE_6,
    PICTURE_PNG_IHDR_CHUNK_FIELD_COLOR_TYPE_NONE,
    PICTURE_PNG_IHDR_CHUNK_FIELD_COLOR_TYPE_CNT,
} PICTUREPNGIHDRCHUNKFIELDCOLORTYPE;
typedef enum {
    PICTURE_PNG_IHDR_CHUNK_FIELD_COMPRESSION_0,
    PICTURE_PNG_IHDR_CHUNK_FIELD_COMPRESSION_NOT_SUPPORTED,
    PICTURE_PNG_IHDR_CHUNK_FIELD_COMPRESSION_CNT,
} PICTUREPNGIHDRCHUNKFIELDCOMPRESSION;
typedef enum {
    PICTURE_PNG_IHDR_CHUNK_FIELD_FILTER_0,
    PICTURE_PNG_IHDR_CHUNK_FIELD_FILTER_OTHER,
    PICTURE_PNG_IHDR_CHUNK_FIELD_FILTER_CNT,
} PICTUREPNGIHDRCHUNKFIELDFILTER;
typedef enum {
    PICTURE_PNG_IHDR_CHUNK_FIELD_INTERLACE_0,
    PICTURE_PNG_IHDR_CHUNK_FIELD_INTERLACE_1,
    PICTURE_PNG_IHDR_CHUNK_FIELD_INTERLACE_OTHER,
    PICTURE_PNG_IHDR_CHUNK_FIELD_INTERLACE_CNT,
} PICTUREPNGIHDRCHUNKFIELDINTERLACE;

static const HANPSTR sg_pPng_IHDR_Name = TEXT("文件头");
static const HANPSTR sg_pChunkFieldName_IHDR[PICTURE_PNG_IHDR_CHUNK_FIELD_CNT] = {
    [PICTURE_PNG_IHDR_CHUNK_FIELD_RESOLUTION] = TEXT("分辨率"),
    [PICTURE_PNG_IHDR_CHUNK_FIELD_BIT_DEPTH] = TEXT("位深度"),
    [PICTURE_PNG_IHDR_CHUNK_FIELD_COLOR_TYPE] = TEXT("颜色类型"),
    [PICTURE_PNG_IHDR_CHUNK_FIELD_COMPRESSION] = TEXT("压缩方法"),
    [PICTURE_PNG_IHDR_CHUNK_FIELD_FILTER] = TEXT("滤波方法"),
    [PICTURE_PNG_IHDR_CHUNK_FIELD_INTERLACE] = TEXT("隔行扫描"),
};
static const HANPSTR sg_pPng_IHDR_ColorTypeName[PICTURE_PNG_IHDR_CHUNK_FIELD_COLOR_TYPE_CNT] = {
    [PICTURE_PNG_IHDR_CHUNK_FIELD_COLOR_TYPE_0] = TEXT("灰度图像"),
    [PICTURE_PNG_IHDR_CHUNK_FIELD_COLOR_TYPE_2] = TEXT("真彩色图像"),
    [PICTURE_PNG_IHDR_CHUNK_FIELD_COLOR_TYPE_3] = TEXT("索引彩色图像"),
    [PICTURE_PNG_IHDR_CHUNK_FIELD_COLOR_TYPE_4] = TEXT("带α通道的灰度图像"),
    [PICTURE_PNG_IHDR_CHUNK_FIELD_COLOR_TYPE_6] = TEXT("带α通道的真彩色图像"),
    [PICTURE_PNG_IHDR_CHUNK_FIELD_COLOR_TYPE_NONE] = TEXT("无效值"),
};
static const HANPSTR sg_pPng_IHDR_CompressionName[PICTURE_PNG_IHDR_CHUNK_FIELD_COMPRESSION_CNT] = {
    [PICTURE_PNG_IHDR_CHUNK_FIELD_COMPRESSION_0] = TEXT("deflate"),
    [PICTURE_PNG_IHDR_CHUNK_FIELD_COMPRESSION_NOT_SUPPORTED] = TEXT("不支持"),
};
static const HANPSTR sg_pPng_IHDR_FilterName[PICTURE_PNG_IHDR_CHUNK_FIELD_FILTER_CNT] = {
    [PICTURE_PNG_IHDR_CHUNK_FIELD_FILTER_0] = TEXT("自适应滤波"),
    [PICTURE_PNG_IHDR_CHUNK_FIELD_FILTER_OTHER] = TEXT("其它"),
};
static const HANPSTR sg_pPng_IHDR_InterlaceName[PICTURE_PNG_IHDR_CHUNK_FIELD_INTERLACE_CNT] = {
    [PICTURE_PNG_IHDR_CHUNK_FIELD_INTERLACE_0] = TEXT("非隔行扫描"),
    [PICTURE_PNG_IHDR_CHUNK_FIELD_INTERLACE_1] = TEXT("Adam7"),
    [PICTURE_PNG_IHDR_CHUNK_FIELD_INTERLACE_OTHER] = TEXT("其它"),
};

HANPSTR GetPng_IHDR_Name(void)
{
    return sg_pPng_IHDR_Name;
}
HANPSTR GetPng_IHDR_FieldName(PICTUREPNGCHUNKFIELD_IHDR eName)
{
    return sg_pChunkFieldName_IHDR[eName];
}
HANPSTR GetPng_IHDR_ColorTypeName(uint8_t cColorType)
{
    HANPSTR pRet;

    switch (cColorType) {
        case 0: { pRet = sg_pPng_IHDR_ColorTypeName[PICTURE_PNG_IHDR_CHUNK_FIELD_COLOR_TYPE_0]; } break;
        case 2: { pRet = sg_pPng_IHDR_ColorTypeName[PICTURE_PNG_IHDR_CHUNK_FIELD_COLOR_TYPE_2]; } break;
        case 3: { pRet = sg_pPng_IHDR_ColorTypeName[PICTURE_PNG_IHDR_CHUNK_FIELD_COLOR_TYPE_3]; } break;
        case 4: { pRet = sg_pPng_IHDR_ColorTypeName[PICTURE_PNG_IHDR_CHUNK_FIELD_COLOR_TYPE_4]; } break;
        case 6: { pRet = sg_pPng_IHDR_ColorTypeName[PICTURE_PNG_IHDR_CHUNK_FIELD_COLOR_TYPE_6]; } break;
        default: { pRet = sg_pPng_IHDR_ColorTypeName[PICTURE_PNG_IHDR_CHUNK_FIELD_COLOR_TYPE_NONE]; } break;
    }

    return pRet;
}
HANPSTR GetPng_IHDR_CompressionName(uint8_t cCompression)
{
    HANPSTR pRet;

    switch (cCompression) {
        case 0: { pRet = sg_pPng_IHDR_CompressionName[PICTURE_PNG_IHDR_CHUNK_FIELD_COMPRESSION_0]; } break;
        default: { pRet = sg_pPng_IHDR_CompressionName[PICTURE_PNG_IHDR_CHUNK_FIELD_COMPRESSION_NOT_SUPPORTED]; } break;
    }

    return pRet;
}
HANPSTR GetPng_IHDR_FilterName(uint8_t cFilter)
{
    HANPSTR pRet;

    switch (cFilter) {
        case 0: { pRet = sg_pPng_IHDR_FilterName[PICTURE_PNG_IHDR_CHUNK_FIELD_FILTER_0]; } break;
        default: { pRet = sg_pPng_IHDR_FilterName[PICTURE_PNG_IHDR_CHUNK_FIELD_FILTER_OTHER]; } break;
    }

    return pRet;
}
HANPSTR GetPng_IHDR_InterlaceName(uint8_t cInterlace)
{
    HANPSTR pRet;

    switch (cInterlace) {
        case 0: { pRet = sg_pPng_IHDR_InterlaceName[PICTURE_PNG_IHDR_CHUNK_FIELD_INTERLACE_0]; } break;
        case 1: { pRet = sg_pPng_IHDR_InterlaceName[PICTURE_PNG_IHDR_CHUNK_FIELD_INTERLACE_1]; } break;
        default: { pRet = sg_pPng_IHDR_InterlaceName[PICTURE_PNG_IHDR_CHUNK_FIELD_INTERLACE_OTHER]; } break;
    }

    return pRet;
}
#endif

#if 1 /******************** gAMA ********************/
static const HANPSTR sg_pPng_gAMA_Name = TEXT("Gamma校正信息");
static const HANPSTR sg_pChunkFieldName_gAMA[PICTURE_PNG_gAMA_CHUNK_FIELD_CNT] = {
    [PICTURE_PNG_gAMA_CHUNK_FIELD_VALUE] = TEXT("Gamma值"),
};

HANPSTR GetPng_gAMA_Name(void)
{
    return sg_pPng_gAMA_Name;
}
HANPSTR GetPng_gAMA_FieldName(PICTUREPNGCHUNKFIELD_gAMA eName)
{
    return sg_pChunkFieldName_gAMA[eName];
}
#endif

#if 1 /******************** pHYs ********************/
typedef enum {
    PICTURE_PNG_pHYs_CHUNK_FIELD_UNIT_0,
    PICTURE_PNG_pHYs_CHUNK_FIELD_UNIT_1,
    PICTURE_PNG_pHYs_CHUNK_FIELD_UNIT_OTHER,
    PICTURE_PNG_pHYs_CHUNK_FIELD_UNIT_CNT,
} PICTUREPNGpHYsCHUNKFIELDUNIT;

static const HANPSTR sg_pPng_pHYs_Name = TEXT("物理像素尺寸");
static const HANPSTR sg_pChunkFieldName_pHYs[PICTURE_PNG_pHYs_CHUNK_FIELD_CNT] = {
    [PICTURE_PNG_pHYs_CHUNK_FIELD_UNIT] = TEXT("物理单位"),
    [PICTURE_PNG_pHYs_CHUNK_FIELD_RESOLUTION_H] = TEXT("水平分辨率"),
    [PICTURE_PNG_pHYs_CHUNK_FIELD_RESOLUTION_V] = TEXT("垂直分辨率"),
    [PICTURE_PNG_pHYs_CHUNK_FIELD_PHY_SIZE] = TEXT("物理尺寸"),
};
static const HANPSTR sg_pPng_pHYs_UnitName[PICTURE_PNG_pHYs_CHUNK_FIELD_UNIT_CNT] = {
    [PICTURE_PNG_pHYs_CHUNK_FIELD_UNIT_0] = TEXT("无"),
    [PICTURE_PNG_pHYs_CHUNK_FIELD_UNIT_1] = TEXT("米"),
    [PICTURE_PNG_pHYs_CHUNK_FIELD_UNIT_OTHER] = TEXT("其它"),
};

HANPSTR GetPng_pHYs_Name(void)
{
    return sg_pPng_pHYs_Name;
}
HANPSTR GetPng_pHYs_FieldName(PICTUREPNGCHUNKFIELD_pHYs eName)
{
    return sg_pChunkFieldName_pHYs[eName];
}
HANPSTR GetPng_pHYs_UnitName(uint8_t cUnit)
{
    HANPSTR pRet;

    switch (cUnit) {
        case 0: { pRet = sg_pPng_pHYs_UnitName[PICTURE_PNG_pHYs_CHUNK_FIELD_UNIT_0]; } break;
        case 1: { pRet = sg_pPng_pHYs_UnitName[PICTURE_PNG_pHYs_CHUNK_FIELD_UNIT_1]; } break;
        default: { pRet = sg_pPng_pHYs_UnitName[PICTURE_PNG_IHDR_CHUNK_FIELD_COLOR_TYPE_NONE]; } break;
    }

    return pRet;
}
#endif

#if 1 /******************** bKGD ********************/
static const HANPSTR sg_pPng_bKGD_Name = TEXT("背景色");
static const HANPSTR sg_pChunkFieldName_bKGD[PICTURE_PNG_bKGD_CHUNK_FIELD_CNT] = {
    [PICTURE_PNG_bKGD_CHUNK_FIELD_BACKGROUND] = TEXT("背景色"),
};

HANPSTR GetPng_bKGD_Name(void)
{
    return sg_pPng_bKGD_Name;
}
HANPSTR GetPng_bKGD_FieldName(PICTUREPNGCHUNKFIELD_bKGD eName)
{
    return sg_pChunkFieldName_bKGD[eName];
}
#endif

#if 1 /******************** tIME ********************/
static const HANPSTR sg_pPng_tIME_Name = TEXT("修改时间");
static const HANPSTR sg_pChunkFieldName_tIME[PICTURE_PNG_tIME_CHUNK_FIELD_CNT] = {
    [PICTURE_PNG_tIME_CHUNK_FIELD_UTC] = TEXT("UTC"),
};

HANPSTR GetPng_tIME_Name(void)
{
    return sg_pPng_tIME_Name;
}
HANPSTR GetPng_tIME_FieldName(PICTUREPNGCHUNKFIELD_tIME eName)
{
    return sg_pChunkFieldName_tIME[eName];
}
#endif

#if 1 /******************** iCCP ********************/
typedef enum {
    PICTURE_PNG_iCCP_CHUNK_FIELD_COMPRESSION_0,
    PICTURE_PNG_iCCP_CHUNK_FIELD_COMPRESSION_NOT_SUPPORTED,
    PICTURE_PNG_iCCP_CHUNK_FIELD_COMPRESSION_CNT,
} PICTUREPNGiCCPCHUNKFIELDCOMPRESSION;

static const HANPSTR sg_pPng_iCCP_Name = TEXT("ICC配置文件");
static const HANPSTR sg_pChunkFieldName_iCCP[PICTURE_PNG_iCCP_CHUNK_FIELD_CNT] = {
    [PICTURE_PNG_iCCP_CHUNK_FIELD_FILE_NAME] = TEXT("文件名"),
    [PICTURE_PNG_iCCP_CHUNK_FIELD_COMPRESSION] = TEXT("压缩方法"),
    [PICTURE_PNG_iCCP_CHUNK_FIELD_DATA] = TEXT("数据"),
};
static const HANPSTR sg_pPng_iCCP_CompressionName[PICTURE_PNG_iCCP_CHUNK_FIELD_COMPRESSION_CNT] = {
    [PICTURE_PNG_iCCP_CHUNK_FIELD_COMPRESSION_0] = TEXT("deflate"),
    [PICTURE_PNG_iCCP_CHUNK_FIELD_COMPRESSION_NOT_SUPPORTED] = TEXT("不支持"),
};

HANPSTR GetPng_iCCP_Name(void)
{
    return sg_pPng_iCCP_Name;
}
HANPSTR GetPng_iCCP_FieldName(PICTUREPNGCHUNKFIELD_iCCP eName)
{
    return sg_pChunkFieldName_iCCP[eName];
}
HANPSTR GetPng_iCCP_CompressionName(uint8_t cCompression)
{
    HANPSTR pRet;

    switch (cCompression) {
        case 0: { pRet = sg_pPng_iCCP_CompressionName[PICTURE_PNG_iCCP_CHUNK_FIELD_COMPRESSION_0]; } break;
        default: { pRet = sg_pPng_iCCP_CompressionName[PICTURE_PNG_iCCP_CHUNK_FIELD_COMPRESSION_NOT_SUPPORTED]; } break;
    }

    return pRet;
}
#endif

#if 1 /******************** cHRM ********************/
static const HANPSTR sg_pPng_cHRM_Name = TEXT("基色和白色点");
static const HANPSTR sg_pChunkFieldName_cHRM[PICTURE_PNG_cHRM_CHUNK_FIELD_CNT] = {
    [PICTURE_PNG_cHRM_CHUNK_FIELD_WHITE_POSITION] = TEXT("白色坐标"),
    [PICTURE_PNG_cHRM_CHUNK_FIELD_RED_POSITION] = TEXT("红色坐标"),
    [PICTURE_PNG_cHRM_CHUNK_FIELD_GREEN_POSITION] = TEXT("绿色坐标"),
    [PICTURE_PNG_cHRM_CHUNK_FIELD_BLUE_POSITION] = TEXT("蓝色坐标"),
};

HANPSTR GetPng_cHRM_Name(void)
{
    return sg_pPng_cHRM_Name;
}
HANPSTR GetPng_cHRM_FieldName(PICTUREPNGCHUNKFIELD_cHRM eName)
{
    return sg_pChunkFieldName_cHRM[eName];
}
#endif

#if 1 /******************** PLTE ********************/
static const HANPSTR sg_pPng_PLTE_Name = TEXT("调色板");

HANPSTR GetPng_PLTE_Name(void)
{
    return sg_pPng_PLTE_Name;
}
#endif

#if 1 /******************** tRNS ********************/
static const HANPSTR sg_pPng_tRNS_Name = TEXT("透明度");

HANPSTR GetPng_tRNS_Name(void)
{
    return sg_pPng_tRNS_Name;
}
#endif

#if 1 /******************** IDAT ********************/
static const HANPSTR sg_pPng_IDAT_Name = TEXT("图像数据");
static const HANPSTR sg_pChunkFieldName_IDAT[PICTURE_PNG_IDAT_CHUNK_FIELD_CNT] = {
    [PICTURE_PNG_IDAT_CHUNK_FIELD_LEN] = TEXT("长度"),
    [PICTURE_PNG_IDAT_CHUNK_FIELD_DATA] = TEXT("数据"),
};

HANPSTR GetPng_IDAT_Name(void)
{
    return sg_pPng_IDAT_Name;
}
HANPSTR GetPng_IDAT_FieldName(PICTUREPNGCHUNKFIELD_IDAT eName)
{
    return sg_pChunkFieldName_IDAT[eName];
}
#endif

#if 1 /******************** tEXT ********************/
static const HANPSTR sg_pPng_tEXT_Name = TEXT("文本数据");

HANPSTR GetPng_tEXT_Name(void)
{
    return sg_pPng_tEXT_Name;
}
#endif
