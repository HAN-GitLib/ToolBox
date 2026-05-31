#include <string.h>
#include <Windows.h>
#include <CommCtrl.h>

#include "HAN_PictureBitmap.h"
#include "HAN_PictureBitmapDef.h"
#include "HAN_PictureBitMapLib.h"
#include "HAN_PictureBitMapPaint.h"
#include "..\EditTool\HAN_PictureEditTool.h"

typedef enum {
    PICTURE_BITMAP_KEY_RESOLUTION,
    PICTURE_BITMAP_KEY_PIXEL_BITS,
    PICTURE_BITMAP_KEY_PLANES,
    PICTURE_BITMAP_KEY_CNT,
} PICTUREBITMAPKEY;

typedef enum {
    PICTURE_BITMAP_ROW_ORDER_POS,
    PICTURE_BITMAP_ROW_ORDER_NEG,
} PICTUREBITMAPROWORDER;

typedef enum {
    PICTURE_BITMAP_PICTURE_INFO_MAIN,
    PICTURE_BITMAP_PICTURE_INFO_SIZE,
    PICTURE_BITMAP_PICTURE_INFO_PIXEL,
    PICTURE_BITMAP_PICTURE_INFO_CNT,
} PICTUREBITMAPPICTUREINFO;

typedef struct tagPICTUREBITMAPWNDEXTRA {
    HANDLE                          hHeap;
    HINSTANCE                       hInst;
    HWND                            hSelf;
    HWND                            hInfo;
    HWND                            hShow;
    HWND                            hEditTool;
    HWND                            hPictureInfo;
    PICTURECREATEPARAM              paramPicture;
    struct {
        uint32_t                    nFileSize;
        uint32_t                    nPixelOffset;
        PICTUREBITMAPINFO           bmInfo;
        PICTUREBITMAPROWORDER       rowOrder;
        HANPPICTUREINFO             pPictureInfo;       /* 指向最终的图像信息 */
    } pictureData;
    struct {
        HFONT                       hHex;
        HFONT                       hSys;
    } hFont;                        /* 字体 */
    uint8_t                         pBuf[];
} PICTUREBITMAPWNDEXTRA, * PPICTUREBITMAPWNDEXTRA;

static LRESULT CALLBACK PictureBitmapWndProc(HWND hPictureBitmap, UINT message, WPARAM wParam, LPARAM lParam);

static LRESULT CreateCallback(HWND hPictureBitmap, LPARAM lParam);
static void SizeCallback(HWND hPictureBitmap, PPICTUREBITMAPWNDEXTRA bmInfo);
static void DestroyCallback(PPICTUREBITMAPWNDEXTRA bmInfo);
static LRESULT GetSaveParamCallback(PPICTUREBITMAPWNDEXTRA bmInfo, PPICTURESAVEPARAM pSaveParam);
static void InitBitmapInfo(HWND hBitmapInfo);
static void SetBitmapPictureInfoParts(PPICTUREBITMAPWNDEXTRA bmInfo);
static void BitmapProcess(PPICTUREBITMAPWNDEXTRA bmInfo);
static PPICTUREBITMAPWNDEXTRA ReallocBitmapInfoMemory(PPICTUREBITMAPWNDEXTRA bmInfo);
static void GetBitmapShowSize(PPICTUREBITMAPWNDEXTRA bmInfo, HANINT* pW, HANINT* pH);
static void UpdateBitmapPictureInfo(PPICTUREBITMAPWNDEXTRA bmInfo);

static BOOL DecodeBitmap(PPICTUREBITMAPWNDEXTRA bmInfo);
static void ReverseBitmapRows(PPICTUREBITMAPWNDEXTRA bmInfo);

static const uint8_t sg_pBitmapHeader[2] = { 'B', 'M', };
static const HANPSTR sg_pBitmapInfoKeyName[PICTURE_BITMAP_KEY_CNT] = {
    [PICTURE_BITMAP_KEY_RESOLUTION] = TEXT("分辨率"),
    [PICTURE_BITMAP_KEY_PIXEL_BITS] = TEXT("像素位数"),
    [PICTURE_BITMAP_KEY_PLANES] = TEXT("平面数"),
};
static const HANINT sg_pBitmapPictureInfoWidth[PICTURE_BITMAP_PICTURE_INFO_CNT] = {
    [PICTURE_BITMAP_PICTURE_INFO_MAIN] = 300,
    [PICTURE_BITMAP_PICTURE_INFO_SIZE] = 200,
    [PICTURE_BITMAP_PICTURE_INFO_PIXEL] = -1,
};

