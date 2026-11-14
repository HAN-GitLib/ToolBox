/* 以下所有提及的参考标准均指文件《Rec. ITU-T H.264》
 * H.264 对图片的采样与 JPEG 基本相同，用 YCbCr 的方式进行 4:4:4、4:2:2、4:2:0 的采样，由 chroma_format_idc 决定，但有帧和场的概念
 * 帧：正常采样，直接扫描出一张完整的图
 * 场：一张图片分两次隔行扫描，奇数行为顶场，偶数行为底场
 */

#include <stdbool.h>
#include <math.h>

#include "HAN_VideoH264.h"
#include "HAN_VideoH264Std.h"
#include "HAN_VideoH264Addr.h"
#include "HAN_VideoH264CABAC.h"

typedef enum {
    VIDEO_H264_COMPONENT_Y,
    VIDEO_H264_COMPONENT_Cb,
    VIDEO_H264_COMPONENT_Cr,
    VIDEO_H264_COMPONENT_CNT,
} VIDEOH264COMPONENT;
typedef enum {
    VIDEO_H264_BLOCK_DC,
    VIDEO_H264_BLOCK_AC,
    VIDEO_H264_BLOCK_4x4,
    VIDEO_H264_BLOCK_8x8,
    VIDEO_H264_BLOCK_CNT,
} VIDEOH264BLOCK;

#if 1 /* 码流处理 */
#if 1 /* 一些杂七杂八的比特流处理和运算 */
static inline void H264AlignByte(PVIDEOH264READBIT pReadBit);
static bool H264CheckMoreRbspData(PVIDEOH264READBIT pReadBit);
#endif

#if 1 /* 指数哥伦布码，也是 CAVLC 用到的读码方式，规则比较简单，参考标准的 9.1 章节或者网上查资料就行，这里不详细介绍 */
static void H264InitCAVLCHandler(PVIDEOH264DECODER pDecoder);
static inline VIDEOH264READBITRET H264ReadUE(PVIDEOH264READBIT pReadBit);
static inline VIDEOH264READBITRET H264ReadSE(PVIDEOH264READBIT pReadBit);
#endif
#endif

static inline uint8_t H264BoolToU8(bool bExpression);

static VIDEOH264READBITRET DecodeH264_nal_unit(PVIDEOH264_nal_unit pNALU, PVIDEOH264READBIT pReadBit);

static VIDEOH264READBITRET DecodeH264Parameter_seq_parameter_set_data_ProfileIdcExtendedData(PVIDEOH264SPS pSPS, PVIDEOH264READBIT pReadBit);
static VIDEOH264READBITRET DecodeH264Parameter_vui_parameters(PVIDEOH264PARAMETER_vui_parameters pVUI, PVIDEOH264READBIT pReadBit);
static VIDEOH264READBITRET DecodeH264Parameter_hrd_parameters(PVIDEOH264PARAMETER_hrd_parameters pHRD, PVIDEOH264READBIT pReadBit);

static VIDEOH264READBITRET DecodeH264_user_data_unregistered(PVIDEOH264READBIT pReadBit, HANSIZE payloadSize);

static VIDEOH264READBITRET DecodeH264ParameterRB_slice_header(PVIDEOH264SLICE pSlice, PVIDEOH264READBIT pReadBit, PVIDEOH264PARAMLIST pParamList);
static VIDEOH264READBITRET DecodeH264Parameter_ref_pic_list_mvc_modification(PVIDEOH264PARAMETER_ref_pic_list_mvc pRefPicListMVC, uint32_t sliceType, PVIDEOH264READBIT pReadBit);
static VIDEOH264READBITRET DecodeH264Parameter_ref_pic_list_modification(PVIDEOH264PARAMETER_ref_pic_list pRefPicList, uint32_t sliceType, PVIDEOH264READBIT pReadBit);
static VIDEOH264READBITRET DecodeH264Parameter_pred_weight_table(PVIDEOH264PARAMETER_pred_weight_table pPredWeightTable, PCVIDEOH264SLICE pSlice, PVIDEOH264READBIT pReadBit);
static VIDEOH264READBITRET DecodeH264Parameter_dec_ref_pic_marking(PVIDEOH264PARAMETER_dec_ref_pic_marking pDecRefPicMarking, uint8_t idrPicFlag, PVIDEOH264READBIT pReadBit);

static VIDEOH264READBITRET DecodeH264_slice_data(PVIDEOH264PICTUREINFO pPicInfo, PVIDEOH264READBIT pReadBit);
static VIDEOH264READBITRET DecodeH264SliceProcess(PVIDEOH264DECODER pDecoder);
static VIDEOH264READBITRET DecodeH264_macroblock(PVIDEOH264DECODER pDecoder);
static VIDEOH264READBITRET DecodeH264_mb_pred(PVIDEOH264DECODER pDecoder);
static VIDEOH264READBITRET DecodeH264_sub_mb_pred(PVIDEOH264DECODER pDecoder);
static VIDEOH264READBITRET DecodeH264_residual(PVIDEOH264DECODER pDecoder, int32_t startIdx, int32_t endIdx);
static VIDEOH264READBITRET DecodeH264_residual_luma(PVIDEOH264DECODER pDecoder, PVIDEOH264COEFFLEVELBLOCK levelBlock, int32_t startIdx, int32_t endIdx, VIDEOH264COMPONENT component);
static inline void DecodeH264_residual_block(PVIDEOH264DECODER pDecoder, PVIDEOH264COEFFLEVEL coeffLevel, int32_t startIdx, int32_t endIdx, uint8_t maxNumCoeff,
    uint8_t iCbCr, uint8_t i8x8, uint8_t i4x4, VIDEOH264BLOCKCAT blockCat);

static void DecodeH264SkipBlock(PVIDEOH264DECODER pDecoder);
static HANSIZE DecodeH264NextMbAddress(HANSIZE n, PCVIDEOH264SLICE pSlice);

static VIDEOH264READBITRET DecodeH264_mb_pred_I_NxN(PVIDEOH264DECODER pDecoder);
static VIDEOH264READBITRET DecodeH264_mb_pred_NotDirect(PVIDEOH264DECODER pDecoder);

#if 1 /* 宏块类型相关函数 */
static uint8_t DecodeH264GetNumSubMbPart(PCVIDEOH264MACROBLOCK pMacroBlock);
#endif

static bool DecodeH264GenerateMbToSliceGroupMap(PVIDEOH264SLICE pSlice);                        // 本函数实现标准的 8.2.2 章节所述：选择 MbToSliceGroupMap 的生成方式
static void DecodeH264GenerateMbToSliceGroupMapInterleaved(PVIDEOH264SLICE pSlice);             // 本函数实现标准的 8.2.2.1 章节所述功能
static void DecodeH264GenerateMbToSliceGroupMapDispersed(PVIDEOH264SLICE pSlice);               // 本函数实现标准的 8.2.2.2 章节所述功能
static void DecodeH264GenerateMbToSliceGroupMapForegroundWithLeftOver(PVIDEOH264SLICE pSlice);  // 本函数实现标准的 8.2.2.3 章节所述功能
static void DecodeH264GenerateMbToSliceGroupMapBoxOut(PVIDEOH264SLICE pSlice);                  // 本函数实现标准的 8.2.2.4 章节所述功能
static void DecodeH264GenerateMbToSliceGroupMapRasterScan(PVIDEOH264SLICE pSlice);              // 本函数实现标准的 8.2.2.5 章节所述功能
static void DecodeH264GenerateMbToSliceGroupMapWipe(PVIDEOH264SLICE pSlice);                    // 本函数实现标准的 8.2.2.6 章节所述功能
static void DecodeH264GenerateMbToSliceGroupMapExplicit(PVIDEOH264SLICE pSlice);                // 本函数实现标准的 8.2.2.7 章节所述功能

static inline void DecodeH264ReadSyntaxElementCallback_coded_block_pattern(PVIDEOH264MACROBLOCK pMacroBlock, uint32_t coded_block_pattern);

static const VIDEOH264BLOCKCAT sg_pBlockCatMap[VIDEO_H264_COMPONENT_CNT][VIDEO_H264_BLOCK_CNT] = {
    [VIDEO_H264_COMPONENT_Y] = {
        [VIDEO_H264_BLOCK_DC] = VIDEO_H264_BLOCK_CAT_LumaDC,
        [VIDEO_H264_BLOCK_AC] = VIDEO_H264_BLOCK_CAT_LumaAC,
        [VIDEO_H264_BLOCK_4x4] = VIDEO_H264_BLOCK_CAT_Luma4x4,
        [VIDEO_H264_BLOCK_8x8] = VIDEO_H264_BLOCK_CAT_Luma8x8,
    },
    [VIDEO_H264_COMPONENT_Cb] = {
        [VIDEO_H264_BLOCK_DC] = VIDEO_H264_BLOCK_CAT_CbDC,
        [VIDEO_H264_BLOCK_AC] = VIDEO_H264_BLOCK_CAT_CbAC,
        [VIDEO_H264_BLOCK_4x4] = VIDEO_H264_BLOCK_CAT_Cb4x4,
        [VIDEO_H264_BLOCK_8x8] = VIDEO_H264_BLOCK_CAT_Cb8x8,
    },
    [VIDEO_H264_COMPONENT_Cr] = {
        [VIDEO_H264_BLOCK_DC] = VIDEO_H264_BLOCK_CAT_CrDC,
        [VIDEO_H264_BLOCK_AC] = VIDEO_H264_BLOCK_CAT_CrAC,
        [VIDEO_H264_BLOCK_4x4] = VIDEO_H264_BLOCK_CAT_Cr4x4,
        [VIDEO_H264_BLOCK_8x8] = VIDEO_H264_BLOCK_CAT_Cr8x8,
    },
};

size_t GetmapUnitToSliceGroupMapSize(PCVIDEOH264SPS pSPS)
{
    return (pSPS->PicSizeInMapUnits * sizeof(VIDEOH264SLICEGROUPMAP));
}
size_t GetMbToSliceGroupMapSize(PCVIDEOH264SPS pSPS)
{
    /* mapUnitToSliceGroupMap 的大小是 PicSizeInMapUnits，在 SPS 中决定
     * MbToSliceGroupMap 的大小是 PicSizeInMbs，在 slice 中决定
     * 这两个大小的计算公式如下：
     * PicSizeInMapUnits = PicWidthInMbs * PicHeightInMapUnits
     * PicSizeInMbs      = PicWidthInMbs * PicHeightInMbs
     * 其中：
     * PicHeightInMbs    = FrameHeightInMbs / (1 + field_pic_flag)
     * FrameHeightInMbs  = (2 - pData->frame_mbs_only_flag) * PicHeightInMapUnits
     * 得到：
     * PicHeightInMbs    = (2 - pData->frame_mbs_only_flag) * PicHeightInMapUnits / (1 + field_pic_flag)
     * PicSizeInMbs      = PicWidthInMbs * PicHeightInMapUnits * (2 - pData->frame_mbs_only_flag) / (1 + field_pic_flag)
     *                   = PicSizeInMapUnits * (2 - pData->frame_mbs_only_flag) / (1 + field_pic_flag)
     * 由于 frame_mbs_only_flag 和 field_pic_flag 都只可能是 0 或 1
     * 且只有 frame_mbs_only_flag == 0 时， field_pic_flag 才有效。当 frame_mbs_only_flag == 1 时，field_pic_flag 一定等于 0
     * 所以 PicSizeInMbs 要么是 PicSizeInMapUnits 的两倍，要么等于 PicSizeInMapUnits
     */
    return (2 * GetmapUnitToSliceGroupMapSize(pSPS));
}

VIDEOH264PROFILETYPE GetSPSProfileType(PCVIDEOH264SPS pSPS)
{
    VIDEOH264PROFILETYPE eRet;

    switch (pSPS->profile_idc) {
        case 66: { eRet = VIDEO_H264_PROFILE_TYPE_BASELINE; } break;
        case 77: { eRet = VIDEO_H264_PROFILE_TYPE_MAIN; } break;
        case 88: { eRet = VIDEO_H264_PROFILE_TYPE_EXTENDED; } break;
        case 100: {
            if (1 == pSPS->constraint_set4_flag)
            {
                if (1 == pSPS->constraint_set5_flag) { eRet = VIDEO_H264_PROFILE_TYPE_CONSTRAINED_HIGH; }
                else { eRet = VIDEO_H264_PROFILE_TYPE_PROGRESSIVE_HIGH; }
            }
            else { eRet = VIDEO_H264_PROFILE_TYPE_HIGH; }
        } break;
        case 110: {
            if (1 == pSPS->constraint_set3_flag) { eRet = VIDEO_H264_PROFILE_TYPE_HIGH_10_INTRA; }
            else if (1 == pSPS->constraint_set4_flag) { eRet = VIDEO_H264_PROFILE_TYPE_PROGRESSIVE_HIGH_10; }
            else { eRet = VIDEO_H264_PROFILE_TYPE_HIGH_10; }
        } break;
        case 122: {
            if (1 == pSPS->constraint_set3_flag) { eRet = VIDEO_H264_PROFILE_TYPE_HIGH_422_INTRA; }
            else { eRet = VIDEO_H264_PROFILE_TYPE_HIGH_422; }
        } break;
        case 244: {
            if (1 == pSPS->constraint_set3_flag) { eRet = VIDEO_H264_PROFILE_TYPE_HIGH_444_INTRA; }
            else { eRet = VIDEO_H264_PROFILE_TYPE_HIGH_444_PREDICTIVE; }
        } break;
        case 44: { eRet = VIDEO_H264_PROFILE_TYPE_CAVLC_444_INTRA; } break;
        default: { eRet = VIDEO_H264_PROFILE_TYPE_CNT; } break;
    }

    return eRet;
}

void GetNalUnit(uint8_t pData, PVIDEOH264_nal_unit pNALU)
{
    pNALU->nal_ref_idc = (pData & 0x60) >> 5;
    pNALU->nal_unit_type = pData & 0x1F;
}
uint8_t GetIdrPicFlag(uint8_t nal_unit_type)
{
    uint8_t nRet;

    if (5 == nal_unit_type) { nRet = 1; }
    else { nRet = 0; }

    return nRet;
}

