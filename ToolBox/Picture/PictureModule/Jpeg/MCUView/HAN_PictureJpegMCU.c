#include <Windows.h>
#include <CommCtrl.h>

#include "HAN_PictureJpegMCU.h"
#include "..\HAN_PictureJpegMath.h"
#include "..\..\..\..\..\HAN_Lib\HAN_wingdi.h"

#define HAN_PICTURE_JPEG_MCU_BLOCK_CLASS            TEXT("HAN_PictureJpegMCUBlock")

typedef enum {
    PJMBM_NOTUSED = PCTM_MODELBASE,     // 不使用的消息值，自定义消息请在下方添加
    PJMBM_UPDATE,                       // 更新数据（WPARAM：未使用，LPARAM：未使用）
} PICTUREEDITTOOLMESSAGE;

typedef struct tagPICTUREJPEGMCUWNDEXTRA {
    HANDLE                          hHeap;
    HINSTANCE                       hInst;
    HWND                            hSelf;
    PICTUREJPEGMCUCREATEPARAM       jpegInfo;
    HWND                            hMCUId;
    HWND                            hMCUIdInput[2];
    HWND                            hDCT;
    HWND                            hQuantize;
    HWND                            hLeft;
    HWND                            hUp;
    HWND                            hRight;
    HWND                            hDown;
    HWND                            hUpdate;
    struct {
        HWND                        hTitle;
        HWND                        hTable;
    } qt[PICTURE_JPEG_SEGMENT_QT_MAX];
    HWND                            hComponentTab;
    struct {
        HFONT                       hHex;
        HFONT                       hSys;
    } hFont;                        /* 字体 */
} PICTUREJPEGMCUWNDEXTRA, * PPICTUREJPEGMCUWNDEXTRA;

typedef struct tagPICTUREJPEGMCUBLOCKWNDEXTRA
{
    PPICTUREJPEGMCUWNDEXTRA         pMCUInfo;
    uint8_t                         nComponent;
    HWND                            hSOSList;
    HWND                            hBlockName[PICTURE_JPEG_SEGMENT_SAMPLING_FACTOR_MAX * PICTURE_JPEG_SEGMENT_SAMPLING_FACTOR_MAX];
    HWND                            hBlockList[PICTURE_JPEG_SEGMENT_SAMPLING_FACTOR_MAX * PICTURE_JPEG_SEGMENT_SAMPLING_FACTOR_MAX];
    HWND                            hBlockPos[PICTURE_JPEG_SEGMENT_SAMPLING_FACTOR_MAX * PICTURE_JPEG_SEGMENT_SAMPLING_FACTOR_MAX];
} PICTUREJPEGMCUBLOCKWNDEXTRA, * PPICTUREJPEGMCUBLOCKWNDEXTRA;

