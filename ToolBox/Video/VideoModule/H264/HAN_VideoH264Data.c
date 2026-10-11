#include "HAN_VideoH264Data.h"

#define VIDEO_H264_NumMbPart_NA                 (uint8_t)(0)    // 16x16 的宏块必然只能分成 1、2、4，而 0 一定是无效值

typedef struct tagVIDEOH264MBTYPEINFO {
    VIDEOH264MBPREDMODECAT          mbPredModeCat;
    VIDEOH264MBPARTPREDMODE         MbPartPredMode[VIDEO_H264_MACRO_BLOCK_PART_SIZE];
    uint8_t                         Intra16x16PredMode;
    uint8_t                         CodedBlockPatternChroma;
    uint8_t                         CodedBlockPatternLuma;
    uint8_t                         NumMbPart;
    uint8_t                         MbPartWidth;
    uint8_t                         MbPartHeight;
} VIDEOH264MBTYPEINFO;
typedef struct tagVIDEOH264SUBMBTYPEINFO {
    uint8_t                         NumSubMbPart;
    VIDEOH264MBPARTPREDMODE         SubMbPredMode;
    uint8_t                         SubMbPartWidth;
    uint8_t                         SubMbPartHeight;
} VIDEOH264SUBMBTYPEINFO;

static const VIDEOH264MBTYPE sg_pMacroBlockTypeI[26] = { // 参考标准的 Table 7-11
    [0] = VIDEO_H264_MB_TYPE_I_NxN,
    [1] = VIDEO_H264_MB_TYPE_I_16x16_0_0_0,
    [2] = VIDEO_H264_MB_TYPE_I_16x16_1_0_0,
    [3] = VIDEO_H264_MB_TYPE_I_16x16_2_0_0,
    [4] = VIDEO_H264_MB_TYPE_I_16x16_3_0_0,
    [5] = VIDEO_H264_MB_TYPE_I_16x16_0_1_0,
    [6] = VIDEO_H264_MB_TYPE_I_16x16_1_1_0,
    [7] = VIDEO_H264_MB_TYPE_I_16x16_2_1_0,
    [8] = VIDEO_H264_MB_TYPE_I_16x16_3_1_0,
    [9] = VIDEO_H264_MB_TYPE_I_16x16_0_2_0,
    [10] = VIDEO_H264_MB_TYPE_I_16x16_1_2_0,
    [11] = VIDEO_H264_MB_TYPE_I_16x16_2_2_0,
    [12] = VIDEO_H264_MB_TYPE_I_16x16_3_2_0,
    [13] = VIDEO_H264_MB_TYPE_I_16x16_0_0_1,
    [14] = VIDEO_H264_MB_TYPE_I_16x16_1_0_1,
    [15] = VIDEO_H264_MB_TYPE_I_16x16_2_0_1,
    [16] = VIDEO_H264_MB_TYPE_I_16x16_3_0_1,
    [17] = VIDEO_H264_MB_TYPE_I_16x16_0_1_1,
    [18] = VIDEO_H264_MB_TYPE_I_16x16_1_1_1,
    [19] = VIDEO_H264_MB_TYPE_I_16x16_2_1_1,
    [20] = VIDEO_H264_MB_TYPE_I_16x16_3_1_1,
    [21] = VIDEO_H264_MB_TYPE_I_16x16_0_2_1,
    [22] = VIDEO_H264_MB_TYPE_I_16x16_1_2_1,
    [23] = VIDEO_H264_MB_TYPE_I_16x16_2_2_1,
    [24] = VIDEO_H264_MB_TYPE_I_16x16_3_2_1,
    [25] = VIDEO_H264_MB_TYPE_I_PCM,
};
static const VIDEOH264MBTYPE sg_pMacroBlockTypeSI[1] = { // 参考标准的 Table 7-12
    [0] = VIDEO_H264_MB_TYPE_SI,
};
static const VIDEOH264MBTYPE sg_pMacroBlockTypePAndSP[5] = { // 参考标准的 Table 7-13
    [0] = VIDEO_H264_MB_TYPE_P_L0_16x16,
    [1] = VIDEO_H264_MB_TYPE_P_L0_L0_16x8,
    [2] = VIDEO_H264_MB_TYPE_P_L0_L0_8x16,
    [3] = VIDEO_H264_MB_TYPE_P_8x8,
    [4] = VIDEO_H264_MB_TYPE_P_8x8ref0,
};
static const VIDEOH264MBTYPE sg_pMacroBlockTypeB[23] = { // 参考标准的 Table 7-14
    [0] = VIDEO_H264_MB_TYPE_B_Direct_16x16,
    [1] = VIDEO_H264_MB_TYPE_B_L0_16x16,
    [2] = VIDEO_H264_MB_TYPE_B_L1_16x16,
    [3] = VIDEO_H264_MB_TYPE_B_Bi_16x16,
    [4] = VIDEO_H264_MB_TYPE_B_L0_L0_16x8,
    [5] = VIDEO_H264_MB_TYPE_B_L0_L0_8x16,
    [6] = VIDEO_H264_MB_TYPE_B_L1_L1_16x8,
    [7] = VIDEO_H264_MB_TYPE_B_L1_L1_8x16,
    [8] = VIDEO_H264_MB_TYPE_B_L0_L1_16x8,
    [9] = VIDEO_H264_MB_TYPE_B_L0_L1_8x16,
    [10] = VIDEO_H264_MB_TYPE_B_L1_L0_16x8,
    [11] = VIDEO_H264_MB_TYPE_B_L1_L0_8x16,
    [12] = VIDEO_H264_MB_TYPE_B_L0_Bi_16x8,
    [13] = VIDEO_H264_MB_TYPE_B_L0_Bi_8x16,
    [14] = VIDEO_H264_MB_TYPE_B_L1_Bi_16x8,
    [15] = VIDEO_H264_MB_TYPE_B_L1_Bi_8x16,
    [16] = VIDEO_H264_MB_TYPE_B_Bi_L0_16x8,
    [17] = VIDEO_H264_MB_TYPE_B_Bi_L0_8x16,
    [18] = VIDEO_H264_MB_TYPE_B_Bi_L1_16x8,
    [19] = VIDEO_H264_MB_TYPE_B_Bi_L1_8x16,
    [20] = VIDEO_H264_MB_TYPE_B_Bi_Bi_16x8,
    [21] = VIDEO_H264_MB_TYPE_B_Bi_Bi_8x16,
    [22] = VIDEO_H264_MB_TYPE_B_8x8,
};
static const VIDEOH264MBTYPEINFO sg_pMacroBlockTypeInfo[VIDEO_H264_MB_TYPE_CNT] = {
    /* I_NxN 相比其它类型比较复杂，在代码里实现 */
    [VIDEO_H264_MB_TYPE_I_16x16_0_0_0] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTRA,
        .NumMbPart = VIDEO_H264_NumMbPart_NA,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .Intra16x16PredMode = 0,
        .CodedBlockPatternChroma = 0,
        .CodedBlockPatternLuma = 0,
    },
    [VIDEO_H264_MB_TYPE_I_16x16_1_0_0] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTRA,
        .NumMbPart = VIDEO_H264_NumMbPart_NA,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .Intra16x16PredMode = 1,
        .CodedBlockPatternChroma = 0,
        .CodedBlockPatternLuma = 0,
    },
    [VIDEO_H264_MB_TYPE_I_16x16_2_0_0] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTRA,
        .NumMbPart = VIDEO_H264_NumMbPart_NA,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .Intra16x16PredMode = 2,
        .CodedBlockPatternChroma = 0,
        .CodedBlockPatternLuma = 0,
    },
    [VIDEO_H264_MB_TYPE_I_16x16_3_0_0] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTRA,
        .NumMbPart = VIDEO_H264_NumMbPart_NA,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .Intra16x16PredMode = 3,
        .CodedBlockPatternChroma = 0,
        .CodedBlockPatternLuma = 0,
    },
    [VIDEO_H264_MB_TYPE_I_16x16_0_1_0] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTRA,
        .NumMbPart = VIDEO_H264_NumMbPart_NA,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .Intra16x16PredMode = 0,
        .CodedBlockPatternChroma = 1,
        .CodedBlockPatternLuma = 0,
    },
    [VIDEO_H264_MB_TYPE_I_16x16_1_1_0] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTRA,
        .NumMbPart = VIDEO_H264_NumMbPart_NA,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .Intra16x16PredMode = 1,
        .CodedBlockPatternChroma = 1,
        .CodedBlockPatternLuma = 0,
    },
    [VIDEO_H264_MB_TYPE_I_16x16_2_1_0] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTRA,
        .NumMbPart = VIDEO_H264_NumMbPart_NA,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .Intra16x16PredMode = 2,
        .CodedBlockPatternChroma = 1,
        .CodedBlockPatternLuma = 0,
    },
    [VIDEO_H264_MB_TYPE_I_16x16_3_1_0] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTRA,
        .NumMbPart = VIDEO_H264_NumMbPart_NA,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .Intra16x16PredMode = 3,
        .CodedBlockPatternChroma = 1,
        .CodedBlockPatternLuma = 0,
    },
    [VIDEO_H264_MB_TYPE_I_16x16_0_2_0] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTRA,
        .NumMbPart = VIDEO_H264_NumMbPart_NA,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .Intra16x16PredMode = 0,
        .CodedBlockPatternChroma = 2,
        .CodedBlockPatternLuma = 0,
    },
    [VIDEO_H264_MB_TYPE_I_16x16_1_2_0] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTRA,
        .NumMbPart = VIDEO_H264_NumMbPart_NA,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .Intra16x16PredMode = 1,
        .CodedBlockPatternChroma = 2,
        .CodedBlockPatternLuma = 0,
    },
    [VIDEO_H264_MB_TYPE_I_16x16_2_2_0] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTRA,
        .NumMbPart = VIDEO_H264_NumMbPart_NA,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .Intra16x16PredMode = 2,
        .CodedBlockPatternChroma = 2,
        .CodedBlockPatternLuma = 0,
    },
    [VIDEO_H264_MB_TYPE_I_16x16_3_2_0] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTRA,
        .NumMbPart = VIDEO_H264_NumMbPart_NA,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .Intra16x16PredMode = 3,
        .CodedBlockPatternChroma = 2,
        .CodedBlockPatternLuma = 0,
    },
    [VIDEO_H264_MB_TYPE_I_16x16_0_0_1] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTRA,
        .NumMbPart = VIDEO_H264_NumMbPart_NA,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .Intra16x16PredMode = 0,
        .CodedBlockPatternChroma = 0,
        .CodedBlockPatternLuma = 15,
    },
    [VIDEO_H264_MB_TYPE_I_16x16_1_0_1] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTRA,
        .NumMbPart = VIDEO_H264_NumMbPart_NA,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .Intra16x16PredMode = 1,
        .CodedBlockPatternChroma = 0,
        .CodedBlockPatternLuma = 15,
    },
    [VIDEO_H264_MB_TYPE_I_16x16_2_0_1] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTRA,
        .NumMbPart = VIDEO_H264_NumMbPart_NA,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .Intra16x16PredMode = 2,
        .CodedBlockPatternChroma = 0,
        .CodedBlockPatternLuma = 15,
    },
    [VIDEO_H264_MB_TYPE_I_16x16_3_0_1] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTRA,
        .NumMbPart = VIDEO_H264_NumMbPart_NA,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .Intra16x16PredMode = 3,
        .CodedBlockPatternChroma = 0,
        .CodedBlockPatternLuma = 15,
    },
    [VIDEO_H264_MB_TYPE_I_16x16_0_1_1] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTRA,
        .NumMbPart = VIDEO_H264_NumMbPart_NA,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .Intra16x16PredMode = 0,
        .CodedBlockPatternChroma = 1,
        .CodedBlockPatternLuma = 15,
    },
    [VIDEO_H264_MB_TYPE_I_16x16_1_1_1] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTRA,
        .NumMbPart = VIDEO_H264_NumMbPart_NA,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .Intra16x16PredMode = 1,
        .CodedBlockPatternChroma = 1,
        .CodedBlockPatternLuma = 15,
    },
    [VIDEO_H264_MB_TYPE_I_16x16_2_1_1] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTRA,
        .NumMbPart = VIDEO_H264_NumMbPart_NA,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .Intra16x16PredMode = 2,
        .CodedBlockPatternChroma = 1,
        .CodedBlockPatternLuma = 15,
    },
    [VIDEO_H264_MB_TYPE_I_16x16_3_1_1] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTRA,
        .NumMbPart = VIDEO_H264_NumMbPart_NA,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .Intra16x16PredMode = 3,
        .CodedBlockPatternChroma = 1,
        .CodedBlockPatternLuma = 15,
    },
    [VIDEO_H264_MB_TYPE_I_16x16_0_2_1] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTRA,
        .NumMbPart = VIDEO_H264_NumMbPart_NA,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .Intra16x16PredMode = 0,
        .CodedBlockPatternChroma = 2,
        .CodedBlockPatternLuma = 15,
    },
    [VIDEO_H264_MB_TYPE_I_16x16_1_2_1] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTRA,
        .NumMbPart = VIDEO_H264_NumMbPart_NA,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .Intra16x16PredMode = 1,
        .CodedBlockPatternChroma = 2,
        .CodedBlockPatternLuma = 15,
    },
    [VIDEO_H264_MB_TYPE_I_16x16_2_2_1] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTRA,
        .NumMbPart = VIDEO_H264_NumMbPart_NA,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .Intra16x16PredMode = 2,
        .CodedBlockPatternChroma = 2,
        .CodedBlockPatternLuma = 15,
    },
    [VIDEO_H264_MB_TYPE_I_16x16_3_2_1] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTRA,
        .NumMbPart = VIDEO_H264_NumMbPart_NA,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .Intra16x16PredMode = 3,
        .CodedBlockPatternChroma = 2,
        .CodedBlockPatternLuma = 15,
    },
    [VIDEO_H264_MB_TYPE_I_PCM] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_OTHER,
        .NumMbPart = VIDEO_H264_NumMbPart_NA,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_NA, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
    },
    [VIDEO_H264_MB_TYPE_SI] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTRA,
        .NumMbPart = VIDEO_H264_NumMbPart_NA,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Intra_4x4, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        /* CodedBlockPatternChroma 和 CodedBlockPatternLuma 公式计算 */
    },
    [VIDEO_H264_MB_TYPE_P_L0_16x16] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .NumMbPart = 1,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L0, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .MbPartWidth = 16,
        .MbPartHeight = 16,
    },
    [VIDEO_H264_MB_TYPE_P_L0_L0_16x8] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .NumMbPart = 2,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L0, [1] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L0, },
        .MbPartWidth = 16,
        .MbPartHeight = 8,
    },
    [VIDEO_H264_MB_TYPE_P_L0_L0_8x16] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .NumMbPart = 2,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L0, [1] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L0, },
        .MbPartWidth = 8,
        .MbPartHeight = 16,
    },
    [VIDEO_H264_MB_TYPE_P_8x8] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .NumMbPart = 4,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_NA, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .MbPartWidth = 8,
        .MbPartHeight = 8,
    },
    [VIDEO_H264_MB_TYPE_P_8x8ref0] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .NumMbPart = 4,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_NA, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .MbPartWidth = 8,
        .MbPartHeight = 8,
    },
    [VIDEO_H264_MB_TYPE_P_Skip] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .NumMbPart = 1,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L0, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .MbPartWidth = 16,
        .MbPartHeight = 16,
    },
    [VIDEO_H264_MB_TYPE_B_Direct_16x16] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .NumMbPart = VIDEO_H264_NumMbPart_NA,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Direct, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .MbPartWidth = 8,
        .MbPartHeight = 8,
    },
    [VIDEO_H264_MB_TYPE_B_L0_16x16] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .NumMbPart = 1,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L0, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .MbPartWidth = 16,
        .MbPartHeight = 16,
    },
    [VIDEO_H264_MB_TYPE_B_L1_16x16] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .NumMbPart = 1,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L1, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .MbPartWidth = 16,
        .MbPartHeight = 16,
    },
    [VIDEO_H264_MB_TYPE_B_Bi_16x16] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .NumMbPart = 1,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_BiPred, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .MbPartWidth = 16,
        .MbPartHeight = 16,
    },
    [VIDEO_H264_MB_TYPE_B_L0_L0_16x8] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .NumMbPart = 2,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L0, [1] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L0, },
        .MbPartWidth = 16,
        .MbPartHeight = 8,
    },
    [VIDEO_H264_MB_TYPE_B_L0_L0_8x16] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .NumMbPart = 2,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L0, [1] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L0, },
        .MbPartWidth = 8,
        .MbPartHeight = 16,
    },
    [VIDEO_H264_MB_TYPE_B_L1_L1_16x8] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .NumMbPart = 2,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L1, [1] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L1, },
        .MbPartWidth = 16,
        .MbPartHeight = 8,
    },
    [VIDEO_H264_MB_TYPE_B_L1_L1_8x16] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .NumMbPart = 2,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L1, [1] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L1, },
        .MbPartWidth = 8,
        .MbPartHeight = 16,
    },
    [VIDEO_H264_MB_TYPE_B_L0_L1_16x8] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .NumMbPart = 2,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L0, [1] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L1, },
        .MbPartWidth = 16,
        .MbPartHeight = 8,
    },
    [VIDEO_H264_MB_TYPE_B_L0_L1_8x16] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .NumMbPart = 2,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L0, [1] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L1, },
        .MbPartWidth = 8,
        .MbPartHeight = 16,
    },
    [VIDEO_H264_MB_TYPE_B_L1_L0_16x8] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .NumMbPart = 2,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L1, [1] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L0, },
        .MbPartWidth = 16,
        .MbPartHeight = 8,
    },
    [VIDEO_H264_MB_TYPE_B_L1_L0_8x16] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .NumMbPart = 2,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L1, [1] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L0, },
        .MbPartWidth = 8,
        .MbPartHeight = 16,
    },
    [VIDEO_H264_MB_TYPE_B_L0_Bi_16x8] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .NumMbPart = 2,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L0, [1] = VIDEO_H264_MB_PART_PRED_MODE_BiPred, },
        .MbPartWidth = 16,
        .MbPartHeight = 8,
    },
    [VIDEO_H264_MB_TYPE_B_L0_Bi_8x16] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .NumMbPart = 2,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L0, [1] = VIDEO_H264_MB_PART_PRED_MODE_BiPred, },
        .MbPartWidth = 8,
        .MbPartHeight = 16,
    },
    [VIDEO_H264_MB_TYPE_B_L1_Bi_16x8] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .NumMbPart = 2,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L1, [1] = VIDEO_H264_MB_PART_PRED_MODE_BiPred, },
        .MbPartWidth = 16,
        .MbPartHeight = 8,
    },
    [VIDEO_H264_MB_TYPE_B_L1_Bi_8x16] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .NumMbPart = 2,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L1, [1] = VIDEO_H264_MB_PART_PRED_MODE_BiPred, },
        .MbPartWidth = 8,
        .MbPartHeight = 16,
    },
    [VIDEO_H264_MB_TYPE_B_Bi_L0_16x8] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .NumMbPart = 2,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_BiPred, [1] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L0, },
        .MbPartWidth = 16,
        .MbPartHeight = 8,
    },
    [VIDEO_H264_MB_TYPE_B_Bi_L0_8x16] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .NumMbPart = 2,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_BiPred, [1] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L0, },
        .MbPartWidth = 8,
        .MbPartHeight = 16,
    },
    [VIDEO_H264_MB_TYPE_B_Bi_L1_16x8] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .NumMbPart = 2,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_BiPred, [1] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L1, },
        .MbPartWidth = 16,
        .MbPartHeight = 8,
    },
    [VIDEO_H264_MB_TYPE_B_Bi_L1_8x16] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .NumMbPart = 2,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_BiPred, [1] = VIDEO_H264_MB_PART_PRED_MODE_Pred_L1, },
        .MbPartWidth = 8,
        .MbPartHeight = 16,
    },
    [VIDEO_H264_MB_TYPE_B_Bi_Bi_16x8] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .NumMbPart = 2,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_BiPred, [1] = VIDEO_H264_MB_PART_PRED_MODE_BiPred, },
        .MbPartWidth = 16,
        .MbPartHeight = 8,
    },
    [VIDEO_H264_MB_TYPE_B_Bi_Bi_8x16] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .NumMbPart = 2,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_BiPred, [1] = VIDEO_H264_MB_PART_PRED_MODE_BiPred, },
        .MbPartWidth = 8,
        .MbPartHeight = 16,
    },
    [VIDEO_H264_MB_TYPE_B_8x8] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .NumMbPart = 4,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_NA, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .MbPartWidth = 8,
        .MbPartHeight = 8,
    },
    [VIDEO_H264_MB_TYPE_B_Skip] = {
        .mbPredModeCat = VIDEO_H264_MB_PRED_MODE_CAT_INTER,
        .MbPartPredMode = { [0] = VIDEO_H264_MB_PART_PRED_MODE_Direct, [1] = VIDEO_H264_MB_PART_PRED_MODE_NA, },
        .MbPartWidth = 8,
        .MbPartHeight = 8,
    },
};

