#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include <windows.h>

#include "HAN_windows.h"

#ifdef HAN_CHECK_ALLOC_LEAK

#define HAN_CHECK_ALLOC_LEAK_TAB_ALLOC_SIZE     1024

typedef struct tagHANALLOCTAB {
    void*           pMemory;
    HANSIZE         nSize;
    HANSIZE         nEditTimes;
} HANALLOCTAB, * HANPALLOCTAB;
typedef struct tagHANALLOCTABINFO {
    HANPALLOCTAB    pTab;
    HANSIZE         nTabLen;
    HANSIZE         nMemoryCnt;
} HANALLOCTABINFO;

static HANSIZE HANWinAllocCheckLeak(void* pOld, void* pNew, HANSIZE nSize);
static void HANWinFreeCheckLeak(void* pMemory);

static HANALLOCTABINFO sg_allocTab = {
    .pTab = NULL,
    .nTabLen = 0,
};

#endif

HANINT MultiByteToHANStr(
    UINT CodePage,
    DWORD dwFlags,
    PCCH pMultiByteStr,     // 需要转换的字符
    int cbMultiByte,        // 需要转换的字符的个数，-1 表示以 \0 结尾
    HANPSTR pHANStr,        // 接收字符串的数组
    HANINT* pHANStrSize,    // 用来指示接收字符串的数组大小，如果指向的值为 0，函数会将指向的变量设置为需要的数组大小
    PWCHAR pBuf,            // 中间转换用的缓存
    HANINT* pBufSize,       // 用来指示中间转换用的缓存大小，如果指向的值为 0，函数会将指向的变量设置为需要的缓存大小
    LPCCH lpDefaultChar,
    LPBOOL lpUsedDefaultChar)
{
    HANINT nRet = 0;
    HANINT nSize = MultiByteToWideChar(CodePage, 0, pMultiByteStr, cbMultiByte, pBuf, *pBufSize);

    if (0 == *pBufSize)
    {
        *pBufSize = nSize;
    }
    else
    {
        nSize = WStrToHANStr(CP_ACP, dwFlags, pBuf, *pHANStrSize, pHANStr, *pHANStrSize, lpDefaultChar, lpUsedDefaultChar);
        if (0 == *pHANStrSize) { *pHANStrSize = nSize; }
        else { nRet = nSize; }
    }

    return nRet;
}

int HANStrToWStr(
    UINT CodePage,
    DWORD dwFlags,
    HANPCSTR lpMultiByteStr,
    int cbMultiByte,
    LPWSTR lpWideCharStr,
    int cchWideChar)
{
    int nRet;
#ifdef UNICODE
    if (0 != cchWideChar)
    {
        wcsncpy(lpWideCharStr, lpMultiByteStr, cchWideChar);
        lpWideCharStr[cchWideChar - 1] = TEXT('\0');
        nRet = (int)wcslen(lpWideCharStr);
    }
    else
    {
        nRet = (int)wcslen(lpMultiByteStr);
    }
#else
    nRet = MultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);
#endif
    return nRet;
}
int WStrToHANStr(
    UINT CodePage,
    DWORD dwFlags,
    LPCWCH lpWideCharStr,
    int cchWideChar,
    HANPSTR lpMultiByteStr,
    int cbMultiByte,
    LPCCH lpDefaultChar,
    LPBOOL lpUsedDefaultChar)
{
    int nRet;
#ifdef UNICODE
    if (0 != cbMultiByte)
    {
        wcsncpy(lpMultiByteStr, lpWideCharStr, cbMultiByte);
        lpMultiByteStr[cchWideChar - 1] = TEXT('\0');
        nRet = (int)wcslen(lpMultiByteStr);
    }
    else
    {
        nRet = (int)wcslen(lpWideCharStr);
    }
#else
    nRet = WideCharToMultiByte(
        CodePage,
        dwFlags,
        lpWideCharStr,
        cchWideChar,
        lpMultiByteStr,
        cbMultiByte,
        lpDefaultChar,
        lpUsedDefaultChar
    );
#endif
    return nRet;
}

LPVOID HANWinHeapAlloc(HANDLE hHeap, void* pReAllocMem, SIZE_T dwBytes)
{
    LPVOID pRet;

    if (NULL == pReAllocMem) { pRet = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwBytes); }
    else { pRet = HeapReAlloc(hHeap, HEAP_ZERO_MEMORY, pReAllocMem, dwBytes); }

