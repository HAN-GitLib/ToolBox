#include <Windows.h>
#include <CommCtrl.h>
#include <shlobj.h>
#include <shlwapi.h>

#include "HAN_PictureIcon.h"
#include "..\..\HAN_PictureLib.h"
#include "..\Bitmap\HAN_PictureBitmapLib.h"
#include "..\EditTool\HAN_PictureEditTool.h"

typedef enum {
    PICTURE_ICON_KEY_TOTAL,
    PICTURE_ICON_KEY_ID,
    PICTURE_ICON_KEY_RESOLUTION,
    PICTURE_ICON_KEY_PIXEL_BITS,
    PICTURE_ICON_KEY_ROW_BYTES,
    PICTURE_ICON_KEY_PLANES,
    PICTURE_ICON_KEY_ALPHA_MODE,
    PICTURE_ICON_KEY_CNT,
} PICTUREICONKEY;

typedef enum {
    PICTURE_ICON_ALPHA_MODE_NONE,
    PICTURE_ICON_ALPHA_MODE_MASK,
    PICTURE_ICON_ALPHA_MODE_ALPHA_CHANNEL,
    PICTURE_ICON_ALPHA_MODE_CNT,
} PICTUREICONALPHAMODE;

typedef struct tagPICTUREICONBITMAPINFO {
    BITMAP                      bmBitmap;
    PICTUREBITMAPINFO           bmInfo;
    PICTUREICONALPHAMODE        eAlpha;
} PICTUREICONBITMAPINFO, * PPICTUREICONBITMAPINFO;

typedef struct tagPICTUREICONEXTRA {
    HANDLE                      hHeap;
    HINSTANCE                   hInst;
    HWND                        hSelf;
    HWND                        hInfo;  
    HWND                        hEditTool;
    struct {
        HWND                    hIconList;
        HIMAGELIST              imageList;
        HANINT                  nIconCnt;
        HANINT                  nCursel;
    } iconList;
    struct {
        PPICTUREICONBITMAPINFO  pIconBitmapInfo;
        uint8_t*                pBitmapData;
        HANPPICTUREINFO         pPictureInfo;
    } pictureData;
    struct {
        HFONT                   hHex;
        HFONT                   hSys;
    } hFont;                    /* 字体 */
    uint8_t                     pBuf[];
} PICTUREICONEXTRA, * PPICTUREICONEXTRA;

static LRESULT CALLBACK PictureIconWndProc(HWND hPictureIcon, UINT message, WPARAM wParam, LPARAM lParam);

static LRESULT CreateCallback(HWND hPictureIcon, LPARAM lParam);
static void SizeCallback(HWND hPictureIcon, PPICTUREICONEXTRA iconInfo);
static LRESULT NotifyCallback(PPICTUREICONEXTRA iconInfo, NMHDR* pNotify);
static void DestroyCallback(PPICTUREICONEXTRA iconInfo);
static LRESULT GetSaveParamCallback(PPICTUREICONEXTRA iconInfo, PPICTURESAVEPARAM pSaveParam);
static void InitIconList(HWND hIconList);
static void InitIconInfo(HWND hIconInfo);
static void LoadIconToImageList(PPICTUREICONEXTRA iconInfo, HANPCSTR pFileName);
static PPICTUREICONEXTRA ReallocIconInfoMemory(PPICTUREICONEXTRA iconInfo);
static SIZE_T GetIconsPictureMemSize(PPICTUREICONEXTRA iconInfo, PPICTURERESOLUTION pResolution);
static SIZE_T GetIconsBitmapMemSize(HANINT nCnt, PPICTURERESOLUTION pResolution);
static void LoadPictureFromIcon(PPICTUREICONEXTRA iconInfo);
static void UpdateIconInfo(PPICTUREICONEXTRA iconInfo);
static LRESULT IconListNotifyCallback(PPICTUREICONEXTRA iconInfo, NMHDR* pNotify);

