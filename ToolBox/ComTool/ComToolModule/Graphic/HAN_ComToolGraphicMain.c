#include <Windows.h>
#include <CommCtrl.h>

#include "..\..\..\..\HAN_Lib\HAN_plot.h"
#include "..\..\..\..\HAN_Lib\HAN_Data.h"
#include "..\..\HAN_ComToolDef.h"
#include "HAN_ComToolGraphicMain.h"

#define COM_TOOL_GRAPHIC_TIP                                \
    TEXT("\n\n\n\n\n")                                    \
    TEXT("选择协议，使用“ctrl + 鼠标左击”逐个选择变量\n")      \
    TEXT("或使用“shift + 鼠标左击”批量选择变量\n")            \
    TEXT("选择变量后右击\n")                                 \
    TEXT("在弹出的菜单中选择要绘制的图形\n")                   \
    TEXT("部分图形对变量数有要求，若数量不匹配，无法选中")

#define COM_TOOL_GRAPHIC_TAB_ITEM_SIZE_W        50
#define COM_TOOL_GRAPHIC_TAB_ITEM_SIZE_H        28

#define COM_TOOL_GRAPHIC_GRAPHIC_NAME_SIZE      64

#define COM_TOOL_GRAPHIC_INVALID_ID             ((HANSIZE)(-1))

typedef enum {
    COMTOOL_GRAPHIC_TYPE_POLYLINE,
    COMTOOL_GRAPHIC_TYPE_SCATTER,
    COMTOOL_GRAPHIC_TYPE_CNT,
} COMTOOLGRAPHICTYPE;
typedef enum {
    COMTOOL_GRAPHIC_TAB_CLOSE_CLOSE_TARGET,
    COMTOOL_GRAPHIC_TAB_CLOSE_CLOSE_OTHER,
    COMTOOL_GRAPHIC_TAB_CLOSE_CLOSE_LEFT,
    COMTOOL_GRAPHIC_TAB_CLOSE_CLOSE_RIGHT,
    COMTOOL_GRAPHIC_TAB_CLOSE_CLOSE_ALL,
    COMTOOL_GRAPHIC_TAB_CLOSE_TYPE_CNT,
} COMTOOLGRAPHICTABCLOSETYPE;

typedef struct tagCOMTOOLGRAPHICMAINEXTRA {
    HANDLE                  hHeap;
    HINSTANCE               hInst;
    HWND                    hSelf;
    PCOMTOOLEXTRA           ctInfo;
    struct {
        uint8_t             pData[COMTOOL_COM_FRAME_BUF_SIZE * 2];
        HANSIZE             nLen;
        void                (*GetDataCallback)(
                                struct tagCOMTOOLGRAPHICMAINEXTRA* gmInfo,
                                const uint8_t* pData,
                                HANSIZE nLen);
        HANSIZE             nValueLen;
    } dataHandle;
    struct {
        HWND                hMsgList;
        HANINT              iChosenMsg;
    } msgList;
    struct {
        HWND                hVarTable;
        HMENU               hRightClickMenu;
    } varTable;
    struct {
        HWND                hInput;
        HWND                hOk;
        HANPSTR             pName;
    } graphicName;
    struct {
        HWND                hTab;
        HWND                hTip;
        HMENU               hRightClickMenu;
        HANLIST             listGraphic;
    } graphicWindow;
    struct {
        HFONT               hHex;
        HFONT               hSys;
        HFONT               hTip;
    } hFont;                /* 字体 */
} COMTOOLGRAPHICMAINEXTRA, * PCOMTOOLGRAPHICMAINWND;

typedef struct tagCOMTOOLGRAPHICTABCLOSE {
    HANPCSTR                pCloseText;
    void                    (*CloseOperation)(PCOMTOOLGRAPHICMAINWND gmInfo, HANINT nTabId);
} COMTOOLGRAPHICTABCLOSE, * PCOMTOOLGRAPHICTABCLOSE;

typedef struct tagCOMTOOLGRAPHICPLOTVAR {
    HANINT                  nVarId;
    HANPCSTR                pValueText;
    HANPLOTNUM              nValue;
} COMTOOLGRAPHICPLOTVAR, * PCOMTOOLGRAPHICPLOTVAR;
typedef struct tagCOMTOOLGRAPHICPLOTINFO {
    HWND                    hWindow;
    HANINT                  nMsgId;
    HANINT                  nPlotId;
    PCOMTOOLGRAPHICPLOTVAR  pVar;
    HANINT                  nVarCnt;
    RECT                    rcPos;
    HANCHAR                 pName[COM_TOOL_GRAPHIC_GRAPHIC_NAME_SIZE];
} COMTOOLGRAPHICPLOTINFO, * PCOMTOOLGRAPHICPLOTINFO;

typedef struct tagCOMTOOLGRAPHICPLOT {
    HANPCSTR                pName;
    void                    (*RegisterPlot)(HINSTANCE hInst);
    BOOL                    (*OptionalCondition)(PCOMTOOLGRAPHICMAINWND gmInfo, PCOMTOOLGRAPHICPLOTINFO plotInfo);
    HWND                    (*CreatePlot)(PCOMTOOLGRAPHICMAINWND gmInfo, PCOMTOOLGRAPHICPLOTINFO plotInfo);
    void                    (*PushData)(HWND hPlot, PCOMTOOLGRAPHICPLOTVAR pVar, HANINT nVarLen);
} COMTOOLGRAPHICPLOT, * PCOMTOOLGRAPHICPLOT;

static LRESULT CALLBACK ComToolGraphicMainWndProc(HWND hComToolGraphicMain, UINT message, WPARAM wParam, LPARAM lParam);

