#include "HAN_VideoH264Data.h"
#include "HAN_VideoH264Std.h"
#include "HAN_VideoH264Addr.h"

#define VIDEO_H264_CABAC_BYPASS                 (-1)
#define VIDEO_H264_CABAC_CTXBLOCKCAT_CNT        14

typedef int16_t VIDEOH264CABACCONTEXMN;
typedef uint32_t VIDEOH264CABACCTXIDX;

typedef struct tagVIDEOH264BINSTRING {
    uint32_t                        binString;
    uint32_t                        maxBinId;
} VIDEOH264BINSTRING;

typedef uint32_t (*GETCTXIDXCALLBACK)(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);

/* 解码过程：
* 1. 解码器初始化，根据标准的 9.3 章节第 3 段，在开始解码任何 slice data 前，都应执行一次初始化，具体过程在 9.3.1 章节
* 2. 在解码任何需要调用 ae(v) 的元素时，先进行二值化，具体过程在 9.3.2 章节
* 3. 根据第 2 步的二值化结果等到元素的值，具体过程在 9.3.3 章节
*/
#if 1 /* 确认需要使用的二值化函数，参考标准的 9.3.2 章节的 Table 9-34，其中关于参数 maxBinIdxCtx 和 ctxIdxOffset，相关功能做在 GetctxIdx 系列函数里 */
#if 1 /* 以下函数能在 Table 9-39 中查到计算 ctxIdx 的方法，使用通用逻辑 */
static VIDEOH264VALUE H264ReadAE_mb_skip_flag(PVIDEOH264DECODEINFO pDecodeInfo);
static VIDEOH264VALUE H264ReadAE_mb_field_decoding_flag(PVIDEOH264DECODEINFO pDecodeInfo);
static VIDEOH264VALUE H264ReadAE_mb_type(PVIDEOH264DECODEINFO pDecodeInfo);                         // mb_type 的二值化方法，参考标准的 9.3.2.5 章节
static VIDEOH264VALUE H264ReadAE_transform_size_8x8_flag(PVIDEOH264DECODEINFO pDecodeInfo);
static VIDEOH264VALUE H264ReadAE_coded_block_pattern(PVIDEOH264DECODEINFO pDecodeInfo);             // coded_block_pattern 的二值化方法，参考标准的 9.3.2.6 章节
static VIDEOH264VALUE H264ReadAE_mb_qp_delta(PVIDEOH264DECODEINFO pDecodeInfo);                     // mb_qp_delta 的二值化方法，参考标准的 9.3.2.7 章节
static VIDEOH264VALUE H264ReadAE_end_of_slice_flag(PVIDEOH264DECODEINFO pDecodeInfo);
static VIDEOH264VALUE H264ReadAE_sub_mb_type(PVIDEOH264DECODEINFO pDecodeInfo);
static VIDEOH264VALUE H264ReadAE_prev_intraNxN_pred_mode_flag(PVIDEOH264DECODEINFO pDecodeInfo);
static VIDEOH264VALUE H264ReadAE_rem_intraNxN_pred_mode(PVIDEOH264DECODEINFO pDecodeInfo);
static VIDEOH264VALUE H264ReadAE_intra_chroma_pred_mode(PVIDEOH264DECODEINFO pDecodeInfo);
static VIDEOH264VALUE H264ReadAE_mvd_l0_0(PVIDEOH264DECODEINFO pDecodeInfo);
static VIDEOH264VALUE H264ReadAE_mvd_l1_0(PVIDEOH264DECODEINFO pDecodeInfo);
static VIDEOH264VALUE H264ReadAE_mvd_l0_1(PVIDEOH264DECODEINFO pDecodeInfo);
static VIDEOH264VALUE H264ReadAE_mvd_l1_1(PVIDEOH264DECODEINFO pDecodeInfo);
static inline VIDEOH264VALUE H264ReadAE_mvd_lX(PVIDEOH264DECODEINFO pDecodeInfo, GETCTXIDXCALLBACK GetctxIdx);
static VIDEOH264VALUE H264ReadAE_ref_idx_l0(PVIDEOH264DECODEINFO pDecodeInfo);
static VIDEOH264VALUE H264ReadAE_ref_idx_l1(PVIDEOH264DECODEINFO pDecodeInfo);
static VIDEOH264VALUE H264ReadAE_coeff_sign_flag(PVIDEOH264DECODEINFO pDecodeInfo);
/* 上述二值化方法要用到的函数，包括通用的二值化方法和 mb_type 的查表 */
static uint32_t H264ReadAEBinU(PVIDEOH264DECODEINFO pDecodeInfo, GETCTXIDXCALLBACK GetctxIdx);                  // U 的二值化方法，参考标准的 9.3.2.1 章节
static uint32_t H264ReadAEBinTU(PVIDEOH264DECODEINFO pDecodeInfo, uint32_t cMax, GETCTXIDXCALLBACK GetctxIdx);  // TU 的二值化方法，参考标准的 9.3.2.2 章节
static uint32_t H264ReadAEBinUEGk(PVIDEOH264DECODEINFO pDecodeInfo, GETCTXIDXCALLBACK GetctxIdx);               // UEGk 的二值化方法，参考标准的 9.3.2.3 章节
static uint32_t H264ReadAEBinFL(PVIDEOH264DECODEINFO pDecodeInfo, uint32_t cMax, GETCTXIDXCALLBACK GetctxIdx);  // FL 的二值化方法，参考标准的 9.3.2.4 章节
static uint32_t H264ReadAEBinMBTypeI(PVIDEOH264DECODEINFO pDecodeInfo, GETCTXIDXCALLBACK GetctxIdx);            // I 类型宏块和子宏块的解码，参考标准的表格 9-36
static uint32_t H264ReadAEBinMBTypeP(PVIDEOH264DECODEINFO pDecodeInfo);     // P/SP 类型宏块的解码，参考标准的表格 9-37
static uint32_t H264ReadAEBinMBTypeB(PVIDEOH264DECODEINFO pDecodeInfo);     // B 类型宏块的解码，参考标准的表格 9-37
static uint32_t H264ReadAEBinSubMBTypeP(PVIDEOH264DECODEINFO pDecodeInfo);  // P/SP 类型子宏块的解码，参考标准的表格 9-38
static uint32_t H264ReadAEBinSubMBTypeB(PVIDEOH264DECODEINFO pDecodeInfo);  // B 类型子宏块的解码，参考标准的表格 9-38
#endif
#if 1 /* 以下函数不在 Table 9-39 中，涉及到复杂参数，所以单独写逻辑 */
static VIDEOH264VALUE H264ReadAE_coded_block_flag(PVIDEOH264DECODEINFO pDecodeInfo);
static VIDEOH264VALUE H264ReadAE_significant_coeff_flag(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t levelListIdx);
static VIDEOH264VALUE H264ReadAE_last_significant_coeff_flag(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t levelListIdx);
static VIDEOH264VALUE H264ReadAE_coeff_abs_level_minus1(PVIDEOH264DECODEINFO pDecodeInfo);
#endif
#endif
#if 1 /* 读取 bit 并转化成值，参考标准的 9.3.3 章节 */
/* 关于 maxBinIdxCtx 参数，表示该语法元素可能用到的最大 binIdx
 * 当不得不读到 binIdx >= maxBinIdxCtx（如 mb_qp_delta，理论上可以读很长），ctxIdx 取 maxBinIdxCtx 所使用的 ctxIdx（参考标准的章节 9.3.3.1 里关于 maxBinIdxCtx 的描述）
 * 这与标准中的 Table 9-39 其实也是相符的
 * 还是以 mb_qp_delta 举例：mb_qp_delta 的 maxBinIdxCtx 等于 2，Table 9-39 列出的 binIdx 最大给到 6，但其实 binIdx 从 2 到 6 所对应的 ctxIdxInc 就都是 3 了，与上文描述相符
 * 再比如 B、P/SP 帧的 mb_type 在读取后缀时 maxBinIdxCtx 都只有 5，但这种情况下读取的是 I 类型 mb_type，最长的 binIdx 有 6
 * 而 Table 9-39 里对应的 binIdx 从 5 到 6 的 ctxIdxInc 都是 3，也与上文描述相符
 * 所以其实 maxBinIdxCtx 理论上可以忽略，GetctxIdx 系列的代码参考 Table 9-39 编写，没有对 maxBinIdxCtx 进行判断，把 binIdx 从 0 到 6 的情况都写上了
 */
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mb_type_SISlice_Prefix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mb_type_SISlice_Suffix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mb_type_ISlice(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mb_skip_flag_PSlice(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mb_type_PSlice_Prefix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mb_type_PSlice_Suffix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_sub_mb_type_PSlice(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mb_skip_flag_BSlice(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mb_type_BSlice_Prefix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mb_type_BSlice_Suffix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_sub_mb_type_BSlice(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mvd_l0_0_Prefix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mvd_l1_0_Prefix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mvd_l0_1_Prefix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mvd_l1_1_Prefix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static inline VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mvd_lX_Prefix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx, VIDEOH264CABACCTXIDX ctxIdxOffset, uint8_t lX);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mvd_lX_Suffix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_ref_idx_l0(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_ref_idx_l1(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mb_qp_delta(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_intra_chroma_pred_mode(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_prev_intraNxN_pred_mode_flag(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_rem_intraNxN_pred_mode(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mb_field_decoding_flag(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_coded_block_pattern_Prefix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_coded_block_pattern_Suffix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_coeff_sign_flag(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_end_of_slice_flag(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_transform_size_8x8_flag(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_coded_block_flag(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_significant_coeff_flag(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_last_significant_coeff_flag(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_coeff_abs_level_minus1_Prefix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_coeff_abs_level_minus1_Suffix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx);

static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdxInc_mb_skip_flag(PVIDEOH264DECODEINFO pDecodeInfo);                                                              // 参考标准的 9.3.3.1.1.1 章节
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdxInc_mb_field_decoding_flag(PVIDEOH264DECODEINFO pDecodeInfo);                                                    // 参考标准的 9.3.3.1.1.2 章节
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdxInc_mb_type(PVIDEOH264DECODEINFO pDecodeInfo, VIDEOH264CABACCTXIDX ctxIdxOffset);                                // 参考标准的 9.3.3.1.1.3 章节
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdxInc_coded_block_pattern(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx, VIDEOH264CABACCTXIDX ctxIdxOffset);    // 参考标准的 9.3.3.1.1.4 章节
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdxInc_mb_qp_delta(PVIDEOH264DECODEINFO pDecodeInfo);                                                               // 参考标准的 9.3.3.1.1.5 章节
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdxInc_ref_idx_lX(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t lX);                                                    // 参考标准的 9.3.3.1.1.6 章节
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdxInc_mvd_lX(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t lX, VIDEOH264CABACCTXIDX ctxIdxOffset);                     // 参考标准的 9.3.3.1.1.7 章节
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdxInc_intra_chroma_pred_mode(PVIDEOH264DECODEINFO pDecodeInfo);                                                    // 参考标准的 9.3.3.1.1.8 章节
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdxInc_coded_block_flag(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t ctxBlockCat);                                     // 参考标准的 9.3.3.1.1.9 章节
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdxInc_transform_size_8x8_flag(PVIDEOH264DECODEINFO pDecodeInfo);                                                   // 参考标准的 9.3.3.1.1.10 章节
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdxInc_prior_decoded_bin(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx, VIDEOH264CABACCTXIDX ctxIdxOffset);      // 参考标准的 9.3.3.1.2 章节
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdxInc_significant_coeff_flag(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t ctxBlockCat);                               // 参考标准的 9.3.3.1.3 章节
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdxInc_last_significant_coeff_flag(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t ctxBlockCat);                          // 参考标准的 9.3.3.1.3 章节
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdxInc_coeff_abs_level_minus1(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx, uint8_t ctxBlockCat);               // 参考标准的 9.3.3.1.3 章节

static inline void H264ReadAEGetNeighbouringBlock_coded_block_flag_0_6_10(PVIDEOH264DECODEINFO pDecodeInfo, HANSIZE mbAddrN[2], uint8_t transBlockN[2]);
static inline void H264ReadAEGetNeighbouringBlock_coded_block_flag_1_2(PVIDEOH264DECODEINFO pDecodeInfo, HANSIZE mbAddrN[2], uint8_t transBlockN[2]);
static inline void H264ReadAEGetNeighbouringBlock_coded_block_flag_3(PVIDEOH264DECODEINFO pDecodeInfo, HANSIZE mbAddrN[2], uint8_t transBlockN[2]);
static inline void H264ReadAEGetNeighbouringBlock_coded_block_flag_4(PVIDEOH264DECODEINFO pDecodeInfo, HANSIZE mbAddrN[2], uint8_t transBlockN[2]);
static inline void H264ReadAEGetNeighbouringBlock_coded_block_flag_5(PVIDEOH264DECODEINFO pDecodeInfo, HANSIZE mbAddrN[2], uint8_t transBlockN[2]);
static inline void H264ReadAEGetNeighbouringBlock_coded_block_flag_7_8(PVIDEOH264DECODEINFO pDecodeInfo, HANSIZE mbAddrN[2], uint8_t transBlockN[2]);
static inline void H264ReadAEGetNeighbouringBlock_coded_block_flag_9(PVIDEOH264DECODEINFO pDecodeInfo, HANSIZE mbAddrN[2], uint8_t transBlockN[2]);
static inline void H264ReadAEGetNeighbouringBlock_coded_block_flag_11_12(PVIDEOH264DECODEINFO pDecodeInfo, HANSIZE mbAddrN[2], uint8_t transBlockN[2]);
static inline void H264ReadAEGetNeighbouringBlock_coded_block_flag_13(PVIDEOH264DECODEINFO pDecodeInfo, HANSIZE mbAddrN[2], uint8_t transBlockN[2]);

static uint8_t H264ReadAEDecodeBin(PVIDEOH264DECODEINFO pDecodeInfo, VIDEOH264CABACCTXIDX ctxIdx);
static uint8_t H264ReadAEDecodeDecision(PVIDEOH264DECODEINFO pDecodeInfo, VIDEOH264CABACCTXIDX ctxIdx); // 参考标准的 9.3.3.2.1 章节
static void H264ReadAERenormD(PVIDEOH264DECODEINFO pDecodeInfo);                                        // 参考标准的 9.3.3.2.2 章节
static uint8_t H264ReadAEDecodeBypass(PVIDEOH264DECODEINFO pDecodeInfo);                                // 参考标准的 9.3.3.2.3 章节
static uint8_t H264ReadAEDecodeTerminate(PVIDEOH264DECODEINFO pDecodeInfo);                             // 参考标准的 9.3.3.2.4 章节
#endif

static void DecodeH264_residual_block_cabac(PVIDEOH264DECODEINFO pDecodeInfo);
static inline VIDEOH264COEFFLEVEL DecodeH264CalcCoeffLevel(PVIDEOH264DECODEINFO pDecodeInfo, uint32_t coeff_abs_level_minus1, uint8_t coeff_sign_flag);

static inline PVIDEOH264MACROBLOCK GetCurrentMacroBlock(PVIDEOH264DECODEINFO pDecodeInfo);
static VIDEOH264FRAMEFIELDMODE GetMacroBlockFrameFiledMode(PVIDEOH264DECODEINFO pDecodeInfo, PVIDEOH264MACROBLOCK pMacroBlock);
static inline uint8_t GetDecodingBlockIdx(PVIDEOH264DECODEINFO pDecodeInfo);

static const VIDEOH264CABACCONTEXMN sg_pCABACContexTableMN[VIDEO_H264_CABAC_CONTEXT_CABAC_INIT_IDC_CNT][VIDEO_H264_CABAC_CONTEXT_MN][VIDEO_H264_CABAC_CONTEXT_SIZE] = {
    /* 以下内容摘录自标准 9.3.1.1 章节的表格 Table 9-12 到 9-33
     * 数组维度按 [I帧/cabac_init_idc][m/n参数][上下文总数量] 的顺序排列
     * 通过指定索引号将表格中连续的参数按组初始化
     * 未初始化的参数由于在解码过程中不会用到，所以不用关注
     */
    [VIDEO_H264_CABAC_CONTEXT_CABAC_INIT_IDC_I_SLICE] = {
        [VIDEO_H264_CABAC_CONTEXT_M] = {
            /* Table 9-12 */ [0] = 20, 2, 3, 20, 2, 3, -28, -23, -6, -1, 7,
            /* Table 9-16 */ [399] = 31, 31, 25,
            /* Table 9-17 */ [60] = 0, 0, 0, 0, -9, 4, 0, -7, 13, 3,
            /* Table 9-18 */ [70] = 0, 1, 0, -17, -13, 0, -7, -21, -27, -31, -24, -18, -27, -21, -30, -17, -12, -16,
                             [88] = -11, -12, -2, -15, -13, -3, -8, -10, -30, -1, -6, -7, -20, -4, -5, -7, -22,
            /* Table 9-19 */ [105] = -7, -11, -3, -5, -4, -4, -12, -7, -7, 8, 5, -2, 1, 0, -2, 1, 7, 10, 0, 11, 1, 0, 5, 31, 1, 7, 28, 16, 14, -13, -15,
                             [136] = -13, -13, -12, -10, -16, -10, -7, -13, -19, 1, 0, -5, 18, -8, -15, 0, -4, 2, -11, -3, 15, -13, 0, 0, 21, -15, 9, 16, 0, 12,
            /* Table 9-20 */ [166] = 24, 15, 8, 13, 15, 13, 10, 12, 6, 20, 15, 4, 1, 0, 7, 12, 11, 15, 11, 13, 16, 12, 10, 30, 18, 10, 17, 17, 0, 26, 22,
                             [197] = 26, 30, 28, 33, 37, 33, 40, 38, 33, 40, 41, 38, 41, 30, 27, 26, 37, 35, 38, 38, 37, 38, 42, 35, 39, 14, 27, 21, 12, 2,
            /* Table 9-21 */ [227] = -3, -6, -5, -3, -2, 0, 1, -2, -1, -9, -5, -5, -3, -2, 0, -16, -8, -10, -6, -10, -12, -15, -10, -6, -4,
                             [252] = -12, -8, -7, -9, -17, -11, -20, -11, -6, -4, -13, -13, -11, -19, -8, -5, -4, -2, -3, -13, -10, -12, -13, -14,
            /* Table 9-22 */ [277] = -6, -6, -8, 0, -1, 0, -2, -2, -5, -3, -4, -9, -1, 0, 3, 10, -7, 15, 14, 16, 12, 1, 20, 18, 5, 1, 10, 17, 9, -12, -11,
                             [308] = -16, -7, -8, -7, -9, -13, 4, -3, -3, -6, 10, -1, -1, -7, -14, 2, 0, -5, 0, -11, 1, 0, -14, 3, 4, -1, -13, 11, 5, 12,
            /* Table 9-23 */ [338] = 15, 6, 7, 12, 18, 13, 13, 15, 12, 13, 15, 14, 14, 17, 17, 24, 21, 25, 31, 22, 19, 14, 10, 7, -2, -4, -3, 9, -12, 36, 36,
                             [369] = 32, 37, 44, 34, 34, 40, 33, 35, 33, 38, 33, 23, 13, 29, 26, 22, 31, 35, 34, 34, 36, 34, 32, 35, 34, 39, 30, 34, 29, 19,
            /* Table 9-24 */ [402] = -17, -20, -18, -11, -15, -14, -26, -15, -14, 0, -14, -24, -23, -24, -11, 23, 26, 40, 49, 44, 45, 44, 33, 19, -3, -1, 1, 1, 0,
                             [431] = -2, 0, 1, 0, -9, -14, -13, -15, -12, -18, -10, -9, -14, -10, -10, -10, -5, -9, -5, 2, 21, 24, 28, 28, 29, 29, 35, 29, 14,
            /* Table 9-25 */ [460] = -17, -12, -16, -11, -12, -2, -15, -13, -3, -8, -10, -30,
                             [472] = -17, -12, -16, -11, -12, -2, -15, -13, -3, -8, -10, -30,
            /* Table 9-26 */ [484] = -7, -11, -3, -5, -4, -4, -12, -7, -7, 8, 5, -2, 1, 0, -2,
                             [528] = -7, -11, -3, -5, -4, -4, -12, -7, -7, 8, 5, -2, 1, 0, -2,
                             [499] = 1, 7, 10, 0, 11, 1, 0, 5, 31, 1, 7, 28, 16, 14, -13, -15, -13, -13, -12, -10, -16, -10, -7, -13, -19, 1, 0, -5, 18,
                             [543] = 1, 7, 10, 0, 11, 1, 0, 5, 31, 1, 7, 28, 16, 14, -13, -15, -13, -13, -12, -10, -16, -10, -7, -13, -19, 1, 0, -5, 18,
            /* Table 9-27 */ [572] = 24, 15, 8, 13, 15, 13, 10, 12, 6, 20, 15, 4, 1, 0, 7, 12, 11, 15, 11, 13, 16, 12, 10, 30, 18, 10, 17, 17, 0, 26, 22,
                             [616] = 24, 15, 8, 13, 15, 13, 10, 12, 6, 20, 15, 4, 1, 0, 7, 12, 11, 15, 11, 13, 16, 12, 10, 30, 18, 10, 17, 17, 0, 26, 22,
                             [603] = 26, 30, 28, 33, 37, 33, 40, 38, 33, 40, 41, 38, 41,
                             [647] = 26, 30, 28, 33, 37, 33, 40, 38, 33, 40, 41, 38, 41,
            /* Table 9-28 */ [660] = -17, -20, -18, -11, -15, -14, -26, -15, -14, 0, -14, -24, -23, -24, -11, -14, -13, -15, -12, -18, -10, -9, -14, -10, -10, -10, -5, -9, -5,
                             [689] = 2, 23, 26, 40, 49, 44, 45, 44, 33, 19, 21, 24, 28, 28, 29, 29, 35, 29, 14, -3, -1, 1, 1, 0, -2, 0, 1, 0, -9,
            /* Table 9-29 */ [718] = -17, -20, -18, -11, -15, -14, -26, -15, -14, 0, -14, -24, -23, -24, -11, -14, -13, -15, -12, -18, -10, -9, -14, -10, -10, -10, -5, -9, -5,
                             [747] = 2, 23, 26, 40, 49, 44, 45, 44, 33, 19, 21, 24, 28, 28, 29, 29, 35, 29, 14, -3, -1, 1, 1, 0, -2, 0, 1, 0, -9,
            /* Table 9-30 */ [776] = -6, -6, -8, 0, -1, 0, -2, -2, -5, -3, -4, -9, -1, 0, 3, 10, -7, 15, 14, 16, 12, 1, 20, 18, 5, 1, 10, 17, 9, -12, -11,
                             [820] = -6, -6, -8, 0, -1, 0, -2, -2, -5, -3, -4, -9, -1, 0, 3, 10, -7, 15, 14, 16, 12, 1, 20, 18, 5, 1, 10, 17, 9, -12, -11,
                             [807] = -16, -7, -8, -7, -9, -13, 4, -3, -3, -6, 10, -1, -1,
                             [851] = -16, -7, -8, -7, -9, -13, 4, -3, -3, -6, 10, -1, -1,
            /* Table 9-31 */ [864] = 15, 6, 7, 12, 18, 13, 13, 15, 12, 13, 15, 14, 14, 17, 17, 24, 21, 25, 31, 22, 19, 14, 10, 7, -2, -4, -3, 9, -12, 36, 36,
                             [908] = 15, 6, 7, 12, 18, 13, 13, 15, 12, 13, 15, 14, 14, 17, 17, 24, 21, 25, 31, 22, 19, 14, 10, 7, -2, -4, -3, 9, -12, 36, 36,
                             [895] = 32, 37, 44, 34, 34, 40, 33, 35, 33, 38, 33, 23, 13,
                             [939] = 32, 37, 44, 34, 34, 40, 33, 35, 33, 38, 33, 23, 13,
            /* Table 9-32 */ [952] = -3, -6, -5, -3, -2, 0, 1, -2, -1, -9, -5, -5, -3, -2, 0, -16, -8, -10, -6, -10, -12, -15, -10, -6, -4, -12, -8, -7, -9, -17,
                             [982] = -3, -6, -5, -3, -2, 0, 1, -2, -1, -9, -5, -5, -3, -2, 0, -16, -8, -10, -6, -10, -12, -15, -10, -6, -4, -12, -8, -7, -9, -17,
            /* Table 9-33 */ [1012] = -3, -8, -10, -30, -3, -8,
                             [1018] = -10, -30, -3, -8, -10, -30,
        },
        [VIDEO_H264_CABAC_CONTEXT_N] = {
            /* Table 9-12 */ [0] = -15, 54, 74, -15, 54, 74, 127, 104, 53, 54, 51,
            /* Table 9-16 */ [399] = 21, 31, 50,
            /* Table 9-17 */ [60] = 41, 63, 63, 63, 83, 86, 97, 72, 41, 62,
            /* Table 9-18 */ [70] = 11, 55, 69, 127, 102, 82, 74, 107, 127, 127, 127, 95, 127, 114, 127, 123, 115, 122,
                             [88] = 115, 63, 68, 84, 104, 70, 93, 90, 127, 74, 97, 91, 127, 56, 82, 76, 125,
            /* Table 9-19 */ [105] = 93, 87, 77, 71, 63, 68, 84, 62, 65, 61, 56, 66, 64, 61, 78, 50, 52, 35, 44, 38, 45, 46, 44, 17, 51, 50, 19, 33, 62, 108, 100,
                             [136] = 101, 91, 94, 88, 84, 86, 83, 87, 94, 70, 72, 74, 59, 102, 100, 95, 75, 72, 75, 71, 46, 69, 62, 65, 37, 72, 57, 54, 62, 72,
            /* Table 9-20 */ [166] = 0, 9, 25, 18, 9, 19, 37, 18, 29, 33, 30, 45, 58, 62, 61, 38, 45, 39, 42, 44, 45, 41, 49, 34, 42, 55, 51, 46, 89, -19, -17,
                             [197] = -17, -25, -20, -23, -27, -23, -28, -17, -11, -15, -6, 1, 17, -6, 3, 22, -16, -4, -8, -3, 3, 5, 0, 16, 22, 48, 37, 60, 68, 97,
            /* Table 9-21 */ [227] = 71, 42, 50, 54, 62, 58, 63, 72, 74, 91, 67, 27, 39, 44, 46, 64, 68, 78, 77, 86, 92, 55, 60, 62, 65,
                             [252] = 73, 76, 80, 88, 110, 97, 84, 79, 73, 74, 86, 96, 97, 117, 78, 33, 48, 53, 62, 71, 79, 86, 90, 97,
            /* Table 9-22 */ [277] = 93, 84, 79, 66, 71, 62, 60, 59, 75, 62, 58, 66, 79, 71, 68, 44, 62, 36, 40, 27, 29, 44, 36, 32, 42, 48, 62, 46, 64, 104, 97,
                             [308] = 96, 88, 85, 85, 85, 88, 66, 77, 76, 76, 58, 76, 83, 99, 95, 95, 76, 74, 70, 75, 68, 65, 73, 62, 62, 68, 75, 55, 64, 70,
            /* Table 9-23 */ [338] = 6, 19, 16, 14, 13, 11, 15, 16, 23, 23, 20, 26, 44, 40, 47, 17, 21, 22, 27, 29, 35, 50, 57, 63, 77, 82, 94, 69, 109, -35, -34,
                             [369] = -26, -30, -32, -18, -15, -15, -7, -5, 0, 2, 13, 35, 58, -3, 0, 30, -7, -15, -3, 3, -1, 5, 11, 5, 12, 11, 29, 26, 39, 66,
            /* Table 9-24 */ [402] = 120, 112, 114, 85, 92, 89, 71, 81, 80, 68, 70, 56, 68, 50, 74, -13, -13, -15, -14, 3, 6, 34, 54, 82, 75, 23, 34, 43, 54,
                             [431] = 55, 61, 64, 68, 92, 106, 97, 90, 90, 88, 73, 79, 86, 73, 70, 69, 66, 64, 58, 59, -10, -11, -8, -1, 3, 9, 20, 36, 67,
            /* Table 9-25 */ [460] = 123, 115, 122, 115, 63, 68, 84, 104, 70, 93, 90, 127,
                             [472] = 123, 115, 122, 115, 63, 68, 84, 104, 70, 93, 90, 127,
            /* Table 9-26 */ [484] = 93, 87, 77, 71, 63, 68, 84, 62, 65, 61, 56, 66, 64, 61, 78,
                             [528] = 93, 87, 77, 71, 63, 68, 84, 62, 65, 61, 56, 66, 64, 61, 78,
                             [499] = 50, 52, 35, 44, 38, 45, 46, 44, 17, 51, 50, 19, 33, 62, 108, 100, 101, 91, 94, 88, 84, 86, 83, 87, 94, 70, 72, 74, 59,
                             [543] = 50, 52, 35, 44, 38, 45, 46, 44, 17, 51, 50, 19, 33, 62, 108, 100, 101, 91, 94, 88, 84, 86, 83, 87, 94, 70, 72, 74, 59,
            /* Table 9-27 */ [572] = 0, 9, 25, 18, 9, 19, 37, 18, 29, 33, 30, 45, 58, 62, 61, 38, 45, 39, 42, 44, 45, 41, 49, 34, 42, 55, 51, 46, 89, -19, -17,
                             [616] = 0, 9, 25, 18, 9, 19, 37, 18, 29, 33, 30, 45, 58, 62, 61, 38, 45, 39, 42, 44, 45, 41, 49, 34, 42, 55, 51, 46, 89, -19, -17,
                             [603] = -17, -25, -20, -23, -27, -23, -28, -17, -11, -15, -6, 1, 17,
                             [647] = -17, -25, -20, -23, -27, -23, -28, -17, -11, -15, -6, 1, 17,
            /* Table 9-28 */ [660] = 120, 112, 114, 85, 92, 89, 71, 81, 80, 68, 70, 56, 68, 50, 74, 106, 97, 90, 90, 88, 73, 79, 86, 73, 70, 69, 66, 64, 58,
                             [689] = 59, -13, -13, -15, -14, 3, 6, 34, 54, 82, -10, -11, -8, -1, 3, 9, 20, 36, 67, 75, 23, 34, 43, 54, 55, 61, 64, 68, 92,
            /* Table 9-29 */ [718] = 120, 112, 114, 85, 92, 89, 71, 81, 80, 68, 70, 56, 68, 50, 74, 106, 97, 90, 90, 88, 73, 79, 86, 73, 70, 69, 66, 64, 58,
                             [747] = 59, -13, -13, -15, -14, 3, 6, 34, 54, 82, -10, -11, -8, -1, 3, 9, 20, 36, 67, 75, 23, 34, 43, 54, 55, 61, 64, 68, 92,
            /* Table 9-30 */ [776] = 93, 84, 79, 66, 71, 62, 60, 59, 75, 62, 58, 66, 79, 71, 68, 44, 62, 36, 40, 27, 29, 44, 36, 32, 42, 48, 62, 46, 64, 104, 97,
                             [820] = 93, 84, 79, 66, 71, 62, 60, 59, 75, 62, 58, 66, 79, 71, 68, 44, 62, 36, 40, 27, 29, 44, 36, 32, 42, 48, 62, 46, 64, 104, 97,
                             [807] = 96, 88, 85, 85, 85, 88, 66, 77, 76, 76, 58, 76, 83,
                             [851] = 96, 88, 85, 85, 85, 88, 66, 77, 76, 76, 58, 76, 83,
            /* Table 9-31 */ [864] = 6, 19, 16, 14, 13, 11, 15, 16, 23, 23, 20, 26, 44, 40, 47, 17, 21, 22, 27, 29, 35, 50, 57, 63, 77, 82, 94, 69, 109, -35, -34,
                             [908] = 6, 19, 16, 14, 13, 11, 15, 16, 23, 23, 20, 26, 44, 40, 47, 17, 21, 22, 27, 29, 35, 50, 57, 63, 77, 82, 94, 69, 109, -35, -34,
                             [895] = -26, -30, -32, -18, -15, -15, -7, -5, 0, 2, 13, 35, 58,
                             [939] = -26, -30, -32, -18, -15, -15, -7, -5, 0, 2, 13, 35, 58,
            /* Table 9-32 */ [952] = 71, 42, 50, 54, 62, 58, 63, 72, 74, 91, 67, 27, 39, 44, 46, 64, 68, 78, 77, 86, 92, 55, 60, 62, 65, 73, 76, 80, 88, 110,
                             [982] = 71, 42, 50, 54, 62, 58, 63, 72, 74, 91, 67, 27, 39, 44, 46, 64, 68, 78, 77, 86, 92, 55, 60, 62, 65, 73, 76, 80, 88, 110,
            /* Table 9-33 */ [1012] = 70, 93, 90, 127, 70, 93,
                             [1018] = 90, 127, 70, 93, 90, 127,
        },
    },
    [VIDEO_H264_CABAC_CONTEXT_CABAC_INIT_IDC_0] = {
        [VIDEO_H264_CABAC_CONTEXT_M] = {
            /* Table 9-12 */ [0] = 20, 2, 3, 20, 2, 3, -28, -23, -6, -1, 7,
            /* Table 9-13 */ [11] = 23, 23, 21, 1, 0, -37, 5, -13, -11, 1, 12, -4, 17,
            /* Table 9-14 */ [24] = 18, 9, 29, 26, 16, 9, -46, -20, 1, -13, -11, 1, -6, -17, -6, 9,
            /* Table 9-15 */ [40] = -3, -6, -11, 6, 7, -5, 2, 0, -3, -10, 5, 4, -3, 0,
            /* Table 9-16 */ [54] = -7, -5, -4, -5, -7, 1,
                             [399] = 12, 11, 14,
            /* Table 9-17 */ [60] = 0, 0, 0, 0, -9, 4, 0, -7, 13, 3,
            /* Table 9-18 */ [70] = 0, -4, -3, -27, -28, -25, -23, -28, -20, -16, -22, -21, -18, -13, -29, -7, -5, -7,
                             [88] = -13, -3, -1, -1, -9, -3, -9, -8, -23, 5, 6, 6, 6, -1, 0, -4, -8,
            /* Table 9-19 */ [105] = -2, -6, -1, -7, 2, 5, -3, 1, 6, -4, 1, -4, 0, 2, -2, 11, 4, 1, 11, 18, 12, 13, 13, -10, -7, -2, 13, 9, -7, 9, 2,
                             [136] = 5, -2, 0, 0, -13, -5, -1, 4, -6, 4, 14, 4, 13, 3, 1, 9, 7, 16, 5, 4, 11, -5, -1, 0, 22, 5, 14, -1, 0, 9,
            /* Table 9-20 */ [166] = 11, 2, 3, 0, 0, 2, 2, 0, 4, 2, 6, 0, 3, 2, 4, 6, 6, 7, 6, 6, 11, 14, 8, -1, 7, -3, 15, 22, -1, 25, 30,
                             [197] = 28, 28, 32, 34, 30, 30, 32, 31, 26, 26, 37, 28, 17, 1, 5, 9, 16, 18, 18, 22, 24, 23, 18, 20, 11, 9, 9, -1, -2, -9,
            /* Table 9-21 */ [227] = -6, -2, 0, 0, -3, -2, -4, -4, -8, -17, -9, 3, 0, 0, 0, -6, -7, -12, -11, -30, 1, -3, -1, 1, 2,
                             [252] = -6, 0, 0, -3, -10, 0, -4, 5, 7, 1, -2, -3, -3, -11, 0, 8, 10, 14, 13, 2, 0, -3, -6, -8,
            /* Table 9-22 */ [277] = -13, -16, -10, -21, -18, -14, -22, -21, -18, -21, -23, -26, -10, -12, -5, -9, -22, -5, 9, -4, -10, -1, 7, 9, 5, 12, 15, 18, 17, 10, 7,
                             [308] = -1, 7, 8, 9, 6, 2, 13, 10, 6, 5, 13, 4, 6, -2, -2, 6, 10, 9, 12, 3, 14, 10, -3, 13, 17, 7, 7, 13, 10, 26,
            /* Table 9-23 */ [338] = 14, 11, 9, 18, 21, 23, 32, 32, 34, 39, 42, 41, 46, 38, 21, 45, 53, 48, 65, 43, 39, 30, 18, 20, 0, -14, -5, -19, -35, 27, 28,
                             [369] = 31, 27, 34, 30, 24, 33, 22, 26, 21, 26, 23, 16, 14, 8, 6, 17, 21, 23, 26, 27, 28, 28, 23, 24, 28, 23, 19, 22, 22, 11,
            /* Table 9-24 */ [402] = -4, -7, -5, -9, -8, -10, -19, -12, -16, -15, -20, -19, -16, -22, -20, 9, 26, 33, 39, 41, 45, 49, 45, 36, -6, -7, -7, -8, -5,
                             [431] = -12, -6, -5, -8, -8, -5, -6, -10, -7, -17, -18, -4, -10, -9, -9, -1, -8, -14, 0, 2, 21, 33, 39, 46, 51, 60, 61, 55, 42,
            /* Table 9-25 */ [460] = -7, -5, -7, -13, -3, -1, -1, -9, -3, -9, -8, -23,
                             [472] = -7, -5, -7, -13, -3, -1, -1, -9, -3, -9, -8, -23,
            /* Table 9-26 */ [484] = -2, -6, -1, -7, 2, 5, -3, 1, 6, -4, 1, -4, 0, 2, -2,
                             [528] = -2, -6, -1, -7, 2, 5, -3, 1, 6, -4, 1, -4, 0, 2, -2,
                             [499] = 11, 4, 1, 11, 18, 12, 13, 13, -10, -7, -2, 13, 9, -7, 9, 2, 5, -2, 0, 0, -13, -5, -1, 4, -6, 4, 14, 4, 13,
                             [543] = 11, 4, 1, 11, 18, 12, 13, 13, -10, -7, -2, 13, 9, -7, 9, 2, 5, -2, 0, 0, -13, -5, -1, 4, -6, 4, 14, 4, 13,
            /* Table 9-27 */ [572] = 11, 2, 3, 0, 0, 2, 2, 0, 4, 2, 6, 0, 3, 2, 4, 6, 6, 7, 6, 6, 11, 14, 8, -1, 7, -3, 15, 22, -1, 25, 30,
                             [616] = 11, 2, 3, 0, 0, 2, 2, 0, 4, 2, 6, 0, 3, 2, 4, 6, 6, 7, 6, 6, 11, 14, 8, -1, 7, -3, 15, 22, -1, 25, 30,
                             [603] = 28, 28, 32, 34, 30, 30, 32, 31, 26, 26, 37, 28, 17,
                             [647] = 28, 28, 32, 34, 30, 30, 32, 31, 26, 26, 37, 28, 17,
            /* Table 9-28 */ [660] = -4, -7, -5, -9, -8, -10, -19, -12, -16, -15, -20, -19, -16, -22, -20, -5, -6, -10, -7, -17, -18, -4, -10, -9, -9, -1, -8, -14, 0,
                             [689] = 2, 9, 26, 33, 39, 41, 45, 49, 45, 36, 21, 33, 39, 46, 51, 60, 61, 55, 42, -6, -7, -7, -8, -5, -12, -6, -5, -8, -8,
            /* Table 9-29 */ [718] = -4, -7, -5, -9, -8, -10, -19, -12, -16, -15, -20, -19, -16, -22, -20, -5, -6, -10, -7, -17, -18, -4, -10, -9, -9, -1, -8, -14, 0,
                             [747] = 2, 9, 26, 33, 39, 41, 45, 49, 45, 36, 21, 33, 39, 46, 51, 60, 61, 55, 42, -6, -7, -7, -8, -5, -12, -6, -5, -8, -8,
            /* Table 9-30 */ [776] = -13, -16, -10, -21, -18, -14, -22, -21, -18, -21, -23, -26, -10, -12, -5, -9, -22, -5, 9, -4, -10, -1, 7, 9, 5, 12, 15, 18, 17, 10, 7,
                             [820] = -13, -16, -10, -21, -18, -14, -22, -21, -18, -21, -23, -26, -10, -12, -5, -9, -22, -5, 9, -4, -10, -1, 7, 9, 5, 12, 15, 18, 17, 10, 7,
                             [807] = -1, 7, 8, 9, 6, 2, 13, 10, 6, 5, 13, 4, 6,
                             [851] = -1, 7, 8, 9, 6, 2, 13, 10, 6, 5, 13, 4, 6,
            /* Table 9-31 */ [864] = 14, 11, 9, 18, 21, 23, 32, 32, 34, 39, 42, 41, 46, 38, 21, 45, 53, 48, 65, 43, 39, 30, 18, 20, 0, -14, -5, -19, -35, 27, 28,
                             [908] = 14, 11, 9, 18, 21, 23, 32, 32, 34, 39, 42, 41, 46, 38, 21, 45, 53, 48, 65, 43, 39, 30, 18, 20, 0, -14, -5, -19, -35, 27, 28,
                             [895] = 31, 27, 34, 30, 24, 33, 22, 26, 21, 26, 23, 16, 14,
                             [939] = 31, 27, 34, 30, 24, 33, 22, 26, 21, 26, 23, 16, 14,
            /* Table 9-32 */ [952] = -6, -2, 0, 0, -3, -2, -4, -4, -8, -17, -9, 3, 0, 0, 0, -6, -7, -12, -11, -30, 1, -3, -1, 1, 2, -6, 0, 0, -3, -10,
                             [982] = -6, -2, 0, 0, -3, -2, -4, -4, -8, -17, -9, 3, 0, 0, 0, -6, -7, -12, -11, -30, 1, -3, -1, 1, 2, -6, 0, 0, -3, -10,
            /* Table 9-33 */ [1012] = -3, -9, -8, -23, -3, -9,
                             [1018] = -8, -23, -3, -9, -8, -23,
        },
        [VIDEO_H264_CABAC_CONTEXT_N] = {
            /* Table 9-12 */ [0] = -15, 54, 74, -15, 54, 74, 127, 104, 53, 54, 51,
            /* Table 9-13 */ [11] = 33, 2, 0, 9, 49, 118, 57, 78, 65, 62, 49, 73, 50,
            /* Table 9-14 */ [24] = 64, 43, 0, 67, 90, 104, 127, 104, 67, 78, 65, 62, 86, 95, 61, 45,
            /* Table 9-15 */ [40] = 69, 81, 96, 55, 67, 86, 88, 58, 76, 94, 54, 69, 81, 88,
            /* Table 9-16 */ [54] = 67, 74, 74, 80, 72, 58,
                             [399] = 40, 51, 59,
            /* Table 9-17 */ [60] = 41, 63, 63, 63, 83, 86, 97, 72, 41, 62,
            /* Table 9-18 */ [70] = 45, 78, 96, 126, 98, 101, 67, 82, 94, 83, 110, 91, 102, 93, 127, 92, 89, 96,
                             [88] = 108, 46, 65, 57, 93, 74, 92, 87, 126, 54, 60, 59, 69, 48, 68, 69, 88,
            /* Table 9-19 */ [105] = 85, 78, 75, 77, 54, 50, 68, 50, 42, 81, 63, 70, 67, 57, 76, 35, 64, 61, 35, 25, 24, 29, 36, 93, 73, 73, 46, 49, 100, 53, 53,
                             [136] = 53, 61, 56, 56, 63, 60, 62, 57, 69, 57, 39, 51, 68, 64, 61, 63, 50, 39, 44, 52, 48, 60, 59, 59, 33, 44, 43, 78, 60, 69,
            /* Table 9-20 */ [166] = 28, 40, 44, 49, 46, 44, 51, 47, 39, 62, 46, 54, 54, 58, 63, 51, 57, 53, 52, 55, 45, 36, 53, 82, 55, 78, 46, 31, 84, 7, -7,
                             [197] = 3, 4, 0, -1, 6, 6, 9, 19, 27, 30, 20, 34, 70, 67, 59, 67, 30, 32, 35, 29, 31, 38, 43, 41, 63, 59, 64, 94, 89, 108,
            /* Table 9-21 */ [227] = 76, 44, 45, 52, 64, 59, 70, 75, 82, 102, 77, 24, 42, 48, 55, 59, 71, 83, 87, 119, 58, 29, 36, 38, 43,
                             [252] = 55, 58, 64, 74, 90, 70, 29, 31, 42, 59, 58, 72, 81, 97, 58, 5, 14, 18, 27, 40, 58, 70, 79, 85,
            /* Table 9-22 */ [277] = 106, 106, 87, 114, 110, 98, 110, 106, 103, 107, 108, 112, 96, 95, 91, 93, 94, 86, 67, 80, 85, 70, 60, 58, 61, 50, 50, 49, 54, 41, 46,
                             [308] = 51, 49, 52, 41, 47, 55, 41, 44, 50, 53, 49, 63, 64, 69, 59, 70, 44, 31, 43, 53, 34, 38, 52, 40, 32, 44, 38, 50, 57, 43,
            /* Table 9-23 */ [338] = 11, 14, 11, 11, 9, -2, -15, -15, -21, -23, -33, -31, -28, -12, 29, -24, -45, -26, -43, -19, -10, 9, 26, 27, 57, 82, 75, 97, 125, 0, 0,
                             [369] = -4, 6, 8, 10, 22, 19, 32, 31, 41, 44, 47, 65, 71, 60, 63, 65, 24, 20, 23, 32, 23, 24, 40, 32, 29, 42, 57, 53, 61, 86,
            /* Table 9-24 */ [402] = 79, 71, 69, 70, 66, 68, 73, 69, 70, 67, 62, 70, 66, 65, 63, -2, -9, -9, -7, -2, 3, 9, 27, 59, 66, 35, 42, 45, 48,
                             [431] = 56, 60, 62, 66, 76, 85, 81, 77, 81, 80, 73, 74, 83, 71, 67, 61, 66, 66, 59, 59, -13, -14, -7, -2, 2, 6, 17, 34, 62,
            /* Table 9-25 */ [460] = 92, 89, 96, 108, 46, 65, 57, 93, 74, 92, 87, 126,
                             [472] = 92, 89, 96, 108, 46, 65, 57, 93, 74, 92, 87, 126,
            /* Table 9-26 */ [484] = 85, 78, 75, 77, 54, 50, 68, 50, 42, 81, 63, 70, 67, 57, 76,
                             [528] = 85, 78, 75, 77, 54, 50, 68, 50, 42, 81, 63, 70, 67, 57, 76,
                             [499] = 35, 64, 61, 35, 25, 24, 29, 36, 93, 73, 73, 46, 49, 100, 53, 53, 53, 61, 56, 56, 63, 60, 62, 57, 69, 57, 39, 51, 68,
                             [543] = 35, 64, 61, 35, 25, 24, 29, 36, 93, 73, 73, 46, 49, 100, 53, 53, 53, 61, 56, 56, 63, 60, 62, 57, 69, 57, 39, 51, 68,
            /* Table 9-27 */ [572] = 28, 40, 44, 49, 46, 44, 51, 47, 39, 62, 46, 54, 54, 58, 63, 51, 57, 53, 52, 55, 45, 36, 53, 82, 55, 78, 46, 31, 84, 7, -7,
                             [616] = 28, 40, 44, 49, 46, 44, 51, 47, 39, 62, 46, 54, 54, 58, 63, 51, 57, 53, 52, 55, 45, 36, 53, 82, 55, 78, 46, 31, 84, 7, -7,
                             [603] = 3, 4, 0, -1, 6, 6, 9, 19, 27, 30, 20, 34, 70,
                             [647] = 3, 4, 0, -1, 6, 6, 9, 19, 27, 30, 20, 34, 70,
            /* Table 9-28 */ [660] = 79, 71, 69, 70, 66, 68, 73, 69, 70, 67, 62, 70, 66, 65, 63, 85, 81, 77, 81, 80, 73, 74, 83, 71, 67, 61, 66, 66, 59,
                             [689] = 59, -2, -9, -9, -7, -2, 3, 9, 27, 59, -13, -14, -7, -2, 2, 6, 17, 34, 62, 66, 35, 42, 45, 48, 56, 60, 62, 66, 76,
            /* Table 9-29 */ [718] = 79, 71, 69, 70, 66, 68, 73, 69, 70, 67, 62, 70, 66, 65, 63, 85, 81, 77, 81, 80, 73, 74, 83, 71, 67, 61, 66, 66, 59,
                             [747] = 59, -2, -9, -9, -7, -2, 3, 9, 27, 59, -13, -14, -7, -2, 2, 6, 17, 34, 62, 66, 35, 42, 45, 48, 56, 60, 62, 66, 76,
            /* Table 9-30 */ [776] = 106, 106, 87, 114, 110, 98, 110, 106, 103, 107, 108, 112, 96, 95, 91, 93, 94, 86, 67, 80, 85, 70, 60, 58, 61, 50, 50, 49, 54, 41, 46,
                             [820] = 106, 106, 87, 114, 110, 98, 110, 106, 103, 107, 108, 112, 96, 95, 91, 93, 94, 86, 67, 80, 85, 70, 60, 58, 61, 50, 50, 49, 54, 41, 46,
                             [807] = 51, 49, 52, 41, 47, 55, 41, 44, 50, 53, 49, 63, 64,
                             [851] = 51, 49, 52, 41, 47, 55, 41, 44, 50, 53, 49, 63, 64,
            /* Table 9-31 */ [864] = 11, 14, 11, 11, 9, -2, -15, -15, -21, -23, -33, -31, -28, -12, 29, -24, -45, -26, -43, -19, -10, 9, 26, 27, 57, 82, 75, 97, 125, 0, 0,
                             [908] = 11, 14, 11, 11, 9, -2, -15, -15, -21, -23, -33, -31, -28, -12, 29, -24, -45, -26, -43, -19, -10, 9, 26, 27, 57, 82, 75, 97, 125, 0, 0,
                             [895] = -4, 6, 8, 10, 22, 19, 32, 31, 41, 44, 47, 65, 71,
                             [939] = -4, 6, 8, 10, 22, 19, 32, 31, 41, 44, 47, 65, 71,
            /* Table 9-32 */ [952] = 76, 44, 45, 52, 64, 59, 70, 75, 82, 102, 77, 24, 42, 48, 55, 59, 71, 83, 87, 119, 58, 29, 36, 38, 43, 55, 58, 64, 74, 90,
                             [982] = 76, 44, 45, 52, 64, 59, 70, 75, 82, 102, 77, 24, 42, 48, 55, 59, 71, 83, 87, 119, 58, 29, 36, 38, 43, 55, 58, 64, 74, 90,
            /* Table 9-33 */ [1012] = 74, 92, 87, 126, 74, 92,
                             [1018] = 87, 126, 74, 92, 87, 126,
        },
    },
    [VIDEO_H264_CABAC_CONTEXT_CABAC_INIT_IDC_1] = {
        [VIDEO_H264_CABAC_CONTEXT_M] = {
            /* Table 9-12 */ [0] = 20, 2, 3, 20, 2, 3, -28, -23, -6, -1, 7,
            /* Table 9-13 */ [11] = 22, 34, 16, -2, 4, -29, 2, -6, -13, 5, 9, -3, 10,
            /* Table 9-14 */ [24] = 26, 19, 40, 57, 41, 26, -45, -15, -4, -6, -13, 5, 6, -13, 0, 8,
            /* Table 9-15 */ [40] = -2, -5, -10, 2, 2, -3, -3, 1, -3, -6, 0, -3, -7, -5,
            /* Table 9-16 */ [54] = -1, -1, 1, -2, -5, 0,
                             [399] = 25, 21, 21,
            /* Table 9-17 */ [60] = 0, 0, 0, 0, -9, 4, 0, -7, 13, 3,
            /* Table 9-18 */ [70] = 13, 7, 2, -39, -18, -17, -26, -35, -24, -23, -27, -24, -21, -18, -36, 0, -5, -7,
                             [88] = -4, 0, 0, -15, -35, -2, -12, -9, -31, 3, 7, 7, 8, -3, 0, -7, -9,
            /* Table 9-19 */ [105] = -13, -13, -9, -14, -8, -12, -23, -24, -10, -20, -17, -78, -70, -50, -46, -4, -5, -4, -8, 2, -1, -7, -6, -8, -34, -3, 32, 30, -44, 0, -5,
                             [136] = 0, -1, -3, -8, -25, -14, -5, 5, 2, 0, -9, -11, 18, -4, 0, 7, 9, 18, 9, 5, 9, 0, 0, 2, 19, -4, 15, 12, 9, 0,
            /* Table 9-20 */ [166] = 4, 10, 10, 33, 52, 18, 28, 35, 38, 34, 39, 32, 102, 0, 56, 33, 29, 37, 51, 39, 52, 69, 67, 44, 32, 55, 32, 0, 27, 33, 34,
                             [197] = 36, 38, 38, 34, 35, 34, 32, 37, 35, 30, 28, 26, 29, 0, 2, 8, 14, 18, 17, 21, 17, 20, 18, 27, 16, 7, 16, 11, 10, -10,
            /* Table 9-21 */ [227] = -23, -15, -7, 0, -5, -11, -9, -9, -10, -34, -21, -3, -5, -7, -11, -15, -17, -25, -25, -28, -11, -10, -10, -10, -9,
                             [252] = -16, -7, -4, -5, -9, 2, -9, 1, 11, 5, -2, -2, 0, -8, 3, 7, 10, 17, 16, 3, -1, -5, -1, -4,
            /* Table 9-22 */ [277] = -21, -23, -20, -26, -25, -17, -27, -27, -17, -26, -27, -33, -10, -14, -8, -17, -28, -6, -2, -4, -9, -8, -1, 5, 1, 9, 0, 1, 7, -7, -6,
                             [308] = -16, -2, 2, -6, -3, 2, -3, -3, 0, 9, -1, -2, -2, -1, -9, 14, 16, 0, 18, 11, 12, 10, 2, 12, 13, 0, 3, 19, 3, 18,
            /* Table 9-23 */ [338] = 19, 18, 14, 26, 31, 33, 33, 37, 39, 42, 47, 45, 49, 41, 32, 69, 63, 66, 77, 54, 52, 41, 36, 40, 30, 28, 23, 12, 11, 37, 39,
                             [369] = 40, 38, 46, 42, 40, 49, 38, 40, 38, 46, 31, 29, 25, 12, 11, 26, 22, 23, 27, 33, 26, 30, 27, 18, 25, 18, 12, 21, 14, 11,
            /* Table 9-24 */ [402] = -5, -6, -10, -7, -17, -18, -4, -10, -9, -9, -1, -8, -14, 0, 2, 17, 32, 42, 49, 53, 64, 68, 66, 47, -5, 0, -1, -2, -2,
                             [431] = -9, -6, -4, -4, -7, -3, -3, -7, -6, -12, -14, -3, -6, -5, -5, 0, -4, -9, 1, 2, 17, 32, 42, 49, 53, 64, 68, 66, 47,
            /* Table 9-25 */ [460] = 0, -5, -7, -4, 0, 0, -15, -35, -2, -12, -9, -31,
                             [472] = 0, -5, -7, -4, 0, 0, -15, -35, -2, -12, -9, -31,
            /* Table 9-26 */ [484] = -13, -13, -9, -14, -8, -12, -23, -24, -10, -20, -17, -78, -70, -50, -46,
                             [528] = -13, -13, -9, -14, -8, -12, -23, -24, -10, -20, -17, -78, -70, -50, -46,
                             [499] = -4, -5, -4, -8, 2, -1, -7, -6, -8, -34, -3, 32, 30, -44, 0, -5, 0, -1, -3, -8, -25, -14, -5, 5, 2, 0, -9, -11, 18,
                             [543] = -4, -5, -4, -8, 2, -1, -7, -6, -8, -34, -3, 32, 30, -44, 0, -5, 0, -1, -3, -8, -25, -14, -5, 5, 2, 0, -9, -11, 18,
            /* Table 9-27 */ [572] = 4, 10, 10, 33, 52, 18, 28, 35, 38, 34, 39, 32, 102, 0, 56, 33, 29, 37, 51, 39, 52, 69, 67, 44, 32, 55, 32, 0, 27, 33, 34,
                             [616] = 4, 10, 10, 33, 52, 18, 28, 35, 38, 34, 39, 32, 102, 0, 56, 33, 29, 37, 51, 39, 52, 69, 67, 44, 32, 55, 32, 0, 27, 33, 34,
                             [603] = 36, 38, 38, 34, 35, 34, 32, 37, 35, 30, 28, 26, 29,
                             [647] = 36, 38, 38, 34, 35, 34, 32, 37, 35, 30, 28, 26, 29,
            /* Table 9-28 */ [660] = -5, -6, -10, -7, -17, -18, -4, -10, -9, -9, -1, -8, -14, 0, 2, -3, -3, -7, -6, -12, -14, -3, -6, -5, -5, 0, -4, -9, 1,
                             [689] = 2, 17, 32, 42, 49, 53, 64, 68, 66, 47, 17, 32, 42, 49, 53, 64, 68, 66, 47, -5, 0, -1, -2, -2, -9, -6, -4, -4, -7,
            /* Table 9-29 */ [718] = -5, -6, -10, -7, -17, -18, -4, -10, -9, -9, -1, -8, -14, 0, 2, -3, -3, -7, -6, -12, -14, -3, -6, -5, -5, 0, -4, -9, 1,
                             [747] = 2, 17, 32, 42, 49, 53, 64, 68, 66, 47, 17, 32, 42, 49, 53, 64, 68, 66, 47, -5, 0, -1, -2, -2, -9, -6, -4, -4, -7,
            /* Table 9-30 */ [776] = -21, -23, -20, -26, -25, -17, -27, -27, -17, -26, -27, -33, -10, -14, -8, -17, -28, -6, -2, -4, -9, -8, -1, 5, 1, 9, 0, 1, 7, -7, -6,
                             [820] = -21, -23, -20, -26, -25, -17, -27, -27, -17, -26, -27, -33, -10, -14, -8, -17, -28, -6, -2, -4, -9, -8, -1, 5, 1, 9, 0, 1, 7, -7, -6,
                             [807] = -16, -2, 2, -6, -3, 2, -3, -3, 0, 9, -1, -2, -2,
                             [851] = -16, -2, 2, -6, -3, 2, -3, -3, 0, 9, -1, -2, -2,
            /* Table 9-31 */ [864] = 19, 18, 14, 26, 31, 33, 33, 37, 39, 42, 47, 45, 49, 41, 32, 69, 63, 66, 77, 54, 52, 41, 36, 40, 30, 28, 23, 12, 11, 37, 39,
                             [908] = 19, 18, 14, 26, 31, 33, 33, 37, 39, 42, 47, 45, 49, 41, 32, 69, 63, 66, 77, 54, 52, 41, 36, 40, 30, 28, 23, 12, 11, 37, 39,
                             [895] = 40, 38, 46, 42, 40, 49, 38, 40, 38, 46, 31, 29, 25,
                             [939] = 40, 38, 46, 42, 40, 49, 38, 40, 38, 46, 31, 29, 25,
            /* Table 9-32 */ [952] = -23, -15, -7, 0, -5, -11, -9, -9, -10, -34, -21, -3, -5, -7, -11, -15, -17, -25, -25, -28, -11, -10, -10, -10, -9, -16, -7, -4, -5, -9,
                             [982] = -23, -15, -7, 0, -5, -11, -9, -9, -10, -34, -21, -3, -5, -7, -11, -15, -17, -25, -25, -28, -11, -10, -10, -10, -9, -16, -7, -4, -5, -9,
            /* Table 9-33 */ [1012] = -2, -12, -9, -31, -2, -12,
                             [1018] = -9, -31, -2, -12, -9, -31,
        },
        [VIDEO_H264_CABAC_CONTEXT_N] = {
            /* Table 9-12 */ [0] = -15, 54, 74, -15, 54, 74, 127, 104, 53, 54, 51,
            /* Table 9-13 */ [11] = 25, 0, 0, 9, 41, 118, 65, 71, 79, 52, 50, 70, 54,
            /* Table 9-14 */ [24] = 34, 22, 0, 2, 36, 69, 127, 101, 76, 71, 79, 52, 69, 90, 52, 43,
            /* Table 9-15 */ [40] = 69, 82, 96, 59, 75, 87, 100, 56, 74, 85, 59, 81, 86, 95,
            /* Table 9-16 */ [54] = 66, 77, 70, 86, 72, 61,
                             [399] = 32, 49, 54,
            /* Table 9-17 */ [60] = 41, 63, 63, 63, 83, 86, 97, 72, 41, 62,
            /* Table 9-18 */ [70] = 15, 51, 80, 127, 91, 96, 81, 98, 102, 97, 119, 99, 110, 102, 127, 80, 89, 94,
                             [88] = 92, 39, 65, 84, 127, 73, 104, 91, 127, 55, 56, 55, 61, 53, 68, 74, 88,
            /* Table 9-19 */ [105] = 103, 91, 89, 92, 76, 87, 110, 105, 78, 112, 99, 127, 127, 127, 127, 66, 78, 71, 72, 59, 55, 70, 75, 89, 119, 75, 20, 22, 127, 54, 61,
                             [136] = 58, 60, 61, 67, 84, 74, 65, 52, 57, 61, 69, 70, 55, 71, 58, 61, 41, 25, 32, 43, 47, 44, 51, 46, 38, 66, 38, 42, 34, 89,
            /* Table 9-20 */ [166] = 45, 28, 31, -11, -43, 15, 0, -22, -25, 0, -18, -12, -94, 0, -15, -4, 10, -5, -29, -9, -34, -58, -63, -5, 7, -29, 1, 0, 36, -25, -30,
                             [197] = -28, -28, -27, -18, -16, -14, -8, -6, 0, 10, 18, 25, 41, 75, 72, 77, 35, 31, 35, 30, 45, 42, 45, 26, 54, 66, 56, 73, 67, 116,
            /* Table 9-21 */ [227] = 112, 71, 61, 53, 66, 77, 80, 84, 87, 127, 101, 39, 53, 61, 75, 77, 91, 107, 111, 122, 76, 44, 52, 57, 58,
                             [252] = 72, 69, 69, 74, 86, 66, 34, 32, 31, 52, 55, 67, 73, 89, 52, 4, 8, 8, 19, 37, 61, 73, 70, 78,
            /* Table 9-22 */ [277] = 126, 124, 110, 126, 124, 105, 121, 117, 102, 117, 116, 122, 95, 100, 95, 111, 114, 89, 80, 82, 85, 81, 72, 64, 67, 56, 69, 69, 69, 69, 67,
                             [308] = 77, 64, 61, 67, 64, 57, 65, 66, 62, 51, 66, 71, 75, 70, 72, 60, 37, 47, 35, 37, 41, 41, 48, 41, 41, 59, 50, 40, 66, 50,
            /* Table 9-23 */ [338] = -6, -6, 0, -12, -16, -25, -22, -28, -30, -30, -42, -36, -34, -17, 9, -71, -63, -64, -74, -39, -35, -10, 0, -1, 14, 26, 37, 55, 65, -33, -36,
                             [369] = -37, -30, -33, -30, -24, -29, -12, -10, -3, -5, 20, 30, 44, 48, 49, 45, 22, 22, 21, 20, 28, 24, 34, 42, 39, 50, 70, 54, 71, 83,
            /* Table 9-24 */ [402] = 85, 81, 77, 81, 80, 73, 74, 83, 71, 67, 61, 66, 66, 59, 59, -10, -13, -9, -5, 0, 3, 10, 27, 57, 71, 24, 36, 42, 52,
                             [431] = 57, 63, 65, 67, 82, 81, 76, 72, 78, 72, 68, 70, 76, 66, 62, 57, 61, 60, 54, 58, -10, -13, -9, -5, 0, 3, 10, 27, 57,
            /* Table 9-25 */ [460] = 80, 89, 94, 92, 39, 65, 84, 127, 73, 104, 91, 127,
                             [472] = 80, 89, 94, 92, 39, 65, 84, 127, 73, 104, 91, 127,
            /* Table 9-26 */ [484] = 103, 91, 89, 92, 76, 87, 110, 105, 78, 112, 99, 127, 127, 127, 127,
                             [528] = 103, 91, 89, 92, 76, 87, 110, 105, 78, 112, 99, 127, 127, 127, 127,
                             [499] = 66, 78, 71, 72, 59, 55, 70, 75, 89, 119, 75, 20, 22, 127, 54, 61, 58, 60, 61, 67, 84, 74, 65, 52, 57, 61, 69, 70, 55,
                             [543] = 66, 78, 71, 72, 59, 55, 70, 75, 89, 119, 75, 20, 22, 127, 54, 61, 58, 60, 61, 67, 84, 74, 65, 52, 57, 61, 69, 70, 55,
            /* Table 9-27 */ [572] = 45, 28, 31, -11, -43, 15, 0, -22, -25, 0, -18, -12, -94, 0, -15, -4, 10, -5, -29, -9, -34, -58, -63, -5, 7, -29, 1, 0, 36, -25, -30,
                             [616] = 45, 28, 31, -11, -43, 15, 0, -22, -25, 0, -18, -12, -94, 0, -15, -4, 10, -5, -29, -9, -34, -58, -63, -5, 7, -29, 1, 0, 36, -25, -30,
                             [603] = -28, -28, -27, -18, -16, -14, -8, -6, 0, 10, 18, 25, 41,
                             [647] = -28, -28, -27, -18, -16, -14, -8, -6, 0, 10, 18, 25, 41,
            /* Table 9-28 */ [660] = 85, 81, 77, 81, 80, 73, 74, 83, 71, 67, 61, 66, 66, 59, 59, 81, 76, 72, 78, 72, 68, 70, 76, 66, 62, 57, 61, 60, 54,
                             [689] = 58, -10, -13, -9, -5, 0, 3, 10, 27, 57, -10, -13, -9, -5, 0, 3, 10, 27, 57, 71, 24, 36, 42, 52, 57, 63, 65, 67, 82,
            /* Table 9-29 */ [718] = 85, 81, 77, 81, 80, 73, 74, 83, 71, 67, 61, 66, 66, 59, 59, 81, 76, 72, 78, 72, 68, 70, 76, 66, 62, 57, 61, 60, 54,
                             [747] = 58, -10, -13, -9, -5, 0, 3, 10, 27, 57, -10, -13, -9, -5, 0, 3, 10, 27, 57, 71, 24, 36, 42, 52, 57, 63, 65, 67, 82,
            /* Table 9-30 */ [776] = 126, 124, 110, 126, 124, 105, 121, 117, 102, 117, 116, 122, 95, 100, 95, 111, 114, 89, 80, 82, 85, 81, 72, 64, 67, 56, 69, 69, 69, 69, 67,
                             [820] = 126, 124, 110, 126, 124, 105, 121, 117, 102, 117, 116, 122, 95, 100, 95, 111, 114, 89, 80, 82, 85, 81, 72, 64, 67, 56, 69, 69, 69, 69, 67,
                             [807] = 77, 64, 61, 67, 64, 57, 65, 66, 62, 51, 66, 71, 75,
                             [851] = 77, 64, 61, 67, 64, 57, 65, 66, 62, 51, 66, 71, 75,
            /* Table 9-31 */ [864] = -6, -6, 0, -12, -16, -25, -22, -28, -30, -30, -42, -36, -34, -17, 9, -71, -63, -64, -74, -39, -35, -10, 0, -1, 14, 26, 37, 55, 65, -33, -36,
                             [908] = -6, -6, 0, -12, -16, -25, -22, -28, -30, -30, -42, -36, -34, -17, 9, -71, -63, -64, -74, -39, -35, -10, 0, -1, 14, 26, 37, 55, 65, -33, -36,
                             [895] = -37, -30, -33, -30, -24, -29, -12, -10, -3, -5, 20, 30, 44,
                             [939] = -37, -30, -33, -30, -24, -29, -12, -10, -3, -5, 20, 30, 44,
            /* Table 9-32 */ [952] = 112, 71, 61, 53, 66, 77, 80, 84, 87, 127, 101, 39, 53, 61, 75, 77, 91, 107, 111, 122, 76, 44, 52, 57, 58, 72, 69, 69, 74, 86,
                             [982] = 112, 71, 61, 53, 66, 77, 80, 84, 87, 127, 101, 39, 53, 61, 75, 77, 91, 107, 111, 122, 76, 44, 52, 57, 58, 72, 69, 69, 74, 86,
            /* Table 9-33 */ [1012] = 73, 104, 91, 127, 73, 104,
                             [1018] = 91, 127, 73, 104, 91, 127,
        },
    },
    [VIDEO_H264_CABAC_CONTEXT_CABAC_INIT_IDC_2] = {
        [VIDEO_H264_CABAC_CONTEXT_M] = {
            /* Table 9-12 */ [0] = 20, 2, 3, 20, 2, 3, -28, -23, -6, -1, 7,
            /* Table 9-13 */ [11] = 29, 25, 14, -10, -3, -27, 26, -4, -24, 5, 6, -17, 14,
            /* Table 9-14 */ [24] = 20, 20, 29, 54, 37, 12, -32, -22, -2, -4, -24, 5, -6, -14, -6, 4,
            /* Table 9-15 */ [40] = -11, -15, -21, 19, 20, 4, 6, 1, -5, -13, 5, 6, -3, -1,
            /* Table 9-16 */ [54] = 3, -4, -2, -12, -7, 1,
                             [399] = 21, 19, 17,
            /* Table 9-17 */ [60] = 0, 0, 0, 0, -9, 4, 0, -7, 13, 3,
            /* Table 9-18 */ [70] = 7, -9, -20, -36, -17, -14, -25, -25, -12, -17, -31, -14, -18, -13, -37, 11, 5, 2,
                             [88] = 5, -6, 4, -14, -37, -5, -11, -11, -30, 0, -2, 0, -4, -6, 3, -8, -13,
            /* Table 9-19 */ [105] = -4, -12, -5, -3, -4, -8, -16, -9, -1, 5, 4, -4, -2, 2, -1, -4, -1, 0, -7, -4, -6, -3, -6, 8, -9, -11, 9, 0, -5, 1, -15,
                             [136] = -5, -8, -21, -21, -13, -25, -29, 9, 17, -8, -5, -2, 13, 3, -7, 8, -10, 3, -3, -20, 0, 1, -3, -21, 16, -23, 17, 44, 50, -22,
            /* Table 9-20 */ [166] = 4, 0, 7, 11, 8, 6, 7, 3, 8, 13, 13, 4, 3, 2, 6, 8, 11, 14, 7, 4, 4, 13, 9, 19, 10, 12, 0, 20, 8, 35, 33,
                             [197] = 28, 24, 27, 34, 52, 39, 19, 31, 36, 24, 34, 30, 22, 20, 19, 27, 19, 15, 15, 21, 25, 30, 31, 27, 24, 0, 14, 15, 26, -24,
            /* Table 9-21 */ [227] = -24, -22, -9, 0, 0, -14, -13, -13, -11, -29, -21, -14, -12, -11, -10, -21, -16, -23, -15, -37, -10, -8, -8, -8, -7,
                             [252] = -14, -10, -9, -12, -18, -4, -22, -16, -2, 1, -13, -9, -4, -13, -13, -6, -13, -6, -2, -16, -10, -13, -9, -10,
            /* Table 9-22 */ [277] = -22, -25, -25, -27, -19, -23, -25, -26, -24, -28, -31, -37, -10, -15, -10, -13, -50, -5, 17, -5, -13, -12, -2, 0, -1, 4, -7, 5, 15, 1, 0,
                             [308] = -10, 1, 0, 2, 0, -5, 7, 5, 2, 14, 15, 5, 2, -2, -18, 12, 5, -12, 11, 5, 0, 2, -6, 5, 7, -6, -11, -2, -2, 25,
            /* Table 9-23 */ [338] = 17, 16, 17, 27, 37, 41, 42, 48, 39, 46, 52, 46, 52, 43, 32, 61, 56, 62, 81, 45, 35, 28, 34, 39, 30, 20, 18, 15, 0, 36, 37,
                             [369] = 37, 32, 34, 29, 24, 34, 31, 35, 31, 33, 36, 27, 21, 18, 19, 36, 24, 27, 24, 31, 22, 22, 16, 15, 14, 3, -16, 21, 22, 25,
            /* Table 9-24 */ [402] = -3, -8, -9, -10, -18, -12, -11, -5, -17, -14, -16, -8, -14, -9, -11, 9, 30, 31, 33, 33, 31, 37, 31, 20, -9, -7, -8, -11, -10,
                             [431] = -12, -8, -9, -6, -10, -3, -8, -9, -10, -18, -12, -11, -5, -17, -14, -16, -8, -14, -9, -11, 9, 30, 31, 33, 33, 31, 37, 31, 20,
            /* Table 9-25 */ [460] = 11, 5, 2, 5, -6, 4, -14, -37, -5, -11, -11, -30,
                             [472] = 11, 5, 2, 5, -6, 4, -14, -37, -5, -11, -11, -30,
            /* Table 9-26 */ [484] = -4, -12, -5, -3, -4, -8, -16, -9, -1, 5, 4, -4, -2, 2, -1,
                             [528] = -4, -12, -5, -3, -4, -8, -16, -9, -1, 5, 4, -4, -2, 2, -1,
                             [499] = -4, -1, 0, -7, -4, -6, -3, -6, 8, -9, -11, 9, 0, -5, 1, -15, -5, -8, -21, -21, -13, -25, -29, 9, 17, -8, -5, -2, 13,
                             [543] = -4, -1, 0, -7, -4, -6, -3, -6, 8, -9, -11, 9, 0, -5, 1, -15, -5, -8, -21, -21, -13, -25, -29, 9, 17, -8, -5, -2, 13,
            /* Table 9-27 */ [572] = 4, 0, 7, 11, 8, 6, 7, 3, 8, 13, 13, 4, 3, 2, 6, 8, 11, 14, 7, 4, 4, 13, 9, 19, 10, 12, 0, 20, 8, 35, 33,
                             [616] = 4, 0, 7, 11, 8, 6, 7, 3, 8, 13, 13, 4, 3, 2, 6, 8, 11, 14, 7, 4, 4, 13, 9, 19, 10, 12, 0, 20, 8, 35, 33,
                             [603] = 28, 24, 27, 34, 52, 39, 19, 31, 36, 24, 34, 30, 22,
                             [647] = 28, 24, 27, 34, 52, 39, 19, 31, 36, 24, 34, 30, 22,
            /* Table 9-28 */ [660] = -3, -8, -9, -10, -18, -12, -11, -5, -17, -14, -16, -8, -14, -9, -11, -3, -8, -9, -10, -18, -12, -11, -5, -17, -14, -16, -8, -14, -9,
                             [689] = -11, 9, 30, 31, 33, 33, 31, 37, 31, 20, 9, 30, 31, 33, 33, 31, 37, 31, 20, -9, -7, -8, -11, -10, -12, -8, -9, -6, -10,
            /* Table 9-29 */ [718] = -3, -8, -9, -10, -18, -12, -11, -5, -17, -14, -16, -8, -14, -9, -11, -3, -8, -9, -10, -18, -12, -11, -5, -17, -14, -16, -8, -14, -9,
                             [747] = -11, 9, 30, 31, 33, 33, 31, 37, 31, 20, 9, 30, 31, 33, 33, 31, 37, 31, 20, -9, -7, -8, -11, -10, -12, -8, -9, -6, -10,
            /* Table 9-30 */ [776] = -22, -25, -25, -27, -19, -23, -25, -26, -24, -28, -31, -37, -10, -15, -10, -13, -50, -5, 17, -5, -13, -12, -2, 0, -1, 4, -7, 5, 15, 1, 0,
                             [820] = -22, -25, -25, -27, -19, -23, -25, -26, -24, -28, -31, -37, -10, -15, -10, -13, -50, -5, 17, -5, -13, -12, -2, 0, -1, 4, -7, 5, 15, 1, 0,
                             [807] = -10, 1, 0, 2, 0, -5, 7, 5, 2, 14, 15, 5, 2,
                             [851] = -10, 1, 0, 2, 0, -5, 7, 5, 2, 14, 15, 5, 2,
            /* Table 9-31 */ [864] = 17, 16, 17, 27, 37, 41, 42, 48, 39, 46, 52, 46, 52, 43, 32, 61, 56, 62, 81, 45, 35, 28, 34, 39, 30, 20, 18, 15, 0, 36, 37,
                             [908] = 17, 16, 17, 27, 37, 41, 42, 48, 39, 46, 52, 46, 52, 43, 32, 61, 56, 62, 81, 45, 35, 28, 34, 39, 30, 20, 18, 15, 0, 36, 37,
                             [895] = 37, 32, 34, 29, 24, 34, 31, 35, 31, 33, 36, 27, 21,
                             [939] = 37, 32, 34, 29, 24, 34, 31, 35, 31, 33, 36, 27, 21,
            /* Table 9-32 */ [952] = -24, -22, -9, 0, 0, -14, -13, -13, -11, -29, -21, -14, -12, -11, -10, -21, -16, -23, -15, -37, -10, -8, -8, -8, -7, -14, -10, -9, -12, -18,
                             [982] = -24, -22, -9, 0, 0, -14, -13, -13, -11, -29, -21, -14, -12, -11, -10, -21, -16, -23, -15, -37, -10, -8, -8, -8, -7, -14, -10, -9, -12, -18,
            /* Table 9-33 */ [1012] = -5, -11, -11, -30, -5, -11,
                             [1018] = -11, -30, -5, -11, -11, -30,
        },
        [VIDEO_H264_CABAC_CONTEXT_N] = {
            /* Table 9-12 */ [0] = -15, 54, 74, -15, 54, 74, 127, 104, 53, 54, 51,
            /* Table 9-13 */ [11] = 16, 0, 0, 51, 62, 99, 16, 85, 102, 57, 57, 73, 57,
            /* Table 9-14 */ [24] = 40, 10, 0, 0, 42, 97, 127, 117, 74, 85, 102, 57, 93, 88, 44, 55,
            /* Table 9-15 */ [40] = 89, 103, 116, 57, 58, 84, 96, 63, 85, 106, 63, 75, 90, 101,
            /* Table 9-16 */ [54] = 55, 79, 75, 97, 50, 60,
                             [399] = 33, 50, 61,
            /* Table 9-17 */ [60] = 41, 63, 63, 63, 83, 86, 97, 72, 41, 62,
            /* Table 9-18 */ [70] = 34, 88, 127, 127, 91, 95, 84, 86, 89, 91, 127, 76, 103, 90, 127, 80, 76, 84,
                             [88] = 78, 55, 61, 83, 127, 79, 104, 91, 127, 65, 79, 72, 92, 56, 68, 71, 98,
            /* Table 9-19 */ [105] = 86, 88, 82, 72, 67, 72, 89, 69, 59, 66, 57, 71, 71, 58, 74, 44, 69, 62, 51, 47, 42, 41, 53, 76, 78, 83, 52, 67, 90, 67, 72,
                             [136] = 75, 80, 83, 64, 31, 64, 94, 75, 63, 74, 35, 27, 91, 65, 69, 77, 66, 62, 68, 81, 30, 7, 23, 74, 66, 124, 37, -18, -34, 127,
            /* Table 9-20 */ [166] = 39, 42, 34, 29, 31, 37, 42, 40, 33, 43, 36, 47, 55, 58, 60, 44, 44, 42, 48, 56, 52, 37, 49, 58, 48, 45, 69, 33, 63, -18, -25,
                             [197] = -3, 10, 0, -14, -44, -24, 17, 25, 29, 33, 15, 20, 73, 34, 31, 44, 16, 36, 36, 28, 21, 20, 12, 16, 42, 93, 56, 57, 38, 127,
            /* Table 9-21 */ [227] = 115, 82, 62, 53, 59, 85, 89, 94, 92, 127, 100, 57, 67, 71, 77, 85, 88, 104, 98, 127, 82, 48, 61, 66, 70,
                             [252] = 75, 79, 83, 92, 108, 79, 69, 75, 58, 58, 78, 83, 81, 99, 81, 38, 62, 58, 59, 73, 76, 86, 83, 87,
            /* Table 9-22 */ [277] = 127, 127, 120, 127, 114, 117, 118, 117, 113, 118, 120, 124, 94, 102, 99, 106, 127, 92, 57, 86, 94, 91, 77, 71, 73, 64, 81, 64, 57, 67, 68,
                             [308] = 67, 68, 77, 64, 68, 78, 55, 59, 65, 54, 44, 60, 70, 76, 86, 70, 64, 70, 55, 56, 69, 65, 74, 54, 54, 76, 82, 77, 77, 42,
            /* Table 9-23 */ [338] = -13, -9, -12, -21, -30, -40, -41, -47, -32, -40, -51, -41, -39, -19, 11, -55, -46, -50, -67, -20, -2, 15, 1, 1, 17, 38, 45, 54, 79, -16, -14,
                             [369] = -17, 1, 15, 15, 25, 22, 16, 18, 28, 41, 28, 47, 62, 31, 26, 24, 23, 16, 30, 29, 41, 42, 60, 52, 60, 78, 123, 53, 56, 61,
            /* Table 9-24 */ [402] = 78, 74, 72, 72, 75, 71, 63, 70, 75, 72, 67, 53, 59, 52, 68, -2, -10, -4, -1, 7, 12, 23, 38, 64, 71, 37, 44, 49, 56,
                             [431] = 59, 63, 67, 68, 79, 78, 74, 72, 72, 75, 71, 63, 70, 75, 72, 67, 53, 59, 52, 68, -2, -10, -4, -1, 7, 12, 23, 38, 64,
            /* Table 9-25 */ [460] = 80, 76, 84, 78, 55, 61, 83, 127, 79, 104, 91, 127,
                             [472] = 80, 76, 84, 78, 55, 61, 83, 127, 79, 104, 91, 127,
            /* Table 9-26 */ [484] = 86, 88, 82, 72, 67, 72, 89, 69, 59, 66, 57, 71, 71, 58, 74,
                             [528] = 86, 88, 82, 72, 67, 72, 89, 69, 59, 66, 57, 71, 71, 58, 74,
                             [499] = 44, 69, 62, 51, 47, 42, 41, 53, 76, 78, 83, 52, 67, 90, 67, 72, 75, 80, 83, 64, 31, 64, 94, 75, 63, 74, 35, 27, 91,
                             [543] = 44, 69, 62, 51, 47, 42, 41, 53, 76, 78, 83, 52, 67, 90, 67, 72, 75, 80, 83, 64, 31, 64, 94, 75, 63, 74, 35, 27, 91,
            /* Table 9-27 */ [572] = 39, 42, 34, 29, 31, 37, 42, 40, 33, 43, 36, 47, 55, 58, 60, 44, 44, 42, 48, 56, 52, 37, 49, 58, 48, 45, 69, 33, 63, -18, -25,
                             [616] = 39, 42, 34, 29, 31, 37, 42, 40, 33, 43, 36, 47, 55, 58, 60, 44, 44, 42, 48, 56, 52, 37, 49, 58, 48, 45, 69, 33, 63, -18, -25,
                             [603] = -3, 10, 0, -14, -44, -24, 17, 25, 29, 33, 15, 20, 73,
                             [647] = -3, 10, 0, -14, -44, -24, 17, 25, 29, 33, 15, 20, 73,
            /* Table 9-28 */ [660] = 78, 74, 72, 72, 75, 71, 63, 70, 75, 72, 67, 53, 59, 52, 68, 78, 74, 72, 72, 75, 71, 63, 70, 75, 72, 67, 53, 59, 52,
                             [689] = 68, -2, -10, -4, -1, 7, 12, 23, 38, 64, -2, -10, -4, -1, 7, 12, 23, 38, 64, 71, 37, 44, 49, 56, 59, 63, 67, 68, 79,
            /* Table 9-29 */ [718] = 78, 74, 72, 72, 75, 71, 63, 70, 75, 72, 67, 53, 59, 52, 68, 78, 74, 72, 72, 75, 71, 63, 70, 75, 72, 67, 53, 59, 52,
                             [747] = 68, -2, -10, -4, -1, 7, 12, 23, 38, 64, -2, -10, -4, -1, 7, 12, 23, 38, 64, 71, 37, 44, 49, 56, 59, 63, 67, 68, 79,
            /* Table 9-30 */ [776] = 127, 127, 120, 127, 114, 117, 118, 117, 113, 118, 120, 124, 94, 102, 99, 106, 127, 92, 57, 86, 94, 91, 77, 71, 73, 64, 81, 64, 57, 67, 68,
                             [820] = 127, 127, 120, 127, 114, 117, 118, 117, 113, 118, 120, 124, 94, 102, 99, 106, 127, 92, 57, 86, 94, 91, 77, 71, 73, 64, 81, 64, 57, 67, 68,
                             [807] = 67, 68, 77, 64, 68, 78, 55, 59, 65, 54, 44, 60, 70,
                             [851] = 67, 68, 77, 64, 68, 78, 55, 59, 65, 54, 44, 60, 70,
            /* Table 9-31 */ [864] = -13, -9, -12, -21, -30, -40, -41, -47, -32, -40, -51, -41, -39, -19, 11, -55, -46, -50, -67, -20, -2, 15, 1, 1, 17, 38, 45, 54, 79, -16, -14,
                             [908] = -13, -9, -12, -21, -30, -40, -41, -47, -32, -40, -51, -41, -39, -19, 11, -55, -46, -50, -67, -20, -2, 15, 1, 1, 17, 38, 45, 54, 79, -16, -14,
                             [895] = -17, 1, 15, 15, 25, 22, 16, 18, 28, 41, 28, 47, 62,
                             [939] = -17, 1, 15, 15, 25, 22, 16, 18, 28, 41, 28, 47, 62,
            /* Table 9-32 */ [952] = 115, 82, 62, 53, 59, 85, 89, 94, 92, 127, 100, 57, 67, 71, 77, 85, 88, 104, 98, 127, 82, 48, 61, 66, 70, 75, 79, 83, 92, 108,
                             [982] = 115, 82, 62, 53, 59, 85, 89, 94, 92, 127, 100, 57, 67, 71, 77, 85, 88, 104, 98, 127, 82, 48, 61, 66, 70, 75, 79, 83, 92, 108,
            /* Table 9-33 */ [1012] = 79, 104, 91, 127, 79, 104,
                             [1018] = 91, 127, 79, 104, 91, 127,
        },
    },
};
static const VIDEOH264CABACENGINENUM sg_pCABACrangeTabLPS[64][4] = {
    [0] =  { 128, 176, 208, 240, },
    [1] =  { 128, 167, 197, 227, },
    [2] =  { 128, 158, 187, 216, },
    [3] =  { 123, 150, 178, 205, },
    [4] =  { 116, 142, 169, 195, },
    [5] =  { 111, 135, 160, 185, },
    [6] =  { 105, 128, 152, 175, },
    [7] =  { 100, 122, 144, 166, },
    [8] =  {  95, 116, 137, 158, },
    [9] =  {  90, 110, 130, 150, },
    [10] = {  85, 104, 123, 142, },
    [11] = {  81,  99, 117, 135, },
    [12] = {  77,  94, 111, 128, },
    [13] = {  73,  89, 105, 122, },
    [14] = {  69,  85, 100, 116, },
    [15] = {  66,  80,  95, 110, },
    [16] = {  62,  76,  90, 104, },
    [17] = {  59,  72,  86,  99, },
    [18] = {  56,  69,  81,  94, },
    [19] = {  53,  65,  77,  89, },
    [20] = {  51,  62,  73,  85, },
    [21] = {  48,  59,  69,  80, },
    [22] = {  46,  56,  66,  76, },
    [23] = {  43,  53,  63,  72, },
    [24] = {  41,  50,  59,  69, },
    [25] = {  39,  48,  56,  65, },
    [26] = {  37,  45,  54,  62, },
    [27] = {  35,  43,  51,  59, },
    [28] = {  33,  41,  48,  56, },
    [29] = {  32,  39,  46,  53, },
    [30] = {  30,  37,  43,  50, },
    [31] = {  29,  35,  41,  48, },
    [32] = {  27,  33,  39,  45, },
    [33] = {  26,  31,  37,  43, },
    [34] = {  24,  30,  35,  41, },
    [35] = {  23,  28,  33,  39, },
    [36] = {  22,  27,  32,  37, },
    [37] = {  21,  26,  30,  35, },
    [38] = {  20,  24,  29,  33, },
    [39] = {  19,  23,  27,  31, },
    [40] = {  18,  22,  26,  30, },
    [41] = {  17,  21,  25,  28, },
    [42] = {  16,  20,  23,  27, },
    [43] = {  15,  19,  22,  25, },
    [44] = {  14,  18,  21,  24, },
    [45] = {  14,  17,  20,  23, },
    [46] = {  13,  16,  19,  22, },
    [47] = {  12,  15,  18,  21, },
    [48] = {  12,  14,  17,  20, },
    [49] = {  11,  14,  16,  19, },
    [50] = {  11,  13,  15,  18, },
    [51] = {  10,  12,  15,  17, },
    [52] = {  10,  12,  14,  16, },
    [53] = {   9,  11,  13,  15, },
    [54] = {   9,  11,  12,  14, },
    [55] = {   8,  10,  12,  14, },
    [56] = {   8,   9,  11,  13, },
    [57] = {   7,   9,  11,  12, },
    [58] = {   7,   9,  10,  12, },
    [59] = {   7,   8,  10,  11, },
    [60] = {   6,   8,   9,  11, },
    [61] = {   6,   7,   9,  10, },
    [62] = {   6,   7,   8,   9, },
    [63] = {   2,   2,   2,   2, },
};
static const VIDEOH264CABACCONTEXTNUM sg_pCABACtransIdxLPS[64] = {
     0,  0,  1,  2,  2,  4,  4,  5,  6,  7,  8,  9,  9, 11, 11, 12,
    13, 13, 15, 15, 16, 16, 18, 18, 19, 19, 21, 21, 22, 22, 23, 24,
    24, 25, 26, 26, 27, 27, 28, 29, 29, 30, 30, 30, 31, 32, 32, 33,
    33, 33, 34, 34, 35, 35, 35, 36, 36, 36, 37, 37, 37, 38, 38, 63,
};
static const VIDEOH264CABACCONTEXTNUM sg_pCABACtransIdxMPS[64] = {
     1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 62, 63,
};

static const VIDEOH264BINSTRING sg_pMacroBlockTypeIBinStringInfo[26] = { // 参考标准的 Table 9-36
    [0] = { .binString = 0b0, .maxBinId = 0, },
    [1] = { .binString = 0b100000, .maxBinId = 5, },
    [2] = { .binString = 0b100001, .maxBinId = 5, },
    [3] = { .binString = 0b100010, .maxBinId = 5, },
    [4] = { .binString = 0b100011, .maxBinId = 5, },
    [5] = { .binString = 0b1001000, .maxBinId = 6, },
    [6] = { .binString = 0b1001001, .maxBinId = 6, },
    [7] = { .binString = 0b1001010, .maxBinId = 6, },
    [8] = { .binString = 0b1001011, .maxBinId = 6, },
    [9] = { .binString = 0b1001100, .maxBinId = 6, },
    [10] = { .binString = 0b1001101, .maxBinId = 6, },
    [11] = { .binString = 0b1001110, .maxBinId = 6, },
    [12] = { .binString = 0b1001111, .maxBinId = 6, },
    [13] = { .binString = 0b101000, .maxBinId = 5, },
    [14] = { .binString = 0b101001, .maxBinId = 5, },
    [15] = { .binString = 0b101010, .maxBinId = 5, },
    [16] = { .binString = 0b101011, .maxBinId = 5, },
    [17] = { .binString = 0b1011000, .maxBinId = 6, },
    [18] = { .binString = 0b1011001, .maxBinId = 6, },
    [19] = { .binString = 0b1011010, .maxBinId = 6, },
    [20] = { .binString = 0b1011011, .maxBinId = 6, },
    [21] = { .binString = 0b1011100, .maxBinId = 6, },
    [22] = { .binString = 0b1011101, .maxBinId = 6, },
    [23] = { .binString = 0b1011110, .maxBinId = 6, },
    [24] = { .binString = 0b1011111, .maxBinId = 6, },
    [25] = { .binString = 0b11, .maxBinId = 2, },
};
static const VIDEOH264BINSTRING sg_pMacroBlockTypePAndSPBinStringInfo[6] = { // 参考标准的 Table 9-37 的 P/SP 部分
    [0] = { .binString = 0b000, .maxBinId = 2, },
    [1] = { .binString = 0b011, .maxBinId = 2, },
    [2] = { .binString = 0b010, .maxBinId = 2, },
    [3] = { .binString = 0b001, .maxBinId = 2, },
    [5] = { .binString = 0b1, .maxBinId = 0, },
};
static const VIDEOH264BINSTRING sg_pMacroBlockTypeBBinStringInfo[24] = { // 参考标准的 Table 9-37 的 B 部分
    [0] = { .binString = 0b0, .maxBinId = 0, },
    [1] = { .binString = 0b100, .maxBinId = 2, },
    [2] = { .binString = 0b101, .maxBinId = 2, },
    [3] = { .binString = 0b110000, .maxBinId = 5, },
    [4] = { .binString = 0b110001, .maxBinId = 5, },
    [5] = { .binString = 0b110010, .maxBinId = 5, },
    [6] = { .binString = 0b110011, .maxBinId = 5, },
    [7] = { .binString = 0b110100, .maxBinId = 5, },
    [8] = { .binString = 0b110101, .maxBinId = 5, },
    [9] = { .binString = 0b110110, .maxBinId = 5, },
    [10] = { .binString = 0b110111, .maxBinId = 5, },
    [11] = { .binString = 0b111110, .maxBinId = 5, },
    [12] = { .binString = 0b1110000, .maxBinId = 6, },
    [13] = { .binString = 0b1110001, .maxBinId = 6, },
    [14] = { .binString = 0b1110010, .maxBinId = 6, },
    [15] = { .binString = 0b1110011, .maxBinId = 6, },
    [16] = { .binString = 0b1110100, .maxBinId = 6, },
    [17] = { .binString = 0b1110101, .maxBinId = 6, },
    [18] = { .binString = 0b1110110, .maxBinId = 6, },
    [19] = { .binString = 0b1110111, .maxBinId = 6, },
    [20] = { .binString = 0b1111000, .maxBinId = 6, },
    [21] = { .binString = 0b1111001, .maxBinId = 6, },
    [22] = { .binString = 0b111111, .maxBinId = 5, },
    [23] = { .binString = 0b111101, .maxBinId = 5, },
};
static const VIDEOH264BINSTRING sg_pSubMacroBlockTypePAndSPBinStringInfo[4] = { // 参考标准的 Table 9-38 的 P/SP 部分
    [0] = { .binString = 0b1, .maxBinId = 0, },
    [1] = { .binString = 0b00, .maxBinId = 1, },
    [2] = { .binString = 0b011, .maxBinId = 2, },
    [3] = { .binString = 0b010, .maxBinId = 2, },
};
static const VIDEOH264BINSTRING sg_pSubMacroBlockTypeBBinStringInfo[13] = { // 参考标准的 Table 9-38 的 B 部分
    [0] = { .binString = 0b0, .maxBinId = 0, },
    [1] = { .binString = 0b100, .maxBinId = 2, },
    [2] = { .binString = 0b101, .maxBinId = 2, },
    [3] = { .binString = 0b11000, .maxBinId = 4, },
    [4] = { .binString = 0b11001, .maxBinId = 4, },
    [5] = { .binString = 0b11010, .maxBinId = 4, },
    [6] = { .binString = 0b11011, .maxBinId = 4, },
    [7] = { .binString = 0b111000, .maxBinId = 5, },
    [8] = { .binString = 0b111001, .maxBinId = 5, },
    [9] = { .binString = 0b111010, .maxBinId = 5, },
    [10] = { .binString = 0b111011, .maxBinId = 5, },
    [11] = { .binString = 0b11110, .maxBinId = 4, },
    [12] = { .binString = 0b11111, .maxBinId = 4, },
};

static const uint8_t sg_pCtxBlockCatValue[VIDEO_H264_BLOCK_CAT_CNT] = { // 参考标准的 Table 9-42
    [VIDEO_H264_BLOCK_CAT_LumaDC] = 0,
    [VIDEO_H264_BLOCK_CAT_LumaAC] = 1,
    [VIDEO_H264_BLOCK_CAT_Luma4x4] = 2,
    [VIDEO_H264_BLOCK_CAT_ChromaDC] = 3,
    [VIDEO_H264_BLOCK_CAT_ChromaAC] = 4,
    [VIDEO_H264_BLOCK_CAT_Luma8x8] = 5,
    [VIDEO_H264_BLOCK_CAT_CbDC] = 6,
    [VIDEO_H264_BLOCK_CAT_CbAC] = 7,
    [VIDEO_H264_BLOCK_CAT_Cb4x4] = 8,
    [VIDEO_H264_BLOCK_CAT_Cb8x8] = 9,
    [VIDEO_H264_BLOCK_CAT_CrDC] = 10,
    [VIDEO_H264_BLOCK_CAT_CrAC] = 11,
    [VIDEO_H264_BLOCK_CAT_Cr4x4] = 12,
    [VIDEO_H264_BLOCK_CAT_Cr8x8] = 13,
};
static const VIDEOH264CABACCTXIDX sg_pCtxIdxBlockCatOffset_coded_block_flag[VIDEO_H264_CABAC_CTXBLOCKCAT_CNT] = { // 参考标准的 Table 9-40 的 coded_block_flag 部分
    0, 4, 8, 12, 16, 0, 0, 4, 8, 4, 0, 4, 8, 8,
};
static const VIDEOH264CABACCTXIDX sg_pCtxIdxBlockCatOffset_significant_coeff_flag[VIDEO_H264_CABAC_CTXBLOCKCAT_CNT] = { // 参考标准的 Table 9-40 的 significant_coeff_flag 部分
    0, 15, 29, 44, 47, 0, 0, 15, 29, 0, 0, 15, 15, 29,
};
static const VIDEOH264CABACCTXIDX sg_pCtxIdxBlockCatOffset_last_significant_coeff_flag[VIDEO_H264_CABAC_CTXBLOCKCAT_CNT] = { // 参考标准的 Table 9-40 的 last_significant_coeff_flag 部分
    0, 15, 29, 44, 47, 0, 0, 15, 29, 0, 0, 15, 15, 29,
};
static const VIDEOH264CABACCTXIDX sg_pCtxIdxBlockCatOffset_coeff_abs_level_minus1[VIDEO_H264_CABAC_CTXBLOCKCAT_CNT] = { // 参考标准的 Table 9-40 的 coeff_abs_level_minus1 部分
    0, 10, 20, 30, 39, 0, 0, 10, 20, 0, 0, 10, 20, 0,
};
static const VIDEOH264CABACCTXIDX sg_pCtxIdxInc8x8Frame_significant_coeff_flag[63] = { // 参考标准的 Table 9-43 的 significant_coeff_flag(frame coded macroblocks) 部分
    0, 1, 2, 3, 4, 5, 5, 4, 4, 3, 3, 4, 4, 4, 5, 5, 4, 4, 4, 4, 3, 3, 6, 7, 7, 7, 8, 9, 10, 9, 8, 7,
    7, 6, 11, 12, 13, 11, 6, 7, 8, 9, 14, 10, 9, 8, 6, 11, 12, 13, 11, 6, 9, 14, 10, 9, 11, 12, 13, 11, 14, 10, 12,
};
static const VIDEOH264CABACCTXIDX sg_pCtxIdxInc8x8Field_significant_coeff_flag[63] = { // 参考标准的 Table 9-43 的 significant_coeff_flag(field coded macroblocks) 部分
    0, 1, 1, 2, 2, 3, 3, 4, 5, 6, 7, 7, 7, 8, 4, 5, 6, 9, 10, 10, 8, 11, 12, 11, 9, 9, 10, 10, 8, 11, 12, 11,
    9, 9, 10, 10, 8, 11, 12, 11, 9, 9, 10, 10, 8, 13, 13, 9, 9, 10, 10, 8, 13, 13, 9, 9, 10, 10, 14, 14, 14, 14, 14,
};
static const VIDEOH264CABACCTXIDX sg_pCtxIdxInc8x8_last_significant_coeff_flag[63] = { // 参考标准的 Table 9-43 的 last_significant_coeff_flag 部分
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8,
};

void H264InitCABACHandler(PVIDEOH264DECODER pDecoder)
{
    /* slice_data */
    pDecoder->readSE.mb_skip_flag = H264ReadAE_mb_skip_flag;
    pDecoder->readSE.mb_field_decoding_flag = H264ReadAE_mb_field_decoding_flag;
    pDecoder->readSE.end_of_slice_flag = H264ReadAE_end_of_slice_flag;
    /* 宏块 */
    pDecoder->readSE.mb_type = H264ReadAE_mb_type;
    pDecoder->readSE.transform_size_8x8_flag = H264ReadAE_transform_size_8x8_flag;
    pDecoder->readSE.coded_block_pattern = H264ReadAE_coded_block_pattern;
    pDecoder->readSE.mb_qp_delta = H264ReadAE_mb_qp_delta;
    /* mb_pred */
    pDecoder->readSE.prev_intraNxN_pred_mode_flag = H264ReadAE_prev_intraNxN_pred_mode_flag;
    pDecoder->readSE.rem_intraNxN_pred_mode = H264ReadAE_rem_intraNxN_pred_mode;
    pDecoder->readSE.intra_chroma_pred_mode = H264ReadAE_intra_chroma_pred_mode;
    pDecoder->readSE.mvd_l0[0] = H264ReadAE_mvd_l0_0;
    pDecoder->readSE.mvd_l0[1] = H264ReadAE_mvd_l0_1;
    pDecoder->readSE.mvd_l1[0] = H264ReadAE_mvd_l1_0;
    pDecoder->readSE.mvd_l1[1] = H264ReadAE_mvd_l1_1;
    pDecoder->readSE.ref_idx_l0 = H264ReadAE_ref_idx_l0;
    pDecoder->readSE.ref_idx_l1 = H264ReadAE_ref_idx_l1;
    /* sub_mb_pred */
    pDecoder->readSE.sub_mb_type = H264ReadAE_sub_mb_type;
    /* residual_block */
    pDecoder->residual_block = DecodeH264_residual_block_cabac;
}

bool H264CABACInitContext(PCVIDEOH264SLICE pSlice, PVIDEOH264DECODEINFO pDecodeInfo)
{
    bool bRet = true;
    PVIDEOH264READAE pCABAC = pDecodeInfo->pCABAC;
    VIDEOH264SLICETYPE sliceType = pSlice->type;
    uint32_t nCabacInitIdc = pSlice->slice_header.cabac_init_idc;
    const VIDEOH264CABACCONTEXMN* pTableM;
    const VIDEOH264CABACCONTEXMN* pTableN;
    VIDEOH264CABACCONTEXMN m;
    VIDEOH264CABACCONTEXMN n;
    int32_t preCtxState;

    if ((VIDEO_H264_SLICE_TYPE_I == sliceType) || (VIDEO_H264_SLICE_TYPE_SI == sliceType))
    {
        pTableM = sg_pCABACContexTableMN[VIDEO_H264_CABAC_CONTEXT_CABAC_INIT_IDC_I_SLICE][VIDEO_H264_CABAC_CONTEXT_M];
        pTableN = sg_pCABACContexTableMN[VIDEO_H264_CABAC_CONTEXT_CABAC_INIT_IDC_I_SLICE][VIDEO_H264_CABAC_CONTEXT_N];
    }
    else if (nCabacInitIdc <= 2)
    {
        pTableM = sg_pCABACContexTableMN[VIDEO_H264_CABAC_CONTEXT_CABAC_INIT_IDC_0 + nCabacInitIdc][VIDEO_H264_CABAC_CONTEXT_M];
        pTableN = sg_pCABACContexTableMN[VIDEO_H264_CABAC_CONTEXT_CABAC_INIT_IDC_0 + nCabacInitIdc][VIDEO_H264_CABAC_CONTEXT_N];
    }
    else { bRet = false; }

    if (true == bRet)
    {
        for (HANSIZE ctxIdx = 0; ctxIdx < VIDEO_H264_CABAC_CONTEXT_SIZE; ctxIdx++)
        {
            m = pTableM[ctxIdx];
            n = pTableN[ctxIdx];

            preCtxState = H264Clip3(1, 126, ((m * H264Clip3(0, 51, pSlice->SliceQPY)) >> 4) + n);
            if (preCtxState <= 63)
            {
                pCABAC->context[ctxIdx].pStateIdx = 63 - preCtxState;
                pCABAC->context[ctxIdx].valMPS = 0;
            }
            else
            {
                pCABAC->context[ctxIdx].pStateIdx = preCtxState - 64;
                pCABAC->context[ctxIdx].valMPS = 1;
            }
        }
    }

    return bRet;
}

void H264CABACInitEngine(PVIDEOH264DECODEINFO pDecodeInfo)
{
    pDecodeInfo->pCABAC->engine.codIRange = 510;
    pDecodeInfo->pCABAC->engine.codIOffset = H264ReadBits(pDecodeInfo->pReadBit, 9);
}

static VIDEOH264VALUE H264ReadAE_mb_skip_flag(PVIDEOH264DECODEINFO pDecodeInfo)
{
    VIDEOH264VALUE cValue;
    VIDEOH264SLICETYPE sliceType = pDecodeInfo->pPicInfo->slice.type;

    switch (sliceType) {
        case VIDEO_H264_SLICE_TYPE_P:
        case VIDEO_H264_SLICE_TYPE_SP: {
            cValue.u32 = H264ReadAEBinFL(pDecodeInfo, 1, H264ReadAEGetctxIdx_mb_skip_flag_PSlice);
        } break;
        case VIDEO_H264_SLICE_TYPE_B: {
            cValue.u32 = H264ReadAEBinFL(pDecodeInfo, 1, H264ReadAEGetctxIdx_mb_skip_flag_BSlice);
        } break;
        default: { cValue.u32 = 0; } break;
    }

    return cValue;
}
static VIDEOH264VALUE H264ReadAE_mb_field_decoding_flag(PVIDEOH264DECODEINFO pDecodeInfo)
{
    VIDEOH264VALUE cValue;
    
    cValue.u32 = H264ReadAEBinFL(pDecodeInfo, 1, H264ReadAEGetctxIdx_mb_field_decoding_flag);

    return cValue;
}
static VIDEOH264VALUE H264ReadAE_mb_type(PVIDEOH264DECODEINFO pDecodeInfo)
{
    VIDEOH264VALUE cValue;
    VIDEOH264SLICETYPE sliceType = pDecodeInfo->pPicInfo->slice.type;

    switch (sliceType) {
        case VIDEO_H264_SLICE_TYPE_SI: {
            cValue.u32 = H264ReadAEDecodeBin(pDecodeInfo, H264ReadAEGetctxIdx_mb_type_SISlice_Prefix(pDecodeInfo, 0));
            if (1 == cValue.u32)
            {
                cValue.u32 = H264ReadAEBinMBTypeI(pDecodeInfo, H264ReadAEGetctxIdx_mb_type_SISlice_Suffix) + 1;
            }
        } break;
        case VIDEO_H264_SLICE_TYPE_I: {
            cValue.u32 = H264ReadAEBinMBTypeI(pDecodeInfo, H264ReadAEGetctxIdx_mb_type_ISlice);
        } break;
        case VIDEO_H264_SLICE_TYPE_SP:
        case VIDEO_H264_SLICE_TYPE_P: {
            cValue.u32 = H264ReadAEBinMBTypeP(pDecodeInfo);
            if (5 == cValue.u32)
            {
                cValue.u32 = H264ReadAEBinMBTypeI(pDecodeInfo, H264ReadAEGetctxIdx_mb_type_PSlice_Suffix) + 5;
            }
        } break;
        case VIDEO_H264_SLICE_TYPE_B: {
            cValue.u32 = H264ReadAEBinMBTypeB(pDecodeInfo);
            if (23 == cValue.u32)
            {
                cValue.u32 = H264ReadAEBinMBTypeI(pDecodeInfo, H264ReadAEGetctxIdx_mb_type_BSlice_Suffix) + 23;
            }
        } break;
        default: { cValue.u32 = 0; } break;
    }

    return cValue;
}
static VIDEOH264VALUE H264ReadAE_transform_size_8x8_flag(PVIDEOH264DECODEINFO pDecodeInfo)
{
    VIDEOH264VALUE cValue;
    
    cValue.u32 = H264ReadAEBinFL(pDecodeInfo, 1, H264ReadAEGetctxIdx_transform_size_8x8_flag);

    return cValue;
}
static VIDEOH264VALUE H264ReadAE_coded_block_pattern(PVIDEOH264DECODEINFO pDecodeInfo)
{
    VIDEOH264VALUE cValue;
    uint32_t ChromaArrayType = pDecodeInfo->pPicInfo->slice.pSPS->ChromaArrayType;

    cValue.u32 = H264ReadAEBinFL(pDecodeInfo, 15, H264ReadAEGetctxIdx_coded_block_pattern_Prefix);
    if ((0 != ChromaArrayType) && (3 != ChromaArrayType))
    {
        cValue.u32 |= H264ReadAEBinTU(pDecodeInfo, 2, H264ReadAEGetctxIdx_coded_block_pattern_Suffix) << 4;
    }

    return cValue;
}
static VIDEOH264VALUE H264ReadAE_mb_qp_delta(PVIDEOH264DECODEINFO pDecodeInfo)
{
    VIDEOH264VALUE cValue;
    int32_t s32;

    cValue.u32 = H264ReadAEBinU(pDecodeInfo, H264ReadAEGetctxIdx_mb_qp_delta);
    if ((1 & cValue.u32) == 0) { s32 = -1; }
    else { s32 = 1; }
    cValue.s32 = s32 * ((cValue.u32 + 1) >> 1);

    return cValue;
}
static VIDEOH264VALUE H264ReadAE_end_of_slice_flag(PVIDEOH264DECODEINFO pDecodeInfo)
{
    VIDEOH264VALUE cValue;
    
    cValue.u32 = H264ReadAEBinFL(pDecodeInfo, 1, H264ReadAEGetctxIdx_end_of_slice_flag);

    return cValue;
}
static VIDEOH264VALUE H264ReadAE_sub_mb_type(PVIDEOH264DECODEINFO pDecodeInfo)
{
    VIDEOH264VALUE cValue;
    VIDEOH264SLICETYPE sliceType = pDecodeInfo->pPicInfo->slice.type;

    switch (sliceType) {
        case VIDEO_H264_SLICE_TYPE_P:
        case VIDEO_H264_SLICE_TYPE_SP: {
            cValue.u32 = H264ReadAEBinSubMBTypeP(pDecodeInfo);
        } break;
        case VIDEO_H264_SLICE_TYPE_B: {
            cValue.u32 = H264ReadAEBinSubMBTypeB(pDecodeInfo);
        } break;
        default: { cValue.u32 = 0; } break;
    }

    return cValue;
}
static VIDEOH264VALUE H264ReadAE_prev_intraNxN_pred_mode_flag(PVIDEOH264DECODEINFO pDecodeInfo)
{
    VIDEOH264VALUE cValue;
    
    cValue.u32 = H264ReadAEBinFL(pDecodeInfo, 1, H264ReadAEGetctxIdx_prev_intraNxN_pred_mode_flag);

    return cValue;
}
static VIDEOH264VALUE H264ReadAE_rem_intraNxN_pred_mode(PVIDEOH264DECODEINFO pDecodeInfo)
{
    VIDEOH264VALUE cValue;
    
    cValue.u32 = H264ReadAEBinFL(pDecodeInfo, 7, H264ReadAEGetctxIdx_rem_intraNxN_pred_mode);

    return cValue;
}
static VIDEOH264VALUE H264ReadAE_intra_chroma_pred_mode(PVIDEOH264DECODEINFO pDecodeInfo)
{
    VIDEOH264VALUE cValue;
    
    cValue.u32 = H264ReadAEBinTU(pDecodeInfo, 3, H264ReadAEGetctxIdx_intra_chroma_pred_mode);

    return cValue;
}
static VIDEOH264VALUE H264ReadAE_mvd_l0_0(PVIDEOH264DECODEINFO pDecodeInfo)
{
    return H264ReadAE_mvd_lX(pDecodeInfo, H264ReadAEGetctxIdx_mvd_l0_0_Prefix);
}
static VIDEOH264VALUE H264ReadAE_mvd_l1_0(PVIDEOH264DECODEINFO pDecodeInfo)
{
    return H264ReadAE_mvd_lX(pDecodeInfo, H264ReadAEGetctxIdx_mvd_l1_0_Prefix);
}
static VIDEOH264VALUE H264ReadAE_mvd_l0_1(PVIDEOH264DECODEINFO pDecodeInfo)
{
    return H264ReadAE_mvd_lX(pDecodeInfo, H264ReadAEGetctxIdx_mvd_l0_1_Prefix);
}
static VIDEOH264VALUE H264ReadAE_mvd_l1_1(PVIDEOH264DECODEINFO pDecodeInfo)
{
    return H264ReadAE_mvd_lX(pDecodeInfo, H264ReadAEGetctxIdx_mvd_l1_1_Prefix);
}
static inline VIDEOH264VALUE H264ReadAE_mvd_lX(PVIDEOH264DECODEINFO pDecodeInfo, GETCTXIDXCALLBACK GetctxIdx)
{
    VIDEOH264VALUE cValue;

    /* signedValFlag = 1（有符号）, uCoff = 9 */
    cValue.u32 = H264ReadAEBinTU(pDecodeInfo, 9, GetctxIdx);
    if (0 != cValue.u32)
    {
        cValue.u32 += H264ReadAEBinUEGk(pDecodeInfo, H264ReadAEGetctxIdx_mvd_lX_Suffix);
    }
    if (0 != H264ReadAEDecodeBin(pDecodeInfo, VIDEO_H264_CABAC_BYPASS)) { cValue.s32 = -cValue.s32; }

    return cValue;
}
static VIDEOH264VALUE H264ReadAE_ref_idx_l0(PVIDEOH264DECODEINFO pDecodeInfo)
{
    VIDEOH264VALUE cValue;
    
    cValue.u32 = H264ReadAEBinU(pDecodeInfo, H264ReadAEGetctxIdx_ref_idx_l0);

    return cValue;
}
static VIDEOH264VALUE H264ReadAE_ref_idx_l1(PVIDEOH264DECODEINFO pDecodeInfo)
{
    VIDEOH264VALUE cValue;
    
    cValue.u32 = H264ReadAEBinU(pDecodeInfo, H264ReadAEGetctxIdx_ref_idx_l1);

    return cValue;
}
static VIDEOH264VALUE H264ReadAE_coeff_sign_flag(PVIDEOH264DECODEINFO pDecodeInfo)
{
    VIDEOH264VALUE cValue;
    
    cValue.u32 = H264ReadAEBinFL(pDecodeInfo, 1, H264ReadAEGetctxIdx_coeff_sign_flag);

    return cValue;
}
static uint32_t H264ReadAEBinU(PVIDEOH264DECODEINFO pDecodeInfo, GETCTXIDXCALLBACK GetctxIdx)
{
    uint32_t nRet = 0;

    while (0 != H264ReadAEDecodeBin(pDecodeInfo, GetctxIdx(pDecodeInfo, nRet))) { nRet++; }

    return nRet;
}
static uint32_t H264ReadAEBinTU(PVIDEOH264DECODEINFO pDecodeInfo, uint32_t cMax, GETCTXIDXCALLBACK GetctxIdx)
{
    uint32_t nRet = 0;

    for (uint8_t binIdx = 0; binIdx < cMax; binIdx++)
    {
        if (0 != H264ReadAEDecodeBin(pDecodeInfo, GetctxIdx(pDecodeInfo, binIdx))) { nRet++; }
        else { break; }
    }

    return nRet;
}
static uint32_t H264ReadAEBinUEGk(PVIDEOH264DECODEINFO pDecodeInfo, GETCTXIDXCALLBACK GetctxIdx)
{
    (void)GetctxIdx; // 指数哥伦布码的后缀都使用了 Bypass 编码，等收录到了需要 ctxIdx 的时候再补逻辑，这里保留参数，入参的时候可以提供函数指针，也可以传入 NULL
    uint32_t nLen = 0;
    uint32_t value1;
    uint32_t value2;

    /* k 阶指数哥伦布码先读取前缀连续的 1，到 0 截止，这里读到的连续 1 就是值，而非将 1 个数作为值，比如读到 4 个 1，那么值就是 15 */
    while (1 == H264ReadAEDecodeBin(pDecodeInfo, VIDEO_H264_CABAC_BYPASS)) { nLen++; }
    value1 = (1 << nLen) - 1;
    /* 然后读取 0 后面的部分，个数和前面的 1 的个数一样 */
    value2 = 0;
    for (uint32_t iLoop = 0; iLoop < nLen; iLoop++)
    {
        value2 = (value2 << 1) + H264ReadAEDecodeBin(pDecodeInfo, VIDEO_H264_CABAC_BYPASS);
    }
    /* 返回两都的和 */
    return (value1 + value2);
}
static uint32_t H264ReadAEBinFL(PVIDEOH264DECODEINFO pDecodeInfo, uint32_t cMax, GETCTXIDXCALLBACK GetctxIdx)
{
    uint32_t nLen = H264CeilLog2(cMax);

    pDecodeInfo->pCABAC->cValue.u32 = 0;
    for (uint8_t binIdx = 0; binIdx < nLen; binIdx++)
    {
        pDecodeInfo->pCABAC->cValue.u32 |= (H264ReadAEDecodeBin(pDecodeInfo, GetctxIdx(pDecodeInfo, binIdx)) << binIdx);
    }

    return pDecodeInfo->pCABAC->cValue.u32;
}
static uint32_t H264ReadAEBinMBTypeI(PVIDEOH264DECODEINFO pDecodeInfo, GETCTXIDXCALLBACK GetctxIdx)
{
    uint32_t nRet = 0;
    uint8_t binString = 0;
    bool bBreak = false;
    
    for (uint8_t binIdx = 0; binIdx < 7; binIdx++)
    {
        binString = (binString << 1) | H264ReadAEDecodeBin(pDecodeInfo, GetctxIdx(pDecodeInfo, binIdx));
        pDecodeInfo->pCABAC->cValue.u32 = binString;
        for (uint32_t iLoop = 0; iLoop < ArrLen(sg_pMacroBlockTypeIBinStringInfo); iLoop++)
        {
            if ((sg_pMacroBlockTypeIBinStringInfo[iLoop].binString == binString) && (sg_pMacroBlockTypeIBinStringInfo[iLoop].maxBinId == binIdx))
            {
                nRet = iLoop;
                bBreak = true;
                break;
            }
        }
        if (true == bBreak) { break; }
    }

    return nRet;
}
static uint32_t H264ReadAEBinMBTypeP(PVIDEOH264DECODEINFO pDecodeInfo)
{
    uint32_t nRet = 0;
    uint8_t binString = 0;
    bool bBreak = false;
    
    for (uint8_t binIdx = 0; binIdx < 3; binIdx++)
    {
        binString = (binString << 1) | H264ReadAEDecodeBin(pDecodeInfo, H264ReadAEGetctxIdx_mb_type_PSlice_Prefix(pDecodeInfo, binIdx));
        pDecodeInfo->pCABAC->cValue.u32 = binString;
        for (uint32_t iLoop = 0; iLoop < ArrLen(sg_pMacroBlockTypePAndSPBinStringInfo); iLoop++)
        {
            if ((sg_pMacroBlockTypePAndSPBinStringInfo[iLoop].binString == binString) && (sg_pMacroBlockTypePAndSPBinStringInfo[iLoop].maxBinId == binIdx))
            {
                nRet = iLoop;
                bBreak = true;
                break;
            }
        }
        if (true == bBreak) { break; }
    }

    return nRet;
}
static uint32_t H264ReadAEBinMBTypeB(PVIDEOH264DECODEINFO pDecodeInfo)
{
    uint32_t nRet = 0;
    uint8_t binString = 0;
    bool bBreak = false;
    
    for (uint8_t binIdx = 0; binIdx < 3; binIdx++)
    {
        binString = (binString << 1) | H264ReadAEDecodeBin(pDecodeInfo, H264ReadAEGetctxIdx_mb_type_BSlice_Prefix(pDecodeInfo, binIdx));
        pDecodeInfo->pCABAC->cValue.u32 = binString;
        for (uint32_t iLoop = 0; iLoop < ArrLen(sg_pMacroBlockTypeBBinStringInfo); iLoop++)
        {
            if ((sg_pMacroBlockTypeBBinStringInfo[iLoop].binString == binString) && (sg_pMacroBlockTypeBBinStringInfo[iLoop].maxBinId == binIdx))
            {
                nRet = iLoop;
                bBreak = true;
                break;
            }
        }
        if (true == bBreak) { break; }
    }

    return nRet;
}
static uint32_t H264ReadAEBinSubMBTypeP(PVIDEOH264DECODEINFO pDecodeInfo)
{
    uint32_t nRet = 0;
    uint8_t binString = 0;
    bool bBreak = false;
    
    for (uint8_t binIdx = 0; binIdx < 3; binIdx++)
    {
        binString = (binString << 1) | H264ReadAEDecodeBin(pDecodeInfo, H264ReadAEGetctxIdx_sub_mb_type_PSlice(pDecodeInfo, binIdx));
        pDecodeInfo->pCABAC->cValue.u32 = binString;
        for (uint32_t iLoop = 0; iLoop < ArrLen(sg_pSubMacroBlockTypePAndSPBinStringInfo); iLoop++)
        {
            if ((sg_pSubMacroBlockTypePAndSPBinStringInfo[iLoop].binString == binString) && (sg_pSubMacroBlockTypePAndSPBinStringInfo[iLoop].maxBinId == binIdx))
            {
                nRet = iLoop;
                bBreak = true;
                break;
            }
        }
        if (true == bBreak) { break; }
    }

    return nRet;
}
static uint32_t H264ReadAEBinSubMBTypeB(PVIDEOH264DECODEINFO pDecodeInfo)
{
    uint32_t nRet = 0;
    uint8_t binString = 0;
    bool bBreak = false;
    
    for (uint8_t binIdx = 0; binIdx < 3; binIdx++)
    {
        binString = (binString << 1) | H264ReadAEDecodeBin(pDecodeInfo, H264ReadAEGetctxIdx_sub_mb_type_BSlice(pDecodeInfo, binIdx));
        pDecodeInfo->pCABAC->cValue.u32 = binString;
        for (uint32_t iLoop = 0; iLoop < ArrLen(sg_pSubMacroBlockTypeBBinStringInfo); iLoop++)
        {
            if ((sg_pSubMacroBlockTypeBBinStringInfo[iLoop].binString == binString) && (sg_pSubMacroBlockTypeBBinStringInfo[iLoop].maxBinId == binIdx))
            {
                nRet = iLoop;
                bBreak = true;
                break;
            }
        }
        if (true == bBreak) { break; }
    }

    return nRet;
}

static VIDEOH264VALUE H264ReadAE_coded_block_flag(PVIDEOH264DECODEINFO pDecodeInfo)
{
    VIDEOH264VALUE cValue;
    
    cValue.u32 = H264ReadAEBinFL(pDecodeInfo, 1, H264ReadAEGetctxIdx_coded_block_flag);

    return cValue;
}
static VIDEOH264VALUE H264ReadAE_significant_coeff_flag(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t levelListIdx)
{
    VIDEOH264VALUE cValue;

    pDecodeInfo->levelListIdx = levelListIdx;
    cValue.u32 = H264ReadAEBinFL(pDecodeInfo, 1, H264ReadAEGetctxIdx_significant_coeff_flag);

    return cValue;
}
static VIDEOH264VALUE H264ReadAE_last_significant_coeff_flag(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t levelListIdx)
{
    VIDEOH264VALUE cValue;

    pDecodeInfo->levelListIdx = levelListIdx;
    cValue.u32 = H264ReadAEBinFL(pDecodeInfo, 1, H264ReadAEGetctxIdx_last_significant_coeff_flag);

    return cValue;
}
static VIDEOH264VALUE H264ReadAE_coeff_abs_level_minus1(PVIDEOH264DECODEINFO pDecodeInfo)
{
    VIDEOH264VALUE cValue;

    /* signedValFlag = 0（无符号）, uCoff = 14 */
    cValue.u32 = H264ReadAEBinTU(pDecodeInfo, 14, H264ReadAEGetctxIdx_coeff_abs_level_minus1_Prefix);
    if (14 == cValue.u32)
    {
        cValue.u32 += H264ReadAEBinUEGk(pDecodeInfo, H264ReadAEGetctxIdx_coeff_abs_level_minus1_Suffix);
    }

    return cValue;
}

static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mb_type_SISlice_Prefix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    VIDEOH264CABACCTXIDX ctxIdx;
    VIDEOH264CABACCTXIDX ctxIdxOffset = 0;

    /* SI 的 mb_type 前缀只读 1 比特，其它的 binIdx 无意义 */
    switch (binIdx) {
        case 0: { ctxIdx = ctxIdxOffset + H264ReadAEGetctxIdxInc_mb_type(pDecodeInfo, ctxIdxOffset); } break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        default: { ctxIdx = (VIDEOH264CABACCTXIDX)VIDEO_H264_CABAC_BYPASS; } break; // >= 6
    }

    return ctxIdx;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mb_type_SISlice_Suffix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    /* SI 的 mb_type 后缀缀完全等于 I 帧的 mb_type */
    return H264ReadAEGetctxIdx_mb_type_ISlice(pDecodeInfo, binIdx);
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mb_type_ISlice(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    VIDEOH264CABACCTXIDX ctxIdx;
    VIDEOH264CABACCTXIDX ctxIdxOffset = 3;

    switch (binIdx) {
        case 0: { ctxIdx = ctxIdxOffset + H264ReadAEGetctxIdxInc_mb_type(pDecodeInfo, ctxIdxOffset); } break;
        case 1: { ctxIdx = 276; } break;
        case 2: { ctxIdx = ctxIdxOffset + 3; } break;
        case 3: { ctxIdx = ctxIdxOffset + 4; } break;
        case 4:
        case 5: { ctxIdx = ctxIdxOffset + H264ReadAEGetctxIdxInc_prior_decoded_bin(pDecodeInfo, binIdx, ctxIdxOffset); } break;
        default: { ctxIdx = ctxIdxOffset + 7; } break; // >= 6
    }

    return ctxIdx;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mb_skip_flag_PSlice(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    VIDEOH264CABACCTXIDX ctxIdx;
    VIDEOH264CABACCTXIDX ctxIdxOffset = 11;

    /* mb_skip_flag 只读 1 比特，其它的 binIdx 无意义 */
    switch (binIdx) {
        case 0: { ctxIdx = ctxIdxOffset + H264ReadAEGetctxIdxInc_mb_skip_flag(pDecodeInfo); } break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        default: { ctxIdx = (VIDEOH264CABACCTXIDX)VIDEO_H264_CABAC_BYPASS; } break; // >= 6
    }

    return ctxIdx;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mb_type_PSlice_Prefix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    VIDEOH264CABACCTXIDX ctxIdx;
    VIDEOH264CABACCTXIDX ctxIdxOffset = 14;

    /* P/SP 帧的 mb_type 前缀最多读 3 比特，其它的 binIdx 无意义 */
    switch (binIdx) {
        case 0: { ctxIdx = ctxIdxOffset + H264ReadAEGetctxIdxInc_mb_type(pDecodeInfo, ctxIdxOffset); } break;
        case 1: { ctxIdx = ctxIdxOffset + 3; } break;
        case 2: { ctxIdx = ctxIdxOffset + H264ReadAEGetctxIdxInc_prior_decoded_bin(pDecodeInfo, binIdx, ctxIdxOffset); } break;
        case 3:
        case 4:
        case 5:
        default: { ctxIdx = (VIDEOH264CABACCTXIDX)VIDEO_H264_CABAC_BYPASS; } break; // >= 6
    }

    return ctxIdx;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mb_type_PSlice_Suffix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    VIDEOH264CABACCTXIDX ctxIdx;
    VIDEOH264CABACCTXIDX ctxIdxOffset = 17;

    /* P/SP 帧的 mb_type 后缀 maxBinIdxCtx = 5 */
    switch (binIdx) {
        case 0: { ctxIdx = ctxIdxOffset + 0; } break;
        case 1: { ctxIdx = 276; } break;
        case 2: { ctxIdx = ctxIdxOffset + 1; } break;
        case 3: { ctxIdx = ctxIdxOffset + 2; } break;
        case 4: { ctxIdx = ctxIdxOffset + H264ReadAEGetctxIdxInc_prior_decoded_bin(pDecodeInfo, binIdx, ctxIdxOffset); } break;
        case 5:
        default: { ctxIdx = ctxIdxOffset + 3; } break; // >= 6
    }

    return ctxIdx;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_sub_mb_type_PSlice(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    (void)pDecodeInfo;
    VIDEOH264CABACCTXIDX ctxIdx;
    VIDEOH264CABACCTXIDX ctxIdxOffset = 21;

    /* P/SP 帧的 sub_mb_type 只读 3 比特，其它的 binIdx 无意义 */
    switch (binIdx) {
        case 0: { ctxIdx = ctxIdxOffset + 0; } break;
        case 1: { ctxIdx = ctxIdxOffset + 1; } break;
        case 2: { ctxIdx = ctxIdxOffset + 2; } break;
        case 3:
        case 4:
        case 5:
        default: { ctxIdx = (VIDEOH264CABACCTXIDX)VIDEO_H264_CABAC_BYPASS; } break; // >= 6
    }

    return ctxIdx;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mb_skip_flag_BSlice(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    VIDEOH264CABACCTXIDX ctxIdx;
    VIDEOH264CABACCTXIDX ctxIdxOffset = 24;

    /* mb_skip_flag 只读 1 比特，其它的 binIdx 无意义 */
    switch (binIdx) {
        case 0: { ctxIdx = ctxIdxOffset + H264ReadAEGetctxIdxInc_mb_skip_flag(pDecodeInfo); } break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        default: { ctxIdx = (VIDEOH264CABACCTXIDX)VIDEO_H264_CABAC_BYPASS; } break; // >= 6
    }

    return ctxIdx;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mb_type_BSlice_Prefix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    VIDEOH264CABACCTXIDX ctxIdx;
    VIDEOH264CABACCTXIDX ctxIdxOffset = 27;

    /* B 帧的 mb_type 前缀最多读 6 比特，但 maxBinIdxCtx 等于 3 */
    switch (binIdx) {
        case 0: { ctxIdx = ctxIdxOffset + 0; } break;
        case 1: { ctxIdx = ctxIdxOffset + 1; } break;
        case 2: { ctxIdx = ctxIdxOffset + H264ReadAEGetctxIdxInc_prior_decoded_bin(pDecodeInfo, binIdx, ctxIdxOffset); } break;
        case 3:
        case 4:
        case 5:
        default: { ctxIdx = ctxIdxOffset + 5; } break; // >= 6
    }

    return ctxIdx;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mb_type_BSlice_Suffix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    VIDEOH264CABACCTXIDX ctxIdx;
    VIDEOH264CABACCTXIDX ctxIdxOffset = 32;

    /* B 帧的 mb_type 后缀最多读 6 比特，但 maxBinIdxCtx 等于 5 */
    switch (binIdx) {
        case 0: { ctxIdx = ctxIdxOffset + 0; } break;
        case 1: { ctxIdx = 276; } break;
        case 2: { ctxIdx = ctxIdxOffset + 1; } break;
        case 3: { ctxIdx = ctxIdxOffset + 2; } break;
        case 4: { ctxIdx = ctxIdxOffset + H264ReadAEGetctxIdxInc_prior_decoded_bin(pDecodeInfo, binIdx, ctxIdxOffset); } break;
        case 5:
        default: { ctxIdx = ctxIdxOffset + 3; } break; // >= 6
    }

    return ctxIdx;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_sub_mb_type_BSlice(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    VIDEOH264CABACCTXIDX ctxIdx;
    VIDEOH264CABACCTXIDX ctxIdxOffset = 36;

    /* B 帧的 mb_type 前缀最多读 6 比特，但 maxBinIdxCtx 等于 3 */
    switch (binIdx) {
        case 0: { ctxIdx = ctxIdxOffset + 0; } break;
        case 1: { ctxIdx = ctxIdxOffset + 1; } break;
        case 2: { ctxIdx = ctxIdxOffset + H264ReadAEGetctxIdxInc_prior_decoded_bin(pDecodeInfo, binIdx, ctxIdxOffset); } break;
        case 3:
        case 4:
        case 5: { ctxIdx = ctxIdxOffset + 3; } break;
        default: { ctxIdx = (VIDEOH264CABACCTXIDX)VIDEO_H264_CABAC_BYPASS; } break; // >= 6
    }

    return ctxIdx;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mvd_l0_0_Prefix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    return H264ReadAEGetctxIdx_mvd_lX_Prefix(pDecodeInfo, binIdx, 40, 0);
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mvd_l1_0_Prefix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    return H264ReadAEGetctxIdx_mvd_lX_Prefix(pDecodeInfo, binIdx, 40, 1);
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mvd_l0_1_Prefix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    return H264ReadAEGetctxIdx_mvd_lX_Prefix(pDecodeInfo, binIdx, 47, 0);
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mvd_l1_1_Prefix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    return H264ReadAEGetctxIdx_mvd_lX_Prefix(pDecodeInfo, binIdx, 47, 1);
}
static inline VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mvd_lX_Prefix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx, VIDEOH264CABACCTXIDX ctxIdxOffset, uint8_t lX)
{
    VIDEOH264CABACCTXIDX ctxIdx;

    /* mvd_lX[][][0] 的前缀的 maxBinIdxCtx 等于 4 */
    switch (binIdx) {
        case 0: { ctxIdx = ctxIdxOffset + H264ReadAEGetctxIdxInc_mvd_lX(pDecodeInfo, lX, ctxIdxOffset); } break;
        case 1: { ctxIdx = ctxIdxOffset + 3; } break;
        case 2: { ctxIdx = ctxIdxOffset + 4; } break;
        case 3: { ctxIdx = ctxIdxOffset + 5; } break;
        case 4:
        case 5:
        default: { ctxIdx = ctxIdxOffset + 6; } break; // >= 6
    }

    return ctxIdx;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mvd_lX_Suffix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    (void)pDecodeInfo;
    (void)binIdx;
    return (VIDEOH264CABACCTXIDX)VIDEO_H264_CABAC_BYPASS;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_ref_idx_l0(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    VIDEOH264CABACCTXIDX ctxIdx;
    VIDEOH264CABACCTXIDX ctxIdxOffset = 54;

    /* ref_idx_lX 的 maxBinIdxCtx 等于 2 */
    switch (binIdx) {
        case 0: { ctxIdx = ctxIdxOffset + H264ReadAEGetctxIdxInc_ref_idx_lX(pDecodeInfo, 0); } break;
        case 1: { ctxIdx = ctxIdxOffset + 4; } break;
        case 2:
        case 3:
        case 4:
        case 5:
        default: { ctxIdx = ctxIdxOffset + 5; } break; // >= 6
    }

    return ctxIdx;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_ref_idx_l1(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    VIDEOH264CABACCTXIDX ctxIdx;
    VIDEOH264CABACCTXIDX ctxIdxOffset = 54;

    /* ref_idx_lX 的 maxBinIdxCtx 等于 2 */
    switch (binIdx) {
        case 0: { ctxIdx = ctxIdxOffset + H264ReadAEGetctxIdxInc_ref_idx_lX(pDecodeInfo, 1); } break;
        case 1: { ctxIdx = ctxIdxOffset + 4; } break;
        case 2:
        case 3:
        case 4:
        case 5:
        default: { ctxIdx = ctxIdxOffset + 5; } break; // >= 6
    }

    return ctxIdx;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mb_qp_delta(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    VIDEOH264CABACCTXIDX ctxIdx;
    VIDEOH264CABACCTXIDX ctxIdxOffset = 60;

    /* mb_qp_delta 理论上可以读几十比特，但 maxBinIdxCtx 等于 2 */
    switch (binIdx) {
        case 0: { ctxIdx = ctxIdxOffset + H264ReadAEGetctxIdxInc_mb_qp_delta(pDecodeInfo); } break;
        case 1: { ctxIdx = ctxIdxOffset + 2; } break;
        case 2:
        case 3:
        case 4:
        case 5:
        default: { ctxIdx = ctxIdxOffset + 3; } break; // >= 6
    }

    return ctxIdx;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_intra_chroma_pred_mode(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    VIDEOH264CABACCTXIDX ctxIdx;
    VIDEOH264CABACCTXIDX ctxIdxOffset = 64;

    /* mb_skip_flag 最多读 2 比特，其它的 binIdx 无意义，不过表格中给到了 binIdx = 2，而 maxBinIdxCtx 等于 1，所以写上也无所谓 */
    switch (binIdx) {
        case 0: { ctxIdx = ctxIdxOffset + H264ReadAEGetctxIdxInc_intra_chroma_pred_mode(pDecodeInfo); } break;
        case 1:
        case 2: { ctxIdx = ctxIdxOffset + 3; } break;
        case 3:
        case 4:
        case 5:
        default: { ctxIdx = (VIDEOH264CABACCTXIDX)VIDEO_H264_CABAC_BYPASS; } break; // >= 6
    }

    return ctxIdx;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_prev_intraNxN_pred_mode_flag(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    (void)pDecodeInfo;
    VIDEOH264CABACCTXIDX ctxIdx;
    VIDEOH264CABACCTXIDX ctxIdxOffset = 68;

    /* intraNxN_pred_mode_flag 只读 1 比特，其它的 binIdx 无意义 */
    switch (binIdx) {
        case 0: { ctxIdx = ctxIdxOffset + 0; } break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        default: { ctxIdx = (VIDEOH264CABACCTXIDX)VIDEO_H264_CABAC_BYPASS; } break; // >= 6
    }

    return ctxIdx;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_rem_intraNxN_pred_mode(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    (void)pDecodeInfo;
    VIDEOH264CABACCTXIDX ctxIdx;
    VIDEOH264CABACCTXIDX ctxIdxOffset = 69;

    /* rem_intraNxN_pred_mode 只读 3 比特，其它的 binIdx 无意义 */
    switch (binIdx) {
        case 0:
        case 1:
        case 2: { ctxIdx = ctxIdxOffset + 0; } break;
        case 3:
        case 4:
        case 5:
        default: { ctxIdx = (VIDEOH264CABACCTXIDX)VIDEO_H264_CABAC_BYPASS; } break; // >= 6
    }

    return ctxIdx;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_mb_field_decoding_flag(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    VIDEOH264CABACCTXIDX ctxIdx;
    VIDEOH264CABACCTXIDX ctxIdxOffset = 70;

    /* mb_field_decoding_flag 只读 1 比特，其它的 binIdx 无意义 */
    switch (binIdx) {
        case 0: { ctxIdx = ctxIdxOffset + H264ReadAEGetctxIdxInc_mb_field_decoding_flag(pDecodeInfo); } break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        default: { ctxIdx = (VIDEOH264CABACCTXIDX)VIDEO_H264_CABAC_BYPASS; } break; // >= 6
    }

    return ctxIdx;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_coded_block_pattern_Prefix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    VIDEOH264CABACCTXIDX ctxIdx;
    VIDEOH264CABACCTXIDX ctxIdxOffset = 73;

    /* coded_block_pattern 前缀只读 4 比特，其它的 binIdx 无意义 */
    switch (binIdx) {
        case 0:
        case 1:
        case 2:
        case 3: { ctxIdx = ctxIdxOffset + H264ReadAEGetctxIdxInc_coded_block_pattern(pDecodeInfo, binIdx, ctxIdxOffset); } break;
        case 4:
        case 5:
        default: { ctxIdx = (VIDEOH264CABACCTXIDX)VIDEO_H264_CABAC_BYPASS; } break; // >= 6
    }

    return ctxIdx;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_coded_block_pattern_Suffix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    VIDEOH264CABACCTXIDX ctxIdx;
    VIDEOH264CABACCTXIDX ctxIdxOffset = 77;

    /* coded_block_pattern 后缀只读 2 比特，其它的 binIdx 无意义 */
    switch (binIdx) {
        case 0:
        case 1: { ctxIdx = ctxIdxOffset + H264ReadAEGetctxIdxInc_coded_block_pattern(pDecodeInfo, binIdx, ctxIdxOffset); } break;
        case 2:
        case 3:
        case 4:
        case 5:
        default: { ctxIdx = (VIDEOH264CABACCTXIDX)VIDEO_H264_CABAC_BYPASS; } break; // >= 6
    }

    return ctxIdx;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_coeff_sign_flag(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    (void)pDecodeInfo;
    (void)binIdx;
    return (VIDEOH264CABACCTXIDX)VIDEO_H264_CABAC_BYPASS;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_end_of_slice_flag(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    (void)pDecodeInfo;
    VIDEOH264CABACCTXIDX ctxIdx;
    VIDEOH264CABACCTXIDX ctxIdxOffset = 276;

    /* end_of_slice_flag 只读 1 比特，其它的 binIdx 无意义 */
    switch (binIdx) {
        case 0: { ctxIdx = ctxIdxOffset + 0; } break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        default: { ctxIdx = (VIDEOH264CABACCTXIDX)VIDEO_H264_CABAC_BYPASS; } break; // >= 6
    }

    return ctxIdx;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_transform_size_8x8_flag(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    VIDEOH264CABACCTXIDX ctxIdx;
    VIDEOH264CABACCTXIDX ctxIdxOffset = 399;

    /* transform_size_8x8_flag 只读 1 比特，其它的 binIdx 无意义 */
    switch (binIdx) {
        case 0: { ctxIdx = ctxIdxOffset + H264ReadAEGetctxIdxInc_transform_size_8x8_flag(pDecodeInfo); } break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        default: { ctxIdx = (VIDEOH264CABACCTXIDX)VIDEO_H264_CABAC_BYPASS; } break; // >= 6
    }

    return ctxIdx;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_coded_block_flag(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    (void)binIdx;
    uint8_t ctxBlockCat = sg_pCtxBlockCatValue[pDecodeInfo->blockCat];
    VIDEOH264CABACCTXIDX ctxIdxOffset;
    VIDEOH264CABACCTXIDX ctxIdxBlockCatOffset = sg_pCtxIdxBlockCatOffset_coded_block_flag[ctxBlockCat];
    
    switch (ctxBlockCat) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4: { ctxIdxOffset = 85; } break; // ctxBlockCat < 5
        case 6:
        case 7:
        case 8: { ctxIdxOffset = 460; } break; // 5 < ctxBlockCat < 9
        case 10:
        case 11:
        case 12: { ctxIdxOffset = 472; } break; // 9 < ctxBlockCat < 13
        default: { ctxIdxOffset = 1012; } break; // ctxBlockCat == 5, 9, or 13
    }

    return (ctxIdxOffset + ctxIdxBlockCatOffset + H264ReadAEGetctxIdxInc_coded_block_flag(pDecodeInfo, ctxBlockCat));
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_significant_coeff_flag(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    (void)binIdx;
    uint8_t ctxBlockCat = sg_pCtxBlockCatValue[pDecodeInfo->blockCat];
    VIDEOH264CABACCTXIDX ctxIdxOffset;
    VIDEOH264CABACCTXIDX ctxIdxBlockCatOffset = sg_pCtxIdxBlockCatOffset_significant_coeff_flag[ctxBlockCat];
    
    switch (ctxBlockCat) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4: { // ctxBlockCat < 5
            if (VIDEO_H264_FRAME_FIELD_MODE_FRAME == GetMacroBlockFrameFiledMode(pDecodeInfo, GetCurrentMacroBlock(pDecodeInfo))) { ctxIdxOffset = 105; }
            else { ctxIdxOffset = 277; }
        } break;
        case 5: { // ctxBlockCat == 5
            if (VIDEO_H264_FRAME_FIELD_MODE_FRAME == GetMacroBlockFrameFiledMode(pDecodeInfo, GetCurrentMacroBlock(pDecodeInfo))) { ctxIdxOffset = 402; }
            else { ctxIdxOffset = 436; }
        } break;
        case 6:
        case 7:
        case 8: { // 5 < ctxBlockCat < 9
            if (VIDEO_H264_FRAME_FIELD_MODE_FRAME == GetMacroBlockFrameFiledMode(pDecodeInfo, GetCurrentMacroBlock(pDecodeInfo))) { ctxIdxOffset = 484; }
            else { ctxIdxOffset = 776; }
        } break;
        case 10:
        case 11:
        case 12: { // 9 < ctxBlockCat < 13
            if (VIDEO_H264_FRAME_FIELD_MODE_FRAME == GetMacroBlockFrameFiledMode(pDecodeInfo, GetCurrentMacroBlock(pDecodeInfo))) { ctxIdxOffset = 528; }
            else { ctxIdxOffset = 820; }
        } break;
        case 9: { // ctxBlockCat == 9
            if (VIDEO_H264_FRAME_FIELD_MODE_FRAME == GetMacroBlockFrameFiledMode(pDecodeInfo, GetCurrentMacroBlock(pDecodeInfo))) { ctxIdxOffset = 660; }
            else { ctxIdxOffset = 675; }
        } break;
        default: { // ctxBlockCat == 13
            if (VIDEO_H264_FRAME_FIELD_MODE_FRAME == GetMacroBlockFrameFiledMode(pDecodeInfo, GetCurrentMacroBlock(pDecodeInfo))) { ctxIdxOffset = 718; }
            else { ctxIdxOffset = 733; }
        } break;
    }

    return (ctxIdxOffset + ctxIdxBlockCatOffset + H264ReadAEGetctxIdxInc_significant_coeff_flag(pDecodeInfo, ctxBlockCat));
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_last_significant_coeff_flag(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    (void)binIdx;
    uint8_t ctxBlockCat = sg_pCtxBlockCatValue[pDecodeInfo->blockCat];
    VIDEOH264CABACCTXIDX ctxIdxOffset;
    VIDEOH264CABACCTXIDX ctxIdxBlockCatOffset = sg_pCtxIdxBlockCatOffset_last_significant_coeff_flag[ctxBlockCat];
    
    switch (ctxBlockCat) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4: { // ctxBlockCat < 5
            if (VIDEO_H264_FRAME_FIELD_MODE_FRAME == GetMacroBlockFrameFiledMode(pDecodeInfo, GetCurrentMacroBlock(pDecodeInfo))) { ctxIdxOffset = 166; }
            else { ctxIdxOffset = 338; }
        } break;
        case 5: { // ctxBlockCat == 5
            if (VIDEO_H264_FRAME_FIELD_MODE_FRAME == GetMacroBlockFrameFiledMode(pDecodeInfo, GetCurrentMacroBlock(pDecodeInfo))) { ctxIdxOffset = 417; }
            else { ctxIdxOffset = 451; }
        } break;
        case 6:
        case 7:
        case 8: { // 5 < ctxBlockCat < 9
            if (VIDEO_H264_FRAME_FIELD_MODE_FRAME == GetMacroBlockFrameFiledMode(pDecodeInfo, GetCurrentMacroBlock(pDecodeInfo))) { ctxIdxOffset = 572; }
            else { ctxIdxOffset = 864; }
        } break;
        case 10:
        case 11:
        case 12: { // 9 < ctxBlockCat < 13
            if (VIDEO_H264_FRAME_FIELD_MODE_FRAME == GetMacroBlockFrameFiledMode(pDecodeInfo, GetCurrentMacroBlock(pDecodeInfo))) { ctxIdxOffset = 616; }
            else { ctxIdxOffset = 908; }
        } break;
        case 9: { // ctxBlockCat == 9
            if (VIDEO_H264_FRAME_FIELD_MODE_FRAME == GetMacroBlockFrameFiledMode(pDecodeInfo, GetCurrentMacroBlock(pDecodeInfo))) { ctxIdxOffset = 690; }
            else { ctxIdxOffset = 699; }
        } break;
        default: { // ctxBlockCat == 13
            if (VIDEO_H264_FRAME_FIELD_MODE_FRAME == GetMacroBlockFrameFiledMode(pDecodeInfo, GetCurrentMacroBlock(pDecodeInfo))) { ctxIdxOffset = 748; }
            else { ctxIdxOffset = 757; }
        } break;
    }

    return (ctxIdxOffset + ctxIdxBlockCatOffset + H264ReadAEGetctxIdxInc_last_significant_coeff_flag(pDecodeInfo, ctxBlockCat));
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_coeff_abs_level_minus1_Prefix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    uint8_t ctxBlockCat = sg_pCtxBlockCatValue[pDecodeInfo->blockCat];
    VIDEOH264CABACCTXIDX ctxIdxOffset;
    VIDEOH264CABACCTXIDX ctxIdxBlockCatOffset = sg_pCtxIdxBlockCatOffset_coeff_abs_level_minus1[ctxBlockCat];
    
    switch (ctxBlockCat) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4: { ctxIdxOffset = 227; } break; //  ctxBlockCat < 5
        case 5: { ctxIdxOffset = 426; } break; //  ctxBlockCat == 5
        case 6:
        case 7:
        case 8: { ctxIdxOffset = 952; } break; // 5 < ctxBlockCat < 9
        case 10:
        case 11:
        case 12: { ctxIdxOffset = 982; } break; // 9 < ctxBlockCat < 13
        case 9: { ctxIdxOffset = 708; } break; // ctxBlockCat == 9
        default: { ctxIdxOffset = 766; } break; // ctxBlockCat == 13
    }

    return (ctxIdxOffset + ctxIdxBlockCatOffset + H264ReadAEGetctxIdxInc_coeff_abs_level_minus1(pDecodeInfo, binIdx, ctxBlockCat));
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdx_coeff_abs_level_minus1_Suffix(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx)
{
    (void)pDecodeInfo;
    (void)binIdx;
    return (VIDEOH264CABACCTXIDX)VIDEO_H264_CABAC_BYPASS;
}

static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdxInc_mb_skip_flag(PVIDEOH264DECODEINFO pDecodeInfo)
{
    PVIDEOH264PICTUREINFO pPicInfo = pDecodeInfo->pPicInfo;
    PVIDEOH264MACROBLOCK pMacroBlock = pPicInfo->pMacroBlock;
    HANSIZE mbAddrN[2];
    VIDEOH264CABACCTXIDX condTermFlagN[2];

    DerivationProcessForNeighbouringMacroblocks(pDecodeInfo, mbAddrN);

    for (HANSIZE iLoop = 0; iLoop < 2; iLoop++)
    {
        if (VIDEO_H264_MACRO_BLOCK_ADDR_INVALID != mbAddrN[iLoop])
        {
            if (0 == pMacroBlock[mbAddrN[iLoop]].mb_skip_flag) { condTermFlagN[iLoop] = 1; }
            else { condTermFlagN[iLoop] = 0; }
        }
        else { condTermFlagN[iLoop] = 0; }
    }
    
    return (condTermFlagN[VIDEO_H264_NEIGHBOUR_A] + condTermFlagN[VIDEO_H264_NEIGHBOUR_B]);
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdxInc_mb_field_decoding_flag(PVIDEOH264DECODEINFO pDecodeInfo)
{
    PVIDEOH264PICTUREINFO pPicInfo = pDecodeInfo->pPicInfo;
    PVIDEOH264MACROBLOCK pMacroBlock = pPicInfo->pMacroBlock;
    HANSIZE mbAddrN[2];
    VIDEOH264CABACCTXIDX condTermFlagN[2];

    DerivationProcessForNeighbouringMacroblocks(pDecodeInfo, mbAddrN);
    
    for (HANSIZE iLoop = 0; iLoop < 2; iLoop++)
    {
        if ((VIDEO_H264_MACRO_BLOCK_ADDR_INVALID == mbAddrN[iLoop]) || (0 == pMacroBlock[mbAddrN[iLoop]].mb_field_decoding_flag))
        {
            condTermFlagN[iLoop] = 0;
        }
        else { condTermFlagN[iLoop] = 1; }
    }

    return (condTermFlagN[VIDEO_H264_NEIGHBOUR_A] + condTermFlagN[VIDEO_H264_NEIGHBOUR_B]);
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdxInc_mb_type(PVIDEOH264DECODEINFO pDecodeInfo, VIDEOH264CABACCTXIDX ctxIdxOffset)
{
    PVIDEOH264PICTUREINFO pPicInfo = pDecodeInfo->pPicInfo;
    PVIDEOH264MACROBLOCK pMacroBlock = pPicInfo->pMacroBlock;
    HANSIZE mbAddrN[2];
    VIDEOH264CABACCTXIDX condTermFlagN[2];

    DerivationProcessForNeighbouringMacroblocks(pDecodeInfo, mbAddrN);

    for (HANSIZE iLoop = 0; iLoop < 2; iLoop++)
    {
        if ((VIDEO_H264_MACRO_BLOCK_ADDR_INVALID == mbAddrN[iLoop]) ||
            ((0 == ctxIdxOffset) && (VIDEO_H264_MB_TYPE_SI == pMacroBlock[mbAddrN[iLoop]].type)) ||
            ((3 == ctxIdxOffset) && (VIDEO_H264_MB_TYPE_I_NxN == pMacroBlock[mbAddrN[iLoop]].type)) ||
            ((27 == ctxIdxOffset) && (
                (VIDEO_H264_MB_TYPE_B_Skip == pMacroBlock[mbAddrN[iLoop]].type) ||
                (VIDEO_H264_MB_TYPE_B_Direct_16x16 == pMacroBlock[mbAddrN[iLoop]].type)
            ))
        )
        {
            condTermFlagN[iLoop] = 0;
        }
        else { condTermFlagN[iLoop] = 1; }
    }

    return (condTermFlagN[VIDEO_H264_NEIGHBOUR_A] + condTermFlagN[VIDEO_H264_NEIGHBOUR_B]);
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdxInc_coded_block_pattern(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx, VIDEOH264CABACCTXIDX ctxIdxOffset)
{
    PVIDEOH264PICTUREINFO pPicInfo = pDecodeInfo->pPicInfo;
    PVIDEOH264MACROBLOCK pMacroBlock = pPicInfo->pMacroBlock;
    HANSIZE mbAddrN[2];
    uint8_t luma8x8BlkIdxN[2];
    VIDEOH264CABACCTXIDX condTermFlagN[2];
    uint8_t binInc;

    if (73 == ctxIdxOffset)
    {
        DerivationProcessForNeighbouring8x8LumaBlock(pDecodeInfo, binIdx, mbAddrN, luma8x8BlkIdxN);

        for (HANSIZE iLoop = 0; iLoop < 2; iLoop++)
        {
            if ((VIDEO_H264_MACRO_BLOCK_ADDR_INVALID == mbAddrN[iLoop]) ||
                (VIDEO_H264_MB_TYPE_I_PCM == pMacroBlock[mbAddrN[iLoop]].type) ||
                (
                    (mbAddrN[iLoop] != pDecodeInfo->CurrMbAddr) && 
                    (
                        (VIDEO_H264_MB_TYPE_P_Skip != pMacroBlock[mbAddrN[iLoop]].type) &&
                        (VIDEO_H264_MB_TYPE_B_Skip != pMacroBlock[mbAddrN[iLoop]].type)
                    ) &&
                    (0 != ((pMacroBlock[mbAddrN[iLoop]].CodedBlockPatternLuma >> luma8x8BlkIdxN[iLoop]) & 1))
                ) ||
                (
                    (mbAddrN[iLoop] == pDecodeInfo->CurrMbAddr) &&
                    (0 != ((pDecodeInfo->pCABAC->cValue.u32 >> luma8x8BlkIdxN[iLoop]) & 1))
                )
            )
            {
                condTermFlagN[iLoop] = 0;
            }
            else { condTermFlagN[iLoop] = 1; }
        }

        binInc = 0;
    }
    else // ctxIdxOffset == 77
    {
        DerivationProcessForNeighbouringMacroblocks(pDecodeInfo, mbAddrN);
        for (HANSIZE iLoop = 0; iLoop < 2; iLoop++)
        {
            if ((VIDEO_H264_MACRO_BLOCK_ADDR_INVALID != mbAddrN[iLoop]) && (VIDEO_H264_MB_TYPE_I_PCM == pMacroBlock[mbAddrN[iLoop]].type)) { condTermFlagN[iLoop] = 1; }
            else
            {
                if ((
                        (VIDEO_H264_MACRO_BLOCK_ADDR_INVALID == mbAddrN[iLoop]) ||
                        ((VIDEO_H264_MB_TYPE_P_Skip == pMacroBlock[mbAddrN[iLoop]].type) || (VIDEO_H264_MB_TYPE_B_Skip == pMacroBlock[mbAddrN[iLoop]].type))
                    ) ||
                    ((0 == binIdx) && (0 == pMacroBlock[mbAddrN[iLoop]].CodedBlockPatternChroma)) ||
                    ((1 == binIdx) && (2 != pMacroBlock[mbAddrN[iLoop]].CodedBlockPatternChroma))
                )
                {
                    condTermFlagN[iLoop] = 0;
                }
                else { condTermFlagN[iLoop] = 1; }
            }
        }

        if (1 == binIdx) { binInc = 4; }
        else { binInc = 0; }
    }

    return condTermFlagN[VIDEO_H264_NEIGHBOUR_A] + (2 * condTermFlagN[VIDEO_H264_NEIGHBOUR_B]) + binInc;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdxInc_mb_qp_delta(PVIDEOH264DECODEINFO pDecodeInfo)
{
    VIDEOH264CABACCTXIDX incRet;
    PVIDEOH264PICTUREINFO pPicInfo = pDecodeInfo->pPicInfo;
    PVIDEOH264MACROBLOCK pMacroBlock = pPicInfo->pMacroBlock;
    HANSIZE prevMbAddr = H264GetprevMbAddr(pDecodeInfo);

    if ((VIDEO_H264_MACRO_BLOCK_ADDR_INVALID == prevMbAddr) ||
        (VIDEO_H264_MB_TYPE_P_Skip == pMacroBlock[prevMbAddr].type) ||
        (VIDEO_H264_MB_TYPE_B_Skip == pMacroBlock[prevMbAddr].type) ||
        (VIDEO_H264_MB_TYPE_I_PCM == pMacroBlock[prevMbAddr].type) ||
        (
            (VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16 != pMacroBlock[prevMbAddr].MbPartPredMode[0]) &&
            (0 == pMacroBlock[prevMbAddr].CodedBlockPatternLuma) && (0 == pMacroBlock[prevMbAddr].CodedBlockPatternChroma)
        ) ||
        (0 == pMacroBlock[prevMbAddr].mb_qp_delta)
    )
    {
        incRet = 0;
    }
    else { incRet = 1; }

    return incRet;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdxInc_ref_idx_lX(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t lX)
{
    PVIDEOH264PICTUREINFO pPicInfo = pDecodeInfo->pPicInfo;
    PCVIDEOH264SLICE pSlice = &(pPicInfo->slice);
    PVIDEOH264MACROBLOCK pMacroBlock = pPicInfo->pMacroBlock;
    HANSIZE CurrMbAddr = pDecodeInfo->CurrMbAddr;
    uint32_t* ref_idx_lX;
    VIDEOH264MBPARTPREDMODE Pred_LX;
    HANSIZE mbAddrN[2];
    uint8_t mbPartIdxN[2];
    uint8_t refIdxZeroFlagN[2];
    uint8_t predModeEqualFlagN[2];
    uint8_t condTermFlagN[2];

    pDecodeInfo->subMbPartIdx = 0;
    DerivationProcessForNeighbouringPartitions(pDecodeInfo, mbAddrN, mbPartIdxN, NULL);

    for (uint8_t iLoop = 0; iLoop < 2; iLoop++)
    {
        if (VIDEO_H264_MACRO_BLOCK_ADDR_INVALID != mbAddrN[iLoop])
        {
            /* 确定 lX */
            if (0 == lX)
            {
                ref_idx_lX = pMacroBlock[mbAddrN[iLoop]].ref_idx_l0;
                Pred_LX = VIDEO_H264_MB_PART_PRED_MODE_Pred_L0;
            }
            else
            {
                ref_idx_lX = pMacroBlock[mbAddrN[iLoop]].ref_idx_l1;
                Pred_LX = VIDEO_H264_MB_PART_PRED_MODE_Pred_L1;
            }
            /* refIdxZeroFlagN */
            if ((1 == pSlice->MbaffFrameFlag) && (0 == pMacroBlock[CurrMbAddr].mb_field_decoding_flag) && (1 == pMacroBlock[CurrMbAddr].mb_field_decoding_flag))
            {
                if (1 < ref_idx_lX[mbPartIdxN[iLoop]]) { refIdxZeroFlagN[iLoop] = 0; }
                else { refIdxZeroFlagN[iLoop] = 1; }
            }
            else
            {
                if (0 < ref_idx_lX[mbPartIdxN[iLoop]]) { refIdxZeroFlagN[iLoop] = 0; }
                else { refIdxZeroFlagN[iLoop] = 1; }
            }
            /* predModeEqualFlagN */
            if ((VIDEO_H264_MB_TYPE_B_Direct_16x16 == pMacroBlock[mbAddrN[iLoop]].type) || (VIDEO_H264_MB_TYPE_B_Skip == pMacroBlock[mbAddrN[iLoop]].type))
            {
                predModeEqualFlagN[iLoop] = 0;
            }
            else if ((VIDEO_H264_MB_TYPE_P_8x8 == pMacroBlock[mbAddrN[iLoop]].type) || (VIDEO_H264_MB_TYPE_B_8x8 == pMacroBlock[mbAddrN[iLoop]].type))
            {
                if ((Pred_LX != pMacroBlock[mbAddrN[iLoop]].SubMbPredMode[mbPartIdxN[iLoop]]) &&
                (VIDEO_H264_MB_PART_PRED_MODE_BiPred != pMacroBlock[mbAddrN[iLoop]].SubMbPredMode[mbPartIdxN[iLoop]]))
                {
                    predModeEqualFlagN[iLoop] = 0;
                }
                else { predModeEqualFlagN[iLoop] = 1; }
            }
            else
            {
                if ((Pred_LX != pMacroBlock[mbAddrN[iLoop]].MbPartPredMode[mbPartIdxN[iLoop]]) &&
                (VIDEO_H264_MB_PART_PRED_MODE_BiPred != pMacroBlock[mbAddrN[iLoop]].MbPartPredMode[mbPartIdxN[iLoop]]))
                {
                    predModeEqualFlagN[iLoop] = 0;
                }
                else { predModeEqualFlagN[iLoop] = 1; }
            }
            /* condTermFlagN */
            if ((
                    (VIDEO_H264_MB_TYPE_P_Skip == (pMacroBlock[mbAddrN[iLoop]].type)) ||
                    (VIDEO_H264_MB_TYPE_B_Skip == (pMacroBlock[mbAddrN[iLoop]].type))
                ) ||
                (VIDEO_H264_MB_PRED_MODE_CAT_INTER == pMacroBlock[mbAddrN[iLoop]].mbPredModeCat) ||
                (0 == predModeEqualFlagN[iLoop]) ||
                (1 == refIdxZeroFlagN[iLoop]))
            {
                condTermFlagN[iLoop] = 0;
            }
            else { condTermFlagN[iLoop] = 1; }
        }
        else { condTermFlagN[iLoop] = 0; }
    }

    return condTermFlagN[VIDEO_H264_NEIGHBOUR_A] + 2 * condTermFlagN[VIDEO_H264_NEIGHBOUR_B];
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdxInc_mvd_lX(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t lX, VIDEOH264CABACCTXIDX ctxIdxOffset)
{
    VIDEOH264CABACCTXIDX ctxIdxInc;
    PVIDEOH264PICTUREINFO pPicInfo = pDecodeInfo->pPicInfo;
    PCVIDEOH264SLICE pSlice = &(pPicInfo->slice);
    PVIDEOH264MACROBLOCK pMacroBlock = pPicInfo->pMacroBlock;
    HANSIZE CurrMbAddr = pDecodeInfo->CurrMbAddr;
    int32_t (*mvd_lX)[VIDEO_H264_MACRO_BLOCK_PART_SIZE][VIDEO_H264_SUB_MACRO_BLOCK_PART_SIZE][2];
    VIDEOH264MBPARTPREDMODE Pred_LX;
    uint8_t compIdx;
    HANSIZE mbAddrN[2];
    uint8_t mbPartIdxN[2];
    uint8_t subMbPartIdxN[2];
    uint8_t predModeEqualFlagN[2];
    uint32_t absMvdCompN[2];

    DerivationProcessForNeighbouringPartitions(pDecodeInfo, mbAddrN, mbPartIdxN, subMbPartIdxN);

    for (uint8_t iLoop = 0; iLoop < 2; iLoop++)
    {
        if (VIDEO_H264_MACRO_BLOCK_ADDR_INVALID != mbAddrN[iLoop])
        {
            /* 确定 lX */
            if (0 == lX)
            {
                mvd_lX = &(pMacroBlock[mbAddrN[iLoop]].mvd_l0);
                Pred_LX = VIDEO_H264_MB_PART_PRED_MODE_Pred_L0;
            }
            else
            {
                mvd_lX = &(pMacroBlock[mbAddrN[iLoop]].mvd_l1);
                Pred_LX = VIDEO_H264_MB_PART_PRED_MODE_Pred_L1;
            }
            /* 确定 compIdx */
            if (40 == ctxIdxOffset) { compIdx = 0; }
            else { compIdx = 1; }
            /* 确定 predModeEqualFlagN */
            if ((VIDEO_H264_MB_TYPE_B_Direct_16x16 == pMacroBlock[mbAddrN[iLoop]].type) || (VIDEO_H264_MB_TYPE_B_Skip == pMacroBlock[mbAddrN[iLoop]].type))
            {
                predModeEqualFlagN[iLoop] = 0;
            }
            else if ((VIDEO_H264_MB_TYPE_P_8x8 == pMacroBlock[mbAddrN[iLoop]].type) || (VIDEO_H264_MB_TYPE_B_8x8 == pMacroBlock[mbAddrN[iLoop]].type))
            {
                if ((Pred_LX != pMacroBlock[mbAddrN[iLoop]].SubMbPredMode[mbPartIdxN[iLoop]]) &&
                    (VIDEO_H264_MB_PART_PRED_MODE_BiPred != pMacroBlock[mbAddrN[iLoop]].SubMbPredMode[mbPartIdxN[iLoop]]))
                {
                    predModeEqualFlagN[iLoop] = 0;
                }
                else { predModeEqualFlagN[iLoop] = 1; }
            }
            else
            {
                if ((Pred_LX != pMacroBlock[mbAddrN[iLoop]].MbPartPredMode[mbPartIdxN[iLoop]]) &&
                    (VIDEO_H264_MB_PART_PRED_MODE_BiPred != pMacroBlock[mbAddrN[iLoop]].MbPartPredMode[mbPartIdxN[iLoop]]))
                {
                    predModeEqualFlagN[iLoop] = 0;
                }
                else { predModeEqualFlagN[iLoop] = 1; }
            }
            /* 确定 absMvdCompN */
            if ((
                    (VIDEO_H264_MB_TYPE_P_Skip == (pMacroBlock[mbAddrN[iLoop]].type)) ||
                    (VIDEO_H264_MB_TYPE_B_Skip == (pMacroBlock[mbAddrN[iLoop]].type))
                ) ||
                (VIDEO_H264_MB_PRED_MODE_CAT_INTER == pMacroBlock[mbAddrN[iLoop]].mbPredModeCat) ||
                (0 == predModeEqualFlagN[iLoop]))
            { absMvdCompN[iLoop] = 0; }
            else
            {
                if ((1 == compIdx) && (1 == pSlice->MbaffFrameFlag) && (0 == pMacroBlock[CurrMbAddr].mb_field_decoding_flag) && (1 == pMacroBlock[mbAddrN[iLoop]].mb_field_decoding_flag))
                {
                    absMvdCompN[iLoop] = H264AbsInt((*mvd_lX)[mbPartIdxN[iLoop]][subMbPartIdxN[iLoop]][compIdx]) * 2;
                }
                else if ((1 == compIdx) && (1 == pSlice->MbaffFrameFlag) && (1 == pMacroBlock[CurrMbAddr].mb_field_decoding_flag) && (0 == pMacroBlock[mbAddrN[iLoop]].mb_field_decoding_flag))
                {
                    absMvdCompN[iLoop] = H264AbsInt((*mvd_lX)[mbPartIdxN[iLoop]][subMbPartIdxN[iLoop]][compIdx]) / 2;
                }
                else { absMvdCompN[iLoop] = H264AbsInt((*mvd_lX)[mbPartIdxN[iLoop]][subMbPartIdxN[iLoop]][compIdx]); }
            }
        }
        else
        {
            absMvdCompN[iLoop] = 0;
        }
    }

    if ((32 < absMvdCompN[VIDEO_H264_NEIGHBOUR_A]) || (32 < absMvdCompN[VIDEO_H264_NEIGHBOUR_B])) { ctxIdxInc = 2; }
    else if (32 < (absMvdCompN[VIDEO_H264_NEIGHBOUR_A] + absMvdCompN[VIDEO_H264_NEIGHBOUR_B])) { ctxIdxInc = 2; }
    else if (2 < (absMvdCompN[VIDEO_H264_NEIGHBOUR_A] + absMvdCompN[VIDEO_H264_NEIGHBOUR_B])) { ctxIdxInc = 1; }
    else { ctxIdxInc = 0; }

    return ctxIdxInc;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdxInc_intra_chroma_pred_mode(PVIDEOH264DECODEINFO pDecodeInfo)
{
    PVIDEOH264PICTUREINFO pPicInfo = pDecodeInfo->pPicInfo;
    PVIDEOH264MACROBLOCK pMacroBlock = pPicInfo->pMacroBlock;
    HANSIZE mbAddrN[2];
    VIDEOH264CABACCTXIDX condTermFlagN[2];

    DerivationProcessForNeighbouringMacroblocks(pDecodeInfo, mbAddrN);
    
    for (HANSIZE iLoop = 0; iLoop < 2; iLoop++)
    {
        if ((VIDEO_H264_MACRO_BLOCK_ADDR_INVALID == mbAddrN[iLoop]) ||
            (VIDEO_H264_MB_PRED_MODE_CAT_INTER == pMacroBlock[mbAddrN[iLoop]].mbPredModeCat) ||
            (VIDEO_H264_MB_TYPE_I_PCM == pMacroBlock[mbAddrN[iLoop]].type) ||
            (0 == pMacroBlock[mbAddrN[iLoop]].intra_chroma_pred_mode)
        )
        {
            condTermFlagN[iLoop] = 0;
        }
        else { condTermFlagN[iLoop] = 1; }
    }

    return (condTermFlagN[VIDEO_H264_NEIGHBOUR_A] + condTermFlagN[VIDEO_H264_NEIGHBOUR_B]);
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdxInc_coded_block_flag(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t ctxBlockCat)
{
    PVIDEOH264PICTUREINFO pPicInfo = pDecodeInfo->pPicInfo;
    PVIDEOH264MACROBLOCK pMacroBlock = pPicInfo->pMacroBlock;
    HANSIZE CurrMbAddr = pDecodeInfo->CurrMbAddr;
    uint8_t nal_unit_type = pDecodeInfo->pPicInfo->slice.slice_header.nalUnit.nal_unit_type;
    HANSIZE mbAddrN[2];
    uint8_t transBlockN[2];
    VIDEOH264CABACCTXIDX condTermFlagN[2];

    /* 获取正在解码块的相信块的 ID（transBlockN），以及该相邻块的所属宏块地址（mbAddrN）
     * 部分函数需要的额外参数（如当前正在解码的块 ID、分量 iCbCr 等），通过 pDecodeInfo 直接传递了，不再做成函数形参
     */
    switch (ctxBlockCat) {
        case 0:
        case 6:
        case 10: { H264ReadAEGetNeighbouringBlock_coded_block_flag_0_6_10(pDecodeInfo, mbAddrN, transBlockN); } break; // ctxBlockCat == 0, 6, or 10
        case 1:
        case 2: { H264ReadAEGetNeighbouringBlock_coded_block_flag_1_2(pDecodeInfo, mbAddrN, transBlockN); } break; // ctxBlockCat == 1, or 2
        case 3: { H264ReadAEGetNeighbouringBlock_coded_block_flag_3(pDecodeInfo, mbAddrN, transBlockN); }; break; // ctxBlockCat == 3
        case 4: { H264ReadAEGetNeighbouringBlock_coded_block_flag_4(pDecodeInfo, mbAddrN, transBlockN); }; break; // ctxBlockCat == 4
        case 5: { H264ReadAEGetNeighbouringBlock_coded_block_flag_5(pDecodeInfo, mbAddrN, transBlockN); }; break; // ctxBlockCat == 5
        case 7:
        case 8: { H264ReadAEGetNeighbouringBlock_coded_block_flag_7_8(pDecodeInfo, mbAddrN, transBlockN); }; break; // ctxBlockCat == 7 or 8
        case 9: { H264ReadAEGetNeighbouringBlock_coded_block_flag_9(pDecodeInfo, mbAddrN, transBlockN); }; break; // ctxBlockCat == 9
        case 11:
        case 12: { H264ReadAEGetNeighbouringBlock_coded_block_flag_11_12(pDecodeInfo, mbAddrN, transBlockN); }; break; // ctxBlockCat == 11 or 12
        default: { H264ReadAEGetNeighbouringBlock_coded_block_flag_13(pDecodeInfo, mbAddrN, transBlockN); } break; // ctxBlockCat == 13
    }

    for (HANSIZE iLoop = 0; iLoop < 2; iLoop++)
    {
        if (
            (
                (VIDEO_H264_MACRO_BLOCK_ADDR_INVALID == mbAddrN[iLoop]) &&
                (VIDEO_H264_MB_PRED_MODE_CAT_INTER == pMacroBlock[CurrMbAddr].mbPredModeCat)
            ) ||
            (
                (VIDEO_H264_MACRO_BLOCK_ADDR_INVALID != mbAddrN[iLoop]) &&
                (VIDEO_H264_BLOCK_IDX_INVALID == transBlockN[iLoop]) &&
                (VIDEO_H264_MB_TYPE_I_PCM != pMacroBlock[mbAddrN[iLoop]].type)
            ) ||
            (
                (VIDEO_H264_MB_PRED_MODE_CAT_INTRA == pMacroBlock[CurrMbAddr].mbPredModeCat) &&
                (1 == pDecodeInfo->pPicInfo->slice.pPPS->constrained_intra_pred_flag) &&
                (
                    (VIDEO_H264_MACRO_BLOCK_ADDR_INVALID != mbAddrN[iLoop]) &&
                    (VIDEO_H264_MB_PRED_MODE_CAT_INTER == pMacroBlock[mbAddrN[iLoop]].mbPredModeCat) &&
                    ((2 <= nal_unit_type) && (nal_unit_type <= 4)) // 这条可能用不上吧...
                )
            )
        )
        { condTermFlagN[iLoop] = 0; }
        else if (
            (
                (VIDEO_H264_MACRO_BLOCK_ADDR_INVALID == mbAddrN[iLoop]) &&
                (VIDEO_H264_MB_PRED_MODE_CAT_INTRA == pMacroBlock[CurrMbAddr].mbPredModeCat)
            ) ||
            (VIDEO_H264_MB_TYPE_I_PCM == pMacroBlock[mbAddrN[iLoop]].type)
        )
        { condTermFlagN[iLoop] = 1; }
        else
        {
            condTermFlagN[iLoop] = pMacroBlock[mbAddrN[iLoop]].residual_block_SE.cabac.coded_block_flag[ctxBlockCat][pDecodeInfo->iCbCr][transBlockN[iLoop]];
        }
    }

    return (condTermFlagN[VIDEO_H264_NEIGHBOUR_A] + 2 * condTermFlagN[VIDEO_H264_NEIGHBOUR_B]);
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdxInc_transform_size_8x8_flag(PVIDEOH264DECODEINFO pDecodeInfo)
{
    PVIDEOH264PICTUREINFO pPicInfo = pDecodeInfo->pPicInfo;
    PVIDEOH264MACROBLOCK pMacroBlock = pPicInfo->pMacroBlock;
    HANSIZE mbAddrN[2];
    VIDEOH264CABACCTXIDX condTermFlagN[2];

    DerivationProcessForNeighbouringMacroblocks(pDecodeInfo, mbAddrN);

    for (HANSIZE iLoop = 0; iLoop < 2; iLoop++)
    {
        if ((VIDEO_H264_MACRO_BLOCK_ADDR_INVALID == mbAddrN[iLoop]) ||
            (0 == pMacroBlock[mbAddrN[iLoop]].transform_size_8x8_flag))
        {
            condTermFlagN[iLoop] = 0;
        }
        else { condTermFlagN[iLoop] = 1; }
    }

    return (condTermFlagN[VIDEO_H264_NEIGHBOUR_A] + condTermFlagN[VIDEO_H264_NEIGHBOUR_B]);
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdxInc_prior_decoded_bin(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx, VIDEOH264CABACCTXIDX ctxIdxOffset)
{
    VIDEOH264CABACCTXIDX ctxIdxInc = 0;

    switch (ctxIdxOffset) {
        case 3: {
            switch (binIdx) {
                case 4: {
                    if ((pDecodeInfo->pCABAC->cValue.u32 & 1) != 0) { ctxIdxInc = 5; }
                    else { ctxIdxInc = 6; }
                } break;
                case 5: {
                    if ((pDecodeInfo->pCABAC->cValue.u32 & 2) != 0) { ctxIdxInc = 6; }
                    else { ctxIdxInc = 7; }
                } break;
                default: { } break;
            }
        } break;
        case 14: {
            if ((pDecodeInfo->pCABAC->cValue.u32 & 1) != 0) { ctxIdxInc = 2; }
            else { ctxIdxInc = 3; }
        } break;
        case 17: {
            if ((pDecodeInfo->pCABAC->cValue.u32 & 1) != 0) { ctxIdxInc = 2; }
            else { ctxIdxInc = 3; }
        } break;
        case 27: {
            if ((pDecodeInfo->pCABAC->cValue.u32 & 1) != 0) { ctxIdxInc = 4; }
            else { ctxIdxInc = 5; }
        } break;
        case 32: {
            if ((pDecodeInfo->pCABAC->cValue.u32 & 1) != 0) { ctxIdxInc = 2; }
            else { ctxIdxInc = 3; }
        } break;
        case 36: {
            if ((pDecodeInfo->pCABAC->cValue.u32 & 1) != 0) { ctxIdxInc = 2; }
            else { ctxIdxInc = 3; }
        } break;
        default: { } break;
    }

    return ctxIdxInc;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdxInc_significant_coeff_flag(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t ctxBlockCat)
{
    VIDEOH264CABACCTXIDX ctxIdxInc;
    uint8_t levelListIdx = pDecodeInfo->levelListIdx;

    switch (ctxBlockCat) {
        case 3: { ctxIdxInc = H264Min(levelListIdx / pDecodeInfo->NumC8x8, 2); } break; // ctxBlockCat == 3
        case 5:
        case 9:
        case 13: { // ctxBlockCat == 5, 9, or 13
            if (VIDEO_H264_FRAME_FIELD_MODE_FRAME == GetMacroBlockFrameFiledMode(pDecodeInfo, GetCurrentMacroBlock(pDecodeInfo)))
            {
                ctxIdxInc = sg_pCtxIdxInc8x8Frame_significant_coeff_flag[levelListIdx];
            }
            else { ctxIdxInc = sg_pCtxIdxInc8x8Field_significant_coeff_flag[levelListIdx]; }
        } break;
        default: { ctxIdxInc = levelListIdx; } break; // ctxBlockCat != 3, 5, 9, and 13
    }
    
    return ctxIdxInc;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdxInc_last_significant_coeff_flag(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t ctxBlockCat)
{
    VIDEOH264CABACCTXIDX ctxIdxInc;
    uint8_t levelListIdx = pDecodeInfo->levelListIdx;

    switch (ctxBlockCat) {
        case 3: { ctxIdxInc = H264Min(levelListIdx / pDecodeInfo->NumC8x8, 2); } break; // ctxBlockCat == 3
        case 5:
        case 9:
        case 13: { ctxIdxInc = sg_pCtxIdxInc8x8_last_significant_coeff_flag[levelListIdx]; } break; // ctxBlockCat == 5, 9, or 13
        default: { ctxIdxInc = levelListIdx; } break; // ctxBlockCat != 3, 5, 9, and 13
    }
    
    return ctxIdxInc;
}
static VIDEOH264CABACCTXIDX H264ReadAEGetctxIdxInc_coeff_abs_level_minus1(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t binIdx, uint8_t ctxBlockCat)
{
    VIDEOH264CABACCTXIDX ctxIdxInc;

    if (0 == binIdx)
    {
        if (0 != pDecodeInfo->pCABAC->numDecodAbsLevelGt1) { ctxIdxInc = 0; }
        else { ctxIdxInc = H264Min(4, 1 + pDecodeInfo->pCABAC->numDecodAbsLevelEq1); }
    }
    else // binIdx > 0
    {
        if (3 == ctxBlockCat) { ctxIdxInc = 5 + H264Min(3, pDecodeInfo->pCABAC->numDecodAbsLevelGt1); }
        else { ctxIdxInc = 5 + H264Min(4, pDecodeInfo->pCABAC->numDecodAbsLevelGt1); }
    }

    return ctxIdxInc;
}

static inline void H264ReadAEGetNeighbouringBlock_coded_block_flag_0_6_10(PVIDEOH264DECODEINFO pDecodeInfo, HANSIZE mbAddrN[2], uint8_t transBlockN[2])
{
    PVIDEOH264MACROBLOCK pMacroBlock = pDecodeInfo->pPicInfo->pMacroBlock;

    /* 对于 ctxBlockCat 等于 0，6，10 来说，这三种情况是在对 DC 块进行解码
     * 0 对应 Luma 的 DC，6 对应 Cb 的 DC，10 对应 Cr 的 DC
     * 不管是哪个分量，DC 块都只有一块，DC 块就是宏块本身
     * 所以 mbAddrN 就是相邻宏块的地址
     * transBlockN = 0，因为只有一块
     */
    DerivationProcessForNeighbouringMacroblocks(pDecodeInfo, mbAddrN);
    
    for (HANSIZE iLoop = 0; iLoop < 2; iLoop++)
    {
        if ((VIDEO_H264_MACRO_BLOCK_ADDR_INVALID != mbAddrN[iLoop]) && (VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16 == pMacroBlock[mbAddrN[iLoop]].MbPartPredMode[0]))
        {
            transBlockN[iLoop] = 0;
        }
        else { transBlockN[iLoop] = VIDEO_H264_BLOCK_IDX_INVALID; }
    }
}
static inline void H264ReadAEGetNeighbouringBlock_coded_block_flag_1_2(PVIDEOH264DECODEINFO pDecodeInfo, HANSIZE mbAddrN[2], uint8_t transBlockN[2])
{
    PVIDEOH264MACROBLOCK pMacroBlock = pDecodeInfo->pPicInfo->pMacroBlock;
    uint8_t luma4x4BlkIdx = GetDecodingBlockIdx(pDecodeInfo);
    uint8_t luma4x4BlkIdxN[2];

    DerivationProcessForNeighbouring4x4LumaBlock(pDecodeInfo, luma4x4BlkIdx, mbAddrN, luma4x4BlkIdxN);
    for (HANSIZE iLoop = 0; iLoop < 2; iLoop++)
    {
        if (
            (VIDEO_H264_MACRO_BLOCK_ADDR_INVALID != mbAddrN[iLoop]) &&
            (
                (VIDEO_H264_MB_TYPE_P_Skip != pMacroBlock[mbAddrN[iLoop]].type) &&
                (VIDEO_H264_MB_TYPE_B_Skip != pMacroBlock[mbAddrN[iLoop]].type) &&
                (VIDEO_H264_MB_TYPE_I_PCM != pMacroBlock[mbAddrN[iLoop]].type)
            ) && 
            (0 != ((pMacroBlock[mbAddrN[iLoop]].CodedBlockPatternLuma >> (luma4x4BlkIdxN[iLoop] >> 2)) & 1)) &&
            (0 == pMacroBlock[mbAddrN[iLoop]].transform_size_8x8_flag)
        )
        { transBlockN[iLoop] = luma4x4BlkIdxN[iLoop]; }
        else if (
            (VIDEO_H264_MACRO_BLOCK_ADDR_INVALID != mbAddrN[iLoop]) &&
            (
                (VIDEO_H264_MB_TYPE_P_Skip != pMacroBlock[mbAddrN[iLoop]].type) &&
                (VIDEO_H264_MB_TYPE_B_Skip != pMacroBlock[mbAddrN[iLoop]].type)
            ) && 
            (0 != ((pMacroBlock[mbAddrN[iLoop]].CodedBlockPatternLuma >> (luma4x4BlkIdxN[iLoop] >> 2)) & 1)) &&
            (1 == pMacroBlock[mbAddrN[iLoop]].transform_size_8x8_flag)
        )
        { transBlockN[iLoop] = luma4x4BlkIdxN[iLoop] >> 2; }
        else { transBlockN[iLoop] = VIDEO_H264_BLOCK_IDX_INVALID; }
    }
}
static inline void H264ReadAEGetNeighbouringBlock_coded_block_flag_3(PVIDEOH264DECODEINFO pDecodeInfo, HANSIZE mbAddrN[2], uint8_t transBlockN[2])
{
    PVIDEOH264MACROBLOCK pMacroBlock = pDecodeInfo->pPicInfo->pMacroBlock;

    DerivationProcessForNeighbouringMacroblocks(pDecodeInfo, mbAddrN);
    for (HANSIZE iLoop = 0; iLoop < 2; iLoop++)
    {
        if ((VIDEO_H264_MACRO_BLOCK_ADDR_INVALID != mbAddrN[iLoop]) &&
            (
                (VIDEO_H264_MB_TYPE_P_Skip != pMacroBlock[mbAddrN[iLoop]].type) &&
                (VIDEO_H264_MB_TYPE_B_Skip != pMacroBlock[mbAddrN[iLoop]].type) &&
                (VIDEO_H264_MB_TYPE_I_PCM != pMacroBlock[mbAddrN[iLoop]].type)
            ) &&
            (0 != pMacroBlock[mbAddrN[iLoop]].CodedBlockPatternChroma)
        )
        {
            transBlockN[iLoop] = 0;
        }
        else { transBlockN[iLoop] = VIDEO_H264_BLOCK_IDX_INVALID; }
    }
}
static inline void H264ReadAEGetNeighbouringBlock_coded_block_flag_4(PVIDEOH264DECODEINFO pDecodeInfo, HANSIZE mbAddrN[2], uint8_t transBlockN[2])
{
    PVIDEOH264MACROBLOCK pMacroBlock = pDecodeInfo->pPicInfo->pMacroBlock;
    uint8_t chroma4x4BlkIdx = GetDecodingBlockIdx(pDecodeInfo);
    uint8_t chroma4x4BlkIdxN[2];

    DerivationProcessForNeighbouring4x4ChromaBlock(pDecodeInfo, chroma4x4BlkIdx, mbAddrN, chroma4x4BlkIdxN);
    for (HANSIZE iLoop = 0; iLoop < 2; iLoop++)
    {
        if (
            (VIDEO_H264_MACRO_BLOCK_ADDR_INVALID != mbAddrN[iLoop]) &&
            (
                (VIDEO_H264_MB_TYPE_P_Skip != pMacroBlock[mbAddrN[iLoop]].type) &&
                (VIDEO_H264_MB_TYPE_B_Skip != pMacroBlock[mbAddrN[iLoop]].type) &&
                (VIDEO_H264_MB_TYPE_I_PCM != pMacroBlock[mbAddrN[iLoop]].type)
            ) && 
            (2 == pMacroBlock[mbAddrN[iLoop]].CodedBlockPatternChroma)
        )
        { transBlockN[iLoop] = chroma4x4BlkIdxN[iLoop]; }
        else { transBlockN[iLoop] = VIDEO_H264_BLOCK_IDX_INVALID; }
    }
}
static inline void H264ReadAEGetNeighbouringBlock_coded_block_flag_5(PVIDEOH264DECODEINFO pDecodeInfo, HANSIZE mbAddrN[2], uint8_t transBlockN[2])
{
    PVIDEOH264MACROBLOCK pMacroBlock = pDecodeInfo->pPicInfo->pMacroBlock;
    uint8_t luma8x8BlkIdx = pDecodeInfo->i8x8;
    uint8_t luma8x8BlkIdxN[2];

    DerivationProcessForNeighbouring8x8LumaBlock(pDecodeInfo, luma8x8BlkIdx, mbAddrN, luma8x8BlkIdxN);
    for (HANSIZE iLoop = 0; iLoop < 2; iLoop++)
    {
        if (
            (VIDEO_H264_MACRO_BLOCK_ADDR_INVALID != mbAddrN[iLoop]) &&
            (
                (VIDEO_H264_MB_TYPE_P_Skip != pMacroBlock[mbAddrN[iLoop]].type) &&
                (VIDEO_H264_MB_TYPE_B_Skip != pMacroBlock[mbAddrN[iLoop]].type) &&
                (VIDEO_H264_MB_TYPE_I_PCM != pMacroBlock[mbAddrN[iLoop]].type)
            ) && 
            (0 != ((pMacroBlock[mbAddrN[iLoop]].CodedBlockPatternLuma >> luma8x8BlkIdx) & 1)) &&
            (1 == pMacroBlock[mbAddrN[iLoop]].transform_size_8x8_flag)
        )
        { transBlockN[iLoop] = luma8x8BlkIdxN[iLoop]; }
        else { transBlockN[iLoop] = VIDEO_H264_BLOCK_IDX_INVALID; }
    }
}
static inline void H264ReadAEGetNeighbouringBlock_coded_block_flag_7_8(PVIDEOH264DECODEINFO pDecodeInfo, HANSIZE mbAddrN[2], uint8_t transBlockN[2])
{
    PVIDEOH264MACROBLOCK pMacroBlock = pDecodeInfo->pPicInfo->pMacroBlock;
    uint8_t cb4x4BlkIdx = GetDecodingBlockIdx(pDecodeInfo);
    uint8_t cb4x4BlkIdxN[2];

    /* 代码其实和 H264ReadAEGetNeighbouringBlock_coded_block_flag_1_2 函数完全一样，只是换掉了块索引变量的名字，逻辑是完全一致的 */
    DerivationProcessForNeighbouring4x4ChromaBlockForChromaArrayType3(pDecodeInfo, cb4x4BlkIdx, mbAddrN, cb4x4BlkIdxN);
    for (HANSIZE iLoop = 0; iLoop < 2; iLoop++)
    {
        if (
            (VIDEO_H264_MACRO_BLOCK_ADDR_INVALID != mbAddrN[iLoop]) &&
            (
                (VIDEO_H264_MB_TYPE_P_Skip != pMacroBlock[mbAddrN[iLoop]].type) &&
                (VIDEO_H264_MB_TYPE_B_Skip != pMacroBlock[mbAddrN[iLoop]].type) &&
                (VIDEO_H264_MB_TYPE_I_PCM != pMacroBlock[mbAddrN[iLoop]].type)
            ) && 
            (0 != ((pMacroBlock[mbAddrN[iLoop]].CodedBlockPatternLuma >> (cb4x4BlkIdxN[iLoop] >> 2)) & 1)) &&
            (0 == pMacroBlock[mbAddrN[iLoop]].transform_size_8x8_flag)
        )
        { transBlockN[iLoop] = cb4x4BlkIdxN[iLoop]; }
        else if (
            (VIDEO_H264_MACRO_BLOCK_ADDR_INVALID != mbAddrN[iLoop]) &&
            (
                (VIDEO_H264_MB_TYPE_P_Skip != pMacroBlock[mbAddrN[iLoop]].type) &&
                (VIDEO_H264_MB_TYPE_B_Skip != pMacroBlock[mbAddrN[iLoop]].type)
            ) && 
            (0 != ((pMacroBlock[mbAddrN[iLoop]].CodedBlockPatternLuma >> (cb4x4BlkIdxN[iLoop] >> 2)) & 1)) &&
            (1 == pMacroBlock[mbAddrN[iLoop]].transform_size_8x8_flag)
        )
        { transBlockN[iLoop] = cb4x4BlkIdxN[iLoop] >> 2; }
        else { transBlockN[iLoop] = VIDEO_H264_BLOCK_IDX_INVALID; }
    }
}
static inline void H264ReadAEGetNeighbouringBlock_coded_block_flag_9(PVIDEOH264DECODEINFO pDecodeInfo, HANSIZE mbAddrN[2], uint8_t transBlockN[2])
{
    PVIDEOH264MACROBLOCK pMacroBlock = pDecodeInfo->pPicInfo->pMacroBlock;
    uint8_t cb8x8BlkIdx = pDecodeInfo->i8x8;
    uint8_t cb8x8BlkIdxN[2];

    /* 代码其实和 H264ReadAEGetNeighbouringBlock_coded_block_flag_5 函数完全一样，只是换掉了块索引变量的名字，逻辑是完全一致的 */
    DerivationProcessForNeighbouring8x8LumaBlock(pDecodeInfo, cb8x8BlkIdx, mbAddrN, cb8x8BlkIdxN);
    for (HANSIZE iLoop = 0; iLoop < 2; iLoop++)
    {
        if (
            (VIDEO_H264_MACRO_BLOCK_ADDR_INVALID != mbAddrN[iLoop]) &&
            (
                (VIDEO_H264_MB_TYPE_P_Skip != pMacroBlock[mbAddrN[iLoop]].type) &&
                (VIDEO_H264_MB_TYPE_B_Skip != pMacroBlock[mbAddrN[iLoop]].type) &&
                (VIDEO_H264_MB_TYPE_I_PCM != pMacroBlock[mbAddrN[iLoop]].type)
            ) && 
            (0 != ((pMacroBlock[mbAddrN[iLoop]].CodedBlockPatternLuma >> cb8x8BlkIdx) & 1)) &&
            (1 == pMacroBlock[mbAddrN[iLoop]].transform_size_8x8_flag)
        )
        { transBlockN[iLoop] = cb8x8BlkIdxN[iLoop]; }
        else { transBlockN[iLoop] = VIDEO_H264_BLOCK_IDX_INVALID; }
    }
}
static inline void H264ReadAEGetNeighbouringBlock_coded_block_flag_11_12(PVIDEOH264DECODEINFO pDecodeInfo, HANSIZE mbAddrN[2], uint8_t transBlockN[2])
{
    PVIDEOH264MACROBLOCK pMacroBlock = pDecodeInfo->pPicInfo->pMacroBlock;
    uint8_t cr4x4BlkIdx = GetDecodingBlockIdx(pDecodeInfo);
    uint8_t cr4x4BlkIdxN[2];

    /* 代码其实和 H264ReadAEGetNeighbouringBlock_coded_block_flag_1_2 函数完全一样，只是换掉了块索引变量的名字，逻辑是完全一致的 */
    DerivationProcessForNeighbouring4x4ChromaBlockForChromaArrayType3(pDecodeInfo, cr4x4BlkIdx, mbAddrN, cr4x4BlkIdxN);
    for (HANSIZE iLoop = 0; iLoop < 2; iLoop++)
    {
        if (
            (VIDEO_H264_MACRO_BLOCK_ADDR_INVALID != mbAddrN[iLoop]) &&
            (
                (VIDEO_H264_MB_TYPE_P_Skip != pMacroBlock[mbAddrN[iLoop]].type) &&
                (VIDEO_H264_MB_TYPE_B_Skip != pMacroBlock[mbAddrN[iLoop]].type) &&
                (VIDEO_H264_MB_TYPE_I_PCM != pMacroBlock[mbAddrN[iLoop]].type)
            ) && 
            (0 != ((pMacroBlock[mbAddrN[iLoop]].CodedBlockPatternLuma >> (cr4x4BlkIdxN[iLoop] >> 2)) & 1)) &&
            (0 == pMacroBlock[mbAddrN[iLoop]].transform_size_8x8_flag)
        )
        { transBlockN[iLoop] = cr4x4BlkIdxN[iLoop]; }
        else if (
            (VIDEO_H264_MACRO_BLOCK_ADDR_INVALID != mbAddrN[iLoop]) &&
            (
                (VIDEO_H264_MB_TYPE_P_Skip != pMacroBlock[mbAddrN[iLoop]].type) &&
                (VIDEO_H264_MB_TYPE_B_Skip != pMacroBlock[mbAddrN[iLoop]].type)
            ) && 
            (0 != ((pMacroBlock[mbAddrN[iLoop]].CodedBlockPatternLuma >> (cr4x4BlkIdxN[iLoop] >> 2)) & 1)) &&
            (1 == pMacroBlock[mbAddrN[iLoop]].transform_size_8x8_flag)
        )
        { transBlockN[iLoop] = cr4x4BlkIdxN[iLoop] >> 2; }
        else { transBlockN[iLoop] = VIDEO_H264_BLOCK_IDX_INVALID; }
    }
}
static inline void H264ReadAEGetNeighbouringBlock_coded_block_flag_13(PVIDEOH264DECODEINFO pDecodeInfo, HANSIZE mbAddrN[2], uint8_t transBlockN[2])
{
    PVIDEOH264MACROBLOCK pMacroBlock = pDecodeInfo->pPicInfo->pMacroBlock;
    uint8_t cr8x8BlkIdx = pDecodeInfo->i8x8;
    uint8_t cr8x8BlkIdxN[2];

    /* 代码其实和 H264ReadAEGetNeighbouringBlock_coded_block_flag_5 函数完全一样，只是换掉了块索引变量的名字，逻辑是完全一致的 */
    DerivationProcessForNeighbouring8x8LumaBlock(pDecodeInfo, cr8x8BlkIdx, mbAddrN, cr8x8BlkIdxN);
    for (HANSIZE iLoop = 0; iLoop < 2; iLoop++)
    {
        if (
            (VIDEO_H264_MACRO_BLOCK_ADDR_INVALID != mbAddrN[iLoop]) &&
            (
                (VIDEO_H264_MB_TYPE_P_Skip != pMacroBlock[mbAddrN[iLoop]].type) &&
                (VIDEO_H264_MB_TYPE_B_Skip != pMacroBlock[mbAddrN[iLoop]].type) &&
                (VIDEO_H264_MB_TYPE_I_PCM != pMacroBlock[mbAddrN[iLoop]].type)
            ) && 
            (0 != ((pMacroBlock[mbAddrN[iLoop]].CodedBlockPatternLuma >> cr8x8BlkIdx) & 1)) &&
            (1 == pMacroBlock[mbAddrN[iLoop]].transform_size_8x8_flag)
        )
        { transBlockN[iLoop] = cr8x8BlkIdxN[iLoop]; }
        else { transBlockN[iLoop] = VIDEO_H264_BLOCK_IDX_INVALID; }
    }
}

static uint8_t H264ReadAEDecodeBin(PVIDEOH264DECODEINFO pDecodeInfo, VIDEOH264CABACCTXIDX ctxIdx)
{
    uint8_t bRet;

    switch (ctxIdx) {
        case 276: { bRet = H264ReadAEDecodeTerminate(pDecodeInfo); } break;
        case (VIDEOH264CABACCTXIDX)VIDEO_H264_CABAC_BYPASS: { bRet = H264ReadAEDecodeBypass(pDecodeInfo); } break;
        default: { bRet = H264ReadAEDecodeDecision(pDecodeInfo, ctxIdx); } break;
    }

    return bRet;
}
static uint8_t H264ReadAEDecodeDecision(PVIDEOH264DECODEINFO pDecodeInfo, VIDEOH264CABACCTXIDX ctxIdx)
{
    uint8_t binVal;
    PVIDEOH264READAE pCABAC = pDecodeInfo->pCABAC;
    VIDEOH264CABACCONTEXTNUM* pStateIdx = &(pCABAC->context[ctxIdx].pStateIdx);
    VIDEOH264CABACCONTEXTNUM* valMPS = &(pCABAC->context[ctxIdx].valMPS);
    VIDEOH264CABACENGINENUM* codIRange = &(pCABAC->engine.codIRange);
    VIDEOH264CABACENGINENUM* codIOffset = &(pCABAC->engine.codIOffset);
    VIDEOH264CABACENGINENUM qCodIRangeIdx = (*codIRange >> 6) & 3; // 参考标准的公式 9-25
    VIDEOH264CABACENGINENUM codIRangeLPS = sg_pCABACrangeTabLPS[*pStateIdx][qCodIRangeIdx]; // 参考标准的公式 9-26

    /* 标准的流程图 9-3 概括了章节 9.3.3.2.1 和 9.3.3.2.1.1 的内容，且更直观，代码参照该流程图实现，逻辑与章节的文字介绍是一致的 */
    *codIRange = *codIRange - codIRangeLPS;
    if (*codIRange <= *codIOffset)
    {
        binVal = 1 - *valMPS;
        *codIOffset -= *codIRange;
        *codIRange = codIRangeLPS;
        if (0 == *pStateIdx) { *valMPS = 1 - *valMPS; }
        *pStateIdx = sg_pCABACtransIdxLPS[*pStateIdx];
    }
    else
    {
        binVal = *valMPS;
        *pStateIdx = sg_pCABACtransIdxMPS[*pStateIdx];
    }
    H264ReadAERenormD(pDecodeInfo);

    return binVal;
}
static void H264ReadAERenormD(PVIDEOH264DECODEINFO pDecodeInfo)
{
    PVIDEOH264READAE pCABAC = pDecodeInfo->pCABAC;
    VIDEOH264CABACENGINENUM* codIRange = &(pCABAC->engine.codIRange);
    VIDEOH264CABACENGINENUM* codIOffset = &(pCABAC->engine.codIOffset);

    while (*codIRange < 256)
    {
        *codIRange <<= 1;
        *codIOffset = (*codIOffset << 1) | H264ReadBit(pDecodeInfo->pReadBit);
    }
    /* 该章节的最后一段要求 codIOffset 的最终结果必须小于 codIRange，这两个变量都是实际只使用 9 bit 的 */
    *codIOffset &= 0x01FF;
}
static uint8_t H264ReadAEDecodeBypass(PVIDEOH264DECODEINFO pDecodeInfo)
{
    uint8_t binVal;
    PVIDEOH264READAE pCABAC = pDecodeInfo->pCABAC;
    VIDEOH264CABACENGINENUM* codIRange = &(pCABAC->engine.codIRange);
    VIDEOH264CABACENGINENUM* codIOffset = &(pCABAC->engine.codIOffset);
    
    /* 参考标准 9.3.3.2.3 章节以及流程图 9-5 */
    *codIOffset = (*codIOffset << 1) | H264ReadBits(pDecodeInfo->pReadBit, 1);
    if (*codIRange <= *codIOffset)
    {
        binVal = 1;
        *codIOffset -= *codIRange;
    }
    else { binVal = 0; }
    /* 由于 Bypass 模式不涉及 codIRange 的更新，并且 codIOffset 会自动更新成小于 codIRange，所以这里不必再限制 codIOffset 的位数，自动控制在 9 位的 */

    return binVal;
}
static uint8_t H264ReadAEDecodeTerminate(PVIDEOH264DECODEINFO pDecodeInfo)
{
    uint8_t binVal;
    PVIDEOH264READAE pCABAC = pDecodeInfo->pCABAC;
    VIDEOH264CABACENGINENUM* codIRange = &(pCABAC->engine.codIRange);
    VIDEOH264CABACENGINENUM* codIOffset = &(pCABAC->engine.codIOffset);
    
    /* 参考标准 9.3.3.2.4 章节以及流程图 9-6 */
    *codIRange -= 2;
    if (*codIRange <= *codIOffset) { binVal = 1; }
    else
    {
        binVal = 0;
        H264ReadAERenormD(pDecodeInfo);
    }

    return binVal;
}

static void DecodeH264_residual_block_cabac(PVIDEOH264DECODEINFO pDecodeInfo)
{
    PVIDEOH264PICTUREINFO pPicInfo = pDecodeInfo->pPicInfo;
    PCVIDEOH264SPS pSPS = pPicInfo->slice.pSPS;
    PVIDEOH264MACROBLOCK pMacroBlock = &(pPicInfo->pMacroBlock[pDecodeInfo->CurrMbAddr]);
    PVIDEOH264RESIDUALBLOCKSECABAC pSE = &(pMacroBlock->residual_block_SE.cabac);
    uint8_t ctxBlockCat = sg_pCtxBlockCatValue[pDecodeInfo->blockCat];

    PVIDEOH264COEFFLEVEL coeffLevel = pDecodeInfo->coeffLevel;
    int32_t startIdx = pDecodeInfo->startIdx;
    int32_t endIdx = pDecodeInfo->endIdx;
    uint8_t maxNumCoeff = pDecodeInfo->maxNumCoeff;
    int32_t iLoop;
    uint8_t numCoeff;

    pDecodeInfo->pCABAC->numDecodAbsLevelEq1 = 0;
    pDecodeInfo->pCABAC->numDecodAbsLevelGt1 = 0;

    if ((64 != maxNumCoeff) || (3 != pSPS->ChromaArrayType))
    {
        pSE->coded_block_flag[ctxBlockCat][pDecodeInfo->iCbCr][GetDecodingBlockIdx(pDecodeInfo)] = H264ReadAE_coded_block_flag(pDecodeInfo).u32;
    }
    for (iLoop = 0; iLoop < maxNumCoeff; iLoop++)
    {
        coeffLevel[iLoop] = 0;
    }
    if (0 != pSE->coded_block_flag[ctxBlockCat][pDecodeInfo->iCbCr][GetDecodingBlockIdx(pDecodeInfo)])
    {
        numCoeff = (uint8_t)(endIdx + 1);
        iLoop = (uint8_t)startIdx;
        while (iLoop < (numCoeff - 1))
        {
            pSE->significant_coeff_flag[iLoop] = H264ReadAE_significant_coeff_flag(pDecodeInfo, iLoop).u32;
            if (0 != pSE->significant_coeff_flag[iLoop])
            {
                pSE->last_significant_coeff_flag[iLoop] = H264ReadAE_last_significant_coeff_flag(pDecodeInfo, iLoop).u32;
                if (0 != pSE->last_significant_coeff_flag[iLoop])
                {
                    numCoeff = iLoop + 1;
                }
            }
            iLoop++;
        }
        pSE->coeff_abs_level_minus1[numCoeff - 1] = H264ReadAE_coeff_abs_level_minus1(pDecodeInfo).u32;
        pSE->coeff_sign_flag[numCoeff - 1] = H264ReadAE_coeff_sign_flag(pDecodeInfo).u32;
        coeffLevel[numCoeff - 1] = DecodeH264CalcCoeffLevel(pDecodeInfo, pSE->coeff_abs_level_minus1[numCoeff - 1], pSE->coeff_sign_flag[numCoeff - 1]);
        for (iLoop = numCoeff - 2; iLoop >= startIdx; iLoop--)
        {
            pSE->coeff_abs_level_minus1[iLoop] = H264ReadAE_coeff_abs_level_minus1(pDecodeInfo).u32;
            pSE->coeff_sign_flag[iLoop] = H264ReadAE_coeff_sign_flag(pDecodeInfo).u32;
            coeffLevel[iLoop] = DecodeH264CalcCoeffLevel(pDecodeInfo, pSE->coeff_abs_level_minus1[iLoop], pSE->coeff_sign_flag[iLoop]);
        }
    }
}
static inline VIDEOH264COEFFLEVEL DecodeH264CalcCoeffLevel(PVIDEOH264DECODEINFO pDecodeInfo, uint32_t coeff_abs_level_minus1, uint8_t coeff_sign_flag)
{
    VIDEOH264COEFFLEVEL coeffLevel = (VIDEOH264COEFFLEVEL)(coeff_abs_level_minus1 + 1);
    
    /* coeffLevel[i] = (coeff_abs_level_minus1[i] + 1) * (1 ? 2 * coeff_sign_flag[i]) 这条语句太啰嗦
     * 其实就是 coeff_abs_level_minus1 = abs(coeffLevel) - 1，coeff_sign_flag 是 coeffLevel 的正负号，0 表示正数，1 表示负数
     * 并且在读取 coeff_abs_level_minus1 还需要计算 abs(coeffLevel) 等于 1 和大于 1 的计数（numDecodAbsLevelEq1 和 numDecodAbsLevelGt1）
     * 这等价于 coeff_abs_level_minus1 等于 0 和大于 0 的计数
     * 所以这里不直接照抄标准的代码了，改用逻辑判断，并且计数方式改成判断 coeff_abs_level_minus1
     */
    if (0 != coeff_sign_flag) { coeffLevel = -coeffLevel; }
    if (0 == coeff_abs_level_minus1) { (pDecodeInfo->pCABAC->numDecodAbsLevelEq1)++; }
    else { (pDecodeInfo->pCABAC->numDecodAbsLevelGt1)++; }

    return coeffLevel;
}

static inline PVIDEOH264MACROBLOCK GetCurrentMacroBlock(PVIDEOH264DECODEINFO pDecodeInfo)
{
    return &(pDecodeInfo->pPicInfo->pMacroBlock[pDecodeInfo->CurrMbAddr]);
}
static VIDEOH264FRAMEFIELDMODE GetMacroBlockFrameFiledMode(PVIDEOH264DECODEINFO pDecodeInfo, PVIDEOH264MACROBLOCK pMacroBlock)
{
    (void)pDecodeInfo;
    VIDEOH264FRAMEFIELDMODE retMode;

    if (0 == pMacroBlock->mb_field_decoding_flag) { retMode = VIDEO_H264_FRAME_FIELD_MODE_FRAME; }
    else { retMode = VIDEO_H264_FRAME_FIELD_MODE_FIELD; }

    return retMode;
}
static inline uint8_t GetDecodingBlockIdx(PVIDEOH264DECODEINFO pDecodeInfo)
{
    return ((pDecodeInfo->i8x8 << 2) + pDecodeInfo->i4x4);
}
