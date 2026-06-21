#include "HAN_VideoH264.h"
#include "HAN_VideoH264Lib.h"

typedef enum {
    VIDEO_H264_READ_UE_RET_OK,
    VIDEO_H264_READ_UE_RET_ERROR,
} VIDEOMP4READUERET;

typedef struct tagVIDEOMP4READUE {
    const uint8_t*                  pData;
    HANSIZE                         nDataLen;
    HANCPRIVATE uint32_t            cData;
    HANCPRIVATE HANSIZE             iByte;
    HANCPRIVATE uint8_t             iBit;
    HANCPUBLIC union {
        uint32_t                    u32;
        int32_t                     s32;
    } cValue;
} VIDEOMP4READUE, * PVIDEOMP4READUE;

static inline VIDEOMP4READUE H264InitReadUE(const uint8_t* pData, HANSIZE nLen);
static inline VIDEOMP4READUERET H264ReadUE(PVIDEOMP4READUE pReadUE);
static inline VIDEOMP4READUERET H264ReadSE(PVIDEOMP4READUE pReadUE);
static inline uint8_t H264ReadUEBit(PVIDEOMP4READUE pReadUE);
static inline uint32_t H264ReadUEBits(PVIDEOMP4READUE pReadUE, uint8_t nLen);
static BOOL H264CheckMoreRbspData(PVIDEOMP4READUE pReadUE);

static VIDEOMP4READUERET DecodeH264Parameter_seq_parameter_set_data_ProfileIdcExtendedData(PVIDEOH264PARAMETER_seq_parameter_set pSPS, PVIDEOMP4READUE pReadUE);
static VIDEOMP4READUERET DecodeH264Parameter_vui_parameters(PVIDEOH264PARAMETER_vui_parameters pVUI, PVIDEOMP4READUE pReadUE);
static VIDEOMP4READUERET DecodeH264Parameter_hrd_parameters(PVIDEOH264PARAMETER_hrd_parameters pHRD, PVIDEOMP4READUE pReadUE);

static VIDEOMP4READUERET DecodeH264Parameter_ref_pic_list_mvc_modification(PVIDEOH264PARAMETER_ref_pic_list_mvc pRefPicListMVC, uint32_t sliceType, PVIDEOMP4READUE pReadUE);
static VIDEOMP4READUERET DecodeH264Parameter_ref_pic_list_modification(PVIDEOH264PARAMETER_ref_pic_list pRefPicList, uint32_t sliceType, PVIDEOMP4READUE pReadUE);
// static VIDEOMP4READUERET DecodeH264Parameter_pred_weight_table(PVIDEOH264PARAMETER_ref_pic_list pRefPicList, uint32_t sliceType, PVIDEOMP4READUE pReadUE);
static VIDEOMP4READUERET DecodeH264Parameter_dec_ref_pic_marking(PVIDEOH264PARAMETER_dec_ref_pic_marking pDecRefPicMarking, uint8_t idrPicFlag, PVIDEOMP4READUE pReadUE);

// static void DecodeH264ParameterDataToInfo_vuiParam(PVIDEOH264INFOSPS pSPS, PCVIDEOH264PARAMETER_vui_parameters pVUI);

// static VIDEOH264PROFILETYPE GetSPSProfileType(PCVIDEOH264PARAMETER_seq_parameter_set pSPS);

// static const VIDEOH264INFOSPS sg_spsDefaultInfo = {
//     .nChromaFormatIdc = 1,
//     .bSeparateColourPlane = FALSE,
//     .nBitDepthY = 8,
//     .nQpBdOffsetY = 0,
//     .nBitDepthC = 8,
//     .nQpBdOffsetC = 0,
//     .seqScalingMatrix = {
//         .bValid = FALSE,
//     },
//     .bMbAdaptiveFrameField = FALSE,
//     .frameCrop = {
//         .left = 0,
//         .right = 0,
//         .top = 0,
//         .bottom = 0,
//     },
//     .vuiParam = {
//         .bValid = FALSE,
//         .aspectRatio = {
//             .bValid = FALSE,
//         },
//         .eOverscan = VIDEO_H264_OVERSCAN_NONE,
//         .videoSignalType = {
//             .eVideoFormat = 5,
//             .bVideoFullRange = FALSE,
//             .colourDescription = {
//                 .eColourPrimaries = 2,
//                 .eTransferCharacteristics = 2,
//                 .eMatrixCoefficients = 2,
//             },
//         },
//         .chromaLoc = {
//             .top = 0,
//             .bottom = 0,
//         },
//         .timming.bValid = FALSE,
//         .nal.bValid = FALSE,
//         .vcl.bValid = FALSE,
//         .bLowDelayHRD = TRUE,
//         .picStruct = {
//             .bValid = FALSE,
//             .bMotionVectorsOverPicBoundaries = TRUE,
//             .nMaxBytesPerPicDenom = 2,
//             .nMaxBitsPerMbDenom = 1,
//             .nMaxMvLenH = 15,
//             .nMaxMvLenV = 15,
//             .nMaxNumReorderFrames = 0,
//             .nMaxDecFrameBuffering = 0,
//         },
//     },
//     /* 解码用 */
//     .nSubWidthC = 0,
//     .nSubHeightC = 0,
// };

uint8_t GetIdrPicFlag(uint8_t nal_unit_type)
{
    uint8_t nRet;

    if (5 == nal_unit_type) { nRet = 1; }
    else { nRet = 0; }

    return nRet;
}