#ifdef HAN_CHECK_ALLOC_LEAK
    HANSIZE nId;
    nId = HANWinAllocCheckLeak(pReAllocMem, pRet, dwBytes);
    // if ((nId == 23))
    // {
    //     printf("nId = " HANSIZE_PRINT_FORMAT ", pOld = %p, pNew = %p\n", nId, pReAllocMem, pRet);
    // }
    (void)nId;
#endif

    return pRet;
}
void HANWinHeapFree(HANDLE hHeap, DWORD nFlag, void* pMem)
{
    HeapFree(hHeap, nFlag, pMem);

#ifdef HAN_CHECK_ALLOC_LEAK
    HANWinFreeCheckLeak(pMem);
#endif
}

void RotatePoint(POINT* pDest, const POINT* pSrc, const POINT* pCenter, HANDOUBLE radAngle)
{
    POINT ptRet = {
        .x = (LONG)(pSrc->x * cos(radAngle) - pSrc->y * sin(radAngle)
           + pCenter->x - pCenter->x * cos(radAngle) + pCenter->y * sin(radAngle)),
        .y = (LONG)(pSrc->x * sin(radAngle) + pSrc->y * cos(radAngle)
           + pCenter->y - pCenter->x * sin(radAngle) - pCenter->y * cos(radAngle)),
    };
    *pDest = ptRet;
}

/******************** 颜色 ********************/
void PRGBToPHSB(const COLORREF* crRGB, COLORHSB* crHSB)
{
    UINT8 pRGB[] = { GetRValue(*crRGB), GetGValue(*crRGB), GetBValue(*crRGB) };
    UINT8 idMax = 0;
    UINT8 idMin = 0;
    for (UINT8 i = 0; i < ArrLen(pRGB); i++)
    {
        if (pRGB[i] > pRGB[idMax]) { idMax = i; }
        if (pRGB[i] < pRGB[idMin]) { idMin = i; }
    }
    // H
    if (idMax == idMin) { crHSB->kHue = 0; }
    else if ((0 == idMax) && (pRGB[1] >= pRGB[2])) { crHSB->kHue = 60 * ((pRGB[1] - pRGB[2]) * (FLOAT)1.0 / (pRGB[idMax] - pRGB[idMin])); }
    else if ((0 == idMax) && (pRGB[1] < pRGB[2])) { crHSB->kHue = 60 * ((pRGB[1] - pRGB[2]) * (FLOAT)1.0 / (pRGB[idMax] - pRGB[idMin])) + 360; }
    else if (1 == idMax) { crHSB->kHue = (60 * ((pRGB[2] - pRGB[0]) * (FLOAT)1.0 / (pRGB[idMax] - pRGB[idMin]))) + 120; }
    else if (2 == idMax) { crHSB->kHue = (60 * ((pRGB[0] - pRGB[1]) * (FLOAT)1.0 / (pRGB[idMax] - pRGB[idMin]))) + 240; }
    else { }
    // S
    if (0 == pRGB[idMax]) { crHSB->kSaturation = 0; }
    else { crHSB->kSaturation = 1 - (pRGB[idMin] * (FLOAT)1.0 / pRGB[idMax]); }
    // B
    crHSB->kBrightness = pRGB[idMax] * (FLOAT)1.0 / 255;
}
void PHSBToPRGB(const COLORHSB* crHSB, COLORREF* crRGB)
{
    int nI = (int)floor(crHSB->kHue / 60) % 6;
    float nF = (crHSB->kHue / 60) - nI;
    float nV = (crHSB->kBrightness * 255);
    float nP = nV * (1 - crHSB->kSaturation);
    float nQ = nV * (1 - (nF * crHSB->kSaturation));
    float nT = nV * (1 - ((1.0F - nF) * crHSB->kSaturation));
    switch (nI) {
        case 0: {
            *crRGB = RGB(nV, nT, nP);
            break;
        }
        case 1: {
            *crRGB = RGB(nQ, nV, nP);
            break;
        }
        case 2: {
            *crRGB = RGB(nP, nV, nT);
            break;
        }
        case 3: {
            *crRGB = RGB(nP, nQ, nV);
            break;
        }
        case 4: {
            *crRGB = RGB(nT, nP, nV);
            break;
        }
        case 5: {
            *crRGB = RGB(nV, nP, nQ);
            break;
        }
    }
}
COLORHSB RGBToHSB(COLORREF crRGB)
{
    COLORHSB crHSB;
    PRGBToPHSB(&crRGB, &crHSB);
    return crHSB;
}
COLORREF HSBToRGB(COLORHSB crHSB)
{
    COLORREF crRGB;
    PHSBToPRGB(&crHSB, &crRGB);
    return crRGB;
}

