#ifndef HAN_VIDEO_H264_H
#define HAN_VIDEO_H264_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "..\..\HAN_VideoDef.h"
#include "HAN_VideoH264CABACDef.h"

#define VIDEO_H264_MAX_VALUE_num_ref_frames_in_pic_order_cnt_cycle          255
#define VIDEO_H264_MAX_VALUE_cpb_cnt_minus1                                 31
#define VIDEO_H264_MAX_VALUE_num_slice_groups_minus1                        7
#define VIDEO_H264_MAX_VALUE_cabac_init_idc                                 2
#define VIDEO_H264_MAX_VALUE_num_ref_idx_lx_active_minus1                   31
#define VIDEO_H264_EXTENDED_SAR                                             255

#define VIDEO_H264_MACRO_BLOCK_PART_SIZE                                    4
#define VIDEO_H264_SUB_MACRO_BLOCK_PART_SIZE                                4

typedef enum {
    VIDEO_H264_SLICE_TYPE_P = 0,
    VIDEO_H264_SLICE_TYPE_B = 1,
    VIDEO_H264_SLICE_TYPE_I = 2,
    VIDEO_H264_SLICE_TYPE_SP = 3,
    VIDEO_H264_SLICE_TYPE_SI = 4,
} VIDEOH264SLICETYPE;

typedef enum {
    VIDEO_H264_OVERSCAN_NONE,
    VIDEO_H264_OVERSCAN_OVERSCAN,
    VIDEO_H264_OVERSCAN_UNDERSCAN,
    VIDEO_H264_OVERSCAN_CNT,
} VIDEOH264OVERSCAN;

typedef enum {
    VIDEO_H264_PROFILE_TYPE_BASELINE,
    VIDEO_H264_PROFILE_TYPE_CONSTRAINED_BASELINE,
    VIDEO_H264_PROFILE_TYPE_MAIN,
    VIDEO_H264_PROFILE_TYPE_EXTENDED,
    VIDEO_H264_PROFILE_TYPE_HIGH,
    VIDEO_H264_PROFILE_TYPE_PROGRESSIVE_HIGH,
    VIDEO_H264_PROFILE_TYPE_CONSTRAINED_HIGH,
    VIDEO_H264_PROFILE_TYPE_HIGH_10,
    VIDEO_H264_PROFILE_TYPE_PROGRESSIVE_HIGH_10,
    VIDEO_H264_PROFILE_TYPE_HIGH_422,
    VIDEO_H264_PROFILE_TYPE_HIGH_444_PREDICTIVE,
    VIDEO_H264_PROFILE_TYPE_HIGH_10_INTRA,
    VIDEO_H264_PROFILE_TYPE_HIGH_422_INTRA,
    VIDEO_H264_PROFILE_TYPE_HIGH_444_INTRA,
    VIDEO_H264_PROFILE_TYPE_CAVLC_444_INTRA,
    VIDEO_H264_PROFILE_TYPE_CNT,
} VIDEOH264PROFILETYPE;

typedef enum {
    VIDEO_H264_SPS_FIELD_PROFILE_IDC,
    VIDEO_H264_SPS_FIELD_CONSTRAINT_SET0,
    VIDEO_H264_SPS_FIELD_CONSTRAINT_SET1,
    VIDEO_H264_SPS_FIELD_CONSTRAINT_SET2,
    VIDEO_H264_SPS_FIELD_CONSTRAINT_SET3,
    VIDEO_H264_SPS_FIELD_CONSTRAINT_SET4,
    VIDEO_H264_SPS_FIELD_CONSTRAINT_SET5,
    VIDEO_H264_SPS_FIELD_LEVEL_IDC,
    VIDEO_H264_SPS_FIELD_SPS_ID,
    VIDEO_H264_SPS_FIELD_CHROMA_FORMAT,
    VIDEO_H264_SPS_FIELD_SEPARATE_COLOUR_PLANE,
    VIDEO_H264_SPS_FIELD_LUMA_BIT_DEPTH,
    VIDEO_H264_SPS_FIELD_LUMA_QUANTIZATION_PARAMETER_RANGE_OFFSET,
    VIDEO_H264_SPS_FIELD_CHROMA_BIT_DEPTH,
    VIDEO_H264_SPS_FIELD_CHROMA_QUANTIZATION_PARAMETER_RANGE_OFFSET,
    VIDEO_H264_SPS_FIELD_TRANSFORM_BYPASS,
    VIDEO_H264_SPS_FIELD_MAX_FRAME_NUM,
    VIDEO_H264_SPS_FIELD_POC_TYPE,
    VIDEO_H264_SPS_FIELD_POC0_MAX_PIC_ORDER_CNT_LSB,
    VIDEO_H264_SPS_FIELD_POC1_DELTA_PIC_ORDER_ALWAYSZ_ERO,
    VIDEO_H264_SPS_FIELD_POC1_NON_REF_PIC_OFFSET,
    VIDEO_H264_SPS_FIELD_POC1_TOP_TO_BOTTOM_FIELD_OFFSET,
    VIDEO_H264_SPS_FIELD_POC1_REF_FRAMES_NUM,
    VIDEO_H264_SPS_FIELD_POC1_REF_FRAME_OFFSET,
    VIDEO_H264_SPS_FIELD_MAX_NUM_REF_FRAMES,
    VIDEO_H264_SPS_FIELD_GAPS_IN_FRAME_ALLOWED,
    VIDEO_H264_SPS_FIELD_WIDTH,
    VIDEO_H264_SPS_FIELD_HEIGHT,
    VIDEO_H264_SPS_FIELD_ADAPTIVE_FRAME_FIELD,
    VIDEO_H264_SPS_FIELD_DIRECT_8X8_INFERENCE,
    VIDEO_H264_SPS_FIELD_FRAME_CROP_LEFT_OFFSET,
    VIDEO_H264_SPS_FIELD_FRAME_CROP_RIGHT_OFFSET,
    VIDEO_H264_SPS_FIELD_FRAME_CROP_TOP_OFFSET,
    VIDEO_H264_SPS_FIELD_FRAME_CROP_BOTTOM_OFFSET,
    VIDEO_H264_SPS_FIELD_CNT,
} VIDEOH264FIELDSPS;

