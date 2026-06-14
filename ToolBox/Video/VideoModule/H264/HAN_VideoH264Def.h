#ifndef HAN_VIDEO_H264_DEF_H
#define HAN_VIDEO_H264_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "..\..\HAN_VideoDef.h"

#define VIDEO_H264_MAX_VALUE_num_ref_frames_in_pic_order_cnt_cycle          255
#define VIDEO_H264_MAX_VALUE_cpb_cnt_minus1                                 31
#define VIDEO_H264_MAX_VALUE_num_slice_groups_minus1                        7
#define VIDEO_H264_EXTENDED_SAR                                             255

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

#if 1 /* SPS 原始数据结构体 */
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

typedef struct tagVIDEOH264PARAMETER_seq_parameter_set_data {
    uint8_t                                     nal_ref_idc;
    uint8_t                                     nal_unit_type;
    uint8_t                                     profile_idc;
    uint8_t                                     constraint_set0_flag;
    uint8_t                                     constraint_set1_flag;
    uint8_t                                     constraint_set2_flag;
    uint8_t                                     constraint_set3_flag;
    uint8_t                                     constraint_set4_flag;
    uint8_t                                     constraint_set5_flag;
    uint8_t                                     reserved_zero_2bits;
    uint8_t                                     level_idc;
    uint32_t                                    seq_parameter_set_id;
    #if 1 /* profile_idc 等于一些高画质时的数据 */
        BOOL                                    bExtendedProfileIdcData;
        uint32_t                                chroma_format_idc;
        #if 1 /* chroma_format_idc == 3 */
            uint8_t                             separate_colour_plane_flag;
        #endif
        uint32_t                                bit_depth_luma_minus8;
        uint32_t                                bit_depth_chroma_minus8;
        uint8_t                                 qpprime_y_zero_transform_bypass_flag;
        uint8_t                                 seq_scaling_matrix_present_flag;
        #if 1 /* seq_scaling_matrix_present_flag */
            /* 暂未实现 */
        #endif
    #endif
    uint32_t                                    log2_max_frame_num_minus4;
    uint32_t                                    pic_order_cnt_type;
    #if 1 /* pic_order_cnt_type == 0 */
        uint32_t                                log2_max_pic_order_cnt_lsb_minus4;
    #endif
    #if 1 /* pic_order_cnt_type == 1 */
        uint8_t                                 delta_pic_order_always_zero_flag;
        int32_t                                 offset_for_non_ref_pic;
        int32_t                                 offset_for_top_to_bottom_field;
        uint32_t                                num_ref_frames_in_pic_order_cnt_cycle;
        int32_t                                 offset_for_ref_frame[VIDEO_H264_MAX_VALUE_num_ref_frames_in_pic_order_cnt_cycle];
    #endif
    uint32_t                                    max_num_ref_frames;
    uint8_t                                     gaps_in_frame_num_value_allowed_flag;
    uint32_t                                    pic_width_in_mbs_minus1;
    uint32_t                                    pic_height_in_map_units_minus1;
    uint8_t                                     frame_mbs_only_flag;
    #if 1 /* !frame_mbs_only_flag */
        uint8_t                                 mb_adaptive_frame_field_flag;
    #endif
    uint8_t                                     direct_8x8_inference_flag;
    uint8_t                                     frame_cropping_flag;
    #if 1 /* frame_cropping_flag */
        uint32_t                                frame_crop_left_offset;
        uint32_t                                frame_crop_right_offset;
        uint32_t                                frame_crop_top_offset;
        uint32_t                                frame_crop_bottom_offset;
    #endif
    uint8_t                                     vui_parameters_present_flag;
    #if 1 /* vui_parameters_present_flag */
        VIDEOH264PARAMETER_vui_parameters       vui_parameters;
    #endif
} VIDEOH264PARAMETER_seq_parameter_set, * PVIDEOH264PARAMETER_seq_parameter_set;
typedef const VIDEOH264PARAMETER_seq_parameter_set* PCVIDEOH264PARAMETER_seq_parameter_set;