uint32_t DecodeH264Parameter_seq_parameter_set(const uint8_t* pData, uint32_t nSize, PVIDEOH264SPS pSPS)
{
    BOOL bOk = TRUE;
    VIDEOMP4READBIT readUE = H264InitReadBit(pData, nSize);
    
    (void)H264ReadBit(&readUE); // forbidden_zero_bit
    pSPS->nal_ref_idc = (uint8_t)H264ReadBits(&readUE, 2);
    pSPS->nal_unit_type = (uint8_t)H264ReadBits(&readUE, 5);
    pSPS->profile_idc = (uint8_t)H264ReadBits(&readUE, 8);
    pSPS->constraint_set0_flag = H264ReadBit(&readUE);
    pSPS->constraint_set1_flag = H264ReadBit(&readUE);
    pSPS->constraint_set2_flag = H264ReadBit(&readUE);
    pSPS->constraint_set3_flag = H264ReadBit(&readUE);
    pSPS->constraint_set4_flag = H264ReadBit(&readUE);
    pSPS->constraint_set5_flag = H264ReadBit(&readUE);
    pSPS->reserved_zero_2bits = (uint8_t)H264ReadBits(&readUE, 2);
    pSPS->level_idc = (uint8_t)H264ReadBits(&readUE, 8);
    (void)H264ReadUE(&readUE); pSPS->seq_parameter_set_id = readUE.cValue.u32;
    DecodeH264Parameter_seq_parameter_set_data_ProfileIdcExtendedData(pSPS, &readUE);
    (void)H264ReadUE(&readUE); pSPS->log2_max_frame_num_minus4 = readUE.cValue.u32;
    (void)H264ReadUE(&readUE); pSPS->pic_order_cnt_type = readUE.cValue.u32;
    if (0 == pSPS->pic_order_cnt_type) { (void)H264ReadUE(&readUE); pSPS->log2_max_pic_order_cnt_lsb_minus4 = readUE.cValue.u32; }
    else if (1 == pSPS->pic_order_cnt_type)
    {
        pSPS->delta_pic_order_always_zero_flag = H264ReadBit(&readUE);
        (void)H264ReadSE(&readUE); pSPS->offset_for_non_ref_pic = readUE.cValue.s32;
        (void)H264ReadSE(&readUE); pSPS->offset_for_top_to_bottom_field = readUE.cValue.s32;
        (void)H264ReadUE(&readUE); pSPS->num_ref_frames_in_pic_order_cnt_cycle = readUE.cValue.u32;
        if (pSPS->num_ref_frames_in_pic_order_cnt_cycle <= VIDEO_H264_MAX_VALUE_num_ref_frames_in_pic_order_cnt_cycle)
        {
            for (HANSIZE iLoop = 0; iLoop < pSPS->num_ref_frames_in_pic_order_cnt_cycle; iLoop++)
            {
                (void)H264ReadSE(&readUE);
                pSPS->offset_for_ref_frame[iLoop] = readUE.cValue.s32;
            }
        }
        else { bOk = FALSE; }
    }
    else { /* 根据标准，只有 0 和 1 需要读额外的内容 */ }
    if (bOk == TRUE)
    {
        (void)H264ReadUE(&readUE); pSPS->max_num_ref_frames = readUE.cValue.u32;
        pSPS->gaps_in_frame_num_value_allowed_flag = H264ReadBit(&readUE);
        (void)H264ReadUE(&readUE); pSPS->pic_width_in_mbs_minus1 = readUE.cValue.u32;
        (void)H264ReadUE(&readUE); pSPS->pic_height_in_map_units_minus1 = readUE.cValue.u32;
        pSPS->frame_mbs_only_flag = H264ReadBit(&readUE);
        if (0 == pSPS->frame_mbs_only_flag) { pSPS->mb_adaptive_frame_field_flag = H264ReadBit(&readUE); }
        pSPS->direct_8x8_inference_flag = H264ReadBit(&readUE);
        pSPS->frame_cropping_flag = H264ReadBit(&readUE);
        if (0 != pSPS->frame_cropping_flag)
        {
            (void)H264ReadUE(&readUE); pSPS->frame_crop_left_offset = readUE.cValue.u32;
            (void)H264ReadUE(&readUE); pSPS->frame_crop_right_offset = readUE.cValue.u32;
            (void)H264ReadUE(&readUE); pSPS->frame_crop_top_offset = readUE.cValue.u32;
            (void)H264ReadUE(&readUE); pSPS->frame_crop_bottom_offset = readUE.cValue.u32;
        }
        pSPS->vui_parameters_present_flag = H264ReadBit(&readUE);
        if (0 != pSPS->vui_parameters_present_flag) { DecodeH264Parameter_vui_parameters(&(pSPS->vui_parameters), &readUE); }
    }

    return (uint32_t)(readUE.iByte + 1);
}
bool SPSProfileIdcHasExtendedData(uint8_t nProfileIdc)
{
    bool bRet = false;

    if ((100 == nProfileIdc) ||
        (110 == nProfileIdc) ||
        (122 == nProfileIdc) ||
        (244 == nProfileIdc) ||
        (44 == nProfileIdc) ||
        (83 == nProfileIdc) ||
        (86 == nProfileIdc) ||
        (118 == nProfileIdc) ||
        (128 == nProfileIdc) ||
        (138 == nProfileIdc) ||
        (139 == nProfileIdc) ||
        (134 == nProfileIdc) ||
        (135 == nProfileIdc))
    { bRet = true; }

    return bRet;
}

uint32_t DecodeH264Parameter_pic_parameter_set(const uint8_t* pData, uint32_t nSize, PVIDEOH264PPS pPPS, uint32_t* slice_group_id)
{
    VIDEOMP4READBIT readUE = H264InitReadBit(pData, nSize);
    
    (void)H264ReadBit(&readUE); // forbidden_zero_bit
    pPPS->nal_ref_idc = (uint8_t)H264ReadBits(&readUE, 2);
    pPPS->nal_unit_type = (uint8_t)H264ReadBits(&readUE, 5);
    (void)H264ReadUE(&readUE); pPPS->pic_parameter_set_id = readUE.cValue.u32;
    (void)H264ReadUE(&readUE); pPPS->seq_parameter_set_id = readUE.cValue.u32;
    pPPS->entropy_coding_mode_flag = H264ReadBit(&readUE);
    pPPS->bottom_field_pic_order_in_frame_present_flag = H264ReadBit(&readUE);
    (void)H264ReadUE(&readUE); pPPS->num_slice_groups_minus1 = readUE.cValue.u32;

    if (0 < pPPS->num_slice_groups_minus1)
    {
        (void)H264ReadUE(&readUE); pPPS->slice_group_map_type = readUE.cValue.u32;
        if (0 == pPPS->slice_group_map_type)
        {
            for (uint32_t iLoop = 0; iLoop <= pPPS->slice_group_map_type; iLoop++)
            {
                (void)H264ReadUE(&readUE); pPPS->run_length_minus1[iLoop] = readUE.cValue.u32;
            }
        }
        else if (2 == pPPS->slice_group_map_type)
        {
            for (uint32_t iLoop = 0; iLoop < pPPS->slice_group_map_type; iLoop++)
            {
                (void)H264ReadUE(&readUE); pPPS->top_left[iLoop] = readUE.cValue.u32;
                (void)H264ReadUE(&readUE); pPPS->bottom_right[iLoop] = readUE.cValue.u32;
            }
        }
        else if ((3 == pPPS->slice_group_map_type) ||
                 (4 == pPPS->slice_group_map_type) ||
                 (5 == pPPS->slice_group_map_type))
        {
            pPPS->slice_group_change_direction_flag = H264ReadBit(&readUE);
            (void)H264ReadUE(&readUE); pPPS->slice_group_change_rate_minus1 = readUE.cValue.u32;
        }
        else if (6 == pPPS->slice_group_map_type)
        {
            (void)H264ReadUE(&readUE); pPPS->pic_size_in_map_units_minus1 = readUE.cValue.u32;
            if (NULL != slice_group_id)
            {
                pPPS->slice_group_id = slice_group_id;
                for (uint32_t iLoop = 0; iLoop < pPPS->pic_size_in_map_units_minus1; iLoop++)
                {
                    // slice_group_id[iLoop] = u(v);
                }
            }
        }
        else { }
    }
    (void)H264ReadUE(&readUE); pPPS->num_ref_idx_l0_default_active_minus1 = readUE.cValue.u32;
    (void)H264ReadUE(&readUE); pPPS->num_ref_idx_l1_default_active_minus1 = readUE.cValue.u32;
    pPPS->weighted_pred_flag = H264ReadBit(&readUE);
    pPPS->weighted_bipred_idc = (uint8_t)H264ReadBits(&readUE, 2);
    (void)H264ReadSE(&readUE); pPPS->pic_init_qp_minus26 = readUE.cValue.s32;
    (void)H264ReadSE(&readUE); pPPS->pic_init_qs_minus26 = readUE.cValue.s32;
    (void)H264ReadSE(&readUE); pPPS->chroma_qp_index_offset = readUE.cValue.s32;
    pPPS->deblocking_filter_control_present_flag = H264ReadBit(&readUE);
    pPPS->constrained_intra_pred_flag = H264ReadBit(&readUE);
    pPPS->redundant_pic_cnt_present_flag = H264ReadBit(&readUE);

    if (true == H264CheckMoreRbspData(&readUE))
    {
        pPPS->bMoreRbspData = TRUE;
        pPPS->transform_8x8_mode_flag = H264ReadBit(&readUE);
        pPPS->pic_scaling_matrix_present_flag = H264ReadBit(&readUE);
        if (0 != pPPS->pic_scaling_matrix_present_flag)
        {
            ;
        }
        (void)H264ReadSE(&readUE); pPPS->second_chroma_qp_index_offset = readUE.cValue.s32;
    }
    else { pPPS->bMoreRbspData = FALSE; }

    return (uint32_t)(readUE.iByte + 1);
}

uint32_t DecodeH264Parameter_slice_header(const uint8_t* pData, uint32_t nSize, PVIDEOH264SLICE pSlice, PVIDEOH264PARAMLIST pParamList)
{
    VIDEOMP4READBIT readBit = H264InitReadBit(pData, nSize);

    DecodeH264ParameterRB_slice_header(pSlice, &readBit, pParamList);

    return (uint32_t)(readBit.iByte + 1);
}
VIDEOH264SLICETYPE DecodeH264NormalizeSliceType(VIDEOH264SLICETYPE eType)
{
    return (eType % 5);
}

bool H264DecodeBy_slice_layer_without_partitioning(PVIDEOH264_nal_unit pNALU)
{
    bool bRet;
    
    if ((1 == pNALU->nal_unit_type) || (5 == pNALU->nal_unit_type)) { bRet = true; }
    else { bRet = false; }

    return bRet;
}
uint32_t DecodeH264_slice_layer_without_partitioning(const uint8_t* pData, uint32_t nSize, PVIDEOH264PICTUREINFO pPicInfo, PVIDEOH264PARAMLIST pParamList)
{
    VIDEOMP4READBIT readBit = H264InitReadBit(pData, nSize);
    PVIDEOH264SLICE pSlice = &(pPicInfo->slice);

    DecodeH264ParameterRB_slice_header(pSlice, &readBit, pParamList);
    DecodeH264BinToVariableSliceHeader(pSlice);
    (void)DecodeH264GenerateMbToSliceGroupMap(pSlice); // 根据标准的 8.2.2 章节第三行，该函数在解析任何 slice 前都应重新生成一次 MbToSliceGroupMap
    DecodeH264_slice_data(pPicInfo, &readBit);

    return (uint32_t)(readBit.iByte + 1);
}

uint32_t DecodeH264_sei(const uint8_t* pData, uint32_t nSize)
{
    VIDEOMP4READBIT readBit = H264InitReadBit(pData, nSize);
    HANSIZE payloadSize;
    uint32_t payloadType;
    uint8_t seiByte;

    do {
        /* sei_message */
        payloadType = 0;
        do {
            seiByte = H264ReadBits(&readBit, 8);
            payloadType += seiByte;
        } while (0xFF == seiByte);
        payloadSize = 0;
        do {
            seiByte = H264ReadBits(&readBit, 8);
            payloadSize += seiByte;
        } while (0xFF == seiByte);

        switch (payloadType) {
            case 5: { DecodeH264_user_data_unregistered(&readBit, payloadSize); } break;
            default: { } break;
        }
    } while (true == H264CheckMoreRbspData(&readBit));

    return (uint32_t)(readBit.iByte + 1);
}

