/* 名词解释：
 *      H264标准：Rec. ITU-T H.264
 *      视频标准：ISO/IEC 14496-12
 */

#include <inttypes.h>
#include <Windows.h>
#include <Shlwapi.h>
#include <CommCtrl.h>

#include "HAN_VideoMP4.h"
#include "HAN_VideoMP4Def.h"
#include "HAN_VideoMP4Box.h"
#include "DecoderModule\HAN_VideoMP4Decoder.h"

#include "..\H264\HAN_VideoH264.h"
#include "..\H264\HAN_VideoH264Text.h"

typedef struct tagVIDEOMP4TRACK {
    VIDEOMP4BOXINFO_tkhd                tkhd;
    VIDEOMP4BOXINFO_hdlr                hdlr;
    VIDEOMP4BOXINFO_stblSubBoxDefault   stts;
    VIDEOMP4BOXINFO_stblSubBoxDefault   ctts;
    VIDEOMP4BOXINFO_stblSubBoxDefault   stss;
    VIDEOMP4BOXINFO_stblSubBoxDefault   stsc;
    VIDEOMP4BOXINFO_stsz                stsz;
    VIDEOMP4BOXINFO_stco64              stco64;
    HANSIZE                             nSampleCnt;
    PVIDEOMP4DECODEINFOSAMPLE           pSample;
    VIDEOMP4DECODER                     decoder;
} VIDEOMP4TRACK, * PVIDEOMP4TRACK;

typedef struct tagVIDEOMP4BOXINFO {
    struct {
        PVIDEOMP4TRACK                  pList;
        HANSIZE                         nCnt;
        HANSIZE                         nTargetId;
    } track;
} VIDEOMP4BOXINFO, * PVIDEOMP4BOXINFO;
typedef const VIDEOMP4BOXINFO* PCVIDEOMP4BOXINFO;

typedef struct tagVIDEOMP4WNDEXTRA {
    HANDLE                          hHeap;
    HINSTANCE                       hInst;
    HWND                            hSelf;
    VIDEOCREATEPARAM                paramVideo;
    struct {
        HWND                        hTree;
        HWND                        hInfo;
        struct {
            HWND                    hText;
            HWND                    hList;
        } trackId;
        struct {
            BOOL                    bLock;
            HWND                    hText;
            HWND                    hInput;
            HWND                    hUpDown;
        } sampleId;
        HTREEITEM                   hFileTree;
        VIDEOMP4BOXINFO             boxInfo;
        struct {
            HANSIZE                 nCnt;
            PVIDEOMP4BOXTREE        pBoxTree;
            PVIDEOMP4BOXTREE        pChoosenBox;
        } map;
    } box;
    struct {
        HANSIZE                     nSampleCnt;
    } size;
    VIDEOMP4DECODER                 decoder;
    struct {
        HFONT                       hHex;
        HFONT                       hSys;
    } hFont;                        /* 字体 */
    uint8_t*                        pBufPtr;
    uint8_t                         pBuf[];
} VIDEOMP4WNDEXTRA, *PVIDEOMP4WNDEXTRA;

typedef struct tagVIDEOMP4BOXTYPEINFO {
    HANPSTR                         pName;
    HANSIZE                         nReadSubBoxOffset;
    UPDATEBOXINFOWINDOWCALLBACK     UpdateBoxInfoWindow;    // 可以填 NULL 来填充纯数据
} VIDEOMP4BOXTYPEINFO;

typedef struct tagVIDEOMP4BOXGETSIZE {
    PVIDEOMP4WNDEXTRA               mp4Info;
    VIDEOMP4DECODERTYPE             decoderType;
    HANPSIZE                        pBoxCnt;
    HANPSIZE                        pDecoderCnt;
    HANPSIZE                        pDecoderSize;
} VIDEOMP4BOXGETSIZE, * PVIDEOMP4BOXGETSIZE;

typedef struct tagVIDEOMP4BOXPROCESS {
    PVIDEOMP4WNDEXTRA               mp4Info;
    PVIDEOMP4BOXTREE                pTree;
    VIDEOMP4DECODERTYPE             decoderType;
    uint8_t*                        pDecoder;
    HANPSIZE                        pBoxCnt;
    HANPSIZE                        pDecoderCnt;
    HANPSIZE                        pDecoderBufOffset;
} VIDEOMP4BOXPROCESS, * PVIDEOMP4BOXPROCESS;

typedef struct tagVIDEOMP4BOXTRACKHANDLERINFO {
    uint8_t                         pType[5];
} VIDEOMP4BOXTRACKHANDLERINFO;

static LRESULT CALLBACK VideoMP4WndProc(HWND hVideoMP4, UINT message, WPARAM wParam, LPARAM lParam);

