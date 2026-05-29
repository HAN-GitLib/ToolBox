#include <Windows.h>
#include <CommCtrl.h>

#include "HAN_EthernetToolSomeIP.h"
#include "HAN_EthernetToolSomeIPClient.h"
#include "HAN_EthernetToolSomeIPServer.h"

#define HAN_ETHERNET_TOOL_SOME_IP_CLASS          TEXT("HAN_EthernetToolSomeIP")

typedef enum {
    SOME_IP_CS_CLIENT,
    SOME_IP_CS_SERVER,
    SOME_IP_CS_CNT_MAX,
} SOMEIPCS;

typedef struct tagETHERNETTOOLSOMEIPEXTRA {
    HANDLE                  hHeap;
    HINSTANCE               hInst;
    HWND                    hSelf;
    HWND                    hTab;
    HWND                    hClient;
    HWND                    hServer;
    struct {
        HFONT               hHex;
        HFONT               hSys;
    } hFont;                /* 字体 */
} ETHERNETTOOLSOMEIPEXTRA, * PETHERNETTOOLSOMEIPEXTRA;

static LRESULT CALLBACK EthernetToolSomeIPWndProc(HWND hEthernetToolSomeIP, UINT message, WPARAM wParam, LPARAM lParam);

static LRESULT CreateCallback(HWND hEthernetToolSomeIP, LPARAM lParam);
static void NotifyCallback(PETHERNETTOOLSOMEIPEXTRA siInfo, LPARAM lParam);

void RegisterHANEthernetToolSomeIP(HINSTANCE hInst)
{
    WNDCLASSEX wcex = {
        .cbSize         = sizeof(WNDCLASSEX),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = EthernetToolSomeIPWndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = sizeof(PETHERNETTOOLSOMEIPEXTRA),
        .hInstance      = hInst,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName   = NULL,
        .lpszClassName  = HAN_ETHERNET_TOOL_SOME_IP_CLASS,
        .hIconSm        = NULL,
    };
    RegisterClassEx(&wcex);
}

HWND CreateEthernetToolSomeIPWindow(PETHERNETTOOLEXTRA etInfo)
{
    return CreateWindow(HAN_ETHERNET_TOOL_SOME_IP_CLASS, TEXT("SOME/IP"),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 1000, 500, NULL, NULL, etInfo->hInst, etInfo);
}

static LRESULT CALLBACK EthernetToolSomeIPWndProc(HWND hEthernetToolSomeIP, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    PETHERNETTOOLSOMEIPEXTRA siInfo = (PETHERNETTOOLSOMEIPEXTRA)GetWindowLongPtr(hEthernetToolSomeIP, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = CreateCallback(hEthernetToolSomeIP, lParam);
        } break;
        case WM_NOTIFY: {
            NotifyCallback(siInfo, lParam);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hEthernetToolSomeIP, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}

static LRESULT CreateCallback(HWND hEthernetToolSomeIP, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PETHERNETTOOLSOMEIPEXTRA siInfo;
    HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
    RECT rcClientSize;
    RECT rcSomeIPSize;
    TCITEM tiItem;

    HANDLE hHeap = GetProcessHeap();
    if (NULL == hHeap) { lWndProcRet = -1; }
    if (-1 != lWndProcRet)
    {
        siInfo = (PETHERNETTOOLSOMEIPEXTRA)HANWinHeapAlloc(hHeap, NULL, sizeof(ETHERNETTOOLSOMEIPEXTRA));
        if (NULL == siInfo) { lWndProcRet = -1; }
    }

    if (-1 != lWndProcRet)
    {
        SetWindowLongPtr(hEthernetToolSomeIP, 0, (LONG_PTR)siInfo);

        siInfo->hHeap = hHeap;

        GetClientRect(hEthernetToolSomeIP, &rcClientSize);
        rcSomeIPSize = rcClientSize;
        rcSomeIPSize.top += 28;

        siInfo->hFont.hHex = CreateFontIndirect(&g_lfHexFont);
        siInfo->hFont.hSys = CreateFontIndirect(&g_lfSysFont);

        siInfo->hTab = CreateWindow(WC_TABCONTROL, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
            0, 0, rcClientSize.right, rcClientSize.bottom,
            hEthernetToolSomeIP, (HMENU)WID_ETHERNET_TOOL_SOME_IP_TAB, hInst, NULL);
        siInfo->hClient = CreateEthernetToolSomeIPClientWindow(&rcSomeIPSize, hEthernetToolSomeIP, hInst);
        siInfo->hServer = CreateEthernetToolSomeIPServerWindow(&rcSomeIPSize, hEthernetToolSomeIP, hInst);
        
        SendMessage(siInfo->hTab, WM_SETFONT, (WPARAM)(siInfo->hFont.hSys), TRUE);
            
        tiItem.mask = TCIF_TEXT;
        tiItem.pszText = TEXT("客户端");
        TabCtrl_InsertItem(siInfo->hTab, SOME_IP_CS_CLIENT, &tiItem);
        tiItem.pszText = TEXT("服务端");
        TabCtrl_InsertItem(siInfo->hTab, SOME_IP_CS_SERVER, &tiItem);

        ShowWindow(siInfo->hServer, SW_HIDE);
        ShowWindow(siInfo->hClient, SW_SHOW);
    }

    return lWndProcRet;
}
static void NotifyCallback(PETHERNETTOOLSOMEIPEXTRA siInfo, LPARAM lParam)
{
    NMHDR* pNMHDR = (NMHDR*)lParam;
    if (WID_ETHERNET_TOOL_SOME_IP_TAB == pNMHDR->idFrom)
    {
        switch (pNMHDR->code) {
            case TCN_SELCHANGE: {
                int nPage = TabCtrl_GetCurSel(siInfo->hTab);
                if (SOME_IP_CS_CLIENT == nPage)
                {
                    ShowWindow(siInfo->hServer, SW_HIDE);
                    ShowWindow(siInfo->hClient, SW_SHOW);
                }
                else if (SOME_IP_CS_SERVER == nPage)
                {
                    ShowWindow(siInfo->hClient, SW_HIDE);
                    ShowWindow(siInfo->hServer, SW_SHOW);
                }
                else { }
                break;
            }
        }
    }
}
