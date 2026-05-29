#ifndef HAN_PICTURE_JPEG_DEF_H
#define HAN_PICTURE_JPEG_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <Windows.h>

#include "..\..\HAN_PictureDef.h"

#define HAN_PICTURE_JPEG_TEXT_BUF_SIZE                              1024

#define PICTURE_JPEG_BACKGROUND_COLOR                               RGB(0xF0, 0xF0, 0xF0)

#define PICTURE_JPEG_INFO_HEIGHT                                    PICTURE_INFO_HEIGHT

#define PICTURE_JPEG_DECODE_PROCEDURE_MODE_WIDTH_THRESHOLD          (500)

#define PICTURE_JPEG_MCU_VIEW_COMPONENT_MAX                         10

#define PICTURE_JPEG_MCU_VIEW_MCU_ID_INPUT_LEFT_TEXT_WIDTH          20
#define PICTURE_JPEG_MCU_VIEW_MCU_ID_INPUT_INPUT_WIDTH              100
#define PICTURE_JPEG_MCU_VIEW_MCU_ID_INPUT_RIGHT_TEXT_WIDTH         100
#define PICTURE_JPEG_MCU_VIEW_MCU_ID_INPUT_WIDTH                    ( PICTURE_JPEG_MCU_VIEW_MCU_ID_INPUT_LEFT_TEXT_WIDTH \
                                                                    + PICTURE_JPEG_MCU_VIEW_MCU_ID_INPUT_INPUT_WIDTH \
                                                                    + PICTURE_JPEG_MCU_VIEW_MCU_ID_INPUT_RIGHT_TEXT_WIDTH)
#define PICTURE_JPEG_MCU_VIEW_MCU_ID_INPUT_HEIGHT                   22

#define PICTURE_JPEG_MCU_VIEW_CELL_WIDTH                            50
#define PICTURE_JPEG_MCU_VIEW_CELL_HEIGHT                           18

#define PICTURE_JPEG_MCU_VIEW_SOS_LIST_WIDTH                        200
#define PICTURE_JPEG_MCU_VIEW_SOS_LIST_TEXT_HEIGHT                  22

#define PICTURE_JPEG_MCU_VIEW_POS_HEIGHT                            60

#define PICTURE_JPEG_SAVE_ENCODE_BUF_SIZE                           65540   // 包含标签（2字节）+ 长度（2字节）+ 数据（最长 65536 字节）

#define PICTURE_JPEG_SAVE_PARAM_CHOOSE_BUTTON_WIDTH                 120
#define PICTURE_JPEG_SAVE_PARAM_CHOOSE_BUTTON_HEIGHT                20

#define PICTURE_JPEG_SAVE_PARAM_RADIO_BUTTON_WIDTH                  120
#define PICTURE_JPEG_SAVE_PARAM_RADIO_BUTTON_HEIGHT                 20

typedef enum {
    PICTURE_JPEG_SEGMENT_INFO_HEADER_FIELD,
    PICTURE_JPEG_SEGMENT_INFO_HEADER_VALUE,
    PICTURE_JPEG_SEGMENT_INFO_HEADER_CNT,
} PICTUREJPEGSEGMENTINFOHEADER;

static inline uint16_t ReadJpegData2ByteMSB(const uint8_t pData[2])
{
    return (((uint16_t)pData[0] << 8) + (uint16_t)pData[1]);
}
static inline uint32_t ReadJpegData4ByteMSB(const uint8_t pData[4])
{
    return (((uint32_t)pData[0] << 24) + ((uint32_t)pData[1] << 16) + ((uint32_t)pData[2] << 8) + (uint32_t)pData[3]);
}

static inline void WriteJpegData2ByteMSB(uint8_t pData[2], uint16_t nValue)
{
    pData[0] = (uint8_t)(nValue >> 8);
    pData[1] = (uint8_t)nValue;
}

#ifdef __cplusplus
}
#endif

#endif
