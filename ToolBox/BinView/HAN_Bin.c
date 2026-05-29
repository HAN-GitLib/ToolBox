#include "..\..\HAN_Lib\HAN_CRC.h"
#include "..\GlobalVariables.h"
#include "..\HexView\HAN_Hex.h"
#include "HAN_Bin.h"

#define INI_BIN_VIEW_VALUE_SIZE         INI_MAIN_VALUE_STR_SIZE
#define INI_BIN_VIEW_APP_NAME           TEXT("bvConfig")

#define BIN_VIEW_ADDR_STATIC_W      300
#define BIN_VIEW_CRC_RANGE_START_W  117
#define BIN_VIEW_CRC_RANGE_END_W    45
#define BIN_VIEW_CRC_POS_W          BIN_VIEW_CRC_RANGE_START_W

typedef enum {
    WID_BIN_VIEW_BASE = HAN_WID_BASE,
    WID_BIN_VIEW_VIEW,
    WID_BIN_VIEW_OPEN_BIN_FILE,
    WID_BIN_VIEW_ZERO_BASE,
    WID_BIN_VIEW_START_ADDR_INPUT,
    WID_BIN_VIEW_LEN_INPUT,
    WID_BIN_VIEW_SAVE_HEX_FILE,
    WID_BIN_VIEW_DATA_POS_INPUT,
    WID_BIN_VIEW_DATA_LEN_INPUT,
    WID_BIN_VIEW_SET_DATA,
    WID_BIN_VIEW_CRC_LIST,
    WID_BIN_VIEW_CRC_START,
    WID_BIN_VIEW_CRC_END,
    WID_BIN_VIEW_CRC_POS,
    WID_BIN_VIEW_SET_CRC,
} WIDBINVIEW;

typedef struct tagBINVIEWTEXT {
    HANPSTR         pText;
    uint32_t        nTextBufSize;
    uint32_t        nTextLen;
    BINFILE         bfBinFile;
} BINVIEWTEXT, * PBINVIEWTEXT;

typedef struct tagBINVIEWWNDEXTRA {
    HANDLE          hHeap;
    BINVIEWTEXT     btBinText;
    HWND            hOpenBinFile;
    HWND            hZeroBase;
    HWND            hStartAddr;
    HWND            hLen;
    HWND            hSaveHexFile;
    HWND            hBinViewTitleStatic;
    HWND            hDataPos;
    HWND            hDataLen;
    HWND            hSetData;
    HWND            hCrcList;
    HWND            hCrcStart;
    HWND            hCrcEnd;
    HWND            hCrcPos;
    HWND            hSetCrc;
    HWND            hBinViewEdit;
    HFONT           hHexFont;
    HFONT           hSysFont;
    BOOL            bBinOpen;
    BINVIEWCFG      pBvConfig;
} BINVIEWWNDEXTRA, * PBINVIEWWNDEXTRA;

typedef struct tagBINVIEWREADWRITECFG {
    HANPCSTR            pKey;
    HANPCSTR            pDefValue;
    void                (*CfgWindowToText)(PBINVIEWWNDEXTRA bvInfo, HANPSTR pText);
    void                (*CfgTextToWindow)(PBINVIEWWNDEXTRA bvInfo, HANPCSTR pText);
} BINVIEWREADWRITECFG;

typedef struct tagCRCINFO {
    HANCHAR     pCrcName[CRC_NAME_MAX_CNT];
    uint32_t    (*CrcAction)(const uint8_t* pData, uint32_t nLen);
    uint32_t    nCrcValueSize;
} CRCINFO, * PCRCINFO;

static uint32_t CETC_CRC16(const uint8_t* pData, uint32_t nLen);
static uint32_t BinView_CRC4_ITU(const uint8_t* pData, uint32_t nLen) { return CRC4_ITU(pData, nLen); }
static uint32_t BinView_CRC5_EPC(const uint8_t* pData, uint32_t nLen) { return CRC5_EPC(pData, nLen); }
static uint32_t BinView_CRC5_ITU(const uint8_t* pData, uint32_t nLen) { return CRC5_ITU(pData, nLen); }
static uint32_t BinView_CRC5_USB(const uint8_t* pData, uint32_t nLen) { return CRC5_USB(pData, nLen); }
static uint32_t BinView_CRC6_ITU(const uint8_t* pData, uint32_t nLen) { return CRC6_ITU(pData, nLen); }
static uint32_t BinView_CRC7_MMC(const uint8_t* pData, uint32_t nLen) { return CRC7_MMC(pData, nLen); }
static uint32_t BinView_CRC8_STD(const uint8_t* pData, uint32_t nLen) { return CRC8_STD(pData, nLen); }
static uint32_t BinView_CRC8_ITU(const uint8_t* pData, uint32_t nLen) { return CRC8_ITU(pData, nLen); }
static uint32_t BinView_CRC8_ROHC(const uint8_t* pData, uint32_t nLen) { return CRC8_ROHC(pData, nLen); }
static uint32_t BinView_CRC8_MAXIM(const uint8_t* pData, uint32_t nLen) { return CRC8_MAXIM(pData, nLen); }
static uint32_t BinView_CRC16_IBM(const uint8_t* pData, uint32_t nLen) { return CRC16_IBM(pData, nLen); }
static uint32_t BinView_CRC16_MAXIM(const uint8_t* pData, uint32_t nLen) { return CRC16_MAXIM(pData, nLen); }
static uint32_t BinView_CRC16_USB(const uint8_t* pData, uint32_t nLen) { return CRC16_USB(pData, nLen); }
static uint32_t BinView_CRC16_MODEBUS(const uint8_t* pData, uint32_t nLen) { return CRC16_MODEBUS(pData, nLen); }
static uint32_t BinView_CRC16_CCITT(const uint8_t* pData, uint32_t nLen) { return CRC16_CCITT(pData, nLen); }
static uint32_t BinView_CRC16_CCITT_FALSE(const uint8_t* pData, uint32_t nLen) { return CRC16_CCITT_FALSE(pData, nLen); }
static uint32_t BinView_CRC16_X25(const uint8_t* pData, uint32_t nLen) { return CRC16_X25(pData, nLen); }
static uint32_t BinView_CRC16_XMODEM(const uint8_t* pData, uint32_t nLen) { return CRC16_XMODEM(pData, nLen); }
static uint32_t BinView_CRC16_DNP(const uint8_t* pData, uint32_t nLen) { return CRC16_DNP(pData, nLen); }
static uint32_t BinView_CRC32_STD(const uint8_t* pData, uint32_t nLen) { return CRC32_STD(pData, nLen); }
static uint32_t BinView_CRC32_MPEG2(const uint8_t* pData, uint32_t nLen) { return CRC32_MPEG2(pData, nLen); }

static LRESULT CALLBACK BinViewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CreateCallback(HWND hWnd, LPARAM lParam);
static void LoadBinViewCfg(PBINVIEWWNDEXTRA bvInfo, PBINVIEWCFG pBvConfig);
static void CommandCallback(HWND hWnd, PBINVIEWWNDEXTRA bvInfo, WPARAM wParam, LPARAM lParam);
static HANERROR OpenFileAction(PBINVIEWWNDEXTRA bvInfo);
static void PrintBinView(PBINVIEWWNDEXTRA bvInfo);
static void SaveHexFileAction(PBINVIEWWNDEXTRA bvInfo);
static BOOL GetZeroBase(PBINVIEWWNDEXTRA bvInfo);
static uint32_t GetStartAddr(PBINVIEWWNDEXTRA bvInfo);
static void UpdateAddrOffset(PBINVIEWWNDEXTRA bvInfo);
static void SetDataAction(PBINVIEWWNDEXTRA bvInfo);
static void SetCrcAction(PBINVIEWWNDEXTRA bvInfo);
static void InitAddrInput(PBINVIEWWNDEXTRA bvInfo);

