/* 很多计算相邻块索引的函数用到的参数，如：luma8x8BlkIdx, luma4x4BlkIdx, chroma4x4BlkIdx 等索引变量
 * 这些变量是在解码残差块的过程中，读取语法元素时的当前正在解码的 8x8 或 4x4 块的索引，它们有 2 种计算方法
 * 1. 根据块在宏块中的位置计算块索引，这种方法在遇到多 case 的情况（比如不同采样率的 chroma4x4BlkIdx，它的计算和取值范围都有不同）下比较复杂
 * 2. 块索引也可以是解码残差的过程中在遍历 8x8 和 4x4 块时的已循环次数
 * 由于循环过程就是按照块的位置顺序的，所以这两种计算方法等价
 * 以亮度举例，在解码亮度时，会遍历 4 个 8x8 块，每次遍历还会嵌套遍历 4 次 4x4 块，并在 4x4 块的循环体内调用 residual_block 函数
 * 当 i8x8 = 2, i4x4 = 1 时，luma8x8BlkIdx 一定等于 2，并且前 2 次 8x8 块的循环已经各执行了 4 次 4x4 块的循环
 * 所以此时 luma4x4BlkIdx 一定等于 9，这个结果也符合位置索引，如下图：
 * ┌───┬───┬───┬───┐
 * │ 0 │ 1 │ 4 │ 5 │
 * │───┼───┼───┼───│
 * │ 2 │ 3 │ 6 │ 7 │
 * ├───┼───┼───┼───┤
 * │ 8 │(9)│ 12│ 13│
 * │───┼───┼───┼───┤
 * │ 10│ 11│ 14│ 15│
 * └───┴───┴───┴───┘
 * 在遍历 8x8 和 4x4 块时，会先遍历左上角 4 个块（由索引分别为 0, 1, 2, 3 的 4x4 块构成的 8x8 块），再遍历右上角 4 个块（由索引分别为 4, 5, 6, 7 的 4x4 块构成的 8x8 块）
 * 所以块的索引变量 luma8x8BlkIdx, luma4x4BlkIdx, chroma4x4BlkIdx 等，本质上就是循环次数的累加器
 */

#ifndef HAN_VIDEO_H264_ADDR_H
#define HAN_VIDEO_H264_ADDR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "HAN_VideoH264Data.h"

#define VIDEO_H264_MACRO_BLOCK_ADDR_INVALID     (HANSIZE)(-1)
#define VIDEO_H264_BLOCK_IDX_INVALID            (uint8_t)(-1)
#define VIDEO_H264_PARTITION_IDX_INVALID        (uint8_t)(-1)

#define VIDEO_H264_LOCATION_X                   0
#define VIDEO_H264_LOCATION_Y                   1

#define VIDEO_H264_NEIGHBOUR_A                  0
#define VIDEO_H264_NEIGHBOUR_B                  1
#define VIDEO_H264_NEIGHBOUR_LEFT               VIDEO_H264_NEIGHBOUR_A
#define VIDEO_H264_NEIGHBOUR_ABOVE              VIDEO_H264_NEIGHBOUR_B

/* 参考标准的 6.4.9 章节 */
HANSIZE H264GetprevMbAddr(PVIDEOH264DECODEINFO pDecodeInfo);

/* 参考标准的 6.4.11.1 章节 */
void DerivationProcessForNeighbouringMacroblocks(PVIDEOH264DECODEINFO pDecodeInfo, HANSIZE mbAddrN[2]);

/* 参考标准的 6.4.11.2 章节 */
void DerivationProcessForNeighbouring8x8LumaBlock(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t luma8x8BlkIdx, HANSIZE mbAddrN[2], uint8_t luma8x8BlkIdxN[2]);

/* 参考标准的 6.4.11.3 章节 */
#define DerivationProcessForNeighbouring8x8ChromaBlockForChromaArrayType3 DerivationProcessForNeighbouring8x8LumaBlock // 参考标准对 4:4:4 的色度 8x8 块的说明，它等价于亮度 8x8 块

/* 参考标准的 6.4.11.4 章节 */
void DerivationProcessForNeighbouring4x4LumaBlock(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t luma4x4BlkIdx, HANSIZE mbAddrN[2], uint8_t luma4x4BlkIdxN[2]);

/* 参考标准的 6.4.11.5 章节 */
void DerivationProcessForNeighbouring4x4ChromaBlock(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t chroma4x4BlkIdx, HANSIZE mbAddrN[2], uint8_t chroma4x4BlkIdxN[2]);

/* 参考标准的 6.4.11.6 章节 */
#define DerivationProcessForNeighbouring4x4ChromaBlockForChromaArrayType3 DerivationProcessForNeighbouring4x4LumaBlock // 参考标准对 4:4:4 的色度 4x4 块的说明，它等价于亮度 4x4 块

/* 参考标准的 6.4.11.7 章节 */
void DerivationProcessForNeighbouringPartitions(PVIDEOH264DECODEINFO pDecodeInfo, HANSIZE mbAddrN[2], uint8_t mbPartIdxN[2], uint8_t subMbPartIdxN[2]);

#ifdef __cplusplus
}
#endif

#endif
