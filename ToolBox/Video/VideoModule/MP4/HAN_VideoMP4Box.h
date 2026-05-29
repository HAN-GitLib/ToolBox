#ifndef HAN_VIDEO_MP4_BOX_H
#define HAN_VIDEO_MP4_BOX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\..\HAN_VideoDef.h"

typedef enum {
    VIDEO_MP4_PROFILE_TYPE_BASELINE,
    VIDEO_MP4_PROFILE_TYPE_CONSTRAINED_BASELINE,
    VIDEO_MP4_PROFILE_TYPE_MAIN,
    VIDEO_MP4_PROFILE_TYPE_EXTENDED,
    VIDEO_MP4_PROFILE_TYPE_HIGH,
    VIDEO_MP4_PROFILE_TYPE_PROGRESSIVE_HIGH,
    VIDEO_MP4_PROFILE_TYPE_CONSTRAINED_HIGH,
    VIDEO_MP4_PROFILE_TYPE_HIGH_10,
    VIDEO_MP4_PROFILE_TYPE_PROGRESSIVE_HIGH_10,
    VIDEO_MP4_PROFILE_TYPE_HIGH_422,
    VIDEO_MP4_PROFILE_TYPE_HIGH_444_PREDICTIVE,
    VIDEO_MP4_PROFILE_TYPE_HIGH_10_INTRA,
    VIDEO_MP4_PROFILE_TYPE_HIGH_422_INTRA,
    VIDEO_MP4_PROFILE_TYPE_HIGH_444_INTRA,
    VIDEO_MP4_PROFILE_TYPE_CAVLC_444_INTRA,
    VIDEO_MP4_PROFILE_TYPE_CNT,
} VIDEOMP4PROFILETYPE;

typedef enum {
    VIDEO_MP4_BOX_TYPE_ftyp,
    VIDEO_MP4_BOX_TYPE_free,
    VIDEO_MP4_BOX_TYPE_moov,
    VIDEO_MP4_BOX_TYPE_mvhd,
    VIDEO_MP4_BOX_TYPE_trak,
    VIDEO_MP4_BOX_TYPE_tkhd,
    VIDEO_MP4_BOX_TYPE_edts,
    VIDEO_MP4_BOX_TYPE_elst,
    VIDEO_MP4_BOX_TYPE_mdia,
    VIDEO_MP4_BOX_TYPE_mdhd,
    VIDEO_MP4_BOX_TYPE_hdlr,
    VIDEO_MP4_BOX_TYPE_minf,
    VIDEO_MP4_BOX_TYPE_vmhd,
    VIDEO_MP4_BOX_TYPE_dinf,
    VIDEO_MP4_BOX_TYPE_dref,
    VIDEO_MP4_BOX_TYPE_url_,
    VIDEO_MP4_BOX_TYPE_stbl,
    VIDEO_MP4_BOX_TYPE_stsd,
    VIDEO_MP4_BOX_TYPE_avc1,
    VIDEO_MP4_BOX_TYPE_avcC,
    VIDEO_MP4_BOX_TYPE_CNT,
} VIDEOMP4BOXTYPE;

typedef enum {
    VIDEO_MP4_TRACK_HANDLER_TYPE_VIDEO,
    VIDEO_MP4_TRACK_HANDLER_TYPE_SOUND,
    VIDEO_MP4_TRACK_HANDLER_TYPE_SUBTITLE,
    VIDEO_MP4_TRACK_HANDLER_TYPE_HINT,
    VIDEO_MP4_TRACK_HANDLER_TYPE_METADATA,
    VIDEO_MP4_TRACK_HANDLER_TYPE_TEXT,
    VIDEO_MP4_TRACK_HANDLER_TYPE_TIMECODE,
    VIDEO_MP4_TRACK_HANDLER_TYPE_CNT,
} VIDEOMP4TRACKHANDLERTYPE;

