#ifndef HAN_PICTURE_DEF_H
#define HAN_PICTURE_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\..\HAN_Lib\HAN_windows.h"
#include "..\GlobalVariables.h"

#define PICTURE_WINDOW_DY               10
#define PICTURE_WINDOW_DX               10

#define PICTURE_TEXT_BUF_SIZE           1024

#define PICTURE_TOOL_SIZE               16

#define PICTURE_BUTTON_HEIGHT           30
#define PICTURE_TAB_HEIGHT              25
#define PICTURE_TAB_TEXT_SIZE           PICTURE_TEXT_BUF_SIZE
#define PICTURE_TAB_CAT_STR             TEXT("    ")
#define PICTURE_TAB_CLOSE_SIZE          16

#define PICTURE_BACKGROUND_BRUSH        (NULL_BRUSH)

#define PICTURE_HEX_PRINT_LEN           10

#define PICTURE_INFO_INVALID_TEXT       TEXT("NA")

#define PICTURE_INFO_HEIGHT             200

typedef enum {
    WID_PICTURE_BASE = HAN_WID_BASE,
    WID_PICTURE_TOOLBAR,
    WID_PICTURE_TOOLBAR_OPEN_PICTURE,
    WID_PICTURE_TOOLBAR_LOAD_ICON,
    WID_PICTURE_TOOLBAR_SAVE,
    WID_PICTURE_TAB,
    WID_PICTURE_TAB_CLOSE_BUTTON,
    WID_PICTURE_PICTURE_WINDOW,
    /* 加速键指令码 */
    WID_PICTURE_ACCEL_COMMAND_SAVE,
    WID_PICTURE_ACCEL_COMMAND_ZOOM_IN,
    WID_PICTURE_ACCEL_COMMAND_ZOOM_OUT,
    /* 编辑工具 */
    WID_PICTURE_EDIT_TOOL,
    WID_PICTURE_EDIT_TOOL_PAINT,
    WID_PICTURE_EDIT_TOOL_VSCROLL,
    WID_PICTURE_EDIT_TOOL_HSCROLL,
    WID_PICTURE_EDIT_TOOL_ZOOM,
    WID_PICTURE_EDIT_TOOL_PAPER,
    WID_PICTURE_EDIT_TOOL_CONSOLE,
    WID_PICTURE_EDIT_TOOL_STATUS_BAR,
    /* 图标工具 */
    WID_PICTURE_ICON_LIST,
    WID_PICTURE_ICON_INFO,
    /* 位图 */
    WID_PICTURE_BITMAP_INFO,
    WID_PICTURE_BITMAP_SHOW,
    /* JPEG */
    WID_PICTURE_JPEG_MCU_VIEW_BUTTON,
    WID_PICTURE_JPEG_MCU_VIEW_MCU_ID,
    WID_PICTURE_JPEG_MCU_VIEW_X,
    WID_PICTURE_JPEG_MCU_VIEW_Y,
    WID_PICTURE_JPEG_MCU_VIEW_DCT,
    WID_PICTURE_JPEG_MCU_VIEW_QUANTIZE,
    WID_PICTURE_JPEG_MCU_VIEW_LEFT,
    WID_PICTURE_JPEG_MCU_VIEW_UP,
    WID_PICTURE_JPEG_MCU_VIEW_RIGHT,
    WID_PICTURE_JPEG_MCU_VIEW_DOWN,
    WID_PICTURE_JPEG_MCU_VIEW_UPDATE,
    WID_PICTURE_JPEG_MCU_VIEW_QT,
    WID_PICTURE_JPEG_MCU_VIEW_COMPONENT_TAB,
    WID_PICTURE_JPEG_MCU_VIEW_BLOCK,
    WID_PICTURE_JPEG_MCU_VIEW_BLOCK_SOS_LIST,
    WID_PICTURE_JPEG_MCU_VIEW_BLOCK_NAME,
    WID_PICTURE_JPEG_MCU_VIEW_BLOCK_LIST,
    WID_PICTURE_JPEG_MCU_VIEW_BLOCK_POS,
    WID_PICTURE_JPEG_WARNING_ICON,
    WID_PICTURE_JPEG_WARNING_TOOL_TIP,
    WID_PICTURE_JPEG_SEGMENT_LIST,
    WID_PICTURE_JPEG_SEGMENT_INFO,
    WID_PICTURE_JPEG_SHOW,
    WID_PICTURE_JPEG_SAVE_PARAM_OK,
    WID_PICTURE_JPEG_SAVE_PARAM_CANCEL,
    WID_PICTURE_JPEG_SAVE_PARAM_SAMPLE_QUALITY_TITLE,
    WID_PICTURE_JPEG_SAVE_PARAM_SAMPLE_QUALITY_LOW,
    WID_PICTURE_JPEG_SAVE_PARAM_SAMPLE_QUALITY_MIDDLE,
    WID_PICTURE_JPEG_SAVE_PARAM_SAMPLE_QUALITY_HIGH,
    WID_PICTURE_JPEG_SAVE_PARAM_QUANTIZE_QUALITY_TITLE,
    WID_PICTURE_JPEG_SAVE_PARAM_QUANTIZE_QUALITY_OFFICIAL,
    WID_PICTURE_JPEG_SAVE_PARAM_QUANTIZE_QUALITY_HIGH,
    WID_PICTURE_JPEG_SAVE_PARAM_TYPE_TITLE,
    WID_PICTURE_JPEG_SAVE_PARAM_TYPE_BASELINE,
    WID_PICTURE_JPEG_SAVE_PARAM_TYPE_PROGRESSIVE,
    /* PNG */
    WID_PICTURE_PNG_CHUNK_LIST,
    WID_PICTURE_PNG_CHUNK_INFO,
    WID_PICTURE_PNG_SHOW,
} WIDPICTURE;