/******************** 共享内存 ********************/
void* ConnectToSharedMem(HANPCSTR pMemName, DWORD nSize, HANDLE* phFile)
{
    // 尝试打开一个内存映射文件
    HANDLE hRet = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, pMemName);
    void* ret = NULL;
    if (hRet != NULL)
    {
        // 打开成功，获取共享内存的指针
        ret = MapViewOfFile(hRet, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    }
    else
    {
        // 打开失败，创建内存映射文件
        hRet = CreateFileMapping(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            nSize,
            pMemName
        );
        if (hRet == NULL) { ret = NULL; }
        else { ret = MapViewOfFile(hRet, FILE_MAP_ALL_ACCESS, 0, 0, 0); }
    }
    if ((hRet != NULL) && (phFile != NULL)) { *phFile = hRet; }
    return ret;
}

void DisconnectFromSharedMem(void* pMemory) { if (pMemory != NULL) { UnmapViewOfFile((LPCVOID)pMemory); } }

void CloseSharedMem(HANDLE hFile) { CloseHandle(hFile); }

/******************** 串口 ********************/
HANERROR OpenCOM(HANDLE* phCOM, PCCOMCFG pCOMInfo)
{
    HANERROR eRet = RET_OK;
    HANDLE hRet;
    HANCHAR pCOMName[16];
    (void)HAN_sprintf(pCOMName, TEXT("\\\\.\\COM%d"), pCOMInfo->comFile.id);
    
    hRet = CreateFile(
        pCOMName,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        pCOMInfo->comFile.dwFlagsAndAttributes,
        NULL
    );
    if (hRet == INVALID_HANDLE_VALUE) { eRet = CREATE_COM_FILE_ERR; }

    if (RET_OK == eRet)
    {
        if (FALSE == SetupComm(hRet, pCOMInfo->comBuf.dwInQueue, pCOMInfo->comBuf.dwOutQueue))
        {
            eRet = SET_COM_QUEUE_ERR;
        }
    }

    if (RET_OK == eRet)
    {
        COMMTIMEOUTS ctTimeOut = {
            .ReadIntervalTimeout = pCOMInfo->comTime.ReadIntervalTimeout,
            .ReadTotalTimeoutConstant = pCOMInfo->comTime.ReadTotalTimeoutConstant,
            .ReadTotalTimeoutMultiplier = pCOMInfo->comTime.ReadTotalTimeoutMultiplier,
            .WriteTotalTimeoutConstant = pCOMInfo->comTime.WriteTotalTimeoutConstant,
            .WriteTotalTimeoutMultiplier = pCOMInfo->comTime.WriteTotalTimeoutMultiplier,
        };
        if (!SetCommTimeouts(hRet, &ctTimeOut)) { eRet = SET_COM_TIME_OUT_ERR; }
    }

    if (RET_OK == eRet)
    {
        DCB dcbCOMState;
        if (!GetCommState(hRet, &dcbCOMState)) { eRet = SET_COM_STATE_ERR; }
        if (RET_OK == eRet)
        {
            dcbCOMState.BaudRate = pCOMInfo->comState.BaudRate;
            dcbCOMState.ByteSize = pCOMInfo->comState.ByteSize;
            dcbCOMState.Parity = pCOMInfo->comState.Parity;
            dcbCOMState.StopBits = pCOMInfo->comState.StopBits;
            if (!SetCommState(hRet, &dcbCOMState)) { eRet = SET_COM_STATE_ERR; }
        }
    }

    if (RET_OK == eRet) { *phCOM = hRet; }
    
    return eRet;
}

DWORD GetCOMDataRevCount(HANDLE hCOM)
{
    DWORD nCOMErr;
    COMSTAT csCOMStat;
    ClearCommError(hCOM, &nCOMErr, &csCOMStat);
    return csCOMStat.cbInQue;
}