typedef enum {
    VIDEO_MP4_VIDEO_GRAPHICS_MODE_COPY,
    VIDEO_MP4_VIDEO_GRAPHICS_MODE_BLEND,
    VIDEO_MP4_VIDEO_GRAPHICS_MODE_TRANSPARENT,
    VIDEO_MP4_VIDEO_GRAPHICS_MODE_DITHER_COPY,
    VIDEO_MP4_VIDEO_GRAPHICS_MODE_ADD,
    VIDEO_MP4_VIDEO_GRAPHICS_MODE_ADD_PIN,
    VIDEO_MP4_VIDEO_GRAPHICS_MODE_SUB,
    VIDEO_MP4_VIDEO_GRAPHICS_MODE_SUB_PIN,
    VIDEO_MP4_VIDEO_GRAPHICS_MODE_AD_MAX,
    VIDEO_MP4_VIDEO_GRAPHICS_MODE_AD_MIN,
    VIDEO_MP4_VIDEO_GRAPHICS_MODE_INVERT,
    VIDEO_MP4_VIDEO_GRAPHICS_MODE_INVERT_ADD,
    VIDEO_MP4_VIDEO_GRAPHICS_MODE_ADD_OVER,
    VIDEO_MP4_VIDEO_GRAPHICS_MODE_HIGHLIGHT,
    VIDEO_MP4_VIDEO_GRAPHICS_MODE_CNT,
} VIDEOMP4VIDEOGRAPHICSMODE;

