#ifndef HAN_VIDEO_MP4_DECODER_DEF_H
#define HAN_VIDEO_MP4_DECODER_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\HAN_VideoMP4Def.h"
#include "..\HAN_VideoMP4Box.h"

typedef enum {
    /* 视频 */
    VIDEO_MP4_DECODER_TYPE_H264,
    /* 音频 */
    VIDEO_MP4_DECODER_TYPE_MPEG4AUDIO,
    /* End */
    VIDEO_MP4_DECODER_TYPE_CNT,
} VIDEOMP4DECODERTYPE, * PVIDEOMP4DECODERTYPE;

typedef struct tagVIDEOMP4DECODERMEDIAINFO {
    VIDEORESOLUTION pxResolution;
} VIDEOMP4DECODERMEDIAINFO, * PVIDEOMP4DECODERMEDIAINFO;

typedef struct tagVIDEOMP4DECODERINFO { // 提供给解码器的信息
    BOOL                        (*GetVideoDecoderType)(const uint8_t* pData, HANSIZE nLen);                                         // 确定解码器类型，如果是所属的解码器，返回 TRUE
    HANSIZE                     (*GetVideoDecoderSize)(const uint8_t* pData, HANSIZE nLen, PVIDEOMP4DECODERMEDIAINFO pMediaInfo);   // 获取解码器所需的内存大小
    HANSIZE                     (*InitVideoDecoder)(const uint8_t* pData, HANSIZE nLen, uint8_t* pDecoder, HANSIZE nSize);          // 初始化解码器，返回实际占用的内存大小
    HANPSTR                     (*GetBoxName)(VIDEOMP4BOXTYPE boxType);
    HANSIZE                     (*GetSubBoxOffset)(VIDEOMP4BOXTYPE boxType);
    UPDATEBOXINFOWINDOWCALLBACK (*GetUpdateBoxInfoWindowCallback)(VIDEOMP4BOXTYPE boxType);
    void                        (*UpdateBoxInfoWindowSample)(const uint8_t* pData, HANSIZE nLen, HWND hWindow, uint8_t* pDecoder);
    void                        (*DecodeMedia)(const uint8_t* pData, HANSIZE nLen, uint8_t* pDecoder);
} VIDEOMP4DECODERHANDLER;

typedef struct tagVIDEOMP4DECODER { // 提供给 MP4 调度器的信息
    VIDEOMP4DECODERMEDIAINFO    mediaInfo;
    struct {
        uint8_t*                pData;
        HANSIZE                 nSize;
    } buf;
    struct {
        PVIDEOMP4DECODERTYPE    pType;
        uint8_t**               pTable;
        HANSIZE                 nCnt;
    } table;
} VIDEOMP4DECODER, * PVIDEOMP4DECODER;

#ifdef __cplusplus
}
#endif

#endif