typedef enum {
    VIDEO_H264_VUI_PARAM_FIELD_ASPECT_RATIO,
    VIDEO_H264_VUI_PARAM_FIELD_OVERSCAN,
    VIDEO_H264_VUI_PARAM_FIELD_VIDEO_FORMAT,
    VIDEO_H264_VUI_PARAM_FIELD_VIDEO_FULL_RANGE,
    VIDEO_H264_VUI_PARAM_FIELD_COLOUR_PRIMARIES,
    VIDEO_H264_VUI_PARAM_FIELD_TRANSFER_CHARACTERISTICS,
    VIDEO_H264_VUI_PARAM_FIELD_MATRIX_COEFFICIENTS,
    VIDEO_H264_VUI_PARAM_FIELD_CHROMA_LOC_TOP,
    VIDEO_H264_VUI_PARAM_FIELD_CHROMA_LOC_BOTTOM,
    VIDEO_H264_VUI_PARAM_FIELD_NUM_UNITS_IN_TICK,
    VIDEO_H264_VUI_PARAM_FIELD_TIME_SCALE,
    VIDEO_H264_VUI_PARAM_FIELD_FIXED_FRAME_RATE,
    VIDEO_H264_VUI_PARAM_FIELD_LOW_DELAY_HRD,
    VIDEO_H264_VUI_PARAM_FIELD_PIC_STRUCT,
    VIDEO_H264_VUI_PARAM_FIELD_MOTION_VECTORS_OVER_PIC_BOUNDARIES,
    VIDEO_H264_VUI_PARAM_FIELD_MAX_BYTES_PER_PIC_DENOM,
    VIDEO_H264_VUI_PARAM_FIELD_MAX_BITS_PER_MB_DENOM,
    VIDEO_H264_VUI_PARAM_FIELD_MAX_MV_LENGTH_H,
    VIDEO_H264_VUI_PARAM_FIELD_MAX_MV_LENGTH_V,
    VIDEO_H264_VUI_PARAM_FIELD_MAX_NUM_REORDER_FRAMES,
    VIDEO_H264_VUI_PARAM_FIELD_MAX_DEC_FRAME_BUFFERING,
    VIDEO_H264_VUI_PARAM_FIELD_CNT,
} VIDEOH264FIELDVUIPARAM;

typedef enum {
    VIDEO_H264_MB_PART_PRED_MODE_Intra_4x4,
    VIDEO_H264_MB_PART_PRED_MODE_Intra_8x8,
    VIDEO_H264_MB_PART_PRED_MODE_Intra_16x16,
    VIDEO_H264_MB_PART_PRED_MODE_Pred_L0,
    VIDEO_H264_MB_PART_PRED_MODE_Pred_L1,
    VIDEO_H264_MB_PART_PRED_MODE_BiPred,
    VIDEO_H264_MB_PART_PRED_MODE_Direct,
    VIDEO_H264_MB_PART_PRED_MODE_NA,
} VIDEOH264MBPARTPREDMODE;