bool DecodeH264BinToVariableSPS(PVIDEOH264SPS pSPS)
{
    bool bRet = true;

    if (0 == pSPS->separate_colour_plane_flag)
    {
        switch (pSPS->chroma_format_idc) {
            case 1: { pSPS->SubWidthC = 2; pSPS->SubHeightC = 2; } break;
            case 2: { pSPS->SubWidthC = 2; pSPS->SubHeightC = 1; } break;
            case 3: { pSPS->SubWidthC = 1; pSPS->SubHeightC = 1; } break;
            default: { pSPS->SubWidthC = 0; pSPS->SubHeightC = 0; } break;
        }
    }
    else { pSPS->SubWidthC = 0; pSPS->SubHeightC = 0; }
    if (0 != pSPS->SubWidthC) { pSPS->MbWidthC = 16 / pSPS->SubWidthC; }
    else { pSPS->MbWidthC = 0; }
    if (0 != pSPS->SubHeightC) { pSPS->MbHeightC = 16 / pSPS->SubHeightC; }
    else { pSPS->MbHeightC = 0; }
    if (0 == pSPS->separate_colour_plane_flag) { pSPS->ChromaArrayType = pSPS->chroma_format_idc; }
    else { pSPS->ChromaArrayType = 0; }
    pSPS->BitDepthY = 8 + pSPS->bit_depth_luma_minus8;
    pSPS->QpBdOffsetY = 6 * pSPS->bit_depth_luma_minus8;
    pSPS->BitDepthC = 8 + pSPS->bit_depth_chroma_minus8;
    pSPS->QpBdOffsetC = 6 * pSPS->bit_depth_chroma_minus8;
    pSPS->RawMbBits = 256 * pSPS->BitDepthY + 2 * pSPS->MbWidthC * pSPS->MbHeightC * pSPS->BitDepthC;
    pSPS->PicWidthInMbs = pSPS->pic_width_in_mbs_minus1 + 1;
    pSPS->PicWidthInSamplesL = pSPS->PicWidthInMbs * 16;
    pSPS->PicWidthInSamplesC = pSPS->PicWidthInMbs * pSPS->MbWidthC;
    pSPS->PicHeightInMapUnits = pSPS->pic_height_in_map_units_minus1 + 1;
    pSPS->PicSizeInMapUnits = pSPS->PicWidthInMbs * pSPS->PicHeightInMapUnits;
    pSPS->FrameHeightInMbs = (2 - pSPS->frame_mbs_only_flag) * pSPS->PicHeightInMapUnits;
    if (0 == pSPS->ChromaArrayType)
    {
        pSPS->CropUnitX = 1;
        pSPS->CropUnitY = 2 - pSPS->frame_mbs_only_flag;
    }
    else
    {
        pSPS->CropUnitX = pSPS->SubWidthC;
        pSPS->CropUnitY =  pSPS->SubHeightC * (2 - pSPS->frame_mbs_only_flag);
    }

    return bRet;
}
bool DecodeH264BinToVariablePPS(PVIDEOH264PPS pPPS)
{
    bool bRet = true;

    pPPS->SliceGroupChangeRate = pPPS->slice_group_change_rate_minus1 + 1;

    return bRet;
}
bool DecodeH264BinToVariableSliceHeader(PVIDEOH264SLICE pSlice)
{
    bool bRet = true;
    PCVIDEOH264SPS pSPS = pSlice->pSPS;
    PCVIDEOH264PPS pPPS = pSlice->pPPS;
    PCVIDEOH264PARAMETER_slice_header pHeader = &(pSlice->slice_header);

    pSlice->type = DecodeH264NormalizeSliceType(pHeader->slice_type);
    pSlice->MbaffFrameFlag = H264BoolToU8((0 != pSPS->mb_adaptive_frame_field_flag) && (0 == pHeader->field_pic_flag)); // 参考标准公式 7-25
    pSlice->PicHeightInMbs = pSPS->FrameHeightInMbs / (1 + pHeader->field_pic_flag);                                    // 参考标准公式 7-26
    pSlice->PicHeightInSamplesL = pSlice->PicHeightInMbs * 16;                                                          // 参考标准公式 7-27
    pSlice->PicHeightInSamplesC = pSlice->PicHeightInMbs * pSPS->MbHeightC;                                             // 参考标准公式 7-28
    pSlice->PicSizeInMbs = pSPS->PicWidthInMbs * pSlice->PicHeightInMbs;                                                // 参考标准公式 7-29
    if (0 == pHeader->field_pic_flag) { pSlice->MaxPicNum = pSPS->MaxFrameNum; }
    else { pSlice->MaxPicNum = 2 * pSPS->MaxFrameNum; }
    if (0 == pHeader->field_pic_flag) { pSlice->CurrPicNum = pHeader->frame_num; }
    else { pSlice->CurrPicNum = 2 * pHeader->frame_num + 1; }
    pSlice->SliceQPY = 26 + pPPS->pic_init_qp_minus26 + pHeader->slice_qp_delta;                                        // 参考标准公式 7-30
    pSlice->QSY = 26 + pPPS->pic_init_qs_minus26 + pHeader->slice_qs_delta;                                             // 参考标准公式 7-31
    pSlice->FilterOffsetA = pHeader->slice_alpha_c0_offset_div2 << 1;                                                   // 参考标准公式 7-32
    pSlice->FilterOffsetB = pHeader->slice_beta_offset_div2 << 1;                                                       // 参考标准公式 7-33
    pSlice->MapUnitsInSliceGroup0 = H264Min(pHeader->slice_group_change_cycle * pPPS->SliceGroupChangeRate, pSPS->PicSizeInMapUnits);

    return bRet;
}

static inline void H264AlignByte(PVIDEOH264READBIT pReadBit)
{
    while (0 != pReadBit->iBit) { H264ReadBit(pReadBit); }
}
static bool H264CheckMoreRbspData(PVIDEOH264READBIT pReadBit)
{
    bool bRet = false;
    uint8_t cByte;

    if (pReadBit->iByte < pReadBit->nDataLen)
    {
        for (HANSIZE iLoop = pReadBit->nDataLen - 1; pReadBit->iByte <= iLoop; iLoop--)
        {
            if (0 != pReadBit->pData[iLoop])
            {
                if (iLoop != pReadBit->iByte) { bRet = true; }
                else
                {
                    /* 从左往右从 0 开始数，当前指向第 n 比特，那么只要 n + 1 比特开始，直到 bit 7 任意不为 0 就说明还有数据 */
                    cByte = 0xFF >> (pReadBit->iBit + 1);
                    cByte &= pReadBit->pData[iLoop];
                    if (cByte != 0) { bRet = true; }
                }
                break;
            }
        }
    }

    return bRet;
}

#if 1 /* CAVLC */
static void H264InitCAVLCHandler(PVIDEOH264DECODER pDecoder)
{
    /* slice_data */
    pDecoder->readSE.mb_skip_run = NULL;
    pDecoder->readSE.mb_field_decoding_flag = NULL;
    /* 宏块 */
    pDecoder->readSE.mb_type = NULL;
    pDecoder->readSE.transform_size_8x8_flag = NULL;
    pDecoder->readSE.coded_block_pattern = NULL;
    pDecoder->readSE.mb_qp_delta = NULL;
    /* mb_pred */
    pDecoder->readSE.prev_intraNxN_pred_mode_flag = NULL;
    pDecoder->readSE.rem_intraNxN_pred_mode = NULL;
    pDecoder->readSE.intra_chroma_pred_mode = NULL;
}
static inline VIDEOH264READBITRET H264ReadUE(PVIDEOH264READBIT pReadBit)
{
    VIDEOH264READBITRET eRet = VIDEO_H264_READ_BIT_RET_ERROR;
    HANSIZE nBitLen = ((pReadBit->nDataLen - pReadBit->iByte) * 8) - pReadBit->iBit;
    HANSIZE nReadBitMax = nBitLen;
    uint8_t nZeroBits;
    uint32_t cX;

    if (31 < nReadBitMax) { nReadBitMax = 31; }

    for (nZeroBits = 0; nZeroBits < nReadBitMax; nZeroBits++)
    {
        if (0 != H264ReadBit(pReadBit))
        {
            nBitLen -= 1 + nZeroBits;
            break;
        }
    }
    if (nZeroBits <= nBitLen)
    {
        cX = 0;
        for (uint8_t iLoop = 0; iLoop < nZeroBits; iLoop++)
        {
            cX = (cX << 1) + H264ReadBit(pReadBit);
        }
        pReadBit->cValue.u32 = (1 << nZeroBits) - 1 + cX;

        eRet = VIDEO_H264_READ_BIT_RET_OK;
    }
    
    return eRet;
}
static inline VIDEOH264READBITRET H264ReadSE(PVIDEOH264READBIT pReadBit)
{
    VIDEOH264READBITRET eRet = H264ReadUE(pReadBit);
    int32_t s32;

    if (VIDEO_H264_READ_BIT_RET_OK == eRet)
    {
        if ((1 & pReadBit->cValue.u32) == 0) { s32 = -1; }
        else { s32 = 1; }
        pReadBit->cValue.s32 = s32 * ((pReadBit->cValue.u32 + 1) >> 1);
    }
    
    return eRet;
}
#endif

static inline uint8_t H264BoolToU8(bool bExpression)
{
    uint8_t nRet;

    if (false == bExpression) { nRet = 0; }
    else { nRet = 1; }

    return nRet;
}

static VIDEOH264READBITRET DecodeH264_nal_unit(PVIDEOH264_nal_unit pNALU, PVIDEOH264READBIT pReadBit)
{
    VIDEOH264READBITRET eRet = VIDEO_H264_READ_BIT_RET_OK;

    (void)H264ReadBit(pReadBit);
    pNALU->nal_ref_idc = H264ReadBits(pReadBit, 2);
    pNALU->nal_unit_type = H264ReadBits(pReadBit, 5);

    return eRet;
}

static VIDEOH264READBITRET DecodeH264Parameter_seq_parameter_set_data_ProfileIdcExtendedData(PVIDEOH264SPS pSPS, PVIDEOH264READBIT pReadBit)
{
    VIDEOH264READBITRET eRet = VIDEO_H264_READ_BIT_RET_OK;

    if (TRUE == SPSProfileIdcHasExtendedData(pSPS->profile_idc))
    {
        pSPS->bExtendedProfileIdcData = TRUE;
        (void)H264ReadUE(pReadBit); pSPS->chroma_format_idc = pReadBit->cValue.u32;
        if (3 == pSPS->chroma_format_idc) { pSPS->separate_colour_plane_flag = H264ReadBit(pReadBit); }
        (void)H264ReadUE(pReadBit); pSPS->bit_depth_luma_minus8 = pReadBit->cValue.u32;
        (void)H264ReadUE(pReadBit); pSPS->bit_depth_chroma_minus8 = pReadBit->cValue.u32;
        pSPS->qpprime_y_zero_transform_bypass_flag = H264ReadBit(pReadBit);
        pSPS->seq_scaling_matrix_present_flag = H264ReadBit(pReadBit);
        if (FALSE != pSPS->seq_scaling_matrix_present_flag)
        {
            printf("矩阵信息解码程序未开发\n");
        }
    }
    else { pSPS->bExtendedProfileIdcData = FALSE; }

    return eRet;
}
static VIDEOH264READBITRET DecodeH264Parameter_vui_parameters(PVIDEOH264PARAMETER_vui_parameters pVUI, PVIDEOH264READBIT pReadBit)
{
    VIDEOH264READBITRET eRet = VIDEO_H264_READ_BIT_RET_OK;

    pVUI->aspect_ratio_info_present_flag = H264ReadBit(pReadBit);
    if (0 != pVUI->aspect_ratio_info_present_flag)
    {
        pVUI->aspect_ratio_idc = H264ReadBits(pReadBit, 8);
        if (VIDEO_H264_EXTENDED_SAR == pVUI->aspect_ratio_idc)
        {
            pVUI->sar_width = (uint16_t)H264ReadBits(pReadBit, 16);
            pVUI->sar_height = (uint16_t)H264ReadBits(pReadBit, 16);
        }
    }
    pVUI->overscan_info_present_flag = H264ReadBit(pReadBit);
    if (0 != pVUI->overscan_info_present_flag) { pVUI->overscan_appropriate_flag = H264ReadBit(pReadBit); }
    pVUI->video_signal_type_present_flag = H264ReadBit(pReadBit);
    if (0 != pVUI->video_signal_type_present_flag)
    {
        pVUI->video_format = (uint8_t)H264ReadBits(pReadBit, 3);
        pVUI->video_full_range_flag = H264ReadBit(pReadBit);
        pVUI->colour_description_present_flag = H264ReadBit(pReadBit);
        if (0 != pVUI->colour_description_present_flag)
        {
            pVUI->colour_primaries = (uint8_t)H264ReadBits(pReadBit, 8);
            pVUI->transfer_characteristics = (uint8_t)H264ReadBits(pReadBit, 8);
            pVUI->matrix_coefficients = (uint8_t)H264ReadBits(pReadBit, 8);
        }
    }
    pVUI->chroma_loc_info_present_flag = H264ReadBit(pReadBit);
    if (0 != pVUI->chroma_loc_info_present_flag)
    {
        (void)H264ReadUE(pReadBit); pVUI->chroma_sample_loc_type_top_field = pReadBit->cValue.u32;
        (void)H264ReadUE(pReadBit); pVUI->chroma_sample_loc_type_bottom_field = pReadBit->cValue.u32;
    }
    pVUI->timing_info_present_flag = H264ReadBit(pReadBit);
    if (0 != pVUI->timing_info_present_flag)
    {
        pVUI->num_units_in_tick = H264ReadBits(pReadBit, 32);
        pVUI->time_scale = H264ReadBits(pReadBit, 32);
        pVUI->fixed_frame_rate_flag = H264ReadBit(pReadBit);
    }
    pVUI->nal_hrd_parameters_present_flag = H264ReadBit(pReadBit);
    if (0 != pVUI->nal_hrd_parameters_present_flag) { (void)DecodeH264Parameter_hrd_parameters(&(pVUI->nal.hrd_parameters), pReadBit); }
    pVUI->vcl_hrd_parameters_present_flag = H264ReadBit(pReadBit);
    if (0 != pVUI->vcl_hrd_parameters_present_flag) { (void)DecodeH264Parameter_hrd_parameters(&(pVUI->vcl.hrd_parameters), pReadBit); }
    if ((0 != pVUI->nal_hrd_parameters_present_flag) || (0 != pVUI->vcl_hrd_parameters_present_flag)) { pVUI->low_delay_hrd_flag = H264ReadBit(pReadBit); }
    pVUI->pic_struct_present_flag = H264ReadBit(pReadBit);
    pVUI->bitstream_restriction_flag = H264ReadBit(pReadBit);
    if (0 != pVUI->bitstream_restriction_flag)
    {
        pVUI->motion_vectors_over_pic_boundaries_flag = H264ReadBit(pReadBit);
        (void)H264ReadUE(pReadBit); pVUI->max_bytes_per_pic_denom = pReadBit->cValue.u32;
        (void)H264ReadUE(pReadBit); pVUI->max_bits_per_mb_denom = pReadBit->cValue.u32;
        (void)H264ReadUE(pReadBit); pVUI->log2_max_mv_length_horizontal = pReadBit->cValue.u32;
        (void)H264ReadUE(pReadBit); pVUI->log2_max_mv_length_vertical = pReadBit->cValue.u32;
        (void)H264ReadUE(pReadBit); pVUI->max_num_reorder_frames = pReadBit->cValue.u32;
        (void)H264ReadUE(pReadBit); pVUI->max_dec_frame_buffering = pReadBit->cValue.u32;
    }

    return eRet;
}
static VIDEOH264READBITRET DecodeH264Parameter_hrd_parameters(PVIDEOH264PARAMETER_hrd_parameters pHRD, PVIDEOH264READBIT pReadBit)
{
    VIDEOH264READBITRET eRet = VIDEO_H264_READ_BIT_RET_OK;

    (void)H264ReadUE(pReadBit); pHRD->cpb_cnt_minus1 = pReadBit->cValue.u32;
    pHRD->bit_rate_scale = (uint8_t)H264ReadBits(pReadBit, 4);
    pHRD->cpb_size_scale = (uint8_t)H264ReadBits(pReadBit, 4);
    if (pHRD->cpb_cnt_minus1 <= VIDEO_H264_MAX_VALUE_cpb_cnt_minus1)
    {
        for (uint8_t iLoop = 0; iLoop <= pHRD->cpb_cnt_minus1; iLoop++)
        {
            (void)H264ReadUE(pReadBit); pHRD->bit_rate_value_minus1[iLoop] = pReadBit->cValue.u32;
            (void)H264ReadUE(pReadBit); pHRD->cpb_size_value_minus1[iLoop] = pReadBit->cValue.u32;
            pHRD->cbr_flag[iLoop] = H264ReadBit(pReadBit);
        }
        pHRD->initial_cpb_removal_delay_length_minus1 = (uint8_t)H264ReadBits(pReadBit, 5);
        pHRD->cpb_removal_delay_length_minus1 = (uint8_t)H264ReadBits(pReadBit, 5);
        pHRD->dpb_output_delay_length_minus1 = (uint8_t)H264ReadBits(pReadBit, 5);
        pHRD->time_offset_length = (uint8_t)H264ReadBits(pReadBit, 5);
    }

    return eRet;
}

