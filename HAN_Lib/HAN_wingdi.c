#include <stdio.h>

#include "HAN_wingdi.h"

#define HANSTATIC

#if 1 /******************** HANInput ********************/
#define HSTBT_WID_LEFT_TEXT     0
#define HSTBT_WID_INPUT         1
#define HSTBT_WID_RIGHT_TEXT    2

typedef struct tagHANINPUT {
    HANDLE              hHeap;
    HWND                hLeftText;
    HWND                hInput;
    HWND                hRightText;
} HANINPUT, * HANPINPUT;
// 内部函数声明
static LONG HANInputGetEditStyle(LONG cInputStyle);
// WndProc & 实现 声明
HANSTATIC LRESULT CALLBACK HANInputWndProc(HWND hInput, UINT message, WPARAM wParam, LPARAM lParam);
HANSTATIC void s_HANInputMoveWindow(HWND hWnd, RECT* rcWin, BOOL bRepaint);
// 接口定义
void RegisterHANInput(HINSTANCE hInstance)
{
    WNDCLASSEX wcexHANInput = {
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_PARENTDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = HANInputWndProc,
        .cbClsExtra = 0,
        .cbWndExtra = sizeof(HANINPUT),
        .hInstance = hInstance,
        .hIcon = 0,
        .hCursor = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName = NULL,
        .lpszClassName = HINPUT_CLASS,
        .hIconSm = NULL
    };
    RegisterClassEx(&wcexHANInput);
}
void HANInputSetLeftTextFont(HWND hInput, HFONT hFOnt, BOOL bRepaint)
{
    SendMessage(hInput, HIPM_SETLEFTTEXTFONT, (WPARAM)hFOnt, (LPARAM)bRepaint);
}
void HANInputSetInputFont(HWND hInput, HFONT hFOnt, BOOL bRepaint)
{
    SendMessage(hInput, HIPM_SETINPUTFONT, (WPARAM)hFOnt, (LPARAM)bRepaint);
}
void HANInputSetRightTextFont(HWND hInput, HFONT hFOnt, BOOL bRepaint)
{
    SendMessage(hInput, HIPM_SETRIGHTTEXTFONT, (WPARAM)hFOnt, (LPARAM)bRepaint);
}
void HANInputMoveLeftText(HWND hInput, int x, int y, int w, int h, BOOL bRepaint)
{
    RECT rcWin = {
        .left = x,
        .top = y,
        .right = x + w,
        .bottom = y + h,
    };
    SendMessage(hInput, HIPM_MOVELEFTTEXT, (WPARAM)bRepaint, (LPARAM)(&rcWin));
}
void HANInputMoveInput(HWND hInput, int x, int y, int w, int h, BOOL bRepaint)
{
    RECT rcWin = {
        .left = x,
        .top = y,
        .right = x + w,
        .bottom = y + h,
    };
    SendMessage(hInput, HIPM_MOVEINPUT, (WPARAM)bRepaint, (LPARAM)(&rcWin));
}
void HANInputMoveRightText(HWND hInput, int x, int y, int w, int h, BOOL bRepaint)
{
    RECT rcWin = {
        .left = x,
        .top = y,
        .right = x + w,
        .bottom = y + h,
    };
    SendMessage(hInput, HIPM_MOVERIGHTTEXT, (WPARAM)bRepaint, (LPARAM)(&rcWin));
}
void HANInputSetLeftTextText(HWND hInput, HANPCSTR pText)
{
    SendMessage(hInput, HIPM_SETLEFTTEXTTEXT, 0, (LPARAM)pText);
}
void HANInputSetInputText(HWND hInput, HANPCSTR pText)
{
    SendMessage(hInput, HIPM_SETINPUTTEXT, 0, (LPARAM)pText);
}
void HANInputSetRightTextText(HWND hInput, HANPCSTR pText)
{
    SendMessage(hInput, HIPM_SETRIGHTTEXTTEXT, 0, (LPARAM)pText);
}
void HANInputGetLeftTextText(HWND hInput, HANPSTR pText, int nMaxCnt)
{
    SendMessage(hInput, HIPM_GETLEFTTEXTTEXT, (WPARAM)nMaxCnt, (LPARAM)pText);
}
void HANInputGetInputText(HWND hInput, HANPSTR pText, int nMaxCnt)
{
    SendMessage(hInput, HIPM_GETINPUTTEXT, (WPARAM)nMaxCnt, (LPARAM)pText);
}
void HANInputGetRightTextText(HWND hInput, HANPSTR pText, int nMaxCnt)
{
    SendMessage(hInput, HIPM_GETRIGHTTEXTTEXT, (WPARAM)nMaxCnt, (LPARAM)pText);
}
// 内部函数定义
static LONG HANInputGetEditStyle(LONG cInputStyle)
{
    LONG cRet = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT;

    if (0 != (cInputStyle & HIPS_NUMBER)) { cRet |= ES_NUMBER; }

    return cRet;
}
// WndProc & 实现 定义
HANSTATIC LRESULT CALLBACK HANInputWndProc(HWND hInput, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    HANPINPUT ipInfo = (HANPINPUT)GetWindowLongPtr(hInput, 0);

    switch(message) {
        case WM_CREATE: {
            HANDLE hHeap = GetProcessHeap();
            if (NULL == hHeap) { lWndProcRet = -1; break; }
            ipInfo = (HANPINPUT)HANWinHeapAlloc(hHeap, NULL, sizeof(HANINPUT));
            if (NULL == ipInfo) { lWndProcRet = -1; break; }
            SetWindowLongPtr(hInput, 0, (LONG_PTR)ipInfo);
            ipInfo->hHeap = hHeap;

            // 获取窗口大小
            RECT rcWin;
            GetClientRect(hInput, &rcWin);
            LONG nWinW = GetRectW(&rcWin);
            LONG nWinH = GetRectH(&rcWin);

            int xInput = nWinW / 3;
            int xRightText = nWinW * 2 / 3;

            // 创建子窗口
            HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
            ipInfo->hLeftText = CreateWindow(
                TEXT("static"), NULL, WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_CENTERIMAGE,
                0, 0, xInput, nWinH, hInput, (HMENU)HSTBT_WID_LEFT_TEXT, hInst, NULL);
            ipInfo->hInput = CreateWindow(
                TEXT("edit"), NULL, HANInputGetEditStyle(((LPCREATESTRUCT)lParam)->style),
                xInput, 0, xRightText - xInput, nWinH, hInput, (HMENU)HSTBT_WID_LEFT_TEXT, hInst, NULL);
            ipInfo->hRightText = CreateWindow(
                TEXT("static"), NULL, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE,
                xRightText, 0, nWinW - xRightText, nWinH, hInput, (HMENU)HSTBT_WID_RIGHT_TEXT, hInst, NULL);
        } break;
        case WM_CTLCOLORSTATIC: {
            lWndProcRet = (INT_PTR)GetStockObject(NULL_BRUSH);
        } break;
        case WM_GETTEXT: {
            GetWindowText(ipInfo->hInput, (HANPSTR)lParam, (HANINT)wParam);
        } break;
        case WM_SETTEXT: {
            SetWindowText(ipInfo->hInput, (HANPCSTR)lParam);
        } break;
        case WM_DESTROY: {
            HANWinHeapFree(ipInfo->hHeap, 0, ipInfo);
            lWndProcRet = DefWindowProc(hInput, message, wParam, lParam);
        } break;

        case HIPM_SETLEFTTEXTFONT: {
            SendMessage(ipInfo->hLeftText, WM_SETFONT, wParam, lParam);
        } break;
        case HIPM_SETINPUTFONT: {
            SendMessage(ipInfo->hInput, WM_SETFONT, wParam, lParam);
        } break;
        case HIPM_SETRIGHTTEXTFONT: {
            SendMessage(ipInfo->hRightText, WM_SETFONT, wParam, lParam);
        } break;
        case HIPM_MOVELEFTTEXT: {
            s_HANInputMoveWindow(ipInfo->hLeftText, (RECT*)lParam, (BOOL)wParam);
        } break;
        case HIPM_MOVEINPUT: {
            s_HANInputMoveWindow(ipInfo->hInput, (RECT*)lParam, (BOOL)wParam);
        } break;
        case HIPM_MOVERIGHTTEXT: {
            s_HANInputMoveWindow(ipInfo->hRightText, (RECT*)lParam, (BOOL)wParam);
        } break;
        case HIPM_SETLEFTTEXTTEXT: {
            SetWindowText(ipInfo->hLeftText, (HANPCSTR)lParam);
        } break;
        case HIPM_SETINPUTTEXT: {
            SetWindowText(ipInfo->hInput, (HANPCSTR)lParam);
        } break;
        case HIPM_SETRIGHTTEXTTEXT: {
            SetWindowText(ipInfo->hRightText, (HANPCSTR)lParam);
        } break;
        case HIPM_GETLEFTTEXTTEXT: {
            GetWindowText(ipInfo->hLeftText, (HANPSTR)lParam, (int)wParam);
        } break;
        case HIPM_GETINPUTTEXT: {
            GetWindowText(ipInfo->hInput, (HANPSTR)lParam, (int)wParam);
        } break;
        case HIPM_GETRIGHTTEXTTEXT: {
            GetWindowText(ipInfo->hRightText, (HANPSTR)lParam, (int)wParam);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hInput, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}
HANSTATIC void s_HANInputMoveWindow(HWND hWnd, RECT* rcWin, BOOL bRepaint)
{
    MoveWindow(hWnd, rcWin->left, rcWin->top, GetRectW(rcWin), GetRectH(rcWin), bRepaint);
}
#endif

#if 1 /******************** HANStateButton ********************/
#define HSTBT_WID_BUTTON          0

typedef struct tagHANSTBTSTATELIST HANSTBTSTATELIST, * HANPSTBTSTATELIST;
typedef const HANSTBTSTATELIST* HANPCSTBTSTATELIST;
struct tagHANSTBTSTATELIST {
    HSTBTSTATE          sbsState;
    HANPSTBTSTATELIST   pNext;
};
typedef struct tagHANSTATEBUTTON {
    HANDLE              hHeap;
    HWND                hButton;
    HANPSTBTSTATELIST   pHead;
    HANPSTBTSTATELIST   pTarget;
    HANSIZE             nListLen;
    void*               pParam;
} HANSTATEBUTTON, * HANPSTATEBUTTON;
typedef const HANSTATEBUTTON* HANPCSTATEBUTTON;

// 内部函数声明
HANSTATIC void HANStateButtonButtonCallback(HWND hStateButton, HANPSTATEBUTTON sbInfo);
// WndProc & 实现 声明
HANSTATIC LRESULT CALLBACK HANStateButtonWndProc(HWND hStateButton, UINT message, WPARAM wParam, LPARAM lParam);
HANSTATIC HANERROR s_HANStateButtonAddState(HANPSTATEBUTTON sbInfo, HPCSTBTSTATE pState);
HANSTATIC void s_HANStateButtonClick(HANPCSTATEBUTTON sbInfo);
// 接口定义
void RegisterHANStateButton(HINSTANCE hInstance)
{
    WNDCLASSEX wcexHANStateButton = {
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_PARENTDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = HANStateButtonWndProc,
        .cbClsExtra = 0,
        .cbWndExtra = sizeof(HANPSTATEBUTTON),
        .hInstance = hInstance,
        .hIcon = 0,
        .hCursor = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName = NULL,
        .lpszClassName = HSTBT_CLASS,
        .hIconSm = NULL
    };
    RegisterClassEx(&wcexHANStateButton);
}
HANERROR HANStateButtonAddState(HWND hStateButton, HPCSTBTSTATE pState)
{
    return SendMessage(hStateButton, HSTBTM_ADDSTATE, 0, (LPARAM)pState);
}
void HANStateButtonClick(HWND hStateButton)
{
    SendMessage(hStateButton, HSTBTM_CLICK, 0, 0);
}
// 内部函数定义
HANSTATIC void HANStateButtonButtonCallback(HWND hStateButton, HANPSTATEBUTTON sbInfo)
{
    if (NULL != sbInfo->pTarget)
    {
        HPCSTBTSTATE pState = &(sbInfo->pTarget->sbsState);
        HANSIZE nNextCnt = 1;
        if (NULL != pState->Callback) { nNextCnt = pState->Callback(hStateButton, sbInfo->hButton, sbInfo->pParam); }
        for (HANSIZE i = 0; i < nNextCnt; i++) { sbInfo->pTarget = sbInfo->pTarget->pNext; }
        SetWindowText(sbInfo->hButton, sbInfo->pTarget->sbsState.pText);
    }
}
// WndProc & 实现 定义
HANSTATIC LRESULT CALLBACK HANStateButtonWndProc(HWND hStateButton, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    HANPSTATEBUTTON sbInfo = (HANPSTATEBUTTON)GetWindowLongPtr(hStateButton, 0);

    switch(message) {
        case WM_CREATE: {
            HANDLE hHeap = GetProcessHeap();
            if (NULL == hHeap) { lWndProcRet = -1; break; }
            sbInfo = (HANPSTATEBUTTON)HANWinHeapAlloc(hHeap, NULL, sizeof(HANSTATEBUTTON));
            if (NULL == sbInfo) { lWndProcRet = -1; break; }
            SetWindowLongPtr(hStateButton, 0, (LONG_PTR)sbInfo);
            sbInfo->hHeap = hHeap;

            // 获取窗口大小
            RECT rcWin;
            GetClientRect(hStateButton, &rcWin);
            LONG nWinW = GetRectW(&rcWin);
            LONG nWinH = GetRectH(&rcWin);

            // 创建子窗口
            HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
            sbInfo->hButton = CreateWindow(
                TEXT("button"), NULL, WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                0, 0, nWinW, nWinH, hStateButton, (HMENU)HSTBT_WID_BUTTON, hInst, NULL);

            sbInfo->pParam = ((LPCREATESTRUCT)lParam)->lpCreateParams;
        } break;
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case HSTBT_WID_BUTTON: {
                    HANStateButtonButtonCallback(hStateButton, sbInfo);
                } break;

                default: {
                    /* idle */
                } break;
            }
        } break;
        case WM_SETFONT: {
            SendMessage(sbInfo->hButton, WM_SETFONT, wParam, lParam);
        } break;
        case WM_DESTROY: {
            HANPSTBTSTATELIST pNode = sbInfo->pHead;
            HANPSTBTSTATELIST pNext;
            for (HANSIZE iLoop = 0; iLoop < sbInfo->nListLen; iLoop++)
            {
                pNext = pNode->pNext;
                HANWinHeapFree(sbInfo->hHeap, 0, pNode);
                pNode = pNext;
            }
            HANWinHeapFree(sbInfo->hHeap, 0, sbInfo);
            lWndProcRet = DefWindowProc(hStateButton, message, wParam, lParam);
        } break;

        case HSTBTM_ADDSTATE: {
            lWndProcRet = s_HANStateButtonAddState(sbInfo, (HPCSTBTSTATE)lParam);
        } break;
        case HSTBTM_CLICK: {
            s_HANStateButtonClick(sbInfo);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hStateButton, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}
HANSTATIC HANERROR s_HANStateButtonAddState(HANPSTATEBUTTON sbInfo, HPCSTBTSTATE pState)
{
    HANERROR eRet = RET_OK;
    HANPSTBTSTATELIST pNewStateNode = (HANPSTBTSTATELIST)HANWinHeapAlloc(sbInfo->hHeap, NULL, sizeof(HANSTBTSTATELIST));
    if (NULL == pNewStateNode) { eRet = NOT_ENOUGH_MEMORY; }
    if (RET_OK == eRet)
    {
        pNewStateNode->sbsState = *pState;
        if (NULL == sbInfo->pHead)
        {
            sbInfo->pTarget = pNewStateNode;
            sbInfo->pHead = pNewStateNode;
            pNewStateNode->pNext = pNewStateNode;
            SetWindowText(sbInfo->hButton, pState->pText);
        }
        else
        {
            HANPSTBTSTATELIST pNode = sbInfo->pHead;
            for (HANSIZE i = 0; i < (sbInfo->nListLen - (HANSIZE)1); i++)
            {
                pNode = pNode->pNext;
            }
            pNode->pNext = pNewStateNode;
            pNewStateNode->pNext = sbInfo->pHead;
        }
        sbInfo->nListLen++;
    }
    return eRet;
}
HANSTATIC void s_HANStateButtonClick(HANPCSTATEBUTTON sbInfo)
{
    SendMessage(sbInfo->hButton, BM_CLICK, 0, 0);
}

#endif

#if 1 /******************** HANCOM ********************/
#define HCOM_WID_OPEN_COM       0
#define HCOM_WID_PORT_ID        1
#define HCOM_WID_BAUDRATE       2
#define HCOM_WID_BYTEBITS       3
#define HCOM_WID_PARITY         4
#define HCOM_WID_STOPBITS       5

#define HCOM_CID_TIMER          0
#define HCOM_DEF_TIMER_PERIOD   50          // 默认定时器周期，ms

static HANPCSTR g_pBaudrate[] = {
    TEXT("110"),
    TEXT("300"),
    TEXT("600"),
    TEXT("1200"),
    TEXT("2400"),
    TEXT("4800"),
    TEXT("9600"),
    TEXT("14400"),
    TEXT("19200"),
    TEXT("38400"),
    TEXT("56000"),
    TEXT("57600"),
    TEXT("115200"),
    TEXT("128000"),
    TEXT("230400"),
    TEXT("256000"),
    TEXT("460800"),
    TEXT("500000"),
    TEXT("512000"),
    TEXT("600000"),
    TEXT("750000"),
    TEXT("921600"),
    TEXT("1000000"),
    TEXT("1500000"),
    TEXT("2000000"),
};
static HANPCSTR g_pByteBits[] = {
    TEXT("5"),
    TEXT("6"),
    TEXT("7"),
    TEXT("8"),
};
static HANPCSTR g_pParity[] = {
    [NOPARITY] = TEXT("NONE"),
    [ODDPARITY] = TEXT("ODD"),
    [EVENPARITY] = TEXT("EVEN"),
    [MARKPARITY] = TEXT("MARK"),
    [SPACEPARITY] = TEXT("SPACE"),
};
static HANPCSTR g_pStopBits[] = {
    [ONESTOPBIT] = TEXT("1"),
    [ONE5STOPBITS] = TEXT("1.5"),
    [TWOSTOPBITS] = TEXT("2"),
};
static COMCFG g_cfgDefCOM = {
    .comFile = {
        .id = 3,
        .dwFlagsAndAttributes = FILE_FLAG_OVERLAPPED,
    },
    .comBuf = {
        .dwInQueue = 100000,
        .dwOutQueue = 100000,
    },
    .comTime = {
        .ReadIntervalTimeout = 0,
        .ReadTotalTimeoutMultiplier = 0,
        .ReadTotalTimeoutConstant = 0,
        .WriteTotalTimeoutMultiplier = 0,
        .WriteTotalTimeoutConstant = 0,
    },
    .comState = {
        .BaudRate = 115200,
        .ByteSize = 8,
        .Parity = NOPARITY,
        .StopBits = ONESTOPBIT,
    },
};

typedef struct tagHANCOM {
    HANDLE      hHeap;
    HANDLE      hCOM;
    HMENU       hMenu;
    HWND        hParent;
    HWND        hSelf;
    HWND        hOpenCOM;
    HWND        hPortID;
    HWND        hBaudrate;
    HWND        hByteSize;
    HWND        hParity;
    HWND        hStopBits;
    COMCFG      cfgCOM;
    UINT        tTimerPeriod;
    DWORD       nBufSize;
    DWORD       nDataLen;
    DWORD       nFrameLen;
    BOOL        bReceiving;
    OVERLAPPED  olRead;
    OVERLAPPED  olWrite;
} HANCOM, * HANPCOM;
typedef const HANCOM* HANPCCOM;

// 内部函数声明
HANSTATIC HANSIZE HANCOMOpenCOMProc(HWND hStateButton, HWND hChildButton, HANPCOM hcInfo);
HANSTATIC HANSIZE HANCOMCloseCOMProc(HWND hStateButton, HWND hChildButton, HANPCOM hcInfo);
HANSTATIC void HANCOMUpdateCOMCfg(HANPCOM hcInfo);
HANSTATIC BOOL HANCOMUpdatePortList(HANPCOM hcInfo, uint16_t nTargetId);
HANSTATIC void HANCOMSweepCOMCallback(const LPBYTE nCOMId, HANPCSTR pCOMName, DWORD nCount, HANPCOM hcInfo);
HANSTATIC void HANCOMPortIdCallback(HANPCOM hcInfo, WPARAM wParam);
HANSTATIC void HANCOMBaudrateCallback(HANPCOM hcInfo, WPARAM wParam);
HANSTATIC void HANCOMByteSizeCallback(HANPCOM hcInfo, WPARAM wParam);
HANSTATIC void HANCOMParityCallback(HANPCOM hcInfo, WPARAM wParam);
HANSTATIC void HANCOMStopBitsCallback(HANPCOM hcInfo, WPARAM wParam);
HANSTATIC void HANCOMTimerCallback(HANPCOM hcInfo);
// WndProc & 实现 声明
HANSTATIC LRESULT CALLBACK HANCOMWndProc(HWND hCOM, UINT message, WPARAM wParam, LPARAM lParam);
HANSTATIC void HANCOMShowWindowCallback(HWND hWnd, BOOL bShow);
HANSTATIC void HANCOMMoveWindowCallback(HWND hWnd, RECT* rcWin, BOOL bRepaint);
HANSTATIC BOOL HANCOMOpenCOMCallback(HANPCOM hcInfo);
HANSTATIC HANERROR HANCOMSetPortIdCallback(HANPCOM hcInfo, uint16_t nPortId);
HANSTATIC BOOL HANCOMSetBaudrateCallback(HANPCOM hcInfo, DWORD nBaudrate);
HANSTATIC BOOL HANCOMSetByteSizeCallback(HANPCOM hcInfo, BYTE nByteSize);
HANSTATIC BOOL HANCOMSetParityCallback(HANPCOM hcInfo, BYTE nParity);
HANSTATIC BOOL HANCOMSetStopBitsCallback(HANPCOM hcInfo, BYTE nStopBits);
HANSTATIC HANERROR HANCOMGetRevLenCallback(HANPCCOM hcInfo, DWORD* pLen);
HANSTATIC HANERROR HANCOMReadDataCallback(HANPCOM hcInfo, HANPCOMREADDATA rdReadData);
HANSTATIC HANERROR HANCOMWriteDataCallback(HANPCOM hcInfo, void* pData, DWORD nLen);
// 接口定义
void RegisterHANCOM(HINSTANCE hInstance)
{
    WNDCLASSEX wcexHANCOM = {
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_PARENTDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = HANCOMWndProc,
        .cbClsExtra = 0,
        .cbWndExtra = sizeof(HANPCOM),
        .hInstance = hInstance,
        .hIcon = 0,
        .hCursor = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName = NULL,
        .lpszClassName = HCOM_CLASS,
        .hIconSm = NULL
    };
    RegisterClassEx(&wcexHANCOM);
}
void HANCOMShowOpenCOM(HWND hCOM, BOOL bShow)
{
    SendMessage(hCOM, HCOMM_SHOWOPENCOM, (WPARAM)bShow, 0);
}
void HANCOMShowPortId(HWND hCOM, BOOL bShow)
{
    SendMessage(hCOM, HCOMM_SHOWPORTID, (WPARAM)bShow, 0);
}
void HANCOMShowBaudrate(HWND hCOM, BOOL bShow)
{
    SendMessage(hCOM, HCOMM_SHOWBAUDRATE, (WPARAM)bShow, 0);
}
void HANCOMShowByteSize(HWND hCOM, BOOL bShow)
{
    SendMessage(hCOM, HCOMM_SHOWBYTESIZE, (WPARAM)bShow, 0);
}
void HANCOMShowParity(HWND hCOM, BOOL bShow)
{
    SendMessage(hCOM, HCOMM_SHOWPARITY, (WPARAM)bShow, 0);
}
void HANCOMShowStopBits(HWND hCOM, BOOL bShow)
{
    SendMessage(hCOM, HCOMM_SHOWSTOPBITS, (WPARAM)bShow, 0);
}
void HANCOMMoveOpenCOM(HWND hCOM, int x, int y, int w, int h, BOOL bRepaint)
{
    RECT rcWin = {
        .left = x,
        .top = y,
        .right = x + w,
        .bottom = y + h,
    };
    SendMessage(hCOM, HCOMM_MOVEOPENCOM, (WPARAM)bRepaint, (LPARAM)(&rcWin));
}
void HANCOMMovePortId(HWND hCOM, int x, int y, int w, int h, BOOL bRepaint)
{
    RECT rcWin = {
        .left = x,
        .top = y,
        .right = x + w,
        .bottom = y + h,
    };
    SendMessage(hCOM, HCOMM_MOVEPORTID, (WPARAM)bRepaint, (LPARAM)(&rcWin));
}
void HANCOMMoveBaudrate(HWND hCOM, int x, int y, int w, int h, BOOL bRepaint)
{
    RECT rcWin = {
        .left = x,
        .top = y,
        .right = x + w,
        .bottom = y + h,
    };
    SendMessage(hCOM, HCOMM_MOVEBAUDRATE, (WPARAM)bRepaint, (LPARAM)(&rcWin));
}
void HANCOMMoveByteSize(HWND hCOM, int x, int y, int w, int h, BOOL bRepaint)
{
    RECT rcWin = {
        .left = x,
        .top = y,
        .right = x + w,
        .bottom = y + h,
    };
    SendMessage(hCOM, HCOMM_MOVEBYTESIZE, (WPARAM)bRepaint, (LPARAM)(&rcWin));
}
void HANCOMMoveParity(HWND hCOM, int x, int y, int w, int h, BOOL bRepaint)
{
    RECT rcWin = {
        .left = x,
        .top = y,
        .right = x + w,
        .bottom = y + h,
    };
    SendMessage(hCOM, HCOMM_MOVEPARITY, (WPARAM)bRepaint, (LPARAM)(&rcWin));
}
void HANCOMMoveStopBits(HWND hCOM, int x, int y, int w, int h, BOOL bRepaint)
{
    RECT rcWin = {
        .left = x,
        .top = y,
        .right = x + w,
        .bottom = y + h,
    };
    SendMessage(hCOM, HCOMM_MOVESTOPBITS, (WPARAM)bRepaint, (LPARAM)(&rcWin));
}
BOOL HANCOMOpenCOM(HWND hCOM)
{
    return (BOOL)SendMessage(hCOM, HCOMM_OPENCOM, 0, 0);
}
HANERROR HANCOMSetPortId(HWND hCOM, uint16_t nPortId)
{
    return (HANERROR)SendMessage(hCOM, HCOMM_SETPORTID, (WPARAM)nPortId, 0);
}
BOOL HANCOMSetBaudrate(HWND hCOM, DWORD nBaudrate)
{
    return (BOOL)SendMessage(hCOM, HCOMM_SETBAUDRATE, (WPARAM)nBaudrate, 0);
}
BOOL HANCOMSetByteSize(HWND hCOM, BYTE nByteSize)
{
    return (BOOL)SendMessage(hCOM, HCOMM_SETBYTESIZE, (WPARAM)nByteSize, 0);
}
BOOL HANCOMSetParity(HWND hCOM, BYTE nParity)
{
    return (BOOL)SendMessage(hCOM, HCOMM_SETPARITY, (WPARAM)nParity, 0);
}
BOOL HANCOMSetStopBits(HWND hCOM, BYTE nStopBits)
{
    return (BOOL)SendMessage(hCOM, HCOMM_SETSTOPBITS, (WPARAM)nStopBits, 0);
}
HANERROR HANCOMGetRevLen(HWND hCOM, DWORD* pLen)
{
    return SendMessage(hCOM, HCOMM_GETREVLEN, 0, (LPARAM)pLen);
}
HANERROR HANCOMReadData(HWND hCOM, void* pData, DWORD nBufSize, DWORD* nRevSize)
{
    HANCOMREADDATA rdReadData = {
        .pData = pData,
        .nBufSize = nBufSize,
        .nRevSize = nRevSize,
    };
    return (HANERROR)SendMessage(hCOM, HCOMM_READDATA, 0, (LPARAM)(&rdReadData));
}
HANERROR HANCOMWriteData(HWND hCOM, const void* pData, DWORD nLen)
{
    return (HANERROR)SendMessage(hCOM, HCOMM_WRITEDATA, (WPARAM)nLen, (LPARAM)pData);
}
// 内部函数定义
HANSTATIC HANSIZE HANCOMOpenCOMProc(HWND hStateButton, HWND hChildButton, HANPCOM hcInfo)
{
    (void)hStateButton;
    (void)hChildButton;

    HANSIZE nRet = 1;
    DWORD style = GetWindowStyle(hcInfo->hSelf);
    
    HANCOMUpdateCOMCfg(hcInfo);
    if (RET_OK != OpenCOM(&(hcInfo->hCOM), &hcInfo->cfgCOM))
    {
        if (0 != (style & HCOMMS_MSG_BOX))
        {
            MessageBox(NULL, TEXT("打开串口失败，可能是以下原因：\r\n1. 端口号选择错误\r\n2. 接线松了\r\n3. 端口被其它进程占用"), NULL, 0);
        }
        nRet = 0;
    }
    else
    {
        SetTimer(hcInfo->hSelf, HCOM_CID_TIMER, hcInfo->tTimerPeriod, NULL);
    }

    return nRet;
}
HANSTATIC HANSIZE HANCOMCloseCOMProc(HWND hStateButton, HWND hChildButton, HANPCOM hcInfo)
{
    (void)hStateButton;
    (void)hChildButton;

    KillTimer(hcInfo->hSelf, HCOM_CID_TIMER);
    CloseHandle(hcInfo->hCOM);
    hcInfo->hCOM = INVALID_HANDLE_VALUE;

    return 1;
}
HANSTATIC void HANCOMUpdateCOMCfg(HANPCOM hcInfo)
{
    HANCHAR pPortIdText[PATH_STR_SIZE];
    HANCHAR pNumText[HCOM_NUM_TEXT_SIZE];
    HANPCSTR pId = pPortIdText;

    // 端口号
    GetWindowText(hcInfo->hPortID, pPortIdText, PATH_STR_SIZE);
    if (pPortIdText[0] != TEXT('\0'))
    {
        while ((!HAN_isdigit(*pId)) && (*pId != TEXT('\0'))) { pId++; }
        hcInfo->cfgCOM.comFile.id = (uint16_t)HAN_strtol(pId, NULL, 10);
    }

    GetWindowText(hcInfo->hBaudrate, pNumText, HCOM_NUM_TEXT_SIZE);
    hcInfo->cfgCOM.comState.BaudRate = HAN_strtol(pNumText, NULL, 10);
    
    GetWindowText(hcInfo->hByteSize, pNumText, HCOM_NUM_TEXT_SIZE);
    hcInfo->cfgCOM.comState.ByteSize = (BYTE)HAN_strtol(pNumText, NULL, 10);

    hcInfo->cfgCOM.comState.Parity = (BYTE)SendMessage(hcInfo->hParity, CB_GETCURSEL, 0, 0);

    hcInfo->cfgCOM.comState.StopBits = (BYTE)SendMessage(hcInfo->hStopBits, CB_GETCURSEL, 0, 0);
}
HANSTATIC BOOL HANCOMUpdatePortList(HANPCOM hcInfo, uint16_t nTargetId)
{
    BOOL bRet = TRUE;
    HANCHAR pPortId[PATH_STR_SIZE];
    while (SendMessage(hcInfo->hPortID, CB_DELETESTRING, 0, 0) > 0) { }
    SweepCOMFromRegedit((void (*)(const LPBYTE, HANPCSTR, DWORD, void*))HANCOMSweepCOMCallback, hcInfo);
    HAN_sprintf(pPortId, TEXT("COM%u"), nTargetId);
    HANINT box_id = (HANINT)SendMessage(hcInfo->hPortID, CB_FINDSTRING, -1, (LPARAM)pPortId);
    if (box_id != CB_ERR)
    {
        (void)ComboBoxSetCursel(hcInfo->hPortID, box_id);
    }
    else
    {
        bRet = FALSE;
    }
    return bRet;
}
HANSTATIC void HANCOMSweepCOMCallback(const LPBYTE nCOMId, HANPCSTR pCOMName, DWORD nCount, HANPCOM hcInfo)
{
    (void)nCount;
    HANCHAR pPortIdText[PATH_STR_SIZE];
    HANPCSTR pCOMNameId = pCOMName;
    HANPCSTR pDevice = HAN_strstr(pCOMName, TEXT("\\Device\\"));

    if (NULL != pDevice)
    {
        if (pCOMName == pDevice) { pCOMNameId = &pCOMName[HAN_strlen(TEXT("\\Device\\"))]; }
    }
    (void)HAN_sprintf(pPortIdText, TEXT("%s: %s"), (HANPCSTR)nCOMId, pCOMNameId);
    (void)ComboBoxAddString(hcInfo->hPortID, pPortIdText);
}
HANSTATIC void HANCOMPortIdCallback(HANPCOM hcInfo, WPARAM wParam)
{
    switch (HIWORD(wParam)) {
        case CBN_DROPDOWN: {
            (void)HANCOMUpdatePortList(hcInfo, hcInfo->cfgCOM.comFile.id);
            break;
        }
        case CBN_SELCHANGE: {
            HANCHAR pPortId[PATH_STR_SIZE];
            HANPCSTR pId = pPortId;
            
            GetWindowText(hcInfo->hPortID, pPortId, ArrLen(pPortId));
            if (pPortId[0] != TEXT('\0'))
            {
                while (!HAN_isdigit(*pId) && (*pId != TEXT('\0'))) { pId++; }
                uint16_t nPortId = (uint16_t)HAN_strtol(pId, NULL, 10);
                if (RET_OK != HANCOMSetPortIdCallback(hcInfo, nPortId))
                {
                    if (0 != (GetWindowStyle(hcInfo->hSelf) & HCOMMS_MSG_BOX))
                    {
                        MessageBox(NULL, TEXT("打开串口失败"), NULL, 0);
                    }
                    HANStateButtonClick(hcInfo->hOpenCOM);
                }
            }
            break;
        }

        default: {
            break;
        }
    }
}
HANSTATIC void HANCOMBaudrateCallback(HANPCOM hcInfo, WPARAM wParam)
{
    switch (HIWORD(wParam)) {
        case CBN_SELCHANGE: {
            DWORD nBaudrate;
            HANCHAR pText[HCOM_NUM_TEXT_SIZE];

            GetWindowText(hcInfo->hBaudrate, pText, HCOM_NUM_TEXT_SIZE);
            nBaudrate = HAN_strtoul(pText, NULL, 10);

            HANCOMSetBaudrateCallback(hcInfo, nBaudrate);
            break;
        }

        default: {
            break;
        }
    }
}
HANSTATIC void HANCOMByteSizeCallback(HANPCOM hcInfo, WPARAM wParam)
{
    switch (HIWORD(wParam)) {
        case CBN_SELCHANGE: {
            BYTE nByteSize;
            HANCHAR pText[HCOM_NUM_TEXT_SIZE];

            GetWindowText(hcInfo->hByteSize, pText, HCOM_NUM_TEXT_SIZE);
            nByteSize = (BYTE)HAN_strtoul(pText, NULL, 10);

            if ((5 <= nByteSize) && (nByteSize <= 8)) { HANCOMSetByteSizeCallback(hcInfo, nByteSize); }
            break;
        }

        default: {
            break;
        }
    }
}
HANSTATIC void HANCOMParityCallback(HANPCOM hcInfo, WPARAM wParam)
{
    switch (HIWORD(wParam)) {
        case CBN_SELCHANGE: {
            int nParity = ComboBoxGetCursel(hcInfo->hParity);
            if (CB_ERR != nParity) { HANCOMSetParityCallback(hcInfo, (BYTE)nParity); }
            break;
        }

        default: {
            break;
        }
    }
}
HANSTATIC void HANCOMStopBitsCallback(HANPCOM hcInfo, WPARAM wParam)
{
    switch (HIWORD(wParam)) {
        case CBN_SELCHANGE: {
            int nStopBits = ComboBoxGetCursel(hcInfo->hStopBits);
            if (CB_ERR != nStopBits) { HANCOMSetByteSizeCallback(hcInfo, (BYTE)nStopBits); }
            break;
        }

        default: {
            break;
        }
    }
}
HANSTATIC void HANCOMTimerCallback(HANPCOM hcInfo)
{
    NMHDR nmh = { .idFrom = GetDlgCtrlID(hcInfo->hSelf), .hwndFrom = hcInfo->hSelf, };
    HWND hParent = hcInfo->hParent;
    DWORD nBufLen = GetCOMDataRevCount(hcInfo->hCOM);
    
    hcInfo->nFrameLen = nBufLen - hcInfo->nDataLen;
    hcInfo->nDataLen = nBufLen;
    if (0 < hcInfo->nFrameLen)
    {
        hcInfo->bReceiving = TRUE;
        if (hcInfo->nDataLen < hcInfo->nBufSize)
        {
            nmh.code = HCOMN_GETDATA;
            SendMessage(hParent, WM_NOTIFY, nmh.idFrom, (LPARAM)(&nmh));
        }
        else
        {
            nmh.code = HCOMN_OVERFLOW;
            SendMessage(hParent, WM_NOTIFY, nmh.idFrom, (LPARAM)(&nmh));
        }
    }
    else if ((0 < hcInfo->nDataLen) && (TRUE == hcInfo->bReceiving))
    {
        nmh.code = HCOMN_GETMSG;
        SendMessage(hParent, WM_NOTIFY, nmh.idFrom, (LPARAM)(&nmh));
        hcInfo->bReceiving = FALSE;
    }
    else {}
}
// WndProc & 实现 定义
HANSTATIC LRESULT CALLBACK HANCOMWndProc(HWND hCOM, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    HANPCOM hcInfo = (HANPCOM)GetWindowLongPtr(hCOM, 0);

    switch(message) {
        case WM_CREATE: {
            HANDLE hHeap = GetProcessHeap();
            if (NULL == hHeap) { lWndProcRet = -1; break; }
            hcInfo = (HANPCOM)HANWinHeapAlloc(hHeap, NULL, sizeof(HANCOM));
            if (NULL == hcInfo) { lWndProcRet = -1; break; }
            SetWindowLongPtr(hCOM, 0, (LONG_PTR)hcInfo);
            hcInfo->hHeap = hHeap;
            
            LPCREATESTRUCT pCreate = (LPCREATESTRUCT)lParam;
            HINSTANCE hInst = pCreate->hInstance;

            // 注册状态按钮类
            RegisterHANStateButton(hInst);

            // 创建子窗口
            hcInfo->hOpenCOM = CreateWindow(
                HSTBT_CLASS, NULL, WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                10, 10, 100, 40, hCOM, (HMENU)HCOM_WID_OPEN_COM, hInst, hcInfo);
            hcInfo->hPortID = CreateWindow(
                TEXT("combobox"), NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
                120, 16, 210, 100, hCOM, (HMENU)HCOM_WID_PORT_ID, hInst, NULL);
            hcInfo->hBaudrate = CreateWindow(
                TEXT("combobox"), NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWN,
                10, 60, 100, 150, hCOM, (HMENU)HCOM_WID_BAUDRATE, hInst, NULL);
            hcInfo->hByteSize = CreateWindow(
                TEXT("combobox"), NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
                120, 60, 50, 126, hCOM, (HMENU)HCOM_WID_BYTEBITS, hInst, NULL);
            hcInfo->hParity = CreateWindow(
                TEXT("combobox"), NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
                180, 60, 80, 126, hCOM, (HMENU)HCOM_WID_PARITY, hInst, NULL);
            hcInfo->hStopBits = CreateWindow(
                TEXT("combobox"), NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
                270, 60, 60, 126, hCOM, (HMENU)HCOM_WID_STOPBITS, hInst, NULL);

            // 初始化基本参数
            hcInfo->hParent = pCreate->hwndParent;
            hcInfo->hSelf = hCOM;
            hcInfo->hMenu = pCreate->hMenu;
            hcInfo->tTimerPeriod = HCOM_DEF_TIMER_PERIOD;
            hcInfo->nBufSize = HCOM_DEF_BUF_SIZE;
            hcInfo->nDataLen = 0;
            hcInfo->bReceiving = FALSE;
            // 初始化串口配置
            hcInfo->cfgCOM = g_cfgDefCOM;
            // 初始化按钮
            HSTBTSTATE hssButtonState[] = {
                { .pText = TEXT("打开串口"), .Callback = (HSTBTCALLBACK)HANCOMOpenCOMProc, },
                { .pText = TEXT("关闭串口"), .Callback = (HSTBTCALLBACK)HANCOMCloseCOMProc, },
            };
            (void)HANStateButtonAddState(hcInfo->hOpenCOM, &hssButtonState[0]);
            (void)HANStateButtonAddState(hcInfo->hOpenCOM, &hssButtonState[1]);
            // 初始化端口号
            (void)HANCOMUpdatePortList(hcInfo, hcInfo->cfgCOM.comFile.id);
            // 初始化波特率
            (void)ComboBoxAddStringArr(hcInfo->hBaudrate, g_pBaudrate, ArrLen(g_pBaudrate));
            HANCHAR Baudrate[16];
            HAN_sprintf(Baudrate, TEXT("%lu"), hcInfo->cfgCOM.comState.BaudRate);
            SetWindowText(hcInfo->hBaudrate, Baudrate);
            // 初始化字节长
            (void)ComboBoxAddStringArr(hcInfo->hByteSize, g_pByteBits, ArrLen(g_pByteBits));
            (void)ComboBoxSetCursel(hcInfo->hByteSize, hcInfo->cfgCOM.comState.ByteSize - HAN_strtol(g_pByteBits[0], NULL, 10));
            // 初始化校验位
            (void)ComboBoxAddStringArr(hcInfo->hParity, g_pParity, ArrLen(g_pParity));
            (void)ComboBoxSetCursel(hcInfo->hParity, hcInfo->cfgCOM.comState.Parity);
            // 初始化停止位
            (void)ComboBoxAddStringArr(hcInfo->hStopBits, g_pStopBits, ArrLen(g_pStopBits));
            (void)ComboBoxSetCursel(hcInfo->hStopBits, hcInfo->cfgCOM.comState.StopBits);

            break;
        }
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case HCOM_WID_PORT_ID: {
                    HANCOMPortIdCallback(hcInfo, wParam);
                    break;
                }
                case HCOM_WID_BAUDRATE: {
                    HANCOMBaudrateCallback(hcInfo, wParam);
                    break;
                }
                case HCOM_WID_BYTEBITS: {
                    HANCOMByteSizeCallback(hcInfo, wParam);
                    break;
                }
                case HCOM_WID_PARITY: {
                    HANCOMParityCallback(hcInfo, wParam);
                    break;
                }
                case HCOM_WID_STOPBITS: {
                    HANCOMStopBitsCallback(hcInfo, wParam);
                    break;
                }

                default: {
                    break;
                }
            }
            break;
        }
        case WM_TIMER: {
            HANCOMTimerCallback(hcInfo);
            break;
        }
        case HCOMM_SHOWOPENCOM: {
            HANCOMShowWindowCallback(hcInfo->hOpenCOM, (BOOL)wParam);
            break;
        }
        case HCOMM_SHOWPORTID: {
            HANCOMShowWindowCallback(hcInfo->hPortID, (BOOL)wParam);
            break;
        }
        case HCOMM_SHOWBAUDRATE: {
            HANCOMShowWindowCallback(hcInfo->hBaudrate, (BOOL)wParam);
            break;
        }
        case HCOMM_SHOWBYTESIZE: {
            HANCOMShowWindowCallback(hcInfo->hByteSize, (BOOL)wParam);
            break;
        }
        case HCOMM_SHOWPARITY: {
            HANCOMShowWindowCallback(hcInfo->hParity, (BOOL)wParam);
            break;
        }
        case HCOMM_SHOWSTOPBITS: {
            HANCOMShowWindowCallback(hcInfo->hStopBits, (BOOL)wParam);
            break;
        }
        case HCOMM_MOVEOPENCOM: {
            HANCOMMoveWindowCallback(hcInfo->hOpenCOM, (RECT*)lParam, (BOOL)wParam);
            break;
        }
        case HCOMM_MOVEPORTID: {
            HANCOMMoveWindowCallback(hcInfo->hPortID, (RECT*)lParam, (BOOL)wParam);
            break;
        }
        case HCOMM_MOVEBAUDRATE: {
            HANCOMMoveWindowCallback(hcInfo->hBaudrate, (RECT*)lParam, (BOOL)wParam);
            break;
        }
        case HCOMM_MOVEBYTESIZE: {
            HANCOMMoveWindowCallback(hcInfo->hByteSize, (RECT*)lParam, (BOOL)wParam);
            break;
        }
        case HCOMM_MOVEPARITY: {
            HANCOMMoveWindowCallback(hcInfo->hParity, (RECT*)lParam, (BOOL)wParam);
            break;
        }
        case HCOMM_MOVESTOPBITS: {
            HANCOMMoveWindowCallback(hcInfo->hStopBits, (RECT*)lParam, (BOOL)wParam);
            break;
        }
        case HCOMM_OPENCOM: {
            lWndProcRet = HANCOMOpenCOMCallback(hcInfo);
            break;
        }
        case HCOMM_SETPORTID: {
            lWndProcRet = HANCOMSetPortIdCallback(hcInfo, (uint16_t)wParam);
            break;
        }
        case HCOMM_SETBAUDRATE: {
            lWndProcRet = HANCOMSetBaudrateCallback(hcInfo, (DWORD)wParam);
            break;
        }
        case HCOMM_SETBYTESIZE: {
            lWndProcRet = HANCOMSetByteSizeCallback(hcInfo, (BYTE)wParam);
            break;
        }
        case HCOMM_SETPARITY: {
            lWndProcRet = HANCOMSetParityCallback(hcInfo, (BYTE)wParam);
            break;
        }
        case HCOMM_SETSTOPBITS: {
            lWndProcRet = HANCOMSetStopBitsCallback(hcInfo, (BYTE)wParam);
            break;
        }
        case HCOMM_GETREVLEN: {
            lWndProcRet = HANCOMGetRevLenCallback(hcInfo, (DWORD*)lParam);
            break;
        }
        case HCOMM_READDATA: {
            lWndProcRet = HANCOMReadDataCallback(hcInfo, (HANPCOMREADDATA)lParam);
            break;
        }
        case HCOMM_WRITEDATA: {
            lWndProcRet = HANCOMWriteDataCallback(hcInfo, (void*)lParam, (DWORD)wParam);
            break;
        }

        default: {
            lWndProcRet = DefWindowProc(hCOM, message, wParam, lParam);
            break;
        }
    }

    return lWndProcRet;
}
HANSTATIC void HANCOMShowWindowCallback(HWND hWnd, BOOL bShow)
{
    int nShow;
    if (FALSE == bShow) { nShow = SW_HIDE; }
    else { nShow = SW_SHOW; }
    ShowWindow(hWnd, nShow);
}
HANSTATIC void HANCOMMoveWindowCallback(HWND hWnd, RECT* rcWin, BOOL bRepaint)
{
    MoveWindow(hWnd, rcWin->left, rcWin->top, GetRectW(rcWin), GetRectH(rcWin), bRepaint);
}
HANSTATIC BOOL HANCOMOpenCOMCallback(HANPCOM hcInfo)
{
    BOOL bRet = TRUE;
    DWORD nFlag;
    SendMessage(hcInfo->hOpenCOM, HSTBTM_CLICK, 0, 0);
    bRet = GetHandleInformation(hcInfo->hCOM, &nFlag);
    return bRet;
}
HANSTATIC HANERROR HANCOMSetPortIdCallback(HANPCOM hcInfo, uint16_t nPortId)
{
    HANERROR nRet = RET_OK;
    DWORD nFlag;

    if (FALSE == HANCOMUpdatePortList(hcInfo, nPortId))
    {
        nRet = HCOM_PORT_ID_NOT_EXIST;
    }

    if (RET_OK == nRet)
    {
        if (nPortId != hcInfo->cfgCOM.comFile.id)
        {
            if (TRUE == GetHandleInformation(hcInfo->hCOM, &nFlag))
            {
                COMCFG cfgCOM = hcInfo->cfgCOM;
                HANDLE hCOM;
                cfgCOM.comFile.id = nPortId;
                nRet = OpenCOM(&hCOM, &cfgCOM);
                if (RET_OK == nRet)
                {
                    CloseHandle(hcInfo->hCOM);
                    hcInfo->hCOM = hCOM;
                    hcInfo->cfgCOM.comFile.id = nPortId;
                }
            }
            else
            {
                hcInfo->cfgCOM.comFile.id = nPortId;
            }
        }
    }

    return nRet;
}
HANSTATIC BOOL HANCOMSetBaudrateCallback(HANPCOM hcInfo, DWORD nBaudrate)
{
    BOOL bRet = TRUE;
    DCB dcbCOMState;
    DWORD nFlag;

    if (TRUE == GetHandleInformation(hcInfo->hCOM, &nFlag)) // 检查句柄有效性，若端口已打开，必须配置通过后再使参数生效
    {
        if (!GetCommState(hcInfo->hCOM, &dcbCOMState)) { bRet = FALSE; }
        if (TRUE == bRet)
        {
            dcbCOMState.BaudRate = nBaudrate;
            if (!SetCommState(hcInfo->hCOM, &dcbCOMState)) { bRet = FALSE; }
        }
    }
    if (TRUE == bRet)
    {
        HANCHAR pText[HCOM_NUM_TEXT_SIZE];
        HAN_snprintf(pText, HCOM_NUM_TEXT_SIZE, TEXT("%lu"), nBaudrate);
        pText[HCOM_NUM_TEXT_SIZE - 1] = TEXT('\0');
        SetWindowText(hcInfo->hBaudrate, pText);
        hcInfo->cfgCOM.comState.BaudRate = nBaudrate;
    }

    return bRet;
}
HANSTATIC BOOL HANCOMSetByteSizeCallback(HANPCOM hcInfo, BYTE nByteSize)
{
    BOOL bRet;
    DCB dcbCOMState;
    DWORD nFlag;

    if ((5 <= nByteSize) && (nByteSize <= 8)) { bRet = TRUE; }  // 检查参数有效性
    else { bRet = FALSE; }
    if (TRUE == GetHandleInformation(hcInfo->hCOM, &nFlag)) // 检查句柄有效性，若端口已打开，必须配置通过后再使参数生效
    {
        if (!GetCommState(hcInfo->hCOM, &dcbCOMState)) { bRet = FALSE; }
        if (TRUE == bRet)
        {
            dcbCOMState.ByteSize = nByteSize;
            if (!SetCommState(hcInfo->hCOM, &dcbCOMState)) { bRet = FALSE; }
        }
    }
    if (TRUE == bRet)
    {
        (void)ComboBoxSetCursel(hcInfo->hByteSize, nByteSize - 5);
        hcInfo->cfgCOM.comState.ByteSize = nByteSize;
    }

    return bRet;
}
HANSTATIC BOOL HANCOMSetParityCallback(HANPCOM hcInfo, BYTE nParity)
{
    BOOL bRet;
    DCB dcbCOMState;
    DWORD nFlag;

    if (nParity < ArrLen(g_pParity)) { bRet = TRUE; }
    else { bRet = FALSE; }
    if (TRUE == GetHandleInformation(hcInfo->hCOM, &nFlag)) // 检查句柄有效性，若端口已打开，必须配置通过后再使参数生效
    {
        if (!GetCommState(hcInfo->hCOM, &dcbCOMState)) { bRet = FALSE; }
        if (TRUE == bRet)
        {
            dcbCOMState.Parity = nParity;
            if (!SetCommState(hcInfo->hCOM, &dcbCOMState)) { bRet = FALSE; }
        }
    }
    if (TRUE == bRet)
    {
        (void)ComboBoxSetCursel(hcInfo->hParity, nParity);
        hcInfo->cfgCOM.comState.Parity = nParity;
    }

    return bRet;
}
HANSTATIC BOOL HANCOMSetStopBitsCallback(HANPCOM hcInfo, BYTE nStopBits)
{
    BOOL bRet;
    DCB dcbCOMState;
    DWORD nFlag;

    if (nStopBits < ArrLen(g_pStopBits)) { bRet = TRUE; }
    else { bRet = FALSE; }
    if (TRUE == GetHandleInformation(hcInfo->hCOM, &nFlag)) // 检查句柄有效性，若端口已打开，必须配置通过后再使参数生效
    {
        if (!GetCommState(hcInfo->hCOM, &dcbCOMState)) { bRet = FALSE; }
        if (TRUE == bRet)
        {
            dcbCOMState.StopBits = nStopBits;
            if (!SetCommState(hcInfo->hCOM, &dcbCOMState)) { bRet = FALSE; }
        }
    }
    if (TRUE == bRet)
    {
        (void)ComboBoxSetCursel(hcInfo->hStopBits, nStopBits);
        hcInfo->cfgCOM.comState.StopBits = nStopBits;
    }

    return bRet;
}
HANSTATIC HANERROR HANCOMGetRevLenCallback(HANPCCOM hcInfo, DWORD* pLen)
{
    HANERROR nRet = RET_OK;
    DWORD nFlag;

    if (FALSE == GetHandleInformation(hcInfo->hCOM, &nFlag))
    {
        nRet = HCOM_NOT_OPEN;
    }
    else
    {
        *pLen = GetCOMDataRevCount(hcInfo->hCOM);
    }

    return nRet;
}
HANSTATIC HANERROR HANCOMReadDataCallback(HANPCOM hcInfo, HANPCOMREADDATA rdReadData)
{
    HANERROR nRet = RET_OK;
    DWORD nFlag;

    if (!GetHandleInformation(hcInfo->hCOM, &nFlag)) { nRet = HCOM_NOT_OPEN; }
    if (RET_OK == nRet)
    {
        nRet = ReadCOM(hcInfo->hCOM, rdReadData->pData, rdReadData->nBufSize, rdReadData->nRevSize, &(hcInfo->olRead));
    }

    return nRet;
}
HANSTATIC HANERROR HANCOMWriteDataCallback(HANPCOM hcInfo, void* pData, DWORD nLen)
{
    HANERROR nRet = RET_OK;
    DWORD nWord;

    if (FALSE == GetHandleInformation(hcInfo->hCOM, &nWord)) { nRet = HCOM_NOT_OPEN; }
    if (RET_OK == nRet)
    {
        if (FALSE == GetOverlappedResult(hcInfo->hCOM, &(hcInfo->olWrite), &nWord, FALSE))
        {
            nRet = HCOM_WRITING;
        }
    }
    if (RET_OK == nRet)
    {
        (void)WriteFile(hcInfo->hCOM, pData, nLen, NULL, &(hcInfo->olWrite));
    }

    return nRet;
}
#endif

