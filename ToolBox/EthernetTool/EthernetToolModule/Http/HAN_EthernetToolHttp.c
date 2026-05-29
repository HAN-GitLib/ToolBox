#include <winsock2.h>
#include <ws2def.h>
#include <Windows.h>
#include <CommCtrl.h>
#include <iphlpapi.h>
#include <winhttp.h>
#include <WS2tcpip.h>
#include <shlwapi.h>

#include "HAN_EthernetToolHttp.h"

#define HAN_ETHERNET_TOOL_HTTP_CLASS            TEXT("HAN_EthernetToolHttp")

#define HAN_ETHERNET_TOOL_HTTP_ERROR_CAPTION    TEXT("Http错误")

#define HAN_ETHERNET_TOOL_HTTP_DEFAULT_SCHEME   L"https"

typedef enum {
    HTTP_SERVER_TYPE_HTTP,
    HTTP_SERVER_TYPE_HTTPS,
    HTTP_SERVER_TYPE_CNT,
} HTTPSERVERTYPE;

typedef enum tagHTTPCODE {
    HTTP_CODE_UTF8,
    HTTP_CODE_GBK,
    HTTP_CODE_CNT_MAX,
} HTTPCODE;

typedef struct tagHTTPSERVER {
    HANPCSTR                pScheme;
    HANPCSTR                pPort;
} HTTPSERVER;

typedef struct tagHTTPBUF {
    void*                   pBuf;
    HANSIZE                 nLen;
} HTTPBUF;
typedef struct tagURLBUF {
    HANSIZE                 nLen;
    struct {
        HANPSTR             pInput;
        HANPSTR             pScheme;
        HANPSTR             pHostName;
        HANPSTR             pUrlPath;
        HANPSTR             pExtraInfo;
        HANPSTR             pObjectName;
    } hanStr;
    struct {
        PWCHAR              pInput;
        PWCHAR              pScheme;
        PWCHAR              pHostName;
        PWCHAR              pUrlPath;
        PWCHAR              pExtraInfo;
        PWCHAR              pObjectName;
    } wStr;
    uint8_t                 pBuf[];
} URLBUF, * PURLBUF;

typedef struct tagETHERNETTOOLHTTPEXTRA {
    HANDLE                  hHeap;
    HINSTANCE               hInst;
    HWND                    hSelf;
    PETHERNETTOOLEXTRA      etInfo;
    HMODULE                 hHttpModule;
    struct {
        HWND                hUrlToIp;
        HWND                hIP;
    } ip;
    struct {
        HWND                hTitle;
        HWND                hInput;
        PURLBUF             pBuf;
    } url;
    HWND                    hGet;
    struct {
        HWND                hText;
        HTTPBUF             bufWideChar;
        HTTPBUF             bufMultiLine;
        HTTPBUF             bufWindowChar;
    } printData;
    HTTPBUF                 rev;
    HTTPBUF                 header;
    struct {
        HFONT               hHex;
        HFONT               hSys;
    } hFont;                /* 字体 */
} ETHERNETTOOLHTTPEXTRA, * PETHERNETTOOLHTTPEXTRA;

typedef struct tagHTTPCODEINFO {
    PCSTR                   pNameA;
    PCWSTR                  pNameW;
    UINT                    codePage;
} HTTPCODEINFO;

static LRESULT CALLBACK EthernetToolHttpWndProc(HWND hEthernetToolHttp, UINT message, WPARAM wParam, LPARAM lParam);

static LRESULT CreateCallback(HWND hEthernetToolHttp, LPARAM lParam);
static void CommandCallback(HWND hEthernetToolHttp, PETHERNETTOOLHTTPEXTRA httpInfo, WPARAM wParam, LPARAM lParam);
static void SizeCallback(PETHERNETTOOLHTTPEXTRA httpInfo);
static void DestroyCallback(PETHERNETTOOLHTTPEXTRA httpInfo);
static void UrlToIpCallback(PETHERNETTOOLHTTPEXTRA httpInfo);
static void GetCallback(PETHERNETTOOLHTTPEXTRA httpInfo);
static PURLBUF GetUrlFromWindow(PETHERNETTOOLHTTPEXTRA httpInfo);
static BOOL UpdateUrlBuf(PURLBUF pUrlBuf);
static BOOL GetIpByHostName(PETHERNETTOOLHTTPEXTRA httpInfo, HANPCSTR pHostName);
static HTTPCODE GetHttpCode(PCWSTR pHeader, PCSTR pHttp);
static BOOL DecodeHttp(PETHERNETTOOLHTTPEXTRA httpInfo);

static void MessageBoxPrintLastError(PETHERNETTOOLHTTPEXTRA httpInfo);

static DWORD GetRequestThread(PETHERNETTOOLHTTPEXTRA httpInfo);

static void* AllocHttp(void* pOldData, size_t nNewSize, void* pParam);
static void FreeHttpListNode(void* pData, void* pParam);