static const HANPCSTR sg_pAlphaModeName[PICTURE_ICON_ALPHA_MODE_CNT] = {
    [PICTURE_ICON_ALPHA_MODE_NONE] = TEXT("无Alpha通道"),
    [PICTURE_ICON_ALPHA_MODE_MASK] = TEXT("蒙板图"),
    [PICTURE_ICON_ALPHA_MODE_ALPHA_CHANNEL] = TEXT("32位Alpha通道"),
};
static const HANPSTR sg_pIconInfoKeyName[PICTURE_ICON_KEY_CNT] = {
    [PICTURE_ICON_KEY_TOTAL] = TEXT("图标总数"),
    [PICTURE_ICON_KEY_ID] = TEXT("图标序号"),
    [PICTURE_ICON_KEY_RESOLUTION] = TEXT("分辨率"),
    [PICTURE_ICON_KEY_PIXEL_BITS] = TEXT("像素位数"),
    [PICTURE_ICON_KEY_ROW_BYTES] = TEXT("每行字节数"),
    [PICTURE_ICON_KEY_PLANES] = TEXT("平面数"),
    [PICTURE_ICON_KEY_ALPHA_MODE] = TEXT("Alpha通道模式"),
};

BOOL CheckPathHaveIcons(HINSTANCE hInst, HANPCSTR pPath)
{
    BOOL bRet = FALSE;

    if (0 < (HANINT)((INT_PTR)ExtractIcon(hInst, pPath, -1))) { bRet = TRUE; }

    return bRet;
}

void RegisterHANPictureIcon(HINSTANCE hInst)
{
    WNDCLASSEX wcex = {
        .cbSize         = sizeof(WNDCLASSEX),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = PictureIconWndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = sizeof(PPICTUREICONEXTRA),
        .hInstance      = hInst,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName   = NULL,
        .lpszClassName  = HAN_PICTURE_ICON_CLASS,
        .hIconSm        = NULL,
    };
    RegisterClassEx(&wcex);
}