typedef enum {
    VIDEO_MP4_ftyp_BOX_FIELD_MAJOR_BRAND,
    VIDEO_MP4_ftyp_BOX_FIELD_MINOR_VERSION,
    VIDEO_MP4_ftyp_BOX_FIELD_COMPATIBLE_BRANDS,
    VIDEO_MP4_ftyp_BOX_FIELD_CNT,
} VIDEOMP4BOXFIELD_ftyp;
typedef enum {
    VIDEO_MP4_free_BOX_FIELD_LEN,
    VIDEO_MP4_free_BOX_FIELD_DATA,
    VIDEO_MP4_free_BOX_FIELD_TEXT,
    VIDEO_MP4_free_BOX_FIELD_CNT,
} VIDEOMP4BOXFIELD_free;
typedef enum {
    VIDEO_MP4_mvhd_BOX_FIELD_VERSION,
    VIDEO_MP4_mvhd_BOX_FIELD_FLAGS,
    VIDEO_MP4_mvhd_BOX_FIELD_CREATION_TIME,
    VIDEO_MP4_mvhd_BOX_FIELD_MODIFICATION_TIME,
    VIDEO_MP4_mvhd_BOX_FIELD_DURATION,
    VIDEO_MP4_mvhd_BOX_FIELD_RATE,
    VIDEO_MP4_mvhd_BOX_FIELD_VOLUME,
    VIDEO_MP4_mvhd_BOX_FIELD_MATRIX,
    VIDEO_MP4_mvhd_BOX_FIELD_NEXT_TRACK_ID,
    VIDEO_MP4_mvhd_BOX_FIELD_CNT,
} VIDEOMP4BOXFIELD_mvhd;
typedef enum {
    VIDEO_MP4_tkhd_BOX_FIELD_VERSION,
    VIDEO_MP4_tkhd_BOX_FIELD_FLAGS,
    VIDEO_MP4_tkhd_BOX_FIELD_CREATION_TIME,
    VIDEO_MP4_tkhd_BOX_FIELD_MODIFICATION_TIME,
    VIDEO_MP4_tkhd_BOX_FIELD_TRACK_ID,
    VIDEO_MP4_tkhd_BOX_FIELD_DURATION,
    VIDEO_MP4_tkhd_BOX_FIELD_LAYER,
    VIDEO_MP4_tkhd_BOX_FIELD_ALTERNATE_GROUP,
    VIDEO_MP4_tkhd_BOX_FIELD_VOLUME,
    VIDEO_MP4_tkhd_BOX_FIELD_MATRIX,
    VIDEO_MP4_tkhd_BOX_FIELD_RESOLUTION,
    VIDEO_MP4_tkhd_BOX_FIELD_CNT,
} VIDEOMP4BOXFIELD_tkhd;
typedef enum {
    VIDEO_MP4_mdhd_BOX_FIELD_VERSION,
    VIDEO_MP4_mdhd_BOX_FIELD_FLAGS,
    VIDEO_MP4_mdhd_BOX_FIELD_CREATION_TIME,
    VIDEO_MP4_mdhd_BOX_FIELD_MODIFICATION_TIME,
    VIDEO_MP4_mdhd_BOX_FIELD_DURATION,
    VIDEO_MP4_mdhd_BOX_FIELD_LANGUAGE,
    VIDEO_MP4_mdhd_BOX_FIELD_CNT,
} VIDEOMP4BOXFIELD_mdhd;
typedef enum {
    VIDEO_MP4_hdlr_BOX_FIELD_VERSION,
    VIDEO_MP4_hdlr_BOX_FIELD_FLAGS,
    VIDEO_MP4_hdlr_BOX_FIELD_HANDLER_TYPE,
    VIDEO_MP4_hdlr_BOX_FIELD_NAME,
    VIDEO_MP4_hdlr_BOX_FIELD_CNT,
} VIDEOMP4BOXFIELD_hdlr;
typedef enum {
    VIDEO_MP4_vmhd_BOX_FIELD_VERSION,
    VIDEO_MP4_vmhd_BOX_FIELD_FLAGS,
    VIDEO_MP4_vmhd_BOX_FIELD_GRAPHICS_MODE,
    VIDEO_MP4_vmhd_BOX_FIELD_OP_COLOR,
    VIDEO_MP4_vmhd_BOX_FIELD_CNT,
} VIDEOMP4BOXFIELD_vmhd;
typedef enum {
    VIDEO_MP4_url__BOX_FIELD_VERSION,
    VIDEO_MP4_url__BOX_FIELD_FLAGS,
    VIDEO_MP4_url__BOX_FIELD_LOCATION,
    VIDEO_MP4_url__BOX_FIELD_CNT,
} VIDEOMP4BOXFIELD_url_;
typedef enum {
    VIDEO_MP4_avc1_BOX_FIELD_DATA_REF_INDEX,
    VIDEO_MP4_avc1_BOX_FIELD_VERSION,
    VIDEO_MP4_avc1_BOX_FIELD_REVISION,
    VIDEO_MP4_avc1_BOX_FIELD_VENDOR,
    VIDEO_MP4_avc1_BOX_FIELD_TEMPORAL_QUALITY,
    VIDEO_MP4_avc1_BOX_FIELD_SPATIAL_QUALITY,
    VIDEO_MP4_avc1_BOX_FIELD_WIDTH,
    VIDEO_MP4_avc1_BOX_FIELD_HEIGHT,
    VIDEO_MP4_avc1_BOX_FIELD_HORIZ_RESOLUTION,
    VIDEO_MP4_avc1_BOX_FIELD_VERT_RESOLUTION,
    VIDEO_MP4_avc1_BOX_FIELD_DATA_SIZE,
    VIDEO_MP4_avc1_BOX_FIELD_FRAME_COUNT,
    VIDEO_MP4_avc1_BOX_FIELD_COMPRESSOR_NAME,
    VIDEO_MP4_avc1_BOX_FIELD_DEPTH,
    VIDEO_MP4_avc1_BOX_FIELD_COLOR_TABLE,
    VIDEO_MP4_avc1_BOX_FIELD_CNT,
} VIDEOMP4BOXFIELD_avc1;
typedef enum {
    VIDEO_MP4_avcC_BOX_FIELD_CONFIGURATION_VERSION,
    VIDEO_MP4_avcC_BOX_FIELD_AVC_PROFILE_INDICATION,
    VIDEO_MP4_avcC_BOX_FIELD_PROFILE_COMPATIBILITY,
    VIDEO_MP4_avcC_BOX_FIELD_AVC_LEVEL_INDICATION,
    VIDEO_MP4_avcC_BOX_FIELD_AVC_NALU_LENGTH_SIZE,
    VIDEO_MP4_avcC_BOX_FIELD_AVC_SPS,
    VIDEO_MP4_avcC_BOX_FIELD_AVC_PPS,
    VIDEO_MP4_avcC_BOX_FIELD_CNT,
} VIDEOMP4BOXFIELD_avcC;