static VIDEOH264READBITRET DecodeH264_user_data_unregistered(PVIDEOH264READBIT pReadBit, HANSIZE payloadSize)
{
    VIDEOH264READBITRET eRet = VIDEO_H264_READ_BIT_RET_OK;

    for (HANSIZE iLoop = 0; iLoop < 16; iLoop++) { (void)H264ReadBits(pReadBit, 8); }
    for (HANSIZE iLoop = 16; iLoop < payloadSize; iLoop++) { (void)H264ReadBits(pReadBit, 8); }

    return eRet;
}

static VIDEOH264READBITRET DecodeH264ParameterRB_slice_header(PVIDEOH264SLICE pSlice, PVIDEOH264READBIT pReadBit, PVIDEOH264PARAMLIST pParamList)
{
    VIDEOH264READBITRET eRet = VIDEO_H264_READ_BIT_RET_OK;
    HANSIZE nSPSCnt = pParamList->spsList.nCnt;
    PCVIDEOH264SPS pSPS = pParamList->spsList.pSPS;
    HANSIZE nPPSCnt = pParamList->ppsList.nCnt;
    PCVIDEOH264PPS pPPS = pParamList->ppsList.pPPS;
    PVIDEOH264PARAMETER_slice_header pHeader = &(pSlice->slice_header);
    VIDEOH264SLICETYPE sliceType;
    uint8_t idrPicFlag;

    DecodeH264_nal_unit(&(pHeader->nalUnit), pReadBit);
    eRet = H264ReadUE(pReadBit); pHeader->first_mb_in_slice = pReadBit->cValue.u32;
    eRet = H264ReadUE(pReadBit); pHeader->slice_type = pReadBit->cValue.u32;
    eRet = H264ReadUE(pReadBit); pHeader->pic_parameter_set_id = pReadBit->cValue.u32;

    idrPicFlag = GetIdrPicFlag(pHeader->nalUnit.nal_unit_type);
    sliceType = pHeader->slice_type % 5;

    for (HANSIZE iLoop = 0; iLoop < nPPSCnt; iLoop++)
    {
        if (pPPS[iLoop].pic_parameter_set_id == pHeader->pic_parameter_set_id)
        {
            pPPS = &pPPS[iLoop];
            for (HANSIZE jLoop = 0; jLoop < nSPSCnt; jLoop++)
            {
                if (pSPS[jLoop].seq_parameter_set_id == pPPS->seq_parameter_set_id)
                {
                    pSPS = &pSPS[jLoop];
                    pSlice->pPPS = pPPS;
                    pSlice->pSPS = pSPS;
                    if (1 == pSPS->separate_colour_plane_flag)
                    {
                        pHeader->colour_plane_id = H264ReadBits(pReadBit, 2);
                    }
                    pHeader->frame_num = H264ReadBits(pReadBit, pSPS->log2_max_frame_num_minus4 + 4);
                    if (0 == pSPS->frame_mbs_only_flag)
                    {
                        pHeader->field_pic_flag = H264ReadBit(pReadBit);
                        if (0 != pHeader->field_pic_flag)
                        {
                            pHeader->bottom_field_flag = H264ReadBit(pReadBit);
                        }
                    }
                    if (0 != idrPicFlag)
                    {
                        eRet = H264ReadUE(pReadBit); pHeader->idr_pic_id = pReadBit->cValue.u32;
                    }
                    if (0 == pSPS->pic_order_cnt_type)
                    {
                        pHeader->pic_order_cnt_lsb = H264ReadBits(pReadBit, pSPS->log2_max_pic_order_cnt_lsb_minus4 + 4);
                        if ((0 != pPPS->bottom_field_pic_order_in_frame_present_flag) && (0 == pHeader->field_pic_flag))
                        {
                            eRet = H264ReadSE(pReadBit); pHeader->delta_pic_order_cnt_bottom = pReadBit->cValue.s32;
                        }
                    }
                    if ((1 == pSPS->pic_order_cnt_type) && (0 == pSPS->delta_pic_order_always_zero_flag))
                    {
                        eRet = H264ReadSE(pReadBit); pHeader->delta_pic_order_cnt[0] = pReadBit->cValue.s32;
                        if ((0 != pPPS->bottom_field_pic_order_in_frame_present_flag) && (0 == pHeader->field_pic_flag))
                        {
                            eRet = H264ReadSE(pReadBit); pHeader->delta_pic_order_cnt[1] = pReadBit->cValue.s32;
                        }
                    }
                    if (0 != pPPS->redundant_pic_cnt_present_flag)
                    {
                        eRet = H264ReadUE(pReadBit); pHeader->redundant_pic_cnt = pReadBit->cValue.u32;
                    }
                    if (VIDEO_H264_SLICE_TYPE_B == sliceType)
                    {
                        pHeader->direct_spatial_mv_pred_flag = H264ReadBit(pReadBit);
                    }
                    if ((VIDEO_H264_SLICE_TYPE_P == sliceType) || (VIDEO_H264_SLICE_TYPE_SP == sliceType) || (VIDEO_H264_SLICE_TYPE_B == sliceType))
                    {
                        pHeader->num_ref_idx_active_override_flag = H264ReadBit(pReadBit);
                        if (0 != pHeader->num_ref_idx_active_override_flag)
                        {
                            eRet = H264ReadUE(pReadBit); pHeader->num_ref_idx_l0_active_minus1 = pReadBit->cValue.u32;
                            if (VIDEO_H264_SLICE_TYPE_B == sliceType)
                            {
                                eRet = H264ReadUE(pReadBit); pHeader->num_ref_idx_l1_active_minus1 = pReadBit->cValue.u32;
                            }
                        }
                    }
                    if ((20 == pHeader->nalUnit.nal_unit_type) || (21 == pHeader->nalUnit.nal_unit_type))
                    {
                        eRet = DecodeH264Parameter_ref_pic_list_mvc_modification(&(pHeader->ref_pic_list_mvc), pHeader->slice_type, pReadBit);
                    }
                    else
                    {
                        eRet = DecodeH264Parameter_ref_pic_list_modification(&(pHeader->ref_pic_list), pHeader->slice_type, pReadBit);
                    }
                    if (((0 != pPPS->weighted_pred_flag) && ((VIDEO_H264_SLICE_TYPE_P == sliceType) || (VIDEO_H264_SLICE_TYPE_SP == sliceType))) ||
                        ((1 == pPPS->weighted_bipred_idc) && (VIDEO_H264_SLICE_TYPE_B == sliceType)))
                    {
                        DecodeH264Parameter_pred_weight_table(&(pHeader->pred_weight_table), pSlice, pReadBit);
                    }
                    if (0 != pHeader->nalUnit.nal_ref_idc)
                    {
                        eRet = DecodeH264Parameter_dec_ref_pic_marking(&(pHeader->dec_ref_pic_marking), idrPicFlag, pReadBit);
                    }
                    if ((0 != pPPS->entropy_coding_mode_flag) && (VIDEO_H264_SLICE_TYPE_I != sliceType) && (VIDEO_H264_SLICE_TYPE_SI != sliceType))
                    {
                        eRet = H264ReadUE(pReadBit); pHeader->cabac_init_idc = pReadBit->cValue.u32;
                    }
                    eRet = H264ReadSE(pReadBit); pHeader->slice_qp_delta = pReadBit->cValue.s32;
                    if ((VIDEO_H264_SLICE_TYPE_SP == sliceType) || (VIDEO_H264_SLICE_TYPE_SI == sliceType))
                    {
                        if (VIDEO_H264_SLICE_TYPE_SP == sliceType)
                        {
                            pHeader->sp_for_switch_flag = H264ReadBit(pReadBit);
                        }
                        eRet = H264ReadSE(pReadBit); pHeader->slice_qs_delta = pReadBit->cValue.s32;
                    }
                    if (0 != pPPS->deblocking_filter_control_present_flag)
                    {
                        eRet = H264ReadUE(pReadBit); pHeader->disable_deblocking_filter_idc = pReadBit->cValue.u32;
                        if (1 != pHeader->disable_deblocking_filter_idc)
                        {
                            eRet = H264ReadSE(pReadBit); pHeader->slice_alpha_c0_offset_div2 = pReadBit->cValue.s32;
                            eRet = H264ReadSE(pReadBit); pHeader->slice_beta_offset_div2 = pReadBit->cValue.s32;
                        }
                    }
                    if ((0 < pPPS->num_slice_groups_minus1) && (3 <= pPPS->slice_group_map_type) && (pPPS->slice_group_map_type <= 5))
                    {
                        pHeader->slice_group_change_cycle = H264ReadBits(pReadBit, H264CeilLog2(pSPS->PicSizeInMapUnits / pPPS->SliceGroupChangeRate)); 
                    }

                    /* slice_data */
                    if (0 != pPPS->entropy_coding_mode_flag)
                    {
                        pSlice->slice_data.cabac_alignment_one_bit.nLen = 8 - pReadBit->iBit;
                    }
                }
            }
        }
    }

    return eRet;
}
static VIDEOH264READBITRET DecodeH264Parameter_ref_pic_list_mvc_modification(PVIDEOH264PARAMETER_ref_pic_list_mvc pRefPicListMVC, uint32_t sliceType, PVIDEOH264READBIT pReadBit)
{
    VIDEOH264READBITRET eRet = VIDEO_H264_READ_BIT_RET_OK;
    uint32_t modSliceType = DecodeH264NormalizeSliceType(sliceType);

    if ((2 != modSliceType) && (4 != modSliceType))
    {
        pRefPicListMVC->ref_pic_list_modification_flag_l0 = H264ReadBit(pReadBit);
        if (0 != pRefPicListMVC->ref_pic_list_modification_flag_l0)
        {
            pRefPicListMVC->bValid_abs_diff_pic_num_minus1 = 0;
            pRefPicListMVC->bValid_long_term_pic_num = 0;
            pRefPicListMVC->bValid_abs_diff_view_idx_minus1 = 0;
            do {
                (void)H264ReadUE(pReadBit); pRefPicListMVC->modification_of_pic_nums_idc = pReadBit->cValue.u32;
                if ((0 == pRefPicListMVC->modification_of_pic_nums_idc) || (1 == pRefPicListMVC->modification_of_pic_nums_idc))
                {
                    pRefPicListMVC->bValid_abs_diff_pic_num_minus1 = 1;
                    (void)H264ReadUE(pReadBit); pRefPicListMVC->abs_diff_pic_num_minus1 = pReadBit->cValue.u32;
                }
                else if (2 == pRefPicListMVC->modification_of_pic_nums_idc)
                {
                    pRefPicListMVC->bValid_long_term_pic_num = 1;
                    (void)H264ReadUE(pReadBit); pRefPicListMVC->long_term_pic_num = pReadBit->cValue.u32;
                }
                else if ((4 == pRefPicListMVC->modification_of_pic_nums_idc) || (5 == pRefPicListMVC->modification_of_pic_nums_idc))
                {
                    pRefPicListMVC->bValid_abs_diff_view_idx_minus1 = 1;
                    (void)H264ReadUE(pReadBit); pRefPicListMVC->abs_diff_view_idx_minus1 = pReadBit->cValue.u32;
                }
                else { }
            } while (3 != pRefPicListMVC->modification_of_pic_nums_idc);
        }
    }
    if (1 == modSliceType)
    {
        pRefPicListMVC->ref_pic_list_modification_flag_l1 = H264ReadBit(pReadBit);
        if (0 != pRefPicListMVC->ref_pic_list_modification_flag_l1)
        {
            pRefPicListMVC->bValid_abs_diff_pic_num_minus1 = 0;
            pRefPicListMVC->bValid_long_term_pic_num = 0;
            pRefPicListMVC->bValid_abs_diff_view_idx_minus1 = 0;
            do {
                (void)H264ReadUE(pReadBit); pRefPicListMVC->modification_of_pic_nums_idc = pReadBit->cValue.u32;
                if ((0 == pRefPicListMVC->modification_of_pic_nums_idc) || (1 == pRefPicListMVC->modification_of_pic_nums_idc))
                {
                    pRefPicListMVC->bValid_abs_diff_pic_num_minus1 = 1;
                    (void)H264ReadUE(pReadBit); pRefPicListMVC->abs_diff_pic_num_minus1 = pReadBit->cValue.u32;
                }
                else if (2 == pRefPicListMVC->modification_of_pic_nums_idc)
                {
                    pRefPicListMVC->bValid_long_term_pic_num = 1;
                    (void)H264ReadUE(pReadBit); pRefPicListMVC->long_term_pic_num = pReadBit->cValue.u32;
                }
                else if ((4 == pRefPicListMVC->modification_of_pic_nums_idc) || (5 == pRefPicListMVC->modification_of_pic_nums_idc))
                {
                    pRefPicListMVC->bValid_abs_diff_view_idx_minus1 = 1;
                    (void)H264ReadUE(pReadBit); pRefPicListMVC->abs_diff_view_idx_minus1 = pReadBit->cValue.u32;
                }
                else { }
            } while (3 != pRefPicListMVC->modification_of_pic_nums_idc);
        }
    }
    
    return eRet;
}
static VIDEOH264READBITRET DecodeH264Parameter_ref_pic_list_modification(PVIDEOH264PARAMETER_ref_pic_list pRefPicList, uint32_t sliceType, PVIDEOH264READBIT pReadBit)
{
    VIDEOH264READBITRET eRet = VIDEO_H264_READ_BIT_RET_OK;
    uint32_t modSliceType = sliceType % 5;

    if ((2 != modSliceType) && (4 != modSliceType))
    {
        pRefPicList->ref_pic_list_modification_flag_l0 = H264ReadBit(pReadBit);
        if (0 != pRefPicList->ref_pic_list_modification_flag_l0)
        {
            pRefPicList->bValid_abs_diff_pic_num_minus1 = 0;
            pRefPicList->bValid_long_term_pic_num = 0;
            do {
                (void)H264ReadUE(pReadBit); pRefPicList->modification_of_pic_nums_idc = pReadBit->cValue.u32;
                if ((0 == pRefPicList->modification_of_pic_nums_idc) || (1 == pRefPicList->modification_of_pic_nums_idc))
                {
                    pRefPicList->bValid_abs_diff_pic_num_minus1 = 1;
                    (void)H264ReadUE(pReadBit); pRefPicList->abs_diff_pic_num_minus1 = pReadBit->cValue.u32;
                }
                else if (2 == pRefPicList->modification_of_pic_nums_idc)
                {
                    pRefPicList->long_term_pic_num = 1;
                    (void)H264ReadUE(pReadBit); pRefPicList->long_term_pic_num = pReadBit->cValue.u32;
                }
                else { }
            } while (3 != pRefPicList->modification_of_pic_nums_idc);
        }
    }
    if (1 == modSliceType)
    {
        pRefPicList->ref_pic_list_modification_flag_l1 = H264ReadBit(pReadBit);
        if (0 != pRefPicList->ref_pic_list_modification_flag_l1)
        {
            pRefPicList->bValid_abs_diff_pic_num_minus1 = 0;
            pRefPicList->bValid_long_term_pic_num = 0;
            do {
                (void)H264ReadUE(pReadBit); pRefPicList->modification_of_pic_nums_idc = pReadBit->cValue.u32;
                if ((0 == pRefPicList->modification_of_pic_nums_idc) || (1 == pRefPicList->modification_of_pic_nums_idc))
                {
                    pRefPicList->bValid_abs_diff_pic_num_minus1 = 1;
                    (void)H264ReadUE(pReadBit); pRefPicList->abs_diff_pic_num_minus1 = pReadBit->cValue.u32;
                }
                else if (2 == pRefPicList->modification_of_pic_nums_idc)
                {
                    pRefPicList->long_term_pic_num = 1;
                    (void)H264ReadUE(pReadBit); pRefPicList->long_term_pic_num = pReadBit->cValue.u32;
                }
                else { }
            } while (3 != pRefPicList->modification_of_pic_nums_idc);
        }
    }
    
    return eRet;
}
static VIDEOH264READBITRET DecodeH264Parameter_pred_weight_table(PVIDEOH264PARAMETER_pred_weight_table pPredWeightTable, PCVIDEOH264SLICE pSlice, PVIDEOH264READBIT pReadBit)
{
    VIDEOH264READBITRET eRet = VIDEO_H264_READ_BIT_RET_OK;
    uint32_t modSliceType = pSlice->slice_header.slice_type % 5;
    uint32_t ChromaArrayType = pSlice->pSPS->ChromaArrayType;
    uint32_t num_ref_idx_l0_active_minus1 = pSlice->slice_header.num_ref_idx_l0_active_minus1;
    uint32_t num_ref_idx_l1_active_minus1 = pSlice->slice_header.num_ref_idx_l1_active_minus1;

    (void)H264ReadUE(pReadBit); pPredWeightTable->luma_log2_weight_denom = pReadBit->cValue.u32;
    if (0 != ChromaArrayType)
    {
        (void)H264ReadUE(pReadBit); pPredWeightTable->chroma_log2_weight_denom = pReadBit->cValue.u32;
    }
    for (uint32_t iLoop = 0; iLoop < num_ref_idx_l0_active_minus1; iLoop++)
    {
        pPredWeightTable->luma_weight_l0_flag[iLoop] = H264ReadBit(pReadBit);
        if (0 != pPredWeightTable->luma_weight_l0_flag[iLoop])
        {
            (void)H264ReadSE(pReadBit); pPredWeightTable->luma_weight_l0[iLoop] = pReadBit->cValue.s32;
            (void)H264ReadSE(pReadBit); pPredWeightTable->luma_offset_l0[iLoop] = pReadBit->cValue.s32;
        }
        if (0 != ChromaArrayType)
        {
            pPredWeightTable->chroma_weight_l0_flag[iLoop] = H264ReadBit(pReadBit);
            if (0 != pPredWeightTable->chroma_weight_l0_flag[iLoop])
            {
                for (uint8_t jLoop = 0; jLoop < 2; jLoop++)
                {
                    (void)H264ReadSE(pReadBit); pPredWeightTable->chroma_weight_l0[iLoop][jLoop] = pReadBit->cValue.s32;
                    (void)H264ReadSE(pReadBit); pPredWeightTable->chroma_offset_l0[iLoop][jLoop] = pReadBit->cValue.s32;
                }
            }
        }
    }
    if (1 == modSliceType)
    {
        for (uint32_t iLoop = 0; iLoop < num_ref_idx_l1_active_minus1; iLoop++)
        {
            pPredWeightTable->luma_weight_l1_flag[iLoop] = H264ReadBit(pReadBit);
            if (0 != pPredWeightTable->luma_weight_l1_flag[iLoop])
            {
                (void)H264ReadSE(pReadBit); pPredWeightTable->luma_weight_l1[iLoop] = pReadBit->cValue.s32;
                (void)H264ReadSE(pReadBit); pPredWeightTable->luma_offset_l1[iLoop] = pReadBit->cValue.s32;
            }
            if (0 != ChromaArrayType)
            {
                pPredWeightTable->chroma_weight_l1_flag[iLoop] = H264ReadBit(pReadBit);
                if (0 != pPredWeightTable->chroma_weight_l1_flag[iLoop])
                {
                    for (uint8_t jLoop = 0; jLoop < 2; jLoop++)
                    {
                        (void)H264ReadSE(pReadBit); pPredWeightTable->chroma_weight_l1[iLoop][jLoop] = pReadBit->cValue.s32;
                        (void)H264ReadSE(pReadBit); pPredWeightTable->chroma_offset_l1[iLoop][jLoop] = pReadBit->cValue.s32;
                    }
                }
            }
        }
    }

    return eRet;
}
static VIDEOH264READBITRET DecodeH264Parameter_dec_ref_pic_marking(PVIDEOH264PARAMETER_dec_ref_pic_marking pDecRefPicMarking, uint8_t idrPicFlag, PVIDEOH264READBIT pReadBit)
{
    VIDEOH264READBITRET eRet = VIDEO_H264_READ_BIT_RET_OK;

    if (0 != idrPicFlag)
    {
        pDecRefPicMarking->no_output_of_prior_pics_flag = H264ReadBit(pReadBit);
        pDecRefPicMarking->long_term_reference_flag = H264ReadBit(pReadBit);
    }
    else
    {
        pDecRefPicMarking->adaptive_ref_pic_marking_mode_flag = H264ReadBit(pReadBit);
        if (0 != pDecRefPicMarking->adaptive_ref_pic_marking_mode_flag)
        {
            pDecRefPicMarking->bValid_difference_of_pic_nums_minus1 = 0;
            pDecRefPicMarking->bValid_long_term_pic_num = 0;
            pDecRefPicMarking->bValid_long_term_frame_idx = 0;
            pDecRefPicMarking->bValid_max_long_term_frame_idx_plus1 = 0;
            do {
                (void)H264ReadUE(pReadBit); pDecRefPicMarking->memory_management_control_operation = pReadBit->cValue.u32;
                if ((1 == pDecRefPicMarking->memory_management_control_operation) || (3 == pDecRefPicMarking->memory_management_control_operation))
                {
                    pDecRefPicMarking->bValid_difference_of_pic_nums_minus1 = 1;
                    (void)H264ReadUE(pReadBit); pDecRefPicMarking->difference_of_pic_nums_minus1 = pReadBit->cValue.u32;
                }
                if (2 == pDecRefPicMarking->memory_management_control_operation)
                {
                    pDecRefPicMarking->bValid_long_term_pic_num = 1;
                    (void)H264ReadUE(pReadBit); pDecRefPicMarking->long_term_pic_num = pReadBit->cValue.u32;
                }
                if ((3 == pDecRefPicMarking->memory_management_control_operation) || (6 == pDecRefPicMarking->memory_management_control_operation))
                {
                    pDecRefPicMarking->bValid_long_term_frame_idx = 1;
                    (void)H264ReadUE(pReadBit); pDecRefPicMarking->long_term_frame_idx = pReadBit->cValue.u32;
                }
                if (4 == pDecRefPicMarking->memory_management_control_operation)
                {
                    pDecRefPicMarking->bValid_max_long_term_frame_idx_plus1 = 1;
                    (void)H264ReadUE(pReadBit); pDecRefPicMarking->max_long_term_frame_idx_plus1 = pReadBit->cValue.u32;
                }
            } while (0 != pDecRefPicMarking->memory_management_control_operation);
        }
    }

    return eRet;
}

