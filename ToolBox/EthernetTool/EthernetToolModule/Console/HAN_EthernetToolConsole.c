#include <winsock2.h>
#include <ws2def.h>
#include <Windows.h>
#include <CommCtrl.h>
#include <iphlpapi.h>

#include "HAN_EthernetToolConsole.h"
#include "..\GlobalVariables\HAN_EthernetToolGlobalVariables.h"

#define HAN_ETHERNET_TOOL_CONSOLE_CLASS     TEXT("EthernetToolConsole")

typedef enum {
    TCP_THREAD_STATUS_START,
    TCP_THREAD_STATUS_RECV,
    TCP_THREAD_STATUS_END,
} TCPTHREADSTATUS;

static LRESULT CALLBACK EthernetToolConsoleWndProc(HWND hEthernetToolConsole, UINT message, WPARAM wParam, LPARAM lParam);

static LRESULT CreateCallback(HWND hEthernetToolConsole, LPARAM lParam);
static void CommandCallback(HWND hEthernetToolConsole, PETHERNETTOOLEXTRA etInfo, WPARAM wParam, LPARAM lParam);
static void DestroyCallback(PETHERNETTOOLEXTRA etInfo);
static HANSIZE OpenPortCallback(HWND hStateButton, HWND hChildButton, void* pParam);
static HANSIZE ClosePortCallback(HWND hStateButton, HWND hChildButton, void* pParam);
static void ChooseProtocolCallback(PETHERNETTOOLEXTRA etInfo, WPARAM wParam);
static void ClearWindowCallback(PETHERNETTOOLEXTRA etInfo);
static void SendDataCallback(PETHERNETTOOLEXTRA etInfo);
static void SysIPCallback(PETHERNETTOOLEXTRA etInfo);
static void HexButtonCallback(PETHERNETTOOLEXTRA etInfo);
static BOOL CreateTcpServerLink(PETHERNETTOOLEXTRA etInfo);
static BOOL CreateTcpClientLink(PETHERNETTOOLEXTRA etInfo);
static BOOL CreateUdpLink(PETHERNETTOOLEXTRA etInfo);
static void GetIPv4Value(PETHERNETIPV4 pIPv4);
static void CloseEthernetToolSocket(SOCKET* pSocket);
static void PrintSocketData(PETHERNETTOOLEXTRA etInfo, const char* pBuf, HANSIZE nBufLen);
static void SetIPPortEnable(PETHERNETTOOLEXTRA etInfo);
static void SetConnectSuccessfullyText(HANPSTR pText, HANSIZE nTextLen, PCSOCKADDR_IN sockLocal, PCSOCKADDR_IN sockRemote);

static DWORD TCPServerThread(PETHERNETTOOLEXTRA etInfo);
static DWORD TCPClientThread(PETHERNETTOOLEXTRA etInfo);
static DWORD UDPThread(PETHERNETTOOLEXTRA etInfo);

static void CfgWindowToTextLocalIPv4(PETHERNETTOOLEXTRA etInfo, HANPSTR pText);
static void CfgWindowToTextLocalPort(PETHERNETTOOLEXTRA etInfo, HANPSTR pText);
static void CfgWindowToTextRemoteIPv4(PETHERNETTOOLEXTRA etInfo, HANPSTR pText);
static void CfgWindowToTextRemotePort(PETHERNETTOOLEXTRA etInfo, HANPSTR pText);
static void CfgWindowToTextProtocol(PETHERNETTOOLEXTRA etInfo, HANPSTR pText);
static void CfgWindowToTextHex(PETHERNETTOOLEXTRA etInfo, HANPSTR pText);

static void CfgTextToWindowLocalIPv4(PETHERNETTOOLEXTRA etInfo, HANPCSTR pText);
static void CfgTextToWindowLocalPort(PETHERNETTOOLEXTRA etInfo, HANPCSTR pText);
static void CfgTextToWindowRemoteIPv4(PETHERNETTOOLEXTRA etInfo, HANPCSTR pText);
static void CfgTextToWindowRemotePort(PETHERNETTOOLEXTRA etInfo, HANPCSTR pText);
static void CfgTextToWindowProtocol(PETHERNETTOOLEXTRA etInfo, HANPCSTR pText);
static void CfgTextToWindowHex(PETHERNETTOOLEXTRA etInfo, HANPCSTR pText);

static const ETHERNETTOOLREADWRITECFG sg_pEthernetToolCfgInfo[INI_ETHERNET_TOOL_CFG_CNT] = {
    [INI_ETHERNET_TOOL_LOCAL_IPV4] = {
        .pKey = TEXT("LocalIPv4"),
        .pDefValue = TEXT("127.0.0.1"),
        .CfgWindowToText = CfgWindowToTextLocalIPv4,
        .CfgTextToWindow = CfgTextToWindowLocalIPv4,
    },
    [INI_ETHERNET_TOOL_LOCAL_PORT] = {
        .pKey = TEXT("LocalPort"),
        .pDefValue = TEXT("80"),
        .CfgWindowToText = CfgWindowToTextLocalPort,
        .CfgTextToWindow = CfgTextToWindowLocalPort,
    },
    [INI_ETHERNET_TOOL_REMOTE_IPV4] = {
        .pKey = TEXT("RemoteIPv4"),
        .pDefValue = TEXT("127.0.0.1"),
        .CfgWindowToText = CfgWindowToTextRemoteIPv4,
        .CfgTextToWindow = CfgTextToWindowRemoteIPv4,
    },
    [INI_ETHERNET_TOOL_REMOTE_PORT] = {
        .pKey = TEXT("RemotePort"),
        .pDefValue = TEXT("80"),
        .CfgWindowToText = CfgWindowToTextRemotePort,
        .CfgTextToWindow = CfgTextToWindowRemotePort,
    },
    [INI_ETHERNET_TOOL_PROTOCOL] = {
        .pKey = TEXT("Protocol"),
        .pDefValue = TEXT("UDP"),
        .CfgWindowToText = CfgWindowToTextProtocol,
        .CfgTextToWindow = CfgTextToWindowProtocol,
    },
    [INI_ETHERNET_TOOL_HEX] = {
        .pKey = TEXT("Hex"),
        .pDefValue = TEXT("FALSE"),
        .CfgWindowToText = CfgWindowToTextHex,
        .CfgTextToWindow = CfgTextToWindowHex,
    },
};
static const ETHERNETTOOLPROTOCOL  sg_pEthernetProtocol[ETHERNETTOOL_PROTOCOL_CNT] = {
    [ETHERNETTOOL_PROTOCOL_TCP_SERVER] = {
        .pName = TEXT("TCP服务端"),
        .bLocalIPOptional = TRUE,
        .bLocalPortOptional = TRUE,
        .bRemoteIPOptional = FALSE,
        .bRemotePortOptional = FALSE,
        .CreateSocketCallback = CreateTcpServerLink,
    },
    [ETHERNETTOOL_PROTOCOL_TCP_CLIENT] = {
        .pName = TEXT("TCP客户端"),
        .bLocalIPOptional = FALSE,
        .bLocalPortOptional = FALSE,
        .bRemoteIPOptional = TRUE,
        .bRemotePortOptional = TRUE,
        .CreateSocketCallback = CreateTcpClientLink,
    },
    [ETHERNETTOOL_PROTOCOL_UDP] = {
        .pName = TEXT("UDP"),
        .bLocalIPOptional = TRUE,
        .bLocalPortOptional = TRUE,
        .bRemoteIPOptional = TRUE,
        .bRemotePortOptional = TRUE,
        .CreateSocketCallback = CreateUdpLink,
    },
};