static const VIDEOH264SUBMBTYPE sg_pSubMacroBlockTypeP[4] = {
    [0] = VIDEO_H264_SUB_MB_TYPE_P_L0_8x8,
    [1] = VIDEO_H264_SUB_MB_TYPE_P_L0_8x4,
    [2] = VIDEO_H264_SUB_MB_TYPE_P_L0_4x8,
    [3] = VIDEO_H264_SUB_MB_TYPE_P_L0_4x4,
};
static const VIDEOH264SUBMBTYPE sg_pSubMacroBlockTypeB[13] = {
    [0] = VIDEO_H264_SUB_MB_TYPE_B_Direct_8x8,
    [1] = VIDEO_H264_SUB_MB_TYPE_B_L0_8x8,
    [2] = VIDEO_H264_SUB_MB_TYPE_B_L1_8x8,
    [3] = VIDEO_H264_SUB_MB_TYPE_B_Bi_8x8,
    [4] = VIDEO_H264_SUB_MB_TYPE_B_L0_8x4,
    [5] = VIDEO_H264_SUB_MB_TYPE_B_L0_4x8,
    [6] = VIDEO_H264_SUB_MB_TYPE_B_L1_8x4,
    [7] = VIDEO_H264_SUB_MB_TYPE_B_L1_4x8,
    [8] = VIDEO_H264_SUB_MB_TYPE_B_Bi_8x4,
    [9] = VIDEO_H264_SUB_MB_TYPE_B_Bi_4x8,
    [10] = VIDEO_H264_SUB_MB_TYPE_B_L0_4x4,
    [11] = VIDEO_H264_SUB_MB_TYPE_B_L1_4x4,
    [12] = VIDEO_H264_SUB_MB_TYPE_B_Bi_4x4,
};
static const VIDEOH264SUBMBTYPEINFO sg_pSubMacroBlockTypeInfo[VIDEO_H264_SUB_MB_TYPE_CNT] = {
    [VIDEO_H264_SUB_MB_TYPE_P_L0_8x8] = {
        .NumSubMbPart = 1,
        .SubMbPredMode = VIDEO_H264_MB_PART_PRED_MODE_Pred_L0,
        .SubMbPartWidth = 8,
        .SubMbPartHeight = 8,
    },
    [VIDEO_H264_SUB_MB_TYPE_P_L0_8x4] = {
        .NumSubMbPart = 1,
        .SubMbPredMode = VIDEO_H264_MB_PART_PRED_MODE_Pred_L0,
        .SubMbPartWidth = 8,
        .SubMbPartHeight = 4,
    },
    [VIDEO_H264_SUB_MB_TYPE_P_L0_4x8] = {
        .NumSubMbPart = 1,
        .SubMbPredMode = VIDEO_H264_MB_PART_PRED_MODE_Pred_L0,
        .SubMbPartWidth = 4,
        .SubMbPartHeight = 8,
    },
    [VIDEO_H264_SUB_MB_TYPE_P_L0_4x4] = {
        .NumSubMbPart = 1,
        .SubMbPredMode = VIDEO_H264_MB_PART_PRED_MODE_Pred_L0,
        .SubMbPartWidth = 4,
        .SubMbPartHeight = 4,
    },
    [VIDEO_H264_SUB_MB_TYPE_B_Direct_8x8] = {
        .NumSubMbPart = 4,
        .SubMbPredMode = VIDEO_H264_MB_PART_PRED_MODE_Direct,
        .SubMbPartWidth = 4,
        .SubMbPartHeight = 4,
    },
    [VIDEO_H264_SUB_MB_TYPE_B_L0_8x8] = {
        .NumSubMbPart = 1,
        .SubMbPredMode = VIDEO_H264_MB_PART_PRED_MODE_Pred_L0,
        .SubMbPartWidth = 8,
        .SubMbPartHeight = 8,
    },
    [VIDEO_H264_SUB_MB_TYPE_B_L1_8x8] = {
        .NumSubMbPart = 1,
        .SubMbPredMode = VIDEO_H264_MB_PART_PRED_MODE_Pred_L1,
        .SubMbPartWidth = 8,
        .SubMbPartHeight = 8,
    },
    [VIDEO_H264_SUB_MB_TYPE_B_Bi_8x8] = {
        .NumSubMbPart = 1,
        .SubMbPredMode = VIDEO_H264_MB_PART_PRED_MODE_BiPred,
        .SubMbPartWidth = 8,
        .SubMbPartHeight = 8,
    },
    [VIDEO_H264_SUB_MB_TYPE_B_L0_8x4] = {
        .NumSubMbPart = 2,
        .SubMbPredMode = VIDEO_H264_MB_PART_PRED_MODE_Pred_L0,
        .SubMbPartWidth = 8,
        .SubMbPartHeight = 4,
    },
    [VIDEO_H264_SUB_MB_TYPE_B_L0_4x8] = {
        .NumSubMbPart = 2,
        .SubMbPredMode = VIDEO_H264_MB_PART_PRED_MODE_Pred_L0,
        .SubMbPartWidth = 4,
        .SubMbPartHeight = 8,
    },
    [VIDEO_H264_SUB_MB_TYPE_B_L1_8x4] = {
        .NumSubMbPart = 2,
        .SubMbPredMode = VIDEO_H264_MB_PART_PRED_MODE_Pred_L1,
        .SubMbPartWidth = 8,
        .SubMbPartHeight = 4,
    },
    [VIDEO_H264_SUB_MB_TYPE_B_L1_4x8] = {
        .NumSubMbPart = 2,
        .SubMbPredMode = VIDEO_H264_MB_PART_PRED_MODE_Pred_L1,
        .SubMbPartWidth = 4,
        .SubMbPartHeight = 8,
    },
    [VIDEO_H264_SUB_MB_TYPE_B_Bi_8x4] = {
        .NumSubMbPart = 2,
        .SubMbPredMode = VIDEO_H264_MB_PART_PRED_MODE_BiPred,
        .SubMbPartWidth = 8,
        .SubMbPartHeight = 4,
    },
    [VIDEO_H264_SUB_MB_TYPE_B_Bi_4x8] = {
        .NumSubMbPart = 2,
        .SubMbPredMode = VIDEO_H264_MB_PART_PRED_MODE_BiPred,
        .SubMbPartWidth = 4,
        .SubMbPartHeight = 8,
    },
    [VIDEO_H264_SUB_MB_TYPE_B_L0_4x4] = {
        .NumSubMbPart = 4,
        .SubMbPredMode = VIDEO_H264_MB_PART_PRED_MODE_Pred_L0,
        .SubMbPartWidth = 4,
        .SubMbPartHeight = 4,
    },
    [VIDEO_H264_SUB_MB_TYPE_B_L1_4x4] = {
        .NumSubMbPart = 4,
        .SubMbPredMode = VIDEO_H264_MB_PART_PRED_MODE_Pred_L1,
        .SubMbPartWidth = 4,
        .SubMbPartHeight = 4,
    },
    [VIDEO_H264_SUB_MB_TYPE_B_Bi_4x4] = {
        .NumSubMbPart = 4,
        .SubMbPredMode = VIDEO_H264_MB_PART_PRED_MODE_BiPred,
        .SubMbPartWidth = 4,
        .SubMbPartHeight = 4,
    },
};

