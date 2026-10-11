#ifndef HAN_VIDEO_H264_DEF_H
#define HAN_VIDEO_H264_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "..\..\HAN_VideoDef.h"
#include "HAN_VideoH264.h"
#include "HAN_VideoH264ReadBit.h"
#include "HAN_VideoH264Def.h"
#include "HAN_VideoH264CABACDef.h"

#if 1 /* 解码器 */
typedef int32_t VIDEOH264COEFFLEVEL, * PVIDEOH264COEFFLEVEL;
typedef VIDEOH264COEFFLEVEL VIDEOH264COEFF16x16DCLEVEL[16];
typedef VIDEOH264COEFFLEVEL VIDEOH264COEFF16x16ACLEVEL[16][15];
typedef VIDEOH264COEFFLEVEL VIDEOH264COEFFLEVEL4x4[16][16];
typedef VIDEOH264COEFFLEVEL VIDEOH264COEFFLEVEL8x8[4][64];

typedef struct tagVIDEOH264COEFFLEVELBLOCK {
    VIDEOH264COEFF16x16DCLEVEL      i16x16DClevel;
    VIDEOH264COEFF16x16ACLEVEL      i16x16AClevel;
    VIDEOH264COEFFLEVEL4x4          level4x4;
    VIDEOH264COEFFLEVEL8x8          level8x8;
} VIDEOH264COEFFLEVELBLOCK, * PVIDEOH264COEFFLEVELBLOCK;

typedef struct tagVIDEOH264RESIDUALBLOCKPARAM { /* 除了 coeffLevel, startIdx, endIdx, maxNumCoeff 这几个必备的参数外，CABAC 还需要块 ID、分量 ID 等参数 */
    VIDEOH264BLOCKCAT               blockCat;               // 当前正在解码的块类型
    PVIDEOH264COEFFLEVEL            coeffLevel;
    int32_t                         startIdx;
    int32_t                         endIdx;
    uint8_t                         maxNumCoeff;
    uint8_t                         iCbCr;
    uint8_t                         i8x8;
    uint8_t                         i4x4;
} VIDEOH264RESIDUALBLOCKPARAM;
typedef const VIDEOH264RESIDUALBLOCKPARAM* PCVIDEOH264RESIDUALBLOCKPARAM;

typedef struct tagVIDEOH264DECODEINFO {
    PVIDEOH264READBIT               pReadBit;               // 指向与解码无关的二进制读取器，读取器仅从码流中读取比特
    PVIDEOH264PICTUREINFO           pPicInfo;               // 图片信息，存放与视频图片相关的参数
    /* 宏块层级参数 */
    HANSIZE                         CurrMbAddr;             // 当前正在解码的宏块地址
    uint8_t                         mbPartIdx;              // 当前正在解码的分块 ID
    uint8_t                         subMbPartIdx;           // 当前正在解码的子块 ID
    /* residual 参数 */
    uint8_t                         NumC8x8;
    VIDEOH264BLOCKCAT               blockCat;               // 当前正在解码的 residual 块类型
    PVIDEOH264COEFFLEVEL            coeffLevel;
    int32_t                         startIdx;
    int32_t                         endIdx;
    uint8_t                         maxNumCoeff;
    uint8_t                         iCbCr;
    uint8_t                         i8x8;
    uint8_t                         i4x4;
    uint8_t                         levelListIdx;
    /* residual block 系数 */
    VIDEOH264COEFFLEVELBLOCK        Luma;
    union{
        struct { // ChromaArrayType == 1 或 2
            VIDEOH264COEFF16x16DCLEVEL      ChromaDCLevel[2];
            VIDEOH264COEFF16x16ACLEVEL      ChromaACLevel[2];
        };
        struct { // ChromaArrayType == 3
            VIDEOH264COEFFLEVELBLOCK        Cb;
            VIDEOH264COEFFLEVELBLOCK        Cr;
        };
    };
    /* 读码器的专用参数（仅限不涉及标准第 7 章的变量），考虑到不同读码方式需要不同的参数，且会不共存，所以用 union 给它们预留空位 */
    union {
        PVIDEOH264READAE            pCABAC;
    };
} VIDEOH264DECODEINFO, * PVIDEOH264DECODEINFO;

