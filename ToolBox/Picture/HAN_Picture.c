#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <Windows.h>
#include <shlwapi.h>
#include <CommCtrl.h>

#include "..\..\HAN_Lib\HAN_windows.h"
#include "..\..\HAN_Lib\HAN_CommCtrl.h"
#include "..\GlobalVariables.h"
#include "HAN_Picture.h"
#include "PictureModule\EditTool\HAN_PictureEditTool.h"
#include "PictureModule\Icon\HAN_PictureIcon.h"
#include "PictureModule\Bitmap\HAN_PictureBitmap.h"
#include "PictureModule\Png\HAN_PicturePng.h"
#include "PictureModule\Jpeg\HAN_PictureJpeg.h"

#define INI_PICTURE_VALUE_SIZE          INI_MAIN_VALUE_STR_SIZE
#define INI_HAN_Picture_APP_NAME        TEXT("pctConfig")

typedef enum {
    PICTURE_TOOLBAR_BUTTON_OPEN_PICTURE,
    PICTURE_TOOLBAR_BUTTON_LOAD_ICON,
    PICTURE_TOOLBAR_BUTTON_SAVE,
    PICTURE_TOOLBAR_BUTTON_CNT,
} PICTURETOOLBARBUTTON;

typedef enum {
    PICTURE_TYPE_BITMAP,
    PICTURE_TYPE_JPEG,
    PICTURE_TYPE_PNG,
    PICTURE_TYPE_CNT,
} PICTURETYPE;

typedef enum {
    PICTURE_PROCESS_STATUS_OPEN_FILE,
    PICTURE_PROCESS_STATUS_GET_PICTURE_TYPE,
    PICTURE_PROCESS_STATUS_READ_FILE,
    PICTURE_PROCESS_STATUS_CHECK_PICTURE_TYPE,
    PICTURE_PROCESS_STATUS_TRY_OTHER_PICTURE_TYPE,
    PICTURE_PROCESS_STATUS_PROCESS,
    PICTURE_PROCESS_STATUS_EXIT,
} PICTUREPROCESSSTATUS;

typedef struct tagPICTUREBUF {
    uint8_t*                pBuf;
    HANSIZE                 nLen;
} PICTUREBUF, * PPICTUREBUF;

typedef struct tagPICTUREPROCESS {
    HANPCSTR                pTypeName;
    HANSIZE                 nPathExtensionCnt;
    const HANPCSTR*         pPathExtension;
    HANPCSTR                pPictureClass;
    BOOL                    (*CheckPictureType)(const uint8_t* pData, HANSIZE nLen);
    void                    (*RegisterHANPictureModule)(HINSTANCE hInst);
    void                    (*SavePicture)(HANPCSTR pFileName, PPICTURESAVEPARAM pSaveParam);
} PICTUREPROCESS, * PPICTUREPROCESS;

typedef struct tagPICTUREWNDEXTRA {
    HANDLE                  hHeap;
    HINSTANCE               hInst;
    HWND                    hSelf;
    HWND                    hTab;
    HBRUSH                  hEditToolBackground;
    struct {
        ACCEL               pAccel[HKID_PICTURE_CNT];
        HACCEL              hAccel;
    } hotKey;
    struct {
        HWND                hToolbar;
        HIMAGELIST          hImageList;
    } toolbar;
    struct {
        HFONT               hHex;
        HFONT               hSys;
        HFONT               hTab;
    } hFont;                /* 字体 */
} PICTUREWNDEXTRA, * PPICTUREWNDEXTRA;

typedef struct tagPICTURETABPARAM {
    PICTURECREATEPARAM      createParam;
    HWND                    hPicture;
    HWND                    hClose;
} PICTURETABPARAM, * PPICTURETABPARAM;

