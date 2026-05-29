#include "HAN_VideoMP4Box.h"
#include "HAN_VideoMP4Def.h"

typedef enum {
    VIDEO_MP4_BOX_FIELD_BOX_VERSION_0,
    VIDEO_MP4_BOX_FIELD_BOX_VERSION_1,
    VIDEO_MP4_BOX_FIELD_BOX_VERSION_DEFAULT,
    VIDEO_MP4_BOX_FIELD_BOX_VERSION_CNT,
} VEDIOMP4BOXFIELDBOXVERSION;

static const HANPSTR sg_pMP4_BoxVersionName[VIDEO_MP4_BOX_FIELD_BOX_VERSION_CNT] = {
    [VIDEO_MP4_BOX_FIELD_BOX_VERSION_0] = TEXT("32位"),
    [VIDEO_MP4_BOX_FIELD_BOX_VERSION_1] = TEXT("64位"),
    [VIDEO_MP4_BOX_FIELD_BOX_VERSION_DEFAULT] = TEXT("未知"),
};

HANPSTR GetMP4BoxVersionName(uint8_t nVersion)
{
    HANPSTR pRet;

    switch (nVersion) {
        case 0: { pRet = sg_pMP4_BoxVersionName[VIDEO_MP4_BOX_FIELD_BOX_VERSION_0]; } break;
        case 1: { pRet = sg_pMP4_BoxVersionName[VIDEO_MP4_BOX_FIELD_BOX_VERSION_1]; } break;
        default: { pRet = sg_pMP4_BoxVersionName[VIDEO_MP4_BOX_FIELD_BOX_VERSION_DEFAULT]; } break;
    }

    return pRet;
}

#if 1 /******************** ftyp ********************/
static const HANPSTR sg_pMP4_ftyp_Name = TEXT("文件类型");
static const HANPSTR sg_pBoxFieldName_ftyp[VIDEO_MP4_ftyp_BOX_FIELD_CNT] = {
    [VIDEO_MP4_ftyp_BOX_FIELD_MAJOR_BRAND] = TEXT("主要解码格式"),
    [VIDEO_MP4_ftyp_BOX_FIELD_MINOR_VERSION] = TEXT("次要版本号"),
    [VIDEO_MP4_ftyp_BOX_FIELD_COMPATIBLE_BRANDS] = TEXT("兼容格式"),
};

HANPSTR GetMP4_ftyp_Name(void)
{
    return sg_pMP4_ftyp_Name;
}
HANPSTR GetMP4_ftyp_FieldName(VIDEOMP4BOXFIELD_ftyp eName)
{
    return sg_pBoxFieldName_ftyp[eName];
}
#endif

#if 1 /******************** free ********************/
static const HANPSTR sg_pMP4_free_Name = TEXT("占位符");
static const HANPSTR sg_pBoxFieldName_free[VIDEO_MP4_free_BOX_FIELD_CNT] = {
    [VIDEO_MP4_free_BOX_FIELD_LEN] = TEXT("长度"),
    [VIDEO_MP4_free_BOX_FIELD_DATA] = TEXT("数据"),
    [VIDEO_MP4_free_BOX_FIELD_TEXT] = TEXT("文本"),
};

HANPSTR GetMP4_free_Name(void)
{
    return sg_pMP4_free_Name;
}
HANPSTR GetMP4_free_FieldName(VIDEOMP4BOXFIELD_free eName)
{
    return sg_pBoxFieldName_free[eName];
}
#endif

#if 1 /******************** moov ********************/
static const HANPSTR sg_pMP4_moov_Name = TEXT("媒体元数据信息");

HANPSTR GetMP4_moov_Name(void)
{
    return sg_pMP4_moov_Name;
}
#endif

#if 1 /******************** mvhd ********************/
static const HANPSTR sg_pMP4_mvhd_Name = TEXT("媒体数据头");
static const HANPSTR sg_pBoxFieldName_mvhd[VIDEO_MP4_mvhd_BOX_FIELD_CNT] = {
    [VIDEO_MP4_mvhd_BOX_FIELD_VERSION] = TEXT("版本"),
    [VIDEO_MP4_mvhd_BOX_FIELD_FLAGS] = TEXT("标志"),
    [VIDEO_MP4_mvhd_BOX_FIELD_CREATION_TIME] = TEXT("创建时间"),
    [VIDEO_MP4_mvhd_BOX_FIELD_MODIFICATION_TIME] = TEXT("修改时间"),
    [VIDEO_MP4_mvhd_BOX_FIELD_DURATION] = TEXT("时长"),
    [VIDEO_MP4_mvhd_BOX_FIELD_RATE] = TEXT("播放速率"),
    [VIDEO_MP4_mvhd_BOX_FIELD_VOLUME] = TEXT("音量"),
    [VIDEO_MP4_mvhd_BOX_FIELD_MATRIX] = TEXT("视频变换矩阵"),
    [VIDEO_MP4_mvhd_BOX_FIELD_NEXT_TRACK_ID] = TEXT("下一个轨道的 ID"),
};