typedef struct tagVIDEOMP4BOX {
    uint32_t                        nSize;
    uint8_t                         pType[4];
    const uint8_t*                  pData;
} VIDEOMP4BOX, * PVIDEOMP4BOX;
typedef const VIDEOMP4BOX* PCVIDEOMP4BOX;

typedef struct tagVIDEOMP4BOXINFODATETIME {
    ULARGE_INTEGER                  uTime;
    SYSTEMTIME                      stTime;
} VIDEOMP4BOXINFODATETIME, * PVIDEOMP4BOXINFODATETIME;
typedef struct tagVIDEOMP4BOXINFOTIMEDURATION {
    uint32_t                        nTimescale;
    ULARGE_INTEGER                  nDuration;
    ULARGE_INTEGER                  sTimeDuration;  // 实际时长（秒）
} VIDEOMP4BOXINFOTIMEDURATION, * PVIDEOMP4BOXINFOTIMEDURATION;
typedef struct tagVIDEOMP4BOXINFOINTDOUBLE {
    union {
        int16_t                     s16;
        uint16_t                    u16;
        int32_t                     s32;
        uint32_t                    u32;
    };
    HANDOUBLE                       nPhy;
} VIDEOMP4BOXINFOINTDOUBLE, * PVIDEOMP4BOXINFOINTDOUBLE;

typedef struct tagVIDEOMP4BOXINFO_mvhd {
    uint8_t                         nVersion;
    uint32_t                        cFlags;
    VIDEOMP4BOXINFODATETIME         creationTime;
    VIDEOMP4BOXINFODATETIME         modificationTime;
    VIDEOMP4BOXINFOTIMEDURATION     timeDuration;
    VIDEOMP4BOXINFOINTDOUBLE        nRate;
    VIDEOMP4BOXINFOINTDOUBLE        nVolume;
    uint32_t                        pMatrix[9];
    uint32_t                        nNextTrackId;
} VIDEOMP4BOXINFO_mvhd, * PVIDEOMP4BOXINFO_mvhd;

typedef struct tagVIDEOMP4BOXINFO_tkhd {
    uint8_t                         nVersion;
    uint32_t                        cFlags;
    VIDEOMP4BOXINFODATETIME         creationTime;
    VIDEOMP4BOXINFODATETIME         modificationTime;
    uint32_t                        nTrackId;
    ULARGE_INTEGER                  nDuration;
    uint16_t                        nLayer;
    uint16_t                        nAlternateGroup;
    VIDEOMP4BOXINFOINTDOUBLE        nVolume;
    uint32_t                        pMatrix[9];
    VIDEOMP4BOXINFOINTDOUBLE        nWidth;
    VIDEOMP4BOXINFOINTDOUBLE        nHeight;
} VIDEOMP4BOXINFO_tkhd, * PVIDEOMP4BOXINFO_tkhd;

typedef struct tagVIDEOMP4BOXINFO_elst {
    ULARGE_INTEGER                  nSegmentDuration;
    ULARGE_INTEGER                  nMediaTime;
    VIDEOMP4BOXINFOINTDOUBLE        mediaRate;
} VIDEOMP4BOXINFO_elst, * PVIDEOMP4BOXINFO_elst;

