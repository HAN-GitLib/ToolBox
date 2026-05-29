#include <stdio.h>

#include "HAN_Asc.h"
#include "..\GlobalVariables.h"

typedef enum {
    WID_ASC_VIEW_BASE = HAN_WID_BASE,
    WID_ASC_VIEW_EDIT,
    WID_ASC_VIEW_OPEN_ASC_FILE_BUTTON,
    WID_ASC_VIEW_GROUP_BY_ID,
    WID_ASC_VIEW_GROUP_BY_CHANNEL,
    WID_ASC_VIEW_TO_CSV,
} WIDASCVIEW;

typedef struct tagASCVIEWTEXT {
    HANPSTR     pText;
    uint32_t    nTextBufSize;
    uint32_t    nTextLen;
    ASCFILE     afAscFile;
} ASCVIEWTEXT, * PASCVIEWTEXT;

typedef struct tagASCVIEWWNDEXTRA {
    HANDLE          hHeap;
    ASCVIEWTEXT     atAscText;
    HWND            hOpenAscFileButton;
    HWND            hGroupById;
    HWND            hGroupByChannel;
    HWND            hAscToCsv;
    HWND            hAscViewEdit;
    HFONT           hHexFont;
    HFONT           hSysFont;
    BOOL            bAscOpen;
} ASCVIEWWNDEXTRA, * PASCVIEWWNDEXTRA;

static HANERROR HANAscFileTextToAsc(HANDLE hHeap, PASCFILE pAscFile, char* pText);

static LRESULT CALLBACK AscViewWndProc(HWND hAscView, UINT message, WPARAM wParam, LPARAM lParam);
static void PrintAscView(PASCVIEWWNDEXTRA avInfo);
static LRESULT CreateCallback(HWND hWnd, LPARAM lParam);
static void CommandCallback(HWND hWnd, PASCVIEWWNDEXTRA avInfo, WPARAM wParam, LPARAM lParam);
static void OpenFileAction(PASCVIEWWNDEXTRA avInfo);
static void AscToCsvAction(PASCVIEWWNDEXTRA avInfo);
static inline char* GoToNextLine(char* pText);
static inline char* IgnoreSpace(char* pText);

void RegisterHANAscView(HINSTANCE hInst)
{
    WNDCLASSEX wcex = {
        .cbSize         = sizeof(WNDCLASSEX),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = AscViewWndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = sizeof(PASCVIEWWNDEXTRA),
        .hInstance      = hInst,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName   = NULL,
        .lpszClassName  = HAN_ASC_VIEW_CLASS,
        .hIconSm        = NULL,
    };
    RegisterClassEx(&wcex);
}