static const ETHERNETTOOLHTTPEXTRA  sg_httpDefaultCfg = {
    .url = {
        .pBuf = NULL,
    },
    .rev = {
        .pBuf = NULL,
        .nLen = 0,
    },
    .header = {
        .pBuf = NULL,
        .nLen = 0,
    },
    .printData = {
        .bufWideChar = {
            .pBuf = NULL,
            .nLen = 0,
        },
        .bufWindowChar = {
            .pBuf = NULL,
            .nLen = 0,
        },
    },
};
static const HTTPCODEINFO sg_httpCodeInfo[HTTP_CODE_CNT_MAX] = {
    [HTTP_CODE_UTF8] = {
        .pNameA = "utf-8",
        .pNameW = L"utf-8",
        .codePage = CP_UTF8,
    },
    [HTTP_CODE_GBK] = {
        .pNameA = "gbk",
        .pNameW = L"gbk",
        .codePage = CP_ACP,
    },
};

void RegisterHANEthernetToolHttp(HINSTANCE hInst)
{
    WNDCLASSEX wcex = {
        .cbSize         = sizeof(WNDCLASSEX),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = EthernetToolHttpWndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = sizeof(PETHERNETTOOLHTTPEXTRA),
        .hInstance      = hInst,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName   = NULL,
        .lpszClassName  = HAN_ETHERNET_TOOL_HTTP_CLASS,
        .hIconSm        = NULL,
    };
    RegisterClassEx(&wcex);
}

void CreateEthernetToolHttpWindow(PETHERNETTOOLEXTRA etInfo)
{
    BOOL bOK = TRUE;
    HANALLOCDATA adAllocGraphic = {
        .AllocOperation = AllocHttp,
        .pParam = etInfo->hHeap,
    };
    HWND hHttp = CreateWindow(HAN_ETHERNET_TOOL_HTTP_CLASS, TEXT("http/https"),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 1000, 500, NULL, NULL, etInfo->hInst, etInfo);

    if (NULL != hHttp)
    {
        if (RET_OK != HANListAppend(&(etInfo->listChildModel), hHttp, &adAllocGraphic))
        {
            bOK = FALSE;
            DestroyWindow(hHttp);
        }
    }
    else { bOK = FALSE; }
    if (FALSE == bOK)
    {
        MessageBox(etInfo->hSelf, TEXT("创建窗口失败"), NULL, 0);
    }
}