uint32_t DecodeH264Parameter_seq_parameter_set(const uint8_t* pData, uint32_t nSize, PVIDEOH264PARAMETER_seq_parameter_set pSPS)
{
    BOOL bOk = TRUE;
    VIDEOMP4READUE readUE = H264InitReadUE(pData, nSize);
    
    (void)H264ReadUEBit(&readUE); // forbidden_zero_bit
    pSPS->nal_ref_idc = (uint8_t)H264ReadUEBits(&readUE, 2);
    pSPS->nal_unit_type = (uint8_t)H264ReadUEBits(&readUE, 5);
    pSPS->profile_idc = (uint8_t)H264ReadUEBits(&readUE, 8);
    pSPS->constraint_set0_flag = H264ReadUEBit(&readUE);
    pSPS->constraint_set1_flag = H264ReadUEBit(&readUE);
    pSPS->constraint_set2_flag = H264ReadUEBit(&readUE);
    pSPS->constraint_set3_flag = H264ReadUEBit(&readUE);
    pSPS->constraint_set4_flag = H264ReadUEBit(&readUE);
    pSPS->constraint_set5_flag = H264ReadUEBit(&readUE);
    pSPS->reserved_zero_2bits = (uint8_t)H264ReadUEBits(&readUE, 2);
    pSPS->level_idc = (uint8_t)H264ReadUEBits(&readUE, 8);
    (void)H264ReadUE(&readUE); pSPS->seq_parameter_set_id = readUE.cValue.u32;
    DecodeH264Parameter_seq_parameter_set_data_ProfileIdcExtendedData(pSPS, &readUE);
    (void)H264ReadUE(&readUE); pSPS->log2_max_frame_num_minus4 = readUE.cValue.u32;
    (void)H264ReadUE(&readUE); pSPS->pic_order_cnt_type = readUE.cValue.u32;
    if (0 == pSPS->pic_order_cnt_type) { (void)H264ReadUE(&readUE); pSPS->log2_max_pic_order_cnt_lsb_minus4 = readUE.cValue.u32; }
    else if (1 == pSPS->pic_order_cnt_type)
    {
        pSPS->delta_pic_order_always_zero_flag = H264ReadUEBit(&readUE);
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
    else { /* 根据规范，只有 0 和 1 需要读额外的内容 */ }
    if (bOk == TRUE)
    {
        (void)H264ReadUE(&readUE); pSPS->max_num_ref_frames = readUE.cValue.u32;
        pSPS->gaps_in_frame_num_value_allowed_flag = H264ReadUEBit(&readUE);
        (void)H264ReadUE(&readUE); pSPS->pic_width_in_mbs_minus1 = readUE.cValue.u32;
        (void)H264ReadUE(&readUE); pSPS->pic_height_in_map_units_minus1 = readUE.cValue.u32;
        pSPS->frame_mbs_only_flag = H264ReadUEBit(&readUE);
        if (0 == pSPS->frame_mbs_only_flag) { pSPS->mb_adaptive_frame_field_flag = H264ReadUEBit(&readUE); }
        pSPS->direct_8x8_inference_flag = H264ReadUEBit(&readUE);
        pSPS->frame_cropping_flag = H264ReadUEBit(&readUE);
        if (0 != pSPS->frame_cropping_flag)
        {
            (void)H264ReadUE(&readUE); pSPS->frame_crop_left_offset = readUE.cValue.u32;
            (void)H264ReadUE(&readUE); pSPS->frame_crop_right_offset = readUE.cValue.u32;
            (void)H264ReadUE(&readUE); pSPS->frame_crop_top_offset = readUE.cValue.u32;
            (void)H264ReadUE(&readUE); pSPS->frame_crop_bottom_offset = readUE.cValue.u32;
        }
        pSPS->vui_parameters_present_flag = H264ReadUEBit(&readUE);
        if (0 != pSPS->vui_parameters_present_flag) { DecodeH264Parameter_vui_parameters(&(pSPS->vui_parameters), &readUE); }
    }

    return (uint32_t)(readUE.iByte + 1);
}
BOOL SPSProfileIdcHasExtendedData(uint8_t nProfileIdc)
{
    BOOL bRet = FALSE;

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
    { bRet = TRUE; }

    return bRet;
}

uint32_t DecodeH264Parameter_pic_parameter_set(const uint8_t* pData, uint32_t nSize, PVIDEOH264PARAMETER_pic_parameter_set pPPS, uint32_t* slice_group_id)
{
    VIDEOMP4READUE readUE = H264InitReadUE(pData, nSize);
    
    (void)H264ReadUEBit(&readUE); // forbidden_zero_bit
    pPPS->nal_ref_idc = (uint8_t)H264ReadUEBits(&readUE, 2);
    pPPS->nal_unit_type = (uint8_t)H264ReadUEBits(&readUE, 5);
    (void)H264ReadUE(&readUE); pPPS->pic_parameter_set_id = readUE.cValue.u32;
    (void)H264ReadUE(&readUE); pPPS->seq_parameter_set_id = readUE.cValue.u32;
    pPPS->entropy_coding_mode_flag = H264ReadUEBit(&readUE);
    pPPS->bottom_field_pic_order_in_frame_present_flag = H264ReadUEBit(&readUE);
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
            pPPS->slice_group_change_direction_flag = H264ReadUEBit(&readUE);
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
    pPPS->weighted_pred_flag = H264ReadUEBit(&readUE);
    pPPS->weighted_bipred_idc = (uint8_t)H264ReadUEBits(&readUE, 2);
    (void)H264ReadSE(&readUE); pPPS->pic_init_qp_minus26 = readUE.cValue.s32;
    (void)H264ReadSE(&readUE); pPPS->pic_init_qs_minus26 = readUE.cValue.s32;
    (void)H264ReadSE(&readUE); pPPS->chroma_qp_index_offset = readUE.cValue.s32;
    pPPS->deblocking_filter_control_present_flag = H264ReadUEBit(&readUE);
    pPPS->constrained_intra_pred_flag = H264ReadUEBit(&readUE);
    pPPS->redundant_pic_cnt_present_flag = H264ReadUEBit(&readUE);

    if (TRUE == H264CheckMoreRbspData(&readUE))
    {
        pPPS->bMoreRbspData = TRUE;
        pPPS->transform_8x8_mode_flag = H264ReadUEBit(&readUE);
        pPPS->pic_scaling_matrix_present_flag = H264ReadUEBit(&readUE);
        if (0 != pPPS->pic_scaling_matrix_present_flag)
        {
            ;
        }
        (void)H264ReadSE(&readUE); pPPS->second_chroma_qp_index_offset = readUE.cValue.s32;
    }
    else { pPPS->bMoreRbspData = FALSE; }

    return (uint32_t)(readUE.iByte + 1);
}

uint32_t DecodeH264Parameter_slice_layer_without_partitioning(const uint8_t* pData, uint32_t nSize, PVIDEOH264PARAMETER_slice_layer_without_partitioning pSlice)
{
    VIDEOMP4READUE readUE = H264InitReadUE(pData, nSize);
    HANSIZE nSPSCnt = pSlice->SPS.nCnt;
    PCVIDEOH264PARAMETER_seq_parameter_set pSPS = pSlice->SPS.pSPS;
    HANSIZE nPPSCnt = pSlice->PPS.nCnt;
    PCVIDEOH264PARAMETER_pic_parameter_set pPPS = pSlice->PPS.pPPS;
    VIDEOH264SLICETYPE sliceType;
    uint8_t idrPicFlag;

    (void)H264ReadUEBit(&readUE);
    pSlice->slice_header.nal_ref_idc = H264ReadUEBits(&readUE, 2);
    pSlice->slice_header.nal_unit_type = H264ReadUEBits(&readUE, 5);
    (void)H264ReadUE(&readUE); pSlice->slice_header.first_mb_in_slice = readUE.cValue.u32;
    (void)H264ReadUE(&readUE); pSlice->slice_header.slice_type = readUE.cValue.u32;
    (void)H264ReadUE(&readUE); pSlice->slice_header.pic_parameter_set_id = readUE.cValue.u32;

    idrPicFlag = GetIdrPicFlag(pSlice->slice_header.nal_unit_type);
    sliceType = pSlice->slice_header.slice_type % 5;

    for (HANSIZE iLoop = 0; iLoop < nPPSCnt; iLoop++)
    {
        if (pPPS[iLoop].pic_parameter_set_id == pSlice->slice_header.pic_parameter_set_id)
        {
            pPPS = &pPPS[iLoop];
            for (HANSIZE jLoop = 0; jLoop < nSPSCnt; jLoop++)
            {
                if (pSPS[jLoop].seq_parameter_set_id == pPPS->seq_parameter_set_id)
                {
                    pSPS = &pSPS[jLoop];
                    if (1 == pSPS->separate_colour_plane_flag)
                    {
                        pSlice->slice_header.colour_plane_id = H264ReadUEBits(&readUE, 2);
                    }
                    pSlice->slice_header.frame_num = H264ReadUEBits(&readUE, pSPS->log2_max_frame_num_minus4 + 4);
                    if (0 == pSPS->frame_mbs_only_flag)
                    {
                        pSlice->slice_header.field_pic_flag = H264ReadUEBit(&readUE);
                        if (0 != pSlice->slice_header.field_pic_flag)
                        {
                            pSlice->slice_header.bottom_field_flag = H264ReadUEBit(&readUE);
                        }
                    }
                    if (0 != idrPicFlag)
                    {
                        (void)H264ReadUE(&readUE); pSlice->slice_header.idr_pic_id = readUE.cValue.u32;
                    }
                    if (0 == pSPS->pic_order_cnt_type)
                    {
                        pSlice->slice_header.pic_order_cnt_lsb = H264ReadUEBits(&readUE, pSPS->log2_max_pic_order_cnt_lsb_minus4 + 4);
                        if ((0 != pPPS->bottom_field_pic_order_in_frame_present_flag) && (0 == pSlice->slice_header.field_pic_flag))
                        {
                            H264ReadSE(&readUE); pSlice->slice_header.delta_pic_order_cnt_bottom = readUE.cValue.s32;
                        }
                    }
                    if ((1 == pSPS->pic_order_cnt_type) && (0 == pSPS->delta_pic_order_always_zero_flag))
                    {
                        H264ReadSE(&readUE); pSlice->slice_header.delta_pic_order_cnt[0] = readUE.cValue.s32;
                        if ((0 != pPPS->bottom_field_pic_order_in_frame_present_flag) && (0 == pSlice->slice_header.field_pic_flag))
                        {
                            H264ReadSE(&readUE); pSlice->slice_header.delta_pic_order_cnt[1] = readUE.cValue.s32;
                        }
                    }
                    if (0 != pPPS->redundant_pic_cnt_present_flag)
                    {
                        (void)H264ReadUE(&readUE); pSlice->slice_header.redundant_pic_cnt = readUE.cValue.u32;
                    }
                    if (VIDEO_H264_SLICE_TYPE_B == sliceType)
                    {
                        pSlice->slice_header.direct_spatial_mv_pred_flag = H264ReadUEBit(&readUE);
                    }
                    if ((VIDEO_H264_SLICE_TYPE_P == sliceType) || (VIDEO_H264_SLICE_TYPE_SP == sliceType) || (VIDEO_H264_SLICE_TYPE_B == sliceType))
                    {
                        pSlice->slice_header.num_ref_idx_active_override_flag = H264ReadUEBit(&readUE);
                        if (0 != pSlice->slice_header.num_ref_idx_active_override_flag)
                        {
                            (void)H264ReadUE(&readUE); pSlice->slice_header.num_ref_idx_l0_active_minus1 = readUE.cValue.u32;
                            if (VIDEO_H264_SLICE_TYPE_B == sliceType)
                            {
                                (void)H264ReadUE(&readUE); pSlice->slice_header.num_ref_idx_l1_active_minus1 = readUE.cValue.u32;
                            }
                        }
                    }
                    if ((20 == pSlice->slice_header.nal_unit_type) || (21 == pSlice->slice_header.nal_unit_type))
                    {
                        DecodeH264Parameter_ref_pic_list_mvc_modification(&(pSlice->slice_header.ref_pic_list_mvc), pSlice->slice_header.slice_type, &readUE);
                    }
                    else
                    {
                        DecodeH264Parameter_ref_pic_list_modification(&(pSlice->slice_header.ref_pic_list), pSlice->slice_header.slice_type, &readUE);
                    }
                    if (((0 != pPPS->weighted_pred_flag) && ((VIDEO_H264_SLICE_TYPE_P == sliceType) || (VIDEO_H264_SLICE_TYPE_SP == sliceType))) ||
                        ((1 == pPPS->weighted_bipred_idc) && (VIDEO_H264_SLICE_TYPE_B == sliceType)))
                    {
                        // DecodeH264Parameter_pred_weight_table;
                        printf("pred_weight_table 未实现\n");
                    }
                    if (0 != pSlice->slice_header.nal_ref_idc)
                    {
                        DecodeH264Parameter_dec_ref_pic_marking(&(pSlice->slice_header.dec_ref_pic_marking), idrPicFlag, &readUE);
                    }
                    if ((0 != pPPS->entropy_coding_mode_flag) && (VIDEO_H264_SLICE_TYPE_I != sliceType) && (VIDEO_H264_SLICE_TYPE_SI != sliceType))
                    {
                        (void)H264ReadUE(&readUE); pSlice->slice_header.cabac_init_idc = readUE.cValue.u32;
                    }
                    (void)H264ReadSE(&readUE); pSlice->slice_header.slice_qp_delta = readUE.cValue.s32;
                    if ((VIDEO_H264_SLICE_TYPE_SP == sliceType) || (VIDEO_H264_SLICE_TYPE_SI == sliceType))
                    {
                        if (VIDEO_H264_SLICE_TYPE_SP == sliceType)
                        {
                            pSlice->slice_header.sp_for_switch_flag = H264ReadUEBit(&readUE);
                        }
                        (void)H264ReadSE(&readUE); pSlice->slice_header.slice_qs_delta = readUE.cValue.s32;
                    }
                    if (0 != pPPS->deblocking_filter_control_present_flag)
                    {
                        (void)H264ReadUE(&readUE); pSlice->slice_header.disable_deblocking_filter_idc = readUE.cValue.u32;
                        if (1 != pSlice->slice_header.disable_deblocking_filter_idc)
                        {
                            (void)H264ReadSE(&readUE); pSlice->slice_header.slice_alpha_c0_offset_div2 = readUE.cValue.s32;
                            (void)H264ReadSE(&readUE); pSlice->slice_header.slice_beta_offset_div2 = readUE.cValue.s32;
                        }
                    }
                    if ((0 < pPPS->num_slice_groups_minus1) && (3 <= pPPS->slice_group_map_type) && (pPPS->slice_group_map_type <= 5))
                    {
                        (void)H264ReadUE(&readUE); pSlice->slice_header.slice_group_change_cycle = readUE.cValue.u32;
                    }
                }
            }
        }
    }

    return (uint32_t)(readUE.iByte + 1);
}

// BOOL DecodeH264ParameterDataToInfo_SPS(PVIDEOH264INFOSPS pInfo, PCVIDEOH264PARAMETER_seq_parameter_set pData)
// {
//     BOOL bRet = TRUE;

//     *pInfo = sg_spsDefaultInfo;
    
//     pInfo->eProfile = GetSPSProfileType(pData);
//     pInfo->nProfileIdc = pData->profile_idc;
//     pInfo->bConstraintSetFlags.bSet0 = pData->constraint_set0_flag;
//     pInfo->bConstraintSetFlags.bSet1 = pData->constraint_set1_flag;
//     pInfo->bConstraintSetFlags.bSet2 = pData->constraint_set2_flag;
//     pInfo->bConstraintSetFlags.bSet3 = pData->constraint_set3_flag;
//     pInfo->bConstraintSetFlags.bSet4 = pData->constraint_set4_flag;
//     pInfo->bConstraintSetFlags.bSet5 = pData->constraint_set5_flag;
//     pInfo->nLevelIdc = pData->level_idc;

//     if (TRUE == pData->bExtendedProfileIdcData)
//     {
//         pInfo->nChromaFormatIdc = pData->chroma_format_idc;
//         if (3 == pData->chroma_format_idc) { pInfo->bSeparateColourPlane = pData->separate_colour_plane_flag; }
//         pInfo->nBitDepthY = 8 + pData->bit_depth_luma_minus8;
//         pInfo->nQpBdOffsetY = 6 * pData->bit_depth_luma_minus8;
//         pInfo->nBitDepthC = 8 + pData->bit_depth_luma_minus8;
//         pInfo->nQpBdOffsetC = 6 * pData->bit_depth_luma_minus8;
//         pInfo->bTransformBypass = pData->qpprime_y_zero_transform_bypass_flag;
//         pInfo->seqScalingMatrix.bValid = H264UintToBool(pData->seq_scaling_matrix_present_flag);
//     }
//     if (FALSE == pInfo->bSeparateColourPlane) { pInfo->nChromaArrayType = pInfo->nChromaFormatIdc; }
//     else { pInfo->nChromaArrayType = 0; }
//     switch (pData->chroma_format_idc) {
//         case 1: {
//             pInfo->nSubWidthC = 2;
//             pInfo->nSubHeightC = 2;
//         } break;
//         case 2: {
//             pInfo->nSubWidthC = 2;
//             pInfo->nSubHeightC = 1;
//         } break;
//         case 3: {
//             pInfo->nSubWidthC = 1;
//             pInfo->nSubHeightC = 1;
//         } break;
//         default: { } break;
//     }
//     if (0 != pInfo->nSubWidthC) { pInfo->nMbWidthC = 16 / pInfo->nSubWidthC; }
//     else { pInfo->nMbWidthC = 0; }
//     if (0 != pInfo->nSubHeightC) { pInfo->nMbHeightC = 16 / pInfo->nSubHeightC; }
//     else { pInfo->nMbHeightC = 0; }
//     pInfo->nRawMbBits = 256 * pInfo->nBitDepthY + 2 * pInfo->nMbWidthC * pInfo->nMbHeightC * pInfo->nBitDepthC;

//     pInfo->nMaxFrameNum = ((HANSIZE)1) << ((HANSIZE)(pData->log2_max_frame_num_minus4) + (HANSIZE)4);
//     /* POC */
//     pInfo->nPOCType = pData->pic_order_cnt_type;
//     if (0 == pData->pic_order_cnt_type)
//     {
//         pInfo->POC0.nMaxPicOrderCntLsb = ((HANSIZE)1) << ((HANSIZE)(pData->log2_max_pic_order_cnt_lsb_minus4) + (HANSIZE)4);
//     }
//     else if (1 == pData->pic_order_cnt_type)
//     {
//         pInfo->POC1.bDeltaPicOrderAlwaysZero = pData->delta_pic_order_always_zero_flag;
//         pInfo->POC1.nNonRefPicOffset = pData->offset_for_non_ref_pic;
//         pInfo->POC1.nTopToBottomFieldOffset = pData->offset_for_top_to_bottom_field;
//         pInfo->POC1.nRefFramesNum = pData->num_ref_frames_in_pic_order_cnt_cycle;
//         pInfo->nExpectedDeltaPerPicOrderCntCycle = 0;
//         for (uint32_t iLoop = 0; iLoop < pData->num_ref_frames_in_pic_order_cnt_cycle; iLoop++)
//         {
//             pInfo->POC1.pRefFrameOffset[iLoop] = pData->offset_for_ref_frame[iLoop];
//             pInfo->nExpectedDeltaPerPicOrderCntCycle += pData->offset_for_ref_frame[iLoop];
//         }
//     }

//     pInfo->nMaxNumRefFrames = pData->max_num_ref_frames;
//     pInfo->bGapsInFrameNumValueAllowedFlag = pData->gaps_in_frame_num_value_allowed_flag;
//     /* 大小 */
//     pInfo->nPicWidthInMbs = pData->pic_width_in_mbs_minus1 + 1;
//     pInfo->nPicHeightInMapUnits = pData->pic_height_in_map_units_minus1 + 1;
//     pInfo->nPicWidthInSamplesL = pInfo->nPicWidthInMbs * 16;
//     pInfo->nPicWidthInSamplesC = pInfo->nPicWidthInMbs * pInfo->nMbWidthC;
//     pInfo->nPicSizeInMapUnits = pInfo->nPicWidthInMbs * pInfo->nPicHeightInMapUnits;

//     pInfo->nFrameMacroBlockOnly = pData->frame_mbs_only_flag;
//     pInfo->bDirect8x8Inference = pData->direct_8x8_inference_flag;
//     if (0 == pData->frame_mbs_only_flag)
//     {
//         pInfo->bMbAdaptiveFrameField = pData->mb_adaptive_frame_field_flag;
//         pInfo->bDirect8x8Inference = 1;
//     }
//     pInfo->nFrameHeightInMbs = (2 - pInfo->nFrameMacroBlockOnly) * pInfo->nPicHeightInMapUnits;

//     if (0 == pInfo->nChromaArrayType)
//     {
//         pInfo->nCropUnitX = 1;
//         pInfo->nCropUnitY = 2 - pData->frame_mbs_only_flag;
//     }
//     else
//     {
//         pInfo->nCropUnitX = pInfo->nSubWidthC;
//         pInfo->nCropUnitY = pInfo->nSubHeightC * (2 - pData->frame_mbs_only_flag);
//     }

//     if (0 != pData->frame_cropping_flag)
//     {
//         pInfo->frameCrop.left = pData->frame_crop_left_offset * pInfo->nCropUnitX;
//         pInfo->frameCrop.right = pData->frame_crop_right_offset * pInfo->nCropUnitX;
//         pInfo->frameCrop.top = pData->frame_crop_top_offset * pInfo->nCropUnitY;
//         pInfo->frameCrop.bottom = pData->frame_crop_bottom_offset * pInfo->nCropUnitY;
//     }

//     if (0 != pData->vui_parameters_present_flag) { DecodeH264ParameterDataToInfo_vuiParam(pInfo, &(pData->vui_parameters)); }

//     return bRet;
// }

static inline VIDEOMP4READUE H264InitReadUE(const uint8_t* pData, HANSIZE nLen)
{
    VIDEOMP4READUE rueRet = {
        .pData = pData,
        .nDataLen = nLen,
        .cData = pData[0],
        .iByte = 0,
        .iBit = 0,
    };

    return rueRet;
}
static inline VIDEOMP4READUERET H264ReadUE(PVIDEOMP4READUE pReadUE)
{
    VIDEOMP4READUERET eRet = VIDEO_H264_READ_UE_RET_ERROR;
    HANSIZE nBitLen = ((pReadUE->nDataLen - pReadUE->iByte) * 8) - pReadUE->iBit;
    HANSIZE nReadBitMax = nBitLen;
    uint8_t nZeroBits;
    uint32_t cX;

    if (31 < nReadBitMax) { nReadBitMax = 31; }

    for (nZeroBits = 0; nZeroBits < nReadBitMax; nZeroBits++)
    {
        if (0 != H264ReadUEBit(pReadUE))
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
            cX = (cX << 1) + H264ReadUEBit(pReadUE);
        }
        pReadUE->cValue.u32 = (1 << nZeroBits) - 1 + cX;

        eRet = VIDEO_H264_READ_UE_RET_OK;
    }
    
    return eRet;
}
static inline VIDEOMP4READUERET H264ReadSE(PVIDEOMP4READUE pReadUE)
{
    VIDEOMP4READUERET eRet = H264ReadUE(pReadUE);
    int32_t s32;

    if (VIDEO_H264_READ_UE_RET_OK == eRet)
    {
        if ((1 & pReadUE->cValue.u32) == 0) { s32 = -1; }
        else { s32 = 1; }
        pReadUE->cValue.s32 = s32 * ((pReadUE->cValue.u32 + 1) >> 1);
    }
    
    return eRet;
}
static inline uint8_t H264ReadUEBit(PVIDEOMP4READUE pReadUE)
{
    uint8_t cRet = (pReadUE->cData << pReadUE->iBit) & 0x00000080;

    pReadUE->iBit++;
    if (8 == pReadUE->iBit)
    {
        pReadUE->iBit = 0;
        pReadUE->iByte++;
        pReadUE->cData <<= 8;
        pReadUE->cData += pReadUE->pData[pReadUE->iByte];
        if ((uint32_t)0x00000003 == (pReadUE->cData & (uint32_t)0x00FFFFFF))
        {
            /* 根据规范 7.4.1 对 emulation_prevention_three_byte 的描述
             * NALU 出现以下字节序列时，需要在最后两个字节间插入一个防竞争字节 0x03
             * 0x00 0x00 0x00
             * 0x00 0x00 0x01
             * 0x00 0x00 0x02
             * 0x00 0x00 0x03
             * 插入的防竞争字节仅占位，无含义，解码器应当丢弃该字节
             */
            if (((pReadUE->iByte + 1) < pReadUE->nDataLen) && (pReadUE->pData[pReadUE->iByte + 1] <= 3))
            {
                pReadUE->cData &= 0xFFFFFF00;
                pReadUE->iByte++;
                pReadUE->cData += pReadUE->pData[pReadUE->iByte];
            }
        }
    }
    if (0 != cRet) { cRet = 1; }

    return cRet;
}
static inline uint32_t H264ReadUEBits(PVIDEOMP4READUE pReadUE, uint8_t nLen)
{
    pReadUE->cValue.u32 = 0;
    if (nLen <= 32)
    {
        for (uint8_t iLoop = 0; iLoop < nLen; iLoop++)
        {
            pReadUE->cValue.u32 <<= 1;
            pReadUE->cValue.u32 |= H264ReadUEBit(pReadUE);
        }
    }

    return pReadUE->cValue.u32;
}
static BOOL H264CheckMoreRbspData(PVIDEOMP4READUE pReadUE)
{
    BOOL bRet = FALSE;
    uint8_t cByte;

    if (pReadUE->iByte < pReadUE->nDataLen)
    {
        for (HANSIZE iLoop = pReadUE->nDataLen - 1; pReadUE->iByte <= iLoop; iLoop--)
        {
            if (0 != pReadUE->pData[iLoop])
            {
                if (iLoop != pReadUE->iByte) { bRet = TRUE; }
                else
                {
                    /* 从左往右从 0 开始数，当前指向第 n 比特，那么只要 n + 1 比特开始，直到 bit 7 任意不为 0 就说明还有数据 */
                    cByte = 0xFF >> (pReadUE->iBit + 1);
                    cByte &= pReadUE->pData[iLoop];
                    if (cByte != 0) { bRet = TRUE; }
                }
                break;
            }
        }
    }

    return bRet;
}

