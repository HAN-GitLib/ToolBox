#ifndef HAN_VIDEO_H264_READ_BIT_H
#define HAN_VIDEO_H264_READ_BIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\..\HAN_VideoDef.h"

typedef enum {
    VIDEO_H264_READ_BIT_RET_OK,
    VIDEO_H264_READ_BIT_RET_ERROR,
} VIDEOH264READBITRET;

typedef union {
    uint32_t    u32;
    int32_t     s32;
} VIDEOH264VALUE;

typedef struct tagVIDEOMP4READBIT {
    const uint8_t*                  pData;
    HANSIZE                         nDataLen;
    HANCPRIVATE uint32_t            cData;
    HANCPRIVATE HANSIZE             iByte;
    HANCPRIVATE uint8_t             iBit;
    VIDEOH264VALUE                  cValue;
    VIDEOH264READBITRET             retError;
} VIDEOMP4READBIT, * PVIDEOH264READBIT;

VIDEOMP4READBIT H264InitReadBit(const uint8_t* pData, HANSIZE nLen);

uint8_t H264ReadBit(PVIDEOH264READBIT pReadBit);

uint32_t H264ReadBits(PVIDEOH264READBIT pReadBit, uint8_t nLen);

#ifdef __cplusplus
}
#endif

#endif