static LRESULT CALLBACK EthernetToolHttpWndProc(HWND hEthernetToolHttp, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    PETHERNETTOOLHTTPEXTRA httpInfo = (PETHERNETTOOLHTTPEXTRA)GetWindowLongPtr(hEthernetToolHttp, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = CreateCallback(hEthernetToolHttp, lParam);
        } break;
        case WM_COMMAND: {
            CommandCallback(hEthernetToolHttp, httpInfo, wParam, lParam);
        } break;
        case WM_SIZE: {
            SizeCallback(httpInfo);
        } break;
        case WM_CTLCOLORSTATIC: {
            lWndProcRet = (INT_PTR)GetStockObject(WHITE_BRUSH);
        } break;
        case WM_DESTROY: {
            DestroyCallback(httpInfo);
            lWndProcRet = DefWindowProc(hEthernetToolHttp, message, wParam, lParam);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hEthernetToolHttp, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}

static LRESULT CreateCallback(HWND hEthernetToolHttp, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PETHERNETTOOLHTTPEXTRA httpInfo;
    PETHERNETTOOLEXTRA etInfo = ((LPCREATESTRUCT)lParam)->lpCreateParams;
    HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;

    HANDLE hHeap = GetProcessHeap();
    if (NULL == hHeap) { lWndProcRet = -1; }
    if (-1 != lWndProcRet)
    {
        httpInfo = (PETHERNETTOOLHTTPEXTRA)HANWinHeapAlloc(hHeap, NULL, sizeof(ETHERNETTOOLHTTPEXTRA));
        if (NULL == httpInfo) { lWndProcRet = -1; }
    }

    if (-1 != lWndProcRet)
    {
        int nWinX = ETHERNETTOOL_WINDOW_DX;
        int nWinY = ETHERNETTOOL_WINDOW_DY;

        SetWindowLongPtr(hEthernetToolHttp, 0, (LONG_PTR)httpInfo);

        *httpInfo = sg_httpDefaultCfg;
        httpInfo->hHeap = hHeap;
        httpInfo->hSelf = hEthernetToolHttp;
        httpInfo->etInfo = etInfo;

        httpInfo->hHttpModule = LoadLibrary(TEXT("winhttp.dll"));

        httpInfo->hFont.hHex = CreateFontIndirect(&g_lfHexFont);
        httpInfo->hFont.hSys = CreateFontIndirect(&g_lfSysFont);

        nWinX = ETHERNETTOOL_HTTP_URL_TITLE_X;
        httpInfo->url.hTitle = CreateWindow(TEXT("static"), TEXT("网址"),
            WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_CENTERIMAGE,
            nWinX, nWinY, ETHERNETTOOL_HTTP_URL_TITLE_W, ETHERNETTOOL_WINDOW_TEXT_HEIGHT,
            hEthernetToolHttp, (HMENU)WID_ETHERNET_TOOL_HTTP_URL_TITLE, hInst, NULL);
        nWinX = ETHERNETTOOL_HTTP_URL_INPUT_X;
        httpInfo->url.hInput = CreateWindow(TEXT("edit"), TEXT(""),
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            nWinX, nWinY, ETHERNETTOOL_HTTP_URL_INPUT_W, ETHERNETTOOL_WINDOW_TEXT_HEIGHT,
            hEthernetToolHttp, (HMENU)WID_ETHERNET_TOOL_HTTP_URL_INPUT, hInst, NULL);

        nWinX += ETHERNETTOOL_HTTP_URL_INPUT_W + ETHERNETTOOL_WINDOW_DX;
        httpInfo->ip.hUrlToIp = CreateWindow(TEXT("button"), TEXT("→"),
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, nWinX, nWinY, 25, ETHERNETTOOL_WINDOW_TEXT_HEIGHT,
            hEthernetToolHttp, (HMENU)WID_ETHERNET_TOOL_HTTP_URL_TO_IP_BUTTON, hInst, NULL);
        nWinX += 25 + ETHERNETTOOL_WINDOW_DX;
        httpInfo->ip.hIP = CreateWindow(WC_IPADDRESS, TEXT(""),
            WS_CHILD | WS_VISIBLE, nWinX, nWinY, ETHERNETTOOL_IPV4_W, ETHERNETTOOL_WINDOW_TEXT_HEIGHT,
            hEthernetToolHttp, (HMENU)WID_ETHERNET_TOOL_HTTP_IP, hInst, NULL);

        nWinX = ETHERNETTOOL_WINDOW_DX;
        nWinY += ETHERNETTOOL_WINDOW_DY + ETHERNETTOOL_WINDOW_TEXT_HEIGHT;
        httpInfo->hGet = CreateWindow(TEXT("button"), TEXT("GET"),
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, nWinX, nWinY, 100, 30,
            hEthernetToolHttp, (HMENU)WID_ETHERNET_TOOL_HTTP_GET, hInst, NULL);
        
        nWinX = ETHERNETTOOL_HTTP_TEXT_X;
        nWinY = ETHERNETTOOL_HTTP_TEXT_Y;
        httpInfo->printData.hText = CreateWindow(TEXT("edit"), NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER | WS_HSCROLL | WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY,
            nWinX, nWinY, ETHERNETTOOL_HTTP_TEXT_MIN_W, ETHERNETTOOL_HTTP_TEXT_MIN_H,
            hEthernetToolHttp, (HMENU)WID_ETHERNET_TOOL_HTTP_TEXT, hInst, NULL);

        EnableWindow(httpInfo->ip.hIP, FALSE);

        SendMessage(httpInfo->url.hTitle, WM_SETFONT, (WPARAM)(httpInfo->hFont.hSys), (LPARAM)TRUE);
        SendMessage(httpInfo->url.hInput, WM_SETFONT, (WPARAM)(httpInfo->hFont.hSys), (LPARAM)TRUE);
        SendMessage(httpInfo->ip.hUrlToIp, WM_SETFONT, (WPARAM)(httpInfo->hFont.hSys), (LPARAM)TRUE);
        SendMessage(httpInfo->ip.hIP, WM_SETFONT, (WPARAM)(httpInfo->hFont.hSys), (LPARAM)TRUE);
        SendMessage(httpInfo->hGet, WM_SETFONT, (WPARAM)(httpInfo->hFont.hSys), (LPARAM)TRUE);
        SendMessage(httpInfo->printData.hText, WM_SETFONT, (WPARAM)(httpInfo->hFont.hSys), (LPARAM)TRUE);
        
        SizeCallback(httpInfo);
    }

    return lWndProcRet;
}
static void CommandCallback(HWND hEthernetToolHttp, PETHERNETTOOLHTTPEXTRA httpInfo, WPARAM wParam, LPARAM lParam)
{
    (void)hEthernetToolHttp;
    (void)lParam;
    
    switch (LOWORD(wParam)) {
        case WID_ETHERNET_TOOL_HTTP_URL_TO_IP_BUTTON: {
            UrlToIpCallback(httpInfo);
        } break;
        case WID_ETHERNET_TOOL_HTTP_GET: {
            GetCallback(httpInfo);
        } break;

        default: { } break;
    }
}
static void SizeCallback(PETHERNETTOOLHTTPEXTRA httpInfo)
{
    RECT rcClientSize;
    HANINT nWinW;
    HANINT nWinH;
    
    GetClientRect(httpInfo->hSelf, &rcClientSize);
    nWinW = GetRectW(&rcClientSize) - (ETHERNETTOOL_HTTP_TEXT_X + ETHERNETTOOL_WINDOW_DX);
    nWinH = GetRectH(&rcClientSize) - (ETHERNETTOOL_HTTP_TEXT_Y + ETHERNETTOOL_WINDOW_DY);

    if (nWinW < ETHERNETTOOL_HTTP_TEXT_MIN_W) { nWinW = ETHERNETTOOL_HTTP_TEXT_MIN_W; }
    if (nWinH < ETHERNETTOOL_HTTP_TEXT_MIN_H) { nWinH = ETHERNETTOOL_HTTP_TEXT_MIN_H; }

    MoveWindow(httpInfo->printData.hText,
        ETHERNETTOOL_HTTP_TEXT_X, ETHERNETTOOL_HTTP_TEXT_Y,
        nWinW, nWinH,
        TRUE);
}
static void DestroyCallback(PETHERNETTOOLHTTPEXTRA httpInfo)
{
    HANPLIST pList = &(httpInfo->etInfo->listChildModel);
    HANFREEDATA fdFreeHttpListNode = {
        .FreeOperation = FreeHttpListNode,
        .pParam = httpInfo->hHeap,
    };

    for (size_t iLoop = 0; iLoop < HANListGetLen(pList); iLoop++)
    {
        if (HANListGetData(pList, iLoop) == httpInfo->hSelf)
        {
            HANListDelete(pList, iLoop, &fdFreeHttpListNode, NULL);
            break;
        }
    }
    HANWinHeapFree(httpInfo->hHeap, 0, httpInfo->header.pBuf);
    HANWinHeapFree(httpInfo->hHeap, 0, httpInfo->rev.pBuf);
    HANWinHeapFree(httpInfo->hHeap, 0, httpInfo->printData.bufWideChar.pBuf);
    HANWinHeapFree(httpInfo->hHeap, 0, httpInfo->printData.bufMultiLine.pBuf);
    HANWinHeapFree(httpInfo->hHeap, 0, httpInfo->printData.bufWindowChar.pBuf);
    HANWinHeapFree(httpInfo->hHeap, 0, httpInfo->url.pBuf);
    HANWinHeapFree(httpInfo->hHeap, 0, httpInfo);
}
static void UrlToIpCallback(PETHERNETTOOLHTTPEXTRA httpInfo)
{
    PURLBUF pUrlBuf = GetUrlFromWindow(httpInfo);
    
    if (NULL != pUrlBuf)
    {
        if (TRUE == GetIpByHostName(httpInfo, pUrlBuf->hanStr.pHostName)) { }
        else { MessageBox(httpInfo->hSelf, TEXT("无法解析网址"), HAN_ETHERNET_TOOL_HTTP_ERROR_CAPTION, 0); }
    }
}
static void GetCallback(PETHERNETTOOLHTTPEXTRA httpInfo)
{
    (void)GetRequestThread(httpInfo);
}
static PURLBUF GetUrlFromWindow(PETHERNETTOOLHTTPEXTRA httpInfo)
{
    PURLBUF pUrlBuf = NULL;
    BOOL bAlloc = FALSE;
    int urlLen;
    HANSIZE nOffset = 0;

    urlLen = GetWindowTextLength(httpInfo->url.hInput) + 1;

    if (NULL == httpInfo->url.pBuf)
    {
        bAlloc = TRUE;
    }
    else {
        if ((int)(httpInfo->url.pBuf->nLen) < urlLen) { bAlloc = TRUE; }
        else { bAlloc = FALSE; pUrlBuf = httpInfo->url.pBuf; }
    }

    if (TRUE == bAlloc)
    {
        HANSIZE nBufSize = sizeof(URLBUF) + (
            sizeof(HANCHAR) * sizeof(pUrlBuf->hanStr) / sizeof(void*) + 
            sizeof(WCHAR) * sizeof(pUrlBuf->wStr) / sizeof(void*))
             * urlLen;
        pUrlBuf = (PURLBUF)HANWinHeapAlloc(httpInfo->hHeap, httpInfo->url.pBuf, nBufSize);
        if (NULL != pUrlBuf)
        {
            pUrlBuf->nLen = urlLen;

            pUrlBuf->hanStr.pInput = (HANPSTR)&(pUrlBuf->pBuf[nOffset]); nOffset += sizeof(HANCHAR) * urlLen;
            pUrlBuf->hanStr.pScheme = (HANPSTR)&(pUrlBuf->pBuf[nOffset]); nOffset += sizeof(HANCHAR) * urlLen;
            pUrlBuf->hanStr.pHostName = (HANPSTR)&(pUrlBuf->pBuf[nOffset]); nOffset += sizeof(HANCHAR) * urlLen;
            pUrlBuf->hanStr.pUrlPath = (HANPSTR)&(pUrlBuf->pBuf[nOffset]); nOffset += sizeof(HANCHAR) * urlLen;
            pUrlBuf->hanStr.pExtraInfo = (HANPSTR)&(pUrlBuf->pBuf[nOffset]); nOffset += sizeof(HANCHAR) * urlLen;
            pUrlBuf->hanStr.pObjectName = (HANPSTR)&(pUrlBuf->pBuf[nOffset]); nOffset += sizeof(HANCHAR) * urlLen;

            pUrlBuf->wStr.pInput = (PWCHAR)&(pUrlBuf->pBuf[nOffset]); nOffset += sizeof(WCHAR) * urlLen;
            pUrlBuf->wStr.pScheme = (PWCHAR)&(pUrlBuf->pBuf[nOffset]); nOffset += sizeof(WCHAR) * urlLen;
            pUrlBuf->wStr.pHostName = (PWCHAR)&(pUrlBuf->pBuf[nOffset]); nOffset += sizeof(WCHAR) * urlLen;
            pUrlBuf->wStr.pUrlPath = (PWCHAR)&(pUrlBuf->pBuf[nOffset]); nOffset += sizeof(WCHAR) * urlLen;
            pUrlBuf->wStr.pExtraInfo = (PWCHAR)&(pUrlBuf->pBuf[nOffset]); nOffset += sizeof(WCHAR) * urlLen;
            pUrlBuf->wStr.pObjectName = (PWCHAR)&(pUrlBuf->pBuf[nOffset]); nOffset += sizeof(WCHAR) * urlLen;
            
            httpInfo->url.pBuf = pUrlBuf;
        }
    }

    if (NULL != pUrlBuf)
    {
        GetWindowText(httpInfo->url.hInput, pUrlBuf->hanStr.pInput, urlLen);
        HANStrToWStr(CP_ACP, 0, pUrlBuf->hanStr.pInput, -1, pUrlBuf->wStr.pInput, (HANINT)(pUrlBuf->nLen));

        if (FALSE == UpdateUrlBuf(pUrlBuf))
        {
            wcscpy(pUrlBuf->wStr.pHostName, pUrlBuf->wStr.pInput);
            wcscpy(pUrlBuf->wStr.pScheme, HAN_ETHERNET_TOOL_HTTP_DEFAULT_SCHEME);
            wcscpy(pUrlBuf->wStr.pUrlPath, L"");
            wcscpy(pUrlBuf->wStr.pExtraInfo, L"");
        }

        wcscpy(pUrlBuf->wStr.pObjectName, pUrlBuf->wStr.pUrlPath);
        if (L'#' != pUrlBuf->wStr.pExtraInfo[0]) { wcscat(pUrlBuf->wStr.pObjectName, pUrlBuf->wStr.pExtraInfo); }

        WStrToHANStr(CP_ACP, 0, pUrlBuf->wStr.pScheme, -1, pUrlBuf->hanStr.pScheme, (HANINT)(pUrlBuf->nLen), NULL, NULL);
        WStrToHANStr(CP_ACP, 0, pUrlBuf->wStr.pHostName, -1, pUrlBuf->hanStr.pHostName, (HANINT)(pUrlBuf->nLen), NULL, NULL);
        WStrToHANStr(CP_ACP, 0, pUrlBuf->wStr.pUrlPath, -1, pUrlBuf->hanStr.pUrlPath, (HANINT)(pUrlBuf->nLen), NULL, NULL);
        WStrToHANStr(CP_ACP, 0, pUrlBuf->wStr.pExtraInfo, -1, pUrlBuf->hanStr.pExtraInfo, (HANINT)(pUrlBuf->nLen), NULL, NULL);
        WStrToHANStr(CP_ACP, 0, pUrlBuf->wStr.pObjectName, -1, pUrlBuf->hanStr.pObjectName, (HANINT)(pUrlBuf->nLen), NULL, NULL);
    }

    return pUrlBuf;
}
static BOOL UpdateUrlBuf(PURLBUF pUrlBuf)
{
    BOOL bRet;
    URL_COMPONENTS urlComp;

    memset(&urlComp, 0, sizeof(urlComp));
    urlComp.dwStructSize = sizeof(urlComp);
    urlComp.lpszScheme = pUrlBuf->wStr.pScheme;
    urlComp.dwSchemeLength = (DWORD)pUrlBuf->nLen;
    urlComp.lpszHostName = pUrlBuf->wStr.pHostName;
    urlComp.dwHostNameLength = (DWORD)pUrlBuf->nLen;
    urlComp.lpszUrlPath = pUrlBuf->wStr.pUrlPath;
    urlComp.dwUrlPathLength = (DWORD)pUrlBuf->nLen;
    urlComp.lpszExtraInfo = pUrlBuf->wStr.pExtraInfo;
    urlComp.dwExtraInfoLength = (DWORD)pUrlBuf->nLen;

    if (TRUE == WinHttpCrackUrl(pUrlBuf->wStr.pInput, 0, 0, &urlComp))
    {
        for (DWORD iLoop = 0; iLoop < urlComp.dwSchemeLength; iLoop++)
        {
            pUrlBuf->wStr.pScheme[iLoop] = towlower(pUrlBuf->wStr.pScheme[iLoop]);
        }
        bRet = TRUE;
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}
static BOOL GetIpByHostName(PETHERNETTOOLHTTPEXTRA httpInfo, HANPCSTR pHostName)
{
    BOOL bRet;
    ADDRINFOT aiAddr;
    ADDRINFOT* pAddr = NULL;
    PSOCKADDR_IN pSockAddr;
    ULONG nAddr;
    LPARAM lpAddr;
        
    memset(&aiAddr, 0, sizeof(aiAddr));
    aiAddr.ai_family = AF_INET;
    aiAddr.ai_socktype = SOCK_STREAM;
    aiAddr.ai_protocol = IPPROTO_TCP;

    if (0 == GetAddrInfo(pHostName, NULL, &aiAddr, &pAddr))
    {
        pSockAddr = (PSOCKADDR_IN)(pAddr->ai_addr);
        nAddr = pSockAddr->sin_addr.s_addr;
        lpAddr = MAKEIPADDRESS(
        FOURTH_IPADDRESS(nAddr),
        THIRD_IPADDRESS(nAddr),
        SECOND_IPADDRESS(nAddr),
        FIRST_IPADDRESS(nAddr)
        );
        SendMessage(httpInfo->ip.hIP, IPM_SETADDRESS, 0, lpAddr);
        bRet = TRUE;
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}
static HTTPCODE GetHttpCode(PCWSTR pHeader, PCSTR pHttp)
{
    HTTPCODE eRet = HTTP_CODE_CNT_MAX;

    for (HTTPCODE iLoop = 0; iLoop < HTTP_CODE_CNT_MAX; iLoop++)
    {
        if (NULL != StrStrIW(pHeader, sg_httpCodeInfo[iLoop].pNameW)) { eRet = iLoop; }
    }
    if (HTTP_CODE_CNT_MAX == eRet)
    {
        for (HTTPCODE iLoop = 0; iLoop < HTTP_CODE_CNT_MAX; iLoop++)
        {
            if (NULL != StrStrIA(pHttp, sg_httpCodeInfo[iLoop].pNameA)) { eRet = iLoop; }
        }
    }
    if (HTTP_CODE_CNT_MAX == eRet)
    {
        eRet = HTTP_CODE_UTF8;
    }

    return eRet;
}
static BOOL DecodeHttp(PETHERNETTOOLHTTPEXTRA httpInfo)
{
    BOOL bRet = TRUE;
    HTTPCODE eCode = GetHttpCode(httpInfo->header.pBuf, httpInfo->rev.pBuf);
    PWSTR pWideCharBuf = httpInfo->printData.bufWideChar.pBuf;
    PWSTR pMultiLineBuf = httpInfo->printData.bufMultiLine.pBuf;
    HANPSTR pWindowCharBuf = httpInfo->printData.bufWindowChar.pBuf;
    HANSIZE nWideCharLen = httpInfo->printData.bufWideChar.nLen;
    HANSIZE nMultiLineLen = httpInfo->printData.bufMultiLine.nLen;
    HANSIZE nWindowCharLen = httpInfo->printData.bufWindowChar.nLen;
    HANSIZE nOffset;
    int nBufLen;
    HANSIZE iLoop;

    if (HTTP_CODE_CNT_MAX <= eCode) { bRet = FALSE; }
    
    // 先把网页的编码转成 WCHAR
    if (TRUE == bRet)
    {
        nBufLen = HANStrToWStr(sg_httpCodeInfo[eCode].codePage, 0, httpInfo->rev.pBuf, -1, NULL, 0) + 1;
        if (nWideCharLen < (HANSIZE)nBufLen)
        {
            pWideCharBuf = HANWinHeapAlloc(httpInfo->hHeap, pWideCharBuf, sizeof(WCHAR) * nBufLen);
            if (NULL != pWideCharBuf)
            {
                httpInfo->printData.bufWideChar.pBuf = pWideCharBuf;
                httpInfo->printData.bufWideChar.nLen = nBufLen;
            }
            else { bRet = FALSE; }
        }
        if (NULL != pWideCharBuf)
        {
            HANStrToWStr(sg_httpCodeInfo[eCode].codePage, 0, httpInfo->rev.pBuf, -1, pWideCharBuf, nBufLen);
        }
    }
    // 把单独的 \n 替换成 \r\n
    if (TRUE == bRet)
    {
        nBufLen *= 2;
        if (nMultiLineLen < (HANSIZE)nBufLen)
        {
            pMultiLineBuf = HANWinHeapAlloc(httpInfo->hHeap, pMultiLineBuf, sizeof(WCHAR) * nBufLen);
            if (NULL != pMultiLineBuf)
            {
                httpInfo->printData.bufMultiLine.pBuf = pMultiLineBuf;
                httpInfo->printData.bufMultiLine.nLen = nBufLen;
            }
            else { bRet = FALSE; }
        }
        if (NULL != pMultiLineBuf)
        {
            nOffset = 0;
            iLoop = 0;
            while (L'\0' != pWideCharBuf[iLoop])
            {
                if (L'\r' == pWideCharBuf[iLoop])
                {
                    pMultiLineBuf[nOffset] = pWideCharBuf[iLoop]; nOffset++; iLoop++;
                    pMultiLineBuf[nOffset] = pWideCharBuf[iLoop]; nOffset++;
                    if (L'\0' != pWideCharBuf[iLoop]) { iLoop++; }
                }
                else if (L'\n' == pWideCharBuf[iLoop])
                {
                    pMultiLineBuf[nOffset] = L'\r'; nOffset++;
                    pMultiLineBuf[nOffset] = L'\n'; nOffset++; iLoop++;
                }
                else
                {
                    pMultiLineBuf[nOffset] = pWideCharBuf[iLoop]; nOffset++; iLoop++;
                }
            }
            if (nOffset < (HANSIZE)nBufLen) { pMultiLineBuf[nOffset] = L'\0'; }
        }
    }
    // 再把 WCHAR 转成窗口的编码
    if (TRUE == bRet)
    {
        nBufLen = WStrToHANStr(CP_ACP, 0, pMultiLineBuf, -1, NULL, 0, NULL, FALSE) + 1;
        if (nWindowCharLen < (HANSIZE)nBufLen)
        {
            pWindowCharBuf = HANWinHeapAlloc(httpInfo->hHeap, pWindowCharBuf, sizeof(HANCHAR) * nBufLen);
            if (NULL != pWindowCharBuf)
            {
                httpInfo->printData.bufWindowChar.pBuf = pWindowCharBuf;
                httpInfo->printData.bufWindowChar.nLen = nBufLen;
            }
            else { bRet = FALSE; }
        }
        if (NULL != pWindowCharBuf)
        {
            WStrToHANStr(CP_ACP, 0, pMultiLineBuf, -1, pWindowCharBuf, nBufLen, NULL, FALSE);
        }
    }

    return bRet;
}

static void MessageBoxPrintLastError(PETHERNETTOOLHTTPEXTRA httpInfo)
{
    DWORD nErr = GetLastError();
    DWORD nFormatMsgErr;
    void* pBuf = NULL;
    
    nFormatMsgErr = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_IGNORE_INSERTS,
        httpInfo->hHttpModule,
        nErr,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&pBuf,
        0, NULL);
    if (0 == nFormatMsgErr)
    {
        if (NULL != pBuf)
        {
            LocalFree(pBuf);
            pBuf = NULL;
        }
        if (ERROR_MR_MID_NOT_FOUND == GetLastError())
        {
            nFormatMsgErr = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE |FORMAT_MESSAGE_IGNORE_INSERTS,
                httpInfo->hHttpModule,
                nErr,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&pBuf,
                0, NULL);
        }
    }

    if (0 == nFormatMsgErr) { MessageBox(httpInfo->hSelf, TEXT("未知错误"), TEXT("Http错误"), 0); }
    else { MessageBox(httpInfo->hSelf, pBuf, HAN_ETHERNET_TOOL_HTTP_ERROR_CAPTION, 0); }
    
    if (NULL != pBuf)
    {
        LocalFree(pBuf);
        pBuf = NULL;
    }
}

static DWORD GetRequestThread(PETHERNETTOOLHTTPEXTRA httpInfo)
{
    DWORD nRet = 0;
    DWORD nRevLen;
    DWORD nTotRevLen;
    DWORD nOffset;
    DWORD nDownloadLen;
    DWORD nHeaderLen = 0;
    PURLBUF pUrl = GetUrlFromWindow(httpInfo);
    HINTERNET hSession = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;
    INTERNET_PORT nServerPort;
    DWORD reqFlag;
    BOOL bReqResult = FALSE;
    uint8_t* pBuf;

    if (NULL != pUrl)
    {
        // 打开 HTTP
        hSession = WinHttpOpen(NULL, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if (NULL != hSession) // 连接
        {
            if (!HAN_strcmp(TEXT("http"), pUrl->hanStr.pScheme)) { nServerPort = INTERNET_DEFAULT_HTTP_PORT; }
            else if (!HAN_strcmp(TEXT("https"), pUrl->hanStr.pScheme)) { nServerPort = INTERNET_DEFAULT_HTTPS_PORT; }
            else { nServerPort = INTERNET_DEFAULT_PORT; }
            hConnect = WinHttpConnect(hSession, pUrl->wStr.pHostName, nServerPort, 0);
        }
        if (NULL != hConnect) // 打开请求
        {
            if (INTERNET_DEFAULT_HTTPS_PORT == nServerPort) { reqFlag = WINHTTP_FLAG_SECURE; }
            else { reqFlag = 0; }
            hRequest = WinHttpOpenRequest(hConnect, L"GET", pUrl->wStr.pObjectName, NULL,
                WINHTTP_NO_REFERER,  WINHTTP_DEFAULT_ACCEPT_TYPES,  reqFlag);
        }
        if (NULL != hRequest) // header
        {
            bReqResult = TRUE;  // 预留，这里输出 TRUE 即可
        }
        if (FALSE != bReqResult) // 发送请求
        {
            bReqResult = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
        }
        if (FALSE != bReqResult) // 接收响应
        {
            bReqResult = WinHttpReceiveResponse(hRequest, NULL);
        }
        if (FALSE == bReqResult) { MessageBoxPrintLastError(httpInfo); } // 预操作完成，开始接收并解析响应内容
        else
        {
            bReqResult = WinHttpQueryHeaders(
                hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF,
                WINHTTP_HEADER_NAME_BY_INDEX, NULL,
                &nHeaderLen, WINHTTP_NO_HEADER_INDEX);
            pBuf = httpInfo->header.pBuf;
            if (httpInfo->header.nLen < nHeaderLen)
            {
                pBuf = (uint8_t*)HANWinHeapAlloc(httpInfo->hHeap, pBuf, nHeaderLen);
                if (NULL == pBuf)
                {
                    MessageBox(httpInfo->hSelf, TEXT("内存不足"), HAN_ETHERNET_TOOL_HTTP_ERROR_CAPTION, 0);
                    bReqResult = FALSE;
                }
                else
                {
                    httpInfo->header.pBuf = pBuf;
                    httpInfo->header.nLen = nHeaderLen;
                }
            }
            if (NULL != pBuf)
            {
                bReqResult = WinHttpQueryHeaders(
                    hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF,
                    WINHTTP_HEADER_NAME_BY_INDEX, pBuf,
                    &nHeaderLen, WINHTTP_NO_HEADER_INDEX);
            }
        }
        if (FALSE != bReqResult) /* 开始接收 */
        {
            bReqResult = TRUE;
            nTotRevLen = 0; // 累计接收数据
            do {
                nOffset = nTotRevLen; // 记录当前接收缓存的偏移量
                nRevLen = 0;

                if (FALSE != bReqResult) // 检测接收长度
                {
                    bReqResult = WinHttpQueryDataAvailable(hRequest, &nRevLen);
                    if (FALSE == bReqResult) { MessageBoxPrintLastError(httpInfo); }
                }

                if (FALSE != bReqResult) // 更新总长和偏移量，分配内存
                {
                    nTotRevLen += nRevLen;
                    bReqResult = TRUE;
                    if (httpInfo->rev.nLen < nTotRevLen) /* 检查内存，确保空间足够 */
                    {
                        pBuf = HANWinHeapAlloc(httpInfo->hHeap, httpInfo->rev.pBuf, nTotRevLen + 1);
                        if (NULL == pBuf)
                        {
                            MessageBox(httpInfo->hSelf, TEXT("内存不足"), HAN_ETHERNET_TOOL_HTTP_ERROR_CAPTION, 0);
                            bReqResult = FALSE;
                        }
                        else
                        {
                            httpInfo->rev.pBuf = pBuf;
                            httpInfo->rev.nLen = nTotRevLen + 1;
                        }
                    }
                }

                if (FALSE != bReqResult) // 读取数据
                {
                    pBuf = httpInfo->rev.pBuf;
                    memset(&pBuf[nOffset], 0, nRevLen + 1);
                    bReqResult = WinHttpReadData(hRequest, &pBuf[nOffset], nRevLen, &nDownloadLen);
                    if (FALSE == bReqResult) { MessageBoxPrintLastError(httpInfo); }
                }

                if (FALSE == bReqResult) { break; }

            } while (0 < nRevLen);

            if (TRUE == bReqResult)
            {
                if (TRUE == DecodeHttp(httpInfo))
                {
                    SetWindowText(httpInfo->printData.hText, httpInfo->printData.bufWindowChar.pBuf);
                }
                else
                {
                    MessageBox(httpInfo->hSelf, TEXT("解码失败"), HAN_ETHERNET_TOOL_HTTP_ERROR_CAPTION, 0);
                }
            }
        }
    }

    if (NULL != hSession) { WinHttpCloseHandle(hSession); }
    if (NULL != hConnect) { WinHttpCloseHandle(hConnect); }
    if (NULL != hRequest) { WinHttpCloseHandle(hRequest); }

    return nRet;
}

static void* AllocHttp(void* pOldData, size_t nNewSize, void* pParam)
{
    return HANWinHeapAlloc(pParam, pOldData, nNewSize);
}
static void FreeHttpListNode(void* pData, void* pParam)
{
    HANWinHeapFree(pParam, 0, pData);
}
