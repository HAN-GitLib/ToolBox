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

typedef struct tagVIDEOMP4DECODERINFO { /* 解码器必须实现以下函数，并注册到文件 HAN_VideoMP4Decoder.c 内的全局变量 sg_pDecoderInfo 内 */
    /* 确定解码器类型，如果是所属的解码器，返回 TRUE
     * pData        解码器 BOX（stsd 的所有子 BOX）
     * nLen         pData 的长度
     */
    BOOL                        (*GetVideoDecoderType)(const uint8_t* pData, HANSIZE nLen);
    /* 获取解码器所需的内存大小
     * pData        解码器 BOX（stsd 的所有子 BOX）
     * nLen         pData 的长度
     * pMediaInfo   媒体信息，包含视频分辨率、音频等所有信息
     */
    HANSIZE                     (*GetVideoDecoderSize)(const uint8_t* pData, HANSIZE nLen, PVIDEOMP4DECODERMEDIAINFO pMediaInfo);
    /* 初始化解码器，返回实际占用的内存大小
     * pData        解码器 BOX（stsd 的所有子 BOX）
     * nLen         pData 的长度
     * pDecoder     存放解码器内容的指针，会在解码文件时通过参数传递
     * nSize        pDecoder 的大小，该值一定等于 GetVideoDecoderSize 的返回值
     */
    HANSIZE                     (*InitVideoDecoder)(const uint8_t* pData, HANSIZE nLen, uint8_t* pDecoder, HANSIZE nSize);
    /* 返回 BOX 的名称字符串，打印用 */
    HANPSTR                     (*GetBoxName)(VIDEOMP4BOXTYPE boxType);
    /* 返回 boxType 标识的 BOX 内部的子 BOX 的偏移量，用于 MP4 调度器读取子 BOX 用。如果没有子 BOX，返回 HAN_VIDEO_MP4_INVALID_SIZE
     * 偏移量从数据段第一个字节开始算，如下：
     * [4字节Size][4字节BOX名][数据段]
     * 如果整个数据段都是子 BOX（也就是从第一个字节开始就是子 BOX），返回 0
     */
    HANSIZE                     (*GetSubBoxOffset)(VIDEOMP4BOXTYPE boxType);
    /* 返回一个函数指针，该函数实现打印 BOX 的方法 */
    UPDATEBOXINFOWINDOWCALLBACK (*GetUpdateBoxInfoWindowCallback)(VIDEOMP4BOXTYPE boxType);
    /* 打印一帧图片的信息
     * pData        图片数据
     * nLen         pData 的长度
     * hWindow      要打印的窗口，是个 ListView
     * pDecoder     InitVideoDecoder 所填充的内容
     */
    void                        (*UpdateBoxInfoWindowDecoderSample)(const uint8_t* pData, HANSIZE nLen, HWND hWindow, uint8_t* pDecoder);
    /* 解码图片
     * pData        图片数据
     * nLen         pData 的长度
     * pDecoder     InitVideoDecoder 所填充的内容
     */
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
