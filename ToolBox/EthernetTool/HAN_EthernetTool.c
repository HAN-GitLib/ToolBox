#include "HAN_EthernetTool.h"
#include "EthernetToolModule\GlobalVariables\HAN_EthernetToolGlobalVariables.h"
#include "EthernetToolModule\Console\HAN_EthernetToolConsole.h"
#include "EthernetToolModule\Http\HAN_EthernetToolHttp.h"
#include "EthernetToolModule\SomeIP\HAN_EthernetToolSomeIP.h"
#include "EthernetToolModule\SomeIP\HAN_EthernetToolSomeIPClient.h"
#include "EthernetToolModule\SomeIP\HAN_EthernetToolSomeIPServer.h"

static LRESULT CALLBACK EthernetToolWndProc(HWND hEthernetTool, UINT message, WPARAM wParam, LPARAM lParam);

static LRESULT CreateCallback(HWND hEthernetTool, LPARAM lParam);
static void SizeCallback(PETHERNETTOOLEXTRA etInfo);
static void CommandCallback(PETHERNETTOOLEXTRA etInfo, WPARAM wParam);
static void DestroyCallback(PETHERNETTOOLEXTRA etInfo);

void RegisterEthernetTool(HINSTANCE hInst)
{
    WNDCLASSEX wcex = {
        .cbSize         = sizeof(WNDCLASSEX),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = EthernetToolWndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = sizeof(PETHERNETTOOLEXTRA),
        .hInstance      = hInst,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName   = NULL,
        .lpszClassName  = HAN_ETHERNET_TOOL_CLASS,
        .hIconSm        = NULL,
    };
    RegisterClassEx(&wcex);
}

void ReadEthernetToolIniFile(HANPCSTR pIniPath, void* pParam)
{
    ReadEthernetToolConsoleIniFile(pIniPath, pParam);
}

void WriteEthernetToolIniFile(HANPCSTR pIniPath, HWND hEthernetTool)
{
    WriteEthernetToolConsoleIniFile(pIniPath, hEthernetTool);
}