HANPSTR GetMP4_mvhd_Name(void)
{
    return sg_pMP4_mvhd_Name;
}
HANPSTR GetMP4_mvhd_FieldName(VIDEOMP4BOXFIELD_mvhd eName)
{
    return sg_pBoxFieldName_mvhd[eName];
}
HANPSTR GetMP4_mvhd_VersionName(uint8_t nVersion)
{
    return GetMP4BoxVersionName(nVersion);
}
#endif

#if 1 /******************** trak ********************/
static const HANPSTR sg_pMP4_trak_Name = TEXT("轨道数据信息");

HANPSTR GetMP4_trak_Name(void)
{
    return sg_pMP4_trak_Name;
}
#endif

#if 1 /******************** tkhd ********************/
typedef enum {
    VIDEO_MP4_tkhd_BOX_FIELD_FLAGS_000001,
    VIDEO_MP4_tkhd_BOX_FIELD_FLAGS_000002,
    VIDEO_MP4_tkhd_BOX_FIELD_FLAGS_000004,
    VIDEO_MP4_tkhd_BOX_FIELD_FLAGS_000008,
    VIDEO_MP4_tkhd_BOX_FIELD_FLAGS_CNT,
} VEDIOMP4tkhdBOXFIELDFLAGS;

static const HANPSTR sg_pMP4_tkhd_Name = TEXT("轨道全局属性");
static const HANPSTR sg_pBoxFieldName_tkhd[VIDEO_MP4_tkhd_BOX_FIELD_CNT] = {
    [VIDEO_MP4_tkhd_BOX_FIELD_VERSION] = TEXT("版本"),
    [VIDEO_MP4_tkhd_BOX_FIELD_FLAGS] = TEXT("标志"),
    [VIDEO_MP4_tkhd_BOX_FIELD_CREATION_TIME] = TEXT("创建时间"),
    [VIDEO_MP4_tkhd_BOX_FIELD_MODIFICATION_TIME] = TEXT("修改时间"),
    [VIDEO_MP4_tkhd_BOX_FIELD_TRACK_ID] = TEXT("轨道ID"),
    [VIDEO_MP4_tkhd_BOX_FIELD_DURATION] = TEXT("时长"),
    [VIDEO_MP4_tkhd_BOX_FIELD_LAYER] = TEXT("视频层"),
    [VIDEO_MP4_tkhd_BOX_FIELD_ALTERNATE_GROUP] = TEXT("替代组"),
    [VIDEO_MP4_tkhd_BOX_FIELD_VOLUME] = TEXT("音量"),
    [VIDEO_MP4_tkhd_BOX_FIELD_MATRIX] = TEXT("变换矩阵"),
    [VIDEO_MP4_tkhd_BOX_FIELD_RESOLUTION] = TEXT("分辨率"),
};
static const HANPSTR sg_pMP4_tkhd_FlagsName[VIDEO_MP4_tkhd_BOX_FIELD_FLAGS_CNT] = {
    [VIDEO_MP4_tkhd_BOX_FIELD_FLAGS_000001] = TEXT("轨道启用"),
    [VIDEO_MP4_tkhd_BOX_FIELD_FLAGS_000002] = TEXT("视频"),
    [VIDEO_MP4_tkhd_BOX_FIELD_FLAGS_000004] = TEXT("预览"),
    [VIDEO_MP4_tkhd_BOX_FIELD_FLAGS_000008] = TEXT("封面"),
};