static VIDEOH264READBITRET DecodeH264_slice_data(PVIDEOH264PICTUREINFO pPicInfo, PVIDEOH264READBIT pReadBit)
{
    VIDEOH264READBITRET eRet = VIDEO_H264_READ_BIT_RET_OK;
    VIDEOH264DECODER decoder = {
        .decodeInfo = {
            .pReadBit = pReadBit,
            .pPicInfo = pPicInfo,
        },
    };
    
    if (0 == pPicInfo->slice.pPPS->entropy_coding_mode_flag)
    {
        H264InitCAVLCHandler(&decoder);
    }
    else
    {
        VIDEOH264READAE readAE;
        H264InitCABACHandler(&decoder);
        decoder.decodeInfo.pCABAC = &readAE;
        
        /* CABAC 的初始化涉及读取 bit，标准的 7.3.4 章节明确该模式下要先字节对齐 */
        H264AlignByte(pReadBit);
        /* 根据标准的 9.3.1 章节，任何 slice data 解析前都要初始化 CABAC */
        H264CABACInitContext(&(pPicInfo->slice), &(decoder.decodeInfo));
        H264CABACInitEngine(&(decoder.decodeInfo));
    }
    eRet = DecodeH264SliceProcess(&decoder);

    return eRet;
}
static VIDEOH264READBITRET DecodeH264SliceProcess(PVIDEOH264DECODER pDecoder)
{
    VIDEOH264READBITRET eRet = VIDEO_H264_READ_BIT_RET_OK;
    PCVIDEOH264READSECALLBACK pReadSE = &(pDecoder->readSE);
    PVIDEOH264DECODEINFO pDecodeInfo = &(pDecoder->decodeInfo);
    PVIDEOH264PICTUREINFO pPicInfo = pDecodeInfo->pPicInfo;
    PVIDEOH264SLICE pSlice = &(pPicInfo->slice);
    PCVIDEOH264SPS pSPS = pSlice->pSPS;
    PCVIDEOH264PPS pPPS = pSlice->pPPS;
    PCVIDEOH264PARAMETER_slice_header pSliceHeader = &(pSlice->slice_header);
    PVIDEOH264PARAMETER_slice_data pSliceData = &(pSlice->slice_data);
    HANPSIZE pCurrMbAddr = &(pDecodeInfo->CurrMbAddr);
    PVIDEOH264MACROBLOCK pMacroBlock = &(pPicInfo->pMacroBlock[*pCurrMbAddr]);
    VIDEOH264SLICETYPE slice_type;
    uint8_t moreDataFlag;
    uint8_t prevMbSkipped;

    slice_type = pSlice->type;
    pDecodeInfo->NumC8x8 = 4 / (pSPS->SubWidthC * pSPS->SubHeightC); // residual 要用到，这个变量没必要每次解码宏块的时候重新算一遍

    *pCurrMbAddr = pSliceHeader->first_mb_in_slice * (1 + pSlice->MbaffFrameFlag);
    moreDataFlag = 1;
    prevMbSkipped = 0;
    do {
        if ((VIDEO_H264_SLICE_TYPE_I != slice_type) && (VIDEO_H264_SLICE_TYPE_SI != slice_type))
        {
            if (0 == pPPS->entropy_coding_mode_flag)
            {
                pMacroBlock->mb_skip_run = pReadSE->mb_skip_run(pDecodeInfo).u32;
                prevMbSkipped = H264BoolToU8(pMacroBlock->mb_skip_run > 0);
                for (uint32_t iLoop = 0; iLoop < pMacroBlock->mb_skip_run; iLoop++)
                {
                    DecodeH264SkipBlock(pDecoder);
                    *pCurrMbAddr = DecodeH264NextMbAddress(*pCurrMbAddr, pSlice);
                }
                if (pMacroBlock->mb_skip_run > 0) { moreDataFlag = H264BoolToU8(H264CheckMoreRbspData(pDecodeInfo->pReadBit)); }
            }
            else
            {
                pMacroBlock->mb_skip_flag = pReadSE->mb_skip_flag(pDecodeInfo).u32;
                moreDataFlag = H264BoolToU8(!(pMacroBlock->mb_skip_flag));
                if (0 != pMacroBlock->mb_skip_flag) { DecodeH264SkipBlock(pDecoder); }
            }
        }
        if (0 != moreDataFlag)
        {
            if ((0 != pSlice->MbaffFrameFlag) && (((*pCurrMbAddr % 2) == 0) || (((*pCurrMbAddr % 2) == 1) && (0 != prevMbSkipped))))
            {
                pMacroBlock->mb_field_decoding_flag = pReadSE->mb_field_decoding_flag(pDecodeInfo).u32;
            }
            DecodeH264_macroblock(pDecoder);
        }
        if (0 == pPPS->entropy_coding_mode_flag) { moreDataFlag = H264BoolToU8(H264CheckMoreRbspData(pDecodeInfo->pReadBit)); }
        else
        {
            if ((VIDEO_H264_SLICE_TYPE_I != slice_type) && (VIDEO_H264_SLICE_TYPE_SI != slice_type))
            {
                prevMbSkipped = pMacroBlock->mb_skip_flag;
            }
            if ((0 != pSlice->MbaffFrameFlag) && ((*pCurrMbAddr % 2) == 0))
            {
                moreDataFlag = 1;
            }
            else
            {
                pSliceData->end_of_slice_flag = pReadSE->end_of_slice_flag(pDecodeInfo).u32;
                moreDataFlag = H264BoolToU8(!(pSliceData->end_of_slice_flag));
            }
        }
        *pCurrMbAddr = DecodeH264NextMbAddress(*pCurrMbAddr, pSlice);
        pMacroBlock = &(pPicInfo->pMacroBlock[*pCurrMbAddr]);
    } while ((0 != moreDataFlag) && (pDecodeInfo->pReadBit->iByte < pDecodeInfo->pReadBit->nDataLen));

    return eRet;
}
static VIDEOH264READBITRET DecodeH264_macroblock(PVIDEOH264DECODER pDecoder)
{
    VIDEOH264READBITRET eRet = VIDEO_H264_READ_BIT_RET_OK;
    PCVIDEOH264READSECALLBACK pReadSE = &(pDecoder->readSE);
    PVIDEOH264DECODEINFO pDecodeInfo = &(pDecoder->decodeInfo);
    PVIDEOH264PICTUREINFO pPicInfo = pDecodeInfo->pPicInfo;
    PCVIDEOH264SPS pSPS = pPicInfo->slice.pSPS;
    PCVIDEOH264PPS pPPS = pPicInfo->slice.pPPS;
    PVIDEOH264MACROBLOCK pMacroBlock = &(pPicInfo->pMacroBlock[pDecodeInfo->CurrMbAddr]);
    HANSIZE loopLuma = 256;
    HANSIZE loopChroma = (HANSIZE)2 * pSPS->MbWidthC * pSPS->MbHeightC;
    uint8_t noSubMbPartSizeLessThan8x8Flag;
    VIDEOH264MBTYPE mb_type;

    pMacroBlock->mb_type = pReadSE->mb_type(pDecodeInfo).u32;
    DecodeH264InitMacroBlock(pDecodeInfo, pMacroBlock->mb_type);
    mb_type = pMacroBlock->type;

    if (VIDEO_H264_MB_TYPE_I_PCM == mb_type)
    {
        H264AlignByte(pDecodeInfo->pReadBit);
        for (HANSIZE iLoop = 0; iLoop < loopLuma; iLoop++)
        {
            pMacroBlock->pcm_sample_luma[iLoop] = H264ReadBits(pDecodeInfo->pReadBit, pSPS->BitDepthY);
        }
        for (HANSIZE iLoop = 0; iLoop < loopChroma; iLoop++)
        {
            pMacroBlock->pcm_sample_chroma[iLoop] = H264ReadBits(pDecodeInfo->pReadBit, pSPS->BitDepthC);
        }
    }
    else
    {
        noSubMbPartSizeLessThan8x8Flag = 1;
        if (
            (VIDEO_H264_MB_TYPE_I_NxN != mb_type) &&
            (VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16 != pMacroBlock->MbPartPredMode[0]) &&
            (4 == pMacroBlock->NumMbPart)
        )
        {
            eRet = DecodeH264_sub_mb_pred(pDecoder);
            for (uint8_t mbPartIdx = 0; mbPartIdx < 4; mbPartIdx++)
            {
                if (VIDEO_H264_SUB_MB_TYPE_B_Direct_8x8 != pMacroBlock->sub_mb_type[mbPartIdx])
                {
                    if (1 < DecodeH264GetNumSubMbPart(pMacroBlock)) { noSubMbPartSizeLessThan8x8Flag = 0; }
                }
                else if (0 == pSPS->direct_8x8_inference_flag) { noSubMbPartSizeLessThan8x8Flag = 0; }
            }
        }
        else
        {
            if ((0 != pPPS->transform_8x8_mode_flag) && (VIDEO_H264_MB_TYPE_I_NxN == mb_type))
            {
                pMacroBlock->transform_size_8x8_flag = pReadSE->transform_size_8x8_flag(pDecodeInfo).u32;
            }
            eRet = DecodeH264_mb_pred(pDecoder);
        }
        // 获取 CodedBlockPatternLuma 和 CodedBlockPatternChroma
        if (VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16 != pMacroBlock->MbPartPredMode[0])
        {
            pMacroBlock->coded_block_pattern = pReadSE->coded_block_pattern(pDecodeInfo).u32;
            DecodeH264ReadSyntaxElementCallback_coded_block_pattern(pMacroBlock, pMacroBlock->coded_block_pattern);
            if (
                (0 < pMacroBlock->CodedBlockPatternLuma) &&
                (0 != pPPS->transform_8x8_mode_flag) &&
                (VIDEO_H264_MB_TYPE_I_NxN != mb_type) &&
                (0 != noSubMbPartSizeLessThan8x8Flag) &&
                ((VIDEO_H264_MB_TYPE_B_Direct_16x16 != mb_type) || (0 != pSPS->direct_8x8_inference_flag))
            )
            {
                pMacroBlock->transform_size_8x8_flag = pReadSE->transform_size_8x8_flag(pDecodeInfo).u32;
            }
        }
        // 处理 CodedBlockPatternLuma 和 CodedBlockPatternChroma 非零的情况，也就是宏块有内容
        if (
            (0 < pMacroBlock->CodedBlockPatternLuma) ||
            (0 < pMacroBlock->CodedBlockPatternChroma) ||
            (VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16 == pMacroBlock->MbPartPredMode[0])
        )
        {
            pMacroBlock->mb_qp_delta = pReadSE->mb_qp_delta(pDecodeInfo).s32;
            DecodeH264_residual(pDecoder, 0, 15);
        }
    }

    return eRet;
}
static VIDEOH264READBITRET DecodeH264_mb_pred(PVIDEOH264DECODER pDecoder)
{
    VIDEOH264READBITRET eRet = VIDEO_H264_READ_BIT_RET_OK;
    PVIDEOH264DECODEINFO pDecodeInfo = &(pDecoder->decodeInfo);
    PVIDEOH264PICTUREINFO pPicInfo = pDecodeInfo->pPicInfo;
    PVIDEOH264MACROBLOCK pMacroBlock = &(pPicInfo->pMacroBlock[pDecodeInfo->CurrMbAddr]);

    switch (pMacroBlock->MbPartPredMode[0]) {
        case VIDEO_H264_MB_PART_PRED_MODE_Intra_4x4:
        case VIDEO_H264_MB_PART_PRED_MODE_Intra_8x8:
        case VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16: {
            DecodeH264_mb_pred_I_NxN(pDecoder);
        } break;
        case VIDEO_H264_MB_PART_PRED_MODE_Direct: { } break;    // Direct 模式没有逻辑
        default: { DecodeH264_mb_pred_NotDirect(pDecoder); } break;
    }

    return eRet;
}
static VIDEOH264READBITRET DecodeH264_sub_mb_pred(PVIDEOH264DECODER pDecoder)
{
    VIDEOH264READBITRET eRet = VIDEO_H264_READ_BIT_RET_OK;
    PCVIDEOH264READSECALLBACK pReadSE = &(pDecoder->readSE);
    PVIDEOH264DECODEINFO pDecodeInfo = &(pDecoder->decodeInfo);
    PVIDEOH264PICTUREINFO pPicInfo = pDecodeInfo->pPicInfo;
    PCVIDEOH264PARAMETER_slice_header pSliceHeader = &(pPicInfo->slice.slice_header);
    PVIDEOH264MACROBLOCK pMacroBlock = &(pPicInfo->pMacroBlock[pDecodeInfo->CurrMbAddr]);
    uint8_t* mbPartIdx = &(pDecodeInfo->mbPartIdx);
    uint8_t* subMbPartIdx = &(pDecodeInfo->subMbPartIdx);

    for (*mbPartIdx = 0; *mbPartIdx < 4; (*mbPartIdx)++)
    {
        pMacroBlock->sub_mb_type[*mbPartIdx] = pReadSE->sub_mb_type(pDecodeInfo).u32;
    }
    DecodeH264InitSubMacroBlock(pDecodeInfo);
    for (*mbPartIdx = 0; *mbPartIdx < 4; (*mbPartIdx)++)
    {
        if ((
                (0 < pSliceHeader->num_ref_idx_l0_active_minus1) ||
                (pMacroBlock->mb_field_decoding_flag != pSliceHeader->field_pic_flag)
            ) &&
            (VIDEO_H264_MB_TYPE_P_8x8ref0 != pMacroBlock->mb_type) &&
            (VIDEO_H264_SUB_MB_TYPE_B_Direct_8x8 != pMacroBlock->sub_mb_type[*mbPartIdx]) && 
            (VIDEO_H264_MB_PART_PRED_MODE_Pred_L1 != pMacroBlock->SubMbPredMode[*mbPartIdx])
        )
        { pMacroBlock->ref_idx_l0[*mbPartIdx] = pReadSE->ref_idx_l0(pDecodeInfo).u32; }
    }
    for (*mbPartIdx = 0; *mbPartIdx < 4; (*mbPartIdx)++)
    {
        if ((
                (0 < pSliceHeader->num_ref_idx_l1_active_minus1) ||
                (pMacroBlock->mb_field_decoding_flag != pSliceHeader->field_pic_flag)
            ) &&
            (VIDEO_H264_SUB_MB_TYPE_B_Direct_8x8 != pMacroBlock->sub_mb_type[*mbPartIdx]) && 
            (VIDEO_H264_MB_PART_PRED_MODE_Pred_L0 != pMacroBlock->SubMbPredMode[*mbPartIdx])
        )
        { pMacroBlock->ref_idx_l1[*mbPartIdx] = pReadSE->ref_idx_l1(pDecodeInfo).u32; }
    }
    for (*mbPartIdx = 0; *mbPartIdx < 4; (*mbPartIdx)++)
    {
        if ((VIDEO_H264_SUB_MB_TYPE_B_Direct_8x8 != pMacroBlock->sub_mb_type[*mbPartIdx]) && 
            (VIDEO_H264_MB_PART_PRED_MODE_Pred_L1 != pMacroBlock->SubMbPredMode[*mbPartIdx])
        )
        {
            for (*subMbPartIdx = 0; *subMbPartIdx < pMacroBlock->NumSubMbPart[*mbPartIdx]; (*subMbPartIdx)++)
            {
                for (uint8_t compIdx = 0; compIdx < 2; compIdx++)
                {
                    pMacroBlock->mvd_l0[*mbPartIdx][*subMbPartIdx][compIdx] = pReadSE->mvd_l0[compIdx](pDecodeInfo).s32;
                }
            }
        }
    }
    for (*mbPartIdx = 0; *mbPartIdx < 4; (*mbPartIdx)++)
    {
        if ((VIDEO_H264_SUB_MB_TYPE_B_Direct_8x8 != pMacroBlock->sub_mb_type[*mbPartIdx]) && 
            (VIDEO_H264_MB_PART_PRED_MODE_Pred_L0 != pMacroBlock->SubMbPredMode[*mbPartIdx])
        )
        {
            for (*subMbPartIdx = 0; *subMbPartIdx < pMacroBlock->NumSubMbPart[*mbPartIdx]; (*subMbPartIdx)++)
            {
                for (uint8_t compIdx = 0; compIdx < 2; compIdx++)
                {
                    pMacroBlock->mvd_l1[*mbPartIdx][*subMbPartIdx][compIdx] = pReadSE->mvd_l1[compIdx](pDecodeInfo).s32;
                }
            }
        }
    }

    return eRet;
}
static VIDEOH264READBITRET DecodeH264_residual(PVIDEOH264DECODER pDecoder, int32_t startIdx, int32_t endIdx)
{
    VIDEOH264READBITRET eRet = VIDEO_H264_READ_BIT_RET_OK;
    PVIDEOH264DECODEINFO pDecodeInfo = &(pDecoder->decodeInfo);
    PVIDEOH264PICTUREINFO pPicInfo = pDecodeInfo->pPicInfo;
    PCVIDEOH264SPS pSPS = pPicInfo->slice.pSPS;
    PVIDEOH264MACROBLOCK pMacroBlock = &(pPicInfo->pMacroBlock[pDecodeInfo->CurrMbAddr]);

    uint8_t NumC8x8;

    /* 标准的第一步是根据编码模式选择 residual_block，这里就不再在每次处理宏块的时候选择了，直接在解码 slice_data 的初始化 Handler 里设置了 */
    DecodeH264_residual_luma(pDecoder, &(pDecodeInfo->Luma), startIdx, endIdx, VIDEO_H264_COMPONENT_Y);
    if ((1 == pSPS->ChromaArrayType) || (2 == pSPS->ChromaArrayType))
    {
        NumC8x8 = pDecodeInfo->NumC8x8; // NumC8x8 只可能等于 1 或 2
        for (uint8_t iCbCr = 0; iCbCr < 2; iCbCr++)
        {
            if ((0 != (pMacroBlock->CodedBlockPatternChroma & 3)) && (0 == startIdx))
            {
                DecodeH264_residual_block(pDecoder, pDecodeInfo->ChromaDCLevel[iCbCr], 0, (4 * NumC8x8) - 1, 4 * NumC8x8,
                    iCbCr, 0, 0, VIDEO_H264_BLOCK_CAT_ChromaDC);
            }
            else
            {
                for (uint8_t iLoop = 0; iLoop < (4 * NumC8x8); iLoop++)
                {
                    pDecodeInfo->ChromaDCLevel[iCbCr][iLoop] = 0;
                }
            }
        }
        for (uint8_t iCbCr = 0; iCbCr < 2; iCbCr++)
        {
            for (uint8_t i8x8 = 0; i8x8 < NumC8x8; i8x8++)
            {
                for (uint8_t i4x4 = 0; i4x4 < 4; i4x4++)
                {
                    if (0 != (pMacroBlock->CodedBlockPatternChroma & 2))
                    {
                        DecodeH264_residual_block(pDecoder, pDecodeInfo->ChromaACLevel[iCbCr][i8x8 * 4 + i4x4], H264IMax(0, startIdx - 1), endIdx - 1, 15,
                            iCbCr, i8x8, i4x4, VIDEO_H264_BLOCK_CAT_ChromaAC);
                    }
                    else
                    {
                        for (uint8_t iLoop = 0; iLoop < 15; iLoop++)
                        {
                            pDecodeInfo->ChromaACLevel[iCbCr][i8x8 * 4 + i4x4][iLoop] = 0;
                        }
                    }
                }
            }
        }
    }
    else if (3 == pSPS->ChromaArrayType)
    {
        DecodeH264_residual_luma(pDecoder, &(pDecodeInfo->Cb), startIdx, endIdx, VIDEO_H264_COMPONENT_Cb);
        DecodeH264_residual_luma(pDecoder, &(pDecodeInfo->Cr), startIdx, endIdx, VIDEO_H264_COMPONENT_Cr);
    }
    
    return eRet;
}
static VIDEOH264READBITRET DecodeH264_residual_luma(PVIDEOH264DECODER pDecoder, PVIDEOH264COEFFLEVELBLOCK levelBlock, int32_t startIdx, int32_t endIdx, VIDEOH264COMPONENT component)
{
    VIDEOH264READBITRET eRet = VIDEO_H264_READ_BIT_RET_OK;
    PVIDEOH264DECODEINFO pDecodeInfo = &(pDecoder->decodeInfo);
    PVIDEOH264PICTUREINFO pPicInfo = pDecodeInfo->pPicInfo;
    PCVIDEOH264PPS pPPS = pPicInfo->slice.pPPS;
    PVIDEOH264MACROBLOCK pMacroBlock = &(pPicInfo->pMacroBlock[pDecodeInfo->CurrMbAddr]);

    if ((0 == startIdx) && (VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16 == pMacroBlock->MbPartPredMode[0]))
    {
        DecodeH264_residual_block(pDecoder, levelBlock->i16x16DClevel, 0, 15, 16,
            0, 0, 0, sg_pBlockCatMap[component][VIDEO_H264_BLOCK_DC]);
    }
    for (uint8_t i8x8 = 0; i8x8 < 4; i8x8++)
    {
        if ((0 == pMacroBlock->transform_size_8x8_flag) || (0 == pPPS->entropy_coding_mode_flag))
        {
            for (uint8_t i4x4 = 0; i4x4 < 4; i4x4++)
            {
                if (0 != (pMacroBlock->CodedBlockPatternLuma & (1 << i8x8)))
                {
                    if (VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16 == pMacroBlock->MbPartPredMode[0])
                    {
                        DecodeH264_residual_block(pDecoder, levelBlock->i16x16AClevel[i8x8 * 4 + i4x4], H264IMax(0, startIdx - 1), endIdx - 1, 15,
                            0, i8x8, i4x4, sg_pBlockCatMap[component][VIDEO_H264_BLOCK_AC]);
                    }
                    else
                    {
                        DecodeH264_residual_block(pDecoder, levelBlock->level4x4[i8x8 * 4 + i4x4], startIdx, endIdx, 15,
                            0, i8x8, i4x4, sg_pBlockCatMap[component][VIDEO_H264_BLOCK_4x4]);
                    }
                }
                else if (VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16 == pMacroBlock->MbPartPredMode[0])
                {
                    for (uint8_t iLoop = 0; iLoop < 15; iLoop++)
                    {
                        levelBlock->i16x16AClevel[i8x8 * 4 + i4x4][iLoop] = 0;
                    }
                }
                else
                {
                    for (uint8_t iLoop = 0; iLoop < 16; iLoop++)
                    {
                        levelBlock->level4x4[i8x8 * 4 + i4x4][iLoop] = 0;
                    }
                }
                if ((0 == pPPS->entropy_coding_mode_flag) && (0 != pMacroBlock->transform_size_8x8_flag))
                {
                    for (uint8_t iLoop = 0; iLoop < 16; iLoop++)
                    {
                        levelBlock->level8x8[i8x8][4 * iLoop + i4x4] = levelBlock->level4x4[i8x8 * 4 + i4x4][iLoop];
                    }
                }
            }
        }
        else if (0 != (pMacroBlock->CodedBlockPatternLuma & (1 << i8x8)))
        {
            DecodeH264_residual_block(pDecoder, levelBlock->level8x8[i8x8], 4 * startIdx, 4 * endIdx + 3, 64,
                0, i8x8, 0, sg_pBlockCatMap[component][VIDEO_H264_BLOCK_8x8]);
        }
        else
        {
            for (uint8_t iLoop = 0; iLoop < 64; iLoop++)
            {
                levelBlock->level8x8[i8x8][iLoop] = 0;
            }
        }
    }

    return eRet;
}
static inline void DecodeH264_residual_block(PVIDEOH264DECODER pDecoder, PVIDEOH264COEFFLEVEL coeffLevel, int32_t startIdx, int32_t endIdx, uint8_t maxNumCoeff,
    uint8_t iCbCr, uint8_t i8x8, uint8_t i4x4, VIDEOH264BLOCKCAT blockCat)
{
    PVIDEOH264DECODEINFO pDecodeInfo = &(pDecoder->decodeInfo);

    pDecodeInfo->coeffLevel = coeffLevel;
    pDecodeInfo->startIdx = startIdx;
    pDecodeInfo->endIdx = endIdx;
    pDecodeInfo->maxNumCoeff = maxNumCoeff;
    pDecodeInfo->iCbCr = iCbCr;
    pDecodeInfo->i8x8 = i8x8;
    pDecodeInfo->i4x4 = i4x4;
    pDecodeInfo->blockCat = blockCat;

    if (VIDEO_H264_READ_BIT_RET_OK == pDecodeInfo->pReadBit->retError) { pDecoder->residual_block(pDecodeInfo); }
}

