#include "HAN_PictureJpegPhotoshop8BIM.h"

typedef enum {
    PHOTOSHOP_8BIM_RESOLUTION_UNIT_1,
    PHOTOSHOP_8BIM_RESOLUTION_UNIT_2,
    PHOTOSHOP_8BIM_RESOLUTION_UNIT_DEFAULT,
    PHOTOSHOP_8BIM_RESOLUTION_UNIT_CNT,
} PHOTOSHOP8BIMRESOLUTIONUNIT;
typedef enum {
    PHOTOSHOP_8BIM_COPY_RIGHT_FLAG_0,
    PHOTOSHOP_8BIM_COPY_RIGHT_FLAG_1,
    PHOTOSHOP_8BIM_COPY_RIGHT_FLAG_DEFAULT,
    PHOTOSHOP_8BIM_COPY_RIGHT_FLAG_CNT,
} PHOTOSHOP8BIMCOPYRIGHTFLAG;
typedef enum {
    PHOTOSHOP_8BIM_PRINT_SCALE_INFO_PRINT_STYLE_0,
    PHOTOSHOP_8BIM_PRINT_SCALE_INFO_PRINT_STYLE_1,
    PHOTOSHOP_8BIM_PRINT_SCALE_INFO_PRINT_STYLE_2,
    PHOTOSHOP_8BIM_PRINT_SCALE_INFO_PRINT_STYLE_DEFAULT,
    PHOTOSHOP_8BIM_PRINT_SCALE_INFO_PRINT_STYLE_CNT,
} PHOTOSHOP8BIMPRINTSCALEINFOPRINTSTYLE;
typedef enum {
    PHOTOSHOP_8BIM_THUMBNAIL_FORMAT_0,
    PHOTOSHOP_8BIM_THUMBNAIL_FORMAT_1,
    PHOTOSHOP_8BIM_THUMBNAIL_FORMAT_DEFAULT,
    PHOTOSHOP_8BIM_THUMBNAIL_FORMAT_CNT,
} PHOTOSHOP8BIMTHUMBNAILFORMAT;

static const HANPSTR sg_pResolutionUnit[PHOTOSHOP_8BIM_RESOLUTION_UNIT_CNT] = {
    [PHOTOSHOP_8BIM_RESOLUTION_UNIT_1] = TEXT("英寸"),
    [PHOTOSHOP_8BIM_RESOLUTION_UNIT_2] = TEXT("厘米"),
    [PHOTOSHOP_8BIM_RESOLUTION_UNIT_DEFAULT] = TEXT("未知"),
};
static const HANPSTR sg_pCopyRightFlagName[PHOTOSHOP_8BIM_COPY_RIGHT_FLAG_CNT] = {
    [PHOTOSHOP_8BIM_COPY_RIGHT_FLAG_0] = TEXT("无版权"),
    [PHOTOSHOP_8BIM_COPY_RIGHT_FLAG_1] = TEXT("有版权"),
    [PHOTOSHOP_8BIM_COPY_RIGHT_FLAG_DEFAULT] = TEXT("未知"),
};
static const HANPSTR sg_pPrintScaleInfoPrintStyleName[PHOTOSHOP_8BIM_PRINT_SCALE_INFO_PRINT_STYLE_CNT] = {
    [PHOTOSHOP_8BIM_PRINT_SCALE_INFO_PRINT_STYLE_0] = TEXT("自适应居中"),
    [PHOTOSHOP_8BIM_PRINT_SCALE_INFO_PRINT_STYLE_1] = TEXT("自定义缩放"),
    [PHOTOSHOP_8BIM_PRINT_SCALE_INFO_PRINT_STYLE_2] = TEXT("无缩放"),
    [PHOTOSHOP_8BIM_PRINT_SCALE_INFO_PRINT_STYLE_DEFAULT] = TEXT("未知"),
};
static const HANPSTR sg_pThumbnailFormatName[PHOTOSHOP_8BIM_THUMBNAIL_FORMAT_CNT] = {
    [PHOTOSHOP_8BIM_THUMBNAIL_FORMAT_0] = TEXT("RAW数据"),
    [PHOTOSHOP_8BIM_THUMBNAIL_FORMAT_1] = TEXT("JPEG数据"),
    [PHOTOSHOP_8BIM_THUMBNAIL_FORMAT_DEFAULT] = TEXT("未知格式"),
};

HANPSTR GetJpeg_APP13_PhotoshopResolutionUnitName(uint16_t cUnit)
{
    HANPSTR pRet;

    switch(cUnit) {
        case 1: { pRet = sg_pResolutionUnit[PHOTOSHOP_8BIM_RESOLUTION_UNIT_1]; } break;
        case 2: { pRet = sg_pResolutionUnit[PHOTOSHOP_8BIM_RESOLUTION_UNIT_2]; } break;
        default: { pRet = sg_pResolutionUnit[PHOTOSHOP_8BIM_RESOLUTION_UNIT_DEFAULT]; } break;
    }

    return pRet;
}
HANPSTR GetJpeg_APP13_PhotoshopCopyRightFlagName(uint8_t cFlag)
{
    HANPSTR pRet;

    switch(cFlag) {
        case 0: { pRet = sg_pCopyRightFlagName[PHOTOSHOP_8BIM_COPY_RIGHT_FLAG_0]; } break;
        case 1: { pRet = sg_pCopyRightFlagName[PHOTOSHOP_8BIM_COPY_RIGHT_FLAG_1]; } break;
        default: { pRet = sg_pCopyRightFlagName[PHOTOSHOP_8BIM_COPY_RIGHT_FLAG_DEFAULT]; } break;
    }

    return pRet;
}
HANPSTR GetJpeg_APP13_PhotoshopPrintScaleInfoPrintStyleName(uint16_t cStyle)
{
    HANPSTR pRet;

    switch(cStyle) {
        case 0: { pRet = sg_pPrintScaleInfoPrintStyleName[PHOTOSHOP_8BIM_PRINT_SCALE_INFO_PRINT_STYLE_0]; } break;
        case 1: { pRet = sg_pPrintScaleInfoPrintStyleName[PHOTOSHOP_8BIM_PRINT_SCALE_INFO_PRINT_STYLE_1]; } break;
        case 2: { pRet = sg_pPrintScaleInfoPrintStyleName[PHOTOSHOP_8BIM_PRINT_SCALE_INFO_PRINT_STYLE_1]; } break;
        default: { pRet = sg_pPrintScaleInfoPrintStyleName[PHOTOSHOP_8BIM_PRINT_SCALE_INFO_PRINT_STYLE_DEFAULT]; } break;
    }

    return pRet;
}
HANPSTR GetJpeg_APP13_PhotoshopThumbnailFormatName(uint32_t nFormat)
{
    HANPSTR pRet;

    switch(nFormat) {
        case 0: { pRet = sg_pThumbnailFormatName[PHOTOSHOP_8BIM_THUMBNAIL_FORMAT_0]; } break;
        case 1: { pRet = sg_pThumbnailFormatName[PHOTOSHOP_8BIM_THUMBNAIL_FORMAT_1]; } break;
        default: { pRet = sg_pThumbnailFormatName[PHOTOSHOP_8BIM_THUMBNAIL_FORMAT_DEFAULT]; } break;
    }

    return pRet;
}