HANPSTR GetMP4_tkhd_Name(void)
{
    return sg_pMP4_tkhd_Name;
}
HANPSTR GetMP4_tkhd_FieldName(VIDEOMP4BOXFIELD_tkhd eName)
{
    return sg_pBoxFieldName_tkhd[eName];
}
HANPSTR GetMP4_tkhd_VersionName(uint8_t nVersion)
{
    return GetMP4BoxVersionName(nVersion);
}
void GetMP4_tkhd_FlagsName(uint32_t cFlags, HANPSTR pText, HANSIZE nLen)
{
    HANCHAR pTarget[HAN_VIDEO_MP4_TEXT_BUF_SIZE];
    HANSIZE nOffsetText = 0;
    HANSIZE nOffsetTar = 0;
    HANSIZE nSize;
    BOOL bFirst = TRUE;
    
    if (0 != (cFlags & 0x000001))
    {
        HAN_snprintf(pText, nLen, TEXT("%s"), sg_pMP4_tkhd_FlagsName[VIDEO_MP4_tkhd_BOX_FIELD_FLAGS_000001]);
        pText[nLen - 1] = TEXT('\0');
        nOffsetText = HAN_strlen(pText);
    }
    if (1 < cFlags)
    {
        HAN_strncat(pText, TEXT("，"), HAN_VIDEO_MP4_TEXT_BUF_SIZE - nOffsetText);
        nOffsetText += HAN_strlen(&pText[nOffsetText]);
        if (0 != (cFlags & 0x000002))
        {
            nSize = HAN_VIDEO_MP4_TEXT_BUF_SIZE - nOffsetTar;
            if (TRUE == bFirst) { HAN_snprintf(&pTarget[nOffsetTar], nSize, TEXT("%s"), sg_pMP4_tkhd_FlagsName[VIDEO_MP4_tkhd_BOX_FIELD_FLAGS_000002]); }
            else { HAN_snprintf(&pTarget[nOffsetTar], nSize, TEXT("、%s"), sg_pMP4_tkhd_FlagsName[VIDEO_MP4_tkhd_BOX_FIELD_FLAGS_000002]); }
            nOffsetTar += HAN_strlen(&pTarget[nOffsetTar]);
            bFirst = FALSE;
        }
        if (0 != (cFlags & 0x000004))
        {
            nSize = HAN_VIDEO_MP4_TEXT_BUF_SIZE - nOffsetTar;
            if (TRUE == bFirst) { HAN_snprintf(&pTarget[nOffsetTar], nSize, TEXT("%s"), sg_pMP4_tkhd_FlagsName[VIDEO_MP4_tkhd_BOX_FIELD_FLAGS_000004]); }
            else { HAN_snprintf(&pTarget[nOffsetTar], nSize, TEXT("、%s"), sg_pMP4_tkhd_FlagsName[VIDEO_MP4_tkhd_BOX_FIELD_FLAGS_000004]); }
            nOffsetTar += HAN_strlen(&pTarget[nOffsetTar]);
            bFirst = FALSE;
        }
        if (0 != (cFlags & 0x000008))
        {
            nSize = HAN_VIDEO_MP4_TEXT_BUF_SIZE - nOffsetTar;
            if (TRUE == bFirst) { HAN_snprintf(&pTarget[nOffsetTar], nSize, TEXT("%s"), sg_pMP4_tkhd_FlagsName[VIDEO_MP4_tkhd_BOX_FIELD_FLAGS_000008]); }
            else { HAN_snprintf(&pTarget[nOffsetTar], nSize, TEXT("、%s"), sg_pMP4_tkhd_FlagsName[VIDEO_MP4_tkhd_BOX_FIELD_FLAGS_000008]); }
            nOffsetTar += HAN_strlen(&pTarget[nOffsetTar]);
            bFirst = FALSE;
        }
        HAN_snprintf(&pText[nOffsetText], nLen - nOffsetText, TEXT("轨道在%s中使用"), pTarget);
    }
}
#endif

#if 1 /******************** edts ********************/
static const HANPSTR sg_pMP4_edts_Name = TEXT("编辑列表容器");

HANPSTR GetMP4_edts_Name(void)
{
    return sg_pMP4_edts_Name;
}
#endif

#if 1 /******************** elst ********************/
static const HANPSTR sg_pMP4_elst_Name = TEXT("编辑列表");

HANPSTR GetMP4_elst_Name(void)
{
    return sg_pMP4_elst_Name;
}
#endif

#if 1 /******************** mdia ********************/
static const HANPSTR sg_pMP4_mdia_Name = TEXT("轨道媒体信息容器");

HANPSTR GetMP4_mdia_Name(void)
{
    return sg_pMP4_mdia_Name;
}
#endif

