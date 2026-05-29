#include "HAN_PictureBitmapPaint.h"
#include "..\..\HAN_PictureLib.h"

typedef struct tagPICTUREBITMAPPAINTEXTRA {
    HANDLE                          hHeap;
    HINSTANCE                       hInst;
    HWND                            hSelf;
    HANPPICTURE                     pPicture;
    HANSIZE                         nBufSize;
    PICTUREWINPIXEL                 pBuf[];
} PICTUREBITMAPPAINTEXTRA, * PPICTUREBITMAPPAINTEXTRA;

static LRESULT CALLBACK PictureBitmapPaintWndProc(HWND hPictureBitmapPaint, UINT message, WPARAM wParam, LPARAM lParam);

static LRESULT CreateCallback(HWND hPictureBitmapPaint, LPARAM lParam);
static void PaintCallback(HDC hDC, PPICTUREBITMAPPAINTEXTRA bmPaintInfo, const RECT* rcWin);
static void DestroyCallback(PPICTUREBITMAPPAINTEXTRA bmPaintInfo);
static void PaintPng(PPICTUREBITMAPPAINTEXTRA bmPaintInfo, const RECT* rcWin);
static void PaintBackground(PPICTUREBITMAPPAINTEXTRA bmPaintInfo, const RECT* rcWin);

static const PICTURERGBA sg_pPngAlphaBackgroundColor[2] = {
    [0] = {
        .r = 0xFF,
        .g = 0xFF,
        .b = 0xFF,
        .a = 0xFF,
    },
    [1] = {
        .r = 0x55,
        .g = 0x55,
        .b = 0x55,
        .a = 0xFF,
    },
};

void RegisterHANPictureBitmapPaint(HINSTANCE hInst)
{
    WNDCLASSEX wcex = {
        .cbSize         = sizeof(WNDCLASSEX),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = PictureBitmapPaintWndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = sizeof(PPICTUREBITMAPPAINTEXTRA),
        .hInstance      = hInst,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName   = NULL,
        .lpszClassName  = HAN_PICTURE_BITMAP_PAINT_CLASS,
        .hIconSm        = NULL,
    };
    RegisterClassEx(&wcex);
}

static LRESULT CALLBACK PictureBitmapPaintWndProc(HWND hPictureBitmapPaint, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    PPICTUREBITMAPPAINTEXTRA bmPaintInfo = (PPICTUREBITMAPPAINTEXTRA)GetWindowLongPtr(hPictureBitmapPaint, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = CreateCallback(hPictureBitmapPaint, lParam);
        } break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            /* 开始绘图 */
            HDC hDC = BeginPaint(hPictureBitmapPaint, &ps);
            /* 获取窗口大小 */
            RECT rcWin;
            GetClientRect(hPictureBitmapPaint, &rcWin);
            rcWin.right++;
            rcWin.bottom++;
            /* 绘制内容 */
            PaintCallback(hDC, bmPaintInfo, &rcWin);
            /* 结束绘图 */
            EndPaint(hPictureBitmapPaint, &ps);
        } break;
        case WM_DESTROY: {
            DestroyCallback(bmPaintInfo);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hPictureBitmapPaint, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}