typedef enum {
    /* I */
    VIDEO_H264_MB_TYPE_I_NxN,
    VIDEO_H264_MB_TYPE_I_16x16_0_0_0,
    VIDEO_H264_MB_TYPE_I_16x16_1_0_0,
    VIDEO_H264_MB_TYPE_I_16x16_2_0_0,
    VIDEO_H264_MB_TYPE_I_16x16_3_0_0,
    VIDEO_H264_MB_TYPE_I_16x16_0_1_0,
    VIDEO_H264_MB_TYPE_I_16x16_1_1_0,
    VIDEO_H264_MB_TYPE_I_16x16_2_1_0,
    VIDEO_H264_MB_TYPE_I_16x16_3_1_0,
    VIDEO_H264_MB_TYPE_I_16x16_0_2_0,
    VIDEO_H264_MB_TYPE_I_16x16_1_2_0,
    VIDEO_H264_MB_TYPE_I_16x16_2_2_0,
    VIDEO_H264_MB_TYPE_I_16x16_3_2_0,
    VIDEO_H264_MB_TYPE_I_16x16_0_0_1,
    VIDEO_H264_MB_TYPE_I_16x16_1_0_1,
    VIDEO_H264_MB_TYPE_I_16x16_2_0_1,
    VIDEO_H264_MB_TYPE_I_16x16_3_0_1,
    VIDEO_H264_MB_TYPE_I_16x16_0_1_1,
    VIDEO_H264_MB_TYPE_I_16x16_1_1_1,
    VIDEO_H264_MB_TYPE_I_16x16_2_1_1,
    VIDEO_H264_MB_TYPE_I_16x16_3_1_1,
    VIDEO_H264_MB_TYPE_I_16x16_0_2_1,
    VIDEO_H264_MB_TYPE_I_16x16_1_2_1,
    VIDEO_H264_MB_TYPE_I_16x16_2_2_1,
    VIDEO_H264_MB_TYPE_I_16x16_3_2_1,
    VIDEO_H264_MB_TYPE_I_PCM,
    /* SI */
    VIDEO_H264_MB_TYPE_SI,
    /* P */
    VIDEO_H264_MB_TYPE_P_L0_16x16,
    VIDEO_H264_MB_TYPE_P_L0_L0_16x8,
    VIDEO_H264_MB_TYPE_P_L0_L0_8x16,
    VIDEO_H264_MB_TYPE_P_8x8,
    VIDEO_H264_MB_TYPE_P_8x8ref0,
    VIDEO_H264_MB_TYPE_P_Skip,
    /* B */
    VIDEO_H264_MB_TYPE_B_Direct_16x16,
    VIDEO_H264_MB_TYPE_B_L0_16x16,
    VIDEO_H264_MB_TYPE_B_L1_16x16,
    VIDEO_H264_MB_TYPE_B_Bi_16x16,
    VIDEO_H264_MB_TYPE_B_L0_L0_16x8,
    VIDEO_H264_MB_TYPE_B_L0_L0_8x16,
    VIDEO_H264_MB_TYPE_B_L1_L1_16x8,
    VIDEO_H264_MB_TYPE_B_L1_L1_8x16,
    VIDEO_H264_MB_TYPE_B_L0_L1_16x8,
    VIDEO_H264_MB_TYPE_B_L0_L1_8x16,
    VIDEO_H264_MB_TYPE_B_L1_L0_16x8,
    VIDEO_H264_MB_TYPE_B_L1_L0_8x16,
    VIDEO_H264_MB_TYPE_B_L0_Bi_16x8,
    VIDEO_H264_MB_TYPE_B_L0_Bi_8x16,
    VIDEO_H264_MB_TYPE_B_L1_Bi_16x8,
    VIDEO_H264_MB_TYPE_B_L1_Bi_8x16,
    VIDEO_H264_MB_TYPE_B_Bi_L0_16x8,
    VIDEO_H264_MB_TYPE_B_Bi_L0_8x16,
    VIDEO_H264_MB_TYPE_B_Bi_L1_16x8,
    VIDEO_H264_MB_TYPE_B_Bi_L1_8x16,
    VIDEO_H264_MB_TYPE_B_Bi_Bi_16x8,
    VIDEO_H264_MB_TYPE_B_Bi_Bi_8x16,
    VIDEO_H264_MB_TYPE_B_8x8,
    VIDEO_H264_MB_TYPE_B_Skip,
    VIDEO_H264_MB_TYPE_CNT,
} VIDEOH264MBTYPE;
typedef enum {
    VIDEO_H264_SUB_MB_TYPE_P_L0_8x8,
    VIDEO_H264_SUB_MB_TYPE_P_L0_8x4,
    VIDEO_H264_SUB_MB_TYPE_P_L0_4x8,
    VIDEO_H264_SUB_MB_TYPE_P_L0_4x4,
    VIDEO_H264_SUB_MB_TYPE_B_Direct_8x8,
    VIDEO_H264_SUB_MB_TYPE_B_L0_8x8,
    VIDEO_H264_SUB_MB_TYPE_B_L1_8x8,
    VIDEO_H264_SUB_MB_TYPE_B_Bi_8x8,
    VIDEO_H264_SUB_MB_TYPE_B_L0_8x4,
    VIDEO_H264_SUB_MB_TYPE_B_L0_4x8,
    VIDEO_H264_SUB_MB_TYPE_B_L1_8x4,
    VIDEO_H264_SUB_MB_TYPE_B_L1_4x8,
    VIDEO_H264_SUB_MB_TYPE_B_Bi_8x4,
    VIDEO_H264_SUB_MB_TYPE_B_Bi_4x8,
    VIDEO_H264_SUB_MB_TYPE_B_L0_4x4,
    VIDEO_H264_SUB_MB_TYPE_B_L1_4x4,
    VIDEO_H264_SUB_MB_TYPE_B_Bi_4x4,
    VIDEO_H264_SUB_MB_TYPE_CNT,
} VIDEOH264SUBMBTYPE;

typedef uint32_t VIDEOH264SLICEGROUPMAP, * PVIDEOH264SLICEGROUPMAP;

#if 1 /* nal_unit */
typedef struct tagVIDEOH264_nal_unit {
    uint8_t                                     nal_ref_idc;
    uint8_t                                     nal_unit_type;
} VIDEOH264_nal_unit, * PVIDEOH264_nal_unit;
#endif