static LRESULT CALLBACK PictureIconWndProc(HWND hPictureIcon, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    PPICTUREICONEXTRA iconInfo = (PPICTUREICONEXTRA)GetWindowLongPtr(hPictureIcon, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = CreateCallback(hPictureIcon, lParam);
        } break;
        case WM_SIZE: {
            SizeCallback(hPictureIcon, iconInfo);
        } break;
        case WM_CTLCOLORSTATIC: {
            lWndProcRet = (INT_PTR)GetStockObject(WHITE_BRUSH);
        } break;
        case WM_NOTIFY: {
            lWndProcRet = NotifyCallback(iconInfo, (NMHDR*)lParam);
        } break;
        case WM_DESTROY: {
            DestroyCallback(iconInfo);
        } break;

        case PCTM_GETSAVEPARAM: {
            lWndProcRet = GetSaveParamCallback(iconInfo, (PPICTURESAVEPARAM)lParam);
        } break;
        case PCTM_ZOOM: {
            (void)SendMessage(iconInfo->hEditTool, message, wParam, lParam);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hPictureIcon, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}

static LRESULT CreateCallback(HWND hPictureIcon, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PPICTUREICONEXTRA iconInfo;
    PPICTUREICONEXTRA iconTempInfo;
    HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
    PPICTURECREATEPARAM pPictureCreateParam = (PPICTURECREATEPARAM)(((LPCREATESTRUCT)lParam)->lpCreateParams);
    RECT rcClientSize;
    HANINT nIconCnt;

    HANINT nWinX;
    HANINT nWinY;
    HANINT nWinW;
    HANINT nWinH;

    HANDLE hHeap = GetProcessHeap();
    if (NULL == hHeap) { lWndProcRet = -1; }
    if (-1 != lWndProcRet)
    {
        nIconCnt = (HANINT)((INT_PTR)ExtractIcon(hInst, pPictureCreateParam->pFileName, -1));
        if (nIconCnt <= 0) { lWndProcRet = -1; }
    }
    if (-1 != lWndProcRet)
    {
        iconInfo = (PPICTUREICONEXTRA)HANWinHeapAlloc(hHeap, NULL, sizeof(PICTUREICONEXTRA));
        if (NULL == iconInfo) { lWndProcRet = -1; }
    }
    /* 创建窗口 */
    if (-1 != lWndProcRet)
    {
        SetWindowLongPtr(hPictureIcon, 0, (LONG_PTR)iconInfo);

        iconInfo->hHeap = hHeap;
        iconInfo->hInst = hInst;
        iconInfo->hSelf = hPictureIcon;
        iconInfo->iconList.nIconCnt = nIconCnt;
        iconInfo->iconList.nCursel = 0;
        
        GetClientRect(hPictureIcon, &rcClientSize);

        iconInfo->hFont.hHex = CreateFontIndirect(&g_lfHexFont);
        iconInfo->hFont.hSys = CreateFontIndirect(&g_lfInfoFont);

        nWinX = PICTURE_WINDOW_DX;
        nWinY = PICTURE_WINDOW_DY;
        nWinW = PICTURE_ICON_INFO_KEY_WIDTH + PICTURE_ICON_INFO_VALUE_WIDTH;
        nWinH = PICTURE_ICON_INFO_HEIGHT;
        iconInfo->hInfo = CreateWindow(WC_LISTVIEW, NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL,
            nWinX, nWinY, nWinW, nWinH,
            hPictureIcon, (HMENU)WID_PICTURE_ICON_INFO, hInst, NULL
        );
        nWinX += nWinW + PICTURE_WINDOW_DX;
        nWinW = 320;
        iconInfo->iconList.hIconList = CreateWindow(WC_LISTVIEW, NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_ICON | LVS_SINGLESEL,
            nWinX, nWinY, nWinW, nWinH,
            hPictureIcon, (HMENU)WID_PICTURE_ICON_LIST, hInst, NULL
        );

        SendMessage(iconInfo->iconList.hIconList, WM_SETFONT, (WPARAM)(iconInfo->hFont.hSys), (LPARAM)TRUE);
        SendMessage(iconInfo->hInfo, WM_SETFONT, (WPARAM)(iconInfo->hFont.hSys), (LPARAM)TRUE);

        InitIconList(iconInfo->iconList.hIconList);
        LoadIconToImageList(iconInfo, pPictureCreateParam->pFileName);
        InitIconInfo(iconInfo->hInfo);
    }
    /* 重新分配 iconInfo 内存 */
    if (-1 != lWndProcRet)
    {
        iconTempInfo = ReallocIconInfoMemory(iconInfo);
        if (NULL != iconTempInfo)
        {
            iconInfo = iconTempInfo;
            SetWindowLongPtr(hPictureIcon, 0, (LONG_PTR)iconInfo);
        }
        else { lWndProcRet = -1; }
    }
    /* 获取图片 */
    if (-1 != lWndProcRet)
    {
        LoadPictureFromIcon(iconInfo);
        UpdateIconInfo(iconInfo);
        nWinX = PICTURE_WINDOW_DX;
        nWinY += PICTURE_ICON_INFO_HEIGHT + PICTURE_WINDOW_DY;
        nWinW = GetRectW(&rcClientSize) - (PICTURE_WINDOW_DY * 2);
        nWinH = GetRectH(&rcClientSize) - nWinY - PICTURE_WINDOW_DY;
        iconInfo->hEditTool = CreateWindow(HAN_PICTURE_EDIT_TOOL_CLASS, NULL,
            WS_CHILD | WS_VISIBLE, nWinX, nWinY, nWinW, nWinH,
            hPictureIcon, (HMENU)WID_PICTURE_EDIT_TOOL, hInst, iconInfo->pictureData.pPictureInfo
        );
    }

    return lWndProcRet;
}
static void SizeCallback(HWND hPictureIcon, PPICTUREICONEXTRA iconInfo)
{
    HANINT nWinX = PICTURE_WINDOW_DX;
    HANINT nWinY = PICTURE_ICON_INFO_HEIGHT + (PICTURE_WINDOW_DY * 2);
    HANINT nWinW;
    HANINT nWinH;
    RECT rcClientSize;
    
    GetClientRect(hPictureIcon, &rcClientSize);
    nWinW = GetRectW(&rcClientSize) - (PICTURE_WINDOW_DY * 2);
    nWinH = GetRectH(&rcClientSize) - nWinY - PICTURE_WINDOW_DY;

    MoveWindow(iconInfo->hEditTool, nWinX, nWinY, nWinW, nWinH, TRUE);
}
static LRESULT NotifyCallback(PPICTUREICONEXTRA iconInfo, NMHDR* pNotify)
{
    LRESULT lWndProcRet = 0;

    switch (pNotify->idFrom) {
        case WID_PICTURE_ICON_LIST: {
            lWndProcRet = IconListNotifyCallback(iconInfo, pNotify);
        } break;

        default: { } break;
    }

    return lWndProcRet;
}
static void DestroyCallback(PPICTUREICONEXTRA iconInfo)
{
    ImageList_Destroy(iconInfo->iconList.imageList);
    HANWinHeapFree(iconInfo->hHeap, 0, iconInfo);
}
static LRESULT GetSaveParamCallback(PPICTUREICONEXTRA iconInfo, PPICTURESAVEPARAM pSaveParam)
{
    HAN_strcpy(pSaveParam->pOpenClassName, HAN_PICTURE_ICON_CLASS);
    pSaveParam->pPicture = iconInfo->pictureData.pPictureInfo->pPicture[PictureEditToolGetPaintId(iconInfo->hEditTool)];
    pSaveParam->pParam = iconInfo;

    return TRUE;
}
static void InitIconList(HWND hIconList)
{
    LVCOLUMN lvTitle = { .mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM, };
    lvTitle.iSubItem = 0;
    lvTitle.pszText = TEXT("图标名称");
    lvTitle.cx = 100;
    lvTitle.fmt = LVCFMT_CENTER;
    ListView_InsertColumn(hIconList, 0, &lvTitle);
}
static void InitIconInfo(HWND hIconInfo)
{
    HANCHAR pText[HAN_PICTURE_ICON_TEXT_BUF_SIZE];
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
    lvTitle.cx = PICTURE_ICON_INFO_KEY_WIDTH;
    lvTitle.pszText = TEXT("属性");
    ListView_InsertColumn(hIconInfo, 0, &lvTitle);
    lvTitle.iSubItem = 1;
    lvTitle.cx = PICTURE_ICON_INFO_VALUE_WIDTH;
    lvTitle.pszText = TEXT("值");
    ListView_InsertColumn(hIconInfo, 1, &lvTitle);

    for (PICTUREICONKEY iLoop = 0; iLoop < PICTURE_ICON_KEY_CNT; iLoop++)
    {
        lvItem.iItem = iLoop;
        lvItem.pszText = sg_pIconInfoKeyName[iLoop];
        ListView_InsertItem(hIconInfo, &lvItem);
    }
}
static void LoadIconToImageList(PPICTUREICONEXTRA iconInfo, HANPCSTR pFileName)
{
    HANINT nIconCnt = iconInfo->iconList.nIconCnt;
    HIMAGELIST hImageList;
    HICON hIcon;
    HANCHAR pText[PATH_STR_SIZE];
    LVITEM lvItem = { .mask = LVIF_TEXT | LVIF_IMAGE, };
    
    hImageList = ImageList_Create(GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), ILC_MASK | ILC_COLOR32, 1, 1);
    ImageList_SetBkColor(hImageList, RGB(0xFF, 0xFF, 0xFF));
    ListView_DeleteAllItems(iconInfo->iconList.hIconList);
    for (HANINT iLoop = 0; iLoop < nIconCnt; iLoop++)
    {
        hIcon = ExtractIcon(iconInfo->hInst, pFileName, iLoop);
        HAN_snprintf(pText, PATH_STR_SIZE, TEXT("图标%d"), iLoop + 1);
        lvItem.iItem = iLoop;
        lvItem.iImage = iLoop;
        lvItem.iSubItem = 0;
        lvItem.pszText = pText;
        ListView_InsertItem(iconInfo->iconList.hIconList, &lvItem);
        ImageList_AddIcon(hImageList, hIcon);
    }
    ListView_SetImageList(iconInfo->iconList.hIconList, hImageList, LVSIL_NORMAL);
    iconInfo->iconList.imageList = hImageList;
}
static PPICTUREICONEXTRA ReallocIconInfoMemory(PPICTUREICONEXTRA iconInfo)
{
    PPICTUREICONEXTRA pRet = NULL;
    HANDLE hHeap = iconInfo->hHeap;
    HANINT nIconCnt = iconInfo->iconList.nIconCnt;
    PPICTURERESOLUTION pResolution;
    SIZE_T nOffset;
    SIZE_T nPictureInfoSize;
    SIZE_T nPictureSize;
    SIZE_T nIconBitmapInfoSize;
    SIZE_T nBitmapDataSize;

    pResolution = (PPICTURERESOLUTION)HANWinHeapAlloc(hHeap, NULL, sizeof(PICTURERESOLUTION) * nIconCnt);
    if (NULL != pResolution)
    {
        nPictureInfoSize = GetPictureInfoMemSize(nIconCnt);
        nPictureSize = GetIconsPictureMemSize(iconInfo, pResolution);
        nIconBitmapInfoSize = sizeof(PICTUREICONBITMAPINFO) * nIconCnt;
        nBitmapDataSize = GetIconsBitmapMemSize(nIconCnt, pResolution);
        if (0 < nPictureSize)
        {
            pRet = (PPICTUREICONEXTRA)HANWinHeapAlloc(
                hHeap, iconInfo,
                sizeof(PICTUREICONEXTRA)
                + nPictureInfoSize
                + nPictureSize
                + nIconBitmapInfoSize
                + nBitmapDataSize
            );
        }

        if (NULL != pRet)
        {
            nOffset = 0;
            pRet->pictureData.pPictureInfo = (HANPPICTUREINFO)&(pRet->pBuf[nOffset]); nOffset += nPictureInfoSize;
            pRet->pictureData.pPictureInfo->nCnt = nIconCnt;
            for (HANINT iLoop = 0; iLoop < nIconCnt; iLoop++)
            {
                nPictureSize = GetPictureMemSize(&pResolution[iLoop]);
                pRet->pictureData.pPictureInfo->pPicture[iLoop] = (HANPPICTURE)&(pRet->pBuf[nOffset]); nOffset += nPictureSize;
                UpdatePictureMap(pRet->pictureData.pPictureInfo->pPicture[iLoop], &pResolution[iLoop]);
            }
            pRet->pictureData.pIconBitmapInfo = (PPICTUREICONBITMAPINFO)&(pRet->pBuf[nOffset]); nOffset += nIconBitmapInfoSize;
            pRet->pictureData.pBitmapData = (uint8_t*)&(pRet->pBuf[nOffset]); nOffset += nPictureSize;
        }
    }

    if (NULL != pResolution) { HANWinHeapFree(hHeap, 0, pResolution); }

    return pRet;
}
static SIZE_T GetIconsPictureMemSize(PPICTUREICONEXTRA iconInfo, PPICTURERESOLUTION pResolution)
{
    SIZE_T nRet;
    HANINT nIconCnt = iconInfo->iconList.nIconCnt;
    HICON hIcon;
    ICONINFO iiInfo;
    BITMAP bmIcon;
    SIZE_T nPictureSize;

    nRet = 0;
    for (HANINT iLoop = 0; iLoop < nIconCnt; iLoop++)
    {
        nPictureSize = 0;
        hIcon = ImageList_GetIcon(iconInfo->iconList.imageList, iLoop, 0);
        GetIconInfo(hIcon, &iiInfo);
        if (NULL != iiInfo.hbmColor)
        {
            GetObject(iiInfo.hbmColor, sizeof(BITMAP), &bmIcon);
            pResolution[iLoop].pxWidth = bmIcon.bmWidth;
            pResolution[iLoop].pxHeight = bmIcon.bmHeight;
            nPictureSize = GetPictureMemSize(&pResolution[iLoop]);
        }
        else if (NULL != iiInfo.hbmMask)
        {
            GetObject(iiInfo.hbmMask, sizeof(BITMAP), &bmIcon);
            pResolution[iLoop].pxWidth = bmIcon.bmWidth;
            pResolution[iLoop].pxHeight = bmIcon.bmHeight;
            nPictureSize = GetPictureMemSize(&pResolution[iLoop]);
        }
        else
        {
            nRet = 0;
            break;
        }
        nRet += nPictureSize;
        
        if (NULL != iiInfo.hbmMask){ DeleteObject(iiInfo.hbmMask); }
        if (NULL != iiInfo.hbmColor){ DeleteObject(iiInfo.hbmColor); }
        DestroyIcon(hIcon);
    }

    return nRet;
}
static SIZE_T GetIconsBitmapMemSize(HANINT nCnt, PPICTURERESOLUTION pResolution)
{
    SIZE_T nRet;
    SIZE_T nSize;

    nRet = 0;
    for (HANINT iLoop = 0; iLoop < nCnt; iLoop++)
    {
        nSize = 4 * pResolution[iLoop].pxWidth * pResolution[iLoop].pxHeight;
        if (nRet < nSize) { nRet = nSize; }
    }

    return nRet;
}
static void LoadPictureFromIcon(PPICTUREICONEXTRA iconInfo)
{
    HDC hdcIcon;
    HDC hdcMem;
    HIMAGELIST hImageList = iconInfo->iconList.imageList;
    uint8_t* pBitmapData = iconInfo->pictureData.pBitmapData;
    HANPPICTUREINFO pPictureInfo = iconInfo->pictureData.pPictureInfo;
    HANINT nIconCnt = (HANINT)(pPictureInfo->nCnt);
    PPICTUREICONBITMAPINFO pIconBitmapInfo;
    HICON hIcon;
    ICONINFO iiInfo;
    BITMAPINFO bmiIcon = { 0 };
    PICTURERESOLUTION pxResolution;
    BOOL bAlpha;
    HANPPICTURE pPicture;

    hdcIcon = GetDC(iconInfo->hSelf);
    hdcMem = CreateCompatibleDC(hdcIcon);
    
    for (HANINT iLoop = 0; iLoop < nIconCnt; iLoop++)
    {
        pIconBitmapInfo = &(iconInfo->pictureData.pIconBitmapInfo[iLoop]);
        pIconBitmapInfo->eAlpha = PICTURE_ICON_ALPHA_MODE_NONE;
        hIcon = ImageList_GetIcon(hImageList, iLoop, 0);
        /* 获取图标掩码和色彩的属性 */
        if (FALSE == GetIconInfo(hIcon, &iiInfo))
        {
            iiInfo.hbmMask = NULL;
            iiInfo.hbmColor = NULL;
        }
        /* 获取掩码属性（如果有） */
        if (NULL != iiInfo.hbmMask)
        {
            if (0 != GetObject(iiInfo.hbmMask, sizeof(BITMAP), &(pIconBitmapInfo->bmBitmap)))
            {
                pxResolution.pxWidth = pIconBitmapInfo->bmBitmap.bmWidth;
                pxResolution.pxHeight = pIconBitmapInfo->bmBitmap.bmHeight;
                CopyBitmapToPictureBitmapInfo(&(pIconBitmapInfo->bmInfo), &(pIconBitmapInfo->bmBitmap));
            }
        }
        /* 获取色彩属性（如果有） */
        if (NULL != iiInfo.hbmColor)
        {
            if (0 != GetObject(iiInfo.hbmColor, sizeof(BITMAP), &(pIconBitmapInfo->bmBitmap)))
            {
                pxResolution.pxWidth = pIconBitmapInfo->bmBitmap.bmWidth;
                pxResolution.pxHeight = pIconBitmapInfo->bmBitmap.bmHeight;
                CopyBitmapToPictureBitmapInfo(&(pIconBitmapInfo->bmInfo), &(pIconBitmapInfo->bmBitmap));
            }
        }
        /* 读取位图的值 */
        bmiIcon.bmiHeader = pIconBitmapInfo->bmInfo.bmiHeader;
        bmiIcon.bmiHeader.biHeight *= -1;
        pIconBitmapInfo->bmInfo.pixelData.pData = pBitmapData;
        pIconBitmapInfo->bmInfo.colorTable.pRGBQuad = bmiIcon.bmiColors;
        bAlpha = FALSE;
        pPicture = pPictureInfo->pPicture[iLoop];
        if (NULL != iiInfo.hbmColor) // 扫描色彩，确认是否含 Alpha 通道
        {
            if (0 != GetDIBits(hdcMem, iiInfo.hbmColor, 0, pxResolution.pxHeight, pBitmapData, &bmiIcon, DIB_RGB_COLORS))
            {
                DecodeBitmapColor(pPicture, &(pIconBitmapInfo->bmInfo), &bAlpha);
                if (TRUE == bAlpha) { pIconBitmapInfo->eAlpha = PICTURE_ICON_ALPHA_MODE_ALPHA_CHANNEL; }
            }
        }
        if ((NULL != iiInfo.hbmMask) && (FALSE == bAlpha)) // 扫描掩码
        {
            if (0 != GetDIBits(hdcMem, iiInfo.hbmMask, 0, pxResolution.pxHeight, pBitmapData, &bmiIcon, DIB_RGB_COLORS))
            {
                DecodeBitmapMask(pPicture, &(pIconBitmapInfo->bmInfo));
                pIconBitmapInfo->eAlpha = PICTURE_ICON_ALPHA_MODE_MASK;
            }
        }
        
        if (NULL != iiInfo.hbmMask){ DeleteObject(iiInfo.hbmMask); }
        if (NULL != iiInfo.hbmColor){ DeleteObject(iiInfo.hbmColor); }
        DestroyIcon(hIcon);
    }

    DeleteDC(hdcMem);
    ReleaseDC(iconInfo->hSelf, hdcIcon);
}
static void UpdateIconInfo(PPICTUREICONEXTRA iconInfo)
{
    HWND hInfo = iconInfo->hInfo;
    HANCHAR pText[HAN_PICTURE_ICON_TEXT_BUF_SIZE];
    HANCHAR pCat[HAN_PICTURE_ICON_TEXT_BUF_SIZE];
    LVITEM lvItem =
    {
        .mask = LVIF_TEXT,
        .iSubItem = 1,
        .pszText = pText,
    };
    HANINT nCursel = iconInfo->iconList.nCursel;
    PPICTUREICONBITMAPINFO pIconBitmapInfo = &(iconInfo->pictureData.pIconBitmapInfo[nCursel]);

    lvItem.iItem = PICTURE_ICON_KEY_TOTAL;
    HAN_snprintf(pText, ArrLen(pText), TEXT("%d"), iconInfo->iconList.nIconCnt);
    ListView_SetItem(hInfo, &lvItem);

    lvItem.iItem = PICTURE_ICON_KEY_ID;
    HAN_snprintf(pText, ArrLen(pText), TEXT("%d"), nCursel + 1);
    ListView_SetItem(hInfo, &lvItem);

    lvItem.iItem = PICTURE_ICON_KEY_RESOLUTION;
    HAN_snprintf(pText, ArrLen(pText), TEXT("%ld×%ld"), pIconBitmapInfo->bmBitmap.bmWidth, pIconBitmapInfo->bmBitmap.bmHeight);
    ListView_SetItem(hInfo, &lvItem);

    lvItem.iItem = PICTURE_ICON_KEY_PIXEL_BITS;
    GetBitmapPixelSizeName(pIconBitmapInfo->bmBitmap.bmBitsPixel, pCat, ArrLen(pCat));
    HAN_snprintf(pText, ArrLen(pText), TEXT("%u（%s）"), pIconBitmapInfo->bmBitmap.bmBitsPixel, pCat);
    ListView_SetItem(hInfo, &lvItem);

    lvItem.iItem = PICTURE_ICON_KEY_ROW_BYTES;
    HAN_snprintf(pText, ArrLen(pText), TEXT("%ld"), pIconBitmapInfo->bmBitmap.bmWidthBytes);
    ListView_SetItem(hInfo, &lvItem);

    lvItem.iItem = PICTURE_ICON_KEY_PLANES;
    HAN_snprintf(pText, ArrLen(pText), TEXT("%u"), pIconBitmapInfo->bmBitmap.bmPlanes);
    ListView_SetItem(hInfo, &lvItem);

    lvItem.iItem = PICTURE_ICON_KEY_ALPHA_MODE;
    HAN_snprintf(pText, ArrLen(pText), TEXT("%s"), sg_pAlphaModeName[pIconBitmapInfo->eAlpha]);
    ListView_SetItem(hInfo, &lvItem);
}
static LRESULT IconListNotifyCallback(PPICTUREICONEXTRA iconInfo, NMHDR* pNotify)
{
    LRESULT lWndProcRet = 0;

    switch (pNotify->code) {
        case NM_CLICK: {
            NMITEMACTIVATE* pItemAct = (NMITEMACTIVATE*)pNotify;
            if (-1 != pItemAct->iItem)
            {
                if (pItemAct->iItem != iconInfo->iconList.nCursel)
                {
                    iconInfo->iconList.nCursel = pItemAct->iItem;
                    UpdateIconInfo(iconInfo);
                    PictureEditToolSetPaintId(iconInfo->hEditTool, (HANSIZE)(iconInfo->iconList.nCursel), TRUE);
                }
            }
        } break;

        default: { } break;
    }

    return lWndProcRet;
}
