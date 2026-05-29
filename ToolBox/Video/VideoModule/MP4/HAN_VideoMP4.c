#include <inttypes.h>
#include <Windows.h>
#include <Shlwapi.h>
#include <CommCtrl.h>

#include "HAN_VideoMP4.h"
#include "HAN_VideoMP4Box.h"

typedef enum {
    VIDEO_MP4_BOX_INFO_HEADER_FIELD,
    VIDEO_MP4_BOX_INFO_HEADER_VALUE,
    VIDEO_MP4_BOX_INFO_HEADER_CNT,
} VIDEOMP4BOXINFOHEADER;

typedef enum {
    VIDEO_MP4_READ_UE_RET_OK,
    VIDEO_MP4_READ_UE_RET_ERROR,
} VIDEOMP4READUERET;

typedef struct tagVIDEOMP4BOXTREE {
    HTREEITEM                       hItem;
    VIDEOMP4BOX                     mp4Box;
    struct tagVIDEOMP4BOXTREE*      pParent;
} VIDEOMP4BOXTREE, * PVIDEOMP4BOXTREE;

typedef struct tagVIDEOMP4READBOXMACHINE {
    const uint8_t*                  pData;
    HANSIZE                         nLen;
    HANPSIZE                        pBoxCnt;
    PVIDEOMP4BOXTREE                pTree;
} VIDEOMP4READBOXMACHINE, * PVIDEOMP4READBOXMACHINE;

typedef struct tagVIDEOMP4READUE {
    const uint8_t*                  pData;
    HANSIZE                         nDataLen;
    HANCPRIVATE uint8_t             cData;
    HANCPRIVATE HANSIZE             iByte;
    HANCPRIVATE uint8_t             iBit;
    HANCPUBLIC uint32_t             cValue;
} VIDEOMP4READUE, * PVIDEOMP4READUE;

typedef struct tagVIDEOMP4WNDEXTRA {
    HANDLE                          hHeap;
    HINSTANCE                       hInst;
    HWND                            hSelf;
    VIDEOCREATEPARAM                paramVideo;
    struct {
        HWND                        hTree;
        HWND                        hInfo;
        HTREEITEM                   hFileTree;
        struct {
            HANSIZE                 nCnt;
            PVIDEOMP4BOXTREE        pBoxTree;
        } map;
    } box;
    struct {
        HFONT                       hHex;
        HFONT                       hSys;
    } hFont;                        /* 字体 */
    uint8_t                         pBuf[];
} VIDEOMP4WNDEXTRA, *PVIDEOMP4WNDEXTRA;

typedef struct tagVIDEOMP4BOXTYPEINFO {
    PCHAR                           pType;
    HANPSTR                         (*GetBoxName)(void);
    HANSIZE                         (*ReadSubBox)(PVIDEOMP4READBOXMACHINE pReadBox, PVIDEOMP4WNDEXTRA mp4Info);
    void                            (*UpdateBoxInfoWindow)(PVIDEOMP4BOXTREE pBoxTree, HWND hListView);
} VIDEOMP4BOXTYPEINFO;

typedef struct tagVIDEOMP4BOXTRACKHANDLERINFO {
    uint8_t                         pType[5];
} VIDEOMP4BOXTRACKHANDLERINFO;

typedef struct tagVIDEOMP4BOXPROFILEIDCINFO {
    HANPSTR                         pName;
} VIDEOMP4BOXPROFILEIDCINFO;

static LRESULT CALLBACK VideoMP4WndProc(HWND hVideoMP4, UINT message, WPARAM wParam, LPARAM lParam);

static LRESULT CreateCallback(HWND hVideoMP4, LPARAM lParam);
static LRESULT NotifyCallback(PVIDEOMP4WNDEXTRA mp4Info, NMHDR* pNotify);
static void DestroyCallback(PVIDEOMP4WNDEXTRA mp4Info);
static void InitMP4BoxInfoWindow(PVIDEOMP4WNDEXTRA mp4Info);
static void InitMP4BoxTreeWindow(PVIDEOMP4WNDEXTRA mp4Info);
static HANSIZE MP4Process(const uint8_t* pData, HANSIZE nLen, PVIDEOMP4WNDEXTRA mp4Info);
static HANSIZE ReadMP4Box(PVIDEOMP4READBOXMACHINE pReadBox, PVIDEOMP4WNDEXTRA mp4Info);
static HANSIZE ReadMP4BoxesOffset0(PVIDEOMP4READBOXMACHINE pReadBox, PVIDEOMP4WNDEXTRA mp4Info);
static HANSIZE ReadMP4BoxesOffset8(PVIDEOMP4READBOXMACHINE pReadBox, PVIDEOMP4WNDEXTRA mp4Info);
static HANSIZE ReadMP4BoxesOffset78(PVIDEOMP4READBOXMACHINE pReadBox, PVIDEOMP4WNDEXTRA mp4Info);
static void UpdateBoxTreeMap(PVIDEOMP4READBOXMACHINE pReadBox, PVIDEOMP4WNDEXTRA mp4Info);
static void PrintMP4Text4Bytes(HANCHAR pDest[5], const uint8_t pSrc[4]);
static VIDEOMP4BOXTYPE GetMP4BoxType(const uint8_t* pType);
static PVIDEOMP4WNDEXTRA ReallocMP4InfoMemory(PVIDEOMP4WNDEXTRA mp4Info);
static void VideoMP4PrintHexData(HANPSTR pText, HANSIZE nTextLen, const uint8_t* pData, HANSIZE nDataLen);
static LRESULT BoxTreeNotifyCallback(PVIDEOMP4WNDEXTRA mp4Info, NMHDR* pNotify);
static void UpdateBoxInfoWindow(PVIDEOMP4WNDEXTRA mp4Info, TVITEM* pItem);

static inline HANINT GetBoxInfoWindowWidth(void);
static inline uint16_t ReadMP4Data2ByteMSB(const uint8_t pData[2]);
static inline uint32_t ReadMP4Data3ByteMSB(const uint8_t pData[3]);
static inline uint32_t ReadMP4Data4ByteMSB(const uint8_t pData[4]);

static void UpdateBoxInfoWindow_InsertLine(HANPSTR pField, HANPSTR pValue, HANINT nId, HWND hListView);
static void UpdateBoxInfoWindow_PrintTimeDuration(HANPSTR pText, HANSIZE nLen, PULARGE_INTEGER pTimeDuration);
static void UpdateBoxInfoWindow_BlankBox(const uint8_t* pData, uint32_t nLen, HWND hListView);
static void UpdateBoxInfoWindow_SetTitle(VIDEOMP4BOXTYPE eType, HWND hListView);
static void UpdateBoxInfoWindow_Default(PVIDEOMP4BOXTREE pBoxTree, HWND hListView);
static void UpdateBoxInfoWindow_ftyp(PVIDEOMP4BOXTREE pBoxTree, HWND hListView);
static void UpdateBoxInfoWindow_free(PVIDEOMP4BOXTREE pBoxTree, HWND hListView);
static void UpdateBoxInfoWindow_mvhd(PVIDEOMP4BOXTREE pBoxTree, HWND hListView);
static void UpdateBoxInfoWindow_tkhd(PVIDEOMP4BOXTREE pBoxTree, HWND hListView);
static void UpdateBoxInfoWindow_elst(PVIDEOMP4BOXTREE pBoxTree, HWND hListView);
static void UpdateBoxInfoWindow_mdhd(PVIDEOMP4BOXTREE pBoxTree, HWND hListView);
static void UpdateBoxInfoWindow_hdlr(PVIDEOMP4BOXTREE pBoxTree, HWND hListView);
static void UpdateBoxInfoWindow_vmhd(PVIDEOMP4BOXTREE pBoxTree, HWND hListView);
static void UpdateBoxInfoWindow_dref(PVIDEOMP4BOXTREE pBoxTree, HWND hListView);
static void UpdateBoxInfoWindow_url_(PVIDEOMP4BOXTREE pBoxTree, HWND hListView);
static void UpdateBoxInfoWindow_stsd(PVIDEOMP4BOXTREE pBoxTree, HWND hListView);
static void UpdateBoxInfoWindow_avc1(PVIDEOMP4BOXTREE pBoxTree, HWND hListView);
static void UpdateBoxInfoWindow_avcC(PVIDEOMP4BOXTREE pBoxTree, HWND hListView);

static HANSIZE DecodeBoxDataReadDataByVersion(const uint8_t* pData, uint8_t nVersion, PULARGE_INTEGER pLargeInt);
static HANSIZE DecodeBoxDataDateTime(const uint8_t* pData, uint8_t nVersion, PVIDEOMP4BOXINFODATETIME pDateTime);
static BOOL DecodeBoxData_mvhd(const uint8_t* pData, PVIDEOMP4BOXINFO_mvhd pmvhd);
static BOOL DecodeBoxData_tkhd(const uint8_t* pData, PVIDEOMP4BOXINFO_tkhd ptkhd);
static HANSIZE DecodeBoxData_elst(const uint8_t* pData, uint8_t nVersion, PVIDEOMP4BOXINFO_elst pelst);
static BOOL DecodeBoxData_mdhd(const uint8_t* pData, PVIDEOMP4BOXINFO_mdhd pmdhd);
static void DecodeBoxData_hdlr(const uint8_t* pData, PVIDEOMP4BOXINFO_hdlr phdlr);
static void DecodeBoxData_vmhd(const uint8_t* pData, PVIDEOMP4BOXINFO_vmhd pvmhd);
static VIDEOMP4TRACKHANDLERTYPE DecodeBoxDataGetTrackHandlerType(uint8_t pType[4]);
static BOOL DecodeBoxData_url_(const uint8_t* pData, PVIDEOMP4BOXINFO_url_ purl_);
static void DecodeBoxData_avc1(const uint8_t* pData, PVIDEOMP4BOXINFO_avc1 pavc1);
static void DecodeBoxData_avcC(const uint8_t* pData, PVIDEOMP4BOXINFO_avcC pavcC);
static uint32_t DecodeBoxData_SPS(const uint8_t* pData, uint32_t nSize, PVIDEOMP4BOXINFO_SPS pSPS);
static uint32_t DecodeBoxData_PPS(const uint8_t* pData, uint32_t nSize);
static VIDEOMP4READUERET DecodeBoxData_SPSProfileParam(PVIDEOMP4BOXINFO_SPS pSPS, PVIDEOMP4READUE pReadUE);
static VIDEOMP4PROFILETYPE DecodeBoxData_GetSPSProfileType(PVIDEOMP4BOXINFO_SPS pSPS);