static LRESULT CALLBACK EthernetToolWndProc(HWND hEthernetTool, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    PETHERNETTOOLEXTRA etInfo = (PETHERNETTOOLEXTRA)GetWindowLongPtr(hEthernetTool, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = CreateCallback(hEthernetTool, lParam);
        } break;
        case WM_SIZE: {
            SizeCallback(etInfo);
        } break;
        case WM_CTLCOLORSTATIC: {
            lWndProcRet = (INT_PTR)GetStockObject(WHITE_BRUSH);
        } break;
        case WM_COMMAND: {
            CommandCallback(etInfo, wParam);
        } break;
        case WM_DESTROY: {
            DestroyCallback(etInfo);
            lWndProcRet = DefWindowProc(hEthernetTool, message, wParam, lParam);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hEthernetTool, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}

static LRESULT CreateCallback(HWND hEthernetTool, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PETHERNETTOOLEXTRA etInfo;
    HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
    PETHERNETTOOLCFG pEtConfig = ((LPCREATESTRUCT)lParam)->lpCreateParams;
    RECT rcClientSize;

    HANDLE hHeap = GetProcessHeap();
    if (NULL == hHeap) { lWndProcRet = -1; }
    if (-1 != lWndProcRet)
    {
        etInfo = (PETHERNETTOOLEXTRA)HANWinHeapAlloc(hHeap, NULL, sizeof(ETHERNETTOOLEXTRA));
        if (NULL == etInfo) { lWndProcRet = -1; }
    }

    if (-1 != lWndProcRet)
    {
        int nWinX = ETHERNETTOOL_WINDOW_DX;
        int nWinY = ETHERNETTOOL_WINDOW_DY;

        SetWindowLongPtr(hEthernetTool, 0, (LONG_PTR)etInfo);

        *etInfo = g_etDefaultCfg;
        etInfo->hHeap = hHeap;
        etInfo->hInst = hInst;
        etInfo->hSelf = hEthernetTool;
        if (NULL != pEtConfig)
        {
            etInfo->etConfig = *pEtConfig;
        }

        GetClientRect(hEthernetTool, &rcClientSize);

        RegisterHANEthernetToolConsole(hInst);
        RegisterHANEthernetToolHttp(hInst);
        RegisterHANEthernetToolSomeIP(hInst);
        RegisterHANEthernetToolSomeIPClient(hInst);
        RegisterHANEthernetToolSomeIPServer(hInst);

        etInfo->hFont.hHex = CreateFontIndirect(&g_lfHexFont);
        etInfo->hFont.hSys = CreateFontIndirect(&g_lfSysFont);

        etInfo->printData.printData.hText = CreateWindow(TEXT("edit"), NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_LEFT | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY,
            nWinX, nWinY, ETHERNETTOOL_TEXT_WINDOW_MIN_W, ETHERNETTOOL_TEXT_WINDOW_MIN_H,
            hEthernetTool, (HMENU)WID_ETHERNET_TOOL_TEXT, hInst, NULL);
        
        nWinX = ETHERNETTOOL_WINDOW_DX;
        nWinY += ETHERNETTOOL_TEXT_WINDOW_MIN_H + ETHERNETTOOL_WINDOW_DY;
        etInfo->hConsole = CreateEthernetToolConsoleWindow(
            nWinX, nWinY, hEthernetTool, (HMENU)WID_ETHERNET_TOOL_CONSOLE, hInst, etInfo
        );

        nWinX += COMTOOL_CONSOLE_WINDOW_W + (ETHERNETTOOL_WINDOW_DX * 2);
        etInfo->hHttp = CreateWindow(TEXT("button"), TEXT("http/https控制台"), WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            nWinX, nWinY, ETHERNETTOOL_HTTP_BUTTON_W, ETHERNETTOOL_HTTP_BUTTON_H,
            hEthernetTool, (HMENU)WID_ETHERNET_TOOL_HTTP, hInst, etInfo
        );
        nWinY += ETHERNETTOOL_HTTP_BUTTON_H + ETHERNETTOOL_WINDOW_DY;
        etInfo->hSomeIP = CreateWindow(TEXT("button"), TEXT("SOME/IP控制台"), WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            nWinX, nWinY, ETHERNETTOOL_SOMEIP_BUTTON_W, ETHERNETTOOL_SOMEIP_BUTTON_H,
            hEthernetTool, (HMENU)WID_ETHERNET_TOOL_SOME_IP, hInst, etInfo
        );

        SendMessage(etInfo->printData.printData.hText, WM_SETFONT, (WPARAM)(etInfo->hFont.hHex), (LPARAM)TRUE);
        SendMessage(etInfo->hHttp, WM_SETFONT, (WPARAM)(etInfo->hFont.hSys), (LPARAM)TRUE);
        SendMessage(etInfo->hSomeIP, WM_SETFONT, (WPARAM)(etInfo->hFont.hSys), (LPARAM)TRUE);

        EditSetLimitText(etInfo->printData.printData.hText, ETHERNETTOOL_TEXT_WINDOW_LIMIT);

        SizeCallback(etInfo);
    }

    return lWndProcRet;
}
static void SizeCallback(PETHERNETTOOLEXTRA etInfo)
{
    RECT rcClientSize;
    HANINT nWinX;
    HANINT nWinY;
    HANINT nWinW;
    HANINT nWinH;
    
    GetClientRect(etInfo->hSelf, &rcClientSize);
    nWinW = GetRectW(&rcClientSize) - (2 * ETHERNETTOOL_WINDOW_DX);
    nWinH = GetRectH(&rcClientSize) - (3 * ETHERNETTOOL_WINDOW_DY) - ETHERNETTOOL_CONSOLE_WINDOW_H;

    if (nWinW < ETHERNETTOOL_TEXT_WINDOW_MIN_W) { nWinW = ETHERNETTOOL_TEXT_WINDOW_MIN_W; }
    if (nWinH < ETHERNETTOOL_TEXT_WINDOW_MIN_H) { nWinH = ETHERNETTOOL_TEXT_WINDOW_MIN_H; }

    nWinX = ETHERNETTOOL_WINDOW_DX;
    nWinY = ETHERNETTOOL_WINDOW_DY;
    MoveWindow(etInfo->printData.printData.hText, nWinX, nWinY, nWinW, nWinH, TRUE);
    nWinY += nWinH + ETHERNETTOOL_WINDOW_DY;
    MoveWindow(etInfo->hConsole, nWinX, nWinY, ETHERNETTOOL_CONSOLE_WINDOW_W, ETHERNETTOOL_CONSOLE_WINDOW_H, TRUE);
    nWinX += COMTOOL_CONSOLE_WINDOW_W + (ETHERNETTOOL_WINDOW_DX * 2);
    MoveWindow(etInfo->hHttp, nWinX, nWinY, ETHERNETTOOL_HTTP_BUTTON_W, ETHERNETTOOL_HTTP_BUTTON_H, TRUE);
    nWinY += ETHERNETTOOL_HTTP_BUTTON_H + ETHERNETTOOL_WINDOW_DY;
    MoveWindow(etInfo->hSomeIP, nWinX, nWinY, ETHERNETTOOL_SOMEIP_BUTTON_W, ETHERNETTOOL_SOMEIP_BUTTON_H, TRUE);
}
static void CommandCallback(PETHERNETTOOLEXTRA etInfo, WPARAM wParam)
{
    switch (LOWORD(wParam)) {
        case WID_ETHERNET_TOOL_HTTP: {
            CreateEthernetToolHttpWindow(etInfo);
        } break;
        case WID_ETHERNET_TOOL_SOME_IP: {
            CreateEthernetToolSomeIPWindow(etInfo);
        } break;

        default: { } break;
    }
}
static void DestroyCallback(PETHERNETTOOLEXTRA etInfo)
{
    while (0 < HANListGetLen(&etInfo->listChildModel))
    {
        DestroyWindow(HANListGetData(&etInfo->listChildModel, 0));
    }
    HANWinHeapFree(etInfo->hHeap, 0, etInfo);
}