static VIDEOMP4READUERET DecodeH264Parameter_seq_parameter_set_data_ProfileIdcExtendedData(PVIDEOH264PARAMETER_seq_parameter_set pSPS, PVIDEOMP4READUE pReadUE)
{
    VIDEOMP4READUERET eRet = VIDEO_H264_READ_UE_RET_OK;

    if (TRUE == SPSProfileIdcHasExtendedData(pSPS->profile_idc))
    {
        pSPS->bExtendedProfileIdcData = TRUE;
        (void)H264ReadUE(pReadUE); pSPS->chroma_format_idc = pReadUE->cValue.u32;
        if (3 == pSPS->chroma_format_idc) { pSPS->separate_colour_plane_flag = H264ReadUEBit(pReadUE); }
        (void)H264ReadUE(pReadUE); pSPS->bit_depth_luma_minus8 = pReadUE->cValue.u32;
        (void)H264ReadUE(pReadUE); pSPS->bit_depth_chroma_minus8 = pReadUE->cValue.u32;
        pSPS->qpprime_y_zero_transform_bypass_flag = H264ReadUEBit(pReadUE);
        pSPS->seq_scaling_matrix_present_flag = H264ReadUEBit(pReadUE);
        if (FALSE != pSPS->seq_scaling_matrix_present_flag)
        {
            printf("矩阵信息解码程序未开发\n");
        }
    }
    else { pSPS->bExtendedProfileIdcData = FALSE; }

    return eRet;
}
static VIDEOMP4READUERET DecodeH264Parameter_vui_parameters(PVIDEOH264PARAMETER_vui_parameters pVUI, PVIDEOMP4READUE pReadUE)
{
    VIDEOMP4READUERET eRet = VIDEO_H264_READ_UE_RET_OK;

    pVUI->aspect_ratio_info_present_flag = H264ReadUEBit(pReadUE);
    if (0 != pVUI->aspect_ratio_info_present_flag)
    {
        pVUI->aspect_ratio_idc = H264ReadUEBits(pReadUE, 8);
        if (VIDEO_H264_EXTENDED_SAR == pVUI->aspect_ratio_idc)
        {
            pVUI->sar_width = (uint16_t)H264ReadUEBits(pReadUE, 16);
            pVUI->sar_height = (uint16_t)H264ReadUEBits(pReadUE, 16);
        }
    }
    pVUI->overscan_info_present_flag = H264ReadUEBit(pReadUE);
    if (0 != pVUI->overscan_info_present_flag) { pVUI->overscan_appropriate_flag = H264ReadUEBit(pReadUE); }
    pVUI->video_signal_type_present_flag = H264ReadUEBit(pReadUE);
    if (0 != pVUI->video_signal_type_present_flag)
    {
        pVUI->video_format = (uint8_t)H264ReadUEBits(pReadUE, 3);
        pVUI->video_full_range_flag = H264ReadUEBit(pReadUE);
        pVUI->colour_description_present_flag = H264ReadUEBit(pReadUE);
        if (0 != pVUI->colour_description_present_flag)
        {
            pVUI->colour_primaries = (uint8_t)H264ReadUEBits(pReadUE, 8);
            pVUI->transfer_characteristics = (uint8_t)H264ReadUEBits(pReadUE, 8);
            pVUI->matrix_coefficients = (uint8_t)H264ReadUEBits(pReadUE, 8);
        }
    }
    pVUI->chroma_loc_info_present_flag = H264ReadUEBit(pReadUE);
    if (0 != pVUI->chroma_loc_info_present_flag)
    {
        (void)H264ReadUE(pReadUE); pVUI->chroma_sample_loc_type_top_field = pReadUE->cValue.u32;
        (void)H264ReadUE(pReadUE); pVUI->chroma_sample_loc_type_bottom_field = pReadUE->cValue.u32;
    }
    pVUI->timing_info_present_flag = H264ReadUEBit(pReadUE);
    if (0 != pVUI->timing_info_present_flag)
    {
        pVUI->num_units_in_tick = H264ReadUEBits(pReadUE, 32);
        pVUI->time_scale = H264ReadUEBits(pReadUE, 32);
        pVUI->fixed_frame_rate_flag = H264ReadUEBit(pReadUE);
    }
    pVUI->nal_hrd_parameters_present_flag = H264ReadUEBit(pReadUE);
    if (0 != pVUI->nal_hrd_parameters_present_flag) { (void)DecodeH264Parameter_hrd_parameters(&(pVUI->nal.hrd_parameters), pReadUE); }
    pVUI->vcl_hrd_parameters_present_flag = H264ReadUEBit(pReadUE);
    if (0 != pVUI->vcl_hrd_parameters_present_flag) { (void)DecodeH264Parameter_hrd_parameters(&(pVUI->vcl.hrd_parameters), pReadUE); }
    if ((0 != pVUI->nal_hrd_parameters_present_flag) || (0 != pVUI->vcl_hrd_parameters_present_flag)) { pVUI->low_delay_hrd_flag = H264ReadUEBit(pReadUE); }
    pVUI->pic_struct_present_flag = H264ReadUEBit(pReadUE);
    pVUI->bitstream_restriction_flag = H264ReadUEBit(pReadUE);
    if (0 != pVUI->bitstream_restriction_flag)
    {
        pVUI->motion_vectors_over_pic_boundaries_flag = H264ReadUEBit(pReadUE);
        (void)H264ReadUE(pReadUE); pVUI->max_bytes_per_pic_denom = pReadUE->cValue.u32;
        (void)H264ReadUE(pReadUE); pVUI->max_bits_per_mb_denom = pReadUE->cValue.u32;
        (void)H264ReadUE(pReadUE); pVUI->log2_max_mv_length_horizontal = pReadUE->cValue.u32;
        (void)H264ReadUE(pReadUE); pVUI->log2_max_mv_length_vertical = pReadUE->cValue.u32;
        (void)H264ReadUE(pReadUE); pVUI->max_num_reorder_frames = pReadUE->cValue.u32;
        (void)H264ReadUE(pReadUE); pVUI->max_dec_frame_buffering = pReadUE->cValue.u32;
    }

    return eRet;
}
static VIDEOMP4READUERET DecodeH264Parameter_hrd_parameters(PVIDEOH264PARAMETER_hrd_parameters pHRD, PVIDEOMP4READUE pReadUE)
{
    VIDEOMP4READUERET eRet = VIDEO_H264_READ_UE_RET_OK;

    (void)H264ReadUE(pReadUE); pHRD->cpb_cnt_minus1 = pReadUE->cValue.u32;
    pHRD->bit_rate_scale = (uint8_t)H264ReadUEBits(pReadUE, 4);
    pHRD->cpb_size_scale = (uint8_t)H264ReadUEBits(pReadUE, 4);
    if (pHRD->cpb_cnt_minus1 <= VIDEO_H264_MAX_VALUE_cpb_cnt_minus1)
    {
        for (uint8_t iLoop = 0; iLoop <= pHRD->cpb_cnt_minus1; iLoop++)
        {
            (void)H264ReadUE(pReadUE); pHRD->bit_rate_value_minus1[iLoop] = pReadUE->cValue.u32;
            (void)H264ReadUE(pReadUE); pHRD->cpb_size_value_minus1[iLoop] = pReadUE->cValue.u32;
            pHRD->cbr_flag[iLoop] = H264ReadUEBit(pReadUE);
        }
        pHRD->initial_cpb_removal_delay_length_minus1 = (uint8_t)H264ReadUEBits(pReadUE, 5);
        pHRD->cpb_removal_delay_length_minus1 = (uint8_t)H264ReadUEBits(pReadUE, 5);
        pHRD->dpb_output_delay_length_minus1 = (uint8_t)H264ReadUEBits(pReadUE, 5);
        pHRD->time_offset_length = (uint8_t)H264ReadUEBits(pReadUE, 5);
    }

    return eRet;
}