static void CfgWindowToTextSaveStartZero(PBINVIEWWNDEXTRA bvInfo, HANPSTR pText);
static void CfgWindowToTextSaveStartAddr(PBINVIEWWNDEXTRA bvInfo, HANPSTR pText);
static void CfgWindowToTextSaveLen(PBINVIEWWNDEXTRA bvInfo, HANPSTR pText);
static void CfgWindowToTextDataPos(PBINVIEWWNDEXTRA bvInfo, HANPSTR pText);
static void CfgWindowToTextDataLen(PBINVIEWWNDEXTRA bvInfo, HANPSTR pText);
static void CfgWindowToTextParityName(PBINVIEWWNDEXTRA bvInfo, HANPSTR pText);
static void CfgWindowToTextParityStart(PBINVIEWWNDEXTRA bvInfo, HANPSTR pText);
static void CfgWindowToTextParityEnd(PBINVIEWWNDEXTRA bvInfo, HANPSTR pText);
static void CfgWindowToTextParityPos(PBINVIEWWNDEXTRA bvInfo, HANPSTR pText);

static void CfgTextToWindowSaveStartZero(PBINVIEWWNDEXTRA bvInfo, HANPCSTR pText);
static void CfgTextToWindowSaveStartAddr(PBINVIEWWNDEXTRA bvInfo, HANPCSTR pText);
static void CfgTextToWindowSaveLen(PBINVIEWWNDEXTRA bvInfo, HANPCSTR pText);
static void CfgTextToWindowDataPos(PBINVIEWWNDEXTRA bvInfo, HANPCSTR pText);
static void CfgTextToWindowDataLen(PBINVIEWWNDEXTRA bvInfo, HANPCSTR pText);
static void CfgTextToWindowParityName(PBINVIEWWNDEXTRA bvInfo, HANPCSTR pText);
static void CfgTextToWindowParityStart(PBINVIEWWNDEXTRA bvInfo, HANPCSTR pText);
static void CfgTextToWindowParityEnd(PBINVIEWWNDEXTRA bvInfo, HANPCSTR pText);
static void CfgTextToWindowParityPos(PBINVIEWWNDEXTRA bvInfo, HANPCSTR pText);

static const CRCINFO g_pCrcInfo[] = {
    { .pCrcName = TEXT("CRC4_ITU"), .CrcAction = BinView_CRC4_ITU, .nCrcValueSize = 1, },
    { .pCrcName = TEXT("CRC5_EPC"), .CrcAction = BinView_CRC5_EPC, .nCrcValueSize = 1, },
    { .pCrcName = TEXT("CRC5_ITU"), .CrcAction = BinView_CRC5_ITU, .nCrcValueSize = 1, },
    { .pCrcName = TEXT("CRC5_USB"), .CrcAction = BinView_CRC5_USB, .nCrcValueSize = 1, },
    { .pCrcName = TEXT("CRC6_ITU"), .CrcAction = BinView_CRC6_ITU, .nCrcValueSize = 1, },
    { .pCrcName = TEXT("CRC7_MMC"), .CrcAction = BinView_CRC7_MMC, .nCrcValueSize = 1, },
    { .pCrcName = TEXT("CRC8_STD"), .CrcAction = BinView_CRC8_STD, .nCrcValueSize = 1, },
    { .pCrcName = TEXT("CRC8_ITU"), .CrcAction = BinView_CRC8_ITU, .nCrcValueSize = 1, },
    { .pCrcName = TEXT("CRC8_ROHC"), .CrcAction = BinView_CRC8_ROHC, .nCrcValueSize = 1, },
    { .pCrcName = TEXT("CRC8_MAXIM"), .CrcAction = BinView_CRC8_MAXIM, .nCrcValueSize = 1, },
    { .pCrcName = TEXT("CRC16_IBM"), .CrcAction = BinView_CRC16_IBM, .nCrcValueSize = 2, },
    { .pCrcName = TEXT("CRC16_MAXIM"), .CrcAction = BinView_CRC16_MAXIM, .nCrcValueSize = 2, },
    { .pCrcName = TEXT("CRC16_USB"), .CrcAction = BinView_CRC16_USB, .nCrcValueSize = 2, },
    { .pCrcName = TEXT("CRC16_MODEBUS"), .CrcAction = BinView_CRC16_MODEBUS, .nCrcValueSize = 2, },
    { .pCrcName = TEXT("CRC16_CCITT"), .CrcAction = BinView_CRC16_CCITT, .nCrcValueSize = 2, },
    { .pCrcName = TEXT("CRC16_CCITT_FALSE"), .CrcAction = BinView_CRC16_CCITT_FALSE, .nCrcValueSize = 2, },
    { .pCrcName = TEXT("CRC16_UDS"), .CrcAction = CETC_CRC16, .nCrcValueSize = 2, },
    { .pCrcName = TEXT("CRC16_X25"), .CrcAction = BinView_CRC16_X25, .nCrcValueSize = 2, },
    { .pCrcName = TEXT("CRC16_XMODEM"), .CrcAction = BinView_CRC16_XMODEM, .nCrcValueSize = 2, },
    { .pCrcName = TEXT("CRC16_DNP"), .CrcAction = BinView_CRC16_DNP, .nCrcValueSize = 2, },
    { .pCrcName = TEXT("CRC32_STD"), .CrcAction = BinView_CRC32_STD, .nCrcValueSize = 4, },
    { .pCrcName = TEXT("CRC32_MPEG2"), .CrcAction = BinView_CRC32_MPEG2, .nCrcValueSize = 4, },
};
static const BINVIEWREADWRITECFG sg_pBinViewCfgInfo[INI_BIN_VIEW_CFG_CNT] = {
    [INI_BIN_VIEW_SAVE_START_ZERO] = {
        .pKey = TEXT("SaveStartZero"),
        .pDefValue = TEXT("FALSE"),
        .CfgWindowToText = CfgWindowToTextSaveStartZero,
        .CfgTextToWindow = CfgTextToWindowSaveStartZero,
    },
    [INI_BIN_VIEW_SAVE_START_ADDR] = {
        .pKey = TEXT("SaveStartAddr"),
        .pDefValue = DEFAULT_START_ADDR_TEXT,
        .CfgWindowToText = CfgWindowToTextSaveStartAddr,
        .CfgTextToWindow = CfgTextToWindowSaveStartAddr,
    },
    [INI_BIN_VIEW_SAVE_LEN] = {
        .pKey = TEXT("SaveLen"),
        .pDefValue = TEXT(""),
        .CfgWindowToText = CfgWindowToTextSaveLen,
        .CfgTextToWindow = CfgTextToWindowSaveLen,
    },
    [INI_BIN_VIEW_DATA_POS] = {
        .pKey = TEXT("DataPos"),
        .pDefValue = DEFAULT_DATA_POS_TEXT,
        .CfgWindowToText = CfgWindowToTextDataPos,
        .CfgTextToWindow = CfgTextToWindowDataPos,
    },
    [INI_BIN_VIEW_DATA_LEN] = {
        .pKey = TEXT("DataLen"),
        .pDefValue = TEXT(""),
        .CfgWindowToText = CfgWindowToTextDataLen,
        .CfgTextToWindow = CfgTextToWindowDataLen,
    },
    [INI_BIN_VIEW_PARITY_NAME] = {
        .pKey = TEXT("ParityName"),
        .pDefValue = TEXT("0"),
        .CfgWindowToText = CfgWindowToTextParityName,
        .CfgTextToWindow = CfgTextToWindowParityName,
    },
    [INI_BIN_VIEW_PARITY_START] = {
        .pKey = TEXT("ParityStart"),
        .pDefValue = DEFAULT_CRC_START_TEXT,
        .CfgWindowToText = CfgWindowToTextParityStart,
        .CfgTextToWindow = CfgTextToWindowParityStart,
    },
    [INI_BIN_VIEW_PARITY_END] = {
        .pKey = TEXT("ParityEnd"),
        .pDefValue = DEFAULT_CRC_END_TEXT,
        .CfgWindowToText = CfgWindowToTextParityEnd,
        .CfgTextToWindow = CfgTextToWindowParityEnd,
    },
    [INI_BIN_VIEW_PARITY_POS] = {
        .pKey = TEXT("ParityPos"),
        .pDefValue = DEFAULT_CRC_POS_TEXT,
        .CfgWindowToText = CfgWindowToTextParityPos,
        .CfgTextToWindow = CfgTextToWindowParityPos,
    },
};

