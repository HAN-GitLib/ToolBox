#ifndef HAN_PICTURE_PNG_DEF_H
#define HAN_PICTURE_PNG_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <Windows.h>

#include "..\..\HAN_PictureDef.h"

#define HAN_PICTURE_PNG_TEXT_BUF_SIZE               1024

#define PICTURE_PNG_INFO_HEIGHT                     PICTURE_INFO_HEIGHT

#define PICTURE_PNG_ALPHA_BACKGROUND_W0             (7)
#define PICTURE_PNG_ALPHA_BACKGROUND_W2             (1)
#define PICTURE_PNG_ALPHA_BACKGROUND_W              (PICTURE_PNG_ALPHA_BACKGROUND_W0 + PICTURE_PNG_ALPHA_BACKGROUND_W2)

#define PICTURE_PNG_IDAT_MAX_SIZE                   0xFFFFFFFF

#ifdef __cplusplus
}
#endif

#endif
