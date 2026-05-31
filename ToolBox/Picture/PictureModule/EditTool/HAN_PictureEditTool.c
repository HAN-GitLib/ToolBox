#include <Windows.h>
#include <CommCtrl.h>

#include "HAN_PictureEditTool.h"
#include "HAN_PictureEditToolPaint.h"

typedef struct tagPICTUREPNGWNDEXTRA {
    HANDLE                          hHeap;
    HINSTANCE                       hInst;
    HWND                            hPicture;
    HWND                            hSelf;
    HWND                            hPaint;
    HWND                            hConsole;
    HANPPICTUREINFO                 pPictureInfo;
} PICTUREEDITTOOLWNDEXTRA, * PPICTUREEDITTOOLWNDEXTRA;

static LRESULT CALLBACK PictureEditToolWndProc(HWND hPictureEditTool, UINT message, WPARAM wParam, LPARAM lParam);

static LRESULT CreateCallback(HWND hPictureEditTool, LPARAM lParam);
static void SizeCallback(HWND hPictureEditTool, PPICTUREEDITTOOLWNDEXTRA etInfo);
static void DestroyCallback(PPICTUREEDITTOOLWNDEXTRA etInfo);
static void GetPaintRect(const RECT* pPaint, RECT* pPaper);

static BOOL s_PictureEditToolSetPaintId(PPICTUREEDITTOOLWNDEXTRA etInfo, HANSIZE nId);
static HANSIZE s_PictureEditToolGetPaintId(PPICTUREEDITTOOLWNDEXTRA etInfo);
static void s_PictureEditToolSetPixelInfo(PPICTUREEDITTOOLWNDEXTRA etInfo, HANPSTR pPixelInfo);

void RegisterHANPictureEditTool(HINSTANCE hInst, HBRUSH hBackground)
{
    WNDCLASSEX wcex = {
        .cbSize         = sizeof(WNDCLASSEX),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = PictureEditToolWndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = sizeof(PPICTUREEDITTOOLWNDEXTRA),
        .hInstance      = hInst,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = (HBRUSH)GetStockObject(PICTURE_BACKGROUND_BRUSH),
        .lpszMenuName   = NULL,
        .lpszClassName  = HAN_PICTURE_EDIT_TOOL_CLASS,
        .hIconSm        = NULL,
    };
    RegisterClassEx(&wcex);

    RegisterHANPictureEditToolPaint(hInst, hBackground);
}