static LRESULT CreateCallback(HWND hComToolGraphic, LPARAM lParam);
static void CommandOperation(HWND hComToolGraphic, PCOMTOOLGRAPHICMAINWND gmInfo, WPARAM wParam, LPARAM lParam);
static void CreateVarTable(PCOMTOOLGRAPHICMAINWND gmInfo);
static void UpdateMsgInfo(PCOMTOOLGRAPHICMAINWND gmInfo);
static void ChooseMsgListOperation(PCOMTOOLGRAPHICMAINWND gmInfo);
static void GetDataOperation(PCOMTOOLGRAPHICMAINWND gmInfo, const uint8_t* pData, HANSIZE nLen);
static void UpdatePlotData(PCOMTOOLGRAPHICMAINWND gmInfo);
static void NotifyOperation(PCOMTOOLGRAPHICMAINWND gmInfo, WPARAM wParam, LPARAM lParam);
static void VarTableNotifyOperation(PCOMTOOLGRAPHICMAINWND gmInfo, NMHDR* pNotify);
static void UpdateChosenVar(HWND hVarTable, PCOMTOOLGRAPHICPLOTINFO plotInfo);
static BOOL CreateNewGraphicWindow(PCOMTOOLGRAPHICMAINWND gmInfo, HANINT nPlotId, PCOMTOOLGRAPHICPLOTINFO plotInfo);
static void GraphicTabNotifyOperation(PCOMTOOLGRAPHICMAINWND gmInfo, NMHDR* pNotify);
static void ShowTabCurselGraphic(PCOMTOOLGRAPHICMAINWND gmInfo);
static void TabRightClickOperation(PCOMTOOLGRAPHICMAINWND gmInfo);
static void DestroyOperation(PCOMTOOLGRAPHICMAINWND gmInfo);
static BOOL CreateGraphicNameDialog(PCOMTOOLGRAPHICMAINWND gmInfo);
static void GraphicNameDialogInitCallback(HWND hGraphicName, LPARAM lParam);
static void GraphicNameDialogOkCallback(PCOMTOOLGRAPHICMAINWND gmInfo, HWND hGraphicName);

static void* AllocGraphic(void* pOldData, size_t nNewSize, void* pParam);
static void FreeGraphicListNode(void* pData, void* pParam);
static void FreeGraphicPlotInfo(void* pData, void* pParam);
static void DestroyGraphicWindow(void* pGraphicWindow, void* pParam);

static void GraphicTabCloseTarget(PCOMTOOLGRAPHICMAINWND gmInfo, HANINT nTabId);
static void GraphicTabCloseOther(PCOMTOOLGRAPHICMAINWND gmInfo, HANINT nTabId);
static void GraphicTabCloseLeft(PCOMTOOLGRAPHICMAINWND gmInfo, HANINT nTabId);
static void GraphicTabCloseRight(PCOMTOOLGRAPHICMAINWND gmInfo, HANINT nTabId);
static void GraphicTabCloseAll(PCOMTOOLGRAPHICMAINWND gmInfo, HANINT nTabId);

static BOOL ComToolGraphicPolyLineCondition(PCOMTOOLGRAPHICMAINWND gmInfo, PCOMTOOLGRAPHICPLOTINFO plotInfo);
static BOOL ComToolGraphicScatterCondition(PCOMTOOLGRAPHICMAINWND gmInfo, PCOMTOOLGRAPHICPLOTINFO plotInfo);

static HWND ComToolGraphicCreatePolyLine(PCOMTOOLGRAPHICMAINWND gmInfo, PCOMTOOLGRAPHICPLOTINFO plotInfo);
static HWND ComToolGraphicCreateScatter(PCOMTOOLGRAPHICMAINWND gmInfo, PCOMTOOLGRAPHICPLOTINFO plotInfo);

static void ComToolGraphicPolyLinePushData(HWND hPlot, PCOMTOOLGRAPHICPLOTVAR pVar, HANINT nVarLen);
static void ComToolGraphicScatterPushData(HWND hPlot, PCOMTOOLGRAPHICPLOTVAR pVar, HANINT nVarLen);

static LOGFONT sg_lfTipFont = {
    .lfHeight = 24,
    .lfWidth = 0,
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
    .lfFaceName = TEXT("宋体"),
};
static const COMTOOLGRAPHICMAINEXTRA sg_ctgDefaultGraphic = {
    .dataHandle = {
        .nLen = 0,
        .GetDataCallback = NULL,
        .nValueLen = 1000,
    },
};
static const COMTOOLGRAPHICTABCLOSE sg_pGraphicClose[COMTOOL_GRAPHIC_TAB_CLOSE_TYPE_CNT] = {
    [COMTOOL_GRAPHIC_TAB_CLOSE_CLOSE_TARGET] = {
        .pCloseText = TEXT("关闭"),
        .CloseOperation = GraphicTabCloseTarget,
    },
    [COMTOOL_GRAPHIC_TAB_CLOSE_CLOSE_OTHER] = {
        .pCloseText = TEXT("关闭其它"),
        .CloseOperation = GraphicTabCloseOther,
    },
    [COMTOOL_GRAPHIC_TAB_CLOSE_CLOSE_LEFT] = {
        .pCloseText = TEXT("关闭左侧"),
        .CloseOperation = GraphicTabCloseLeft,
    },
    [COMTOOL_GRAPHIC_TAB_CLOSE_CLOSE_RIGHT] = {
        .pCloseText = TEXT("关闭右侧"),
        .CloseOperation = GraphicTabCloseRight,
    },
    [COMTOOL_GRAPHIC_TAB_CLOSE_CLOSE_ALL] = {
        .pCloseText = TEXT("全部关闭"),
        .CloseOperation = GraphicTabCloseAll,
    },
};
static const COMTOOLGRAPHICPLOT sg_pGraphicInfo[COMTOOL_GRAPHIC_TYPE_CNT] = {
    [COMTOOL_GRAPHIC_TYPE_POLYLINE] = {
        .pName = TEXT("折线图"),
        .RegisterPlot = RegisterHANPolyLine,
        .OptionalCondition = ComToolGraphicPolyLineCondition,
        .CreatePlot = ComToolGraphicCreatePolyLine,
        .PushData = ComToolGraphicPolyLinePushData,
    },
    [COMTOOL_GRAPHIC_TYPE_SCATTER] = {
        .pName = TEXT("散点图"),
        .RegisterPlot = RegisterHANScatter,
        .OptionalCondition = ComToolGraphicScatterCondition,
        .CreatePlot = ComToolGraphicCreateScatter,
        .PushData = ComToolGraphicScatterPushData,
    },
};

void RegisterHANComToolGraphicMain(HINSTANCE hInst)
{
    WNDCLASSEX wcex = {
        .cbSize         = sizeof(WNDCLASSEX),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = ComToolGraphicMainWndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = sizeof(PCOMTOOLEXTRA),
        .hInstance      = hInst,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName   = NULL,
        .lpszClassName  = HAN_COM_TOOL_GRAPHIC_MAIN_CLASS,
        .hIconSm        = NULL,
    };
    RegisterClassEx(&wcex);
}

void HANComToolGraphicUpdateMsg(HWND hComToolGraphicMain, const void* pData, HANSIZE nLen)
{
    SendMessage(hComToolGraphicMain, MSGID_COM_TOOL_GRAPHIC_GET_DATA_OPERATION, (WPARAM)nLen, (LPARAM)pData);
}

