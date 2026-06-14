#ifndef HAN_VIDEO_H264_LIB_H
#define HAN_VIDEO_H264_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "HAN_VideoH264Def.h"

uint32_t DecodeH264Parameter_seq_parameter_set(const uint8_t* pData, uint32_t nSize, PVIDEOH264PARAMETER_seq_parameter_set pSPS);
BOOL SPSProfileIdcHasExtendedData(uint8_t nProfileIdc);

uint32_t DecodeH264Parameter_pic_parameter_set(const uint8_t* pData, uint32_t nSize, PVIDEOH264PARAMETER_pic_parameter_set pPPS, uint32_t* slice_group_id);



BOOL DecodeH264ParameterDataToInfo_SPS(PVIDEOH264INFOSPS pInfo, PCVIDEOH264PARAMETER_seq_parameter_set pData);

#ifdef __cplusplus
}
#endif

#endif