static void DecodeH264SkipBlock(PVIDEOH264DECODER pDecoder)
{
    VIDEOH264SLICETYPE sliceType = pDecoder->decodeInfo.pPicInfo->slice.type;
    PVIDEOH264MACROBLOCK pMacroBlock = pDecoder->decodeInfo.pPicInfo->pMacroBlock;
    HANSIZE CurrMbAddr = pDecoder->decodeInfo.CurrMbAddr;

    if (VIDEO_H264_SLICE_TYPE_B == sliceType)
    {
        pMacroBlock[CurrMbAddr].type = VIDEO_H264_MB_TYPE_B_Skip;
    }
    else if ((VIDEO_H264_SLICE_TYPE_P == sliceType) || (VIDEO_H264_SLICE_TYPE_SP == sliceType))
    {
        pMacroBlock[CurrMbAddr].type = VIDEO_H264_MB_TYPE_P_Skip;
    }
    else { pMacroBlock[CurrMbAddr].type = VIDEO_H264_MB_TYPE_CNT; } // 无效的情况，只有 B、P、SP 帧可以跳过宏块
}
static HANSIZE DecodeH264NextMbAddress(HANSIZE n, PCVIDEOH264SLICE pSlice)
{
    HANSIZE iRet = n + 1;
    uint32_t PicSizeInMbs = pSlice->PicSizeInMbs;
    PVIDEOH264SLICEGROUPMAP MbToSliceGroupMap = pSlice->pSPS->MbToSliceGroupMap;

    while ((iRet < PicSizeInMbs) && (MbToSliceGroupMap[iRet] != MbToSliceGroupMap[n])) { iRet++; }

    return iRet;
}