HANERROR ReadCOM(HANDLE hCOM, void* pBuf, DWORD nBufSize, DWORD* nRevSize, LPOVERLAPPED lpOverlapped)
{
    DWORD nReadSize;
    HANERROR ret;
    // 获取已接收的字节数
    DWORD nRevLen = GetCOMDataRevCount(hCOM);
    if (nBufSize == 0) { nReadSize = nRevLen; }
    else
    {
        if (nBufSize < nRevLen) { nReadSize = nBufSize; }
        else { nReadSize = nRevLen; }
    }
    // 开始接收
    if (lpOverlapped == NULL)
    {
        // 同步操作通过判断函数的返回值即可判断是否断开
        if (TRUE == ReadFile(hCOM, pBuf, nReadSize, nRevSize, NULL)) { ret = RET_OK; }
        else { ret = READ_COM_ERR; }
    }
    else
    {
        // 重叠操作需要通过 GetLastError 来判断是否断开
        (void)ReadFile(hCOM, pBuf, nReadSize, NULL, lpOverlapped);
        ret = RET_OK;
        while (!GetOverlappedResult(hCOM, lpOverlapped, nRevSize, FALSE))
        {
            // IO 操作未完成且 IO 状态非 ERROR_IO_PENDING 说明串口异常，可能断开
            if (GetLastError() != ERROR_IO_PENDING) { ret = READ_COM_ERR; break; }
        }
    }
    return ret;
}

DWORD SweepCOMFromRegedit(void (*SweepCallback)(const LPBYTE pCOMId, HANPCSTR pCOMName, DWORD nCount, void* pParam), void* pParam)
{
    HKEY hCOMRegedit;
    DWORD ret = 0;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"), 0, KEY_READ, &hCOMRegedit) == ERROR_SUCCESS)
    {
        BYTE pCOMId[PATH_STR_SIZE];
        HANCHAR pCOMName[PATH_STR_SIZE];
        DWORD nCOMIdLen = PATH_STR_SIZE;
        DWORD nCOMNameLen = PATH_STR_SIZE;
        while (RegEnumValue(hCOMRegedit, ret, pCOMName, &nCOMNameLen, NULL, NULL, pCOMId, &nCOMIdLen) != ERROR_NO_MORE_ITEMS)
        {
            SweepCallback(pCOMId, pCOMName, ret, pParam);
            ret++;
            nCOMIdLen = PATH_STR_SIZE;
            nCOMNameLen = PATH_STR_SIZE;
        }
        RegCloseKey(hCOMRegedit);
    }
    return ret;
}

/******************** 窗口 ********************/
#define HANWIN_DEFAULT_CLASS    TEXT("HANWinClass")
#define HANWIN_DEFAULT_TITLE    TEXT("HANWinTitle")

int CreateMainWindow(HINSTANCE hInstance, int nCmdShow, HANPCWIN wWin)
{
    int nRet = 0;
    BOOL bRet = FALSE;
    if (NULL != wWin)
    {
        HANPCSTR cls = HANWIN_DEFAULT_CLASS;
        if (NULL != wWin->cls) { cls = wWin->cls; }
        WNDCLASSEX wcex = {
            .cbSize         = sizeof(WNDCLASSEX),
            .style          = CS_HREDRAW | CS_VREDRAW,
            .lpfnWndProc    = wWin->WndProc,
            .cbClsExtra     = 0,
            .cbWndExtra     = 0,
            .hInstance      = hInstance,
            .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
            .hCursor        = LoadCursor(NULL, IDC_ARROW),
            .hbrBackground  = wWin->background,
            .lpszMenuName   = NULL,
            .lpszClassName  = cls,
            .hIconSm        = NULL,
        };
        RegisterClassEx(&wcex);

        HANPCSTR title = HANWIN_DEFAULT_TITLE;
        if (NULL != wWin->title) { title = wWin->title; }
        HWND hWnd = CreateWindow(cls, title, wWin->style, wWin->x, wWin->y, wWin->w, wWin->h, NULL, NULL, hInstance, NULL);
        if (!hWnd)
        {
            bRet = TRUE;
        }
        if (FALSE == bRet)
        {
            ShowWindow(hWnd, nCmdShow);
            UpdateWindow(hWnd);

            MSG msg;
            // 主消息线程:
            while (FALSE != GetMessage(&msg, NULL, 0, 0))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            nRet = (int)(msg.wParam);
        }
    }
    return nRet;
}

