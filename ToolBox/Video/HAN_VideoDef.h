#ifndef HAN_VIDEO_DEF_H
#define HAN_VIDEO_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\..\HAN_Lib\HAN_windows.h"
#include "..\GlobalVariables.h"

#define VIDEO_WINDOW_DY                 10
#define VIDEO_WINDOW_DX                 10

#define VIDEO_TEXT_BUF_SIZE             1024

#define VIDEO_TOOL_SIZE                 16

#define VIDEO_BACKGROUND_BRUSH          (NULL_BRUSH)

#define VIDEO_HEX_PRINT_LEN             10

typedef enum {
    WID_VIDEO_BASE = HAN_WID_BASE,
    WID_VIDEO_TOOLBAR,
    WID_VIDEO_TOOLBAR_OPEN_VIDEO,
    WID_VIDEO_TOOLBAR_PROCESS_FRAME,
    WID_VIDEO_VIDEO_WINDOW,
    /* MP4 */
    WID_VIDEO_MP4_BOX_TREE,
    WID_VIDEO_MP4_BOX_INFO,
} WIDVIDEO;

typedef struct tagVIDEOCREATEPARAM {
    HANCHAR             pFileName[PATH_STR_SIZE];       /* 文件名，带路径 */
    uint8_t*            pData;                          /* 文件数据，由父窗口管理，子窗口只能读取，不可修改 */
    HANSIZE             nLen;                           /* 文件大小，由父窗口管理，子窗口只能读取，不可修改 */
} VIDEOCREATEPARAM, * PVIDEOCREATEPARAM;

#ifdef __cplusplus
}
#endif

#endif