void RegisterHANEthernetToolConsole(HINSTANCE hInst)
{
    WNDCLASSEX wcex = {
        .cbSize         = sizeof(WNDCLASSEX),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = EthernetToolConsoleWndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = sizeof(PETHERNETTOOLEXTRA),
        .hInstance      = hInst,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName   = NULL,
        .lpszClassName  = HAN_ETHERNET_TOOL_CONSOLE_CLASS,
        .hIconSm        = NULL,
    };
    RegisterClassEx(&wcex);
}

HWND CreateEthernetToolConsoleWindow(HANINT x, HANINT y, HWND hEthernetTool, HMENU winId, HINSTANCE hInst, PETHERNETTOOLEXTRA etInfo)
{
    return CreateWindow(HAN_ETHERNET_TOOL_CONSOLE_CLASS, NULL,
        WS_CHILD | WS_VISIBLE | WS_BORDER, x, y, COMTOOL_CONSOLE_WINDOW_W, COMTOOL_CONSOLE_WINDOW_H,
        hEthernetTool, winId, hInst, etInfo
    );
}

void ReadEthernetToolConsoleIniFile(HANPCSTR pIniPath, void* pParam)
{
    PETHERNETTOOLCFG pEtConfig = pParam;
    INIETHERNETTOOLCFGID iLoop;

    for (iLoop = 0; iLoop < INI_ETHERNET_TOOL_CFG_CNT; iLoop++)
    {
        HAN_strcpy(pEtConfig->pSysConfig[iLoop].pKey, sg_pEthernetToolCfgInfo[iLoop].pKey);
        HAN_strcpy(pEtConfig->pSysConfig[iLoop].pDefValue, sg_pEthernetToolCfgInfo[iLoop].pDefValue);
        GetPrivateProfileString(
            INI_ETHERNET_TOOL_APP_NAME, pEtConfig->pSysConfig[iLoop].pKey, pEtConfig->pSysConfig[iLoop].pDefValue,
            pEtConfig->pSysConfig[iLoop].pValue, INI_ETHERNET_TOOL_VALUE_SIZE, pIniPath);
    }
}

void WriteEthernetToolConsoleIniFile(HANPCSTR pIniPath, HWND hEthernetTool)
{
    PETHERNETTOOLEXTRA etInfo = (PETHERNETTOOLEXTRA)GetWindowLongPtr(hEthernetTool, 0);
    HANCHAR pBvCfg[INI_ETHERNET_TOOL_CFG_CNT][INI_ETHERNET_TOOL_VALUE_SIZE];
    INIETHERNETTOOLCFGID iLoop;
    
    for (iLoop = 0; iLoop < INI_ETHERNET_TOOL_CFG_CNT; iLoop++)
    {
        sg_pEthernetToolCfgInfo[iLoop].CfgWindowToText(etInfo, pBvCfg[iLoop]);
        WritePrivateProfileString(
            INI_ETHERNET_TOOL_APP_NAME, sg_pEthernetToolCfgInfo[iLoop].pKey, pBvCfg[iLoop], pIniPath
        );
    }
}

void LoadEthernetToolCfg(PETHERNETTOOLEXTRA etInfo, PETHERNETTOOLCFG pEtConfig)
{
    INIETHERNETTOOLCFGID iLoop;

    for (iLoop = 0; iLoop < INI_ETHERNET_TOOL_CFG_CNT; iLoop++)
    {
        sg_pEthernetToolCfgInfo[iLoop].CfgTextToWindow(etInfo, pEtConfig->pSysConfig[iLoop].pValue);
    }
}