#if 1 /******************** mdhd ********************/
static const HANPSTR sg_pMP4_mdhd_Name = TEXT("轨道媒体独立信息");
static const HANPSTR sg_pBoxFieldName_mdhd[VIDEO_MP4_mdhd_BOX_FIELD_CNT] = {
    [VIDEO_MP4_mdhd_BOX_FIELD_VERSION] = TEXT("版本"),
    [VIDEO_MP4_mdhd_BOX_FIELD_FLAGS] = TEXT("标志"),
    [VIDEO_MP4_mdhd_BOX_FIELD_CREATION_TIME] = TEXT("创建时间"),
    [VIDEO_MP4_mdhd_BOX_FIELD_MODIFICATION_TIME] = TEXT("修改时间"),
    [VIDEO_MP4_mdhd_BOX_FIELD_DURATION] = TEXT("时长"),
    [VIDEO_MP4_mdhd_BOX_FIELD_LANGUAGE] = TEXT("语言"),
};

HANPSTR GetMP4_mdhd_Name(void)
{
    return sg_pMP4_mdhd_Name;
}
HANPSTR GetMP4_mdhd_FieldName(VIDEOMP4BOXFIELD_mdhd eName)
{
    return sg_pBoxFieldName_mdhd[eName];
}
HANPSTR GetMP4_mdhd_VersionName(uint8_t nVersion)
{
    return GetMP4BoxVersionName(nVersion);
}
#endif

#if 1 /******************** hdlr ********************/
static const HANPSTR sg_pMP4_hdlr_Name = TEXT("轨道媒体独立信息");
static const HANPSTR sg_pBoxFieldName_hdlr[VIDEO_MP4_hdlr_BOX_FIELD_CNT] = {
    [VIDEO_MP4_hdlr_BOX_FIELD_VERSION] = TEXT("版本"),
    [VIDEO_MP4_hdlr_BOX_FIELD_FLAGS] = TEXT("标志"),
    [VIDEO_MP4_hdlr_BOX_FIELD_HANDLER_TYPE] = TEXT("处理器类型"),
    [VIDEO_MP4_hdlr_BOX_FIELD_NAME] = TEXT("名称"),
};
static const HANPSTR sg_pMP4_hdlr_HandlerTypeName[VIDEO_MP4_TRACK_HANDLER_TYPE_CNT + 1] = {
    [VIDEO_MP4_TRACK_HANDLER_TYPE_VIDEO] = TEXT("视频轨道"),
    [VIDEO_MP4_TRACK_HANDLER_TYPE_SOUND] = TEXT("音频轨道"),
    [VIDEO_MP4_TRACK_HANDLER_TYPE_SUBTITLE] = TEXT("字幕轨道"),
    [VIDEO_MP4_TRACK_HANDLER_TYPE_HINT] = TEXT("流媒体提示轨道"),
    [VIDEO_MP4_TRACK_HANDLER_TYPE_METADATA] = TEXT("元数据轨道"),
    [VIDEO_MP4_TRACK_HANDLER_TYPE_TEXT] = TEXT("文本轨道"),
    [VIDEO_MP4_TRACK_HANDLER_TYPE_TIMECODE] = TEXT("时间码轨道"),
    [VIDEO_MP4_TRACK_HANDLER_TYPE_CNT] = TEXT("未知轨道"),
};

HANPSTR GetMP4_hdlr_Name(void)
{
    return sg_pMP4_hdlr_Name;
}
HANPSTR GetMP4_hdlr_FieldName(VIDEOMP4BOXFIELD_hdlr eName)
{
    return sg_pBoxFieldName_hdlr[eName];
}
HANPSTR GetMP4_hdlr_HandlerTypeName(VIDEOMP4TRACKHANDLERTYPE eType)
{
    return sg_pMP4_hdlr_HandlerTypeName[eType];
}
#endif

#if 1 /******************** minf ********************/
static const HANPSTR sg_pMP4_minf_Name = TEXT("轨道媒体信息");

HANPSTR GetMP4_minf_Name(void)
{
    return sg_pMP4_minf_Name;
}
#endif

