#include <math.h>
#include <float.h>

#include "HAN_PictureEditTool.h"
#include "HAN_PictureEditToolPaint.h"
#include "..\..\HAN_PictureLib.h"

#define HAN_PICTURE_EDIT_TOOL_PAPER_CLASS       TEXT("HAN_PictureEditToolPaper")

#define HAN_PICTURE_EDIT_TOOL_DEFAULT_ZOOM      PICTURE_EDIT_TOOL_ZOOM_010000

typedef enum {
    PETPM_NOTUSED = PCTM_MODELBASE,     // 不使用的消息值，自定义消息请在下方添加
    PETPM_SETPAINTID,                   // 设置需要绘制的图片 ID（WPARAM：从 0 开始的图片 ID，LPARAM：未使用）
    PETPM_GETPAINTID,                   // 获取当前激活的图片 ID（WPARAM：未使用，LPARAM：未使用，return：从 0 开始的图片 ID）
} PICTUREEDITTOOLPAINTMESSAGE;

typedef struct tagPICTUREEDITTOOLPAINTPIXEL{
    PPOINT                          pPicturePos;
    PPICTUREWINPIXEL                pPixel;
    HANSIZE                         nPixelSize;
    uint8_t                         pBuf[];
} PICTUREEDITTOOLPAINTPIXEL, * PPICTUREEDITTOOLPAINTPIXEL;

typedef struct tagPICTUREEDITTOOLPAINTWNDEXTRA {
    HANDLE                          hHeap;
    HINSTANCE                       hInst;
    HWND                            hSelf;
    HWND                            hEditTool;
    HWND                            hConsole;
    struct {
        HWND                        hZoom;
        HANINT                      nZoomId;
    } zoom;
    struct {
        HWND                        hPaper;
        HWND                        hVScroll;
        HWND                        hHScroll;
        HANSIZE                     nPaintId;
        HANPPICTUREINFO             pInfo;
        HANPPICTURE                 pPicture;
    } picture;
    PPICTUREEDITTOOLPAINTPIXEL      pPaintPixel;
    struct {
        HFONT                       hHex;
        HFONT                       hSys;
    } hFont;                        /* 字体 */
    uint8_t                         pBuf[];
} PICTUREEDITTOOLPAINTWNDEXTRA, * PPICTUREEDITTOOLPAINTWNDEXTRA;

typedef struct tagPICTUREEDITTOOLPAPERWNDEXTRA {
    HANDLE                          hHeap;
    HINSTANCE                       hInst;
    HWND                            hSelf;
    PPICTUREEDITTOOLPAINTWNDEXTRA   pPaint;
} PICTUREEDITTOOLPAPERWNDEXTRA, * PPICTUREEDITTOOLPAPERWNDEXTRA;

typedef struct tagPICTUREEDITTOOLPAINTZOOM {
    HANPCSTR                        pText;
    HANINT                          nZoom;
    void                            (*Callback)(PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo, const RECT* pPaperSize, const RECT* pPictureSize);
} PICTUREEDITTOOLPAINTZOOM;

static void RegisterHANPictureEditToolPaper(HINSTANCE hInst);