typedef struct tagVIDEOMP4BOXINFO_mdhd {
    uint8_t                         nVersion;
    uint32_t                        cFlags;
    VIDEOMP4BOXINFODATETIME         creationTime;
    VIDEOMP4BOXINFODATETIME         modificationTime;
    VIDEOMP4BOXINFOTIMEDURATION     timeDuration;
    CHAR                            pLanguage[2];
} VIDEOMP4BOXINFO_mdhd, * PVIDEOMP4BOXINFO_mdhd;

typedef struct tagVIDEOMP4BOXINFO_hdlr {
    uint8_t                         nVersion;
    uint32_t                        cFlags;
    struct {
        uint8_t                     pType[4];
        VIDEOMP4TRACKHANDLERTYPE    eType;
    } handlerType;
    const uint8_t*                  pName;
} VIDEOMP4BOXINFO_hdlr, * PVIDEOMP4BOXINFO_hdlr;

typedef struct tagVIDEOMP4BOXINFO_vmhd {
    uint8_t                         nVersion;
    uint32_t                        cFlags;
    uint16_t                        cGraphicsMode;
    uint16_t                        pOpColor[3];
} VIDEOMP4BOXINFO_vmhd, * PVIDEOMP4BOXINFO_vmhd;

typedef struct tagVIDEOMP4BOXINFO_url_ {
    uint8_t                         nVersion;
    uint32_t                        cFlags;
    const uint8_t*                  pUrl;
} VIDEOMP4BOXINFO_url_, * PVIDEOMP4BOXINFO_url_;

typedef struct tagVIDEOMP4BOXINFO_avc1 {
    uint16_t                        nDataRefIndex;
    uint16_t                        nVersion;
    uint16_t                        nRevision;
    uint32_t                        cVendor;
    uint32_t                        nTemporalQuality;
    uint32_t                        nSpatialQuality;
    uint16_t                        nWidth;
    uint16_t                        nHeight;
    VIDEOMP4BOXINFOINTDOUBLE        nHorizResolution;
    VIDEOMP4BOXINFOINTDOUBLE        nVertResolution;
    uint32_t                        nDataSize;
    uint16_t                        nFrameCount;
    uint8_t                         pCompressorName[32];
    uint16_t                        nDepth;
    uint16_t                        nColorTable;
} VIDEOMP4BOXINFO_avc1, * PVIDEOMP4BOXINFO_avc1;

typedef struct tagVIDEOMP4BOXINFO_avcC {
    uint8_t                         nConfigurationVersion;
    uint8_t                         nAvcProfileIndication;
    uint8_t                         nProfileCompatibility;
    uint8_t                         nAvcLevelIndication;
    uint8_t                         nNALULengthSize;
    struct {
        uint8_t                     nNum;
        HANSIZE                     nSize;
        const uint8_t*              pList;
    } sps;
    struct {
        uint8_t                     nNum;
        HANSIZE                     nSize;
        const uint8_t*              pList;
    } pps;
} VIDEOMP4BOXINFO_avcC, * PVIDEOMP4BOXINFO_avcC;

