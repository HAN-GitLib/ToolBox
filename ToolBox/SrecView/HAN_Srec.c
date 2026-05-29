#include <stdio.h>

#include "..\GlobalVariables.h"
#include "HAN_Srec.h"

#define INI_SREC_VIEW_VALUE_SIZE        INI_MAIN_VALUE_STR_SIZE
#define INI_SREC_VIEW_APP_NAME          TEXT("SvConfig")

typedef enum {
    WID_SREC_VIEW_BASE = HAN_WID_BASE,
    WID_SREC_VIEW_VIEW,
    WID_SREC_VIEW_SREC_MEM_MAP_EDIT,
    WID_SREC_VIEW_OPEN_SREC_FILE_BUTTON,
    WID_SREC_VIEW_START_ADDR_STATIC,
    WID_SREC_VIEW_START_ADDR_EDIT,
    WID_SREC_VIEW_END_ADDR_STATIC,
    WID_SREC_VIEW_END_ADDR_EDIT,
    WID_SREC_VIEW_SAVE_BIN_FILE,
    WID_SREC_VIEW_SAVE_FILLED_SREC_FILE,
    WID_SREC_VIEW_LINK_FILE_LIST,
    WID_SREC_VIEW_LINK_FILE_ADD,
    WID_SREC_VIEW_LINK_FILE_SUB,
    WID_SREC_VIEW_LINK_FILE_LINK,
} WIDSRECVIEW;

typedef enum {
    SREC_ADDR_BIT_LEN_16,
    SREC_ADDR_BIT_LEN_24,
    SREC_ADDR_BIT_LEN_32,
} SRECADDRBITLEN;

typedef struct tagSRECVIEWTEXT {
    HANPSTR     pText;
    uint32_t    nTextBufSize;
    uint32_t    nTextLen;
    SRECFILE    sfSrecFile;
} SRECVIEWTEXT, * PSRECVIEWTEXT;

typedef struct tagSRECVIEWWNDEXTRA {
    HANDLE          hHeap;
    SRECVIEWTEXT    stSrecText;
    HWND            hOpenSrecFileButton;
    HWND            hStartAddrStatic;
    HWND            hStartAddrEdit;
    HWND            hEndAddrStatic;
    HWND            hEndAddrEdit;
    HWND            hSaveBinFile;
    HWND            hSaveFilledSrecFileButton;
    HWND            hLinkFileList;
    HWND            hLinkFileAdd;
    HWND            hLinkFileSub;
    HWND            hLinkFileLink;
    HWND            hSrecViewEdit;
    HWND            hMemMapEdit;
    HFONT           hHexFont;
    HFONT           hSysFont;
    BOOL            bSrecOpen;
    SRECVIEWCFG     pSvConfig;
} SRECVIEWWNDEXTRA, * PSRECVIEWWNDEXTRA;

typedef struct tagSRECVIEWREADWRITECFG {
    HANPCSTR        pKey;
    HANPCSTR        pDefValue;
    void            (*CfgWindowToText)(PSRECVIEWWNDEXTRA bvInfo, HANPSTR pText);
    void            (*CfgTextToWindow)(PSRECVIEWWNDEXTRA bvInfo, HANPCSTR pText);
} SRECVIEWREADWRITECFG;

static HANERROR HANSrecFileTextToSrec(PSRECFILE pSrecFile);
static HANERROR HANAllocSrecBinBuf(PSRECFILE pSrecFile, DWORD nFileSize, HANDLE hHeap);
static HANERROR HANGetSrecRecordLine(char* pText, char** pEnd, PSRECRECORD pRecord);
static uint8_t HANSrecRecordCheck(uint8_t* pRecord, uint32_t nLen);
static HANERROR HANTextToHexValue(const char* pText, uint8_t* pHex);

static LRESULT CALLBACK SrecViewWndProc(HWND hSrecView, UINT message, WPARAM wParam, LPARAM lParam);
static SRECADDRBITLEN GetSrecAddrBitLen(uint8_t cRecordType);
static void PrintSrecView(PSRECVIEWWNDEXTRA svInfo);
static LRESULT CreateCallback(HWND hWnd, LPARAM lParam);
static void LoadSrecViewCfg(PSRECVIEWWNDEXTRA svInfo, PSRECVIEWCFG pSvConfig);
static void CommandCallback(HWND hWnd, PSRECVIEWWNDEXTRA svInfo, WPARAM wParam, LPARAM lParam);
static void OpenFileAction(PSRECVIEWWNDEXTRA svInfo);
static void SaveBinFileAction(PSRECVIEWWNDEXTRA svInfo);
static void AddLinkFileAction(PSRECVIEWWNDEXTRA svInfo);
static void SubLinkFileAction(PSRECVIEWWNDEXTRA svInfo);
static void LinkLinkFileAction(PSRECVIEWWNDEXTRA svInfo);
static void PrintSrecMemMap(PSRECVIEWWNDEXTRA svInfo);
static void AppendSrecMap(PSRECVIEWWNDEXTRA svInfo, PADDRMAP pAddrMap);
static HANERROR SaveFilledSrecFileAction(PSRECVIEWWNDEXTRA svInfo);

static void CfgWindowToTextStartAddr(PSRECVIEWWNDEXTRA svInfo, HANPSTR pText);
static void CfgWindowToTextEndAddr(PSRECVIEWWNDEXTRA svInfo, HANPSTR pText);