typedef struct tagVIDEOH264PARAMETER_pic_parameter_set {
    uint8_t                                     nal_ref_idc;
    uint8_t                                     nal_unit_type;
    uint32_t                                    pic_parameter_set_id;
    uint32_t                                    seq_parameter_set_id;
    uint8_t                                     entropy_coding_mode_flag;
    uint8_t                                     bottom_field_pic_order_in_frame_present_flag;
    uint32_t                                    num_slice_groups_minus1;
    #if 1 /* num_slice_groups_minus1 > 0 */
        uint32_t                                slice_group_map_type;
        #if 1 /* slice_group_map_type == 0 */
            uint32_t                            run_length_minus1[VIDEO_H264_MAX_VALUE_num_slice_groups_minus1 + 1];
        #endif
        #if 1 /* slice_group_map_type == 2 */
            uint32_t                            top_left[VIDEO_H264_MAX_VALUE_num_slice_groups_minus1];
            uint32_t                            bottom_right[VIDEO_H264_MAX_VALUE_num_slice_groups_minus1];
        #endif
        #if 1 /* slice_group_map_type == 3 or 4 or 5 */
            uint8_t                             slice_group_change_direction_flag;
            uint32_t                            slice_group_change_rate_minus1;
        #endif
        #if 1 /* slice_group_map_type == 6 */
            uint32_t                            pic_size_in_map_units_minus1;
            uint32_t*                           slice_group_id; // 由于 slice_group_id 数组的大小是随画面大小而变的，在高清视频中这个数组会非常大，所以内存由调用方分配，传递指针
        #endif
    #endif
    uint32_t                                    num_ref_idx_l0_default_active_minus1;
    uint32_t                                    num_ref_idx_l1_default_active_minus1;
    uint8_t                                     weighted_pred_flag;
    uint8_t                                     weighted_bipred_idc;
    int32_t                                     pic_init_qp_minus26;
    int32_t                                     pic_init_qs_minus26;
    int32_t                                     chroma_qp_index_offset;
    uint8_t                                     deblocking_filter_control_present_flag;
    uint8_t                                     constrained_intra_pred_flag;
    uint8_t                                     redundant_pic_cnt_present_flag;
    #if 1 /* more_rbsp_data() */
        BOOL                                    bMoreRbspData;
        uint8_t                                 transform_8x8_mode_flag;
        uint8_t                                 pic_scaling_matrix_present_flag;
        #if 1 /* pic_scaling_matrix_present_flag */
            uint8_t                             pic_scaling_list_present_flag[12];
        #endif
        int32_t                                 second_chroma_qp_index_offset;
    #endif
} VIDEOH264PARAMETER_pic_parameter_set, * PVIDEOH264PARAMETER_pic_parameter_set;
typedef const VIDEOH264PARAMETER_pic_parameter_set* PCVIDEOH264PARAMETER_pic_parameter_set;
#endif

#if 1 /* SPS 转换后的信息 */
typedef struct tagVIDEOH264INFOHRDPARAM {
    BOOL                            bValid;
    uint8_t                         nCPBCnt;
    struct {
        uint32_t                    nBitRate;
        uint32_t                    nCPBSize;
        BOOL                        bCBR;
    } pCPB[32];
    uint8_t                         nInitialCPBRemovalDelayLen;
    uint8_t                         nCPBRemovalDelayLen;
    uint8_t                         nDPBRemovalDelayLen;
    uint8_t                         nTimeOffsetLen;
} VIDEOH264INFOHRDPARAM, * PVIDEOH264INFOHRDPARAM;

typedef struct tagVIDEOH264INFOVUIPARAM {
    BOOL                            bValid;
    struct {
        BOOL                        bValid;
        uint16_t                    nWidth;
        uint16_t                    nHeight;
    } aspectRatio;
    VIDEOH264OVERSCAN               eOverscan;
    struct {
        uint8_t                     eVideoFormat;
        BOOL                        bVideoFullRange;
        struct {
            uint8_t                 eColourPrimaries;
            uint8_t                 eTransferCharacteristics;
            uint8_t                 eMatrixCoefficients;
        } colourDescription;
    } videoSignalType;
    struct {
        uint8_t                     top;
        uint8_t                     bottom;
    } chromaLoc;
    struct {
        BOOL                        bValid;
        uint32_t                    nNumUnits;
        uint32_t                    nTimeScale;
        BOOL                        bFixedFrameRate;
    } timming;
    VIDEOH264INFOHRDPARAM           nal;
    VIDEOH264INFOHRDPARAM           vcl;
    BOOL                            bLowDelayHRD;
    struct {
        BOOL                        bValid;
        BOOL                        bMotionVectorsOverPicBoundaries;
        uint32_t                    nMaxBytesPerPicDenom;
        uint32_t                    nMaxBitsPerMbDenom;
        uint32_t                    nMaxMvLenH;
        uint32_t                    nMaxMvLenV;
        uint32_t                    nMaxNumReorderFrames;
        uint32_t                    nMaxDecFrameBuffering;
    } picStruct;
} VIDEOH264INFOVUIPARAM, * PVIDEOH264INFOVUIPARAM;