#if 1 /******************** vmhd ********************/
static const HANPSTR sg_pMP4_vmhd_Name = TEXT("视频媒体头");
static const HANPSTR sg_pBoxFieldName_vmhd[VIDEO_MP4_vmhd_BOX_FIELD_CNT] = {
    [VIDEO_MP4_vmhd_BOX_FIELD_VERSION] = TEXT("版本"),
    [VIDEO_MP4_vmhd_BOX_FIELD_FLAGS] = TEXT("标志"),
    [VIDEO_MP4_vmhd_BOX_FIELD_GRAPHICS_MODE] = TEXT("图形合成模式"),
    [VIDEO_MP4_vmhd_BOX_FIELD_OP_COLOR] = TEXT("操作颜色"),
};
static const HANPSTR sg_pMP4_vmhd_GraphicsModeName[VIDEO_MP4_VIDEO_GRAPHICS_MODE_CNT + 1] = {
    [VIDEO_MP4_VIDEO_GRAPHICS_MODE_COPY] = TEXT("直接复制"),
    [VIDEO_MP4_VIDEO_GRAPHICS_MODE_BLEND] = TEXT("混合模式"),
    [VIDEO_MP4_VIDEO_GRAPHICS_MODE_TRANSPARENT] = TEXT("透明模式"),
    [VIDEO_MP4_VIDEO_GRAPHICS_MODE_DITHER_COPY] = TEXT("抖动复制"),
    [VIDEO_MP4_VIDEO_GRAPHICS_MODE_ADD] = TEXT("加法混合"),
    [VIDEO_MP4_VIDEO_GRAPHICS_MODE_ADD_PIN] = TEXT("加法固定"),
    [VIDEO_MP4_VIDEO_GRAPHICS_MODE_SUB] = TEXT("减法混合"),
    [VIDEO_MP4_VIDEO_GRAPHICS_MODE_SUB_PIN] = TEXT("减法固定"),
    [VIDEO_MP4_VIDEO_GRAPHICS_MODE_AD_MAX] = TEXT("取最大值"),
    [VIDEO_MP4_VIDEO_GRAPHICS_MODE_AD_MIN] = TEXT("取最小值"),
    [VIDEO_MP4_VIDEO_GRAPHICS_MODE_INVERT] = TEXT("反转颜色"),
    [VIDEO_MP4_VIDEO_GRAPHICS_MODE_INVERT_ADD] = TEXT("反转后加法"),
    [VIDEO_MP4_VIDEO_GRAPHICS_MODE_ADD_OVER] = TEXT("叠加加法"),
    [VIDEO_MP4_VIDEO_GRAPHICS_MODE_HIGHLIGHT] = TEXT("高亮模式"),
    [VIDEO_MP4_VIDEO_GRAPHICS_MODE_CNT] = TEXT("未知模式"),
};

HANPSTR GetMP4_vmhd_Name(void)
{
    return sg_pMP4_vmhd_Name;
}
HANPSTR GetMP4_vmhd_FieldName(VIDEOMP4BOXFIELD_vmhd eName)
{
    return sg_pBoxFieldName_vmhd[eName];
}
HANPSTR GetMP4_vmhd_GraphicsModeName(uint16_t cMode)
{
    HANPSTR pRet;

    switch (cMode) {
        case 0x0000: { pRet = sg_pMP4_vmhd_GraphicsModeName[VIDEO_MP4_VIDEO_GRAPHICS_MODE_COPY]; } break;
        case 0x0020: { pRet = sg_pMP4_vmhd_GraphicsModeName[VIDEO_MP4_VIDEO_GRAPHICS_MODE_BLEND]; } break;
        case 0x0024: { pRet = sg_pMP4_vmhd_GraphicsModeName[VIDEO_MP4_VIDEO_GRAPHICS_MODE_TRANSPARENT]; } break;
        case 0x0100: { pRet = sg_pMP4_vmhd_GraphicsModeName[VIDEO_MP4_VIDEO_GRAPHICS_MODE_DITHER_COPY]; } break;
        case 0x0200: { pRet = sg_pMP4_vmhd_GraphicsModeName[VIDEO_MP4_VIDEO_GRAPHICS_MODE_ADD]; } break;
        case 0x0300: { pRet = sg_pMP4_vmhd_GraphicsModeName[VIDEO_MP4_VIDEO_GRAPHICS_MODE_ADD_PIN]; } break;
        case 0x0400: { pRet = sg_pMP4_vmhd_GraphicsModeName[VIDEO_MP4_VIDEO_GRAPHICS_MODE_SUB]; } break;
        case 0x0500: { pRet = sg_pMP4_vmhd_GraphicsModeName[VIDEO_MP4_VIDEO_GRAPHICS_MODE_SUB_PIN]; } break;
        case 0x0600: { pRet = sg_pMP4_vmhd_GraphicsModeName[VIDEO_MP4_VIDEO_GRAPHICS_MODE_AD_MAX]; } break;
        case 0x0700: { pRet = sg_pMP4_vmhd_GraphicsModeName[VIDEO_MP4_VIDEO_GRAPHICS_MODE_AD_MIN]; } break;
        case 0x0800: { pRet = sg_pMP4_vmhd_GraphicsModeName[VIDEO_MP4_VIDEO_GRAPHICS_MODE_INVERT]; } break;
        case 0x0900: { pRet = sg_pMP4_vmhd_GraphicsModeName[VIDEO_MP4_VIDEO_GRAPHICS_MODE_INVERT_ADD]; } break;
        case 0x1000: { pRet = sg_pMP4_vmhd_GraphicsModeName[VIDEO_MP4_VIDEO_GRAPHICS_MODE_ADD_OVER]; } break;
        case 0x2000: { pRet = sg_pMP4_vmhd_GraphicsModeName[VIDEO_MP4_VIDEO_GRAPHICS_MODE_HIGHLIGHT]; } break;
        default: { pRet = sg_pMP4_vmhd_GraphicsModeName[VIDEO_MP4_VIDEO_GRAPHICS_MODE_CNT]; } break;
    }

    return pRet;
}
#endif