static void CfgTextToWindowStartAddr(PSRECVIEWWNDEXTRA svInfo, HANPCSTR pText);
static void CfgTextToWindowEndAddr(PSRECVIEWWNDEXTRA svInfo, HANPCSTR pText);

static const SRECVIEWREADWRITECFG sg_pSrecViewCfgInfo[INI_SREC_VIEW_CFG_CNT] = {
    [INI_SREC_VIEW_START_ADDR] = {
        .pKey = TEXT("StartAddr"),
        .pDefValue = DEFAULT_START_ADDR_TEXT,
        .CfgWindowToText = CfgWindowToTextStartAddr,
        .CfgTextToWindow = CfgTextToWindowStartAddr,
    },
    [INI_SREC_VIEW_END_ADDR] = {
        .pKey = TEXT("EndAddr"),
        .pDefValue = DEFAULT_END_ADDR_TEXT,
        .CfgWindowToText = CfgWindowToTextEndAddr,
        .CfgTextToWindow = CfgTextToWindowEndAddr,
    },
};

void RegisterHANSrecView(HINSTANCE hInst)
{
    WNDCLASSEX wcex = {
        .cbSize         = sizeof(WNDCLASSEX),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = SrecViewWndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = sizeof(PSRECVIEWWNDEXTRA),
        .hInstance      = hInst,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName   = NULL,
        .lpszClassName  = HAN_SREC_VIEW_CLASS,
        .hIconSm        = NULL,
    };
    RegisterClassEx(&wcex);
}

void ReadSrecViewIniFile(HANPCSTR pIniPath, void* pParam)
{
    PSRECVIEWCFG pSvConfig = pParam;
    INISRECVIEWCFGID iLoop;

    for (iLoop = 0; iLoop < INI_SREC_VIEW_CFG_CNT; iLoop++)
    {
        HAN_strcpy(pSvConfig->pSysConfig[iLoop].pKey, sg_pSrecViewCfgInfo[iLoop].pKey);
        HAN_strcpy(pSvConfig->pSysConfig[iLoop].pDefValue, sg_pSrecViewCfgInfo[iLoop].pDefValue);
        GetPrivateProfileString(
            INI_SREC_VIEW_APP_NAME, pSvConfig->pSysConfig[iLoop].pKey, pSvConfig->pSysConfig[iLoop].pDefValue,
            pSvConfig->pSysConfig[iLoop].pValue, INI_SREC_VIEW_VALUE_SIZE, pIniPath);
    }
}

void WriteSrecViewIniFile(HANPCSTR pIniPath, HWND hSrecView)
{
    PSRECVIEWWNDEXTRA hvInfo = (PSRECVIEWWNDEXTRA)GetWindowLongPtr(hSrecView, 0);
    HANCHAR pBvCfg[INI_SREC_VIEW_CFG_CNT][INI_SREC_VIEW_VALUE_SIZE];
    INISRECVIEWCFGID iLoop;
    
    for (iLoop = 0; iLoop < INI_SREC_VIEW_CFG_CNT; iLoop++)
    {
        sg_pSrecViewCfgInfo[iLoop].CfgWindowToText(hvInfo, pBvCfg[iLoop]);
        WritePrivateProfileString(
            INI_SREC_VIEW_APP_NAME, sg_pSrecViewCfgInfo[iLoop].pKey, pBvCfg[iLoop], pIniPath
        );
    }
}

void BinDataToSrecFile(HANDLE hFile, SRECADDRSIZE sasAddrSize, uint32_t cStartAddr, uint8_t* pBinData, uint32_t nLen)
{
    uint8_t pBuf[32];
    char pText[64];
    DWORD nWriteLen;
    uint32_t nRecordLen;
    uint32_t nOffset = 0;

    while (nOffset < nLen)
    {
        if (HAN_SREC_DATA_LEN_MAX < (nLen - nOffset)) { nRecordLen = HAN_SREC_DATA_LEN_MAX; }
        else { nRecordLen = nLen - nOffset; }
        nWriteLen = 0;
        pBuf[0] = nRecordLen + sasAddrSize + 1;
        for (SRECADDRSIZE i = 1; i <= sasAddrSize; i++)
        {
            pBuf[i] = (cStartAddr + nOffset) >> ((sasAddrSize - i) * 8);
        }
        for (uint32_t i = 1 + sasAddrSize; i < (pBuf[0]); i++)
        {
            pBuf[i] = pBinData[nOffset + i - 1 - sasAddrSize];
        }
        pBuf[pBuf[0]] = HANSrecRecordCheck(pBuf, pBuf[0]);
        nWriteLen += sprintf(pText + nWriteLen, "S%d", sasAddrSize - 1);
        for (uint32_t i = 0; i < ((uint32_t)(pBuf[0]) + (uint32_t)1); i++)
        {
            nWriteLen += sprintf(pText + nWriteLen, "%02X", pBuf[i]);
        }
        nWriteLen += sprintf(pText + nWriteLen, "\n");
        WriteFile(hFile, pText, nWriteLen, NULL, NULL);
        nOffset += nRecordLen;
    }
}