#if 1 /* SPS */
typedef struct tagVIDEOH264PARAMETER_hrd_parameters {
    uint32_t                                    cpb_cnt_minus1;
    uint8_t                                     bit_rate_scale;
    uint8_t                                     cpb_size_scale;
    #if 1 /* CPB */
        uint32_t                                bit_rate_value_minus1[VIDEO_H264_MAX_VALUE_cpb_cnt_minus1 + 1];
        uint32_t                                cpb_size_value_minus1[VIDEO_H264_MAX_VALUE_cpb_cnt_minus1 + 1];
        uint8_t                                 cbr_flag[VIDEO_H264_MAX_VALUE_cpb_cnt_minus1 + 1];
    #endif
    uint8_t                                     initial_cpb_removal_delay_length_minus1;
    uint8_t                                     cpb_removal_delay_length_minus1;
    uint8_t                                     dpb_output_delay_length_minus1;
    uint8_t                                     time_offset_length;
} VIDEOH264PARAMETER_hrd_parameters, * PVIDEOH264PARAMETER_hrd_parameters;
typedef const VIDEOH264PARAMETER_hrd_parameters* PCVIDEOH264PARAMETER_hrd_parameters;

typedef struct tagVIDEOH264PARAMETER_vui_parameters {
    uint8_t                                     aspect_ratio_info_present_flag;
    #if 1 /* aspect_ratio_info_present_flag */
        uint8_t                                 aspect_ratio_idc;
        #if 1 /* aspect_ratio_idc == Extended_SAR */
            uint16_t                            sar_width;
            uint16_t                            sar_height;
        #endif
    #endif
    uint8_t                                     overscan_info_present_flag;
    #if 1 /* overscan_info_present_flag */
        uint8_t                                 overscan_appropriate_flag;
    #endif
    uint8_t                                     video_signal_type_present_flag;
    #if 1 /* video_signal_type_present_flag */
        uint8_t                                 video_format;
        uint8_t                                 video_full_range_flag;
        uint8_t                                 colour_description_present_flag;
        #if 1 /* colour_description_present_flag */
            uint8_t                             colour_primaries;
            uint8_t                             transfer_characteristics;
            uint8_t                             matrix_coefficients;
        #endif
    #endif
    uint8_t                                     chroma_loc_info_present_flag;
    #if 1 /* chroma_loc_info_present_flag */
        uint32_t                                chroma_sample_loc_type_top_field;
        uint32_t                                chroma_sample_loc_type_bottom_field;
    #endif
    uint8_t                                     timing_info_present_flag;
    #if 1 /* timing_info_present_flag */
        uint32_t                                num_units_in_tick;
        uint32_t                                time_scale;
        uint8_t                                 fixed_frame_rate_flag;
    #endif
    uint8_t                                     nal_hrd_parameters_present_flag;
    #if 1 /* nal_hrd_parameters_present_flag */
    struct {
        VIDEOH264PARAMETER_hrd_parameters       hrd_parameters;
    } nal;
    #endif
    uint8_t                                     vcl_hrd_parameters_present_flag;
    #if 1 /* vcl_hrd_parameters_present_flag */
    struct {
        VIDEOH264PARAMETER_hrd_parameters       hrd_parameters;
    } vcl;
    #endif
    #if 1 /* nal_hrd_parameters_present_flag || vcl_hrd_parameters_present_flag */
        uint8_t                                 low_delay_hrd_flag;
    #endif
    uint8_t                                     pic_struct_present_flag;
    uint8_t                                     bitstream_restriction_flag;
    #if 1 /* bitstream_restriction_flag */
        uint8_t                                 motion_vectors_over_pic_boundaries_flag;
        uint32_t                                max_bytes_per_pic_denom;
        uint32_t                                max_bits_per_mb_denom;
        uint32_t                                log2_max_mv_length_horizontal;
        uint32_t                                log2_max_mv_length_vertical;
        uint32_t                                max_num_reorder_frames;
        uint32_t                                max_dec_frame_buffering;
    #endif
} VIDEOH264PARAMETER_vui_parameters, * PVIDEOH264PARAMETER_vui_parameters;
typedef const VIDEOH264PARAMETER_vui_parameters* PCVIDEOH264PARAMETER_vui_parameters;

