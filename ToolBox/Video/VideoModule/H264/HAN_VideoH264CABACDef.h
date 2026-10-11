#ifndef HAN_VIDEO_H264_CABAC_DEF_H
#define HAN_VIDEO_H264_CABAC_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "HAN_VideoH264ReadBit.h"
#include "HAN_VideoH264Def.h"

#define VIDEO_H264_CABAC_CONTEXT_SIZE                                       1024

typedef enum {
    VIDEO_H264_CABAC_CONTEXT_M,
    VIDEO_H264_CABAC_CONTEXT_N,
    VIDEO_H264_CABAC_CONTEXT_MN,
} VIDEOH264CABACCONTEXTMN;
typedef enum {
    VIDEO_H264_CABAC_CONTEXT_CABAC_INIT_IDC_I_SLICE,
    VIDEO_H264_CABAC_CONTEXT_CABAC_INIT_IDC_0,
    VIDEO_H264_CABAC_CONTEXT_CABAC_INIT_IDC_1,
    VIDEO_H264_CABAC_CONTEXT_CABAC_INIT_IDC_2,
    VIDEO_H264_CABAC_CONTEXT_CABAC_INIT_IDC_CNT,
} VIDEOH264CABACCONTEXTCABACINITIDC;

typedef struct tagVIDEOH264RESIDUALBLOCKSECABAC {
    uint8_t                         coded_block_flag[VIDEO_H264_BLOCK_CAT_CNT][2][16]; // ctxBlockCat = 3 或 4 的时候，需要区分 iCbCr，所有类型里最多 16 个块
    uint8_t                         significant_coeff_flag[64];
    uint8_t                         last_significant_coeff_flag[64];
    uint32_t                        coeff_abs_level_minus1[64];
    uint8_t                         coeff_sign_flag[64];
} VIDEOH264RESIDUALBLOCKSECABAC, * PVIDEOH264RESIDUALBLOCKSECABAC;

typedef uint8_t VIDEOH264CABACCONTEXTNUM;
typedef uint16_t VIDEOH264CABACENGINENUM;
typedef struct tagVIDEOH264READAE {
    struct {
        VIDEOH264CABACCONTEXTNUM    pStateIdx;
        VIDEOH264CABACCONTEXTNUM    valMPS;
    } context[VIDEO_H264_CABAC_CONTEXT_SIZE];
    struct {
        VIDEOH264CABACENGINENUM     codIRange;
        VIDEOH264CABACENGINENUM     codIOffset;
    } engine;
    uint8_t                         numDecodAbsLevelEq1;    // coeffLevel 的绝对值等于 1 的计数
    uint8_t                         numDecodAbsLevelGt1;    // coeffLevel 的绝对值大于 1 的计数
    VIDEOH264VALUE                  cValue;
} VIDEOH264READAE, * PVIDEOH264READAE;

#ifdef __cplusplus
}
#endif

#endif
