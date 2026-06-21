#ifndef HAN_VIDEO_MP4_BOX_H
#define HAN_VIDEO_MP4_BOX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\..\HAN_VideoDef.h"
#include "..\H264\HAN_VideoH264Def.h"

typedef enum {
    VIDEO_MP4_BOX_TYPE_ftyp,
    VIDEO_MP4_BOX_TYPE_free,
    VIDEO_MP4_BOX_TYPE_mdat,
    VIDEO_MP4_BOX_TYPE_moov,
    VIDEO_MP4_BOX_TYPE_mvhd,
    VIDEO_MP4_BOX_TYPE_iods,
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
    VIDEO_MP4_BOX_TYPE_pasp,
    VIDEO_MP4_BOX_TYPE_btrt,
    VIDEO_MP4_BOX_TYPE_stts,
    VIDEO_MP4_BOX_TYPE_ctts,
    VIDEO_MP4_BOX_TYPE_stss,
    VIDEO_MP4_BOX_TYPE_stsc,
    VIDEO_MP4_BOX_TYPE_stsz,
    VIDEO_MP4_BOX_TYPE_stco,
    VIDEO_MP4_BOX_TYPE_co64,
    VIDEO_MP4_BOX_TYPE_smhd,
    VIDEO_MP4_BOX_TYPE_mp4a,
    VIDEO_MP4_BOX_TYPE_esds,
    VIDEO_MP4_BOX_TYPE_udta,
    VIDEO_MP4_BOX_TYPE_meta,
    VIDEO_MP4_BOX_TYPE_ilst,
    VIDEO_MP4_BOX_TYPE_data,
    VIDEO_MP4_BOX_TYPE_desc,
    VIDEO_MP4_BOX_TYPE_Copyright_too,
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
    VIDEO_MP4_VIDEO_FORMAT_COMPONENT,
    VIDEO_MP4_VIDEO_FORMAT_PAL,
    VIDEO_MP4_VIDEO_FORMAT_NTSC,
    VIDEO_MP4_VIDEO_FORMAT_SECAM,
    VIDEO_MP4_VIDEO_FORMAT_MAC,
    VIDEO_MP4_VIDEO_FORMAT_UNSPECIFIED,
    VIDEO_MP4_VIDEO_FORMAT_CNT,
} VIDEOMP4VIDEOFORMAT;

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
    VIDEO_MP4_mdat_BOX_FIELD_TRACK_ID,
    VIDEO_MP4_mdat_BOX_FIELD_SAMPLE_ID,
    VIDEO_MP4_mdat_BOX_FIELD_OFFSET,
    VIDEO_MP4_mdat_BOX_FIELD_SAMPLE_SIZE,
    VIDEO_MP4_mdat_BOX_FIELD_DTS,
    VIDEO_MP4_mdat_BOX_FIELD_PTS,
    VIDEO_MP4_mdat_BOX_FIELD_DURATION,
    VIDEO_MP4_mdat_BOX_FIELD_CNT,
} VIDEOMP4BOXFIELD_mdat;
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
    VIDEO_MP4_iods_BOX_FIELD_VERSION,
    VIDEO_MP4_iods_BOX_FIELD_FLAGS,
    VIDEO_MP4_iods_BOX_FIELD_DATA,
    VIDEO_MP4_iods_BOX_FIELD_CNT,
} VIDEOMP4BOXFIELD_iods;
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
typedef enum {
    VIDEO_MP4_pasp_BOX_FIELD_PIXEL_ASPECT_RATIO,
    VIDEO_MP4_pasp_BOX_FIELD_CNT,
} VIDEOMP4BOXFIELD_pasp;
typedef enum {
    VIDEO_MP4_btrt_BOX_FIELD_BUFFER_SIZE,
    VIDEO_MP4_btrt_BOX_FIELD_MAX_BIT_RATE,
    VIDEO_MP4_btrt_BOX_FIELD_AVG_BIT_RATE,
    VIDEO_MP4_btrt_BOX_FIELD_CNT,
} VIDEOMP4BOXFIELD_btrt;
typedef enum {
    VIDEO_MP4_stts_BOX_FIELD_VERSION,
    VIDEO_MP4_stts_BOX_FIELD_FLAGS,
    VIDEO_MP4_stts_BOX_FIELD_ENTRY,
    VIDEO_MP4_stts_BOX_FIELD_SAMPLE_COUNT,
    VIDEO_MP4_stts_BOX_FIELD_SAMPLE_DELTA,
    VIDEO_MP4_stts_BOX_FIELD_CNT,
} VIDEOMP4BOXFIELD_stts;
typedef enum {
    VIDEO_MP4_ctts_BOX_FIELD_VERSION,
    VIDEO_MP4_ctts_BOX_FIELD_FLAGS,
    VIDEO_MP4_ctts_BOX_FIELD_ENTRY,
    VIDEO_MP4_ctts_BOX_FIELD_SAMPLE_COUNT,
    VIDEO_MP4_ctts_BOX_FIELD_SAMPLE_OFFSET,
    VIDEO_MP4_ctts_BOX_FIELD_CNT,
} VIDEOMP4BOXFIELD_ctts;
typedef enum {
    VIDEO_MP4_stss_BOX_FIELD_VERSION,
    VIDEO_MP4_stss_BOX_FIELD_FLAGS,
    VIDEO_MP4_stss_BOX_FIELD_ENTRY,
    VIDEO_MP4_stss_BOX_FIELD_CNT,
} VIDEOMP4BOXFIELD_stss;
typedef enum {
    VIDEO_MP4_stsc_BOX_FIELD_VERSION,
    VIDEO_MP4_stsc_BOX_FIELD_FLAGS,
    VIDEO_MP4_stsc_BOX_FIELD_ENTRY,
    VIDEO_MP4_stsc_BOX_FIELD_FIRST_CHUNK,
    VIDEO_MP4_stsc_BOX_FIELD_SAMPLE_PER_CHUNK,
    VIDEO_MP4_stsc_BOX_FIELD_SAMPLE_DESCRIPTION_ID,
    VIDEO_MP4_stsc_BOX_FIELD_CNT,
} VIDEOMP4BOXFIELD_stsc;
typedef enum {
    VIDEO_MP4_stsz_BOX_FIELD_VERSION,
    VIDEO_MP4_stsz_BOX_FIELD_FLAGS,
    VIDEO_MP4_stsz_BOX_FIELD_SAMPLE_SIZE,
    VIDEO_MP4_stsz_BOX_FIELD_SIZE,
    VIDEO_MP4_stsz_BOX_FIELD_CNT,
} VIDEOMP4BOXFIELD_stsz;
typedef enum {
    VIDEO_MP4_stco_BOX_FIELD_VERSION,
    VIDEO_MP4_stco_BOX_FIELD_FLAGS,
    VIDEO_MP4_stco_BOX_FIELD_ENTRY,
    VIDEO_MP4_stco_BOX_FIELD_CNT,
} VIDEOMP4BOXFIELD_stco;
typedef enum {
    VIDEO_MP4_smhd_BOX_FIELD_VERSION,
    VIDEO_MP4_smhd_BOX_FIELD_FLAGS,
    VIDEO_MP4_smhd_BOX_FIELD_BALANCE,
    VIDEO_MP4_smhd_BOX_FIELD_CNT,
} VIDEOMP4BOXFIELD_smhd;
typedef enum {
    VIDEO_MP4_mp4a_BOX_FIELD_ID,
    VIDEO_MP4_mp4a_BOX_FIELD_VERSION,
    VIDEO_MP4_mp4a_BOX_FIELD_REVISION,
    VIDEO_MP4_mp4a_BOX_FIELD_VENDOR,
    VIDEO_MP4_mp4a_BOX_FIELD_CHANNEL_COUNT,
    VIDEO_MP4_mp4a_BOX_FIELD_SAMPLE_SIZE,
    VIDEO_MP4_mp4a_BOX_FIELD_COMPRESSION_ID,
    VIDEO_MP4_mp4a_BOX_FIELD_PACKET_SIZE,
    VIDEO_MP4_mp4a_BOX_FIELD_SAMPLE_RATE,
    VIDEO_MP4_mp4a_BOX_FIELD_CNT,
} VIDEOMP4BOXFIELD_mp4a;
typedef enum {
    VIDEO_MP4_data_BOX_DATA_TYPE,
    VIDEO_MP4_data_BOX_LOCALE,
    VIDEO_MP4_data_BOX_DATA,
    VIDEO_MP4_data_BOX_FIELD_CNT,
} VIDEOMP4BOXFIELD_data;