static inline VIDEOMP4READUE MP4InitReadUE(const uint8_t* pData, HANSIZE nLen);
static inline VIDEOMP4READUERET MP4ReadUE(PVIDEOMP4READUE pReadUE);
static inline uint8_t MP4ReadUEBit(PVIDEOMP4READUE pReadUE);

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
static const VIDEOMP4BOXTYPEINFO sg_pMP4BoxType[VIDEO_MP4_BOX_TYPE_CNT] = {
    [VIDEO_MP4_BOX_TYPE_ftyp] = {
        .pType = "ftyp",
        .GetBoxName = GetMP4_ftyp_Name,
        .ReadSubBox = NULL,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_ftyp,
    },
    [VIDEO_MP4_BOX_TYPE_free] = {
        .pType = "free",
        .GetBoxName = GetMP4_free_Name,
        .ReadSubBox = NULL,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_free,
    },
    [VIDEO_MP4_BOX_TYPE_moov] = {
        .pType = "moov",
        .GetBoxName = GetMP4_moov_Name,
        .ReadSubBox = ReadMP4BoxesOffset0,
        .UpdateBoxInfoWindow = NULL,
    },
    [VIDEO_MP4_BOX_TYPE_mvhd] = {
        .pType = "mvhd",
        .GetBoxName = GetMP4_mvhd_Name,
        .ReadSubBox = NULL,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_mvhd,
    },
    [VIDEO_MP4_BOX_TYPE_trak] = {
        .pType = "trak",
        .GetBoxName = GetMP4_trak_Name,
        .ReadSubBox = ReadMP4BoxesOffset0,
        .UpdateBoxInfoWindow = NULL,
    },
    [VIDEO_MP4_BOX_TYPE_tkhd] = {
        .pType = "tkhd",
        .GetBoxName = GetMP4_tkhd_Name,
        .ReadSubBox = NULL,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_tkhd,
    },
    [VIDEO_MP4_BOX_TYPE_edts] = {
        .pType = "edts",
        .GetBoxName = GetMP4_edts_Name,
        .ReadSubBox = ReadMP4BoxesOffset0,
        .UpdateBoxInfoWindow = NULL,
    },
    [VIDEO_MP4_BOX_TYPE_elst] = {
        .pType = "elst",
        .GetBoxName = GetMP4_elst_Name,
        .ReadSubBox = NULL,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_elst,
    },
    [VIDEO_MP4_BOX_TYPE_mdia] = {
        .pType = "mdia",
        .GetBoxName = GetMP4_mdia_Name,
        .ReadSubBox = ReadMP4BoxesOffset0,
        .UpdateBoxInfoWindow = NULL,
    },
    [VIDEO_MP4_BOX_TYPE_mdhd] = {
        .pType = "mdhd",
        .GetBoxName = GetMP4_mdhd_Name,
        .ReadSubBox = NULL,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_mdhd,
    },
    [VIDEO_MP4_BOX_TYPE_hdlr] = {
        .pType = "hdlr",
        .GetBoxName = GetMP4_hdlr_Name,
        .ReadSubBox = NULL,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_hdlr,
    },
    [VIDEO_MP4_BOX_TYPE_minf] = {
        .pType = "minf",
        .GetBoxName = GetMP4_minf_Name,
        .ReadSubBox = ReadMP4BoxesOffset0,
        .UpdateBoxInfoWindow = NULL,
    },
    [VIDEO_MP4_BOX_TYPE_vmhd] = {
        .pType = "vmhd",
        .GetBoxName = GetMP4_vmhd_Name,
        .ReadSubBox = NULL,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_vmhd,
    },
    [VIDEO_MP4_BOX_TYPE_dinf] = {
        .pType = "dinf",
        .GetBoxName = GetMP4_dinf_Name,
        .ReadSubBox = ReadMP4BoxesOffset0,
        .UpdateBoxInfoWindow = NULL,
    },
    [VIDEO_MP4_BOX_TYPE_dref] = {
        .pType = "dref",
        .GetBoxName = GetMP4_dref_Name,
        .ReadSubBox = ReadMP4BoxesOffset8,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_dref,
    },
    [VIDEO_MP4_BOX_TYPE_url_] = {
        .pType = "url ",
        .GetBoxName = GetMP4_url__Name,
        .ReadSubBox = NULL,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_url_,
    },
    [VIDEO_MP4_BOX_TYPE_stbl] = {
        .pType = "stbl",
        .GetBoxName = GetMP4_stbl_Name,
        .ReadSubBox = ReadMP4BoxesOffset0,
        .UpdateBoxInfoWindow = NULL,
    },
    [VIDEO_MP4_BOX_TYPE_stsd] = {
        .pType = "stsd",
        .GetBoxName = GetMP4_stsd_Name,
        .ReadSubBox = ReadMP4BoxesOffset8,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_stsd,
    },
    [VIDEO_MP4_BOX_TYPE_avc1] = {
        .pType = "avc1",
        .GetBoxName = GetMP4_avc1_Name,
        .ReadSubBox = ReadMP4BoxesOffset78,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_avc1,
    },
    [VIDEO_MP4_BOX_TYPE_avcC] = {
        .pType = "avcC",
        .GetBoxName = GetMP4_avcC_Name,
        .ReadSubBox = NULL,
        .UpdateBoxInfoWindow = UpdateBoxInfoWindow_avcC,
    },
};