static LRESULT CALLBACK PictureEditToolPaintWndProc(HWND hPictureEditToolPaint, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK PictureEditToolPaperWndProc(HWND hPictureEditToolPaper, UINT message, WPARAM wParam, LPARAM lParam);

static LRESULT PaintCreateCallback(HWND hPictureEditToolPaint, LPARAM lParam);
static void PaintSizeCallback(HWND hPictureEditToolPaint, PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo);
static LRESULT PaintHScrollCallback(PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo, WORD nReq);
static LRESULT PaintVScrollCallback(PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo, WORD nReq);
static LRESULT PaintMouseWheelCallback(PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo, HANINT nUnit, WORD nVK);
static void PaintCommandCallback(HWND hPictureEditToolPaint, PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo, WPARAM wParam, LPARAM lParam);
static void PaintDestroyCallback(PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo);
static void ExternalZoomCallback(PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo, HANINT nZoom);
static HANSIZE GetPaintPixelInfoSize(const RECT* pPaintRect);
static HANSIZE GetPaintPixelDataSize(const RECT* pPaintRect);
static void UpdatePaintPixelInfo(PPICTUREEDITTOOLPAINTPIXEL pPaintPixel, const RECT* pPaintRect);
static inline void GetPaintClientRect(RECT* pClient, const RECT* pPaint);
static void MovePaintScroll(PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo, const RECT* pPaintRect);
static void GetPaintPictureResolution(PPICTURERESOLUTION pResolution, HANPCPICTUREINFO pPictureInfo);
static void ZoomCallback(PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo, WORD nCommand);
static void UpdatePaintScroll(PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo, HANPPICTURE pPicture, const RECT* pPaintSize, HANINT nZoom);
static HANINT CalcScrollPos(const SCROLLINFO* pOldScroll, const SCROLLINFO* pNewScroll);
static uint32_t GetZoomSize(uint32_t nSize, HANINT nZoom);
static LONG GetRestoreSize(LONG nSize, HANINT nZoom);
static HANPPICTURE GetPaintPicture(PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo);
static void UpdatePicturePaint(PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo); /* 更新要绘制的内容 */

static LRESULT PaperCreateCallback(HWND hPictureEditToolPaper, LPARAM lParam);
static void PaperPaintCallback(HDC hDC, PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo, const RECT* rcWin);
static void PaperMouseMoveCallback(PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo, PPOINT pPos);
static void PaperDestroyCallback(PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo);
static void PaperPaintBackground(PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo, const RECT* pPaper);

static void PaintCallbackZoom001250(PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo, const RECT* pPaperSize, const RECT* pPictureSize);
static void PaintCallbackZoom002500(PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo, const RECT* pPaperSize, const RECT* pPictureSize);
static void PaintCallbackZoom005000(PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo, const RECT* pPaperSize, const RECT* pPictureSize);
static void PaintCallbackZoom010000(PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo, const RECT* pPaperSize, const RECT* pPictureSize);
static void PaintCallbackZoom020000(PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo, const RECT* pPaperSize, const RECT* pPictureSize);
static void PaintCallbackZoom030000(PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo, const RECT* pPaperSize, const RECT* pPictureSize);
static void PaintCallbackZoom040000(PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo, const RECT* pPaperSize, const RECT* pPictureSize);
static void PaintCallbackZoom050000(PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo, const RECT* pPaperSize, const RECT* pPictureSize);
static void PaintCallbackZoom100000(PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo, const RECT* pPaperSize, const RECT* pPictureSize);

static void PaintCallbackZoomOut(PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo, const RECT* pPaperSize, const RECT* pPictureSize, LONG nZoom);
static void PaintCallbackZoomIn(PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo, const RECT* pPaperSize, const RECT* pPictureSize, LONG nZoom);

static void GetPaintPaperRect(HANPCPICTURE pPicture, const RECT* pPaintSize, RECT* pPaperSize, LONG nZoom);
static void GetPaintPictureRect(PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo, HANPCPICTURE pPicture, RECT* pPictureSize);
static inline PICTURERGBA UpdateRGBA(PICTURERGBA rgbaFore, PICTURERGBA rgbaBack, PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo);

static const PICTURERGBA sg_pAlphaColor[2] = {
    [0] = {
        .r = 0xFF,
        .g = 0xFF,
        .b = 0xFF,
        .a = 0xFF,
    },
    [1] = {
        .r = 0xCC,
        .g = 0xCC,
        .b = 0xCC,
        .a = 0xFF,
    },
};
PICTUREEDITTOOLPAINTZOOM sg_pZoom[PICTURE_EDIT_TOOL_ZOOM_CNT] = {
    [PICTURE_EDIT_TOOL_ZOOM_001250] = {
        .pText = TEXT("12.5%"),
        .nZoom = -8,
        .Callback = PaintCallbackZoom001250,
    },
    [PICTURE_EDIT_TOOL_ZOOM_002500] = {
        .pText = TEXT("25%"),
        .nZoom = -4,
        .Callback = PaintCallbackZoom002500,
    },
    [PICTURE_EDIT_TOOL_ZOOM_005000] = {
        .pText = TEXT("50%"),
        .nZoom = -2,
        .Callback = PaintCallbackZoom005000,
    },
    [PICTURE_EDIT_TOOL_ZOOM_010000] = {
        .pText = TEXT("100%"),
        .nZoom = 1,
        .Callback = PaintCallbackZoom010000,
    },
    [PICTURE_EDIT_TOOL_ZOOM_020000] = {
        .pText = TEXT("200%"),
        .nZoom = 2,
        .Callback = PaintCallbackZoom020000,
    },
    [PICTURE_EDIT_TOOL_ZOOM_030000] = {
        .pText = TEXT("300%"),
        .nZoom = 3,
        .Callback = PaintCallbackZoom030000,
    },
    [PICTURE_EDIT_TOOL_ZOOM_040000] = {
        .pText = TEXT("400%"),
        .nZoom = 4,
        .Callback = PaintCallbackZoom040000,
    },
    [PICTURE_EDIT_TOOL_ZOOM_050000] = {
        .pText = TEXT("500%"),
        .nZoom = 5,
        .Callback = PaintCallbackZoom050000,
    },
    [PICTURE_EDIT_TOOL_ZOOM_100000] = {
        .pText = TEXT("1000%"),
        .nZoom = 10,
        .Callback = PaintCallbackZoom100000,
    },
};

void RegisterHANPictureEditToolPaint(HINSTANCE hInst, HBRUSH hBackground)
{
    WNDCLASSEX wcex = {
        .cbSize         = sizeof(WNDCLASSEX),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = PictureEditToolPaintWndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = sizeof(PPICTUREEDITTOOLPAINTWNDEXTRA),
        .hInstance      = hInst,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = hBackground,
        .lpszMenuName   = NULL,
        .lpszClassName  = HAN_PICTURE_EDIT_TOOL_PAINT_CLASS,
        .hIconSm        = NULL,
    };
    RegisterClassEx(&wcex);

    RegisterHANPictureEditToolPaper(hInst);
}

void PictureEditToolPaintSetPaintId(HWND hEditToolPaint, HANSIZE nId)
{
    SendMessage(hEditToolPaint, PETPM_SETPAINTID, (WPARAM)nId, (LPARAM)0);
}
HANSIZE PictureEditToolPaintGetPaintId(HWND hEditToolPaint)
{
    return SendMessage(hEditToolPaint, PETPM_GETPAINTID, (WPARAM)0, (LPARAM)0);
}

static void RegisterHANPictureEditToolPaper(HINSTANCE hInst)
{
    WNDCLASSEX wcex = {
        .cbSize         = sizeof(WNDCLASSEX),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = PictureEditToolPaperWndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = sizeof(PPICTUREEDITTOOLPAPERWNDEXTRA),
        .hInstance      = hInst,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = GetStockObject(GRAY_BRUSH),
        .lpszMenuName   = NULL,
        .lpszClassName  = HAN_PICTURE_EDIT_TOOL_PAPER_CLASS,
        .hIconSm        = NULL,
    };
    RegisterClassEx(&wcex);
}

static LRESULT CALLBACK PictureEditToolPaintWndProc(HWND hPictureEditToolPaint, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo = (PPICTUREEDITTOOLPAINTWNDEXTRA)GetWindowLongPtr(hPictureEditToolPaint, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = PaintCreateCallback(hPictureEditToolPaint, lParam);
        } break;
        case WM_SIZE: {
            PaintSizeCallback(hPictureEditToolPaint, etpInfo);
        } break;
        case WM_HSCROLL: {
            lWndProcRet = PaintHScrollCallback(etpInfo, LOWORD(wParam));
        } break;
        case WM_VSCROLL: {
            lWndProcRet = PaintVScrollCallback(etpInfo, LOWORD(wParam));
        } break;
        case WM_MOUSEWHEEL: {
            PaintMouseWheelCallback(etpInfo, GET_WHEEL_DELTA_WPARAM(wParam), GET_KEYSTATE_WPARAM(wParam));
        } break;
        case WM_COMMAND: {
            PaintCommandCallback(hPictureEditToolPaint, etpInfo, wParam, lParam);
        } break;
        case WM_DESTROY: {
            PaintDestroyCallback(etpInfo);
            lWndProcRet = DefWindowProc(hPictureEditToolPaint, message, wParam, lParam);
        } break;

        case PCTM_ZOOM: {
            ExternalZoomCallback(etpInfo, (HANINT)wParam);
        } break;

        case PETPM_SETPAINTID: {
            etpInfo->picture.nPaintId = (HANSIZE)wParam;
            UpdatePicturePaint(etpInfo);
        } break;
        case PETPM_GETPAINTID: {
            lWndProcRet = (LRESULT)(etpInfo->picture.nPaintId);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hPictureEditToolPaint, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}
static LRESULT CALLBACK PictureEditToolPaperWndProc(HWND hPictureEditToolPaper, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo = (PPICTUREEDITTOOLPAPERWNDEXTRA)GetWindowLongPtr(hPictureEditToolPaper, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = PaperCreateCallback(hPictureEditToolPaper, lParam);
        } break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            RECT rcPaperSize;
            /* 开始绘图 */
            HDC hDC = BeginPaint(hPictureEditToolPaper, &ps);
            /* 绘制内容 */
            GetClientRect(hPictureEditToolPaper, &rcPaperSize);
            PaperPaintCallback(hDC, paperInfo, &rcPaperSize);
            /* 结束绘图 */
            EndPaint(hPictureEditToolPaper, &ps);
        } break;
        case WM_MOUSEMOVE: {
            POINT ptPos = {
                .x = LOWORD(lParam),
                .y = HIWORD(lParam),
            };
            PaperMouseMoveCallback(paperInfo, &ptPos);
        } break;
        case WM_DESTROY: {
            PaperDestroyCallback(paperInfo);
            lWndProcRet = DefWindowProc(hPictureEditToolPaper, message, wParam, lParam);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hPictureEditToolPaper, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}

static LRESULT PaintCreateCallback(HWND hPictureEditToolPaint, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo;
    HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
    HANPPICTUREINFO pPictureInfo = ((LPCREATESTRUCT)lParam)->lpCreateParams;
    RECT rcClientSize;
    RECT rcPaperSize;
    PPICTUREEDITTOOLPAINTPIXEL pPaintPixel;
    PICTURERESOLUTION pxResolution;
    HANSIZE nPaintPixelInfoSize;
    HANSIZE nPaintPictureSize;

    HANINT nWinX;
    HANINT nWinY;
    HANINT nWinW;
    HANINT nWinH;

    HANDLE hHeap = GetProcessHeap();
    if (NULL == hHeap) { lWndProcRet = -1; }
    if (-1 != lWndProcRet)
    {
        GetClientRect(hPictureEditToolPaint, &rcClientSize);
        GetPaintPictureResolution(&pxResolution, pPictureInfo);
        nPaintPixelInfoSize = GetPaintPixelInfoSize(&rcClientSize);
        nPaintPictureSize = GetPictureMemSize(&pxResolution);
        etpInfo = (PPICTUREEDITTOOLPAINTWNDEXTRA)HANWinHeapAlloc(hHeap, NULL, sizeof(PICTUREEDITTOOLPAINTWNDEXTRA) + nPaintPictureSize);
        pPaintPixel = (PPICTUREEDITTOOLPAINTPIXEL)HANWinHeapAlloc(hHeap, NULL, nPaintPixelInfoSize);
        if ((NULL == etpInfo) || (NULL == pPaintPixel))
        {
            if (NULL != etpInfo) { HANWinHeapFree(hHeap, 0, etpInfo); }
            if (NULL != pPaintPixel) { HANWinHeapFree(hHeap, 0, pPaintPixel); }
            lWndProcRet = -1;
        }
    }
    /* 创建窗口 */
    if (-1 != lWndProcRet)
    {
        SetWindowLongPtr(hPictureEditToolPaint, 0, (LONG_PTR)etpInfo);

        etpInfo->hHeap = hHeap;
        etpInfo->hInst = hInst;
        etpInfo->hSelf = hPictureEditToolPaint;
        etpInfo->hEditTool =((LPCREATESTRUCT)lParam)->hwndParent;
        etpInfo->hConsole = NULL;
        etpInfo->picture.nPaintId = 0;
        etpInfo->picture.pInfo = pPictureInfo;
        etpInfo->pPaintPixel = pPaintPixel;
        etpInfo->picture.pPicture = (HANPPICTURE)(etpInfo->pBuf);
        UpdatePaintPixelInfo(pPaintPixel, &rcClientSize);

        etpInfo->hFont.hHex = CreateFontIndirect(&g_lfHexFont);
        etpInfo->hFont.hSys = CreateFontIndirect(&g_lfSysFont);

        UpdatePictureMap(etpInfo->picture.pPicture, &pxResolution);
        UpdatePicturePaint(etpInfo);

        GetPaintPaperRect(pPictureInfo->pPicture[0], &rcClientSize, &rcPaperSize, sg_pZoom[HAN_PICTURE_EDIT_TOOL_DEFAULT_ZOOM].nZoom);
        etpInfo->picture.hPaper = CreateWindow(HAN_PICTURE_EDIT_TOOL_PAPER_CLASS, NULL,
            WS_CHILD | WS_VISIBLE,
            rcPaperSize.left, rcPaperSize.top, GetRectW(&rcPaperSize), GetRectH(&rcPaperSize),
            hPictureEditToolPaint, (HMENU)WID_PICTURE_EDIT_TOOL_PAPER, hInst, etpInfo
        );
        nWinX = rcClientSize.right - HAN_PICTURE_EDIT_TOOL_PAINT_SCROLL_WIDTH;
        nWinY = 0;
        nWinW = HAN_PICTURE_EDIT_TOOL_PAINT_SCROLL_WIDTH;
        nWinH = GetRectH(&rcClientSize) - PICTURE_EDIT_TOOL_ZOOM_EDIT_HEIGHT + 1;
        etpInfo->picture.hVScroll = CreateWindow(TEXT("scrollbar"), NULL,
            WS_CHILD | WS_VISIBLE | SBS_VERT,
            nWinX, nWinY, nWinW, nWinH,
            hPictureEditToolPaint, (HMENU)WID_PICTURE_EDIT_TOOL_VSCROLL, hInst, NULL
        );
        nWinX = 0;
        nWinY = rcClientSize.bottom - HAN_PICTURE_EDIT_TOOL_PAINT_SCROLL_WIDTH;
        nWinW = GetRectW(&rcClientSize) - PICTURE_EDIT_TOOL_ZOOM_EDIT_WIDTH + 1;
        nWinH = HAN_PICTURE_EDIT_TOOL_PAINT_SCROLL_WIDTH;
        etpInfo->picture.hHScroll = CreateWindow(TEXT("scrollbar"), NULL,
            WS_CHILD | WS_VISIBLE | SBS_HORZ,
            nWinX, nWinY, nWinW, nWinH,
            hPictureEditToolPaint, (HMENU)WID_PICTURE_EDIT_TOOL_HSCROLL, hInst, NULL
        );

        nWinX = GetRectW(&rcClientSize) - PICTURE_EDIT_TOOL_ZOOM_EDIT_WIDTH + 1;
        nWinY = GetRectH(&rcClientSize) - PICTURE_EDIT_TOOL_ZOOM_EDIT_HEIGHT + 1;
        nWinW = PICTURE_EDIT_TOOL_ZOOM_EDIT_WIDTH;
        nWinH = PICTURE_EDIT_TOOL_ZOOM_OPTION_HEIGHT * PICTURE_EDIT_TOOL_ZOOM_CNT;
        etpInfo->zoom.hZoom = CreateWindow(TEXT("combobox"), NULL,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
            nWinX, nWinY, nWinW, nWinH,
            hPictureEditToolPaint, (HMENU)WID_PICTURE_EDIT_TOOL_ZOOM, hInst, NULL
        );

        SendMessage(etpInfo->zoom.hZoom, WM_SETFONT, (WPARAM)(etpInfo->hFont.hSys), (LPARAM)TRUE);

        etpInfo->zoom.nZoomId = HAN_PICTURE_EDIT_TOOL_DEFAULT_ZOOM;
        ComboBoxAddStringStructArr(etpInfo->zoom.hZoom, sg_pZoom, sizeof(PICTUREEDITTOOLPAINTZOOM), StructMemberOffset(PICTUREEDITTOOLPAINTZOOM, pText), ArrLen(sg_pZoom));
        ComboBoxSetCursel(etpInfo->zoom.hZoom, etpInfo->zoom.nZoomId);
        UpdatePaintScroll(etpInfo, GetPaintPicture(etpInfo), &rcClientSize, sg_pZoom[etpInfo->zoom.nZoomId].nZoom);
    }

    return lWndProcRet;
}
static void PaintSizeCallback(HWND hPictureEditToolPaint, PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo)
{
    PPICTUREEDITTOOLPAINTPIXEL pPaintPixel = etpInfo->pPaintPixel;
    HANPPICTURE pPicture = GetPaintPicture(etpInfo);
    HANSIZE nPaintPixelSize;
    RECT rcPaintSize;
    RECT rcPaintClientSize;
    RECT rcPaperSize;

    HANINT nWinX;
    HANINT nWinY;
    HANINT nWinW;
    HANINT nWinH;

    GetClientRect(hPictureEditToolPaint, &rcPaintSize);
    GetPaintClientRect(&rcPaintClientSize, &rcPaintSize);
    nPaintPixelSize = GetPaintPixelDataSize(&rcPaintClientSize);
    if (pPaintPixel->nPixelSize < nPaintPixelSize)
    {
        pPaintPixel = (PPICTUREEDITTOOLPAINTPIXEL)HANWinHeapAlloc(etpInfo->hHeap, pPaintPixel, GetPaintPixelInfoSize(&rcPaintClientSize));
        if (NULL != pPaintPixel)
        {
            UpdatePaintPixelInfo(pPaintPixel, &rcPaintClientSize);
            etpInfo->pPaintPixel = pPaintPixel;
        }
    }

    if (NULL != pPaintPixel)
    {
        GetPaintPaperRect(pPicture, &rcPaintClientSize, &rcPaperSize, sg_pZoom[ComboBoxGetCursel(etpInfo->zoom.hZoom)].nZoom);
        nWinX = rcPaperSize.left;
        nWinY = rcPaperSize.top;
        nWinW = GetRectW(&rcPaperSize);
        nWinH = GetRectH(&rcPaperSize);
        SetWindowPos(etpInfo->picture.hPaper, HWND_BOTTOM, nWinX, nWinY, nWinW, nWinH, 0);

        nWinX = GetRectW(&rcPaintSize) - PICTURE_EDIT_TOOL_ZOOM_EDIT_WIDTH + 1;
        nWinY = GetRectH(&rcPaintSize) - PICTURE_EDIT_TOOL_ZOOM_EDIT_HEIGHT + 1;
        nWinW = PICTURE_EDIT_TOOL_ZOOM_EDIT_WIDTH;
        nWinH = PICTURE_EDIT_TOOL_ZOOM_OPTION_HEIGHT * PICTURE_EDIT_TOOL_ZOOM_CNT;
        SetWindowPos(etpInfo->zoom.hZoom, HWND_TOP, nWinX, nWinY, nWinW, nWinH, 0);
        
        MovePaintScroll(etpInfo, &rcPaintSize);
        UpdatePaintScroll(etpInfo, pPicture, &rcPaintClientSize, sg_pZoom[etpInfo->zoom.nZoomId].nZoom);
    }
}
static LRESULT PaintHScrollCallback(PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo, WORD nReq)
{
    LRESULT lRet = (LRESULT)FALSE;
    HANINT nPos = 0;
    SCROLLINFO siScroll = { .cbSize = sizeof(SCROLLINFO), };

    switch (nReq) {
        case SB_THUMBTRACK: {
            siScroll.fMask = SIF_TRACKPOS;
            GetScrollInfo(etpInfo->picture.hHScroll, SB_CTL, &siScroll);
            nPos = siScroll.nTrackPos;
        } break;
        case SB_LINELEFT: {
            siScroll.fMask = SIF_POS | SIF_RANGE;
            GetScrollInfo(etpInfo->picture.hHScroll, SB_CTL, &siScroll);
            nPos = siScroll.nPos - HAN_PICTURE_EDIT_TOOL_PAINT_SCROLL_UNIT;
        } break;
        case SB_LINERIGHT: {
            siScroll.fMask = SIF_POS | SIF_RANGE;
            GetScrollInfo(etpInfo->picture.hHScroll, SB_CTL, &siScroll);
            nPos = siScroll.nPos + HAN_PICTURE_EDIT_TOOL_PAINT_SCROLL_UNIT;
        } break;
        case SB_PAGELEFT: {
            siScroll.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
            GetScrollInfo(etpInfo->picture.hHScroll, SB_CTL, &siScroll);
            nPos = siScroll.nPos - (HANINT)(siScroll.nPage);
        } break;
        case SB_PAGERIGHT: {
            siScroll.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
            GetScrollInfo(etpInfo->picture.hHScroll, SB_CTL, &siScroll);
            nPos = siScroll.nPos + (HANINT)(siScroll.nPage);
        } break;

        default: {
            lRet = (LRESULT)TRUE;
        } break;
    }

    if ((LRESULT)FALSE == lRet)
    {
        siScroll.fMask = SIF_POS;
        siScroll.nPos = nPos;
        SetScrollInfo(etpInfo->picture.hHScroll, SB_CTL, &siScroll, TRUE);
        RedrawWindow(etpInfo->picture.hPaper, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
    }

    return lRet;
}
static LRESULT PaintVScrollCallback(PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo, WORD nReq)
{
    LRESULT lRet = (LRESULT)FALSE;
    HANINT nPos = 0;
    SCROLLINFO siScroll = { .cbSize = sizeof(SCROLLINFO), };

    switch (nReq) {
        case SB_THUMBTRACK: {
            siScroll.fMask = SIF_TRACKPOS;
            GetScrollInfo(etpInfo->picture.hVScroll, SB_CTL, &siScroll);
            nPos = siScroll.nTrackPos;
        } break;
        case SB_LINEUP: {
            siScroll.fMask = SIF_POS | SIF_RANGE;
            GetScrollInfo(etpInfo->picture.hVScroll, SB_CTL, &siScroll);
            nPos = siScroll.nPos - HAN_PICTURE_EDIT_TOOL_PAINT_SCROLL_UNIT;
        } break;
        case SB_LINEDOWN: {
            siScroll.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
            GetScrollInfo(etpInfo->picture.hVScroll, SB_CTL, &siScroll);
            nPos = siScroll.nPos + HAN_PICTURE_EDIT_TOOL_PAINT_SCROLL_UNIT;
        } break;
        case SB_PAGEUP: {
            siScroll.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
            GetScrollInfo(etpInfo->picture.hVScroll, SB_CTL, &siScroll);
            nPos = siScroll.nPos - (HANINT)(siScroll.nPage);
        } break;
        case SB_PAGEDOWN: {
            siScroll.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
            GetScrollInfo(etpInfo->picture.hVScroll, SB_CTL, &siScroll);
            nPos = siScroll.nPos + (HANINT)(siScroll.nPage);
        } break;

        default: {
            lRet = (LRESULT)TRUE;
        } break;
    }

    if ((LRESULT)FALSE == lRet)
    {
        siScroll.fMask = SIF_POS;
        siScroll.nPos = nPos;
        SetScrollInfo(etpInfo->picture.hVScroll, SB_CTL, &siScroll, TRUE);
        RedrawWindow(etpInfo->picture.hPaper, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
    }

    return lRet;
}
static LRESULT PaintMouseWheelCallback(PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo, HANINT nUnit, WORD nVK)
{
    LRESULT lRet = FALSE;
    HANINT nStep = nUnit / WHEEL_DELTA;
    WORD nReq;

    switch (nVK) {
        case 0: {
            if (nStep < 0)
            {
                nStep = -nStep;
                nReq = SB_LINEDOWN;
            }
            else { nReq = SB_LINEUP; }
            for (HANINT iLoop = 0; iLoop < nStep; iLoop++) { PaintVScrollCallback(etpInfo, nReq); }
        } break;
        case MK_CONTROL: {
            if (nStep < 0)
            {
                nStep = -nStep;
                nReq = SB_LINERIGHT;
            }
            else { nReq = SB_LINELEFT; }
            for (HANINT iLoop = 0; iLoop < nStep; iLoop++) { PaintHScrollCallback(etpInfo, nReq); }
        } break;
        default: { lRet = TRUE; } break;
    }

    return lRet;
}
static void PaintCommandCallback(HWND hPictureEditToolPaint, PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo, WPARAM wParam, LPARAM lParam)
{
    (void)hPictureEditToolPaint;
    (void)lParam;
    
    switch (LOWORD(wParam)) {
        case WID_PICTURE_EDIT_TOOL_ZOOM: {
            ZoomCallback(etpInfo, HIWORD(wParam));
        } break;

        default: { } break;
    }
}
static void PaintDestroyCallback(PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo)
{
    HANWinHeapFree(etpInfo->hHeap, 0, etpInfo->pPaintPixel);
    HANWinHeapFree(etpInfo->hHeap, 0, etpInfo);
}
static void ExternalZoomCallback(PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo, HANINT nZoom)
{
    HANINT nCurrentZoom = ComboBoxGetCursel(etpInfo->zoom.hZoom);
    HANINT nTargetZoom;
    
    nTargetZoom = nCurrentZoom + nZoom;

    if (nTargetZoom < 0) { nTargetZoom = 0; }
    if (PICTURE_EDIT_TOOL_ZOOM_CNT <= nTargetZoom) { nTargetZoom = PICTURE_EDIT_TOOL_ZOOM_CNT - 1; }

    if (nTargetZoom != nCurrentZoom)
    {
        ComboBoxSetCursel(etpInfo->zoom.hZoom, nTargetZoom);
        ZoomCallback(etpInfo, CBN_SELCHANGE);
    }
}
static HANSIZE GetPaintPixelInfoSize(const RECT* pPaintRect)
{
    LONG nWinW = GetRectW(pPaintRect);
    LONG nWinH = GetRectH(pPaintRect);

    return (sizeof(PICTUREEDITTOOLPAINTPIXEL) + (nWinW * nWinH * (sizeof(POINT) + sizeof(PICTUREWINPIXEL))));
}
static HANSIZE GetPaintPixelDataSize(const RECT* pPaintRect)
{
    LONG nWinW = GetRectW(pPaintRect);
    LONG nWinH = GetRectH(pPaintRect);

    return (nWinW * nWinH * sizeof(PICTUREWINPIXEL));
}
static void UpdatePaintPixelInfo(PPICTUREEDITTOOLPAINTPIXEL pPaintPixel, const RECT* pPaintRect)
{
    pPaintPixel->nPixelSize = GetPaintPixelDataSize(pPaintRect);
    pPaintPixel->pPixel = (PPICTUREWINPIXEL)(pPaintPixel->pBuf);
    pPaintPixel->pPicturePos = (PPOINT)&(pPaintPixel->pBuf[pPaintPixel->nPixelSize]);
}
static inline void GetPaintClientRect(RECT* pClient, const RECT* pPaint)
{
    pClient->left = pPaint->left;
    pClient->top = pPaint->top;
    pClient->right = pPaint->right - HAN_PICTURE_EDIT_TOOL_PAINT_SCROLL_WIDTH;
    pClient->bottom = pPaint->bottom - HAN_PICTURE_EDIT_TOOL_PAINT_SCROLL_WIDTH;
}
static void MovePaintScroll(PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo, const RECT* pPaintRect)
{
    HANINT nX;
    HANINT nY;
    HANINT nW;
    HANINT nH;
    /* 垂直 */
    nX = pPaintRect->right - HAN_PICTURE_EDIT_TOOL_PAINT_SCROLL_WIDTH;
    nY = 0;
    nW = HAN_PICTURE_EDIT_TOOL_PAINT_SCROLL_WIDTH;
    nH = GetRectH(pPaintRect) - PICTURE_EDIT_TOOL_ZOOM_EDIT_HEIGHT + 1;
    MoveWindow(etpInfo->picture.hVScroll, nX, nY, nW, nH, FALSE);
    /* 水平 */
    nX = 0;
    nY = pPaintRect->bottom - HAN_PICTURE_EDIT_TOOL_PAINT_SCROLL_WIDTH;
    nW = GetRectW(pPaintRect) - PICTURE_EDIT_TOOL_ZOOM_EDIT_WIDTH + 1;
    nH = HAN_PICTURE_EDIT_TOOL_PAINT_SCROLL_WIDTH;
    MoveWindow(etpInfo->picture.hHScroll, nX, nY, nW, nH, FALSE);
}
static void GetPaintPictureResolution(PPICTURERESOLUTION pResolution, HANPCPICTUREINFO pPictureInfo)
{
    *pResolution = pPictureInfo->pPicture[0]->pxResolution;
    for (HANSIZE iLoop = 1; iLoop < pPictureInfo->nCnt; iLoop++)
    {
        if (pResolution->pxWidth < pPictureInfo->pPicture[iLoop]->pxResolution.pxWidth) { pResolution->pxWidth = pPictureInfo->pPicture[iLoop]->pxResolution.pxWidth; }
        if (pResolution->pxHeight < pPictureInfo->pPicture[iLoop]->pxResolution.pxHeight) { pResolution->pxHeight = pPictureInfo->pPicture[iLoop]->pxResolution.pxHeight; }
    }
}
static void ZoomCallback(PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo, WORD nCommand)
{
    RECT rcPaintSize;
    RECT rcPaintClientSize;
    RECT rcPaperSize;
    HANINT nZoomId;
    HANINT nZoom;
    HANPPICTURE pPicture = GetPaintPicture(etpInfo);

    if (CBN_SELCHANGE == nCommand)
    {
        nZoomId = ComboBoxGetCursel(etpInfo->zoom.hZoom);
        if (nZoomId != etpInfo->zoom.nZoomId)
        {
            nZoom = sg_pZoom[nZoomId].nZoom;
            etpInfo->zoom.nZoomId = nZoomId;
            GetClientRect(etpInfo->hSelf, &rcPaintSize);
            GetPaintClientRect(&rcPaintClientSize, &rcPaintSize);
            GetPaintPaperRect(pPicture, &rcPaintClientSize, &rcPaperSize, nZoom);
            SetWindowPos(etpInfo->picture.hPaper, HWND_BOTTOM, rcPaperSize.left, rcPaperSize.top, GetRectW(&rcPaperSize), GetRectH(&rcPaperSize), 0);
            UpdatePaintScroll(etpInfo, pPicture, &rcPaintClientSize, nZoom);
        }
    }
}
static void UpdatePaintScroll(PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo, HANPPICTURE pPicture, const RECT* pPaintSize, HANINT nZoom)
{
    PPICTURERESOLUTION pResolution = &(pPicture->pxResolution);
    UINT nPaintW = (UINT)GetRectW(pPaintSize);
    UINT nPaintH = (UINT)GetRectH(pPaintSize);
    HANINT nPictureZoomW = GetZoomSize(pResolution->pxWidth, nZoom);
    HANINT nPictureZoomH = GetZoomSize(pResolution->pxHeight, nZoom);
    SCROLLINFO siOldScroll = {
        .cbSize = sizeof(SCROLLINFO),
        .fMask = SIF_RANGE | SIF_PAGE | SIF_POS,
        .nMin = 0,
    };
    SCROLLINFO siNewScroll = {
        .cbSize = sizeof(SCROLLINFO),
        .fMask = SIF_RANGE | SIF_PAGE | SIF_POS,
        .nMin = 0,
    };
    
    /* 更新垂直滚动条 */
    GetScrollInfo(etpInfo->picture.hVScroll, SB_CTL, &siOldScroll);
    siNewScroll.nMax = nPictureZoomH - 1;
    siNewScroll.nPage = nPaintH;
    siNewScroll.nPos = CalcScrollPos(&siOldScroll, &siNewScroll);
    SetScrollInfo(etpInfo->picture.hVScroll, SB_CTL, &siNewScroll, FALSE);
    /* 更新水平滚动条 */
    GetScrollInfo(etpInfo->picture.hHScroll, SB_CTL, &siOldScroll);
    siNewScroll.nMax = nPictureZoomW - 1;
    siNewScroll.nPage = nPaintW;
    siNewScroll.nPos = CalcScrollPos(&siOldScroll, &siNewScroll);
    SetScrollInfo(etpInfo->picture.hHScroll, SB_CTL, &siNewScroll, FALSE);

    RedrawWindow(etpInfo->hSelf, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
}
static HANINT CalcScrollPos(const SCROLLINFO* pOldScroll, const SCROLLINFO* pNewScroll)
{
    HANDOUBLE nOldHalfPage = (HANDOUBLE)(pOldScroll->nPage) / (HANDOUBLE)2;
    HANDOUBLE nNewHalfPage = (HANDOUBLE)(pNewScroll->nPage) / (HANDOUBLE)2;
    HANDOUBLE kZoom = (HANDOUBLE)(pNewScroll->nMax + 1) / (HANDOUBLE)(pOldScroll->nMax + 1);
    
    return (HANINT)((pOldScroll->nPos + nOldHalfPage) * kZoom - nNewHalfPage);
}
static uint32_t GetZoomSize(uint32_t nSize, HANINT nZoom)
{
    uint32_t nRet;

    if (0 < nZoom) { nRet = nSize * nZoom; }
    else { nRet = nSize / (-nZoom); }

    return nRet;
}
static LONG GetRestoreSize(LONG nSize, HANINT nZoom)
{
    LONG nRet;

    if (0 < nZoom) { nRet = nSize / nZoom; }
    else { nRet = nSize * (-nZoom); }

    return nRet;
}
static HANPPICTURE GetPaintPicture(PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo)
{
    return etpInfo->picture.pInfo->pPicture[etpInfo->picture.nPaintId];
}
static void UpdatePicturePaint(PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo)
{
    HANPPICTURE pPicture = GetPaintPicture(etpInfo);
    PPICTURERGBA* pSrc = pPicture->pPictureMap;
    PPICTURERGBA* pDest = etpInfo->picture.pPicture->pPictureMap;
    HANSIZE nOffset;

    nOffset = 0;
    for (uint32_t nRow = 0; nRow < pPicture->pxResolution.pxHeight; nRow++)
    {
        for (uint32_t nCol = 0; nCol < pPicture->pxResolution.pxWidth; nCol++)
        {
            pDest[nRow][nCol] = pSrc[nRow][nCol];
            nOffset++;
        }
    }
}

static LRESULT PaperCreateCallback(HWND hPictureEditToolPaper, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo;
    HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
    PPICTUREEDITTOOLPAINTWNDEXTRA pParent = ((LPCREATESTRUCT)lParam)->lpCreateParams;

    HANDLE hHeap = GetProcessHeap();
    if (NULL == hHeap) { lWndProcRet = -1; }
    if (-1 != lWndProcRet)
    {
        paperInfo = (PPICTUREEDITTOOLPAPERWNDEXTRA)HANWinHeapAlloc(hHeap, NULL, sizeof(PICTUREEDITTOOLPAPERWNDEXTRA));
        if (NULL == paperInfo) { lWndProcRet = -1; }
    }
    /* 创建窗口 */
    if (-1 != lWndProcRet)
    {
        SetWindowLongPtr(hPictureEditToolPaper, 0, (LONG_PTR)paperInfo);

        paperInfo->hHeap = hHeap;
        paperInfo->hInst = hInst;
        paperInfo->hSelf = hPictureEditToolPaper;
        paperInfo->pPaint = pParent;
    }

    return lWndProcRet;
}
static void PaperPaintCallback(HDC hDC, PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo, const RECT* pPaperSize)
{
    BITMAPINFO bmiPaint;
    PICTUREWINPIXEL* pPixel = paperInfo->pPaint->pPaintPixel->pPixel;
    HANPPICTURE pPicture = GetPaintPicture(paperInfo->pPaint);
    LONG nWinW = GetRectW(pPaperSize);
    LONG nWinH = GetRectH(pPaperSize);
    RECT rcPictureSize;
    LONG nZoomId = ComboBoxGetCursel(paperInfo->pPaint->zoom.hZoom);

    GetPaintPictureRect(paperInfo->pPaint, pPicture, &rcPictureSize);

    PaperPaintBackground(paperInfo, pPaperSize);
    sg_pZoom[nZoomId].Callback(paperInfo, pPaperSize, &rcPictureSize);

    ZeroMemory(&bmiPaint, sizeof(bmiPaint));
    bmiPaint.bmiHeader.biSize = sizeof(BITMAPINFO);
    bmiPaint.bmiHeader.biWidth = nWinW;
    bmiPaint.bmiHeader.biHeight = -nWinH;
    bmiPaint.bmiHeader.biPlanes = 1;
    bmiPaint.bmiHeader.biBitCount = 32;
    bmiPaint.bmiHeader.biCompression = BI_RGB;
    bmiPaint.bmiHeader.biSizeImage = (DWORD)(paperInfo->pPaint->pPaintPixel->nPixelSize);
    StretchDIBits(hDC, 0, 0, nWinW, nWinH, 0, 0, nWinW, nWinH, pPixel, (BITMAPINFO*)&bmiPaint.bmiHeader, DIB_RGB_COLORS, SRCCOPY);
    
    /* 图片窗口和缩放列表窗口有部分重叠，重绘图片会导致缩放窗口被部分绘制，视觉上就是图片遮住了缩放按钮，所以在绘制完图片后需要重绘缩放窗口 */
    RedrawWindow(paperInfo->pPaint->zoom.hZoom, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
}
static void PaperMouseMoveCallback(PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo, PPOINT pPos)
{
    PPOINT pPixelPos = paperInfo->pPaint->pPaintPixel->pPicturePos;
    HANPPICTURE pPicture = GetPaintPicture(paperInfo->pPaint);
    PPICTURERGBA* pPictureMap = pPicture->pPictureMap;
    HANCHAR pText[HAN_PICTURE_EDIT_TOOL_TEXT_BUF_SIZE];
    PPICTURERGBA pRGBA;
    RECT rcPaperSize;
    HANSIZE nOffset;

    GetClientRect(paperInfo->hSelf, &rcPaperSize);
    nOffset = pPos->y * GetRectW(&rcPaperSize) + pPos->x;
    pPixelPos = &pPixelPos[nOffset];
    pRGBA = &(pPictureMap[pPixelPos->y][pPixelPos->x]);

    HAN_snprintf(pText, HAN_PICTURE_EDIT_TOOL_TEXT_BUF_SIZE, TEXT("位置（%ld, %ld）  颜色（R:%d, G:%d, B:%d, A:%d）"),
        pPixelPos->x, pPixelPos->y,
        pRGBA->r, pRGBA->g, pRGBA->b, pRGBA->a
    );
    PictureEditToolSetPixelInfo(paperInfo->pPaint->hEditTool, pText);
}
static void PaperDestroyCallback(PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo)
{
    HANWinHeapFree(paperInfo->hHeap, 0, paperInfo);
}
static void PaperPaintBackground(PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo, const RECT* pPaper)
{
    PICTUREWINPIXEL* pPixel = paperInfo->pPaint->pPaintPixel->pPixel;
    HANSIZE nAlphaStartIdH;
    HANSIZE nAlphaStartIdV;
    HANSIZE nAlphaStartOffsetIdH;
    HANSIZE nAlphaStartOffsetIdV;
    HANSIZE nAlphaIdH;
    HANSIZE nAlphaIdV;
    HANSIZE nCntH;
    HANSIZE nCntV;
    HANSIZE nOffset;
    SCROLLINFO siV = { .cbSize = sizeof(SCROLLINFO), .fMask = SIF_POS, };
    SCROLLINFO siH = { .cbSize = sizeof(SCROLLINFO), .fMask = SIF_POS, };

    GetScrollInfo(paperInfo->pPaint->picture.hVScroll, SB_CTL, &siV);
    GetScrollInfo(paperInfo->pPaint->picture.hHScroll, SB_CTL, &siH);
    nAlphaStartIdV = (siV.nPos / PICTURE_EDIT_TOOL_ALPHA_BG_SIZE) % ArrLen(sg_pAlphaColor);
    nAlphaStartIdH = (siH.nPos / PICTURE_EDIT_TOOL_ALPHA_BG_SIZE) % ArrLen(sg_pAlphaColor);
    nAlphaStartOffsetIdV = siV.nPos % PICTURE_EDIT_TOOL_ALPHA_BG_SIZE;
    nAlphaStartOffsetIdH = siH.nPos % PICTURE_EDIT_TOOL_ALPHA_BG_SIZE;

    nOffset = 0;
    nAlphaIdV = nAlphaStartIdV;
    nCntV = nAlphaStartOffsetIdV;
    for (LONG iLoop = pPaper->top; iLoop < pPaper->bottom; iLoop++)
    {
        nCntH = nAlphaStartOffsetIdH;
        nAlphaIdH = (nAlphaIdV + nAlphaStartIdH) % ArrLen(sg_pAlphaColor);
        for (LONG jLoop = pPaper->left; jLoop < pPaper->right; jLoop++)
        {
            pPixel[nOffset].ptRGBA = sg_pAlphaColor[nAlphaIdH];
            nCntH++;
            if (PICTURE_EDIT_TOOL_ALPHA_BG_SIZE <= nCntH)
            {
                nCntH = 0;
                nAlphaIdH++;
                if (ArrLen(sg_pAlphaColor) <= nAlphaIdH) { nAlphaIdH = 0; }
            }
            nOffset++;
        }
        nCntV++;
        if (PICTURE_EDIT_TOOL_ALPHA_BG_SIZE <= nCntV)
        {
            nCntV = 0;
            nAlphaIdV++;
            if (ArrLen(sg_pAlphaColor) <= nAlphaIdV) { nAlphaIdV = 0; }
        }
    }
}

static void PaintCallbackZoom001250(PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo, const RECT* pPaperSize, const RECT* pPictureSize)
{
    PaintCallbackZoomOut(paperInfo, pPaperSize, pPictureSize, 8);
}
static void PaintCallbackZoom002500(PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo, const RECT* pPaperSize, const RECT* pPictureSize)
{
    PaintCallbackZoomOut(paperInfo, pPaperSize, pPictureSize, 4);
}
static void PaintCallbackZoom005000(PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo, const RECT* pPaperSize, const RECT* pPictureSize)
{
    PaintCallbackZoomOut(paperInfo, pPaperSize, pPictureSize, 2);
}
static void PaintCallbackZoom010000(PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo, const RECT* pPaperSize, const RECT* pPictureSize)
{
    (void)pPaperSize;
    PaintCallbackZoomOut(paperInfo, pPaperSize, pPictureSize, 1);
}
static void PaintCallbackZoom020000(PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo, const RECT* pPaperSize, const RECT* pPictureSize)
{
    PaintCallbackZoomIn(paperInfo, pPaperSize, pPictureSize, 2);
}
static void PaintCallbackZoom030000(PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo, const RECT* pPaperSize, const RECT* pPictureSize)
{
    PaintCallbackZoomIn(paperInfo, pPaperSize, pPictureSize, 3);
}
static void PaintCallbackZoom040000(PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo, const RECT* pPaperSize, const RECT* pPictureSize)
{
    PaintCallbackZoomIn(paperInfo, pPaperSize, pPictureSize, 4);
}
static void PaintCallbackZoom050000(PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo, const RECT* pPaperSize, const RECT* pPictureSize)
{
    PaintCallbackZoomIn(paperInfo, pPaperSize, pPictureSize, 5);
}
static void PaintCallbackZoom100000(PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo, const RECT* pPaperSize, const RECT* pPictureSize)
{
    PaintCallbackZoomIn(paperInfo, pPaperSize, pPictureSize, 10);
}

static void PaintCallbackZoomOut(PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo, const RECT* pPaperSize, const RECT* pPictureSize, LONG nZoom)
{
    HANPPICTURE pPicture = paperInfo->pPaint->picture.pPicture;
    PPICTUREEDITTOOLPAINTPIXEL pPaintPixel = paperInfo->pPaint->pPaintPixel;
    PPOINT pPicturePos = pPaintPixel->pPicturePos;
    PICTUREWINPIXEL* pPixel = pPaintPixel->pPixel;
    PPICTURERGBA* pRGBA = pPicture->pPictureMap;
    HANSIZE nOffset = 0;
    HANSIZE nRow;
    HANSIZE nCol;

    nRow = pPictureSize->top;
    for (LONG iLoop = pPaperSize->top; iLoop < pPaperSize->bottom; iLoop++)
    {
        nCol = pPictureSize->left;
        for (LONG jLoop = pPaperSize->left; jLoop < pPaperSize->right; jLoop++)
        {
            pPicturePos[nOffset].x = (LONG)nCol;
            pPicturePos[nOffset].y = (LONG)nRow;
            pPixel[nOffset].ptRGBA = UpdateRGBA(pRGBA[nRow][nCol], pPixel[nOffset].ptRGBA, paperInfo->pPaint);
            nCol += nZoom;
            nOffset++;
        }
        nRow += nZoom;
    }
}
static void PaintCallbackZoomIn(PPICTUREEDITTOOLPAPERWNDEXTRA paperInfo, const RECT* pPaperSize, const RECT* pPictureSize, LONG nZoom)
{
    HANPPICTURE pPicture = paperInfo->pPaint->picture.pPicture;
    PPICTUREEDITTOOLPAINTPIXEL pPaintPixel = paperInfo->pPaint->pPaintPixel;
    PPOINT pPicturePos = pPaintPixel->pPicturePos;
    PICTUREWINPIXEL* pPixel = pPaintPixel->pPixel;
    PPICTURERGBA* pRGBA = pPicture->pPictureMap;
    HANSIZE nOffset = 0;
    HANSIZE nRow;
    HANSIZE nCol;
    LONG nStartX;
    LONG nStartY;
    LONG nCntX;
    LONG nCntY;
    SCROLLINFO siV = { .cbSize = sizeof(SCROLLINFO), .fMask = SIF_POS, };
    SCROLLINFO siH = { .cbSize = sizeof(SCROLLINFO), .fMask = SIF_POS, };

    GetScrollInfo(paperInfo->pPaint->picture.hVScroll, SB_CTL, &siV);
    GetScrollInfo(paperInfo->pPaint->picture.hHScroll, SB_CTL, &siH);
    nStartX = siH.nPos % nZoom;
    nStartY = siV.nPos % nZoom;

    nCntY = nStartY;
    nRow = pPictureSize->top;
    for (LONG iLoop = pPaperSize->top; iLoop < pPaperSize->bottom; iLoop++)
    {
        nCntX = nStartX;
        nCol = pPictureSize->left;
        for (LONG jLoop = pPaperSize->left; jLoop < pPaperSize->right; jLoop++)
        {
            pPicturePos[nOffset].x = (LONG)nCol;
            pPicturePos[nOffset].y = (LONG)nRow;
            pPixel[nOffset].ptRGBA = UpdateRGBA(pRGBA[nRow][nCol], pPixel[nOffset].ptRGBA, paperInfo->pPaint);
            nCntX++;
            if (nZoom <= nCntX)
            {
                nCntX = 0;
                nCol++;
            }
            nOffset++;
        }
        nCntY++;
        if (nZoom <= nCntY)
        {
            nCntY = 0;
            nRow++;
        }
    }
}

static void GetPaintPaperRect(HANPCPICTURE pPicture, const RECT* pPaintSize, RECT* pPaperSize, LONG nZoom)
{
    LONG nPaintW = GetRectW(pPaintSize);
    LONG nPaintH = GetRectH(pPaintSize);
    LONG nPaperW = pPicture->pxResolution.pxWidth;
    LONG nPaperH = pPicture->pxResolution.pxHeight;
    POINT ptCenter = {
        .x = nPaintW / 2,
        .y = nPaintH / 2,
    };

    if (0 < nZoom)
    {
        nPaperW *= nZoom;
        nPaperH *= nZoom;
    }
    else
    {
        nPaperW /= (-nZoom);
        nPaperH /= (-nZoom);
    }

    pPaperSize->left = ptCenter.x - (nPaperW / 2);
    pPaperSize->top = ptCenter.y - (nPaperH / 2);
    pPaperSize->right = pPaperSize->left + nPaperW;
    pPaperSize->bottom = pPaperSize->top + nPaperH;

    if (pPaperSize->left < pPaintSize->left) { pPaperSize->left = pPaintSize->left; }
    if (pPaperSize->top < pPaintSize->top) { pPaperSize->top = pPaintSize->top; }
    if (pPaintSize->right < pPaperSize->right) { pPaperSize->right = pPaintSize->right; }
    if (pPaintSize->bottom < pPaperSize->bottom) { pPaperSize->bottom = pPaintSize->bottom; }
}
static void GetPaintPictureRect(PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo, HANPCPICTURE pPicture, RECT* pPictureSize)
{
    /* 滚动条的范围是缩放后的图片大小，Page是显示的窗口大小 */
    LONG nPictureW = pPicture->pxResolution.pxWidth;
    LONG nPictureH = pPicture->pxResolution.pxHeight;
    HANINT nZoom = sg_pZoom[etpInfo->zoom.nZoomId].nZoom;
    SCROLLINFO siV = { .cbSize = sizeof(SCROLLINFO), .fMask = SIF_POS | SIF_PAGE, };
    SCROLLINFO siH = { .cbSize = sizeof(SCROLLINFO), .fMask = SIF_POS | SIF_PAGE, };

    GetScrollInfo(etpInfo->picture.hVScroll, SB_CTL, &siV);
    GetScrollInfo(etpInfo->picture.hHScroll, SB_CTL, &siH);

    pPictureSize->left = siH.nPos;
    pPictureSize->top = siV.nPos;
    pPictureSize->right = pPictureSize->left + siH.nPage;
    pPictureSize->bottom = pPictureSize->top + siV.nPage;

    pPictureSize->left = GetRestoreSize(pPictureSize->left, nZoom);
    pPictureSize->top = GetRestoreSize(pPictureSize->top, nZoom);
    pPictureSize->right = GetRestoreSize(pPictureSize->right, nZoom);
    pPictureSize->bottom = GetRestoreSize(pPictureSize->bottom, nZoom);

    if (pPictureSize->left < 0) { pPictureSize->left = 0; }
    if (pPictureSize->top < 0) { pPictureSize->top = 0; }
    if (nPictureW < pPictureSize->right) { pPictureSize->right = nPictureW; }
    if (nPictureH < pPictureSize->bottom) { pPictureSize->bottom = nPictureH; }
}
static inline PICTURERGBA UpdateRGBA(PICTURERGBA rgbaFore, PICTURERGBA rgbaBack, PPICTUREEDITTOOLPAINTWNDEXTRA etpInfo)
{
    (void)etpInfo;
    PICTURERGBA rgbaRet;

    rgbaRet = BlendRGBA(rgbaFore, rgbaBack);

    return rgbaRet;
}