void RegisterHANBinView(HINSTANCE hInst)
{
    WNDCLASSEX wcex = {
        .cbSize         = sizeof(WNDCLASSEX),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = BinViewWndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = sizeof(PBINVIEWWNDEXTRA),
        .hInstance      = hInst,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName   = NULL,
        .lpszClassName  = HAN_BIN_VIEW_CLASS,
        .hIconSm        = NULL,
    };
    RegisterClassEx(&wcex);
}

void ReadBinViewIniFile(HANPCSTR pIniPath, void* pParam)
{
    PBINVIEWCFG pBvConfig = pParam;
    INIBINVIEWCFGID iLoop;

    for (iLoop = 0; iLoop < INI_BIN_VIEW_CFG_CNT; iLoop++)
    {
        HAN_strcpy(pBvConfig->pSysConfig[iLoop].pKey, sg_pBinViewCfgInfo[iLoop].pKey);
        HAN_strcpy(pBvConfig->pSysConfig[iLoop].pDefValue, sg_pBinViewCfgInfo[iLoop].pDefValue);
        GetPrivateProfileString(
            INI_BIN_VIEW_APP_NAME, pBvConfig->pSysConfig[iLoop].pKey, pBvConfig->pSysConfig[iLoop].pDefValue,
            pBvConfig->pSysConfig[iLoop].pValue, INI_BIN_VIEW_VALUE_SIZE, pIniPath);
    }
}

void WriteBinViewIniFile(HANPCSTR pIniPath, HWND hBinView)
{
    PBINVIEWWNDEXTRA bvInfo = (PBINVIEWWNDEXTRA)GetWindowLongPtr(hBinView, 0);
    HANCHAR pBvCfg[INI_BIN_VIEW_CFG_CNT][INI_BIN_VIEW_VALUE_SIZE];
    INIBINVIEWCFGID iLoop;
    
    for (iLoop = 0; iLoop < INI_BIN_VIEW_CFG_CNT; iLoop++)
    {
        sg_pBinViewCfgInfo[iLoop].CfgWindowToText(bvInfo, pBvCfg[iLoop]);
        WritePrivateProfileString(
            INI_BIN_VIEW_APP_NAME, sg_pBinViewCfgInfo[iLoop].pKey, pBvCfg[iLoop], pIniPath
        );
    }
}

