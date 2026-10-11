#ifndef HAN_VIDEO_H264_CABAC_H
#define HAN_VIDEO_H264_CABAC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "HAN_VideoH264Data.h"

/* 初始化处理程序，包括语法元素的读取方法和 residual_block 的处理过程 */
void H264InitCABACHandler(PVIDEOH264DECODER pDecoder);

/* 初始化上下文，仅在开始读取 slice_data 时需要执行，参考标准的 9.3.1.1 章节 */
bool H264CABACInitContext(PCVIDEOH264SLICE pSlice, PVIDEOH264DECODEINFO pDecodeInfo);

/* 初始化引擎，参考标准的 9.3.1.2 章节，以下情况需要调用该函数：
 * 1. 开始读取 slice_data 时
 * 2. 开始处理任何 I_PCM 时
 * 3. 处理完一个 I_PCM 里的所有 pcm_sample_luma 和 pcm_sample_chroma 后
 */
void H264CABACInitEngine(PVIDEOH264DECODEINFO pDecodeInfo);

#ifdef __cplusplus
}
#endif

#endif