LONG GetRectW(const RECT* rcWin)
{
    return rcWin->right - rcWin->left;
}
LONG GetRectH(const RECT* rcWin)
{
    return rcWin->bottom - rcWin->top;
}
BOOL PosInRect(const POINT* ptPos, const RECT* rcRect)
{
    BOOL bRet;
    if ((ptPos->x >= rcRect->left) &&
        (ptPos->x <= rcRect->right) &&
        (ptPos->y >= rcRect->top) &&
        (ptPos->y <= rcRect->bottom)
    ) { bRet = TRUE; }
    else { bRet = FALSE; }
    return bRet;
}
BOOL SimilarRect(RECT* pDist, const RECT* pSource, const POINT* pAnchor1, const POINT* pAnchor2)
{
    BOOL bRet = TRUE;
    if ((pAnchor1->x == pAnchor2->x) || (pAnchor1->y == pAnchor2->y)) { bRet = FALSE; }
    if (TRUE == bRet)
    {
        LONG wPoint = pAnchor2->x - pAnchor1->x;
        LONG hPoint = pAnchor2->y - pAnchor1->y;
        double kSource = fabs((double)GetRectW(pSource) / (double)GetRectH(pSource));
        double kPoint = fabs((double)(pAnchor1->x - pAnchor2->x) / (double)(pAnchor1->y - pAnchor2->y));
        LONG wDist;
        LONG hDist;
        if (kSource > kPoint)
        {
            hDist = hPoint;
            wDist = (LONG)(hDist * kSource);
        }
        else
        {
            wDist = wPoint;
            hDist = (LONG)(wDist / kSource);
        }
        if (wDist < 0) { wDist = -wDist; }
        if (hDist < 0) { hDist = -hDist; }
        if (wPoint > 0)
        {
            pDist->left = pAnchor1->x;
            pDist->right = pDist->left + wDist;
        }
        else
        {
            pDist->right = pAnchor1->x;
            pDist->left = pDist->right - wDist;
        }
        if (hPoint > 0)
        {
            pDist->top = pAnchor1->y;
            pDist->bottom = pDist->top + hDist;
        }
        else
        {
            pDist->bottom = pAnchor1->y;
            pDist->top = pDist->bottom - hDist;
        }
    }
    return TRUE;
}
LONG GetWindowStyle(HWND hWnd)
{
    return GetWindowLong(hWnd, GWL_STYLE);
}
LOGFONT GetDefaultLogFont(LONG lfHeight, LONG lfWidth, HANPCSTR pFaceName)
{
    LOGFONT lfRet = {
        .lfHeight = lfHeight,
        .lfWidth = lfWidth,
        .lfEscapement = 0,
        .lfOrientation = 0,
        .lfWeight = FW_NORMAL,
        .lfItalic = FALSE,
        .lfUnderline = FALSE,
        .lfStrikeOut = FALSE,
        .lfCharSet = DEFAULT_CHARSET,
        .lfOutPrecision = OUT_CHARACTER_PRECIS,
        .lfClipPrecision = CLIP_CHARACTER_PRECIS,
        .lfQuality = DEFAULT_QUALITY,
        .lfPitchAndFamily = FF_DONTCARE,
    };

    HAN_strncpy(lfRet.lfFaceName, pFaceName, ArrLen(lfRet.lfFaceName));

    return lfRet;
}