typedef struct tagVIDEOH264SPS {
    struct { // 原始码流
        uint8_t                         nal_ref_idc;
        uint8_t                         nal_unit_type;
        uint8_t                         profile_idc;
        uint8_t                         constraint_set0_flag;
        uint8_t                         constraint_set1_flag;
        uint8_t                         constraint_set2_flag;
        uint8_t                         constraint_set3_flag;
        uint8_t                         constraint_set4_flag;
        uint8_t                         constraint_set5_flag;
        uint8_t                         reserved_zero_2bits;
        uint8_t                         level_idc;
        uint32_t                        seq_parameter_set_id;
        BOOL                            bExtendedProfileIdcData;
        uint32_t                        chroma_format_idc;
        uint8_t                         separate_colour_plane_flag;
        uint32_t                        bit_depth_luma_minus8;
        uint32_t                        bit_depth_chroma_minus8;
        uint8_t                         qpprime_y_zero_transform_bypass_flag;
        uint8_t                         seq_scaling_matrix_present_flag;
        /* 暂未实现 */
        uint32_t                        log2_max_frame_num_minus4;
        uint32_t                        pic_order_cnt_type;
        uint32_t                        log2_max_pic_order_cnt_lsb_minus4;
        uint8_t                         delta_pic_order_always_zero_flag;
        int32_t                         offset_for_non_ref_pic;
        int32_t                         offset_for_top_to_bottom_field;
        uint32_t                        num_ref_frames_in_pic_order_cnt_cycle;
        int32_t                         offset_for_ref_frame[VIDEO_H264_MAX_VALUE_num_ref_frames_in_pic_order_cnt_cycle];
        uint32_t                        max_num_ref_frames;
        uint8_t                         gaps_in_frame_num_value_allowed_flag;
        uint32_t                        pic_width_in_mbs_minus1;
        uint32_t                        pic_height_in_map_units_minus1;
        uint8_t                         frame_mbs_only_flag;
        uint8_t                         mb_adaptive_frame_field_flag;
        uint8_t                         direct_8x8_inference_flag;
        uint8_t                         frame_cropping_flag;
        uint32_t                        frame_crop_left_offset;
        uint32_t                        frame_crop_right_offset;
        uint32_t                        frame_crop_top_offset;
        uint32_t                        frame_crop_bottom_offset;
        uint8_t                         vui_parameters_present_flag;
        VIDEOH264PARAMETER_vui_parameters           vui_parameters;
    };
    struct { // 变量
        uint32_t                        SubWidthC;
        uint32_t                        SubHeightC;
        uint32_t                        MbWidthC;
        uint32_t                        MbHeightC;
        uint32_t                        ChromaArrayType;
        uint32_t                        BitDepthY;
        uint32_t                        QpBdOffsetY;
        uint32_t                        BitDepthC;
        uint32_t                        QpBdOffsetC;
        uint32_t                        RawMbBits;
        uint32_t                        MaxFrameNum;
        uint32_t                        MaxPicOrderCntLsb;
        uint32_t                        PicWidthInMbs;
        uint32_t                        PicWidthInSamplesL;
        uint32_t                        PicWidthInSamplesC;
        uint32_t                        PicHeightInMapUnits;
        uint32_t                        PicSizeInMapUnits;
        uint32_t                        FrameHeightInMbs;
        uint32_t                        CropUnitX;
        uint32_t                        CropUnitY;
        PVIDEOH264SLICEGROUPMAP         mapUnitToSliceGroupMap;
        PVIDEOH264SLICEGROUPMAP         MbToSliceGroupMap;
    };
} VIDEOH264SPS, * PVIDEOH264SPS;
typedef const VIDEOH264SPS* PCVIDEOH264SPS;
#endif

#if 1 /* PPS */
typedef struct tagVIDEOH264PPS {
    struct { // 原始码流
        uint8_t                          nal_ref_idc;
        uint8_t                          nal_unit_type;
        uint32_t                         pic_parameter_set_id;
        uint32_t                         seq_parameter_set_id;
        uint8_t                          entropy_coding_mode_flag;
        uint8_t                          bottom_field_pic_order_in_frame_present_flag;
        uint32_t                         num_slice_groups_minus1;
        uint32_t                         slice_group_map_type;
        uint32_t                         run_length_minus1[VIDEO_H264_MAX_VALUE_num_slice_groups_minus1 + 1];
        uint32_t                         top_left[VIDEO_H264_MAX_VALUE_num_slice_groups_minus1];
        uint32_t                         bottom_right[VIDEO_H264_MAX_VALUE_num_slice_groups_minus1];
        uint8_t                          slice_group_change_direction_flag;
        uint32_t                         slice_group_change_rate_minus1;
        uint32_t                         pic_size_in_map_units_minus1;
        uint32_t*                        slice_group_id; // 由于 slice_group_id 数组的大小等于 PicSizeInMapUnits（大小由 SPS 决定），所以内存由调用方分配，传递指针
        uint32_t                         num_ref_idx_l0_default_active_minus1;
        uint32_t                         num_ref_idx_l1_default_active_minus1;
        uint8_t                          weighted_pred_flag;
        uint8_t                          weighted_bipred_idc;
        int32_t                          pic_init_qp_minus26;
        int32_t                          pic_init_qs_minus26;
        int32_t                          chroma_qp_index_offset;
        uint8_t                          deblocking_filter_control_present_flag;
        uint8_t                          constrained_intra_pred_flag;
        uint8_t                          redundant_pic_cnt_present_flag;
        BOOL                             bMoreRbspData;
        uint8_t                          transform_8x8_mode_flag;
        uint8_t                          pic_scaling_matrix_present_flag;
        uint8_t                          pic_scaling_list_present_flag[12];
        int32_t                          second_chroma_qp_index_offset;
    };
    struct { // 变量
        uint32_t                        SliceGroupChangeRate;
    };
} VIDEOH264PPS, * PVIDEOH264PPS;
typedef const VIDEOH264PPS* PCVIDEOH264PPS;
#endif