#if 1 /******************** HANTabControl ********************/
// typedef struct tagHANTABCONTROL {
//     HANDLE              hHeap;
// } HANTABCONTROL, * HANPTABCONTROL;
// typedef const HANTABCONTROL* HANPCTABCONTROL;

// // WndProc & 实现 声明
// HANSTATIC LRESULT CALLBACK HANTabControlWndProc(HWND hTabControl, UINT message, WPARAM wParam, LPARAM lParam);
// // 接口定义
// void RegisterHANTabControl(HINSTANCE hInstance)
// {
//     WNDCLASSEX wcexHANTabControl = {
//         .cbSize = sizeof(WNDCLASSEX),
//         .style = CS_PARENTDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW,
//         .lpfnWndProc = HANTabControlWndProc,
//         .cbClsExtra = 0,
//         .cbWndExtra = sizeof(HANPTABCONTROL),
//         .hInstance = hInstance,
//         .hIcon = 0,
//         .hCursor = LoadCursor(NULL, IDC_ARROW),
//         .hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
//         .lpszMenuName = NULL,
//         .lpszClassName = HTC_CLASS,
//         .hIconSm = NULL
//     };
//     RegisterClassEx(&wcexHANTabControl);
// }
// // WndProc & 实现 定义
// HANSTATIC LRESULT CALLBACK HANTabControlWndProc(HWND hStateButton, UINT message, WPARAM wParam, LPARAM lParam)
// {
//     LRESULT lWndProcRet = 0;

