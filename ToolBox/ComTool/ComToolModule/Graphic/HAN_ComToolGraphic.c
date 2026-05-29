#include "HAN_ComToolGraphic.h"
#include "HAN_ComToolGraphicMain.h"
#include "..\..\..\GlobalVariables.h"
#include "..\..\..\..\HAN_Lib\HAN_Data.h"

#define HAN_COM_TOOL_GRAPHIC_CLASS          TEXT("HAN_ComToolGraphic")

static LRESULT CALLBACK ComToolGraphicWndProc(HWND hComToolGraphic, UINT message, WPARAM wParam, LPARAM lParam);

static LRESULT CreateCallback(HWND hWnd, LPARAM lParam);
static void CommandCallback(HWND hComToolGraphic, PCOMTOOLEXTRA ctInfo, WPARAM wParam, LPARAM lParam);
static void DestroyCallback(PCOMTOOLEXTRA ctInfo);
static void ImportDllAction(PCOMTOOLEXTRA ctInfo);
static COMTOOLGRAPHICERRNO LoadDllFile(PCOMTOOLEXTRA ctInfo, HANPCSTR pDllPath);
static void LoadDllOkAction(PCOMTOOLEXTRA ctInfo, HANPCSTR pDllPath);
static void GraphicButtonCallback(PCOMTOOLEXTRA ctInfo);

static void* AllocGraphic(void* pOldData, size_t nNewSize, void* pParam);

void RegisterHANComToolGraphic(HINSTANCE hInst)
{
    WNDCLASSEX wcex = {
        .cbSize         = sizeof(WNDCLASSEX),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = ComToolGraphicWndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = sizeof(PCOMTOOLEXTRA),
        .hInstance      = hInst,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName   = NULL,
        .lpszClassName  = HAN_COM_TOOL_GRAPHIC_CLASS,
        .hIconSm        = NULL,
    };
    RegisterClassEx(&wcex);
}

HWND CreateComToolGraphicWindow(HANINT x, HANINT y, HWND hComTool, HMENU winId, HINSTANCE hInst, PCOMTOOLEXTRA ctInfo)
{
    return CreateWindow(HAN_COM_TOOL_GRAPHIC_CLASS, NULL,
        WS_CHILD | WS_VISIBLE | WS_BORDER, x, y, COMTOOL_GRAPHIC_WINDOW_W, COMTOOL_GRAPHIC_WINDOW_H,
        hComTool, winId, hInst, ctInfo
    );
}

void HANComToolGraphicGetDataOperation(PCOMTOOLEXTRA ctInfo)
{
    HWND hGraphic;
    uint8_t nIndex = 1 - ctInfo->revData.nIndex;
    uint8_t* pBuf = ctInfo->revData.pBuf[nIndex].pData.pData;
    COMTOOLSIZE nLen = ctInfo->revData.pBuf[nIndex].pData.nLen;

    HANListReForData(&ctInfo->graphicClass.listGraphic);
    hGraphic = HANListForData(&ctInfo->graphicClass.listGraphic);
    while (NULL != hGraphic)
    {
        HANComToolGraphicUpdateMsg(hGraphic, pBuf, nLen);
        hGraphic = HANListForData(&ctInfo->graphicClass.listGraphic);
    }
}

