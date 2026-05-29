#include <windows.h>
#include <shlwapi.h>

#include "HAN_ComToolConsole.h"
#include "..\GlobalVariables\HAN_ComToolGlobalVariables.h"
#include "..\Graphic\HAN_ComToolGraphic.h"
#include "..\..\..\..\HAN_Lib\HAN_wingdi.h"

#define HAN_COM_TOOL_CONSOLE_CLASS       TEXT("ComToolConsole")

static LRESULT CALLBACK ComToolConsoleWndProc(HWND hComToolConsole, UINT message, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK ComToolUserBaudrateDialogProc(HWND hUserBaudrate, UINT message, WPARAM wParam, LPARAM lParam);

static LRESULT CreateCallback(HWND hComToolConsole, LPARAM lParam);
static void CommandCallback(HWND hComToolConsole, PCOMTOOLEXTRA ctInfo, WPARAM wParam, LPARAM lParam);
static HANSIZE OpenComCallback(HWND hStateButton, HWND hChildButton, void* pParam);
static HANSIZE CloseComCallback(HWND hStateButton, HWND hChildButton, void* pParam);
static void ClearWindowCallback(PCOMTOOLEXTRA ctInfo);
static void UpdateComCfg(PCOMTOOLEXTRA ctInfo, COMTOOLUPDATECOMCFG cfgMask);
static void UpdatePortList(PCOMTOOLEXTRA ctInfo, uint32_t nTargetId);
static void SweepCOMAction(const LPBYTE nCOMId, HANPCSTR pCOMName, DWORD nCount, void* pParam);
static void ReadComGetByteSuccessAction(PCOMTOOLEXTRA ctInfo, HANSIZE nLen);
static void ReadComAppendInfo(PCOMTOOLEXTRA ctInfo, COMTOOLDATATYPE cType);
static void SwapComBuf(PCOMTOOLEXTRA ctInfo);
static void PrintStartInfo(PCOMTOOLEXTRA ctInfo, PSYSTEMTIME pTime);
static void PrintEndInfo(PCOMTOOLEXTRA ctInfo, PSYSTEMTIME pTime);
static void PrintDataASCII(PCOMTOOLEXTRA ctInfo, const uint8_t* pBuf, COMTOOLSIZE nLen);
static void PrintDataHex(PCOMTOOLEXTRA ctInfo, const uint8_t* pBuf, COMTOOLSIZE nLen);
static void PrintSumData(PCOMTOOLEXTRA ctInfo);
static void BaudrateAction(PCOMTOOLEXTRA ctInfo);
static void ByteSizeAction(PCOMTOOLEXTRA ctInfo);
static void StopBitsAction(PCOMTOOLEXTRA ctInfo);
static void ParityAction(PCOMTOOLEXTRA ctInfo);
static void HexButtonAction(PCOMTOOLEXTRA ctInfo);
static void PrintStartTimeButtonAction(PCOMTOOLEXTRA ctInfo);
static void PrintEndTimeButtonAction(PCOMTOOLEXTRA ctInfo);
static void SaveToFileAction(PCOMTOOLEXTRA ctInfo);
static HANPCSTR GetDefaultFileName(HANPSTR pPath, DWORD nSize);
static void TextWindowAppendText(PCOMTOOLEXTRA ctInfo, HANPCSTR pText, int nTextLen);
static DWORD CreateBaudrateDialog(PCOMTOOLEXTRA ctInfo);
static void BaudrateDialogInitAction(HWND hUserBaudrate, LPARAM lParam);
static void BaudrateDialogOkAction(PCOMTOOLEXTRA ctInfo, HWND hUserBaudrate);

static void ReadComThread(PCOMTOOLEXTRA ctInfo);
static DWORD PrintComThread(PCOMTOOLEXTRA ctInfo);
static void PrintComThreadSubDeal(PCOMTOOLEXTRA ctInfo);

static void CfgWindowToTextComId(PCOMTOOLEXTRA ctInfo, HANPSTR pText);
static void CfgWindowToTextBaudrate(PCOMTOOLEXTRA ctInfo, HANPSTR pText);
static void CfgWindowToTextUserBaudrate(PCOMTOOLEXTRA ctInfo, HANPSTR pText);
static void CfgWindowToTextByteSize(PCOMTOOLEXTRA ctInfo, HANPSTR pText);
static void CfgWindowToTextStopBits(PCOMTOOLEXTRA ctInfo, HANPSTR pText);
static void CfgWindowToTextParity(PCOMTOOLEXTRA ctInfo, HANPSTR pText);
static void CfgWindowToTextHex(PCOMTOOLEXTRA ctInfo, HANPSTR pText);
static void CfgWindowToTextPrintStartTime(PCOMTOOLEXTRA ctInfo, HANPSTR pText);
static void CfgWindowToTextPrintEndTime(PCOMTOOLEXTRA ctInfo, HANPSTR pText);

static void CfgTextToWindowComId(PCOMTOOLEXTRA ctInfo, HANPCSTR pText);
static void CfgTextToWindowBaudrate(PCOMTOOLEXTRA ctInfo, HANPCSTR pText);
static void CfgTextToWindowUserBaudrate(PCOMTOOLEXTRA ctInfo, HANPCSTR pText);
static void CfgTextToWindowByteSize(PCOMTOOLEXTRA ctInfo, HANPCSTR pText);
static void CfgTextToWindowStopBits(PCOMTOOLEXTRA ctInfo, HANPCSTR pText);
static void CfgTextToWindowParity(PCOMTOOLEXTRA ctInfo, HANPCSTR pText);
static void CfgTextToWindowHex(PCOMTOOLEXTRA ctInfo, HANPCSTR pText);
static void CfgTextToWindowPrintStartTime(PCOMTOOLEXTRA ctInfo, HANPCSTR pText);
static void CfgTextToWindowPrintEndTime(PCOMTOOLEXTRA ctInfo, HANPCSTR pText);

static const COMTOOLREADWRITECFG sg_pComToolCfgInfo[INI_COM_TOOL_CFG_CNT] = {
    [INI_COM_TOOL_COM_ID] = {
        .pKey = TEXT("ComId"),
        .pDefValue = TEXT("1"),
        .CfgWindowToText = CfgWindowToTextComId,
        .CfgTextToWindow = CfgTextToWindowComId,
    },
    [INI_COM_TOOL_BAUDRATE] = {
        .pKey = TEXT("Baudrate"),
        .pDefValue = TEXT("115200"),
        .CfgWindowToText = CfgWindowToTextBaudrate,
        .CfgTextToWindow = CfgTextToWindowBaudrate,
    },
    [INI_COM_TOOL_USER_BAUDRATE] = {
        .pKey = TEXT("UserBaudrate"),
        .pDefValue = TEXT("115200"),
        .CfgWindowToText = CfgWindowToTextUserBaudrate,
        .CfgTextToWindow = CfgTextToWindowUserBaudrate,
    },
    [INI_COM_TOOL_BYTE_SIZE] = {
        .pKey = TEXT("ByteSize"),
        .pDefValue = TEXT("8"),
        .CfgWindowToText = CfgWindowToTextByteSize,
        .CfgTextToWindow = CfgTextToWindowByteSize,
    },
    [INI_COM_TOOL_STOP_BITS] = {
        .pKey = TEXT("StopBits"),
        .pDefValue = TEXT("1"),
        .CfgWindowToText = CfgWindowToTextStopBits,
        .CfgTextToWindow = CfgTextToWindowStopBits,
    },
    [INI_COM_TOOL_PARITY] = {
        .pKey = TEXT("Parity"),
        .pDefValue = TEXT("NONE"),
        .CfgWindowToText = CfgWindowToTextParity,
        .CfgTextToWindow = CfgTextToWindowParity,
    },
    [INI_COM_TOOL_HEX] = {
        .pKey = TEXT("Hex"),
        .pDefValue = TEXT("FALSE"),
        .CfgWindowToText = CfgWindowToTextHex,
        .CfgTextToWindow = CfgTextToWindowHex,
    },
    [INI_COM_TOOL_PRINT_START_TIME] = {
        .pKey = TEXT("PrintStartTime"),
        .pDefValue = TEXT("FALSE"),
        .CfgWindowToText = CfgWindowToTextPrintStartTime,
        .CfgTextToWindow = CfgTextToWindowPrintStartTime,
    },
    [INI_COM_TOOL_PRINT_END_TIME] = {
        .pKey = TEXT("PrintEndTime"),
        .pDefValue = TEXT("FALSE"),
        .CfgWindowToText = CfgWindowToTextPrintEndTime,
        .CfgTextToWindow = CfgTextToWindowPrintEndTime,
    },
};

void RegisterHANComToolConsole(HINSTANCE hInst)
{
    WNDCLASSEX wcex = {
        .cbSize         = sizeof(WNDCLASSEX),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = ComToolConsoleWndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = sizeof(PCOMTOOLEXTRA),
        .hInstance      = hInst,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName   = NULL,
        .lpszClassName  = HAN_COM_TOOL_CONSOLE_CLASS,
        .hIconSm        = NULL,
    };
    RegisterClassEx(&wcex);
}

HWND CreateComToolConsoleWindow(HANINT x, HANINT y, HWND hComTool, HMENU winId, HINSTANCE hInst, PCOMTOOLEXTRA ctInfo)
{
    return CreateWindow(HAN_COM_TOOL_CONSOLE_CLASS, NULL,
        WS_CHILD | WS_VISIBLE | WS_BORDER, x, y, COMTOOL_CONSOLE_WINDOW_W, COMTOOL_CONSOLE_WINDOW_H,
        hComTool, winId, hInst, ctInfo
    );
}

void ReadComToolConsoleIniFile(HANPCSTR pIniPath, void* pParam)
{
    PCOMTOOLCFG pCtConfig = pParam;
    INICOMTOOLCFGID iLoop;

    for (iLoop = 0; iLoop < INI_COM_TOOL_CFG_CNT; iLoop++)
    {
        HAN_strcpy(pCtConfig->pSysConfig[iLoop].pKey, sg_pComToolCfgInfo[iLoop].pKey);
        HAN_strcpy(pCtConfig->pSysConfig[iLoop].pDefValue, sg_pComToolCfgInfo[iLoop].pDefValue);
        GetPrivateProfileString(
            INI_COM_TOOL_APP_NAME, pCtConfig->pSysConfig[iLoop].pKey, pCtConfig->pSysConfig[iLoop].pDefValue,
            pCtConfig->pSysConfig[iLoop].pValue, INI_COM_TOOL_VALUE_SIZE, pIniPath);
    }
}

void WriteComToolConsoleIniFile(HANPCSTR pIniPath, HWND hComTool)
{
    PCOMTOOLEXTRA ctInfo = (PCOMTOOLEXTRA)GetWindowLongPtr(hComTool, 0);
    HANCHAR pBvCfg[INI_COM_TOOL_CFG_CNT][INI_COM_TOOL_VALUE_SIZE];
    INICOMTOOLCFGID iLoop;

    UpdateComCfg(ctInfo, COMTOOL_UPDATE_COM_CFG_ALL);
    
    for (iLoop = 0; iLoop < INI_COM_TOOL_CFG_CNT; iLoop++)
    {
        sg_pComToolCfgInfo[iLoop].CfgWindowToText(ctInfo, pBvCfg[iLoop]);
        WritePrivateProfileString(
            INI_COM_TOOL_APP_NAME, sg_pComToolCfgInfo[iLoop].pKey, pBvCfg[iLoop], pIniPath
        );
    }
}

void LoadComToolCfg(PCOMTOOLEXTRA ctInfo, PCOMTOOLCFG pCtConfig)
{
    INICOMTOOLCFGID iLoop;

    for (iLoop = 0; iLoop < INI_COM_TOOL_CFG_CNT; iLoop++)
    {
        sg_pComToolCfgInfo[iLoop].CfgTextToWindow(ctInfo, pCtConfig->pSysConfig[iLoop].pValue);
    }
    UpdateComCfg(ctInfo, COMTOOL_UPDATE_COM_CFG_ALL);
}

static LRESULT CALLBACK ComToolConsoleWndProc(HWND hComToolConsole, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    PCOMTOOLEXTRA ctInfo = (PCOMTOOLEXTRA)GetWindowLongPtr(hComToolConsole, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = CreateCallback(hComToolConsole, lParam);
        } break;
        case WM_COMMAND: {
            CommandCallback(hComToolConsole, ctInfo, wParam, lParam);
        } break;
        case WM_TIMER: {
            ReadComThread(ctInfo);
        } break;
        case WM_CTLCOLORSTATIC: {
            lWndProcRet = (INT_PTR)GetStockObject(WHITE_BRUSH);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hComToolConsole, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}
static INT_PTR CALLBACK ComToolUserBaudrateDialogProc(HWND hUserBaudrate, UINT message, WPARAM wParam, LPARAM lParam)
{
    INT_PTR nRet = TRUE;
    
    // 读取属性
    PCOMTOOLEXTRA ctInfo = (PCOMTOOLEXTRA)GetWindowLongPtr(hUserBaudrate, DWLP_USER);

    switch (message) {
        case WM_INITDIALOG: {
            BaudrateDialogInitAction(hUserBaudrate, lParam);
            break;
        }
        case WM_COMMAND: {
            switch(LOWORD(wParam)) {
                case WID_COM_TOOL_CONSOLE_USER_BAUDRATE_OK: {
                    BaudrateDialogOkAction(ctInfo, hUserBaudrate);
                    break;
                }

                default: {
                    break;
                }
            }
            break;
        }
        case WM_CLOSE: {
            EndDialog(hUserBaudrate, 0);
            break;
        }
        case WM_DESTROY: {
            EndDialog(hUserBaudrate, 0);
            break;
        }

        default: {
            nRet = FALSE;
            break;
        }
    }

    return nRet;
}

static LRESULT CreateCallback(HWND hComToolConsole, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PCOMTOOLEXTRA ctInfo = ((LPCREATESTRUCT)lParam)->lpCreateParams;
    RECT rcClientSize;

    int nWinX = COMTOOL_WINDOW_DX;
    int nWinY = COMTOOL_WINDOW_DY;

    SetWindowLongPtr(hComToolConsole, 0, (LONG_PTR)ctInfo);

    GetClientRect(hComToolConsole, &rcClientSize);

    RegisterHANStateButton(ctInfo->hInst);

    ctInfo->sumData.sumRx.hWnd = CreateWindow(TEXT("static"), COMTOOL_SUM_RX_TITLE TEXT("0"),
        WS_CHILD | WS_VISIBLE, nWinX, nWinY, 200, 18,
        hComToolConsole, (HMENU)WID_COM_TOOL_CONSOLE_SUM_RX, ctInfo->hInst, NULL);
    nWinX += 200 + COMTOOL_WINDOW_DX;
    ctInfo->sumData.sumTx.hWnd = CreateWindow(TEXT("static"), COMTOOL_SUM_TX_TITLE TEXT("0"),
        WS_CHILD | WS_VISIBLE, nWinX, nWinY, 200, 18,
        hComToolConsole, (HMENU)WID_COM_TOOL_CONSOLE_SUM_TX, ctInfo->hInst, NULL);

    nWinX = COMTOOL_WINDOW_DX;
    nWinY += 18 + COMTOOL_WINDOW_DY;
    ctInfo->hOpenCom = CreateWindow(HSTBT_CLASS, NULL,
        WS_CHILD | WS_VISIBLE, nWinX, nWinY, 100, 30,
        hComToolConsole, (HMENU)WID_COM_TOOL_CONSOLE_OPEN_COM, ctInfo->hInst, ctInfo);
    nWinX += 100 + COMTOOL_WINDOW_DX;
    ctInfo->printData.printData.hClear = CreateWindow(TEXT("button"), TEXT("清除窗口"),
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, nWinX, nWinY, 100, 30,
        hComToolConsole, (HMENU)WID_COM_TOOL_CONSOLE_CLEAR, ctInfo->hInst, ctInfo);

    nWinX += 100 + COMTOOL_WINDOW_DX;
    nWinY += 4;
    ctInfo->cfgCOM.hComId = CreateWindow(TEXT("combobox"), NULL,
        COMTOOL_COMBOBOX_STYLE, nWinX, nWinY, 200, 100,
        hComToolConsole, (HMENU)WID_COM_TOOL_CONSOLE_COM_ID, ctInfo->hInst, NULL);

    nWinX = COMTOOL_WINDOW_DX;
    nWinY += 26 + COMTOOL_WINDOW_DY;
    ctInfo->cfgCOM.hBaudrate = CreateWindow(TEXT("combobox"), NULL,
        COMTOOL_COMBOBOX_STYLE, nWinX, nWinY, 180, 200,
        hComToolConsole, (HMENU)WID_COM_TOOL_CONSOLE_BAUDRATE, ctInfo->hInst, NULL);
    nWinX += 180 + COMTOOL_WINDOW_DX;
    ctInfo->cfgCOM.hByteSize = CreateWindow(TEXT("combobox"), NULL,
        COMTOOL_COMBOBOX_STYLE, nWinX, nWinY, 90, 200,
        hComToolConsole, (HMENU)WID_COM_TOOL_CONSOLE_BYTESIZE, ctInfo->hInst, NULL);
    nWinX += 90 + COMTOOL_WINDOW_DX;
    ctInfo->cfgCOM.hStopBits = CreateWindow(TEXT("combobox"), NULL,
        COMTOOL_COMBOBOX_STYLE, nWinX, nWinY, 90, 200,
        hComToolConsole, (HMENU)WID_COM_TOOL_CONSOLE_STOPBITS, ctInfo->hInst, NULL);
    nWinX += 90 + COMTOOL_WINDOW_DX;
    ctInfo->cfgCOM.hParity = CreateWindow(TEXT("combobox"), NULL,
        COMTOOL_COMBOBOX_STYLE, nWinX, nWinY, 90, 200,
        hComToolConsole, (HMENU)WID_COM_TOOL_CONSOLE_PARITY, ctInfo->hInst, NULL);

    nWinX = COMTOOL_WINDOW_DX;
    nWinY += 26 + COMTOOL_WINDOW_DY;
    ctInfo->printHex.hHex = CreateWindow(TEXT("button"), TEXT("HEX"),
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, nWinX, nWinY, 55, 26,
        hComToolConsole, (HMENU)WID_COM_TOOL_CONSOLE_HEX, ctInfo->hInst, NULL);
    nWinX += 55 + COMTOOL_WINDOW_DX;
    ctInfo->printData.printStartTime.hStart = CreateWindow(TEXT("button"), TEXT("打印开始时间"),
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, nWinX, nWinY, 140, 26,
        hComToolConsole, (HMENU)WID_COM_TOOL_CONSOLE_PRINT_START_TIME, ctInfo->hInst, NULL);
    nWinX += 140 + COMTOOL_WINDOW_DX;
    ctInfo->printData.printEndTime.hEnd = CreateWindow(TEXT("button"), TEXT("打印结束时间"),
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, nWinX, nWinY, 140, 26,
        hComToolConsole, (HMENU)WID_COM_TOOL_CONSOLE_PRINT_END_TIME, ctInfo->hInst, NULL);

    nWinX = COMTOOL_WINDOW_DX;
    nWinY += 26 + COMTOOL_WINDOW_DY;
    ctInfo->saveToFile.hPath = CreateWindow(TEXT("edit"), NULL,
        WS_CHILD | WS_VISIBLE | WS_BORDER, nWinX, nWinY, TOOL_BOX_PATH_INPUT_EDIT_W, TOOL_BOX_TEXT_INPUT_EDIT_H,
        hComToolConsole, (HMENU)WID_COM_TOOL_CONSOLE_SAVE_FILE_PATH, ctInfo->hInst, NULL);
    nWinX += TOOL_BOX_PATH_INPUT_EDIT_W + COMTOOL_WINDOW_DX;
    ctInfo->saveToFile.hSave = CreateWindow(TEXT("button"), TEXT("保存数据到文件"),
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, nWinX, nWinY, 150, TOOL_BOX_TEXT_INPUT_EDIT_H,
        hComToolConsole, (HMENU)WID_COM_TOOL_CONSOLE_SAVE_FILE_SAVE, ctInfo->hInst, NULL);

    SendMessage(ctInfo->printData.printData.hClear, WM_SETFONT, (WPARAM)(ctInfo->hFont.hSys), (LPARAM)TRUE);
    SendMessage(ctInfo->sumData.sumRx.hWnd, WM_SETFONT, (WPARAM)(ctInfo->hFont.hHex), (LPARAM)TRUE);
    SendMessage(ctInfo->sumData.sumTx.hWnd, WM_SETFONT, (WPARAM)(ctInfo->hFont.hHex), (LPARAM)TRUE);
    SendMessage(ctInfo->cfgCOM.hComId, WM_SETFONT, (WPARAM)(ctInfo->hFont.hHex), (LPARAM)TRUE);
    SendMessage(ctInfo->hOpenCom, WM_SETFONT, (WPARAM)(ctInfo->hFont.hSys), (LPARAM)TRUE);
    SendMessage(ctInfo->cfgCOM.hBaudrate, WM_SETFONT, (WPARAM)(ctInfo->hFont.hHex), (LPARAM)TRUE);
    SendMessage(ctInfo->cfgCOM.hByteSize, WM_SETFONT, (WPARAM)(ctInfo->hFont.hHex), (LPARAM)TRUE);
    SendMessage(ctInfo->cfgCOM.hStopBits, WM_SETFONT, (WPARAM)(ctInfo->hFont.hHex), (LPARAM)TRUE);
    SendMessage(ctInfo->cfgCOM.hParity, WM_SETFONT, (WPARAM)(ctInfo->hFont.hHex), (LPARAM)TRUE);
    SendMessage(ctInfo->printHex.hHex, WM_SETFONT, (WPARAM)(ctInfo->hFont.hSys), (LPARAM)TRUE);
    SendMessage(ctInfo->printData.printStartTime.hStart, WM_SETFONT, (WPARAM)(ctInfo->hFont.hSys), (LPARAM)TRUE);
    SendMessage(ctInfo->printData.printEndTime.hEnd, WM_SETFONT, (WPARAM)(ctInfo->hFont.hSys), (LPARAM)TRUE);
    SendMessage(ctInfo->saveToFile.hPath, WM_SETFONT, (WPARAM)(ctInfo->hFont.hSys), (LPARAM)TRUE);
    SendMessage(ctInfo->saveToFile.hSave, WM_SETFONT, (WPARAM)(ctInfo->hFont.hSys), (LPARAM)TRUE);

    HSTBTSTATE buttonState[2] = {
        { .pText = TEXT("打开串口"), .Callback = (HSTBTCALLBACK)OpenComCallback, },
        { .pText = TEXT("关闭串口"), .Callback = (HSTBTCALLBACK)CloseComCallback, },
    };
    HANStateButtonAddState(ctInfo->hOpenCom, &buttonState[0]);
    HANStateButtonAddState(ctInfo->hOpenCom, &buttonState[1]);
    ComboBoxAddStringArr(ctInfo->cfgCOM.hBaudrate, g_pBaudrate, ArrLen(g_pBaudrate));
    ComboBoxAddStringArr(ctInfo->cfgCOM.hByteSize, g_pByteSize, ArrLen(g_pByteSize));
    ComboBoxAddStringStructArr(ctInfo->cfgCOM.hStopBits, &g_pStopBits, sizeof(COMSBPAPARAM), StructMemberOffset(COMSBPAPARAM, pText), ArrLen(g_pStopBits));
    ComboBoxAddStringStructArr(ctInfo->cfgCOM.hParity, &g_pParity, sizeof(COMSBPAPARAM), StructMemberOffset(COMSBPAPARAM, pText), ArrLen(g_pParity));
    if (TRUE == ctInfo->printHex.bHex) { ButtonSetChecked(ctInfo->printHex.hHex); }
    if (TRUE == ctInfo->printData.printStartTime.bStart) { ButtonSetChecked(ctInfo->printData.printStartTime.hStart); }
    if (TRUE == ctInfo->printData.printEndTime.bEnd) { ButtonSetChecked(ctInfo->printData.printEndTime.hEnd); }
    ctInfo->hConsole = hComToolConsole;

    LoadComToolCfg(ctInfo, &(ctInfo->ctConfig));

    return lWndProcRet;
}
static void CommandCallback(HWND hComToolConsole, PCOMTOOLEXTRA ctInfo, WPARAM wParam, LPARAM lParam)
{
    (void)hComToolConsole;
    (void)lParam;
    
    switch (LOWORD(wParam)) {
        case WID_COM_TOOL_CONSOLE_CLEAR: {
            ClearWindowCallback(ctInfo);
        } break;
        case WID_COM_TOOL_CONSOLE_BAUDRATE: {
            if (CBN_SELCHANGE == HIWORD(wParam)) { BaudrateAction(ctInfo); }
        } break;
        case WID_COM_TOOL_CONSOLE_BYTESIZE: {
            if (CBN_SELCHANGE == HIWORD(wParam)) { ByteSizeAction(ctInfo); }
        } break;
        case WID_COM_TOOL_CONSOLE_STOPBITS: {
            if (CBN_SELCHANGE == HIWORD(wParam)) { StopBitsAction(ctInfo); }
        } break;
        case WID_COM_TOOL_CONSOLE_PARITY: {
            if (CBN_SELCHANGE == HIWORD(wParam)) { ParityAction(ctInfo); }
        } break;
        case WID_COM_TOOL_CONSOLE_HEX: {
            HexButtonAction(ctInfo);
        } break;
        case WID_COM_TOOL_CONSOLE_PRINT_START_TIME: {
            PrintStartTimeButtonAction(ctInfo);
        } break;
        case WID_COM_TOOL_CONSOLE_PRINT_END_TIME: {
            PrintEndTimeButtonAction(ctInfo);
        } break;
        case WID_COM_TOOL_CONSOLE_SAVE_FILE_SAVE: {
            SaveToFileAction(ctInfo);
        } break;

        default: { } break;
    }
}
static HANSIZE OpenComCallback(HWND hStateButton, HWND hChildButton, void* pParam)
{
    (void)hStateButton;
    (void)hChildButton;

    HANSIZE nRet = 0;
    PCOMTOOLEXTRA ctInfo = pParam;
    HANERROR eComRet;

    UpdateComCfg(ctInfo, COMTOOL_UPDATE_COM_CFG_ALL);
    
    eComRet = OpenCOM(&(ctInfo->hComFile), &(ctInfo->cfgCOM.cfgCOM));
    switch (eComRet) {
        case RET_OK: {
            ctInfo->revData.nIndex = 0;
            (void)memset(&(ctInfo->revData), 0, sizeof(ctInfo->revData));
            ctInfo->printData.printData.hPrintComSemaphore = CreateSemaphore(NULL, 0, 1, NULL);
            ctInfo->revData.cStatus = COMTOOL_READ_STATUS_WAITING;
            SetTimer(ctInfo->hConsole, 0, COMTOOL_READ_COM_PERIOD, NULL);
            ctInfo->printData.printData.hPrintComThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PrintComThread, (void*)ctInfo, 0, NULL);
            nRet = 1;
        } break;

        default: {
            MessageBox(NULL, TEXT("打开串口失败，请检查是否被占用"), TEXT("错误"), 0);
        } break;
    }

    return nRet;
}
static HANSIZE CloseComCallback(HWND hStateButton, HWND hChildButton, void* pParam)
{
    (void)hStateButton;
    (void)hChildButton;

    PCOMTOOLEXTRA ctInfo = pParam;

    if (INVALID_HANDLE_VALUE != ctInfo->hComFile)
    {
        CloseHandle(ctInfo->hComFile);
        KillTimer(ctInfo->hConsole, 0);
        CloseHandle(ctInfo->printData.printData.hPrintComThread);
        CloseHandle(ctInfo->printData.printData.hPrintComSemaphore);
        ctInfo->hComFile = INVALID_HANDLE_VALUE;
    }

    return 1;
}
static void ClearWindowCallback(PCOMTOOLEXTRA ctInfo)
{
    SetWindowText(ctInfo->printData.printData.hText, TEXT(""));
    SetWindowText(ctInfo->sumData.sumRx.hWnd, COMTOOL_SUM_RX_TITLE TEXT("0"));
    ctInfo->sumData.sumRx.nSum = 0;
}
static void UpdateComCfg(PCOMTOOLEXTRA ctInfo, COMTOOLUPDATECOMCFG cfgMask)
{
    int iCursel;
    HANCHAR pText[PATH_STR_SIZE];

    /* 更新端口号 */
    if (0 != (cfgMask & COMTOOL_UPDATE_COM_CFG_PORT))
    {
        GetWindowText(ctInfo->cfgCOM.hComId, pText, PATH_STR_SIZE);
        if (0 == memcmp(pText, "COM", 3 * sizeof(HANCHAR)))
        {
            ctInfo->cfgCOM.cfgCOM.comFile.id = (uint16_t)HAN_strtoul(&pText[3], NULL, 10);
        }
        if (0 == ctInfo->cfgCOM.cfgCOM.comFile.id) { ctInfo->cfgCOM.cfgCOM.comFile.id = 1; }
    }
    /* 更新波特率 */
    if (0 != (cfgMask & COMTOOL_UPDATE_COM_CFG_BAUDRATE))
    {
        iCursel = ComboBoxGetCursel(ctInfo->cfgCOM.hBaudrate);
        if (0 == iCursel)
        {
            DWORD nBaudrate = CreateBaudrateDialog(ctInfo);
            if (0 != nBaudrate)
            {
                ctInfo->cfgCOM.cfgCOM.comState.BaudRate = nBaudrate;
                HAN_snprintf(pText, PATH_STR_SIZE, TEXT("自定义：%lu"), nBaudrate);
                SendMessage(ctInfo->cfgCOM.hBaudrate, CB_DELETESTRING, 0, 0);
                SendMessage(ctInfo->cfgCOM.hBaudrate, CB_INSERTSTRING, 0, (LPARAM)pText);
                ComboBoxSetCursel(ctInfo->cfgCOM.hBaudrate, 0);
            }
            else
            {
                ComboBoxSetCursel(ctInfo->cfgCOM.hBaudrate, ctInfo->nBaudrateListCurselId);
            }
        }
        else
        {
            ctInfo->cfgCOM.cfgCOM.comState.BaudRate = HAN_strtoul(g_pBaudrate[iCursel], NULL, 10);
        }
        ctInfo->nBaudrateListCurselId = ComboBoxGetCursel(ctInfo->cfgCOM.hBaudrate);
    }
    /* 更新字节大小 */
    if (0 != (cfgMask & COMTOOL_UPDATE_COM_CFG_BYTESIZE))
    {
        iCursel = ComboBoxGetCursel(ctInfo->cfgCOM.hByteSize);
        ctInfo->cfgCOM.cfgCOM.comState.ByteSize = (BYTE)HAN_strtoul(g_pByteSize[iCursel], NULL, 10);
    }
    /* 更新停止位 */
    if (0 != (cfgMask & COMTOOL_UPDATE_COM_CFG_STOPBITS))
    {
        iCursel = ComboBoxGetCursel(ctInfo->cfgCOM.hStopBits);
        ctInfo->cfgCOM.cfgCOM.comState.StopBits = g_pStopBits[iCursel].value;
    }
    /* 更新校验位 */
    if (0 != (cfgMask & COMTOOL_UPDATE_COM_CFG_PARITY))
    {
        iCursel = ComboBoxGetCursel(ctInfo->cfgCOM.hParity);
        ctInfo->cfgCOM.cfgCOM.comState.Parity = g_pParity[iCursel].value;
    }
}
static void UpdatePortList(PCOMTOOLEXTRA ctInfo, uint32_t nTargetId)
{
    HANCHAR pPortId[PATH_STR_SIZE];

    while (0 < SendMessage(ctInfo->cfgCOM.hComId, CB_DELETESTRING, 0, 0)) { }
    SweepCOMFromRegedit((void (*)(const LPBYTE, HANPCSTR, DWORD, void*))SweepCOMAction, ctInfo);

    if (0 < nTargetId)
    {
        HAN_snprintf(pPortId, PATH_STR_SIZE, TEXT("COM%d"), nTargetId);
        HANINT nBoxId = (HANINT)SendMessage(ctInfo->cfgCOM.hComId, CB_FINDSTRING, -1, (LPARAM)pPortId);
        if (nBoxId != CB_ERR)
        {
            (void)ComboBoxSetCursel(ctInfo->cfgCOM.hComId, nBoxId);
        }
        else
        {
            (void)ComboBoxSetCursel(ctInfo->cfgCOM.hComId, 0);
        }
    }
}
static void SweepCOMAction(const LPBYTE nCOMId, HANPCSTR pCOMName, DWORD nCount, void* pParam)
{
    (void)nCount;
    PCOMTOOLEXTRA ctInfo = pParam;
    HANCHAR pPortIdText[PATH_STR_SIZE];
    HANPCSTR pCOMNameId = pCOMName;
    HANPCSTR pDevice = HAN_strstr(pCOMName, TEXT("\\Device\\"));

    if (NULL != pDevice)
    {
        if (pCOMName == pDevice) { pCOMNameId = &pCOMName[HAN_strlen(TEXT("\\Device\\"))]; }
        (void)HAN_snprintf(pPortIdText, PATH_STR_SIZE, TEXT("%s: %s"), (HANPCSTR)nCOMId, pCOMNameId);
        (void)ComboBoxAddString(ctInfo->cfgCOM.hComId, pPortIdText);
    }
}
static void ReadComGetByteSuccessAction(PCOMTOOLEXTRA ctInfo, HANSIZE nLen)
{
    uint8_t nIndex = ctInfo->revData.nIndex;
    COMTOOLSIZE nInfoLen = ctInfo->revData.pBuf[nIndex].pInfo.nLen;

    ctInfo->revData.pBuf[nIndex].pData.nLen += nLen;
    ctInfo->revData.pBuf[nIndex].pInfo.pInfo[nInfoLen - 1].nBlockLen += nLen;
}
static void ReadComAppendInfo(PCOMTOOLEXTRA ctInfo, COMTOOLDATATYPE cType)
{
    uint8_t nIndex = ctInfo->revData.nIndex;
    COMTOOLSIZE* nLen = &(ctInfo->revData.pBuf[nIndex].pInfo.nLen);

    ctInfo->revData.pBuf[nIndex].pInfo.pInfo[*nLen].nBlockLen = 0;
    ctInfo->revData.pBuf[nIndex].pInfo.pInfo[*nLen].cType = cType;
    ctInfo->revData.pBuf[nIndex].pInfo.pInfo[*nLen].tTime = ctInfo->revData.tTime.timeLastByte;
    (*nLen)++;
}
static void SwapComBuf(PCOMTOOLEXTRA ctInfo)
{
    uint8_t nIndex = 1 - ctInfo->revData.nIndex;

    ctInfo->revData.nIndex = nIndex;
    ctInfo->revData.pBuf[nIndex].pData.nLen = 0;

    ctInfo->revData.pBuf[nIndex].pInfo.nLen = 1;
    ctInfo->revData.pBuf[nIndex].pInfo.pInfo[0].cType = COMTOOL_DATA_TYPE_DATA;
    ctInfo->revData.pBuf[nIndex].pInfo.pInfo[0].nBlockLen = 0;
}
static void PrintStartInfo(PCOMTOOLEXTRA ctInfo, PSYSTEMTIME pTime)
{
    int nLen = HAN_snprintf(
        ctInfo->revData.pBuf[0].pText, sizeof(ctInfo->revData.pBuf[0].pText),
        TEXT("\r\n[%u:%u:%u.%u] 收 -> "),
        pTime->wHour, pTime->wMinute, pTime->wSecond, pTime->wMilliseconds
    );
    TextWindowAppendText(ctInfo, ctInfo->revData.pBuf[0].pText, nLen);
}
static void PrintEndInfo(PCOMTOOLEXTRA ctInfo, PSYSTEMTIME pTime)
{
    int nLen = HAN_snprintf(
        ctInfo->revData.pBuf[0].pText, sizeof(ctInfo->revData.pBuf[0].pText),
        TEXT("\r\n[%u:%u:%u.%u]\r\n"),
        pTime->wHour, pTime->wMinute, pTime->wSecond, pTime->wMilliseconds
    );
    TextWindowAppendText(ctInfo, ctInfo->revData.pBuf[0].pText, nLen);
}
static void PrintDataASCII(PCOMTOOLEXTRA ctInfo, const uint8_t* pBuf, COMTOOLSIZE nLen)
{
    COMTOOLSIZE nOffset = 0;
    for (COMTOOLSIZE i = 0; i < nLen; i++)
    {
        if (TEXT('\0') != pBuf[i]) { (void)memcpy(&ctInfo->revData.pBuf[0].pText[nOffset], &pBuf[i], 1); }
        else { (void)memcpy(&ctInfo->revData.pBuf[0].pText[nOffset], TEXT("\\0"), 2); nOffset++; }
        nOffset++;
    }
    ctInfo->revData.pBuf[0].pText[nOffset] = TEXT('\0');
    TextWindowAppendText(ctInfo, ctInfo->revData.pBuf[0].pText, (HANINT)nOffset);
}
static void PrintDataHex(PCOMTOOLEXTRA ctInfo, const uint8_t* pBuf, COMTOOLSIZE nLen)
{
    for (COMTOOLSIZE i = 0; i < nLen; i++)
    {
        HAN_snprintf(&ctInfo->revData.pBuf[0].pText[i * 3], sizeof(ctInfo->revData.pBuf[0].pText) - (i * 3),
            TEXT(" %02X"), pBuf[i]
        );
    }
    EditAppendText(ctInfo->printData.printData.hText, ctInfo->revData.pBuf[0].pText, FALSE);
}
static void PrintSumData(PCOMTOOLEXTRA ctInfo)
{
    HANCHAR pText[COMTOOL_SUM_DATA_TEXT_SIZE];

    HAN_snprintf(pText, COMTOOL_SUM_DATA_TEXT_SIZE, COMTOOL_SUM_RX_TITLE COMTOOL_SIZE_PRINT_FORMAT, ctInfo->sumData.sumRx.nSum);
    SetWindowText(ctInfo->sumData.sumRx.hWnd, pText);
}
static void BaudrateAction(PCOMTOOLEXTRA ctInfo)
{
    UpdateComCfg(ctInfo, COMTOOL_UPDATE_COM_CFG_BAUDRATE);
    if (INVALID_HANDLE_VALUE != ctInfo->hComFile)
    {
        DCB dcbCOMState;
        if (TRUE == GetCommState(ctInfo->hComFile, &dcbCOMState))
        {
            dcbCOMState.BaudRate = ctInfo->cfgCOM.cfgCOM.comState.BaudRate;
            SetCommState(ctInfo->hComFile, &dcbCOMState);
        }
    }
}
static void ByteSizeAction(PCOMTOOLEXTRA ctInfo)
{
    UpdateComCfg(ctInfo, COMTOOL_UPDATE_COM_CFG_BYTESIZE);
    if (INVALID_HANDLE_VALUE != ctInfo->hComFile)
    {
        DCB dcbCOMState;
        if (TRUE == GetCommState(ctInfo->hComFile, &dcbCOMState))
        {
            dcbCOMState.ByteSize = ctInfo->cfgCOM.cfgCOM.comState.ByteSize;
            SetCommState(ctInfo->hComFile, &dcbCOMState);
        }
    }
}
static void StopBitsAction(PCOMTOOLEXTRA ctInfo)
{
    UpdateComCfg(ctInfo, COMTOOL_UPDATE_COM_CFG_STOPBITS);
    if (INVALID_HANDLE_VALUE != ctInfo->hComFile)
    {
        DCB dcbCOMState;
        if (TRUE == GetCommState(ctInfo->hComFile, &dcbCOMState))
        {
            dcbCOMState.StopBits = ctInfo->cfgCOM.cfgCOM.comState.StopBits;
            SetCommState(ctInfo->hComFile, &dcbCOMState);
        }
    }
}
static void ParityAction(PCOMTOOLEXTRA ctInfo)
{
    UpdateComCfg(ctInfo, COMTOOL_UPDATE_COM_CFG_PARITY);
    if (INVALID_HANDLE_VALUE != ctInfo->hComFile)
    {
        DCB dcbCOMState;
        if (TRUE == GetCommState(ctInfo->hComFile, &dcbCOMState))
        {
            dcbCOMState.Parity = ctInfo->cfgCOM.cfgCOM.comState.Parity;
            SetCommState(ctInfo->hComFile, &dcbCOMState);
        }
    }
}
static void HexButtonAction(PCOMTOOLEXTRA ctInfo)
{
    int nCheck = ButtonGetCheck(ctInfo->printHex.hHex);
    if (BST_CHECKED == nCheck) { ctInfo->printHex.bHex = TRUE; }
    else { ctInfo->printHex.bHex = FALSE; }
    SetWindowText(ctInfo->printData.printData.hText, NULL);
    ctInfo->nTextWindowLen = 0;
}
static void PrintStartTimeButtonAction(PCOMTOOLEXTRA ctInfo)
{
    int nCheck = ButtonGetCheck(ctInfo->printData.printStartTime.hStart);
    if (BST_CHECKED == nCheck) { ctInfo->printData.printStartTime.bStart = TRUE; }
    else { ctInfo->printData.printStartTime.bStart = FALSE; }
}
static void PrintEndTimeButtonAction(PCOMTOOLEXTRA ctInfo)
{
    int nCheck = ButtonGetCheck(ctInfo->printData.printEndTime.hEnd);
    if (BST_CHECKED == nCheck) { ctInfo->printData.printEndTime.bEnd = TRUE; }
    else { ctInfo->printData.printEndTime.bEnd = FALSE; }
}
static void SaveToFileAction(PCOMTOOLEXTRA ctInfo)
{
    HANCHAR pText[SAVE_FILE_PATH_MESSAGEBOX_STR_SIZE];
    HANINT bCheck = ButtonGetCheck(ctInfo->saveToFile.hSave);

    if (BST_CHECKED == bCheck)
    {
        GetWindowText(ctInfo->saveToFile.hPath, ctInfo->saveToFile.pPath, PATH_STR_SIZE);
        if (FALSE == PathFileExists(ctInfo->saveToFile.pPath))
        {
            GetDefaultFileName(ctInfo->saveToFile.pPath, PATH_STR_SIZE);
        }
        ctInfo->saveToFile.hFile = CreateFile(ctInfo->saveToFile.pPath, FILE_APPEND_DATA, FILE_SHARE_READ,
            NULL, OPEN_ALWAYS, FILE_FLAG_OVERLAPPED, NULL
        );

        if (INVALID_HANDLE_VALUE != ctInfo->saveToFile.hFile)
        {
            HAN_snprintf(
                pText, SAVE_FILE_PATH_MESSAGEBOX_STR_SIZE,
                SAVE_FILE_PATH_MESSAGEBOX_TITLE TEXT("%s"), ctInfo->saveToFile.pPath
            );
            MessageBox(ctInfo->hSelf, pText, TEXT("保存数据"), 0);
            ctInfo->saveToFile.bSave = TRUE;
        }
        else
        {
            MessageBox(ctInfo->hSelf, TEXT("打开文件失败"), TEXT("文件错误"), 0);
            ButtonSetUnchecked(ctInfo->saveToFile.hSave);
        }
    }
    else
    {
        ctInfo->saveToFile.bSave = FALSE;
        CloseHandle(ctInfo->saveToFile.hFile);
        ctInfo->saveToFile.hFile = INVALID_HANDLE_VALUE;
    }
}
static HANPCSTR GetDefaultFileName(HANPSTR pPath, DWORD nSize)
{
    HANSIZE nLen;
    SYSTEMTIME tTime;
    
    GetModuleFileName(NULL, pPath, nSize);
    PathRemoveFileSpec(pPath);
    HAN_strcat(pPath, TEXT("\\Data"));

    if (FALSE == PathFileExists(pPath))
    {
        CreateDirectory(pPath, NULL);
    }

    nLen = HAN_strlen(pPath);
    GetLocalTime(&tTime);
    HAN_snprintf(&pPath[nLen], nSize - nLen, TEXT("\\COMData_%u_%u_%u_%u_%u_%u_%u.CTDAT"),
        tTime.wYear, tTime.wMonth, tTime.wDay, tTime.wHour, tTime.wMinute, tTime.wSecond, tTime.wMilliseconds
    );

    return pPath;
}
static void TextWindowAppendText(PCOMTOOLEXTRA ctInfo, HANPCSTR pText, int nTextLen)
{
    if ((ctInfo->nTextWindowLen + nTextLen) < COMTOOL_TEXT_WINDOW_LIMIT)
    {
        EditAppendText(ctInfo->printData.printData.hText, pText, FALSE);
        ctInfo->nTextWindowLen += nTextLen;
    }
    else
    {
        SetWindowText(ctInfo->printData.printData.hText, pText);
        ctInfo->nTextWindowLen = nTextLen;
    }
}
static DWORD CreateBaudrateDialog(PCOMTOOLEXTRA ctInfo)
{
    DWORD nBaudrate;
    HGLOBAL hData;
    uint8_t* pDialogInfo;
    DLGTEMPLATE* pDlgInfoTitle;
    WORD* pWord;
    WCHAR* pText;
    uint8_t nOffset = 0;
    int nTextLen;

    hData = GlobalAlloc(GMEM_ZEROINIT, 1024);
    if (NULL != hData)
    {
        pDialogInfo = GlobalLock(hData);
        if (NULL != pDialogInfo)
        {
            pDlgInfoTitle = (void*)(&pDialogInfo[nOffset]);
            pDlgInfoTitle->style = WS_POPUP | WS_BORDER | WS_SYSMENU | WS_BORDER | DS_MODALFRAME | WS_CAPTION;
            pDlgInfoTitle->cdit = 0;
            pDlgInfoTitle->x = 100;
            pDlgInfoTitle->y = 100;
            pDlgInfoTitle->cx = COMTOOL_USER_BAUDRATE_DLU_W;
            pDlgInfoTitle->cy = COMTOOL_DLU_TEXT_H * 2 + 3 * COMTOOL_DLU_WINDOW_DY;
            nOffset += sizeof(DLGTEMPLATE);

            pWord = (void*)(&pDialogInfo[nOffset]);
            pWord[0] = 0;
            pWord[1] = 0;
            nOffset += sizeof(*pWord) * 2;
            pText = (void*)(&pDialogInfo[nOffset]);
            nTextLen = 1 + MultiByteToWideChar(CP_ACP, 0, "自定义波特率", -1, pText, 50);
            nOffset += (uint8_t)(sizeof(WCHAR) * nTextLen);

            GlobalUnlock(hData);
            nBaudrate = (DWORD)DialogBoxIndirectParam(
                ctInfo->hInst, (LPDLGTEMPLATE)hData, ctInfo->hSelf,
                ComToolUserBaudrateDialogProc, (LPARAM)ctInfo
            );
            GlobalFree(hData);
        }
    }
    else
    {
        nBaudrate = 0;
    }

    return nBaudrate;
}
static void BaudrateDialogInitAction(HWND hUserBaudrate, LPARAM lParam)
{
    HANCHAR pText[PATH_STR_SIZE];
    PCOMTOOLEXTRA ctInfo = (PCOMTOOLEXTRA)lParam;
    HINSTANCE hInst = ctInfo->hInst;
    SetWindowLongPtr(hUserBaudrate, DWLP_USER, (LONG_PTR)ctInfo);

    RECT rcInput = {
        .left = COMTOOL_DLU_WINDOW_DX, .right = COMTOOL_USER_BAUDRATE_DLU_W - COMTOOL_DLU_WINDOW_DX,
        .top = COMTOOL_DLU_WINDOW_DY, .bottom = COMTOOL_DLU_WINDOW_DX + COMTOOL_DLU_TEXT_H,
    };
    RECT rcOk = {
        .left = COMTOOL_USER_BAUDRATE_DLU_W - 50, .right = COMTOOL_USER_BAUDRATE_DLU_W - COMTOOL_DLU_WINDOW_DX,
        .top = COMTOOL_DLU_TEXT_H + (2 * COMTOOL_DLU_WINDOW_DY),
        .bottom = (COMTOOL_DLU_TEXT_H + COMTOOL_DLU_WINDOW_DY) * 2,
    };
    MapDialogRect(hUserBaudrate, &rcInput);
    MapDialogRect(hUserBaudrate, &rcOk);

    ctInfo->userBaudrate.hInput = CreateWindow(TEXT("edit"), NULL,
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
        rcInput.left, rcInput.top,
        GetRectW(&rcInput), GetRectH(&rcInput),
        hUserBaudrate, (HMENU)WID_COM_TOOL_CONSOLE_USER_BAUDRATE_INPUT, hInst, NULL);
    ctInfo->userBaudrate.hOk = CreateWindow(TEXT("button"), TEXT("确定"),
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        rcOk.left, rcOk.top,
        GetRectW(&rcOk), GetRectH(&rcOk),
        hUserBaudrate, (HMENU)WID_COM_TOOL_CONSOLE_USER_BAUDRATE_OK, hInst, NULL);

    SendMessage(ctInfo->userBaudrate.hInput, WM_SETFONT, (WPARAM)(ctInfo->hFont.hHex), (LPARAM)TRUE);
    SendMessage(ctInfo->userBaudrate.hOk, WM_SETFONT, (WPARAM)(ctInfo->hFont.hSys), (LPARAM)TRUE);
    
    HAN_snprintf(pText, PATH_STR_SIZE, TEXT("%lu"), ctInfo->cfgCOM.cfgCOM.comState.BaudRate);
    SetWindowText(ctInfo->userBaudrate.hInput, pText);
}
static void BaudrateDialogOkAction(PCOMTOOLEXTRA ctInfo, HWND hUserBaudrate)
{
    HANCHAR pText[PATH_STR_SIZE];
    DWORD nBaudrate;
    
    GetWindowText(ctInfo->userBaudrate.hInput, pText, PATH_STR_SIZE);
    nBaudrate = HAN_strtoul(pText, NULL, 10);

    EndDialog(hUserBaudrate, nBaudrate);
}

static void ReadComThread(PCOMTOOLEXTRA ctInfo)
{
    DWORD nRevLen;
    DWORD msTime;
    uint8_t* pData;
    HANSIZE nIndex;
    HANSIZE nOffset;
    
    nIndex = ctInfo->revData.nIndex;
    nOffset = ctInfo->revData.pBuf[nIndex].pData.nLen;
    pData = &(ctInfo->revData.pBuf[nIndex].pData.pData[nOffset]);

    ReadCOM(ctInfo->hComFile, pData, COMTOOL_COM_FRAME_BUF_SIZE, &nRevLen, NULL);
    msTime = GetTickCount();
    
    if (0 < nRevLen)
    {
        GetLocalTime(&(ctInfo->revData.tTime.timeLastByte));
        ctInfo->revData.tTime.msTime = msTime;
    }

    switch (ctInfo->revData.cStatus) {
        case COMTOOL_READ_STATUS_READING: {
            if (ctInfo->revData.tTime.msTimeOut < (msTime - ctInfo->revData.tTime.msTime))
            {
                ReadComAppendInfo(ctInfo, COMTOOL_DATA_TYPE_END);
                ctInfo->revData.cStatus = COMTOOL_READ_STATUS_WAITING;
            }
        } break;
        case COMTOOL_READ_STATUS_WAITING: {
            if (0 < nRevLen)
            {
                ReadComAppendInfo(ctInfo, COMTOOL_DATA_TYPE_START);
                ReadComAppendInfo(ctInfo, COMTOOL_DATA_TYPE_DATA);
                ctInfo->revData.cStatus = COMTOOL_READ_STATUS_READING;
            }
        } break;

        default: {
            /* idle */
        } break;
    }

    if (0 < nRevLen)
    {
        ReadComGetByteSuccessAction(ctInfo, nRevLen);
    }

    if ((FALSE == ctInfo->printData.printData.bPrinting) && 
        (COMTOOL_PRINT_COM_PERIOD < (msTime - ctInfo->revData.tTime.msPrintComTime)))
    {
        ctInfo->revData.tTime.msPrintComTime = msTime;
        SwapComBuf(ctInfo);
        
        ctInfo->printData.printData.bPrinting = TRUE;
        ReleaseSemaphore(ctInfo->printData.printData.hPrintComSemaphore, 1, NULL);
    }
}
static DWORD PrintComThread(PCOMTOOLEXTRA ctInfo)
{
    while (INVALID_HANDLE_VALUE != ctInfo->hComFile)
    {
        WaitForSingleObject(ctInfo->printData.printData.hPrintComSemaphore, INFINITE);

        PrintComThreadSubDeal(ctInfo);

        HANComToolGraphicGetDataOperation(ctInfo);

        ctInfo->printData.printData.bPrinting = FALSE;
    }

    return 0;
}
static void PrintComThreadSubDeal(PCOMTOOLEXTRA ctInfo)
{
    uint8_t nIndex;
    uint8_t* pBuf;
    COMTOOLSIZE nInfoLen;
    COMTOOLSIZE nDataLen;
    COMTOOLSIZE iLoop;
    COMTOOLSIZE nOffset;

    nIndex = 1 - ctInfo->revData.nIndex;
    nInfoLen = ctInfo->revData.pBuf[nIndex].pInfo.nLen;
    pBuf = ctInfo->revData.pBuf[nIndex].pData.pData;
    iLoop = 0;
    nOffset = 0;
    
    while (iLoop < nInfoLen)
    {
        nDataLen = ctInfo->revData.pBuf[nIndex].pInfo.pInfo[iLoop].nBlockLen;

        switch (ctInfo->revData.pBuf[nIndex].pInfo.pInfo[iLoop].cType) {
            case COMTOOL_DATA_TYPE_START: {
                if (TRUE == ctInfo->printData.printStartTime.bStart)
                {
                    PrintStartInfo(ctInfo, &(ctInfo->revData.pBuf[nIndex].pInfo.pInfo[iLoop].tTime));
                }
            } break;
            case COMTOOL_DATA_TYPE_END: {
                if (TRUE == ctInfo->printData.printEndTime.bEnd)
                {
                    PrintEndInfo(ctInfo, &(ctInfo->revData.pBuf[nIndex].pInfo.pInfo[iLoop].tTime));
                }
            } break;
            case COMTOOL_DATA_TYPE_DATA: {
                if (0 < nDataLen)
                {
                    if (FALSE == ctInfo->printHex.bHex) { PrintDataASCII(ctInfo, &pBuf[nOffset], nDataLen); }
                    else { PrintDataHex(ctInfo, &pBuf[nOffset], nDataLen); }
                    nOffset += nDataLen;
                }
            } break;
        }

        iLoop++;
    }

    ctInfo->sumData.sumRx.nSum += ctInfo->revData.pBuf[nIndex].pData.nLen;
    PrintSumData(ctInfo);
    
    if ((TRUE == ctInfo->saveToFile.bSave) && (0 < ctInfo->revData.pBuf[nIndex].pData.nLen))
    {
        WriteFile(
            ctInfo->saveToFile.hFile,
            pBuf,
            (DWORD)(ctInfo->revData.pBuf[nIndex].pData.nLen),
            NULL,
            &(ctInfo->saveToFile.olFile)
        );
    }
}

static void CfgWindowToTextComId(PCOMTOOLEXTRA ctInfo, HANPSTR pText)
{
    HAN_snprintf(pText, INI_COM_TOOL_VALUE_SIZE, TEXT("%u"), ctInfo->cfgCOM.cfgCOM.comFile.id);
}
static void CfgWindowToTextBaudrate(PCOMTOOLEXTRA ctInfo, HANPSTR pText)
{
    HAN_snprintf(pText, INI_COM_TOOL_VALUE_SIZE, TEXT("%lu"), ctInfo->cfgCOM.cfgCOM.comState.BaudRate);
}
static void CfgWindowToTextUserBaudrate(PCOMTOOLEXTRA ctInfo, HANPSTR pText)
{
    HANCHAR pTempText[INI_COM_TOOL_VALUE_SIZE];
    COMTOOLBAUDRATEID iBaudrate = ComboBoxGetCursel(ctInfo->cfgCOM.hBaudrate);
    DWORD nBaudrate;

    if (COMTOOL_BAUDRATE_USER == iBaudrate)
    {
        GetWindowText(ctInfo->cfgCOM.hBaudrate, pTempText, INI_COM_TOOL_VALUE_SIZE);
        nBaudrate = HAN_strtoul(&pTempText[HAN_strlen(TEXT("自定义："))], NULL, 10);
        HAN_snprintf(pText, INI_COM_TOOL_VALUE_SIZE, TEXT("%lu"), nBaudrate);
    }
    else
    {
        HAN_snprintf(pText, INI_COM_TOOL_VALUE_SIZE, TEXT("0"));
    }
}
static void CfgWindowToTextByteSize(PCOMTOOLEXTRA ctInfo, HANPSTR pText)
{
    COMTOOLBYTESIZEID iByteSize = ComboBoxGetCursel(ctInfo->cfgCOM.hByteSize);
    
    HAN_strcpy(pText, g_pByteSize[iByteSize]);
}
static void CfgWindowToTextStopBits(PCOMTOOLEXTRA ctInfo, HANPSTR pText)
{
    COMTOOLSTOPBITSID iStopBits = ComboBoxGetCursel(ctInfo->cfgCOM.hStopBits);
    
    HAN_strcpy(pText, g_pStopBits[iStopBits].pText);
}
static void CfgWindowToTextParity(PCOMTOOLEXTRA ctInfo, HANPSTR pText)
{
    COMTOOLPARITYID iParity = ComboBoxGetCursel(ctInfo->cfgCOM.hParity);

    HAN_strcpy(pText, g_pParity[iParity].pText);
}
static void CfgWindowToTextHex(PCOMTOOLEXTRA ctInfo, HANPSTR pText)
{
    HANINT bCheck = ButtonGetCheck(ctInfo->printHex.hHex);

    if (BST_CHECKED == bCheck) { HAN_snprintf(pText, INI_COM_TOOL_VALUE_SIZE, TEXT("TRUE")); }
    else { HAN_snprintf(pText, INI_COM_TOOL_VALUE_SIZE, TEXT("FALSE")); }
}
static void CfgWindowToTextPrintStartTime(PCOMTOOLEXTRA ctInfo, HANPSTR pText)
{
    HANINT bCheck = ButtonGetCheck(ctInfo->printData.printStartTime.hStart);

    if (BST_CHECKED == bCheck) { HAN_snprintf(pText, INI_COM_TOOL_VALUE_SIZE, TEXT("TRUE")); }
    else { HAN_snprintf(pText, INI_COM_TOOL_VALUE_SIZE, TEXT("FALSE")); }
}
static void CfgWindowToTextPrintEndTime(PCOMTOOLEXTRA ctInfo, HANPSTR pText)
{
    HANINT bCheck = ButtonGetCheck(ctInfo->printData.printEndTime.hEnd);

    if (BST_CHECKED == bCheck) { HAN_snprintf(pText, INI_COM_TOOL_VALUE_SIZE, TEXT("TRUE")); }
    else { HAN_snprintf(pText, INI_COM_TOOL_VALUE_SIZE, TEXT("FALSE")); }
}

static void CfgTextToWindowComId(PCOMTOOLEXTRA ctInfo, HANPCSTR pText)
{
    uint16_t nTargetId = (uint16_t)HAN_strtoul(pText, NULL, 10);
    
    UpdatePortList(ctInfo, nTargetId);
}
static void CfgTextToWindowBaudrate(PCOMTOOLEXTRA ctInfo, HANPCSTR pText)
{
    for (COMTOOLBAUDRATEID iLoop = 1; iLoop < COMTOOL_BAUDRATE_CNT; iLoop++)
    {
        if (0 == HAN_strcmp(pText, g_pBaudrate[iLoop]))
        {
            ComboBoxSetCursel(ctInfo->cfgCOM.hBaudrate, iLoop);
        }
    }
}
static void CfgTextToWindowUserBaudrate(PCOMTOOLEXTRA ctInfo, HANPCSTR pText)
{
    HANINT iCursel = ComboBoxGetCursel(ctInfo->cfgCOM.hBaudrate);
    HANCHAR pTempText[INI_COM_TOOL_VALUE_SIZE];

    if (0 == iCursel)
    {
        DWORD nBaudrate = HAN_strtoul(pText, NULL, 10);
        if (0 != nBaudrate)
        {
            ctInfo->cfgCOM.cfgCOM.comState.BaudRate = nBaudrate;
            HAN_snprintf(pTempText, PATH_STR_SIZE, TEXT("自定义：%lu"), nBaudrate);
            SendMessage(ctInfo->cfgCOM.hBaudrate, CB_DELETESTRING, 0, 0);
            SendMessage(ctInfo->cfgCOM.hBaudrate, CB_INSERTSTRING, 0, (LPARAM)pTempText);
            ComboBoxSetCursel(ctInfo->cfgCOM.hBaudrate, 0);
        }
        else
        {
            ComboBoxSetCursel(ctInfo->cfgCOM.hBaudrate, COMTOOL_DEFAULT_BAUDRATE_ID);
        }
    }
    else
    {
        ctInfo->cfgCOM.cfgCOM.comState.BaudRate = HAN_strtoul(g_pBaudrate[iCursel], NULL, 10);
    }
}
static void CfgTextToWindowByteSize(PCOMTOOLEXTRA ctInfo, HANPCSTR pText)
{
    HANINT iCursel = ComboBoxFindString(ctInfo->cfgCOM.hByteSize, 0, pText);

    if (iCursel == CB_ERR) { iCursel = COMTOOL_DEFAULT_BYTE_SIZE; }
    ComboBoxSetCursel(ctInfo->cfgCOM.hByteSize, iCursel);
}
static void CfgTextToWindowStopBits(PCOMTOOLEXTRA ctInfo, HANPCSTR pText)
{
    HANINT iCursel = COMTOOL_DEFAULT_STOP_BITS;
    for (HANINT iLoop = 0; iLoop < COMTOOL_STOPBITS_CNT; iLoop++)
    {
        if (0 == HAN_strcmp(g_pStopBits[iLoop].pText, pText)) { iCursel = iLoop; break; }
    }
    ComboBoxSetCursel(ctInfo->cfgCOM.hStopBits, iCursel);
}
static void CfgTextToWindowParity(PCOMTOOLEXTRA ctInfo, HANPCSTR pText)
{
    HANINT iCursel = ComboBoxFindString(ctInfo->cfgCOM.hParity, 0, pText);

    if (iCursel == CB_ERR) { iCursel = COMTOOL_DEFAULT_PARITY; }
    ComboBoxSetCursel(ctInfo->cfgCOM.hParity, iCursel);
}
static void CfgTextToWindowHex(PCOMTOOLEXTRA ctInfo, HANPCSTR pText)
{
    if (0 == HAN_strcmp(pText, TEXT("TRUE"))) { ButtonSetChecked(ctInfo->printHex.hHex); }
    else { ButtonSetUnchecked(ctInfo->printHex.hHex); }
}
static void CfgTextToWindowPrintStartTime(PCOMTOOLEXTRA ctInfo, HANPCSTR pText)
{
    if (0 == HAN_strcmp(pText, TEXT("TRUE"))) { ButtonSetChecked(ctInfo->printData.printStartTime.hStart); }
    else { ButtonSetUnchecked(ctInfo->printData.printStartTime.hStart); }
}
static void CfgTextToWindowPrintEndTime(PCOMTOOLEXTRA ctInfo, HANPCSTR pText)
{
    if (0 == HAN_strcmp(pText, TEXT("TRUE"))) { ButtonSetChecked(ctInfo->printData.printEndTime.hEnd); }
    else { ButtonSetUnchecked(ctInfo->printData.printEndTime.hEnd); }
}