#if 1 /* macroblock */
typedef struct tagVIDEOH264MACROBLOCK {
    VIDEOH264MBTYPE                         type;
    uint8_t                                 NumMbPart;
    VIDEOH264MBPREDMODECAT                  mbPredModeCat;
    VIDEOH264MBPARTPREDMODE                 MbPartPredMode[VIDEO_H264_MACRO_BLOCK_PART_SIZE];
    uint8_t                                 Intra16x16PredMode;
    uint8_t                                 CodedBlockPatternLuma;
    uint8_t                                 CodedBlockPatternChroma;
    uint8_t                                 MbPartWidth;
    uint8_t                                 MbPartHeight;
    VIDEOH264SUBMBTYPE                      subType[VIDEO_H264_SUB_MACRO_BLOCK_PART_SIZE];
    uint8_t                                 NumSubMbPart[VIDEO_H264_SUB_MACRO_BLOCK_PART_SIZE];
    uint8_t                                 SubMbPartWidth[VIDEO_H264_SUB_MACRO_BLOCK_PART_SIZE];
    uint8_t                                 SubMbPartHeight[VIDEO_H264_SUB_MACRO_BLOCK_PART_SIZE];
    VIDEOH264MBPARTPREDMODE                 SubMbPredMode[VIDEO_H264_MACRO_BLOCK_PART_SIZE];
    /* slice 层的变量，但必须每个宏块都要有 */
    uint32_t                                mb_skip_run;
    uint32_t                                mb_skip_flag;
    uint32_t                                mb_field_decoding_flag;
    /* 宏块层的变量 */
    uint8_t                                 mb_type;
    uint16_t                                pcm_sample_luma[256];
    uint16_t                                pcm_sample_chroma[512];
    uint32_t                                transform_size_8x8_flag;
    uint32_t                                coded_block_pattern;
    int32_t                                 mb_qp_delta;
    /* mb_pred */
    union {
        struct {
            uint8_t                         prev_intra4x4_pred_mode_flag[16];
            uint8_t                         rem_intra4x4_pred_mode[16];
        };
        struct {
            uint8_t                         prev_intra8x8_pred_mode_flag[4];
            uint8_t                         rem_intra8x8_pred_mode[4];
        };
    };
    uint32_t                                ref_idx_l0[VIDEO_H264_MACRO_BLOCK_PART_SIZE];
    uint32_t                                ref_idx_l1[VIDEO_H264_MACRO_BLOCK_PART_SIZE];
    int32_t                                 mvd_l0[VIDEO_H264_MACRO_BLOCK_PART_SIZE][VIDEO_H264_SUB_MACRO_BLOCK_PART_SIZE][2];
    int32_t                                 mvd_l1[VIDEO_H264_MACRO_BLOCK_PART_SIZE][VIDEO_H264_SUB_MACRO_BLOCK_PART_SIZE][2];
    uint32_t                                intra_chroma_pred_mode;
    /* 子块 */
    uint8_t                                 sub_mb_type[VIDEO_H264_SUB_MACRO_BLOCK_PART_SIZE];
    /* residual_block */
    union {
        struct { // CAVLC
            uint8_t                         trailing_ones_sign_flag;
        };
        VIDEOH264RESIDUALBLOCKSECABAC       cabac;
    } residual_block_SE;
} VIDEOH264MACROBLOCK, * PVIDEOH264MACROBLOCK;
typedef const VIDEOH264MACROBLOCK* PCVIDEOH264MACROBLOCK;
#endif

#if 1 /* Slice */
typedef struct tagVIDEOH264PARAMETER_ref_pic_list {
    union {
        #if 1 /* slice_type % 5 != 2 && slice_type % 5 != 4 */
            uint8_t                                 ref_pic_list_modification_flag_l0;
        #endif
        #if 1 /* slice_type % 5 == 3 */
            uint8_t                                 ref_pic_list_modification_flag_l1;
        #endif
    };
        #if 1 /* ref_pic_list_modification_flag_l0 */
            #if 1 /* do */
                uint32_t                            modification_of_pic_nums_idc;
                #if 1 /* modification_of_pic_nums_idc == 0 || modification_of_pic_nums_idc == 1 */
                    uint8_t                         bValid_abs_diff_pic_num_minus1;
                    uint32_t                        abs_diff_pic_num_minus1;
                #endif
                #if 1 /* modification_of_pic_nums_idc == 2 */
                    uint8_t                         bValid_long_term_pic_num;
                    uint32_t                        long_term_pic_num;
                #endif
                #if 1 /* modification_of_pic_nums_idc == 4 || modification_of_pic_nums_idc == 5 */
                    uint8_t                         bValid_abs_diff_view_idx_minus1;
                    uint32_t                        abs_diff_view_idx_minus1;
                #endif
            #endif /* while(modification_of_pic_nums_idc != 3) */
        #endif
} VIDEOH264PARAMETER_ref_pic_list, * PVIDEOH264PARAMETER_ref_pic_list;
typedef const VIDEOH264PARAMETER_ref_pic_list* PCVIDEOH264PARAMETER_ref_pic_list;
typedef VIDEOH264PARAMETER_ref_pic_list     VIDEOH264PARAMETER_ref_pic_list_mvc;
typedef PVIDEOH264PARAMETER_ref_pic_list    PVIDEOH264PARAMETER_ref_pic_list_mvc;
typedef PCVIDEOH264PARAMETER_ref_pic_list   PCVIDEOH264PARAMETER_ref_pic_list_mvc;

