#include <float.h>

#include "HAN_ComTool.h"
#include "ComToolModule\GlobalVariables\HAN_ComToolGlobalVariables.h"
#include "ComToolModule\Console\HAN_ComToolConsole.h"
#include "ComToolModule\Graphic\HAN_ComToolGraphic.h"
#include "..\..\HAN_Lib\HAN_wingdi.h"
#include "..\..\HAN_Lib\HAN_Data.h"

static LRESULT CALLBACK ComToolWndProc(HWND hComTool, UINT message, WPARAM wParam, LPARAM lParam);

static LRESULT CreateCallback(HWND hComTool, LPARAM lParam);
static void SizeCallback(PCOMTOOLEXTRA ctInfo);
static void DestroyCallback(PCOMTOOLEXTRA ctInfo);

void RegisterHANComTool(HINSTANCE hInst)
{
    WNDCLASSEX wcex = {
        .cbSize         = sizeof(WNDCLASSEX),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = ComToolWndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = sizeof(PCOMTOOLEXTRA),
        .hInstance      = hInst,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName   = NULL,
        .lpszClassName  = HAN_COM_TOOL_CLASS,
        .hIconSm        = NULL,
    };
    RegisterClassEx(&wcex);
}

void ReadComToolIniFile(HANPCSTR pIniPath, void* pParam)
{
    ReadComToolConsoleIniFile(pIniPath, pParam);
}

void WriteComToolIniFile(HANPCSTR pIniPath, HWND hComTool)
{
    WriteComToolConsoleIniFile(pIniPath, hComTool);
}

static LRESULT CALLBACK ComToolWndProc(HWND hComTool, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // ¶ÁÈ¡ÊôÐÔ
    PCOMTOOLEXTRA ctInfo = (PCOMTOOLEXTRA)GetWindowLongPtr(hComTool, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = CreateCallback(hComTool, lParam);
        } break;
        case WM_SIZE: {
            SizeCallback(ctInfo);
        } break;
        case WM_CTLCOLORSTATIC: {
            lWndProcRet = (INT_PTR)GetStockObject(WHITE_BRUSH);
        } break;
        case WM_DESTROY: {
            DestroyCallback(ctInfo);
            lWndProcRet = DefWindowProc(hComTool, message, wParam, lParam);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hComTool, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}

static LRESULT CreateCallback(HWND hComTool, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PCOMTOOLEXTRA ctInfo;
    HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
    PCOMTOOLCFG pCtConfig = ((LPCREATESTRUCT)lParam)->lpCreateParams;
    RECT rcClientSize;

    HANDLE hHeap = GetProcessHeap();
    if (NULL == hHeap) { lWndProcRet = -1; }
    if (-1 != lWndProcRet)
    {
        ctInfo = (PCOMTOOLEXTRA)HANWinHeapAlloc(hHeap, NULL, sizeof(COMTOOLEXTRA));
        if (NULL == ctInfo) { lWndProcRet = -1; }
    }

    if (-1 != lWndProcRet)
    {
        int nWinX = COMTOOL_WINDOW_DX;
        int nWinY = COMTOOL_WINDOW_DY;

        SetWindowLongPtr(hComTool, 0, (LONG_PTR)ctInfo);

        *ctInfo = g_ftDefaultCfg;
        ctInfo->hHeap = hHeap;
        ctInfo->hInst = hInst;
        ctInfo->hSelf = hComTool;
        ctInfo->graphicClass.listGraphic = HANListInit();
        if (NULL != pCtConfig)
        {
            ctInfo->ctConfig = *pCtConfig;
        }

        GetClientRect(hComTool, &rcClientSize);

        RegisterHANComToolConsole(hInst);
        RegisterHANComToolGraphic(hInst);

        ctInfo->hFont.hHex = CreateFontIndirect(&g_lfHexFont);
        ctInfo->hFont.hSys = CreateFontIndirect(&g_lfSysFont);

        ctInfo->printData.printData.hText = CreateWindow(TEXT("edit"), NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_LEFT | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY,
            nWinX, nWinY, COMTOOL_TEXT_WINDOW_MIN_W, COMTOOL_TEXT_WINDOW_MIN_H,
            hComTool, (HMENU)WID_COM_TOOL_TEXT, hInst, NULL);
        
        nWinX = COMTOOL_WINDOW_DX;
        nWinY += COMTOOL_TEXT_WINDOW_MIN_H + COMTOOL_WINDOW_DY;
        ctInfo->hConsole = CreateComToolConsoleWindow(nWinX, nWinY, hComTool, (HMENU)WID_COM_TOOL_CONSOLE, hInst, ctInfo);
        nWinX += COMTOOL_CONSOLE_WINDOW_W + COMTOOL_WINDOW_DX;
        ctInfo->hGraphic = CreateComToolGraphicWindow(nWinX, nWinY, hComTool, (HMENU)WID_COM_TOOL_GRAPHIC, hInst, ctInfo);

        SendMessage(ctInfo->printData.printData.hText, WM_SETFONT, (WPARAM)(ctInfo->hFont.hHex), (LPARAM)TRUE);

        EditSetLimitText(ctInfo->printData.printData.hText, COMTOOL_TEXT_WINDOW_LIMIT);

        SizeCallback(ctInfo);
    }

    return lWndProcRet;
}
static void SizeCallback(PCOMTOOLEXTRA ctInfo)
{
    RECT rcClientSize;
    HANINT nWinW;
    HANINT nWinH;
    
    GetClientRect(ctInfo->hSelf, &rcClientSize);
    nWinW = GetRectW(&rcClientSize) - (2 * COMTOOL_WINDOW_DX);
    nWinH = GetRectH(&rcClientSize) - (3 * COMTOOL_WINDOW_DY) - COMTOOL_CONSOLE_WINDOW_H;

    if (nWinW < COMTOOL_TEXT_WINDOW_MIN_W) { nWinW = COMTOOL_TEXT_WINDOW_MIN_W; }
    if (nWinH < COMTOOL_TEXT_WINDOW_MIN_H) { nWinH = COMTOOL_TEXT_WINDOW_MIN_H; }

    MoveWindow(ctInfo->printData.printData.hText,
        COMTOOL_WINDOW_DX, COMTOOL_WINDOW_DY,
        nWinW, nWinH,
        TRUE
    );
    MoveWindow(ctInfo->hConsole,
        COMTOOL_WINDOW_DX, nWinH + (2 * COMTOOL_WINDOW_DY),
        COMTOOL_CONSOLE_WINDOW_W, COMTOOL_CONSOLE_WINDOW_H,
        TRUE
    );
    MoveWindow(ctInfo->hGraphic,
        (2 * COMTOOL_WINDOW_DX) + COMTOOL_CONSOLE_WINDOW_W, nWinH + (2 * COMTOOL_WINDOW_DY),
        COMTOOL_GRAPHIC_WINDOW_W, COMTOOL_GRAPHIC_WINDOW_H,
        TRUE
    );
}
static void DestroyCallback(PCOMTOOLEXTRA ctInfo)
{
    DestroyWindow(ctInfo->hConsole);
    DestroyWindow(ctInfo->hGraphic);
    HANWinHeapFree(ctInfo->hHeap, 0, ctInfo);
}