static VIDEOMP4READUERET DecodeH264Parameter_ref_pic_list_mvc_modification(PVIDEOH264PARAMETER_ref_pic_list_mvc pRefPicListMVC, uint32_t sliceType, PVIDEOMP4READUE pReadUE)
{
    VIDEOMP4READUERET eRet = VIDEO_H264_READ_UE_RET_OK;
    uint32_t modSliceType = sliceType % VIDEO_H264_SLICE_TYPE_ALL;

    if ((2 != modSliceType) && (4 != modSliceType))
    {
        pRefPicListMVC->ref_pic_list_modification_flag_l0 = H264ReadUEBit(pReadUE);
        if (0 != pRefPicListMVC->ref_pic_list_modification_flag_l0)
        {
            pRefPicListMVC->bValid_abs_diff_pic_num_minus1 = 0;
            pRefPicListMVC->bValid_long_term_pic_num = 0;
            pRefPicListMVC->bValid_abs_diff_view_idx_minus1 = 0;
            do {
                (void)H264ReadUE(pReadUE); pRefPicListMVC->modification_of_pic_nums_idc = pReadUE->cValue.u32;
                if ((0 == pRefPicListMVC->modification_of_pic_nums_idc) || (1 == pRefPicListMVC->modification_of_pic_nums_idc))
                {
                    pRefPicListMVC->bValid_abs_diff_pic_num_minus1 = 1;
                    (void)H264ReadUE(pReadUE); pRefPicListMVC->abs_diff_pic_num_minus1 = pReadUE->cValue.u32;
                }
                else if (2 == pRefPicListMVC->modification_of_pic_nums_idc)
                {
                    pRefPicListMVC->bValid_long_term_pic_num = 1;
                    (void)H264ReadUE(pReadUE); pRefPicListMVC->long_term_pic_num = pReadUE->cValue.u32;
                }
                else if ((4 == pRefPicListMVC->modification_of_pic_nums_idc) || (5 == pRefPicListMVC->modification_of_pic_nums_idc))
                {
                    pRefPicListMVC->bValid_abs_diff_view_idx_minus1 = 1;
                    (void)H264ReadUE(pReadUE); pRefPicListMVC->abs_diff_view_idx_minus1 = pReadUE->cValue.u32;
                }
                else { }
            } while (3 != pRefPicListMVC->modification_of_pic_nums_idc);
        }
    }
    if (1 == modSliceType)
    {
        pRefPicListMVC->ref_pic_list_modification_flag_l1 = H264ReadUEBit(pReadUE);
        if (0 != pRefPicListMVC->ref_pic_list_modification_flag_l1)
        {
            pRefPicListMVC->bValid_abs_diff_pic_num_minus1 = 0;
            pRefPicListMVC->bValid_long_term_pic_num = 0;
            pRefPicListMVC->bValid_abs_diff_view_idx_minus1 = 0;
            do {
                (void)H264ReadUE(pReadUE); pRefPicListMVC->modification_of_pic_nums_idc = pReadUE->cValue.u32;
                if ((0 == pRefPicListMVC->modification_of_pic_nums_idc) || (1 == pRefPicListMVC->modification_of_pic_nums_idc))
                {
                    pRefPicListMVC->bValid_abs_diff_pic_num_minus1 = 1;
                    (void)H264ReadUE(pReadUE); pRefPicListMVC->abs_diff_pic_num_minus1 = pReadUE->cValue.u32;
                }
                else if (2 == pRefPicListMVC->modification_of_pic_nums_idc)
                {
                    pRefPicListMVC->bValid_long_term_pic_num = 1;
                    (void)H264ReadUE(pReadUE); pRefPicListMVC->long_term_pic_num = pReadUE->cValue.u32;
                }
                else if ((4 == pRefPicListMVC->modification_of_pic_nums_idc) || (5 == pRefPicListMVC->modification_of_pic_nums_idc))
                {
                    pRefPicListMVC->bValid_abs_diff_view_idx_minus1 = 1;
                    (void)H264ReadUE(pReadUE); pRefPicListMVC->abs_diff_view_idx_minus1 = pReadUE->cValue.u32;
                }
                else { }
            } while (3 != pRefPicListMVC->modification_of_pic_nums_idc);
        }
    }
    
    return eRet;
}
static VIDEOMP4READUERET DecodeH264Parameter_ref_pic_list_modification(PVIDEOH264PARAMETER_ref_pic_list pRefPicList, uint32_t sliceType, PVIDEOMP4READUE pReadUE)
{
    VIDEOMP4READUERET eRet = VIDEO_H264_READ_UE_RET_OK;
    uint32_t modSliceType = sliceType % 5;

    if ((2 != modSliceType) && (4 != modSliceType))
    {
        pRefPicList->ref_pic_list_modification_flag_l0 = H264ReadUEBit(pReadUE);
        if (0 != pRefPicList->ref_pic_list_modification_flag_l0)
        {
            pRefPicList->bValid_abs_diff_pic_num_minus1 = 0;
            pRefPicList->bValid_long_term_pic_num = 0;
            do {
                (void)H264ReadUE(pReadUE); pRefPicList->modification_of_pic_nums_idc = pReadUE->cValue.u32;
                if ((0 == pRefPicList->modification_of_pic_nums_idc) || (1 == pRefPicList->modification_of_pic_nums_idc))
                {
                    pRefPicList->bValid_abs_diff_pic_num_minus1 = 1;
                    (void)H264ReadUE(pReadUE); pRefPicList->abs_diff_pic_num_minus1 = pReadUE->cValue.u32;
                }
                else if (2 == pRefPicList->modification_of_pic_nums_idc)
                {
                    pRefPicList->long_term_pic_num = 1;
                    (void)H264ReadUE(pReadUE); pRefPicList->long_term_pic_num = pReadUE->cValue.u32;
                }
                else { }
            } while (3 != pRefPicList->modification_of_pic_nums_idc);
        }
    }
    if (1 == modSliceType)
    {
        pRefPicList->ref_pic_list_modification_flag_l1 = H264ReadUEBit(pReadUE);
        if (0 != pRefPicList->ref_pic_list_modification_flag_l1)
        {
            pRefPicList->bValid_abs_diff_pic_num_minus1 = 0;
            pRefPicList->bValid_long_term_pic_num = 0;
            do {
                (void)H264ReadUE(pReadUE); pRefPicList->modification_of_pic_nums_idc = pReadUE->cValue.u32;
                if ((0 == pRefPicList->modification_of_pic_nums_idc) || (1 == pRefPicList->modification_of_pic_nums_idc))
                {
                    pRefPicList->bValid_abs_diff_pic_num_minus1 = 1;
                    (void)H264ReadUE(pReadUE); pRefPicList->abs_diff_pic_num_minus1 = pReadUE->cValue.u32;
                }
                else if (2 == pRefPicList->modification_of_pic_nums_idc)
                {
                    pRefPicList->long_term_pic_num = 1;
                    (void)H264ReadUE(pReadUE); pRefPicList->long_term_pic_num = pReadUE->cValue.u32;
                }
                else { }
            } while (3 != pRefPicList->modification_of_pic_nums_idc);
        }
    }
    
    return eRet;
}
// static VIDEOMP4READUERET DecodeH264Parameter_pred_weight_table(PVIDEOH264PARAMETER_ref_pic_list pRefPicList, uint32_t sliceType, PVIDEOMP4READUE pReadUE);
static VIDEOMP4READUERET DecodeH264Parameter_dec_ref_pic_marking(PVIDEOH264PARAMETER_dec_ref_pic_marking pDecRefPicMarking, uint8_t idrPicFlag, PVIDEOMP4READUE pReadUE)
{
    VIDEOMP4READUERET eRet = VIDEO_H264_READ_UE_RET_OK;

    if (0 != idrPicFlag)
    {
        pDecRefPicMarking->no_output_of_prior_pics_flag = H264ReadUEBit(pReadUE);
        pDecRefPicMarking->long_term_reference_flag = H264ReadUEBit(pReadUE);
    }
    else
    {
        pDecRefPicMarking->adaptive_ref_pic_marking_mode_flag = H264ReadUEBit(pReadUE);
        if (0 != pDecRefPicMarking->adaptive_ref_pic_marking_mode_flag)
        {
            pDecRefPicMarking->bValid_difference_of_pic_nums_minus1 = 0;
            pDecRefPicMarking->bValid_long_term_pic_num = 0;
            pDecRefPicMarking->bValid_long_term_frame_idx = 0;
            pDecRefPicMarking->bValid_max_long_term_frame_idx_plus1 = 0;
            do {
                (void)H264ReadUE(pReadUE); pDecRefPicMarking->memory_management_control_operation = pReadUE->cValue.u32;
                if ((1 == pDecRefPicMarking->memory_management_control_operation) || (3 == pDecRefPicMarking->memory_management_control_operation))
                {
                    pDecRefPicMarking->bValid_difference_of_pic_nums_minus1 = 1;
                    (void)H264ReadUE(pReadUE); pDecRefPicMarking->difference_of_pic_nums_minus1 = pReadUE->cValue.u32;
                }
                if (2 == pDecRefPicMarking->memory_management_control_operation)
                {
                    pDecRefPicMarking->bValid_long_term_pic_num = 1;
                    (void)H264ReadUE(pReadUE); pDecRefPicMarking->long_term_pic_num = pReadUE->cValue.u32;
                }
                if ((3 == pDecRefPicMarking->memory_management_control_operation) || (6 == pDecRefPicMarking->memory_management_control_operation))
                {
                    pDecRefPicMarking->bValid_long_term_frame_idx = 1;
                    (void)H264ReadUE(pReadUE); pDecRefPicMarking->long_term_frame_idx = pReadUE->cValue.u32;
                }
                if (4 == pDecRefPicMarking->memory_management_control_operation)
                {
                    pDecRefPicMarking->bValid_max_long_term_frame_idx_plus1 = 1;
                    (void)H264ReadUE(pReadUE); pDecRefPicMarking->max_long_term_frame_idx_plus1 = pReadUE->cValue.u32;
                }
            } while (0 != pDecRefPicMarking->memory_management_control_operation);
        }
    }

    return eRet;
}