HANERROR HANOpenBinFile(PBINFILE pBinFile, HANPCSTR pFileName, HANDLE hHeap)
{
    HANERROR nRet = RET_OK;
    HANDLE hFile = CreateFile(pFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    DWORD nFileSize;
    
    if (INVALID_HANDLE_VALUE == hFile) { nRet = OPEN_FILE_ERR; }

    if (RET_OK == nRet)
    {
        nFileSize = GetFileSize(hFile, NULL);
        uint8_t* pData = (uint8_t*)HANWinHeapAlloc(hHeap, pBinFile->pBinData, nFileSize);
        if (NULL == pData) { nRet = NOT_ENOUGH_MEMORY; }
        else { pBinFile->pBinData = pData; }
    }
    if (RET_OK == nRet)
    {
        DWORD nReadSize;
        (void)ReadFile(hFile, pBinFile->pBinData, nFileSize, &nReadSize, NULL);
        pBinFile->nBinLen = (uint32_t)nReadSize;
    }

    if (INVALID_HANDLE_VALUE != hFile) { CloseHandle(hFile); }

    return nRet;
}
void HANBinFileSetData(uint8_t* pDest, uint32_t nDestLen, HANPCSTR pSrcName, uint32_t nOffset, uint32_t nSrcLen)
{
    if (nOffset < nDestLen)
    {
        HANDLE hFile = CreateFile(pSrcName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        HANDLE hHeap = GetProcessHeap();
        uint32_t nLen = nSrcLen;
        if (INVALID_HANDLE_VALUE != hFile)
        {
            DWORD nFileSize = GetFileSize(hFile, NULL);
            uint8_t* pSrc = (uint8_t*)HANWinHeapAlloc(hHeap, NULL, nFileSize);

            if (0 == nLen) { nLen = nFileSize; }
            if (NULL != pSrc)
            {
                if (nFileSize < nLen) { nLen = nFileSize; }
                (void)ReadFile(hFile, pSrc, nLen, NULL, NULL);
                if (nOffset + nLen > nDestLen)
                {
                    nLen = nDestLen - nOffset;
                }

                memcpy(&pDest[nOffset], pSrc, nLen);

                HANWinHeapFree(hHeap, 0, pSrc);
            }

            CloseHandle(hFile);
        }
    }
}
uint32_t HANBinFileSetCrc(uint8_t* pData, uint32_t nLen, uint32_t cCrcId, uint8_t* pCrcResult)
{
    uint32_t cCrcValue = g_pCrcInfo[cCrcId].CrcAction(pData, nLen);
    for (uint32_t i = 0; i < g_pCrcInfo[cCrcId].nCrcValueSize; i++)
    {
        pCrcResult[i] = cCrcValue >> ((g_pCrcInfo[cCrcId].nCrcValueSize - 1 - i) * 8);
    }
    return cCrcValue;
}

BOOL HANBinViewGetCrcRoutine(uint32_t nCrcId, HANPSTR pCrcName, uint32_t nNameMaxCnt, uint32_t* pCrcSize)
{
    BOOL bRet = TRUE;
    if (nCrcId < ArrLen(g_pCrcInfo))
    {
        if (NULL != pCrcName) { HAN_strncpy(pCrcName, g_pCrcInfo[nCrcId].pCrcName, nNameMaxCnt); }
        if (NULL != pCrcSize) { *pCrcSize = g_pCrcInfo[nCrcId].nCrcValueSize; }
    }
    else { bRet = FALSE; }
    return bRet;
}

static uint32_t CETC_CRC16(const uint8_t* pData, uint32_t nLen)
{
    uint8_t  i;
    uint32_t index = 0x0u;
    uint16_t cRet = (uint16_t)0xFFFF;

    while(nLen != 0u)
    {
        for (i = 0x80u; i != 0u; i >>= 1u) 
        {
            if ((cRet & 0x8000u) != 0u) 
            {
                cRet <<= 1u; 
                cRet ^= 0x1021u;
            } 
            else 
            {
                cRet <<= 1u;
            }
            
            if ((pData[index] & i) != 0u) 
            {
                cRet ^= 0x1021u; 
            }

        }
        
        index++;
        nLen--;   
    }

    return (uint32_t)cRet;
}

static LRESULT CALLBACK BinViewWndProc(HWND hBinView, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    PBINVIEWWNDEXTRA bvInfo = (PBINVIEWWNDEXTRA)GetWindowLongPtr(hBinView, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = CreateCallback(hBinView, lParam);
        } break;
        case WM_COMMAND: {
            CommandCallback(hBinView, bvInfo, wParam, lParam);
        } break;
        case WM_CTLCOLORSTATIC: {
            lWndProcRet = (INT_PTR)GetStockObject(WHITE_BRUSH);
        } break;
        case WM_DESTROY: {
            HANWinHeapFree(bvInfo->hHeap, 0, bvInfo);
            lWndProcRet = DefWindowProc(hBinView, message, wParam, lParam);
        }

        default: {
            lWndProcRet = DefWindowProc(hBinView, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}
static LRESULT CreateCallback(HWND hWnd, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PBINVIEWWNDEXTRA bvInfo;
    HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
    PBINVIEWCFG pBvConfig = ((LPCREATESTRUCT)lParam)->lpCreateParams;
    RECT rcClientSize;

    HANDLE hHeap = GetProcessHeap();
    if (NULL == hHeap) { lWndProcRet = -1; }
    if (-1 != lWndProcRet)
    {
        bvInfo = (PBINVIEWWNDEXTRA)HANWinHeapAlloc(hHeap, NULL, sizeof(BINVIEWWNDEXTRA));
        if (NULL == bvInfo) { lWndProcRet = -1; }
    }

    if (-1 != lWndProcRet)
    {
        int nWinX;
        int nWinY;

        RegisterHANInput(hInst);

        SetWindowLongPtr(hWnd, 0, (LONG_PTR)bvInfo);
        bvInfo->hHeap = hHeap;

        GetClientRect(hWnd, &rcClientSize);

        bvInfo->hHexFont = CreateFontIndirect(&g_lfHexFont);
        bvInfo->hSysFont = CreateFontIndirect(&g_lfSysFont);

        bvInfo->hOpenBinFile = CreateWindow(TEXT("button"), TEXT("打开Bin文件"),
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 30, 30, 130, 30,
            hWnd, (HMENU)WID_BIN_VIEW_OPEN_BIN_FILE, hInst, NULL);

        bvInfo->hZeroBase = CreateWindow(TEXT("button"), TEXT("地址从0开始"),
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 200, 30, 130, 30,
            hWnd, (HMENU)WID_BIN_VIEW_ZERO_BASE, hInst, NULL);

        nWinX = 30; nWinY = 100;
        bvInfo->hStartAddr = CreateWindow(HINPUT_CLASS, NULL,
            WS_CHILD | WS_VISIBLE, nWinX, nWinY, BIN_VIEW_ADDR_STATIC_W + SYS_TEXT_W, SYS_TEXT_H,
            hWnd, (HMENU)WID_BIN_VIEW_START_ADDR_INPUT, hInst, NULL);
        nWinY += SYS_TEXT_H + 10;
        bvInfo->hLen = CreateWindow(HINPUT_CLASS, NULL,
            WS_CHILD | WS_VISIBLE, nWinX, nWinY, BIN_VIEW_ADDR_STATIC_W + SYS_TEXT_W, SYS_TEXT_H,
            hWnd, (HMENU)WID_BIN_VIEW_LEN_INPUT, hInst, NULL);
        nWinY += SYS_TEXT_H + 10;
        bvInfo->hSaveHexFile = CreateWindow(TEXT("button"), TEXT("保存Hex文件"),
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, nWinX, nWinY, 120, 30,
            hWnd, (HMENU)WID_BIN_VIEW_SAVE_HEX_FILE, hInst, NULL);

        nWinX = 30; nWinY += 70;
        bvInfo->hDataPos = CreateWindow(HINPUT_CLASS, NULL,
            WS_CHILD | WS_VISIBLE, nWinX, nWinY, BIN_VIEW_ADDR_STATIC_W + SYS_TEXT_W, SYS_TEXT_H,
            hWnd, (HMENU)WID_BIN_VIEW_DATA_POS_INPUT, hInst, NULL);
        nWinY += SYS_TEXT_H + 10;
        bvInfo->hDataLen = CreateWindow(HINPUT_CLASS, NULL,
            WS_CHILD | WS_VISIBLE, nWinX, nWinY, BIN_VIEW_ADDR_STATIC_W + SYS_TEXT_W, SYS_TEXT_H,
            hWnd, (HMENU)WID_BIN_VIEW_DATA_LEN_INPUT, hInst, NULL);
        nWinY += SYS_TEXT_H + 10;
        bvInfo->hSetData = CreateWindow(TEXT("button"), TEXT("导入Bin文件"),
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, nWinX, nWinY, 120, 30,
            hWnd, (HMENU)WID_BIN_VIEW_SET_DATA, hInst, NULL);
        
        nWinX = 30; nWinY += 70;
        bvInfo->hCrcList = CreateWindow(TEXT("combobox"), NULL,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST, nWinX, nWinY, 200, 150,
            hWnd, (HMENU)WID_BIN_VIEW_CRC_LIST, hInst, NULL);
        nWinY += SYS_TEXT_H + 10;
        bvInfo->hCrcStart = CreateWindow(HINPUT_CLASS, NULL,
            WS_CHILD | WS_VISIBLE, nWinX, nWinY, BIN_VIEW_CRC_RANGE_START_W + SYS_TEXT_W, SYS_TEXT_H,
            hWnd, (HMENU)WID_BIN_VIEW_CRC_START, hInst, NULL);
        nWinX += BIN_VIEW_CRC_RANGE_START_W + SYS_TEXT_W;
        bvInfo->hCrcEnd = CreateWindow(HINPUT_CLASS, NULL,
            WS_CHILD | WS_VISIBLE, nWinX, nWinY, BIN_VIEW_CRC_RANGE_END_W + SYS_TEXT_W, SYS_TEXT_H,
            hWnd, (HMENU)WID_BIN_VIEW_CRC_END, hInst, NULL);
        nWinX = 30; nWinY += SYS_TEXT_H + 10;
        bvInfo->hCrcPos = CreateWindow(HINPUT_CLASS, NULL,
            WS_CHILD | WS_VISIBLE, nWinX, nWinY, BIN_VIEW_CRC_POS_W + SYS_TEXT_W, SYS_TEXT_H,
            hWnd, (HMENU)WID_BIN_VIEW_CRC_POS, hInst, NULL);
        nWinY += SYS_TEXT_H + 10;
        bvInfo->hSetCrc = CreateWindow(TEXT("button"), TEXT("计算CRC"),
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, nWinX, nWinY, 120, 30,
            hWnd, (HMENU)WID_BIN_VIEW_SET_CRC, hInst, NULL);

        nWinX = 500; nWinY = 10;
        bvInfo->hBinViewTitleStatic = CreateWindow(TEXT("static"),
            TEXT("           00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F"),
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            nWinX + 2, nWinY, 610, 17,
            hWnd, (HMENU)WID_BIN_VIEW_SAVE_HEX_FILE, hInst, NULL);
        nWinY += 17;
        bvInfo->hBinViewEdit = CreateWindow(TEXT("edit"), NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL | ES_LEFT | ES_MULTILINE | ES_READONLY,
            nWinX, nWinY, 610, GetRectH(&rcClientSize) - 50,
            hWnd, (HMENU)WID_BIN_VIEW_VIEW, hInst, NULL);
        
        InitAddrInput(bvInfo);

        for (uint32_t i = 0; i < ArrLen(g_pCrcInfo); i++)
        {
            ComboBoxAddString(bvInfo->hCrcList, g_pCrcInfo[i].pCrcName);
        }
        ComboBoxSetCursel(bvInfo->hCrcList, DEFAULT_CRC_ID);

        SendMessage(bvInfo->hOpenBinFile, WM_SETFONT, (WPARAM)(bvInfo->hSysFont), (LPARAM)TRUE);
        SendMessage(bvInfo->hZeroBase, WM_SETFONT, (WPARAM)(bvInfo->hSysFont), (LPARAM)TRUE);
        SendMessage(bvInfo->hSaveHexFile, WM_SETFONT, (WPARAM)(bvInfo->hSysFont), (LPARAM)TRUE);
        SendMessage(bvInfo->hSetData, WM_SETFONT, (WPARAM)(bvInfo->hSysFont), (LPARAM)TRUE);
        SendMessage(bvInfo->hCrcList, WM_SETFONT, (WPARAM)(bvInfo->hSysFont), (LPARAM)TRUE);
        SendMessage(bvInfo->hSetCrc, WM_SETFONT, (WPARAM)(bvInfo->hSysFont), (LPARAM)TRUE);
        SendMessage(bvInfo->hBinViewTitleStatic, WM_SETFONT, (WPARAM)(bvInfo->hHexFont), (LPARAM)TRUE);
        SendMessage(bvInfo->hBinViewEdit, WM_SETFONT, (WPARAM)(bvInfo->hHexFont), (LPARAM)TRUE);

        bvInfo->bBinOpen = FALSE;

        if (NULL != pBvConfig)
        {
            (void)memcpy(&(bvInfo->pBvConfig), pBvConfig, sizeof(bvInfo->pBvConfig));
            LoadBinViewCfg(bvInfo, pBvConfig);
        }
    }

    return lWndProcRet;
}
static void LoadBinViewCfg(PBINVIEWWNDEXTRA bvInfo, PBINVIEWCFG pBvConfig)
{
    INIBINVIEWCFGID iLoop;

    for (iLoop = 0; iLoop < INI_BIN_VIEW_CFG_CNT; iLoop++)
    {
        sg_pBinViewCfgInfo[iLoop].CfgTextToWindow(bvInfo, pBvConfig->pSysConfig[iLoop].pValue);
    }
}
static void CommandCallback(HWND hWnd, PBINVIEWWNDEXTRA bvInfo, WPARAM wParam, LPARAM lParam)
{
    (void)lParam;
    
    switch (LOWORD(wParam)) {
        case WID_BIN_VIEW_OPEN_BIN_FILE: {
            OpenFileAction(bvInfo);
            break;
        }
        case WID_BIN_VIEW_ZERO_BASE: {
            if (TRUE == bvInfo->bBinOpen)
            {
                if (BN_CLICKED == HIWORD(wParam))
                {
                    UpdateAddrOffset(bvInfo);
                }
            }
            break;
        }
        case WID_BIN_VIEW_SAVE_HEX_FILE: {
            if (TRUE == bvInfo->bBinOpen) { SaveHexFileAction(bvInfo); }
            else { MessageBox(hWnd, TEXT("请先打开一个Bin文件"), TEXT("错误"), 0); }
            break;
        }
        case WID_BIN_VIEW_SET_DATA: {
            if (TRUE == bvInfo->bBinOpen) { SetDataAction(bvInfo); }
            else { MessageBox(hWnd, TEXT("请先打开一个Bin文件"), TEXT("错误"), 0); }
            break;
        }
        case WID_BIN_VIEW_SET_CRC: {
            if (TRUE == bvInfo->bBinOpen) { SetCrcAction(bvInfo); }
            else { MessageBox(hWnd, TEXT("请先打开一个Bin文件"), TEXT("错误"), 0); }
            break;
        }

        default: {
            break;
        }
    }
}
static HANERROR OpenFileAction(PBINVIEWWNDEXTRA bvInfo)
{
    HANERROR nRet = RET_OK;
    HANCHAR pInputName[MAX_PATH] = { 0 };
    OPENFILENAME ofnOpenFile;
    ZeroMemory(&ofnOpenFile, sizeof(ofnOpenFile));
    ofnOpenFile.lStructSize = sizeof(ofnOpenFile);
    ofnOpenFile.hwndOwner = NULL;
    ofnOpenFile.lpstrFilter = TEXT("所有文件\0*.*\0\0");
    ofnOpenFile.nFilterIndex = 0;
    ofnOpenFile.lpstrFile = pInputName;
    ofnOpenFile.nMaxFile = MAX_PATH;
    ofnOpenFile.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
    if (GetOpenFileName(&ofnOpenFile))
    {
        if (RET_OK == HANOpenBinFile(&(bvInfo->btBinText.bfBinFile), pInputName, bvInfo->hHeap))
        {
            bvInfo->bBinOpen = TRUE;
            PrintBinView(bvInfo);
        }
    }
    return nRet;
}
static void PrintBinView(PBINVIEWWNDEXTRA bvInfo)
{
    PBINFILE pBinFile = &(bvInfo->btBinText.bfBinFile);
    uint32_t nBinLen = pBinFile->nBinLen;
    uint32_t nBinLenAlign = nBinLen + (nBinLen % (uint32_t)16);
    uint32_t nBinLineCnt = (nBinLenAlign / (uint32_t)16) + (uint32_t)1;

    /* 公式 = 行数 * (8 + 2 + (16 * 3) + 2)
     * 8        4 字节地址打印占 8 个字符
     * 2        地址和数据中间隔开，这里取 2 个空格
     * (16 * 3) 每行打印 16 个字节的数据，用 " %02X" 占 3 个字符（这样打印第一个字符和地址间总共有 3 个空格）
     * 2        \r\n
     */
    uint32_t nTextBufCharCnt = nBinLineCnt * ((uint32_t)8 + (uint32_t)2 + ((uint32_t)16 * (uint32_t)3) + (uint32_t)2);
    HANPSTR pText = HANWinHeapAlloc(bvInfo->hHeap, bvInfo->btBinText.pText, sizeof(HANCHAR) * nTextBufCharCnt);
    if (NULL != pText)
    {
        HANSIZE nOffset = 0;
        uint32_t nLineLen;
        uint32_t cBaseAddr = 0;

        bvInfo->btBinText.pText = pText;
        bvInfo->btBinText.nTextBufSize = sizeof(HANCHAR) * nTextBufCharCnt;
        if (FALSE == GetZeroBase(bvInfo)) { cBaseAddr = GetStartAddr(bvInfo); }
        for (uint32_t i = 0; i < pBinFile->nBinLen; )
        {
            nLineLen = pBinFile->nBinLen - i;
            if (16 < nLineLen) { nLineLen = 16; }
            HAN_snprintf(&pText[nOffset], nTextBufCharCnt - nOffset, TEXT("%08X  "), i + cBaseAddr);
            nOffset += HAN_strlen(&pText[nOffset]);
            for (uint32_t j = 0; j < nLineLen; j++)
            {
                HAN_snprintf(&pText[nOffset], nTextBufCharCnt - nOffset, TEXT(" %02X"), pBinFile->pBinData[i]);
                nOffset += HAN_strlen(&pText[nOffset]);
                i++;
            }
            HAN_snprintf(&pText[nOffset], nTextBufCharCnt - nOffset, TEXT("\r\n"));
            nOffset += HAN_strlen(&pText[nOffset]);
        }
        EditSetLimitText(bvInfo->hBinViewEdit, nTextBufCharCnt);
        pText[nTextBufCharCnt - 1] = TEXT('\0');
        SetWindowText(bvInfo->hBinViewEdit, pText);
    }
}
static void SaveHexFileAction(PBINVIEWWNDEXTRA bvInfo)
{
    HANCHAR pInputName[MAX_PATH] = { 0 };
    OPENFILENAME ofnOpenFile;
    ZeroMemory(&ofnOpenFile, sizeof(ofnOpenFile));
    ofnOpenFile.lStructSize = sizeof(ofnOpenFile);
    ofnOpenFile.hwndOwner = NULL;
    ofnOpenFile.lpstrFilter = TEXT("所有文件\0*.*\0\0");
    ofnOpenFile.nFilterIndex = 0;
    ofnOpenFile.lpstrFile = pInputName;
    ofnOpenFile.nMaxFile = MAX_PATH;
    ofnOpenFile.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
    if (GetSaveFileName(&ofnOpenFile))
    {
        HANDLE hFile = CreateFile(pInputName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
        if (INVALID_HANDLE_VALUE != hFile)
        {
            HANCHAR pText[2][32];

            HANInputGetInputText(bvInfo->hStartAddr, pText[0], ArrLen(pText[0]));
            HANInputGetInputText(bvInfo->hLen, pText[1], ArrLen(pText[1]));
            uint32_t cStartAddr = HAN_strtoul(pText[0], NULL, 16);
            uint32_t nLen = HAN_strtoul(pText[1], NULL, 16);
            if (0 == nLen) { nLen = bvInfo->btBinText.bfBinFile.nBinLen; }
            BinDataToHexFile(hFile, cStartAddr, bvInfo->btBinText.bfBinFile.pBinData, nLen);
            CloseHandle(hFile);
        }
    }
}
static BOOL GetZeroBase(PBINVIEWWNDEXTRA bvInfo)
{
    BOOL bRet = FALSE;

    if (BST_CHECKED == ButtonGetCheck(bvInfo->hZeroBase))
    {
        bRet = TRUE;
    }

    return bRet;
}
static uint32_t GetStartAddr(PBINVIEWWNDEXTRA bvInfo)
{
    HANCHAR pText[32];
    HANInputGetInputText(bvInfo->hStartAddr, pText, ArrLen(pText));
    return HAN_strtoul(pText, NULL, 16);
}
static void UpdateAddrOffset(PBINVIEWWNDEXTRA bvInfo)
{
    PBINFILE pBinFile = &(bvInfo->btBinText.bfBinFile);
    uint32_t nOffset = 0;
    HANPSTR pText = bvInfo->btBinText.pText;
    uint32_t nTextBufSize = bvInfo->btBinText.nTextBufSize;
    uint32_t cBaseAddr = 0;
    HANCHAR pTextAddr[32];

    if (FALSE == GetZeroBase(bvInfo)) { cBaseAddr = GetStartAddr(bvInfo); }
    for (uint32_t i = 0; i < pBinFile->nBinLen; i += 16)
    {
        HAN_snprintf(pTextAddr, nTextBufSize, TEXT("%08X"), i + cBaseAddr);
        memcpy(&pText[nOffset], pTextAddr, sizeof(HANCHAR) * 8);
        nOffset += 60;
    }
    SetWindowText(bvInfo->hBinViewEdit, pText);
}
static void SetDataAction(PBINVIEWWNDEXTRA bvInfo)
{
    HANCHAR pInputName[MAX_PATH] = { 0 };
    OPENFILENAME ofnOpenFile;
    ZeroMemory(&ofnOpenFile, sizeof(ofnOpenFile));
    ofnOpenFile.lStructSize = sizeof(ofnOpenFile);
    ofnOpenFile.hwndOwner = NULL;
    ofnOpenFile.lpstrFilter = TEXT("所有文件\0*.*\0\0");
    ofnOpenFile.nFilterIndex = 0;
    ofnOpenFile.lpstrFile = pInputName;
    ofnOpenFile.nMaxFile = MAX_PATH;
    ofnOpenFile.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
    if (GetOpenFileName(&ofnOpenFile))
    {
        uint8_t* pDestData = bvInfo->btBinText.bfBinFile.pBinData;
        uint32_t nDestLen = bvInfo->btBinText.bfBinFile.nBinLen;
        HANCHAR pText[2][32];

        HANInputGetInputText(bvInfo->hDataPos, pText[0], ArrLen(pText[0]));
        HANInputGetInputText(bvInfo->hDataLen, pText[1], ArrLen(pText[1]));
        uint32_t nOffset = HAN_strtoul(pText[0], NULL, 16);
        uint32_t nLen = HAN_strtoul(pText[1], NULL, 16);
        HANBinFileSetData(pDestData, nDestLen, pInputName, nOffset, nLen);
        PrintBinView(bvInfo);
    }
}
static void SetCrcAction(PBINVIEWWNDEXTRA bvInfo)
{
    uint8_t* pData = bvInfo->btBinText.bfBinFile.pBinData;
    HANCHAR pCrcStart[64];
    HANCHAR pCrcEnd[64];
    HANCHAR pCrcPos[64];
    uint32_t nCrcStart;
    uint32_t nCrcEnd;
    uint32_t nCrcPos;
    uint32_t nLen;
    int nCrcId;

    HANInputGetInputText(bvInfo->hCrcStart, pCrcStart, ArrLen(pCrcStart));
    HANInputGetInputText(bvInfo->hCrcEnd, pCrcEnd, ArrLen(pCrcEnd));
    HANInputGetInputText(bvInfo->hCrcPos, pCrcPos, ArrLen(pCrcPos));
    nCrcId = ComboBoxGetCursel(bvInfo->hCrcList);
    nCrcStart = HAN_strtoul(pCrcStart, NULL, 16);
    nCrcEnd = HAN_strtoul(pCrcEnd, NULL, 16);
    nCrcPos = HAN_strtoul(pCrcPos, NULL, 16);
    nLen = nCrcEnd - nCrcStart + 1;
    HANBinFileSetCrc(pData, nLen, nCrcId, &pData[nCrcPos]);
    PrintBinView(bvInfo);
}
static void InitAddrInput(PBINVIEWWNDEXTRA bvInfo)
{
    HANInputMoveLeftText(bvInfo->hStartAddr, 0, 0, BIN_VIEW_ADDR_STATIC_W, SYS_TEXT_H, TRUE);
    HANInputMoveInput(bvInfo->hStartAddr, BIN_VIEW_ADDR_STATIC_W, 0, SYS_TEXT_W, SYS_TEXT_H, TRUE);
    HANInputMoveRightText(bvInfo->hStartAddr, BIN_VIEW_ADDR_STATIC_W + SYS_TEXT_W, 0, 0, 0, TRUE);
    HANInputSetLeftTextText(bvInfo->hStartAddr, TEXT("起始地址：                  0x"));
    HANInputSetInputText(bvInfo->hStartAddr, DEFAULT_START_ADDR_TEXT);
    HANInputSetLeftTextFont(bvInfo->hStartAddr, bvInfo->hHexFont, TRUE);
    HANInputSetInputFont(bvInfo->hStartAddr, bvInfo->hHexFont, TRUE);
    
    HANInputMoveLeftText(bvInfo->hLen, 0, 0, BIN_VIEW_ADDR_STATIC_W, SYS_TEXT_H, TRUE);
    HANInputMoveInput(bvInfo->hLen, BIN_VIEW_ADDR_STATIC_W, 0, SYS_TEXT_W, SYS_TEXT_H, TRUE);
    HANInputMoveRightText(bvInfo->hLen, BIN_VIEW_ADDR_STATIC_W + SYS_TEXT_W, 0, 0, 0, TRUE);
    HANInputSetLeftTextText(bvInfo->hLen, TEXT("长度（空白表示原文件大小）：0x"));
    HANInputSetLeftTextFont(bvInfo->hLen, bvInfo->hHexFont, TRUE);
    HANInputSetInputFont(bvInfo->hLen, bvInfo->hHexFont, TRUE);

    HANInputMoveLeftText(bvInfo->hDataPos, 0, 0, BIN_VIEW_ADDR_STATIC_W, SYS_TEXT_H, TRUE);
    HANInputMoveInput(bvInfo->hDataPos, BIN_VIEW_ADDR_STATIC_W, 0, SYS_TEXT_W, SYS_TEXT_H, TRUE);
    HANInputMoveRightText(bvInfo->hDataPos, BIN_VIEW_ADDR_STATIC_W + SYS_TEXT_W, 0, 0, 0, TRUE);
    HANInputSetLeftTextText(bvInfo->hDataPos, TEXT("偏移量：                    0x"));
    HANInputSetInputText(bvInfo->hDataPos, DEFAULT_DATA_POS_TEXT);
    HANInputSetLeftTextFont(bvInfo->hDataPos, bvInfo->hHexFont, TRUE);
    HANInputSetInputFont(bvInfo->hDataPos, bvInfo->hHexFont, TRUE);
    
    HANInputMoveLeftText(bvInfo->hDataLen, 0, 0, BIN_VIEW_ADDR_STATIC_W, SYS_TEXT_H, TRUE);
    HANInputMoveInput(bvInfo->hDataLen, BIN_VIEW_ADDR_STATIC_W, 0, SYS_TEXT_W, SYS_TEXT_H, TRUE);
    HANInputMoveRightText(bvInfo->hDataLen, BIN_VIEW_ADDR_STATIC_W + SYS_TEXT_W, 0, 0, 0, TRUE);
    HANInputSetLeftTextText(bvInfo->hDataLen, TEXT("长度（空白表示原文件大小）：0x"));
    HANInputSetLeftTextFont(bvInfo->hDataLen, bvInfo->hHexFont, TRUE);
    HANInputSetInputFont(bvInfo->hDataLen, bvInfo->hHexFont, TRUE);
    
    HANInputMoveLeftText(bvInfo->hCrcStart, 0, 0, BIN_VIEW_CRC_RANGE_START_W, SYS_TEXT_H, TRUE);
    HANInputMoveInput(bvInfo->hCrcStart, BIN_VIEW_CRC_RANGE_START_W, 0, SYS_TEXT_W, SYS_TEXT_H, TRUE);
    HANInputMoveRightText(bvInfo->hCrcStart, BIN_VIEW_CRC_RANGE_START_W + SYS_TEXT_W, 0, 0, 0, TRUE);
    HANInputSetLeftTextText(bvInfo->hCrcStart, TEXT("校验范围：0x"));
    HANInputSetInputText(bvInfo->hCrcStart, DEFAULT_CRC_START_TEXT);
    HANInputSetLeftTextFont(bvInfo->hCrcStart, bvInfo->hHexFont, TRUE);
    HANInputSetInputFont(bvInfo->hCrcStart, bvInfo->hHexFont, TRUE);
    
    HANInputMoveLeftText(bvInfo->hCrcEnd, 0, 0, BIN_VIEW_CRC_RANGE_END_W, SYS_TEXT_H, TRUE);
    HANInputMoveInput(bvInfo->hCrcEnd, BIN_VIEW_CRC_RANGE_END_W, 0, SYS_TEXT_W, SYS_TEXT_H, TRUE);
    HANInputMoveRightText(bvInfo->hCrcEnd, BIN_VIEW_CRC_RANGE_END_W + SYS_TEXT_W, 0, 0, 0, TRUE);
    HANInputSetLeftTextText(bvInfo->hCrcEnd, TEXT(" ~ 0x"));
    HANInputSetInputText(bvInfo->hCrcEnd, DEFAULT_CRC_END_TEXT);
    HANInputSetLeftTextFont(bvInfo->hCrcEnd, bvInfo->hHexFont, TRUE);
    HANInputSetInputFont(bvInfo->hCrcEnd, bvInfo->hHexFont, TRUE);
    
    HANInputMoveLeftText(bvInfo->hCrcPos, 0, 0, BIN_VIEW_CRC_POS_W, SYS_TEXT_H, TRUE);
    HANInputMoveInput(bvInfo->hCrcPos, BIN_VIEW_CRC_POS_W, 0, SYS_TEXT_W, SYS_TEXT_H, TRUE);
    HANInputMoveRightText(bvInfo->hCrcPos, BIN_VIEW_CRC_POS_W + SYS_TEXT_W, 0, 0, 0, TRUE);
    HANInputSetLeftTextText(bvInfo->hCrcPos, TEXT("存储位置：0x"));
    HANInputSetInputText(bvInfo->hCrcPos, DEFAULT_CRC_POS_TEXT);
    HANInputSetLeftTextFont(bvInfo->hCrcPos, bvInfo->hHexFont, TRUE);
    HANInputSetInputFont(bvInfo->hCrcPos, bvInfo->hHexFont, TRUE);
}

static void CfgWindowToTextSaveStartZero(PBINVIEWWNDEXTRA bvInfo, HANPSTR pText)
{
    HANINT bCheck = ButtonGetCheck(bvInfo->hZeroBase);

    if (BST_CHECKED == bCheck) { HAN_snprintf(pText, INI_BIN_VIEW_VALUE_SIZE, TEXT("TRUE")); }
    else { HAN_snprintf(pText, INI_BIN_VIEW_VALUE_SIZE, TEXT("FALSE")); }
}
static void CfgWindowToTextSaveStartAddr(PBINVIEWWNDEXTRA bvInfo, HANPSTR pText)
{
    HANCHAR pTempText[INI_BIN_VIEW_VALUE_SIZE];
    HANSIZE cAddr;

    GetWindowText(bvInfo->hStartAddr, pTempText, INI_BIN_VIEW_VALUE_SIZE);
    cAddr = HAN_strtoul(pTempText, NULL, 16);
    HAN_snprintf(pText, INI_BIN_VIEW_VALUE_SIZE, TEXT(ADDR_PRINT_FORMAT_SINGLE), cAddr);
}
static void CfgWindowToTextSaveLen(PBINVIEWWNDEXTRA bvInfo, HANPSTR pText)
{
    HANSIZE nLen;

    GetWindowText(bvInfo->hLen, pText, INI_BIN_VIEW_VALUE_SIZE);
    if (0 < HAN_strlen(pText))
    {
        nLen = HAN_strtoul(pText, NULL, 16);
        HAN_snprintf(pText, INI_BIN_VIEW_VALUE_SIZE, TEXT(ADDR_PRINT_FORMAT_SINGLE), nLen);
    }
}
static void CfgWindowToTextDataPos(PBINVIEWWNDEXTRA bvInfo, HANPSTR pText)
{
    HANCHAR pTempText[INI_BIN_VIEW_VALUE_SIZE];
    HANSIZE nPos;

    GetWindowText(bvInfo->hDataPos, pTempText, INI_BIN_VIEW_VALUE_SIZE);
    nPos = HAN_strtoul(pTempText, NULL, 16);
    HAN_snprintf(pText, INI_BIN_VIEW_VALUE_SIZE, TEXT(ADDR_PRINT_FORMAT_SINGLE), nPos);
}
static void CfgWindowToTextDataLen(PBINVIEWWNDEXTRA bvInfo, HANPSTR pText)
{
    HANSIZE nLen;

    GetWindowText(bvInfo->hDataLen, pText, INI_BIN_VIEW_VALUE_SIZE);
    if (0 < HAN_strlen(pText))
    {
        nLen = HAN_strtoul(pText, NULL, 16);
        HAN_snprintf(pText, INI_BIN_VIEW_VALUE_SIZE, TEXT(ADDR_PRINT_FORMAT_SINGLE), nLen);
    }
}
static void CfgWindowToTextParityName(PBINVIEWWNDEXTRA bvInfo, HANPSTR pText)
{
    HANINT nId = ComboBoxGetCursel(bvInfo->hCrcList);

    if (CB_ERR == nId) { nId = DEFAULT_CRC_ID; }
    HAN_snprintf(pText, INI_BIN_VIEW_VALUE_SIZE, TEXT("%d"), nId);
}
static void CfgWindowToTextParityStart(PBINVIEWWNDEXTRA bvInfo, HANPSTR pText)
{
    HANCHAR pTempText[INI_BIN_VIEW_VALUE_SIZE];
    HANSIZE cAddr;

    GetWindowText(bvInfo->hCrcStart, pTempText, INI_BIN_VIEW_VALUE_SIZE);
    cAddr = HAN_strtoul(pTempText, NULL, 16);
    HAN_snprintf(pText, INI_BIN_VIEW_VALUE_SIZE, TEXT(ADDR_PRINT_FORMAT_SINGLE), cAddr);
}
static void CfgWindowToTextParityEnd(PBINVIEWWNDEXTRA bvInfo, HANPSTR pText)
{
    HANCHAR pTempText[INI_BIN_VIEW_VALUE_SIZE];
    HANSIZE cAddr;

    GetWindowText(bvInfo->hCrcEnd, pTempText, INI_BIN_VIEW_VALUE_SIZE);
    cAddr = HAN_strtoul(pTempText, NULL, 16);
    HAN_snprintf(pText, INI_BIN_VIEW_VALUE_SIZE, TEXT(ADDR_PRINT_FORMAT_SINGLE), cAddr);
}
static void CfgWindowToTextParityPos(PBINVIEWWNDEXTRA bvInfo, HANPSTR pText)
{
    HANCHAR pTempText[INI_BIN_VIEW_VALUE_SIZE];
    HANSIZE nPos;

    GetWindowText(bvInfo->hCrcPos, pTempText, INI_BIN_VIEW_VALUE_SIZE);
    nPos = HAN_strtoul(pTempText, NULL, 16);
    HAN_snprintf(pText, INI_BIN_VIEW_VALUE_SIZE, TEXT(ADDR_PRINT_FORMAT_SINGLE), nPos);
}

static void CfgTextToWindowSaveStartZero(PBINVIEWWNDEXTRA bvInfo, HANPCSTR pText)
{
    if (0 == HAN_strcmp(pText, TEXT("TRUE"))) { ButtonSetChecked(bvInfo->hZeroBase); }
    else { ButtonSetUnchecked(bvInfo->hZeroBase); }
}
static void CfgTextToWindowSaveStartAddr(PBINVIEWWNDEXTRA bvInfo, HANPCSTR pText)
{
    HANSIZE cAddr = HAN_strtoul(pText, NULL, 16);
    HANCHAR pTempText[ADDR_STR_SIZE];

    HAN_snprintf(pTempText, ADDR_STR_SIZE, TEXT(ADDR_PRINT_FORMAT_FULL), (uint32_t)cAddr);
    pTempText[ADDR_STR_SIZE - 1] = TEXT('\0');
    SetWindowText(bvInfo->hStartAddr, pTempText);
}
static void CfgTextToWindowSaveLen(PBINVIEWWNDEXTRA bvInfo, HANPCSTR pText)
{
    HANSIZE nLen = HAN_strtoul(pText, NULL, 16);
    HANCHAR pTempText[ADDR_STR_SIZE];

    if (0 < HAN_strlen(pText))
    {
        HAN_snprintf(pTempText, ADDR_STR_SIZE, TEXT(ADDR_PRINT_FORMAT_SINGLE), nLen);
        pTempText[ADDR_STR_SIZE - 1] = TEXT('\0');
        SetWindowText(bvInfo->hLen, pTempText);
    }
}
static void CfgTextToWindowDataPos(PBINVIEWWNDEXTRA bvInfo, HANPCSTR pText)
{
    HANSIZE nPos = HAN_strtoul(pText, NULL, 16);
    HANCHAR pTempText[ADDR_STR_SIZE];

    HAN_snprintf(pTempText, ADDR_STR_SIZE, TEXT(ADDR_PRINT_FORMAT_FULL), (uint32_t)nPos);
    pTempText[ADDR_STR_SIZE - 1] = TEXT('\0');
    SetWindowText(bvInfo->hDataPos, pTempText);
}
static void CfgTextToWindowDataLen(PBINVIEWWNDEXTRA bvInfo, HANPCSTR pText)
{
    HANSIZE nLen = HAN_strtoul(pText, NULL, 16);
    HANCHAR pTempText[ADDR_STR_SIZE];

    if (0 < HAN_strlen(pText))
    {
        HAN_snprintf(pTempText, ADDR_STR_SIZE, TEXT(ADDR_PRINT_FORMAT_SINGLE), nLen);
        pTempText[ADDR_STR_SIZE - 1] = TEXT('\0');
        SetWindowText(bvInfo->hDataLen, pTempText);
    }
}
static void CfgTextToWindowParityName(PBINVIEWWNDEXTRA bvInfo, HANPCSTR pText)
{
    HANINT nId = HAN_strtol(pText, NULL, 10);
    
    if (nId < 0) { nId = 0; }
    if (CRCID_CNT_MAX < nId) { nId = CRCID_CNT_MAX - 1; }
    ComboBoxSetCursel(bvInfo->hCrcList, nId);
}
static void CfgTextToWindowParityStart(PBINVIEWWNDEXTRA bvInfo, HANPCSTR pText)
{
    HANSIZE cAddr = HAN_strtoul(pText, NULL, 16);
    HANCHAR pTempText[ADDR_STR_SIZE];

    HAN_snprintf(pTempText, ADDR_STR_SIZE, TEXT(ADDR_PRINT_FORMAT_SINGLE), cAddr);
    pTempText[ADDR_STR_SIZE - 1] = TEXT('\0');
    SetWindowText(bvInfo->hCrcStart, pTempText);
}
static void CfgTextToWindowParityEnd(PBINVIEWWNDEXTRA bvInfo, HANPCSTR pText)
{
    HANSIZE cAddr = HAN_strtoul(pText, NULL, 16);
    HANCHAR pTempText[ADDR_STR_SIZE];

    HAN_snprintf(pTempText, ADDR_STR_SIZE, TEXT(ADDR_PRINT_FORMAT_SINGLE), cAddr);
    pTempText[ADDR_STR_SIZE - 1] = TEXT('\0');
    SetWindowText(bvInfo->hCrcEnd, pTempText);
}
static void CfgTextToWindowParityPos(PBINVIEWWNDEXTRA bvInfo, HANPCSTR pText)
{
    HANSIZE nPos = HAN_strtoul(pText, NULL, 16);
    HANCHAR pTempText[ADDR_STR_SIZE];

    HAN_snprintf(pTempText, ADDR_STR_SIZE, TEXT(ADDR_PRINT_FORMAT_SINGLE), nPos);
    pTempText[ADDR_STR_SIZE - 1] = TEXT('\0');
    SetWindowText(bvInfo->hCrcPos, pTempText);
}