typedef struct tagVIDEOH264PARAMETER_pred_weight_table {
    uint32_t                                    luma_log2_weight_denom;
    uint32_t                                    chroma_log2_weight_denom;
    uint8_t                                     luma_weight_l0_flag[VIDEO_H264_MAX_VALUE_num_ref_idx_lx_active_minus1 + 1];
    int32_t                                     luma_weight_l0[VIDEO_H264_MAX_VALUE_num_ref_idx_lx_active_minus1 + 1];
    int32_t                                     luma_offset_l0[VIDEO_H264_MAX_VALUE_num_ref_idx_lx_active_minus1 + 1];
    uint8_t                                     chroma_weight_l0_flag[VIDEO_H264_MAX_VALUE_num_ref_idx_lx_active_minus1 + 1];
    int32_t                                     chroma_weight_l0[VIDEO_H264_MAX_VALUE_num_ref_idx_lx_active_minus1 + 1][2];
    int32_t                                     chroma_offset_l0[VIDEO_H264_MAX_VALUE_num_ref_idx_lx_active_minus1 + 1][2];
    uint8_t                                     luma_weight_l1_flag[VIDEO_H264_MAX_VALUE_num_ref_idx_lx_active_minus1 + 1];
    int32_t                                     luma_weight_l1[VIDEO_H264_MAX_VALUE_num_ref_idx_lx_active_minus1 + 1];
    int32_t                                     luma_offset_l1[VIDEO_H264_MAX_VALUE_num_ref_idx_lx_active_minus1 + 1];
    uint8_t                                     chroma_weight_l1_flag[VIDEO_H264_MAX_VALUE_num_ref_idx_lx_active_minus1 + 1];
    int32_t                                     chroma_weight_l1[VIDEO_H264_MAX_VALUE_num_ref_idx_lx_active_minus1 + 1][2];
    int32_t                                     chroma_offset_l1[VIDEO_H264_MAX_VALUE_num_ref_idx_lx_active_minus1 + 1][2];
} VIDEOH264PARAMETER_pred_weight_table, * PVIDEOH264PARAMETER_pred_weight_table;
typedef const VIDEOH264PARAMETER_pred_weight_table* PCVIDEOH264PARAMETER_pred_weight_table;

typedef struct tagVIDEOH264PARAMETER_dec_ref_pic_marking {
    #if 1 /* IdrPicFlag */
        uint8_t                                     no_output_of_prior_pics_flag;
        uint8_t                                     long_term_reference_flag;
    #endif
    #if 1 /* else */
        uint8_t                                     adaptive_ref_pic_marking_mode_flag;
        #if 1 /* adaptive_ref_pic_marking_mode_flag */
            #if 1 /* do */
                uint32_t                            memory_management_control_operation;
                #if 1 /* memory_management_control_operation == 1 || memory_management_control_operation == 3 */
                    uint8_t                         bValid_difference_of_pic_nums_minus1;
                    uint32_t                        difference_of_pic_nums_minus1;
                #endif
                #if 1 /* memory_management_control_operation == 2 */
                    uint8_t                         bValid_long_term_pic_num;
                    uint32_t                        long_term_pic_num;
                #endif
                #if 1 /* memory_management_control_operation == 3 || memory_management_control_operation == 6 */
                    uint8_t                         bValid_long_term_frame_idx;
                    uint32_t                        long_term_frame_idx;
                #endif
                #if 1 /* memory_management_control_operation == 4 */
                    uint8_t                         bValid_max_long_term_frame_idx_plus1;
                    uint32_t                        max_long_term_frame_idx_plus1;
                #endif
            #endif /* while(memory_management_control_operation != 0) */
        #endif
    #endif
} VIDEOH264PARAMETER_dec_ref_pic_marking, * PVIDEOH264PARAMETER_dec_ref_pic_marking;
typedef const VIDEOH264PARAMETER_dec_ref_pic_marking* PCVIDEOH264PARAMETER_dec_ref_pic_marking;

typedef struct tagVIDEOH264PARAMETER_slice_header {
    VIDEOH264_nal_unit                          nalUnit;
    uint32_t                                    first_mb_in_slice;
    uint32_t                                    slice_type;
    uint32_t                                    pic_parameter_set_id;
    uint8_t                                     colour_plane_id;
    uint32_t                                    frame_num;
    uint8_t                                     field_pic_flag;
    uint8_t                                     bottom_field_flag;
    uint32_t                                    idr_pic_id;
    uint32_t                                    pic_order_cnt_lsb;  // 这个参数的长度等于 log2_max_pic_order_cnt_lsb_minus4 + 4，SPS 里有这个参数
    int32_t                                     delta_pic_order_cnt_bottom;
    int32_t                                     delta_pic_order_cnt[2];
    uint32_t                                    redundant_pic_cnt;
    uint8_t                                     direct_spatial_mv_pred_flag;
    uint8_t                                     num_ref_idx_active_override_flag;
    uint32_t                                    num_ref_idx_l0_active_minus1;
    uint32_t                                    num_ref_idx_l1_active_minus1;
    union {
        VIDEOH264PARAMETER_ref_pic_list_mvc     ref_pic_list_mvc;
        VIDEOH264PARAMETER_ref_pic_list         ref_pic_list;
    };
    VIDEOH264PARAMETER_pred_weight_table        pred_weight_table;
    VIDEOH264PARAMETER_dec_ref_pic_marking      dec_ref_pic_marking;
    uint32_t                                    cabac_init_idc;
    int32_t                                     slice_qp_delta;
    uint8_t                                     sp_for_switch_flag;
    int32_t                                     slice_qs_delta;
    uint32_t                                    disable_deblocking_filter_idc;
    int32_t                                     slice_alpha_c0_offset_div2;
    int32_t                                     slice_beta_offset_div2;
    uint32_t                                    slice_group_change_cycle;
} VIDEOH264PARAMETER_slice_header, * PVIDEOH264PARAMETER_slice_header;
typedef const VIDEOH264PARAMETER_slice_header* PCVIDEOH264PARAMETER_slice_header;