void LinkSrecFile(HANPCSTR pFileDest, HANPSTR pFileSrc, uint32_t nLen)
{
    HANDLE hDest;
    HANDLE hSrc;
    uint8_t* pStr = NULL;
    uint8_t* pStrTemp;

    hDest = CreateFile(pFileDest, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (INVALID_HANDLE_VALUE != hDest)
    {
        for (uint32_t i = 0; i < nLen; i++)
        {
            hSrc = CreateFile(&pFileSrc[i * PATH_STR_SIZE], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
            if (INVALID_HANDLE_VALUE != hSrc)
            {
                DWORD nFileSize = GetFileSize(hSrc, NULL);
                pStrTemp = HANWinHeapAlloc(GetProcessHeap(), pStr, nFileSize);
                if (pStrTemp == NULL) { break; }
                else
                {
                    pStr = pStrTemp;
                    (void)ReadFile(hSrc, pStr, nFileSize, NULL, NULL);
                    WriteFile(hDest, pStr, nFileSize, NULL, NULL);
                }
                CloseHandle(hSrc);
            }
        }
    }
    CloseHandle(hDest);
}

HANERROR HANOpenSrecFile(PSRECFILE pSrecFile, HANPCSTR pFileName, HANDLE hHeap)
{
    HANERROR nRet = RET_OK;
    HANDLE hFile = CreateFile(pFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    DWORD nFileSize;
    
    if (INVALID_HANDLE_VALUE == hFile) { nRet = OPEN_FILE_ERR; }

    if (RET_OK == nRet)
    {
        nFileSize = GetFileSize(hFile, NULL);
        nRet = HANAllocSrecBinBuf(pSrecFile, nFileSize, hHeap);
    }
    if (RET_OK == nRet)
    {
        DWORD nReadSize;
        (void)ReadFile(hFile, pSrecFile->pText, nFileSize, &nReadSize, NULL);
        pSrecFile->pText[nFileSize] = '\0';
        pSrecFile->nTextLen = (uint32_t)nFileSize;
        nRet = HANSrecFileTextToSrec(pSrecFile);
    }

    if (INVALID_HANDLE_VALUE != hFile) { CloseHandle(hFile); }

    return nRet;
}
HANERROR SrecViewSrecToBin(PSRECFILE pSrecFile, PADDRMAP pAddrMap, HANDLE hHeap)
{
    HANERROR     nRet = RET_OK;
    uint32_t        nBinSize;
    uint8_t*        pBinData;
    ADDRMAP         amRecordMap = { .cStartAddr = 0, .cEndAddr = 0, };
    ADDRMAP         amCopyMap;
    
    nBinSize = (pAddrMap->cEndAddr - pAddrMap->cStartAddr) + (uint32_t)1;
    pBinData = (uint8_t*)HANWinHeapAlloc(hHeap, pSrecFile->pBinData, nBinSize);

    if (NULL == pBinData) { nRet = NOT_ENOUGH_MEMORY; }
    if (RET_OK == nRet)
    {
        memset(pBinData, 0xFF, nBinSize);
        pSrecFile->pBinData = pBinData;
        pSrecFile->amBinAddrMap = *pAddrMap;
        for (uint32_t i = 0; i < pSrecFile->nSrecRecordCnt; i++)
        {
            PSRECRECORD pRecord = &(pSrecFile->pSrecRecord[i]);
            if ((1 <= pRecord->cDataType) && (pRecord->cDataType <= 3))
            {
                amRecordMap.cStartAddr = pRecord->cAddr;
                amRecordMap.cEndAddr = amRecordMap.cStartAddr + pRecord->nDataLen - GetSrecAddrBitLen(pRecord->cDataType) - (uint32_t)4;
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
            }
        }
    }

    return nRet;
}

static HANERROR HANAllocSrecBinBuf(PSRECFILE pSrecFile, DWORD nFileSize, HANDLE hHeap)
{
    HANERROR nRet = RET_OK;
    uint32_t nByteMaxLen = ((nFileSize - (nFileSize % (DWORD)2)) + (DWORD)2) / (DWORD)2;
    uint32_t nSrecRecordCnt = ((nByteMaxLen - (nByteMaxLen % (uint32_t)5)) + (uint32_t)5) / (uint32_t)5;

    pSrecFile->pText = (char*)HANWinHeapAlloc(hHeap, pSrecFile->pText, sizeof(char) * (nFileSize + (DWORD)1));
    pSrecFile->pSrecRecord = (SRECRECORD*)HANWinHeapAlloc(hHeap, pSrecFile->pSrecRecord, sizeof(SRECRECORD) * nSrecRecordCnt);
    if ((NULL == pSrecFile->pText) || (NULL == pSrecFile->pSrecRecord))
    {
        HANWinHeapFree(hHeap, 0, pSrecFile->pText);
        HANWinHeapFree(hHeap, 0, pSrecFile->pSrecRecord);
        pSrecFile->pText = NULL;
        pSrecFile->pSrecRecord = NULL;
        nRet = NOT_ENOUGH_MEMORY;
    }
    pSrecFile->nTextLen = 0;
    pSrecFile->nSrecRecordCnt = 0;

    return nRet;
}
static HANERROR HANSrecFileTextToSrec(PSRECFILE pSrecFile)
{
    HANERROR nRet = RET_OK;
    uint32_t nSrecRecordCnt = 0;
    char* pText = pSrecFile->pText;

    while('\0' != pText[0])
    {
        nRet = HANGetSrecRecordLine(pText, &pText, &(pSrecFile->pSrecRecord[nSrecRecordCnt]));
        if (RET_OK == nRet) { nSrecRecordCnt++; }
        else { break; }
        while ('S' != pText[0] && 's' != pText[0] && '\0' != pText[0]) { pText = &pText[1]; }
    }
    if (RET_OK == nRet)
    {
        pSrecFile->nSrecRecordCnt = nSrecRecordCnt;
    }

    return nRet;
}
static HANERROR HANGetSrecRecordLine(char* pText, char** pEnd, PSRECRECORD pRecord)
{
    HANERROR nRet = RET_OK;
    uint8_t pBuf[300];
    
    if ('S' != pText[0] && 's' != pText[0]) { nRet = SREC_FILE_FORMAT_ERR; }
    if (RET_OK == nRet)
    {
        pBuf[0] = pText[1] - '0';
        if ((uint8_t)9 < pBuf[0]) { nRet = SREC_FILE_FORMAT_ERR; }
    }
    if (RET_OK == nRet)
    {
        nRet = HANTextToHexValue(&pText[2], &pBuf[1]);
    }
    if (RET_OK == nRet)
    {
        for (uint32_t i = 2; i < (uint32_t)pBuf[1] + 2; i++)
        {
            nRet = HANTextToHexValue(&pText[i * (uint32_t)2], &pBuf[i]);
            if (RET_OK != nRet) { break; }
        }
    }
    if (RET_OK == nRet)
    {
        SRECADDRBITLEN cAddrBitLen = GetSrecAddrBitLen(pBuf[0]);
        uint8_t nAddrLen = 2;

        pRecord->nDataLen = pBuf[1];
        switch (cAddrBitLen) {
            case SREC_ADDR_BIT_LEN_16: {
                pRecord->cAddr = ((uint32_t)pBuf[2]) << 8 | pBuf[3];
                nAddrLen = 2;
                break;
            }
            case SREC_ADDR_BIT_LEN_24: {
                pRecord->cAddr = ((uint32_t)pBuf[2]) << 16 | ((uint32_t)pBuf[3]) << 8 | pBuf[4];
                nAddrLen = 3;
                break;
            }
            case SREC_ADDR_BIT_LEN_32: {
                pRecord->cAddr = ((uint32_t)pBuf[2]) << 24 | ((uint32_t)pBuf[3]) << 16 | ((uint32_t)pBuf[4]) << 8 | pBuf[5];
                nAddrLen = 4;
                break;
            }
        }
        (void)memcpy((void*)(&(pRecord->cDataType)), (void*)(&pBuf[0]), 1);
        (void)memcpy((void*)(pRecord->pData), (void*)(&pBuf[2 + nAddrLen]), pBuf[1] - (uint8_t)1 - nAddrLen);
        (void)memcpy((void*)(&(pRecord->cCheckValue)), (void*)(&pBuf[(uint32_t)1 + (uint32_t)pBuf[1]]), 1);
        if (HANSrecRecordCheck(&pBuf[1], pBuf[1]) == pRecord->cCheckValue) { pRecord->bCheckOK = TRUE; }
        else { pRecord->bCheckOK = FALSE; }
        *pEnd = &pText[((uint32_t)pBuf[1] + (uint32_t)2) * (uint32_t)2];
    }

    return nRet;
}
static uint8_t HANSrecRecordCheck(uint8_t* pRecord, uint32_t nLen)
{
    uint8_t nSum = 0;
    for (uint32_t i = 0; i < nLen; i++)
    {
        nSum += pRecord[i];
    }
    return (0xFF - nSum);
}
static HANERROR HANTextToHexValue(const char* pText, uint8_t* pHex)
{
    HANERROR nRet = RET_OK;
    char pBuf[3] = { pText[0], pText[1], '\0' };
    char* pEnd = pBuf;
    
    *pHex = (uint8_t)strtol(pBuf, &pEnd, 16);
    if (pEnd != &pBuf[2]) { nRet = SREC_FILE_FORMAT_ERR; }

    return nRet;
}

static LRESULT CALLBACK SrecViewWndProc(HWND hSrecView, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    PSRECVIEWWNDEXTRA svInfo = (PSRECVIEWWNDEXTRA)GetWindowLongPtr(hSrecView, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = CreateCallback(hSrecView, lParam);
        } break;
        case WM_CTLCOLORSTATIC: {
            lWndProcRet = (INT_PTR)GetStockObject(WHITE_BRUSH);
        } break;
        case WM_COMMAND: {
            CommandCallback(hSrecView, svInfo, wParam, lParam);
        } break;
        case WM_DESTROY: {
            HANWinHeapFree(svInfo->hHeap, 0, svInfo);
            lWndProcRet = DefWindowProc(hSrecView, message, wParam, lParam);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hSrecView, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}
static SRECADDRBITLEN GetSrecAddrBitLen(uint8_t cRecordType)
{
    SRECADDRBITLEN eRet = SREC_ADDR_BIT_LEN_16;
    if (2 == cRecordType || 6 == cRecordType || 8 == cRecordType) { eRet = SREC_ADDR_BIT_LEN_24; }
    if (3 == cRecordType || 7 == cRecordType) { eRet = SREC_ADDR_BIT_LEN_32; }
    return eRet;
}
static void PrintSrecView(PSRECVIEWWNDEXTRA svInfo)
{
    HANPCSTR pAddrFormat[3] = {
        [SREC_ADDR_BIT_LEN_16] = TEXT("%04X "),
        [SREC_ADDR_BIT_LEN_24] = TEXT("%06X "),
        [SREC_ADDR_BIT_LEN_32] = TEXT("%08X "),
    };
    PCSRECRECORD pSrecRecord = svInfo->stSrecText.sfSrecFile.pSrecRecord;
    PSRECFILE pSrecFile = &(svInfo->stSrecText.sfSrecFile);
    HANPSTR pText = svInfo->stSrecText.pText;
    uint32_t nTextBufSize = svInfo->stSrecText.nTextBufSize;
    SRECADDRBITLEN cAddrBitLen;
    HANSIZE nOffset = 0;

    for (uint32_t i = 0; i < pSrecFile->nSrecRecordCnt; i++)
    {
        cAddrBitLen = GetSrecAddrBitLen(pSrecRecord->cDataType);
        pSrecRecord = &(pSrecFile->pSrecRecord[i]);
        if (TRUE == pSrecRecord->bCheckOK)
        {
            HAN_snprintf(&pText[nOffset], nTextBufSize - nOffset, TEXT("√ "));
            nOffset += HAN_strlen(&pText[nOffset]);
        }
        else
        {
            HAN_snprintf(&pText[nOffset], nTextBufSize - nOffset, TEXT("× "));
            nOffset += HAN_strlen(&pText[nOffset]);
        }
        HAN_snprintf(&pText[nOffset], nTextBufSize - nOffset, TEXT("S%1X "), pSrecRecord->cDataType);
        nOffset += HAN_strlen(&pText[nOffset]);
        HAN_snprintf(&pText[nOffset], nTextBufSize - nOffset, TEXT("%02X "), pSrecRecord->nDataLen);
        nOffset += HAN_strlen(&pText[nOffset]);
        HAN_snprintf(&pText[nOffset], nTextBufSize - nOffset, pAddrFormat[cAddrBitLen], pSrecRecord->cAddr);
        nOffset += HAN_strlen(&pText[nOffset]);
        for (uint32_t j = 0; j < ((uint32_t)(pSrecRecord->nDataLen) - (uint32_t)3); j++)
        {
            HAN_snprintf(&pText[nOffset], nTextBufSize - nOffset, TEXT("%02X"), pSrecRecord->pData[j]);
            nOffset += HAN_strlen(&pText[nOffset]);
        }
        if (pSrecRecord->nDataLen > 0)
        {
            HAN_snprintf(&pText[nOffset], nTextBufSize - nOffset, TEXT(" "));
            nOffset += HAN_strlen(&pText[nOffset]);
        }
        HAN_snprintf(&pText[nOffset], nTextBufSize - nOffset, TEXT("%02X\r\n"), pSrecRecord->cCheckValue);
        nOffset += HAN_strlen(&pText[nOffset]);
    }
    pText[nTextBufSize - 1] = TEXT('\0');
    SetWindowText(svInfo->hSrecViewEdit, pText);
}
static LRESULT CreateCallback(HWND hWnd, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PSRECVIEWWNDEXTRA svInfo;
    HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
    PSRECVIEWCFG pSvConfig = ((LPCREATESTRUCT)lParam)->lpCreateParams;
    RECT rcClientSize;

    HANDLE hHeap = GetProcessHeap();
    if (NULL == hHeap) { lWndProcRet = -1; }
    if (-1 != lWndProcRet)
    {
        svInfo = (PSRECVIEWWNDEXTRA)HANWinHeapAlloc(hHeap, NULL, sizeof(SRECVIEWWNDEXTRA));
        if (NULL == svInfo) { lWndProcRet = -1; }
    }

    if (-1 != lWndProcRet)
    {
        HANINT nWinX;
        HANINT nWinY;

        SetWindowLongPtr(hWnd, 0, (LONG_PTR)svInfo);
        svInfo->hHeap = hHeap;

        GetClientRect(hWnd, &rcClientSize);

        svInfo->hHexFont = CreateFontIndirect(&g_lfHexFont);
        svInfo->hSysFont = CreateFontIndirect(&g_lfSysFont);

        svInfo->hOpenSrecFileButton = CreateWindow(TEXT("button"), TEXT("打开Srec文件"),
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 30, 30, 130, 30,
            hWnd, (HMENU)WID_SREC_VIEW_OPEN_SREC_FILE_BUTTON, hInst, NULL);

        nWinX = 30; nWinY = 100;
        svInfo->hStartAddrStatic = CreateWindow(TEXT("static"), TEXT("0x"),
            WS_CHILD | WS_VISIBLE | SS_RIGHT, nWinX, nWinY, 25, 25,
            hWnd, (HMENU)WID_SREC_VIEW_START_ADDR_STATIC, hInst, NULL);
        nWinX += 25;
        svInfo->hStartAddrEdit = CreateWindow(TEXT("edit"), DEFAULT_START_ADDR_TEXT,
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, nWinX, nWinY - 1, 100, 25,
            hWnd, (HMENU)WID_SREC_VIEW_START_ADDR_EDIT, hInst, NULL);
        nWinX += 100;
        svInfo->hEndAddrStatic = CreateWindow(TEXT("static"), TEXT(" ~ 0x"),
            WS_CHILD | WS_VISIBLE | SS_RIGHT, nWinX, nWinY, 51, 25,
            hWnd, (HMENU)WID_SREC_VIEW_END_ADDR_STATIC, hInst, NULL);
        nWinX += 51;
        svInfo->hEndAddrEdit = CreateWindow(TEXT("edit"), DEFAULT_END_ADDR_TEXT,
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, nWinX, nWinY - 1, 100, 25,
            hWnd, (HMENU)WID_SREC_VIEW_END_ADDR_EDIT, hInst, NULL);
        nWinX = 30; nWinY += 35;
        svInfo->hSaveBinFile = CreateWindow(TEXT("button"), TEXT("保存Bin文件"),
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, nWinX, nWinY, 120, 30,
            hWnd, (HMENU)WID_SREC_VIEW_SAVE_BIN_FILE, hInst, NULL);
        nWinX += 130;
        svInfo->hSaveFilledSrecFileButton = CreateWindow(TEXT("button"), TEXT("保存填充后的Srec文件"),
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, nWinX, nWinY, 200, 30,
            hWnd, (HMENU)WID_SREC_VIEW_SAVE_FILLED_SREC_FILE, hInst, NULL);
        nWinX = 30; nWinY += 70;
        svInfo->hLinkFileList = CreateWindow(
            TEXT("listbox"), NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_DISABLENOSCROLL, nWinX, nWinY, 250, 180,
            hWnd, (HMENU)WID_SREC_VIEW_LINK_FILE_LIST, hInst, NULL);
        nWinX += 280; nWinY += 50;
        svInfo->hLinkFileAdd = CreateWindow(
            TEXT("button"), TEXT("＋"),
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, nWinX, nWinY, 25, 25,
            hWnd, (HMENU)WID_SREC_VIEW_LINK_FILE_ADD, hInst, NULL);
        nWinY += 50;
        svInfo->hLinkFileSub = CreateWindow(
            TEXT("button"), TEXT("－"),
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, nWinX, nWinY, 25, 25,
            hWnd, (HMENU)WID_SREC_VIEW_LINK_FILE_SUB, hInst, NULL);
        nWinX = 30; nWinY += 90;
        svInfo->hLinkFileSub = CreateWindow(
            TEXT("button"), TEXT("保存拼接后的Srec文件"),
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, nWinX, nWinY, 200, 30,
            hWnd, (HMENU)WID_SREC_VIEW_LINK_FILE_LINK, hInst, NULL);
        
        nWinX = 500;
        svInfo->hMemMapEdit = CreateWindow(
            TEXT("edit"), NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_READONLY,
            nWinX, 10, 260, GetRectH(&rcClientSize) - 50,
            hWnd, (HMENU)WID_SREC_VIEW_SREC_MEM_MAP_EDIT, hInst, NULL);
        nWinX += 270;
        svInfo->hSrecViewEdit = CreateWindow(
            TEXT("edit"), NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL | ES_LEFT | ES_MULTILINE | ES_READONLY,
            nWinX, 10, 510, GetRectH(&rcClientSize) - 50,
            hWnd, (HMENU)WID_SREC_VIEW_VIEW, hInst, NULL);

        SendMessage(svInfo->hOpenSrecFileButton, WM_SETFONT, (WPARAM)(svInfo->hSysFont), (LPARAM)TRUE);
        SendMessage(svInfo->hStartAddrStatic, WM_SETFONT, (WPARAM)(svInfo->hHexFont), (LPARAM)TRUE);
        SendMessage(svInfo->hStartAddrEdit, WM_SETFONT, (WPARAM)(svInfo->hHexFont), (LPARAM)TRUE);
        SendMessage(svInfo->hEndAddrStatic, WM_SETFONT, (WPARAM)(svInfo->hHexFont), (LPARAM)TRUE);
        SendMessage(svInfo->hEndAddrEdit, WM_SETFONT, (WPARAM)(svInfo->hHexFont), (LPARAM)TRUE);
        SendMessage(svInfo->hSaveBinFile, WM_SETFONT, (WPARAM)(svInfo->hSysFont), (LPARAM)TRUE);
        SendMessage(svInfo->hSaveFilledSrecFileButton, WM_SETFONT, (WPARAM)(svInfo->hSysFont), (LPARAM)TRUE);
        SendMessage(svInfo->hSrecViewEdit, WM_SETFONT, (WPARAM)(svInfo->hHexFont), (LPARAM)TRUE);
        SendMessage(svInfo->hMemMapEdit, WM_SETFONT, (WPARAM)(svInfo->hHexFont), (LPARAM)TRUE);
        SendMessage(svInfo->hLinkFileList, WM_SETFONT, (WPARAM)(svInfo->hHexFont), (LPARAM)TRUE);
        SendMessage(svInfo->hLinkFileAdd, WM_SETFONT, (WPARAM)(svInfo->hSysFont), (LPARAM)TRUE);
        SendMessage(svInfo->hLinkFileSub, WM_SETFONT, (WPARAM)(svInfo->hSysFont), (LPARAM)TRUE);
        SendMessage(svInfo->hLinkFileLink, WM_SETFONT, (WPARAM)(svInfo->hSysFont), (LPARAM)TRUE);

        svInfo->bSrecOpen = FALSE;

        if (NULL != pSvConfig)
        {
            (void)memcpy(&(svInfo->pSvConfig), pSvConfig, sizeof(svInfo->pSvConfig));
            LoadSrecViewCfg(svInfo, pSvConfig);
        }
    }

    return lWndProcRet;
}
static void LoadSrecViewCfg(PSRECVIEWWNDEXTRA svInfo, PSRECVIEWCFG pSvConfig)
{
    INISRECVIEWCFGID iLoop;

    for (iLoop = 0; iLoop < INI_SREC_VIEW_CFG_CNT; iLoop++)
    {
        sg_pSrecViewCfgInfo[iLoop].CfgTextToWindow(svInfo, pSvConfig->pSysConfig[iLoop].pValue);
    }
}
static void CommandCallback(HWND hWnd, PSRECVIEWWNDEXTRA svInfo, WPARAM wParam, LPARAM lParam)
{
    (void)lParam;

    switch (LOWORD(wParam)) {
        case WID_SREC_VIEW_OPEN_SREC_FILE_BUTTON: {
            OpenFileAction(svInfo);
            break;
        }
        case WID_SREC_VIEW_SAVE_BIN_FILE: {
            if (TRUE == svInfo->bSrecOpen) { SaveBinFileAction(svInfo); }
            else { MessageBox(hWnd, TEXT("请先打开一个Srec文件"), TEXT("错误"), 0); }
            break;
        }
        case WID_SREC_VIEW_SAVE_FILLED_SREC_FILE: {
            if (TRUE == svInfo->bSrecOpen) { SaveFilledSrecFileAction(svInfo); }
            else { MessageBox(hWnd, TEXT("请先打开一个Srec文件"), TEXT("错误"), 0); }
            break;
        }
        case WID_SREC_VIEW_LINK_FILE_ADD: {
            AddLinkFileAction(svInfo);
            break;
        }
        case WID_SREC_VIEW_LINK_FILE_SUB: {
            SubLinkFileAction(svInfo);
            break;
        }
        case WID_SREC_VIEW_LINK_FILE_LINK: {
            LinkLinkFileAction(svInfo);
            break;
        }

        default: {
            break;
        }
    }
}
static void OpenFileAction(PSRECVIEWWNDEXTRA svInfo)
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
        HANOpenSrecFile(&(svInfo->stSrecText.sfSrecFile), pInputName, svInfo->hHeap);
        uint32_t nTextBufSize = sizeof(HANCHAR) * (svInfo->stSrecText.sfSrecFile.nTextLen * 2);
        HANPSTR pText = HANWinHeapAlloc(svInfo->hHeap, svInfo->stSrecText.pText, nTextBufSize);
        if (NULL != pText)
        {
            svInfo->stSrecText.pText = pText;
            svInfo->stSrecText.nTextBufSize = nTextBufSize;
            SetWindowText(svInfo->hMemMapEdit, TEXT(""));
            PrintSrecView(svInfo);
            PrintSrecMemMap(svInfo);
            svInfo->bSrecOpen = TRUE;
        }
    }
}
static void SaveBinFileAction(PSRECVIEWWNDEXTRA svInfo)
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
    
        GetWindowText(svInfo->hStartAddrEdit, pText[0], ArrLen(pText[0]));
        GetWindowText(svInfo->hEndAddrEdit, pText[1], ArrLen(pText[1]));
        amAddrMap.cStartAddr = HAN_strtoul(pText[0], NULL, 16);
        amAddrMap.cEndAddr = HAN_strtoul(pText[1], NULL, 16);
        SrecViewSrecToBin(&(svInfo->stSrecText.sfSrecFile), &amAddrMap, svInfo->hHeap);

        hFile = CreateFile(pInputName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
        if (INVALID_HANDLE_VALUE != hFile)
        {
            PSRECFILE pSrecFile = &(svInfo->stSrecText.sfSrecFile);
            DWORD nLen = pSrecFile->amBinAddrMap.cEndAddr - pSrecFile->amBinAddrMap.cStartAddr + (uint32_t)1;
            WriteFile(hFile, pSrecFile->pBinData, nLen, NULL, NULL);
            CloseHandle(hFile);
        }
    }
}
static void AddLinkFileAction(PSRECVIEWWNDEXTRA svInfo)
{
    HANCHAR pText[PATH_STR_SIZE];
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
        ListBoxAddString(svInfo->hLinkFileList, pText);
    }
}
static void SubLinkFileAction(PSRECVIEWWNDEXTRA svInfo)
{
    HANINT nTar = ListBoxGetCursel(svInfo->hLinkFileList);
    SendMessage(svInfo->hLinkFileList, LB_DELETESTRING, (WPARAM)nTar, 0);
}
static void LinkLinkFileAction(PSRECVIEWWNDEXTRA svInfo)
{
    HANPSTR pFileName;
    HANCHAR pDestName[PATH_STR_SIZE];
    HANINT nForMax = LINK_FILE_CNT_MAX;
    HANINT nListStrCnt = (HANINT)SendMessage(svInfo->hLinkFileList, LB_GETCOUNT, 0, 0);
    OPENFILENAME ofnOpenFile;

    pFileName = (HANPSTR)HANWinHeapAlloc(svInfo->hHeap, NULL, LINK_FILE_CNT_MAX * PATH_STR_SIZE);
    if (NULL != pFileName)
    {
        if (nListStrCnt < nForMax) { nForMax = nListStrCnt; }
        for (HANINT i = 0; i < nForMax; i++)
        {
            SendMessage(svInfo->hLinkFileList, LB_GETTEXT, (WPARAM)i, (LPARAM)(&pFileName[i * PATH_STR_SIZE]));
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
            LinkSrecFile(pDestName, pFileName, nForMax);
        }
    }

    if (NULL != pFileName) { HANWinHeapFree(svInfo->hHeap, 0, pFileName); }
}
static void PrintSrecMemMap(PSRECVIEWWNDEXTRA svInfo)
{
    uint32_t cLastAddr = 0;
    ADDRMAP amAddrMap = { .cStartAddr = 0x00000000, .cEndAddr = 0x00000000 };
    PSRECFILE pSrecFile = &(svInfo->stSrecText.sfSrecFile);
    BOOL bStart = TRUE;
    BOOL bNewMap = FALSE;

    for (uint32_t i = 0; i < pSrecFile->nSrecRecordCnt; i++)
    {
        PSRECRECORD pRecord = &(pSrecFile->pSrecRecord[i]);
        if ((1 <= pRecord->cDataType) && (pRecord->cDataType <= 3))
        {
            if (cLastAddr < pRecord->cAddr)
            {
                bNewMap = TRUE;
                if (TRUE == bStart)
                {
                    bStart = FALSE;
                    amAddrMap.cStartAddr = pRecord->cAddr;
                }
                else
                {
                    amAddrMap.cEndAddr = cLastAddr - (uint32_t)1;
                    AppendSrecMap(svInfo, &amAddrMap);
                    amAddrMap.cStartAddr = pRecord->cAddr;
                    bNewMap = FALSE;
                }
            }
            cLastAddr = pRecord->cAddr + pRecord->nDataLen - GetSrecAddrBitLen(pRecord->cDataType) - 2;
        }
    }
    if (TRUE == bNewMap)
    {
        amAddrMap.cEndAddr = cLastAddr - (uint32_t)1;
        AppendSrecMap(svInfo, &amAddrMap);
    }
}
static void AppendSrecMap(PSRECVIEWWNDEXTRA svInfo, PADDRMAP pAddrMap)
{
    HANCHAR pText[64];

    HAN_snprintf(pText, ArrLen(pText), TEXT("0x%08X ~ 0x%08X\r\n"), pAddrMap->cStartAddr, pAddrMap->cEndAddr);
    EditAppendText(svInfo->hMemMapEdit, pText, FALSE);
}
static HANERROR SaveFilledSrecFileAction(PSRECVIEWWNDEXTRA svInfo)
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
    
        GetWindowText(svInfo->hStartAddrEdit, pText[0], ArrLen(pText[0]));
        GetWindowText(svInfo->hEndAddrEdit, pText[1], ArrLen(pText[1]));
        amAddrMap.cStartAddr = HAN_strtoul(pText[0], NULL, 16);
        amAddrMap.cEndAddr = HAN_strtoul(pText[1], NULL, 16);
        SrecViewSrecToBin(&(svInfo->stSrecText.sfSrecFile), &amAddrMap, svInfo->hHeap);

        if (RET_OK == nRet)
        {
            hFile = CreateFile(pInputName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
            if (INVALID_HANDLE_VALUE != hFile)
            {
                PSRECFILE pSrecFile = &(svInfo->stSrecText.sfSrecFile);
                uint32_t nBinDataLen = pSrecFile->amBinAddrMap.cEndAddr - pSrecFile->amBinAddrMap.cStartAddr + (uint32_t)1;
                BinDataToSrecFile(hFile, 4, pSrecFile->amBinAddrMap.cStartAddr, pSrecFile->pBinData, nBinDataLen);
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

static void CfgWindowToTextStartAddr(PSRECVIEWWNDEXTRA svInfo, HANPSTR pText)
{
    HANCHAR pTempText[INI_SREC_VIEW_VALUE_SIZE];
    HANSIZE cAddr;

    GetWindowText(svInfo->hStartAddrEdit, pTempText, INI_SREC_VIEW_VALUE_SIZE);
    cAddr = HAN_strtoul(pTempText, NULL, 16);
    HAN_snprintf(pText, INI_SREC_VIEW_VALUE_SIZE, TEXT(ADDR_PRINT_FORMAT_SINGLE), cAddr);
}
static void CfgWindowToTextEndAddr(PSRECVIEWWNDEXTRA svInfo, HANPSTR pText)
{
    HANCHAR pTempText[INI_SREC_VIEW_VALUE_SIZE];
    HANSIZE cAddr;

    GetWindowText(svInfo->hEndAddrEdit, pTempText, INI_SREC_VIEW_VALUE_SIZE);
    cAddr = HAN_strtoul(pTempText, NULL, 16);
    HAN_snprintf(pText, INI_SREC_VIEW_VALUE_SIZE, TEXT(ADDR_PRINT_FORMAT_SINGLE), cAddr);
}

static void CfgTextToWindowStartAddr(PSRECVIEWWNDEXTRA svInfo, HANPCSTR pText)
{
    HANSIZE cAddr = HAN_strtoul(pText, NULL, 16);
    HANCHAR pTempText[ADDR_STR_SIZE];

    HAN_snprintf(pTempText, ADDR_STR_SIZE, TEXT(ADDR_PRINT_FORMAT_FULL), (uint32_t)cAddr);
    pTempText[ADDR_STR_SIZE - 1] = TEXT('\0');
    SetWindowText(svInfo->hStartAddrEdit, pTempText);
}
static void CfgTextToWindowEndAddr(PSRECVIEWWNDEXTRA svInfo, HANPCSTR pText)
{
    HANSIZE cAddr = HAN_strtoul(pText, NULL, 16);
    HANCHAR pTempText[ADDR_STR_SIZE];

    HAN_snprintf(pTempText, ADDR_STR_SIZE, TEXT(ADDR_PRINT_FORMAT_FULL), (uint32_t)cAddr);
    pTempText[ADDR_STR_SIZE - 1] = TEXT('\0');
    SetWindowText(svInfo->hEndAddrEdit, pTempText);
}