BOOL PictureEditToolSetPaintId(HWND hEditTool, HANSIZE nId, BOOL bRedraw)
{
    BOOL bRet = (BOOL)SendMessage(hEditTool, PETM_SETPAINTID, (WPARAM)nId, (LPARAM)0);
    
    if ((TRUE == bRet) && (TRUE == bRedraw)) { RedrawWindow(hEditTool, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }

    return bRet;
}

HANSIZE PictureEditToolGetPaintId(HWND hEditTool)
{
    return SendMessage(hEditTool, PETM_GETPAINTID, (WPARAM)0, (LPARAM)0);
}

void PictureEditToolSetPixelInfo(HWND hEditTool, HANPSTR pPixelInfo)
{
    SendMessage(hEditTool, PETM_SETPIXELINFO, (WPARAM)0, (LPARAM)pPixelInfo);
}

static LRESULT CALLBACK PictureEditToolWndProc(HWND hPictureEditTool, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    PPICTUREEDITTOOLWNDEXTRA etInfo = (PPICTUREEDITTOOLWNDEXTRA)GetWindowLongPtr(hPictureEditTool, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = CreateCallback(hPictureEditTool, lParam);
        } break;
        case WM_SIZE: {
            SizeCallback(hPictureEditTool, etInfo);
        } break;
        case WM_CTLCOLORSTATIC: {
            lWndProcRet = (INT_PTR)GetStockObject(WHITE_BRUSH);
        } break;
        case WM_DESTROY: {
            DestroyCallback(etInfo);
            lWndProcRet = DefWindowProc(hPictureEditTool, message, wParam, lParam);
        } break;

        case PCTM_ZOOM: {
            (void)SendMessage(etInfo->hPaint, message, wParam, lParam);
        } break;

        case PETM_SETPAINTID: {
            lWndProcRet = s_PictureEditToolSetPaintId(etInfo, (HANSIZE)wParam);
        } break;
        case PETM_GETPAINTID: {
            lWndProcRet = s_PictureEditToolGetPaintId(etInfo);
        } break;
        case PETM_SETPIXELINFO: {
            s_PictureEditToolSetPixelInfo(etInfo, (HANPSTR)lParam);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hPictureEditTool, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}

static LRESULT CreateCallback(HWND hPictureEditTool, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PPICTUREEDITTOOLWNDEXTRA etInfo;
    HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
    HANPPICTUREINFO pPictureInfo = ((LPCREATESTRUCT)lParam)->lpCreateParams;
    RECT rcClientSize;
    RECT rcPaintSize;

    HANINT nWinX;
    HANINT nWinY;
    HANINT nWinW;
    HANINT nWinH;

    HANDLE hHeap = GetProcessHeap();
    if (NULL == hHeap) { lWndProcRet = -1; }
    if (-1 != lWndProcRet)
    {
        etInfo = (PPICTUREEDITTOOLWNDEXTRA)HANWinHeapAlloc(hHeap, NULL, sizeof(PICTUREEDITTOOLWNDEXTRA));
        if (NULL == etInfo) { lWndProcRet = -1; }
    }
    /* 创建窗口 */
    if (-1 != lWndProcRet)
    {
        SetWindowLongPtr(hPictureEditTool, 0, (LONG_PTR)etInfo);

        etInfo->hHeap = hHeap;
        etInfo->hInst = hInst;
        etInfo->hSelf = hPictureEditTool;
        etInfo->hPicture = ((LPCREATESTRUCT)lParam)->hwndParent;
        etInfo->pPictureInfo = pPictureInfo;

        GetClientRect(hPictureEditTool, &rcClientSize);
        GetPaintRect(&rcClientSize, &rcPaintSize);

        nWinX = 0;
        nWinY = 0;
        nWinW = GetRectW(&rcPaintSize);
        nWinH = GetRectH(&rcPaintSize);
        etInfo->hPaint = CreateWindow(HAN_PICTURE_EDIT_TOOL_PAINT_CLASS, NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            nWinX, nWinY, nWinW, nWinH,
            hPictureEditTool, (HMENU)WID_PICTURE_EDIT_TOOL_PAINT, hInst, pPictureInfo
        );
        nWinX += nWinW + PICTURE_WINDOW_DX;
        nWinY = 0;
        nWinW = PICTURE_EDIT_TOOL_CONSOLE_WIDTH;
        nWinH = GetRectH(&rcPaintSize);
        etInfo->hConsole = CreateWindow(TEXT("static"), NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            nWinX, nWinY, nWinW, nWinH,
            hPictureEditTool, (HMENU)WID_PICTURE_EDIT_TOOL_CONSOLE, hInst, NULL);
    }

    return lWndProcRet;
}
static void SizeCallback(HWND hPictureEditTool, PPICTUREEDITTOOLWNDEXTRA etInfo)
{
    RECT rcClientSize;
    RECT rcPaintSize;

    GetClientRect(hPictureEditTool, &rcClientSize);
    GetPaintRect(&rcClientSize, &rcPaintSize);

    MoveWindow(etInfo->hPaint, rcPaintSize.left, rcPaintSize.top, GetRectW(&rcPaintSize), GetRectH(&rcPaintSize), TRUE);
    MoveWindow(etInfo->hConsole, rcPaintSize.right + PICTURE_WINDOW_DX, rcPaintSize.top, PICTURE_EDIT_TOOL_CONSOLE_WIDTH, GetRectH(&rcPaintSize), TRUE);

    RedrawWindow(etInfo->hPaint, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
    RedrawWindow(etInfo->hConsole, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}
static void DestroyCallback(PPICTUREEDITTOOLWNDEXTRA etInfo)
{
    HANWinHeapFree(etInfo->hHeap, 0, etInfo);
}
static void GetPaintRect(const RECT* pPaint, RECT* pPaper)
{
    LONG nPaperW = GetRectW(pPaint) - PICTURE_EDIT_TOOL_CONSOLE_WIDTH - PICTURE_WINDOW_DX;
    LONG nPaperH = GetRectH(pPaint);

    if (nPaperW < HAN_PICTURE_EDIT_TOOL_PAINT_MIN_W) { nPaperW = HAN_PICTURE_EDIT_TOOL_PAINT_MIN_W; }
    if (nPaperH < HAN_PICTURE_EDIT_TOOL_PAINT_MIN_H) { nPaperH = HAN_PICTURE_EDIT_TOOL_PAINT_MIN_H; }

    pPaper->left = pPaint->left;
    pPaper->top = pPaint->top;
    pPaper->right = pPaper->left + nPaperW;
    pPaper->bottom = pPaper->top + nPaperH;
}

static BOOL s_PictureEditToolSetPaintId(PPICTUREEDITTOOLWNDEXTRA etInfo, HANSIZE nId)
{
    BOOL bRet = FALSE;
    
    if (nId < etInfo->pPictureInfo->nCnt)
    {
        PictureEditToolPaintSetPaintId(etInfo->hPaint, nId);
        bRet = TRUE;
    }

    return bRet;
}
static HANSIZE s_PictureEditToolGetPaintId(PPICTUREEDITTOOLWNDEXTRA etInfo)
{
    return PictureEditToolPaintGetPaintId(etInfo->hPaint);
}
static void s_PictureEditToolSetPixelInfo(PPICTUREEDITTOOLWNDEXTRA etInfo, HANPSTR pPixelInfo)
{
    SendMessage(etInfo->hPicture, PCTM_SETPIXELINFO, (WPARAM)0, (LPARAM)pPixelInfo);
}