void DecodeH264InitMacroBlock(PVIDEOH264DECODEINFO pDecodeInfo, uint32_t mb_type)
{
    PVIDEOH264PICTUREINFO pPicInfo = pDecodeInfo->pPicInfo;
    PVIDEOH264MACROBLOCK pMacroBlock = &(pPicInfo->pMacroBlock[pDecodeInfo->CurrMbAddr]);
    VIDEOH264MBTYPE mbType;

    /* 更新宏块类型 */
    switch (pPicInfo->slice.type) {
        case VIDEO_H264_SLICE_TYPE_I: {
            if (mb_type <= 25) { mbType = sg_pMacroBlockTypeI[mb_type]; }
            else { mbType = VIDEO_H264_MB_TYPE_CNT; }
        } break;
        case VIDEO_H264_SLICE_TYPE_SI: {
            if (0 == mb_type) { mbType = sg_pMacroBlockTypeSI[mb_type]; }
            else if (mb_type <= 26) { mbType = sg_pMacroBlockTypeI[mb_type - 1]; }
            else { mbType = VIDEO_H264_MB_TYPE_CNT; }
        } break;
        case VIDEO_H264_SLICE_TYPE_P:
        case VIDEO_H264_SLICE_TYPE_SP: {
            if (mb_type <= 4) { mbType = sg_pMacroBlockTypePAndSP[mb_type]; }
            else if (mb_type <= 30) { mbType = sg_pMacroBlockTypeI[mb_type - 5]; }
            else { mbType = VIDEO_H264_MB_TYPE_CNT; }
        } break;
        case VIDEO_H264_SLICE_TYPE_B: {
            if (mb_type <= 22) { mbType = sg_pMacroBlockTypeB[mb_type]; }
            else if (mb_type <= 48) { mbType = sg_pMacroBlockTypeI[mb_type - 23]; }
            else { mbType = VIDEO_H264_MB_TYPE_CNT; }
        } break;
        default: { mbType = VIDEO_H264_MB_TYPE_CNT; } break;
    }
    pMacroBlock->type = mbType;
    /* 更新 MbPartPredMode */
    pMacroBlock->mbPredModeCat = sg_pMacroBlockTypeInfo[mbType].mbPredModeCat;
    for (HANSIZE iLoop = 0; iLoop < VIDEO_H264_MACRO_BLOCK_PART_SIZE; iLoop++)
    {
        if (VIDEO_H264_MB_TYPE_I_NxN == mbType)
        {
            switch (pMacroBlock->transform_size_8x8_flag) {
                case 0: { pMacroBlock->MbPartPredMode[iLoop] = VIDEO_H264_MB_PART_PRED_MODE_Intra_4x4; } break;
                case 1: { pMacroBlock->MbPartPredMode[iLoop] = VIDEO_H264_MB_PART_PRED_MODE_Intra_8x8; } break;
                default: { pMacroBlock->MbPartPredMode[iLoop] = VIDEO_H264_MB_PART_PRED_MODE_NA; } break;
            }
        }
        else if ((0 <= mbType) && (mbType < VIDEO_H264_MB_TYPE_CNT)) { pMacroBlock->MbPartPredMode[iLoop] = sg_pMacroBlockTypeInfo[mbType].MbPartPredMode[iLoop]; }
        else { pMacroBlock->MbPartPredMode[iLoop] = VIDEO_H264_MB_PART_PRED_MODE_NA; }
    }
    /* 更新 NumMbPart */
    if ((0 <= mbType) && (mbType < VIDEO_H264_MB_TYPE_CNT)) { pMacroBlock->NumMbPart = sg_pMacroBlockTypeInfo[mbType].NumMbPart; }
    else { pMacroBlock->NumMbPart = VIDEO_H264_NumMbPart_NA; }
    /* 更新 SI 和 I_NxN 类型的 CodedBlockPatternLuma 和 CodedBlockPatternChroma 以及 Intra16x16PredMode */
    if ((VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16 == pMacroBlock->MbPartPredMode[0]) && (mbType < VIDEO_H264_MB_TYPE_CNT))
    {
        pMacroBlock->Intra16x16PredMode = sg_pMacroBlockTypeInfo[mbType].Intra16x16PredMode;
        pMacroBlock->CodedBlockPatternLuma = sg_pMacroBlockTypeInfo[mbType].CodedBlockPatternLuma;
        pMacroBlock->CodedBlockPatternChroma = sg_pMacroBlockTypeInfo[mbType].CodedBlockPatternChroma;
    }
    /* 更新预测分区大小 */
    pMacroBlock->MbPartWidth = sg_pMacroBlockTypeInfo[mbType].MbPartWidth;
    pMacroBlock->MbPartHeight = sg_pMacroBlockTypeInfo[mbType].MbPartHeight;
    /* 其它 */
    pMacroBlock->transform_size_8x8_flag = 0;
}