static LRESULT CALLBACK PictureJpegMCUWndProc(HWND hPictureJpegMCU, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK PictureJpegMCUBlockWndProc(HWND hPictureJpegMCU, UINT message, WPARAM wParam, LPARAM lParam);
/* MCU */
static LRESULT CreateMCUCallback(HWND hPictureJpegMCU, LPARAM lParam);
static void CommandMCUCallback(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo, WPARAM wParam);
static LRESULT NotifyMCUCallback(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo, NMHDR* pNMHDR);
static void DestroyMCUCallback(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo);
static void InitMCUIdInput(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo);
static void InitQT(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo);
static void InitComponentTab(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo);
static void CreateMCUTable(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo);
static BOOL CheckStaticColorWindow(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo, HWND hWindow);
static void DCTCallback(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo);
static void QuantizeCallback(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo);
static void MCUDirLeftCallback(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo);
static void MCUDirUpCallback(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo);
static void MCUDirRightCallback(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo);
static void MCUDirDownCallback(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo);
static void UpdateCallback(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo);
/* Block */
static LRESULT CreateBlockCallback(HWND hPictureJpegMCUBlock, LPARAM lParam);
static void CommandBlockCallback(PPICTUREJPEGMCUBLOCKWNDEXTRA jpegMCUBlockInfo, WPARAM wParam);
static void DestroyBlockCallback(PPICTUREJPEGMCUBLOCKWNDEXTRA jpegMCUBlockInfo);
static void InitMCUBlock(HWND hListView);
static void UpdateBlockCallback(PPICTUREJPEGMCUBLOCKWNDEXTRA jpegMCUBlockInfo);
static void MCUBlockQuantize(PPICTUREJPEGMCUBLOCKWNDEXTRA jpegMCUBlockInfo, PICTUREJPEGDCTMATRIX2D pDest);
static void SosListCallback(PPICTUREJPEGMCUBLOCKWNDEXTRA jpegMCUBlockInfo, WPARAM wParam);

void RegisterHANPictureJpegMCU(HINSTANCE hInst)
{
    WNDCLASSEX wcex = {
        .cbSize         = sizeof(WNDCLASSEX),
        .style          = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc    = PictureJpegMCUWndProc,
        .cbClsExtra     = 0,
        .cbWndExtra     = sizeof(PPICTUREJPEGMCUWNDEXTRA),
        .hInstance      = hInst,
        .hIcon          = LoadIcon(NULL, IDI_INFORMATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName   = NULL,
        .lpszClassName  = HAN_PICTURE_JPEG_MCU_CLASS,
        .hIconSm        = NULL,
    };
    RegisterClassEx(&wcex);
    /* 注册块 */
    wcex.lpfnWndProc    = PictureJpegMCUBlockWndProc;
    wcex.hbrBackground  = (HBRUSH)GetStockObject(NULL_BRUSH);
    wcex.lpszClassName  = HAN_PICTURE_JPEG_MCU_BLOCK_CLASS;
    RegisterClassEx(&wcex);

    RegisterHANInput(hInst);
}

static LRESULT CALLBACK PictureJpegMCUWndProc(HWND hPictureJpegMCU, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo = (PPICTUREJPEGMCUWNDEXTRA)GetWindowLongPtr(hPictureJpegMCU, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = CreateMCUCallback(hPictureJpegMCU, lParam);
        } break;
        case WM_CTLCOLORSTATIC: {
            if (CheckStaticColorWindow(jpegMCUInfo, (HWND)lParam)) { lWndProcRet = (INT_PTR)GetStockObject(WHITE_BRUSH); }
            else { lWndProcRet = DefWindowProc(hPictureJpegMCU, message, wParam, lParam); }
        } break;
        case WM_COMMAND: {
            CommandMCUCallback(jpegMCUInfo, wParam);
        } break;
        case WM_NOTIFY: {
            lWndProcRet = NotifyMCUCallback(jpegMCUInfo, (NMHDR*)lParam);
        } break;
        case WM_DESTROY: {
            DestroyMCUCallback(jpegMCUInfo);
            lWndProcRet = DefWindowProc(hPictureJpegMCU, message, wParam, lParam);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hPictureJpegMCU, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}
static LRESULT CALLBACK PictureJpegMCUBlockWndProc(HWND hPictureJpegMCUBlock, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;

    // 读取属性
    PPICTUREJPEGMCUBLOCKWNDEXTRA jpegMCUBlockInfo = (PPICTUREJPEGMCUBLOCKWNDEXTRA)GetWindowLongPtr(hPictureJpegMCUBlock, 0);

    switch (message) {
        case WM_CREATE: {
            lWndProcRet = CreateBlockCallback(hPictureJpegMCUBlock, lParam);
        } break;
        case WM_COMMAND: {
            CommandBlockCallback(jpegMCUBlockInfo, wParam);
        } break;
        case WM_DESTROY: {
            DestroyBlockCallback(jpegMCUBlockInfo);
        } break;

        case PJMBM_UPDATE: {
            UpdateBlockCallback(jpegMCUBlockInfo);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hPictureJpegMCUBlock, message, wParam, lParam);
        } break;
    }

    return lWndProcRet;
}
/* MCU */
static LRESULT CreateMCUCallback(HWND hPictureJpegMCU, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo;
    HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
    PPICTUREJPEGMCUCREATEPARAM pJpegInfo = ((LPCREATESTRUCT)lParam)->lpCreateParams;
    PPICTURERESOLUTION pMCUBlockCnt = &(pJpegInfo->pSegmentInfo->SOFn.pxMCUBlockCnt);
    HANINT nWinX = PICTURE_WINDOW_DX;
    HANINT nWinY = PICTURE_WINDOW_DY;
    HANINT nWinW;
    HANINT nWinH;
    HANINT nMCUDirY;
    RECT rcWindow;

    HANDLE hHeap = GetProcessHeap();
    if (NULL == hHeap) { lWndProcRet = -1; }
    if (-1 != lWndProcRet)
    {
        jpegMCUInfo = (PPICTUREJPEGMCUWNDEXTRA)HANWinHeapAlloc(hHeap, NULL, sizeof(PICTUREJPEGMCUWNDEXTRA));
        if (NULL == jpegMCUInfo) { lWndProcRet = -1; }
    }

    if (-1 != lWndProcRet)
    {
        SetWindowLongPtr(hPictureJpegMCU, 0, (LONG_PTR)jpegMCUInfo);

        jpegMCUInfo->hHeap = hHeap;
        jpegMCUInfo->hInst = hInst;
        jpegMCUInfo->hSelf = hPictureJpegMCU;
        jpegMCUInfo->jpegInfo = *pJpegInfo;


        GetWindowRect(hPictureJpegMCU, &rcWindow);

        jpegMCUInfo->hFont.hSys = CreateFontIndirect(&g_lfSysFont);
        jpegMCUInfo->hFont.hHex = CreateFontIndirect(&g_lfHexFont);

        jpegMCUInfo->hMCUId = CreateWindow(TEXT("static"), TEXT("MCU ID"),
            WS_CHILD | WS_VISIBLE, nWinX, nWinY,
            100, 20,
            jpegMCUInfo->hSelf, (HMENU)WID_PICTURE_JPEG_MCU_VIEW_MCU_ID, hInst, NULL);
        nWinY += 25;
        for (HANSIZE iLoop = 0; iLoop < 2; iLoop++)
        {
            jpegMCUInfo->hMCUIdInput[iLoop] = CreateWindow(HINPUT_CLASS, NULL,
                WS_CHILD | WS_VISIBLE | HIPS_NUMBER, nWinX, nWinY,
                PICTURE_JPEG_MCU_VIEW_MCU_ID_INPUT_WIDTH, PICTURE_JPEG_MCU_VIEW_MCU_ID_INPUT_HEIGHT,
                jpegMCUInfo->hSelf, (HMENU)WID_PICTURE_JPEG_MCU_VIEW_X, hInst, NULL);
            nWinY += PICTURE_JPEG_MCU_VIEW_MCU_ID_INPUT_HEIGHT + PICTURE_WINDOW_DY;
        }
        nMCUDirY = nWinY;
        nWinW = 100;
        nWinH = 20;
        jpegMCUInfo->hDCT = CreateWindow(TEXT("button"), TEXT("DCT"),
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, nWinX, nWinY, nWinW, nWinH,
            jpegMCUInfo->hSelf, (HMENU)WID_PICTURE_JPEG_MCU_VIEW_DCT, hInst, NULL);
        nWinY += nWinH + PICTURE_WINDOW_DY;
        jpegMCUInfo->hQuantize = CreateWindow(TEXT("button"), TEXT("量化"),
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, nWinX, nWinY, nWinW, nWinH,
            jpegMCUInfo->hSelf, (HMENU)WID_PICTURE_JPEG_MCU_VIEW_QUANTIZE, hInst, NULL);
        nWinX += nWinW + (PICTURE_WINDOW_DX * 3);
        nWinY = nMCUDirY + PICTURE_BUTTON_HEIGHT + PICTURE_WINDOW_DY;
        nWinW = PICTURE_BUTTON_HEIGHT;
        nWinH = PICTURE_BUTTON_HEIGHT;
        jpegMCUInfo->hLeft = CreateWindow(TEXT("button"), TEXT("←"),
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, nWinX, nWinY, nWinW, nWinH,
            jpegMCUInfo->hSelf, (HMENU)WID_PICTURE_JPEG_MCU_VIEW_LEFT, hInst, NULL);
        nWinX += nWinW + PICTURE_WINDOW_DX;
        nWinY -= nWinH + PICTURE_WINDOW_DY;
        jpegMCUInfo->hUp = CreateWindow(TEXT("button"), TEXT("↑"),
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, nWinX, nWinY, nWinW, nWinH,
            jpegMCUInfo->hSelf, (HMENU)WID_PICTURE_JPEG_MCU_VIEW_UP, hInst, NULL);
        nWinX += nWinW + PICTURE_WINDOW_DX;
        nWinY += nWinH + PICTURE_WINDOW_DY;
        jpegMCUInfo->hRight = CreateWindow(TEXT("button"), TEXT("→"),
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, nWinX, nWinY, nWinW, nWinH,
            jpegMCUInfo->hSelf, (HMENU)WID_PICTURE_JPEG_MCU_VIEW_RIGHT, hInst, NULL);
        nWinX -= nWinW + PICTURE_WINDOW_DX;
        nWinY += nWinH + PICTURE_WINDOW_DY;
        jpegMCUInfo->hDown = CreateWindow(TEXT("button"), TEXT("↓"),
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, nWinX, nWinY, nWinW, nWinH,
            jpegMCUInfo->hSelf, (HMENU)WID_PICTURE_JPEG_MCU_VIEW_DOWN, hInst, NULL);
        nWinY -= nWinH + PICTURE_WINDOW_DY;
        jpegMCUInfo->hUpdate = CreateWindow(TEXT("button"), TEXT("√"),
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, nWinX, nWinY, nWinW, nWinH,
            jpegMCUInfo->hSelf, (HMENU)WID_PICTURE_JPEG_MCU_VIEW_UPDATE, hInst, NULL);
        nWinX = PICTURE_WINDOW_DX;
        nWinY += (nWinH + PICTURE_WINDOW_DY) * 2;
        nWinW = PICTURE_JPEG_MCU_VIEW_CELL_WIDTH * 8;
        /* 创建量化表 */
        for (HANSIZE iLoop = 0; iLoop < jpegMCUInfo->jpegInfo.pSegmentInfo->DQT.nQTCnt; iLoop++)
        {
            nWinH = PICTURE_JPEG_MCU_VIEW_CELL_HEIGHT;
            jpegMCUInfo->qt[iLoop].hTitle = CreateWindow(TEXT("static"), NULL,
                WS_CHILD | WS_VISIBLE | WS_BORDER | SS_LEFT | SS_CENTERIMAGE, nWinX, nWinY, nWinW, nWinH,
                jpegMCUInfo->hSelf, (HMENU)WID_PICTURE_JPEG_MCU_VIEW_QT, hInst, NULL);
            nWinY += nWinH;
            nWinH *= 8;
            jpegMCUInfo->qt[iLoop].hTable = CreateWindow(WC_LISTVIEW, NULL,
                WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_NOCOLUMNHEADER, nWinX, nWinY, nWinW, nWinH,
                jpegMCUInfo->hSelf, (HMENU)WID_PICTURE_JPEG_MCU_VIEW_QT, hInst, NULL);
            nWinY += nWinH + PICTURE_WINDOW_DY;
        }
        rcWindow.bottom = rcWindow.top + nWinY;
        /* 创建 MCU Tab */
        nWinX += nWinW + PICTURE_WINDOW_DX;
        nWinY = PICTURE_WINDOW_DY;
        nWinW = (((PICTURE_JPEG_MCU_VIEW_CELL_WIDTH * 8) + PICTURE_WINDOW_DX) * pMCUBlockCnt->pxWidth) + PICTURE_WINDOW_DX;
        nWinH = PICTURE_TAB_HEIGHT + PICTURE_WINDOW_DY + PICTURE_JPEG_MCU_VIEW_SOS_LIST_TEXT_HEIGHT
              + ((PICTURE_WINDOW_DY + (PICTURE_JPEG_MCU_VIEW_CELL_HEIGHT * 9) + PICTURE_JPEG_MCU_VIEW_POS_HEIGHT) * pMCUBlockCnt->pxHeight)
              + PICTURE_WINDOW_DY;
        jpegMCUInfo->hComponentTab = CreateWindow(WC_TABCONTROL, NULL,
            WS_CHILD | WS_VISIBLE, nWinX, nWinY, nWinW, nWinH,
            jpegMCUInfo->hSelf, (HMENU)WID_PICTURE_JPEG_MCU_VIEW_COMPONENT_TAB, hInst, NULL);
        nWinX += nWinW + PICTURE_WINDOW_DX;
        nWinY += nWinH + PICTURE_WINDOW_DY;
        rcWindow.right = rcWindow.left + nWinX;
        if (GetRectH(&rcWindow) < nWinY) { rcWindow.bottom = rcWindow.top + nWinY; }
        rcWindow.right += 16;
        rcWindow.bottom += 40;
        MoveWindow(hPictureJpegMCU, rcWindow.left, rcWindow.top, GetRectW(&rcWindow), GetRectH(&rcWindow), FALSE);

        SendMessage(jpegMCUInfo->hMCUId, WM_SETFONT, (WPARAM)(jpegMCUInfo->hFont.hSys), (LPARAM)TRUE);
        SendMessage(jpegMCUInfo->hDCT, WM_SETFONT, (WPARAM)(jpegMCUInfo->hFont.hSys), (LPARAM)TRUE);
        SendMessage(jpegMCUInfo->hQuantize, WM_SETFONT, (WPARAM)(jpegMCUInfo->hFont.hSys), (LPARAM)TRUE);
        SendMessage(jpegMCUInfo->hLeft, WM_SETFONT, (WPARAM)(jpegMCUInfo->hFont.hSys), (LPARAM)TRUE);
        SendMessage(jpegMCUInfo->hUp, WM_SETFONT, (WPARAM)(jpegMCUInfo->hFont.hSys), (LPARAM)TRUE);
        SendMessage(jpegMCUInfo->hRight, WM_SETFONT, (WPARAM)(jpegMCUInfo->hFont.hSys), (LPARAM)TRUE);
        SendMessage(jpegMCUInfo->hDown, WM_SETFONT, (WPARAM)(jpegMCUInfo->hFont.hSys), (LPARAM)TRUE);
        SendMessage(jpegMCUInfo->hUpdate, WM_SETFONT, (WPARAM)(jpegMCUInfo->hFont.hSys), (LPARAM)TRUE);
        for (HANSIZE iLoop = 0; iLoop < PICTURE_JPEG_SEGMENT_QT_MAX; iLoop++)
        {
            SendMessage(jpegMCUInfo->qt[iLoop].hTitle, WM_SETFONT, (WPARAM)(jpegMCUInfo->hFont.hSys), (LPARAM)TRUE);
            SendMessage(jpegMCUInfo->qt[iLoop].hTable, WM_SETFONT, (WPARAM)(jpegMCUInfo->hFont.hHex), (LPARAM)TRUE);
        }
        SendMessage(jpegMCUInfo->hComponentTab, WM_SETFONT, (WPARAM)(jpegMCUInfo->hFont.hSys), (LPARAM)TRUE);

        InitMCUIdInput(jpegMCUInfo);
        InitQT(jpegMCUInfo);
        InitComponentTab(jpegMCUInfo);
        CreateMCUTable(jpegMCUInfo);
        UpdateCallback(jpegMCUInfo);
    }

    return lWndProcRet;
}
static void CommandMCUCallback(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo, WPARAM wParam)
{
    switch (LOWORD(wParam)) {
        case WID_PICTURE_JPEG_MCU_VIEW_DCT: {
            DCTCallback(jpegMCUInfo);
        } break;
        case WID_PICTURE_JPEG_MCU_VIEW_QUANTIZE: {
            QuantizeCallback(jpegMCUInfo);
        } break;
        case WID_PICTURE_JPEG_MCU_VIEW_LEFT: {
            MCUDirLeftCallback(jpegMCUInfo);
        } break;
        case WID_PICTURE_JPEG_MCU_VIEW_UP: {
            MCUDirUpCallback(jpegMCUInfo);
        } break;
        case WID_PICTURE_JPEG_MCU_VIEW_RIGHT: {
            MCUDirRightCallback(jpegMCUInfo);
        } break;
        case WID_PICTURE_JPEG_MCU_VIEW_DOWN: {
            MCUDirDownCallback(jpegMCUInfo);
        } break;
        case WID_PICTURE_JPEG_MCU_VIEW_UPDATE: {
            UpdateCallback(jpegMCUInfo);
        } break;

        default: { } break;
    }
}
static LRESULT NotifyMCUCallback(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo, NMHDR* pNMHDR)
{
    LRESULT nRet = 0;
    INT nCurSel;
    INT nItemCnt;
    TCITEM tiItem = { .mask = TCIF_PARAM };

    if (WID_PICTURE_JPEG_MCU_VIEW_COMPONENT_TAB == pNMHDR->idFrom)
    {
        switch (pNMHDR->code) {
            case TCN_SELCHANGE: {
                nItemCnt = TabCtrl_GetItemCount(jpegMCUInfo->hComponentTab);
                nCurSel = TabCtrl_GetCurSel(jpegMCUInfo->hComponentTab);
                for (INT iLoop = 0; iLoop < nItemCnt; iLoop++)
                {
                    TabCtrl_GetItem(jpegMCUInfo->hComponentTab, iLoop, &tiItem);
                    if (NULL != (HWND)(tiItem.lParam))
                    {
                        if (nCurSel == iLoop) { ShowWindow((HWND)(tiItem.lParam), SW_SHOW); }
                        else { ShowWindow((HWND)(tiItem.lParam), SW_HIDE); }
                    }
                }
            } break;
        }
    }

    return nRet;
}
static void DestroyMCUCallback(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo)
{
    *(jpegMCUInfo->jpegInfo.pWindow) = NULL;
    HANWinHeapFree(jpegMCUInfo->hHeap, 0, jpegMCUInfo);
}
static void InitMCUIdInput(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo)
{
    HANINT nWinX;
    HANINT nWinY;
    HANINT nWinW;
    HANINT nWinH;
    HANCHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    PCPICTURERESOLUTION pMCUCnt = &(jpegMCUInfo->jpegInfo.pSegmentInfo->SOFn.pxMCUCnt);

    HANInputSetLeftTextText(jpegMCUInfo->hMCUIdInput[0], TEXT("X "));
    HANInputSetLeftTextText(jpegMCUInfo->hMCUIdInput[1], TEXT("Y "));
    HANInputSetInputText(jpegMCUInfo->hMCUIdInput[0], TEXT("0"));
    HANInputSetInputText(jpegMCUInfo->hMCUIdInput[1], TEXT("0"));
    HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("（0 ~ %u）"), pMCUCnt->pxWidth - 1);
    HANInputSetRightTextText(jpegMCUInfo->hMCUIdInput[0], pText);
    HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("（0 ~ %u）"), pMCUCnt->pxHeight - 1);
    HANInputSetRightTextText(jpegMCUInfo->hMCUIdInput[1], pText);
    for (HANSIZE iLoop = 0; iLoop < 2; iLoop++)
    {
        HANInputSetLeftTextFont(jpegMCUInfo->hMCUIdInput[iLoop], jpegMCUInfo->hFont.hSys, FALSE);
        HANInputSetInputFont(jpegMCUInfo->hMCUIdInput[iLoop], jpegMCUInfo->hFont.hHex, FALSE);
        HANInputSetRightTextFont(jpegMCUInfo->hMCUIdInput[iLoop], jpegMCUInfo->hFont.hHex, FALSE);
        nWinX = 0;
        nWinY = 0;
        nWinW = PICTURE_JPEG_MCU_VIEW_MCU_ID_INPUT_LEFT_TEXT_WIDTH;
        nWinH = PICTURE_JPEG_MCU_VIEW_MCU_ID_INPUT_HEIGHT;
        HANInputMoveLeftText(jpegMCUInfo->hMCUIdInput[iLoop], nWinX, nWinY, nWinW, nWinH, TRUE);
        nWinX += nWinW;
        nWinW = PICTURE_JPEG_MCU_VIEW_MCU_ID_INPUT_INPUT_WIDTH;
        HANInputMoveInput(jpegMCUInfo->hMCUIdInput[iLoop], nWinX, nWinY, nWinW, nWinH, TRUE);
        nWinX += nWinW;
        nWinW = PICTURE_JPEG_MCU_VIEW_MCU_ID_INPUT_RIGHT_TEXT_WIDTH;
        HANInputMoveRightText(jpegMCUInfo->hMCUIdInput[iLoop], nWinX, nWinY, nWinW, nWinH, TRUE);
    }
}
static void InitQT(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo)
{
    PICTUREJPEGQUANTTABLE* pQT = jpegMCUInfo->jpegInfo.pSegmentInfo->DQT.pQT;
    HANCHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE] = TEXT("");
    LVCOLUMN lvColum = {
        .mask = LVCF_FMT | LVCF_WIDTH | LVCF_SUBITEM,
        .fmt = LVCFMT_RIGHT,
        .cx = 0,
    };
    LVITEM lvCell = {
        .mask = LVIF_TEXT,
        .pszText = pText,
    };

    for (HANSIZE iLoop = 0; iLoop < jpegMCUInfo->jpegInfo.pSegmentInfo->DQT.nQTCnt; iLoop++)
    {
        HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("量化表" HANSIZE_PRINT_FORMAT), iLoop);
        pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE - 1] = TEXT('\0');
        SetWindowText(jpegMCUInfo->qt[iLoop].hTitle, pText);

        lvColum.iSubItem = 0;
        lvColum.cx = 0;
        ListView_InsertColumn(jpegMCUInfo->qt[iLoop].hTable, 0, &lvColum);
        lvColum.cx = PICTURE_JPEG_MCU_VIEW_CELL_WIDTH;
        for (HANINT jLoop = 0; jLoop < 8; jLoop++)
        {
            lvColum.iSubItem = jLoop + 1;
            ListView_InsertColumn(jpegMCUInfo->qt[iLoop].hTable, jLoop + 1, &lvColum);
        }
        for (HANINT nRow = 0; nRow < 8; nRow++)
        {
            lvCell.iItem = nRow;
            lvCell.iSubItem = 0;
            ListView_InsertItem(jpegMCUInfo->qt[iLoop].hTable, &lvCell);
            for (HANINT nCol = 0; nCol < 8; nCol++)
            {
                HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), pQT[iLoop][nRow][nCol]);
                lvCell.iSubItem = nCol + 1;
                ListView_SetItem(jpegMCUInfo->qt[iLoop].hTable, &lvCell);
            }
        }
    }
}
static void InitComponentTab(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo)
{
    uint8_t nComponentCnt = jpegMCUInfo->jpegInfo.pSegmentInfo->SOFn.nComponentCnt;
    HANCHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    TCITEM tiItem = {
        .mask = TCIF_TEXT,
        .pszText = pText,
    };

    for (uint8_t iLoop = 0; iLoop < nComponentCnt; iLoop++)
    {
        HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("分量%u"), (uint16_t)iLoop + (uint16_t)1);
        TabCtrl_InsertItem(jpegMCUInfo->hComponentTab, iLoop, &tiItem);
    }

    TabCtrl_SetCurSel(jpegMCUInfo->hComponentTab, 0);
}
static void CreateMCUTable(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo)
{
    PPICTUREJPEGSEGMENTINFO pSegmentInfo = jpegMCUInfo->jpegInfo.pSegmentInfo;
    uint8_t nComponentCnt =pSegmentInfo->SOFn.nComponentCnt;
    RECT rcTab;
    DWORD nMCUBlockStyle = WS_CHILD | WS_VISIBLE;
    TCITEM tiItem = { .mask = TCIF_PARAM, };
    PICTUREJPEGMCUBLOCKWNDEXTRA jpegMCUBlockInfo = { .pMCUInfo = jpegMCUInfo, };

    GetClientRect(jpegMCUInfo->hComponentTab, &rcTab);
    rcTab.left += 1;
    rcTab.top += PICTURE_TAB_HEIGHT;
    rcTab.right -= 1;
    rcTab.bottom -= 1;

    for (uint8_t iLoop = 0; iLoop < nComponentCnt; iLoop++)
    {
        jpegMCUBlockInfo.nComponent = iLoop;
        tiItem.lParam = (LPARAM)CreateWindow(HAN_PICTURE_JPEG_MCU_BLOCK_CLASS, NULL,
            nMCUBlockStyle, rcTab.left, rcTab.top, GetRectW(&rcTab), GetRectH(&rcTab),
            jpegMCUInfo->hComponentTab, (HMENU)WID_PICTURE_JPEG_MCU_VIEW_BLOCK, jpegMCUInfo->hInst, &jpegMCUBlockInfo);
        TabCtrl_SetItem(jpegMCUInfo->hComponentTab, iLoop, &tiItem);
        nMCUBlockStyle = WS_CHILD;
    }
}
static BOOL CheckStaticColorWindow(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo, HWND hWindow)
{
    BOOL bRet = FALSE;

    if ((hWindow == jpegMCUInfo->hMCUId) ||
        (hWindow == jpegMCUInfo->hDCT) ||
        (hWindow == jpegMCUInfo->hQuantize))
    {
        bRet = TRUE;
    }

    return bRet;
}
static void DCTCallback(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo)
{
    HANINT nChecked = ButtonGetCheck(jpegMCUInfo->hDCT);

    if (BST_UNCHECKED == nChecked) { ButtonSetUnchecked(jpegMCUInfo->hQuantize); }
    UpdateCallback(jpegMCUInfo);
}
static void QuantizeCallback(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo)
{
    HANINT nChecked = ButtonGetCheck(jpegMCUInfo->hQuantize);

    if (BST_CHECKED == nChecked) { ButtonSetChecked(jpegMCUInfo->hDCT); }
    UpdateCallback(jpegMCUInfo);
}
static void MCUDirLeftCallback(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo)
{
    HANCHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE] = TEXT("");
    uint32_t nMCUX;

    GetWindowText(jpegMCUInfo->hMCUIdInput[0], pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    nMCUX = HAN_strtoul(pText, NULL, 10);
    if (0 < nMCUX) { nMCUX--; }
    HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), nMCUX);
    pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE - 1] = TEXT('\0');
    SetWindowText(jpegMCUInfo->hMCUIdInput[0], pText);

    UpdateCallback(jpegMCUInfo);
}
static void MCUDirUpCallback(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo)
{
    HANCHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE] = TEXT("");
    uint32_t nMCUY;

    GetWindowText(jpegMCUInfo->hMCUIdInput[1], pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    nMCUY = HAN_strtoul(pText, NULL, 10);
    if (0 < nMCUY) { nMCUY--; }
    HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), nMCUY);
    pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE - 1] = TEXT('\0');
    SetWindowText(jpegMCUInfo->hMCUIdInput[1], pText);

    UpdateCallback(jpegMCUInfo);
}
static void MCUDirRightCallback(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo)
{
    HANCHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE] = TEXT("");
    uint32_t nMCUX;

    GetWindowText(jpegMCUInfo->hMCUIdInput[0], pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    nMCUX = HAN_strtoul(pText, NULL, 10);
    if (nMCUX < (jpegMCUInfo->jpegInfo.pSegmentInfo->SOFn.pxMCUCnt.pxWidth - 1)) { nMCUX++; }
    HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), nMCUX);
    pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE - 1] = TEXT('\0');
    SetWindowText(jpegMCUInfo->hMCUIdInput[0], pText);

    UpdateCallback(jpegMCUInfo);
}
static void MCUDirDownCallback(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo)
{
    HANCHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE] = TEXT("");
    uint32_t nMCUY;

    GetWindowText(jpegMCUInfo->hMCUIdInput[1], pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    nMCUY = HAN_strtoul(pText, NULL, 10);
    if (nMCUY < (jpegMCUInfo->jpegInfo.pSegmentInfo->SOFn.pxMCUCnt.pxHeight - 1)) { nMCUY++; }
    HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), nMCUY);
    pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE - 1] = TEXT('\0');
    SetWindowText(jpegMCUInfo->hMCUIdInput[1], pText);

    UpdateCallback(jpegMCUInfo);
}
static void UpdateCallback(PPICTUREJPEGMCUWNDEXTRA jpegMCUInfo)
{
    PPICTUREJPEGSEGMENTINFO pSegmentInfo = jpegMCUInfo->jpegInfo.pSegmentInfo;
    PPICTURERESOLUTION pMCUCnt = &(pSegmentInfo->SOFn.pxMCUCnt);
    HWND hComponent;
    PICTURERESOLUTION pxMCUPos;
    HANCHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE] = TEXT("");
    TCITEM tiItem = { .mask = TCIF_PARAM, };
    /* 检查边界 */
    GetWindowText(jpegMCUInfo->hMCUIdInput[0], pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    pxMCUPos.pxWidth = HAN_strtoul(pText, NULL, 10);
    GetWindowText(jpegMCUInfo->hMCUIdInput[1], pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    pxMCUPos.pxHeight = HAN_strtoul(pText, NULL, 10);
    if (pMCUCnt->pxWidth <= pxMCUPos.pxWidth)
    {
        HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), pMCUCnt->pxWidth - 1);
        pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE - 1] = TEXT('\0');
        SetWindowText(jpegMCUInfo->hMCUIdInput[0], pText);
    }
    if (pMCUCnt->pxHeight <= pxMCUPos.pxHeight)
    {
        HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%u"), pMCUCnt->pxHeight - 1);
        pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE - 1] = TEXT('\0');
        SetWindowText(jpegMCUInfo->hMCUIdInput[1], pText);
    }
    /* 发送更新消息 */
    for (HANINT iLoop = 0; iLoop < pSegmentInfo->SOFn.nComponentCnt; iLoop++)
    {
        TabCtrl_GetItem(jpegMCUInfo->hComponentTab, iLoop, &tiItem);
        hComponent = (HWND)tiItem.lParam; { if (NULL != hComponent) SendMessage(hComponent, PJMBM_UPDATE, (WPARAM)0, (LPARAM)0); }
    }
}
/* Block */
static LRESULT CreateBlockCallback(HWND hPictureJpegMCUBlock, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    PPICTUREJPEGMCUBLOCKWNDEXTRA jpegMCUBlockInfo;
    PPICTUREJPEGMCUBLOCKWNDEXTRA pCreateParam = ((LPCREATESTRUCT)lParam)->lpCreateParams;
    PPICTUREJPEGSEGMENTINFO pSegmentInfo = pCreateParam->pMCUInfo->jpegInfo.pSegmentInfo;
    HANCHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE];
    PPICTUREJPEGMCUTABLE pMCUTable;
    uint32_t nBlockCnt;
    HANINT nWinX;
    HANINT nWinY;
    HANINT nWinW;
    HANINT nWinH;

    jpegMCUBlockInfo = (PPICTUREJPEGMCUBLOCKWNDEXTRA)HANWinHeapAlloc(pCreateParam->pMCUInfo->hHeap, NULL, sizeof(PICTUREJPEGMCUBLOCKWNDEXTRA));
    if (NULL == jpegMCUBlockInfo) { lWndProcRet = -1; }

    if (-1 != lWndProcRet)
    {
        SetWindowLongPtr(hPictureJpegMCUBlock, 0, (LONG_PTR)jpegMCUBlockInfo);

        *jpegMCUBlockInfo = *pCreateParam;
        /* 先创建选项下拉框 */
        pMCUTable = &(pCreateParam->pMCUInfo->jpegInfo.pMCUTable[pCreateParam->nComponent]);
        nWinX = PICTURE_WINDOW_DX;
        nWinY = PICTURE_WINDOW_DY;
        nWinW = PICTURE_JPEG_MCU_VIEW_SOS_LIST_WIDTH;
        nWinH = PICTURE_JPEG_MCU_VIEW_SOS_LIST_TEXT_HEIGHT * (HANINT)(pMCUTable->nCnt + 1);
        jpegMCUBlockInfo->hSOSList = CreateWindow(TEXT("combobox"), NULL,
            WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, nWinX, nWinY, nWinW, nWinH,
            hPictureJpegMCUBlock, (HMENU)WID_PICTURE_JPEG_MCU_VIEW_BLOCK_SOS_LIST, jpegMCUBlockInfo->pMCUInfo->hInst, NULL
        );
        SendMessage(jpegMCUBlockInfo->hSOSList, WM_SETFONT, (WPARAM)(jpegMCUBlockInfo->pMCUInfo->hFont.hSys), (LPARAM)TRUE);
        for (HANSIZE iLoop = 0; iLoop < pMCUTable->nCnt; iLoop++)
        {
            HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE,
                TEXT("频谱%u~%u，渐进位%u~%u"),
                pMCUTable->pInfo[iLoop].nSs, pMCUTable->pInfo[iLoop].nSe,
                pMCUTable->pInfo[iLoop].nAh, pMCUTable->pInfo[iLoop].nAl
            );
            ComboBoxAddString(jpegMCUBlockInfo->hSOSList, pText);
        }
        ComboBoxSetCursel(jpegMCUBlockInfo->hSOSList, 0);
        /* 创建 Block 和对应的位置文本 */
        nBlockCnt = 0;
        nWinY += PICTURE_JPEG_MCU_VIEW_SOS_LIST_TEXT_HEIGHT + PICTURE_WINDOW_DY;
        nWinW = PICTURE_JPEG_MCU_VIEW_CELL_WIDTH * 8;
        for (uint8_t nRow = 0; nRow < pSegmentInfo->SOFn.pSamplingCoe[pCreateParam->nComponent][1]; nRow++)
        {
            nWinX = PICTURE_WINDOW_DX;
            for (uint8_t nCol = 0; nCol < pSegmentInfo->SOFn.pSamplingCoe[pCreateParam->nComponent][0]; nCol++)
            {
                nWinH = PICTURE_JPEG_MCU_VIEW_CELL_HEIGHT;
                jpegMCUBlockInfo->hBlockName[nBlockCnt] = CreateWindow(TEXT("static"), NULL,
                    WS_CHILD | WS_VISIBLE | WS_BORDER | SS_ENDELLIPSIS, nWinX, nWinY, nWinW, nWinH,
                    hPictureJpegMCUBlock, (HMENU)WID_PICTURE_JPEG_MCU_VIEW_BLOCK_NAME, jpegMCUBlockInfo->pMCUInfo->hInst, NULL
                );
                nWinY += PICTURE_JPEG_MCU_VIEW_CELL_HEIGHT;
                nWinH = PICTURE_JPEG_MCU_VIEW_CELL_HEIGHT * 8;
                jpegMCUBlockInfo->hBlockList[nBlockCnt] = CreateWindow(WC_LISTVIEW, NULL,
                    WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_NOCOLUMNHEADER, nWinX, nWinY, nWinW, nWinH,
                    hPictureJpegMCUBlock, (HMENU)WID_PICTURE_JPEG_MCU_VIEW_BLOCK_LIST, jpegMCUBlockInfo->pMCUInfo->hInst, NULL
                );
                nWinY += nWinH;
                nWinH = PICTURE_JPEG_MCU_VIEW_POS_HEIGHT;
                jpegMCUBlockInfo->hBlockPos[nBlockCnt] = CreateWindow(TEXT("static"), NULL,
                    WS_CHILD | WS_VISIBLE, nWinX, nWinY, nWinW, nWinH,
                    hPictureJpegMCUBlock, (HMENU)WID_PICTURE_JPEG_MCU_VIEW_BLOCK_POS, jpegMCUBlockInfo->pMCUInfo->hInst, NULL
                );
                nWinX += nWinW + PICTURE_WINDOW_DY;
                nWinY -= PICTURE_JPEG_MCU_VIEW_CELL_HEIGHT * 9;

                SendMessage(jpegMCUBlockInfo->hBlockName[nBlockCnt], WM_SETFONT, (WPARAM)(jpegMCUBlockInfo->pMCUInfo->hFont.hSys), (LPARAM)TRUE);
                SendMessage(jpegMCUBlockInfo->hBlockList[nBlockCnt], WM_SETFONT, (WPARAM)(jpegMCUBlockInfo->pMCUInfo->hFont.hSys), (LPARAM)TRUE);
                SendMessage(jpegMCUBlockInfo->hBlockPos[nBlockCnt], WM_SETFONT, (WPARAM)(jpegMCUBlockInfo->pMCUInfo->hFont.hSys), (LPARAM)TRUE);

                InitMCUBlock(jpegMCUBlockInfo->hBlockList[nBlockCnt]);

                nBlockCnt++;
            }
            nWinY += (PICTURE_JPEG_MCU_VIEW_CELL_HEIGHT * 9) + PICTURE_JPEG_MCU_VIEW_POS_HEIGHT + PICTURE_WINDOW_DY;
        }
    }

    return lWndProcRet;
}
static void CommandBlockCallback(PPICTUREJPEGMCUBLOCKWNDEXTRA jpegMCUBlockInfo, WPARAM wParam)
{
    switch (LOWORD(wParam)) {
        case WID_PICTURE_JPEG_MCU_VIEW_BLOCK_SOS_LIST: {
            SosListCallback(jpegMCUBlockInfo, wParam);
        } break;

        default: { } break;
    }
}
static void DestroyBlockCallback(PPICTUREJPEGMCUBLOCKWNDEXTRA jpegMCUBlockInfo)
{
    HANWinHeapFree(jpegMCUBlockInfo->pMCUInfo->hHeap, 0, jpegMCUBlockInfo);
}
static void InitMCUBlock(HWND hListView)
{
    LVCOLUMN lvColum = {
        .mask = LVCF_FMT | LVCF_WIDTH | LVCF_SUBITEM,
        .fmt = LVCFMT_RIGHT,
        .cx = 0,
    };
    LVITEM lvCell = {
        .mask = 0,
    };

    lvColum.iSubItem = 0;
    ListView_InsertColumn(hListView, 0, &lvColum);
    lvColum.cx = PICTURE_JPEG_MCU_VIEW_CELL_WIDTH;
    for (HANINT jLoop = 0; jLoop < 8; jLoop++)
    {
        lvColum.iSubItem = jLoop + 1;
        ListView_InsertColumn(hListView, jLoop + 1, &lvColum);
    }
    for (HANINT jLoop = 0; jLoop < 8; jLoop++)
    {
        lvCell.iItem = jLoop;
        lvCell.iSubItem = 0;
        ListView_InsertItem(hListView, &lvCell);
    }
}
static void UpdateBlockCallback(PPICTUREJPEGMCUBLOCKWNDEXTRA jpegMCUBlockInfo)
{
    PPICTUREJPEGMCUWNDEXTRA pMCUInfo = jpegMCUBlockInfo->pMCUInfo;
    PPICTUREJPEGSEGMENTINFO pSegmentInfo = pMCUInfo->jpegInfo.pSegmentInfo;
    HANPPICTURE pPicture = pMCUInfo->jpegInfo.pPicture;
    uint8_t nComponent = jpegMCUBlockInfo->nComponent;
    uint8_t nMCUComponentBlockTotal = pSegmentInfo->SOFn.nMCUComponentBlockTotal;
    uint8_t nMCUComponentBlockCnt = pSegmentInfo->SOFn.pMCUComponentBlockCnt[jpegMCUBlockInfo->nComponent];
    uint8_t* pMCUComponentBlockOffset = pSegmentInfo->SOFn.pMCUComponentBlockOffset;
    uint8_t* pSamplingCoe = pSegmentInfo->SOFn.pSamplingCoe[jpegMCUBlockInfo->nComponent];
    HANINT nSOSId = ComboBoxGetCursel(jpegMCUBlockInfo->hSOSList);
    PPICTUREJPEGMCUINFO pMCUBlockInfo = &(pMCUInfo->jpegInfo.pMCUTable[jpegMCUBlockInfo->nComponent].pInfo[nSOSId]);
    PPICTURERESOLUTION pResolution = &(pPicture->pxResolution);
    PPICTURERESOLUTION pMCUSize = &(pSegmentInfo->SOFn.pxMCUSize);
    PPICTURERESOLUTION pMCUCnt = &(pSegmentInfo->SOFn.pxMCUCnt);
    PPICTURERESOLUTION pMCUBlockCnt = &(pSegmentInfo->SOFn.pxMCUBlockCnt);
    PICTURERESOLUTION pxMCUPos;
    PICTURERESOLUTION pxBlockSize;
    PICTURERESOLUTION pxBlock1Pos;
    PICTURERESOLUTION pxBlocknPos;
    PICTURERESOLUTION pxSampleStep;
    PICTURERESOLUTION pxSample;
    uint32_t nBlockCnt;
    HANSIZE nMCUId;
    HANSIZE nBlockId;
    HANSIZE nComponentBlockId;
    PICTUREJPEGDCTMATRIX2D pOutput;
    HANINT nDCT;
    HANINT nQuantize;
    HANCHAR pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE] = TEXT("");
    LVITEM lvCell = {
        .mask = LVIF_TEXT,
        .pszText = pText,
    };

    GetWindowText(pMCUInfo->hMCUIdInput[0], pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    pxMCUPos.pxWidth = HAN_strtoul(pText, NULL, 10);
    GetWindowText(pMCUInfo->hMCUIdInput[1], pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE);
    pxMCUPos.pxHeight = HAN_strtoul(pText, NULL, 10);
    nMCUId = (HANSIZE)(pxMCUPos.pxHeight * pMCUCnt->pxWidth) + (HANSIZE)(pxMCUPos.pxWidth);
    nBlockId = nMCUId * nMCUComponentBlockTotal + pMCUComponentBlockOffset[nComponent];
    nComponentBlockId = nMCUId * nMCUComponentBlockCnt;

    pxBlock1Pos.pxWidth = pxMCUPos.pxWidth * pMCUSize->pxWidth;
    pxBlock1Pos.pxHeight = pxMCUPos.pxHeight * pMCUSize->pxHeight;

    pxSampleStep.pxWidth = pMCUBlockCnt->pxWidth / pSamplingCoe[0];
    pxSampleStep.pxHeight = pMCUBlockCnt->pxHeight / pSamplingCoe[1];
    pxBlockSize.pxWidth = pxSampleStep.pxWidth << 3;    // 等效于乘以 8
    pxBlockSize.pxHeight = pxSampleStep.pxHeight << 3;  // 等效于乘以 8

    nBlockCnt = 0;
    for (uint8_t nRow = 0; nRow < pSamplingCoe[1]; nRow++)
    {
        for (uint8_t nCol = 0; nCol < pSamplingCoe[0]; nCol++)
        {
            pxBlocknPos.pxWidth = pxBlock1Pos.pxWidth + (nCol * pxBlockSize.pxWidth);
            pxBlocknPos.pxHeight = pxBlock1Pos.pxHeight + (nRow * pxBlockSize.pxHeight);
            HAN_snprintf(
                pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE,
                TEXT("横向：%u~%u, 纵向：%u~%u"),
                pxBlocknPos.pxWidth, pxBlocknPos.pxWidth + pxBlockSize.pxWidth - 1,
                pxBlocknPos.pxHeight, pxBlocknPos.pxHeight + pxBlockSize.pxHeight - 1
            );
            pText[HAN_PICTURE_JPEG_TEXT_BUF_SIZE - 1] = TEXT('\0');
            SetWindowText(jpegMCUBlockInfo->hBlockName[nBlockCnt], pText);
            
            memcpy(pOutput, jpegMCUBlockInfo->pMCUInfo->jpegInfo.pComponent[nBlockId], sizeof(pOutput));
            nDCT = ButtonGetCheck(jpegMCUBlockInfo->pMCUInfo->hDCT);
            nQuantize = ButtonGetCheck(jpegMCUBlockInfo->pMCUInfo->hQuantize);
            if (BST_CHECKED == nDCT) { memcpy(pOutput, jpegMCUBlockInfo->pMCUInfo->jpegInfo.pDCT[nBlockId], sizeof(pOutput)); }
            if (BST_CHECKED == nQuantize) { MCUBlockQuantize(jpegMCUBlockInfo, pOutput); }

            pxSample.pxHeight = pxBlocknPos.pxHeight;
            for (uint8_t nX = 0; nX < 8; nX++)
            {
                pxSample.pxWidth = pxBlocknPos.pxWidth;
                for (uint8_t nY = 0; nY < 8; nY++)
                {
                    if ((pResolution->pxHeight <= pxSample.pxHeight) || (pResolution->pxWidth <= pxSample.pxWidth))
                    {
                        HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("x"));
                    }
                    else { HAN_snprintf(pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE, TEXT("%d"), pOutput[nX][nY]); }

                    lvCell.iItem = nX;
                    lvCell.iSubItem = nY + 1;
                    ListView_SetItem(jpegMCUBlockInfo->hBlockList[nBlockCnt], &lvCell);

                    pxSample.pxWidth += pxSampleStep.pxWidth;
                }
                pxSample.pxHeight += pxSampleStep.pxHeight;
            }

            if (TRUE == pMCUBlockInfo->pBlockPos[nComponentBlockId].bValid)
            {
                HAN_snprintf(
                    pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE,
                    TEXT("起始字节：" HANSIZE_PRINT_FORMAT "\r\n起始比特：%u\r\n比特个数：" HANSIZE_PRINT_FORMAT),
                    pMCUBlockInfo->pBlockPos[nComponentBlockId].nByteStart,
                    pMCUBlockInfo->pBlockPos[nComponentBlockId].nBitStart,
                    pMCUBlockInfo->pBlockPos[nComponentBlockId].nBitCnt
                );
            }
            else
            {
                HAN_snprintf(
                    pText, HAN_PICTURE_JPEG_TEXT_BUF_SIZE,
                    TEXT("起始字节：" PICTURE_INFO_INVALID_TEXT "\r\n起始比特：" PICTURE_INFO_INVALID_TEXT "\r\n比特个数：" PICTURE_INFO_INVALID_TEXT)
                );
            }
            SetWindowText(jpegMCUBlockInfo->hBlockPos[nBlockCnt], pText);
            
            nBlockId++;
            nBlockCnt++;
            nComponentBlockId++;
        }
    }
}
static void MCUBlockQuantize(PPICTUREJPEGMCUBLOCKWNDEXTRA jpegMCUBlockInfo, PICTUREJPEGDCTMATRIX2D pDest)
{
    PCPICTUREJPEGSEGMENTINFO pSegmentInfo = jpegMCUBlockInfo->pMCUInfo->jpegInfo.pSegmentInfo;
    uint8_t nComponent = jpegMCUBlockInfo->nComponent;
    const PICTUREJPEGQUANTTABLE* pQT = &(pSegmentInfo->DQT.pQT[pSegmentInfo->SOFn.pQuantTblId[nComponent]]);

    for (HANSIZE iLoop = 0; iLoop < 8; iLoop++)
    {
        for (HANSIZE jLoop = 0; jLoop < 8; jLoop++)
        {
            pDest[iLoop][jLoop] /= (*pQT)[iLoop][jLoop];
        }
    }
}
static void SosListCallback(PPICTUREJPEGMCUBLOCKWNDEXTRA jpegMCUBlockInfo, WPARAM wParam)
{
    switch (HIWORD(wParam)) {
        case CBN_SELENDOK: {
            UpdateBlockCallback(jpegMCUBlockInfo);
        } break;

        default: { } break;
    }
}
