#include <Windows.h>
#include <shlwapi.h>
#include <CommCtrl.h>

#include "HAN_Video.h"
#include "HAN_VideoDef.h"
#include "VideoModule\MP4\HAN_VideoMP4.h"

typedef enum {
    VIDEO_TOOLBAR_BUTTON_OPEN_VIDEO,
    VIDEO_TOOLBAR_BUTTON_PROCESS_FRAME,
    VIDEO_TOOLBAR_BUTTON_CNT,
} VIDEOTOOLBARBUTTON;

typedef enum {
    VIDEO_TYPE_MP4,
    VIDEO_TYPE_CNT,
} VIDEOTYPE;

typedef enum {
    VIDEO_PROCESS_STATUS_OPEN_FILE,
    VIDEO_PROCESS_STATUS_GET_VIDEO_TYPE,
    VIDEO_PROCESS_STATUS_READ_FILE,
    VIDEO_PROCESS_STATUS_CHECK_VIDEO_TYPE,
    VIDEO_PROCESS_STATUS_TRY_OTHER_VIDEO_TYPE,
    VIDEO_PROCESS_STATUS_PROCESS,
    VIDEO_PROCESS_STATUS_EXIT,
} VIDEOPROCESSSTATUS;

typedef struct tagVIDEOROCESS {
    HANPCSTR                pTypeName;
    HANSIZE                 nPathExtensionCnt;
    const HANPCSTR*         pPathExtension;
    HANPCSTR                pVideoClass;
    BOOL                    (*CheckVideoType)(const uint8_t* pData, HANSIZE nLen);
    void                    (*RegisterHANVideoModule)(HINSTANCE hInst);
} VIDEOPROCESS, * PVIDEOPROCESS;

typedef struct tagVIDEOWNDEXTRA {
    HANDLE                  hHeap;
    HINSTANCE               hInst;
    HWND                    hSelf;
    HWND                    hVideo;
    struct {
        HWND                hToolbar;
        HIMAGELIST          hImageList;
    } toolbar;
    struct {
        HFONT               hHex;
        HFONT               hSys;
    } hFont;                /* 字体 */
} VIDEOWNDEXTRA, * PVIDEOWNDEXTRA;

static LRESULT CALLBACK VideoWndProc(HWND hVideo, UINT message, WPARAM wParam, LPARAM lParam);

static LRESULT CreateCallback(HWND hVideo, LPARAM lParam);
static void CommandCallback(PVIDEOWNDEXTRA vdInfo, WPARAM wParam);
static void DestroyCallback(PVIDEOWNDEXTRA vdInfo);
static void InitToolbar(PVIDEOWNDEXTRA vdInfo);
static void GetSubWindowSize(PVIDEOWNDEXTRA vdInfo, RECT* pSize);
static void OpenFileCallback(PVIDEOWNDEXTRA vdInfo);
static BOOL OpenVideoFile(HANPSTR pFileName, HWND hOwner);
static VIDEOTYPE GetVideoFileType(HANPSTR pFileName);
static BOOL ReadVideoFile(HANPSTR pFileName, PVIDEOWNDEXTRA vdInfo, PVIDEOCREATEPARAM pCreateParam);
static VIDEOTYPE TryOtherVideoType(PVIDEOCREATEPARAM pCreateParam);
static BOOL ConfirmOpenNewType(PVIDEOWNDEXTRA vdInfo, VIDEOTYPE vdType);
static void FileDataFormatErrorCallback(PVIDEOWNDEXTRA vdInfo);

static const VIDEOWNDEXTRA sg_vdDefaultCfg = { 0 };
static const TBBUTTON sg_pToolbarInfo[VIDEO_TOOLBAR_BUTTON_CNT] = {
    [VIDEO_TOOLBAR_BUTTON_OPEN_VIDEO] = {
        .iBitmap = MAKELONG(STD_FILEOPEN, 0),
        .idCommand = WID_VIDEO_TOOLBAR_OPEN_VIDEO,
        .fsState = TBSTATE_ENABLED,
        .fsStyle = BTNS_AUTOSIZE,
        .bReserved = { 0 },
        .dwData = 0,
        .iString = (INT_PTR)TEXT("打开视频"),
    },
    [VIDEO_TOOLBAR_BUTTON_PROCESS_FRAME] = {
        .iBitmap = MAKELONG(STD_PRINTPRE, 0),
        .idCommand = WID_VIDEO_TOOLBAR_PROCESS_FRAME,
        .fsState = TBSTATE_ENABLED,
        .fsStyle = BTNS_AUTOSIZE,
        .bReserved = { 0 },
        .dwData = 0,
        .iString = (INT_PTR)TEXT("逐帧处理"),
    },
};
static const HANPCSTR sg_mp4PathExtension[] = { TEXT("mp4"), };
static const VIDEOPROCESS sg_pVideoInfo[VIDEO_TYPE_CNT] = {
    [VIDEO_TYPE_MP4] = {
        .pTypeName = TEXT("MP4"),
        .nPathExtensionCnt = ArrLen(sg_mp4PathExtension),
        .pPathExtension = sg_mp4PathExtension,
        .pVideoClass = HAN_VIDEO_MP4_CLASS,
        .CheckVideoType = CheckMP4Type,
        .RegisterHANVideoModule = RegisterHANVideoMP4,
    },
};