// static void DecodeH264ParameterDataToInfo_vuiParam(PVIDEOH264INFOSPS pSPS, PCVIDEOH264PARAMETER_vui_parameters pVUI)
// {
//     uint16_t* pWidth = &(pSPS->vuiParam.aspectRatio.nWidth);
//     uint16_t* pHeight = &(pSPS->vuiParam.aspectRatio.nHeight);
//     PVIDEOH264INFOVUIPARAM pInfo = &(pSPS->vuiParam);

//     pInfo->bValid = TRUE;

//     if (0 != pVUI->aspect_ratio_info_present_flag)
//     {
//         pInfo->aspectRatio.bValid = TRUE;
//         switch (pVUI->aspect_ratio_idc) {
//             case 1: { *pWidth = 1; *pHeight = 1; } break;
//             case 2: { *pWidth = 12; *pHeight = 11; } break;
//             case 3: { *pWidth = 10; *pHeight = 11; } break;
//             case 4: { *pWidth = 16; *pHeight = 11; } break;
//             case 5: { *pWidth = 40; *pHeight = 33; } break;
//             case 6: { *pWidth = 24; *pHeight = 11; } break;
//             case 7: { *pWidth = 20; *pHeight = 11; } break;
//             case 8: { *pWidth = 32; *pHeight = 11; } break;
//             case 9: { *pWidth = 80; *pHeight = 33; } break;
//             case 10: { *pWidth = 18; *pHeight = 11; } break;
//             case 11: { *pWidth = 15; *pHeight = 11; } break;
//             case 12: { *pWidth = 64; *pHeight = 33; } break;
//             case 13: { *pWidth = 160; *pHeight = 99; } break;
//             case 14: { *pWidth = 4; *pHeight = 3; } break;
//             case 15: { *pWidth = 3; *pHeight = 2; } break;
//             case 16: { *pWidth = 2; *pHeight = 1; } break;
//             case 255: { *pWidth = pVUI->sar_width; *pHeight = pVUI->sar_height; } break;
//             default: { pInfo->aspectRatio.bValid = FALSE; } break;
//         }
//     }
//     if (0 != pVUI->overscan_info_present_flag)
//     {
//         if (0 == pVUI->overscan_appropriate_flag) { pInfo->eOverscan = VIDEO_H264_OVERSCAN_UNDERSCAN; }
//         else { pInfo->eOverscan = VIDEO_H264_OVERSCAN_OVERSCAN; }
//     }
//     if (0 != pVUI->video_signal_type_present_flag)
//     {
//         pInfo->videoSignalType.eVideoFormat = pVUI->video_format;
//         pInfo->videoSignalType.bVideoFullRange = H264UintToBool(pVUI->video_full_range_flag);
//         if (0 != pVUI->colour_description_present_flag)
//         {
//             pInfo->videoSignalType.colourDescription.eColourPrimaries = pVUI->colour_primaries;
//             pInfo->videoSignalType.colourDescription.eTransferCharacteristics = pVUI->transfer_characteristics;
//             pInfo->videoSignalType.colourDescription.eMatrixCoefficients = pVUI->matrix_coefficients;
//         }
//     }
//     if (0 != pVUI->chroma_loc_info_present_flag)
//     {
//         pInfo->chromaLoc.top = pVUI->chroma_sample_loc_type_top_field;
//         pInfo->chromaLoc.bottom = pVUI->chroma_sample_loc_type_bottom_field;
//     }
//     if (0 != pVUI->timing_info_present_flag)
//     {
//         pInfo->timming.nNumUnits = pVUI->num_units_in_tick;
//         pInfo->timming.nTimeScale = pVUI->time_scale;
//         pInfo->timming.bFixedFrameRate = H264UintToBool(pVUI->fixed_frame_rate_flag);
//     }
//     if (0 != pVUI->nal_hrd_parameters_present_flag) {  }
//     if (0 != pVUI->vcl_hrd_parameters_present_flag) {  }
//     pInfo->bLowDelayHRD = H264UintToBool(pVUI->low_delay_hrd_flag);
//     if (0 != pVUI->pic_struct_present_flag)
//     {
//         pInfo->picStruct.bValid = TRUE;
//         pInfo->picStruct.bMotionVectorsOverPicBoundaries = H264UintToBool(pVUI->motion_vectors_over_pic_boundaries_flag);
//         pInfo->picStruct.nMaxBytesPerPicDenom = pVUI->max_bytes_per_pic_denom;
//         pInfo->picStruct.nMaxBitsPerMbDenom = (128 + pSPS->nRawMbBits) / pVUI->max_bits_per_mb_denom;
//     }
// }