static LRESULT CALLBACK EthernetToolConsoleWndProc(HWND hEthernetToolConsole, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    PETHERNETTOOLEXTRA etInfo = (PETHERNETTOOLEXTRA)GetWindowLongPtr(hEthernetToolConsole, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = CreateCallback(hEthernetToolConsole, lParam);
        } break;
        case WM_COMMAND: {
            CommandCallback(hEthernetToolConsole, etInfo, wParam, lParam);
        } break;
        case WM_CTLCOLORSTATIC: {
            lWndProcRet = (INT_PTR)GetStockObject(NULL_BRUSH);
        } break;
        case WM_DESTROY: {
            DestroyCallback(etInfo);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hEthernetToolConsole, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}

static LRESULT CreateCallback(HWND hEthernetToolConsole, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PETHERNETTOOLEXTRA etInfo = ((LPCREATESTRUCT)lParam)->lpCreateParams;
    RECT rcClientSize;
    INITCOMMONCONTROLSEX icex;
    WSADATA wsaData;

    int nWinX = ETHERNETTOOL_WINDOW_DX;
    int nWinY = ETHERNETTOOL_WINDOW_DY;
    int nWinW;
    int nWinH;

    SetWindowLongPtr(hEthernetToolConsole, 0, (LONG_PTR)etInfo);

    GetClientRect(hEthernetToolConsole, &rcClientSize);
    nWinW = GetRectW(&rcClientSize);
    nWinH = GetRectH(&rcClientSize);
    
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC  = ICC_INTERNET_CLASSES ;
    InitCommonControlsEx(&icex);

    etInfo->cfgEthernet.wsaError = WSAStartup(MAKEWORD(2, 2), &wsaData);

    nWinX = ETHERNETTOOL_LR_TITLE_X;
    etInfo->cfgEthernet.local.hTitle = CreateWindow(TEXT("static"), TEXT("本地"),
        WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE,
        nWinX, nWinY, ETHERNETTOOL_LR_TITLE_W, ETHERNETTOOL_WINDOW_TEXT_HEIGHT,
        hEthernetToolConsole, (HMENU)WID_ETHERNET_TOOL_LOCAL_TITLE, etInfo->hInst, NULL);
    nWinX = ETHERNETTOOL_IPV4_X;
    etInfo->cfgEthernet.local.hIPv4 = CreateWindow(WC_IPADDRESS, TEXT(""),
        WS_CHILD | WS_VISIBLE, nWinX, nWinY, ETHERNETTOOL_IPV4_W, ETHERNETTOOL_WINDOW_TEXT_HEIGHT,
        hEthernetToolConsole, (HMENU)WID_ETHERNET_TOOL_LOCAL_IPV4, etInfo->hInst, NULL);
    nWinX = ETHERNETTOOL_PORT_X;
    etInfo->cfgEthernet.local.hPort = CreateWindow(TEXT("edit"), TEXT(""),
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER, nWinX, nWinY, ETHERNETTOOL_PORT_W, ETHERNETTOOL_WINDOW_TEXT_HEIGHT,
        hEthernetToolConsole, (HMENU)WID_ETHERNET_TOOL_LOCAL_PORT, etInfo->hInst, NULL);
    nWinX = ETHERNETTOOL_SYS_IP_X;
    etInfo->cfgEthernet.hSysIP = CreateWindow(TEXT("button"), TEXT("本机IP"),
        WS_CHILD | WS_VISIBLE, nWinX, nWinY, ETHERNETTOOL_SYS_IP_W, ETHERNETTOOL_WINDOW_TEXT_HEIGHT,
        hEthernetToolConsole, (HMENU)WID_ETHERNET_TOOL_SYS_IP, etInfo->hInst, NULL);

    nWinX = ETHERNETTOOL_LR_TITLE_X;
    nWinY += ETHERNETTOOL_WINDOW_DY + ETHERNETTOOL_WINDOW_TEXT_HEIGHT;
    etInfo->cfgEthernet.remote.hTitle = CreateWindow(TEXT("static"), TEXT("远程"),
        WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE,
        nWinX, nWinY, ETHERNETTOOL_LR_TITLE_W, ETHERNETTOOL_WINDOW_TEXT_HEIGHT,
        hEthernetToolConsole, (HMENU)WID_ETHERNET_TOOL_REMOTE_TITLE, etInfo->hInst, NULL);
    nWinX = ETHERNETTOOL_IPV4_X;
    etInfo->cfgEthernet.remote.hIPv4 = CreateWindow(WC_IPADDRESS, TEXT(""),
        WS_CHILD | WS_VISIBLE, nWinX, nWinY, ETHERNETTOOL_IPV4_W, ETHERNETTOOL_WINDOW_TEXT_HEIGHT,
        hEthernetToolConsole, (HMENU)WID_ETHERNET_TOOL_REMOTE_IPV4, etInfo->hInst, NULL);
    nWinX = ETHERNETTOOL_PORT_X;
    etInfo->cfgEthernet.remote.hPort = CreateWindow(TEXT("edit"), TEXT(""),
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER, nWinX, nWinY, ETHERNETTOOL_PORT_W, ETHERNETTOOL_WINDOW_TEXT_HEIGHT,
        hEthernetToolConsole, (HMENU)WID_ETHERNET_TOOL_REMOTE_PORT, etInfo->hInst, NULL);

    nWinX = ETHERNETTOOL_WINDOW_DX;
    nWinY += ETHERNETTOOL_WINDOW_DY + ETHERNETTOOL_WINDOW_TEXT_HEIGHT;
    etInfo->cfgEthernet.protocol.hWnd = CreateWindow(TEXT("combobox"), TEXT(""),
        ETHERNETTOOL_COMBOBOX_STYLE, nWinX, nWinY, ETHERNETTOOL_PROTOCOL_W, 80,
        hEthernetToolConsole, (HMENU)WID_ETHERNET_TOOL_PROTOCOL, etInfo->hInst, NULL);
    nWinY += 30;
    etInfo->hOpenPort = CreateWindow(HSTBT_CLASS, NULL,
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, nWinX, nWinY, 100, 30,
        hEthernetToolConsole, (HMENU)WID_ETHERNET_TOOL_OPEN_PORT, etInfo->hInst, etInfo);
    nWinX += 100 + ETHERNETTOOL_WINDOW_DX;
    etInfo->printData.printData.hClear = CreateWindow(TEXT("button"), TEXT("清除窗口"),
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, nWinX, nWinY, 100, 30,
        hEthernetToolConsole, (HMENU)WID_ETHERNET_TOOL_CLEAR, etInfo->hInst, NULL);

    nWinX = ETHERNETTOOL_WINDOW_DX;
    nWinY += 30 + ETHERNETTOOL_WINDOW_DY;
    etInfo->printHex.hHex = CreateWindow(TEXT("button"), TEXT("HEX"),
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, nWinX, nWinY, 55, 26,
        hEthernetToolConsole, (HMENU)WID_ETHERNET_TOOL_HEX, etInfo->hInst, NULL);

    nWinY = ETHERNETTOOL_WINDOW_DY;
    nWinX = ETHERNETTOOL_PROTOCOL_W + (ETHERNETTOOL_WINDOW_DX * 2);
    etInfo->sendData.hEdit = CreateWindow(TEXT("edit"), TEXT(""),
        WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_LEFT | ES_AUTOVSCROLL | ES_MULTILINE,
        nWinX, nWinY, nWinW - nWinX - ETHERNETTOOL_WINDOW_DY,
        nWinH - 30 - (ETHERNETTOOL_WINDOW_DY * 3),
        hEthernetToolConsole, (HMENU)WID_ETHERNET_TOOL_SEND_DATA_EDIT, etInfo->hInst, NULL);
    nWinY = nWinH - 30 - ETHERNETTOOL_WINDOW_DY;
    etInfo->sendData.hSend = CreateWindow(TEXT("button"), TEXT("发送"),
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, nWinX, nWinY, 100, 30,
        hEthernetToolConsole, (HMENU)WID_ETHERNET_TOOL_SEND, etInfo->hInst, NULL);

    SendMessage(etInfo->cfgEthernet.local.hTitle, WM_SETFONT, (WPARAM)(etInfo->hFont.hSys), (LPARAM)TRUE);
    SendMessage(etInfo->cfgEthernet.local.hIPv4, WM_SETFONT, (WPARAM)(etInfo->hFont.hSys), (LPARAM)TRUE);
    SendMessage(etInfo->cfgEthernet.local.hPort, WM_SETFONT, (WPARAM)(etInfo->hFont.hSys), (LPARAM)TRUE);
    SendMessage(etInfo->cfgEthernet.hSysIP, WM_SETFONT, (WPARAM)(etInfo->hFont.hSys), (LPARAM)TRUE);
    SendMessage(etInfo->cfgEthernet.remote.hTitle, WM_SETFONT, (WPARAM)(etInfo->hFont.hSys), (LPARAM)TRUE);
    SendMessage(etInfo->cfgEthernet.remote.hIPv4, WM_SETFONT, (WPARAM)(etInfo->hFont.hSys), (LPARAM)TRUE);
    SendMessage(etInfo->cfgEthernet.remote.hPort, WM_SETFONT, (WPARAM)(etInfo->hFont.hSys), (LPARAM)TRUE);
    SendMessage(etInfo->cfgEthernet.protocol.hWnd, WM_SETFONT, (WPARAM)(etInfo->hFont.hSys), (LPARAM)TRUE);
    SendMessage(etInfo->hOpenPort, WM_SETFONT, (WPARAM)(etInfo->hFont.hSys), (LPARAM)TRUE);
    SendMessage(etInfo->printData.printData.hClear, WM_SETFONT, (WPARAM)(etInfo->hFont.hSys), (LPARAM)TRUE);
    SendMessage(etInfo->printHex.hHex, WM_SETFONT, (WPARAM)(etInfo->hFont.hSys), (LPARAM)TRUE);
    SendMessage(etInfo->sendData.hEdit, WM_SETFONT, (WPARAM)(etInfo->hFont.hHex), (LPARAM)TRUE);
    SendMessage(etInfo->sendData.hSend, WM_SETFONT, (WPARAM)(etInfo->hFont.hSys), (LPARAM)TRUE);

    HSTBTSTATE buttonState[2] = {
        { .pText = TEXT("打开端口"), .Callback = (HSTBTCALLBACK)OpenPortCallback, },
        { .pText = TEXT("关闭端口"), .Callback = (HSTBTCALLBACK)ClosePortCallback, },
    };
    HANStateButtonAddState(etInfo->hOpenPort, &buttonState[0]);
    HANStateButtonAddState(etInfo->hOpenPort, &buttonState[1]);
    ComboBoxAddStringStructArr(etInfo->cfgEthernet.protocol.hWnd, sg_pEthernetProtocol, sizeof(ETHERNETTOOLPROTOCOL), StructMemberOffset(ETHERNETTOOLPROTOCOL, pName), ArrLen(sg_pEthernetProtocol));

    etInfo->hConsole = hEthernetToolConsole;

    LoadEthernetToolCfg(etInfo, &(etInfo->etConfig));

    return lWndProcRet;
}
static void CommandCallback(HWND hEthernetToolConsole, PETHERNETTOOLEXTRA etInfo, WPARAM wParam, LPARAM lParam)
{
    (void)hEthernetToolConsole;
    (void)lParam;
    
    switch (LOWORD(wParam)) {
        case WID_ETHERNET_TOOL_PROTOCOL: {
            ChooseProtocolCallback(etInfo, wParam);
        } break;
        case WID_ETHERNET_TOOL_CLEAR: {
            ClearWindowCallback(etInfo);
        } break;
        case WID_ETHERNET_TOOL_SEND: {
            SendDataCallback(etInfo);
        } break;
        case WID_ETHERNET_TOOL_SYS_IP: {
            SysIPCallback(etInfo);
        } break;
        case WID_ETHERNET_TOOL_HEX: {
            HexButtonCallback(etInfo);
        } break;

        default: { } break;
    }
}
static void DestroyCallback(PETHERNETTOOLEXTRA etInfo)
{
    if (0 != etInfo->cfgEthernet.wsaError) { WSACleanup(); }
}
static HANSIZE OpenPortCallback(HWND hStateButton, HWND hChildButton, void* pParam)
{
    (void)hStateButton;
    (void)hChildButton;

    HANSIZE nRet = 0;
    PETHERNETTOOLEXTRA etInfo = (PETHERNETTOOLEXTRA)pParam;
    ETHERNETTOOLPROTOCOLID eProtocol = ComboBoxGetCursel(etInfo->cfgEthernet.protocol.hWnd);
    BOOL bOk;

    if (eProtocol < ETHERNETTOOL_PROTOCOL_CNT)
    {
        bOk = sg_pEthernetProtocol[eProtocol].CreateSocketCallback(etInfo);
        if (TRUE == bOk)
        {
            EnableWindow(etInfo->cfgEthernet.local.hIPv4, FALSE);
            EnableWindow(etInfo->cfgEthernet.local.hPort, FALSE);
            EnableWindow(etInfo->cfgEthernet.remote.hIPv4, FALSE);
            EnableWindow(etInfo->cfgEthernet.remote.hPort, FALSE);
            EnableWindow(etInfo->cfgEthernet.protocol.hWnd, FALSE);
            EnableWindow(etInfo->cfgEthernet.hSysIP, FALSE);
            nRet = 1;
        }
        else
        {
            nRet = 0;
        }
    }
    else
    {
        MessageBox(NULL, TEXT("找不到协议"), TEXT("协议错误"), 0);
        nRet = 0;
    }

    return nRet;
}
static HANSIZE ClosePortCallback(HWND hStateButton, HWND hChildButton, void* pParam)
{
    (void)hStateButton;
    (void)hChildButton;
    
    PETHERNETTOOLEXTRA etInfo = (PETHERNETTOOLEXTRA)pParam;

    CloseEthernetToolSocket(&(etInfo->cfgEthernet.local.sct));
    etInfo->cfgEthernet.local.sct = INVALID_SOCKET;

    CloseEthernetToolSocket(&(etInfo->cfgEthernet.remote.sct));
    etInfo->cfgEthernet.remote.sct = INVALID_SOCKET;

    if (NULL != etInfo->cfgEthernet.hSocketThread)
    {
        CloseHandle(etInfo->cfgEthernet.hSocketThread);
        etInfo->cfgEthernet.hSocketThread = NULL;
    }

    EnableWindow(etInfo->cfgEthernet.protocol.hWnd, TRUE);
    EnableWindow(etInfo->cfgEthernet.hSysIP, TRUE);
    SetIPPortEnable(etInfo);

    return 1;
}
static void ChooseProtocolCallback(PETHERNETTOOLEXTRA etInfo, WPARAM wParam)
{
    switch (HIWORD(wParam)) {
        case CBN_SELCHANGE: {
            SetIPPortEnable(etInfo);
        } break;

        default: { } break;
    }
}
static void ClearWindowCallback(PETHERNETTOOLEXTRA etInfo)
{
    SetWindowText(etInfo->printData.printData.hText, TEXT(""));
    etInfo->printData.nTextWindowLen = 0;
}
static void SendDataCallback(PETHERNETTOOLEXTRA etInfo)
{
    ETHERNETTOOLPROTOCOLID eProtocol;
    int nTextLen;
    void* pBuf;

    nTextLen = GetWindowTextLength(etInfo->sendData.hEdit);
    if (0 < nTextLen)
    {
        if (etInfo->sendData.nBufLen < (HANSIZE)(nTextLen + 1))
        {
            pBuf = HANWinHeapAlloc(etInfo->hHeap, etInfo->sendData.pBuf, sizeof(HANCHAR) * (nTextLen + 1));
            if (NULL != pBuf)
            {
                etInfo->sendData.pBuf = pBuf;
                etInfo->sendData.nBufLen = (nTextLen + 1);
            }
        }

        eProtocol = ComboBoxGetCursel(etInfo->cfgEthernet.protocol.hWnd);
        GetWindowText(etInfo->sendData.hEdit, etInfo->sendData.pBuf, (HANINT)(etInfo->sendData.nBufLen));
        switch (eProtocol) {
            case ETHERNETTOOL_PROTOCOL_TCP_SERVER: {
                if (INVALID_SOCKET != etInfo->cfgEthernet.remote.sct)
                {
                    send(etInfo->cfgEthernet.remote.sct, etInfo->sendData.pBuf, nTextLen, 0);
                }
            } break;
            case ETHERNETTOOL_PROTOCOL_TCP_CLIENT: {
                if (INVALID_SOCKET != etInfo->cfgEthernet.remote.sct)
                {
                    send(etInfo->cfgEthernet.remote.sct, etInfo->sendData.pBuf, nTextLen, 0);
                }
            } break;
            case ETHERNETTOOL_PROTOCOL_UDP: {
                SOCKADDR_IN sockAddr;
                HANCHAR pText[100];
                CHAR pChar[100];
                int nAddrSize = sizeof(sockAddr);

                sockAddr.sin_family = AF_INET;
                CfgWindowToTextRemoteIPv4(etInfo, pText);
                snprintf(pChar, ArrLen(pChar), PCHAR_PRINT_HANPSTR_FORMAT, pText);
                sockAddr.sin_addr.s_addr = inet_addr(pChar);
                GetWindowText(etInfo->cfgEthernet.remote.hPort, pText, ArrLen(pText));
                sockAddr.sin_port = htons((u_short)HAN_strtoul(pText, NULL, 10));
                sendto(
                    etInfo->cfgEthernet.local.sct,
                    etInfo->sendData.pBuf,
                    nTextLen,
                    0,
                    (SOCKADDR*)&sockAddr,
                    nAddrSize
                );
            } break;

            default: { } break;
        }
    }
}
static void SysIPCallback(PETHERNETTOOLEXTRA etInfo)
{
    PIP_ADAPTER_INFO apInfo = NULL;
    PIP_ADAPTER_INFO apInfoTmp;
    ULONG nSize = sizeof(IP_ADAPTER_INFO);
    BOOL bErr = FALSE;
    uint32_t nAddr;
    LPARAM lpAddr;

    // 运行两次，第一次确定缓存需要多大，第二次实际接收
    for (HANSIZE iLoop = 0; iLoop < 2; iLoop++)
    {
        apInfoTmp = (PIP_ADAPTER_INFO)HANWinHeapAlloc(etInfo->hHeap, apInfo, nSize);
        if (NULL == apInfoTmp) { bErr = TRUE; }
        else { apInfo = apInfoTmp; }
        //如果一次就成功，可以直接跳出去
        if ((FALSE == bErr) && (ERROR_BUFFER_OVERFLOW != GetAdaptersInfo(apInfo, &nSize))) { break; }
    }

    if (FALSE == bErr)
    {
        nAddr = inet_addr(apInfo->IpAddressList.IpAddress.String);
        lpAddr = MAKEIPADDRESS(
            FOURTH_IPADDRESS(nAddr),
            THIRD_IPADDRESS(nAddr),
            SECOND_IPADDRESS(nAddr),
            FIRST_IPADDRESS(nAddr)
        );
        SendMessage(etInfo->cfgEthernet.local.hIPv4, IPM_SETADDRESS, 0, lpAddr);
    }

    if (NULL != apInfo)
    {
        HANWinHeapFree(etInfo->hHeap, 0, apInfo);
    }
}
static void HexButtonCallback(PETHERNETTOOLEXTRA etInfo)
{
    int nCheck = ButtonGetCheck(etInfo->printHex.hHex);
    if (BST_CHECKED == nCheck) { etInfo->printHex.bHex = TRUE; }
    else { etInfo->printHex.bHex = FALSE; }
    SetWindowText(etInfo->printData.printData.hText, TEXT(""));
    etInfo->printData.nTextWindowLen = 0;
}
static BOOL CreateTcpServerLink(PETHERNETTOOLEXTRA etInfo)
{
    BOOL bRet = TRUE;
    SOCKET socketRet;
    SOCKADDR_IN sockAddr;
    HANCHAR pText[100];
    CHAR pChar[100];
    
    socketRet = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == socketRet)
    {
        bRet = FALSE;
        HAN_snprintf(pText, ArrLen(pText), TEXT("打开socket失败：%d"), WSAGetLastError());
        MessageBox(etInfo->hConsole, pText, TEXT("TCP服务端错误"), 0);
    }

    if (TRUE == bRet)
    {
        sockAddr.sin_family = AF_INET;
        CfgWindowToTextLocalIPv4(etInfo, pText);
        snprintf(pChar, ArrLen(pChar), PCHAR_PRINT_HANPSTR_FORMAT, pText);
        sockAddr.sin_addr.s_addr = inet_addr(pChar);
        GetWindowText(etInfo->cfgEthernet.local.hPort, pText, ArrLen(pText));
        sockAddr.sin_port = htons((u_short)HAN_strtoul(pText, NULL, 10));
        if (SOCKET_ERROR == bind(socketRet, (SOCKADDR*)&sockAddr, sizeof(sockAddr)))
        {
            bRet = FALSE;
            HAN_snprintf(pText, ArrLen(pText), TEXT("绑定端口失败：%d"), WSAGetLastError());
            pText[ArrLen(pText) - 1] = TEXT('\0');
            MessageBox(etInfo->hConsole, pText, TEXT("TCP服务端错误"), 0);
        }
    }

    if (TRUE == bRet)
    {
        if (SOCKET_ERROR == listen(socketRet, SOMAXCONN))
        {
            bRet = FALSE;
            HAN_snprintf(pText, ArrLen(pText), TEXT("监听错误：%d"), WSAGetLastError());
            pText[ArrLen(pText) - 1] = TEXT('\0');
            MessageBox(etInfo->hConsole, pText, TEXT("TCP服务端错误"), 0);
        }
    }

    if ((bRet == FALSE) && (INVALID_SOCKET != socketRet)) { CloseEthernetToolSocket(&socketRet); }
    else
    {
        etInfo->cfgEthernet.local.sct = socketRet;
        etInfo->cfgEthernet.hSocketThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TCPServerThread, (void*)etInfo, 0, NULL);
    }

    return bRet;
}
static BOOL CreateTcpClientLink(PETHERNETTOOLEXTRA etInfo)
{
    BOOL bRet = TRUE;
    SOCKADDR_IN sockAddr;
    HANCHAR pText[100];
    CHAR pChar[100];

    sockAddr.sin_family = AF_INET;
    CfgWindowToTextRemoteIPv4(etInfo, pText);
    snprintf(pChar, ArrLen(pChar), PCHAR_PRINT_HANPSTR_FORMAT, pText);
    sockAddr.sin_addr.s_addr = inet_addr(pChar);
    GetWindowText(etInfo->cfgEthernet.remote.hPort, pText, ArrLen(pText));
    sockAddr.sin_port = htons((u_short)HAN_strtoul(pText, NULL, 10));
    etInfo->cfgEthernet.remote.sockAddr = sockAddr;
    etInfo->cfgEthernet.hSocketThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TCPClientThread, (void*)etInfo, 0, NULL);

    return bRet;
}
static BOOL CreateUdpLink(PETHERNETTOOLEXTRA etInfo)
{
    BOOL bRet = TRUE;
    SOCKET socketRet;
    SOCKADDR_IN sockAddr;
    HANCHAR pText[100];
    CHAR pChar[100];
    
    socketRet = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (INVALID_SOCKET == socketRet)
    {
        bRet = FALSE;
        HAN_snprintf(pText, ArrLen(pText), TEXT("打开socket失败：%d"), WSAGetLastError());
        pText[ArrLen(pText) - 1] = TEXT('\0');
        MessageBox(etInfo->hConsole, pText, TEXT("UDP错误"), 0);
    }

    if (TRUE == bRet)
    {
        sockAddr.sin_family = AF_INET;
        CfgWindowToTextLocalIPv4(etInfo, pText);
        snprintf(pChar, ArrLen(pChar), PCHAR_PRINT_HANPSTR_FORMAT, pText);
        sockAddr.sin_addr.s_addr = inet_addr(pChar);
        GetWindowText(etInfo->cfgEthernet.local.hPort, pText, ArrLen(pText));
        sockAddr.sin_port = htons((u_short)HAN_strtoul(pText, NULL, 10));
        if (SOCKET_ERROR == bind(socketRet, (SOCKADDR*)&sockAddr, sizeof(sockAddr)))
        {
            bRet = FALSE;
            HAN_snprintf(pText, ArrLen(pText), TEXT("绑定端口失败：%d"), WSAGetLastError());
            pText[ArrLen(pText) - 1] = TEXT('\0');
            MessageBox(etInfo->hConsole, pText, TEXT("UDP错误"), 0);
        }
    }
    
    if ((bRet == FALSE) && (INVALID_SOCKET != socketRet)) { CloseEthernetToolSocket(&socketRet); }
    else
    {
        etInfo->cfgEthernet.local.sct = socketRet;
        etInfo->cfgEthernet.hSocketThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)UDPThread, (void*)etInfo, 0, NULL);
    }

    return bRet;
}
static void GetIPv4Value(PETHERNETIPV4 pIPv4)
{
    DWORD nIPv4;

    SendMessage(pIPv4->hIPv4, IPM_GETADDRESS, 0, (LPARAM)(&nIPv4));
    pIPv4->pIPv4[0] = FIRST_IPADDRESS(nIPv4);
    pIPv4->pIPv4[1] = SECOND_IPADDRESS(nIPv4);
    pIPv4->pIPv4[2] = THIRD_IPADDRESS(nIPv4);
    pIPv4->pIPv4[3] = FOURTH_IPADDRESS(nIPv4);
}
static void CloseEthernetToolSocket(SOCKET* pSocket)
{
    if (INVALID_SOCKET != *pSocket)
    {
        closesocket(*pSocket);
        *pSocket = INVALID_SOCKET;
    }
}
static void PrintSocketData(PETHERNETTOOLEXTRA etInfo, const char* pBuf, HANSIZE nBufLen)
{
    HANPSTR pText;
    HANSIZE iLoop;
    HANSIZE nTextSize = (ETHERNETTOOL_BUF_SIZE * 3) + 1;
    HANINT nTextLen = 0;
    
    pText = HANWinHeapAlloc(etInfo->hHeap, NULL, nTextSize);
    if (NULL != pText)
    {
        if (FALSE == etInfo->printHex.bHex)
        {
            for (iLoop = 0; iLoop < nBufLen; iLoop++)
            {
                if (TEXT('\0') != pBuf[iLoop])
                {
                    pText[nTextLen] = pBuf[iLoop];
                    nTextLen++;
                }
                else
                {
                    pText[nTextLen] = TEXT('\\');
                    pText[nTextLen + 1] = TEXT('0');
                    nTextLen += 2;
                }
            }
            pText[nTextLen] = TEXT('\0');
        }
        else
        {
            for (iLoop = 0; iLoop < nBufLen; iLoop++)
            {
                HAN_snprintf(&pText[nTextLen], nTextSize - nTextLen, TEXT("%02X "), pBuf[iLoop]);
                nTextLen += (HANINT)HAN_strlen(&pText[nTextLen]);
            }
            pText[nTextLen] = TEXT('\0');
        }

        if ((etInfo->printData.nTextWindowLen + nTextLen) < ETHERNETTOOL_TEXT_WINDOW_LIMIT)
        {
            EditAppendText(etInfo->printData.printData.hText, pText, FALSE);
            etInfo->printData.nTextWindowLen += nTextLen;
        }
        else
        {
            SetWindowText(etInfo->printData.printData.hText, pText);
            etInfo->printData.nTextWindowLen = nTextLen;
        }

        HANWinHeapFree(etInfo->hHeap, 0, pText);
    }
}
static void SetIPPortEnable(PETHERNETTOOLEXTRA etInfo)
{
    ETHERNETTOOLPROTOCOLID eProtocol = ComboBoxGetCursel(etInfo->cfgEthernet.protocol.hWnd);

    EnableWindow(etInfo->cfgEthernet.local.hIPv4, sg_pEthernetProtocol[eProtocol].bLocalIPOptional);
    EnableWindow(etInfo->cfgEthernet.local.hPort, sg_pEthernetProtocol[eProtocol].bLocalPortOptional);
    EnableWindow(etInfo->cfgEthernet.remote.hIPv4, sg_pEthernetProtocol[eProtocol].bRemoteIPOptional);
    EnableWindow(etInfo->cfgEthernet.remote.hPort, sg_pEthernetProtocol[eProtocol].bRemotePortOptional);
}
static void SetConnectSuccessfullyText(HANPSTR pText, HANSIZE nTextLen, PCSOCKADDR_IN sockLocal, PCSOCKADDR_IN sockRemote)
{
    char pLocalAddrText[32];
    ETHERNETPORT nLocalPort;
    char pRemoteAddrText[32];
    ETHERNETPORT nRemotePort;

    strcpy(pLocalAddrText, inet_ntoa(sockLocal->sin_addr));
    nLocalPort = ntohs(sockLocal->sin_port);
    strcpy(pRemoteAddrText, inet_ntoa(sockRemote->sin_addr));
    nRemotePort = ntohs(sockRemote->sin_port);

    HAN_snprintf(pText, nTextLen,
        TEXT("建立连接：\r\n")
        TEXT("本地IP：" HANPSTR_PRINT_PCHAR_FORMAT "，端口：%d\r\n")
        TEXT("远程IP：" HANPSTR_PRINT_PCHAR_FORMAT "，端口：%d\r\n"),
        pLocalAddrText, nLocalPort,
        pRemoteAddrText, nRemotePort);
}