void RegisterHANVideo(HINSTANCE hInst)
{
    WNDCLASSEX wcex = {
        .cbSize         = sizeof(WNDCLASSEX),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = VideoWndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = sizeof(PVIDEOWNDEXTRA),
        .hInstance      = hInst,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName   = NULL,
        .lpszClassName  = HAN_VIDEO_CLASS,
        .hIconSm        = NULL,
    };
    RegisterClassEx(&wcex);
}

static LRESULT CALLBACK VideoWndProc(HWND hVideo, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    PVIDEOWNDEXTRA vdInfo = (PVIDEOWNDEXTRA)GetWindowLongPtr(hVideo, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = CreateCallback(hVideo, lParam);
        } break;
        case WM_COMMAND: {
            CommandCallback(vdInfo, wParam);
        } break;
        case WM_DESTROY: {
            DestroyCallback(vdInfo);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hVideo, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}

static LRESULT CreateCallback(HWND hVideo, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PVIDEOWNDEXTRA vdInfo;
    HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
    RECT rcClientSize;
    RECT rcToolbarSize;

    // INT nWinX = VIDEO_WINDOW_DX;
    // INT nWinY = VIDEO_WINDOW_DY;
    // INT nWinW;
    // INT nWinH;

    HANDLE hHeap = GetProcessHeap();
    if (NULL == hHeap) { lWndProcRet = -1; }
    if (-1 != lWndProcRet)
    {
        vdInfo = (PVIDEOWNDEXTRA)HANWinHeapAlloc(hHeap, NULL, sizeof(VIDEOWNDEXTRA));
        if (NULL == vdInfo) { lWndProcRet = -1; }
    }

    if (-1 != lWndProcRet)
    {
        SetWindowLongPtr(hVideo, 0, (LONG_PTR)vdInfo);

        *vdInfo = sg_vdDefaultCfg;

        vdInfo->hHeap = hHeap;
        vdInfo->hInst = hInst;
        vdInfo->hSelf = hVideo;

        GetClientRect(hVideo, &rcClientSize);

        for (VIDEOTYPE iLoop = 0; iLoop < VIDEO_TYPE_CNT; iLoop++)
        {
            sg_pVideoInfo[iLoop].RegisterHANVideoModule(hInst);
        }

        vdInfo->hFont.hHex = CreateFontIndirect(&g_lfHexFont);
        vdInfo->hFont.hSys = CreateFontIndirect(&g_lfSysFont);

        vdInfo->toolbar.hToolbar = CreateWindow(TOOLBARCLASSNAME, NULL,
            WS_CHILD | TBSTYLE_LIST | TBSTYLE_TOOLTIPS,
            0, 0, 0, 0,
            hVideo, (HMENU)WID_VIDEO_TOOLBAR, hInst, NULL
        );
        InitToolbar(vdInfo);
        GetClientRect(vdInfo->toolbar.hToolbar, &rcToolbarSize);
    }

    return lWndProcRet;
}
static void CommandCallback(PVIDEOWNDEXTRA vdInfo, WPARAM wParam)
{
    switch (LOWORD(wParam)) {
        case WID_VIDEO_TOOLBAR_OPEN_VIDEO: {
            OpenFileCallback(vdInfo);
        } break;

        default: { } break;
    }
}
static void DestroyCallback(PVIDEOWNDEXTRA vdInfo)
{
    HANWinHeapFree(vdInfo->hHeap, 0, vdInfo);
}
static void InitToolbar(PVIDEOWNDEXTRA vdInfo)
{
    vdInfo->toolbar.hImageList = ImageList_Create(VIDEO_TOOL_SIZE, VIDEO_TOOL_SIZE, ILC_COLOR16 | ILC_MASK, VIDEO_TOOLBAR_BUTTON_CNT, 0);
    SendMessage(vdInfo->toolbar.hToolbar, TB_SETIMAGELIST, (WPARAM)0, (LPARAM)(vdInfo->toolbar.hImageList));
    SendMessage(vdInfo->toolbar.hToolbar, TB_LOADIMAGES, (WPARAM)IDB_STD_SMALL_COLOR, (LPARAM)HINST_COMMCTRL);
    SendMessage(vdInfo->toolbar.hToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
    SendMessage(vdInfo->toolbar.hToolbar, TB_ADDBUTTONS, (WPARAM)VIDEO_TOOLBAR_BUTTON_CNT, (LPARAM)sg_pToolbarInfo);
    SendMessage(vdInfo->toolbar.hToolbar, TB_AUTOSIZE, 0, 0);
    SendMessage(vdInfo->toolbar.hToolbar, TB_SETMAXTEXTROWS, 0, 0);
    ShowWindow(vdInfo->toolbar.hToolbar, SW_SHOW);
}
static void GetSubWindowSize(PVIDEOWNDEXTRA vdInfo, RECT* pSize)
{
    RECT rcToolbarSize;

    GetClientRect(vdInfo->toolbar.hToolbar, &rcToolbarSize);
    GetClientRect(vdInfo->hSelf, pSize);
    pSize->left += VIDEO_WINDOW_DX;
    pSize->top += VIDEO_WINDOW_DY + GetRectH(&rcToolbarSize);
    pSize->right -= VIDEO_WINDOW_DX;
    pSize->bottom -= VIDEO_WINDOW_DY;
}
static void OpenFileCallback(PVIDEOWNDEXTRA vdInfo)
{
    VIDEOPROCESSSTATUS eStatus = VIDEO_PROCESS_STATUS_OPEN_FILE;
    HANCHAR pFileName[PATH_STR_SIZE] = { 0 };
    VIDEOTYPE vdType;
    VIDEOTYPE vdRetryType;
    BOOL bReadFile;
    HWND hVideo;
    RECT rcVideoSize;
    BOOL bOk = FALSE;
    VIDEOCREATEPARAM vdCreateParam = {
        .pData = NULL,
    };

    while (VIDEO_PROCESS_STATUS_EXIT != eStatus)
    {
        switch (eStatus) {
            case VIDEO_PROCESS_STATUS_OPEN_FILE: {              // 选择文件
                if (TRUE == OpenVideoFile(pFileName, vdInfo->hSelf)) { eStatus = VIDEO_PROCESS_STATUS_GET_VIDEO_TYPE; }
                else { eStatus = VIDEO_PROCESS_STATUS_EXIT; }
            } break;
            case VIDEO_PROCESS_STATUS_GET_VIDEO_TYPE: {         // 根据文件拓展名查找文件类型
                vdType = GetVideoFileType(pFileName);
                if (vdType < VIDEO_TYPE_CNT) { eStatus = VIDEO_PROCESS_STATUS_READ_FILE; }
                else { eStatus = VIDEO_PROCESS_STATUS_EXIT; }
            } break;
            case VIDEO_PROCESS_STATUS_READ_FILE: {              // 读取文件
                bReadFile = ReadVideoFile(pFileName, vdInfo, &vdCreateParam);
                if (TRUE == bReadFile) { eStatus = VIDEO_PROCESS_STATUS_CHECK_VIDEO_TYPE; }
                else { eStatus = VIDEO_PROCESS_STATUS_EXIT; }
            } break;
            case VIDEO_PROCESS_STATUS_CHECK_VIDEO_TYPE: {       // 检查文件类型
                if (TRUE == sg_pVideoInfo[vdType].CheckVideoType(vdCreateParam.pData, vdCreateParam.nLen)) { eStatus = VIDEO_PROCESS_STATUS_PROCESS; }
                else { eStatus = VIDEO_PROCESS_STATUS_TRY_OTHER_VIDEO_TYPE; }
            } break;
            case VIDEO_PROCESS_STATUS_TRY_OTHER_VIDEO_TYPE: {   // 文件类型检查失败时，尝试其它类型
                vdRetryType = TryOtherVideoType(&vdCreateParam);
                if (vdRetryType < VIDEO_TYPE_CNT)
                {
                    vdType = vdRetryType;
                    if (TRUE == ConfirmOpenNewType(vdInfo, vdRetryType)) { eStatus = VIDEO_PROCESS_STATUS_PROCESS; }
                    else { eStatus = VIDEO_PROCESS_STATUS_EXIT; }
                }
                else
                {
                    FileDataFormatErrorCallback(vdInfo);
                    eStatus = VIDEO_PROCESS_STATUS_EXIT;
                }
            } break;
            case VIDEO_PROCESS_STATUS_PROCESS: {                // 处理视频
                GetSubWindowSize(vdInfo, &rcVideoSize);

                hVideo = CreateWindow(
                    sg_pVideoInfo[vdType].pVideoClass, PathFindFileName(vdCreateParam.pFileName),
                    WS_CHILD | WS_VISIBLE,
                    rcVideoSize.left, rcVideoSize.top, GetRectW(&rcVideoSize), GetRectH(&rcVideoSize),
                    vdInfo->hSelf, (HMENU)WID_VIDEO_VIDEO_WINDOW, vdInfo->hInst, &vdCreateParam
                );
                
                if (NULL != hVideo)
                {
                    if (NULL != vdInfo->hVideo) { DestroyWindow(vdInfo->hVideo); }
                    vdInfo->hVideo = hVideo;
                    bOk = TRUE;
                }
                else { FileDataFormatErrorCallback(vdInfo); }

                eStatus = VIDEO_PROCESS_STATUS_EXIT;
            } break;

            default: { } break;
        }
    }

    if ((FALSE == bOk) && (NULL != vdCreateParam.pData)) { HANWinHeapFree(vdInfo->hHeap, 0, vdCreateParam.pData); }
}
static BOOL OpenVideoFile(HANPSTR pFileName, HWND hOwner)
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
static VIDEOTYPE GetVideoFileType(HANPSTR pFileName)
{
    VIDEOTYPE tRet = VIDEO_TYPE_CNT;
    HANPCSTR pPathExtension = PathFindExtension(pFileName);
    HANSIZE nPathExtensionCnt;

    if (NULL != pPathExtension)
    {
        for (VIDEOTYPE iLoop = 0; iLoop < VIDEO_TYPE_CNT; iLoop++)
        {
            nPathExtensionCnt = sg_pVideoInfo[iLoop].nPathExtensionCnt;
            for (HANSIZE jLoop = 0; jLoop < nPathExtensionCnt; jLoop++)
            {
                if (!HAN_stricmp(&pPathExtension[1], sg_pVideoInfo[iLoop].pPathExtension[jLoop]))
                {
                    tRet = iLoop;
                    break;
                }
            }
        }
    }   

    return tRet;
}
static BOOL ReadVideoFile(HANPSTR pFileName, PVIDEOWNDEXTRA vdInfo, PVIDEOCREATEPARAM pCreateParam)
{
    BOOL bRet = FALSE;
    HANDLE hFile = CreateFile(pFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    HANSIZE nFileSize;

    if (INVALID_HANDLE_VALUE != hFile)
    {
        nFileSize = (HANSIZE)GetFileSize(hFile, NULL);
        pCreateParam->pData = (uint8_t*)HANWinHeapAlloc(vdInfo->hHeap, NULL, nFileSize);
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
static VIDEOTYPE TryOtherVideoType(PVIDEOCREATEPARAM pCreateParam)
{
    VIDEOTYPE eRet = VIDEO_TYPE_CNT;

    for (VIDEOTYPE iLoop = 0; iLoop < VIDEO_TYPE_CNT; iLoop++)
    {
        if (TRUE == sg_pVideoInfo[iLoop].CheckVideoType(pCreateParam->pData, pCreateParam->nLen))
        {
            eRet = iLoop;
            break;
        }
    }

    return eRet;
}
static BOOL ConfirmOpenNewType(PVIDEOWNDEXTRA vdInfo, VIDEOTYPE vdType)
{
    BOOL bRet = FALSE;
    HANCHAR pText[VIDEO_TEXT_BUF_SIZE];

    HAN_snprintf(pText, VIDEO_TEXT_BUF_SIZE, TEXT("检测到视频扩展名与内容不符，可能是“%s”格式，是否以该格式打开视频？"), sg_pVideoInfo[vdType].pTypeName);
    pText[VIDEO_TEXT_BUF_SIZE - 1] = TEXT('\0');
    if (IDYES == MessageBox(vdInfo->hSelf, pText, TEXT("扩展名和文件内容不符"), MB_YESNO)) { bRet = TRUE; }

    return bRet;
}
static void FileDataFormatErrorCallback(PVIDEOWNDEXTRA vdInfo)
{
    MessageBox(vdInfo->hSelf, TEXT("文件格式错误"), TEXT("打开文件失败"), 0);
}