typedef enum {
    PCTM_BASE = WM_USER,    // 不使用的消息值，自定义消息请在下方添加
    PCTM_GETSAVEPARAM,      // 获取保存参数，所有图片模组都要支持该消息（WPARAM：未使用，LPARAM：PPICTURESAVEPARAM 型指针，用于接收参数）
    PCTM_ZOOM,              // 缩放图片（WPARAM：正数表示广大，负数表示缩小，LPARAM：未使用）
    PCTM_MODELBASE,         // 该消息不使用，作为图片模组的消息的起始消息，图片模组自定义的消息都应大于该消息
} PICTUREMESSAGE;

typedef enum { // 快捷键
    HKID_PICTURE_SAVE,
    HKID_PICTURE_ZOOM_IN,
    HKID_PICTURE_ZOOM_OUT,
    HKID_PICTURE_CNT,
} HKIDPICTURE;

typedef struct tagPICTURERGB {
    uint8_t             b;
    uint8_t             g;
    uint8_t             r;
} PICTURERGB, * PPICTURERGB;
typedef const PICTURERGB* PCPICTURERGB;
typedef struct tagPICTURERGBA {
    uint8_t             b;
    uint8_t             g;
    uint8_t             r;
    uint8_t             a;
} PICTURERGBA, * PPICTURERGBA;
typedef const PICTURERGBA* PCPICTURERGBA;
typedef union {
    PICTURERGBA         ptRGBA;
    uint8_t             pRGBA[4];
    COLORREF            crRGBA;
} PICTUREWINPIXEL;

typedef struct tagPICTURERESOLUTION {
    uint32_t            pxWidth;
    uint32_t            pxHeight;
} PICTURERESOLUTION, * PPICTURERESOLUTION;
typedef const PICTURERESOLUTION* PCPICTURERESOLUTION;

typedef struct tagPICTUREPIXELINFO {
    uint8_t             nColorCnt;      /* 像素的颜色个数，不带α通道的真彩色为3，带α通道的真彩色为4，带α通道的灰度为2，其它为1 */
    uint8_t             nColorSize;     /* 单个颜色的大小，8位深及以下为1；16位深为2 */
    uint8_t             nPixelSize;     /* 像素大小，等于 nColorCnt * nColorSize */
} PICTUREPIXELINFO, * PPICTUREPIXELINFO;

typedef struct tagPICTURE {
    PICTURERESOLUTION   pxResolution;   /* 图片分辨率 */
    PPICTURERGBA*       pPictureMap;    /* 二级指针，指向一个指针数组，每个指针指向每行的首地址，可以通过 pPictureMap[i][j] 的写法直接访问到每个像素 */
    uint8_t             pPixel[];       /* 缓存应由两部分组成，具体的数据和每行数据的首地址表
                                         * 数据应为行数 * 列数 * sizeof(PICTURERGBA)，强制带Alpha通道，地址表大小应为行数 * sizeof(void*)
                                         * 例：假设一张图片大小为 1920 * 1080，访问第 100 行，200 列的像素有以下两种写法：
                                         *      ((PPICTURERGBA)pPixel)[100 * 1080 + 200]
                                         * 或   pPictureMap[100][200]
                                         * 图片解码模块需要给 pPixel 分配足够的空间，并设置好 pPictureMap（可以调用函数 UpdatePictureInfo）
                                         */
} HANPICTURE, * HANPPICTURE;
typedef const HANPICTURE* HANPCPICTURE;

typedef struct tagHANPICTUREINFO {
    HANSIZE             nCnt;           /* 图片数量 */
    HANPPICTURE         pPicture[];     /* 指向每张图片的 HANPICTURE 结构地址的地址 */
} HANPICTUREINFO, * HANPPICTUREINFO;
typedef const HANPICTUREINFO* HANPCPICTUREINFO;

typedef struct tagPICTURECREATEPARAM {
    HANCHAR             pFileName[PATH_STR_SIZE];       /* 文件名，带路径 */
    uint8_t*            pData;                          /* 文件数据，由父窗口管理，子窗口只能读取，不可修改 */
    HANSIZE             nLen;                           /* 文件大小，由父窗口管理，子窗口只能读取，不可修改 */
} PICTURECREATEPARAM, * PPICTURECREATEPARAM;

typedef struct tagPICTURESAVEPARAM {
    HANDLE              hHeap;
    HANCHAR             pOpenClassName[CLASS_STR_SIZE]; /* 打开时的原图的图片窗口类名 */
    HANCHAR             pSaveClassName[CLASS_STR_SIZE]; /* 保存时所选的图片窗口类名 */
    HANPPICTURE         pPicture;                       /* 图片数据 */
    void*               pParam;                         /* 用户参数 */
} PICTURESAVEPARAM, * PPICTURESAVEPARAM;

#ifdef __cplusplus
}
#endif

#endif