static DWORD TCPServerThread(PETHERNETTOOLEXTRA etInfo)
{
    DWORD nRet = 0;
    SOCKET socketClient;
    SOCKADDR_IN addrRemote;
    SOCKADDR_IN addrLocal;
    int nAddrSize = sizeof(SOCKADDR_IN);
    PCHAR pBuf = etInfo->revData.pBuf;
    HANPSTR pText = etInfo->revData.pText;
    int nRevLen = 0;

    TCPTHREADSTATUS statusTCP = TCP_THREAD_STATUS_START;

    while (TRUE)
    {
        if (INVALID_SOCKET == etInfo->cfgEthernet.local.sct) { break; }

        switch (statusTCP) {
            case TCP_THREAD_STATUS_START: {
                socketClient = accept(etInfo->cfgEthernet.local.sct, (SOCKADDR*)(&addrRemote), &nAddrSize);
                if (INVALID_SOCKET != socketClient)
                {
                    SetConnectSuccessfullyText(pText, ((HANSIZE)ETHERNETTOOL_BUF_SIZE * (HANSIZE)3) + (HANSIZE)1, &addrLocal, &addrRemote);
                    EditAppendText(etInfo->printData.printData.hText, pText, FALSE);
                    etInfo->cfgEthernet.remote.sct = socketClient;
                    statusTCP = TCP_THREAD_STATUS_RECV;
                }
            } break;
            case TCP_THREAD_STATUS_RECV: {
                nRevLen = recv(socketClient, pBuf, sizeof(pBuf), 0);
                if (0 < nRevLen)
                {
                    PrintSocketData(etInfo, pBuf, nRevLen);
                }
                else if (0 == nRevLen)
                {
                    statusTCP = TCP_THREAD_STATUS_END;
                }
                else { }
            } break;
            case TCP_THREAD_STATUS_END: {
                EditAppendText(etInfo->printData.printData.hText, TEXT("\r\n断开连接\r\n\r\n"), FALSE);
                statusTCP = TCP_THREAD_STATUS_START;
            } break;

            default: { } break;
        }
    }

    return nRet;
}
static DWORD TCPClientThread(PETHERNETTOOLEXTRA etInfo)
{
    DWORD nRet = 0;
    SOCKET socketServer;
    SOCKADDR_IN addrRemote = etInfo->cfgEthernet.remote.sockAddr;
    SOCKADDR_IN addrLocal;
    int nAddrSize = sizeof(SOCKADDR_IN);
    PCHAR pBuf = etInfo->revData.pBuf;
    HANPSTR pText = etInfo->revData.pText;
    int nRevLen = 0;

    TCPTHREADSTATUS statusTCP = TCP_THREAD_STATUS_START;

    while (TRUE)
    {
        socketServer = etInfo->cfgEthernet.remote.sct;
        
        switch (statusTCP) {
            case TCP_THREAD_STATUS_START: {
                socketServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                if (INVALID_SOCKET == socketServer)
                {
                    HAN_snprintf(pText, ((HANSIZE)ETHERNETTOOL_BUF_SIZE * (HANSIZE)3) + (HANSIZE)1, TEXT("打开socket失败：%d"), WSAGetLastError());
                    pText[ETHERNETTOOL_BUF_SIZE * 3] = TEXT('\0');
                    MessageBox(etInfo->hConsole, pText, TEXT("TCP客户端错误"), 0);
                }
                else
                {
                    etInfo->cfgEthernet.remote.sct = socketServer;
                    EditAppendText(etInfo->printData.printData.hText, TEXT("连接中...\r\n"), FALSE);
                    if (SOCKET_ERROR != connect(socketServer, (SOCKADDR*)&addrRemote, nAddrSize))
                    {
                        getsockname(socketServer, (SOCKADDR*)&addrLocal, &nAddrSize);
                        SetConnectSuccessfullyText(pText, (ETHERNETTOOL_BUF_SIZE * 3) + 1, &addrLocal, &addrRemote);
                        EditAppendText(etInfo->printData.printData.hText, pText, FALSE);
                        statusTCP = TCP_THREAD_STATUS_RECV;
                    }
                    else
                    {
                        // 有可能是用户关闭端口导致的连接中断，重新检测socket
                        socketServer = etInfo->cfgEthernet.remote.sct;
                        if (INVALID_SOCKET != socketServer)
                        {
                            EditAppendText(etInfo->printData.printData.hText, TEXT("连接中...\r\n"), FALSE);
                        }
                    }
                }
            } break;
            case TCP_THREAD_STATUS_RECV: {
                nRevLen = recv(etInfo->cfgEthernet.remote.sct, pBuf, sizeof(pBuf), 0);
                if (0 < nRevLen)
                {
                    PrintSocketData(etInfo, pBuf, nRevLen);
                }
                else if (0 == nRevLen)
                {
                    statusTCP = TCP_THREAD_STATUS_END;
                }
                else { }
            } break;
            case TCP_THREAD_STATUS_END: {
                EditAppendText(etInfo->printData.printData.hText, TEXT("\r\n断开连接\r\n\r\n"), FALSE);
                CloseEthernetToolSocket(&(etInfo->cfgEthernet.remote.sct));
                statusTCP = TCP_THREAD_STATUS_START;
                continue;
            } break;

            default: { } break;
        }

        if (INVALID_SOCKET == socketServer) { break; }
    }

    return nRet;
}
static DWORD UDPThread(PETHERNETTOOLEXTRA etInfo)
{
    DWORD nRet = 0;
    SOCKADDR_IN sockAddr;
    int nAddrSize = sizeof(sockAddr);
    char pBuf[ETHERNETTOOL_BUF_SIZE];
    HANCHAR pText[ETHERNETTOOL_BUF_SIZE * 3];
    int nRevLen = 0;

    while (TRUE)
    {
        if (INVALID_SOCKET == etInfo->cfgEthernet.local.sct) { break; }

        nRevLen = recvfrom(etInfo->cfgEthernet.local.sct, pBuf, sizeof(pBuf), 0, (SOCKADDR*)(&sockAddr), &nAddrSize);
        if (0 < nRevLen)
        {
            PrintSocketData(etInfo, pBuf, nRevLen);
        }
        else
        {
            HAN_snprintf(pText, ArrLen(pText), TEXT("发生错误：%d\r\n"), WSAGetLastError());
            EditAppendText(etInfo->printData.printData.hText, pText, FALSE);
        }
    }

    return nRet;
}