BOOL CheckBitmapType(const uint8_t* pData, HANSIZE nLen)
{
    BOOL bRet;

    if (nLen < sizeof(sg_pBitmapHeader)) { bRet = FALSE; }
    else if (0 != memcmp(pData, sg_pBitmapHeader, sizeof(sg_pBitmapHeader))) { bRet = FALSE; }
    else { bRet = TRUE; }

    return bRet;
}

void RegisterHANPictureBitmap(HINSTANCE hInst)
{
    WNDCLASSEX wcex = {
        .cbSize         = sizeof(WNDCLASSEX),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = PictureBitmapWndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = sizeof(PPICTUREBITMAPWNDEXTRA),
        .hInstance      = hInst,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = (HBRUSH)GetStockObject(PICTURE_BACKGROUND_BRUSH),
        .lpszMenuName   = NULL,
        .lpszClassName  = HAN_PICTURE_BITMAP_CLASS,
        .hIconSm        = NULL,
    };
    RegisterClassEx(&wcex);

    RegisterHANPictureBitmapPaint(hInst);
}

void SavePictureBitmap(HANPCSTR pFileName, PPICTURESAVEPARAM pSaveParam)
{
    PPICTUREBITMAPWNDEXTRA bmInfo = (PPICTUREBITMAPWNDEXTRA)(pSaveParam->pParam);
    HANPPICTURE pPicture = pSaveParam->pPicture;
    PPICTURERESOLUTION pResolution = &(pPicture->pxResolution);
    PPICTURERGBA* pMap = pPicture->pPictureMap;
    uint32_t pData[3] = {
        [0] = 54 + (pResolution->pxWidth * pResolution->pxHeight * 4),
        [1] = 0,
        [2] = 54,
    };
    BITMAPINFOHEADER bmiHeader = {
        .biSize = 40,
        .biWidth = pResolution->pxWidth,
        .biHeight = -((int32_t)(pResolution->pxHeight)),
        .biPlanes = 1,
        .biBitCount = 32,
        .biCompression = BI_RGB,
        .biSizeImage = 0,
        .biXPelsPerMeter = 0,
        .biYPelsPerMeter = 0,
        .biClrUsed = 0,
        .biClrImportant = 0,
    };
    DWORD nRowSize = pResolution->pxWidth * 4;
    HANDLE hFile;

    hFile = CreateFile(pFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);

    if (INVALID_HANDLE_VALUE != hFile)
    {
        WriteFile(hFile, sg_pBitmapHeader, sizeof(sg_pBitmapHeader), NULL, NULL);
        WriteFile(hFile, pData, 12, NULL, NULL);
        WriteFile(hFile, &bmiHeader, 40, NULL, NULL);
        for (uint32_t iLoop = 0; iLoop < pResolution->pxHeight; iLoop++)
        {
            WriteFile(hFile, pMap[iLoop], nRowSize, NULL, NULL);
        }

        CloseHandle(hFile);
    }
    else { MessageBox(bmInfo->hSelf, TEXT("打开文件失败"), TEXT("无法保存"), 0); }
}

