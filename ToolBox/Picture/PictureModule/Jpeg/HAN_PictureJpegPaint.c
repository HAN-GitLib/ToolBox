#include "HAN_PictureJpegPaint.h"

typedef struct tagPICTUREJPEGPAINTWNDEXTRA {
    HANDLE                          hHeap;
    HINSTANCE                       hInst;
    HWND                            hSelf;
    HANPPICTURE                     pPicture;
    HANSIZE                         nBufSize;
    PICTUREWINPIXEL                 pBuf[];
} PICTUREJPEGPAINTWNDEXTRA, * PPICTUREJPEGPAINTWNDEXTRA;

static LRESULT CALLBACK PictureJpegPaintWndProc(HWND hPictureJpegPaint, UINT message, WPARAM wParam, LPARAM lParam);

static LRESULT CreateCallback(HWND hPictureJpegPaint, LPARAM lParam);
static void PaintCallback(HDC hDC, PPICTUREJPEGPAINTWNDEXTRA JpegPaintInfo, const RECT* rcWin);
static void DestroyCallback(PPICTUREJPEGPAINTWNDEXTRA JpegPaintInfo);
static void PaintJpeg(PPICTUREJPEGPAINTWNDEXTRA JpegPaintInfo, const RECT* rcWin);

void RegisterHANPictureJpegPaint(HINSTANCE hInst)
{
    WNDCLASSEX wcex = {
        .cbSize         = sizeof(WNDCLASSEX),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = PictureJpegPaintWndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = sizeof(PPICTUREJPEGPAINTWNDEXTRA),
        .hInstance      = hInst,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName   = NULL,
        .lpszClassName  = HAN_PICTURE_JPEG_PAINT_CLASS,
        .hIconSm        = NULL,
    };
    RegisterClassEx(&wcex);
}

static LRESULT CALLBACK PictureJpegPaintWndProc(HWND hPictureJpegPaint, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    PPICTUREJPEGPAINTWNDEXTRA jpegPaintInfo = (PPICTUREJPEGPAINTWNDEXTRA)GetWindowLongPtr(hPictureJpegPaint, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = CreateCallback(hPictureJpegPaint, lParam);
        } break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            /* 开始绘图 */
            HDC hDC = BeginPaint(hPictureJpegPaint, &ps);
            /* 获取窗口大小 */
            RECT rcWin;
            GetClientRect(hPictureJpegPaint, &rcWin);
            rcWin.right++;
            rcWin.bottom++;
            /* 绘制内容 */
            PaintCallback(hDC, jpegPaintInfo, &rcWin);
            /* 结束绘图 */
            EndPaint(hPictureJpegPaint, &ps);
        } break;
        case WM_DESTROY: {
            DestroyCallback(jpegPaintInfo);
            lWndProcRet = DefWindowProc(hPictureJpegPaint, message, wParam, lParam);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hPictureJpegPaint, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}

static LRESULT CreateCallback(HWND hPictureJpegPaint, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PPICTUREJPEGPAINTWNDEXTRA jpegPaintInfo;
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
        GetClientRect(hPictureJpegPaint, &rcClientSize);
        nWinW = GetRectW(&rcClientSize) + 1;
        nWinH = GetRectH(&rcClientSize) + 1;
        nBufSize = nWinW * nWinH * sizeof(uint8_t) * sizeof(PICTUREWINPIXEL);
        jpegPaintInfo = (PPICTUREJPEGPAINTWNDEXTRA)HANWinHeapAlloc(hHeap, NULL, sizeof(PICTUREJPEGPAINTWNDEXTRA) + nBufSize);
        if (NULL == jpegPaintInfo) { lWndProcRet = -1; }
    }

    if (-1 != lWndProcRet)
    {
        SetWindowLongPtr(hPictureJpegPaint, 0, (LONG_PTR)jpegPaintInfo);

        jpegPaintInfo->hHeap = hHeap;
        jpegPaintInfo->hInst = hInst;
        jpegPaintInfo->hSelf = hPictureJpegPaint;
        jpegPaintInfo->pPicture = pPicture;
        jpegPaintInfo->nBufSize = nBufSize;
    }

    return lWndProcRet;
}
static void PaintCallback(HDC hDC, PPICTUREJPEGPAINTWNDEXTRA jpegPaintInfo, const RECT* rcWin)
{
    PICTUREWINPIXEL* pPixel = jpegPaintInfo->pBuf;
    BITMAPINFO bmiPaint;
    LONG nWinW = GetRectW(rcWin);
    LONG nWinH = GetRectH(rcWin);

    PaintJpeg(jpegPaintInfo, rcWin);

    ZeroMemory(&bmiPaint, sizeof(bmiPaint));
    bmiPaint.bmiHeader.biSize = sizeof(BITMAPINFO);
    bmiPaint.bmiHeader.biWidth = nWinW;
    bmiPaint.bmiHeader.biHeight = -nWinH;
    bmiPaint.bmiHeader.biPlanes = 1;
    bmiPaint.bmiHeader.biBitCount = 32;
    bmiPaint.bmiHeader.biCompression = BI_RGB;
    bmiPaint.bmiHeader.biSizeImage = (DWORD)(jpegPaintInfo->nBufSize);
    StretchDIBits(hDC, 0, 0, nWinW, nWinH, 0, 0, nWinW, nWinH, pPixel, (BITMAPINFO*)&bmiPaint.bmiHeader, DIB_RGB_COLORS, SRCCOPY);
}
static void DestroyCallback(PPICTUREJPEGPAINTWNDEXTRA JpegPaintInfo)
{
    HANWinHeapFree(JpegPaintInfo->hHeap, 0, JpegPaintInfo);
}
static void PaintJpeg(PPICTUREJPEGPAINTWNDEXTRA jpegPaintInfo, const RECT* rcWin)
{
    PICTURERESOLUTION pxResolution = jpegPaintInfo->pPicture->pxResolution;
    PPICTURERGBA* pRGBA = jpegPaintInfo->pPicture->pPictureMap;
    PICTUREWINPIXEL* pPixel = jpegPaintInfo->pBuf;
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
            pPixel[nOffset].ptRGBA = pRGBA[pxRow][pxCol];
            nOffset++;
        }
    }
}