typedef struct tagVIDEOH264READSECALLBACK {
    /* slice_data */
    VIDEOH264VALUE                  (*mb_skip_run)(PVIDEOH264DECODEINFO pDecodeInfo);
    VIDEOH264VALUE                  (*mb_skip_flag)(PVIDEOH264DECODEINFO pDecodeInfo);
    VIDEOH264VALUE                  (*mb_field_decoding_flag)(PVIDEOH264DECODEINFO pDecodeInfo);
    VIDEOH264VALUE                  (*end_of_slice_flag)(PVIDEOH264DECODEINFO pDecodeInfo);
    /* 宏块 */
    VIDEOH264VALUE                  (*mb_type)(PVIDEOH264DECODEINFO pDecodeInfo);
    VIDEOH264VALUE                  (*pcm_sample_luma)(PVIDEOH264DECODEINFO pDecodeInfo);
    VIDEOH264VALUE                  (*pcm_sample_chroma)(PVIDEOH264DECODEINFO pDecodeInfo);
    VIDEOH264VALUE                  (*transform_size_8x8_flag)(PVIDEOH264DECODEINFO pDecodeInfo);
    VIDEOH264VALUE                  (*coded_block_pattern)(PVIDEOH264DECODEINFO pDecodeInfo);
    VIDEOH264VALUE                  (*mb_qp_delta)(PVIDEOH264DECODEINFO pDecodeInfo);
    /* mb_pred */
    VIDEOH264VALUE                  (*prev_intraNxN_pred_mode_flag)(PVIDEOH264DECODEINFO pDecodeInfo);
    VIDEOH264VALUE                  (*rem_intraNxN_pred_mode)(PVIDEOH264DECODEINFO pDecodeInfo);
    VIDEOH264VALUE                  (*intra_chroma_pred_mode)(PVIDEOH264DECODEINFO pDecodeInfo);
    VIDEOH264VALUE                  (*ref_idx_l0)(PVIDEOH264DECODEINFO pDecodeInfo);
    VIDEOH264VALUE                  (*ref_idx_l1)(PVIDEOH264DECODEINFO pDecodeInfo);
    VIDEOH264VALUE                  (*mvd_l0[2])(PVIDEOH264DECODEINFO pDecodeInfo);
    VIDEOH264VALUE                  (*mvd_l1[2])(PVIDEOH264DECODEINFO pDecodeInfo);
    /* sub_mb_pred */
    VIDEOH264VALUE                  (*sub_mb_type)(PVIDEOH264DECODEINFO pDecodeInfo);
    /* residual_block 的语法元素读取封装在各文件的 residual_block 函数里，主程序不关心 */
} VIDEOH264READSECALLBACK, * PVIDEOH264READSECALLBACK;
typedef const VIDEOH264READSECALLBACK* PCVIDEOH264READSECALLBACK;

typedef void (*RESIDUALBLOCKHANDLER)(PVIDEOH264DECODEINFO pDecodeInfo); // residual block 的参数在 PVIDEOH264DECODEINFO 结构体的 residualBlockParam 中

typedef struct tagVIDEOH264DECODER {
    VIDEOH264DECODEINFO             decodeInfo;
    VIDEOH264READSECALLBACK         readSE;
    RESIDUALBLOCKHANDLER            residual_block;
} VIDEOH264DECODER, * PVIDEOH264DECODER;

/* 读完 mb_type 后调用 */
void DecodeH264InitMacroBlock(PVIDEOH264DECODEINFO pDecodeInfo, uint32_t mb_type);

/* 读完所有 4 个 sub_mb_type 后调用 */
void DecodeH264InitSubMacroBlock(PVIDEOH264DECODEINFO pDecodeInfo);

#endif

#ifdef __cplusplus
}
#endif

#endif
