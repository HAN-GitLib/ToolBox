#include <Windows.h>
#include <CommCtrl.h>

#include "HAN_EthernetToolSomeIPClient.h"

#define HAN_ETHERNET_TOOL_SOME_IP_CLIENT_CLASS      TEXT("HAN_EthernetToolSomeIPClient")

typedef struct tagETHERNETTOOLSOMEIPCLIENTEXTRA {
    HANDLE                  hHeap;
    HINSTANCE               hInst;
    HWND                    hSelf;
    struct {
        ETHERNETIPV4        local;
        ETHERNETIPV4        multi;
    } cfgEthernet;
    struct {
        HWND                hFindService;
        HWND                hServiceListTitle;
        HWND                hServiceList;
    } sdConsole;
    struct {
        HFONT               hHex;
        HFONT               hSys;
    } hFont;                /* 字体 */
} ETHERNETTOOLSOMEIPCLIENTEXTRA, * PETHERNETTOOLSOMEIPCLIENTEXTRA;

static LRESULT CALLBACK EthernetToolSomeIPClientWndProc(HWND hEthernetToolSomeIPClient, UINT message, WPARAM wParam, LPARAM lParam);

static LRESULT CreateCallback(HWND hEthernetToolSomeIPClient, LPARAM lParam);

void RegisterHANEthernetToolSomeIPClient(HINSTANCE hInst)
{
    WNDCLASSEX wcex = {
        .cbSize         = sizeof(WNDCLASSEX),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = EthernetToolSomeIPClientWndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = sizeof(PETHERNETTOOLSOMEIPCLIENTEXTRA),
        .hInstance      = hInst,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName   = NULL,
        .lpszClassName  = HAN_ETHERNET_TOOL_SOME_IP_CLIENT_CLASS,
        .hIconSm        = NULL,
    };
    RegisterClassEx(&wcex);
}

HWND CreateEthernetToolSomeIPClientWindow(RECT* rcClient, HWND hEthernetToolSomeIP, HINSTANCE hInst)
{
    int nWinW = GetRectW(rcClient);
    int nWinH = GetRectH(rcClient);

    return CreateWindow(
        HAN_ETHERNET_TOOL_SOME_IP_CLIENT_CLASS, NULL, WS_CHILD | WS_VISIBLE,
        rcClient->left, rcClient->top, nWinW, nWinH, hEthernetToolSomeIP, (HMENU)WID_ETHERNET_TOOL_SOME_IP_CLIENT, hInst, NULL
    );
}

static LRESULT CALLBACK EthernetToolSomeIPClientWndProc(HWND hEthernetToolSomeIPClient, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    // PETHERNETTOOLSOMEIPEXTRA siInfo = (PETHERNETTOOLSOMEIPEXTRA)GetWindowLongPtr(hEthernetToolSomeIP, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = CreateCallback(hEthernetToolSomeIPClient, lParam);
        } break;
        case WM_CTLCOLORSTATIC: {
            lWndProcRet = (INT_PTR)GetStockObject(WHITE_BRUSH);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hEthernetToolSomeIPClient, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}