#if 1 /******************** dinf ********************/
static const HANPSTR sg_pMP4_dinf_Name = TEXT("数据位置信息");

HANPSTR GetMP4_dinf_Name(void)
{
    return sg_pMP4_dinf_Name;
}
#endif

#if 1 /******************** dref ********************/
static const HANPSTR sg_pMP4_dref_Name = TEXT("数据引用");

HANPSTR GetMP4_dref_Name(void)
{
    return sg_pMP4_dref_Name;
}
#endif

#if 1 /******************** url  ********************/
typedef enum {
    VIDEO_MP4_url__BOX_FIELD_FLAGS_000000,
    VIDEO_MP4_url__BOX_FIELD_FLAGS_000001,
    VIDEO_MP4_url__BOX_FIELD_FLAGS_DEFAULT,
    VIDEO_MP4_url__BOX_FIELD_FLAGS_CNT,
} VEDIOMP4url_BOXFIELDFLAGS;

static const HANPSTR sg_pMP4_url__Name = TEXT("数据入口Url");
static const HANPSTR sg_pBoxFieldName_url_[VIDEO_MP4_url__BOX_FIELD_CNT] = {
    [VIDEO_MP4_url__BOX_FIELD_VERSION] = TEXT("版本"),
    [VIDEO_MP4_url__BOX_FIELD_FLAGS] = TEXT("标志"),
    [VIDEO_MP4_url__BOX_FIELD_LOCATION] = TEXT("引用位置"),
};
static const HANPSTR sg_pMP4_url__FlagsName[VIDEO_MP4_url__BOX_FIELD_FLAGS_CNT] = {
    [VIDEO_MP4_url__BOX_FIELD_FLAGS_000000] = TEXT("自引用"),
    [VIDEO_MP4_url__BOX_FIELD_FLAGS_000001] = TEXT("外部引用"),
    [VIDEO_MP4_url__BOX_FIELD_FLAGS_DEFAULT] = TEXT("未知引用"),
};

HANPSTR GetMP4_url__Name(void)
{
    return sg_pMP4_url__Name;
}
HANPSTR GetMP4_url__FieldName(VIDEOMP4BOXFIELD_url_ eName)
{
    return sg_pBoxFieldName_url_[eName];
}
HANPSTR GetMP4_url__FlagsName(uint32_t cFlags)
{
    HANPSTR pRet;

    switch (cFlags) {
        case 0x000000: { pRet = sg_pMP4_url__FlagsName[VIDEO_MP4_url__BOX_FIELD_FLAGS_000000]; } break;
        case 0x000001: { pRet = sg_pMP4_url__FlagsName[VIDEO_MP4_url__BOX_FIELD_FLAGS_000001]; } break;
        default: { pRet = sg_pMP4_url__FlagsName[VIDEO_MP4_url__BOX_FIELD_FLAGS_DEFAULT]; } break;
    }

    return pRet;
}
#endif