static LRESULT CreateCallback(HWND hVideoMP4, LPARAM lParam);
static void CommandCallback(PVIDEOMP4WNDEXTRA mp4Info, WPARAM wParam);
static LRESULT NotifyCallback(PVIDEOMP4WNDEXTRA mp4Info, NMHDR* pNotify);
static void DestroyCallback(PVIDEOMP4WNDEXTRA mp4Info);
static void InitMP4BoxInfo(PVIDEOMP4WNDEXTRA mp4Info);
static void GetMP4BoxInfoWindowPos(PVIDEOMP4WNDEXTRA mp4Info, PRECT pPos, VIDEOMP4BOXTYPE boxType);
static void InitMP4BoxInfoWindow(PVIDEOMP4WNDEXTRA mp4Info);
static void InitMP4BoxTreeWindow(PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxTreeMap(PVIDEOMP4BOXPROCESS pProcess, PVIDEOMP4WNDEXTRA mp4Info);
static void PrintMP4Text4Bytes(HANCHAR pDest[5], const uint8_t pSrc[4]);
static PVIDEOMP4WNDEXTRA ReallocMP4InfoMemory(PVIDEOMP4WNDEXTRA mp4Info);
static void VideoMP4PrintHexData(HANPSTR pText, HANSIZE nTextLen, const uint8_t* pData, HANSIZE nDataLen);
static LRESULT BoxTreeNotifyCallback(PVIDEOMP4WNDEXTRA mp4Info, NMHDR* pNotify);
static void UpdateBoxInfoPos(PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxInfoWindow(PVIDEOMP4WNDEXTRA mp4Info, TVITEM* pItem);
static void InitTrackIdListWindow(PVIDEOMP4WNDEXTRA mp4Info);
static void ChooseBoxInfoTrackCallback(PVIDEOMP4WNDEXTRA mp4Info);

static BOOL GetMP4InfoSize(PVIDEOMP4WNDEXTRA mp4Info);
static BOOL MP4Process(PVIDEOMP4WNDEXTRA mp4Info);
static void ReadMP4BoxCallback_GetSize(PCVIDEOMP4BOX pBox, void* pParam);
static void ReadMP4BoxCallback_GetSize_stsd(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXGETSIZE pGetSize);
static void ReadMP4BoxCallback_GetSize_DecoderBox(PCVIDEOMP4BOX pBox, void* pParam);
static void ReadMP4BoxCallback_Process(PCVIDEOMP4BOX pBox, void* pParam);
static void ReadMP4BoxCallback_Process_stsd(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXPROCESS pProcess);
static void ReadMP4BoxCallback_Process_DecoderBox(PCVIDEOMP4BOX pBox, void* pParam);

static inline HANINT GetBoxInfoWindowWidth(void);
static inline uint16_t ReadMP4Data2ByteMSB(const uint8_t pData[2]);
static inline uint32_t ReadMP4Data3ByteMSB(const uint8_t pData[3]);
static inline uint32_t ReadMP4Data4ByteMSB(const uint8_t pData[4]);

static void ReadMP4BoxCallback_ProcessInittrak(PVIDEOMP4WNDEXTRA mp4Info, PVIDEOMP4BOXPROCESS pProcess);

static BOOL UpdateMP4Info_trakProcess(PVIDEOMP4WNDEXTRA mp4Info);
static BOOL UpdateMP4Info_stts(PVIDEOMP4WNDEXTRA mp4Info);
static BOOL UpdateMP4Info_ctts(PVIDEOMP4WNDEXTRA mp4Info);
static BOOL UpdateMP4Info_stss(PVIDEOMP4WNDEXTRA mp4Info);
static BOOL UpdateMP4Info_stsc(PVIDEOMP4WNDEXTRA mp4Info);
static BOOL UpdateMP4Info_stsz(PVIDEOMP4WNDEXTRA mp4Info);
static BOOL UpdateMP4Info_stco64(PVIDEOMP4WNDEXTRA mp4Info);

static HANINT UpdateBoxInfoWindow_FullBoxVersionFlags(HANINT nId, HWND hListView, PVIDEOMP4BOXINFOFULLBOXVERFLAGS pFB);
static HANINT UpdateBoxInfoWindow_BlankBox(const uint8_t* pData, HANSIZE nLen, HANINT nId, HWND hListView);
static void UpdateBoxInfoWindow_PrintTimeDuration(HANPSTR pText, HANSIZE nLen, PULARGE_INTEGER pTimeDuration);
static void UpdateBoxInfoWindow_SetTitle(PVIDEOMP4BOXTREE pBoxTree, HWND hListView);
static void UpdateBoxInfoWindow_Default(PVIDEOMP4BOXTREE pBoxTree, HWND hListView);
static void UpdateBoxInfoWindow_ftyp(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam);
static void UpdateBoxInfoWindow_free(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam);
static void UpdateBoxInfoWindow_mdat(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam);
static void UpdateBoxInfoWindow_mvhd(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam);
static void UpdateBoxInfoWindow_iods(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam);
static void UpdateBoxInfoWindow_tkhd(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam);
static void UpdateBoxInfoWindow_elst(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam);
static void UpdateBoxInfoWindow_mdhd(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam);
static void UpdateBoxInfoWindow_hdlr(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam);
static void UpdateBoxInfoWindow_vmhd(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam);
static void UpdateBoxInfoWindow_dref(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam);
static void UpdateBoxInfoWindow_url_(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam);
static void UpdateBoxInfoWindow_stsd(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam);
static void UpdateBoxInfoWindow_stts(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam);
static void UpdateBoxInfoWindow_ctts(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam);
static void UpdateBoxInfoWindow_stss(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam);
static void UpdateBoxInfoWindow_stsc(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam);
static void UpdateBoxInfoWindow_stsz(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam);
static void UpdateBoxInfoWindow_stco(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam);
static void UpdateBoxInfoWindow_co64(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam);
static void UpdateBoxInfoWindow_smhd(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam);
static void UpdateBoxInfoWindow_meta(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam);
static void UpdateBoxInfoWindow_data(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam);

static void UpdateBoxInfoWindow_mdatSample(HANSIZE nTrackId, HANSIZE nSampleId, PVIDEOMP4WNDEXTRA mp4Info);

static HANSIZE DecodeBoxDataReadDataByVersion(const uint8_t* pData, uint8_t nVersion, PULARGE_INTEGER pLargeInt);
static HANSIZE DecodeBoxDataDateTime(const uint8_t* pData, uint8_t nVersion, PVIDEOMP4BOXINFODATETIME pDateTime);
static void DecodeBoxData_FullBoxVersionFlags(const uint8_t* pData, PVIDEOMP4BOXINFOFULLBOXVERFLAGS pFB);
static BOOL DecodeBoxData_mvhd(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_mvhd pmvhd);
static BOOL DecodeBoxData_tkhd(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_tkhd ptkhd);
static HANSIZE DecodeBoxData_elst(const uint8_t* pData, uint8_t nVersion, PVIDEOMP4BOXINFO_elst pelst);
static BOOL DecodeBoxData_mdhd(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_mdhd pmdhd);
static void DecodeBoxData_hdlr(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_hdlr phdlr);
static void DecodeBoxData_vmhd(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_vmhd pvmhd);
static VIDEOMP4TRACKHANDLERTYPE DecodeBoxDataGetTrackHandlerType(uint8_t pType[4]);
static BOOL DecodeBoxData_url_(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_url_ purl_);
static void DecodeBoxData_stblSubBoxDefault(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_stblSubBoxDefault pDef);
static void DecodeBoxData_stsd(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_stsd pstsd);
static void DecodeBoxData_stsz(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_stsz pstsz);
static void DecodeBoxData_stco(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_stco64 pstco);
static void DecodeBoxData_co64(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_stco64 pco64);

static BOOL DecodeMP4(PVIDEOMP4WNDEXTRA mp4Info, HANSIZE nFrame);

static const HANINT sg_pMP4BoxInfoHeaderWidth[VIDEO_MP4_BOX_INFO_HEADER_CNT] = {
    [VIDEO_MP4_BOX_INFO_HEADER_FIELD] = 150,
    [VIDEO_MP4_BOX_INFO_HEADER_VALUE] = 300,
};
static const VIDEOMP4BOXTRACKHANDLERINFO sg_pMP4BoxTrackHandlerInfo[VIDEO_MP4_TRACK_HANDLER_TYPE_CNT] = {
    [VIDEO_MP4_TRACK_HANDLER_TYPE_VIDEO] = {
        .pType = "vide",
    },
    [VIDEO_MP4_TRACK_HANDLER_TYPE_SOUND] = {
        .pType = "soun",
    },
    [VIDEO_MP4_TRACK_HANDLER_TYPE_SUBTITLE] = {
        .pType = "subt",
    },
    [VIDEO_MP4_TRACK_HANDLER_TYPE_HINT] = {
        .pType = "hint",
    },
    [VIDEO_MP4_TRACK_HANDLER_TYPE_METADATA] = {
        .pType = "meta",
    },
    [VIDEO_MP4_TRACK_HANDLER_TYPE_TEXT] = {
        .pType = "text",
    },
    [VIDEO_MP4_TRACK_HANDLER_TYPE_TIMECODE] = {
        .pType = "tmcd",
    },
};
static const VIDEOMP4BOXTYPEINFO sg_pMP4BoxTypeInfo[VIDEO_MP4_BOX_TYPE_CNT] = {
    [VIDEO_MP4_BOX_TYPE_ftyp] = {
        .pName = TEXT("文件类型"),
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_ftyp,
    },
    [VIDEO_MP4_BOX_TYPE_free] = {
        .pName = TEXT("占位符"),
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_free,
    },
    [VIDEO_MP4_BOX_TYPE_mdat] = {
        .pName = TEXT("媒体数据"),
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_mdat,
    },
    [VIDEO_MP4_BOX_TYPE_moov] = {
        .pName = TEXT("媒体元数据信息"),
        .nReadSubBoxOffset = 0,
        .UpdateBoxInfoWindow = NULL,
    },
    [VIDEO_MP4_BOX_TYPE_mvhd] = {
        .pName = TEXT("媒体数据头"),
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_mvhd,
    },
    [VIDEO_MP4_BOX_TYPE_iods] = {
        .pName = TEXT("初始对象描述符"),
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_iods,
    },
    [VIDEO_MP4_BOX_TYPE_trak] = {
        .pName = TEXT("轨道数据信息"),
        .nReadSubBoxOffset = 0,
        .UpdateBoxInfoWindow = NULL,
    },
    [VIDEO_MP4_BOX_TYPE_tkhd] = {
        .pName = TEXT("轨道全局属性"),
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_tkhd,
    },
    [VIDEO_MP4_BOX_TYPE_edts] = {
        .pName = TEXT("编辑列表容器"),
        .nReadSubBoxOffset = 0,
        .UpdateBoxInfoWindow = NULL,
    },
    [VIDEO_MP4_BOX_TYPE_elst] = {
        .pName = TEXT("编辑列表"),
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_elst,
    },
    [VIDEO_MP4_BOX_TYPE_mdia] = {
        .pName = TEXT("轨道媒体信息容器"),
        .nReadSubBoxOffset = 0,
        .UpdateBoxInfoWindow = NULL,
    },
    [VIDEO_MP4_BOX_TYPE_mdhd] = {
        .pName = TEXT("轨道媒体独立信息"),
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_mdhd,
    },
    [VIDEO_MP4_BOX_TYPE_hdlr] = {
        .pName = TEXT("轨道媒体独立信息"),
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_hdlr,
    },
    [VIDEO_MP4_BOX_TYPE_minf] = {
        .pName = TEXT("轨道媒体信息"),
        .nReadSubBoxOffset = 0,
        .UpdateBoxInfoWindow = NULL,
    },
    [VIDEO_MP4_BOX_TYPE_vmhd] = {
        .pName = TEXT("视频媒体头"),
        .nReadSubBoxOffset = 0,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_vmhd,
    },
    [VIDEO_MP4_BOX_TYPE_dinf] = {
        .pName = TEXT("数据位置信息"),
        .nReadSubBoxOffset = 0,
        .UpdateBoxInfoWindow = NULL,
    },
    [VIDEO_MP4_BOX_TYPE_dref] = {
        .pName = TEXT("数据引用"),
        .nReadSubBoxOffset = 8,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_dref,
    },
    [VIDEO_MP4_BOX_TYPE_url_] = {
        .pName = TEXT("数据入口Url"),
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_url_,
    },
    [VIDEO_MP4_BOX_TYPE_stbl] = {
        .pName = TEXT("样本索引表"),
        .nReadSubBoxOffset = 0,
        .UpdateBoxInfoWindow = NULL,
    },
    [VIDEO_MP4_BOX_TYPE_stsd] = {
        .pName = TEXT("解码器信息"),
        .nReadSubBoxOffset = 8,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_stsd,
    },
    [VIDEO_MP4_BOX_TYPE_stts] = {
        .pName = TEXT("解码时间-样本映射表"),
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_stts,
    },
    [VIDEO_MP4_BOX_TYPE_ctts] = {
        .pName = TEXT("合成时间-样本映射表"),
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_ctts,
    },
    [VIDEO_MP4_BOX_TYPE_stss] = {
        .pName = TEXT("同步样本表"),
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_stss,
    },
    [VIDEO_MP4_BOX_TYPE_stsc] = {
        .pName = TEXT("样本与块映射表"),
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_stsc,
    },
    [VIDEO_MP4_BOX_TYPE_stsz] = {
        .pName = TEXT("样本大小表"),
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_stsz,
    },
    [VIDEO_MP4_BOX_TYPE_stco] = {
        .pName = TEXT("块偏移表"),
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_stco,
    },
    [VIDEO_MP4_BOX_TYPE_co64] = {
        .pName = TEXT("块偏移表"),
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_co64,
    },
    [VIDEO_MP4_BOX_TYPE_smhd] = {
        .pName = TEXT("音频媒体头"),
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_smhd,
    },
    [VIDEO_MP4_BOX_TYPE_udta] = {
        .pName = TEXT("用户数据"),
        .nReadSubBoxOffset = 0,
        .UpdateBoxInfoWindow = NULL,
    },
    [VIDEO_MP4_BOX_TYPE_meta] = {
        .pName = TEXT("元数据"),
        .nReadSubBoxOffset = 4,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_meta,
    },
    [VIDEO_MP4_BOX_TYPE_ilst] = {
        .pName = TEXT("条目列表"),
        .nReadSubBoxOffset = 0,
        .UpdateBoxInfoWindow = NULL,
    },
    [VIDEO_MP4_BOX_TYPE_data] = {
        .pName = TEXT("数据"),
        .nReadSubBoxOffset = HAN_VIDEO_MP4_INVALID_SIZE,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_data,
    },
    [VIDEO_MP4_BOX_TYPE_desc] = {
        .pName = TEXT("描述"),
        .nReadSubBoxOffset = 0,
        .UpdateBoxInfoWindow = NULL,
    },
    [VIDEO_MP4_BOX_TYPE_Copyright_too] = {
        .pName = TEXT("版权信息"),
        .nReadSubBoxOffset = 0,
        .UpdateBoxInfoWindow = NULL,
    },
};

BOOL CheckMP4Type(const uint8_t* pData, HANSIZE nLen)
{
    BOOL bRet = FALSE;

    if ((8 <= nLen) && (!memcmp(&pData[4], "ftyp", 4))) { bRet = TRUE; }

    return bRet;
}

void RegisterHANVideoMP4(HINSTANCE hInst)
{
    WNDCLASSEX wcex = {
        .cbSize         = sizeof(WNDCLASSEX),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = VideoMP4WndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = sizeof(PVIDEOMP4WNDEXTRA),
        .hInstance      = hInst,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = (HBRUSH)GetStockObject(VIDEO_BACKGROUND_BRUSH),
        .lpszMenuName   = NULL,
        .lpszClassName  = HAN_VIDEO_MP4_CLASS,
        .hIconSm        = NULL,
    };
    RegisterClassEx(&wcex);

    INITCOMMONCONTROLSEX icex = {0};
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_UPDOWN_CLASS;
    InitCommonControlsEx(&icex);
}

static LRESULT CALLBACK VideoMP4WndProc(HWND hVideoMP4, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    PVIDEOMP4WNDEXTRA mp4Info = (PVIDEOMP4WNDEXTRA)GetWindowLongPtr(hVideoMP4, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = CreateCallback(hVideoMP4, lParam);
        } break;
        case WM_COMMAND: {
            CommandCallback(mp4Info, wParam);
        } break;
        case WM_CTLCOLORSTATIC: {
            lWndProcRet = (INT_PTR)GetStockObject(NULL_BRUSH);
        } break;
        case WM_NOTIFY: {
            lWndProcRet = NotifyCallback(mp4Info, (NMHDR*)lParam);
        } break;
        case WM_DESTROY: {
            DestroyCallback(mp4Info);
            lWndProcRet = DefWindowProc(hVideoMP4, message, wParam, lParam);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hVideoMP4, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}

static LRESULT CreateCallback(HWND hVideoMP4, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PVIDEOMP4WNDEXTRA mp4Info;
    PVIDEOMP4WNDEXTRA mp4TempInfo;
    HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
    PVIDEOCREATEPARAM pVideoCreateParam = ((LPCREATESTRUCT)lParam)->lpCreateParams;
    RECT rcClientSize;
    RECT rcBoxInfoSize;

    HANINT nWinX;
    HANINT nWinY;
    HANINT nWinW;
    HANINT nWinH;

    HANDLE hHeap = GetProcessHeap();
    if (NULL == hHeap) { lWndProcRet = -1; }
    if (-1 != lWndProcRet)
    {
        mp4Info = (PVIDEOMP4WNDEXTRA)HANWinHeapAlloc(hHeap, NULL, sizeof(VIDEOMP4WNDEXTRA));
        if (NULL == mp4Info) { lWndProcRet = -1; }
    }
    /* 创建窗口 */
    if (-1 != lWndProcRet)
    {
        SetWindowLongPtr(hVideoMP4, 0, (LONG_PTR)mp4Info);

        mp4Info->hHeap = hHeap;
        mp4Info->hInst = hInst;
        mp4Info->hSelf = hVideoMP4;

        InitMP4BoxInfo(mp4Info);

        GetClientRect(hVideoMP4, &rcClientSize);

        mp4Info->hFont.hHex = CreateFontIndirect(&g_lfHexFont);
        mp4Info->hFont.hSys = CreateFontIndirect(&g_lfInfoFont);

        nWinX = 0;
        nWinY = 0;
        nWinW = HAN_VIDEO_MP4_BOX_TREE_W;
        nWinH = GetRectH(&rcClientSize);
        mp4Info->box.hTree = CreateWindow(WC_TREEVIEW, TEXT("BOX总览"),
            WS_CHILD | WS_VISIBLE | WS_BORDER | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS,
            nWinX, nWinY, nWinW, nWinH,
            hVideoMP4, (HMENU)WID_VIDEO_MP4_BOX_TREE, hInst, NULL);

        nWinX += nWinW + VIDEO_WINDOW_DX;
        nWinY = VIDEO_WINDOW_DY;
        nWinW = HAN_VIDEO_MP4_CHOOSE_BOX_INFO_SAMPLE_TEXT_WIDTH;
        nWinH = HAN_VIDEO_MP4_CHOOSE_BOX_INFO_SAMPLE_TEXT_HEIGHT;
        mp4Info->box.trackId.hText = CreateWindow(TEXT("static"), TEXT("轨道 ID"),
            WS_CHILD | SS_CENTERIMAGE,
            nWinX, nWinY, nWinW, nWinH,
            hVideoMP4, (HMENU)WID_VIDEO_MP4_BOX_TRACK_ID_TEXT, hInst, NULL);
        nWinX += nWinW;
        nWinW = GetBoxInfoWindowWidth() - nWinW;
        nWinH = HAN_VIDEO_MP4_CHOOSE_BOX_INFO_TRACK_LIST_HEIGHT;
        mp4Info->box.trackId.hList = CreateWindow(TEXT("combobox"), NULL,
            WS_CHILD | WS_VSCROLL | CBS_DROPDOWNLIST,
            nWinX, nWinY, nWinW, nWinH,
            hVideoMP4, (HMENU)WID_VIDEO_MP4_BOX_TRACK_ID_LIST, hInst, NULL);

        nWinX -= HAN_VIDEO_MP4_CHOOSE_BOX_INFO_SAMPLE_TEXT_WIDTH;
        nWinY += HAN_VIDEO_MP4_CHOOSE_BOX_INFO_SAMPLE_TEXT_HEIGHT + VIDEO_WINDOW_DY;
        nWinW = HAN_VIDEO_MP4_CHOOSE_BOX_INFO_SAMPLE_TEXT_WIDTH;
        nWinH = HAN_VIDEO_MP4_CHOOSE_BOX_INFO_SAMPLE_TEXT_HEIGHT;
        mp4Info->box.sampleId.hText = CreateWindow(TEXT("static"), TEXT("样本索引"),
            WS_CHILD | SS_CENTERIMAGE,
            nWinX, nWinY, nWinW, nWinH,
            hVideoMP4, (HMENU)WID_VIDEO_MP4_BOX_SAMPLE_ID_TEXT, hInst, NULL);
        nWinX += nWinW;
        nWinW = GetBoxInfoWindowWidth() - nWinW;
        mp4Info->box.sampleId.hInput = CreateWindow(TEXT("edit"), TEXT("0"),
            WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | ES_NUMBER,
            nWinX, nWinY, nWinW, nWinH,
            hVideoMP4, (HMENU)WID_VIDEO_MP4_BOX_SAMPLE_ID_INPUT, hInst, NULL);
        mp4Info->box.sampleId.hUpDown = CreateWindow(UPDOWN_CLASS, TEXT(""),
            WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_SETBUDDYINT | UDS_NOTHOUSANDS,
            0, 0, 0, 0,
            hVideoMP4, (HMENU)WID_VIDEO_MP4_BOX_SAMPLE_ID_UPDOWN, hInst, NULL);

        GetMP4BoxInfoWindowPos(mp4Info, &rcBoxInfoSize, VIDEO_MP4_BOX_TYPE_CNT);
        mp4Info->box.hInfo = CreateWindow(WC_LISTVIEW, TEXT("BOX信息"),
            WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL,
            rcBoxInfoSize.left, rcBoxInfoSize.top, GetRectW(&rcBoxInfoSize), GetRectH(&rcBoxInfoSize),
            hVideoMP4, (HMENU)WID_VIDEO_MP4_BOX_INFO, hInst, NULL);

        SendMessage(mp4Info->box.trackId.hText, WM_SETFONT, (WPARAM)(mp4Info->hFont.hSys), (LPARAM)TRUE);
        SendMessage(mp4Info->box.trackId.hList, WM_SETFONT, (WPARAM)(mp4Info->hFont.hSys), (LPARAM)TRUE);
        SendMessage(mp4Info->box.sampleId.hText, WM_SETFONT, (WPARAM)(mp4Info->hFont.hSys), (LPARAM)TRUE);
        SendMessage(mp4Info->box.sampleId.hInput, WM_SETFONT, (WPARAM)(mp4Info->hFont.hSys), (LPARAM)TRUE);

        InitMP4BoxInfoWindow(mp4Info);
    }
    /* 解码文件的数据段，填充信息 */
    if (-1 != lWndProcRet)
    {
        mp4Info->paramVideo = *pVideoCreateParam;
        mp4Info->box.map.nCnt = 0;
        if (FALSE == GetMP4InfoSize(mp4Info)) { lWndProcRet = -1; }
    }
    /* 重新分配 mp4Info 内存 */
    if (-1 != lWndProcRet)
    {
        mp4TempInfo = ReallocMP4InfoMemory(mp4Info);
        if (NULL != mp4TempInfo)
        {
            mp4Info = mp4TempInfo;
            SetWindowLongPtr(hVideoMP4, 0, (LONG_PTR)mp4Info);
        }
        else { lWndProcRet = -1; }
    }
    /* 视频解码 */
    if (-1 != lWndProcRet)
    {
        BOOL bDecodeRet = TRUE;

        InitMP4BoxTreeWindow(mp4Info);
        if (FALSE == MP4Process(mp4Info)) { lWndProcRet = -1; }
        else { bDecodeRet = DecodeMP4(mp4Info, 0); }

        (void)bDecodeRet;
    }

    return lWndProcRet;
}
static void CommandCallback(PVIDEOMP4WNDEXTRA mp4Info, WPARAM wParam)
{
    switch (LOWORD(wParam)) {
        case WID_VIDEO_MP4_BOX_TRACK_ID_LIST: {
            if (CBN_SELCHANGE == HIWORD(wParam)) { ChooseBoxInfoTrackCallback(mp4Info); }
        } break;
        case WID_VIDEO_MP4_BOX_SAMPLE_ID_INPUT: {
            if ((FALSE == mp4Info->box.sampleId.bLock) && (EN_CHANGE == HIWORD(wParam))) { UpdateBoxInfoWindow_mdat(NULL, mp4Info->box.hInfo, mp4Info); }
        } break;

        default: { } break;
    }
}
static LRESULT NotifyCallback(PVIDEOMP4WNDEXTRA mp4Info, NMHDR* pNotify)
{
    LRESULT lWndProcRet = 0;

    switch (pNotify->idFrom) {
        case WID_VIDEO_MP4_BOX_TREE: {
            lWndProcRet = BoxTreeNotifyCallback(mp4Info, pNotify);
        } break;

        default: { } break;
    }

    return lWndProcRet;
}
static void DestroyCallback(PVIDEOMP4WNDEXTRA mp4Info)
{
    HANWinHeapFree(mp4Info->hHeap, 0, mp4Info);
}
static void InitMP4BoxInfo(PVIDEOMP4WNDEXTRA mp4Info)
{
    mp4Info->box.map.pBoxTree = NULL;
    mp4Info->box.map.pChoosenBox = NULL;
    mp4Info->box.sampleId.bLock = FALSE;
    mp4Info->box.boxInfo.track.nCnt = 0;
    mp4Info->box.boxInfo.track.nTargetId = 0;
    mp4Info->size.nSampleCnt = 0;
    mp4Info->decoder.table.nCnt = 0;
    mp4Info->decoder.buf.nSize = 0;
    mp4Info->pBufPtr = NULL;
}
static void GetMP4BoxInfoWindowPos(PVIDEOMP4WNDEXTRA mp4Info, PRECT pPos, VIDEOMP4BOXTYPE boxType)
{
    RECT rcClientSize;

    GetClientRect(mp4Info->hSelf, &rcClientSize);
    pPos->left = HAN_VIDEO_MP4_BOX_TREE_W + VIDEO_WINDOW_DX;
    pPos->right = pPos->left + GetBoxInfoWindowWidth();
    pPos->bottom = GetRectH(&rcClientSize);

    switch (boxType) {
        case VIDEO_MP4_BOX_TYPE_mdat: {
            pPos->top = (HAN_VIDEO_MP4_CHOOSE_BOX_INFO_SAMPLE_TEXT_HEIGHT * 2) + (VIDEO_WINDOW_DY * 3);
        } break;
        default: { pPos->top = 0; } break;
    }
}
static void InitMP4BoxInfoWindow(PVIDEOMP4WNDEXTRA mp4Info)
{
    LVCOLUMN lvTitle = { .mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM, };
    
    SendMessage(mp4Info->box.sampleId.hUpDown, UDM_SETBUDDY, (WPARAM)(mp4Info->box.sampleId.hInput), 0);
    ListView_SetExtendedListViewStyle(mp4Info->box.hInfo, LVS_EX_FULLROWSELECT);
    
    for (VIDEOMP4BOXINFOHEADER iLoop = 0; iLoop < VIDEO_MP4_BOX_INFO_HEADER_CNT; iLoop++)
    {
        lvTitle.iSubItem = iLoop;
        lvTitle.pszText = TEXT("");
        lvTitle.cx = sg_pMP4BoxInfoHeaderWidth[iLoop];
        lvTitle.fmt = LVCFMT_LEFT;
        ListView_InsertColumn(mp4Info->box.hInfo, iLoop, &lvTitle);
    }
}
static void InitMP4BoxTreeWindow(PVIDEOMP4WNDEXTRA mp4Info)
{
    HANPSTR pFileName = PathFindFileName(mp4Info->paramVideo.pFileName);
    TVINSERTSTRUCT tviInsert;

    tviInsert.hParent = NULL;
    tviInsert.hInsertAfter = TVI_FIRST;
    tviInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
    tviInsert.item.pszText = pFileName;
    tviInsert.item.cchTextMax = (HANINT)HAN_strlen(pFileName) + 1;
    tviInsert.item.lParam = (LPARAM)NULL;
    mp4Info->box.hFileTree = TreeView_InsertItem(mp4Info->box.hTree, &tviInsert);
}
static void UpdateBoxTreeMap(PVIDEOMP4BOXPROCESS pProcess, PVIDEOMP4WNDEXTRA mp4Info)
{
    HANCHAR pText[5];
    PVIDEOMP4BOXTREE pTree = pProcess->pTree;
    PVIDEOMP4BOXTREE pParent = pProcess->pTree->pParent;
    TVINSERTSTRUCT tviInsert;

    PrintMP4Text4Bytes(pText, pTree->mp4Box.pType);

    if (NULL != pParent) { tviInsert.hParent = pParent->hItem; }
    else { tviInsert.hParent = mp4Info->box.hFileTree; }
    tviInsert.hInsertAfter = TVI_LAST;
    tviInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
    tviInsert.item.pszText = pText;
    tviInsert.item.cchTextMax = ArrLen(pText);
    tviInsert.item.lParam = (LPARAM)pTree;
    pProcess->pTree->hItem = TreeView_InsertItem(mp4Info->box.hTree, &tviInsert);
}
static void PrintMP4Text4Bytes(HANCHAR pDest[5], const uint8_t pSrc[4])
{
    CHAR pMultiBytes[5] = { pSrc[0], pSrc[1], pSrc[2], pSrc[3], '\0', };
    HAN_snprintf(pDest, 5, HANPSTR_PRINT_PCHAR_FORMAT, pMultiBytes);
}
static PVIDEOMP4WNDEXTRA ReallocMP4InfoMemory(PVIDEOMP4WNDEXTRA mp4Info)
{
    PVIDEOMP4WNDEXTRA pRet;
    PVIDEOMP4BOXINFO pBoxInfo = &(mp4Info->box.boxInfo);
    HANSIZE nBoxCnt = mp4Info->box.map.nCnt;
    SIZE_T nOffset;
    SIZE_T nBoxTreeSize;
    SIZE_T nTrackSize;
    SIZE_T nSampleSize;
    SIZE_T nDecoderTypeSize;
    SIZE_T nDecoderTableSize;
    SIZE_T nDecoderBufSize;

    nBoxTreeSize = sizeof(VIDEOMP4BOXTREE) * nBoxCnt;
    nTrackSize = sizeof(VIDEOMP4TRACK) * pBoxInfo->track.nCnt;
    nSampleSize = sizeof(VIDEOMP4DECODEINFOSAMPLE) * mp4Info->size.nSampleCnt;
    nDecoderTypeSize = sizeof(VIDEOMP4DECODERTYPE) * mp4Info->decoder.table.nCnt;
    nDecoderTableSize = sizeof(*(mp4Info->decoder.table.pTable)) * mp4Info->decoder.table.nCnt;
    nDecoderBufSize = mp4Info->decoder.buf.nSize;
    pRet = (PVIDEOMP4WNDEXTRA)HANWinHeapAlloc(
        mp4Info->hHeap, mp4Info,
        sizeof(VIDEOMP4WNDEXTRA)
         + nBoxTreeSize
         + nTrackSize
         + nSampleSize
         + nDecoderTypeSize
         + nDecoderTableSize
         + nDecoderBufSize
    );

    if (NULL != pRet)
    {
        nOffset = 0;
        pRet->box.map.pBoxTree = (PVIDEOMP4BOXTREE)&(pRet->pBuf[nOffset]); nOffset += nBoxTreeSize;
        pRet->box.boxInfo.track.pList = (PVIDEOMP4TRACK)&(pRet->pBuf[nOffset]); nOffset += nTrackSize;
        pRet->decoder.table.pType = (PVIDEOMP4DECODERTYPE)&(pRet->pBuf[nOffset]); nOffset += nDecoderTypeSize;
        pRet->decoder.table.pTable = (uint8_t**)&(pRet->pBuf[nOffset]); nOffset += nDecoderTableSize;
        pRet->decoder.buf.pData = &(pRet->pBuf[nOffset]); nOffset += nDecoderBufSize;
        pRet->pBufPtr = &(pRet->pBuf[nOffset]);
    }

    return pRet;
}
static void VideoMP4PrintHexData(HANPSTR pText, HANSIZE nTextLen, const uint8_t* pData, HANSIZE nDataLen)
{
    HANSIZE nLoopLen = VIDEO_HEX_PRINT_LEN;
    HANSIZE nOffset = 0;

    HAN_strcpy(pText, TEXT(""));
    if (nDataLen < VIDEO_HEX_PRINT_LEN) { nLoopLen = nDataLen; }
    for (HANSIZE iLoop = 0; iLoop < nLoopLen; iLoop++)
    {
        HAN_snprintf(&pText[nOffset], nTextLen - nOffset, TEXT("%02X "), pData[iLoop]);
        nOffset += HAN_strlen(&pText[nOffset]);
    }
    if (VIDEO_HEX_PRINT_LEN < nDataLen) { HAN_snprintf(&pText[nOffset], nTextLen - nOffset, TEXT("...")); }
}
static LRESULT BoxTreeNotifyCallback(PVIDEOMP4WNDEXTRA mp4Info, NMHDR* pNotify)
{
    LRESULT lWndProcRet = 0;

    switch (pNotify->code) {
        case TVN_SELCHANGED: {
            NMTREEVIEW* pTreeView = (NMTREEVIEW*)pNotify;
            UpdateBoxInfoWindow(mp4Info, &(pTreeView->itemNew));
            UpdateBoxInfoPos(mp4Info);
        } break;

        default: { } break;
    }

    return lWndProcRet;
}
static void UpdateBoxInfoPos(PVIDEOMP4WNDEXTRA mp4Info)
{
    RECT rcBoxInfoPos;
    PVIDEOMP4BOXTREE pBoxTree = mp4Info->box.map.pChoosenBox;
    VIDEOMP4BOXTYPE boxType;

    if (NULL == pBoxTree) { boxType = VIDEO_MP4_BOX_TYPE_CNT; }
    else { boxType = pBoxTree->mp4Box.eType; }

    GetMP4BoxInfoWindowPos(mp4Info, &rcBoxInfoPos, boxType);
    MoveWindow(mp4Info->box.hInfo, rcBoxInfoPos.left, rcBoxInfoPos.top, GetRectW(&rcBoxInfoPos), GetRectH(&rcBoxInfoPos), TRUE);
    if (VIDEO_MP4_BOX_TYPE_mdat == boxType)
    {
        ShowWindow(mp4Info->box.trackId.hText, SW_SHOW);
        ShowWindow(mp4Info->box.trackId.hList, SW_SHOW);
        ShowWindow(mp4Info->box.sampleId.hText, SW_SHOW);
        ShowWindow(mp4Info->box.sampleId.hInput, SW_SHOW);
    }
    else
    {
        ShowWindow(mp4Info->box.trackId.hText, SW_HIDE);
        ShowWindow(mp4Info->box.trackId.hList, SW_HIDE);
        ShowWindow(mp4Info->box.sampleId.hText, SW_HIDE);
        ShowWindow(mp4Info->box.sampleId.hInput, SW_HIDE);
    }
}
static void UpdateBoxInfoWindow(PVIDEOMP4WNDEXTRA mp4Info, TVITEM* pItem)
{
    VIDEOMP4BOXTYPE eType = VIDEO_MP4_BOX_TYPE_CNT;
    PVIDEOMP4BOXTREE pBoxTree = (PVIDEOMP4BOXTREE)(pItem->lParam);

    if (((LPARAM)NULL) != pItem->lParam)
    {
        mp4Info->box.map.pChoosenBox = pBoxTree;
        eType = pBoxTree->mp4Box.eType;

        SendMessage(mp4Info->box.hInfo, WM_SETREDRAW, FALSE, 0);

        ListView_DeleteAllItems(mp4Info->box.hInfo);
        if (eType < VIDEO_MP4_BOX_TYPE_CNT)
        {
            UpdateBoxInfoWindow_SetTitle(pBoxTree, mp4Info->box.hInfo);
            if (NULL != pBoxTree->UpdateBoxInfoWindow) { pBoxTree->UpdateBoxInfoWindow(pBoxTree, mp4Info->box.hInfo, mp4Info); }
            else { (void)UpdateBoxInfoWindow_BlankBox(pBoxTree->mp4Box.pData, pBoxTree->mp4Box.nDataLen, 0, mp4Info->box.hInfo); }
        }
        else { UpdateBoxInfoWindow_Default(pBoxTree, mp4Info->box.hInfo); }

        SendMessage(mp4Info->box.hInfo, WM_SETREDRAW, TRUE, 0);
        InvalidateRect(mp4Info->box.hInfo, NULL, TRUE);
    }
}
static void InitTrackIdListWindow(PVIDEOMP4WNDEXTRA mp4Info)
{
    PVIDEOMP4TRACK pTrack = mp4Info->box.boxInfo.track.pList;
    HANSIZE nTrackCnt = mp4Info->box.boxInfo.track.nCnt;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE];
    VIDEOMP4TRACKHANDLERTYPE eType;
    HANINT nCursel = 0;

    for (HANSIZE iLoop = 0; iLoop < nTrackCnt; iLoop++)
    {
        eType = pTrack[iLoop].hdlr.handlerType.eType;
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("轨道%u（%s，样本数：" HANSIZE_PRINT_FORMAT "）"),
            pTrack[iLoop].tkhd.nTrackId, GetMP4_hdlr_HandlerTypeName(eType), pTrack[iLoop].nSampleCnt);
        ComboBoxAddString(mp4Info->box.trackId.hList, pText);
        if (VIDEO_MP4_TRACK_HANDLER_TYPE_VIDEO == eType) { nCursel = (HANINT)iLoop; }
    }
    ComboBoxSetCursel(mp4Info->box.trackId.hList, nCursel);
    SendMessage(mp4Info->box.sampleId.hUpDown, UDM_SETRANGE32, (WPARAM)0, (LPARAM)(pTrack[nCursel].nSampleCnt - 1));
}
static void ChooseBoxInfoTrackCallback(PVIDEOMP4WNDEXTRA mp4Info)
{
    PVIDEOMP4TRACK pTrack = mp4Info->box.boxInfo.track.pList;
    HANINT nTrackId;
    HANSIZE nTrackCnt;
    HANSIZE nSampleCnt;

    if (NULL != pTrack)
    {
        nTrackId = ComboBoxGetCursel(mp4Info->box.trackId.hList);
        nTrackCnt = mp4Info->box.boxInfo.track.nCnt;
        if ((0 <= nTrackId) && (nTrackId < (HANINT)nTrackCnt))
        {
            nSampleCnt = pTrack[nTrackId].nSampleCnt;
            SendMessage(mp4Info->box.sampleId.hUpDown, UDM_SETRANGE32, (WPARAM)0, (LPARAM)(nSampleCnt - 1));
            UpdateBoxInfoWindow_mdat(NULL, mp4Info->box.hInfo, mp4Info);
        }
    }
}

static BOOL GetMP4InfoSize(PVIDEOMP4WNDEXTRA mp4Info)
{
    BOOL bRet;
    VIDEOMP4BOXGETSIZE boxGetSize;
    HANSIZE nBoxCnt = 0;
    HANSIZE nDecoderCnt = 0;
    HANSIZE nDecoderSize = 0;

    boxGetSize.mp4Info = mp4Info;
    boxGetSize.pBoxCnt = &nBoxCnt;
    boxGetSize.pDecoderCnt = &nDecoderCnt;
    boxGetSize.pDecoderSize = &nDecoderSize;

    if (0 != ReadMP4Boxes(mp4Info->paramVideo.pData, mp4Info->paramVideo.nLen, ReadMP4BoxCallback_GetSize, &boxGetSize))
    {
        mp4Info->box.map.nCnt = nBoxCnt;
        mp4Info->decoder.table.nCnt = nDecoderCnt;
        mp4Info->decoder.buf.nSize = nDecoderSize;
        bRet = TRUE;
    }
    else { bRet = FALSE; }

    return bRet;
}
static BOOL MP4Process(PVIDEOMP4WNDEXTRA mp4Info)
{
    BOOL bRet;
    VIDEOMP4BOXPROCESS boxProcess;
    HANSIZE nBoxCnt = 0;
    HANSIZE nDecoderCnt = 0;
    HANSIZE nDecoderBufOffset = 0;

    mp4Info->box.boxInfo.track.nTargetId = 0;
    boxProcess.mp4Info = mp4Info;
    boxProcess.pTree = NULL;
    boxProcess.pBoxCnt = &nBoxCnt;
    boxProcess.pDecoderCnt = &nDecoderCnt;
    boxProcess.pDecoderBufOffset = &nDecoderBufOffset;

    if (0 != ReadMP4Boxes(mp4Info->paramVideo.pData, mp4Info->paramVideo.nLen, ReadMP4BoxCallback_Process, &boxProcess))
    {
        InitTrackIdListWindow(mp4Info);
        bRet = TRUE;
    }
    else { bRet = FALSE; }

    return bRet;
}
static void ReadMP4BoxCallback_GetSize(PCVIDEOMP4BOX pBox, void* pParam)
{
    PVIDEOMP4BOXGETSIZE pGetSize = (PVIDEOMP4BOXGETSIZE)pParam;
    PVIDEOMP4WNDEXTRA mp4Info = pGetSize->mp4Info;
    VIDEOMP4BOXTYPE boxType = pBox->eType;
    HANSIZE nReadSubBoxOffset;
    VIDEOMP4BOXINFO_tkhd tkhdInfo;
    VIDEOMP4BOXINFO_stsz stszInfo;

    /* 先累加数量 */
    (*(pGetSize->pBoxCnt))++;
    /* 处理子 BOX 前的预处理 */
    switch (boxType) {
        case VIDEO_MP4_BOX_TYPE_trak: { // 统计轨道数
            mp4Info->box.boxInfo.track.nCnt++;
        } break;
        case VIDEO_MP4_BOX_TYPE_tkhd: { // 获取分辨率
            if (TRUE == DecodeBoxData_tkhd(pBox, &tkhdInfo))
            {
                mp4Info->decoder.mediaInfo.pxResolution.pxWidth = tkhdInfo.nWidth.u32 / (uint32_t)0x10000;
                mp4Info->decoder.mediaInfo.pxResolution.pxHeight = tkhdInfo.nHeight.u32 / (uint32_t)0x10000;
            }
        } break;
        case VIDEO_MP4_BOX_TYPE_stsz: { // 统计样本数
            DecodeBoxData_stsz(pBox, &stszInfo);
            mp4Info->size.nSampleCnt += stszInfo.nSampleCnt;
        } break;
        default: { } break;
    }
    /* 读取子 BOX */
    if ((0 <= boxType) && (boxType < VIDEO_MP4_BOX_TYPE_CNT))
    {
        /* 尝试解析子 BOX，把数据段传下去 */
        nReadSubBoxOffset = sg_pMP4BoxTypeInfo[boxType].nReadSubBoxOffset;
        if (HAN_VIDEO_MP4_INVALID_SIZE != nReadSubBoxOffset)
        {
            switch (boxType) {
                case VIDEO_MP4_BOX_TYPE_stsd: { // stsd 要处理解码器，走单独的逻辑
                    ReadMP4BoxCallback_GetSize_stsd(pBox, pGetSize);
                } break;
                default: {
                    ReadMP4Boxes(&pBox->pData[nReadSubBoxOffset], pBox->nDataLen - nReadSubBoxOffset, ReadMP4BoxCallback_GetSize, pGetSize);
                } break;
            }
        }
    }
}
static void ReadMP4BoxCallback_GetSize_stsd(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXGETSIZE pGetSize)
{
    PVIDEOMP4WNDEXTRA mp4Info = pGetSize->mp4Info;
    const uint8_t* pData = pBox->pData;
    HANSIZE nOffset = sg_pMP4BoxTypeInfo[VIDEO_MP4_BOX_TYPE_stsd].nReadSubBoxOffset;
    HANPSIZE pDecoderCnt = pGetSize->pDecoderCnt;
    HANPSIZE pDecoderSize = pGetSize->pDecoderSize;
    VIDEOMP4BOXINFO_stsd stsdInfo;
    HANSIZE nSubBoxSize;

    DecodeBoxData_stsd(pBox, &stsdInfo);
    for (uint32_t iLoop = 0; iLoop < stsdInfo.nDecoderCnt; iLoop++)
    {
        nSubBoxSize = ReadMP4Data4ByteMSB(&pData[nOffset]);
        pGetSize->decoderType = GetVideoDecoderType(&pData[nOffset], nSubBoxSize);
        (*pDecoderSize) += GetVideoDecoderSize(&pBox->pData[nOffset], pBox->nDataLen - nOffset, pGetSize->decoderType, &(mp4Info->decoder.mediaInfo));
        ReadMP4Boxes(&pBox->pData[nOffset], pBox->nDataLen - nOffset, ReadMP4BoxCallback_GetSize_DecoderBox, pGetSize);
        nOffset += nSubBoxSize;
        (*pDecoderCnt)++;
    }
}
static void ReadMP4BoxCallback_GetSize_DecoderBox(PCVIDEOMP4BOX pBox, void* pParam)
{
    PVIDEOMP4BOXGETSIZE pGetSize = (PVIDEOMP4BOXGETSIZE)pParam;
    HANSIZE nReadSubBoxOffset = GetSubBoxOffset(pGetSize->decoderType, pBox->eType);

    (*(pGetSize->pBoxCnt))++;
    if (HAN_VIDEO_MP4_INVALID_SIZE != nReadSubBoxOffset)
    {
        ReadMP4Boxes(&pBox->pData[nReadSubBoxOffset], pBox->nDataLen - nReadSubBoxOffset, ReadMP4BoxCallback_GetSize_DecoderBox, pGetSize);
    }
}
static void ReadMP4BoxCallback_Process(PCVIDEOMP4BOX pBox, void* pParam)
{
    PVIDEOMP4BOXPROCESS pProcess = (PVIDEOMP4BOXPROCESS)pParam;
    VIDEOMP4BOXPROCESS boxProcess = *pProcess;
    PVIDEOMP4WNDEXTRA mp4Info = pProcess->mp4Info;
    VIDEOMP4BOXTYPE boxType = pBox->eType;
    HANSIZE nReadSubBoxOffset = sg_pMP4BoxTypeInfo[boxType].nReadSubBoxOffset;
    HANPSIZE pBoxCnt = pProcess->pBoxCnt;
    PVIDEOMP4BOXTREE pTree;
    PVIDEOMP4TRACK pTrack;
    
    /* 处理子 BOX 前的预处理（初始化） */
    switch (boxType) {
        case VIDEO_MP4_BOX_TYPE_trak: { ReadMP4BoxCallback_ProcessInittrak(mp4Info, &boxProcess); } break;
        default: { } break;
    }
    /* 先做好树 */
    pTree = &(mp4Info->box.map.pBoxTree)[*pBoxCnt];
    pTree->mp4Box = *pBox;
    pTree->pBoxName = sg_pMP4BoxTypeInfo[boxType].pName;
    if ((0 <= boxType) && (boxType < VIDEO_MP4_BOX_TYPE_CNT))
    {
        pTree->UpdateBoxInfoWindow = sg_pMP4BoxTypeInfo[boxType].UpdateBoxInfoWindow;
        pTree->pWinParam = mp4Info;
    }
    else { pTree->UpdateBoxInfoWindow = NULL; }
    pTree->pParent = pProcess->pTree;
    boxProcess.pTree = pTree;
    UpdateBoxTreeMap(&boxProcess, mp4Info);
    /* 指向下一个 BOX */
    (*pBoxCnt)++;
    /* 读取子 BOX */
    if ((0 <= boxType) && (boxType < VIDEO_MP4_BOX_TYPE_CNT))
    {
        /* 尝试解析子 BOX，把数据段传下去 */
        if (HAN_VIDEO_MP4_INVALID_SIZE != nReadSubBoxOffset)
        {
            switch (boxType) {
                case VIDEO_MP4_BOX_TYPE_stsd: { // stsd 要处理解码器，走单独的逻辑
                    ReadMP4BoxCallback_Process_stsd(pBox, &boxProcess);
                } break;
                default: {
                    ReadMP4Boxes(&pBox->pData[nReadSubBoxOffset], pBox->nDataLen - nReadSubBoxOffset, ReadMP4BoxCallback_Process, &boxProcess);
                } break;
            }
        }
    }
    /* 读完子 BOX 后的处理 */
    pTrack = &(mp4Info->box.boxInfo.track.pList)[mp4Info->box.boxInfo.track.nTargetId];
    switch (boxType) {
        case VIDEO_MP4_BOX_TYPE_trak: { (void)UpdateMP4Info_trakProcess(mp4Info); } break;
        case VIDEO_MP4_BOX_TYPE_tkhd: { (void)DecodeBoxData_tkhd(pBox, &(pTrack->tkhd)); } break;
        case VIDEO_MP4_BOX_TYPE_hdlr: { DecodeBoxData_hdlr(pBox, &(pTrack->hdlr)); } break;
        case VIDEO_MP4_BOX_TYPE_stts: { DecodeBoxData_stblSubBoxDefault(pBox, &(pTrack->stts)); } break;
        case VIDEO_MP4_BOX_TYPE_ctts: { DecodeBoxData_stblSubBoxDefault(pBox, &(pTrack->ctts)); } break;
        case VIDEO_MP4_BOX_TYPE_stss: { DecodeBoxData_stblSubBoxDefault(pBox, &(pTrack->stss)); } break;
        case VIDEO_MP4_BOX_TYPE_stsc: { DecodeBoxData_stblSubBoxDefault(pBox, &(pTrack->stsc)); } break;
        case VIDEO_MP4_BOX_TYPE_stsz: { DecodeBoxData_stsz(pBox, &(pTrack->stsz)); } break;
        case VIDEO_MP4_BOX_TYPE_stco: { DecodeBoxData_stco(pBox, &(pTrack->stco64)); } break;
        case VIDEO_MP4_BOX_TYPE_co64: { DecodeBoxData_co64(pBox, &(pTrack->stco64)); } break;
        default: { } break;
    }
}
static void ReadMP4BoxCallback_Process_stsd(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXPROCESS pProcess)
{
    PVIDEOMP4WNDEXTRA mp4Info = pProcess->mp4Info;
    const uint8_t* pData = pBox->pData;
    HANSIZE nOffset = sg_pMP4BoxTypeInfo[VIDEO_MP4_BOX_TYPE_stsd].nReadSubBoxOffset;
    HANSIZE nDecoderBufSize = mp4Info->decoder.buf.nSize;
    PVIDEOMP4TRACK pTrack = &(mp4Info->box.boxInfo.track.pList)[mp4Info->box.boxInfo.track.nTargetId];
    HANPSIZE pDecoderCnt = pProcess->pDecoderCnt;
    HANPSIZE pDecoderBufOffset = pProcess->pDecoderBufOffset;
    uint8_t** pDecoderTable = pTrack->decoder.table.pTable;
    VIDEOMP4BOXINFO_stsd stsdInfo;
    HANSIZE nSubBoxSize;
    HANSIZE nDecoderBufOffset;

    DecodeBoxData_stsd(pBox, &stsdInfo);
    for (uint32_t iLoop = 0; iLoop < stsdInfo.nDecoderCnt; iLoop++)
    {
        nSubBoxSize = ReadMP4Data4ByteMSB(&pData[nOffset]);
        pProcess->decoderType = GetVideoDecoderType(&pData[nOffset], nSubBoxSize);
        nDecoderBufOffset = *pDecoderBufOffset;
        pDecoderTable[iLoop] = &(mp4Info->decoder.buf.pData[nDecoderBufOffset]);
        (*pDecoderBufOffset) = InitVideoDecoder(&pBox->pData[nOffset], pBox->nDataLen - nOffset, pProcess->decoderType, pDecoderTable[iLoop], nDecoderBufSize - nDecoderBufOffset);
        pTrack->decoder.table.pType[iLoop] = pProcess->decoderType;
        pTrack->decoder.table.nCnt++;
        pProcess->pDecoder = pDecoderTable[iLoop];
        ReadMP4Boxes(&pBox->pData[nOffset], pBox->nDataLen - nOffset, ReadMP4BoxCallback_Process_DecoderBox, pProcess);
        nOffset += nSubBoxSize;
        (*pDecoderCnt)++;
    }
}
static void ReadMP4BoxCallback_Process_DecoderBox(PCVIDEOMP4BOX pBox, void* pParam)
{
    PVIDEOMP4BOXPROCESS pProcess = (PVIDEOMP4BOXPROCESS)pParam;
    VIDEOMP4BOXPROCESS boxProcess = *pProcess;
    PVIDEOMP4WNDEXTRA mp4Info = pProcess->mp4Info;
    HANSIZE nReadSubBoxOffset = GetSubBoxOffset(pProcess->decoderType, pBox->eType);
    HANPSIZE pBoxCnt = pProcess->pBoxCnt;
    PVIDEOMP4BOXTREE pTree;

    pTree = &(mp4Info->box.map.pBoxTree)[*pBoxCnt];
    pTree->mp4Box = *pBox;
    pTree->pBoxName = GetDecoderBoxName(pProcess->decoderType, pBox->eType);
    pTree->UpdateBoxInfoWindow = GetUpdateBoxInfoWindowCallback(pProcess->decoderType, pBox->eType);
    pTree->pWinParam = pProcess->pDecoder;
    pTree->pParent = pProcess->pTree;
    boxProcess.pTree = pTree;
    UpdateBoxTreeMap(&boxProcess, mp4Info);
    (*pBoxCnt)++;

    if (HAN_VIDEO_MP4_INVALID_SIZE != nReadSubBoxOffset)
    {
        ReadMP4Boxes(&pBox->pData[nReadSubBoxOffset], pBox->nDataLen - nReadSubBoxOffset, ReadMP4BoxCallback_Process_DecoderBox, &boxProcess);
    }
}

static inline HANINT GetBoxInfoWindowWidth(void)
{
    HANINT nRet = 0;
    for (VIDEOMP4BOXINFOHEADER iLoop = 0; iLoop < VIDEO_MP4_BOX_INFO_HEADER_CNT; iLoop++)
    {
        nRet += sg_pMP4BoxInfoHeaderWidth[iLoop];
    }
    return nRet;
}
static inline uint16_t ReadMP4Data2ByteMSB(const uint8_t pData[2])
{
    return (((uint16_t)pData[0] << 8) + (uint16_t)pData[1]);
}
static inline uint32_t ReadMP4Data3ByteMSB(const uint8_t pData[3])
{
    return (((uint32_t)pData[0] << 16) + ((uint32_t)pData[1] << 8) + (uint32_t)pData[2]);
}
static inline uint32_t ReadMP4Data4ByteMSB(const uint8_t pData[4])
{
    return (((uint32_t)pData[0] << 24) + ((uint32_t)pData[1] << 16) + ((uint32_t)pData[2] << 8) + (uint32_t)pData[3]);
}

static void ReadMP4BoxCallback_ProcessInittrak(PVIDEOMP4WNDEXTRA mp4Info, PVIDEOMP4BOXPROCESS pProcess)
{
    PVIDEOMP4TRACK pTrack = &(mp4Info->box.boxInfo.track.pList[mp4Info->box.boxInfo.track.nTargetId]);
    
    pTrack->stts.pRawData = NULL;
    pTrack->ctts.pRawData = NULL;
    pTrack->stss.pRawData = NULL;
    pTrack->stsc.pRawData = NULL;
    pTrack->stsz.pRawData = NULL;
    pTrack->stco64.pRawData = NULL;
    pTrack->decoder.table.pTable = &(mp4Info->decoder.table.pTable)[*(pProcess->pDecoderCnt)];
    pTrack->decoder.table.pType = &(mp4Info->decoder.table.pType)[*(pProcess->pDecoderCnt)];
}

static BOOL UpdateMP4Info_trakProcess(PVIDEOMP4WNDEXTRA mp4Info)
{
    BOOL bRet = TRUE;
    PVIDEOMP4TRACK pTrack = &(mp4Info->box.boxInfo.track.pList[mp4Info->box.boxInfo.track.nTargetId]);

    /* 解码样本信息 */
    pTrack->pSample = (PVIDEOMP4DECODEINFOSAMPLE)(mp4Info->pBufPtr);
    pTrack->nSampleCnt = pTrack->stsz.nSampleCnt;

    if ((FALSE == UpdateMP4Info_stts(mp4Info)) ||
        (FALSE == UpdateMP4Info_ctts(mp4Info)) ||
        (FALSE == UpdateMP4Info_stss(mp4Info)) ||
        (FALSE == UpdateMP4Info_stsc(mp4Info)) ||
        (FALSE == UpdateMP4Info_stsz(mp4Info)) ||
        (FALSE == UpdateMP4Info_stco64(mp4Info)))
    {
        bRet = FALSE;
    }

    mp4Info->pBufPtr = (uint8_t*)(&(pTrack->pSample)[pTrack->nSampleCnt]);
    (mp4Info->box.boxInfo.track.nTargetId)++;

    return bRet;
}
static BOOL UpdateMP4Info_stts(PVIDEOMP4WNDEXTRA mp4Info)
{
    BOOL bRet = TRUE;
    PVIDEOMP4TRACK pTrack = &(mp4Info->box.boxInfo.track.pList[mp4Info->box.boxInfo.track.nTargetId]);
    PVIDEOMP4DECODEINFOSAMPLE pSample = pTrack->pSample;
    HANSIZE nSampleCnt = pTrack->nSampleCnt;
    const uint8_t* pEntry = pTrack->stts.pEntry;
    HANSIZE nEntrySize = pTrack->stts.nEntrySize;
    HANSIZE nRecordCnt = pTrack->stts.nCnt;
    HANSIZE idSample = 0;
    HANSIZE nOffset = 0;
    HANSIZE timeDTS = 0;
    uint32_t timeCnt;
    uint32_t timeDelta;

    for (HANSIZE iLoop = 0; iLoop < nRecordCnt; iLoop++)
    {
        if (nEntrySize < (nOffset + 8)) { bRet = FALSE; }
        {
            timeCnt = ReadMP4Data4ByteMSB(&pEntry[nOffset]); nOffset += 4;
            timeDelta = ReadMP4Data4ByteMSB(&pEntry[nOffset]); nOffset += 4;
            for (uint32_t jLoop = 0; jLoop < timeCnt; jLoop++)
            {
                if (idSample < nSampleCnt)
                {
                    pSample[idSample].timeDTS = timeDTS;
                    pSample[idSample].timeDuration = timeDelta;
                    timeDTS += timeDelta;
                    idSample++;
                }
                else
                {
                    bRet = FALSE;
                    break;
                }
            }
        }
        if (FALSE == bRet) { break; }
    }

    if (idSample != pTrack->stsz.nSampleCnt) { bRet = FALSE; }
    
    return bRet;
}
static BOOL UpdateMP4Info_ctts(PVIDEOMP4WNDEXTRA mp4Info)
{
    BOOL bRet = TRUE;
    PVIDEOMP4TRACK pTrack = &(mp4Info->box.boxInfo.track.pList[mp4Info->box.boxInfo.track.nTargetId]);
    PVIDEOMP4DECODEINFOSAMPLE pSample = pTrack->pSample;
    HANSIZE nSampleCnt = pTrack->nSampleCnt;
    const uint8_t* pEntry = pTrack->ctts.pEntry;
    uint8_t nVersion = pTrack->ctts.fbVF.nVersion;
    HANSIZE nEntrySize = pTrack->ctts.nEntrySize;
    HANSIZE nRecordCnt = pTrack->ctts.nCnt;
    HANSIZE idSample = 0;
    HANSIZE nOffset = 0;
    uint32_t timeCnt;
    ULARGE_INTEGER timeOffset;

    if (NULL != pTrack->ctts.pRawData) /* 存在有效的 ctts BOX */
    {
        for (HANSIZE iLoop = 0; iLoop < nRecordCnt; iLoop++)
        {
            if (nEntrySize < (nOffset + 8)) { bRet = FALSE; }
            else
            {
                timeCnt = ReadMP4Data4ByteMSB(&pEntry[nOffset]); nOffset += 4;
                nOffset += DecodeBoxDataReadDataByVersion(&pEntry[nOffset], nVersion, &timeOffset);
                for (uint32_t jLoop = 0; jLoop < timeCnt; jLoop++)
                {
                    if (idSample < nSampleCnt)
                    {
                        pSample[idSample].timePTS = pSample[idSample].timeDTS + timeOffset.QuadPart;
                        idSample++;
                    }
                    else
                    {
                        bRet = FALSE;
                        break;
                    }
                }
            }
            if (FALSE == bRet) { break; }
        }
    }
    else
    {
        idSample = nSampleCnt;
        for (HANSIZE iLoop = 0; iLoop < idSample; iLoop++)
        {
            pSample[iLoop].timePTS = pSample[iLoop].timeDTS;
        }
    }

    if (idSample != nSampleCnt) { bRet = FALSE; }

    return bRet;
}
static BOOL UpdateMP4Info_stss(PVIDEOMP4WNDEXTRA mp4Info)
{
    BOOL bRet = TRUE;
    PVIDEOMP4TRACK pTrack = &(mp4Info->box.boxInfo.track.pList[mp4Info->box.boxInfo.track.nTargetId]);
    PVIDEOMP4DECODEINFOSAMPLE pSample = pTrack->pSample;
    HANSIZE nSampleCnt = pTrack->nSampleCnt;
    const uint8_t* pEntry = pTrack->stss.pEntry;
    HANSIZE nEntrySize = pTrack->ctts.nEntrySize;
    HANSIZE nRecordCnt;
    HANSIZE nOffset = 0;
    HANSIZE idKeyFrame;
    uint32_t nPrevKeyFrame;
    uint32_t nNextKeyFrame;
    uint32_t idPrevKeyFrame;
    uint32_t idNextKeyFrame = 0;

    if (NULL == pTrack->stss.pRawData)
    {
        for (HANSIZE iLoop = 0; iLoop < nSampleCnt; iLoop++)
        {
            pSample[iLoop].idKeyFrame = iLoop;
        }
    }
    else
    {
        nRecordCnt = pTrack->stss.nCnt;
        if (0 == nRecordCnt)
        {
            for (HANSIZE iLoop = 0; iLoop < nSampleCnt; iLoop++)
            {
                pSample[iLoop].idKeyFrame = HAN_VIDEO_MP4_INVALID_SIZE;
            }
        }
        else
        {
            nPrevKeyFrame = ReadMP4Data4ByteMSB(&pEntry[nOffset]); nOffset += 4;
            if ((0 == nPrevKeyFrame) || (nSampleCnt < nPrevKeyFrame)) { bRet = FALSE; }
            else
            {
                // 0 ~ 第一个 ID
                idPrevKeyFrame = nPrevKeyFrame - 1;
                for (HANSIZE iLoop = 0; iLoop < idPrevKeyFrame; iLoop++)
                {
                    pSample[iLoop].idKeyFrame = HAN_VIDEO_MP4_INVALID_SIZE;
                }
                idKeyFrame = idPrevKeyFrame;
                // 第一个 ID ~ 最后一个 ID
                for (HANSIZE iLoop = 1; iLoop < nRecordCnt; iLoop++)
                {
                    if (nEntrySize < (nOffset + 4)) { bRet = FALSE; }
                    if (TRUE == bRet)
                    {
                        nNextKeyFrame = ReadMP4Data4ByteMSB(&pEntry[nOffset]); nOffset += 4;
                        if ((nSampleCnt < nNextKeyFrame) || (nNextKeyFrame <= nPrevKeyFrame)) { bRet = FALSE; } // 《根据视频标准，章节 8.6.2.1 规定，关键帧 ID 必须递增》
                        else
                        {
                            idPrevKeyFrame = nPrevKeyFrame - 1;
                            idNextKeyFrame = nNextKeyFrame - 1;
                            for (HANSIZE jLoop = idPrevKeyFrame; jLoop < idNextKeyFrame; jLoop++)
                            {
                                pSample[jLoop].idKeyFrame = idKeyFrame;
                            }
                        }
                    }
                    if (FALSE == bRet) { break; }
                    idKeyFrame = idNextKeyFrame;
                    nPrevKeyFrame = nNextKeyFrame;
                }
                if (TRUE == bRet) // 最后一个 ID ~ 尾
                {
                    if (idNextKeyFrame < nSampleCnt)
                    {
                        for (HANSIZE iLoop = idNextKeyFrame; iLoop < nSampleCnt; iLoop++)
                        {
                            pSample[iLoop].idKeyFrame = idKeyFrame;
                        }
                    }
                }
            }
        }
    }

    return bRet;
}
static BOOL UpdateMP4Info_stsc(PVIDEOMP4WNDEXTRA mp4Info)
{
    BOOL bRet = TRUE;
    PVIDEOMP4TRACK pTrack = &(mp4Info->box.boxInfo.track.pList[mp4Info->box.boxInfo.track.nTargetId]);
    PVIDEOMP4DECODEINFOSAMPLE pSample = pTrack->pSample;
    HANSIZE nSampleCnt = pTrack->nSampleCnt;
    const uint8_t* pEntry = pTrack->stsc.pEntry;
    HANSIZE nEntrySize = pTrack->stsc.nEntrySize;
    HANSIZE idSample = 0;
    HANSIZE nOffset = 0;
    uint32_t idChunk = 0;
    uint32_t nRecordCnt = pTrack->stsc.nCnt;
    uint32_t nLoopCnt = nRecordCnt - 1;
    uint32_t nPrevFirstChunk;
    uint32_t nNextFirstChunk;
    uint32_t nPrevSampleCnt;
    uint32_t nNextSampleCnt;
    uint32_t nPrevDsc;
    uint32_t nNextDsc;
    uint32_t idDescription;
    uint32_t nChunkCntInGroup;

    nNextFirstChunk = ReadMP4Data4ByteMSB(&pEntry[nOffset]); nOffset += 4;
    nNextSampleCnt = ReadMP4Data4ByteMSB(&pEntry[nOffset]); nOffset += 4;
    nNextDsc = ReadMP4Data4ByteMSB(&pEntry[nOffset]); nOffset += 4;
    if (1 == nNextFirstChunk) // 《根据视频标准，章节 8.7.4.3 规定，第一个 first_chunk 必须是 1 》
    {
        for (uint32_t iLoop = 0; iLoop < nLoopCnt; iLoop++)
        {
            if (nEntrySize < (nOffset + 12)) { bRet = FALSE; }
            nPrevFirstChunk = nNextFirstChunk;
            nPrevSampleCnt = nNextSampleCnt;
            nPrevDsc = nNextDsc;
            nNextFirstChunk = ReadMP4Data4ByteMSB(&pEntry[nOffset]); nOffset += 4;
            nNextSampleCnt = ReadMP4Data4ByteMSB(&pEntry[nOffset]); nOffset += 4;
            nNextDsc = ReadMP4Data4ByteMSB(&pEntry[nOffset]); nOffset += 4;
            if ((nSampleCnt < nNextFirstChunk) || (nNextFirstChunk <= nPrevFirstChunk)) { bRet = FALSE; }
            if (TRUE == bRet)
            {
                nChunkCntInGroup = nNextFirstChunk - nPrevFirstChunk;
                idDescription = nPrevDsc - 1;
                for (uint32_t jLoop = 0; jLoop < nChunkCntInGroup; jLoop++)
                {
                    for (uint32_t kLoop = 0; kLoop < nPrevSampleCnt; kLoop++)
                    {
                        if (idSample < nSampleCnt)
                        {
                            pSample[idSample].idChunkGroup = iLoop;
                            pSample[idSample].idChunk = idChunk;
                            pSample[idSample].idDescription = idDescription;
                            idSample++;
                        }
                        else
                        {
                            bRet = FALSE;
                            break;
                        }
                    }
                    if (FALSE == bRet) { break; }
                    idChunk++;
                }
            }
        }
        if (TRUE == bRet)
        {
            idDescription = nNextDsc;
            while (idSample < nSampleCnt)
            {
                for (HANSIZE iLoop = 0; iLoop < nNextSampleCnt; iLoop++)
                {
                    if (idSample < nSampleCnt)
                    {
                        pSample[idSample].idChunkGroup = nLoopCnt;
                        pSample[idSample].idChunk = idChunk;
                        pSample[idSample].idDescription = idDescription;
                        idSample++;
                    }
                    else
                    {
                        bRet = FALSE;
                        break;
                    }
                }
                idChunk++;
            }
        }
    }
    else { bRet = FALSE; }

    return bRet;
}
static BOOL UpdateMP4Info_stsz(PVIDEOMP4WNDEXTRA mp4Info)
{
    BOOL bRet = TRUE;
    PVIDEOMP4TRACK pTrack = &(mp4Info->box.boxInfo.track.pList[mp4Info->box.boxInfo.track.nTargetId]);
    PVIDEOMP4DECODEINFOSAMPLE pSample = pTrack->pSample;
    const uint8_t* pEntry = pTrack->stsz.pEntry;
    HANSIZE nEntrySize = pTrack->stsz.nEntrySize;
    HANSIZE nSampleCnt = pTrack->stsz.nSampleCnt;
    HANSIZE nOffset = 0;

    if (NULL != pEntry)
    {
        for (HANSIZE iLoop = 0; iLoop < nSampleCnt; iLoop++)
        {
            if (nEntrySize < (nOffset + 4))
            {
                bRet = FALSE;
                break;
            }
            pSample[iLoop].nSize = ReadMP4Data4ByteMSB(&pEntry[nOffset]);
            nOffset += 4;
        }
    }
    else
    {
        for (HANSIZE iLoop = 0; iLoop < nSampleCnt; iLoop++)
        {
            pSample[iLoop].nSize = pTrack->stsz.nSampleSize;
        }
    }

    return bRet;
}
static BOOL UpdateMP4Info_stco64(PVIDEOMP4WNDEXTRA mp4Info)
{
    BOOL bRet = TRUE;
    PVIDEOMP4TRACK pTrack = &(mp4Info->box.boxInfo.track.pList[mp4Info->box.boxInfo.track.nTargetId]);
    PVIDEOMP4DECODEINFOSAMPLE pSample = pTrack->pSample;
    HANSIZE nSampleCnt = pTrack->nSampleCnt;
    uint8_t nDataSize = pTrack->stco64.nDataSize;
    const uint8_t* pEntry = pTrack->stco64.pEntry;
    HANSIZE nEntrySize = pTrack->stco64.nEntrySize;
    HANSIZE nOffset;
    HANSIZE sOffset;
    uint32_t idChunk = 0;
    ULARGE_INTEGER ulOffset;
    uint8_t nVersion;

    switch (nDataSize) {
        case 4: { nVersion = 0; } break;
        case 8: { nVersion = 1; } break;
        default: { bRet = FALSE; } break;
    }
    if (TRUE == bRet)
    {
        nOffset = DecodeBoxDataReadDataByVersion(pEntry, nVersion, &ulOffset);
        sOffset = ulOffset.QuadPart;
        for (HANSIZE iLoop = 0; iLoop < nSampleCnt; iLoop++)
        {
            if (idChunk != pSample[iLoop].idChunk)
            {
                if (nEntrySize < (nOffset + nDataSize))
                {
                    bRet = FALSE;
                    break;
                }
                nOffset += DecodeBoxDataReadDataByVersion(&pEntry[nOffset], nVersion, &ulOffset);
                sOffset = ulOffset.QuadPart;
                idChunk = pSample[iLoop].idChunk;
            }
            pSample[iLoop].sOffset = sOffset;
            sOffset += pSample[iLoop].nSize;
        }
    }

    return bRet;
}

static HANINT UpdateBoxInfoWindow_FullBoxVersionFlags(HANINT nId, HWND hListView, PVIDEOMP4BOXINFOFULLBOXVERFLAGS pFB)
{
    HANINT nRet = nId;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE];

    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u（%s）"), pFB->nVersion, GetMP4BoxVersionName(pFB->nVersion));
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("版本"), pText, nRet, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%06X"), pFB->cFlags);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("标志"), pText, nRet, hListView);

    return nRet;
}
static HANINT UpdateBoxInfoWindow_BlankBox(const uint8_t* pData, HANSIZE nLen, HANINT nId, HWND hListView)
{
    HANINT nRet = nId;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    
    /* 长度 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT(HANSIZE_PRINT_FORMAT), nLen);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("长度"), pText, nRet, hListView);
    /* 数据 */
    VideoMP4PrintHexData(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, pData, nLen);
    nRet = UpdateBoxInfoWindow_InsertLine(TEXT("数据"), pText, nRet, hListView);

    return nRet;
}
static void UpdateBoxInfoWindow_PrintTimeDuration(HANPSTR pText, HANSIZE nLen, PULARGE_INTEGER pTimeDuration)
{
    uint32_t nHour = (uint32_t)(pTimeDuration->QuadPart / 3600);
    uint8_t nMinute = (uint8_t)((pTimeDuration->QuadPart % 3600) / 60);
    uint8_t nSecond = (uint8_t)(pTimeDuration->QuadPart % 60);

    HAN_snprintf(pText, nLen, TEXT("%u:%u:%u"), nHour, nMinute, nSecond);
}
static void UpdateBoxInfoWindow_SetTitle(PVIDEOMP4BOXTREE pBoxTree, HWND hListView)
{
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    LVCOLUMN lvTitle = { .mask = LVCF_TEXT, };

    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, pBoxTree->mp4Box.pType);
    lvTitle.pszText = pText;
    ListView_SetColumn(hListView, VIDEO_MP4_BOX_INFO_HEADER_FIELD, &lvTitle);
    lvTitle.pszText = pBoxTree->pBoxName;
    ListView_SetColumn(hListView, VIDEO_MP4_BOX_INFO_HEADER_VALUE, &lvTitle);
}
static void UpdateBoxInfoWindow_Default(PVIDEOMP4BOXTREE pBoxTree, HWND hListView)
{
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE];
    LVCOLUMN lvTitle = { .mask = LVCF_TEXT, };
    
    PrintMP4Text4Bytes(pText, pBoxTree->mp4Box.pType);
    lvTitle.pszText = pText;
    ListView_SetColumn(hListView, VIDEO_MP4_BOX_INFO_HEADER_FIELD, &lvTitle);
    lvTitle.pszText = TEXT("不支持该数据块");
    ListView_SetColumn(hListView, VIDEO_MP4_BOX_INFO_HEADER_VALUE, &lvTitle);

    (void)UpdateBoxInfoWindow_BlankBox(pBoxTree->mp4Box.pData, pBoxTree->mp4Box.nDataLen, 0, hListView);
}
static void UpdateBoxInfoWindow_ftyp(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam)
{
    (void)pParam;
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANCHAR pField[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANPSTR pCompatibleBrandsName;
    uint32_t nVersion;
    HANINT nId = 0;
    HANINT nNameId;
    
    /* major brand */
    PrintMP4Text4Bytes(pText, &pData[0]);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_ftyp_FieldName(VIDEO_MP4_ftyp_BOX_FIELD_MAJOR_BRAND), pText, nId, hListView);
    /* minor version */
    nVersion = ReadMP4Data4ByteMSB(&pData[4]);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%08X"), nVersion);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_ftyp_FieldName(VIDEO_MP4_ftyp_BOX_FIELD_MINOR_VERSION), pText, nId, hListView);
    /* compatible brands */
    pCompatibleBrandsName = GetMP4_ftyp_FieldName(VIDEO_MP4_ftyp_BOX_FIELD_COMPATIBLE_BRANDS);
    nNameId = 0;
    for (HANSIZE nOffset = 8; nOffset < pBoxTree->mp4Box.nDataLen; nOffset += 4)
    {
        HAN_snprintf(pField, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%s%d"), pCompatibleBrandsName, nNameId + 1);
        PrintMP4Text4Bytes(pText, &pData[nOffset]);
        nId = UpdateBoxInfoWindow_InsertLine(pField, pText, nId, hListView);
        nNameId++;
    }
}
static void UpdateBoxInfoWindow_free(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam)
{
    (void)pParam;
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HANSIZE nLen = pBoxTree->mp4Box.nDataLen;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    CHAR pBuf[HAN_VIDEO_MP4_TEXT_BUF_SIZE];
    HANINT nId = 0;

    /* 长度 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT(HANSIZE_PRINT_FORMAT), nLen);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_free_FieldName(VIDEO_MP4_free_BOX_FIELD_LEN), pText, nId, hListView);
    /* 数据 */
    VideoMP4PrintHexData(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, pData, nLen);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_free_FieldName(VIDEO_MP4_free_BOX_FIELD_DATA), pText, nId, hListView);
    /* 文本 */
    if (HAN_VIDEO_MP4_TEXT_BUF_SIZE <= nLen) { nLen = HAN_VIDEO_MP4_TEXT_BUF_SIZE - 1; }
    memcpy(pBuf, pData, nLen);
    pBuf[nLen] = '\0';
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, pBuf);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_free_FieldName(VIDEO_MP4_free_BOX_FIELD_TEXT), pText, nId, hListView);
}
static void UpdateBoxInfoWindow_mdat(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam)
{
    (void)pBoxTree;
    (void)hListView;
    PVIDEOMP4WNDEXTRA mp4Info = (PVIDEOMP4WNDEXTRA)pParam;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE];
    HANINT nTrackId = ComboBoxGetCursel(mp4Info->box.trackId.hList);
    PVIDEOMP4TRACK pTrack = mp4Info->box.boxInfo.track.pList;
    HANSIZE nSampleCnt;
    HANSIZE idSample;
    HANSIZE nTrackCnt;
    DWORD nTextCursel;

    if (NULL != pTrack)
    {
        nTrackCnt = mp4Info->box.boxInfo.track.nCnt;
        if ((0 <= nTrackId) && (nTrackId < (HANINT)nTrackCnt))
        {
            mp4Info->box.sampleId.bLock = TRUE;

            pTrack = &pTrack[nTrackId];
            nSampleCnt = pTrack->nSampleCnt;

            GetWindowText(mp4Info->box.sampleId.hInput, pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE);
            idSample = HAN_strtoul(pText, NULL, 10);
            if (nSampleCnt <= idSample) { idSample = nSampleCnt - 1; }
            HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT(HANSIZE_PRINT_FORMAT), idSample);
            pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE - 1] = TEXT('\0');
            SendMessage(mp4Info->box.sampleId.hInput, EM_GETSEL, (WPARAM)&nTextCursel, (LPARAM)NULL);
            SetWindowText(mp4Info->box.sampleId.hInput, pText);
            SendMessage(mp4Info->box.sampleId.hInput, EM_SETSEL, nTextCursel, nTextCursel);

            ListView_DeleteAllItems(mp4Info->box.hInfo);
            UpdateBoxInfoWindow_mdatSample(nTrackId, idSample, mp4Info);

            mp4Info->box.sampleId.bLock = FALSE;
        }
    }
}
static void UpdateBoxInfoWindow_mvhd(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam)
{
    (void)pParam;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_mvhd mvhdInfo;

    if (TRUE == DecodeBoxData_mvhd(&(pBoxTree->mp4Box), &mvhdInfo))
    {
        nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &(mvhdInfo.fbVF));
        /* 创建时间 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u/%u/%u %02u:%02u:%02u"),
            mvhdInfo.creationTime.stTime.wYear, mvhdInfo.creationTime.stTime.wMonth, mvhdInfo.creationTime.stTime.wDay,
            mvhdInfo.creationTime.stTime.wHour, mvhdInfo.creationTime.stTime.wMinute, mvhdInfo.creationTime.stTime.wSecond);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mvhd_FieldName(VIDEO_MP4_mvhd_BOX_FIELD_CREATION_TIME), pText, nId, hListView);
        /* 修改时间 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u/%u/%u %02u:%02u:%02u"),
            mvhdInfo.modificationTime.stTime.wYear, mvhdInfo.modificationTime.stTime.wMonth, mvhdInfo.modificationTime.stTime.wDay,
            mvhdInfo.modificationTime.stTime.wHour, mvhdInfo.modificationTime.stTime.wMinute, mvhdInfo.modificationTime.stTime.wSecond);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mvhd_FieldName(VIDEO_MP4_mvhd_BOX_FIELD_MODIFICATION_TIME), pText, nId, hListView);
        /* 时长 */
        UpdateBoxInfoWindow_PrintTimeDuration(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, &mvhdInfo.timeDuration.sTimeDuration);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mvhd_FieldName(VIDEO_MP4_mvhd_BOX_FIELD_DURATION), pText, nId, hListView);
        /* 播放速率 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%.1lf"), mvhdInfo.nRate.nPhy);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mvhd_FieldName(VIDEO_MP4_mvhd_BOX_FIELD_RATE), pText, nId, hListView);
        /* 音量 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%.1lf"), mvhdInfo.nVolume.nPhy);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mvhd_FieldName(VIDEO_MP4_mvhd_BOX_FIELD_VOLUME), pText, nId, hListView);
        /* 视频变换矩阵 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), mvhdInfo.pMatrix[0]);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mvhd_FieldName(VIDEO_MP4_mvhd_BOX_FIELD_MATRIX), pText, nId, hListView);
        /* 下一个轨道的 ID */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), mvhdInfo.nNextTrackId);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mvhd_FieldName(VIDEO_MP4_mvhd_BOX_FIELD_NEXT_TRACK_ID), pText, nId, hListView);
    }
}
static void UpdateBoxInfoWindow_iods(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam)
{
    (void)pParam;
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFOFULLBOXVERFLAGS fbVF;

    DecodeBoxData_FullBoxVersionFlags(pData, &fbVF);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &fbVF);
    /* 数据 */
    VideoMP4PrintHexData(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, &pData[4], pBoxTree->mp4Box.nDataLen - 4);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_iods_FieldName(VIDEO_MP4_iods_BOX_FIELD_DATA), pText, nId, hListView);
}
static void UpdateBoxInfoWindow_tkhd(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam)
{
    (void)pParam;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANCHAR pFlagsName[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_tkhd tkhdInfo;

    if (TRUE == DecodeBoxData_tkhd(&(pBoxTree->mp4Box), &tkhdInfo))
    {
        /* 版本 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u（%s）"), tkhdInfo.fbVF.nVersion, GetMP4BoxVersionName(tkhdInfo.fbVF.nVersion));
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(VIDEO_MP4_tkhd_BOX_FIELD_VERSION), pText, nId, hListView);
        /* 标志 */
        GetMP4_tkhd_FlagsName(tkhdInfo.fbVF.cFlags, pFlagsName, HAN_VIDEO_MP4_TEXT_BUF_SIZE);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%06X（%s）"), tkhdInfo.fbVF.cFlags, pFlagsName);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(VIDEO_MP4_tkhd_BOX_FIELD_FLAGS), pText, nId, hListView);
        /* 创建时间 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u/%u/%u %02u:%02u:%02u"),
            tkhdInfo.creationTime.stTime.wYear, tkhdInfo.creationTime.stTime.wMonth, tkhdInfo.creationTime.stTime.wDay,
            tkhdInfo.creationTime.stTime.wHour, tkhdInfo.creationTime.stTime.wMinute, tkhdInfo.creationTime.stTime.wSecond);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(VIDEO_MP4_tkhd_BOX_FIELD_CREATION_TIME), pText, nId, hListView);
        /* 修改时间 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u/%u/%u %02u:%02u:%02u"),
            tkhdInfo.modificationTime.stTime.wYear, tkhdInfo.modificationTime.stTime.wMonth, tkhdInfo.modificationTime.stTime.wDay,
            tkhdInfo.modificationTime.stTime.wHour, tkhdInfo.modificationTime.stTime.wMinute, tkhdInfo.modificationTime.stTime.wSecond);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(VIDEO_MP4_tkhd_BOX_FIELD_MODIFICATION_TIME), pText, nId, hListView);
        /* 轨道ID */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), tkhdInfo.nTrackId);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(VIDEO_MP4_tkhd_BOX_FIELD_TRACK_ID), pText, nId, hListView);
        /* 轨道时长 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%" PRIu64), tkhdInfo.nDuration.QuadPart);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(VIDEO_MP4_tkhd_BOX_FIELD_DURATION), pText, nId, hListView);
        /* 视频层 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), tkhdInfo.nLayer);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(VIDEO_MP4_tkhd_BOX_FIELD_LAYER), pText, nId, hListView);
        /* 替代组 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), tkhdInfo.nAlternateGroup);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(VIDEO_MP4_tkhd_BOX_FIELD_ALTERNATE_GROUP), pText, nId, hListView);
        /* 音量 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%.1lf"), tkhdInfo.nVolume.nPhy);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(VIDEO_MP4_tkhd_BOX_FIELD_VOLUME), pText, nId, hListView);
        /* 变换矩阵 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), tkhdInfo.pMatrix[0]);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(VIDEO_MP4_tkhd_BOX_FIELD_MATRIX), pText, nId, hListView);
        /* 分辨率 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%g * %g"), tkhdInfo.nWidth.nPhy, tkhdInfo.nHeight.nPhy);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(VIDEO_MP4_tkhd_BOX_FIELD_RESOLUTION), pText, nId, hListView);
    }
}
static void UpdateBoxInfoWindow_elst(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam)
{
    (void)pParam;
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFOFULLBOXVERFLAGS fbVF;
    uint32_t nListCnt;
    VIDEOMP4BOXINFO_elst elstInfo;
    HANSIZE nOffset = 0;
    
    DecodeBoxData_FullBoxVersionFlags(pData, &fbVF);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &fbVF);
    /* 条目数 */
    nListCnt = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nListCnt);
    nId = UpdateBoxInfoWindow_InsertLine(TEXT("条目数"), pText, nId, hListView);

    for (uint32_t iLoop = 0; iLoop < nListCnt; iLoop++)
    {
        nId = UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("条目%d"), iLoop + 1);
        nId = UpdateBoxInfoWindow_InsertLine(pText, TEXT(""), nId, hListView);

        nOffset += DecodeBoxData_elst(&pData[nOffset], fbVF.nVersion, &elstInfo);
        /* 段时长 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%" PRIu64), elstInfo.nSegmentDuration.QuadPart);
        nId = UpdateBoxInfoWindow_InsertLine(TEXT("段时长"), pText, nId, hListView);
        /* 时间点 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%" PRIu64), elstInfo.nMediaTime.QuadPart);
        nId = UpdateBoxInfoWindow_InsertLine(TEXT("时间点"), pText, nId, hListView);
        /* 播放速率 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%.1lf"), elstInfo.mediaRate.nPhy);
        nId = UpdateBoxInfoWindow_InsertLine(TEXT("播放速率"), pText, nId, hListView);
    }
}
static void UpdateBoxInfoWindow_mdhd(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam)
{
    (void)pParam;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    CHAR pLanguage[3];
    HANINT nId = 0;
    VIDEOMP4BOXINFO_mdhd mdhdInfo;

    if (TRUE == DecodeBoxData_mdhd(&(pBoxTree->mp4Box), &mdhdInfo))
    {
        nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &(mdhdInfo.fbVF));
        /* 创建时间 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u/%u/%u %02u:%02u:%02u"),
            mdhdInfo.creationTime.stTime.wYear, mdhdInfo.creationTime.stTime.wMonth, mdhdInfo.creationTime.stTime.wDay,
            mdhdInfo.creationTime.stTime.wHour, mdhdInfo.creationTime.stTime.wMinute, mdhdInfo.creationTime.stTime.wSecond);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mdhd_FieldName(VIDEO_MP4_mdhd_BOX_FIELD_CREATION_TIME), pText, nId, hListView);
        /* 修改时间 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u/%u/%u %02u:%02u:%02u"),
            mdhdInfo.modificationTime.stTime.wYear, mdhdInfo.modificationTime.stTime.wMonth, mdhdInfo.modificationTime.stTime.wDay,
            mdhdInfo.modificationTime.stTime.wHour, mdhdInfo.modificationTime.stTime.wMinute, mdhdInfo.modificationTime.stTime.wSecond);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mdhd_FieldName(VIDEO_MP4_mdhd_BOX_FIELD_MODIFICATION_TIME), pText, nId, hListView);
        /* 时长 */
        UpdateBoxInfoWindow_PrintTimeDuration(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, &mdhdInfo.timeDuration.sTimeDuration);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mdhd_FieldName(VIDEO_MP4_mdhd_BOX_FIELD_DURATION), pText, nId, hListView);
        /* 语言 */
        pLanguage[0] = mdhdInfo.pLanguage[0];
        pLanguage[1] = mdhdInfo.pLanguage[1];
        pLanguage[2] = '\0';
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, pLanguage);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mdhd_FieldName(VIDEO_MP4_mdhd_BOX_FIELD_LANGUAGE), pText, nId, hListView);
    }
}
static void UpdateBoxInfoWindow_hdlr(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam)
{
    (void)pParam;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_hdlr hdlrInfo;

    DecodeBoxData_hdlr(&(pBoxTree->mp4Box), &hdlrInfo);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &(hdlrInfo.fbVF));
    /* 处理器类型 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT TEXT("（%s）"),
        hdlrInfo.handlerType.pType,
        GetMP4_hdlr_HandlerTypeName(hdlrInfo.handlerType.eType));
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_hdlr_FieldName(VIDEO_MP4_hdlr_BOX_FIELD_HANDLER_TYPE), pText, nId, hListView);
    /* 名称 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, hdlrInfo.pName);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_hdlr_FieldName(VIDEO_MP4_hdlr_BOX_FIELD_NAME), pText, nId, hListView);
}
static void UpdateBoxInfoWindow_vmhd(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam)
{
    (void)pParam;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_vmhd vmhdInfo;

    DecodeBoxData_vmhd(&(pBoxTree->mp4Box), &vmhdInfo);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &(vmhdInfo.fbVF));
    /* 处理器类型 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%04X（%s）"), vmhdInfo.cGraphicsMode, GetMP4_vmhd_GraphicsModeName(vmhdInfo.cGraphicsMode));
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_vmhd_FieldName(VIDEO_MP4_vmhd_BOX_FIELD_GRAPHICS_MODE), pText, nId, hListView);
    /* 名称 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("（%u, %u, %u）"), vmhdInfo.pOpColor[0], vmhdInfo.pOpColor[1], vmhdInfo.pOpColor[2]);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_vmhd_FieldName(VIDEO_MP4_vmhd_BOX_FIELD_OP_COLOR), pText, nId, hListView);
}
static void UpdateBoxInfoWindow_dref(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam)
{
    (void)pParam;
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFOFULLBOXVERFLAGS fbVF;
    uint32_t nListCnt;
    HANSIZE nOffset = 0;

    DecodeBoxData_FullBoxVersionFlags(pData, &fbVF);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &fbVF);
    /* 条目数 */
    nListCnt = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nListCnt);
    nId = UpdateBoxInfoWindow_InsertLine(TEXT("条目数"), pText, nId, hListView);
}
static void UpdateBoxInfoWindow_url_(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam)
{
    (void)pParam;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_url_ urlInfo;

    if (TRUE == DecodeBoxData_url_(&(pBoxTree->mp4Box), &urlInfo))
    {
        /* 版本 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u（%s）"), urlInfo.fbVF.nVersion, GetMP4BoxVersionName(urlInfo.fbVF.nVersion));
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_url__FieldName(VIDEO_MP4_url__BOX_FIELD_VERSION), pText, nId, hListView);
        /* 标志 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%06X（%s）"), urlInfo.fbVF.cFlags, GetMP4_url__FlagsName(urlInfo.fbVF.cFlags));
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_url__FieldName(VIDEO_MP4_url__BOX_FIELD_FLAGS), pText, nId, hListView);
        /* URL */
        if (0x000000 == urlInfo.fbVF.cFlags) { HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, urlInfo.pUrl); }
        else { HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("本文件")); }
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_url__FieldName(VIDEO_MP4_url__BOX_FIELD_LOCATION), pText, nId, hListView);
    }
}
static void UpdateBoxInfoWindow_stsd(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam)
{
    (void)pParam;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_stsd stsdInfo;
    
    DecodeBoxData_stsd(&(pBoxTree->mp4Box), &stsdInfo);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &(stsdInfo.fbVF));
    /* 解码器数量 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), stsdInfo.nDecoderCnt);
    nId = UpdateBoxInfoWindow_InsertLine(TEXT("解码器数量"), pText, nId, hListView);
}
static void UpdateBoxInfoWindow_stts(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam)
{
    (void)pParam;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_stblSubBoxDefault sttsInfo;
    HANSIZE nLen = pBoxTree->mp4Box.nDataLen - 8;
    HANSIZE nOffset;
    uint32_t nSampleCount;
    uint32_t nSampleDelta;

    DecodeBoxData_stblSubBoxDefault(&(pBoxTree->mp4Box), &sttsInfo);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &(sttsInfo.fbVF));
    nOffset = 0;
    for (uint32_t iLoop = 0; iLoop < sttsInfo.nCnt; iLoop++)
    {
        if (nOffset < nLen)
        {
            if (iLoop < HAN_VIDEO_MP4_BOX_ENTRIES_INFO_MAX_CNT)
            {
                nSampleCount = ReadMP4Data4ByteMSB(&(sttsInfo.pEntry)[nOffset]); nOffset += 4;
                nSampleDelta = ReadMP4Data4ByteMSB(&(sttsInfo.pEntry)[nOffset]); nOffset += 4;
                nId = UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%s%u"), GetMP4_stts_FieldName(VIDEO_MP4_stts_BOX_FIELD_ENTRY), iLoop + 1);
                nId = UpdateBoxInfoWindow_InsertLine(pText, TEXT(""), nId, hListView);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nSampleCount);
                nId = UpdateBoxInfoWindow_InsertLine(GetMP4_stts_FieldName(VIDEO_MP4_stts_BOX_FIELD_SAMPLE_COUNT), pText, nId, hListView);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nSampleDelta);
                nId = UpdateBoxInfoWindow_InsertLine(GetMP4_stts_FieldName(VIDEO_MP4_stts_BOX_FIELD_SAMPLE_DELTA), pText, nId, hListView);
            }
            else
            {
                nId = UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("剩余%u个条目..."), sttsInfo.nCnt - HAN_VIDEO_MP4_BOX_ENTRIES_INFO_MAX_CNT);
                nId = UpdateBoxInfoWindow_InsertLine(pText, TEXT(""), nId, hListView);
                break;
            }
        }
    }
}
static void UpdateBoxInfoWindow_ctts(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam)
{
    (void)pParam;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_stblSubBoxDefault cttsInfo;
    HANSIZE nLen = pBoxTree->mp4Box.nDataLen - 8;
    HANSIZE nOffset;
    uint32_t nSampleCount;
    ULARGE_INTEGER nSampleOffset;

    DecodeBoxData_stblSubBoxDefault(&(pBoxTree->mp4Box), &cttsInfo);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &(cttsInfo.fbVF));
    nOffset = 0;
    for (uint32_t iLoop = 0; iLoop < cttsInfo.nCnt; iLoop++)
    {
        if (nOffset < nLen)
        {
            if (iLoop < HAN_VIDEO_MP4_BOX_ENTRIES_INFO_MAX_CNT)
            {
                nSampleCount = ReadMP4Data4ByteMSB(&(cttsInfo.pEntry)[nOffset]); nOffset += 4;
                nOffset += DecodeBoxDataReadDataByVersion(&(cttsInfo.pEntry)[nOffset], cttsInfo.fbVF.nVersion, &nSampleOffset);
                nId = UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%s%u"), GetMP4_ctts_FieldName(VIDEO_MP4_ctts_BOX_FIELD_ENTRY), iLoop + 1);
                nId = UpdateBoxInfoWindow_InsertLine(pText, TEXT(""), nId, hListView);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nSampleCount);
                nId = UpdateBoxInfoWindow_InsertLine(GetMP4_ctts_FieldName(VIDEO_MP4_ctts_BOX_FIELD_SAMPLE_COUNT), pText, nId, hListView);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%" PRIu64), nSampleOffset.QuadPart);
                nId = UpdateBoxInfoWindow_InsertLine(GetMP4_ctts_FieldName(VIDEO_MP4_ctts_BOX_FIELD_SAMPLE_OFFSET), pText, nId, hListView);
            }
            else
            {
                nId = UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("剩余%u个条目..."), cttsInfo.nCnt - HAN_VIDEO_MP4_BOX_ENTRIES_INFO_MAX_CNT);
                nId = UpdateBoxInfoWindow_InsertLine(pText, TEXT(""), nId, hListView);
                break;
            }
        }
    }
}
static void UpdateBoxInfoWindow_stss(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam)
{
    (void)pParam;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANCHAR pField[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_stblSubBoxDefault stssInfo;
    HANSIZE nLen = pBoxTree->mp4Box.nDataLen - 8;
    HANSIZE nOffset;

    DecodeBoxData_stblSubBoxDefault(&(pBoxTree->mp4Box), &stssInfo);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &(stssInfo.fbVF));
    nId = UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
    nOffset = 0;
    for (uint32_t iLoop = 0; iLoop < stssInfo.nCnt; iLoop++)
    {
        if (nOffset < nLen)
        {
            if (iLoop < HAN_VIDEO_MP4_BOX_ENTRIES_INFO_MAX_CNT)
            {
                HAN_snprintf(pField, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%s%u"), GetMP4_stss_FieldName(VIDEO_MP4_stss_BOX_FIELD_ENTRY), iLoop + 1);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("样本%u"), ReadMP4Data4ByteMSB(&(stssInfo.pEntry)[nOffset]));
                nId = UpdateBoxInfoWindow_InsertLine(pField, pText, nId, hListView);
                nOffset += 4;
            }
            else
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("剩余%u个条目..."), stssInfo.nCnt - HAN_VIDEO_MP4_BOX_ENTRIES_INFO_MAX_CNT);
                nId = UpdateBoxInfoWindow_InsertLine(pText, TEXT(""), nId, hListView);
                break;
            }
        }
    }
}
static void UpdateBoxInfoWindow_stsc(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam)
{
    (void)pParam;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_stblSubBoxDefault stscInfo;
    HANSIZE nLen = pBoxTree->mp4Box.nDataLen - 8;
    HANSIZE nOffset;
    uint32_t nFirstChunk;
    uint32_t nSamplesPerChunk;
    uint32_t nSamplesDescriptionId;

    DecodeBoxData_stblSubBoxDefault(&(pBoxTree->mp4Box), &stscInfo);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &(stscInfo.fbVF));
    nOffset = 0;
    for (uint32_t iLoop = 0; iLoop < stscInfo.nCnt; iLoop++)
    {
        if (nOffset < nLen)
        {
            if (iLoop < HAN_VIDEO_MP4_BOX_ENTRIES_INFO_MAX_CNT)
            {
                nFirstChunk = ReadMP4Data4ByteMSB(&(stscInfo.pEntry)[nOffset]); nOffset += 4;
                nSamplesPerChunk = ReadMP4Data4ByteMSB(&(stscInfo.pEntry)[nOffset]); nOffset += 4;
                nSamplesDescriptionId = ReadMP4Data4ByteMSB(&(stscInfo.pEntry)[nOffset]); nOffset += 4;
                nId = UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%s%u"), GetMP4_stsc_FieldName(VIDEO_MP4_stsc_BOX_FIELD_ENTRY), iLoop + 1);
                nId = UpdateBoxInfoWindow_InsertLine(pText, TEXT(""), nId, hListView);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nFirstChunk);
                nId = UpdateBoxInfoWindow_InsertLine(GetMP4_stsc_FieldName(VIDEO_MP4_stsc_BOX_FIELD_FIRST_CHUNK), pText, nId, hListView);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nSamplesPerChunk);
                nId = UpdateBoxInfoWindow_InsertLine(GetMP4_stsc_FieldName(VIDEO_MP4_stsc_BOX_FIELD_SAMPLE_PER_CHUNK), pText, nId, hListView);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nSamplesDescriptionId);
                nId = UpdateBoxInfoWindow_InsertLine(GetMP4_stsc_FieldName(VIDEO_MP4_stsc_BOX_FIELD_SAMPLE_DESCRIPTION_ID), pText, nId, hListView);
            }
            else
            {
                nId = UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("剩余%u个条目..."), stscInfo.nCnt - HAN_VIDEO_MP4_BOX_ENTRIES_INFO_MAX_CNT);
                nId = UpdateBoxInfoWindow_InsertLine(pText, TEXT(""), nId, hListView);
                break;
            }
        }
    }
}
static void UpdateBoxInfoWindow_stsz(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam)
{
    (void)pParam;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANCHAR pField[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_stsz stszInfo;
    HANSIZE nLen = pBoxTree->mp4Box.nDataLen - 12;
    HANSIZE nOffset;
    uint32_t nSize;

    DecodeBoxData_stsz(&(pBoxTree->mp4Box), &stszInfo);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &(stszInfo.fbVF));
    if (0 != stszInfo.nSampleSize)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), stszInfo.nSampleSize);
        nId = UpdateBoxInfoWindow_InsertLine(GetMP4_stsz_FieldName(VIDEO_MP4_stsz_BOX_FIELD_SAMPLE_SIZE), pText, nId, hListView);
    }
    else 
    {
        nId = UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
        nOffset = 0;
        for (uint32_t iLoop = 0; iLoop < stszInfo.nSampleCnt; iLoop++)
        {
            if (nOffset < nLen)
            {
                if (iLoop < HAN_VIDEO_MP4_BOX_ENTRIES_INFO_MAX_CNT)
                {
                    nSize = ReadMP4Data4ByteMSB(&(stszInfo.pEntry)[nOffset]);
                    HAN_snprintf(pField, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%s%u"), GetMP4_stsz_FieldName(VIDEO_MP4_stsz_BOX_FIELD_SIZE), iLoop + 1);
                    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nSize);
                    nId = UpdateBoxInfoWindow_InsertLine(pField, pText, nId, hListView);
                }
                else
                {
                    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("剩余%u个条目..."), stszInfo.nSampleCnt - HAN_VIDEO_MP4_BOX_ENTRIES_INFO_MAX_CNT);
                    nId = UpdateBoxInfoWindow_InsertLine(pText, TEXT(""), nId, hListView);
                    break;
                }
                nOffset += 4;
            }
        }
    }
}
static void UpdateBoxInfoWindow_stco(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam)
{
    (void)pParam;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANCHAR pField[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_stco64 stco64Info;
    HANSIZE nLen = pBoxTree->mp4Box.nDataLen - 8;
    HANSIZE nOffset;
    uint32_t nChunkOffset;

    DecodeBoxData_stco(&(pBoxTree->mp4Box), &stco64Info);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &(stco64Info.fbVF));
    nId = UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
    nOffset = 0;
    for (uint32_t iLoop = 0; iLoop < stco64Info.nChunkCnt; iLoop++)
    {
        if (nOffset < nLen)
        {
            if (iLoop < HAN_VIDEO_MP4_BOX_ENTRIES_INFO_MAX_CNT)
            {
                nChunkOffset = ReadMP4Data4ByteMSB(&(stco64Info.pEntry)[nOffset]); nOffset += 4;
                HAN_snprintf(pField, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%s%u"), GetMP4_stco_FieldName(VIDEO_MP4_stco_BOX_FIELD_ENTRY), iLoop + 1);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nChunkOffset);
                nId = UpdateBoxInfoWindow_InsertLine(pField, pText, nId, hListView);
            }
            else
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("剩余%u个条目..."), stco64Info.nChunkCnt - HAN_VIDEO_MP4_BOX_ENTRIES_INFO_MAX_CNT);
                nId = UpdateBoxInfoWindow_InsertLine(pText, TEXT(""), nId, hListView);
                break;
            }
        }
    }
}
static void UpdateBoxInfoWindow_co64(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam)
{
    (void)pParam;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANCHAR pField[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFO_stco64 stco64Info;
    HANSIZE nLen = pBoxTree->mp4Box.nDataLen - 8;
    HANSIZE nOffset;
    ULARGE_INTEGER nChunkOffset;

    DecodeBoxData_co64(&(pBoxTree->mp4Box), &stco64Info);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &(stco64Info.fbVF));
    nId = UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
    nOffset = 0;
    for (uint32_t iLoop = 0; iLoop < stco64Info.nChunkCnt; iLoop++)
    {
        if (nOffset < nLen)
        {
            if (iLoop < HAN_VIDEO_MP4_BOX_ENTRIES_INFO_MAX_CNT)
            {
                nChunkOffset.HighPart = ReadMP4Data4ByteMSB(&(stco64Info.pEntry)[nOffset]); nOffset += 4;
                nChunkOffset.LowPart = ReadMP4Data4ByteMSB(&(stco64Info.pEntry)[nOffset]); nOffset += 4;
                HAN_snprintf(pField, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%s%u"), GetMP4_stco_FieldName(VIDEO_MP4_stco_BOX_FIELD_ENTRY), iLoop + 1);
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%" PRIu64), nChunkOffset.QuadPart);
                nId = UpdateBoxInfoWindow_InsertLine(pField, pText, nId, hListView);
            }
            else
            {
                HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("剩余%u个条目..."), stco64Info.nChunkCnt - HAN_VIDEO_MP4_BOX_ENTRIES_INFO_MAX_CNT);
                nId = UpdateBoxInfoWindow_InsertLine(pText, TEXT(""), nId, hListView);
                break;
            }
        }
    }
}
static void UpdateBoxInfoWindow_smhd(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam)
{
    (void)pParam;
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId = 0;
    VIDEOMP4BOXINFOFULLBOXVERFLAGS fbVF;
    int16_t nBalance;

    DecodeBoxData_FullBoxVersionFlags(pData, &fbVF);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &fbVF);
    /* 立体声平衡 */
    nBalance = ReadMP4Data2ByteMSB(&pData[4]);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%.1f"), (HANFLOAT)nBalance / (HANFLOAT)0x100);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_smhd_FieldName(VIDEO_MP4_smhd_BOX_FIELD_BALANCE), pText, nId, hListView);
}
static void UpdateBoxInfoWindow_meta(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam)
{
    (void)pParam;
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HANINT nId = 0;
    VIDEOMP4BOXINFOFULLBOXVERFLAGS fbVF;

    DecodeBoxData_FullBoxVersionFlags(pData, &fbVF);
    nId = UpdateBoxInfoWindow_FullBoxVersionFlags(nId, hListView, &fbVF);
    nId = UpdateBoxInfoWindow_BlankBox(&pData[4], pBoxTree->mp4Box.nDataLen - 4, nId, hListView);
}
static void UpdateBoxInfoWindow_data(PVIDEOMP4BOXTREE pBoxTree, HWND hListView, void* pParam)
{
    (void)pParam;
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    WCHAR pWText[HAN_VIDEO_MP4_TEXT_BUF_SIZE];
    CHAR pMText[HAN_VIDEO_MP4_TEXT_BUF_SIZE];
    HANINT nId = 0;
    uint32_t cDataType = ReadMP4Data4ByteMSB(&pData[0]);
    uint32_t cLocale = ReadMP4Data4ByteMSB(&pData[4]);
    const uint8_t* pDataData = &pData[8];
    HANSIZE nDataLen = pBoxTree->mp4Box.nDataLen - 8;
    HANINT nTextSize = HAN_VIDEO_MP4_TEXT_BUF_SIZE;
    HANINT nWTextSize = HAN_VIDEO_MP4_TEXT_BUF_SIZE;

    /* 数据类型 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u（%s）"), cDataType, GetMP4_data_DataTypeName(cDataType));
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_data_FieldName(VIDEO_MP4_data_BOX_DATA_TYPE), pText, nId, hListView);
    /* 语言/地区代码 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), cLocale);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_data_FieldName(VIDEO_MP4_data_BOX_LOCALE), pText, nId, hListView);
    /* 数据 */
    switch (cDataType) {
        case 1: {
            if (HAN_VIDEO_MP4_TEXT_BUF_SIZE <= nDataLen) { nDataLen = HAN_VIDEO_MP4_TEXT_BUF_SIZE - 1; }
            memcpy(pMText, pDataData, nDataLen);
            pMText[nDataLen] = '\0';
            MultiByteToHANStr(CP_UTF8, 0, (PCCH)pDataData, -1, pText, &nTextSize, pWText, &nWTextSize, NULL, NULL);
        } break;
        default: {
            VideoMP4PrintHexData(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, pDataData, nDataLen);
        } break;
    }
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_data_FieldName(VIDEO_MP4_data_BOX_DATA), pText, nId, hListView);
}