static LRESULT CALLBACK ComToolGraphicWndProc(HWND hComToolGraphic, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    PCOMTOOLEXTRA ctInfo = (PCOMTOOLEXTRA)GetWindowLongPtr(hComToolGraphic, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = CreateCallback(hComToolGraphic, lParam);
        } break;
        case WM_COMMAND: {
            CommandCallback(hComToolGraphic, ctInfo, wParam, lParam);
        } break;
        case WM_CTLCOLORSTATIC: {
            lWndProcRet = (INT_PTR)GetStockObject(WHITE_BRUSH);
        } break;
        case WM_DESTROY: {
            DestroyCallback(ctInfo);
            lWndProcRet = DefWindowProc(hComToolGraphic, message, wParam, lParam);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hComToolGraphic, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}

static LRESULT CreateCallback(HWND hComToolGraphic, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PCOMTOOLEXTRA ctInfo = ((LPCREATESTRUCT)lParam)->lpCreateParams;
    RECT rcClientSize;

    int nWinX = COMTOOL_WINDOW_DX;
    int nWinY = COMTOOL_WINDOW_DY;

    SetWindowLongPtr(hComToolGraphic, 0, (LONG_PTR)ctInfo);

    GetClientRect(hComToolGraphic, &rcClientSize);

    RegisterHANComToolGraphicMain(ctInfo->hInst);

    ctInfo->graphicClass.dllConsole.hDllPath = CreateWindow(TEXT("edit"), NULL,
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL | ES_READONLY, nWinX, nWinY,
        TOOL_BOX_PATH_INPUT_EDIT_W, TOOL_BOX_TEXT_INPUT_EDIT_H,
        hComToolGraphic, (HMENU)WID_COM_TOOL_GRAPHIC_DLL_PATH_INPUT, ctInfo->hInst, NULL);
    nWinX += TOOL_BOX_PATH_INPUT_EDIT_W + COMTOOL_WINDOW_DX;
    ctInfo->graphicClass.dllConsole.hImportDll = CreateWindow(TEXT("button"), TEXT("导入DLL文件"),
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, nWinX, nWinY, 120, TOOL_BOX_TEXT_INPUT_EDIT_H,
        hComToolGraphic, (HMENU)WID_COM_TOOL_GRAPHIC_DLL_PATH_BUTTON, ctInfo->hInst, NULL);
    nWinX = COMTOOL_WINDOW_DX;
    nWinY += TOOL_BOX_TEXT_INPUT_EDIT_H + COMTOOL_WINDOW_DY;
    ctInfo->graphicClass.hGraphicButton = CreateWindow(TEXT("button"), TEXT("绘图"),
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, nWinX, nWinY, 120, TOOL_BOX_TEXT_INPUT_EDIT_H,
        hComToolGraphic, (HMENU)WID_COM_TOOL_GRAPHIC_BUTTON, ctInfo->hInst, NULL);

    SendMessage(ctInfo->graphicClass.dllConsole.hDllPath, WM_SETFONT, (WPARAM)(ctInfo->hFont.hSys), (LPARAM)TRUE);
    SendMessage(ctInfo->graphicClass.dllConsole.hImportDll, WM_SETFONT, (WPARAM)(ctInfo->hFont.hSys), (LPARAM)TRUE);
    SendMessage(ctInfo->graphicClass.hGraphicButton, WM_SETFONT, (WPARAM)(ctInfo->hFont.hSys), (LPARAM)TRUE);
    
    return lWndProcRet;
}
static void CommandCallback(HWND hComToolGraphic, PCOMTOOLEXTRA ctInfo, WPARAM wParam, LPARAM lParam)
{
    (void)hComToolGraphic;
    (void)lParam;
    
    switch (LOWORD(wParam)) {
        case WID_COM_TOOL_GRAPHIC_DLL_PATH_BUTTON: {
            ImportDllAction(ctInfo);
        } break;
        case WID_COM_TOOL_GRAPHIC_BUTTON: {
            GraphicButtonCallback(ctInfo);
        } break;

        default: {
            break;
        }
    }
}
static void DestroyCallback(PCOMTOOLEXTRA ctInfo)
{
    while (0 < HANListGetLen(&(ctInfo->graphicClass.listGraphic)))
    {
        DestroyWindow(HANListGetData(&(ctInfo->graphicClass.listGraphic), 0));
    }
}
static void ImportDllAction(PCOMTOOLEXTRA ctInfo)
{
    HANCHAR pInputName[PATH_STR_SIZE] = { 0 };
    OPENFILENAME ofnOpenFile;
    ZeroMemory(&ofnOpenFile, sizeof(ofnOpenFile));
    ofnOpenFile.lStructSize = sizeof(ofnOpenFile);
    ofnOpenFile.hwndOwner = NULL;
    ofnOpenFile.lpstrFilter = TEXT("DLL\0*.DLL\0\0");
    ofnOpenFile.nFilterIndex = 0;
    ofnOpenFile.lpstrFile = pInputName;
    ofnOpenFile.nMaxFile = PATH_STR_SIZE;
    ofnOpenFile.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
    if (GetOpenFileName(&ofnOpenFile))
    {
        switch(LoadDllFile(ctInfo, pInputName)) {
            case COMTOOLGRAPHIC_ERRNO_OK: {
                LoadDllOkAction(ctInfo, pInputName);
                break;
            }
            case COMTOOLGRAPHIC_ERRNO_OPEN_DLL_FAIL: {
                MessageBox(NULL, TEXT("打开DLL文件失败"), NULL, 0);
                break;
            }
            case COMTOOLGRAPHIC_ERRNO_NOT_COMPATIBLE: {
                MessageBox(NULL, TEXT("DLL文件不兼容"), NULL, 0);
                break;
            }
            
            default: {
                break;
            }
        }
    }
}
static COMTOOLGRAPHICERRNO LoadDllFile(PCOMTOOLEXTRA ctInfo, HANPCSTR pDllPath)
{
    COMTOOLGRAPHICERRNO eRet = COMTOOLGRAPHIC_ERRNO_OK;
    HINSTANCE hDll = LoadLibrary(pDllPath);
    
    if (NULL == hDll) { eRet = COMTOOLGRAPHIC_ERRNO_OPEN_DLL_FAIL; }
    else
    {
        BOOL bGet = TRUE;
        PCOMTOOLGRAPHICMSGINFO pComToolMsgInfo = (void*)GetProcAddress(hDll, "g_pComToolGraphicInfo");
        PCOMTOOLGRAPHICMSG pComToolMsg;

        if (NULL == pComToolMsgInfo) { bGet = FALSE; }

        if (TRUE == bGet)
        {
            pComToolMsg = pComToolMsgInfo->pMsg;
            if (NULL == pComToolMsg) { bGet = FALSE; }
        }
        else
        {
            eRet = COMTOOLGRAPHIC_ERRNO_NOT_COMPATIBLE;
            FreeLibrary(hDll);
        }
        
        if (TRUE == bGet)
        {
            for (uint32_t iLoop = 0; iLoop < (pComToolMsgInfo->nMsgCnt); iLoop++)
            {
                if (NULL == pComToolMsg[iLoop].ReadMessage) { bGet = FALSE; }
            }
            if (TRUE == bGet)
            {
                if (NULL != ctInfo->graphicClass.dllConsole.hDll) { FreeLibrary(ctInfo->graphicClass.dllConsole.hDll); }
                ctInfo->graphicClass.dllConsole.hDll = hDll;
                ctInfo->graphicClass.pMsgInfo = pComToolMsgInfo;
            }
            else
            {
                eRet = COMTOOLGRAPHIC_ERRNO_NOT_COMPATIBLE;
                FreeLibrary(hDll);
            }
        }
    }

    return eRet;
}
static void LoadDllOkAction(PCOMTOOLEXTRA ctInfo, HANPCSTR pDllPath)
{
    SetWindowText(ctInfo->graphicClass.dllConsole.hDllPath, pDllPath);
}
static void GraphicButtonCallback(PCOMTOOLEXTRA ctInfo)
{
    BOOL bOK = TRUE;
    HANALLOCDATA adAllocGraphic = {
        .AllocOperation = AllocGraphic,
        .pParam = ctInfo->hHeap,
    };

    if (NULL != ctInfo->graphicClass.pMsgInfo)
    {
        HWND hGraphic = CreateWindow(HAN_COM_TOOL_GRAPHIC_MAIN_CLASS, TEXT("绘图"),
            WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 1000, 500, NULL, NULL, ctInfo->hInst, ctInfo);
        if (NULL != hGraphic)
        {
            if (RET_OK != HANListAppend(&(ctInfo->graphicClass.listGraphic), hGraphic, &adAllocGraphic))
            {
                bOK = FALSE;
                DestroyWindow(hGraphic);
            }
        }
        else { bOK = FALSE; }
        if (FALSE == bOK)
        {
            MessageBox(ctInfo->hSelf, TEXT("创建绘图窗口失败"), NULL, 0);
        }
    }
    else
    {
        MessageBox(ctInfo->hSelf, TEXT("请导入DLL文件"), NULL, 0);
    }
}

static void* AllocGraphic(void* pOldData, size_t nNewSize, void* pParam)
{
    return HANWinHeapAlloc(pParam, pOldData, nNewSize);
}