static LRESULT CALLBACK ComToolGraphicMainWndProc(HWND hComToolGraphicMain, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    PCOMTOOLGRAPHICMAINWND gmInfo = (PCOMTOOLGRAPHICMAINWND)GetWindowLongPtr(hComToolGraphicMain, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = CreateCallback(hComToolGraphicMain, lParam);
        } break;
        case WM_COMMAND: {
            CommandOperation(hComToolGraphicMain, gmInfo, wParam, lParam);
        } break;
        case WM_CTLCOLORSTATIC: {
            lWndProcRet = (INT_PTR)GetStockObject(WHITE_BRUSH);
        } break;
        case WM_NOTIFY: {
            NotifyOperation(gmInfo, wParam, lParam);
        } break;
        case WM_DESTROY: {
            DestroyOperation(gmInfo);
            lWndProcRet = DefWindowProc(hComToolGraphicMain, message, wParam, lParam);
        } break;

        case MSGID_COM_TOOL_GRAPHIC_UPDATE_MSG_INFO: {
            UpdateMsgInfo(gmInfo);
        } break;
        case MSGID_COM_TOOL_GRAPHIC_GET_DATA_OPERATION: {
            GetDataOperation(gmInfo, (uint8_t*)lParam, (HANSIZE)wParam);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hComToolGraphicMain, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}
static INT_PTR CALLBACK ComToolGraphicNameDialogProc(HWND hGraphicName, UINT message, WPARAM wParam, LPARAM lParam)
{
    INT_PTR nRet = FALSE;
    
    // 读取属性
    PCOMTOOLGRAPHICMAINWND gmInfo = (PCOMTOOLGRAPHICMAINWND)GetWindowLongPtr(hGraphicName, DWLP_USER);

    switch (message) {
        case WM_INITDIALOG: {
            GraphicNameDialogInitCallback(hGraphicName, lParam);
            nRet = TRUE;
            break;
        }
        case WM_COMMAND: {
            switch(LOWORD(wParam)) {
                case WID_COM_TOOL_GRAPHIC_NAME_OK: {
                    GraphicNameDialogOkCallback(gmInfo, hGraphicName);
                    nRet = TRUE;
                    break;
                }

                default: {
                    nRet = FALSE;
                    break;
                }
            }
            break;
        }
        case WM_CLOSE: {
            EndDialog(hGraphicName, FALSE);
            nRet = TRUE;
            break;
        }
        case WM_DESTROY: {
            EndDialog(hGraphicName, FALSE);
            nRet = TRUE;
            break;
        }

        default: {
            nRet = FALSE;
            break;
        }
    }

    return nRet;
}

static LRESULT CreateCallback(HWND hComToolGraphicMain, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PCOMTOOLGRAPHICMAINWND gmInfo;
    PCOMTOOLEXTRA ctInfo = ((LPCREATESTRUCT)lParam)->lpCreateParams;
    HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
    RECT rcClientSize;

    HANDLE hHeap = GetProcessHeap();
    if (NULL == hHeap) { lWndProcRet = -1; }
    if (-1 != lWndProcRet)
    {
        gmInfo = (PCOMTOOLGRAPHICMAINWND)HANWinHeapAlloc(hHeap, NULL, sizeof(COMTOOLGRAPHICMAINEXTRA));
        if (NULL == gmInfo) { lWndProcRet = -1; }
    }

    if (-1 != lWndProcRet)
    {
        int nWinX = COMTOOL_WINDOW_DX;
        int nWinY = COMTOOL_WINDOW_DY;

        SetWindowLongPtr(hComToolGraphicMain, 0, (LONG_PTR)gmInfo);

        *gmInfo = sg_ctgDefaultGraphic;
        gmInfo->hHeap = hHeap;
        gmInfo->hInst = hInst;
        gmInfo->hSelf = hComToolGraphicMain;
        gmInfo->ctInfo = ctInfo;
        gmInfo->graphicWindow.listGraphic = HANListInit();

        GetClientRect(hComToolGraphicMain, &rcClientSize);

        gmInfo->hFont.hHex = CreateFontIndirect(&g_lfHexFont);
        gmInfo->hFont.hSys = CreateFontIndirect(&g_lfSysFont);
        gmInfo->hFont.hTip = CreateFontIndirect(&sg_lfTipFont);

        for (COMTOOLGRAPHICTYPE iLoop = 0; iLoop < COMTOOL_GRAPHIC_TYPE_CNT; iLoop++)
        {
            sg_pGraphicInfo[iLoop].RegisterPlot(hInst);
        }

        gmInfo->msgList.hMsgList = CreateWindow(WC_COMBOBOX, NULL,
            COMTOOL_COMBOBOX_STYLE, nWinX, nWinY,
            COMTOOL_GRAPHIC_VAR_TABLE_VAR_W + COMTOOL_GRAPHIC_VAR_TABLE_VALUE_W, 300,
            hComToolGraphicMain, (HMENU)WID_COM_TOOL_GRAPHIC_MSG_LIST, hInst, NULL);
        nWinY += 30 + COMTOOL_WINDOW_DY;
        gmInfo->varTable.hVarTable = CreateWindow(WC_LISTVIEW, NULL,
            WS_CHILD | WS_VISIBLE | LVS_REPORT, nWinX, nWinY,
            COMTOOL_GRAPHIC_VAR_TABLE_VAR_W + COMTOOL_GRAPHIC_VAR_TABLE_VALUE_W, 300,
            hComToolGraphicMain, (HMENU)WID_COM_TOOL_GRAPHIC_VAR_TABLE, hInst, NULL);
        nWinX += COMTOOL_GRAPHIC_VAR_TABLE_VAR_W + COMTOOL_GRAPHIC_VAR_TABLE_VALUE_W + (COMTOOL_WINDOW_DX * 2);
        nWinY = COMTOOL_WINDOW_DY;
        gmInfo->graphicWindow.hTab = CreateWindow(WC_TABCONTROL, NULL, WS_CHILD | WS_VISIBLE,
            nWinX, nWinY, rcClientSize.right - nWinX, rcClientSize.bottom - nWinY,
            hComToolGraphicMain, (HMENU)WID_COM_TOOL_GRAPHIC_TAB, hInst, NULL);
        gmInfo->graphicWindow.hTip = CreateWindow(WC_STATIC, COM_TOOL_GRAPHIC_TIP,
            WS_CHILD | WS_VISIBLE | SS_CENTER, 2, COM_TOOL_GRAPHIC_TAB_ITEM_SIZE_H,
            rcClientSize.right - nWinX - 4, rcClientSize.bottom - nWinY - COM_TOOL_GRAPHIC_TAB_ITEM_SIZE_H - 2,
            gmInfo->graphicWindow.hTab, (HMENU)WID_COM_TOOL_GRAPHIC_TIP, hInst, NULL);

        TabCtrl_SetItemSize(
            gmInfo->graphicWindow.hTab,
            COM_TOOL_GRAPHIC_TAB_ITEM_SIZE_W,
            COM_TOOL_GRAPHIC_TAB_ITEM_SIZE_H
        );
        SendMessage(gmInfo->msgList.hMsgList, WM_SETFONT, (WPARAM)(gmInfo->hFont.hSys), (LPARAM)TRUE);
        SendMessage(gmInfo->graphicWindow.hTab, WM_SETFONT, (WPARAM)(gmInfo->hFont.hSys), (LPARAM)TRUE);
        SendMessage(gmInfo->graphicWindow.hTip, WM_SETFONT, (WPARAM)(gmInfo->hFont.hTip), (LPARAM)TRUE);

        CreateVarTable(gmInfo);
        UpdateMsgInfo(gmInfo);
    }

    return lWndProcRet;
}
static void CommandOperation(HWND hComToolGraphic, PCOMTOOLGRAPHICMAINWND gmInfo, WPARAM wParam, LPARAM lParam)
{
    (void)hComToolGraphic;
    (void)lParam;
    
    switch (LOWORD(wParam)) {
        case WID_COM_TOOL_GRAPHIC_MSG_LIST: {
            if (CBN_SELCHANGE == HIWORD(wParam)) { ChooseMsgListOperation(gmInfo); }
        } break;

        default: {
            break;
        }
    }
}
static void CreateVarTable(PCOMTOOLGRAPHICMAINWND gmInfo)
{
    LVCOLUMN lvTitle = { .mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM, };

    lvTitle.iSubItem = 0;
    lvTitle.pszText = TEXT("变量名");
    lvTitle.cx = COMTOOL_GRAPHIC_VAR_TABLE_VAR_W;
    lvTitle.fmt = LVCFMT_CENTER;
    ListView_InsertColumn(gmInfo->varTable.hVarTable, 0, &lvTitle);

    lvTitle.iSubItem = 1;
    lvTitle.pszText = TEXT("值");
    lvTitle.cx = COMTOOL_GRAPHIC_VAR_TABLE_VALUE_W;
    lvTitle.fmt = LVCFMT_LEFT;
    ListView_InsertColumn(gmInfo->varTable.hVarTable, 1, &lvTitle);
}
static void UpdateMsgInfo(PCOMTOOLGRAPHICMAINWND gmInfo)
{
    PCOMTOOLGRAPHICMSGINFO pMsgInfo = gmInfo->ctInfo->graphicClass.pMsgInfo;
    if (NULL != pMsgInfo)
    {
        ComboBoxClearString(gmInfo->msgList.hMsgList);
        ComboBoxAddStringStructArr(gmInfo->msgList.hMsgList, pMsgInfo->pMsg,
            sizeof(COMTOOLGRAPHICMSG), offsetof(COMTOOLGRAPHICMSG, pMsgName), pMsgInfo->nMsgCnt);
    }
}
static void ChooseMsgListOperation(PCOMTOOLGRAPHICMAINWND gmInfo)
{
    LVITEM lvItem = { .mask = LVIF_TEXT, };

    if (NULL != gmInfo->ctInfo->graphicClass.pMsgInfo)
    {
        HANINT nMsgListIndex = ComboBoxGetCursel(gmInfo->msgList.hMsgList);
        PCOMTOOLGRAPHICMSG pMsg = &(gmInfo->ctInfo->graphicClass.pMsgInfo->pMsg[nMsgListIndex]);
        HANINT nVarCnt = (HANINT)(pMsg->nVarCnt);

        if (nMsgListIndex < nVarCnt)
        {
            gmInfo->msgList.iChosenMsg = nMsgListIndex;
            
            ListView_DeleteAllItems(gmInfo->varTable.hVarTable);
            for (HANINT iLoop = 0; iLoop < nVarCnt; iLoop++)
            {
                lvItem.iItem = iLoop;
                lvItem.iSubItem = 0;
                lvItem.pszText = pMsg->pVar[iLoop].pName;
                ListView_InsertItem(gmInfo->varTable.hVarTable, &lvItem);
            }
            gmInfo->dataHandle.GetDataCallback = GetDataOperation;
        }
    }
}
static void GetDataOperation(PCOMTOOLGRAPHICMAINWND gmInfo, const uint8_t* pData, HANSIZE nLen)
{
    HANSIZE nBufLen = gmInfo->dataHandle.nLen;
    HANSIZE nLastMsgEndPos = 0;
    uint8_t* pBuf = gmInfo->dataHandle.pData;
    HANINT iChosenMsg = gmInfo->msgList.iChosenMsg;
    PCOMTOOLGRAPHICMSGINFO pMsgInfo = gmInfo->ctInfo->graphicClass.pMsgInfo;
    HANSIZE nMsgLen;
    LVITEM lvItem = { .mask = LVIF_TEXT, };
    
    if (NULL != pMsgInfo)
    {
        if (COMTOOL_COM_FRAME_BUF_SIZE < nBufLen) { nBufLen = 0; }

        (void)memcpy(&pBuf[nBufLen], pData, nLen);
        nBufLen += nLen;
        
        for (HANSIZE iLoop = 0; iLoop < nBufLen; iLoop++)
        {
            nMsgLen = pMsgInfo->pMsg[iChosenMsg].ReadMessage(
                &pBuf[iLoop],
                nBufLen - iLoop,
                pMsgInfo->pMsg[iChosenMsg].pVar);
            if (0 < nMsgLen)
            {
                nLastMsgEndPos = iLoop + nMsgLen;
                iLoop += nMsgLen - 1;
                UpdatePlotData(gmInfo);
            }
        }
        if (nLastMsgEndPos <= nBufLen)
        {
            gmInfo->dataHandle.nLen = nBufLen - nLastMsgEndPos;
            if (0 < nLastMsgEndPos)
            {
                (void)memcpy(&pBuf[0], &pBuf[nLastMsgEndPos], nBufLen - nLastMsgEndPos);
            }
        }
        else
        {
            gmInfo->dataHandle.nLen = 0;
        }

        for (HANSIZE iLoop = 0; iLoop < pMsgInfo->pMsg[iChosenMsg].nVarCnt; iLoop++)
        {
            lvItem.iItem = (HANINT)iLoop;
            lvItem.iSubItem = 1;
            lvItem.pszText = pMsgInfo->pMsg[iChosenMsg].pVar[iLoop].pValue;
            ListView_SetItem(gmInfo->varTable.hVarTable, &lvItem);
        }
    }
}
static void UpdatePlotData(PCOMTOOLGRAPHICMAINWND gmInfo)
{
    PCOMTOOLGRAPHICPLOTINFO plotInfo;
    HANPLIST pList = &(gmInfo->graphicWindow.listGraphic);

    HANListReForData(pList);
    plotInfo = HANListForData(pList);
    while (NULL != plotInfo)
    {
        if (gmInfo->msgList.iChosenMsg == plotInfo->nMsgId)
        {
            for (HANINT iLoop = 0; iLoop < plotInfo->nVarCnt; iLoop++)
            {
                plotInfo->pVar[iLoop].nValue = HAN_strtod(plotInfo->pVar[iLoop].pValueText, NULL);
            }
            sg_pGraphicInfo[plotInfo->nPlotId].PushData(plotInfo->hWindow, plotInfo->pVar, plotInfo->nVarCnt);
            RedrawWindow(plotInfo->hWindow, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
        }
        
        plotInfo = HANListForData(pList);
    }
}
static void NotifyOperation(PCOMTOOLGRAPHICMAINWND gmInfo, WPARAM wParam, LPARAM lParam)
{
    (void)wParam;
    NMHDR* pNotify = (NMHDR*)lParam;

    switch (pNotify->idFrom) {
        case WID_COM_TOOL_GRAPHIC_VAR_TABLE: {
            VarTableNotifyOperation(gmInfo, pNotify);
        } break;
        case WID_COM_TOOL_GRAPHIC_TAB: {
            GraphicTabNotifyOperation(gmInfo, pNotify);
        } break;

        default: {
            /* idle */
        } break;
    }
}
static void VarTableNotifyOperation(PCOMTOOLGRAPHICMAINWND gmInfo, NMHDR* pNotify)
{
    POINT posMouse;
    LPNMITEMACTIVATE pItem;
    RECT rcWin;
    BOOL nChosenId;
    UINT cMenuFlag;
    BOOL bSuccess = FALSE;
    
    PCOMTOOLGRAPHICPLOTINFO plotInfo;
    PCOMTOOLGRAPHICPLOTVAR pVar;
    HANINT nVarCnt;

    if (NM_RCLICK == pNotify->code)
    {
        nVarCnt = ListView_GetItemCount(gmInfo->varTable.hVarTable);
        plotInfo = (PCOMTOOLGRAPHICPLOTINFO)HANWinHeapAlloc(gmInfo->hHeap, NULL, sizeof(COMTOOLGRAPHICPLOTINFO));
        pVar = (PCOMTOOLGRAPHICPLOTVAR)HANWinHeapAlloc(gmInfo->hHeap, NULL, sizeof(COMTOOLGRAPHICPLOTVAR) * nVarCnt);
        if ((NULL == plotInfo) || (NULL == pVar))
        {
            if (NULL != plotInfo) { HANWinHeapFree(gmInfo->hHeap, 0, plotInfo); }
            if (NULL != pVar) { HANWinHeapFree(gmInfo->hHeap, 0, pVar); }
            MessageBox(gmInfo->hSelf, TEXT("内存不足"), NULL, 0);
        }
        else
        {
            plotInfo->pVar = pVar;
            plotInfo->nMsgId = gmInfo->msgList.iChosenMsg;
            UpdateChosenVar(gmInfo->varTable.hVarTable, plotInfo);

            GetWindowRect(gmInfo->varTable.hVarTable, &rcWin);
            pItem = (LPNMITEMACTIVATE)pNotify;
            posMouse.x = pItem->ptAction.x + rcWin.left;
            posMouse.y = pItem->ptAction.y + rcWin.top;

            gmInfo->varTable.hRightClickMenu = CreatePopupMenu();

            for (COMTOOLGRAPHICTYPE iLoop = 0; iLoop < COMTOOL_GRAPHIC_TYPE_CNT; iLoop++)
            {
                cMenuFlag = MF_STRING;
                if (FALSE == sg_pGraphicInfo[iLoop].OptionalCondition(gmInfo, plotInfo)) { cMenuFlag |= MF_GRAYED; }
                AppendMenu(gmInfo->varTable.hRightClickMenu, cMenuFlag, iLoop + 1, sg_pGraphicInfo[iLoop].pName);
            }

            nChosenId = TrackPopupMenu(
                gmInfo->varTable.hRightClickMenu,
                TPM_RETURNCMD,
                posMouse.x,
                posMouse.y,
                0,
                gmInfo->varTable.hVarTable,
                NULL
            );

            if (0 < nChosenId)
            {
                plotInfo->nPlotId = nChosenId - 1;
                if (TRUE == CreateNewGraphicWindow(gmInfo, nChosenId - 1, plotInfo)) { bSuccess = TRUE; }
            }
            if (FALSE == bSuccess)
            {
                HANWinHeapFree(gmInfo->hHeap, 0, plotInfo);
                HANWinHeapFree(gmInfo->hHeap, 0, pVar);
            }
        }
    }
}
static void UpdateChosenVar(HWND hVarTable, PCOMTOOLGRAPHICPLOTINFO plotInfo)
{
    HANINT nVarCnt = ListView_GetItemCount(hVarTable);

    plotInfo->nVarCnt = 0;
    for (HANINT iLoop = 0; iLoop < nVarCnt; iLoop++)
    {
        if (0 != ListView_GetItemState(hVarTable, iLoop, LVIS_SELECTED))
        {
            plotInfo->pVar[plotInfo->nVarCnt].nVarId = iLoop;
            plotInfo->nVarCnt++;
        }
    }
}
static BOOL CreateNewGraphicWindow(PCOMTOOLGRAPHICMAINWND gmInfo, HANINT nPlotId, PCOMTOOLGRAPHICPLOTINFO plotInfo)
{
    BOOL bRet = FALSE;
    HANALLOCDATA adAllocGraphic = { .AllocOperation = AllocGraphic, .pParam = gmInfo->hHeap, };

    TCITEM tiItem = { .mask = TCIF_TEXT, };
    HANPLIST pList = &(gmInfo->graphicWindow.listGraphic);

    PCOMTOOLGRAPHICMSG pMsg = &(gmInfo->ctInfo->graphicClass.pMsgInfo->pMsg[gmInfo->msgList.iChosenMsg]);

    for (HANINT iLoop = 0; iLoop < plotInfo->nVarCnt; iLoop++)
    {
        plotInfo->pVar[iLoop].pValueText = pMsg->pVar[plotInfo->pVar[iLoop].nVarId].pValue;
    }

    GetClientRect(gmInfo->graphicWindow.hTab, &(plotInfo->rcPos));
    plotInfo->rcPos.left += 2;
    plotInfo->rcPos.top += COM_TOOL_GRAPHIC_TAB_ITEM_SIZE_H;
    plotInfo->rcPos.right -= 2;
    plotInfo->rcPos.bottom -= 2;

    gmInfo->graphicName.pName = plotInfo->pName;
    tiItem.pszText = plotInfo->pName;
    HAN_strcpy(plotInfo->pName, TEXT(""));
    if (TRUE == CreateGraphicNameDialog(gmInfo))
    {
        plotInfo->hWindow = sg_pGraphicInfo[nPlotId].CreatePlot(gmInfo, plotInfo);
    
        if (RET_OK == HANListAppend(pList, plotInfo, &adAllocGraphic))
        {
            TabCtrl_InsertItem(gmInfo->graphicWindow.hTab, TabCtrl_GetItemCount(gmInfo->graphicWindow.hTab), &tiItem);
            ShowTabCurselGraphic(gmInfo);
            bRet = TRUE;
        }
        else
        {
            DestroyWindow(plotInfo->hWindow);
        }
    }

    return bRet;
}
static void GraphicTabNotifyOperation(PCOMTOOLGRAPHICMAINWND gmInfo, NMHDR* pNotify)
{
    switch (pNotify->code) {
        case TCN_SELCHANGE: {
            ShowTabCurselGraphic(gmInfo);
        } break;
        case NM_RCLICK: {
            TabRightClickOperation(gmInfo);
        } break;

        default: {
            /* idle */
        } break;
    }
}
static void ShowTabCurselGraphic(PCOMTOOLGRAPHICMAINWND gmInfo)
{
    HANINT nCursel;
    size_t iLoop;
    PCOMTOOLGRAPHICPLOTINFO plotInfo;

    nCursel = TabCtrl_GetCurSel(gmInfo->graphicWindow.hTab);
    iLoop = 0;

    ShowWindow(gmInfo->graphicWindow.hTip, SW_HIDE);

    HANListReForData(&gmInfo->graphicWindow.listGraphic);
    plotInfo = HANListForData(&gmInfo->graphicWindow.listGraphic);

    while (NULL != plotInfo)
    {
        if (nCursel == (HANINT)iLoop) { ShowWindow(plotInfo->hWindow, SW_SHOW); }
        else { ShowWindow(plotInfo->hWindow, SW_HIDE); }
        iLoop++;
        plotInfo = HANListForData(&gmInfo->graphicWindow.listGraphic);
    }
}
static void TabRightClickOperation(PCOMTOOLGRAPHICMAINWND gmInfo)
{
    TCHITTESTINFO tabTest;
    HANINT nTabId;
    POINT posMouse;
    UINT cMenuFlag;
    BOOL nMenuId;

    GetCursorPos(&posMouse);
    tabTest.pt = posMouse;
    ScreenToClient(gmInfo->graphicWindow.hTab, &(tabTest.pt));
    nTabId = TabCtrl_HitTest(gmInfo->graphicWindow.hTab, &tabTest);

    gmInfo->graphicWindow.hRightClickMenu = CreatePopupMenu();

    for (COMTOOLGRAPHICTABCLOSETYPE iLoop = 0; iLoop < COMTOOL_GRAPHIC_TAB_CLOSE_TYPE_CNT; iLoop++)
    {
        cMenuFlag = MF_STRING;
        AppendMenu(gmInfo->graphicWindow.hRightClickMenu, cMenuFlag, iLoop + 1, sg_pGraphicClose[iLoop].pCloseText);
    }

    nMenuId = TrackPopupMenu(
        gmInfo->graphicWindow.hRightClickMenu,
        TPM_RETURNCMD,
        posMouse.x,
        posMouse.y,
        0,
        gmInfo->graphicWindow.hTab,
        NULL
    );

    if (0 < nMenuId)
    {
        sg_pGraphicClose[nMenuId - 1].CloseOperation(gmInfo, nTabId);
    }
}
static void DestroyOperation(PCOMTOOLGRAPHICMAINWND gmInfo)
{
    HANPLIST pList = &(gmInfo->ctInfo->graphicClass.listGraphic);
    HANFREEDATA fdFreeGraphicListNode = {
        .FreeOperation = FreeGraphicListNode,
        .pParam = gmInfo->hHeap,
    };
    HANFREEDATA fdFreeGraphicPlotInfo = {
        .FreeOperation = FreeGraphicPlotInfo,
        .pParam = gmInfo->hHeap,
    };

    HANListDeleteAll(&(gmInfo->graphicWindow.listGraphic), &fdFreeGraphicListNode, &fdFreeGraphicPlotInfo);
    for (size_t iLoop = 0; iLoop < HANListGetLen(pList); iLoop++)
    {
        if (HANListGetData(pList, iLoop) == gmInfo->hSelf)
        {
            HANListDelete(pList, iLoop, &fdFreeGraphicListNode, NULL);
            break;
        }
    }
    HANWinHeapFree(gmInfo->hHeap, 0, gmInfo);
}
static BOOL CreateGraphicNameDialog(PCOMTOOLGRAPHICMAINWND gmInfo)
{
    BOOL bRet = FALSE;
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
        pDlgInfoTitle = (void*)(&pDialogInfo[nOffset]);
        pDlgInfoTitle->style = WS_POPUP | WS_BORDER | WS_SYSMENU | WS_BORDER | DS_MODALFRAME | WS_CAPTION;
        pDlgInfoTitle->cdit = 0;
        pDlgInfoTitle->x = 100;
        pDlgInfoTitle->y = 100;
        pDlgInfoTitle->cx = COMTOOL_GRAPHIC_NAME_DLU_W;
        pDlgInfoTitle->cy = COMTOOL_DLU_TEXT_H * 2 + 3 * COMTOOL_DLU_WINDOW_DY,
        nOffset += sizeof(DLGTEMPLATE);

        pWord = (void*)(&pDialogInfo[nOffset]);
        pWord[0] = 0;
        pWord[1] = 0;
        nOffset += sizeof(*pWord) * 2;
        pText = (void*)(&pDialogInfo[nOffset]);
        nTextLen = 1 + MultiByteToWideChar(CP_ACP, 0, "图形名字", -1, pText, 50);
        nOffset += (uint8_t)(sizeof(WCHAR) * nTextLen);
        
        GlobalUnlock(hData);
        bRet = (BOOL)DialogBoxIndirectParam(
            gmInfo->hInst, (LPDLGTEMPLATE)hData, gmInfo->hSelf,
            ComToolGraphicNameDialogProc, (LPARAM)gmInfo
        );
        GlobalFree(hData);
    }

    return bRet;
}
static void GraphicNameDialogInitCallback(HWND hGraphicName, LPARAM lParam)
{
    HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
    PCOMTOOLGRAPHICMAINWND gmInfo = (PCOMTOOLGRAPHICMAINWND)lParam;
    SetWindowLongPtr(hGraphicName, DWLP_USER, (LONG_PTR)gmInfo);

    RECT rcInput = {
        .left = COMTOOL_DLU_WINDOW_DX, .right = COMTOOL_GRAPHIC_NAME_DLU_W - COMTOOL_DLU_WINDOW_DX,
        .top = COMTOOL_DLU_WINDOW_DY, .bottom = COMTOOL_DLU_WINDOW_DX + COMTOOL_DLU_TEXT_H,
    };
    RECT rcOk = {
        .left = COMTOOL_GRAPHIC_NAME_DLU_W - 50, .right = COMTOOL_GRAPHIC_NAME_DLU_W - COMTOOL_DLU_WINDOW_DX,
        .top = COMTOOL_DLU_TEXT_H + (2 * COMTOOL_DLU_WINDOW_DY),
        .bottom = (COMTOOL_DLU_TEXT_H + COMTOOL_DLU_WINDOW_DY) * 2,
    };
    MapDialogRect(hGraphicName, &rcInput);
    MapDialogRect(hGraphicName, &rcOk);

    gmInfo->graphicName.hInput = CreateWindow(TEXT("edit"), NULL,
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        rcInput.left, rcInput.top,
        GetRectW(&rcInput), GetRectH(&rcInput),
        hGraphicName, (HMENU)WID_COM_TOOL_GRAPHIC_NAME_INPUT, hInst, NULL);
    gmInfo->graphicName.hOk = CreateWindow(TEXT("button"), TEXT("确定"),
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        rcOk.left, rcOk.top,
        GetRectW(&rcOk), GetRectH(&rcOk),
        hGraphicName, (HMENU)WID_COM_TOOL_GRAPHIC_NAME_OK, hInst, NULL);

    SendMessage(gmInfo->graphicName.hInput, WM_SETFONT, (WPARAM)(gmInfo->hFont.hSys), (LPARAM)TRUE);
    SendMessage(gmInfo->graphicName.hOk, WM_SETFONT, (WPARAM)(gmInfo->hFont.hSys), (LPARAM)TRUE);
}
static void GraphicNameDialogOkCallback(PCOMTOOLGRAPHICMAINWND gmInfo, HWND hGraphicName)
{
    GetWindowText(gmInfo->graphicName.hInput, gmInfo->graphicName.pName, COM_TOOL_GRAPHIC_GRAPHIC_NAME_SIZE);
    
    EndDialog(hGraphicName, TRUE);
}

static void* AllocGraphic(void* pOldData, size_t nNewSize, void* pParam)
{
    return HANWinHeapAlloc(pParam, pOldData, nNewSize);
}
static void FreeGraphicListNode(void* pData, void* pParam)
{
    HANWinHeapFree(pParam, 0, pData);
}
static void FreeGraphicPlotInfo(void* pData, void* pParam)
{
    PCOMTOOLGRAPHICPLOTINFO plotInfo = (PCOMTOOLGRAPHICPLOTINFO)pData;
    HANWinHeapFree(pParam, 0, plotInfo->pVar);
    HANWinHeapFree(pParam, 0, plotInfo);
}
static void DestroyGraphicWindow(void* pGraphicWindow, void* pParam)
{
    (void)pParam;
    PCOMTOOLGRAPHICPLOTINFO plotInfo = pGraphicWindow;
    HANWinHeapFree(pParam, 0, plotInfo->pVar);
    DestroyWindow(plotInfo->hWindow);
    HANWinHeapFree(pParam, 0, plotInfo);
}

static void GraphicTabCloseTarget(PCOMTOOLGRAPHICMAINWND gmInfo, HANINT nTabId)
{
    HANFREEDATA fdFreeGraphic = {
        .FreeOperation = FreeGraphicListNode,
        .pParam = gmInfo->hHeap,
    };
    HANFREEDATA fdDestroyGraphicWindow = {
        .FreeOperation = DestroyGraphicWindow,
        .pParam = gmInfo->hHeap,
    };

    HANListDelete(&(gmInfo->graphicWindow.listGraphic), (size_t)nTabId, &fdFreeGraphic, &fdDestroyGraphicWindow);
    TabCtrl_DeleteItem(gmInfo->graphicWindow.hTab, nTabId);
}
static void GraphicTabCloseOther(PCOMTOOLGRAPHICMAINWND gmInfo, HANINT nTabId)
{
    HANFREEDATA fdFreeGraphic = {
        .FreeOperation = FreeGraphicListNode,
        .pParam = gmInfo->hHeap,
    };
    HANFREEDATA fdDestroyGraphicWindow = {
        .FreeOperation = DestroyGraphicWindow,
        .pParam = NULL,
    };
    size_t nLen = HANListGetLen(&(gmInfo->graphicWindow.listGraphic)) - 1;
    HANINT nDeleteId;

    for (size_t iLoop = 0; iLoop < (size_t)nLen; iLoop++)
    {
        if (iLoop < (size_t)nTabId) { nDeleteId = 0; }
        else { nDeleteId = 1; }
        HANListDelete(&(gmInfo->graphicWindow.listGraphic), (size_t)nDeleteId, &fdFreeGraphic, &fdDestroyGraphicWindow);
        TabCtrl_DeleteItem(gmInfo->graphicWindow.hTab, nDeleteId);
    }
}
static void GraphicTabCloseLeft(PCOMTOOLGRAPHICMAINWND gmInfo, HANINT nTabId)
{
    HANFREEDATA fdFreeGraphic = {
        .FreeOperation = FreeGraphicListNode,
        .pParam = gmInfo->hHeap,
    };
    HANFREEDATA fdDestroyGraphicWindow = {
        .FreeOperation = DestroyGraphicWindow,
        .pParam = NULL,
    };

    for (size_t iLoop = 0; iLoop < (size_t)nTabId; iLoop++)
    {
        HANListDelete(&(gmInfo->graphicWindow.listGraphic), (size_t)0, &fdFreeGraphic, &fdDestroyGraphicWindow);
        TabCtrl_DeleteItem(gmInfo->graphicWindow.hTab, 0);
    }
}
static void GraphicTabCloseRight(PCOMTOOLGRAPHICMAINWND gmInfo, HANINT nTabId)
{
    HANFREEDATA fdFreeGraphic = {
        .FreeOperation = FreeGraphicListNode,
        .pParam = gmInfo->hHeap,
    };
    HANFREEDATA fdDestroyGraphicWindow = {
        .FreeOperation = DestroyGraphicWindow,
        .pParam = NULL,
    };
    size_t nLen = HANListGetLen(&(gmInfo->graphicWindow.listGraphic)) - 1;

    for (size_t iLoop = nTabId; iLoop < (size_t)nLen; iLoop++)
    {
        HANListDelete(&(gmInfo->graphicWindow.listGraphic), (size_t)(nTabId + 1), &fdFreeGraphic, &fdDestroyGraphicWindow);
        TabCtrl_DeleteItem(gmInfo->graphicWindow.hTab, nTabId + 1);
    }
}
static void GraphicTabCloseAll(PCOMTOOLGRAPHICMAINWND gmInfo, HANINT nTabId)
{
    (void)nTabId;
    HANFREEDATA fdFreeGraphic = {
        .FreeOperation = FreeGraphicListNode,
        .pParam = gmInfo->hHeap,
    };
    HANFREEDATA fdDestroyGraphicWindow = {
        .FreeOperation = DestroyGraphicWindow,
        .pParam = NULL,
    };
    size_t nLen = HANListGetLen(&(gmInfo->graphicWindow.listGraphic));

    for (size_t iLoop = 0; iLoop < nLen; iLoop++)
    {
        HANListDelete(&(gmInfo->graphicWindow.listGraphic), (size_t)0, &fdFreeGraphic, &fdDestroyGraphicWindow);
    }
    TabCtrl_DeleteAllItems(gmInfo->graphicWindow.hTab);
}

static BOOL ComToolGraphicPolyLineCondition(PCOMTOOLGRAPHICMAINWND gmInfo, PCOMTOOLGRAPHICPLOTINFO plotInfo)
{
    (void)gmInfo;
    BOOL bRet;
    
    if (0 < plotInfo->nVarCnt) { bRet = TRUE; }
    else { bRet = FALSE; }
    
    return bRet;
}
static BOOL ComToolGraphicScatterCondition(PCOMTOOLGRAPHICMAINWND gmInfo, PCOMTOOLGRAPHICPLOTINFO plotInfo)
{
    (void)gmInfo;
    BOOL bRet;
    
    if ((0 < (plotInfo->nVarCnt)) && (0 == ((plotInfo->nVarCnt) % 2))) { bRet = TRUE; }
    else { bRet = FALSE; }
    
    return bRet;
}

static HWND ComToolGraphicCreatePolyLine(PCOMTOOLGRAPHICMAINWND gmInfo, PCOMTOOLGRAPHICPLOTINFO plotInfo)
{
    HWND hRet;
    PCOMTOOLGRAPHICMSG pMsg = &(gmInfo->ctInfo->graphicClass.pMsgInfo->pMsg[plotInfo->nMsgId]);
    RECT* pWindowPos = &(plotInfo->rcPos);

    hRet = CreateWindow(
        HPL_CLASS, plotInfo->pName, WS_CHILD | HPLS_LINE,
        pWindowPos->left, pWindowPos->top, GetRectW(pWindowPos), GetRectH(pWindowPos), gmInfo->graphicWindow.hTab,
        (HMENU)1, gmInfo->hInst, NULL
    );

    if (NULL != hRet)
    {
        HANPolyLineSetBkBorder(hRet, RGB(0xFF, 0xFF, 0xFF), TRUE);
        HANPolyLineSetDataLen(hRet, gmInfo->dataHandle.nValueLen);

        for (HANINT iLoop = 0; iLoop < plotInfo->nVarCnt; iLoop++)
        {
            HANPolyLineAddVar(hRet, pMsg->pVar[plotInfo->pVar[iLoop].nVarId].pName, HPL_VAR_POS_LEFT);
        }
    }

    return hRet;
}
static HWND ComToolGraphicCreateScatter(PCOMTOOLGRAPHICMAINWND gmInfo, PCOMTOOLGRAPHICPLOTINFO plotInfo)
{
    HWND hRet;
    PCOMTOOLGRAPHICMSG pMsg = &(gmInfo->ctInfo->graphicClass.pMsgInfo->pMsg[plotInfo->nMsgId]);
    RECT* pWindowPos = &(plotInfo->rcPos);
    HANCHAR pPlotVarName[HAN_PLOT_STR_LEN_MAX + 1];
    HANPCSTR pMsgVarName[2];
    size_t nNameLen;

    hRet = CreateWindow(
        HST_CLASS, plotInfo->pName, WS_CHILD | HSTS_LINE | HSTS_MAP,
        pWindowPos->left, pWindowPos->top, GetRectW(pWindowPos), GetRectH(pWindowPos), gmInfo->graphicWindow.hTab,
        (HMENU)1, gmInfo->hInst, NULL
    );

    if (NULL != hRet)
    {
        HANScatterSetBkBorder(hRet, RGB(0xFF, 0xFF, 0xFF), TRUE);
        HANScatterSetDataLen(hRet, gmInfo->dataHandle.nValueLen);

        for (HANINT iLoop = 0; iLoop < plotInfo->nVarCnt; iLoop += 2)
        {
            pMsgVarName[0] = pMsg->pVar[plotInfo->pVar[iLoop].nVarId].pName;
            pMsgVarName[1] = pMsg->pVar[plotInfo->pVar[iLoop + 1].nVarId].pName;
            nNameLen = HAN_snprintf(
                pPlotVarName, HAN_PLOT_STR_LEN_MAX + 1, TEXT("%s - %s"), pMsgVarName[0], pMsgVarName[1]
            );
            if (HAN_PLOT_STR_LEN_MAX <= nNameLen) { HAN_strcpy(&pPlotVarName[HAN_PLOT_STR_LEN_MAX - 4], TEXT("...")); }
            HANScatterAddPointVar(hRet, pPlotVarName);
        }
    }

    return hRet;
}

static void ComToolGraphicPolyLinePushData(HWND hPlot, PCOMTOOLGRAPHICPLOTVAR pVar, HANINT nVarLen)
{
    HANPLOTDATA pdData = { .nLen = 1, };

    for (HANINT iLoop = 0; iLoop < nVarLen; iLoop++)
    {
        pdData.pData = &(pVar[iLoop].nValue);
        HANPolyLinePushEndDataById(hPlot, iLoop, &pdData);
    }
}
static void ComToolGraphicScatterPushData(HWND hPlot, PCOMTOOLGRAPHICPLOTVAR pVar, HANINT nVarLen)
{
    HANPLOTDATA pHData = { .nLen = 1, };
    HANPLOTDATA pVData = { .nLen = 1, };

    for (HANINT iLoop = 0; iLoop < (nVarLen / 2); iLoop++)
    {
        pHData.pData = &(pVar[iLoop * 2].nValue);
        pVData.pData = &(pVar[(iLoop * 2) + 1].nValue);
        HANScatterPushEndPointById(hPlot, iLoop, &pHData, &pVData);
    }
}