void DecodeH264InitSubMacroBlock(PVIDEOH264DECODEINFO pDecodeInfo)
{
    PVIDEOH264PICTUREINFO pPicInfo = pDecodeInfo->pPicInfo;
    PVIDEOH264MACROBLOCK pMacroBlock = &(pPicInfo->pMacroBlock[pDecodeInfo->CurrMbAddr]);
    VIDEOH264SUBMBTYPE subType;

    for (uint8_t mbPartIdx = 0; mbPartIdx < 4; mbPartIdx++)
    {
        switch (pPicInfo->slice.type) {
            case VIDEO_H264_SLICE_TYPE_P:
            case VIDEO_H264_SLICE_TYPE_SP: {
                subType = sg_pSubMacroBlockTypeP[pMacroBlock->sub_mb_type[mbPartIdx]];
            } break;
            case VIDEO_H264_SLICE_TYPE_B: {
                subType = sg_pSubMacroBlockTypeB[pMacroBlock->sub_mb_type[mbPartIdx]];
            } break;
            default: { subType = VIDEO_H264_SUB_MB_TYPE_CNT; } break;
        }
        if (subType < VIDEO_H264_SUB_MB_TYPE_CNT)
        {
            pMacroBlock->subType[mbPartIdx] = subType;
            pMacroBlock->NumSubMbPart[mbPartIdx] = sg_pSubMacroBlockTypeInfo[subType].NumSubMbPart;
            pMacroBlock->SubMbPredMode[mbPartIdx] = sg_pSubMacroBlockTypeInfo[subType].SubMbPredMode;
            pMacroBlock->SubMbPartWidth[mbPartIdx] = sg_pSubMacroBlockTypeInfo[subType].SubMbPartWidth;
            pMacroBlock->SubMbPartHeight[mbPartIdx] = sg_pSubMacroBlockTypeInfo[subType].SubMbPartHeight;
        }
    }
}