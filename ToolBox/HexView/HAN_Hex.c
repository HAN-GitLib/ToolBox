#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "..\..\HAN_Lib\HAN_windows.h"
#include "..\GlobalVariables.h"
#include "HAN_Hex.h"

#define HEX_FILE_EOF_TEXT       ":00000001FF\r\n"

#define INI_HEX_VIEW_VALUE_SIZE         INI_MAIN_VALUE_STR_SIZE
#define INI_HEX_VIEW_APP_NAME           TEXT("hvConfig")

typedef enum {
    WID_HEX_VIEW_BASE = HAN_WID_BASE,
    WID_HEX_VIEW_VIEW,
    WID_HEX_VIEW_HEX_MEM_MAP_EDIT,
    WID_HEX_VIEW_OPEN_HEX_FILE_BUTTON,
    WID_HEX_VIEW_START_ADDR_STATIC,
    WID_HEX_VIEW_START_ADDR_EDIT,
    WID_HEX_VIEW_END_ADDR_STATIC,
    WID_HEX_VIEW_END_ADDR_EDIT,
    WID_HEX_VIEW_SAVE_BIN_FILE,
    WID_HEX_VIEW_SAVE_FILLED_HEX_FILE,
    WID_HEX_VIEW_LINK_FILE_LIST,
    WID_HEX_VIEW_LINK_FILE_ADD,
    WID_HEX_VIEW_LINK_FILE_SUB,
    WID_HEX_VIEW_LINK_FILE_LINK,
} WIDHEXVIEW;

typedef struct tagHEXVIEWTEXT {
    HANPSTR     pText;
    uint32_t    nTextBufSize;
    uint32_t    nTextLen;
    HEXFILE     hfHexFile;
} HEXVIEWTEXT, * PHEXVIEWTEXT;

typedef struct tagHEXVIEWWNDEXTRA {
    HANDLE          hHeap;
    HEXVIEWTEXT     htHexText;
    HWND            hOpenHexFileButton;
    HWND            hStartAddrStatic;
    HWND            hStartAddrEdit;
    HWND            hEndAddrStatic;
    HWND            hEndAddrEdit;
    HWND            hSaveBinFile;
    HWND            hSaveFilledHexFileButton;
    HWND            hLinkFileList;
    HWND            hLinkFileAdd;
    HWND            hLinkFileSub;
    HWND            hLinkFileLink;
    HWND            hHexViewEdit;
    HWND            hMemMapEdit;
    HFONT           hHexFont;
    HFONT           hSysFont;
    BOOL            bHexOpen;
    HEXVIEWCFG      pHvConfig;
} HEXVIEWWNDEXTRA, * PHEXVIEWWNDEXTRA;

typedef struct tagHEXVIEWREADWRITECFG {
    HANPCSTR            pKey;
    HANPCSTR            pDefValue;
    void                (*CfgWindowToText)(PHEXVIEWWNDEXTRA bvInfo, HANPSTR pText);
    void                (*CfgTextToWindow)(PHEXVIEWWNDEXTRA bvInfo, HANPCSTR pText);
} HEXVIEWREADWRITECFG;

#define HEX_RECORD_TYPE_DATA                        0
#define HEX_RECORD_TYPE_EOF                         1
#define HEX_RECORD_TYPE_EXTENDED_SEGMENT_ADDRESS    2
#define HEX_RECORD_TYPE_START_SEGMENT_ADDRESS       3
#define HEX_RECORD_TYPE_EXTENDED_LINEAR_ADDRESS     4
#define HEX_RECORD_TYPE_START_LINEAR_ADDRESS        5

static HANERROR HANAllocHexBinBuf(PHEXFILE pHexFile, DWORD nFileSize, HANDLE hHeap);
static HANERROR HANHexFileTextToHex(PHEXFILE pHexFile);
static HANERROR HANGetHexRecordLine(char* pText, char** pEnd, PHEXRECORD pRecord);
static uint8_t HANHexRecordCheck(uint8_t* pRecord, uint32_t nLen);
static HANERROR HANTextToHexValue(const char* pText, uint8_t* pHex);

static LRESULT CALLBACK HexViewWndProc(HWND hHexView, UINT message, WPARAM wParam, LPARAM lParam);
static void PrintHexView(PHEXVIEWWNDEXTRA hvInfo);
static LRESULT CreateCallback(HWND hWnd, LPARAM lParam);
static void LoadHexViewCfg(PHEXVIEWWNDEXTRA hvInfo, PHEXVIEWCFG pHvConfig);
static void CommandCallback(HWND hWnd, PHEXVIEWWNDEXTRA hvInfo, WPARAM wParam, LPARAM lParam);
static void OpenFileAction(PHEXVIEWWNDEXTRA hvInfo);
static void SaveBinFileAction(PHEXVIEWWNDEXTRA hvInfo);
static void AddLinkFileAction(PHEXVIEWWNDEXTRA hvInfo);
static void SubLinkFileAction(PHEXVIEWWNDEXTRA hvInfo);
static void LinkLinkFileAction(PHEXVIEWWNDEXTRA hvInfo);
static void PrintHexMemMap(PHEXVIEWWNDEXTRA hvInfo);
static void AppendHexMap(PHEXVIEWWNDEXTRA hvInfo, PADDRMAP pAddrMap);
static HANERROR SaveFilledHexFileAction(PHEXVIEWWNDEXTRA hvInfo);
static DWORD PrintHexRecord(char* pText, PHEXRECORD pRecord);
static DWORD PrintHexRecordLinearAddr(char* pText, uint16_t cAddr);

static void CfgWindowToTextStartAddr(PHEXVIEWWNDEXTRA hvInfo, HANPSTR pText);
static void CfgWindowToTextEndAddr(PHEXVIEWWNDEXTRA hvInfo, HANPSTR pText);

static void CfgTextToWindowStartAddr(PHEXVIEWWNDEXTRA hvInfo, HANPCSTR pText);
static void CfgTextToWindowEndAddr(PHEXVIEWWNDEXTRA hvInfo, HANPCSTR pText);