static void CfgWindowToTextLocalIPv4(PETHERNETTOOLEXTRA etInfo, HANPSTR pText)
{
    GetIPv4Value(&(etInfo->cfgEthernet.local));
    HAN_snprintf(pText, INI_ETHERNET_TOOL_VALUE_SIZE, TEXT("%u.%u.%u.%u"),
        etInfo->cfgEthernet.local.pIPv4[0],
        etInfo->cfgEthernet.local.pIPv4[1],
        etInfo->cfgEthernet.local.pIPv4[2],
        etInfo->cfgEthernet.local.pIPv4[3]
    );
}
static void CfgWindowToTextLocalPort(PETHERNETTOOLEXTRA etInfo, HANPSTR pText)
{
    HANCHAR pTempText[16];

    GetWindowText(etInfo->cfgEthernet.local.hPort, pTempText, ArrLen(pTempText));
    etInfo->cfgEthernet.local.nPort = (ETHERNETPORT)HAN_strtoul(pTempText, NULL, 10);
    HAN_snprintf(pText, INI_ETHERNET_TOOL_VALUE_SIZE, TEXT("%u"), etInfo->cfgEthernet.local.nPort);
}
static void CfgWindowToTextRemoteIPv4(PETHERNETTOOLEXTRA etInfo, HANPSTR pText)
{
    DWORD nIPv4;

    SendMessage(etInfo->cfgEthernet.remote.hIPv4, IPM_GETADDRESS, 0, (LPARAM)(&nIPv4));

    etInfo->cfgEthernet.remote.pIPv4[0] = FIRST_IPADDRESS(nIPv4);
    etInfo->cfgEthernet.remote.pIPv4[1] = SECOND_IPADDRESS(nIPv4);
    etInfo->cfgEthernet.remote.pIPv4[2] = THIRD_IPADDRESS(nIPv4);
    etInfo->cfgEthernet.remote.pIPv4[3] = FOURTH_IPADDRESS(nIPv4);
    HAN_snprintf(pText, INI_ETHERNET_TOOL_VALUE_SIZE, TEXT("%u.%u.%u.%u"),
        etInfo->cfgEthernet.remote.pIPv4[0],
        etInfo->cfgEthernet.remote.pIPv4[1],
        etInfo->cfgEthernet.remote.pIPv4[2],
        etInfo->cfgEthernet.remote.pIPv4[3]
    );
}
static void CfgWindowToTextRemotePort(PETHERNETTOOLEXTRA etInfo, HANPSTR pText)
{
    HANCHAR pTempText[16];

    GetWindowText(etInfo->cfgEthernet.remote.hPort, pTempText, ArrLen(pTempText));
    etInfo->cfgEthernet.remote.nPort = (ETHERNETPORT)HAN_strtoul(pTempText, NULL, 10);
    HAN_snprintf(pText, INI_ETHERNET_TOOL_VALUE_SIZE, TEXT("%u"), etInfo->cfgEthernet.remote.nPort);
}
static void CfgWindowToTextProtocol(PETHERNETTOOLEXTRA etInfo, HANPSTR pText)
{
    ETHERNETTOOLPROTOCOLID idProtocol = ComboBoxGetCursel(etInfo->cfgEthernet.protocol.hWnd);
    
    HAN_strcpy(pText, sg_pEthernetProtocol[idProtocol].pName);
}
static void CfgWindowToTextHex(PETHERNETTOOLEXTRA etInfo, HANPSTR pText)
{
    HANINT bCheck = ButtonGetCheck(etInfo->printHex.hHex);

    if (BST_CHECKED == bCheck) { HAN_snprintf(pText, INI_COM_TOOL_VALUE_SIZE, TEXT("TRUE")); }
    else { HAN_snprintf(pText, INI_COM_TOOL_VALUE_SIZE, TEXT("FALSE")); }
}