static void UpdateBoxInfoWindow_mdatSample(HANSIZE nTrackId, HANSIZE nSampleId, PVIDEOMP4WNDEXTRA mp4Info)
{
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    const uint8_t* pData = mp4Info->paramVideo.pData;
    HWND hListView = mp4Info->box.hInfo;
    PVIDEOMP4BOXINFO pBoxInfo = &(mp4Info->box.boxInfo);
    PVIDEOMP4TRACK pTrack = pBoxInfo->track.pList;
    PVIDEOMP4DECODEINFOSAMPLE pSample = &(pTrack[nTrackId].pSample[nSampleId]);
    const uint8_t* pSampleData;
    uint32_t nSampleSize;
    HANINT nId = 0;
    
    pSampleData = &pData[pTrack[nTrackId].pSample[nSampleId].sOffset];
    nSampleSize = pTrack[nTrackId].pSample[nSampleId].nSize;

    UpdateBoxInfoWindowSample(pSampleData, nSampleSize, pTrack[nTrackId].decoder.table.pType[0], hListView, pTrack[nTrackId].decoder.table.pTable[0]);
    
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT(HANSIZE_PRINT_FORMAT), nTrackId);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mdat_FieldName(VIDEO_MP4_mdat_BOX_FIELD_TRACK_ID), pText, nId, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSample->idChunkGroup);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mdat_FieldName(VIDEO_MP4_mdat_BOX_FIELD_CHUNK_GROUP_ID), pText, nId, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), pSample->idChunk);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mdat_FieldName(VIDEO_MP4_mdat_BOX_FIELD_CHUNK_ID), pText, nId, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT(HANSIZE_PRINT_FORMAT), nSampleId);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mdat_FieldName(VIDEO_MP4_mdat_BOX_FIELD_SAMPLE_ID), pText, nId, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT(HANSIZE_PRINT_FORMAT), pSample->idKeyFrame);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mdat_FieldName(VIDEO_MP4_mdat_BOX_FIELD_KEY_FRAME_ID), pText, nId, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT(HANSIZE_PRINT_FORMAT), pSample->sOffset);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mdat_FieldName(VIDEO_MP4_mdat_BOX_FIELD_OFFSET), pText, nId, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u 字节"), pSample->nSize);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mdat_FieldName(VIDEO_MP4_mdat_BOX_FIELD_SAMPLE_SIZE), pText, nId, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT(HANSIZE_PRINT_FORMAT), pSample->timeDTS);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mdat_FieldName(VIDEO_MP4_mdat_BOX_FIELD_DTS), pText, nId, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT(HANSIZE_PRINT_FORMAT), pSample->timePTS);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mdat_FieldName(VIDEO_MP4_mdat_BOX_FIELD_PTS), pText, nId, hListView);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT(HANSIZE_PRINT_FORMAT), pSample->timeDuration);
    nId = UpdateBoxInfoWindow_InsertLine(GetMP4_mdat_FieldName(VIDEO_MP4_mdat_BOX_FIELD_DURATION), pText, nId, hListView);
}

