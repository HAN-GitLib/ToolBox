#ifndef HAN_VIDEO_H264_STD_H
#define HAN_VIDEO_H264_STD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

uint32_t H264Min(uint32_t value1, uint32_t value2);
int32_t H264IMax(int32_t value1, int32_t value2);

int32_t H264AbsInt(int32_t n);

int32_t H264Clip3(int32_t x, int32_t y, int32_t z);

uint32_t H264CeilLog2(uint32_t n);

#ifdef __cplusplus
}
#endif

#endif