static LRESULT CreateCallback(HWND hPictureBitmapPaint, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PPICTUREBITMAPPAINTEXTRA bmPaintInfo;
    HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
    HANPPICTURE pPicture = ((LPCREATESTRUCT)lParam)->lpCreateParams;
    RECT rcClientSize;
    INT nWinW;
    INT nWinH;
    HANSIZE nBufSize;

    HANDLE hHeap = GetProcessHeap();
    if (NULL == hHeap) { lWndProcRet = -1; }
    if (-1 != lWndProcRet)
    {
        GetClientRect(hPictureBitmapPaint, &rcClientSize);
        nWinW = GetRectW(&rcClientSize) + 1;
        nWinH = GetRectH(&rcClientSize) + 1;
        nBufSize = nWinW * nWinH * sizeof(uint8_t) * sizeof(PICTUREWINPIXEL);
        bmPaintInfo = (PPICTUREBITMAPPAINTEXTRA)HANWinHeapAlloc(hHeap, NULL, sizeof(PICTUREBITMAPPAINTEXTRA) + nBufSize);
        if (NULL == bmPaintInfo) { lWndProcRet = -1; }
    }

    if (-1 != lWndProcRet)
    {
        SetWindowLongPtr(hPictureBitmapPaint, 0, (LONG_PTR)bmPaintInfo);

        bmPaintInfo->hHeap = hHeap;
        bmPaintInfo->hInst = hInst;
        bmPaintInfo->hSelf = hPictureBitmapPaint;
        bmPaintInfo->pPicture = pPicture;
        bmPaintInfo->nBufSize = nBufSize;
    }

    return lWndProcRet;
}
static void PaintCallback(HDC hDC, PPICTUREBITMAPPAINTEXTRA bmPaintInfo, const RECT* rcWin)
{
    PICTUREWINPIXEL* pPixel = bmPaintInfo->pBuf;
    BITMAPINFO bmiPaint;
    LONG nWinW = GetRectW(rcWin);
    LONG nWinH = GetRectH(rcWin);

    PaintBackground(bmPaintInfo, rcWin);
    PaintPng(bmPaintInfo, rcWin);

    ZeroMemory(&bmiPaint, sizeof(bmiPaint));
    bmiPaint.bmiHeader.biSize = sizeof(BITMAPINFO);
    bmiPaint.bmiHeader.biWidth = nWinW;
    bmiPaint.bmiHeader.biHeight = -nWinH;
    bmiPaint.bmiHeader.biPlanes = 1;
    bmiPaint.bmiHeader.biBitCount = 32;
    bmiPaint.bmiHeader.biCompression = BI_RGB;
    bmiPaint.bmiHeader.biSizeImage = (DWORD)(bmPaintInfo->nBufSize);
    StretchDIBits(hDC, 0, 0, nWinW, nWinH, 0, 0, nWinW, nWinH, pPixel, (BITMAPINFO*)&bmiPaint.bmiHeader, DIB_RGB_COLORS, SRCCOPY);
}
static void DestroyCallback(PPICTUREBITMAPPAINTEXTRA bmPaintInfo)
{
    HANWinHeapFree(bmPaintInfo->hHeap, 0, bmPaintInfo);
}
static void PaintBackground(PPICTUREBITMAPPAINTEXTRA bmPaintInfo, const RECT* rcWin)
{
    PICTUREWINPIXEL* pPixel = bmPaintInfo->pBuf;
    LONG nWinW = GetRectW(rcWin);
    LONG nWinH = GetRectH(rcWin);
    HANSIZE nOffset = 0;
    HANSIZE nRowCnt = 0;
    HANSIZE nColCnt;

    for (LONG nRow = 0; nRow < nWinH; nRow++)
    {
        nColCnt = nRowCnt;
        for (LONG nCol = 0; nCol < nWinW; nCol++)
        {
            if ((nColCnt % PICTURE_BITMAP_ALPHA_BACKGROUND_W) < PICTURE_BITMAP_ALPHA_BACKGROUND_W0)
            {
                pPixel[nOffset].ptRGBA = sg_pPngAlphaBackgroundColor[0];
            }
            else
            {
                pPixel[nOffset].ptRGBA = sg_pPngAlphaBackgroundColor[1];
            }
            nColCnt++;
            nOffset++;
        }
        nRowCnt++;
        if (PICTURE_BITMAP_ALPHA_BACKGROUND_W <= nRowCnt) { nRowCnt = 0; }
    }
}
static void PaintPng(PPICTUREBITMAPPAINTEXTRA bmPaintInfo, const RECT* rcWin)
{
    PICTURERESOLUTION pxResolution = bmPaintInfo->pPicture->pxResolution;
    PPICTURERGBA* pRGBA = bmPaintInfo->pPicture->pPictureMap;
    PICTUREWINPIXEL* pPixel = bmPaintInfo->pBuf;
    LONG nWinW = GetRectW(rcWin);
    LONG nWinH = GetRectH(rcWin);
    LONG pxRow;
    LONG pxCol;
    HANSIZE nOffset = 0;

    for (LONG nRow = 0; nRow < nWinH; nRow++)
    {
        pxRow = (LONG)((HANDOUBLE)nRow / (HANDOUBLE)nWinH * (HANDOUBLE)pxResolution.pxHeight);
        for (LONG nCol = 0; nCol < nWinW; nCol++)
        {
            pxCol = (LONG)((HANDOUBLE)nCol / (HANDOUBLE)nWinW * (HANDOUBLE)pxResolution.pxWidth);
            pPixel[nOffset].ptRGBA = BlendRGBA(pRGBA[pxRow][pxCol], pPixel[nOffset].ptRGBA);
            nOffset++;
        }
    }
}