static LRESULT CreateCallback(HWND hEthernetToolSomeIPClient, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PETHERNETTOOLSOMEIPCLIENTEXTRA sicInfo;
    HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
    RECT rcClientSize;

    HANDLE hHeap = GetProcessHeap();
    if (NULL == hHeap) { lWndProcRet = -1; }
    if (-1 != lWndProcRet)
    {
        sicInfo = (PETHERNETTOOLSOMEIPCLIENTEXTRA)HANWinHeapAlloc(hHeap, NULL, sizeof(ETHERNETTOOLSOMEIPCLIENTEXTRA));
        if (NULL == sicInfo) { lWndProcRet = -1; }
    }

    if (-1 != lWndProcRet)
    {
        int nWinX = ETHERNETTOOL_WINDOW_DX;
        int nWinY = ETHERNETTOOL_WINDOW_DY;
        int nWinH;

        SetWindowLongPtr(hEthernetToolSomeIPClient, 0, (LONG_PTR)sicInfo);

        GetClientRect(hEthernetToolSomeIPClient, &rcClientSize);

        sicInfo->hFont.hHex = CreateFontIndirect(&g_lfHexFont);
        sicInfo->hFont.hSys = CreateFontIndirect(&g_lfSysFont);
        
        nWinX = ETHERNETTOOL_WINDOW_DX;
        sicInfo->cfgEthernet.local.hTitle = CreateWindow(TEXT("static"), TEXT("本机地址："),
            WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE,
            nWinX, nWinY, ETHERNETTOOL_SOMEIP_LM_TITLE_W, ETHERNETTOOL_WINDOW_TEXT_HEIGHT,
            hEthernetToolSomeIPClient, (HMENU)WID_ETHERNET_TOOL_SOME_IP_CLIENT_LOCAL_TITLE, hInst, NULL);
        nWinX += ETHERNETTOOL_SOMEIP_LM_TITLE_W;
        sicInfo->cfgEthernet.local.hIPv4 = CreateWindow(WC_IPADDRESS, TEXT(""),
            WS_CHILD | WS_VISIBLE, nWinX, nWinY, ETHERNETTOOL_IPV4_W, ETHERNETTOOL_WINDOW_TEXT_HEIGHT,
            hEthernetToolSomeIPClient, (HMENU)WID_ETHERNET_TOOL_SOME_IP_CLIENT_LOCAL_IPV4, hInst, NULL);
        nWinX += ETHERNETTOOL_IPV4_W + 5;
        sicInfo->cfgEthernet.local.hPort = CreateWindow(TEXT("edit"), TEXT(""),
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER, nWinX, nWinY, ETHERNETTOOL_PORT_W, ETHERNETTOOL_WINDOW_TEXT_HEIGHT,
            hEthernetToolSomeIPClient, (HMENU)WID_ETHERNET_TOOL_SOME_IP_CLIENT_LOCAL_PORT, hInst, NULL);
        
        nWinX = ETHERNETTOOL_WINDOW_DX;
        nWinY += ETHERNETTOOL_WINDOW_DY + ETHERNETTOOL_WINDOW_TEXT_HEIGHT;
        sicInfo->cfgEthernet.multi.hTitle = CreateWindow(TEXT("static"), TEXT("组播地址："),
            WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE,
            nWinX, nWinY, ETHERNETTOOL_SOMEIP_LM_TITLE_W, ETHERNETTOOL_WINDOW_TEXT_HEIGHT,
            hEthernetToolSomeIPClient, (HMENU)WID_ETHERNET_TOOL_SOME_IP_CLIENT_MULTI_TITLE, hInst, NULL);
        nWinX += ETHERNETTOOL_SOMEIP_LM_TITLE_W;
        sicInfo->cfgEthernet.multi.hIPv4 = CreateWindow(WC_IPADDRESS, TEXT(""),
            WS_CHILD | WS_VISIBLE, nWinX, nWinY, ETHERNETTOOL_IPV4_W, ETHERNETTOOL_WINDOW_TEXT_HEIGHT,
            hEthernetToolSomeIPClient, (HMENU)WID_ETHERNET_TOOL_SOME_IP_CLIENT_MULTI_IPV4, hInst, NULL);
        nWinX += ETHERNETTOOL_IPV4_W + 5;
        sicInfo->cfgEthernet.multi.hPort = CreateWindow(TEXT("edit"), TEXT(""),
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER, nWinX, nWinY, ETHERNETTOOL_PORT_W, ETHERNETTOOL_WINDOW_TEXT_HEIGHT,
            hEthernetToolSomeIPClient, (HMENU)WID_ETHERNET_TOOL_SOME_IP_CLIENT_MULTI_PORT, hInst, NULL);

        nWinX = ETHERNETTOOL_WINDOW_DX;
        nWinY += ETHERNETTOOL_WINDOW_TEXT_HEIGHT + ETHERNETTOOL_WINDOW_DY;
        sicInfo->sdConsole.hFindService = CreateWindow(TEXT("button"), TEXT("查找服务"),
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, nWinX, nWinY, 100, 30,
            hEthernetToolSomeIPClient, (HMENU)WID_ETHERNET_TOOL_SOME_IP_CLIENT_FIND_SERVICE, hInst, NULL);

        nWinX += ETHERNETTOOL_SOMEIP_SD_CFG_W + (ETHERNETTOOL_WINDOW_DY * 3);
        nWinY = ETHERNETTOOL_WINDOW_DY;
        sicInfo->sdConsole.hServiceListTitle = CreateWindow(TEXT("static"), TEXT("服务列表"),
            WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE, nWinX, nWinY, 90, ETHERNETTOOL_WINDOW_TEXT_HEIGHT,
            hEthernetToolSomeIPClient, (HMENU)WID_ETHERNET_TOOL_SOME_IP_CLIENT_SERVICE_LIST_TITLE, hInst, NULL);
        nWinY += ETHERNETTOOL_WINDOW_TEXT_HEIGHT;
        nWinH = GetRectH(&rcClientSize) - nWinY - ETHERNETTOOL_WINDOW_DY;
        sicInfo->sdConsole.hServiceList = CreateWindow(TEXT("listbox"), NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL, nWinX, nWinY, 200, nWinH,
            hEthernetToolSomeIPClient, (HMENU)WID_ETHERNET_TOOL_SOME_IP_CLIENT_SERVICE_LIST, hInst, NULL);
        
        SendMessage(sicInfo->cfgEthernet.local.hTitle, WM_SETFONT, (WPARAM)(sicInfo->hFont.hSys), TRUE);
        SendMessage(sicInfo->cfgEthernet.local.hIPv4, WM_SETFONT, (WPARAM)(sicInfo->hFont.hSys), TRUE);
        SendMessage(sicInfo->cfgEthernet.local.hPort, WM_SETFONT, (WPARAM)(sicInfo->hFont.hSys), TRUE);
        SendMessage(sicInfo->cfgEthernet.multi.hTitle, WM_SETFONT, (WPARAM)(sicInfo->hFont.hSys), TRUE);
        SendMessage(sicInfo->cfgEthernet.multi.hIPv4, WM_SETFONT, (WPARAM)(sicInfo->hFont.hSys), TRUE);
        SendMessage(sicInfo->cfgEthernet.multi.hPort, WM_SETFONT, (WPARAM)(sicInfo->hFont.hSys), TRUE);
        SendMessage(sicInfo->sdConsole.hFindService, WM_SETFONT, (WPARAM)(sicInfo->hFont.hSys), TRUE);
        SendMessage(sicInfo->sdConsole.hServiceListTitle, WM_SETFONT, (WPARAM)(sicInfo->hFont.hSys), TRUE);
        SendMessage(sicInfo->sdConsole.hServiceList, WM_SETFONT, (WPARAM)(sicInfo->hFont.hSys), TRUE);
    }

    return lWndProcRet;
}