typedef struct tagVIDEOMP4BOX {
    HANSIZE                         nSize;
    HANSIZE                         nDataLen;
    uint8_t                         pType[4];
    VIDEOMP4BOXTYPE                 eType;
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
typedef struct tagVIDEOMP4BOXINFOFULLBOXVERFLAGS {
    uint8_t                         nVersion;
    uint32_t                        cFlags;
} VIDEOMP4BOXINFOFULLBOXVERFLAGS, * PVIDEOMP4BOXINFOFULLBOXVERFLAGS;

typedef struct tagVIDEOMP4BOXINFO_mvhd {
    const uint8_t*                  pRawData;
    VIDEOMP4BOXINFOFULLBOXVERFLAGS  fbVF;
    VIDEOMP4BOXINFODATETIME         creationTime;
    VIDEOMP4BOXINFODATETIME         modificationTime;
    VIDEOMP4BOXINFOTIMEDURATION     timeDuration;
    VIDEOMP4BOXINFOINTDOUBLE        nRate;
    VIDEOMP4BOXINFOINTDOUBLE        nVolume;
    uint32_t                        pMatrix[9];
    uint32_t                        nNextTrackId;
} VIDEOMP4BOXINFO_mvhd, * PVIDEOMP4BOXINFO_mvhd;

typedef struct tagVIDEOMP4BOXINFO_tkhd {
    const uint8_t*                  pRawData;
    VIDEOMP4BOXINFOFULLBOXVERFLAGS  fbVF;
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
    const uint8_t*                  pRawData;
    ULARGE_INTEGER                  nSegmentDuration;
    ULARGE_INTEGER                  nMediaTime;
    VIDEOMP4BOXINFOINTDOUBLE        mediaRate;
} VIDEOMP4BOXINFO_elst, * PVIDEOMP4BOXINFO_elst;

typedef struct tagVIDEOMP4BOXINFO_mdhd {
    const uint8_t*                  pRawData;
    VIDEOMP4BOXINFOFULLBOXVERFLAGS  fbVF;
    VIDEOMP4BOXINFODATETIME         creationTime;
    VIDEOMP4BOXINFODATETIME         modificationTime;
    VIDEOMP4BOXINFOTIMEDURATION     timeDuration;
    CHAR                            pLanguage[2];
} VIDEOMP4BOXINFO_mdhd, * PVIDEOMP4BOXINFO_mdhd;

typedef struct tagVIDEOMP4BOXINFO_hdlr {
    const uint8_t*                  pRawData;
    VIDEOMP4BOXINFOFULLBOXVERFLAGS  fbVF;
    struct {
        uint8_t                     pType[5];
        VIDEOMP4TRACKHANDLERTYPE    eType;
    } handlerType;
    const uint8_t*                  pName;
} VIDEOMP4BOXINFO_hdlr, * PVIDEOMP4BOXINFO_hdlr;

typedef struct tagVIDEOMP4BOXINFO_vmhd {
    const uint8_t*                  pRawData;
    VIDEOMP4BOXINFOFULLBOXVERFLAGS  fbVF;
    uint16_t                        cGraphicsMode;
    uint16_t                        pOpColor[3];
} VIDEOMP4BOXINFO_vmhd, * PVIDEOMP4BOXINFO_vmhd;

typedef struct tagVIDEOMP4BOXINFO_url_ {
    const uint8_t*                  pRawData;
    VIDEOMP4BOXINFOFULLBOXVERFLAGS  fbVF;
    const uint8_t*                  pUrl;
} VIDEOMP4BOXINFO_url_, * PVIDEOMP4BOXINFO_url_;

typedef struct tagVIDEOMP4BOXINFO_avc1 {
    const uint8_t*                  pRawData;
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
    const uint8_t*                  pRawData;
    uint8_t                         nConfigurationVersion;
    uint8_t                         nAvcProfileIndication;
    uint8_t                         nProfileCompatibility;
    uint8_t                         nAvcLevelIndication;
    uint8_t                         nNALULengthSize;
    struct {
        uint8_t                                     nNum;
        PVIDEOH264PARAMETER_seq_parameter_set       pSPS;
        HANSIZE                                     nSize;
        const uint8_t*                              pList;
    } sps;
    struct {
        uint8_t                                     nNum;
        PVIDEOH264PARAMETER_pic_parameter_set       pPPS;
        HANSIZE                                     nSize;
        const uint8_t*                              pList;
    } pps;
} VIDEOMP4BOXINFO_avcC, * PVIDEOMP4BOXINFO_avcC;

typedef struct tagVIDEOMP4BOXINFO_stsdSubBoxDefault {
    const uint8_t*                  pRawData;
    VIDEOMP4BOXINFOFULLBOXVERFLAGS  fbVF;
    uint32_t                        nCnt;
    const uint8_t*                  pEntry;
    HANSIZE                         nEntrySize;
} VIDEOMP4BOXINFO_stsdSubBoxDefault, * PVIDEOMP4BOXINFO_stsdSubBoxDefault;

typedef struct tagVIDEOMP4BOXINFO_stsz {
    const uint8_t*                  pRawData;
    VIDEOMP4BOXINFOFULLBOXVERFLAGS  fbVF;
    uint32_t                        nSampleSize;
    uint32_t                        nSampleCnt;
    const uint8_t*                  pEntry;
    HANSIZE                         nEntrySize;
} VIDEOMP4BOXINFO_stsz, * PVIDEOMP4BOXINFO_stsz;

typedef struct tagVIDEOMP4BOXINFO_stco64 {
    const uint8_t*                  pRawData;
    uint8_t                         nDataSize;
    VIDEOMP4BOXINFOFULLBOXVERFLAGS  fbVF;
    uint32_t                        nChunkCnt;
    const uint8_t*                  pEntry;
    HANSIZE                         nEntrySize;
} VIDEOMP4BOXINFO_stco64, * PVIDEOMP4BOXINFO_stco64;

typedef struct tagVIDEOMP4BOXINFO_mp4a {
    const uint8_t*                  pRawData;
    uint16_t                        nId;
    uint16_t                        nVersion;
    uint16_t                        nRevision;
    uint32_t                        nVendor;
    uint16_t                        nChannelCnt;
    uint16_t                        nSampleSize;
    uint16_t                        nCompressionId;
    uint16_t                        nPacketSize;
    HANDOUBLE                       nSampleRate;
} VIDEOMP4BOXINFO_mp4a, * PVIDEOMP4BOXINFO_mp4a;

typedef struct tagVIDEOMP4DECODEINFOSAMPLE {
    HANSIZE                         sOffset;        // 帧在文件中的偏移位置
    HANSIZE                         timeDTS;        // DTS 的帧起始时间
    HANSIZE                         timePTS;        // PTS 的帧起始时间
    HANSIZE                         timeDuration;   // 持续时间
    HANSIZE                         idKeyFrame;     // 所属关键帧的 ID，不用指针指向关键帧因为考虑到以后针对大文件做部分文件加载的话，指针没有意义，而 ID 或偏移量可以快速知道加载哪部分文件
    uint32_t                        nSize;          // 帧的大小
    uint32_t                        idChunkGroup;
    uint32_t                        idChunk;
    HANSIZE                         idDescription;
} VIDEOMP4DECODEINFOSAMPLE, * PVIDEOMP4DECODEINFOSAMPLE;

typedef struct tagVIDEOMP4TRACK {
    VIDEOMP4BOXINFO_tkhd                tkhd;
    VIDEOMP4BOXINFO_hdlr                hdlr;
    VIDEOMP4BOXINFO_avc1                avc1;
    VIDEOMP4BOXINFO_avcC                avcC;
    VIDEOMP4BOXINFO_stsdSubBoxDefault   stts;
    VIDEOMP4BOXINFO_stsdSubBoxDefault   ctts;
    VIDEOMP4BOXINFO_stsdSubBoxDefault   stss;
    VIDEOMP4BOXINFO_stsdSubBoxDefault   stsc;
    VIDEOMP4BOXINFO_stsz                stsz;
    VIDEOMP4BOXINFO_stco64              stco64;
    HANSIZE                             nSampleCnt;
    PVIDEOMP4DECODEINFOSAMPLE           pSample;
} VIDEOMP4TRACK, * PVIDEOMP4TRACK;

typedef struct tagVIDEOMP4BOXINFO {
    struct {
        HANSIZE                         nCnt;
        HANSIZE                         nTargetId;
        PVIDEOMP4TRACK                  pList;
    } track;
} VIDEOMP4BOXINFO, * PVIDEOMP4BOXINFO;
typedef const VIDEOMP4BOXINFO* PCVIDEOMP4BOXINFO;

HANPSTR GetMP4BoxVersionName(uint8_t nVersion);

HANPSTR GetMP4_ftyp_Name(void);
HANPSTR GetMP4_ftyp_FieldName(VIDEOMP4BOXFIELD_ftyp eName);

HANPSTR GetMP4_free_Name(void);
HANPSTR GetMP4_free_FieldName(VIDEOMP4BOXFIELD_free eName);

HANPSTR GetMP4_mdat_Name(void);
HANPSTR GetMP4_mdat_FieldName(VIDEOMP4BOXFIELD_mdat eName);

HANPSTR GetMP4_moov_Name(void);

HANPSTR GetMP4_mvhd_Name(void);
HANPSTR GetMP4_mvhd_FieldName(VIDEOMP4BOXFIELD_mvhd eName);

HANPSTR GetMP4_iods_Name(void);
HANPSTR GetMP4_iods_FieldName(VIDEOMP4BOXFIELD_iods eName);

HANPSTR GetMP4_trak_Name(void);

HANPSTR GetMP4_tkhd_Name(void);
HANPSTR GetMP4_tkhd_FieldName(VIDEOMP4BOXFIELD_tkhd eName);
void GetMP4_tkhd_FlagsName(uint32_t cFlags, HANPSTR pText, HANSIZE nLen);

HANPSTR GetMP4_edts_Name(void);

HANPSTR GetMP4_elst_Name(void);

HANPSTR GetMP4_mdia_Name(void);

HANPSTR GetMP4_mdhd_Name(void);
HANPSTR GetMP4_mdhd_FieldName(VIDEOMP4BOXFIELD_mdhd eName);

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
HANPSTR GetMP4_avcC_ProfileIndicationName(uint8_t nProfileIndication);

HANPSTR GetMP4_pasp_Name(void);
HANPSTR GetMP4_pasp_FieldName(VIDEOMP4BOXFIELD_pasp eName);

HANPSTR GetMP4_btrt_Name(void);
HANPSTR GetMP4_btrt_FieldName(VIDEOMP4BOXFIELD_btrt eName);

HANPSTR GetMP4_stts_Name(void);
HANPSTR GetMP4_stts_FieldName(VIDEOMP4BOXFIELD_stts eName);

HANPSTR GetMP4_ctts_Name(void);
HANPSTR GetMP4_ctts_FieldName(VIDEOMP4BOXFIELD_ctts eName);

HANPSTR GetMP4_stss_Name(void);
HANPSTR GetMP4_stss_FieldName(VIDEOMP4BOXFIELD_stss eName);

HANPSTR GetMP4_stsc_Name(void);
HANPSTR GetMP4_stsc_FieldName(VIDEOMP4BOXFIELD_stsc eName);

HANPSTR GetMP4_stsz_Name(void);
HANPSTR GetMP4_stsz_FieldName(VIDEOMP4BOXFIELD_stsz eName);

HANPSTR GetMP4_stco_Name(void);
HANPSTR GetMP4_stco_FieldName(VIDEOMP4BOXFIELD_stco eName);

HANPSTR GetMP4_smhd_Name(void);
HANPSTR GetMP4_smhd_FieldName(VIDEOMP4BOXFIELD_smhd eName);

HANPSTR GetMP4_mp4a_Name(void);
HANPSTR GetMP4_mp4a_FieldName(VIDEOMP4BOXFIELD_mp4a eName);
HANPSTR GetMP4_mp4a_ChannelCountName(uint16_t nCnt);

HANPSTR GetMP4_esds_Name(void);

HANPSTR GetMP4_udta_Name(void);

HANPSTR GetMP4_meta_Name(void);

HANPSTR GetMP4_ilst_Name(void);

HANPSTR GetMP4_data_Name(void);
HANPSTR GetMP4_data_FieldName(VIDEOMP4BOXFIELD_data eName);
HANPSTR GetMP4_data_DataTypeName(uint32_t cType);

HANPSTR GetMP4_desc_Name(void);

HANPSTR GetMP4_Copyright_too_Name(void);

#ifdef __cplusplus
}
#endif

#endif