//     // 读取属性
//     HANPSTATEBUTTON tcInfo = (HANPSTATEBUTTON)GetWindowLongPtr(hStateButton, 0);

//     switch(message) {
//         case WM_CREATE: {
//             HANDLE hHeap = GetProcessHeap();
//             if (NULL == hHeap) { lWndProcRet = -1; break; }
//             sbInfo = (HANPSTATEBUTTON)HANWinHeapAlloc(hHeap, NULL, sizeof(HANSTATEBUTTON));
//             if (NULL == sbInfo) { lWndProcRet = -1; break; }
//             SetWindowLongPtr(hStateButton, 0, (LONG_PTR)sbInfo);
//             sbInfo->hHeap = hHeap;

//             // 获取窗口大小
//             RECT rcWin;
//             GetClientRect(hStateButton, &rcWin);
//             LONG nWinW = GetRectW(&rcWin);
//             LONG nWinH = GetRectH(&rcWin);

//             // 创建子窗口
//             HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
//             sbInfo->hButton = CreateWindow(
//                 TEXT("button"), NULL, WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
//                 0, 0, nWinW, nWinH, hStateButton, (HMENU)HSTBT_WID_BUTTON, hInst, NULL);

//             sbInfo->pParam = ((LPCREATESTRUCT)lParam)->lpCreateParams;
//         } break;
//         case WM_COMMAND: {
//             switch (LOWORD(wParam)) {
//                 case HSTBT_WID_BUTTON: {
//                     HANStateButtonButtonCallback(hStateButton, sbInfo);
//                 } break;

