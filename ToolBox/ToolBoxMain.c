#include <stdio.h>
#include <Windows.h>
#include <CommCtrl.h>
#include <shlwapi.h>

#include "..\HAN_Lib\HAN_windows.h"
#include "ToolBoxMain.h"
#include "GlobalVariables.h"
#include "ToolBoxInfo.h"

#define TAB_TITLE_SIZE_W        50
#define TAB_TITLE_SIZE_H        28

#define CFG_INI_FILE_NAME       TEXT("ToolBoxCfg.ini")

#define INI_MAIN_APP_NAME       TEXT("mainConfig")

typedef enum {
    WID_MAIN_BASE = HAN_WID_BASE,
    WID_MAIN_TAB,
    WID_MAIN_SUB_WINDOW,
} WIDMAIN;

static void GetIniFileFullPath(HANPSTR pIniFullPath, HANSIZE nLen);
static void ReadCfgIniFile(void);
static void WriteCfgIniFile(void);

static TOOLBOXID    sg_nLastChosenToolId = 0;
static HWND         g_hTab = NULL;

LRESULT CALLBACK ToolBoxMainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    switch (message) {
        case WM_CREATE: {
            HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;

            InitCommonControls();
            
            ReadCfgIniFile();

            RECT rcClient;
            GetClientRect(hWnd, &rcClient);

            INITCOMMONCONTROLSEX icexInitTab;
            TCITEM tiItem;

            icexInitTab.dwSize = sizeof(INITCOMMONCONTROLSEX);
            icexInitTab.dwICC = ICC_TAB_CLASSES;
            InitCommonControlsEx(&icexInitTab);

            g_hTab = CreateWindow(WC_TABCONTROL, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
                0, 0, rcClient.right, rcClient.bottom,
                hWnd, (HMENU)WID_MAIN_TAB, hInst, NULL);
            HFONT hFont = CreateFontIndirect(&g_lfSysFont);
            SendMessage(g_hTab, WM_SETFONT, (WPARAM)hFont, TRUE);
            tiItem.mask = TCIF_TEXT;

            DWORD dwWindowStyle;
            intptr_t hWinId = WID_MAIN_SUB_WINDOW;
            for (TOOLBOXID i = 0; i < TOOL_BOX_ID_CNT_MAX; i++)
            {
                dwWindowStyle = WS_CHILD;
                if (sg_nLastChosenToolId == i) { dwWindowStyle |= WS_VISIBLE; }

                tiItem.pszText = g_tiTabInfo[i].pTitle;
                TabCtrl_InsertItem(g_hTab, i, &tiItem);
                g_tiTabInfo[i].RegisterHANTabWindow(hInst);
                g_tiTabInfo[i].hItem = CreateWindow(g_tiTabInfo[i].pClass, NULL, dwWindowStyle,
                    0, TAB_TITLE_SIZE_H, rcClient.right, rcClient.bottom - TAB_TITLE_SIZE_H,
                    g_hTab, g_tiTabInfo[i].nWinId, hInst, g_tiTabInfo[i].pIni);
                g_tiTabInfo[i].nWinId = (HMENU)hWinId;
                hWinId++;
            }
            TabCtrl_SetItemSize(g_hTab, TAB_TITLE_SIZE_W, TAB_TITLE_SIZE_H);
            TabCtrl_SetCurSel(g_hTab, sg_nLastChosenToolId);
        } break;
        case WM_NOTIFY: {
            NMHDR* pNMHDR = (NMHDR*)lParam;
            if (WID_MAIN_TAB == pNMHDR->idFrom)
            {
                switch (pNMHDR->code) {
                    case TCN_SELCHANGE: {
                        int nPage = TabCtrl_GetCurSel(g_hTab);
                        for (int i = 0; i < TOOL_BOX_ID_CNT_MAX; i++)
                        {
                            if (nPage == i) { ShowWindow(g_tiTabInfo[i].hItem, SW_SHOW); }
                            else { ShowWindow(g_tiTabInfo[i].hItem, SW_HIDE); }
                        }
                        sg_nLastChosenToolId = nPage;
                    } break;
                }
            }
        } break;
        case WM_SIZE: {
            RECT rcClient;
            GetClientRect(hWnd, &rcClient);
            MoveWindow(g_hTab, 0, 0, rcClient.right, rcClient.bottom, TRUE);
            for (TOOLBOXID iLoop = 0; iLoop < TOOL_BOX_ID_CNT_MAX; iLoop++)
            {
                MoveWindow(g_tiTabInfo[iLoop].hItem,
                    0, TAB_TITLE_SIZE_H, rcClient.right,
                    rcClient.bottom - TAB_TITLE_SIZE_H,
                    TRUE
                );
            }
        } break;

        case WM_CLOSE: {
            WriteCfgIniFile();
            return DefWindowProc(hWnd, message, wParam, lParam);
        } break;

        case WM_DESTROY: {
            PostQuitMessage(0);
            break;
        }
        default: {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }

    return lWndProcRet;
}