static VIDEOH264READBITRET DecodeH264_mb_pred_I_NxN(PVIDEOH264DECODER pDecoder)
{
    VIDEOH264READBITRET eRet = VIDEO_H264_READ_BIT_RET_OK;
    PCVIDEOH264READSECALLBACK pReadSE = &(pDecoder->readSE);
    PVIDEOH264DECODEINFO pDecodeInfo = &(pDecoder->decodeInfo);
    PVIDEOH264PICTUREINFO pPicInfo = pDecodeInfo->pPicInfo;
    PVIDEOH264MACROBLOCK pMacroBlock = &(pPicInfo->pMacroBlock[pDecodeInfo->CurrMbAddr]);

    switch (pMacroBlock->MbPartPredMode[0]) {
        case VIDEO_H264_MB_PART_PRED_MODE_Intra_4x4: {
            for (uint8_t luma4x4BlkIdx = 0; luma4x4BlkIdx < 16; luma4x4BlkIdx++)
            {
                pMacroBlock->prev_intra4x4_pred_mode_flag[luma4x4BlkIdx] = pReadSE->prev_intraNxN_pred_mode_flag(pDecodeInfo).u32;
                if (0 == pMacroBlock->prev_intra4x4_pred_mode_flag[luma4x4BlkIdx])
                {
                    pMacroBlock->rem_intra4x4_pred_mode[luma4x4BlkIdx] = pReadSE->rem_intraNxN_pred_mode(pDecodeInfo).u32;
                }
            }
        } break;
        case VIDEO_H264_MB_PART_PRED_MODE_Intra_8x8: {
            for (uint8_t luma8x8BlkIdx = 0; luma8x8BlkIdx < 4; luma8x8BlkIdx++)
            {
                pMacroBlock->prev_intra8x8_pred_mode_flag[luma8x8BlkIdx] = pReadSE->prev_intraNxN_pred_mode_flag(pDecodeInfo).u32;
                if (0 == pMacroBlock->prev_intra8x8_pred_mode_flag[luma8x8BlkIdx])
                {
                    pMacroBlock->rem_intra8x8_pred_mode[luma8x8BlkIdx] = pReadSE->rem_intraNxN_pred_mode(pDecodeInfo).u32;
                }
            }
        } break;
        default: { } break;
    }
    if ((1 == pPicInfo->slice.pSPS->ChromaArrayType) || (2 == pPicInfo->slice.pSPS->ChromaArrayType))
    {
        pMacroBlock->intra_chroma_pred_mode = pReadSE->intra_chroma_pred_mode(pDecodeInfo).u32;
    }

    return eRet;
}
static VIDEOH264READBITRET DecodeH264_mb_pred_NotDirect(PVIDEOH264DECODER pDecoder)
{
    VIDEOH264READBITRET eRet = VIDEO_H264_READ_BIT_RET_OK;
    PCVIDEOH264READSECALLBACK pReadSE = &(pDecoder->readSE);
    PVIDEOH264DECODEINFO pDecodeInfo = &(pDecoder->decodeInfo);
    PVIDEOH264PICTUREINFO pPicInfo = pDecodeInfo->pPicInfo;
    PCVIDEOH264PARAMETER_slice_header pSliceHeader = &(pPicInfo->slice.slice_header);
    PVIDEOH264MACROBLOCK pMacroBlock = &(pPicInfo->pMacroBlock[pDecodeInfo->CurrMbAddr]);
    uint8_t* mbPartIdx = &(pDecodeInfo->mbPartIdx);

    for (*mbPartIdx = 0; *mbPartIdx < pMacroBlock->NumMbPart; (*mbPartIdx)++)
    {
        if ((
                (pSliceHeader->num_ref_idx_l0_active_minus1 > 0) ||
                (pMacroBlock->mb_field_decoding_flag != pSliceHeader->field_pic_flag)
            ) &&
            (VIDEO_H264_MB_PART_PRED_MODE_Pred_L1 != pMacroBlock->MbPartPredMode[*mbPartIdx]))
        {
            pMacroBlock->ref_idx_l0[*mbPartIdx] = pReadSE->ref_idx_l0(pDecodeInfo).u32;
        }
    }
    for (*mbPartIdx = 0; *mbPartIdx < pMacroBlock->NumMbPart; (*mbPartIdx)++)
    {
        if ((
                (pSliceHeader->num_ref_idx_l1_active_minus1 > 0) ||
                (pMacroBlock->mb_field_decoding_flag != pSliceHeader->field_pic_flag)
            ) &&
            (VIDEO_H264_MB_PART_PRED_MODE_Pred_L0 != pMacroBlock->MbPartPredMode[*mbPartIdx]))
        {
            pMacroBlock->ref_idx_l1[*mbPartIdx] = pReadSE->ref_idx_l1(pDecodeInfo).u32;
        }
    }
    for (*mbPartIdx = 0; *mbPartIdx < pMacroBlock->NumMbPart; (*mbPartIdx)++)
    {
        if (VIDEO_H264_MB_PART_PRED_MODE_Pred_L1 != pMacroBlock->MbPartPredMode[*mbPartIdx])
        {
            for (uint8_t compIdx = 0; compIdx < 2; compIdx++)
            {
                pMacroBlock->mvd_l0[*mbPartIdx][0][compIdx] = pReadSE->mvd_l0[compIdx](pDecodeInfo).s32;
            }
        }
    }
    for (*mbPartIdx = 0; *mbPartIdx < pMacroBlock->NumMbPart; (*mbPartIdx)++)
    {
        if (VIDEO_H264_MB_PART_PRED_MODE_Pred_L0 != pMacroBlock->MbPartPredMode[*mbPartIdx])
        {
            for (uint8_t compIdx = 0; compIdx < 2; compIdx++)
            {
                pMacroBlock->mvd_l1[*mbPartIdx][0][compIdx] = pReadSE->mvd_l1[compIdx](pDecodeInfo).s32;
            }
        }
    }

    return eRet;
}

static uint8_t DecodeH264GetNumSubMbPart(PCVIDEOH264MACROBLOCK pMacroBlock)
{
    (void)pMacroBlock;
    return 0;
}