//                 default: {
//                     /* idle */
//                 } break;
//             }
//         } break;
//         case WM_SETFONT: {
//             SendMessage(sbInfo->hButton, WM_SETFONT, wParam, lParam);
//         } break;
//         case WM_DESTROY: {
//             HANPSTBTSTATELIST pNode = sbInfo->pHead;
//             HANPSTBTSTATELIST pNext;
//             for (HANSIZE iLoop = 0; iLoop < sbInfo->nListLen; iLoop++)
//             {
//                 pNext = pNode->pNext;
//                 HANWinHeapFree(sbInfo->hHeap, 0, pNode);
//                 pNode = pNext;
//             }
//             HANWinHeapFree(sbInfo->hHeap, 0, sbInfo);
//             lWndProcRet = DefWindowProc(hStateButton, message, wParam, lParam);
//         } break;

//         case HSTBTM_ADDSTATE: {
//             lWndProcRet = s_HANStateButtonAddState(sbInfo, (HPCSTBTSTATE)lParam);
//         } break;
//         case HSTBTM_CLICK: {
//             s_HANStateButtonClick(sbInfo);
//         } break;

//         default: {
//             lWndProcRet = DefWindowProc(hStateButton, message, wParam, lParam);
//         } break;
//     }

//     return lWndProcRet;
// }
#endif