BOOL ToolBoxTranslateAcceleratorCallback(PMSG pMsg)
{
    BOOL bRet = FALSE;
    HACCEL hAccel = NULL;
    HWND hTool = g_tiTabInfo[sg_nLastChosenToolId].hItem;

    if (NULL != g_tiTabInfo[sg_nLastChosenToolId].TranslateAcceleratorCallback)
    {
        hAccel = g_tiTabInfo[sg_nLastChosenToolId].TranslateAcceleratorCallback(hTool);
        if (0 != TranslateAccelerator(hTool, hAccel, pMsg))
        {
            bRet = TRUE;
        }
    }

    return bRet;
}

static void GetIniFileFullPath(HANPSTR pIniFullPath, HANSIZE nLen)
{
    GetModuleFileName(NULL, pIniFullPath, (DWORD)nLen);
    
    PathRemoveFileSpec(pIniFullPath);
    HAN_strcat(pIniFullPath, TEXT("\\") CFG_INI_FILE_NAME);
}
static void ReadCfgIniFile(void)
{
    HANCHAR pIniPath[PATH_STR_SIZE];
    HANCHAR pText[INI_MAIN_VALUE_STR_SIZE];
    TOOLBOXID iLoop;

    GetIniFileFullPath(pIniPath, PATH_STR_SIZE);

    GetPrivateProfileString(
        INI_MAIN_APP_NAME, TEXT("LastChosenTool"), TEXT("0"),
        pText, INI_MAIN_VALUE_STR_SIZE, pIniPath
    );
    sg_nLastChosenToolId = HAN_strtoul(pText, NULL, 10);

    for (iLoop = 0; iLoop < TOOL_BOX_ID_CNT_MAX; iLoop++)
    {
        if (NULL != g_tiTabInfo[iLoop].ReadIniFile)
        {
            g_tiTabInfo[iLoop].ReadIniFile(pIniPath, g_tiTabInfo[iLoop].pIni);
        }
    }
}
static void WriteCfgIniFile(void)
{
    HANCHAR pIniPath[PATH_STR_SIZE];
    HANCHAR pText[INI_MAIN_VALUE_STR_SIZE];
    TOOLBOXID iLoop;

    GetIniFileFullPath(pIniPath, PATH_STR_SIZE);

    HAN_snprintf(pText, INI_MAIN_VALUE_STR_SIZE, TEXT("%u"), sg_nLastChosenToolId);
    pText[INI_MAIN_VALUE_STR_SIZE - 1] = TEXT('\0');

    WritePrivateProfileString(INI_MAIN_APP_NAME, TEXT("LastChosenTool"), pText, pIniPath);
    
    for (iLoop = 0; iLoop < TOOL_BOX_ID_CNT_MAX; iLoop++)
    {
        if (NULL != g_tiTabInfo[iLoop].ReadIniFile)
        {
            g_tiTabInfo[iLoop].WriteIniFile(pIniPath, g_tiTabInfo[iLoop].hItem);
        }
    }
}