static LRESULT CALLBACK PictureWndProc(HWND hPicture, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT PictureTabSubClassWndProc(HWND hTab, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR nId, DWORD_PTR pParam);

static LRESULT CreateCallback(HWND hPicture, LPARAM lParam);
static void SizeCallback(HWND hPicture, PPICTUREWNDEXTRA pctInfo);
static void CommandCallback(PPICTUREWNDEXTRA pctInfo, WPARAM wParam, LPARAM lParam);
static LRESULT NotifyCallback(PPICTUREWNDEXTRA pctInfo, NMHDR* pNMHDR);
static void DestroyCallback(PPICTUREWNDEXTRA pctInfo);
static void InitToolbar(PPICTUREWNDEXTRA pctInfo);
static void GetTabSize(PPICTUREWNDEXTRA pctInfo, RECT* pSize);
static void GetSubWindowSize(PPICTUREWNDEXTRA pctInfo, RECT* pSize);
static void OpenFileCallback(PPICTUREWNDEXTRA pctInfo);
static void LoadIconCallback(PPICTUREWNDEXTRA pctInfo);
static void SaveFileCallback(PPICTUREWNDEXTRA pctInfo);
static void HotKeyZoomCallback(PPICTUREWNDEXTRA pctInfo, HANINT nZoom);
static BOOL OpenPictureFile(HANPSTR pFileName, HWND hOwner);
static PICTURETYPE GetPictureFileType(HANPSTR pFileName);
static BOOL ReadPictureFile(HANPSTR pFileName, PPICTUREWNDEXTRA pctInfo, PPICTURECREATEPARAM pCreateParam);
static PICTURETYPE TryOtherPictureType(PPICTURECREATEPARAM pCreateParam);
static BOOL ConfirmOpenNewType(PPICTUREWNDEXTRA pctInfo, PICTURETYPE pctType);
static void FileDataFormatErrorCallback(PPICTUREWNDEXTRA pctInfo);
static BOOL AppendPictureTabWindow(PPICTUREWNDEXTRA pctInfo, HWND hPicture, PPICTURECREATEPARAM pCreateParam);
static void ClosePicture(PPICTUREWNDEXTRA pctInfo, HWND hClose);
static void GetCloseButtonPos(const RECT* pItemPos, RECT* pClosePos);
static void UpdateCloseButtonPos(PPICTUREWNDEXTRA pctInfo);
static PICTURETYPE GetPictureClassType(HANPCSTR pClass);
static void GenerateFileNameFilter(HANPSTR pFilter, HANSIZE nLen);
static PICTURETYPE UpdateSaveFileType(HANPSTR pFileName, PICTURETYPE pctType);

static const PICTUREWNDEXTRA sg_pctDefaultCfg = {
    .hotKey = {
        .pAccel = {
            [HKID_PICTURE_SAVE] = {
                .fVirt = FVIRTKEY | FCONTROL,
                .key = 'S',
                .cmd = WID_PICTURE_ACCEL_COMMAND_SAVE,
            },
            [HKID_PICTURE_ZOOM_IN] = {
                .fVirt = FVIRTKEY | FCONTROL,
                .key = VK_ADD,
                .cmd = WID_PICTURE_ACCEL_COMMAND_ZOOM_IN,
            },
            [HKID_PICTURE_ZOOM_OUT] = {
                .fVirt = FVIRTKEY | FCONTROL,
                .key = VK_SUBTRACT,
                .cmd = WID_PICTURE_ACCEL_COMMAND_ZOOM_OUT,
            },
        },
        .hAccel = NULL,
    },
};
static const TBBUTTON sg_pToolbarInfo[PICTURE_TOOLBAR_BUTTON_CNT] = {
    [PICTURE_TOOLBAR_BUTTON_OPEN_PICTURE] = {
        .iBitmap = MAKELONG(STD_FILEOPEN, 0),
        .idCommand = WID_PICTURE_TOOLBAR_OPEN_PICTURE,
        .fsState = TBSTATE_ENABLED,
        .fsStyle = BTNS_AUTOSIZE,
        .bReserved = { 0 },
        .dwData = 0,
        .iString = (INT_PTR)TEXT("打开图片"),
    },
    [PICTURE_TOOLBAR_BUTTON_LOAD_ICON] = {
        .iBitmap = MAKELONG(STD_PROPERTIES, 0),
        .idCommand = WID_PICTURE_TOOLBAR_LOAD_ICON,
        .fsState = TBSTATE_ENABLED,
        .fsStyle = BTNS_AUTOSIZE,
        .bReserved = { 0 },
        .dwData = 0,
        .iString = (INT_PTR)TEXT("提取图标"),
    },
    [PICTURE_TOOLBAR_BUTTON_SAVE] = {
        .iBitmap = MAKELONG(STD_FILESAVE, 0),
        .idCommand = WID_PICTURE_TOOLBAR_SAVE,
        .fsState = 0,
        .fsStyle = BTNS_AUTOSIZE,
        .bReserved = { 0 },
        .dwData = 0,
        .iString = (INT_PTR)TEXT("保存"),
    },
};
static const HANPCSTR sg_bitmapPathExtension[2] = { TEXT("bmp"), TEXT("dib"), };
static const HANPCSTR sg_jpegPathExtension[2] = { TEXT("jpg"), TEXT("jpeg"), };
static const HANPCSTR sg_pngPathExtension[1] = { TEXT("png"), };
static const LOGFONT sg_lfTabFont = {
    .lfHeight = 16,
    .lfWidth = 0,
    .lfEscapement = 0,
    .lfOrientation = 0,
    .lfWeight = FW_NORMAL,
    .lfItalic = FALSE,
    .lfUnderline = FALSE,
    .lfStrikeOut = FALSE,
    .lfCharSet = DEFAULT_CHARSET,
    .lfOutPrecision = OUT_CHARACTER_PRECIS,
    .lfClipPrecision = CLIP_CHARACTER_PRECIS,
    .lfQuality = DEFAULT_QUALITY,
    .lfPitchAndFamily = FF_DONTCARE,
    .lfFaceName = TEXT("等线"),
};
static const PICTUREPROCESS sg_pPictureInfo[PICTURE_TYPE_CNT] = {
    [PICTURE_TYPE_BITMAP] = {
        .pTypeName = TEXT("Bitmap"),
        .nPathExtensionCnt = ArrLen(sg_bitmapPathExtension),
        .pPathExtension = sg_bitmapPathExtension,
        .pPictureClass = HAN_PICTURE_BITMAP_CLASS,
        .CheckPictureType = CheckBitmapType,
        .RegisterHANPictureModule = RegisterHANPictureBitmap,
        .SavePicture = SavePictureBitmap,
    },
    [PICTURE_TYPE_JPEG] = {
        .pTypeName = TEXT("JPEG"),
        .nPathExtensionCnt = ArrLen(sg_jpegPathExtension),
        .pPathExtension = sg_jpegPathExtension,
        .pPictureClass = HAN_PICTURE_JPEG_CLASS,
        .CheckPictureType = CheckJpegType,
        .RegisterHANPictureModule = RegisterHANPictureJpeg,
        .SavePicture = SavePictureJpeg,
    },
    [PICTURE_TYPE_PNG] = {
        .pTypeName = TEXT("PNG"),
        .nPathExtensionCnt = ArrLen(sg_pngPathExtension),
        .pPathExtension = sg_pngPathExtension,
        .pPictureClass = HAN_PICTURE_PNG_CLASS,
        .CheckPictureType = CheckPngType,
        .RegisterHANPictureModule = RegisterHANPicturePng,
        .SavePicture = SavePicturePng,
    },
};

void RegisterHANPicture(HINSTANCE hInst)
{
    WNDCLASSEX wcex = {
        .cbSize         = sizeof(WNDCLASSEX),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = PictureWndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = sizeof(PPICTUREWNDEXTRA),
        .hInstance      = hInst,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName   = NULL,
        .lpszClassName  = HAN_PICTURE_CLASS,
        .hIconSm        = NULL,
    };
    RegisterClassEx(&wcex);
}

HACCEL TranslatePictureAccelerator(HWND hPicture)
{
    PPICTUREWNDEXTRA pctInfo = (PPICTUREWNDEXTRA)GetWindowLongPtr(hPicture, 0);

    return pctInfo->hotKey.hAccel;
}

static LRESULT CALLBACK PictureWndProc(HWND hPicture, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    PPICTUREWNDEXTRA pctInfo = (PPICTUREWNDEXTRA)GetWindowLongPtr(hPicture, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = CreateCallback(hPicture, lParam);
        } break;
        case WM_SIZE: {
            SizeCallback(hPicture, pctInfo);
        } break;
        case WM_CTLCOLORSTATIC: {
            lWndProcRet = (INT_PTR)GetStockObject(WHITE_BRUSH);
        } break;
        case WM_COMMAND: {
            CommandCallback(pctInfo, wParam, lParam);
        } break;
        case WM_NOTIFY: {
            lWndProcRet = NotifyCallback(pctInfo, (NMHDR*)lParam);
        } break;
        case WM_DESTROY: {
            DestroyCallback(pctInfo);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hPicture, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}
static LRESULT PictureTabSubClassWndProc(HWND hTab, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR nId, DWORD_PTR pParam)
{
    (void)nId;
    PPICTUREWNDEXTRA pctInfo = (PPICTUREWNDEXTRA)pParam;

    switch (message) {
        case WM_COMMAND: {
            CommandCallback(pctInfo, wParam, lParam);
        } break;
        case WM_HSCROLL: {
            UpdateCloseButtonPos(pctInfo);
        } break;

        default: { } break;
    }

    return DefSubclassProc(hTab, message, wParam, lParam);
}

static LRESULT CreateCallback(HWND hPicture, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PPICTUREWNDEXTRA pctInfo;
    HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
    RECT rcClientSize;
    RECT rcToolbarSize;

    INT nWinX = PICTURE_WINDOW_DX;
    INT nWinY = PICTURE_WINDOW_DY;
    INT nWinW;
    INT nWinH;

    HANDLE hHeap = GetProcessHeap();
    if (NULL == hHeap) { lWndProcRet = -1; }
    if (-1 != lWndProcRet)
    {
        pctInfo = (PPICTUREWNDEXTRA)HANWinHeapAlloc(hHeap, NULL, sizeof(PICTUREWNDEXTRA));
        if (NULL == pctInfo) { lWndProcRet = -1; }
    }

    if (-1 != lWndProcRet)
    {
        SetWindowLongPtr(hPicture, 0, (LONG_PTR)pctInfo);

        *pctInfo = sg_pctDefaultCfg;

        pctInfo->hHeap = hHeap;
        pctInfo->hInst = hInst;
        pctInfo->hSelf = hPicture;

        GetClientRect(hPicture, &rcClientSize);

        pctInfo->hEditToolBackground = CreateSolidBrush(PICTURE_EDIT_TOOL_BACKGROUND_COLOR);
        RegisterHANPictureEditTool(hInst, pctInfo->hEditToolBackground);
        RegisterHANPictureIcon(hInst);
        for (PICTURETYPE iLoop = 0; iLoop < PICTURE_TYPE_CNT; iLoop++)
        {
            sg_pPictureInfo[iLoop].RegisterHANPictureModule(hInst);
        }

        pctInfo->hFont.hHex = CreateFontIndirect(&g_lfHexFont);
        pctInfo->hFont.hSys = CreateFontIndirect(&g_lfSysFont);
        pctInfo->hFont.hTab = CreateFontIndirect(&sg_lfTabFont);

        pctInfo->toolbar.hToolbar = CreateWindow(TOOLBARCLASSNAME, NULL,
            WS_CHILD | TBSTYLE_LIST | TBSTYLE_TOOLTIPS,
            0, 0, 0, 0,
            hPicture, (HMENU)WID_PICTURE_TOOLBAR, hInst, NULL
        );
        InitToolbar(pctInfo);
        GetClientRect(pctInfo->toolbar.hToolbar, &rcToolbarSize);

        nWinX = PICTURE_WINDOW_DX;
        nWinY = GetRectH(&rcToolbarSize) + PICTURE_WINDOW_DY;
        nWinW = GetRectW(&rcClientSize) - (PICTURE_WINDOW_DX * 2);
        nWinH = GetRectH(&rcClientSize) - nWinY - PICTURE_WINDOW_DX;
        pctInfo->hTab = CreateWindow(WC_TABCONTROL, NULL,
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
            nWinX, nWinY, nWinW, nWinH,
            hPicture, (HMENU)WID_PICTURE_TAB, hInst, NULL
        );
        
        SendMessage(pctInfo->hTab, WM_SETFONT, (WPARAM)(pctInfo->hFont.hTab), (LPARAM)TRUE);

        /* 选项卡子类化 */
        SetWindowSubclass(pctInfo->hTab, PictureTabSubClassWndProc, 0, (DWORD_PTR)pctInfo);
        /* 注册快捷键 */
        pctInfo->hotKey.hAccel = CreateAcceleratorTable(pctInfo->hotKey.pAccel, HKID_PICTURE_CNT);
    }

    return lWndProcRet;
}
static void SizeCallback(HWND hPicture, PPICTUREWNDEXTRA pctInfo)
{
    RECT rcClientSize;
    RECT rcToolbarSize;
    RECT rcPictureSize;
    INT nWinX;
    INT nWinY;
    INT nWinW;
    INT nWinH;
    INT nItemCnt = TabCtrl_GetItemCount(pctInfo->hTab);
    PPICTURETABPARAM pParam;
    TCITEM tiItem = { .mask = TCIF_PARAM, };
    
    SendMessage(pctInfo->toolbar.hToolbar, TB_AUTOSIZE, (WPARAM)0, (LPARAM)0);
    GetClientRect(hPicture, &rcClientSize);
    GetClientRect(pctInfo->toolbar.hToolbar, &rcToolbarSize);
    nWinX = PICTURE_WINDOW_DX;
    nWinY = GetRectH(&rcToolbarSize) + PICTURE_WINDOW_DY;
    nWinW = GetRectW(&rcClientSize) - (PICTURE_WINDOW_DX * 2);
    nWinH = GetRectH(&rcClientSize) - nWinY - PICTURE_WINDOW_DX;
    MoveWindow(pctInfo->hTab, nWinX, nWinY, nWinW, nWinH, TRUE);
    GetClientRect(pctInfo->hTab, &rcPictureSize);
    TabCtrl_AdjustRect(pctInfo->hTab, FALSE, &rcPictureSize);
    nWinW = GetRectW(&rcPictureSize);
    nWinH = GetRectH(&rcPictureSize);
    for (INT iLoop = 0; iLoop < nItemCnt; iLoop++)
    {
        TabCtrl_GetItem(pctInfo->hTab, iLoop, &tiItem);
        pParam = (PPICTURETABPARAM)(tiItem.lParam);
        if (NULL != pParam) { MoveWindow(pParam->hPicture, rcPictureSize.left, rcPictureSize.top, nWinW, nWinH, TRUE); }
    }
}
static void CommandCallback(PPICTUREWNDEXTRA pctInfo, WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam)) {
        case WID_PICTURE_TOOLBAR_OPEN_PICTURE: {
            OpenFileCallback(pctInfo);
        } break;
        case WID_PICTURE_TOOLBAR_LOAD_ICON: {
            LoadIconCallback(pctInfo);
        } break;
        case WID_PICTURE_TOOLBAR_SAVE: {
            SaveFileCallback(pctInfo);
        } break;
        case WID_PICTURE_TAB_CLOSE_BUTTON: {
            ClosePicture(pctInfo, (HWND)lParam);
        } break;
        /* 处理快捷键 */
        case WID_PICTURE_ACCEL_COMMAND_SAVE: {
            SaveFileCallback(pctInfo);
        } break;
        case WID_PICTURE_ACCEL_COMMAND_ZOOM_IN: {
            HotKeyZoomCallback(pctInfo, 1);
        } break;
        case WID_PICTURE_ACCEL_COMMAND_ZOOM_OUT: {
            HotKeyZoomCallback(pctInfo, -1);
        } break;

        default: { } break;
    }
}
static LRESULT NotifyCallback(PPICTUREWNDEXTRA pctInfo, NMHDR* pNMHDR)
{
    LRESULT nRet = 0;
    INT nCurSel;
    INT nItemCnt;
    PPICTURETABPARAM pParam;
    TCITEM tiItem = { .mask = TCIF_PARAM };

    if (WID_PICTURE_TAB == pNMHDR->idFrom)
    {
        switch (pNMHDR->code) {
            case TCN_SELCHANGE: {
                nItemCnt = TabCtrl_GetItemCount(pctInfo->hTab);
                nCurSel = TabCtrl_GetCurSel(pctInfo->hTab);
                for (INT iLoop = 0; iLoop < nItemCnt; iLoop++)
                {
                    TabCtrl_GetItem(pctInfo->hTab, iLoop, &tiItem);
                    pParam = (PPICTURETABPARAM)(tiItem.lParam);
                    if (NULL != pParam)
                    {
                        if (nCurSel == iLoop) { ShowWindow(pParam->hPicture, SW_SHOW); }
                        else { ShowWindow(pParam->hPicture, SW_HIDE); }
                    }
                }
            } break;
        }
    }

    return nRet;
}
static void DestroyCallback(PPICTUREWNDEXTRA pctInfo)
{
    INT nItemCnt = TabCtrl_GetItemCount(pctInfo->hTab);
    TCITEM tiItem = { .mask = TCIF_PARAM, };
    PPICTURETABPARAM pParam;

    for (INT iLoop = 0; iLoop < nItemCnt; iLoop++)
    {
        TabCtrl_GetItem(pctInfo->hTab, iLoop, &tiItem);
        pParam = (PPICTURETABPARAM)(tiItem.lParam);
        if (NULL != pParam)
        {
            DestroyWindow(pParam->hPicture);
            DestroyWindow(pParam->hClose);
            HANWinHeapFree(pctInfo->hHeap, 0, pParam->createParam.pData);
            HANWinHeapFree(pctInfo->hHeap, 0, pParam);
        }
    }

    DestroyAcceleratorTable(pctInfo->hotKey.hAccel);

    HANWinHeapFree(pctInfo->hHeap, 0, pctInfo);
}
static void InitToolbar(PPICTUREWNDEXTRA pctInfo)
{
    pctInfo->toolbar.hImageList = ImageList_Create(PICTURE_TOOL_SIZE, PICTURE_TOOL_SIZE, ILC_COLOR16 | ILC_MASK, PICTURE_TOOLBAR_BUTTON_CNT, 0);
    SendMessage(pctInfo->toolbar.hToolbar, TB_SETIMAGELIST, (WPARAM)0, (LPARAM)(pctInfo->toolbar.hImageList));
    SendMessage(pctInfo->toolbar.hToolbar, TB_LOADIMAGES, (WPARAM)IDB_STD_SMALL_COLOR, (LPARAM)HINST_COMMCTRL);
    SendMessage(pctInfo->toolbar.hToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
    SendMessage(pctInfo->toolbar.hToolbar, TB_ADDBUTTONS, (WPARAM)PICTURE_TOOLBAR_BUTTON_CNT, (LPARAM)sg_pToolbarInfo);
    SendMessage(pctInfo->toolbar.hToolbar, TB_AUTOSIZE, 0, 0);
    SendMessage(pctInfo->toolbar.hToolbar, TB_SETMAXTEXTROWS, 0, 0);
    ShowWindow(pctInfo->toolbar.hToolbar, SW_SHOW);
}
static void GetTabSize(PPICTUREWNDEXTRA pctInfo, RECT* pSize)
{
    GetClientRect(pctInfo->hSelf, pSize);
    pSize->left += PICTURE_WINDOW_DX;
    pSize->right -= PICTURE_WINDOW_DX;
    pSize->top += PICTURE_BUTTON_HEIGHT + (PICTURE_WINDOW_DY * 2);
    pSize->bottom -= PICTURE_WINDOW_DY;
}
static void GetSubWindowSize(PPICTUREWNDEXTRA pctInfo, RECT* pSize)
{
    GetTabSize(pctInfo, pSize);
    pSize->left += 1;
    pSize->top += PICTURE_TAB_HEIGHT;
    pSize->right -= 1;
    pSize->bottom -= 1;
}
static void OpenFileCallback(PPICTUREWNDEXTRA pctInfo)
{
    PICTUREPROCESSSTATUS eStatus = PICTURE_PROCESS_STATUS_OPEN_FILE;
    HANCHAR pFileName[PATH_STR_SIZE] = { 0 };
    PICTURETYPE pctType;
    PICTURETYPE pctRetryType;
    BOOL bReadFile;
    HWND hChildPicture;
    RECT rcPictureSize;
    BOOL bWindowOk = FALSE;
    PICTURECREATEPARAM pctCreateParam = {
        .pData = NULL,
    };

    while (PICTURE_PROCESS_STATUS_EXIT != eStatus)
    {
        switch (eStatus) {
            case PICTURE_PROCESS_STATUS_OPEN_FILE: {                // 选择文件
                if (TRUE == OpenPictureFile(pFileName, pctInfo->hSelf)) { eStatus = PICTURE_PROCESS_STATUS_GET_PICTURE_TYPE; }
                else { eStatus = PICTURE_PROCESS_STATUS_EXIT; }
            } break;
            case PICTURE_PROCESS_STATUS_GET_PICTURE_TYPE: {         // 根据文件拓展名查找文件类型
                pctType = GetPictureFileType(pFileName);
                if (pctType < PICTURE_TYPE_CNT) { eStatus = PICTURE_PROCESS_STATUS_READ_FILE; }
                else { eStatus = PICTURE_PROCESS_STATUS_EXIT; }
            } break;
            case PICTURE_PROCESS_STATUS_READ_FILE: {                // 读取文件
                bReadFile = ReadPictureFile(pFileName, pctInfo, &pctCreateParam);
                if (TRUE == bReadFile) { eStatus = PICTURE_PROCESS_STATUS_CHECK_PICTURE_TYPE; }
                else { eStatus = PICTURE_PROCESS_STATUS_EXIT; }
            } break;
            case PICTURE_PROCESS_STATUS_CHECK_PICTURE_TYPE: {       // 检查文件类型
                if (TRUE == sg_pPictureInfo[pctType].CheckPictureType(pctCreateParam.pData, pctCreateParam.nLen)) { eStatus = PICTURE_PROCESS_STATUS_PROCESS; }
                else { eStatus = PICTURE_PROCESS_STATUS_TRY_OTHER_PICTURE_TYPE; }
            } break;
            case PICTURE_PROCESS_STATUS_TRY_OTHER_PICTURE_TYPE: {   // 文件类型检查失败时，尝试其它类型
                pctRetryType = TryOtherPictureType(&pctCreateParam);
                if (pctRetryType < PICTURE_TYPE_CNT)
                {
                    pctType = pctRetryType;
                    if (TRUE == ConfirmOpenNewType(pctInfo, pctRetryType)) { eStatus = PICTURE_PROCESS_STATUS_PROCESS; }
                    else { eStatus = PICTURE_PROCESS_STATUS_EXIT; }
                }
                else
                {
                    FileDataFormatErrorCallback(pctInfo);
                    eStatus = PICTURE_PROCESS_STATUS_EXIT;
                }
            } break;
            case PICTURE_PROCESS_STATUS_PROCESS: {                  // 处理图片
                GetSubWindowSize(pctInfo, &rcPictureSize);

                hChildPicture = CreateWindow(
                    sg_pPictureInfo[pctType].pPictureClass, PathFindFileName(pctCreateParam.pFileName),
                    WS_CHILD | WS_VISIBLE,
                    rcPictureSize.left, rcPictureSize.top, HAN_PICTURE_EDIT_TOOL_MIN_W, HAN_PICTURE_EDIT_TOOL_MIN_H + PICTURE_INFO_HEIGHT + (PICTURE_WINDOW_DY * 2),
                    pctInfo->hTab, (HMENU)WID_PICTURE_PICTURE_WINDOW, pctInfo->hInst, &pctCreateParam
                );
                
                if (NULL != hChildPicture)
                {
                    bWindowOk = TRUE;
                    if (FALSE == AppendPictureTabWindow(pctInfo, hChildPicture, &pctCreateParam))
                    {
                        DestroyWindow(hChildPicture);
                    }
                    eStatus = PICTURE_PROCESS_STATUS_EXIT;
                }
                else { FileDataFormatErrorCallback(pctInfo); }

                eStatus = PICTURE_PROCESS_STATUS_EXIT;
            } break;

            default: { } break;
        }
    }

    if ((FALSE == bWindowOk) && (NULL != pctCreateParam.pData)) { HANWinHeapFree(pctInfo->hHeap, 0, pctCreateParam.pData); }
}
static void LoadIconCallback(PPICTUREWNDEXTRA pctInfo)
{
    OPENFILENAME ofnOpenFile;
    HANCHAR pFileName[PATH_STR_SIZE] = { 0 };
    PICTURECREATEPARAM pctCreateParam;
    HWND hChildPicture;
    RECT rcClientSize;
    RECT rcPictureSize;

    ZeroMemory(&ofnOpenFile, sizeof(ofnOpenFile));
    ofnOpenFile.lStructSize = sizeof(ofnOpenFile);
    ofnOpenFile.hwndOwner = pctInfo->hSelf;
    ofnOpenFile.lpstrFilter = TEXT("所有文件\0*.*\0\0");
    ofnOpenFile.nFilterIndex = 0;
    ofnOpenFile.lpstrFile = pFileName;
    ofnOpenFile.nMaxFile = PATH_STR_SIZE;
    ofnOpenFile.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

    if (TRUE == GetOpenFileName(&ofnOpenFile))
    {
        if (TRUE == CheckPathHaveIcons(pctInfo->hInst, pFileName))
        {
            GetClientRect(pctInfo->hSelf, &rcClientSize);
            GetSubWindowSize(pctInfo, &rcPictureSize);
            
            HAN_strcpy(pctCreateParam.pFileName, pFileName);
            pctCreateParam.pData = NULL;
            pctCreateParam.nLen = 0;

            hChildPicture = CreateWindow(HAN_PICTURE_ICON_CLASS, NULL,
                WS_CHILD | WS_VISIBLE,
                rcPictureSize.left, rcPictureSize.top, HAN_PICTURE_EDIT_TOOL_MIN_W, HAN_PICTURE_EDIT_TOOL_MIN_H + PICTURE_INFO_HEIGHT + (PICTURE_WINDOW_DY * 2),
                pctInfo->hTab, (HMENU)WID_PICTURE_PICTURE_WINDOW, pctInfo->hInst, &pctCreateParam
            );
                
            if (NULL != hChildPicture)
            {
                HAN_strcat(pFileName, TEXT(" - 图标"));
                if (FALSE == AppendPictureTabWindow(pctInfo, hChildPicture, &pctCreateParam))
                {
                    DestroyWindow(hChildPicture);
                }
            }
        }
        else { MessageBox(pctInfo->hSelf, TEXT("目标文件没有任何图标"), TEXT("打开文件失败"), 0); }
    }
}
static void SaveFileCallback(PPICTUREWNDEXTRA pctInfo)
{
    OPENFILENAME ofnOpenFile;
    HANCHAR pFileName[PATH_STR_SIZE] = { 0 };
    HANCHAR pFilter[PATH_STR_SIZE];
    HANINT nItemCnt;
    HANINT nCursel;
    TCITEM tiItem = { .mask = TCIF_PARAM, };
    PPICTURETABPARAM pTabParam = NULL;
    PICTURESAVEPARAM saveParam;
    PICTURETYPE pctType;
    
    nItemCnt = TabCtrl_GetItemCount(pctInfo->hTab);
    if (0 < nItemCnt)
    {
        nCursel = TabCtrl_GetCurSel(pctInfo->hTab);
        TabCtrl_GetItem(pctInfo->hTab, nCursel, &tiItem);
        pTabParam = (PPICTURETABPARAM)(tiItem.lParam);
    }
    if (NULL != pTabParam)
    {
        if (FALSE != SendMessage(pTabParam->hPicture, PCTM_GETSAVEPARAM, (WPARAM)0, (LPARAM)(&saveParam)))
        {
            pctType = GetPictureClassType(saveParam.pOpenClassName);
            if (PICTURE_TYPE_CNT <= pctType) { pctType = PICTURE_TYPE_PNG; }

            ZeroMemory(&ofnOpenFile, sizeof(ofnOpenFile));
            ofnOpenFile.lStructSize = sizeof(ofnOpenFile);
            ofnOpenFile.hwndOwner = pctInfo->hSelf;
            ofnOpenFile.lpstrFilter = pFilter;
            ofnOpenFile.nFilterIndex = pctType + 1;
            ofnOpenFile.lpstrFile = pFileName;
            ofnOpenFile.nMaxFile = PATH_STR_SIZE;
            ofnOpenFile.Flags = OFN_OVERWRITEPROMPT | OFN_EXPLORER;

            GenerateFileNameFilter(pFilter, ArrLen(pFilter));

            if (GetSaveFileName(&ofnOpenFile))
            {
                pctType = ofnOpenFile.nFilterIndex - 1;
                pctType = UpdateSaveFileType(pFileName, pctType);
                if ((0 <= pctType) && (pctType < PICTURE_TYPE_CNT))
                {
                    HAN_strncpy(saveParam.pSaveClassName, sg_pPictureInfo[pctType].pPictureClass, CLASS_STR_SIZE);
                    saveParam.hHeap = pctInfo->hHeap;
                    sg_pPictureInfo[pctType].SavePicture(pFileName, &saveParam);
                }
            }
        }
    }
}
static void HotKeyZoomCallback(PPICTUREWNDEXTRA pctInfo, HANINT nZoom)
{
    HANINT nItemCnt;
    HANINT nCursel;
    TCITEM tiItem = { .mask = TCIF_PARAM, };
    PPICTURETABPARAM pTabParam = NULL;
    
    nItemCnt = TabCtrl_GetItemCount(pctInfo->hTab);
    if (0 < nItemCnt)
    {
        nCursel = TabCtrl_GetCurSel(pctInfo->hTab);
        TabCtrl_GetItem(pctInfo->hTab, nCursel, &tiItem);
        pTabParam = (PPICTURETABPARAM)(tiItem.lParam);
    }
    if (NULL != pTabParam) { SendMessage(pTabParam->hPicture, PCTM_ZOOM, (WPARAM)nZoom, (LPARAM)0); }
}
static BOOL OpenPictureFile(HANPSTR pFileName, HWND hOwner)
{
    OPENFILENAME ofnOpenFile;

    ZeroMemory(&ofnOpenFile, sizeof(ofnOpenFile));
    ofnOpenFile.lStructSize = sizeof(ofnOpenFile);
    ofnOpenFile.hwndOwner = hOwner;
    ofnOpenFile.lpstrFilter = TEXT("所有文件\0*.*\0\0");
    ofnOpenFile.nFilterIndex = 0;
    ofnOpenFile.lpstrFile = pFileName;
    ofnOpenFile.nMaxFile = PATH_STR_SIZE;
    ofnOpenFile.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

    return GetOpenFileName(&ofnOpenFile);
}
static PICTURETYPE GetPictureFileType(HANPSTR pFileName)
{
    PICTURETYPE tRet = PICTURE_TYPE_CNT;
    HANPCSTR pPathExtension = PathFindExtension(pFileName);
    HANSIZE nPathExtensionCnt;

    if (NULL != pPathExtension)
    {
        for (PICTURETYPE iLoop = 0; iLoop < PICTURE_TYPE_CNT; iLoop++)
        {
            nPathExtensionCnt = sg_pPictureInfo[iLoop].nPathExtensionCnt;
            for (HANSIZE jLoop = 0; jLoop < nPathExtensionCnt; jLoop++)
            {
                if (!HAN_stricmp(&pPathExtension[1], sg_pPictureInfo[iLoop].pPathExtension[jLoop]))
                {
                    tRet = iLoop;
                    break;
                }
            }
        }
    }   

    return tRet;
}
static BOOL ReadPictureFile(HANPSTR pFileName, PPICTUREWNDEXTRA pctInfo, PPICTURECREATEPARAM pCreateParam)
{
    BOOL bRet = FALSE;
    HANDLE hFile = CreateFile(pFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    HANSIZE nFileSize;

    if (INVALID_HANDLE_VALUE != hFile)
    {
        nFileSize = (HANSIZE)GetFileSize(hFile, NULL);
        pCreateParam->pData = (uint8_t*)HANWinHeapAlloc(pctInfo->hHeap, NULL, nFileSize);
        if (NULL != pCreateParam->pData)
        {
            if (TRUE == ReadFile(hFile, pCreateParam->pData, (DWORD)nFileSize, NULL, NULL))
            {
                HAN_strcpy(pCreateParam->pFileName, pFileName);
                pCreateParam->nLen = nFileSize;
                bRet = TRUE;
            }
        }
        CloseHandle(hFile);
    }

    return bRet;
}
static PICTURETYPE TryOtherPictureType(PPICTURECREATEPARAM pCreateParam)
{
    PICTURETYPE eRet = PICTURE_TYPE_CNT;

    for (PICTURETYPE iLoop = 0; iLoop < PICTURE_TYPE_CNT; iLoop++)
    {
        if (TRUE == sg_pPictureInfo[iLoop].CheckPictureType(pCreateParam->pData, pCreateParam->nLen))
        {
            eRet = iLoop;
            break;
        }
    }

    return eRet;
}
static BOOL ConfirmOpenNewType(PPICTUREWNDEXTRA pctInfo, PICTURETYPE pctType)
{
    BOOL bRet = FALSE;
    HANCHAR pText[PICTURE_TEXT_BUF_SIZE];

    HAN_snprintf(pText, PICTURE_TEXT_BUF_SIZE, TEXT("检测到图片扩展名与内容不符，可能是“%s”格式，是否以该格式打开图片？"), sg_pPictureInfo[pctType].pTypeName);
    pText[PICTURE_TEXT_BUF_SIZE - 1] = TEXT('\0');
    if (IDYES == MessageBox(pctInfo->hSelf, pText, TEXT("扩展名和文件内容不符"), MB_YESNO)) { bRet = TRUE; }

    return bRet;
}
static void FileDataFormatErrorCallback(PPICTUREWNDEXTRA pctInfo)
{
    MessageBox(pctInfo->hSelf, TEXT("文件格式错误"), TEXT("打开文件失败"), 0);
}
static BOOL AppendPictureTabWindow(PPICTUREWNDEXTRA pctInfo, HWND hPicture, PPICTURECREATEPARAM pCreateParam)
{
    BOOL bRet = TRUE;
    TCITEM tiItem;
    INT nItemCnt;
    HANCHAR pText[PICTURE_TAB_TEXT_SIZE];
    RECT rcItem;
    RECT rcClosePos;
    RECT rcPictureSize;
    PPICTURETABPARAM pParam = (PPICTURETABPARAM)HANWinHeapAlloc(pctInfo->hHeap, NULL, sizeof(PICTURETABPARAM));

    if (NULL != pParam)
    {
        pParam->createParam = *pCreateParam;
        pParam->hPicture = hPicture;
        /* 新窗口一定显示，隐藏所有窗口 */
        tiItem.mask = TCIF_PARAM;
        nItemCnt = TabCtrl_GetItemCount(pctInfo->hTab);
        for (INT iLoop = 0; iLoop < nItemCnt; iLoop++)
        {
            TabCtrl_GetItem(pctInfo->hTab, iLoop, &tiItem);
            ShowWindow(((PPICTURETABPARAM)(tiItem.lParam))->hPicture, SW_HIDE);
        }
        /* 生成标签，并在尾部添加空格给关闭按钮留出空间 */
        HAN_strncpy(pText, PathFindFileName(pCreateParam->pFileName), PICTURE_TAB_TEXT_SIZE - HAN_strlen(PICTURE_TAB_CAT_STR));
        pText[PICTURE_TAB_TEXT_SIZE - 1] = TEXT('\0');
        HAN_strcat(pText, PICTURE_TAB_CAT_STR);
        /* 插入标签 */
        tiItem.mask = TCIF_TEXT | TCIF_PARAM;
        tiItem.pszText = pText;
        tiItem.lParam = (LPARAM)pParam;
        TabCtrl_InsertItem(pctInfo->hTab, nItemCnt, &tiItem);
        /* 添加关闭按钮 */
        TabCtrl_GetItemRect(pctInfo->hTab, nItemCnt, &rcItem);
        GetCloseButtonPos(&rcItem, &rcClosePos);
        pParam->hClose =  CreateWindow(TEXT("button"), TEXT("×"),
            WS_CHILD | WS_VISIBLE,
            rcClosePos.left, rcClosePos.top, GetRectW(&rcClosePos), GetRectH(&rcClosePos),
            pctInfo->hTab, (HMENU)WID_PICTURE_TAB_CLOSE_BUTTON, pctInfo->hInst, NULL
        );
        SendMessage(pParam->hClose, WM_SETFONT, (WPARAM)(pctInfo->hFont.hTab), (LPARAM)TRUE);
        /* 设置选中项 */
        TabCtrl_SetCurSel(pctInfo->hTab, nItemCnt);
        UpdateCloseButtonPos(pctInfo);
        /* 调整窗口位置 */
        GetClientRect(pctInfo->hTab, &rcPictureSize);
        TabCtrl_AdjustRect(pctInfo->hTab, FALSE, &rcPictureSize);
        MoveWindow(hPicture, rcPictureSize.left, rcPictureSize.top, GetRectW(&rcPictureSize), GetRectH(&rcPictureSize), TRUE);
        /* 更新工具栏的保存按钮状态 */
        SendMessage(pctInfo->toolbar.hToolbar, TB_SETSTATE, WID_PICTURE_TOOLBAR_SAVE, TBSTATE_ENABLED);
    }
    else { bRet = FALSE; }
    
    return bRet;
}
static void ClosePicture(PPICTUREWNDEXTRA pctInfo, HWND hClose)
{
    INT nItemCnt = TabCtrl_GetItemCount(pctInfo->hTab);
    INT nCurrentId;
    BOOL bUpdateShowWindow = FALSE;
    TCITEM tiItem = { .mask = TCIF_PARAM, };
    PPICTURETABPARAM pParam;
    HWND hCurrentWindow;
    /* 获取当前选中项 */
    nCurrentId = TabCtrl_GetCurSel(pctInfo->hTab);
    TabCtrl_GetItem(pctInfo->hTab, nCurrentId, &tiItem);
    pParam = (PPICTURETABPARAM)(tiItem.lParam);
    if (NULL != pParam)
    {
        hCurrentWindow = pParam->hPicture;

        for (INT iLoop = 0; iLoop < nItemCnt; iLoop++)
        {
            TabCtrl_GetItem(pctInfo->hTab, iLoop, &tiItem);
            pParam = (PPICTURETABPARAM)(tiItem.lParam);
            if (hClose == pParam->hClose)
            {
                if (iLoop == nCurrentId) { bUpdateShowWindow = TRUE; }
                DestroyWindow(pParam->hPicture);
                DestroyWindow(pParam->hClose);
                HANWinHeapFree(pctInfo->hHeap, 0, pParam->createParam.pData);
                HANWinHeapFree(pctInfo->hHeap, 0, pParam);
                TabCtrl_DeleteItem(pctInfo->hTab, iLoop);
                /* 更新工具栏的保存按钮状态 */
                if (0 == TabCtrl_GetItemCount(pctInfo->hTab))
                {
                    SendMessage(pctInfo->toolbar.hToolbar, TB_SETSTATE, WID_PICTURE_TOOLBAR_SAVE, 0);
                }
                break;
            }
        }

        if (TRUE == bUpdateShowWindow)
        {
            nItemCnt--;
            if (nCurrentId == nItemCnt) { nCurrentId--; }
            if (0 <= nCurrentId)
            {
                for (INT iLoop = 0; iLoop < nItemCnt; iLoop++)
                {
                    TabCtrl_GetItem(pctInfo->hTab, iLoop, &tiItem);
                    pParam = (PPICTURETABPARAM)(tiItem.lParam);
                    if (iLoop == nCurrentId) { ShowWindow(pParam->hPicture, SW_SHOW); }
                    else { ShowWindow(pParam->hPicture, SW_HIDE); }
                }
                TabCtrl_SetCurSel(pctInfo->hTab, nCurrentId);
            }
        }
        else { RedrawWindow(hCurrentWindow, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }

        UpdateCloseButtonPos(pctInfo);
    }
}
static void GetCloseButtonPos(const RECT* pItemPos, RECT* pClosePos)
{
    pClosePos->left = pItemPos->right - 18;
    pClosePos->top = pItemPos->top + 3;
    pClosePos->right = pClosePos->left + PICTURE_TAB_CLOSE_SIZE - 1;
    pClosePos->bottom = pClosePos->top + PICTURE_TAB_CLOSE_SIZE;
}
static void UpdateCloseButtonPos(PPICTUREWNDEXTRA pctInfo)
{
    /* 添加新图片、关闭图片、标签栏翻页的时候都会触发这个函数，会将所有关闭按钮的位置更新一遍并重绘标签栏
     * 这个方法在标签栏翻页的时候会有界面更新不同步的问题，暂时没想到什么好办法
     * 如果 TabControl 控件用 TCS_OWNERDRAWFIXED 自己绘制的话可能会好点
     * 但自己画关闭按钮太麻烦了，还要捕获选项通知和鼠标动作。所以直接在 TabControl 上方给每个标签创建按钮子窗口，在更新的时候移动窗口
     */
    INT nItemCnt = TabCtrl_GetItemCount(pctInfo->hTab);
    TCITEM tiItem = { .mask = TCIF_PARAM, };
    PPICTURETABPARAM pParam;
    RECT rcItem;
    RECT rcClose;
    RECT rcTab;
    RECT rcDisplay;
    RECT rcHeader;

    for (INT iLoop = 0; iLoop < nItemCnt; iLoop++)
    {
        TabCtrl_GetItemRect(pctInfo->hTab, iLoop, &rcItem);
        TabCtrl_GetItem(pctInfo->hTab, iLoop, &tiItem);
        pParam = (PPICTURETABPARAM)(tiItem.lParam);
        if (NULL != pParam)
        {
            GetCloseButtonPos(&rcItem, &rcClose);
            MoveWindow(pParam->hClose, rcClose.left, rcClose.top, GetRectW(&rcClose), GetRectH(&rcClose), TRUE);
        }
    }

    GetClientRect(pctInfo->hTab, &rcTab);
    rcDisplay = rcTab;
    rcHeader = rcTab;
    TabCtrl_AdjustRect(pctInfo->hTab, FALSE, &rcDisplay);
    rcHeader.bottom = rcDisplay.top;
    RedrawWindow(pctInfo->hTab, &rcHeader, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}
static PICTURETYPE GetPictureClassType(HANPCSTR pClass)
{
    PICTURETYPE tRet = PICTURE_TYPE_CNT;

    for (PICTURETYPE iLoop = 0; iLoop < PICTURE_TYPE_CNT; iLoop++)
    {
        if (!HAN_stricmp(pClass, sg_pPictureInfo[iLoop].pPictureClass))
        {
            tRet = iLoop;
            break;
        }
    }

    return tRet;
}
static void GenerateFileNameFilter(HANPSTR pFilter, HANSIZE nLen)
{
    HANSIZE nOffset;

    nOffset = 0;
    for (PICTURETYPE iLoop = 0; iLoop < PICTURE_TYPE_CNT; iLoop++)
    {
        HAN_strncpy(&pFilter[nOffset], sg_pPictureInfo[iLoop].pTypeName, nLen - nOffset);
        nOffset += HAN_strlen(&pFilter[nOffset]) + 1;
        for (HANSIZE jLoop = 0; jLoop < sg_pPictureInfo[iLoop].nPathExtensionCnt; jLoop++)
        {
            if (0 < jLoop) { HAN_strncpy(&pFilter[nOffset], TEXT(";*."), nLen - nOffset); }
            else { HAN_strncpy(&pFilter[nOffset], TEXT("*."), nLen - nOffset); }
            nOffset += HAN_strlen(&pFilter[nOffset]);
            HAN_strncpy(&pFilter[nOffset], sg_pPictureInfo[iLoop].pPathExtension[jLoop], nLen - nOffset);
            nOffset += HAN_strlen(&pFilter[nOffset]);
        }
        nOffset++;
    }
    pFilter[nOffset] = TEXT('\0');
}
static PICTURETYPE UpdateSaveFileType(HANPSTR pFileName, PICTURETYPE pctType)
{
    PICTURETYPE tRet = GetPictureFileType(pFileName);

    if (PICTURE_TYPE_CNT <= tRet)
    {
        HAN_strcat(pFileName, TEXT("."));
        HAN_strcat(pFileName, sg_pPictureInfo[pctType].pPathExtension[0]);
        tRet = pctType;
    }

    return tRet;
}