#if 1 /******************** stbl ********************/
static const HANPSTR sg_pMP4_stbl_Name = TEXT("样本索引表");

HANPSTR GetMP4_stbl_Name(void)
{
    return sg_pMP4_stbl_Name;
}
#endif

#if 1 /******************** stsd ********************/
static const HANPSTR sg_pMP4_stsd_Name = TEXT("样本索引表");

HANPSTR GetMP4_stsd_Name(void)
{
    return sg_pMP4_stsd_Name;
}
#endif

#if 1 /******************** avc1 ********************/
static const HANPSTR sg_pMP4_avc1_Name = TEXT("H.264/AVC采样入口");
static const HANPSTR sg_pBoxFieldName_avc1[VIDEO_MP4_avc1_BOX_FIELD_CNT] = {
    [VIDEO_MP4_avc1_BOX_FIELD_DATA_REF_INDEX] = TEXT("数据引用索引"),
    [VIDEO_MP4_avc1_BOX_FIELD_VERSION] = TEXT("版本"),
    [VIDEO_MP4_avc1_BOX_FIELD_REVISION] = TEXT("修订"),
    [VIDEO_MP4_avc1_BOX_FIELD_VENDOR] = TEXT("供应商"),
    [VIDEO_MP4_avc1_BOX_FIELD_TEMPORAL_QUALITY] = TEXT("时间质量"),
    [VIDEO_MP4_avc1_BOX_FIELD_SPATIAL_QUALITY] = TEXT("空间质量"),
    [VIDEO_MP4_avc1_BOX_FIELD_WIDTH] = TEXT("宽度（像素）"),
    [VIDEO_MP4_avc1_BOX_FIELD_HEIGHT] = TEXT("高度（像素）"),
    [VIDEO_MP4_avc1_BOX_FIELD_HORIZ_RESOLUTION] = TEXT("水平分辨率"),
    [VIDEO_MP4_avc1_BOX_FIELD_VERT_RESOLUTION] = TEXT("垂直分辨率"),
    [VIDEO_MP4_avc1_BOX_FIELD_DATA_SIZE] = TEXT("数据大小"),
    [VIDEO_MP4_avc1_BOX_FIELD_FRAME_COUNT] = TEXT("每样本帧数"),
    [VIDEO_MP4_avc1_BOX_FIELD_COMPRESSOR_NAME] = TEXT("压缩器名称"),
    [VIDEO_MP4_avc1_BOX_FIELD_DEPTH] = TEXT("颜色深度"),
    [VIDEO_MP4_avc1_BOX_FIELD_COLOR_TABLE] = TEXT("颜色表ID"),
};

HANPSTR GetMP4_avc1_Name(void)
{
    return sg_pMP4_avc1_Name;
}
HANPSTR GetMP4_avc1_FieldName(VIDEOMP4BOXFIELD_avc1 eName)
{
    return sg_pBoxFieldName_avc1[eName];
}
#endif

#if 1 /******************** avcC ********************/
static const HANPSTR sg_pMP4_avcC_Name = TEXT("解码器配置信息");
static const HANPSTR sg_pBoxFieldName_avcC[VIDEO_MP4_avcC_BOX_FIELD_CNT] = {
    [VIDEO_MP4_avcC_BOX_FIELD_CONFIGURATION_VERSION] = TEXT("配置版本"),
    [VIDEO_MP4_avcC_BOX_FIELD_AVC_PROFILE_INDICATION] = TEXT("配置文件标识"),
    [VIDEO_MP4_avcC_BOX_FIELD_PROFILE_COMPATIBILITY] = TEXT("配置文件兼容性"),
    [VIDEO_MP4_avcC_BOX_FIELD_AVC_LEVEL_INDICATION] = TEXT("级别标识"),
    [VIDEO_MP4_avcC_BOX_FIELD_AVC_NALU_LENGTH_SIZE] = TEXT("NALU长度大小"),
    [VIDEO_MP4_avcC_BOX_FIELD_AVC_SPS] = TEXT("SPS条目"),
    [VIDEO_MP4_avcC_BOX_FIELD_AVC_PPS] = TEXT("PPS条目"),
};

HANPSTR GetMP4_avcC_Name(void)
{
    return sg_pMP4_avcC_Name;
}
HANPSTR GetMP4_avcC_FieldName(VIDEOMP4BOXFIELD_avcC eName)
{
    return sg_pBoxFieldName_avcC[eName];
}
#endif
