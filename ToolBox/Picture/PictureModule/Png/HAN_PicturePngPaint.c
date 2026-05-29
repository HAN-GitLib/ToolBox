#include "HAN_PicturePngPaint.h"
#include "..\..\HAN_PictureLib.h"

typedef struct tagPICTUREPNGWNDEXTRA {
    HANDLE                          hHeap;
    HINSTANCE                       hInst;
    HWND                            hSelf;
    HANPPICTURE                     pPicture;
    HANSIZE                         nBufSize;
    PICTUREWINPIXEL                 pBuf[];
} PICTUREPNGPAINTEXTRA, * PPICTUREPNGPAINTEXTRA;

static LRESULT CALLBACK PicturePngPaintWndProc(HWND hPicturePngPaint, UINT message, WPARAM wParam, LPARAM lParam);

static LRESULT CreateCallback(HWND hPicturePngPaint, LPARAM lParam);
static void PaintCallback(HDC hDC, PPICTUREPNGPAINTEXTRA pngPaintInfo, const RECT* rcWin);
static void DestroyCallback(PPICTUREPNGPAINTEXTRA pngPaintInfo);
static void PaintPng(PPICTUREPNGPAINTEXTRA pngPaintInfo, const RECT* rcWin);
static void PaintBackground(PPICTUREPNGPAINTEXTRA pngPaintInfo, const RECT* rcWin);

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

void RegisterHANPicturePngPaint(HINSTANCE hInst)
{
    WNDCLASSEX wcex = {
        .cbSize         = sizeof(WNDCLASSEX),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = PicturePngPaintWndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = sizeof(PPICTUREPNGPAINTEXTRA),
        .hInstance      = hInst,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName   = NULL,
        .lpszClassName  = HAN_PICTURE_PNG_PAINT_CLASS,
        .hIconSm        = NULL,
    };
    RegisterClassEx(&wcex);
}

static LRESULT CALLBACK PicturePngPaintWndProc(HWND hPicturePngPaint, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    PPICTUREPNGPAINTEXTRA pngPaintInfo = (PPICTUREPNGPAINTEXTRA)GetWindowLongPtr(hPicturePngPaint, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = CreateCallback(hPicturePngPaint, lParam);
        } break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            /* 开始绘图 */
            HDC hDC = BeginPaint(hPicturePngPaint, &ps);
            /* 获取窗口大小 */
            RECT rcWin;
            GetClientRect(hPicturePngPaint, &rcWin);
            rcWin.right++;
            rcWin.bottom++;
            /* 绘制内容 */
            PaintCallback(hDC, pngPaintInfo, &rcWin);
            /* 结束绘图 */
            EndPaint(hPicturePngPaint, &ps);
        } break;
        case WM_DESTROY: {
            DestroyCallback(pngPaintInfo);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hPicturePngPaint, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}

static LRESULT CreateCallback(HWND hPicturePngPaint, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PPICTUREPNGPAINTEXTRA pngPaintInfo;
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
        GetClientRect(hPicturePngPaint, &rcClientSize);
        nWinW = GetRectW(&rcClientSize) + 1;
        nWinH = GetRectH(&rcClientSize) + 1;
        nBufSize = nWinW * nWinH * sizeof(uint8_t) * sizeof(PICTUREWINPIXEL);
        pngPaintInfo = (PPICTUREPNGPAINTEXTRA)HANWinHeapAlloc(hHeap, NULL, sizeof(PICTUREPNGPAINTEXTRA) + nBufSize);
        if (NULL == pngPaintInfo) { lWndProcRet = -1; }
    }

    if (-1 != lWndProcRet)
    {
        SetWindowLongPtr(hPicturePngPaint, 0, (LONG_PTR)pngPaintInfo);

        pngPaintInfo->hHeap = hHeap;
        pngPaintInfo->hInst = hInst;
        pngPaintInfo->hSelf = hPicturePngPaint;
        pngPaintInfo->pPicture = pPicture;
        pngPaintInfo->nBufSize = nBufSize;
    }

    return lWndProcRet;
}
static void PaintCallback(HDC hDC, PPICTUREPNGPAINTEXTRA pngPaintInfo, const RECT* rcWin)
{
    PICTUREWINPIXEL* pPixel = pngPaintInfo->pBuf;
    BITMAPINFO bmiPaint;
    LONG nWinW = GetRectW(rcWin);
    LONG nWinH = GetRectH(rcWin);

    PaintBackground(pngPaintInfo, rcWin);
    PaintPng(pngPaintInfo, rcWin);

    ZeroMemory(&bmiPaint, sizeof(bmiPaint));
    bmiPaint.bmiHeader.biSize = sizeof(BITMAPINFO);
    bmiPaint.bmiHeader.biWidth = nWinW;
    bmiPaint.bmiHeader.biHeight = -nWinH;
    bmiPaint.bmiHeader.biPlanes = 1;
    bmiPaint.bmiHeader.biBitCount = 32;
    bmiPaint.bmiHeader.biCompression = BI_RGB;
    bmiPaint.bmiHeader.biSizeImage = (DWORD)(pngPaintInfo->nBufSize);
    StretchDIBits(hDC, 0, 0, nWinW, nWinH, 0, 0, nWinW, nWinH, pPixel, (BITMAPINFO*)&bmiPaint.bmiHeader, DIB_RGB_COLORS, SRCCOPY);
}
static void DestroyCallback(PPICTUREPNGPAINTEXTRA pngPaintInfo)
{
    HANWinHeapFree(pngPaintInfo->hHeap, 0, pngPaintInfo);
}
static void PaintBackground(PPICTUREPNGPAINTEXTRA pngPaintInfo, const RECT* rcWin)
{
    PICTUREWINPIXEL* pPixel = pngPaintInfo->pBuf;
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
            if ((nColCnt % PICTURE_PNG_ALPHA_BACKGROUND_W) < PICTURE_PNG_ALPHA_BACKGROUND_W0)
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
        if (PICTURE_PNG_ALPHA_BACKGROUND_W <= nRowCnt) { nRowCnt = 0; }
    }
}
static void PaintPng(PPICTUREPNGPAINTEXTRA pngPaintInfo, const RECT* rcWin)
{
    PICTURERESOLUTION pxResolution = pngPaintInfo->pPicture->pxResolution;
    PPICTURERGBA* pRGBA = pngPaintInfo->pPicture->pPictureMap;
    PICTUREWINPIXEL* pPixel = pngPaintInfo->pBuf;
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