typedef struct tagVIDEOH264INFOSPS {
    struct {
        uint8_t                     refIdc;
        uint8_t                     naluType;
    } header;
    VIDEOH264PROFILETYPE            eProfile;
    uint8_t                         nProfileIdc;
    struct {
        uint8_t                     bSet0 : 1;
        uint8_t                     bSet1 : 1;
        uint8_t                     bSet2 : 1;
        uint8_t                     bSet3 : 1;
        uint8_t                     bSet4 : 1;
        uint8_t                     bSet5 : 1;
    } bConstraintSetFlags;
    uint8_t                         nLevelIdc;
    uint8_t                         nSPSId;
    uint8_t                         nChromaFormatIdc;
    BOOL                            bSeparateColourPlane;
    uint8_t                         nBitDepthY;
    uint8_t                         nQpBdOffsetY;
    uint8_t                         nBitDepthC;
    uint8_t                         nQpBdOffsetC;
    BOOL                            bTransformBypass;
    struct {
        BOOL                        bValid;
    } seqScalingMatrix;
    HANSIZE                         nMaxFrameNum;                       // log2_max_frame_num_minus4
    uint32_t                        nPOCType;                           // pic_order_cnt_type
    struct {
        HANSIZE                     nMaxPicOrderCntLsb;                 // log2_max_pic_order_cnt_lsb_minus4
    } POC0;
    struct {
        BOOL                        bDeltaPicOrderAlwaysZero;           // delta_pic_order_always_zero_flag
        int32_t                     nNonRefPicOffset;                   // offset_for_non_ref_pic
        int32_t                     nTopToBottomFieldOffset;            // offset_for_top_to_bottom_field
        uint8_t                     nRefFramesNum;                      // num_ref_frames_in_pic_order_cnt_cycle
        int32_t                     pRefFrameOffset[255];               // offset_for_ref_frame
    } POC1;
    uint32_t                        nMaxNumRefFrames;                   // max_num_ref_frames
    BOOL                            bGapsInFrameNumValueAllowedFlag;    // gaps_in_frame_num_value_allowed_flag
    uint32_t                        nWidth;                             // pic_width_in_mbs_minus1
    uint32_t                        nHeight;                            // pic_height_in_map_units_minus1
    uint8_t                         nFrameMacroBlockOnly;
    BOOL                            bMbAdaptiveFrameField;              // mb_adaptive_frame_field_flag
    BOOL                            bDirect8x8Inference;                // direct_8x8_inference_flag
    struct {
        uint32_t                    left;                               // frame_crop_left_offset
        uint32_t                    right;                              // frame_crop_right_offset
        uint32_t                    top;                                // frame_crop_top_offset
        uint32_t                    bottom;                             // frame_crop_bottom_offset
    } frameCrop;
    VIDEOH264INFOVUIPARAM           vuiParam;
    /* 上面的参数用来在界面显示，下面的参数解码用 */
    uint8_t                         nChromaArrayType;
    uint8_t                         nSubWidthC;
    uint8_t                         nSubHeightC;
    uint8_t                         nMbWidthC;
    uint8_t                         nMbHeightC;
    uint16_t                        nRawMbBits;
    HANINT                          nExpectedDeltaPerPicOrderCntCycle;
    uint32_t                        nPicWidthInMbs;
    uint32_t                        nPicWidthInSamplesL;
    uint32_t                        nPicWidthInSamplesC;
    uint32_t                        nPicHeightInMapUnits;
    HANSIZE                         nPicSizeInMapUnits;
    uint32_t                        nFrameHeightInMbs;
    uint8_t                         nCropUnitX;
    uint8_t                         nCropUnitY;
} VIDEOH264INFOSPS, * PVIDEOH264INFOSPS;
#endif

#ifdef __cplusplus
}
#endif

#endif
