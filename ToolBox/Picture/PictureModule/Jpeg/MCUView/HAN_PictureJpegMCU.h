#ifndef HAN_PICTURE_JPEG_MCU_H
#define HAN_PICTURE_JPEG_MCU_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <Windows.h>

#include "..\..\..\HAN_PictureDef.h"
#include "..\HAN_PictureJpegDef.h"
#include "..\HAN_PictureJpegSegment.h"
#include "..\HAN_PictureJpegMath.h"

#define HAN_PICTURE_JPEG_MCU_CLASS                  TEXT("HAN_PictureJpegMCU")

typedef struct tagPICTUREJPEGMCUPOS { // 标记每个 Block 的起始字节，起始比特，比特个数
    BOOL                        bValid;
    HANSIZE                     nByteStart;
    uint8_t                     nBitStart;
    HANSIZE                     nBitCnt;
} PICTUREJPEGMCUPOS, * PPICTUREJPEGBLOCKPOS;
typedef struct tagPICTUREJPEGMCUINFO { // 标记每个分量 SOS 块对应的各个信息
    BOOL                        bValid;
    uint8_t                     nComponent;
    uint8_t                     nSs;
    uint8_t                     nSe;
    uint8_t                     nAh;
    uint8_t                     nAl;
    PPICTUREJPEGBLOCKPOS        pBlockPos;
    HANSIZE                     nBlockCnt;
} PICTUREJPEGMCUINFO, * PPICTUREJPEGMCUINFO;
typedef struct tagPICTUREJPEGMCUTABLE { // 指向一个分量的所有 MCU 信息的表格
    PPICTUREJPEGMCUINFO         pInfo;
    HANSIZE                     nCnt;
} PICTUREJPEGMCUTABLE, * PPICTUREJPEGMCUTABLE;

typedef struct tagPICTUREJPEGMCUCREATEPARAM {
    HWND*                       pWindow;
    HANPPICTURE                 pPicture;
    PPICTUREJPEGSEGMENTINFO     pSegmentInfo;
    PICTUREJPEGDCTMATRIX2D*     pComponent;
    PICTUREJPEGDCTMATRIX2D*     pDCT;
    PPICTUREJPEGMCUTABLE        pMCUTable;
    // 量化数据不单独提供，在解码的时候 IDCT 直接覆盖到量化值上去了，所以在显示量化结果的时候直接用 pDCT 计算
} PICTUREJPEGMCUCREATEPARAM, * PPICTUREJPEGMCUCREATEPARAM;

void RegisterHANPictureJpegMCU(HINSTANCE hInst);

#ifdef __cplusplus
}
#endif

#endif