typedef struct tagVIDEOH264PARAMETER_slice_data {
    struct {
        uint8_t                             nLen;
    } cabac_alignment_one_bit;
    uint32_t                                    end_of_slice_flag;
} VIDEOH264PARAMETER_slice_data, * PVIDEOH264PARAMETER_slice_data;
typedef const VIDEOH264PARAMETER_slice_data* PCVIDEOH264PARAMETER_slice_data;

typedef struct tagVIDEOH264SLICE {
    struct { // 指向使用的参数集
        PCVIDEOH264SPS                          pSPS;
        PCVIDEOH264PPS                          pPPS;
    };
    struct { // 原始码流
        VIDEOH264PARAMETER_slice_header         slice_header;
        VIDEOH264PARAMETER_slice_data           slice_data;
    };
    struct { // 变量
        VIDEOH264SLICETYPE                      type;
        uint8_t                                 MbaffFrameFlag;
        uint32_t                                PicHeightInMbs;
        uint32_t                                PicHeightInSamplesL;
        uint32_t                                PicHeightInSamplesC;
        uint32_t                                PicSizeInMbs;
        uint32_t                                MaxPicNum;
        uint32_t                                CurrPicNum;
        int32_t                                 SliceQPY;
        int32_t                                 QSY;
        int32_t                                 FilterOffsetA;
        int32_t                                 FilterOffsetB;
        uint32_t                                MapUnitsInSliceGroup0;
        uint32_t*                               mapUnitToSliceGroupMap;
        uint32_t*                               MbToSliceGroupMap;
    };
} VIDEOH264SLICE, * PVIDEOH264SLICE;
typedef const VIDEOH264SLICE* PCVIDEOH264SLICE;
#endif

typedef struct tagVIDEOH264PARAMLIST {
    struct { /* SPS 列表，由调用者提供 */
        HANSIZE                                 nCnt;
        PCVIDEOH264SPS                          pSPS;
    } spsList;
    struct { /* PPS 列表，由调用者提供 */
        HANSIZE                                 nCnt;
        PCVIDEOH264PPS                          pPPS;
    } ppsList;
} VIDEOH264PARAMLIST, * PVIDEOH264PARAMLIST;

typedef struct tagVIDEOH264PICTUREINFO {
    PVIDEOH264MACROBLOCK                    pMacroBlock;
    VIDEOH264SLICE                          slice;
} VIDEOH264PICTUREINFO, * PVIDEOH264PICTUREINFO;

size_t GetmapUnitToSliceGroupMapSize(PCVIDEOH264SPS pSPS);
size_t GetMbToSliceGroupMapSize(PCVIDEOH264SPS pSPS);

VIDEOH264PROFILETYPE GetSPSProfileType(PCVIDEOH264SPS pSPS);

void GetNalUnit(uint8_t pData, PVIDEOH264_nal_unit pNALU);
uint8_t GetIdrPicFlag(uint8_t nal_unit_type);

uint32_t DecodeH264Parameter_seq_parameter_set(const uint8_t* pData, uint32_t nSize, PVIDEOH264SPS pSPS);
bool SPSProfileIdcHasExtendedData(uint8_t nProfileIdc);

uint32_t DecodeH264Parameter_pic_parameter_set(const uint8_t* pData, uint32_t nSize, PVIDEOH264PPS pPPS, uint32_t* slice_group_id);

uint32_t DecodeH264Parameter_slice_header(const uint8_t* pData, uint32_t nSize, PVIDEOH264SLICE pSlice, PVIDEOH264PARAMLIST pParamList);
VIDEOH264SLICETYPE DecodeH264NormalizeSliceType(VIDEOH264SLICETYPE eType);

bool H264DecodeBy_slice_layer_without_partitioning(PVIDEOH264_nal_unit pNALU);
uint32_t DecodeH264_slice_layer_without_partitioning(const uint8_t* pData, uint32_t nSize, PVIDEOH264PICTUREINFO pPicInfo, PVIDEOH264PARAMLIST pParamList);

uint32_t DecodeH264_sei(const uint8_t* pData, uint32_t nSize);

bool DecodeH264BinToVariableSPS(PVIDEOH264SPS pSPS);
bool DecodeH264BinToVariablePPS(PVIDEOH264PPS pPPS);
bool DecodeH264BinToVariableSliceHeader(PVIDEOH264SLICE pSlice);

#ifdef __cplusplus
}
#endif

#endif