static LRESULT CALLBACK PictureBitmapWndProc(HWND hPictureBitmap, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    PPICTUREBITMAPWNDEXTRA bmInfo = (PPICTUREBITMAPWNDEXTRA)GetWindowLongPtr(hPictureBitmap, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = CreateCallback(hPictureBitmap, lParam);
        } break;
        case WM_SIZE: {
            SizeCallback(hPictureBitmap, bmInfo);
        } break;
        case WM_CTLCOLORSTATIC: {
            lWndProcRet = (INT_PTR)GetStockObject(WHITE_BRUSH);
        } break;
        case WM_DESTROY: {
            DestroyCallback(bmInfo);
            lWndProcRet = DefWindowProc(hPictureBitmap, message, wParam, lParam);
        } break;

        case PCTM_GETSAVEPARAM: {
            lWndProcRet = GetSaveParamCallback(bmInfo, (PPICTURESAVEPARAM)lParam);
        } break;
        case PCTM_ZOOM: {
            (void)SendMessage(bmInfo->hEditTool, message, wParam, lParam);
        } break;
        case PCTM_SETPIXELINFO: {
            SendMessage(bmInfo->hPictureInfo, SB_SETTEXT, (WPARAM)MAKEWORD(PICTURE_BITMAP_PICTURE_INFO_PIXEL, SBT_NOBORDERS), lParam);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hPictureBitmap, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}

static LRESULT CreateCallback(HWND hPictureBitmap, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PPICTUREBITMAPWNDEXTRA bmInfo;
    PPICTUREBITMAPWNDEXTRA bmTempInfo;
    HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
    PPICTURECREATEPARAM pPictureCreateParam = ((LPCREATESTRUCT)lParam)->lpCreateParams;
    RECT rcClientSize;

    HANINT nWinX = PICTURE_WINDOW_DX;
    HANINT nWinY = PICTURE_WINDOW_DY;
    HANINT nWinW;
    HANINT nWinH;

    HANDLE hHeap = GetProcessHeap();
    if (NULL == hHeap) { lWndProcRet = -1; }
    if (-1 != lWndProcRet)
    {
        bmInfo = (PPICTUREBITMAPWNDEXTRA)HANWinHeapAlloc(hHeap, NULL, sizeof(PICTUREBITMAPWNDEXTRA));
        if (NULL == bmInfo) { lWndProcRet = -1; }
    }
    /* 创建窗口 */
    if (-1 != lWndProcRet)
    {
        SetWindowLongPtr(hPictureBitmap, 0, (LONG_PTR)bmInfo);

        bmInfo->hHeap = hHeap;
        bmInfo->hInst = hInst;
        bmInfo->hSelf = hPictureBitmap;

        GetClientRect(hPictureBitmap, &rcClientSize);

        bmInfo->hFont.hHex = CreateFontIndirect(&g_lfHexFont);
        bmInfo->hFont.hSys = CreateFontIndirect(&g_lfSysFont);

        nWinW = PICTURE_BITMAP_INFO_KEY_WIDTH + PICTURE_BITMAP_INFO_VALUE_WIDTH;
        nWinH = PICTURE_BITMAP_INFO_HEIGHT;
        bmInfo->hInfo = CreateWindow(WC_LISTVIEW, NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL,
            nWinX, nWinY, nWinW, nWinH,
            hPictureBitmap, (HMENU)WID_PICTURE_BITMAP_INFO, hInst, NULL);
            
        bmInfo->hPictureInfo = CreateWindow(STATUSCLASSNAME, NULL,
            WS_CHILD | WS_VISIBLE,
            0, 0, 0, 0,
            hPictureBitmap, (HMENU)WID_PICTURE_PICTURE_STATUS_BAR, hInst, NULL);

        SendMessage(bmInfo->hInfo, WM_SETFONT, (WPARAM)(bmInfo->hFont.hSys), (LPARAM)TRUE);

        ListView_SetExtendedListViewStyle(bmInfo->hInfo, LVS_EX_FULLROWSELECT);
        
        InitBitmapInfo(bmInfo->hInfo);
        SetBitmapPictureInfoParts(bmInfo);
    }
    /* 解码文件，填充信息 */
    if (-1 != lWndProcRet)
    {
        bmInfo->paramPicture = *pPictureCreateParam;
        BitmapProcess(bmInfo);
    }
    /* 重新分配 bmInfo 内存 */
    if (-1 != lWndProcRet)
    {
        bmTempInfo = ReallocBitmapInfoMemory(bmInfo);
        if (NULL != bmTempInfo)
        {
            bmInfo = bmTempInfo;
            SetWindowLongPtr(hPictureBitmap, 0, (LONG_PTR)bmInfo);
        }
        else { lWndProcRet = -1; }
    }
    // /* 图片解码 */
    if (-1 != lWndProcRet)
    {
        BOOL bDecodeRet = TRUE;

        bDecodeRet = DecodeBitmap(bmInfo);
        UpdateBitmapPictureInfo(bmInfo);
        
        /* 绘制预览图 */
        nWinX += nWinW + PICTURE_WINDOW_DX;
        GetBitmapShowSize(bmInfo, &nWinW, &nWinH);
        bmInfo->hShow = CreateWindow(HAN_PICTURE_BITMAP_PAINT_CLASS, NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER, nWinX, nWinY, nWinW, nWinH,
            hPictureBitmap, (HMENU)WID_PICTURE_JPEG_SHOW, hInst, bmInfo->pictureData.pPictureInfo->pPicture[0]
        );
        nWinX = PICTURE_WINDOW_DX;
        nWinY += PICTURE_BITMAP_INFO_HEIGHT + PICTURE_WINDOW_DY;
        nWinW = GetRectW(&rcClientSize) - (PICTURE_WINDOW_DY * 2);
        nWinH = GetRectH(&rcClientSize) - nWinY - PICTURE_WINDOW_DY;
        bmInfo->hEditTool = CreateWindow(HAN_PICTURE_EDIT_TOOL_CLASS, NULL,
            WS_CHILD | WS_VISIBLE, nWinX, nWinY, nWinW, nWinH,
            hPictureBitmap, (HMENU)WID_PICTURE_EDIT_TOOL, hInst, bmInfo->pictureData.pPictureInfo
        );
        if ((FALSE == bDecodeRet) || (NULL == bmInfo->hShow) || (NULL == bmInfo->hEditTool)) { lWndProcRet = -1; }
    }

    return lWndProcRet;
}
static void SizeCallback(HWND hPictureBitmap, PPICTUREBITMAPWNDEXTRA bmInfo)
{
    HANINT nWinX = PICTURE_WINDOW_DX;
    HANINT nWinY = PICTURE_BITMAP_INFO_HEIGHT + (PICTURE_WINDOW_DY * 2);
    HANINT nWinW;
    HANINT nWinH;
    RECT rcClientSize;
    RECT rcPictureInfo;
    
    SendMessage(bmInfo->hPictureInfo, WM_SIZE, 0, 0);
    GetClientRect(bmInfo->hPictureInfo, &rcPictureInfo);
    GetClientRect(hPictureBitmap, &rcClientSize);
    nWinW = GetRectW(&rcClientSize) - (PICTURE_WINDOW_DY * 2);
    nWinH = GetRectH(&rcClientSize) - GetRectH(&rcPictureInfo) - nWinY - PICTURE_WINDOW_DY;

    MoveWindow(bmInfo->hEditTool, nWinX, nWinY, nWinW, nWinH, TRUE);
}
static void DestroyCallback(PPICTUREBITMAPWNDEXTRA bmInfo)
{
    HANWinHeapFree(bmInfo->hHeap, 0, bmInfo);
}
static LRESULT GetSaveParamCallback(PPICTUREBITMAPWNDEXTRA bmInfo, PPICTURESAVEPARAM pSaveParam)
{
    HAN_strcpy(pSaveParam->pOpenClassName, HAN_PICTURE_BITMAP_CLASS);
    pSaveParam->pPicture = bmInfo->pictureData.pPictureInfo->pPicture[0];
    pSaveParam->pParam = bmInfo;

    return TRUE;
}
static void InitBitmapInfo(HWND hBitmapInfo)
{
    HANCHAR pText[HAN_PICTURE_BITMAP_TEXT_BUF_SIZE];
    LVCOLUMN lvTitle = {
        .mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM,
        .fmt = LVCFMT_LEFT,
    };
    LVITEM lvItem = {
        .mask = LVIF_TEXT,
        .iSubItem = 0,
        .pszText = pText,
    };

    lvTitle.iSubItem = 0;
    lvTitle.cx = PICTURE_BITMAP_INFO_KEY_WIDTH;
    lvTitle.pszText = TEXT("属性");
    ListView_InsertColumn(hBitmapInfo, 0, &lvTitle);
    lvTitle.iSubItem = 1;
    lvTitle.cx = PICTURE_BITMAP_INFO_VALUE_WIDTH;
    lvTitle.pszText = TEXT("值");
    ListView_InsertColumn(hBitmapInfo, 1, &lvTitle);

    for (PICTUREBITMAPKEY iLoop = 0; iLoop < PICTURE_BITMAP_KEY_CNT; iLoop++)
    {
        lvItem.iItem = iLoop;
        lvItem.pszText = sg_pBitmapInfoKeyName[iLoop];
        ListView_InsertItem(hBitmapInfo, &lvItem);
    }
}
static void SetBitmapPictureInfoParts(PPICTUREBITMAPWNDEXTRA bmInfo)
{
    HANINT pInfoRightPos = 0;
    HANINT pInfoParts[PICTURE_BITMAP_PICTURE_INFO_CNT];

    for (PICTUREBITMAPPICTUREINFO iLoop = 0; iLoop < PICTURE_BITMAP_PICTURE_INFO_CNT; iLoop++)
    {
        if (-1 != sg_pBitmapPictureInfoWidth[iLoop])
        {
            pInfoRightPos += sg_pBitmapPictureInfoWidth[iLoop];
            pInfoParts[iLoop] = pInfoRightPos;
        }
        else { pInfoParts[iLoop] = -1; }
    }
    SendMessage(bmInfo->hPictureInfo, SB_SETPARTS, PICTURE_BITMAP_PICTURE_INFO_CNT, (LPARAM)pInfoParts);
}
static void BitmapProcess(PPICTUREBITMAPWNDEXTRA bmInfo)
{
    const uint8_t* pData = bmInfo->paramPicture.pData;
    HANCHAR pText[HAN_PICTURE_BITMAP_TEXT_BUF_SIZE];
    HANSIZE nOffset;
    LVITEM lvItem = {
        .mask = LVIF_TEXT,
        .iSubItem = 1,
        .pszText = pText,
    };
    
    nOffset = 2;
    memcpy(&(bmInfo->pictureData.nFileSize), &pData[nOffset], 4); nOffset += 8;
    memcpy(&(bmInfo->pictureData.nPixelOffset), &pData[nOffset], 4); nOffset += 4;
    memcpy(&(bmInfo->pictureData.bmInfo.bmiHeader), &pData[nOffset], sizeof(BITMAPINFOHEADER)); nOffset += sizeof(BITMAPINFOHEADER);

    bmInfo->pictureData.bmInfo.colorTable.pData = &pData[nOffset];
    bmInfo->pictureData.bmInfo.pixelData.pData = &pData[bmInfo->pictureData.nPixelOffset];
    if (0 < bmInfo->pictureData.bmInfo.bmiHeader.biHeight)
    {
        bmInfo->pictureData.rowOrder = PICTURE_BITMAP_ROW_ORDER_NEG;
    }
    else
    {
        bmInfo->pictureData.bmInfo.bmiHeader.biHeight *= -1;
        bmInfo->pictureData.rowOrder = PICTURE_BITMAP_ROW_ORDER_POS;
    }
    
    lvItem.iItem = PICTURE_BITMAP_KEY_RESOLUTION;
    HAN_snprintf(pText, ArrLen(pText), TEXT("%ld×%ld"), bmInfo->pictureData.bmInfo.bmiHeader.biWidth, bmInfo->pictureData.bmInfo.bmiHeader.biHeight);
    ListView_SetItem(bmInfo->hInfo, &lvItem);
    
    lvItem.iItem = PICTURE_BITMAP_KEY_PIXEL_BITS;
    HAN_snprintf(pText, ArrLen(pText), TEXT("%u"), bmInfo->pictureData.bmInfo.bmiHeader.biBitCount);
    ListView_SetItem(bmInfo->hInfo, &lvItem);
    
    lvItem.iItem = PICTURE_BITMAP_KEY_PLANES;
    HAN_snprintf(pText, ArrLen(pText), TEXT("%u"), bmInfo->pictureData.bmInfo.bmiHeader.biPlanes);
    ListView_SetItem(bmInfo->hInfo, &lvItem);
}
static PPICTUREBITMAPWNDEXTRA ReallocBitmapInfoMemory(PPICTUREBITMAPWNDEXTRA bmInfo)
{
    PPICTUREBITMAPWNDEXTRA pRet;
    PICTURERESOLUTION pxResolution = {
        .pxWidth = bmInfo->pictureData.bmInfo.bmiHeader.biWidth,
        .pxHeight = bmInfo->pictureData.bmInfo.bmiHeader.biHeight,
    };
    SIZE_T nOffset;
    SIZE_T nPictureInfoSize;
    SIZE_T nPictureSize;

    nPictureInfoSize = GetPictureInfoMemSize(1);
    nPictureSize = GetPictureMemSize(&pxResolution);
    pRet = (PPICTUREBITMAPWNDEXTRA)HANWinHeapAlloc(
        bmInfo->hHeap, bmInfo,
        sizeof(PICTUREBITMAPWNDEXTRA)
         + nPictureInfoSize
         + nPictureSize
    );

    if (NULL != pRet)
    {
        nOffset = 0;
        pRet->pictureData.pPictureInfo = (HANPPICTUREINFO)&(pRet->pBuf[nOffset]); nOffset += nPictureInfoSize;
        pRet->pictureData.pPictureInfo->nCnt = 1;
        pRet->pictureData.pPictureInfo->pPicture[0] = (HANPPICTURE)&(pRet->pBuf[nOffset]); nOffset += nPictureSize;
        UpdatePictureMap(pRet->pictureData.pPictureInfo->pPicture[0], &pxResolution);
    }

    return pRet;
}
static void GetBitmapShowSize(PPICTUREBITMAPWNDEXTRA bmInfo, HANINT* pW, HANINT* pH)
{
    HANDOUBLE pxWidth = (HANDOUBLE)(bmInfo->pictureData.pPictureInfo->pPicture[0]->pxResolution.pxWidth);
    HANDOUBLE pxHeight = (HANDOUBLE)(bmInfo->pictureData.pPictureInfo->pPicture[0]->pxResolution.pxHeight);
    
    *pH = PICTURE_BITMAP_INFO_HEIGHT;
    *pW = (HANINT)((HANDOUBLE)(*pH) / pxHeight * pxWidth);
}
static void UpdateBitmapPictureInfo(PPICTUREBITMAPWNDEXTRA bmInfo)
{
    PPICTURERESOLUTION pResolution = &(bmInfo->pictureData.pPictureInfo->pPicture[0]->pxResolution);
    HANCHAR pText[HAN_PICTURE_BITMAP_TEXT_BUF_SIZE];

    HAN_strncpy(pText, TEXT("位图"), HAN_PICTURE_BITMAP_TEXT_BUF_SIZE);
    SendMessage(bmInfo->hPictureInfo, SB_SETTEXT, (WPARAM)MAKEWORD(PICTURE_BITMAP_PICTURE_INFO_MAIN, SBT_NOBORDERS), (LPARAM)pText);

    HAN_snprintf(pText, HAN_PICTURE_BITMAP_TEXT_BUF_SIZE, TEXT("%u×%u"), pResolution->pxWidth, pResolution->pxHeight);
    SendMessage(bmInfo->hPictureInfo, SB_SETTEXT, (WPARAM)MAKEWORD(PICTURE_BITMAP_PICTURE_INFO_SIZE, SBT_NOBORDERS), (LPARAM)pText);
}

static BOOL DecodeBitmap(PPICTUREBITMAPWNDEXTRA bmInfo)
{
    BOOL bRet;
    BOOL bAlpha;

    bRet = DecodeBitmapColor(bmInfo->pictureData.pPictureInfo->pPicture[0], &(bmInfo->pictureData.bmInfo), &bAlpha);
    if ((TRUE == bRet) && (PICTURE_BITMAP_ROW_ORDER_NEG == bmInfo->pictureData.rowOrder))
    {
        ReverseBitmapRows(bmInfo);
    }

    return bRet;
}
static void ReverseBitmapRows(PPICTUREBITMAPWNDEXTRA bmInfo)
{
    HANPPICTURE pPicture = bmInfo->pictureData.pPictureInfo->pPicture[0];
    PPICTURERGBA* pMap = pPicture->pPictureMap;
    PPICTURERGBA pEx;
    uint32_t nHeight = pPicture->pxResolution.pxHeight;
    uint32_t nHalfHeight = nHeight / 2;

    for (uint32_t iLoop = 0; iLoop < nHalfHeight; iLoop++)
    {
        pEx = pMap[iLoop];
        pMap[iLoop] = pMap[nHeight - iLoop - 1];
        pMap[nHeight - iLoop - 1] = pEx;
    }
}
