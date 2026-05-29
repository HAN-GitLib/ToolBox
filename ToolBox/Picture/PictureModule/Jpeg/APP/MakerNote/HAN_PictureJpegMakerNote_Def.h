#ifndef HAN_PICTURE_JPEG_MAKER_NOTE_DEF_H
#define HAN_PICTURE_JPEG_MAKER_NOTE_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\IFD\HAN_PictureJpegIFD.h"

typedef struct tagPICTUREJPEGAPP1EXIFMAKERNOTEPRINTTITLEPARAM {
    HWND                        hInfo;
    HANPCSTR                    pMaker;
    HANPCSTR                    pTag;
} PICTUREJPEGAPP1EXIFMAKERNOTEPRINTTITLEPARAM, * PPICTUREJPEGAPP1EXIFMAKERNOTEPRINTTITLEPARAM;

#ifdef __cplusplus
}
#endif

#endif