static void CfgTextToWindowLocalIPv4(PETHERNETTOOLEXTRA etInfo, HANPCSTR pText)
{
    HANPSTR pStr;
    DWORD nIPv4;

    memcpy(&pStr, &pText, sizeof(pStr));

    for (HANSIZE iLoop = 0; iLoop < 4; iLoop++)
    {
        etInfo->cfgEthernet.local.pIPv4[iLoop] = (uint8_t)HAN_strtoul(pStr, &pStr, 10);
        while (!HAN_isdigit(pStr[0])) { pStr = &pStr[1]; }
    }

    nIPv4 = MAKEIPADDRESS(
        etInfo->cfgEthernet.local.pIPv4[0],
        etInfo->cfgEthernet.local.pIPv4[1],
        etInfo->cfgEthernet.local.pIPv4[2],
        etInfo->cfgEthernet.local.pIPv4[3]
    );
    SendMessage(etInfo->cfgEthernet.local.hIPv4, IPM_SETADDRESS, 0, (LPARAM)nIPv4);
}
static void CfgTextToWindowLocalPort(PETHERNETTOOLEXTRA etInfo, HANPCSTR pText)
{
    HANCHAR pTempText[INI_ETHERNET_TOOL_VALUE_SIZE];

    etInfo->cfgEthernet.local.nPort = (ETHERNETPORT)HAN_strtoul(pText, NULL, 10);
    HAN_snprintf(pTempText, INI_ETHERNET_TOOL_VALUE_SIZE, TEXT("%u"), etInfo->cfgEthernet.local.nPort);
    pTempText[INI_ETHERNET_TOOL_VALUE_SIZE - 1] = TEXT('\0');
    SetWindowText(etInfo->cfgEthernet.local.hPort, pTempText);
}
static void CfgTextToWindowRemoteIPv4(PETHERNETTOOLEXTRA etInfo, HANPCSTR pText)
{
    HANPSTR pStr;
    DWORD nIPv4;

    memcpy(&pStr, &pText, sizeof(pStr));

    for (HANSIZE iLoop = 0; iLoop < 4; iLoop++)
    {
        etInfo->cfgEthernet.remote.pIPv4[iLoop] = (uint8_t)HAN_strtoul(pStr, &pStr, 10);
        while (!HAN_isdigit(pStr[0])) { pStr = &pStr[1]; }
    }

    nIPv4 = MAKEIPADDRESS(
        etInfo->cfgEthernet.remote.pIPv4[0],
        etInfo->cfgEthernet.remote.pIPv4[1],
        etInfo->cfgEthernet.remote.pIPv4[2],
        etInfo->cfgEthernet.remote.pIPv4[3]
    );
    SendMessage(etInfo->cfgEthernet.remote.hIPv4, IPM_SETADDRESS, 0, (LPARAM)nIPv4);
}
static void CfgTextToWindowRemotePort(PETHERNETTOOLEXTRA etInfo, HANPCSTR pText)
{
    HANCHAR pTempText[INI_ETHERNET_TOOL_VALUE_SIZE];

    etInfo->cfgEthernet.remote.nPort = (ETHERNETPORT)HAN_strtoul(pText, NULL, 10);
    HAN_snprintf(pTempText, INI_ETHERNET_TOOL_VALUE_SIZE, TEXT("%u"), etInfo->cfgEthernet.remote.nPort);
    pTempText[INI_ETHERNET_TOOL_VALUE_SIZE - 1] = TEXT('\0');
    SetWindowText(etInfo->cfgEthernet.remote.hPort, pTempText);
}
static void CfgTextToWindowProtocol(PETHERNETTOOLEXTRA etInfo, HANPCSTR pText)
{
    HANINT iCursel = ComboBoxFindString(etInfo->cfgEthernet.protocol.hWnd, 0, pText);

    if (iCursel == CB_ERR) { iCursel = g_etDefaultCfg.cfgEthernet.protocol.eId; }
    ComboBoxSetCursel(etInfo->cfgEthernet.protocol.hWnd, iCursel);
    SetIPPortEnable(etInfo);
}
static void CfgTextToWindowHex(PETHERNETTOOLEXTRA etInfo, HANPCSTR pText)
{
    if (0 == HAN_strcmp(pText, TEXT("TRUE"))) { ButtonSetChecked(etInfo->printHex.hHex); }
    else { ButtonSetUnchecked(etInfo->printHex.hHex); }
}