// button
static void DrawButtonBackgroundPressDown(DRAWITEMSTRUCT* pDrawItem);
static void DrawButtonBackgroundPopUp(DRAWITEMSTRUCT* pDrawItem);
int ButtonGetCheck(HWND hButton)
{
    return (int)SendMessage(hButton, BM_GETCHECK, 0, 0);
}
void ButtonSetChecked(HWND hButton)
{
    SendMessage(hButton, BM_SETCHECK, BST_CHECKED, 0);
}
void ButtonSetIndeterminate(HWND hButton)
{
    SendMessage(hButton, BM_SETCHECK, BST_INDETERMINATE, 0);
}
void ButtonSetUnchecked(HWND hButton)
{
    SendMessage(hButton, BM_SETCHECK, BST_UNCHECKED, 0);
}
void DrawButtonBackground(DRAWITEMSTRUCT* pDrawItem, BOOL bDrawFocusDot)
{
    if (0 != (pDrawItem->itemState & ODS_SELECTED))
    {
        DrawButtonBackgroundPressDown(pDrawItem);
    }
    else
    {
        DrawButtonBackgroundPopUp(pDrawItem);
    }

    if ((TRUE == bDrawFocusDot) && (0 != pDrawItem->itemState))
    {
        HPEN hPen = CreatePen(PS_DOT, 1, RGB(0x00, 0x00, 0x00));
        HPEN hOldPen = SelectObject(pDrawItem->hDC, hPen);
        HBRUSH hOldBrush = SelectObject(pDrawItem->hDC, GetStockObject(NULL_BRUSH));

        Rectangle(pDrawItem->hDC, 4, 4, pDrawItem->rcItem.right - 4, pDrawItem->rcItem.bottom - 4);

        SelectObject(pDrawItem->hDC, hOldPen);
        SelectObject(pDrawItem->hDC, hOldBrush);
        DeleteObject(hPen);
    }
}
static void DrawButtonBackgroundPressDown(DRAWITEMSTRUCT* pDrawItem)
{
    HDC hdc = pDrawItem->hDC;
    RECT* rcClient = &(pDrawItem->rcItem);

    HPEN hPen1 = CreatePen(PS_SOLID, 1, RGB(0x64, 0x64, 0x64));
    HPEN hPen2 = CreatePen(PS_SOLID, 1, RGB(0xA0, 0xA0, 0xA0));
    HBRUSH hBrush = CreateSolidBrush(RGB(0xF0, 0xF0, 0xF0));

    HPEN hOldPen = SelectObject(hdc, hPen1);
    HBRUSH hOldBrush = SelectObject(hdc, hBrush);

    Rectangle(hdc, 0, 0, rcClient->right, rcClient->bottom);
    SelectObject(hdc, hPen2);
    Rectangle(hdc, 1, 1, rcClient->right - 1, rcClient->bottom - 1);

    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);

    DeleteObject(hPen1);
    DeleteObject(hPen2);
    DeleteObject(hBrush);
}
static void DrawButtonBackgroundPopUp(DRAWITEMSTRUCT* pDrawItem)
{
    HDC hdc = pDrawItem->hDC;
    RECT* rcClient = &(pDrawItem->rcItem);

    HPEN hPen1 = CreatePen(PS_SOLID, 1, RGB(0x64, 0x64, 0x64));
    HPEN hPen2 = CreatePen(PS_SOLID, 1, RGB(0x69, 0x69, 0x69));
    HPEN hPen3 = GetStockObject(WHITE_PEN);
    HPEN hPen4 = CreatePen(PS_SOLID, 1, RGB(0xA0, 0xA0, 0xA0));
    HPEN hPen5 = CreatePen(PS_SOLID, 1, RGB(0xE3, 0xE3, 0xE3));
    HBRUSH hBrush = CreateSolidBrush(RGB(0xF0, 0xF0, 0xF0));

    HPEN hOldPen = SelectObject(hdc, hPen1);
    HBRUSH hOldBrush = SelectObject(hdc, hBrush);

    POINT ptLine2[3] = {
        [0] = { rcClient->right - 2, 1 },
        [1] = { rcClient->right - 2, rcClient->bottom - 2 },
        [2] = { 0, rcClient->bottom - 2 },
    };
    POINT ptLine3[3] = {
        [0] = { 1, rcClient->bottom - 3 },
        [1] = { 1, 1 },
        [2] = { rcClient->right - 2, 1 },
    };
    POINT ptLine4[3] = {
        [0] = { rcClient->right - 3, 2 },
        [1] = { rcClient->right - 3, rcClient->bottom - 3 },
        [2] = { 1, rcClient->bottom - 3 },
    };
    POINT ptLine5[3] = {
        [0] = { 2, rcClient->bottom - 4 },
        [1] = { 2, 2 },
        [2] = { rcClient->right - 3, 2 },
    };

    Rectangle(hdc, 0, 0, rcClient->right, rcClient->bottom);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));

    Rectangle(hdc, 0, 0, rcClient->right, rcClient->bottom);
    SelectObject(hdc, hPen2);
    Polyline(hdc, ptLine2, ArrLen(ptLine2));
    SelectObject(hdc, hPen3);
    Polyline(hdc, ptLine3, ArrLen(ptLine3));
    SelectObject(hdc, hPen4);
    Polyline(hdc, ptLine4, ArrLen(ptLine4));
    SelectObject(hdc, hPen5);
    Polyline(hdc, ptLine5, ArrLen(ptLine5));

    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);

    DeleteObject(hPen1);
    DeleteObject(hPen2);
    DeleteObject(hPen4);
    DeleteObject(hPen5);
    DeleteObject(hBrush);
}
// edit
void EditSetLimitText(HWND hEdit, size_t Len)
{
    SendMessage(hEdit, EM_SETLIMITTEXT, Len, 0);
}
void EditSetSel(HWND hEdit, int StartChar, int EndChar)
{
    SendMessage(hEdit, EM_SETSEL, StartChar, EndChar);
}
void EditAppendText(HWND hEdit, HANPCSTR pString, BOOL Undo)
{
    SendMessage(hEdit, EM_SETSEL, -1, -1);
    SendMessage(hEdit, EM_REPLACESEL, (WPARAM)Undo, (LPARAM)pString);
}
// listbox
int ListBoxAddString(HWND hListBox, HANPCSTR pString)
{
    return (int)SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)pString);
}
int ListBoxAddStringArr(HWND hListBox, HANPCSTR* StringArr, size_t nMaxCount)
{
    int nRet = 0;
    for (size_t i = 0; i < nMaxCount; i++)
    {
        nRet = (int)SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)(StringArr[i]));
        if ((nRet == LB_ERR) || (nRet == LB_ERRSPACE)) { break; }
    }
    return nRet;
}
int ListBoxAddStringStructArr(HWND hListBox, const void* pStruct, size_t nStructSize, size_t nOffset, size_t nMaxCount)
{
    int nRet = 0;
    for (size_t i = 0; i < nMaxCount; i++)
    {
        nRet = (int)SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)(*(HANPCSTR*)(((BYTE*)pStruct) + (nStructSize * i) + nOffset)));
        if ((nRet == LB_ERR) || (nRet == LB_ERRSPACE)) { break; }
    }
    return nRet;
}
int ListBoxGetCursel(HWND hListBox)
{
    return (int)SendMessage(hListBox, LB_GETCURSEL, 0, 0);
}
int ListBoxSetCursel(HWND hListBox, int Index)
{
    return (int)SendMessage(hListBox, LB_SETCURSEL, (WPARAM)Index, 0);
}
// combobox
int ComboBoxAddString(HWND hComboBox, HANPCSTR pString)
{
    return (int)SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)pString);
}
int ComboBoxAddStringArr(HWND hComboBox, const HANPCSTR* StringArr, size_t nMaxCount)
{
    int nRet = 0;
    for (size_t i = 0; i < nMaxCount; i++)
    {
        nRet = (int)SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)(StringArr[i]));
        if ((nRet == CB_ERR) || (nRet == CB_ERRSPACE)) { break; }
    }
    return nRet;
}
int ComboBoxAddStringStructArr(HWND hComboBox, const void* pStruct, size_t nStructSize, size_t nOffset, size_t nMaxCount)
{
    int nRet = 0;
    for (size_t i = 0; i < nMaxCount; i++)
    {
        nRet = (int)SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)(*(HANPCSTR*)(((BYTE*)pStruct) + (nStructSize * i) + nOffset)));
        if ((nRet == CB_ERR) || (nRet == CB_ERRSPACE)) { break; }
    }
    return nRet;
}
int ComboBoxDeleteString(HWND hComboBox, int Index)
{
    return (int)SendMessage(hComboBox, CB_DELETESTRING, Index, 0);
}
void ComboBoxClearString(HWND hComboBox)
{
    while (0 < SendMessage(hComboBox, CB_DELETESTRING, 0, 0))
    { }
}
int ComboBoxGetCursel(HWND hComboBox)
{
    return (int)SendMessage(hComboBox, CB_GETCURSEL, 0, 0);
}
int ComboBoxSetCursel(HWND hComboBox, int Index)
{
    return (int)SendMessage(hComboBox, CB_SETCURSEL, (WPARAM)Index, 0);
}
int ComboBoxFindString(HWND hComboBox, int StartIndex, HANPCSTR pString)
{
    return (int)SendMessage(hComboBox, CB_FINDSTRING, StartIndex, (LPARAM)pString);
}
int ComboBoxFindStringEx(HWND hComboBox, int StartIndex, HANPCSTR pString)
{
    return (int)SendMessage(hComboBox, CB_FINDSTRINGEXACT, StartIndex, (LPARAM)pString);
}