static HANSIZE DecodeBoxDataReadDataByVersion(const uint8_t* pData, uint8_t nVersion, PULARGE_INTEGER pLargeInt)
{
    HANSIZE nRet;
    
    switch (nVersion) {
        case 0: {
            pLargeInt->HighPart = 0;
            pLargeInt->LowPart = ReadMP4Data4ByteMSB(&pData[0]);
            nRet = 4;
        } break;
        case 1: {
            pLargeInt->HighPart = ReadMP4Data4ByteMSB(&pData[0]);
            pLargeInt->LowPart = ReadMP4Data4ByteMSB(&pData[4]);
            nRet = 8;
        } break;
        default: { nRet = 0; } break;
    }

    return nRet;
}
static HANSIZE DecodeBoxDataDateTime(const uint8_t* pData, uint8_t nVersion, PVIDEOMP4BOXINFODATETIME pDateTime)
{
    HANSIZE nRet = DecodeBoxDataReadDataByVersion(pData, nVersion, &(pDateTime->uTime));
    ULARGE_INTEGER uTime;
    FILETIME ftStd;
    FILETIME ftLocal;

    uTime.QuadPart = (pDateTime->uTime.QuadPart + 9561628800U) * 10000000U;
    ftStd.dwLowDateTime = uTime.LowPart;
    ftStd.dwHighDateTime = uTime.HighPart;
    FileTimeToLocalFileTime(&ftStd, &ftLocal);
    FileTimeToSystemTime(&ftLocal, &(pDateTime->stTime));

    return nRet;
}
static void DecodeBoxData_FullBoxVersionFlags(const uint8_t* pData, PVIDEOMP4BOXINFOFULLBOXVERFLAGS pFB)
{
    pFB->nVersion = pData[0];
    pFB->cFlags = ReadMP4Data3ByteMSB(&pData[1]);
}
static BOOL DecodeBoxData_mvhd(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_mvhd pmvhd)
{
    const uint8_t* pData = pBox->pData;
    BOOL bRet = TRUE;
    HANSIZE nOffset = 0;
    
    pmvhd->pRawData = pData;
    pmvhd->fbVF.nVersion = pData[nOffset]; nOffset += 1;
    if (pmvhd->fbVF.nVersion < 2)
    {
        pmvhd->fbVF.cFlags = ReadMP4Data3ByteMSB(&pData[nOffset]); nOffset += 3;
        nOffset += DecodeBoxDataDateTime(&pData[nOffset], pmvhd->fbVF.nVersion, &(pmvhd->creationTime));
        nOffset += DecodeBoxDataDateTime(&pData[nOffset], pmvhd->fbVF.nVersion, &(pmvhd->modificationTime));
        pmvhd->timeDuration.nTimescale = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
        nOffset += DecodeBoxDataReadDataByVersion(&pData[nOffset], pmvhd->fbVF.nVersion, &(pmvhd->timeDuration.nDuration));
        pmvhd->timeDuration.sTimeDuration.QuadPart = pmvhd->timeDuration.nDuration.QuadPart / pmvhd->timeDuration.nTimescale;
        pmvhd->nRate.u32 = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
        pmvhd->nRate.nPhy += (HANDOUBLE)(pmvhd->nRate.u32) / (HANDOUBLE)0x10000;
        pmvhd->nVolume.u16 = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
        pmvhd->nVolume.nPhy = (HANDOUBLE)(pmvhd->nVolume.u16) / (HANDOUBLE)0x100;
        nOffset += 10;
        memcpy(pmvhd->pMatrix, &pData[nOffset], sizeof(pmvhd->pMatrix)); nOffset += sizeof(pmvhd->pMatrix);
        nOffset += 24;
        pmvhd->nNextTrackId = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
    }
    else { bRet = FALSE; }

    return bRet;
}
static BOOL DecodeBoxData_tkhd(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_tkhd ptkhd)
{
    const uint8_t* pData = pBox->pData;
    BOOL bRet = TRUE;
    HANSIZE nOffset;
    
    ptkhd->pRawData = pData;
    DecodeBoxData_FullBoxVersionFlags(pData, &(ptkhd->fbVF));
    if (ptkhd->fbVF.nVersion < 2)
    {
        nOffset = 4;
        nOffset += DecodeBoxDataDateTime(&pData[nOffset], ptkhd->fbVF.nVersion, &(ptkhd->creationTime));
        nOffset += DecodeBoxDataDateTime(&pData[nOffset], ptkhd->fbVF.nVersion, &(ptkhd->modificationTime));
        ptkhd->nTrackId = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
        nOffset += 4;
        nOffset += DecodeBoxDataReadDataByVersion(&pData[nOffset], ptkhd->fbVF.nVersion, &(ptkhd->nDuration));
        nOffset += 8;
        ptkhd->nLayer = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
        ptkhd->nAlternateGroup = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
        ptkhd->nVolume.u16 = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
        ptkhd->nVolume.nPhy = (HANDOUBLE)(ptkhd->nVolume.u16) / (HANDOUBLE)0x100;
        nOffset += 2;
        memcpy(ptkhd->pMatrix, &pData[nOffset], sizeof(ptkhd->pMatrix)); nOffset += sizeof(ptkhd->pMatrix);
        ptkhd->nWidth.u32 = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
        ptkhd->nWidth.nPhy = (HANDOUBLE)(ptkhd->nWidth.u32) / (HANDOUBLE)0x10000;
        ptkhd->nHeight.u32 = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
        ptkhd->nHeight.nPhy = (HANDOUBLE)(ptkhd->nHeight.u32) / (HANDOUBLE)0x10000;
    }
    else { bRet = FALSE; }

    return bRet;
}
static HANSIZE DecodeBoxData_elst(const uint8_t* pData, uint8_t nVersion, PVIDEOMP4BOXINFO_elst pelst)
{
    HANSIZE nRet = 0;

    pelst->pRawData = pData;
    if (nVersion < 2)
    {
        nRet += DecodeBoxDataReadDataByVersion(&pData[nRet], nVersion, &(pelst->nSegmentDuration));
        nRet += DecodeBoxDataReadDataByVersion(&pData[nRet], nVersion, &(pelst->nMediaTime));
        pelst->mediaRate.u32 = ReadMP4Data4ByteMSB(&pData[nRet]); nRet += 4;
        pelst->mediaRate.nPhy = (HANDOUBLE)(pelst->mediaRate.u32) / (HANDOUBLE)0x10000;
    }

    return nRet;
}
static BOOL DecodeBoxData_mdhd(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_mdhd pmdhd)
{
    const uint8_t* pData = pBox->pData;
    BOOL bRet = TRUE;
    HANSIZE nOffset;
    
    pmdhd->pRawData = pData;
    DecodeBoxData_FullBoxVersionFlags(pData, &(pmdhd->fbVF));
    if (pmdhd->fbVF.nVersion < 2)
    {
        nOffset = 4;
        nOffset += DecodeBoxDataDateTime(&pData[nOffset], pmdhd->fbVF.nVersion, &(pmdhd->creationTime));
        nOffset += DecodeBoxDataDateTime(&pData[nOffset], pmdhd->fbVF.nVersion, &(pmdhd->modificationTime));
        pmdhd->timeDuration.nTimescale = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
        nOffset += DecodeBoxDataReadDataByVersion(&pData[nOffset], pmdhd->fbVF.nVersion, &(pmdhd->timeDuration.nDuration));
        pmdhd->timeDuration.sTimeDuration.QuadPart = pmdhd->timeDuration.nDuration.QuadPart / pmdhd->timeDuration.nTimescale;
        memcpy(pmdhd->pLanguage, &pData[nOffset], 2); nOffset += 2;
    }
    else { bRet = FALSE; }

    return bRet;
}
static void DecodeBoxData_hdlr(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_hdlr phdlr)
{
    const uint8_t* pData = pBox->pData;
    HANSIZE nOffset = 8;
    
    phdlr->pRawData = pData;
    DecodeBoxData_FullBoxVersionFlags(pData, &(phdlr->fbVF));
    memcpy(phdlr->handlerType.pType, &pData[nOffset], 4); nOffset += 4;
    phdlr->handlerType.pType[4] = '\0';
    phdlr->handlerType.eType = DecodeBoxDataGetTrackHandlerType(phdlr->handlerType.pType);
    nOffset += 12;
    phdlr->pName = &pData[nOffset];
}
static void DecodeBoxData_vmhd(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_vmhd pvmhd)
{
    const uint8_t* pData = pBox->pData;
    HANSIZE nOffset = 4;
    
    pvmhd->pRawData = pData;
    DecodeBoxData_FullBoxVersionFlags(pData, &(pvmhd->fbVF));
    pvmhd->cGraphicsMode = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
    pvmhd->pOpColor[0] = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
    pvmhd->pOpColor[1] = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
    pvmhd->pOpColor[2] = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
}
static VIDEOMP4TRACKHANDLERTYPE DecodeBoxDataGetTrackHandlerType(uint8_t pType[4])
{
    VIDEOMP4TRACKHANDLERTYPE eRet = VIDEO_MP4_TRACK_HANDLER_TYPE_CNT;

    for (VIDEOMP4TRACKHANDLERTYPE iLoop = 0; iLoop < VIDEO_MP4_TRACK_HANDLER_TYPE_CNT; iLoop++)
    {
        if (!memcmp(pType, sg_pMP4BoxTrackHandlerInfo[iLoop].pType, 4))
        {
            eRet = iLoop;
            break;
        }
    }

    return eRet;
}
static BOOL DecodeBoxData_url_(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_url_ purl_)
{
    const uint8_t* pData = pBox->pData;
    BOOL bRet = TRUE;
    HANSIZE nOffset = 0;
    
    purl_->pRawData = pData;
    DecodeBoxData_FullBoxVersionFlags(pData, &(purl_->fbVF));
    switch (purl_->fbVF.cFlags) {
        case 0x000000: { purl_->pUrl = &pData[nOffset]; } break;
        case 0x000001: { purl_->pUrl = NULL; } break;
        default: { bRet = FALSE; } break;
    }

    return bRet;
}
static void DecodeBoxData_stblSubBoxDefault(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_stblSubBoxDefault pDef)
{
    const uint8_t* pData = pBox->pData;

    pDef->pRawData = pData;
    DecodeBoxData_FullBoxVersionFlags(pData, &(pDef->fbVF));
    pDef->nCnt = ReadMP4Data4ByteMSB(&pData[4]);
    pDef->pEntry = &pData[8];
    pDef->nEntrySize = pBox->nDataLen - 8;
}
static void DecodeBoxData_stsd(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_stsd pstsd)
{
    const uint8_t* pData = pBox->pData;

    pstsd->pRawData = pData;
    DecodeBoxData_FullBoxVersionFlags(pData, &(pstsd->fbVF));
    pstsd->nDecoderCnt = ReadMP4Data4ByteMSB(&pData[4]);
}
static void DecodeBoxData_stsz(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_stsz pstsz)
{
    const uint8_t* pData = pBox->pData;

    pstsz->pRawData = pData;
    DecodeBoxData_FullBoxVersionFlags(pData, &(pstsz->fbVF));
    pstsz->nSampleSize = ReadMP4Data4ByteMSB(&pData[4]);
    pstsz->nSampleCnt = ReadMP4Data4ByteMSB(&pData[8]);
    if (0 == pstsz->nSampleSize)
    {
        pstsz->pEntry = &pData[12];
        pstsz->nEntrySize = pBox->nDataLen - 12;
    }
    else
    {
        pstsz->pEntry = NULL;
        pstsz->nEntrySize = 0;
    }
}
static void DecodeBoxData_stco(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_stco64 pstco)
{
    const uint8_t* pData = pBox->pData;

    pstco->pRawData = pData;
    pstco->nDataSize = 4;
    DecodeBoxData_FullBoxVersionFlags(pData, &(pstco->fbVF));
    pstco->nChunkCnt = ReadMP4Data4ByteMSB(&pData[4]);
    pstco->pEntry = &pData[8];
    pstco->nEntrySize = pBox->nDataLen - 8;
}
static void DecodeBoxData_co64(PCVIDEOMP4BOX pBox, PVIDEOMP4BOXINFO_stco64 pco64)
{
    const uint8_t* pData = pBox->pData;

    pco64->pRawData = pData;
    pco64->nDataSize = 8;
    DecodeBoxData_FullBoxVersionFlags(pData, &(pco64->fbVF));
    pco64->nChunkCnt = ReadMP4Data4ByteMSB(&pData[4]);
    pco64->pEntry = &pData[8];
    pco64->nEntrySize = pBox->nDataLen - 8;
}

#if 1 /* 解码 */
static BOOL DecodeMP4(PVIDEOMP4WNDEXTRA mp4Info, HANSIZE nFrame)
{
    BOOL bRet = TRUE;
    const uint8_t* pData = mp4Info->paramVideo.pData;
    PVIDEOMP4BOXINFO pBoxInfo = &(mp4Info->box.boxInfo);
    PVIDEOMP4TRACK pTrack = pBoxInfo->track.pList;
    HANSIZE nTrackCnt = pBoxInfo->track.nCnt;
    uint32_t nSampleSize;
    HANSIZE nSampleOffset;

    for (HANSIZE iLoop = 0; iLoop < nTrackCnt; iLoop++)
    {
        if (VIDEO_MP4_TRACK_HANDLER_TYPE_VIDEO == pTrack[iLoop].hdlr.handlerType.eType)
        {
            nSampleSize = pTrack[iLoop].pSample[nFrame].nSize;
            nSampleOffset = pTrack[iLoop].pSample[nFrame].sOffset;

            DecodeMedia(&pData[nSampleOffset], nSampleSize, pTrack[iLoop].decoder.table.pType[0], pTrack[iLoop].decoder.table.pTable[0]);

            break;
        }
    }

    return bRet;
}
#endif