static bool DecodeH264GenerateMbToSliceGroupMap(PVIDEOH264SLICE pSlice)
{
    bool bRet = true;
    uint32_t PicSizeInMapUnits = pSlice->pSPS->PicSizeInMapUnits;
    uint32_t PicSizeInMbs = pSlice->PicSizeInMbs;
    uint32_t PicWidthInMbs = pSlice->pSPS->PicWidthInMbs;
    PVIDEOH264SLICEGROUPMAP mapUnitToSliceGroupMap = pSlice->pSPS->mapUnitToSliceGroupMap;
    PVIDEOH264SLICEGROUPMAP MbToSliceGroupMap = pSlice->pSPS->MbToSliceGroupMap;

    if (0 == pSlice->pPPS->num_slice_groups_minus1)
    {
        for (uint32_t iLoop = 0; iLoop < PicSizeInMapUnits; iLoop++)
        {
            mapUnitToSliceGroupMap[iLoop] = 0;
        }
    }
    else
    {
        switch (pSlice->pPPS->slice_group_map_type) {
            case 0: { DecodeH264GenerateMbToSliceGroupMapInterleaved(pSlice); } break;
            case 1: { DecodeH264GenerateMbToSliceGroupMapDispersed(pSlice); } break;
            case 2: { DecodeH264GenerateMbToSliceGroupMapForegroundWithLeftOver(pSlice); } break;
            case 3: { DecodeH264GenerateMbToSliceGroupMapBoxOut(pSlice); } break;
            case 4: { DecodeH264GenerateMbToSliceGroupMapRasterScan(pSlice); } break;
            case 5: { DecodeH264GenerateMbToSliceGroupMapWipe(pSlice); } break;
            case 6: { DecodeH264GenerateMbToSliceGroupMapExplicit(pSlice); } break;
            default: { bRet = false; } break;
        }
    }

    if ((0 != pSlice->pSPS->frame_mbs_only_flag) || (0 != pSlice->slice_header.field_pic_flag))
    {
        for (uint32_t iLoop = 0; iLoop < PicSizeInMbs; iLoop++)
        {
            MbToSliceGroupMap[iLoop] = mapUnitToSliceGroupMap[iLoop];
        }
    }
    else if (0 != pSlice->MbaffFrameFlag)
    {
        for (uint32_t iLoop = 0; iLoop < PicSizeInMbs; iLoop++)
        {
            MbToSliceGroupMap[iLoop] = mapUnitToSliceGroupMap[iLoop / 2];
        }
    }
    else
    {
        for (uint32_t iLoop = 0; iLoop < PicSizeInMbs; iLoop++)
        {
            MbToSliceGroupMap[iLoop] = mapUnitToSliceGroupMap[((iLoop / (2 * PicWidthInMbs)) * PicWidthInMbs) + (iLoop % PicWidthInMbs)];
        }
    }

    return bRet;
}
static void DecodeH264GenerateMbToSliceGroupMapInterleaved(PVIDEOH264SLICE pSlice)
{
    uint32_t PicSizeInMapUnits = pSlice->pSPS->PicSizeInMapUnits;
    PVIDEOH264SLICEGROUPMAP mapUnitToSliceGroupMap = pSlice->pSPS->mapUnitToSliceGroupMap;
    uint32_t num_slice_groups_minus1 = pSlice->pPPS->num_slice_groups_minus1;
    const uint32_t* run_length_minus1 = pSlice->pPPS->run_length_minus1;
    uint32_t iLoop = 0;

    do {
        for (uint32_t iGroup = 0; (iGroup <= num_slice_groups_minus1) && (iLoop < PicSizeInMapUnits); iLoop += run_length_minus1[iGroup++] + 1)
        {
            for (uint32_t jLoop = 0; jLoop <= run_length_minus1[iGroup] && ((iLoop + jLoop) < PicSizeInMapUnits); jLoop++)
            {
                mapUnitToSliceGroupMap[iLoop + jLoop] = iGroup;
            }
        }
    } while (iLoop < PicSizeInMapUnits);
}
static void DecodeH264GenerateMbToSliceGroupMapDispersed(PVIDEOH264SLICE pSlice)
{
    uint32_t PicSizeInMapUnits = pSlice->pSPS->PicSizeInMapUnits;
    PVIDEOH264SLICEGROUPMAP mapUnitToSliceGroupMap = pSlice->pSPS->mapUnitToSliceGroupMap;
    uint32_t PicWidthInMbs = pSlice->pSPS->PicWidthInMbs;
    uint32_t num_slice_groups_minus1 = pSlice->pPPS->num_slice_groups_minus1;

    for (uint32_t iLoop = 0; iLoop < PicSizeInMapUnits; iLoop++)
    {
        mapUnitToSliceGroupMap[iLoop] = ((iLoop % PicWidthInMbs) + (((iLoop / PicWidthInMbs) * (num_slice_groups_minus1 + 1)) / 2)) % (num_slice_groups_minus1 + 1);
    }
}
static void DecodeH264GenerateMbToSliceGroupMapForegroundWithLeftOver(PVIDEOH264SLICE pSlice)
{
    uint32_t PicSizeInMapUnits = pSlice->pSPS->PicSizeInMapUnits;
    PVIDEOH264SLICEGROUPMAP mapUnitToSliceGroupMap = pSlice->pSPS->mapUnitToSliceGroupMap;
    uint32_t PicWidthInMbs = pSlice->pSPS->PicWidthInMbs;
    uint32_t num_slice_groups_minus1 = pSlice->pPPS->num_slice_groups_minus1;
    uint32_t yTopLeft;
    uint32_t xTopLeft;
    uint32_t yBottomRight;
    uint32_t xBottomRight;
    const uint32_t* top_left = pSlice->pPPS->top_left;
    const uint32_t* bottom_right = pSlice->pPPS->bottom_right;

    for (uint32_t iLoop = 0; iLoop < PicSizeInMapUnits; iLoop++)
    {
        mapUnitToSliceGroupMap[iLoop] = num_slice_groups_minus1;
    }
    for (int32_t iGroup = (int32_t)(num_slice_groups_minus1 - 1); iGroup >= 0; iGroup--)
    {
        yTopLeft = top_left[iGroup] / PicWidthInMbs;
        xTopLeft = top_left[iGroup] % PicWidthInMbs;
        yBottomRight = bottom_right[iGroup] / PicWidthInMbs;
        xBottomRight = bottom_right[iGroup] % PicWidthInMbs;
        for (uint32_t yLoop = yTopLeft; yLoop <= yBottomRight; yLoop++)
        {
            for (uint32_t xLoop = xTopLeft; xLoop <= xBottomRight; xLoop++)
            {
                mapUnitToSliceGroupMap[yLoop * PicWidthInMbs + xLoop] = iGroup;
            }
        }
    }
}
static void DecodeH264GenerateMbToSliceGroupMapBoxOut(PVIDEOH264SLICE pSlice)
{
    uint32_t PicSizeInMapUnits = pSlice->pSPS->PicSizeInMapUnits;
    PVIDEOH264SLICEGROUPMAP mapUnitToSliceGroupMap = pSlice->pSPS->mapUnitToSliceGroupMap;
    uint32_t PicWidthInMbs = pSlice->pSPS->PicWidthInMbs;
    uint32_t PicHeightInMapUnits = pSlice->pSPS->PicHeightInMapUnits;
    uint8_t slice_group_change_direction_flag = pSlice->pPPS->slice_group_change_direction_flag;
    uint32_t MapUnitsInSliceGroup0 = pSlice->MapUnitsInSliceGroup0;
    uint32_t x;
    uint32_t y;
    uint32_t leftBound;
    uint32_t topBound;
    uint32_t rightBound;
    uint32_t bottomBound;
    int32_t xDir;
    int32_t yDir;
    uint8_t mapUnitVacant;

    for (uint32_t iLoop = 0; iLoop < PicSizeInMapUnits; iLoop++)
    {
        mapUnitToSliceGroupMap[iLoop] = 1;
    }
    x = (PicWidthInMbs - slice_group_change_direction_flag) / 2;
    y = (PicHeightInMapUnits - slice_group_change_direction_flag) / 2;
    leftBound = x; topBound = y;
    rightBound = x; bottomBound = y;
    xDir = slice_group_change_direction_flag - 1;
    yDir = slice_group_change_direction_flag;
    for (uint32_t kLoop = 0; kLoop < MapUnitsInSliceGroup0; kLoop += mapUnitVacant)
    {
        mapUnitVacant = H264BoolToU8(mapUnitToSliceGroupMap[y * PicWidthInMbs + x] == 1);
        if (0 != mapUnitVacant) { mapUnitToSliceGroupMap[y * PicWidthInMbs + x] = 0; }

        if ((-1 == xDir) && (x == leftBound))
        {
            leftBound = H264IMax(((int32_t)leftBound) - 1, 0);
            x = leftBound;
            xDir = 0;
            yDir = (2 * slice_group_change_direction_flag) - 1;
        }
        else if ((1 == xDir) && (x == rightBound))
        {
            rightBound = H264Min(rightBound + 1, PicWidthInMbs - 1);
            x = rightBound;
            xDir = 0;
            yDir = 1 - (2 * slice_group_change_direction_flag);
        }
        else if ((-1 == yDir) && (y == topBound))
        {
            topBound = H264IMax(((int32_t)topBound) - 1, 0);
            y = topBound;
            xDir = 1 - (2 * slice_group_change_direction_flag);
            yDir = 0;
        }
        else if ((1 == yDir) && (y == bottomBound))
        {
            bottomBound = H264Min(bottomBound + 1, PicHeightInMapUnits - 1);
            y = bottomBound;
            xDir = (2 * slice_group_change_direction_flag) - 1;
            yDir = 0;
        }
        else
        {
            x = x + xDir;
            y = y + yDir;
        }
    }
}
static void DecodeH264GenerateMbToSliceGroupMapRasterScan(PVIDEOH264SLICE pSlice)
{
    uint32_t PicSizeInMapUnits = pSlice->pSPS->PicSizeInMapUnits;
    PVIDEOH264SLICEGROUPMAP mapUnitToSliceGroupMap = pSlice->pSPS->mapUnitToSliceGroupMap;
    uint8_t slice_group_change_direction_flag = pSlice->pPPS->slice_group_change_direction_flag;
    uint32_t MapUnitsInSliceGroup0 = pSlice->MapUnitsInSliceGroup0;
    uint32_t sizeOfUpperLeftGroup;

    if (0 != slice_group_change_direction_flag) { sizeOfUpperLeftGroup = PicSizeInMapUnits - MapUnitsInSliceGroup0; }
    else { sizeOfUpperLeftGroup = MapUnitsInSliceGroup0; }

    for (uint32_t iLoop = 0; iLoop < PicSizeInMapUnits; iLoop++)
    {
        if (iLoop < sizeOfUpperLeftGroup) { mapUnitToSliceGroupMap[iLoop] = slice_group_change_direction_flag; }
        else { mapUnitToSliceGroupMap[iLoop] = 1 - slice_group_change_direction_flag; }
    }
}
static void DecodeH264GenerateMbToSliceGroupMapWipe(PVIDEOH264SLICE pSlice)
{
    uint32_t PicSizeInMapUnits = pSlice->pSPS->PicSizeInMapUnits;
    PVIDEOH264SLICEGROUPMAP mapUnitToSliceGroupMap = pSlice->pSPS->mapUnitToSliceGroupMap;
    uint32_t PicWidthInMbs = pSlice->pSPS->PicWidthInMbs;
    uint32_t PicHeightInMapUnits = pSlice->pSPS->PicHeightInMapUnits;
    uint8_t slice_group_change_direction_flag = pSlice->pPPS->slice_group_change_direction_flag;
    uint32_t MapUnitsInSliceGroup0 = pSlice->MapUnitsInSliceGroup0;
    uint32_t sizeOfUpperLeftGroup;
    uint32_t k;

    if (0 != slice_group_change_direction_flag) { sizeOfUpperLeftGroup = PicSizeInMapUnits - MapUnitsInSliceGroup0; }
    else { sizeOfUpperLeftGroup = MapUnitsInSliceGroup0; }

    k = 0;
    for (uint32_t jLoop = 0; jLoop < PicWidthInMbs; jLoop++)
    {
        for (uint32_t iLoop = 0; iLoop < PicHeightInMapUnits; iLoop++)
        {
            if (k++ < sizeOfUpperLeftGroup) { mapUnitToSliceGroupMap[iLoop * PicWidthInMbs + jLoop] = slice_group_change_direction_flag; }
            else { mapUnitToSliceGroupMap[iLoop * PicWidthInMbs + jLoop] = 1 - slice_group_change_direction_flag; }
        }
    }
}
static void DecodeH264GenerateMbToSliceGroupMapExplicit(PVIDEOH264SLICE pSlice)
{
    uint32_t PicSizeInMapUnits = pSlice->pSPS->PicSizeInMapUnits;
    PVIDEOH264SLICEGROUPMAP mapUnitToSliceGroupMap = pSlice->pSPS->mapUnitToSliceGroupMap;
    uint32_t* slice_group_id = pSlice->pPPS->slice_group_id;
    
    if (NULL != slice_group_id)
    {
        for (uint32_t iLoop = 0; iLoop < PicSizeInMapUnits; iLoop++)
        {
            mapUnitToSliceGroupMap[iLoop] = slice_group_id[iLoop];
        }
    }
    else
    {
        for (uint32_t iLoop = 0; iLoop < PicSizeInMapUnits; iLoop++)
        {
            mapUnitToSliceGroupMap[iLoop] = 0;
        }
    }
}

static inline void DecodeH264ReadSyntaxElementCallback_coded_block_pattern(PVIDEOH264MACROBLOCK pMacroBlock, uint32_t coded_block_pattern)
{
    /* Intra_16x16 的宏块不应该读到 coded_block_pattern 变量，也就不可能触发这个函数 */
    pMacroBlock->CodedBlockPatternLuma = coded_block_pattern & 0x0F; // CodedBlockPatternLuma = coded_block_pattern % 16
    pMacroBlock->CodedBlockPatternChroma = coded_block_pattern >> 4; // CodedBlockPatternChroma = coded_block_pattern / 16
}