// static VIDEOH264PROFILETYPE GetSPSProfileType(PCVIDEOH264PARAMETER_seq_parameter_set pSPS)
// {
//     VIDEOH264PROFILETYPE eRet;

//     switch (pSPS->profile_idc) {
//         case 66: { eRet = VIDEO_H264_PROFILE_TYPE_BASELINE; } break;
//         case 77: { eRet = VIDEO_H264_PROFILE_TYPE_MAIN; } break;
//         case 88: { eRet = VIDEO_H264_PROFILE_TYPE_EXTENDED; } break;
//         case 100: {
//             if (1 == pSPS->constraint_set4_flag)
//             {
//                 if (1 == pSPS->constraint_set5_flag) { eRet = VIDEO_H264_PROFILE_TYPE_CONSTRAINED_HIGH; }
//                 else { eRet = VIDEO_H264_PROFILE_TYPE_PROGRESSIVE_HIGH; }
//             }
//             else { eRet = VIDEO_H264_PROFILE_TYPE_HIGH; }
//         } break;
//         case 110: {
//             if (1 == pSPS->constraint_set3_flag) { eRet = VIDEO_H264_PROFILE_TYPE_HIGH_10_INTRA; }
//             else if (1 == pSPS->constraint_set4_flag) { eRet = VIDEO_H264_PROFILE_TYPE_PROGRESSIVE_HIGH_10; }
//             else { eRet = VIDEO_H264_PROFILE_TYPE_HIGH_10; }
//         } break;
//         case 122: {
//             if (1 == pSPS->constraint_set3_flag) { eRet = VIDEO_H264_PROFILE_TYPE_HIGH_422_INTRA; }
//             else { eRet = VIDEO_H264_PROFILE_TYPE_HIGH_422; }
//         } break;
//         case 244: {
//             if (1 == pSPS->constraint_set3_flag) { eRet = VIDEO_H264_PROFILE_TYPE_HIGH_444_INTRA; }
//             else { eRet = VIDEO_H264_PROFILE_TYPE_HIGH_444_PREDICTIVE; }
//         } break;
//         case 44: { eRet = VIDEO_H264_PROFILE_TYPE_CAVLC_444_INTRA; } break;
//         default: { eRet = VIDEO_H264_PROFILE_TYPE_CNT; } break;
//     }

//     return eRet;
// }
