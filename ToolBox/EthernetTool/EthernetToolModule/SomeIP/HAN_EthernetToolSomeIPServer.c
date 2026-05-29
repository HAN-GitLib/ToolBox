#include "HAN_EthernetToolSomeIPServer.h"

#define HAN_ETHERNET_TOOL_SOME_IP_SERVER_CLASS      TEXT("HAN_EthernetToolSomeIPServer")

typedef struct tagETHERNETTOOLSOMEIPSERVEREXTRA {
    HANDLE                  hHeap;
    HINSTANCE               hInst;
    HWND                    hSelf;
    HWND                    hStart;
    struct {
        HFONT               hHex;
        HFONT               hSys;
    } hFont;                /* 字体 */
} ETHERNETTOOLSOMEIPSERVEREXTRA, * PETHERNETTOOLSOMEIPSERVEREXTRA;

static LRESULT CALLBACK EthernetToolSomeIPServerWndProc(HWND hEthernetToolSomeIPServer, UINT message, WPARAM wParam, LPARAM lParam);

static LRESULT CreateCallback(HWND hEthernetToolSomeIPServer, LPARAM lParam);

void RegisterHANEthernetToolSomeIPServer(HINSTANCE hInst)
{
    WNDCLASSEX wcex = {
        .cbSize         = sizeof(WNDCLASSEX),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = EthernetToolSomeIPServerWndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = sizeof(PETHERNETTOOLSOMEIPSERVEREXTRA),
        .hInstance      = hInst,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName   = NULL,
        .lpszClassName  = HAN_ETHERNET_TOOL_SOME_IP_SERVER_CLASS,
        .hIconSm        = NULL,
    };
    RegisterClassEx(&wcex);
}

HWND CreateEthernetToolSomeIPServerWindow(RECT* rcClient, HWND hEthernetToolSomeIP, HINSTANCE hInst)
{
    int nWinW = GetRectW(rcClient);
    int nWinH = GetRectH(rcClient);

    return CreateWindow(
        HAN_ETHERNET_TOOL_SOME_IP_SERVER_CLASS, NULL, WS_CHILD | WS_VISIBLE,
        rcClient->left, rcClient->top, nWinW, nWinH, hEthernetToolSomeIP, (HMENU)WID_ETHERNET_TOOL_SOME_IP_SERVER, hInst, NULL
    );
}

static LRESULT CALLBACK EthernetToolSomeIPServerWndProc(HWND hEthernetToolSomeIPServer, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    // PETHERNETTOOLSOMEIPEXTRA siInfo = (PETHERNETTOOLSOMEIPEXTRA)GetWindowLongPtr(hEthernetToolSomeIP, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = CreateCallback(hEthernetToolSomeIPServer, lParam);
        } break;
        case WM_CTLCOLORSTATIC: {
            lWndProcRet = (INT_PTR)GetStockObject(WHITE_BRUSH);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hEthernetToolSomeIPServer, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}

static LRESULT CreateCallback(HWND hEthernetToolSomeIPServer, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PETHERNETTOOLSOMEIPSERVEREXTRA sisInfo;
    HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
    // RECT rcClientSize;

    HANDLE hHeap = GetProcessHeap();
    if (NULL == hHeap) { lWndProcRet = -1; }
    if (-1 != lWndProcRet)
    {
        sisInfo = (PETHERNETTOOLSOMEIPSERVEREXTRA)HANWinHeapAlloc(hHeap, NULL, sizeof(ETHERNETTOOLSOMEIPSERVEREXTRA));
        if (NULL == sisInfo) { lWndProcRet = -1; }
    }

    if (-1 != lWndProcRet)
    {
        int nWinX = ETHERNETTOOL_WINDOW_DX;
        int nWinY = ETHERNETTOOL_WINDOW_DY;

        SetWindowLongPtr(hEthernetToolSomeIPServer, 0, (LONG_PTR)sisInfo);

        // GetClientRect(hEthernetToolSomeIP, &rcClientSize);

        sisInfo->hFont.hHex = CreateFontIndirect(&g_lfHexFont);
        sisInfo->hFont.hSys = CreateFontIndirect(&g_lfSysFont);
        
        sisInfo->hStart = CreateWindow(TEXT("button"), TEXT("启动"), WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            nWinX, nWinY, 100, 30, hEthernetToolSomeIPServer, (HMENU)WID_ETHERNET_TOOL_SOME_IP_SERVER_START, hInst, NULL);
        
        SendMessage(sisInfo->hStart, WM_SETFONT, (WPARAM)(sisInfo->hFont.hSys), TRUE);
    }

    return lWndProcRet;
}