static const HEXVIEWREADWRITECFG sg_pHexViewCfgInfo[INI_HEX_VIEW_CFG_CNT] = {
    [INI_HEX_VIEW_START_ADDR] = {
        .pKey = TEXT("StartAddr"),
        .pDefValue = DEFAULT_START_ADDR_TEXT,
        .CfgWindowToText = CfgWindowToTextStartAddr,
        .CfgTextToWindow = CfgTextToWindowStartAddr,
    },
    [INI_HEX_VIEW_END_ADDR] = {
        .pKey = TEXT("EndAddr"),
        .pDefValue = DEFAULT_END_ADDR_TEXT,
        .CfgWindowToText = CfgWindowToTextEndAddr,
        .CfgTextToWindow = CfgTextToWindowEndAddr,
    },
};

void RegisterHANHexView(HINSTANCE hInst)
{
    WNDCLASSEX wcex = {
        .cbSize         = sizeof(WNDCLASSEX),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = HexViewWndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = sizeof(PHEXVIEWWNDEXTRA),
        .hInstance      = hInst,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName   = NULL,
        .lpszClassName  = HAN_HEX_VIEW_CLASS,
        .hIconSm        = NULL,
    };
    RegisterClassEx(&wcex);
}

void ReadHexViewIniFile(HANPCSTR pIniPath, void* pParam)
{
    PHEXVIEWCFG pHvConfig = pParam;
    INIHEXVIEWCFGID iLoop;

    for (iLoop = 0; iLoop < INI_HEX_VIEW_CFG_CNT; iLoop++)
    {
        HAN_strcpy(pHvConfig->pSysConfig[iLoop].pKey, sg_pHexViewCfgInfo[iLoop].pKey);
        HAN_strcpy(pHvConfig->pSysConfig[iLoop].pDefValue, sg_pHexViewCfgInfo[iLoop].pDefValue);
        GetPrivateProfileString(
            INI_HEX_VIEW_APP_NAME, pHvConfig->pSysConfig[iLoop].pKey, pHvConfig->pSysConfig[iLoop].pDefValue,
            pHvConfig->pSysConfig[iLoop].pValue, INI_HEX_VIEW_VALUE_SIZE, pIniPath);
    }
}

void WriteHexViewIniFile(HANPCSTR pIniPath, HWND hHexView)
{
    PHEXVIEWWNDEXTRA hvInfo = (PHEXVIEWWNDEXTRA)GetWindowLongPtr(hHexView, 0);
    HANCHAR pBvCfg[INI_HEX_VIEW_CFG_CNT][INI_HEX_VIEW_VALUE_SIZE];
    INIHEXVIEWCFGID iLoop;
    
    for (iLoop = 0; iLoop < INI_HEX_VIEW_CFG_CNT; iLoop++)
    {
        sg_pHexViewCfgInfo[iLoop].CfgWindowToText(hvInfo, pBvCfg[iLoop]);
        WritePrivateProfileString(
            INI_HEX_VIEW_APP_NAME, sg_pHexViewCfgInfo[iLoop].pKey, pBvCfg[iLoop], pIniPath
        );
    }
}

void BinDataToHexFile(HANDLE hFile, uint32_t cStartAddr, uint8_t* pBinData, uint32_t nLen)
{
    uint16_t cLinearAddr = (cStartAddr >> (uint32_t)16) & (uint32_t)0x0000FFFF;
    uint32_t cRecordAddr = cStartAddr & (uint32_t)0x0000FFFF;
    char pText[64];
    HEXRECORD hrHexRecord = { .cDataType = HEX_RECORD_TYPE_DATA };
    DWORD nWriteLen;
    uint32_t nOffset = 0;

    nWriteLen = PrintHexRecordLinearAddr(pText, cLinearAddr);
    WriteFile(hFile, pText, nWriteLen, NULL, NULL);
    if ((cRecordAddr % (uint32_t)0x00000010) != (uint32_t)0)
    {
        hrHexRecord.cAddr = cRecordAddr;
        hrHexRecord.nDataLen = cRecordAddr % (uint32_t)0x00000010;
        memcpy(hrHexRecord.pData, &pBinData[nOffset], hrHexRecord.nDataLen);
        nWriteLen = PrintHexRecord(pText, &hrHexRecord);
        WriteFile(hFile, pText, nWriteLen, NULL, NULL);
    }
    while (nOffset < nLen)
    {
        if ((uint32_t)0x00010000 <= cRecordAddr)
        {
            cLinearAddr++;
            cRecordAddr &= (uint32_t)0x0000FFFF;
            nWriteLen = PrintHexRecordLinearAddr(pText, cLinearAddr);
            WriteFile(hFile, pText, nWriteLen, NULL, NULL);
        }
        if ((nLen - nOffset) < (uint32_t)0x00000010) { hrHexRecord.nDataLen = nLen - nOffset; }
        else { hrHexRecord.nDataLen = (uint8_t)0x10; }
        hrHexRecord.cAddr = cRecordAddr;
        memcpy(hrHexRecord.pData, &pBinData[nOffset], hrHexRecord.nDataLen);
        nWriteLen = PrintHexRecord(pText, &hrHexRecord);
        WriteFile(hFile, pText, nWriteLen, NULL, NULL);
        nOffset += hrHexRecord.nDataLen;
        cRecordAddr += hrHexRecord.nDataLen;
    }
    WriteFile(hFile, HEX_FILE_EOF_TEXT, (DWORD)strlen(HEX_FILE_EOF_TEXT), NULL, NULL);
}

