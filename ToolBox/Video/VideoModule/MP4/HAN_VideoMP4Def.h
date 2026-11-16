#ifndef HAN_VIDEO_MP4_DEF_H
#define HAN_VIDEO_MP4_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <Windows.h>
#include <CommCtrl.h>

#include "..\..\HAN_VideoDef.h"
#include "HAN_VideoMP4Box.h"

#define HAN_VIDEO_MP4_TEXT_BUF_SIZE                                 1024

#define HAN_VIDEO_MP4_INVALID_SIZE                                  ((HANSIZE)(-1))

#define HAN_VIDEO_MP4_INVALID_PLAY_TIME                             ((VIDEOPLAYTIME)(-1))

#define HAN_VIDEO_MP4_BOX_TREE_W                                    200

#define HAN_VIDEO_MP4_BOX_ENTRIES_INFO_MAX_CNT                      10

#define HAN_VIDEO_MP4_CHOOSE_BOX_INFO_TRACK_LIST_HEIGHT             100
#define HAN_VIDEO_MP4_CHOOSE_BOX_INFO_SAMPLE_TEXT_WIDTH             80
#define HAN_VIDEO_MP4_CHOOSE_BOX_INFO_SAMPLE_TEXT_HEIGHT            22

struct tagVIDEOMP4BOXTREE;

typedef void (*UPDATEBOXINFOWINDOWCALLBACK)(struct tagVIDEOMP4BOXTREE* pBoxTree, HWND hWindow, void* pWinParam);

typedef struct tagVIDEOMP4BOXTREE {
    HTREEITEM                       hItem;
    VIDEOMP4BOX                     mp4Box;
    HANPSTR                         pBoxName;
    UPDATEBOXINFOWINDOWCALLBACK     UpdateBoxInfoWindow;    // 更新窗口的回调函数
    void*                           pWinParam;              // UpdateBoxInfoWindow 的额外参数，对解码器的 BOX 来说，该参数始终等于解码器地址
    struct tagVIDEOMP4BOXTREE*      pParent;
} VIDEOMP4BOXTREE, * PVIDEOMP4BOXTREE;

typedef struct tagVIDEOMP4READBOXMACHINE {
    const uint8_t*                  pData;
    HANSIZE                         nLen;
    HANPSIZE                        pBoxCnt;
    PVIDEOMP4BOXTREE                pTree;
} VIDEOMP4READBOXMACHINE, * PVIDEOMP4READBOXMACHINE;

#ifdef __cplusplus
}
#endif

#endif