BOOL CheckMP4Type(const uint8_t* pData, HANSIZE nLen)
{
    BOOL bRet = FALSE;

    if ((8 <= nLen) && (!memcmp(&pData[4], sg_pMP4BoxType[VIDEO_MP4_BOX_TYPE_ftyp].pType, 4))) { bRet = TRUE; }

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
    HANSIZE nBoxCnt;

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
        nWinW = GetBoxInfoWindowWidth();
        mp4Info->box.hInfo = CreateWindow(WC_LISTVIEW, TEXT("BOX信息"),
            WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL,
            nWinX, nWinY, nWinW, nWinH,
            hVideoMP4, (HMENU)WID_VIDEO_MP4_BOX_INFO, hInst, NULL);
            
        ListView_SetExtendedListViewStyle(mp4Info->box.hInfo, LVS_EX_FULLROWSELECT);

        InitMP4BoxInfoWindow(mp4Info);
    }
    /* 解码文件的数据段，填充信息 */
    if (-1 != lWndProcRet)
    {
        mp4Info->paramVideo = *pVideoCreateParam;
        nBoxCnt = MP4Process(mp4Info->paramVideo.pData, mp4Info->paramVideo.nLen, NULL);
        mp4Info->box.map.nCnt = nBoxCnt;
        if (0 == nBoxCnt) { lWndProcRet = -1; }
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
        InitMP4BoxTreeWindow(mp4Info);
        MP4Process(pVideoCreateParam->pData, mp4Info->paramVideo.nLen, mp4Info);
    }

    return lWndProcRet;
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
static void InitMP4BoxInfoWindow(PVIDEOMP4WNDEXTRA mp4Info)
{
    LVCOLUMN lvTitle = { .mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM, };
    
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
static HANSIZE MP4Process(const uint8_t* pData, HANSIZE nLen, PVIDEOMP4WNDEXTRA mp4Info)
{
    HANSIZE nBoxCnt = 0;
    VIDEOMP4READBOXMACHINE readBox = {
        .pData = pData,
        .nLen = nLen,
        .pBoxCnt = &nBoxCnt,
        .pTree = NULL,
    };

    if (0 == ReadMP4BoxesOffset0(&readBox, mp4Info)) { nBoxCnt = 0; }

    return nBoxCnt;
}
static HANSIZE ReadMP4Box(PVIDEOMP4READBOXMACHINE pReadBox, PVIDEOMP4WNDEXTRA mp4Info)
{
    HANSIZE nRet = 0;
    const uint8_t* pData = pReadBox->pData;
    VIDEOMP4READBOXMACHINE readSubBox = *pReadBox;
    PVIDEOMP4BOXTREE pTree;
    VIDEOMP4BOXTYPE eType;
    BOOL bOk = TRUE;
    uint32_t nSize;

    nSize = ReadMP4Data4ByteMSB(pData);
    if (pReadBox->nLen < nSize) { bOk = FALSE; }

    if (TRUE == bOk) /* 先处理好当前节点，准备好给子 box 做递归 */
    {
        if (NULL != mp4Info)
        {
            pTree = &mp4Info->box.map.pBoxTree[*(pReadBox->pBoxCnt)];
            pTree->mp4Box.nSize = nSize;
            memcpy(pTree->mp4Box.pType, &pData[4], 4);
            pTree->mp4Box.pData = &pData[8];
            pTree->pParent = pReadBox->pTree;

            readSubBox.pTree = pTree;
            UpdateBoxTreeMap(&readSubBox, mp4Info);
        }
        (*(pReadBox->pBoxCnt))++;
    }

    if (TRUE == bOk)
    {
        eType = GetMP4BoxType(&pData[4]);
        if (eType < VIDEO_MP4_BOX_TYPE_CNT)
        {
            /* 尝试解析子 BOX，把数据段传下去 */
            readSubBox.pData = &pData[8];
            readSubBox.nLen = nSize - 8;
            readSubBox.pBoxCnt = pReadBox->pBoxCnt;
            if ((0 <= eType) && (NULL != sg_pMP4BoxType[eType].ReadSubBox)) { sg_pMP4BoxType[eType].ReadSubBox(&readSubBox, mp4Info); }
        }
    }

    if (TRUE == bOk) { nRet = nSize; }

    return nRet;
}
static HANSIZE ReadMP4BoxesOffset0(PVIDEOMP4READBOXMACHINE pReadBox, PVIDEOMP4WNDEXTRA mp4Info)
{
    HANSIZE nRet = 0;
    const uint8_t* pData = pReadBox->pData;
    HANSIZE nLen = pReadBox->nLen;
    VIDEOMP4READBOXMACHINE readSubBox = *pReadBox;
    HANSIZE nOffset;
    HANSIZE nBoxLen;

    nOffset = 0;
    while (nOffset < nLen)
    {
        nBoxLen = ReadMP4Box(&readSubBox, mp4Info);
        if (0 < nBoxLen)
        {
            nOffset += nBoxLen;
            readSubBox.pData = &pData[nOffset];
            readSubBox.nLen = nLen - nOffset;
            nRet += nBoxLen;
        }
        else
        {
            nRet = 0;
            break;
        }
    }

    return nRet;
}
static HANSIZE ReadMP4BoxesOffset8(PVIDEOMP4READBOXMACHINE pReadBox, PVIDEOMP4WNDEXTRA mp4Info)
{
    VIDEOMP4READBOXMACHINE readSubBox = *pReadBox;
    
    readSubBox.pData = &(pReadBox->pData)[8];
    readSubBox.nLen  = pReadBox->nLen - 8;

    return ReadMP4BoxesOffset0(&readSubBox, mp4Info);
}
static HANSIZE ReadMP4BoxesOffset78(PVIDEOMP4READBOXMACHINE pReadBox, PVIDEOMP4WNDEXTRA mp4Info)
{
    VIDEOMP4READBOXMACHINE readSubBox = *pReadBox;
    
    readSubBox.pData = &(pReadBox->pData)[78];
    readSubBox.nLen  = pReadBox->nLen - 78;

    return ReadMP4BoxesOffset0(&readSubBox, mp4Info);
}
static void UpdateBoxTreeMap(PVIDEOMP4READBOXMACHINE pReadBox, PVIDEOMP4WNDEXTRA mp4Info)
{
    HANCHAR pText[5];
    PVIDEOMP4BOXTREE pTree = pReadBox->pTree;
    PVIDEOMP4BOXTREE pParent = pReadBox->pTree->pParent;
    TVINSERTSTRUCT tviInsert;

    PrintMP4Text4Bytes(pText, pTree->mp4Box.pType);

    if (NULL != pParent) { tviInsert.hParent = pParent->hItem; }
    else { tviInsert.hParent = mp4Info->box.hFileTree; }
    tviInsert.hInsertAfter = TVI_LAST;
    tviInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
    tviInsert.item.pszText = pText;
    tviInsert.item.cchTextMax = ArrLen(pText);
    tviInsert.item.lParam = (LPARAM)pTree;
    pReadBox->pTree->hItem = TreeView_InsertItem(mp4Info->box.hTree, &tviInsert);
}
static void PrintMP4Text4Bytes(HANCHAR pDest[5], const uint8_t pSrc[4])
{
    CHAR pMultiBytes[5] = { pSrc[0], pSrc[1], pSrc[2], pSrc[3], '\0', };
    HAN_snprintf(pDest, 5, HANPSTR_PRINT_PCHAR_FORMAT, pMultiBytes);
}
static VIDEOMP4BOXTYPE GetMP4BoxType(const uint8_t* pType)
{
    VIDEOMP4BOXTYPE eRet = VIDEO_MP4_BOX_TYPE_CNT;

    for (VIDEOMP4BOXTYPE eType = 0; eType < VIDEO_MP4_BOX_TYPE_CNT; eType++)
    {
        if (!memcmp(sg_pMP4BoxType[eType].pType, pType, 4))
        {
            eRet = eType;
        }
    }

    return eRet;
}
static PVIDEOMP4WNDEXTRA ReallocMP4InfoMemory(PVIDEOMP4WNDEXTRA mp4Info)
{
    PVIDEOMP4WNDEXTRA pRet;
    HANSIZE nBoxCnt = mp4Info->box.map.nCnt;
    SIZE_T nOffset;
    SIZE_T nBoxTreeSize;

    nBoxTreeSize = sizeof(VIDEOMP4BOXTREE) * nBoxCnt;
    pRet = (PVIDEOMP4WNDEXTRA)HANWinHeapAlloc(
        mp4Info->hHeap, mp4Info,
        sizeof(VIDEOMP4WNDEXTRA)
         + nBoxTreeSize
    );

    if (NULL != pRet)
    {
        nOffset = 0;
        pRet->box.map.pBoxTree = (PVIDEOMP4BOXTREE)&(pRet->pBuf[nOffset]); nOffset += nBoxTreeSize;
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
        } break;

        default: { } break;
    }

    return lWndProcRet;
}
static void UpdateBoxInfoWindow(PVIDEOMP4WNDEXTRA mp4Info, TVITEM* pItem)
{
    PVIDEOMP4BOXTREE pBoxTree = (PVIDEOMP4BOXTREE)(pItem->lParam);
    VIDEOMP4BOXTYPE eType;

    if (((LPARAM)NULL) != pItem->lParam)
    {
        eType = GetMP4BoxType(pBoxTree->mp4Box.pType);

        SendMessage(mp4Info->box.hInfo, WM_SETREDRAW, FALSE, 0);

        ListView_DeleteAllItems(mp4Info->box.hInfo);
        if ((0 <= eType) && (eType < VIDEO_MP4_BOX_TYPE_CNT))
        {
            UpdateBoxInfoWindow_SetTitle(eType, mp4Info->box.hInfo);
            if (NULL != sg_pMP4BoxType[eType].UpdateBoxInfoWindow) { sg_pMP4BoxType[eType].UpdateBoxInfoWindow(pBoxTree, mp4Info->box.hInfo); }
            else { UpdateBoxInfoWindow_BlankBox(pBoxTree->mp4Box.pData, pBoxTree->mp4Box.nSize - 8, mp4Info->box.hInfo); }
        }
        else { UpdateBoxInfoWindow_Default(pBoxTree, mp4Info->box.hInfo); }

        SendMessage(mp4Info->box.hInfo, WM_SETREDRAW, TRUE, 0);
        InvalidateRect(mp4Info->box.hInfo, NULL, TRUE);
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

static void UpdateBoxInfoWindow_InsertLine(HANPSTR pField, HANPSTR pValue, HANINT nId, HWND hListView)
{
    LVITEM lvItem = { .mask = LVIF_TEXT ,};
    
    lvItem.iItem = nId;
    lvItem.iSubItem = VIDEO_MP4_BOX_INFO_HEADER_FIELD;
    lvItem.pszText = pField;
    ListView_InsertItem(hListView, &lvItem);

    lvItem.iSubItem = VIDEO_MP4_BOX_INFO_HEADER_VALUE;
    lvItem.pszText = pValue;
    ListView_SetItem(hListView, &lvItem);
}
static void UpdateBoxInfoWindow_PrintTimeDuration(HANPSTR pText, HANSIZE nLen, PULARGE_INTEGER pTimeDuration)
{
    uint32_t nHour = (uint32_t)(pTimeDuration->QuadPart / 3600);
    uint8_t nMinute = (uint8_t)((pTimeDuration->QuadPart % 3600) / 60);
    uint8_t nSecond = (uint8_t)(pTimeDuration->QuadPart % 60);

    HAN_snprintf(pText, nLen, TEXT("%u:%u:%u"), nHour, nMinute, nSecond);
}
static void UpdateBoxInfoWindow_BlankBox(const uint8_t* pData, uint32_t nLen, HWND hListView)
{
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId;
    
    nId = 0;
    /* 长度 */
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nLen);
    UpdateBoxInfoWindow_InsertLine(TEXT("长度"), pText, nId, hListView);
    nId++;
    /* 数据 */
    VideoMP4PrintHexData(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, pData, nLen);
    UpdateBoxInfoWindow_InsertLine(TEXT("数据"), pText, nId, hListView);
    nId++;
}
static void UpdateBoxInfoWindow_SetTitle(VIDEOMP4BOXTYPE eType, HWND hListView)
{
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    LVCOLUMN lvTitle = { .mask = LVCF_TEXT, };

    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, sg_pMP4BoxType[eType].pType);
    lvTitle.pszText = pText;
    ListView_SetColumn(hListView, VIDEO_MP4_BOX_INFO_HEADER_FIELD, &lvTitle);
    lvTitle.pszText = sg_pMP4BoxType[eType].GetBoxName();
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

    UpdateBoxInfoWindow_BlankBox(pBoxTree->mp4Box.pData, pBoxTree->mp4Box.nSize - 8, hListView);
}
static void UpdateBoxInfoWindow_ftyp(PVIDEOMP4BOXTREE pBoxTree, HWND hListView)
{
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANCHAR pField[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANPSTR pCompatibleBrandsName;
    uint32_t nVersion;
    HANINT nId;
    HANINT nNameId;
    
    /* major brand */
    nId = VIDEO_MP4_ftyp_BOX_FIELD_MAJOR_BRAND;
    PrintMP4Text4Bytes(pText, &pData[0]);
    UpdateBoxInfoWindow_InsertLine(GetMP4_ftyp_FieldName(nId), pText, nId, hListView);
    /* minor version */
    nId = VIDEO_MP4_ftyp_BOX_FIELD_MINOR_VERSION;
    nVersion = ReadMP4Data4ByteMSB(&pData[4]);
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%08X"), nVersion);
    UpdateBoxInfoWindow_InsertLine(GetMP4_ftyp_FieldName(nId), pText, nId, hListView);
    /* compatible brands */
    pCompatibleBrandsName = GetMP4_ftyp_FieldName(VIDEO_MP4_ftyp_BOX_FIELD_COMPATIBLE_BRANDS);
    nNameId = 0;
    for (HANSIZE nOffset = 8; nOffset < pBoxTree->mp4Box.nSize - 8; nOffset += 4)
    {
        nId = VIDEO_MP4_ftyp_BOX_FIELD_COMPATIBLE_BRANDS + nNameId;
        HAN_snprintf(pField, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%s%d"), pCompatibleBrandsName, nNameId + 1);
        PrintMP4Text4Bytes(pText, &pData[nOffset]);
        UpdateBoxInfoWindow_InsertLine(pField, pText, nId, hListView);
        nNameId++;
    }
}
static void UpdateBoxInfoWindow_free(PVIDEOMP4BOXTREE pBoxTree, HWND hListView)
{
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    uint32_t nLen = pBoxTree->mp4Box.nSize - 8;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    CHAR pBuf[HAN_VIDEO_MP4_TEXT_BUF_SIZE];
    VIDEOMP4BOXFIELD_free nId;

    /* 长度 */
    nId = VIDEO_MP4_free_BOX_FIELD_LEN;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nLen);
    UpdateBoxInfoWindow_InsertLine(GetMP4_free_FieldName(nId), pText, nId, hListView);
    /* 数据 */
    nId = VIDEO_MP4_free_BOX_FIELD_DATA;
    VideoMP4PrintHexData(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, pData, nLen);
    UpdateBoxInfoWindow_InsertLine(GetMP4_free_FieldName(nId), pText, nId, hListView);
    /* 文本 */
    nId = VIDEO_MP4_free_BOX_FIELD_TEXT;
    if (HAN_VIDEO_MP4_TEXT_BUF_SIZE <= nLen) { nLen = HAN_VIDEO_MP4_TEXT_BUF_SIZE - 1; }
    memcpy(pBuf, pData, nLen);
    pBuf[nLen] = '\0';
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, pBuf);
    UpdateBoxInfoWindow_InsertLine(GetMP4_free_FieldName(nId), pText, nId, hListView);
}
static void UpdateBoxInfoWindow_mvhd(PVIDEOMP4BOXTREE pBoxTree, HWND hListView)
{
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    VIDEOMP4BOXFIELD_mvhd nId;
    VIDEOMP4BOXINFO_mvhd mvhdInfo;

    if (TRUE == DecodeBoxData_mvhd(pData, &mvhdInfo))
    {
        /* 版本 */
        nId = VIDEO_MP4_mvhd_BOX_FIELD_VERSION;
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u（%s）"), mvhdInfo.nVersion, GetMP4_mvhd_VersionName(mvhdInfo.nVersion));
        UpdateBoxInfoWindow_InsertLine(GetMP4_mvhd_FieldName(nId), pText, nId, hListView);
        /* 标志 */
        nId = VIDEO_MP4_mvhd_BOX_FIELD_FLAGS;
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%06X"), mvhdInfo.cFlags);
        UpdateBoxInfoWindow_InsertLine(GetMP4_mvhd_FieldName(nId), pText, nId, hListView);
        /* 创建时间 */
        nId = VIDEO_MP4_mvhd_BOX_FIELD_CREATION_TIME;
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u/%u/%u %02u:%02u:%02u"),
            mvhdInfo.creationTime.stTime.wYear, mvhdInfo.creationTime.stTime.wMonth, mvhdInfo.creationTime.stTime.wDay,
            mvhdInfo.creationTime.stTime.wHour, mvhdInfo.creationTime.stTime.wMinute, mvhdInfo.creationTime.stTime.wSecond);
        UpdateBoxInfoWindow_InsertLine(GetMP4_mvhd_FieldName(nId), pText, nId, hListView);
        /* 修改时间 */
        nId = VIDEO_MP4_mvhd_BOX_FIELD_MODIFICATION_TIME;
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u/%u/%u %02u:%02u:%02u"),
            mvhdInfo.modificationTime.stTime.wYear, mvhdInfo.modificationTime.stTime.wMonth, mvhdInfo.modificationTime.stTime.wDay,
            mvhdInfo.modificationTime.stTime.wHour, mvhdInfo.modificationTime.stTime.wMinute, mvhdInfo.modificationTime.stTime.wSecond);
        UpdateBoxInfoWindow_InsertLine(GetMP4_mvhd_FieldName(nId), pText, nId, hListView);
        /* 时长 */
        nId = VIDEO_MP4_mvhd_BOX_FIELD_DURATION;
        UpdateBoxInfoWindow_PrintTimeDuration(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, &mvhdInfo.timeDuration.sTimeDuration);
        UpdateBoxInfoWindow_InsertLine(GetMP4_mvhd_FieldName(nId), pText, nId, hListView);
        /* 播放速率 */
        nId = VIDEO_MP4_mvhd_BOX_FIELD_RATE;
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%.1lf"), mvhdInfo.nRate.nPhy);
        UpdateBoxInfoWindow_InsertLine(GetMP4_mvhd_FieldName(nId), pText, nId, hListView);
        /* 音量 */
        nId = VIDEO_MP4_mvhd_BOX_FIELD_VOLUME;
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%.1lf"), mvhdInfo.nVolume.nPhy);
        UpdateBoxInfoWindow_InsertLine(GetMP4_mvhd_FieldName(nId), pText, nId, hListView);
        /* 视频变换矩阵 */
        nId = VIDEO_MP4_mvhd_BOX_FIELD_MATRIX;
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), mvhdInfo.pMatrix[0]);
        UpdateBoxInfoWindow_InsertLine(GetMP4_mvhd_FieldName(nId), pText, nId, hListView);
        /* 下一个轨道的 ID */
        nId = VIDEO_MP4_mvhd_BOX_FIELD_NEXT_TRACK_ID;
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), mvhdInfo.nNextTrackId);
        UpdateBoxInfoWindow_InsertLine(GetMP4_mvhd_FieldName(nId), pText, nId, hListView);
    }
}
static void UpdateBoxInfoWindow_tkhd(PVIDEOMP4BOXTREE pBoxTree, HWND hListView)
{
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANCHAR pFlagsName[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    VIDEOMP4BOXFIELD_tkhd nId;
    VIDEOMP4BOXINFO_tkhd tkhdInfo;

    if (TRUE == DecodeBoxData_tkhd(pData, &tkhdInfo))
    {
        /* 版本 */
        nId = VIDEO_MP4_tkhd_BOX_FIELD_VERSION;
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u（%s）"), tkhdInfo.nVersion, GetMP4_tkhd_VersionName(tkhdInfo.nVersion));
        UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(nId), pText, nId, hListView);
        /* 标志 */
        nId = VIDEO_MP4_tkhd_BOX_FIELD_FLAGS;
        GetMP4_tkhd_FlagsName(tkhdInfo.cFlags, pFlagsName, HAN_VIDEO_MP4_TEXT_BUF_SIZE);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%06X（%s）"), tkhdInfo.cFlags, pFlagsName);
        UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(nId), pText, nId, hListView);
        /* 创建时间 */
        nId = VIDEO_MP4_tkhd_BOX_FIELD_CREATION_TIME;
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u/%u/%u %02u:%02u:%02u"),
            tkhdInfo.creationTime.stTime.wYear, tkhdInfo.creationTime.stTime.wMonth, tkhdInfo.creationTime.stTime.wDay,
            tkhdInfo.creationTime.stTime.wHour, tkhdInfo.creationTime.stTime.wMinute, tkhdInfo.creationTime.stTime.wSecond);
        UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(nId), pText, nId, hListView);
        /* 修改时间 */
        nId = VIDEO_MP4_tkhd_BOX_FIELD_MODIFICATION_TIME;
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u/%u/%u %02u:%02u:%02u"),
            tkhdInfo.modificationTime.stTime.wYear, tkhdInfo.modificationTime.stTime.wMonth, tkhdInfo.modificationTime.stTime.wDay,
            tkhdInfo.modificationTime.stTime.wHour, tkhdInfo.modificationTime.stTime.wMinute, tkhdInfo.modificationTime.stTime.wSecond);
        UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(nId), pText, nId, hListView);
        /* 轨道ID */
        nId = VIDEO_MP4_tkhd_BOX_FIELD_TRACK_ID;
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), tkhdInfo.nTrackId);
        UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(nId), pText, nId, hListView);
        /* 轨道时长 */
        nId = VIDEO_MP4_tkhd_BOX_FIELD_DURATION;
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%" PRIu64), tkhdInfo.nDuration.QuadPart);
        UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(nId), pText, nId, hListView);
        /* 视频层 */
        nId = VIDEO_MP4_tkhd_BOX_FIELD_LAYER;
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), tkhdInfo.nLayer);
        UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(nId), pText, nId, hListView);
        /* 替代组 */
        nId = VIDEO_MP4_tkhd_BOX_FIELD_ALTERNATE_GROUP;
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), tkhdInfo.nAlternateGroup);
        UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(nId), pText, nId, hListView);
        /* 音量 */
        nId = VIDEO_MP4_tkhd_BOX_FIELD_VOLUME;
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%.1lf"), tkhdInfo.nVolume.nPhy);
        UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(nId), pText, nId, hListView);
        /* 变换矩阵 */
        nId = VIDEO_MP4_tkhd_BOX_FIELD_MATRIX;
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), tkhdInfo.pMatrix[0]);
        UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(nId), pText, nId, hListView);
        /* 分辨率 */
        nId = VIDEO_MP4_tkhd_BOX_FIELD_RESOLUTION;
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%g * %g"), tkhdInfo.nWidth.nPhy, tkhdInfo.nHeight.nPhy);
        UpdateBoxInfoWindow_InsertLine(GetMP4_tkhd_FieldName(nId), pText, nId, hListView);
    }
}
static void UpdateBoxInfoWindow_elst(PVIDEOMP4BOXTREE pBoxTree, HWND hListView)
{
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId;
    uint8_t nVersion;
    uint32_t cFlags;
    uint32_t nListCnt;
    VIDEOMP4BOXINFO_elst elstInfo;
    HANSIZE nOffset;
    
    nOffset = 0;
    nId = 0;
    /* 版本 */
    nVersion = pData[nOffset]; nOffset += 1;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nVersion);
    UpdateBoxInfoWindow_InsertLine(TEXT("版本"), pText, nId, hListView);
    nId++;
    /* 标志 */
    cFlags = ReadMP4Data3ByteMSB(&pData[nOffset]); nOffset += 3;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%06X"), cFlags);
    UpdateBoxInfoWindow_InsertLine(TEXT("标志"), pText, nId, hListView);
    nId++;
    /* 条目数 */
    nListCnt = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nListCnt);
    UpdateBoxInfoWindow_InsertLine(TEXT("条目数"), pText, nId, hListView);
    nId++;

    for (uint32_t iLoop = 0; iLoop < nListCnt; iLoop++)
    {
        UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
        nId++;
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("条目%d"), iLoop + 1);
        UpdateBoxInfoWindow_InsertLine(pText, TEXT(""), nId, hListView);
        nId++;

        nOffset += DecodeBoxData_elst(&pData[nOffset], nVersion, &elstInfo);
        /* 段时长 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%" PRIu64), elstInfo.nSegmentDuration.QuadPart);
        UpdateBoxInfoWindow_InsertLine(TEXT("段时长"), pText, nId, hListView);
        nId++;
        /* 时间点 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%" PRIu64), elstInfo.nMediaTime.QuadPart);
        UpdateBoxInfoWindow_InsertLine(TEXT("时间点"), pText, nId, hListView);
        nId++;
        /* 播放速率 */
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%.1lf"), elstInfo.mediaRate.nPhy);
        UpdateBoxInfoWindow_InsertLine(TEXT("播放速率"), pText, nId, hListView);
        nId++;
    }
}
static void UpdateBoxInfoWindow_mdhd(PVIDEOMP4BOXTREE pBoxTree, HWND hListView)
{
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    CHAR pLanguage[3];
    VIDEOMP4BOXFIELD_mdhd nId;
    VIDEOMP4BOXINFO_mdhd mdhdInfo;

    if (TRUE == DecodeBoxData_mdhd(pData, &mdhdInfo))
    {
        /* 版本 */
        nId = VIDEO_MP4_mdhd_BOX_FIELD_VERSION;
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u（%s）"), mdhdInfo.nVersion, GetMP4_mdhd_VersionName(mdhdInfo.nVersion));
        UpdateBoxInfoWindow_InsertLine(GetMP4_mdhd_FieldName(nId), pText, nId, hListView);
        /* 标志 */
        nId = VIDEO_MP4_mdhd_BOX_FIELD_FLAGS;
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%06X"), mdhdInfo.cFlags);
        UpdateBoxInfoWindow_InsertLine(GetMP4_mdhd_FieldName(nId), pText, nId, hListView);
        /* 创建时间 */
        nId = VIDEO_MP4_mdhd_BOX_FIELD_CREATION_TIME;
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u/%u/%u %02u:%02u:%02u"),
            mdhdInfo.creationTime.stTime.wYear, mdhdInfo.creationTime.stTime.wMonth, mdhdInfo.creationTime.stTime.wDay,
            mdhdInfo.creationTime.stTime.wHour, mdhdInfo.creationTime.stTime.wMinute, mdhdInfo.creationTime.stTime.wSecond);
        UpdateBoxInfoWindow_InsertLine(GetMP4_mdhd_FieldName(nId), pText, nId, hListView);
        /* 修改时间 */
        nId = VIDEO_MP4_mdhd_BOX_FIELD_MODIFICATION_TIME;
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u/%u/%u %02u:%02u:%02u"),
            mdhdInfo.modificationTime.stTime.wYear, mdhdInfo.modificationTime.stTime.wMonth, mdhdInfo.modificationTime.stTime.wDay,
            mdhdInfo.modificationTime.stTime.wHour, mdhdInfo.modificationTime.stTime.wMinute, mdhdInfo.modificationTime.stTime.wSecond);
        UpdateBoxInfoWindow_InsertLine(GetMP4_mdhd_FieldName(nId), pText, nId, hListView);
        /* 时长 */
        nId = VIDEO_MP4_mdhd_BOX_FIELD_DURATION;
        UpdateBoxInfoWindow_PrintTimeDuration(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, &mdhdInfo.timeDuration.sTimeDuration);
        UpdateBoxInfoWindow_InsertLine(GetMP4_mdhd_FieldName(nId), pText, nId, hListView);
        /* 语言 */
        nId = VIDEO_MP4_mdhd_BOX_FIELD_LANGUAGE;
        pLanguage[0] = mdhdInfo.pLanguage[0];
        pLanguage[1] = mdhdInfo.pLanguage[1];
        pLanguage[2] = '\0';
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, pLanguage);
        UpdateBoxInfoWindow_InsertLine(GetMP4_mdhd_FieldName(nId), pText, nId, hListView);
    }
}
static void UpdateBoxInfoWindow_hdlr(PVIDEOMP4BOXTREE pBoxTree, HWND hListView)
{
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    VIDEOMP4BOXFIELD_hdlr nId;
    VIDEOMP4BOXINFO_hdlr hdlrInfo;

    DecodeBoxData_hdlr(pData, &hdlrInfo);
    /* 版本 */
    nId = VIDEO_MP4_hdlr_BOX_FIELD_VERSION;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), hdlrInfo.nVersion);
    UpdateBoxInfoWindow_InsertLine(GetMP4_hdlr_FieldName(nId), pText, nId, hListView);
    /* 标志 */
    nId = VIDEO_MP4_hdlr_BOX_FIELD_FLAGS;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%06X"), hdlrInfo.cFlags);
    UpdateBoxInfoWindow_InsertLine(GetMP4_hdlr_FieldName(nId), pText, nId, hListView);
    /* 处理器类型 */
    nId = VIDEO_MP4_hdlr_BOX_FIELD_HANDLER_TYPE;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT TEXT("（%s）"),
        sg_pMP4BoxTrackHandlerInfo[hdlrInfo.handlerType.eType].pType,
        GetMP4_hdlr_HandlerTypeName(hdlrInfo.handlerType.eType));
    UpdateBoxInfoWindow_InsertLine(GetMP4_hdlr_FieldName(nId), pText, nId, hListView);
    /* 名称 */
    nId = VIDEO_MP4_hdlr_BOX_FIELD_NAME;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, hdlrInfo.pName);
    UpdateBoxInfoWindow_InsertLine(GetMP4_hdlr_FieldName(nId), pText, nId, hListView);
}
static void UpdateBoxInfoWindow_vmhd(PVIDEOMP4BOXTREE pBoxTree, HWND hListView)
{
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    VIDEOMP4BOXFIELD_vmhd nId;
    VIDEOMP4BOXINFO_vmhd vmhdInfo;

    DecodeBoxData_vmhd(pData, &vmhdInfo);
    /* 版本 */
    nId = VIDEO_MP4_vmhd_BOX_FIELD_VERSION;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), vmhdInfo.nVersion);
    UpdateBoxInfoWindow_InsertLine(GetMP4_vmhd_FieldName(nId), pText, nId, hListView);
    /* 标志 */
    nId = VIDEO_MP4_vmhd_BOX_FIELD_FLAGS;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%06X"), vmhdInfo.cFlags);
    UpdateBoxInfoWindow_InsertLine(GetMP4_vmhd_FieldName(nId), pText, nId, hListView);
    /* 处理器类型 */
    nId = VIDEO_MP4_vmhd_BOX_FIELD_GRAPHICS_MODE;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%04X（%s）"), vmhdInfo.cGraphicsMode, GetMP4_vmhd_GraphicsModeName(vmhdInfo.cGraphicsMode));
    UpdateBoxInfoWindow_InsertLine(GetMP4_vmhd_FieldName(nId), pText, nId, hListView);
    /* 名称 */
    nId = VIDEO_MP4_vmhd_BOX_FIELD_OP_COLOR;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("（%u, %u, %u）"), vmhdInfo.pOpColor[0], vmhdInfo.pOpColor[1], vmhdInfo.pOpColor[2]);
    UpdateBoxInfoWindow_InsertLine(GetMP4_vmhd_FieldName(nId), pText, nId, hListView);
}
static void UpdateBoxInfoWindow_dref(PVIDEOMP4BOXTREE pBoxTree, HWND hListView)
{
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId;
    uint8_t nVersion;
    uint32_t cFlags;
    uint32_t nListCnt;
    HANSIZE nOffset;

    nOffset = 0;
    nId = 0;
    /* 版本 */
    nVersion = pData[nOffset]; nOffset += 1;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nVersion);
    UpdateBoxInfoWindow_InsertLine(TEXT("版本"), pText, nId, hListView);
    nId++;
    /* 标志 */
    cFlags = ReadMP4Data3ByteMSB(&pData[nOffset]); nOffset += 3;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%06X"), cFlags);
    UpdateBoxInfoWindow_InsertLine(TEXT("标志"), pText, nId, hListView);
    nId++;
    /* 条目数 */
    nListCnt = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nListCnt);
    UpdateBoxInfoWindow_InsertLine(TEXT("条目数"), pText, nId, hListView);
    nId++;
}
static void UpdateBoxInfoWindow_url_(PVIDEOMP4BOXTREE pBoxTree, HWND hListView)
{
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    VIDEOMP4BOXFIELD_url_ nId;
    VIDEOMP4BOXINFO_url_ urlInfo;

    if (TRUE == DecodeBoxData_url_(pData, &urlInfo))
    {
        /* 版本 */
        nId = VIDEO_MP4_url__BOX_FIELD_VERSION;
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), urlInfo.nVersion);
        UpdateBoxInfoWindow_InsertLine(GetMP4_url__FieldName(nId), pText, nId, hListView);
        /* 标志 */
        nId = VIDEO_MP4_url__BOX_FIELD_FLAGS;
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%06X（%s）"), urlInfo.cFlags, GetMP4_url__FlagsName(urlInfo.cFlags));
        UpdateBoxInfoWindow_InsertLine(GetMP4_url__FieldName(nId), pText, nId, hListView);
        /* URL */
        nId = VIDEO_MP4_url__BOX_FIELD_LOCATION;
        if (0x000000 == urlInfo.cFlags) { HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, urlInfo.pUrl); }
        else { HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("本文件")); }
        UpdateBoxInfoWindow_InsertLine(GetMP4_url__FieldName(nId), pText, nId, hListView);
    }
}
static void UpdateBoxInfoWindow_stsd(PVIDEOMP4BOXTREE pBoxTree, HWND hListView)
{
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    HANINT nId;
    uint8_t nVersion;
    uint32_t cFlags;
    uint32_t nListCnt;
    HANSIZE nOffset;
    
    nOffset = 0;
    nId = 0;
    /* 版本 */
    nVersion = pData[nOffset]; nOffset += 1;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nVersion);
    UpdateBoxInfoWindow_InsertLine(TEXT("版本"), pText, nId, hListView);
    nId++;
    /* 标志 */
    cFlags = ReadMP4Data3ByteMSB(&pData[nOffset]); nOffset += 3;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%06X"), cFlags);
    UpdateBoxInfoWindow_InsertLine(TEXT("标志"), pText, nId, hListView);
    nId++;
    /* 条目数 */
    nListCnt = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nListCnt);
    UpdateBoxInfoWindow_InsertLine(TEXT("条目数"), pText, nId, hListView);
    nId++;
}
static void UpdateBoxInfoWindow_avc1(PVIDEOMP4BOXTREE pBoxTree, HWND hListView)
{
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    VIDEOMP4BOXFIELD_avc1 nId;
    VIDEOMP4BOXINFO_avc1 avc1Info;

    DecodeBoxData_avc1(pData, &avc1Info);
    /* 数据引用索引 */
    nId = VIDEO_MP4_avc1_BOX_FIELD_DATA_REF_INDEX;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nDataRefIndex);
    UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(nId), pText, nId, hListView);
    /* 版本 */
    nId = VIDEO_MP4_avc1_BOX_FIELD_VERSION;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nVersion);
    UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(nId), pText, nId, hListView);
    /* 修订 */
    nId = VIDEO_MP4_avc1_BOX_FIELD_REVISION;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nRevision);
    UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(nId), pText, nId, hListView);
    /* 修订 */
    nId = VIDEO_MP4_avc1_BOX_FIELD_VENDOR;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.cVendor);
    UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(nId), pText, nId, hListView);
    /* 时间质量 */
    nId = VIDEO_MP4_avc1_BOX_FIELD_TEMPORAL_QUALITY;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nTemporalQuality);
    UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(nId), pText, nId, hListView);
    /* 空间质量 */
    nId = VIDEO_MP4_avc1_BOX_FIELD_SPATIAL_QUALITY;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nSpatialQuality);
    UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(nId), pText, nId, hListView);
    /* 宽度（像素） */
    nId = VIDEO_MP4_avc1_BOX_FIELD_WIDTH;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nWidth);
    UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(nId), pText, nId, hListView);
    /* 高度（像素） */
    nId = VIDEO_MP4_avc1_BOX_FIELD_HEIGHT;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nHeight);
    UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(nId), pText, nId, hListView);
    /* 水平分辨率 */
    nId = VIDEO_MP4_avc1_BOX_FIELD_HORIZ_RESOLUTION;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%g"), avc1Info.nHorizResolution.nPhy);
    UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(nId), pText, nId, hListView);
    /* 垂直分辨率 */
    nId = VIDEO_MP4_avc1_BOX_FIELD_VERT_RESOLUTION;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%g"), avc1Info.nVertResolution.nPhy);
    UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(nId), pText, nId, hListView);
    /* 数据大小 */
    nId = VIDEO_MP4_avc1_BOX_FIELD_DATA_SIZE;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nDataSize);
    UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(nId), pText, nId, hListView);
    /* 每样本帧数 */
    nId = VIDEO_MP4_avc1_BOX_FIELD_FRAME_COUNT;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nFrameCount);
    UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(nId), pText, nId, hListView);
    /* 压缩器名称 */
    nId = VIDEO_MP4_avc1_BOX_FIELD_COMPRESSOR_NAME;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, avc1Info.pCompressorName);
    UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(nId), pText, nId, hListView);
    /* 颜色深度 */
    nId = VIDEO_MP4_avc1_BOX_FIELD_DEPTH;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nDepth);
    UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(nId), pText, nId, hListView);
    /* 颜色表ID */
    nId = VIDEO_MP4_avc1_BOX_FIELD_COLOR_TABLE;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avc1Info.nColorTable);
    UpdateBoxInfoWindow_InsertLine(GetMP4_avc1_FieldName(nId), pText, nId, hListView);
}
static void UpdateBoxInfoWindow_avcC(PVIDEOMP4BOXTREE pBoxTree, HWND hListView)
{
    const uint8_t* pData = pBoxTree->mp4Box.pData;
    HANCHAR pText[HAN_VIDEO_MP4_TEXT_BUF_SIZE] = TEXT("");
    VIDEOMP4BOXFIELD_avcC nId;
    VIDEOMP4BOXINFO_avcC avcCInfo;
    VIDEOMP4BOXINFO_SPS spsInfo;
    uint32_t nLen;
    uint32_t nOffset;

    DecodeBoxData_avcC(pData, &avcCInfo);
    /* 配置版本 */
    nId = VIDEO_MP4_avcC_BOX_FIELD_CONFIGURATION_VERSION;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avcCInfo.nConfigurationVersion);
    UpdateBoxInfoWindow_InsertLine(GetMP4_avcC_FieldName(nId), pText, nId, hListView);
    /* 配置文件标识 */
    nId = VIDEO_MP4_avcC_BOX_FIELD_AVC_PROFILE_INDICATION;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avcCInfo.nAvcProfileIndication);
    UpdateBoxInfoWindow_InsertLine(GetMP4_avcC_FieldName(nId), pText, nId, hListView);
    /* 配置文件兼容性 */
    nId = VIDEO_MP4_avcC_BOX_FIELD_PROFILE_COMPATIBILITY;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avcCInfo.nProfileCompatibility);
    UpdateBoxInfoWindow_InsertLine(GetMP4_avcC_FieldName(nId), pText, nId, hListView);
    /* 级别标识 */
    nId = VIDEO_MP4_avcC_BOX_FIELD_AVC_LEVEL_INDICATION;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), avcCInfo.nAvcLevelIndication);
    UpdateBoxInfoWindow_InsertLine(GetMP4_avcC_FieldName(nId), pText, nId, hListView);
    /* NALU长度大小 */
    nId = VIDEO_MP4_avcC_BOX_FIELD_AVC_NALU_LENGTH_SIZE;
    HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u字节"), avcCInfo.nNALULengthSize);
    UpdateBoxInfoWindow_InsertLine(GetMP4_avcC_FieldName(nId), pText, nId, hListView);
    nId++;
    /* SPS 条目 */
    UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
    nId++;
    nOffset = 0;
    for (uint8_t iLoop = 0; iLoop < avcCInfo.sps.nNum; iLoop++)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%s%u"), GetMP4_avcC_FieldName(VIDEO_MP4_avcC_BOX_FIELD_AVC_SPS), iLoop + 1);
        UpdateBoxInfoWindow_InsertLine(pText, TEXT(""), nId, hListView);
        nId++;
        nLen = DecodeBoxData_SPS(&(avcCInfo.sps.pList)[nOffset], (uint32_t)(avcCInfo.sps.nSize) - nOffset, &spsInfo);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nLen - 2);
        UpdateBoxInfoWindow_InsertLine(TEXT("长度"), pText, nId, hListView);
        nId++;
        VideoMP4PrintHexData(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, &(avcCInfo.sps.pList)[nOffset + 2], nLen - 2);
        UpdateBoxInfoWindow_InsertLine(TEXT("数据"), pText, nId, hListView);
        nId++;
        nOffset += nLen;
    }
    /* PPS 条目 */
    UpdateBoxInfoWindow_InsertLine(TEXT(""), TEXT(""), nId, hListView);
    nId++;
    nOffset = 0;
    for (uint8_t iLoop = 0; iLoop < avcCInfo.pps.nNum; iLoop++)
    {
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%s%u"), GetMP4_avcC_FieldName(VIDEO_MP4_avcC_BOX_FIELD_AVC_PPS), iLoop + 1);
        UpdateBoxInfoWindow_InsertLine(pText, TEXT(""), nId, hListView);
        nId++;
        nLen = DecodeBoxData_PPS(&(avcCInfo.pps.pList)[nOffset], (uint32_t)(avcCInfo.pps.nSize) - nOffset);
        HAN_snprintf(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, TEXT("%u"), nLen - 2);
        UpdateBoxInfoWindow_InsertLine(TEXT("长度"), pText, nId, hListView);
        nId++;
        VideoMP4PrintHexData(pText, HAN_VIDEO_MP4_TEXT_BUF_SIZE, &(avcCInfo.pps.pList)[nOffset + 2], nLen - 2);
        UpdateBoxInfoWindow_InsertLine(TEXT("数据"), pText, nId, hListView);
        nId++;
        nOffset += nLen;
    }
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
static BOOL DecodeBoxData_mvhd(const uint8_t* pData, PVIDEOMP4BOXINFO_mvhd pmvhd)
{
    BOOL bRet = TRUE;
    HANSIZE nOffset = 0;
    
    pmvhd->nVersion = pData[nOffset]; nOffset += 1;
    if (pmvhd->nVersion < 2)
    {
        pmvhd->cFlags = ReadMP4Data3ByteMSB(&pData[nOffset]); nOffset += 3;
        nOffset += DecodeBoxDataDateTime(&pData[nOffset], pmvhd->nVersion, &(pmvhd->creationTime));
        nOffset += DecodeBoxDataDateTime(&pData[nOffset], pmvhd->nVersion, &(pmvhd->modificationTime));
        pmvhd->timeDuration.nTimescale = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
        nOffset += DecodeBoxDataReadDataByVersion(&pData[nOffset], pmvhd->nVersion, &(pmvhd->timeDuration.nDuration));
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
static BOOL DecodeBoxData_tkhd(const uint8_t* pData, PVIDEOMP4BOXINFO_tkhd ptkhd)
{
    BOOL bRet = TRUE;
    HANSIZE nOffset = 0;
    
    ptkhd->nVersion = pData[nOffset]; nOffset += 1;
    if (ptkhd->nVersion < 2)
    {
        ptkhd->cFlags = ReadMP4Data3ByteMSB(&pData[nOffset]); nOffset += 3;
        nOffset += DecodeBoxDataDateTime(&pData[nOffset], ptkhd->nVersion, &(ptkhd->creationTime));
        nOffset += DecodeBoxDataDateTime(&pData[nOffset], ptkhd->nVersion, &(ptkhd->modificationTime));
        ptkhd->nTrackId = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
        nOffset += 4;
        nOffset += DecodeBoxDataReadDataByVersion(&pData[nOffset], ptkhd->nVersion, &(ptkhd->nDuration));
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

    if (nVersion < 2)
    {
        nRet += DecodeBoxDataReadDataByVersion(&pData[nRet], nVersion, &(pelst->nSegmentDuration));
        nRet += DecodeBoxDataReadDataByVersion(&pData[nRet], nVersion, &(pelst->nMediaTime));
        pelst->mediaRate.u32 = ReadMP4Data4ByteMSB(&pData[nRet]); nRet += 4;
        pelst->mediaRate.nPhy = (HANDOUBLE)(pelst->mediaRate.u32) / (HANDOUBLE)0x10000;
    }

    return nRet;
}
static BOOL DecodeBoxData_mdhd(const uint8_t* pData, PVIDEOMP4BOXINFO_mdhd pmdhd)
{
    BOOL bRet = TRUE;
    HANSIZE nOffset = 0;
    
    pmdhd->nVersion = pData[nOffset]; nOffset += 1;
    if (pmdhd->nVersion < 2)
    {
        pmdhd->cFlags = ReadMP4Data3ByteMSB(&pData[nOffset]); nOffset += 3;
        nOffset += DecodeBoxDataDateTime(&pData[nOffset], pmdhd->nVersion, &(pmdhd->creationTime));
        nOffset += DecodeBoxDataDateTime(&pData[nOffset], pmdhd->nVersion, &(pmdhd->modificationTime));
        pmdhd->timeDuration.nTimescale = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
        nOffset += DecodeBoxDataReadDataByVersion(&pData[nOffset], pmdhd->nVersion, &(pmdhd->timeDuration.nDuration));
        pmdhd->timeDuration.sTimeDuration.QuadPart = pmdhd->timeDuration.nDuration.QuadPart / pmdhd->timeDuration.nTimescale;
        memcpy(pmdhd->pLanguage, &pData[nOffset], 2); nOffset += 2;
    }
    else { bRet = FALSE; }

    return bRet;
}
static void DecodeBoxData_hdlr(const uint8_t* pData, PVIDEOMP4BOXINFO_hdlr phdlr)
{
    HANSIZE nOffset = 0;
    
    phdlr->nVersion = pData[nOffset]; nOffset += 1;
    phdlr->cFlags = ReadMP4Data3ByteMSB(&pData[nOffset]); nOffset += 3;
    nOffset += 4;
    memcpy(phdlr->handlerType.pType, &pData[nOffset], 4); nOffset += 4;
    phdlr->handlerType.eType = DecodeBoxDataGetTrackHandlerType(phdlr->handlerType.pType);
    nOffset += 12;
    phdlr->pName = &pData[nOffset];
}
static void DecodeBoxData_vmhd(const uint8_t* pData, PVIDEOMP4BOXINFO_vmhd pvmhd)
{
    HANSIZE nOffset = 0;
    
    pvmhd->nVersion = pData[nOffset]; nOffset += 1;
    pvmhd->cFlags = ReadMP4Data3ByteMSB(&pData[nOffset]); nOffset += 3;
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
static BOOL DecodeBoxData_url_(const uint8_t* pData, PVIDEOMP4BOXINFO_url_ purl_)
{
    BOOL bRet = TRUE;
    HANSIZE nOffset = 0;
    
    purl_->nVersion = pData[nOffset]; nOffset += 1;
    purl_->cFlags = ReadMP4Data3ByteMSB(&pData[nOffset]); nOffset += 3;
    switch (purl_->cFlags) {
        case 0x000000: { purl_->pUrl = &pData[nOffset]; } break;
        case 0x000001: { purl_->pUrl = NULL; } break;
        default: { bRet = FALSE; } break;
    }

    return bRet;
}
static void DecodeBoxData_avc1(const uint8_t* pData, PVIDEOMP4BOXINFO_avc1 pavc1)
{
    HANSIZE nOffset = 0;
    
    nOffset += 6;
    pavc1->nDataRefIndex = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
    pavc1->nVersion = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
    pavc1->nRevision = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
    pavc1->cVendor = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
    pavc1->nTemporalQuality = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
    pavc1->nSpatialQuality = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
    pavc1->nWidth = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
    pavc1->nHeight = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
    pavc1->nHorizResolution.u32 = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
    pavc1->nHorizResolution.nPhy = (HANDOUBLE)(pavc1->nHorizResolution.u32) / (HANDOUBLE)0x10000;
    pavc1->nVertResolution.u32 = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
    pavc1->nVertResolution.nPhy = (HANDOUBLE)(pavc1->nVertResolution.u32) / (HANDOUBLE)0x10000;
    pavc1->nDataSize = ReadMP4Data4ByteMSB(&pData[nOffset]); nOffset += 4;
    pavc1->nFrameCount = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
    memcpy(pavc1->pCompressorName, &pData[nOffset], 32); nOffset += 32;
    pavc1->nDepth = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
    pavc1->nColorTable = ReadMP4Data2ByteMSB(&pData[nOffset]); nOffset += 2;
}
static void DecodeBoxData_avcC(const uint8_t* pData, PVIDEOMP4BOXINFO_avcC pavcC)
{
    HANSIZE nOffset = 0;
    uint16_t nUnitLen;
    
    pavcC->nConfigurationVersion = pData[nOffset]; nOffset += 1;
    pavcC->nAvcProfileIndication = pData[nOffset]; nOffset += 1;
    pavcC->nProfileCompatibility = pData[nOffset]; nOffset += 1;
    pavcC->nAvcLevelIndication = pData[nOffset]; nOffset += 1;
    pavcC->nNALULengthSize = (pData[nOffset] & 0x03) + 1; nOffset += 1;
    pavcC->sps.nNum = (pData[nOffset] & 0x1F); nOffset += 1;
    pavcC->sps.pList = &pData[nOffset];
    pavcC->sps.nSize = 0;
    for (uint8_t iLoop = 0; iLoop < pavcC->sps.nNum; iLoop++)
    {
        nUnitLen = ReadMP4Data2ByteMSB(&pData[nOffset]);
        nOffset += (HANSIZE)2 + nUnitLen;
        pavcC->sps.nSize += (HANSIZE)2 + nUnitLen;
    }
    pavcC->pps.nNum = pData[nOffset]; nOffset += 1;
    pavcC->pps.pList = &pData[nOffset];
    for (uint8_t iLoop = 0; iLoop < pavcC->pps.nNum; iLoop++)
    {
        nUnitLen = ReadMP4Data2ByteMSB(&pData[nOffset]);
        nOffset += (HANSIZE)2 + nUnitLen;
        pavcC->pps.nSize += (HANSIZE)2 + nUnitLen;
    }
}
static uint32_t DecodeBoxData_SPS(const uint8_t* pData, uint32_t nSize, PVIDEOMP4BOXINFO_SPS pSPS)
{
    uint32_t nRet = ReadMP4Data2ByteMSB(pData) + 2;
    HANSIZE nOffset;
    VIDEOMP4READUE readUE;

    if (nRet <= nSize)
    {
        nOffset = 2;
        pSPS->header.refIdc = (pData[nOffset] & 0x60) >> 5;
        pSPS->header.naluType = pData[nOffset] & 0x1F;
        nOffset += 1;
        pSPS->nProfileIdc = pData[nOffset]; nOffset += 1;
        pSPS->nConstraintSetFlags.cFlags = pData[nOffset]; nOffset += 1;
        pSPS->eProfile = DecodeBoxData_GetSPSProfileType(pSPS);
        pSPS->nLevelIdc = pData[nOffset]; nOffset += 1;

        readUE = MP4InitReadUE(&pData[nOffset], nSize - nOffset);

        (void)MP4ReadUE(&readUE); pSPS->nSPSId = readUE.cValue;
        DecodeBoxData_SPSProfileParam(pSPS, &readUE);
        (void)MP4ReadUE(&readUE); pSPS->nMaxFrameNum = ((HANSIZE)2) << ((HANSIZE)(readUE.cValue) + (HANSIZE)4);
        (void)MP4ReadUE(&readUE); pSPS->nPOCType = readUE.cValue;
        if (0 == pSPS->nPOCType) { (void)MP4ReadUE(&readUE); pSPS->nMaxPicOrderCntLsb = ((HANSIZE)2) << ((HANSIZE)(readUE.cValue) + (HANSIZE)4); }
        else { printf("未收录的 POC 类型"); }
        (void)MP4ReadUE(&readUE); pSPS->nMaxNumRefFrames = readUE.cValue;
        pSPS->cGapsInFrameNumValueAllowedFlag = MP4ReadUEBit(&readUE);
        (void)MP4ReadUE(&readUE); pSPS->nWidth = (readUE.cValue + 1) << 4;
        (void)MP4ReadUE(&readUE); pSPS->nHeight = (readUE.cValue + 1) << 4;
    }
    else { nRet = nSize; }

    return nRet;
}
static uint32_t DecodeBoxData_PPS(const uint8_t* pData, uint32_t nSize)
{
    uint32_t nRet = ReadMP4Data2ByteMSB(pData) + 2;

    if (nSize < nRet) { nRet = nSize; }

    return nRet;
}
static VIDEOMP4READUERET DecodeBoxData_SPSProfileParam(PVIDEOMP4BOXINFO_SPS pSPS, PVIDEOMP4READUE pReadUE)
{
    VIDEOMP4READUERET eRet = VIDEO_MP4_READ_UE_RET_OK;
    uint8_t nProfileIdc = pSPS->nProfileIdc;
    uint8_t nBitDepth;

    pSPS->profileParam.nChromaFormatIdc = 1; // 参考规范第74页关于 chroma_format_idc 的说明，如果没有色度模式可解析，默认值应是 1（4:2:0）
    pSPS->profileParam.lumaParam.nBitDepth = 8;
    pSPS->profileParam.lumaParam.nQuantizationParamRange = 0;
    pSPS->profileParam.chromaParam.nBitDepth = 8;
    pSPS->profileParam.chromaParam.nQuantizationParamRange = 0;

    if ((100 == nProfileIdc) ||
        (110 == nProfileIdc) ||
        (122 == nProfileIdc) ||
        (244 == nProfileIdc) ||
        (44 == nProfileIdc) ||
        (83 == nProfileIdc) ||
        (86 == nProfileIdc) ||
        (118 == nProfileIdc) ||
        (128 == nProfileIdc) ||
        (138 == nProfileIdc) ||
        (139 == nProfileIdc) ||
        (134 == nProfileIdc) ||
        (135 == nProfileIdc))
    {
        (void)MP4ReadUE(pReadUE); pSPS->profileParam.nChromaFormatIdc = pReadUE->cValue;
        if (3 == pSPS->profileParam.nChromaFormatIdc) { pSPS->profileParam.bSeparateColourPlane = MP4ReadUEBit(pReadUE); }
        (void)MP4ReadUE(pReadUE); nBitDepth = pReadUE->cValue;
        pSPS->profileParam.lumaParam.nBitDepth = 8 + nBitDepth;
        pSPS->profileParam.lumaParam.nQuantizationParamRange = 6 * nBitDepth;
        (void)MP4ReadUE(pReadUE); nBitDepth = pReadUE->cValue;
        pSPS->profileParam.chromaParam.nBitDepth = 8 + nBitDepth;
        pSPS->profileParam.chromaParam.nQuantizationParamRange = 6 * nBitDepth;
        pSPS->profileParam.bBypassTransform = MP4ReadUEBit(pReadUE);
        pSPS->profileParam.seqScalingMatrix.bValid = MP4ReadUEBit(pReadUE);
    }

    return eRet;
}
static VIDEOMP4PROFILETYPE DecodeBoxData_GetSPSProfileType(PVIDEOMP4BOXINFO_SPS pSPS)
{
    VIDEOMP4PROFILETYPE eRet;

    switch (pSPS->nProfileIdc) {
        case 66: { eRet = VIDEO_MP4_PROFILE_TYPE_BASELINE; } break;
        case 77: { eRet = VIDEO_MP4_PROFILE_TYPE_MAIN; } break;
        case 88: { eRet = VIDEO_MP4_PROFILE_TYPE_EXTENDED; } break;
        case 100: {
            if (1 == pSPS->nConstraintSetFlags.bSets.bSet4)
            {
                if (1 == pSPS->nConstraintSetFlags.bSets.bSet5) { eRet = VIDEO_MP4_PROFILE_TYPE_CONSTRAINED_HIGH; }
                else { eRet = VIDEO_MP4_PROFILE_TYPE_PROGRESSIVE_HIGH; }
            }
            else { eRet = VIDEO_MP4_PROFILE_TYPE_HIGH; }
        } break;
        case 110: {
            if (1 == pSPS->nConstraintSetFlags.bSets.bSet3) { eRet = VIDEO_MP4_PROFILE_TYPE_HIGH_10_INTRA; }
            else if (1 == pSPS->nConstraintSetFlags.bSets.bSet4) { eRet = VIDEO_MP4_PROFILE_TYPE_PROGRESSIVE_HIGH_10; }
            else { eRet = VIDEO_MP4_PROFILE_TYPE_HIGH_10; }
        } break;
        case 122: {
            if (1 == pSPS->nConstraintSetFlags.bSets.bSet3) { eRet = VIDEO_MP4_PROFILE_TYPE_HIGH_422_INTRA; }
            else { eRet = VIDEO_MP4_PROFILE_TYPE_HIGH_422; }
        } break;
        case 244: {
            if (1 == pSPS->nConstraintSetFlags.bSets.bSet3) { eRet = VIDEO_MP4_PROFILE_TYPE_HIGH_444_INTRA; }
            else { eRet = VIDEO_MP4_PROFILE_TYPE_HIGH_444_PREDICTIVE; }
        } break;
        case 44: { eRet = VIDEO_MP4_PROFILE_TYPE_CAVLC_444_INTRA; } break;
        default: { eRet = VIDEO_MP4_PROFILE_TYPE_CNT; } break;
    }

    return eRet;
}

static inline VIDEOMP4READUE MP4InitReadUE(const uint8_t* pData, HANSIZE nLen)
{
    VIDEOMP4READUE rueRet = {
        .pData = pData,
        .nDataLen = nLen,
        .cData = pData[0],
        .iByte = 0,
        .iBit = 0,
    };

    return rueRet;
}
static inline VIDEOMP4READUERET MP4ReadUE(PVIDEOMP4READUE pReadUE)
{
    VIDEOMP4READUERET eRet = VIDEO_MP4_READ_UE_RET_ERROR;
    HANSIZE nBitLen = ((pReadUE->nDataLen - pReadUE->iByte) * 8) - pReadUE->iBit;
    uint8_t nZeroBits;
    uint32_t cX;

    if (63 <= nBitLen)
    {
        for (nZeroBits = 0; nZeroBits < 32; nZeroBits++)
        {
            if (0 != MP4ReadUEBit(pReadUE))
            {
                eRet = VIDEO_MP4_READ_UE_RET_OK;
                break;
            }
        }
        if (VIDEO_MP4_READ_UE_RET_OK == eRet)
        {
            cX = 0;
            for (uint8_t iLoop = 0; iLoop < nZeroBits; iLoop++)
            {
                cX = (cX << 1) + MP4ReadUEBit(pReadUE);
            }
            pReadUE->cValue = (1 << nZeroBits) - 1 + cX;
        }
    }
    
    return eRet;
}
static inline uint8_t MP4ReadUEBit(PVIDEOMP4READUE pReadUE)
{
    uint8_t cRet = (pReadUE->cData << pReadUE->iBit) & 0x80;

    pReadUE->iBit++;
    if (8 == pReadUE->iBit)
    {
        pReadUE->iBit = 0;
        pReadUE->iByte++;
        pReadUE->cData = pReadUE->pData[pReadUE->iByte];
    }
    if (0 != cRet) { cRet = 1; }

    return cRet;
}