void LinkHexFile(HANPCSTR pFileDest, HANPSTR pFileSrc, uint32_t nLen)
{
    HANDLE hDest;
    HANDLE hSrc;
    PCHAR pStr = NULL;
    PCHAR pStrTemp;

    hDest = CreateFile(pFileDest, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (INVALID_HANDLE_VALUE != hDest)
    {
        for (uint32_t i = 0; i < nLen; i++)
        {
            hSrc = CreateFile(&pFileSrc[i * PATH_STR_SIZE], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
            if (INVALID_HANDLE_VALUE != hSrc)
            {
                DWORD nFileSize = GetFileSize(hSrc, NULL);
                DWORD nWriteSize;
                pStrTemp = HANWinHeapAlloc(GetProcessHeap(), pStr, nFileSize);
                if (pStrTemp == NULL) { break; }
                else
                {
                    pStr = pStrTemp;
                    (void)ReadFile(hSrc, pStr, nFileSize, NULL, NULL);
                    PCHAR pEof;
                    pEof = strstr(pStr, ":00000001FF\r\n");
                    if (NULL == pEof) { pEof = strstr(pStr, ":00000001FF\n"); }
                    if (NULL == pEof) { nWriteSize = nFileSize; }
                    else { nWriteSize = (DWORD)((intptr_t)pEof - (intptr_t)pStr); }
                    WriteFile(hDest, pStr, nWriteSize, NULL, NULL);
                }
                CloseHandle(hSrc);
            }
        }
        WriteFile(hDest, ":00000001FF\r\n", (DWORD)strlen(":00000001FF\r\n"), NULL, NULL);
    }
    CloseHandle(hDest);
}

HANERROR HANOpenHexFile(PHEXFILE pHexFile, HANPCSTR pFileName, HANDLE hHeap)
{
    HANERROR nRet = RET_OK;
    HANDLE hFile = CreateFile(pFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    DWORD nFileSize;
    
    if (INVALID_HANDLE_VALUE == hFile) { nRet = OPEN_FILE_ERR; }

    if (RET_OK == nRet)
    {
        nFileSize = GetFileSize(hFile, NULL);
        nRet = HANAllocHexBinBuf(pHexFile, nFileSize, hHeap);
    }
    if (RET_OK == nRet)
    {
        DWORD nReadSize;
        (void)ReadFile(hFile, pHexFile->pText, nFileSize, &nReadSize, NULL);
        pHexFile->pText[nFileSize] = '\0';
        pHexFile->nTextLen = (uint32_t)nFileSize;
        nRet = HANHexFileTextToHex(pHexFile);
    }

    if (INVALID_HANDLE_VALUE != hFile) { CloseHandle(hFile); }

    return nRet;
}
HANERROR HexViewHexToBin(PHEXFILE pHexFile, PADDRMAP pAddrMap, HANDLE hHeap)
{
    HANERROR     nRet = RET_OK;
    uint32_t        nBinSize;
    uint8_t*        pBinData;
    ADDRMAP         amRecordMap = { .cStartAddr = 0, .cEndAddr = 0, };
    ADDRMAP         amCopyMap;
    
    nBinSize = (pAddrMap->cEndAddr - pAddrMap->cStartAddr) + (uint32_t)1;
    pBinData = (uint8_t*)HANWinHeapAlloc(hHeap, pHexFile->pBinData, nBinSize);

    if (NULL == pBinData) { nRet = NOT_ENOUGH_MEMORY; }
    if (RET_OK == nRet)
    {
        memset(pBinData, 0xFF, nBinSize);
        pHexFile->pBinData = pBinData;
        pHexFile->amBinAddrMap = *pAddrMap;
        for (uint32_t i = 0; i < pHexFile->nHexRecordCnt; i++)
        {
            PHEXRECORD pRecord = &(pHexFile->pHexRecord[i]);
            switch (pRecord->cDataType) {
                case HEX_RECORD_TYPE_EXTENDED_LINEAR_ADDRESS: {
                    amRecordMap.cStartAddr &= (uint32_t)0x0000FFFF;
                    amRecordMap.cStartAddr |=
                        (((uint32_t)(pRecord->pData[0]) << 8) | ((uint32_t)(pRecord->pData[1]))) << (uint32_t)16;
                    break;
                }
                case HEX_RECORD_TYPE_DATA: {
                    amRecordMap.cStartAddr &= (uint32_t)0xFFFF0000;
                    amRecordMap.cStartAddr |= (uint32_t)(pRecord->cAddr);
                    amRecordMap.cEndAddr = amRecordMap.cStartAddr + pRecord->nDataLen - (uint32_t)1;
                    if (pAddrMap->cStartAddr < amRecordMap.cStartAddr) { amCopyMap.cStartAddr = amRecordMap.cStartAddr; }
                    else { amCopyMap.cStartAddr = pAddrMap->cStartAddr; }
                    if (amRecordMap.cEndAddr < pAddrMap->cEndAddr) { amCopyMap.cEndAddr = amRecordMap.cEndAddr; }
                    else { amCopyMap.cEndAddr = pAddrMap->cEndAddr; }
                    if (amCopyMap.cStartAddr <= amCopyMap.cEndAddr)
                    {
                        uint32_t nDestOffset = amCopyMap.cStartAddr - pAddrMap->cStartAddr;
                        uint32_t nSrcOffset = amCopyMap.cStartAddr - amRecordMap.cStartAddr;
                        uint32_t nCopyLen = (amCopyMap.cEndAddr - amCopyMap.cStartAddr) + (uint32_t)1;
                        memcpy(&pBinData[nDestOffset], &(pRecord->pData[nSrcOffset]), nCopyLen);
                    }
                    break;
                }
            }
        }
    }

    return nRet;
}

static HANERROR HANAllocHexBinBuf(PHEXFILE pHexFile, DWORD nFileSize, HANDLE hHeap)
{
    HANERROR nRet = RET_OK;
    uint32_t nByteMaxLen = ((nFileSize - (nFileSize % (DWORD)2)) + (DWORD)2) / (DWORD)2;
    uint32_t nHexRecordCnt = ((nByteMaxLen - (nByteMaxLen % (uint32_t)6)) + (uint32_t)6) / (uint32_t)6;

    pHexFile->pText = (char*)HANWinHeapAlloc(hHeap, pHexFile->pText, sizeof(char) * (nFileSize + (DWORD)1));
    pHexFile->pHexRecord = (HEXRECORD*)HANWinHeapAlloc(hHeap, pHexFile->pHexRecord, sizeof(HEXRECORD) * nHexRecordCnt);
    if ((NULL == pHexFile->pText) || (NULL == pHexFile->pHexRecord))
    {
        HANWinHeapFree(hHeap, 0, pHexFile->pText);
        HANWinHeapFree(hHeap, 0, pHexFile->pHexRecord);
        pHexFile->pText = NULL;
        pHexFile->pHexRecord = NULL;
        nRet = NOT_ENOUGH_MEMORY;
    }
    pHexFile->nTextLen = 0;
    pHexFile->nHexRecordCnt = 0;

    return nRet;
}
static HANERROR HANHexFileTextToHex(PHEXFILE pHexFile)
{
    HANERROR nRet = RET_OK;
    uint32_t nHexRecordCnt = 0;
    char* pText = pHexFile->pText;

    while('\0' != pText[0])
    {
        nRet = HANGetHexRecordLine(pText, &pText, &(pHexFile->pHexRecord[nHexRecordCnt]));
        if (RET_OK == nRet) { nHexRecordCnt++; }
        else { break; }
        while (':' != pText[0] && '\0' != pText[0]) { pText = &pText[1]; }
    }
    if (RET_OK == nRet)
    {
        pHexFile->nHexRecordCnt = nHexRecordCnt;
    }

    return nRet;
}
static HANERROR HANGetHexRecordLine(char* pText, char** pEnd, PHEXRECORD pRecord)
{
    HANERROR nRet = RET_OK;
    uint8_t pBuf[300];
    
    if (':' != pText[0]) { nRet = HEX_FILE_FORMAT_ERR; }
    if (RET_OK == nRet)
    {
        nRet = HANTextToHexValue(&pText[1], &pBuf[0]);
    }
    if (RET_OK == nRet)
    {
        for (uint32_t i = 1; i < (uint32_t)pBuf[0] + (uint32_t)5; i++)
        {
            nRet = HANTextToHexValue(&pText[(uint32_t)1 + (i * (uint32_t)2)], &pBuf[i]);
            if (RET_OK != nRet) { break; }
        }
    }
    if (RET_OK == nRet)
    {
        pRecord->nDataLen = pBuf[0];
        pRecord->cAddr = ((uint16_t)pBuf[1]) << 8 | pBuf[2];
        (void)memcpy((void*)(&(pRecord->cDataType)), (void*)(&pBuf[3]), 1);
        (void)memcpy((void*)(pRecord->pData), (void*)(&pBuf[4]), pBuf[0]);
        (void)memcpy((void*)(&(pRecord->cCheckValue)), (void*)(&pBuf[(uint32_t)4 + (uint32_t)pBuf[0]]), 1);
        if (HANHexRecordCheck(pBuf, pBuf[0] + 4) == pRecord->cCheckValue) { pRecord->bCheckOK = TRUE; }
        else { pRecord->bCheckOK = FALSE; }
        *pEnd = &pText[(uint32_t)1 + ((uint32_t)pBuf[0] + (uint32_t)5) * (uint32_t)2];
    }

    return nRet;
}
static uint8_t HANHexRecordCheck(uint8_t* pRecord, uint32_t nLen)
{
    uint8_t nSum = 0;
    for (uint32_t i = 0; i < nLen; i++)
    {
        nSum += pRecord[i];
    }
    return (0x100 - nSum);
}
static HANERROR HANTextToHexValue(const char* pText, uint8_t* pHex)
{
    HANERROR nRet = RET_OK;
    char pBuf[3] = { pText[0], pText[1], '\0' };
    char* pEnd = pBuf;
    
    *pHex = (uint8_t)strtol(pBuf, &pEnd, 16);
    if (pEnd != &pBuf[2]) { nRet = HEX_FILE_FORMAT_ERR; }

    return nRet;
}

static LRESULT CALLBACK HexViewWndProc(HWND hHexView, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    PHEXVIEWWNDEXTRA hvInfo = (PHEXVIEWWNDEXTRA)GetWindowLongPtr(hHexView, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = CreateCallback(hHexView, lParam);
        } break;
        case WM_COMMAND: {
            CommandCallback(hHexView, hvInfo, wParam, lParam);
        } break;
        case WM_CTLCOLORSTATIC: {
            lWndProcRet = (INT_PTR)GetStockObject(WHITE_BRUSH);
        } break;
        case WM_DESTROY: {
            HANWinHeapFree(hvInfo->hHeap, 0, hvInfo);
            lWndProcRet = DefWindowProc(hHexView, message, wParam, lParam);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hHexView, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}
static void PrintHexView(PHEXVIEWWNDEXTRA hvInfo)
{
    PCHEXRECORD pHexRecord;
    PHEXFILE pHexFile = &(hvInfo->htHexText.hfHexFile);
    HANPSTR pText = hvInfo->htHexText.pText;
    uint32_t nTextBufSize = hvInfo->htHexText.nTextBufSize;
    HANSIZE nOffset = 0;

    for (uint32_t i = 0; i < pHexFile->nHexRecordCnt; i++)
    {
        pHexRecord = &(pHexFile->pHexRecord[i]);
        if (TRUE == pHexRecord->bCheckOK)
        {
            HAN_snprintf(&pText[nOffset], nTextBufSize - nOffset, TEXT("√ "));
            nOffset += HAN_strlen(&pText[nOffset]);
        }
        else
        {
            HAN_snprintf(&pText[nOffset], nTextBufSize - nOffset, TEXT("× "));
            nOffset += HAN_strlen(&pText[nOffset]);
        }
        HAN_snprintf(&pText[nOffset], nTextBufSize - nOffset, TEXT("%02X "), pHexRecord->nDataLen);
        nOffset += HAN_strlen(&pText[nOffset]);
        HAN_snprintf(&pText[nOffset], nTextBufSize - nOffset, TEXT("%04X "), pHexRecord->cAddr);
        nOffset += HAN_strlen(&pText[nOffset]);
        HAN_snprintf(&pText[nOffset], nTextBufSize - nOffset, TEXT("%02X "), pHexRecord->cDataType);
        nOffset += HAN_strlen(&pText[nOffset]);
        for (uint32_t j = 0; j < pHexRecord->nDataLen; j++)
        {
            HAN_snprintf(&pText[nOffset], nTextBufSize - nOffset, TEXT("%02X"), pHexRecord->pData[j]);
            nOffset += HAN_strlen(&pText[nOffset]);
        }
        if (pHexRecord->nDataLen > 0)
        {
            HAN_snprintf(&pText[nOffset], nTextBufSize - nOffset, TEXT(" "));
            nOffset += HAN_strlen(&pText[nOffset]);
        }
        HAN_snprintf(&pText[nOffset], nTextBufSize - nOffset, TEXT("%02X\r\n"), pHexRecord->cCheckValue);
        nOffset += HAN_strlen(&pText[nOffset]);
    }
    pText[nTextBufSize - 1] = TEXT('\0');
    SetWindowText(hvInfo->hHexViewEdit, pText);
}
static LRESULT CreateCallback(HWND hWnd, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PHEXVIEWWNDEXTRA hvInfo;
    HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
    PHEXVIEWCFG pHvConfig = ((LPCREATESTRUCT)lParam)->lpCreateParams;
    RECT rcClientSize;

    HANDLE hHeap = GetProcessHeap();
    if (NULL == hHeap) { lWndProcRet = -1; }
    if (-1 != lWndProcRet)
    {
        hvInfo = (PHEXVIEWWNDEXTRA)HANWinHeapAlloc(hHeap, NULL, sizeof(HEXVIEWWNDEXTRA));
        if (NULL == hvInfo) { lWndProcRet = -1; }
    }

    if (-1 != lWndProcRet)
    {
        HANINT nWinX;
        HANINT nWinY;

        SetWindowLongPtr(hWnd, 0, (LONG_PTR)hvInfo);
        hvInfo->hHeap = hHeap;

        GetClientRect(hWnd, &rcClientSize);

        hvInfo->hHexFont = CreateFontIndirect(&g_lfHexFont);
        hvInfo->hSysFont = CreateFontIndirect(&g_lfSysFont);

        hvInfo->hOpenHexFileButton = CreateWindow(TEXT("button"), TEXT("打开Hex文件"),
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 30, 30, 130, 30,
            hWnd, (HMENU)WID_HEX_VIEW_OPEN_HEX_FILE_BUTTON, hInst, NULL);

        nWinX = 30; nWinY = 100;
        hvInfo->hStartAddrStatic = CreateWindow(TEXT("static"), TEXT("0x"),
            WS_CHILD | WS_VISIBLE | SS_RIGHT, nWinX, nWinY, 25, 25,
            hWnd, (HMENU)WID_HEX_VIEW_START_ADDR_STATIC, hInst, NULL);
        nWinX += 25;
        hvInfo->hStartAddrEdit = CreateWindow(TEXT("edit"), DEFAULT_START_ADDR_TEXT,
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, nWinX, nWinY - 1, 100, 25,
            hWnd, (HMENU)WID_HEX_VIEW_START_ADDR_EDIT, hInst, NULL);
        nWinX += 100;
        hvInfo->hEndAddrStatic = CreateWindow(TEXT("static"), TEXT(" ~ 0x"),
            WS_CHILD | WS_VISIBLE | SS_RIGHT, nWinX, nWinY, 51, 25,
            hWnd, (HMENU)WID_HEX_VIEW_END_ADDR_STATIC, hInst, NULL);
        nWinX += 51;
        hvInfo->hEndAddrEdit = CreateWindow(TEXT("edit"), DEFAULT_END_ADDR_TEXT,
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, nWinX, nWinY - 1, 100, 25,
            hWnd, (HMENU)WID_HEX_VIEW_END_ADDR_EDIT, hInst, NULL);
        nWinX = 30; nWinY += 35;
        hvInfo->hSaveBinFile = CreateWindow(TEXT("button"), TEXT("保存Bin文件"),
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, nWinX, nWinY, 120, 30,
            hWnd, (HMENU)WID_HEX_VIEW_SAVE_BIN_FILE, hInst, NULL);
        nWinX += 130;
        hvInfo->hSaveFilledHexFileButton = CreateWindow(TEXT("button"), TEXT("保存填充后的Hex文件"),
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, nWinX, nWinY, 200, 30,
            hWnd, (HMENU)WID_HEX_VIEW_SAVE_FILLED_HEX_FILE, hInst, NULL);
        nWinX = 30; nWinY += 70;
        hvInfo->hLinkFileList = CreateWindow(
            TEXT("listbox"), NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_DISABLENOSCROLL, nWinX, nWinY, 250, 180,
            hWnd, (HMENU)WID_HEX_VIEW_LINK_FILE_LIST, hInst, NULL);
        nWinX += 280; nWinY += 50;
        hvInfo->hLinkFileAdd = CreateWindow(
            TEXT("button"), TEXT("＋"),
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, nWinX, nWinY, 25, 25,
            hWnd, (HMENU)WID_HEX_VIEW_LINK_FILE_ADD, hInst, NULL);
        nWinY += 50;
        hvInfo->hLinkFileSub = CreateWindow(
            TEXT("button"), TEXT("－"),
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, nWinX, nWinY, 25, 25,
            hWnd, (HMENU)WID_HEX_VIEW_LINK_FILE_SUB, hInst, NULL);
        nWinX = 30; nWinY += 90;
        hvInfo->hLinkFileSub = CreateWindow(
            TEXT("button"), TEXT("保存拼接后的Hex文件"),
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, nWinX, nWinY, 200, 30,
            hWnd, (HMENU)WID_HEX_VIEW_LINK_FILE_LINK, hInst, NULL);
        
        nWinX = 500;
        hvInfo->hMemMapEdit = CreateWindow(
            TEXT("edit"), NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_READONLY,
            nWinX, 10, 260, GetRectH(&rcClientSize) - 50,
            hWnd, (HMENU)WID_HEX_VIEW_HEX_MEM_MAP_EDIT, hInst, NULL);
        nWinX += 270;
        hvInfo->hHexViewEdit = CreateWindow(
            TEXT("edit"), NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL | ES_LEFT | ES_MULTILINE | ES_READONLY,
            nWinX, 10, 510, GetRectH(&rcClientSize) - 50,
            hWnd, (HMENU)WID_HEX_VIEW_VIEW, hInst, NULL);

        SendMessage(hvInfo->hOpenHexFileButton, WM_SETFONT, (WPARAM)(hvInfo->hSysFont), (LPARAM)TRUE);
        SendMessage(hvInfo->hStartAddrStatic, WM_SETFONT, (WPARAM)(hvInfo->hHexFont), (LPARAM)TRUE);
        SendMessage(hvInfo->hStartAddrEdit, WM_SETFONT, (WPARAM)(hvInfo->hHexFont), (LPARAM)TRUE);
        SendMessage(hvInfo->hEndAddrStatic, WM_SETFONT, (WPARAM)(hvInfo->hHexFont), (LPARAM)TRUE);
        SendMessage(hvInfo->hEndAddrEdit, WM_SETFONT, (WPARAM)(hvInfo->hHexFont), (LPARAM)TRUE);
        SendMessage(hvInfo->hSaveBinFile, WM_SETFONT, (WPARAM)(hvInfo->hSysFont), (LPARAM)TRUE);
        SendMessage(hvInfo->hSaveFilledHexFileButton, WM_SETFONT, (WPARAM)(hvInfo->hSysFont), (LPARAM)TRUE);
        SendMessage(hvInfo->hHexViewEdit, WM_SETFONT, (WPARAM)(hvInfo->hHexFont), (LPARAM)TRUE);
        SendMessage(hvInfo->hMemMapEdit, WM_SETFONT, (WPARAM)(hvInfo->hHexFont), (LPARAM)TRUE);
        SendMessage(hvInfo->hLinkFileList, WM_SETFONT, (WPARAM)(hvInfo->hHexFont), (LPARAM)TRUE);
        SendMessage(hvInfo->hLinkFileAdd, WM_SETFONT, (WPARAM)(hvInfo->hSysFont), (LPARAM)TRUE);
        SendMessage(hvInfo->hLinkFileSub, WM_SETFONT, (WPARAM)(hvInfo->hSysFont), (LPARAM)TRUE);
        SendMessage(hvInfo->hLinkFileLink, WM_SETFONT, (WPARAM)(hvInfo->hSysFont), (LPARAM)TRUE);

        hvInfo->bHexOpen = FALSE;

        if (NULL != pHvConfig)
        {
            (void)memcpy(&(hvInfo->pHvConfig), pHvConfig, sizeof(hvInfo->pHvConfig));
            LoadHexViewCfg(hvInfo, pHvConfig);
        }
    }

    return lWndProcRet;
}
static void LoadHexViewCfg(PHEXVIEWWNDEXTRA hvInfo, PHEXVIEWCFG pHvConfig)
{
    INIHEXVIEWCFGID iLoop;

    for (iLoop = 0; iLoop < INI_HEX_VIEW_CFG_CNT; iLoop++)
    {
        sg_pHexViewCfgInfo[iLoop].CfgTextToWindow(hvInfo, pHvConfig->pSysConfig[iLoop].pValue);
    }
}
static void CommandCallback(HWND hWnd, PHEXVIEWWNDEXTRA hvInfo, WPARAM wParam, LPARAM lParam)
{
    (void)lParam;

    switch (LOWORD(wParam)) {
        case WID_HEX_VIEW_OPEN_HEX_FILE_BUTTON: {
            OpenFileAction(hvInfo);
            break;
        }
        case WID_HEX_VIEW_SAVE_BIN_FILE: {
            if (TRUE == hvInfo->bHexOpen) { SaveBinFileAction(hvInfo); }
            else { MessageBox(hWnd, TEXT("请先打开一个Hex文件"), TEXT("错误"), 0); }
            break;
        }
        case WID_HEX_VIEW_SAVE_FILLED_HEX_FILE: {
            if (TRUE == hvInfo->bHexOpen) { SaveFilledHexFileAction(hvInfo); }
            else { MessageBox(hWnd, TEXT("请先打开一个Hex文件"), TEXT("错误"), 0); }
            break;
        }
        case WID_HEX_VIEW_LINK_FILE_ADD: {
            AddLinkFileAction(hvInfo);
            break;
        }
        case WID_HEX_VIEW_LINK_FILE_SUB: {
            SubLinkFileAction(hvInfo);
            break;
        }
        case WID_HEX_VIEW_LINK_FILE_LINK: {
            LinkLinkFileAction(hvInfo);
            break;
        }

        default: {
            break;
        }
    }
}
static void OpenFileAction(PHEXVIEWWNDEXTRA hvInfo)
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
        HANOpenHexFile(&(hvInfo->htHexText.hfHexFile), pInputName, hvInfo->hHeap);
        uint32_t nTextBufSize = sizeof(HANCHAR) * (hvInfo->htHexText.hfHexFile.nTextLen * 2);
        HANPSTR pText = HANWinHeapAlloc(hvInfo->hHeap, hvInfo->htHexText.pText, nTextBufSize);
        if (NULL != pText)
        {
            hvInfo->htHexText.pText = pText;
            hvInfo->htHexText.nTextBufSize = nTextBufSize;
            SetWindowText(hvInfo->hMemMapEdit, TEXT(""));
            PrintHexView(hvInfo);
            PrintHexMemMap(hvInfo);
            hvInfo->bHexOpen = TRUE;
        }
    }
}
static void SaveBinFileAction(PHEXVIEWWNDEXTRA hvInfo)
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
        HANDLE hFile;
        HANCHAR pText[2][16];
        ADDRMAP amAddrMap;
    
        GetWindowText(hvInfo->hStartAddrEdit, pText[0], ArrLen(pText[0]));
        GetWindowText(hvInfo->hEndAddrEdit, pText[1], ArrLen(pText[1]));
        amAddrMap.cStartAddr = HAN_strtoul(pText[0], NULL, 16);
        amAddrMap.cEndAddr = HAN_strtoul(pText[1], NULL, 16);
        HexViewHexToBin(&(hvInfo->htHexText.hfHexFile), &amAddrMap, hvInfo->hHeap);

        hFile = CreateFile(pInputName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
        if (INVALID_HANDLE_VALUE != hFile)
        {
            PHEXFILE pHexFile = &(hvInfo->htHexText.hfHexFile);
            DWORD nLen = pHexFile->amBinAddrMap.cEndAddr - pHexFile->amBinAddrMap.cStartAddr + (uint32_t)1;
            WriteFile(hFile, pHexFile->pBinData, nLen, NULL, NULL);
            CloseHandle(hFile);
        }
    }
}
static void AddLinkFileAction(PHEXVIEWWNDEXTRA hvInfo)
{
    HANCHAR pText[PATH_STR_SIZE] = { 0 };
    OPENFILENAME ofnOpenFile;
    ZeroMemory(&ofnOpenFile, sizeof(ofnOpenFile));
    ofnOpenFile.lStructSize = sizeof(ofnOpenFile);
    ofnOpenFile.hwndOwner = NULL;
    ofnOpenFile.lpstrFilter = TEXT("所有文件\0*.*\0\0");
    ofnOpenFile.nFilterIndex = 0;
    ofnOpenFile.lpstrFile = pText;
    ofnOpenFile.nMaxFile = PATH_STR_SIZE;
    ofnOpenFile.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
    if (GetOpenFileName(&ofnOpenFile))
    {
        ListBoxAddString(hvInfo->hLinkFileList, pText);
    }
    else
    {
        printf("%lu\n", GetLastError());
    }
}
static void SubLinkFileAction(PHEXVIEWWNDEXTRA hvInfo)
{
    HANINT nTar = ListBoxGetCursel(hvInfo->hLinkFileList);
    SendMessage(hvInfo->hLinkFileList, LB_DELETESTRING, (WPARAM)nTar, 0);
}
static void LinkLinkFileAction(PHEXVIEWWNDEXTRA hvInfo)
{
    HANPSTR pFileName;
    HANCHAR pDestName[PATH_STR_SIZE];
    HANINT nForMax = LINK_FILE_CNT_MAX;
    HANINT nListStrCnt = (HANINT)SendMessage(hvInfo->hLinkFileList, LB_GETCOUNT, 0, 0);
    OPENFILENAME ofnOpenFile;

    pFileName = (HANPSTR)HANWinHeapAlloc(hvInfo->hHeap, NULL, LINK_FILE_CNT_MAX * PATH_STR_SIZE);
    if (NULL != pFileName)
    {
        if (nListStrCnt < nForMax) { nForMax = nListStrCnt; }
        for (HANINT i = 0; i < nForMax; i++)
        {
            SendMessage(hvInfo->hLinkFileList, LB_GETTEXT, (WPARAM)i, (LPARAM)(&pFileName[i * PATH_STR_SIZE]));
        }

        ZeroMemory(&ofnOpenFile, sizeof(ofnOpenFile));
        ofnOpenFile.lStructSize = sizeof(ofnOpenFile);
        ofnOpenFile.hwndOwner = NULL;
        ofnOpenFile.lpstrFilter = TEXT("所有文件\0*.*\0\0");
        ofnOpenFile.nFilterIndex = 0;
        ofnOpenFile.lpstrFile = pDestName;
        ofnOpenFile.nMaxFile = PATH_STR_SIZE;
        ofnOpenFile.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
        if (GetSaveFileName(&ofnOpenFile))
        {
            LinkHexFile(pDestName, pFileName, nForMax);
        }
    }
    
    if (NULL != pFileName) { HANWinHeapFree(hvInfo->hHeap, 0, pFileName); }
}
static void PrintHexMemMap(PHEXVIEWWNDEXTRA hvInfo)
{
    uint32_t cLastAddr = 0;
    uint32_t cLinearAddr = 0;
    uint32_t cRecordAddr = 0;
    ADDRMAP pAddrMap = { .cStartAddr = 0x00000000, .cEndAddr = 0x00000000 };
    PHEXFILE pHexFile = &(hvInfo->htHexText.hfHexFile);
    BOOL bStart = TRUE;
    BOOL bNewMap = FALSE;

    for (uint32_t i = 0; i < pHexFile->nHexRecordCnt; i++)
    {
        PHEXRECORD pRecord = &(pHexFile->pHexRecord[i]);
        switch (pRecord->cDataType) {
            case HEX_RECORD_TYPE_EXTENDED_LINEAR_ADDRESS: {
                cLinearAddr = (((uint32_t)(pRecord->pData[0]) << 8) | ((uint32_t)(pRecord->pData[1]))) << (uint32_t)16;
                break;
            }
            case HEX_RECORD_TYPE_DATA: {
                cRecordAddr = cLinearAddr | pRecord->cAddr;
                if (cLastAddr < cRecordAddr)
                {
                    bNewMap = TRUE;
                    if (TRUE == bStart)
                    {
                        bStart = FALSE;
                        pAddrMap.cStartAddr = cRecordAddr;
                    }
                    else
                    {
                        pAddrMap.cEndAddr = cLastAddr - (uint32_t)1;
                        AppendHexMap(hvInfo, &pAddrMap);
                        pAddrMap.cStartAddr = cRecordAddr;
                        bNewMap = FALSE;
                    }
                }
                cLastAddr = cRecordAddr + pRecord->nDataLen;
                break;
            }
        }
    }
    if (TRUE == bNewMap)
    {
        pAddrMap.cEndAddr = cLastAddr - (uint32_t)1;
        AppendHexMap(hvInfo, &pAddrMap);
    }
}
static void AppendHexMap(PHEXVIEWWNDEXTRA hvInfo, PADDRMAP pAddrMap)
{
    HANCHAR pText[64];

    HAN_snprintf(pText, ArrLen(pText), TEXT("0x%08X ~ 0x%08X\r\n"), pAddrMap->cStartAddr, pAddrMap->cEndAddr);
    EditAppendText(hvInfo->hMemMapEdit, pText, FALSE);
}
static HANERROR SaveFilledHexFileAction(PHEXVIEWWNDEXTRA hvInfo)
{
    HANERROR nRet = RET_OK;
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
        HANDLE hFile;
        HANCHAR pText[2][16];
        ADDRMAP amAddrMap;
    
        GetWindowText(hvInfo->hStartAddrEdit, pText[0], ArrLen(pText[0]));
        GetWindowText(hvInfo->hEndAddrEdit, pText[1], ArrLen(pText[1]));
        amAddrMap.cStartAddr = HAN_strtoul(pText[0], NULL, 16);
        amAddrMap.cEndAddr = HAN_strtoul(pText[1], NULL, 16);
        HexViewHexToBin(&(hvInfo->htHexText.hfHexFile), &amAddrMap, hvInfo->hHeap);

        if (RET_OK == nRet)
        {
            hFile = CreateFile(pInputName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
            if (INVALID_HANDLE_VALUE != hFile)
            {
                PHEXFILE pHexFile = &(hvInfo->htHexText.hfHexFile);
                uint32_t nBinDataLen = pHexFile->amBinAddrMap.cEndAddr - pHexFile->amBinAddrMap.cStartAddr + (uint32_t)1;
                BinDataToHexFile(hFile, pHexFile->amBinAddrMap.cStartAddr, pHexFile->pBinData, nBinDataLen);
                CloseHandle(hFile);
            }
            else
            {
                nRet = OPEN_FILE_ERR;
            }
        }
    }

    return nRet;
}
static DWORD PrintHexRecord(char* pText, PHEXRECORD pRecord)
{
    uint8_t pBuf[32];
    uint32_t iFor;
    DWORD nRet = 0;

    pBuf[0] = pRecord->nDataLen;
    pBuf[1] = (pRecord->cAddr >> (uint16_t)8) & (uint16_t)0x00FF;
    pBuf[2] = pRecord->cAddr & (uint16_t)0x00FF;
    pBuf[3] = pRecord->cDataType;
    memcpy(&pBuf[4], pRecord->pData, pRecord->nDataLen);
    pBuf[4 + pRecord->nDataLen] = HANHexRecordCheck(pBuf, 4 + pRecord->nDataLen);
    nRet += sprintf(pText, ":");
    for (iFor = 0; iFor < ((uint32_t)5 + (uint32_t)(pRecord->nDataLen)); iFor++)
    {
        nRet += sprintf(&pText[(uint32_t)1 + iFor * (uint32_t)2], "%02X", pBuf[iFor]);
    }
    nRet += sprintf(&pText[(uint32_t)1 + iFor * (uint32_t)2], "\r\n");

    return nRet;
}
static DWORD PrintHexRecordLinearAddr(char* pText, uint16_t cAddr)
{
    HEXRECORD hrHexRecord;
    
    hrHexRecord.nDataLen = 0x02;
    hrHexRecord.cAddr = 0x00;
    hrHexRecord.cDataType = HEX_RECORD_TYPE_EXTENDED_LINEAR_ADDRESS;
    hrHexRecord.pData[0] = (cAddr >> (uint16_t)8) & (uint16_t)0x00FF;
    hrHexRecord.pData[1] = cAddr & (uint16_t)0x00FF;
    return PrintHexRecord(pText, &hrHexRecord);
}

static void CfgWindowToTextStartAddr(PHEXVIEWWNDEXTRA hvInfo, HANPSTR pText)
{
    HANCHAR pTempText[INI_HEX_VIEW_VALUE_SIZE];
    HANSIZE cAddr;

    GetWindowText(hvInfo->hStartAddrEdit, pTempText, INI_HEX_VIEW_VALUE_SIZE);
    cAddr = HAN_strtoul(pTempText, NULL, 16);
    HAN_snprintf(pText, INI_HEX_VIEW_VALUE_SIZE, TEXT(ADDR_PRINT_FORMAT_SINGLE), cAddr);
}
static void CfgWindowToTextEndAddr(PHEXVIEWWNDEXTRA hvInfo, HANPSTR pText)
{
    HANCHAR pTempText[INI_HEX_VIEW_VALUE_SIZE];
    HANSIZE cAddr;

    GetWindowText(hvInfo->hEndAddrEdit, pTempText, INI_HEX_VIEW_VALUE_SIZE);
    cAddr = HAN_strtoul(pTempText, NULL, 16);
    HAN_snprintf(pText, INI_HEX_VIEW_VALUE_SIZE, TEXT(ADDR_PRINT_FORMAT_SINGLE), cAddr);
}

static void CfgTextToWindowStartAddr(PHEXVIEWWNDEXTRA hvInfo, HANPCSTR pText)
{
    HANSIZE cAddr = HAN_strtoul(pText, NULL, 16);
    HANCHAR pTempText[ADDR_STR_SIZE];

    HAN_snprintf(pTempText, ADDR_STR_SIZE, TEXT(ADDR_PRINT_FORMAT_FULL), (uint32_t)cAddr);
    pTempText[ADDR_STR_SIZE - 1] = TEXT('\0');
    SetWindowText(hvInfo->hStartAddrEdit, pTempText);
}
static void CfgTextToWindowEndAddr(PHEXVIEWWNDEXTRA hvInfo, HANPCSTR pText)
{
    HANSIZE cAddr = HAN_strtoul(pText, NULL, 16);
    HANCHAR pTempText[ADDR_STR_SIZE];

    HAN_snprintf(pTempText, ADDR_STR_SIZE, TEXT(ADDR_PRINT_FORMAT_FULL), (uint32_t)cAddr);
    pTempText[ADDR_STR_SIZE - 1] = TEXT('\0');
    SetWindowText(hvInfo->hEndAddrEdit, pTempText);
}