typedef struct tagVIDEOMP4BOXINFO_SPS {
    struct {
        uint8_t                     refIdc;
        uint8_t                     naluType;
    } header;
    VIDEOMP4PROFILETYPE             eProfile;
    uint8_t                         nProfileIdc;
    union {
        uint8_t                     cFlags;
        struct {
            uint8_t                 bSet0 : 1;
            uint8_t                 bSet1 : 1;
            uint8_t                 bSet2 : 1;
            uint8_t                 bSet3 : 1;
            uint8_t                 bSet4 : 1;
            uint8_t                 bSet5 : 1;
            uint8_t                 bReserve : 2;
        } bSets;
    } nConstraintSetFlags;
    uint8_t                         nLevelIdc;
    uint8_t                         nSPSId;
    struct {
        uint8_t                     nChromaFormatIdc;
        BOOL                        bSeparateColourPlane;
        struct { // 亮度参数
            uint8_t                 nBitDepth;
            uint8_t                 nQuantizationParamRange;
        } lumaParam;
        struct { // 色度参数
            uint8_t                 nBitDepth;
            uint8_t                 nQuantizationParamRange;
        } chromaParam;
        BOOL                        bBypassTransform;
        struct {
            BOOL                    bValid;
        } seqScalingMatrix;
    } profileParam;
    HANSIZE                         nMaxFrameNum;
    uint32_t                        nPOCType;
    HANSIZE                         nMaxPicOrderCntLsb;
    uint32_t                        nMaxNumRefFrames;
    uint8_t                         cGapsInFrameNumValueAllowedFlag;
    uint32_t                        nWidth;
    uint32_t                        nHeight;
} VIDEOMP4BOXINFO_SPS, * PVIDEOMP4BOXINFO_SPS;

HANPSTR GetMP4_ftyp_Name(void);
HANPSTR GetMP4_ftyp_FieldName(VIDEOMP4BOXFIELD_ftyp eName);

HANPSTR GetMP4_free_Name(void);
HANPSTR GetMP4_free_FieldName(VIDEOMP4BOXFIELD_free eName);

HANPSTR GetMP4_moov_Name(void);

HANPSTR GetMP4_mvhd_Name(void);
HANPSTR GetMP4_mvhd_FieldName(VIDEOMP4BOXFIELD_mvhd eName);
HANPSTR GetMP4_mvhd_VersionName(uint8_t nVersion);

HANPSTR GetMP4_trak_Name(void);

HANPSTR GetMP4_tkhd_Name(void);
HANPSTR GetMP4_tkhd_FieldName(VIDEOMP4BOXFIELD_tkhd eName);
HANPSTR GetMP4_tkhd_VersionName(uint8_t nVersion);
void GetMP4_tkhd_FlagsName(uint32_t cFlags, HANPSTR pText, HANSIZE nLen);

HANPSTR GetMP4_edts_Name(void);

HANPSTR GetMP4_elst_Name(void);

HANPSTR GetMP4_mdia_Name(void);

HANPSTR GetMP4_mdhd_Name(void);
HANPSTR GetMP4_mdhd_FieldName(VIDEOMP4BOXFIELD_mdhd eName);
HANPSTR GetMP4_mdhd_VersionName(uint8_t nVersion);

HANPSTR GetMP4_hdlr_Name(void);
HANPSTR GetMP4_hdlr_FieldName(VIDEOMP4BOXFIELD_hdlr eName);
HANPSTR GetMP4_hdlr_HandlerTypeName(VIDEOMP4TRACKHANDLERTYPE eType);

HANPSTR GetMP4_minf_Name(void);

HANPSTR GetMP4_vmhd_Name(void);
HANPSTR GetMP4_vmhd_FieldName(VIDEOMP4BOXFIELD_vmhd eName);
HANPSTR GetMP4_vmhd_GraphicsModeName(uint16_t cMode);

HANPSTR GetMP4_dinf_Name(void);

HANPSTR GetMP4_dref_Name(void);

HANPSTR GetMP4_url__Name(void);
HANPSTR GetMP4_url__FieldName(VIDEOMP4BOXFIELD_url_ eName);
HANPSTR GetMP4_url__FlagsName(uint32_t cFlags);

HANPSTR GetMP4_stbl_Name(void);

HANPSTR GetMP4_stsd_Name(void);

HANPSTR GetMP4_avc1_Name(void);
HANPSTR GetMP4_avc1_FieldName(VIDEOMP4BOXFIELD_avc1 eName);

HANPSTR GetMP4_avcC_Name(void);
HANPSTR GetMP4_avcC_FieldName(VIDEOMP4BOXFIELD_avcC eName);

#ifdef __cplusplus
}
#endif

#endif