HANERROR HANOpenAscFile(PASCFILE pAscFile, HANPCSTR pFileName, HANDLE hHeap)
{
    HANERROR nRet = RET_OK;
    HANDLE hFile = CreateFile(pFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    DWORD nFileSize;
    
    if (INVALID_HANDLE_VALUE == hFile) { nRet = OPEN_FILE_ERR; }

    if (RET_OK == nRet)
    {
        nFileSize = GetFileSize(hFile, NULL);
        char* pData = (char*)HANWinHeapAlloc(hHeap, NULL, nFileSize);
        if (NULL == pData) { nRet = NOT_ENOUGH_MEMORY; }
        else
        {
            DWORD nReadSize;
            (void)ReadFile(hFile, pData, nFileSize, &nReadSize, NULL);
            pData[nFileSize] = '\0';
            nRet = HANAscFileTextToAsc(hHeap, pAscFile, pData);
            HANWinHeapFree(hHeap, 0, pData);
        }
    }

    if (INVALID_HANDLE_VALUE != hFile) { CloseHandle(hFile); }

    return nRet;
}
HANERROR AscViewAscToCsv(PASCFILE pAscFile, HANPCSTR pFileName, const uint32_t* pIdFilter, const uint32_t* pChFilter)
{
    HANERROR nRet = OPEN_FILE_ERR;
    uint32_t nLogCnt = pAscFile->nCanLogCnt;
    PCANLOG pCanLog = pAscFile->pCanLog;
    BOOL bId;
    BOOL bCh;
    HANCHAR pText[ASC_CSV_LINE_BUF_SIZE] = TEXT("时间,通道,ID,收/发,长度,数据\r\n");
    HANDLE hFile = CreateFile(pFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    HANSIZE nOffset;

    if (INVALID_HANDLE_VALUE != hFile)
    {
        WriteFile(hFile, pText, (DWORD)(HAN_strlen(pText) * sizeof(HANCHAR)), NULL, NULL);
        for (uint32_t i = 0; i < nLogCnt; i++)
        {
            if ((NULL == pIdFilter) || (pCanLog[i].canId == *pIdFilter)) { bId = TRUE; }
            else { bId = FALSE; }
            if ((NULL == pChFilter) || (pCanLog[i].nChannel == *pChFilter)) { bCh = TRUE; }
            else { bCh = FALSE; }
            if ((TRUE == bId) && (TRUE == bCh))
            {
                nOffset = 0;
                HAN_snprintf(&pText[nOffset], ASC_CSV_LINE_BUF_SIZE - nOffset, TEXT("%.6lf,"), pCanLog[i].tTime);
                nOffset += HAN_strlen(&pText[nOffset]);
                HAN_snprintf(&pText[nOffset], ASC_CSV_LINE_BUF_SIZE - nOffset, TEXT("%u,"), pCanLog[i].nChannel);
                nOffset += HAN_strlen(&pText[nOffset]);
                HAN_snprintf(&pText[nOffset], ASC_CSV_LINE_BUF_SIZE - nOffset, TEXT("%X,"), pCanLog[i].canId);
                nOffset += HAN_strlen(&pText[nOffset]);
                if (CAN_LOG_TR_RX == pCanLog[i].cTxRx)
                {
                    HAN_snprintf(&pText[nOffset], ASC_CSV_LINE_BUF_SIZE - nOffset, TEXT("Rx,"));
                    nOffset += HAN_strlen(&pText[nOffset]);
                }
                else
                {
                    HAN_snprintf(&pText[nOffset], ASC_CSV_LINE_BUF_SIZE - nOffset, TEXT("Tx,"));
                    nOffset += HAN_strlen(&pText[nOffset]);
                }
                HAN_snprintf(&pText[nOffset], ASC_CSV_LINE_BUF_SIZE - nOffset, TEXT("%u,"), pCanLog[i].nLen);
                nOffset += HAN_strlen(&pText[nOffset]);
                for (uint32_t j = 0; j < pCanLog[i].nLen; j++)
                {
                    HAN_snprintf(&pText[nOffset], ASC_CSV_LINE_BUF_SIZE - nOffset, TEXT("%02X "), pCanLog[i].pData[j]);
                    nOffset += HAN_strlen(&pText[nOffset]);
                }
                HAN_snprintf(&pText[nOffset], ASC_CSV_LINE_BUF_SIZE - nOffset, TEXT("\r\n"));
                nOffset += HAN_strlen(&pText[nOffset]);
                WriteFile(hFile, pText, (DWORD)nOffset, NULL, NULL);
            }
        }
        CloseHandle(hFile);
        nRet = RET_OK;
    }

    return nRet;
}

static HANERROR HANAscFileTextToAsc(HANDLE hHeap, PASCFILE pAscFile, char* pText)
{
    HANERROR nRet = RET_OK;
    uint32_t nCanLogCnt = 0;
    PCANLOG pTemp;
    char* pLine = pText;
    char* pLineEnd = pLine;
    char* pNumEnd;
    char* pRx;
    char* pTx;

    while('\0' != pLine[0])
    {
        if ((nCanLogCnt % ASC_CAN_LOG_ALLOC_STEP) == 0)
        {
            pTemp = (PCANLOG)HANWinHeapAlloc(hHeap, pAscFile->pCanLog, sizeof(CANLOG) * (nCanLogCnt + ASC_CAN_LOG_ALLOC_STEP));
            if (NULL == pTemp) { nRet = NOT_ENOUGH_MEMORY; }
            else { pAscFile->pCanLog = pTemp; }
        }
        if (RET_OK == nRet)
        {
            pLineEnd = GoToNextLine(pLine);
            pLineEnd[0] = TEXT('\0');
            pRx = strstr(pLine, "Rx");
            pTx = strstr(pLine, "Tx");
            if (((NULL != pRx) && (pRx < pLineEnd)) || ((NULL != pTx) && (pTx < pLineEnd)))
            {
                pTemp[nCanLogCnt].tTime = strtod(pLine, &pNumEnd);
                if (pNumEnd != pLine)
                {
                    pLine = pNumEnd;
                    pTemp[nCanLogCnt].nChannel = strtoul(pLine, &pLine, 10);
                    pTemp[nCanLogCnt].canId = strtoul(pLine, &pLine, 16);
                    pLine = IgnoreSpace(pLine);
                    if (('R' == pLine[0]) && ('x' == pLine[1])) { pTemp[nCanLogCnt].cTxRx = CAN_LOG_TR_RX; }
                    else if (('T' == pLine[0]) && ('x' == pLine[1])) { pTemp[nCanLogCnt].cTxRx = CAN_LOG_TR_TX; }
                    else { continue; }
                    pLine = &pLine[2];
                    pLine = IgnoreSpace(pLine); pLine = &pLine[1];
                    pTemp[nCanLogCnt].nLen = strtoul(pLine, &pLine, 10);
                    for (uint32_t i = 0; i < pTemp[nCanLogCnt].nLen; i++)
                    {
                        pTemp[nCanLogCnt].pData[i] = (uint8_t)strtoul(pLine, &pLine, 16);
                    }
                    nCanLogCnt++;
                }
            }
            pLine = &pLineEnd[1];
        }
        else { break; }
    }
    if (RET_OK == nRet)
    {
        pAscFile->nCanLogCnt = nCanLogCnt;
    }

    return nRet;
}

static LRESULT CALLBACK AscViewWndProc(HWND hAscView, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    PASCVIEWWNDEXTRA avInfo = (PASCVIEWWNDEXTRA)GetWindowLongPtr(hAscView, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = CreateCallback(hAscView, lParam);
        } break;
        case WM_CTLCOLORSTATIC: {
            lWndProcRet = (INT_PTR)GetStockObject(WHITE_BRUSH);
        } break;
        case WM_COMMAND: {
            CommandCallback(hAscView, avInfo, wParam, lParam);
        } break;
        case WM_DESTROY: {
            HANWinHeapFree(avInfo->hHeap, 0, avInfo);
            lWndProcRet = DefWindowProc(hAscView, message, wParam, lParam);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hAscView, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}
static void PrintAscView(PASCVIEWWNDEXTRA avInfo)
{
    PCCANLOG pCanLog = avInfo->atAscText.afAscFile.pCanLog;
    PASCFILE pAscFile = &(avInfo->atAscText.afAscFile);
    HANPSTR pText = avInfo->atAscText.pText;
    uint32_t nTextBufSize = avInfo->atAscText.nTextBufSize;
    HANSIZE nOffset = 0;

    for (uint32_t i = 0; i < pAscFile->nCanLogCnt; i++)
    {
        HAN_snprintf(&pText[nOffset], nTextBufSize - nOffset, TEXT("%016.6lf "), pCanLog[i].tTime);
        nOffset += HAN_strlen(&pText[nOffset]);
        HAN_snprintf(&pText[nOffset], nTextBufSize - nOffset, TEXT("%4u  "), pCanLog[i].nChannel);
        nOffset += HAN_strlen(&pText[nOffset]);
        HAN_snprintf(&pText[nOffset], nTextBufSize - nOffset, TEXT("%6X  "), pCanLog[i].canId);
        nOffset += HAN_strlen(&pText[nOffset]);
        if (CAN_LOG_TR_RX == pCanLog[i].cTxRx)
        {
            HAN_snprintf(&pText[nOffset], nTextBufSize - nOffset, TEXT(" Tx "));
            nOffset += HAN_strlen(&pText[nOffset]);
        }
        else
        {
            HAN_snprintf(&pText[nOffset], nTextBufSize - nOffset, TEXT(" Rx "));
            nOffset += HAN_strlen(&pText[nOffset]);
        }
        HAN_snprintf(&pText[nOffset], nTextBufSize - nOffset, TEXT("%4u    "), pCanLog[i].nLen);
        nOffset += HAN_strlen(&pText[nOffset]);
        for (uint32_t j = 0; j < pCanLog[i].nLen; j++)
        {
            HAN_snprintf(&pText[nOffset], nTextBufSize - nOffset, TEXT("%02X "), pCanLog[i].pData[j]);
            nOffset += HAN_strlen(&pText[nOffset]);
        }
        HAN_snprintf(&pText[nOffset], nTextBufSize - nOffset, TEXT("\r\n"));
        nOffset += HAN_strlen(&pText[nOffset]);
    }
    pText[nTextBufSize - 1] = TEXT('\0');
    SetWindowText(avInfo->hAscViewEdit, pText);
}
static LRESULT CreateCallback(HWND hWnd, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PASCVIEWWNDEXTRA avInfo;
    HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
    RECT rcClientSize;

    HANDLE hHeap = GetProcessHeap();
    if (NULL == hHeap) { lWndProcRet = -1; }
    if (-1 != lWndProcRet)
    {
        avInfo = (PASCVIEWWNDEXTRA)HANWinHeapAlloc(hHeap, NULL, sizeof(ASCVIEWWNDEXTRA));
        if (NULL == avInfo) { lWndProcRet = -1; }
    }

    if (-1 != lWndProcRet)
    {
        int nWinX;
        int nWinY;

        SetWindowLongPtr(hWnd, 0, (LONG_PTR)avInfo);
        avInfo->hHeap = hHeap;

        GetClientRect(hWnd, &rcClientSize);

        avInfo->hHexFont = CreateFontIndirect(&g_lfHexFont);
        avInfo->hSysFont = CreateFontIndirect(&g_lfSysFont);

        avInfo->hOpenAscFileButton = CreateWindow(TEXT("button"), TEXT("打开Asc文件"),
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 30, 30, 130, 30,
            hWnd, (HMENU)WID_ASC_VIEW_OPEN_ASC_FILE_BUTTON, hInst, NULL);

        nWinX = 300;
        avInfo->hAscViewEdit = CreateWindow(
            TEXT("edit"), NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_READONLY,
            nWinX, 10, 800, GetRectH(&rcClientSize) - 50,
            hWnd, (HMENU)WID_ASC_VIEW_EDIT, hInst, NULL);

        nWinX = 40; nWinY = 100;
        avInfo->hGroupById = CreateWindow(TEXT("combobox"), NULL,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_SORT, nWinX, nWinY, 95, 100,
            hWnd, (HMENU)WID_ASC_VIEW_GROUP_BY_ID, hInst, NULL);
        nWinX += 120;
        avInfo->hGroupByChannel = CreateWindow(TEXT("combobox"), NULL,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_SORT, nWinX, nWinY, 115, 100,
            hWnd, (HMENU)WID_ASC_VIEW_GROUP_BY_CHANNEL, hInst, NULL);
        nWinX = 30; nWinY += 35;
        avInfo->hAscToCsv = CreateWindow(TEXT("button"), TEXT("生成CSV文件"),
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, nWinX, nWinY, 120, 25,
            hWnd, (HMENU)WID_ASC_VIEW_TO_CSV, hInst, NULL);

        SendMessage(avInfo->hOpenAscFileButton, WM_SETFONT, (WPARAM)(avInfo->hSysFont), (LPARAM)TRUE);
        SendMessage(avInfo->hGroupById, WM_SETFONT, (WPARAM)(avInfo->hSysFont), (LPARAM)TRUE);
        SendMessage(avInfo->hGroupByChannel, WM_SETFONT, (WPARAM)(avInfo->hSysFont), (LPARAM)TRUE);
        SendMessage(avInfo->hAscToCsv, WM_SETFONT, (WPARAM)(avInfo->hSysFont), (LPARAM)TRUE);
        SendMessage(avInfo->hAscViewEdit, WM_SETFONT, (WPARAM)(avInfo->hHexFont), (LPARAM)TRUE);

        ComboBoxAddString(avInfo->hGroupById, TEXT("无过滤"));
        ComboBoxAddString(avInfo->hGroupByChannel, TEXT("无过滤"));
        ComboBoxSetCursel(avInfo->hGroupById, 0);
        ComboBoxSetCursel(avInfo->hGroupByChannel, 0);

        avInfo->atAscText.afAscFile.pCanLog = NULL;
        avInfo->bAscOpen = FALSE;
    }

    return lWndProcRet;
}
static void CommandCallback(HWND hWnd, PASCVIEWWNDEXTRA avInfo, WPARAM wParam, LPARAM lParam)
{
    (void)hWnd;
    (void)lParam;

    switch (LOWORD(wParam)) {
        case WID_ASC_VIEW_OPEN_ASC_FILE_BUTTON: {
            OpenFileAction(avInfo);
            break;
        }
        case WID_ASC_VIEW_TO_CSV: {
            if (TRUE == avInfo->bAscOpen) { AscToCsvAction(avInfo); }
            break;
        }

        default: {
            break;
        }
    }
}
static void OpenFileAction(PASCVIEWWNDEXTRA avInfo)
{
    HANCHAR pInputName[PATH_STR_SIZE] = { 0 };
    OPENFILENAME ofnOpenFile;
    ZeroMemory(&ofnOpenFile, sizeof(ofnOpenFile));
    ofnOpenFile.lStructSize = sizeof(ofnOpenFile);
    ofnOpenFile.hwndOwner = NULL;
    ofnOpenFile.lpstrFilter = TEXT("所有文件\0*.*\0\0");
    ofnOpenFile.nFilterIndex = 0;
    ofnOpenFile.lpstrFile = pInputName;
    ofnOpenFile.nMaxFile = PATH_STR_SIZE;
    ofnOpenFile.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
    if (GetOpenFileName(&ofnOpenFile))
    {
        HANOpenAscFile(&(avInfo->atAscText.afAscFile), pInputName, avInfo->hHeap);
        HANDLE hFile = CreateFile(pInputName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        uint32_t nLogCnt = avInfo->atAscText.afAscFile.nCanLogCnt;
        PCANLOG pCanLog = avInfo->atAscText.afAscFile.pCanLog;
        HANCHAR pIdText[ASC_CAN_ID_TEXT_BUF_SIZE];
        HANCHAR pChText[ASC_CAN_CH_TEXT_BUF_SIZE];
        for (uint32_t i = 0; i < nLogCnt; i++)
        {
            HAN_snprintf(pIdText, ASC_CAN_ID_TEXT_BUF_SIZE, TEXT("%X"), pCanLog[i].canId);
            HAN_snprintf(pChText, ASC_CAN_CH_TEXT_BUF_SIZE, TEXT("通道%u"), pCanLog[i].nChannel);
            if (CB_ERR == ComboBoxFindStringEx(avInfo->hGroupById, 0, pIdText))
            {
                ComboBoxAddString(avInfo->hGroupById, pIdText);
            }
            if (CB_ERR == ComboBoxFindStringEx(avInfo->hGroupByChannel, 0, pChText))
            {
                ComboBoxAddString(avInfo->hGroupByChannel, pChText);
            }
        }
        if (INVALID_HANDLE_VALUE != hFile)
        {
            HANPSTR pText = HANWinHeapAlloc(avInfo->hHeap, avInfo->atAscText.pText, GetFileSize(hFile, NULL));
            if (NULL != pText)
            {
                avInfo->atAscText.pText = pText;
                avInfo->atAscText.nTextBufSize = GetFileSize(hFile, NULL);
                PrintAscView(avInfo);
                avInfo->bAscOpen = TRUE;
            }
            CloseHandle(hFile);
        }
        avInfo->bAscOpen = TRUE;
    }
}
static void AscToCsvAction(PASCVIEWWNDEXTRA avInfo)
{
    HANCHAR pInputName[PATH_STR_SIZE] = { 0 };
    OPENFILENAME ofnOpenFile;
    ZeroMemory(&ofnOpenFile, sizeof(ofnOpenFile));
    ofnOpenFile.lStructSize = sizeof(ofnOpenFile);
    ofnOpenFile.hwndOwner = NULL;
    ofnOpenFile.lpstrFilter = TEXT("所有文件\0*.*\0\0");
    ofnOpenFile.nFilterIndex = 0;
    ofnOpenFile.lpstrFile = pInputName;
    ofnOpenFile.nMaxFile = PATH_STR_SIZE;
    ofnOpenFile.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
    if (GetSaveFileName(&ofnOpenFile))
    {
        HANCHAR pIdText[ASC_CAN_ID_TEXT_BUF_SIZE];
        HANCHAR pChText[ASC_CAN_CH_TEXT_BUF_SIZE];
        uint32_t nId;
        uint32_t nCh;
        uint32_t* pId = NULL;
        uint32_t* pCh = NULL;
        GetWindowText(avInfo->hGroupById, pIdText, ASC_CAN_ID_TEXT_BUF_SIZE);
        GetWindowText(avInfo->hGroupByChannel, pChText, ASC_CAN_CH_TEXT_BUF_SIZE);
        if (0 != HAN_strcmp(pIdText, TEXT("无过滤"))) { pId = &nId; nId = HAN_strtoul(pIdText, NULL, 16); }
        if (0 != HAN_strcmp(pChText, TEXT("无过滤")))
        {
            pCh = &nCh;
            nCh = HAN_strtoul(&pChText[HAN_strlen(TEXT("通道"))], NULL, 10);
        }
        AscViewAscToCsv(&(avInfo->atAscText.afAscFile), pInputName, pId, pCh);
    }
}
static inline char* GoToNextLine(char* pText)
{
    char* pRet = pText;
    while ((pRet[0] != TEXT('\n')) && (pRet[0] != TEXT('\0'))) { pRet = &pRet[1]; }
    return pRet;
}
static inline char* IgnoreSpace(char* pText)
{
    char* pRet = pText;
    while (HAN_isspace(*pRet) && (pRet[0] != TEXT('\0'))) { pRet = &pRet[1]; }
    return pRet;
}