#ifdef HAN_CHECK_ALLOC_LEAK
void HANAllocPrintLeakInfo(void)
{
    HANSIZE nSumSize = 0;
    printf("\n");
    for (HANSIZE iLoop = 0; iLoop < sg_allocTab.nMemoryCnt; iLoop++)
    {
        if (NULL != sg_allocTab.pTab[iLoop].pMemory)
        {
            printf(
                "内存ID：" HANSIZE_PRINT_FORMAT "：\n    地址：%p\n    大小：" HANSIZE_PRINT_FORMAT " Byte\n",
                iLoop,
                sg_allocTab.pTab[iLoop].pMemory,
                sg_allocTab.pTab[iLoop].nSize
            );
            nSumSize += sg_allocTab.pTab[iLoop].nSize;
        }
    }
    if (0 == nSumSize) { printf("无内存泄漏\n"); }
    else { printf("总计：" HANSIZE_PRINT_FORMAT " Byte\n", nSumSize); }
}
static HANSIZE HANWinAllocCheckLeak(void* pOld, void* pNew, HANSIZE nSize)
{
    HANPALLOCTAB pTab = sg_allocTab.pTab;
    HANSIZE nTabLen = sg_allocTab.nTabLen;
    HANSIZE nMemoryCnt = sg_allocTab.nMemoryCnt;
    HANSIZE nRet = 0;

    if (NULL != pNew)
    {
        if (nTabLen <= nMemoryCnt)
        {
            pTab = (HANPALLOCTAB)realloc(pTab, sizeof(HANALLOCTAB) * (nTabLen + HAN_CHECK_ALLOC_LEAK_TAB_ALLOC_SIZE));
            if (NULL != pTab)
            {
                for (HANSIZE iloop = nTabLen; iloop < nTabLen + HAN_CHECK_ALLOC_LEAK_TAB_ALLOC_SIZE; iloop++)
                {
                    pTab[iloop].pMemory = NULL;
                    pTab[iloop].nSize = 0;
                    pTab[iloop].nEditTimes = 0;
                }

                sg_allocTab.pTab = pTab;
                sg_allocTab.nTabLen += HAN_CHECK_ALLOC_LEAK_TAB_ALLOC_SIZE;
            }
        }

        for (HANSIZE iLoop = 0; iLoop <= nMemoryCnt; iLoop++)
        {
            if (iLoop == nMemoryCnt)
            {
                pTab[iLoop].pMemory = pNew;
                pTab[iLoop].nSize = nSize;
                pTab[iLoop].nEditTimes = 1;
                sg_allocTab.nMemoryCnt++;
#ifdef HAN_CHECK_ALLOC_PRINT_ALLOC_FREE_LOG
                printf("分配新地址，ID：" HANSIZE_PRINT_FORMAT "：\n"
                    "    新地址：%p，大小：" HANSIZE_PRINT_FORMAT "\n",
                    iLoop, pNew, nSize);
#endif
                nRet = iLoop;
            }
            else if (pTab[iLoop].pMemory == pOld)
            {
#ifdef HAN_CHECK_ALLOC_PRINT_ALLOC_FREE_LOG
                printf("重新分配旧地址，ID：" HANSIZE_PRINT_FORMAT "：\n"
                    "    旧地址：%p，大小：" HANSIZE_PRINT_FORMAT "\n"
                    "    新地址：%p，大小：" HANSIZE_PRINT_FORMAT "\n"
                    "    修改次数：" HANSIZE_PRINT_FORMAT "\n",
                    iLoop, pOld, pTab[iLoop].nSize, pNew, nSize, pTab[iLoop].nEditTimes);
#endif
                pTab[iLoop].pMemory = pNew;
                pTab[iLoop].nSize = nSize;
                pTab[iLoop].nEditTimes++;
                nRet = iLoop;
                break;
            }
            else
            {
                /* No Operation */
            }
        }
    }

    return nRet;
}
static void HANWinFreeCheckLeak(void* pMemory)
{
    for (HANSIZE iLoop = 0; iLoop < sg_allocTab.nMemoryCnt; iLoop++)
    {
        if (sg_allocTab.pTab[iLoop].pMemory == pMemory)
        {

#ifdef HAN_CHECK_ALLOC_PRINT_ALLOC_FREE_LOG
            printf("释放内存，ID：" HANSIZE_PRINT_FORMAT "：\n"
                "    地址：%p，大小：" HANSIZE_PRINT_FORMAT "\n"
                "    修改次数：" HANSIZE_PRINT_FORMAT "\n",
                iLoop, sg_allocTab.pTab[iLoop].pMemory, sg_allocTab.pTab[iLoop].nSize, sg_allocTab.pTab[iLoop].nEditTimes);
#endif

            sg_allocTab.pTab[iLoop].pMemory = NULL;
            sg_allocTab.pTab[iLoop].nSize = 0;
            sg_allocTab.pTab[iLoop].nEditTimes = 0;

            break;
        }
    }
}
#endif
