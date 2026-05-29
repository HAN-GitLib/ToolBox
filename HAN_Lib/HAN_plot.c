#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <windows.h>
#include <stdlib.h>

#include "HAN_plot.h"

#if 1 /******************** HANPlot ********************/
#define HAN_PI          3.14159265358979323846

#define COS_18_DEGREE   0.951056516295153572116
#define SIN_18_DEGREE   0.309016994374947424102
#define TAN_18_DEGREE   0.324919696232906326155
#define TAN_30_DEGREE   0.577350269189625764509
// #define COT_30_DEGREE   1.73205080756887729352
// #define TAN_54_DEGREE   1.37638192047117353820
#define COS_54_DEGREE   0.587785252292473129168
#define COS_72_DEGREE   0.309016994374947424102
#define SIN_72_DEGREE   0.951056516295153572116
// HANPlot enum 定义
typedef enum { // 光标模式
    HPLOT_CM_NORMAL,
    HPLOT_CM_MOVE,
    HPLOT_CM_ENLARGE,
} EPLOTCURSORMODE;
typedef enum { // 鼠标点击动作
    HPL_MA_NOACT,
    HPL_MA_LEFTDOWN,
    HPL_MA_LEFTUP,
    HPL_MA_RIGHTDOWN,
    HPL_MA_RIGHTUP,
} EPLOTMOUSEACT;
// HANPlot 数据结构定义
typedef struct tagHANPLOTDS HANPLOTDS, * HANPPLOTDS;
typedef const HANPLOTDS* HANPCPLOTDS;
struct tagHANPLOTDS {
    HANCHAR             pName[HAN_PLOT_STR_LEN_MAX];    // 变量名
    HANCHAR             pText[HAN_PLOT_STR_LEN_MAX];    // 需要打印的缓存
    HANPPLOTNUM         pData;          // 数据
    HANPLOTNUM          nMax;           // 最大值
    HANPLOTNUM          nMin;           // 最小值
    COLORREF            crColor;        // 颜色
    HPEN                hPen;           // 画笔句柄
    HBRUSH              hBrush;         // 画刷句柄
    unsigned int        ePos;           // 变量的数值在绘图区的位置
    BOOL                bDraw;          // 是否绘制该变量
    BOOL                bInRect;        // 该变量是否在绘图区有点
    HANSIZE             nIdBegin;       // 绘图区里点的起始 id
    HANSIZE             nIdEnd;         // 绘图区里点的截止 id
    HANHPOINT           DrawPoint;      // 绘制点时的操作
    HANPPLOTDS          pChild;         // 子节点，供需要一个节点存储多组数据用
    HANPPLOTDS          next;           // 链表的下一个节点
};
#define HPLOT_CWS_BUTTON        (WS_CHILD | BS_VCENTER | BS_CENTER)    // style
// HANPlot 通用接口

static void HANPlotGetDataMaxMin(HANPCPLOTDATA pData, HANPPLOTMAXMIN pMaxMin);
static HANERROR HANPlotUpdateData(HANPPLOTDATA pDest, HANPCPLOTDATA pSrc, EPLOTUPDATEDATA modeUpdate);

static COLORREF HANPlotAddColor(COLORREF OldColor);

static void HANPlotDrawPointCircle(HANPCPLOTDRAWPOINTPARAM pParam);
static void HANPlotDrawPointSquare(HANPCPLOTDRAWPOINTPARAM pParam);
static void HANPlotDrawPointTriangle(HANPCPLOTDRAWPOINTPARAM pParam);
static void HANPlotDrawPointStar(HANPCPLOTDRAWPOINTPARAM pParam);

static void HANPlotGetDataMaxMin(HANPCPLOTDATA pData, HANPPLOTMAXMIN pMaxMin)
{
    pMaxMin->max = pData->pData[0];
    pMaxMin->min = pData->pData[0];

    for (HANSIZE iLoop = 0; iLoop < pData->nLen; iLoop++)
    {
        if (pData->pData[iLoop] > pMaxMin->max) { pMaxMin->max = pData->pData[iLoop]; }
        if (pData->pData[iLoop] < pMaxMin->min) { pMaxMin->min = pData->pData[iLoop]; }
    }
}
static HANERROR HANPlotUpdateData(HANPPLOTDATA pDest, HANPCPLOTDATA pSrc, EPLOTUPDATEDATA modeUpdate)
{
    HANERROR eRet = RET_OK;
    HANSIZE nDifSize;
    
    if (pDest->nLen < pSrc->nLen) { eRet = INDEX_OUT_OF_RANGE; }
    else
    {
        nDifSize = pDest->nLen - pSrc->nLen;
        switch (modeUpdate) {
            case HAN_PLOT_UPDATE_DATA_SET_START: {
                CopyMemory(&(pDest->pData[0]), &(pSrc->pData[0]), sizeof(HANPLOTNUM) * (pSrc->nLen));
            } break;
            case HAN_PLOT_UPDATE_DATA_PUSH_START: {
                MoveMemory(&(pDest->pData[pSrc->nLen]), &(pDest->pData[0]), sizeof(HANPLOTNUM) * nDifSize);
                CopyMemory(&(pDest->pData[0]), &(pSrc->pData[0]), sizeof(HANPLOTNUM) * (pSrc->nLen));
            } break;
            case HAN_PLOT_UPDATE_DATA_SET_END: {
                CopyMemory(&(pDest->pData[nDifSize]), &(pSrc->pData[0]), sizeof(HANPLOTNUM) * (pSrc->nLen));
            } break;
            case HAN_PLOT_UPDATE_DATA_PUSH_END: {
                MoveMemory(&(pDest->pData[0]), &(pDest->pData[pSrc->nLen]), sizeof(HANPLOTNUM) * nDifSize);
                CopyMemory(&(pDest->pData[nDifSize]), &(pSrc->pData[0]), sizeof(HANPLOTNUM) * (pSrc->nLen));
            } break;

            default: {
                /* idle */
            } break;
        }
    }

    return eRet;
}

static COLORREF HANPlotAddColor(COLORREF OldColor)
{
    return (((OldColor << 8) | ((OldColor >> 16) & 0xFF)) + 160) & 0xFFFFFF;
}

static void HANPlotDrawPointCircle(HANPCPLOTDRAWPOINTPARAM pParam)
{
    HANINT nHalfSize = pParam->nSize / 2;
    for (HANSIZE i = 0; i < pParam->nPointLen; i++)
    {
        HANINT left = pParam->ptPoint[i].x - nHalfSize;
        HANINT top = pParam->ptPoint[i].y - nHalfSize;
        HANINT right = left + pParam->nSize;
        HANINT bottom = top + pParam->nSize;
        Ellipse(pParam->hdc, left, top, right, bottom);
    }
}
static void HANPlotDrawPointSquare(HANPCPLOTDRAWPOINTPARAM pParam)
{
    POINT ptPoint[HAN_PLOT_DRAW_POINT_BUF_LEN * (HANSIZE)4];
    INT nPointCnt[HAN_PLOT_DRAW_POINT_BUF_LEN];
    HANSIZE iVec = HAN_PLOT_DRAW_POINT_BUF_LEN - (HANSIZE)1;
    HANINT nHalfSize = pParam->nSize / 2;

    for (HANSIZE i = 0; i < pParam->nPointLen; i++)
    {
        iVec = i % (HANSIZE)HAN_PLOT_DRAW_POINT_BUF_LEN;
        ptPoint[iVec * (HANSIZE)4 + (HANSIZE)0].x = pParam->ptPoint[i].x - nHalfSize;
        ptPoint[iVec * (HANSIZE)4 + (HANSIZE)0].y = pParam->ptPoint[i].y - nHalfSize;
        ptPoint[iVec * (HANSIZE)4 + (HANSIZE)1].x = pParam->ptPoint[i].x - nHalfSize;
        ptPoint[iVec * (HANSIZE)4 + (HANSIZE)1].y = pParam->ptPoint[i].y + nHalfSize;
        ptPoint[iVec * (HANSIZE)4 + (HANSIZE)2].x = pParam->ptPoint[i].x + nHalfSize;
        ptPoint[iVec * (HANSIZE)4 + (HANSIZE)2].y = pParam->ptPoint[i].y + nHalfSize;
        ptPoint[iVec * (HANSIZE)4 + (HANSIZE)3].x = pParam->ptPoint[i].x + nHalfSize;
        ptPoint[iVec * (HANSIZE)4 + (HANSIZE)3].y = pParam->ptPoint[i].y - nHalfSize;
        nPointCnt[iVec] = 4;
        if ((HAN_PLOT_DRAW_POINT_BUF_LEN - (HANSIZE)1) == iVec) { PolyPolygon(pParam->hdc, ptPoint, nPointCnt, (HANINT)iVec + 1); }
    }
    if ((HAN_PLOT_DRAW_POINT_BUF_LEN - (HANSIZE)1) != iVec) { PolyPolygon(pParam->hdc, ptPoint, nPointCnt, (HANINT)iVec + 1); }
}
static void HANPlotDrawPointTriangle(HANPCPLOTDRAWPOINTPARAM pParam)
{
    POINT ptPoint[HAN_PLOT_DRAW_POINT_BUF_LEN * (HANSIZE)3];
    INT nPointCnt[HAN_PLOT_DRAW_POINT_BUF_LEN];
    HANSIZE iVec = HAN_PLOT_DRAW_POINT_BUF_LEN - (HANSIZE)1;

    for (HANSIZE i = 0; i < pParam->nPointLen; i++)
    {
        iVec = i % (HANSIZE)HAN_PLOT_DRAW_POINT_BUF_LEN;
        ptPoint[iVec * (HANSIZE)3 + (HANSIZE)0].x = pParam->ptPoint[i].x;
        ptPoint[iVec * (HANSIZE)3 + (HANSIZE)0].y = pParam->ptPoint[i].y - (HANINT)(pParam->nSize * TAN_30_DEGREE);
        ptPoint[iVec * (HANSIZE)3 + (HANSIZE)1].x = pParam->ptPoint[i].x - (HANINT)(pParam->nSize / 2);
        ptPoint[iVec * (HANSIZE)3 + (HANSIZE)1].y = pParam->ptPoint[i].y + (HANINT)(pParam->nSize / 2 * TAN_30_DEGREE);
        ptPoint[iVec * (HANSIZE)3 + (HANSIZE)2].x = pParam->ptPoint[i].x + (HANINT)(pParam->nSize / 2);
        ptPoint[iVec * (HANSIZE)3 + (HANSIZE)2].y = pParam->ptPoint[i].y + (HANINT)(pParam->nSize / 2 * TAN_30_DEGREE);
        nPointCnt[iVec] = 3;
        if ((HAN_PLOT_DRAW_POINT_BUF_LEN - (HANSIZE)1) == iVec) { PolyPolygon(pParam->hdc, ptPoint, nPointCnt, (HANINT)iVec + 1); }
    }
    if ((HAN_PLOT_DRAW_POINT_BUF_LEN - (HANSIZE)1) != iVec) { PolyPolygon(pParam->hdc, ptPoint, nPointCnt, (HANINT)iVec + 1); }
}
static void HANPlotDrawPointStar(HANPCPLOTDRAWPOINTPARAM pParam)
{
    POINT ptPoint[HAN_PLOT_DRAW_POINT_BUF_LEN * (HANSIZE)10];
    INT nPointCnt[HAN_PLOT_DRAW_POINT_BUF_LEN];
    HANSIZE iVec = HAN_PLOT_DRAW_POINT_BUF_LEN - (HANSIZE)1;

    for (HANSIZE i = 0; i < pParam->nPointLen; i++)
    {
        iVec = i % HAN_PLOT_DRAW_POINT_BUF_LEN;
        HANDOUBLE aTri = pParam->nSize / 2.0 / (1 + COS_72_DEGREE);    /* 外部三角形的腰 */
        HANDOUBLE bTri = aTri * 2.0 * COS_72_DEGREE;                   /* 外部三角形的底边 */
        HANDOUBLE hTri = aTri * SIN_72_DEGREE;                         /* 外部三角形的高 */
        HANDOUBLE lCV = pParam->nSize / 2.0 / COS_18_DEGREE;           /* 中心点到凸点的距离 */
        HANDOUBLE lCC = bTri / 2.0 / COS_54_DEGREE;                    /* 中心点到凹点的距离 */
        ptPoint[iVec * (HANSIZE)10 + (HANSIZE)0].x = (pParam->ptPoint[i]).x;
        ptPoint[iVec * (HANSIZE)10 + (HANSIZE)0].y = (pParam->ptPoint[i]).y - (LONG)lCV;
        ptPoint[iVec * (HANSIZE)10 + (HANSIZE)1].x = (pParam->ptPoint[i]).x - (LONG)(bTri / 2.0);
        ptPoint[iVec * (HANSIZE)10 + (HANSIZE)1].y = (pParam->ptPoint[i]).y - (LONG)(lCV - hTri);
        ptPoint[iVec * (HANSIZE)10 + (HANSIZE)2].x = (pParam->ptPoint[i]).x - (LONG)((bTri / 2.0) + aTri);
        ptPoint[iVec * (HANSIZE)10 + (HANSIZE)2].y = (pParam->ptPoint[i]).y - (LONG)(lCV - hTri);
        ptPoint[iVec * (HANSIZE)10 + (HANSIZE)3].x = (pParam->ptPoint[i]).x - (LONG)((aTri + bTri) * SIN_18_DEGREE);
        ptPoint[iVec * (HANSIZE)10 + (HANSIZE)3].y = (pParam->ptPoint[i]).y + (LONG)((aTri + bTri) * COS_72_DEGREE * TAN_18_DEGREE);
        ptPoint[iVec * (HANSIZE)10 + (HANSIZE)4].x = (pParam->ptPoint[i]).x - (LONG)(pParam->nSize * SIN_18_DEGREE);
        ptPoint[iVec * (HANSIZE)10 + (HANSIZE)4].y = (pParam->ptPoint[i]).y + (LONG)((pParam->nSize * COS_18_DEGREE) - lCV);
        ptPoint[iVec * (HANSIZE)10 + (HANSIZE)5].x = (pParam->ptPoint[i]).x;
        ptPoint[iVec * (HANSIZE)10 + (HANSIZE)5].y = (pParam->ptPoint[i]).y + (LONG)lCC;
        ptPoint[iVec * (HANSIZE)10 + (HANSIZE)6].x = (pParam->ptPoint[i]).x + (LONG)(pParam->nSize * SIN_18_DEGREE);
        ptPoint[iVec * (HANSIZE)10 + (HANSIZE)6].y = (pParam->ptPoint[i]).y + (LONG)((pParam->nSize * COS_18_DEGREE) - lCV);
        ptPoint[iVec * (HANSIZE)10 + (HANSIZE)7].x = (pParam->ptPoint[i]).x + (LONG)((aTri + bTri) * SIN_18_DEGREE);
        ptPoint[iVec * (HANSIZE)10 + (HANSIZE)7].y = (pParam->ptPoint[i]).y + (LONG)((aTri + bTri) * COS_72_DEGREE * TAN_18_DEGREE);
        ptPoint[iVec * (HANSIZE)10 + (HANSIZE)8].x = (pParam->ptPoint[i]).x + (LONG)((bTri / 2.0) + aTri);
        ptPoint[iVec * (HANSIZE)10 + (HANSIZE)8].y = (pParam->ptPoint[i]).y - (LONG)(lCV - hTri);
        ptPoint[iVec * (HANSIZE)10 + (HANSIZE)9].x = (pParam->ptPoint[i]).x + (LONG)((bTri / 2.0));
        ptPoint[iVec * (HANSIZE)10 + (HANSIZE)9].y = (pParam->ptPoint[i]).y - (LONG)(lCV - hTri);
        nPointCnt[iVec] = 10;
        if (HAN_PLOT_DRAW_POINT_BUF_LEN - (HANSIZE)1 == iVec) { PolyPolygon(pParam->hdc, ptPoint, nPointCnt, (HANINT)iVec + 1); }
    }
    if (HAN_PLOT_DRAW_POINT_BUF_LEN - (HANSIZE)1 != iVec) { PolyPolygon(pParam->hdc, ptPoint, nPointCnt, (HANINT)iVec + 1); }
}
static HANHPOINT const g_DrawPointTable[4] = {
    [HAN_PLOT_DRAW_POINT_CIRCLE] = HANPlotDrawPointCircle,
    [HAN_PLOT_DRAW_POINT_SQUARE] = HANPlotDrawPointSquare,
    [HAN_PLOT_DRAW_POINT_TRIANGLE] = HANPlotDrawPointTriangle,
    [HAN_PLOT_DRAW_POINT_STAR] = HANPlotDrawPointStar,
};

HANHPOINT HANPlotGetStockPoint(HANPLOTDRAWPOINTSTYLE nPointStyle)
{
    HANHPOINT hRet = NULL;
    if ((size_t)nPointStyle < ArrLen(g_DrawPointTable)) { hRet = g_DrawPointTable[nPointStyle]; }
    return hRet;
}

#if 1 /******************** HANBarChart ********************/
#define HBC_VAR_TEXT_Y_BEGIN    30
// #define HBCS_2D_3D_MASK         HBCS_3D
#define HBCS_BS_MASK            HBCS_STACKED
#define HBCS_PCT_MASK           HBCS_PERCENTAGE
#define HBCS_3D_CAMERA_ANGLE    45.0                // 镜头角度
#define HBCS_3D_CAMERA_RAD      (HBCS_3D_CAMERA_ANGLE * HAN_PI / 180.0)
#define HBCS_3D_DEPTH           10                  // 深度
#define HBCS_3D_SHADOW_B_K1     0.9                 // 阴影的亮度系数 1
#define HBCS_3D_SHADOW_B_K2     0.8                 // 阴影的亮度系数 2
typedef struct tagHANBARCHARTDP { // 绘图参数
    HANPLOTNUM      nMax;           // 右侧最大值
    HANPLOTNUM      nMin;           // 左侧最小值
} HANBARCHARTDP;
typedef struct tagHANBARCHART HANBARCHART, * HANPBARCHART;
typedef const HANBARCHART* HANPCBARCHART;
struct tagHANBARCHART {
    HANDLE          hHeap;          // 堆句柄
    BOOL            bRedraw;        // 重绘 flag
    HANSIZE         nVarCount;      // 变量个数
    HANSIZE         nDataLen;       // 每个变量容纳的数据个数
    HANPPLOTDS      pVarData;       // 变量的具体信息
    HANSIZE         nVValueCount;   // 绘图界面上垂直方向显示的值的个数
    HPEN            hMarkLine;      // 标线画笔
    HANPSTR         pGroupNames;    // 组名
    HANPPLOTNUM     pSum;           // 每组数据的累加和
    HANINT          nBarWidth;      // 条宽
    HANINT          nBarInterval;   // 同组中的条间距
    BOOL            bAutoRect;      // 自动大小模式
    RECT            rcAutoRectDis;  // 自动大小模式下绘图区到窗口的距离
    RECT            rcRectangle;    // 绘图区域的矩形范围
    COLORREF        crTitleColor;   // 标题文字颜色
    HANINT          nTitleW;        // 标题文字字体宽
    HANINT          nTitleH;        // 标题文字字体高
    HFONT           hTitleFont;     // 标题文字字体对象
    COLORREF        crTextColor;    // 其它文字颜色
    HANINT          nTextW;         // 其它文字字体宽
    HANINT          nTextH;         // 其它文字字体高
    HFONT           hTextFont;      // 其它文字字体对象
    COLORREF        crBkColor;      // 窗口背景色
    COLORREF        crBkBorder;     // 窗口背景边框颜色
    HPEN            hBkBorder;      // 窗口前景边框画笔句柄
    HBRUSH          hBkBrush;       // 窗口背景画刷
    HANBARCHARTDP   dpDrawParam;    // 绘图参数，绘图时所有参数在这里计算
};
// 内部函数声明
static void HANBarChartPaintCallback(HDC hdc, HWND hBarChart, HANPBARCHART bcInfo, const RECT* rcWin);
static void HANBarChartTextOut(HDC hdc, HWND hBarChart, HANPCBARCHART bcInfo, const RECT* rcWin, LONG nStyle);
static void HANBarChartBarOut(HDC hdc, HANPCBARCHART bcInfo, const RECT* rcWin, LONG nStyle);
static void HANBarChartPaintBarShadow(HDC hdc, HANINT left, HANINT top, HANINT right, HANINT bottom, COLORREF crColor);
// WndProc & 实现 声明
static LRESULT CALLBACK HANBarChartWndProc(HWND hBarChart, UINT message, WPARAM wParam, LPARAM lParam);
static HANERROR HANBarChartSetDataLenCallback(HANPBARCHART bcInfo, HANSIZE nDataLen);
static HANERROR HANBarChartAddVarCallback(HANPBARCHART bcInfo, HANPCSTR pVarName);
static void HANBarChartSetGroupNameCallback(HANPBARCHART bcInfo, HANSIZE nGroupCnt, HANPCSTR pName);
static HANERROR HANBarChartSetValueCallback(HANPBARCHART bcInfo, HANPCSTR pVarName, HANSIZE nId, HANPCPLOTNUM pData);
static HANERROR HANBarChartSetBackgroundColorCallback(HANPBARCHART bcInfo, COLORREF rgb);
static HANERROR HANBarChartSetBackgroundBorderCallback(HANPBARCHART bcInfo, COLORREF rgb);
static HANERROR HANBarChartSetTitleFontCallback(HANPBARCHART bcInfo, HANINT w, HANINT h, HANPCSTR Face);
static HANERROR HANBarChartSetTextFontCallback(HANPBARCHART bcInfo, HANINT w, HANINT h, HANPCSTR Face);
static void HANBarChartSetRcRectangleCallback(HWND hBarChart, HANPBARCHART bcInfo, const RECT* rcRectangle, BOOL bAutoRect);
static BOOL HANBarChartGetRcRectangleCallback(HANPBARCHART bcInfo, RECT* rcRectangle);
static void HANBarChartSetVValueCountCallback(HANPBARCHART bcInfo, unsigned int n);
static void HANBarChartSetBarWidthCallback(HANPBARCHART bcInfo, HANINT w);
static void HANBarChartSetBarIntervalCallback(HANPBARCHART bcInfo, HANINT nInterval);
// 接口定义
void RegisterHANBarChart(HINSTANCE hInstance)
{
    WNDCLASSEX wcexHANBarChart = {
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_PARENTDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = HANBarChartWndProc,
        .cbClsExtra = 0,
        .cbWndExtra = sizeof(HANPBARCHART),
        .hInstance = hInstance,
        .hIcon = 0,
        .hCursor = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName = NULL,
        .lpszClassName = HBC_CLASS,
        .hIconSm = NULL
    };
    RegisterClassEx(&wcexHANBarChart);
}
HWND CreateHANBarChart(
    HANPCSTR pWindowName, DWORD dwStyle, HANINT x, HANINT y, HANINT w, HANINT h,
    HWND hParent, HMENU hMenu, HINSTANCE hInstance,
    HANSIZE nVarCnt, HANSIZE nDataLen, const HANPSTR* pVarName, const HANPSTR* pGroupName, const HANPPLOTNUM* pData)
{
    HWND hRet = CreateWindow(HBC_CLASS, pWindowName, dwStyle, x, y, w, h, hParent, hMenu, hInstance, NULL);
    if (NULL != hRet)
    {
        if (RET_OK != HANBarChartSetDataLen(hRet, nDataLen))
        {
            DestroyWindow(hRet);
            hRet = NULL;
        }
    }
    if (NULL != hRet)
    {
        for (HANSIZE i = 0; i < nVarCnt; i++)
        {
            if (RET_OK != HANBarChartAddVar(hRet, pVarName[i]))
            {
                DestroyWindow(hRet);
                hRet = NULL;
                break;
            }
            (void)HANBarChartSetValueById(hRet, i, pData[i]);
        }
    }
    if ((NULL != hRet) && (NULL != pGroupName))
    {
        for (HANSIZE i = 0; i < nDataLen; i++)
        {
            HANBarChartSetGroupName(hRet, i, pGroupName[i], FALSE);
        }
    }

    return hRet;
}
HANERROR HANBarChartSetDataLen(HWND hBarChart, HANSIZE nDataLen)
{
    return SendMessage(hBarChart, HBCM_SETDATALEN, (WPARAM)nDataLen, 0);
}
HANERROR HANBarChartAddVar(HWND hBarChart, HANPCSTR pVarName)
{
    return SendMessage(hBarChart, HBCM_ADDVAR, 0, (LPARAM)pVarName);
}
void HANBarChartSetGroupName(HWND hBarChart, HANSIZE nGroupCnt, HANPCSTR pName, BOOL bRedraw)
{
    SendMessage(hBarChart, HBCM_SETGROUPNAME, (WPARAM)nGroupCnt, (LPARAM)pName);
    if (TRUE == bRedraw) { RedrawWindow(hBarChart, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
}
HANERROR HANBarChartSetValueById(HWND hBarChart, HANSIZE nId, HANPCPLOTNUM pData)
{
    return SendMessage(hBarChart, HBCM_SETVALUEBYID, (WPARAM)nId, (LPARAM)pData);
}
HANERROR HANBarChartSetBackgroundColor(HWND hBarChart, COLORREF rgb, BOOL bRedraw)
{
    HANERROR ret = SendMessage(hBarChart, HBCM_SETBKCOLOR, (WPARAM)rgb, 0);
    if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hBarChart, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    return ret;
}
HANERROR HANBarChartSetBkBorder(HWND hBarChart, COLORREF rgb, BOOL bRedraw)
{
    HANERROR ret = SendMessage(hBarChart, HBCM_SETBKBORDER, (WPARAM)rgb, 0);
    if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hBarChart, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    return ret;
}
HANERROR HANBarChartSetTitleFont(HWND hBarChart, HANINT w, HANINT h, HANPCSTR pFace, BOOL bRedraw)
{
    HANINT pTextSize[2] = { w, h };
    HANERROR ret = SendMessage(hBarChart, HBCM_SETTITLEFONT, (WPARAM)pTextSize, (LPARAM)pFace);
    if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hBarChart, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    return ret;
}
HANERROR HANBarChartSetTextFont(HWND hBarChart, HANINT w, HANINT h, HANPCSTR pFace, BOOL bRedraw)
{
    HANINT pTextSize[2] = { w, h };
    HANERROR ret = SendMessage(hBarChart, HBCM_SETTEXTFONT, (WPARAM)pTextSize, (LPARAM)pFace);
    if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hBarChart, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    return ret;
}
void HANBarChartSetRcRectangle(HWND hBarChart, const RECT* rcRectangle, BOOL bAutoRect, BOOL bRedraw)
{
    SendMessage(hBarChart, HBCM_SETRECT, (WPARAM)bAutoRect, (LPARAM)rcRectangle);
    if (TRUE == bRedraw) { RedrawWindow(hBarChart, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
}
BOOL HANBarChartGetRcRectangle(HWND hBarChart, RECT* rcRectangle)
{
    return (BOOL)SendMessage(hBarChart, HBCM_GETRECT, 0, (LPARAM)rcRectangle);
}
void HANBarChartSetBarWidth(HWND hBarChart, HANINT w, BOOL bRedraw)
{
    SendMessage(hBarChart, HBCM_SETBARWIDTH, w, 0);
    if (TRUE == bRedraw) { RedrawWindow(hBarChart, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
}
void HANBarChartSetBarInterval(HWND hBarChart, HANINT nInterval, BOOL bRedraw)
{
    SendMessage(hBarChart, HBCM_SETBARINTERVAL, nInterval, 0);
    if (TRUE == bRedraw) { RedrawWindow(hBarChart, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
}
// 内部函数定义
static void HANBarChartPaintCallback(HDC hdc, HWND hBarChart, HANPBARCHART bcInfo, const RECT* rcWin)
{
    // 获取最值，先算原始数据里的最值
    LONG nStyle = GetWindowStyle(hBarChart);
    LONG nBarStyle = nStyle & HBCS_BS_MASK;
    bcInfo->dpDrawParam.nMax = 0.0;
    bcInfo->dpDrawParam.nMin = 0.0;
    HANPPLOTDS pNode;
    for (HANSIZE i = 0; i < bcInfo->nDataLen; i++)
    {
        pNode = bcInfo->pVarData;
        HANPLOTNUM nSum = 0.0;
        while (NULL != pNode)
        {
            HANPLOTNUM nVal = pNode->pData[i];
            nSum += HANPlotNumAbs(nVal);
            switch (nBarStyle) {
                case HBCS_CLUSTERED: {
                    if (bcInfo->dpDrawParam.nMax < nVal) { bcInfo->dpDrawParam.nMax = nVal; }
                    if (bcInfo->dpDrawParam.nMin > nVal) { bcInfo->dpDrawParam.nMin = nVal; }
                } break;
                case HBCS_STACKED: {
                    if (bcInfo->dpDrawParam.nMax < nSum) { bcInfo->dpDrawParam.nMax = nSum; }
                    if (bcInfo->dpDrawParam.nMin > nSum) { bcInfo->dpDrawParam.nMin = nSum; }
                } break;

                default: {
                    /* idle */
                } break;
            }
            pNode = pNode->next;
        }
        bcInfo->pSum[i] = nSum;
    }
    // 画条形图
    HANBarChartBarOut(hdc, bcInfo, rcWin, nStyle);
    // 输出文本
    HANBarChartTextOut(hdc, hBarChart, bcInfo, rcWin, nStyle);
}
static void HANBarChartTextOut(HDC hdc, HWND hBarChart, HANPCBARCHART bcInfo, const RECT* rcWin, LONG nStyle)
{
    HANCHAR pText[HAN_PLOT_STR_LEN_MAX];
    LONG nWinW = GetRectW(rcWin);

    // 设置 hdc 参数：标题文字颜色、字体、对齐方式、透明背景
    HFONT hOldFont = SelectObject(hdc, bcInfo->hTitleFont);
    UINT nOldTextAlign = SetTextAlign(hdc, TA_CENTER | TA_TOP);
    HANINT nOldBkMode = SetBkMode(hdc, TRANSPARENT);

    // 绘制标题
    GetWindowText(hBarChart, pText, HAN_PLOT_STR_LEN_MAX);
    TextOut(hdc, nWinW / 2, 1, pText, (HANINT)(HAN_strlen(pText)));

    // 绘制变量
    HANINT nTextY = HBC_VAR_TEXT_Y_BEGIN;
    SelectObject(hdc, bcInfo->hTextFont);
    HANPPLOTDS pNode = bcInfo->pVarData;
    while (NULL != pNode)
    {
        HANPCSTR pName;
        SetTextAlign(hdc, TA_RIGHT | TA_TOP);
        SetTextColor(hdc, pNode->crColor);
        pName = pNode->pName;
        TextOut(hdc, nWinW - 10, nTextY, pName, (HANINT)(HAN_strlen(pName)));
        nTextY += bcInfo->nTextH;
        pNode = pNode->next;
    }

    // 绘制数值
    if (bcInfo->nDataLen > (HANSIZE)0)
    {
        HANDOUBLE kNum;

        // 绘制垂直数值
        SetTextColor(hdc, bcInfo->crTextColor);
        SetTextAlign(hdc, TA_RIGHT);
        HANINT nVValueBeginY = bcInfo->rcRectangle.top - (bcInfo->nTextH / 2);
        kNum = (HANDOUBLE)((GetRectH(&(bcInfo->rcRectangle))) / (bcInfo->nVValueCount - (HANSIZE)1));
        for (HANSIZE i = 0; i < bcInfo->nVValueCount; i++)
        {
            HANINT nTextCharCount;
            if ((LONG)0 != (nStyle & (LONG)HBCS_PERCENTAGE))
            {
                nTextCharCount = HAN_snprintf(pText, HAN_PLOT_STR_LEN_MAX, TEXT("%g%%"), 100.0 / (bcInfo->nVValueCount - (HANSIZE)1) * i);
            }
            else
            {
                nTextCharCount = HAN_snprintf(pText, HAN_PLOT_STR_LEN_MAX, TEXT("%g"),
                    bcInfo->dpDrawParam.nMax - i * (bcInfo->dpDrawParam.nMax - bcInfo->dpDrawParam.nMin) / (bcInfo->nVValueCount - (HANSIZE)1));
            }
            TextOut(hdc, bcInfo->rcRectangle.left - (LONG)3, nVValueBeginY + (HANINT)((HANDOUBLE)i * kNum), pText, nTextCharCount);
        }

        // 绘制水平数值
        SetTextAlign(hdc, TA_CENTER | TA_TOP);
        HANDOUBLE nGroupW = (HANDOUBLE)(GetRectW(&(bcInfo->rcRectangle)) / bcInfo->nDataLen);
        for (HANSIZE i = 0; i < bcInfo->nDataLen; i++)
        {
            HANINT x = bcInfo->rcRectangle.left + (LONG)(nGroupW * ((HANDOUBLE)i + 0.5));
            TextOut(
                hdc, x, (HANINT)(bcInfo->rcRectangle.bottom) + (bcInfo->nTextH / 2),
                bcInfo->pGroupNames + (i * (HANSIZE)HAN_PLOT_STR_LEN_MAX),
                (HANINT)HAN_strlen(&(bcInfo->pGroupNames[i * (HANSIZE)HAN_PLOT_STR_LEN_MAX]))
            );
        }
    }
    
    // 还原 hdc 参数
    SelectObject(hdc, hOldFont);
    SetTextAlign(hdc, nOldTextAlign);
    SetBkMode(hdc, nOldBkMode);
}
static void HANBarChartBarOut(HDC hdc, HANPCBARCHART bcInfo, const RECT* rcWin, LONG nStyle)
{
    // 绘制背景保护 hdc
    HPEN hOldPen = SelectObject(hdc, bcInfo->hBkBorder);
    HBRUSH hOldBrush = SelectObject(hdc, bcInfo->hBkBrush);
    Rectangle(hdc, rcWin->left, rcWin->top, rcWin->right, rcWin->bottom);
    // 边框和标线
    POINT ptPaperLine[] = {
        { .x = (bcInfo->rcRectangle).left, .y = (bcInfo->rcRectangle).top, },
        { .x = (bcInfo->rcRectangle).left, .y = (bcInfo->rcRectangle).bottom, },
        { .x = (bcInfo->rcRectangle).right, .y = (bcInfo->rcRectangle).bottom, },
    };
    Polyline(hdc, ptPaperLine, ArrLen(ptPaperLine));
    POINT ptMarkLine[3];
    LONG nRectW = GetRectW(&(bcInfo->rcRectangle));
    LONG nRectH = GetRectH(&(bcInfo->rcRectangle));
    for (HANSIZE i = 0; i < bcInfo->nVValueCount; i++)
    {
        if (0 != (nStyle & HBCS_3D))
        {
            ptMarkLine[0].x = bcInfo->rcRectangle.left;
            ptMarkLine[0].y = bcInfo->rcRectangle.bottom - (LONG)(i * nRectH / (bcInfo->nVValueCount - (HANSIZE)1));
            ptMarkLine[1].x = ptMarkLine[0].x + (LONG)(HBCS_3D_DEPTH * cos(HBCS_3D_CAMERA_RAD));
            ptMarkLine[1].y = ptMarkLine[0].y - (LONG)(HBCS_3D_DEPTH * sin(HBCS_3D_CAMERA_RAD));
            ptMarkLine[2].x = ptMarkLine[1].x + nRectW;
            ptMarkLine[2].y = ptMarkLine[1].y;
            Polyline(hdc, ptMarkLine, 3);
        }
        else
        {
            if (i > (HANSIZE)0)
            {
                ptMarkLine[0].x = bcInfo->rcRectangle.left;
                ptMarkLine[0].y = bcInfo->rcRectangle.bottom - (LONG)(i * nRectH / (bcInfo->nVValueCount - (HANSIZE)1));
                ptMarkLine[1].x = ptMarkLine[0].x + nRectW;
                ptMarkLine[1].y = ptMarkLine[0].y;
                Polyline(hdc, ptMarkLine, 2);
            }
        }
    }
    // 条形图
    SelectObject(hdc, GetStockObject(NULL_PEN));
    HANDOUBLE nGroupW = (HANDOUBLE)(GetRectW(&(bcInfo->rcRectangle)) / bcInfo->nDataLen);
    for (HANSIZE i = 0; i < bcInfo->nDataLen; i++)
    {
        HANSIZE j = 0;
        HANINT x;
        HANINT y0 = bcInfo->rcRectangle.bottom;
        HANPPLOTDS pNode = bcInfo->pVarData;
        while (NULL != pNode)
        {
            SelectObject(hdc, pNode->hBrush);
            HANINT nGroupCenterX = bcInfo->rcRectangle.left + (LONG)(nGroupW * ((HANDOUBLE)i + 0.5));
            switch (nStyle & HBCS_BS_MASK) {
                case HBCS_CLUSTERED: {
                    HANINT nBarWDX = bcInfo->nBarWidth + bcInfo->nBarInterval;
                    x = nGroupCenterX - (((nBarWDX * (HANINT)(bcInfo->nVarCount)) - (bcInfo->nBarInterval)) / 2) + (nBarWDX * (HANINT)j);
                    if (0 != (nStyle & HBCS_PCT_MASK))
                    {
                        y0 = bcInfo->rcRectangle.bottom - (LONG)(nRectH / bcInfo->pSum[i] * HANPlotNumAbs(pNode->pData[i]));
                    }
                    else
                    {
                        y0 = bcInfo->rcRectangle.bottom - (LONG)(nRectH / (bcInfo->dpDrawParam.nMax - bcInfo->dpDrawParam.nMin) * (pNode->pData[i] - bcInfo->dpDrawParam.nMin));
                    }
                    Rectangle(hdc, x, y0, x + bcInfo->nBarWidth + 1, bcInfo->rcRectangle.bottom + 1);
                    if (0 != (nStyle & HBCS_3D)) { HANBarChartPaintBarShadow(hdc, x, y0, x + bcInfo->nBarWidth, bcInfo->rcRectangle.bottom, pNode->crColor); }
                } break;
                case HBCS_STACKED: {
                    x = nGroupCenterX - (bcInfo->nBarWidth / 2);
                    HANINT y0Prev = y0;
                    if (0 != (nStyle & HBCS_PCT_MASK))
                    {
                        y0 -= (HANINT)(nRectH / bcInfo->pSum[i] * HANPlotNumAbs(pNode->pData[i]));
                    }
                    else
                    {
                        y0 -= (HANINT)(nRectH / (bcInfo->dpDrawParam.nMax - bcInfo->dpDrawParam.nMin) * HANPlotNumAbs(pNode->pData[i]));
                    }
                    Rectangle(hdc, x, y0, x + bcInfo->nBarWidth + 1, y0Prev + 1);
                    if (0 != (nStyle & HBCS_3D)) { HANBarChartPaintBarShadow(hdc, x, y0, x + bcInfo->nBarWidth, y0Prev, pNode->crColor); }
                } break;

                default: {
                    /* idle */
                } break;
            }
            j++;
            pNode = pNode->next;
        }
    }
    // 还原 hdc 参数
    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
}
static void HANBarChartPaintBarShadow(HDC hdc, HANINT left, HANINT top, HANINT right, HANINT bottom, COLORREF crColor)
{
    HANINT w = (HANINT)(HBCS_3D_DEPTH * cos(HBCS_3D_CAMERA_RAD));
    HANINT h = (HANINT)(HBCS_3D_DEPTH * sin(HBCS_3D_CAMERA_RAD));
    POINT pptShadow1[] = { { right, top }, { right + w, top - h }, { right + w, bottom - h }, { right, bottom } };
    POINT pptShadow2[] = { { left, top }, { left + w, top - h }, { right + w, top - h }, { right, top } };
    COLORHSB crHSB;
    COLORREF crRGB1;
    COLORREF crRGB2;
    PRGBToPHSB(&crColor, &crHSB);
    crHSB.kBrightness *= (float)HBCS_3D_SHADOW_B_K1;
    PHSBToPRGB(&crHSB, &crRGB1);
    PRGBToPHSB(&crColor, &crHSB);
    crHSB.kBrightness *= (float)HBCS_3D_SHADOW_B_K2;
    PHSBToPRGB(&crHSB, &crRGB2);
    HBRUSH hShadow1 = CreateSolidBrush(crRGB1);
    HBRUSH hShadow2 = CreateSolidBrush(crRGB2);
    HBRUSH hOldBrush = SelectObject(hdc, hShadow1);
    Polygon(hdc, pptShadow1, ArrLen(pptShadow1));
    SelectObject(hdc, hShadow2);
    Polygon(hdc, pptShadow2, ArrLen(pptShadow2));
    SelectObject(hdc, hOldBrush);
    DeleteObject(hShadow1);
    DeleteObject(hShadow2);
}
// WndProc & 实现 定义
static LRESULT CALLBACK HANBarChartWndProc(HWND hBarChart, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    // 读取属性
    HANPBARCHART bcInfo = (HANPBARCHART)GetWindowLongPtr(hBarChart, 0);
    
    switch (message) {
        case WM_CREATE: { // 创建窗口
            HANDLE hHeap = GetProcessHeap();
            if (NULL == hHeap) { lWndProcRet = -1; break; }
            bcInfo = (HANPBARCHART)HANWinHeapAlloc(hHeap, NULL, sizeof(HANBARCHART));
            if (NULL == bcInfo) { lWndProcRet = -1; break; }
            SetWindowLongPtr(hBarChart, 0, (LONG_PTR)bcInfo);
            bcInfo->hHeap = hHeap;
            // 获取窗口大小
            RECT rcWin;
            GetWindowRect(hBarChart, &rcWin);
            LONG nWinW = GetRectW(&rcWin);
            LONG nWinH = GetRectH(&rcWin);
            // 初始化清零附加结构体
            (void)HANBarChartSetBackgroundColorCallback(bcInfo, RGB(0xFF, 0xFF, 0xFF));
            (void)HANBarChartSetBackgroundBorderCallback(bcInfo, RGB(0x00, 0x00, 0x00));
            (void)HANBarChartSetTitleFontCallback(bcInfo, HBC_DEF_TITLE_W, HBC_DEF_TITLE_H, HAN_PLOT_DEF_TEXT_FACE);
            (void)HANBarChartSetTextFontCallback(bcInfo, HBC_DEF_TEXT_W, HBC_DEF_TEXT_H, HAN_PLOT_DEF_TEXT_FACE);
            RECT rcRectangle = {
                .left = HBC_DEF_RECT_LEFT,
                .top = HBC_DEF_RECT_TOP,
                .right = HBC_DEF_RECT_RIGHT,
                .bottom = HBC_DEF_RECT_BOTTOM,
            };
            if ((LONG)(HBC_DEF_RECT_LEFT + HBC_DEF_RECT_RIGHT) > nWinW) { rcRectangle.left = nWinW / (LONG)3; rcRectangle.right = nWinW / (LONG)3; }
            if ((LONG)(HBC_DEF_RECT_TOP + HBC_DEF_RECT_BOTTOM) > nWinH) { rcRectangle.top = nWinH / (LONG)3; rcRectangle.bottom = nWinH / (LONG)3; }
            HANBarChartSetRcRectangleCallback(hBarChart, bcInfo, &rcRectangle, TRUE);
            HANBarChartSetVValueCountCallback(bcInfo, HBC_DEF_VVALUE_COUNT);
            HANBarChartSetBarWidthCallback(bcInfo, HBC_DEF_BAR_WIDTH);
            HANBarChartSetBarIntervalCallback(bcInfo, HBC_DEF_BAR_DX);
        } break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            // 开始绘图
            HDC hdc = BeginPaint(hBarChart, &ps);
            // 双缓冲绘图，分配资源
            RECT rcWin;
            GetClientRect(hBarChart, &rcWin);
            HDC hMemDC = CreateCompatibleDC(hdc);   // 获取内存设备上下文，图形统一在该 HDC 下绘制
            HBITMAP hBitMap = CreateCompatibleBitmap(hdc, GetRectW(&rcWin), GetRectH(&rcWin));  // 创建位图
            SelectObject(hMemDC, hBitMap);

            // 绘制内容
            HANBarChartPaintCallback(hMemDC, hBarChart, bcInfo, &rcWin);

            // 将位图中绘制好的图像贴到屏幕上
            BitBlt(hdc, 0, 0, GetRectW(&rcWin), GetRectH(&rcWin), hMemDC, 0, 0, SRCCOPY);
            
            // 释放内存和位图
            DeleteDC(hMemDC);
            DeleteObject(hBitMap);
            // 结束绘图
            EndPaint(hBarChart, &ps);
        } break;
        case WM_DESTROY: {
            if (NULL != bcInfo)
            {
                
            }
        } break;

        case HBCM_SETDATALEN: {
            lWndProcRet = HANBarChartSetDataLenCallback(bcInfo, (HANSIZE)wParam);
        } break;
        case HBCM_ADDVAR: {
            lWndProcRet = HANBarChartAddVarCallback(bcInfo, (HANPCSTR)lParam);
        } break;
        case HBCM_SETGROUPNAME: {
            HANBarChartSetGroupNameCallback(bcInfo, (HANSIZE)wParam, (HANPCSTR)lParam);
        } break;
        case HBCM_SETVALUEBYNAME: {
            lWndProcRet = HANBarChartSetValueCallback(bcInfo, (HANPCSTR)wParam, (HANSIZE)(-1), (HANPCPLOTNUM)lParam);
        } break;
        case HBCM_SETVALUEBYID: {
            lWndProcRet = HANBarChartSetValueCallback(bcInfo, (HANPCSTR)NULL, (HANSIZE)wParam, (HANPCPLOTNUM)lParam);
        } break;
        case HBCM_SETBKCOLOR: {
            lWndProcRet = HANBarChartSetBackgroundColorCallback(bcInfo, (COLORREF)wParam);
        } break;
        case HBCM_SETBKBORDER: {
            lWndProcRet = HANBarChartSetBackgroundBorderCallback(bcInfo, (COLORREF)wParam);
        } break;
        case HBCM_SETRECT: {
            HANBarChartSetRcRectangleCallback(hBarChart, bcInfo, (const RECT*)lParam, (BOOL)wParam);
        } break;
        case HBCM_GETRECT: {
            lWndProcRet = HANBarChartGetRcRectangleCallback(bcInfo, (RECT*)lParam);
        } break;
        case HBCM_SETBARWIDTH: {
            HANBarChartSetBarWidthCallback(bcInfo, (HANINT)wParam);
        } break;
        case HBCM_SETBARINTERVAL: {
            HANBarChartSetBarIntervalCallback(bcInfo, (HANINT)wParam);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hBarChart, message, wParam, lParam);
        } break;
    }
    
    // 返回
    return lWndProcRet;
}
static HANERROR HANBarChartSetDataLenCallback(HANPBARCHART bcInfo, HANSIZE nDataLen)
{
    HANERROR eRet = RET_OK;
    HANPSTR pTempGroupName;
    HANPPLOTNUM pTempData;
    HANPPLOTNUM pTempSum;
    if (nDataLen > bcInfo->nDataLen)
    {
        HANPPLOTDS pNode = bcInfo->pVarData;
        while (NULL != pNode)
        {
            pTempData = HANWinHeapAlloc(bcInfo->hHeap, pNode->pData, sizeof(HANPLOTNUM) * (size_t)nDataLen);
            if (NULL == pTempData) { eRet = NOT_ENOUGH_MEMORY; break; }
            else { pNode->pData = pTempData; }
            pNode = pNode->next;
        }
        if (RET_OK == eRet)
        {
            pTempGroupName = HANWinHeapAlloc(bcInfo->hHeap, bcInfo->pGroupNames, sizeof(HANCHAR) * (size_t)nDataLen * (size_t)HAN_PLOT_STR_LEN_MAX);
            if (NULL == pTempGroupName) { eRet = NOT_ENOUGH_MEMORY; }
            else { bcInfo->pGroupNames = pTempGroupName; }
        }
        if (RET_OK == eRet)
        {
            pTempSum = HANWinHeapAlloc(bcInfo->hHeap, bcInfo->pSum, sizeof(HANPLOTNUM) * (size_t)nDataLen);
            if (NULL == pTempSum) { eRet = NOT_ENOUGH_MEMORY; }
            else { bcInfo->pSum = pTempSum; }
        }
    }
    if (RET_OK == eRet)
    {
        HANSIZE iBegin = 0;
        HANSIZE iEnd = 0;
        if (nDataLen > bcInfo->nDataLen) { iBegin = bcInfo->nDataLen; iEnd = nDataLen; }
        else { iBegin = nDataLen; iEnd = bcInfo->nDataLen; }
        for (HANSIZE i = iBegin; i < iEnd; i++)
        {
            HAN_snprintf(&(bcInfo->pGroupNames[i * (HANSIZE)HAN_PLOT_STR_LEN_MAX]), HAN_PLOT_STR_LEN_MAX, TEXT("Group ") TEXT(HANSIZE_PRINT_FORMAT), i);
        }
        bcInfo->nDataLen = nDataLen;
    }

    return eRet;
}
static HANERROR HANBarChartAddVarCallback(HANPBARCHART bcInfo, HANPCSTR pVarName)
{
    HANERROR eRet = RET_OK;
    COLORREF crColor = 0;
    HANPPLOTNUM pData = NULL;
    HANPPLOTDS pNode = NULL;
    HPEN hPen = NULL;
    HBRUSH hBrush = NULL;
    for (HANSIZE i = 0; i < bcInfo->nVarCount; i++)
    {
        crColor = HANPlotAddColor(crColor);
    }
    crColor = HANPlotAddColor(crColor);
    if (bcInfo->nDataLen > (HANSIZE)0)
    {
        pData = (HANPPLOTNUM)HANWinHeapAlloc(bcInfo->hHeap, NULL, bcInfo->nDataLen * sizeof(HANPLOTNUM));
        if (NULL == pData) { eRet = NOT_ENOUGH_MEMORY; }
    }
    if (RET_OK == eRet)
    {
        pNode = (HANPPLOTDS)HANWinHeapAlloc(bcInfo->hHeap, NULL, sizeof(HANPLOTDS));
        if (NULL == pNode) { eRet = NOT_ENOUGH_MEMORY; }
    }
    if (RET_OK == eRet)
    {
        hPen = CreatePen(PS_SOLID, 0, crColor);
        if (NULL == hPen) { eRet = CREATE_PEN_ERR; }
    }
    if (RET_OK == eRet)
    {
        hBrush = CreateSolidBrush(crColor);
        if (NULL == hBrush) { eRet = CREATE_BRUSH_ERR; }
    }

    if (RET_OK != eRet)
    {
        if (NULL != pData) { HANWinHeapFree(bcInfo->hHeap, 0, pData); }
        if (NULL != pNode) { HANWinHeapFree(bcInfo->hHeap, 0, pNode); }
        if (NULL != hPen) { DeleteObject(hPen); }
        if (NULL != hBrush) { DeleteObject(hBrush); }
    }
    else
    {
        // 将节点加入链表
        HANPPLOTDS pLast = bcInfo->pVarData;
        if (NULL == pLast) { bcInfo->pVarData = pNode; }
        else { while (NULL != pLast->next) { pLast = pLast->next; } pLast->next = pNode; }
        pNode->pData = pData;
        pNode->nMax = 0;
        pNode->nMin = 0;
        HAN_snprintf(pNode->pName, HAN_PLOT_STR_LEN_MAX, TEXT("%s"), pVarName);
        pNode->crColor = crColor;
        pNode->hPen = hPen;
        pNode->hBrush = hBrush;
        pNode->bDraw = TRUE;
        pNode->pChild = NULL;
        pNode->next = NULL;
        bcInfo->nVarCount++;
    }
    
    return eRet;
}
static void HANBarChartSetGroupNameCallback(HANPBARCHART bcInfo, HANSIZE nGroupID, HANPCSTR pName)
{
    HAN_snprintf(&(bcInfo->pGroupNames[nGroupID * (HANSIZE)HAN_PLOT_STR_LEN_MAX]), HAN_PLOT_STR_LEN_MAX, TEXT("%s"), pName);
}
static HANERROR HANBarChartSetValueCallback(HANPBARCHART bcInfo, HANPCSTR pVarName, HANSIZE nId, HANPCPLOTNUM pData)
{
    HANERROR eRet = PLOT_VAR_NOT_EXIST;
    HANPPLOTDS pNode = bcInfo->pVarData;
    HANSIZE nCount = 0;
    while (NULL != pNode)
    {
        if (((NULL != pVarName) && (!HAN_strcmp(pNode->pName, pVarName))) ||
            (nId == nCount))
        {
            HANPLOTNUM pnMax = pData[0];
            HANPLOTNUM pnMin = pData[0];
            for (HANSIZE j = 0; j < bcInfo->nDataLen; j++)
            {
                pNode->pData[j] = pData[j];
                if (pData[j] > pnMax) { pnMax = pData[j]; }
                if (pData[j] < pnMin) { pnMin = pData[j]; }
            }
            pNode->nMax = pnMax;
            pNode->nMin = pnMin;
            eRet = RET_OK;
            break;
        }
        pNode = pNode->next;
        nCount++;
    }
    return eRet;
}
static HANERROR HANBarChartSetBackgroundColorCallback(HANPBARCHART bcInfo, COLORREF rgb)
{
    HANERROR eRet = RET_OK;
    HBRUSH hBrush = CreateSolidBrush(rgb);

    if (NULL == hBrush) { eRet = CREATE_BRUSH_ERR; }
    if (RET_OK == eRet)
    {
        if (NULL != bcInfo->hBkBrush) { DeleteObject(bcInfo->hBkBrush); }
        bcInfo->crBkColor = rgb;
        bcInfo->hBkBrush = hBrush;
    }

    return eRet;
}
static HANERROR HANBarChartSetBackgroundBorderCallback(HANPBARCHART bcInfo, COLORREF rgb)
{
    HANERROR eRet = RET_OK;
    HPEN hPen = CreatePen(PS_SOLID, 0, rgb);

    if (NULL == hPen) { eRet = CREATE_PEN_ERR; }
    if (RET_OK == eRet)
    {
        if (NULL != bcInfo->hBkBorder) { DeleteObject(bcInfo->hBkBorder); }
        bcInfo->crBkBorder = rgb;
        bcInfo->hBkBorder = hPen;
    }

    return eRet;
}
static HANERROR HANBarChartSetTitleFontCallback(HANPBARCHART bcInfo, HANINT w, HANINT h, HANPCSTR Face)
{
    HANERROR eRet = RET_OK;
    HFONT hFont = CreateFont(h, w, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, Face);
    
    if (NULL == hFont) { eRet = CREATE_FONT_ERR; }
    if (RET_OK == eRet)
    {
        if (NULL != bcInfo->hTitleFont) { DeleteObject(bcInfo->hTitleFont); }
        bcInfo->nTitleW = abs(w);
        bcInfo->nTitleH = abs(h);
        bcInfo->hTitleFont = hFont;
    }

    return eRet;
}
static HANERROR HANBarChartSetTextFontCallback(HANPBARCHART bcInfo, HANINT w, HANINT h, HANPCSTR Face)
{
    HANERROR eRet = RET_OK;
    HFONT hFont = CreateFont(h, w, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, Face);
    
    if (NULL == hFont) { eRet = CREATE_FONT_ERR; }
    if (RET_OK == eRet)
    {
        if (NULL != bcInfo->hTextFont) { DeleteObject(bcInfo->hTextFont); }
        bcInfo->nTextW = abs(w);
        bcInfo->nTextH = abs(h);
        bcInfo->hTextFont = hFont;
    }
    
    return eRet;
}
static void HANBarChartSetRcRectangleCallback(HWND hBarChart, HANPBARCHART bcInfo, const RECT* rcRectangle, BOOL bAutoRect)
{
    bcInfo->rcRectangle = *rcRectangle;
    if (TRUE == bAutoRect)
    {
        RECT rcWin;
        GetClientRect(hBarChart, &rcWin);
        bcInfo->rcRectangle.right = rcWin.right - rcRectangle->right;
        bcInfo->rcRectangle.bottom = rcWin.bottom - rcRectangle->bottom;
        bcInfo->rcAutoRectDis = *rcRectangle;
    }
    bcInfo->bAutoRect = bAutoRect;
}
static BOOL HANBarChartGetRcRectangleCallback(HANPBARCHART bcInfo, RECT* rcRectangle)
{
    *rcRectangle = bcInfo->rcRectangle;
    return bcInfo->bAutoRect;
}
static void HANBarChartSetVValueCountCallback(HANPBARCHART bcInfo, unsigned int n)
{
    if (n < (unsigned int)2) { bcInfo->nVValueCount = 2; }
    else { bcInfo->nVValueCount = n; }
}
static void HANBarChartSetBarWidthCallback(HANPBARCHART bcInfo, HANINT w)
{
    bcInfo->nBarWidth = w;
}
static void HANBarChartSetBarIntervalCallback(HANPBARCHART bcInfo, HANINT nInterval)
{
    bcInfo->nBarInterval = nInterval;
}
#endif

#if 1 /******************** HANPolyLine ********************/
// 子窗口 HMENU
#define HPL_WID_MOVE            0
#define HPL_WID_ENLARGE         1
#define HPL_WID_REDUCE          2
#define HPL_WID_REFRESH         3
// 子窗口 button 参数
#define HPL_CWY_BUTTON          10  // Y 坐标
#define HPL_CWDX_BUTTON         10  // 横向间隔
#define HPL_CWSIZE_BUTTON       20  // 边长
#define HPL_VAR_TEXT_Y_BEGIN    (HPL_CWY_BUTTON + HPL_CWSIZE_BUTTON)
typedef struct tagHANPOLYLINEDP { // 绘图参数
    BOOL            bLeftExist;     // 有左侧变量
    BOOL            bRightExist;    // 有右侧变量
    HANPLOTNUM      nRightMax;      // 右侧最大值
    HANPLOTNUM      nLeftMax;       // 左侧最大值
    HANPLOTNUM      nRightMin;      // 右侧最小值
    HANPLOTNUM      nLeftMin;       // 左侧最大值
    HANPLOTNUM      nRightRange;    // 右侧取值范围
    HANPLOTNUM      nLeftRange;     // 左侧取值范围
    HANSIZE         sIdBegin;       // 在没有裁剪数据的情况下，需要画的数据起始下标
    HANSIZE         sDrawDataLen;   // 在没有裁剪数据的情况下，需要画的数据总长
    LONG            w;              // 绘图区域宽
    LONG            h;              // 绘图区域高
    BOOL            bPoint;         // 绘制点
    BOOL            bLine;          // 绘制线
    HANSIZE         nPointIdBegin;  // 裁剪后，绘图时计算出来的 pBufPoint 的起始 id
    HANSIZE         nPointIdLen;    // 裁剪后，绘图时计算出来的 pBufPoint 的长度
} HANPOLYLINEDP, * HANPPOLYLINEDP;
typedef struct tagHANPOLYLINE HANPOLYLINE, * HANPPOLYLINE;
typedef const HANPOLYLINE* HANPCPOLYLINE;
struct tagHANPOLYLINE { // 折线图结构体，也是类附加空间存放的指针指向的窗口信息
    HANDLE          hHeap;          // 堆句柄
    BOOL            bRedraw;        // 重绘 flag
    HANSIZE         nVarCount;      // 变量个数
    HANSIZE         nDataLen;       // 每个变量容纳的数据个数
    HANPPLOTDS      pVarData;       // 变量的具体信息
    HANPPLOTDS      pTopVar;        // 顶部数值对应的变量
    HANPPLOTDS      pBottomVar;     // 底部数值对应的变量
    HANSIZE         nHValueCount;   // 绘图界面上水平方向显示的值的个数
    HANSIZE         nVValueCount;   // 绘图界面上垂直方向显示的值的个数
    HPEN            hMarkLine;      // 标线画笔
    BOOL            bAutoRect;      // 自动大小模式
    RECT            rcAutoRectDis;  // 自动大小模式下绘图区到窗口的距离
    RECT            rcRectangle;    // 绘图区域的矩形范围
    HANLSIZE        lVRectangleW;   // 虚拟矩形（画下全部点所需要的矩形）的宽度，绘图时根据实际矩形的宽度和该宽度计算需要绘制的点
    HANLSIZE        lRectangleX;    // 绘图矩形区域在虚拟区的 X 坐标
    LONG            lRcInvH;        // 绘图时，在矩形区域上下留出的空白部分的高，该值必须小于 rcRectangle 的高的一半
    COLORREF        crTitleColor;   // 标题文字颜色
    HANINT             nTitleW;        // 标题文字字体宽
    HANINT             nTitleH;        // 标题文字字体高
    HFONT           hTitleFont;     // 标题文字字体对象
    COLORREF        crTextColor;    // 其它文字颜色
    HANINT             nTextW;         // 其它文字字体宽
    HANINT             nTextH;         // 其它文字字体高
    HFONT           hTextFont;      // 其它文字字体对象
    COLORREF        crBkColor;      // 窗口背景色
    COLORREF        crBkBorder;     // 窗口背景边框颜色
    HPEN            hBkBorder;      // 窗口前景边框画笔句柄
    HBRUSH          hBkBrush;       // 窗口背景画刷
    COLORREF        crRcColor;      // 绘图区域背景色
    COLORREF        crRcBorder;     // 绘图区域边框颜色
    HBRUSH          hRcBrush;       // 绘图区域背景画刷
    HBRUSH          hRcDarBrush;    // 绘图区域背景暗色画刷
    HPEN            hRcBorder;      // 绘图区域边框画笔句柄
    PPOINT          pBufPoint;      // 绘制点的缓存，在设置数据长度时同时分配好该缓存，避免绘图时反复申请内存
    HANPSIZE        pBufId;         // pBufPoint 中每个点对应的数组下标
    HANSIZE         nTipW;          // 鼠标悬停数值提示区域宽
    HANINT             nTipTextH;      // 鼠标悬停数值提示文字高度
    HFONT           hTipFont;       // 鼠标悬停数值提示字体
    COLORREF        crTipBk;        // 鼠标悬停数值提示背景颜色
    COLORREF        crTipBorder;    // 鼠标悬停数值提示边框颜色
    HPEN            hTipBorder;     // 鼠标悬停数值提示边框画笔
    HPEN            hTipBkPen;      // 鼠标悬停数值提示背景画笔
    HBRUSH          hTipBkBrush;    // 鼠标悬停数值提示背景画刷
    HFONT           hButtonFont;    // 按钮字体
    HWND            hbMove;         // 移动按钮句柄
    HWND            hbEnlarge;      // 放大按钮句柄
    HWND            hbReduce;       // 缩小按钮句柄
    HWND            hbRefresh;      // 刷新按钮句柄
    EPLOTCURSORMODE eCursorMode;    // 鼠标指针模式
    EPLOTMOUSEACT   eLastMouseAct;  // 最后一次鼠标动作
    POINT           ptMouseActPos;  // 鼠标按下 / 弹起的位置
    LONG            nEnlargeBeginX; // 放大模式下鼠标按下的位置
    LONG            nEnlargeCurX;   // 放大模式下鼠标的当前位置
    LONG            nMoveBeginX;    // 移动模式下上一刻鼠标的位置
    BOOL            bEnlage;        // 放大 flag
    BOOL            bMove;          // 移动 flag
    HANINT             nPointSize;     // 点的直径
    HANINT             nLineWidth;     // 线宽
    void            (*DrawLine)(HDC, HANPPOLYLINE); // 绘图接口
    BOOL            bLeftMaxMin;    // 由用户设置了左侧最值
    HANPLOTMAXMIN   lMaxMin;
    HANPLOTNUM      nLeftMax;       // 用户设置的左侧最大值
    HANPLOTNUM      nLeftMin;       // 用户设置的左侧最小值
    BOOL            bRightMaxMin;   // 由用户设置了右侧最值
    HANPLOTMAXMIN   rMaxMin;
    HANPLOTNUM      nRightMax;      // 用户设置的右侧最大值
    HANPLOTNUM      nRightMin;      // 用户设置的右侧最大值
    BOOL            bCut;           // 裁剪数据
    HANSIZE         nCutBegin;      // 裁剪的起始下标
    HANSIZE         nCutEnd;        // 裁剪的截止下标
    HANPOLYLINEDP   dpDrawParam;    // 绘图参数，绘图时所有参数在这里计算
};
// 内部函数声明
static HANERROR HANPolyLineUpdateResources(HANPPOLYLINE plInfo);
static void HANPolyLinePaintCallback(HDC hdc, HWND hPolyLine, HANPPOLYLINE plInfo, const RECT* rcWin);
static void HANPolyLineTextOut(HDC hdc, HWND hPolyLine, HANPPOLYLINE plInfo, const RECT* rcWin);
static void HANPolyLineLineOut(HDC hdc, HWND hPolyLine, HANPPOLYLINE plInfo);
static void HANPolyLineLineOutFast(HDC hdc, HANPPOLYLINE plInfo);
static void HANPolyLineLineOutHighAccurate(HDC hdc, HANPPOLYLINE plInfo);
static void HANPolyLineDrawTip(HDC hdc, HWND hPolyLine, HANPCPOLYLINE plInfo);
static void HANPolyLineMoveButtonCallback(HANPPOLYLINE plInfo);
static void HANPolyLineEnlargeButtonCallback(HANPPOLYLINE plInfo);
static BOOL HANPolyLineReduceButtonCallback(HANPPOLYLINE plInfo);
static BOOL HANPolyLineRefreshButtonCallback(HANPPOLYLINE plInfo, BOOL bResetCursor);
static void HANPolyLineSizeCallback(HWND hPolyLine, HANPPOLYLINE plInfo);
static BOOL HANPolyLineMouseMoveCallback(HWND hPolyLine, HANPPOLYLINE plInfo, WPARAM wParam, LPARAM lParam);
static void HANPolyLineLButtonDownCallback(HANPPOLYLINE plInfo, LPARAM lParam);
static BOOL HANPolyLineLButtonUpCallback(HWND hPolyLine, HANPPOLYLINE plInfo, LPARAM lParam);
static HANPPLOTDS HANPolyLineMouseInVarText(HWND hPolyLine, HANPCPOLYLINE plInfo, const POINT* pptMousePos);
static BOOL HANPolyLineDrawVar(HANPCPLOTDS pVar);
// WndProc & 实现 声明
static LRESULT CALLBACK HANPolyLineWndProc(HWND hPolyLine, UINT message, WPARAM wParam, LPARAM lParam);
static void HANPolyLineTimerCallback(HWND hPolyLine, HANPPOLYLINE plInfo);
static HANERROR HANPolyLineSetDataLenCallback(HANPPOLYLINE plInfo, HANSIZE nDataLen);
static HANERROR HANPolyLineAddVarCallback(HANPPOLYLINE plInfo, HANPCSTR pVarName, EHPLVARPOS ePos);
static HANERROR HANPolyLineUpdateDataByNameCallback(HANPPOLYLINE plInfo, HANPCSTR pVarName, HANPCPLOTDATA pData, EPLOTUPDATEDATA modeUpdate);
static HANERROR HANPolyLineUpdateDataByIdCallback(HANPPOLYLINE plInfo, HANSIZE nId, HANPCPLOTDATA pData, EPLOTUPDATEDATA modeUpdate);
static HANERROR HANPolyLineSetVarColorCallback(HANPPOLYLINE plInfo, HANPCSTR pVarName, HANSIZE nId, COLORREF rgb);
static void HANPolyLineSetLeftMaxMinCallback(HANPPOLYLINE plInfo, HANPCPLOTMAXMIN lMaxMin, BOOL bUserMaxMin);
static void HANPolyLineSetRightMaxMinCallback(HANPPOLYLINE plInfo, HANPCPLOTMAXMIN rMaxMin, BOOL bUserMaxMin);
static HANERROR HANPolyLineSetBackgroundColorCallback(HANPPOLYLINE plInfo, COLORREF rgb);
static HANERROR HANPolyLineSetBackgroundBorderCallback(HANPPOLYLINE plInfo, COLORREF rgb);
static HANERROR HANPolyLineSetRectangleColorCallback(HANPPOLYLINE plInfo, COLORREF rgb);
static HANERROR HANPolyLineSetRectangleBorderCallback(HANPPOLYLINE plInfo, COLORREF rgb);
static HANERROR HANPolyLineSetTitleFontCallback(HANPPOLYLINE plInfo, HANINT w, HANINT h, HANPCSTR Face);
static HANERROR HANPolyLineSetTextFontCallback(HANPPOLYLINE plInfo, HANINT w, HANINT h, HANPCSTR Face);
static void HANPolyLineSetRcRectangleCallback(HWND hPolyLine, HANPPOLYLINE plInfo, const RECT* rcRectangle, BOOL bAutoRect);
static BOOL HANPolyLineGetRcRectangleCallback(HANPPOLYLINE plInfo, RECT* rcRectangle);
static HANERROR HANPolyLineSetRcInvHCallback(HANPPOLYLINE plInfo, LONG nHeight);
static void HANPolyLineSetHValueCountCallback(HANPPOLYLINE plInfo, HANSIZE n);
static void HANPolyLineSetVValueCountCallback(HANPPOLYLINE plInfo, HANSIZE n);
static void HANPolyLineEnableMouseCallback(HWND hPolyLine, HANPPOLYLINE plInfo, BOOL bEnable);
static void HANPolyLineSetDataModeCallback(HANPPOLYLINE plInfo, EHPLDATAMODE dmDataMode);
static HANERROR HANPolyLineSetLineSizeCallback(HANPPOLYLINE plInfo, HANINT nWidth);
static void HANPolyLineSetPointSizeCallback(HANPPOLYLINE plInfo, HANINT nSize);
static HANERROR HANPolyLineSetPointStyleCallback(HANPPOLYLINE plInfo, HANPCSTR pVarName, HANSIZE nId, HANHPOINT hPoint);
static void HANPolyLineDrawPointLineCallback(HWND hPolyLine, BOOL bPoint, BOOL bLine);
static void HANPolyLineCutDataCallback(HANPPOLYLINE plInfo, HANPCSIZE pPos, BOOL bCut);
static void HANPolyLineSetTipWCallback(HANPPOLYLINE plInfo, HANSIZE w);
static HANERROR HANPolyLineSetTipFontCallback(HANPPOLYLINE plInfo, HANINT w, HANINT h, HANPCSTR Face);
static HANERROR HANPolyLineSetTipBkColorCallback(HANPPOLYLINE plInfo, COLORREF rgb);
static HANERROR HANPolyLineSetTipBorderCallback(HANPPOLYLINE plInfo, COLORREF rgb);
// 接口定义
void RegisterHANPolyLine(HINSTANCE hInstance)
{
    WNDCLASSEX wcexHANPolyLine = {
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_PARENTDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = HANPolyLineWndProc,
        .cbClsExtra = 0,
        .cbWndExtra = sizeof(HANPPOLYLINE),
        .hInstance = hInstance,
        .hIcon = 0,
        .hCursor = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName = NULL,
        .lpszClassName = HPL_CLASS,
        .hIconSm = NULL
    };
    RegisterClassEx(&wcexHANPolyLine);
}
HANERROR HANPolyLineSetDataLen(HWND hPolyLine, HANSIZE nDataLen)
{
    return SendMessage(hPolyLine, HPLM_SETDATALEN, (WPARAM)nDataLen, 0);
}
HANERROR HANPolyLineAddVar(HWND hPolyLine, HANPCSTR pVarName, EHPLVARPOS vpPos)
{
    return SendMessage(hPolyLine, HPLM_ADDVAR, (WPARAM)vpPos, (LPARAM)pVarName);
}
HANERROR HANPolyLineSetStartDataByName(HWND hPolyLine, HANPCSTR pVarName, HANPCPLOTDATA pData)
{
    HANERROR ret = SendMessage(hPolyLine, HPLM_SETSTARTDATABYNAME, (WPARAM)pVarName, (LPARAM)pData);
    return ret;
}
HANERROR HANPolyLinePushStartDataByName(HWND hPolyLine, HANPCSTR pVarName, HANPCPLOTDATA pData)
{
    HANERROR ret = SendMessage(hPolyLine, HPLM_PUSHSTARTDATABYNAME, (WPARAM)pVarName, (LPARAM)pData);
    return ret;
}
HANERROR HANPolyLineSetEndDataByName(HWND hPolyLine, HANPCSTR pVarName, HANPCPLOTDATA pData)
{
    HANERROR ret = SendMessage(hPolyLine, HPLM_SETENDDATABYNAME, (WPARAM)pVarName, (LPARAM)pData);
    return ret;
}
HANERROR HANPolyLinePushEndDataByName(HWND hPolyLine, HANPCSTR pVarName, HANPCPLOTDATA pData)
{
    HANERROR ret = SendMessage(hPolyLine, HPLM_PUSHENDDATABYNAME, (WPARAM)pVarName, (LPARAM)pData);
    return ret;
}
HANERROR HANPolyLineSetStartDataById(HWND hPolyLine, HANSIZE nId, HANPCPLOTDATA pData)
{
    HANERROR ret = SendMessage(hPolyLine, HPLM_SETSTARTDATABYID, (WPARAM)nId, (LPARAM)pData);
    return ret;
}
HANERROR HANPolyLinePushStartDataById(HWND hPolyLine, HANSIZE nId, HANPCPLOTDATA pData)
{
    HANERROR ret = SendMessage(hPolyLine, HPLM_PUSHSTARTDATABYID, (WPARAM)nId, (LPARAM)pData);
    return ret;
}
HANERROR HANPolyLineSetEndDataById(HWND hPolyLine, HANSIZE nId, HANPCPLOTDATA pData)
{
    HANERROR ret = SendMessage(hPolyLine, HPLM_SETENDDATABYID, (WPARAM)nId, (LPARAM)pData);
    return ret;
}
HANERROR HANPolyLinePushEndDataById(HWND hPolyLine, HANSIZE nId, HANPCPLOTDATA pData)
{
    HANERROR ret = SendMessage(hPolyLine, HPLM_PUSHENDDATABYID, (WPARAM)nId, (LPARAM)pData);
    return ret;
}
HANERROR HANPolyLineSetVarColorByName(HWND hPolyLine, HANPCSTR pVarName, COLORREF rgb, BOOL bRedraw)
{
    HANERROR ret = SendMessage(hPolyLine, HPLM_SETVARCOLORBYNAME, (WPARAM)pVarName, (LPARAM)rgb);
    if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hPolyLine, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    return ret;
}
HANERROR HANPolyLineSetVarColorById(HWND hPolyLine, HANSIZE nId, COLORREF rgb, BOOL bRedraw)
{
    HANERROR ret = SendMessage(hPolyLine, HPLM_SETVARCOLORBYID, (WPARAM)nId, (LPARAM)rgb);
    if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hPolyLine, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    return ret;
}
void HANPolyLineSetLeftMaxMin(HWND hPolyLine, HANPCPLOTMAXMIN lMaxMin, BOOL bUserMaxMin, BOOL bRedraw)
{
    SendMessage(hPolyLine, HPLM_SETLEFTMAXMIN, (WPARAM)bUserMaxMin, (LPARAM)lMaxMin);
    if (TRUE == bRedraw) { RedrawWindow(hPolyLine, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
}
void HANPolyLineSetRightMaxMin(HWND hPolyLine, HANPCPLOTMAXMIN rMaxMin, BOOL bUserMaxMin, BOOL bRedraw)
{
    SendMessage(hPolyLine, HPLM_SETRIGHTMAXMIN, (WPARAM)bUserMaxMin, (LPARAM)rMaxMin);
    if (TRUE == bRedraw) { RedrawWindow(hPolyLine, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
}
HANERROR HANPolyLineSetBackgroundColor(HWND hPolyLine, COLORREF rgb, BOOL bRedraw)
{
    HANERROR ret = SendMessage(hPolyLine, HPLM_SETBKCOLOR, (WPARAM)rgb, 0);
    if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hPolyLine, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    return ret;
}
HANERROR HANPolyLineSetBkBorder(HWND hPolyLine, COLORREF rgb, BOOL bRedraw)
{
    HANERROR ret = SendMessage(hPolyLine, HPLM_SETBKBORDER, (WPARAM)rgb, 0);
    if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hPolyLine, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    return ret;
}
HANERROR HANPolyLineSetRectangleColor(HWND hPolyLine, COLORREF rgb, BOOL bRedraw)
{
    HANERROR ret = SendMessage(hPolyLine, HPLM_SETRECTCOLOR, (WPARAM)rgb, 0);
    if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hPolyLine, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    return ret;
}
HANERROR HANPolyLineSetRectBorder(HWND hPolyLine, COLORREF rgb, BOOL bRedraw)
{
    HANERROR ret = SendMessage(hPolyLine, HPLM_SETRECTBORDER, (WPARAM)rgb, 0);
    if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hPolyLine, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    return ret;
}
HANERROR HANPolyLineSetTitleFont(HWND hPolyLine, HANINT w, HANINT h, HANPCSTR pFace, BOOL bRedraw)
{
    HANINT pTextSize[] = { w, h };
    HANERROR ret = SendMessage(hPolyLine, HPLM_SETTITLEFONT, (WPARAM)pTextSize, (LPARAM)pFace);
    if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hPolyLine, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    return ret;
}
HANERROR HANPolyLineSetTextFont(HWND hPolyLine, HANINT w, HANINT h, HANPCSTR pFace, BOOL bRedraw)
{
    HANINT pTextSize[] = { w, h };
    HANERROR ret = SendMessage(hPolyLine, HPLM_SETTEXTFONT, (WPARAM)pTextSize, (LPARAM)pFace);
    if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hPolyLine, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    return ret;
}
void HANPolyLineSetRcRectangle(HWND hPolyLine, const RECT* rcRectangle, BOOL bAutoRect, BOOL bRedraw)
{
    SendMessage(hPolyLine, HPLM_SETRECT, (WPARAM)bAutoRect, (LPARAM)rcRectangle);
    if (TRUE == bRedraw) { RedrawWindow(hPolyLine, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
}
BOOL HANPolyLineGetRcRectangle(HWND hPolyLine, RECT* rcRectangle)
{
    return (BOOL)SendMessage(hPolyLine, HPLM_GETRECT, 0, (LPARAM)rcRectangle);
}
HANERROR HANPolyLineSetRcInvH(HWND hPolyLine, LONG nHeight, BOOL bRedraw)
{
    HANERROR ret = SendMessage(hPolyLine, HPLM_SETRCINVH, (WPARAM)nHeight, 0);
    if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hPolyLine, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    return ret;
}
void HANPolyLineSetHValueCount(HWND hPolyLine, unsigned int n, BOOL bRedraw)
{
    SendMessage(hPolyLine, HPLM_SETHVALCOUNT, (WPARAM)n, 0);
    if (TRUE == bRedraw) { RedrawWindow(hPolyLine, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
}
void HANPolyLineSetVValueCount(HWND hPolyLine, unsigned int n, BOOL bRedraw)
{
    SendMessage(hPolyLine, HPLM_SETVVALCOUNT, (WPARAM)n, 0);
    if (TRUE == bRedraw) { RedrawWindow(hPolyLine, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
}
void HANPolyLineEnableMouse(HWND hPolyLine, BOOL bEnable)
{
    SendMessage(hPolyLine, HPLM_ENABLEMOUSE, (WPARAM)bEnable, 0);
}
void HANPolyLineSetDataMode(HWND hPolyLine, EHPLDATAMODE dmDataMode, BOOL bRedraw)
{
    SendMessage(hPolyLine, HPLM_SETDATAMODE, (WPARAM)dmDataMode, 0);
    if (TRUE == bRedraw) { RedrawWindow(hPolyLine, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
}
HANERROR HANPolyLineSetPointLineSize(HWND hPolyLine, EHPLPLSTYLE eLineStyle, HANINT nObjSize, BOOL bRedraw)
{
    HANERROR ret = SendMessage(hPolyLine, HPLM_SETPOINTLINESIZE, (WPARAM)eLineStyle, (LPARAM)nObjSize);
    if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hPolyLine, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    return ret;
}
HANERROR HANPolyLineSetPointStyleByName(HWND hPolyLine, HANPCSTR pVarName, HANHPOINT hPoint, BOOL bRedraw)
{
    HANERROR ret;
    if (NULL != hPoint)
    {
        ret = SendMessage(hPolyLine, HPLM_SETPOINTSTYLEBYNAME, (WPARAM)pVarName, (LPARAM)hPoint);
        if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hPolyLine, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    }
    else
    {
        ret = PLOT_INVALID_HPOINT;
    }
    return ret;
}
HANERROR HANPolyLineSetPointStyleById(HWND hPolyLine, HANSIZE nId, HANHPOINT hPoint, BOOL bRedraw)
{
    HANERROR ret;
    if (NULL != hPoint)
    {
        ret = SendMessage(hPolyLine, HPLM_SETPOINTSTYLEBYID, (WPARAM)nId, (LPARAM)hPoint);
        if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hPolyLine, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    }
    else
    {
        ret = PLOT_INVALID_HPOINT;
    }
    return ret;
}
void HANPolyLineDrawPointLine(HWND hPolyLine, BOOL bPoint, BOOL bLine, BOOL bRedraw)
{
    SendMessage(hPolyLine, HPLM_DRAWPOINTLINE, (WPARAM)bPoint, (LPARAM)bLine);
    if (TRUE == bRedraw) { RedrawWindow(hPolyLine, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
}
void HANPolyLineCutData(HWND hPolyLine, HANPCSIZE pPos, BOOL bCut, BOOL bRedraw)
{
    SendMessage(hPolyLine, HPLM_CUTDATA, (WPARAM)bCut, (LPARAM)pPos);
    if (TRUE == bRedraw) { RedrawWindow(hPolyLine, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
}
void HANPolyLineSetTipW(HWND hPolyLine, HANSIZE w, BOOL bRedraw)
{
    SendMessage(hPolyLine, HPLM_SETTIPW, (WPARAM)w, 0);
    if (TRUE == bRedraw) { RedrawWindow(hPolyLine, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
}
HANERROR HANPolyLineSetTipFont(HWND hPolyLine, HANINT w, HANINT h, HANPCSTR pFace, BOOL bRedraw)
{
    HANINT pTextSize[] = { w, h };
    HANERROR ret = SendMessage(hPolyLine, HPLM_SETTIPFONT, (WPARAM)pTextSize, (LPARAM)pFace);
    if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hPolyLine, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    return ret;
}
HANERROR HANPolyLineSetTipBkColor(HWND hPolyLine, COLORREF rgb, BOOL bRedraw)
{
    HANERROR ret = SendMessage(hPolyLine, HPLM_SETTIPBKCOLOR, (WPARAM)rgb, 0);
    if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hPolyLine, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    return ret;
}
HANERROR HANPolyLineSetTipBorder(HWND hPolyLine, COLORREF rgb, BOOL bRedraw)
{
    HANERROR nRet = SendMessage(hPolyLine, HPLM_SETTIPBORDER, (WPARAM)rgb, 0);
    if (bRedraw && (RET_OK == nRet)) { RedrawWindow(hPolyLine, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    return nRet;
}
// 内部函数定义
static HANERROR HANPolyLineUpdateResources(HANPPOLYLINE plInfo)
{
    HANERROR nRet = RET_OK;
    HANPPLOTDS pNode = plInfo->pVarData;
    HPEN* hPen = (HPEN*)HANWinHeapAlloc(plInfo->hHeap, NULL, (plInfo->nVarCount) * sizeof(HPEN));
    HBRUSH* hBrush = (HBRUSH*)HANWinHeapAlloc(plInfo->hHeap, NULL, (plInfo->nVarCount) * sizeof(HBRUSH));
    if ((NULL == hPen) || (NULL == hBrush)) { nRet = NOT_ENOUGH_MEMORY; }
    if (RET_OK == nRet)
    {
        for (HANSIZE i = 0; i < plInfo->nVarCount; i++)
        {
            hPen[i] = CreatePen(PS_SOLID, plInfo->nLineWidth, pNode->crColor);
            hBrush[i] = CreateSolidBrush(pNode->crColor);
            if ((NULL == hPen[i]) || (NULL == hBrush[i]))
            {
                for (HANSIZE j = 0; j <= i; j++)
                {
                    if (NULL != hPen[i]) { DeleteObject(hPen[i]); }
                    if (NULL != hBrush[i]) { DeleteObject(hBrush[i]); }
                }
                nRet = CREATE_OBJECT_ERR;
                break;
            }
            pNode = pNode->next;
        }
    }
    if (RET_OK == nRet)
    {
        pNode = plInfo->pVarData;
        for (HANSIZE i = 0; i < plInfo->nVarCount; i++)
        {
            if (NULL != pNode->hPen) { DeleteObject(pNode->hPen); }
            if (NULL != pNode->hBrush) { DeleteObject(pNode->hBrush); }
            pNode->hPen = hPen[i];
            pNode->hBrush = hBrush[i];
            pNode = pNode->next;
        }
    }

    if (NULL != hPen) { HANWinHeapFree(plInfo->hHeap, 0, hPen); }
    if (NULL != hBrush) { HANWinHeapFree(plInfo->hHeap, 0, hBrush); }
    return RET_OK;
}
static void HANPolyLinePaintCallback(HDC hdc, HWND hPolyLine, HANPPOLYLINE plInfo, const RECT* rcWin)
{
    HANPCPLOTDS pNode = plInfo->pVarData;
    plInfo->dpDrawParam.bLeftExist = FALSE;
    plInfo->dpDrawParam.bRightExist = FALSE;
    // 获取最值，先算原始数据里的最值
    while (NULL != pNode)
    {
        if (HPL_VAR_POS_LEFT == (EHPLVARPOS)(pNode->ePos))
        {
            if (TRUE == plInfo->dpDrawParam.bLeftExist)
            {
                if (pNode->nMax > plInfo->dpDrawParam.nLeftMax) { plInfo->dpDrawParam.nLeftMax = pNode->nMax; }
                if (pNode->nMin < plInfo->dpDrawParam.nLeftMin) { plInfo->dpDrawParam.nLeftMin = pNode->nMin; }
            }
            else
            {
                plInfo->dpDrawParam.nLeftMax = pNode->nMax;
                plInfo->dpDrawParam.nLeftMin = pNode->nMin;
                plInfo->dpDrawParam.bLeftExist = TRUE;
            }
        }
        else if (HPL_VAR_POS_RIGHT == (EHPLVARPOS)(pNode->ePos))
        {
            if (TRUE == plInfo->dpDrawParam.bRightExist)
            {
                if (pNode->nMax > plInfo->dpDrawParam.nRightMax) { plInfo->dpDrawParam.nRightMax = pNode->nMax; }
                if (pNode->nMin < plInfo->dpDrawParam.nRightMin) { plInfo->dpDrawParam.nRightMin = pNode->nMin; }
            }
            else
            {
                plInfo->dpDrawParam.nRightMax = pNode->nMax;
                plInfo->dpDrawParam.nRightMin = pNode->nMin;
                plInfo->dpDrawParam.bRightExist = TRUE;
            }
        }
        else { /* idle */ }
        pNode = pNode->next;
    }
    // 跟据参数修正要画的最值
    if (TRUE == plInfo->dpDrawParam.bLeftExist)
    {
        if (TRUE == plInfo->bLeftMaxMin)
        {
            plInfo->dpDrawParam.nLeftMax = plInfo->nLeftMax;
            plInfo->dpDrawParam.nLeftMin = plInfo->nLeftMin;
        }
        else
        {
            HANPLOTNUM dLeftRange = plInfo->dpDrawParam.nLeftMax - plInfo->dpDrawParam.nLeftMin;
            HANDOUBLE dInvK = (HANDOUBLE)(plInfo->lRcInvH) / (GetRectH(&(plInfo->rcRectangle)) - (2 * plInfo->lRcInvH));
            plInfo->dpDrawParam.nLeftMax += dLeftRange * dInvK;
            plInfo->dpDrawParam.nLeftMin -= dLeftRange * dInvK;
        }
    }
    if (TRUE == plInfo->dpDrawParam.bRightExist)
    {
        if (TRUE == plInfo->bRightMaxMin)
        {
            plInfo->dpDrawParam.nRightMax = plInfo->nRightMax;
            plInfo->dpDrawParam.nRightMin = plInfo->nRightMin;
        }
        else
        {
            HANPLOTNUM dRightRange = plInfo->dpDrawParam.nRightMax - plInfo->dpDrawParam.nRightMin;
            HANDOUBLE dInvK = ((HANDOUBLE)(plInfo->lRcInvH) / (GetRectH(&(plInfo->rcRectangle))) - (2 * plInfo->lRcInvH));
            plInfo->dpDrawParam.nRightMax += dRightRange * dInvK;
            plInfo->dpDrawParam.nRightMin -= dRightRange * dInvK;
        }
    }
    // 计算参数
    LONG lStyle = GetWindowLong(hPolyLine, GWL_STYLE);
    if (0 != (lStyle & HPLS_POINT)) { plInfo->dpDrawParam.bPoint = TRUE; } 
    else { plInfo->dpDrawParam.bPoint = FALSE; }
    if (0 != (lStyle & HPLS_LINE)) { plInfo->dpDrawParam.bLine = TRUE; } 
    else { plInfo->dpDrawParam.bLine = FALSE; }
    plInfo->dpDrawParam.w = GetRectW(&(plInfo->rcRectangle)) - 3;
    plInfo->dpDrawParam.h = GetRectH(&(plInfo->rcRectangle)) - 3;
    plInfo->dpDrawParam.nLeftRange = plInfo->dpDrawParam.nLeftMax - plInfo->dpDrawParam.nLeftMin;
    plInfo->dpDrawParam.nRightRange = plInfo->dpDrawParam.nRightMax - plInfo->dpDrawParam.nRightMin;
    // 画线
    HANPolyLineLineOut(hdc, hPolyLine, plInfo);
    // 画背景，保存现场 hdc
    HPEN hOldPen = SelectObject(hdc, GetStockObject(NULL_PEN));
    HBRUSH hOldBrush = SelectObject(hdc, plInfo->hBkBrush);
    LONG nWinW = GetRectW(rcWin);
    LONG nWinH = GetRectH(rcWin);
    Rectangle(hdc, 1, 1, plInfo->rcRectangle.left + 1, nWinH);
    Rectangle(hdc, plInfo->rcRectangle.left, 1, plInfo->rcRectangle.right + 1, plInfo->rcRectangle.top + 1);
    Rectangle(hdc, plInfo->rcRectangle.right, 1, nWinW, nWinH);
    Rectangle(hdc, plInfo->rcRectangle.left, plInfo->rcRectangle.bottom, plInfo->rcRectangle.right + 1, nWinH);
    // 画边框
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    SelectObject(hdc, plInfo->hBkBorder);
    Rectangle(hdc, 0, 0, nWinW, nWinH);
    SelectObject(hdc, plInfo->hRcBorder);
    Rectangle(hdc, plInfo->rcRectangle.left, plInfo->rcRectangle.top, plInfo->rcRectangle.right, plInfo->rcRectangle.bottom);
    // 还原 hdc
    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
    // 输出文字
    HANPolyLineTextOut(hdc, hPolyLine, plInfo, rcWin);
    // 输出鼠标悬停数值提示
    HANPolyLineDrawTip(hdc, hPolyLine, plInfo);
}
static void HANPolyLineTextOut(HDC hdc, HWND hPolyLine, HANPPOLYLINE plInfo, const RECT* rcWin)
{
    HANCHAR pText[HAN_PLOT_STR_LEN_MAX];
    LONG nWinW = GetRectW(rcWin);
    // 设置 hdc 参数：标题文字颜色、字体、对齐方式、透明背景
    COLORREF crOldTextColor = SetTextColor(hdc, plInfo->crTitleColor);
    HFONT hOldFont = SelectObject(hdc, plInfo->hTitleFont);
    UINT nOldTextAlign = SetTextAlign(hdc, TA_CENTER | TA_TOP);
    HANINT nOldBkMode = SetBkMode(hdc, TRANSPARENT);
    // 绘制标题
    GetWindowText(hPolyLine, pText, HAN_PLOT_STR_LEN_MAX);
    TextOut(hdc, nWinW / 2, 1, pText, (HANINT)(HAN_strlen(pText)));
    // 绘制变量
    HANINT nTextY = HPL_VAR_TEXT_Y_BEGIN;
    HANPCPLOTDS pNode = plInfo->pVarData;
    HANPCSTR pName;
    SelectObject(hdc, plInfo->hTextFont);
    while (NULL != pNode)
    {
        if ((HPL_VAR_POS_LEFT == (EHPLVARPOS)(pNode->ePos)) || (HPL_VAR_POS_RIGHT == (EHPLVARPOS)(pNode->ePos)))
        {
            SetTextAlign(hdc, TA_RIGHT | TA_TOP);
            SetTextColor(hdc, pNode->crColor);
            pName = pNode->pName;
            TextOut(hdc, nWinW - 10, nTextY, pName, (HANINT)(HAN_strlen(pName)));
            nTextY += plInfo->nTextH;
        }
        else if (HPL_VAR_POS_BOTTOM == (EHPLVARPOS)(pNode->ePos))
        {
            SetTextAlign(hdc, TA_CENTER | TA_TOP);
            SetTextColor(hdc, plInfo->crTitleColor);
            TextOut(hdc,
                plInfo->rcRectangle.left + GetRectW(&(plInfo->rcRectangle)) / 2,
                plInfo->rcRectangle.bottom + (plInfo->nTextH + 6),
                pNode->pName, (HANINT)(HAN_strlen(pNode->pName)));
        }
        else if (HPL_VAR_POS_TOP == (EHPLVARPOS)(pNode->ePos))
        {
            SetTextAlign(hdc, TA_CENTER | TA_BOTTOM);
            SetTextColor(hdc, plInfo->crTitleColor);
            TextOut(hdc,
                plInfo->rcRectangle.left + GetRectW(&(plInfo->rcRectangle)) / 2,
                plInfo->rcRectangle.top - (plInfo->nTextH + 6),
                pNode->pName, (HANINT)(HAN_strlen(pNode->pName)));
        }
        else { }
        pNode = pNode->next;
    }
    // 绘制数值
    if (plInfo->nDataLen > (HANSIZE)0)
    {
        HANDOUBLE kNum;
        HANINT nTextCharCount;
        // 绘制垂直数值
        SetTextColor(hdc, plInfo->crTextColor);
        HANINT nVValueBeginY = plInfo->rcRectangle.top - (plInfo->nTextH / 2);
        kNum = (HANDOUBLE)GetRectH(&(plInfo->rcRectangle)) / (HANDOUBLE)(plInfo->nVValueCount - (HANSIZE)1);
        for (HANSIZE i = 0; i < plInfo->nVValueCount; i++)
        {
            if (TRUE == plInfo->dpDrawParam.bLeftExist) // 绘制左侧垂直数值
            {
                SetTextAlign(hdc, TA_RIGHT);
                nTextCharCount = HAN_snprintf(
                    pText, HAN_PLOT_STR_LEN_MAX, TEXT("%g"),
                    plInfo->dpDrawParam.nLeftMax - (i * (plInfo->dpDrawParam.nLeftRange) / (plInfo->nVValueCount - (HANSIZE)1))
                );
                TextOut(hdc, plInfo->rcRectangle.left - (LONG)3, nVValueBeginY + (HANINT)((HANDOUBLE)i * kNum), pText, nTextCharCount);
            }
            if (TRUE == plInfo->dpDrawParam.bRightExist) // 绘制右侧垂直数值
            {
                SetTextAlign(hdc, TA_LEFT);
                nTextCharCount = HAN_snprintf(
                    pText, HAN_PLOT_STR_LEN_MAX, TEXT("%g"),
                    plInfo->dpDrawParam.nRightMax - (i * (plInfo->dpDrawParam.nLeftRange) / (plInfo->nVValueCount - (HANSIZE)1))
                );
                TextOut(hdc, plInfo->rcRectangle.right + (LONG)3, nVValueBeginY + (HANINT)((HANDOUBLE)i * kNum), pText, nTextCharCount);
            }
        }
        // 绘制水平数值
        HANINT nHValueBeginX = plInfo->rcRectangle.left;
        kNum = (HANDOUBLE)GetRectW(&(plInfo->rcRectangle)) / (HANDOUBLE)(plInfo->nHValueCount - (HANSIZE)1);
        HANSIZE idLen = (HANSIZE)round(plInfo->nDataLen * (HANDOUBLE)GetRectW(&(plInfo->rcRectangle)) / plInfo->lVRectangleW);
        HANSIZE idBegin = (HANSIZE)(plInfo->nDataLen * plInfo->lRectangleX / plInfo->lVRectangleW);
        HANSIZE idEnd = idBegin + idLen;
        if (idEnd > plInfo->nDataLen) { idLen -= idEnd - plInfo->nDataLen; } // 检查下标防止越界
        for (HANSIZE i = 0; i < plInfo->nHValueCount; i++)
        {
            SetTextAlign(hdc, TA_CENTER | TA_TOP);
            if (NULL != plInfo->pBottomVar)
            {
                nTextCharCount = HAN_snprintf(
                    pText, HAN_PLOT_STR_LEN_MAX, TEXT("%g"),
                    plInfo->pBottomVar->pData[idBegin + (i * (idLen - (HANSIZE)1) / (plInfo->nHValueCount - (HANSIZE)1))]
                );
            }
            else
            {
                nTextCharCount = HAN_snprintf(
                    pText, HAN_PLOT_STR_LEN_MAX, TEXT("%g"),
                    round(idBegin + ((i * (idLen - (HANSIZE)1)) / ((HANDOUBLE)(plInfo->nHValueCount) - (HANDOUBLE)1)))
                );
            }
            TextOut(hdc, nHValueBeginX + (HANINT)((HANDOUBLE)i * kNum), (HANINT)(plInfo->rcRectangle.bottom) + (plInfo->nTextH / 2), pText, nTextCharCount);
            if (NULL != plInfo->pTopVar)
            {
                nTextCharCount = HAN_snprintf(
                    pText, HAN_PLOT_STR_LEN_MAX, TEXT("%g"),
                    plInfo->pTopVar->pData[idBegin + (i * (idLen - (HANSIZE)1) / (plInfo->nHValueCount - (HANSIZE)1))]
                );
                SetTextAlign(hdc, TA_CENTER | TA_BOTTOM);
                TextOut(hdc, nHValueBeginX + (HANINT)((HANDOUBLE)i * kNum), (HANINT)(plInfo->rcRectangle.top) - (plInfo->nTextH / 2), pText, nTextCharCount);
            }
        }
    }
    // 还原 hdc 参数
    SetTextColor(hdc, crOldTextColor);
    SetTextAlign(hdc, nOldTextAlign);
    SetBkMode(hdc, nOldBkMode);
    SelectObject(hdc, hOldFont);
}
static void HANPolyLineLineOut(HDC hdc, HWND hPolyLine, HANPPOLYLINE plInfo)
{
    // 绘制区域矩形，保护 hdc
    HPEN hOldPen = SelectObject(hdc, GetStockObject(NULL_PEN));
    HBRUSH hOldBrush = SelectObject(hdc, plInfo->hRcBrush);
    Rectangle(hdc, plInfo->rcRectangle.left, plInfo->rcRectangle.top, plInfo->rcRectangle.right, plInfo->rcRectangle.bottom);
    SelectObject(hdc, plInfo->hRcDarBrush);
    // 绘制放大按钮按下时的阴影区域
    if (TRUE == plInfo->bEnlage)
    {
        HANINT nLeftX;
        HANINT nRightX;
        if (plInfo->nEnlargeBeginX < plInfo->nEnlargeCurX) { nLeftX = plInfo->nEnlargeBeginX; nRightX = plInfo->nEnlargeCurX; }
        else { nLeftX = plInfo->nEnlargeCurX; nRightX = plInfo->nEnlargeBeginX; }
        if (nLeftX < plInfo->rcRectangle.left) { nLeftX = plInfo->rcRectangle.left + 1; }
        if (nRightX > plInfo->rcRectangle.right) { nRightX = plInfo->rcRectangle.right - 1; }
        Rectangle(hdc, plInfo->rcRectangle.left + 1, plInfo->rcRectangle.top + 1, nLeftX, plInfo->rcRectangle.bottom - 1);
        Rectangle(hdc, nRightX, plInfo->rcRectangle.top + 1, plInfo->rcRectangle.right - 1, plInfo->rcRectangle.bottom - 1);
    }
    // 标线
    if (0 != (GetWindowLong(hPolyLine, GWL_STYLE) & HPLS_MARKLINE))
    {
        SelectObject(hdc, plInfo->hMarkLine);
        HANDOUBLE nLineDis = (GetRectH(&(plInfo->rcRectangle))) / ((HANDOUBLE)(plInfo->nVValueCount) - (HANDOUBLE)1);
        for (HANSIZE i = 1; i < (plInfo->nVValueCount - (HANSIZE)1); i++)
        {
            LONG nMarkLineY = plInfo->rcRectangle.top + (LONG)((HANDOUBLE)i * nLineDis);
            MoveToEx(hdc, plInfo->rcRectangle.left + 1, nMarkLineY, NULL);
            LineTo(hdc, plInfo->rcRectangle.right - 1, nMarkLineY);
        }
    }
    // 画线
    if (NULL != plInfo->pVarData)
    {
        plInfo->dpDrawParam.sDrawDataLen = (HANSIZE)round((HANDOUBLE)(plInfo->nDataLen) * GetRectW(&(plInfo->rcRectangle)) / plInfo->lVRectangleW);
        plInfo->dpDrawParam.sIdBegin = (HANSIZE)(plInfo->nDataLen * plInfo->lRectangleX / plInfo->lVRectangleW);
        HANSIZE idEnd = plInfo->dpDrawParam.sIdBegin + plInfo->dpDrawParam.sDrawDataLen;
        if (idEnd > plInfo->nDataLen) { plInfo->dpDrawParam.sDrawDataLen -= idEnd - plInfo->nDataLen; } // 检查下标防止越界
        plInfo->DrawLine(hdc, plInfo);
    }
    // 还原 hdc 参数
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
}
static void HANPolyLineLineOutFast(HDC hdc, HANPPOLYLINE plInfo)
{
    BOOL bRet = FALSE;
    HANSIZE sPointLen;
    HANSIZE sCutEnd = plInfo->nCutEnd;

    plInfo->dpDrawParam.nPointIdBegin = 0;
    if (sCutEnd < plInfo->dpDrawParam.sIdBegin) { bRet = TRUE; } // 绘制区不在裁剪区内，直接返回，防止之后无符号减法负数溢出
    if (!bRet)
    {
        if (sCutEnd > plInfo->nDataLen) { sCutEnd = plInfo->nDataLen; }
        if (plInfo->dpDrawParam.sDrawDataLen < (HANSIZE)(plInfo->dpDrawParam.w)) // 数据个数 < 画布长度，遍历数据
        {
            sPointLen = plInfo->dpDrawParam.sDrawDataLen;
            plInfo->dpDrawParam.nPointIdLen = sPointLen;
            if (TRUE == plInfo->bCut) // 计算 for 循环的起止 id，因遍历数据，直接计算 id 即可
            {
                if (plInfo->nCutBegin > plInfo->dpDrawParam.sIdBegin) { plInfo->dpDrawParam.nPointIdBegin = plInfo->nCutBegin - plInfo->dpDrawParam.sIdBegin; }
                if (sCutEnd < (plInfo->dpDrawParam.sIdBegin + sPointLen)) { plInfo->dpDrawParam.nPointIdLen = sCutEnd - plInfo->dpDrawParam.sIdBegin; }
            }
        }
        else // 数据个数 > 画布长度，遍历画布的像素点
        {
            sPointLen = plInfo->dpDrawParam.w;
            plInfo->dpDrawParam.nPointIdLen = sPointLen;
            if (TRUE == plInfo->bCut)
            {
                if (plInfo->nCutBegin > plInfo->dpDrawParam.sIdBegin)
                {
                    HANDOUBLE nForiBeginK = ((HANDOUBLE)(plInfo->nCutBegin) - (HANDOUBLE)(plInfo->dpDrawParam.sIdBegin)) / (HANDOUBLE)(plInfo->dpDrawParam.sDrawDataLen);
                    plInfo->dpDrawParam.nPointIdBegin = (HANSIZE)(plInfo->dpDrawParam.w * nForiBeginK);
                }
                if (sCutEnd < (plInfo->dpDrawParam.sIdBegin + plInfo->dpDrawParam.sDrawDataLen))
                {
                    HANDOUBLE nForLenK = ((HANDOUBLE)(sCutEnd) - (HANDOUBLE)(plInfo->dpDrawParam.nPointIdBegin)) / (HANDOUBLE)(plInfo->dpDrawParam.sDrawDataLen);
                    plInfo->dpDrawParam.nPointIdLen = (HANSIZE)(plInfo->dpDrawParam.w * nForLenK);
                }
            }
        }
        if (sPointLen < (HANSIZE)2) { bRet = TRUE; }
    }

    if (!bRet)
    {
        HANPCPLOTDS pNode = plInfo->pVarData;
        HPEN hOldPen = SelectObject(hdc, pNode->hPen);
        HBRUSH hOldBrush = SelectObject(hdc, pNode->hBrush);

        HANPLOTDRAWPOINTPARAM dpDrawPointParam = {
            .hdc = hdc,
            .nSize = plInfo->nPointSize,
            .ptPoint = plInfo->pBufPoint + plInfo->dpDrawParam.nPointIdBegin,
            .nPointLen = plInfo->dpDrawParam.nPointIdLen - plInfo->dpDrawParam.nPointIdBegin,
        };
        while (NULL != pNode)
        {
            if (TRUE == HANPolyLineDrawVar(pNode))
            {
                HANPLOTNUM nRange;
                HANPCPLOTNUM pData;
                HANPLOTNUM nMax;

                SelectObject(hdc, pNode->hPen);
                SelectObject(hdc, pNode->hBrush);
                pData = &(pNode->pData)[plInfo->dpDrawParam.sIdBegin];
                if (HPL_VAR_POS_LEFT == (EHPLVARPOS)(pNode->ePos)) { nMax = plInfo->dpDrawParam.nLeftMax; nRange = plInfo->dpDrawParam.nLeftRange; }
                else { nMax = plInfo->dpDrawParam.nRightMax; nRange = plInfo->dpDrawParam.nRightRange; }
                LONG nPointXBegin = (plInfo->rcRectangle).left + 1;
                for (HANSIZE i = plInfo->dpDrawParam.nPointIdBegin; i < plInfo->dpDrawParam.nPointIdLen; i++)
                {
                    if (plInfo->dpDrawParam.sDrawDataLen < (HANSIZE)(plInfo->dpDrawParam.w)) // 数据个数 < 画布长度，遍历数据
                    {
                        plInfo->pBufId[i] = plInfo->dpDrawParam.sIdBegin + i;
                    }
                    else // 数据个数 > 画布长度，需要特殊计算跳过部分 id
                    {
                        plInfo->pBufId[i] = (plInfo->dpDrawParam.sDrawDataLen - (HANSIZE)1) * i / (sPointLen - (HANSIZE)1);
                    }
                    plInfo->pBufPoint[i].x = nPointXBegin + (LONG)(plInfo->dpDrawParam.w * (LONG)i / (sPointLen - (HANSIZE)1));
                    if ((HANPLOTNUM)0 != nRange)
                    {
                        plInfo->pBufPoint[i].y = (plInfo->rcRectangle).top + (LONG)round((nMax - pData[i]) * (plInfo->dpDrawParam.h) / nRange);
                    }
                    else
                    {
                        plInfo->pBufPoint[i].y = (plInfo->rcRectangle).top + (plInfo->dpDrawParam.h / (LONG)2);
                    }
                }
                if (TRUE == plInfo->dpDrawParam.bLine)
                {
                    Polyline(
                        hdc,
                        plInfo->pBufPoint + plInfo->dpDrawParam.nPointIdBegin,
                        (HANINT)(plInfo->dpDrawParam.nPointIdLen - plInfo->dpDrawParam.nPointIdBegin)
                    );
                }
                if (TRUE == plInfo->dpDrawParam.bPoint) { pNode->DrawPoint(&dpDrawPointParam); }
            }
            pNode = pNode->next;
        }
        SelectObject(hdc, hOldPen);
        SelectObject(hdc, hOldBrush);
    }
}
static void HANPolyLineLineOutHighAccurate(HDC hdc, HANPPOLYLINE plInfo)
{
    BOOL bRet = FALSE;
    HANPCPLOTDS pNode = plInfo->pVarData;

    if (plInfo->dpDrawParam.sDrawDataLen < (HANSIZE)2) { bRet = TRUE; }

    if (!bRet)
    {
        HPEN hOldPen = SelectObject(hdc, pNode->hPen);
        HBRUSH hOldBrush = SelectObject(hdc, pNode->hBrush);
        
        plInfo->dpDrawParam.nPointIdBegin = 0;
        plInfo->dpDrawParam.nPointIdLen = plInfo->dpDrawParam.sDrawDataLen;
        HANSIZE sCutEnd = plInfo->nCutEnd;
        
        if (TRUE == plInfo->bCut)
        {
            if (sCutEnd < plInfo->dpDrawParam.sIdBegin) { bRet = TRUE; }    // 绘制区不在裁剪区内，直接返回，防止之后无符号减法负数溢出
            if (sCutEnd > plInfo->nDataLen) { sCutEnd = plInfo->nDataLen; }
            if (plInfo->nCutBegin > plInfo->dpDrawParam.sIdBegin) { plInfo->dpDrawParam.nPointIdBegin = plInfo->nCutBegin - plInfo->dpDrawParam.sIdBegin; }
            if (sCutEnd < (plInfo->dpDrawParam.sIdBegin + plInfo->dpDrawParam.sDrawDataLen)) { plInfo->dpDrawParam.nPointIdLen = sCutEnd - plInfo->dpDrawParam.sIdBegin; }
        }
    
        if (!bRet)
        {
            HANPLOTDRAWPOINTPARAM dpDrawPointParam = {
                .hdc = hdc,
                .nSize = plInfo->nPointSize,
                .ptPoint = plInfo->pBufPoint + plInfo->dpDrawParam.nPointIdBegin,
                .nPointLen = plInfo->dpDrawParam.nPointIdLen - plInfo->dpDrawParam.nPointIdBegin,
            };
            while (NULL != pNode)
            {
                if (TRUE == HANPolyLineDrawVar(pNode))
                {
                    HANPLOTNUM nRange;
                    HANPLOTNUM nMax;
                    HANPCPLOTNUM pData;

                    SelectObject(hdc, pNode->hPen);
                    SelectObject(hdc, pNode->hBrush);
                    pData = &(pNode->pData)[plInfo->dpDrawParam.sIdBegin];
                    if (HPL_VAR_POS_LEFT == (EHPLVARPOS)(pNode->ePos)) { nMax = plInfo->dpDrawParam.nLeftMax; nRange = plInfo->dpDrawParam.nLeftRange; }
                    else { nMax = plInfo->dpDrawParam.nRightMax; nRange = plInfo->dpDrawParam.nRightRange; }
                    LONG nPointXBegin = (plInfo->rcRectangle).left + 1;
                    for (HANSIZE i = plInfo->dpDrawParam.nPointIdBegin; i < plInfo->dpDrawParam.nPointIdLen; i++)
                    {
                        plInfo->pBufId[i] = plInfo->dpDrawParam.sIdBegin + i;
                        plInfo->pBufPoint[i].x = nPointXBegin + (LONG)(plInfo->dpDrawParam.w * i / (plInfo->dpDrawParam.sDrawDataLen - (HANSIZE)1));
                        if ((HANPLOTNUM)0 != nRange)
                        {
                            plInfo->pBufPoint[i].y = (plInfo->rcRectangle).top + (LONG)round((nMax - pData[i]) * (plInfo->dpDrawParam.h) / nRange);
                        }
                        else
                        {
                            plInfo->pBufPoint[i].y = (plInfo->rcRectangle).top + (plInfo->dpDrawParam.h / (LONG)2);
                        }
                    }
                    if (TRUE == plInfo->dpDrawParam.bLine)
                    {
                        Polyline(
                            hdc,
                            plInfo->pBufPoint + plInfo->dpDrawParam.nPointIdBegin,
                            (HANINT)(plInfo->dpDrawParam.nPointIdLen - plInfo->dpDrawParam.nPointIdBegin)
                        );
                    }
                    if (TRUE == plInfo->dpDrawParam.bPoint) { pNode->DrawPoint(&dpDrawPointParam); }
                }
                pNode = pNode->next;
            }
            SelectObject(hdc, hOldPen);
            SelectObject(hdc, hOldBrush);
        }
    }
}
static void HANPolyLineDrawTip(HDC hdc, HWND hPolyLine, HANPCPOLYLINE plInfo)
{
    if (0 != (GetWindowLong(hPolyLine, GWL_STYLE) & HPLS_ENABLETIP))
    {
        // 检测数据，生成文本
        POINT ptMousePos;
        GetCursorPos(&ptMousePos);
        ScreenToClient(hPolyLine, &ptMousePos);
        if (TRUE == PosInRect(&ptMousePos, &(plInfo->rcRectangle)))
        {
            HANSIZE nLineCount = 0; // 准备绘制的文本行数
            HANPPLOTDS pNode = plInfo->pVarData;
            HANDOUBLE nPosK = (HANDOUBLE)(ptMousePos.x - plInfo->rcRectangle.left) / GetRectW(&(plInfo->rcRectangle)); // 鼠标位置在绘图区的占比
            HANDOUBLE nIdDisK = (HANDOUBLE)HPL_DEF_TIP_DIS / GetRectW(&(plInfo->rcRectangle)) * plInfo->dpDrawParam.sDrawDataLen; // 对应的 id 位置占比
            HANSIZE nId = (HANSIZE)((HANDOUBLE)(plInfo->dpDrawParam.sDrawDataLen) * nPosK); // 目标 id
            if (nId > plInfo->dpDrawParam.nPointIdLen) { nId = plInfo->dpDrawParam.nPointIdLen; }
            if (nId < plInfo->dpDrawParam.nPointIdBegin) { nId = plInfo->dpDrawParam.nPointIdBegin; }
            HANSIZE nIdBegin = nId - (HANSIZE)round(nIdDisK);
            HANSIZE nIdEnd = nId + (HANSIZE)round(nIdDisK);
            if (nIdEnd == nIdBegin) { nIdEnd++; }
            if (nIdBegin < plInfo->dpDrawParam.nPointIdBegin) { nIdBegin = plInfo->dpDrawParam.nPointIdBegin; }
            if (nIdEnd > plInfo->dpDrawParam.nPointIdLen) { nIdEnd = plInfo->dpDrawParam.nPointIdLen; }
            HANDOUBLE nDisMin;
            HANDOUBLE nDisTemp;
            while (NULL != pNode)
            {
                pNode->pText[0] = TEXT('\0');
                if (TRUE == HANPolyLineDrawVar(pNode))
                {
                    HANSIZE nLineCountAdd = 0;
                    nDisMin = (HANDOUBLE)HPL_DEF_TIP_DIS + (HANDOUBLE)1;
                    for (HANSIZE i = nIdBegin; i < nIdEnd; i++)
                    {
                        HANSIZE nDataId = plInfo->pBufId[i];
                        HANPLOTNUM nMax = (HANPLOTNUM)0;
                        HANPLOTNUM nRange = (HANPLOTNUM)0;
                        LONG lY;
                        if (HPL_VAR_POS_LEFT == (EHPLVARPOS)pNode->ePos) { nMax = plInfo->dpDrawParam.nLeftMax; nRange = plInfo->dpDrawParam.nLeftRange; }
                        if (HPL_VAR_POS_RIGHT == (EHPLVARPOS)pNode->ePos) { nMax = plInfo->dpDrawParam.nRightMax; nRange = plInfo->dpDrawParam.nRightRange; }
                        if ((HANPLOTNUM)0 != nRange) { lY = plInfo->rcRectangle.top + (LONG)round((nMax - pNode->pData[nDataId]) * plInfo->dpDrawParam.h / nRange); }
                        else { lY = (plInfo->rcRectangle).top + (plInfo->dpDrawParam.h / (LONG)2); }
                        nDisTemp = sqrt(pow(plInfo->pBufPoint[i].x - ptMousePos.x, 2) + pow(lY - ptMousePos.y, 2));
                        if (nDisTemp < nDisMin)
                        {
                            HAN_snprintf(
                                pNode->pText, HAN_PLOT_STR_LEN_MAX,
                                TEXT("%s=%g"), pNode->pName, pNode->pData[nDataId]
                            );
                            nLineCountAdd = 1;
                        }
                    }
                    nLineCount += nLineCountAdd;
                }
                pNode = pNode->next;
            }

            if (nLineCount > (HANSIZE)0)
            {
                // 保存 hdc 现场
                COLORREF crOldColor = SetTextColor(hdc, 0);
                HPEN hOldPen = SelectObject(hdc, plInfo->hTipBorder);
                HBRUSH hOldBrush = SelectObject(hdc, plInfo->hTipBkBrush);
                HFONT hOldFont = SelectObject(hdc, plInfo->hTipFont);
                HANINT nOldBkMode = SetBkMode(hdc, TRANSPARENT);

                // 绘制背景
                LONG lTriangleW = 8;
                LONG lTriangleH = 10;
                LONG lRdRectR = 5;
                LONG lTextH = (LONG)(plInfo->nTipTextH) * (LONG)nLineCount;
                LONG lTipRectTop = ptMousePos.y - (lTriangleH / (LONG)2) - lRdRectR;
                RECT rcTipRect = {
                    .left = ptMousePos.x + lTriangleW,
                    .top = lTipRectTop,
                    .right = ptMousePos.x + lTriangleW + (LONG)(plInfo->nTipW),
                    .bottom = lTipRectTop + lTextH,
                };
                if (GetRectH(&rcTipRect) < (lTriangleH + (lRdRectR * 2))) { rcTipRect.bottom = rcTipRect.top + lTriangleH + (lRdRectR * 2); }
                RoundRect(hdc, rcTipRect.left, rcTipRect.top, rcTipRect.right, rcTipRect.bottom, lRdRectR, lRdRectR);
                POINT ptTriangle[] = {
                    { .x = ptMousePos.x, .y = ptMousePos.y, },
                    { .x = ptMousePos.x + lTriangleW, .y = ptMousePos.y - (lTriangleH / (LONG)2), },
                    { .x = ptMousePos.x + lTriangleW, .y = ptMousePos.y + (lTriangleH / (LONG)2), },
                };
                Polygon(hdc, ptTriangle, ArrLen(ptTriangle));
                SelectObject(hdc, plInfo->hTipBkPen);
                POINT ptBkPen[] = {
                    { .x = ptTriangle[1].x, .y = ptTriangle[1].y + 1 },
                    { .x = ptTriangle[2].x, .y = ptTriangle[2].y },
                };
                Polyline(hdc, ptBkPen, ArrLen(ptBkPen));
                // 绘制数值
                rcTipRect.left += 2;
                rcTipRect.right -= 2;
                pNode = plInfo->pVarData;
                while (NULL != pNode)
                {
                    SetTextColor(hdc, pNode->crColor);
                    if (TEXT('\0') != pNode->pText[0])
                    {
                        DrawText(hdc, pNode->pText, -1, &rcTipRect, DT_SINGLELINE);
                        rcTipRect.top += plInfo->nTipTextH;
                    }
                    pNode = pNode->next;
                }
                
                // 恢复 hdc 现场
                SetTextColor(hdc, crOldColor);
                SelectObject(hdc, hOldPen);
                SelectObject(hdc, hOldBrush);
                SelectObject(hdc, hOldFont);
                SetBkMode(hdc, nOldBkMode);
            }
        }
    }
}
static void HANPolyLineMoveButtonCallback(HANPPOLYLINE plInfo)
{
    plInfo->eCursorMode = HPLOT_CM_MOVE;
}
static void HANPolyLineEnlargeButtonCallback(HANPPOLYLINE plInfo)
{
    plInfo->eCursorMode = HPLOT_CM_ENLARGE;
}
static BOOL HANPolyLineReduceButtonCallback(HANPPOLYLINE plInfo)
{
    BOOL bRedraw = FALSE;
    HANLSIZE nDrawW = (HANLSIZE)GetRectW(&(plInfo->rcRectangle));
    HANLSIZE nOldCenterX = plInfo->lRectangleX + (nDrawW / (LONG)2);
    HANLSIZE nOldPaperW = plInfo->lVRectangleW;
    HANLSIZE nOldDataLen = plInfo->nDataLen * nDrawW / nOldPaperW;
    HANLSIZE nNewDataLen = nOldDataLen + (HANSIZE)(plInfo->nDataLen * HPL_DEF_REDUCE_SIZE);
    HANLSIZE nNewPaperW = nDrawW * plInfo->nDataLen / nNewDataLen;
    HANLSIZE nNewCenterX = nOldCenterX * nNewPaperW / nOldPaperW;
    HANLSIZE nNewX;

    if (nNewPaperW < nDrawW) { nNewPaperW = nDrawW; }
    if (nNewPaperW < (nNewCenterX + (nDrawW / 2))) { nNewX = nNewPaperW - nDrawW; }
    else if (nNewCenterX < (nDrawW / 2)) { nNewX = 0; }
    else { nNewX = nNewCenterX - (nDrawW / 2); }
    
    if ((nNewPaperW != plInfo->lVRectangleW) || (nNewX != plInfo->lRectangleX)) { bRedraw = TRUE; }
    plInfo->lVRectangleW = nNewPaperW;
    plInfo->lRectangleX = nNewX;

    return bRedraw;
}
static BOOL HANPolyLineRefreshButtonCallback(HANPPOLYLINE plInfo, BOOL bResetCursor)
{
    BOOL bRedraw = FALSE;

    if (TRUE == bResetCursor) { plInfo->eCursorMode = HPLOT_CM_NORMAL; }
    if (plInfo->lVRectangleW != (HANLSIZE)GetRectW(&(plInfo->rcRectangle))) { bRedraw = TRUE; }
    plInfo->lVRectangleW = GetRectW(&(plInfo->rcRectangle));
    plInfo->lRectangleX = 0;

    return bRedraw;
}
static void HANPolyLineSizeCallback(HWND hPolyLine, HANPPOLYLINE plInfo)
{
    RECT rcClient;
    GetClientRect(hPolyLine, &rcClient);
    LONG nOldWinW = GetRectW(&(plInfo->rcRectangle));
    plInfo->rcRectangle.right = rcClient.right - plInfo->rcAutoRectDis.right;
    plInfo->rcRectangle.bottom = rcClient.bottom - plInfo->rcAutoRectDis.bottom;
    HANDOUBLE kW = (HANDOUBLE)GetRectW(&(plInfo->rcRectangle)) / nOldWinW;
    plInfo->lRectangleX = (HANLSIZE)((HANDOUBLE)(plInfo->lRectangleX) * kW);
    plInfo->lVRectangleW = (HANLSIZE)((HANDOUBLE)(plInfo->lVRectangleW) * kW);
}
static BOOL HANPolyLineMouseMoveCallback(HWND hPolyLine, HANPPOLYLINE plInfo, WPARAM wParam, LPARAM lParam)
{
    BOOL bRedraw = FALSE;
    POINT ptMousePos = {
        .x = LOWORD(lParam),
        .y = HIWORD(lParam),
    };
    if (!(wParam & MK_LBUTTON)) // 按下左键后要锁定指针（跳过该 if ）
    {
        if (TRUE == PosInRect(&ptMousePos, &(plInfo->rcRectangle)))
        {
            switch (plInfo->eCursorMode) {
                case HPLOT_CM_NORMAL: {
                    SetCursor(LoadCursor(NULL, IDC_ARROW));
                } break;
                case HPLOT_CM_MOVE: {
                    SetCursor(LoadCursor(NULL, IDC_SIZEALL));
                } break;
                case HPLOT_CM_ENLARGE: {
                    SetCursor(LoadCursor(NULL, IDC_CROSS));
                } break;

                default: {
                    /* idle */
                } break;
            }
        }
        else
        {
            SetCursor(LoadCursor(NULL, IDC_ARROW));
        }
    }
    else
    {
        switch (plInfo->eCursorMode) {
            case HPLOT_CM_MOVE: {
                LONG dX = LOWORD(lParam) - plInfo->nMoveBeginX;   // 鼠标 X坐标
                HANLSIZE nNewX = plInfo->lRectangleX - dX;
                HANLSIZE nMaxX = plInfo->lVRectangleW - GetRectW(&(plInfo->rcRectangle));
                if ((dX > 0) && (plInfo->lRectangleX < (HANLSIZE)dX)) { nNewX = 0; }
                if (nNewX > nMaxX) { nNewX = nMaxX; }
                if (plInfo->lRectangleX != nNewX)
                {
                    plInfo->lRectangleX = nNewX;
                    plInfo->nMoveBeginX = LOWORD(lParam);
                    bRedraw = TRUE;
                }
            } break;
            case HPLOT_CM_ENLARGE: {
                plInfo->nEnlargeCurX = LOWORD(lParam);
                bRedraw = TRUE;
            } break;

            default: {
                /* idle */
            } break;
        }
    }
    if (0 != (GetWindowLong(hPolyLine, GWL_STYLE) & HPLS_ENABLETIP)) { bRedraw = TRUE; }
    
    return bRedraw;
}
static void HANPolyLineLButtonDownCallback(HANPPOLYLINE plInfo, LPARAM lParam)
{
    POINT ptMousePos = {
        .x = LOWORD(lParam),
        .y = HIWORD(lParam),
    };
    if (TRUE == PosInRect(&ptMousePos, &(plInfo->rcRectangle)))
    {
        switch (plInfo->eCursorMode) {
            case HPLOT_CM_MOVE: {
                plInfo->bMove = TRUE;
                plInfo->nMoveBeginX = LOWORD(lParam);
            } break;
            case HPLOT_CM_ENLARGE: {
                plInfo->bEnlage = TRUE;
                plInfo->nEnlargeBeginX = LOWORD(lParam);
                plInfo->nEnlargeCurX = LOWORD(lParam);
            } break;

            default: {
                /* idle */
            } break;
        }
    }

    plInfo->eLastMouseAct = HPL_MA_LEFTDOWN;
    plInfo->ptMouseActPos = ptMousePos;
}
static BOOL HANPolyLineLButtonUpCallback(HWND hPolyLine, HANPPOLYLINE plInfo, LPARAM lParam)
{
    BOOL bRedraw = FALSE;
    POINT ptMousePos = {
        .x = LOWORD(lParam),
        .y = HIWORD(lParam),
    };
    switch (plInfo->eCursorMode) {
        case HPLOT_CM_MOVE: {
            plInfo->bMove = FALSE;
        } break;
        case HPLOT_CM_ENLARGE: {
            if (HPL_MA_LEFTDOWN == plInfo->eLastMouseAct)
            {
                LONG MouseX = LOWORD(lParam);   // 鼠标 X坐标
                LONG rcRectangleW = GetRectW(&(plInfo->rcRectangle));    // 绘图区域宽
                LONG nLeftX;
                LONG nRightX;
                if (plInfo->nEnlargeBeginX < MouseX) { nLeftX = plInfo->nEnlargeBeginX; nRightX = MouseX; }
                else { nLeftX = MouseX; nRightX = plInfo->nEnlargeBeginX; }
                if (nLeftX < plInfo->rcRectangle.left) { nLeftX = plInfo->rcRectangle.left; }
                if (nRightX > plInfo->rcRectangle.right) { nRightX = plInfo->rcRectangle.right; }
                LONG ChosenW = nRightX - nLeftX;  // 鼠标拖动前后位置的宽
                if (ChosenW > 0)
                {
                    HANLSIZE sNewW = (LONG)(plInfo->lVRectangleW) * rcRectangleW / ChosenW;
                    HANDOUBLE dNewK = (HANDOUBLE)rcRectangleW / (HANDOUBLE)sNewW;
                    HANDOUBLE dDataK = 1.0 / (plInfo->nDataLen - (HANSIZE)1);
                    if (dNewK > dDataK)
                    {
                        plInfo->lRectangleX = (plInfo->lRectangleX + nLeftX - plInfo->rcRectangle.left) * rcRectangleW / ChosenW;
                        plInfo->lVRectangleW = sNewW;
                    }
                    bRedraw = TRUE;
                }
                plInfo->bEnlage = FALSE;
            }
        } break;

        default: {
            /* idle */
        } break;
    }
    
    if (HPL_MA_LEFTDOWN == plInfo->eLastMouseAct)
    {
        HANPPLOTDS pCurrentVar = HANPolyLineMouseInVarText(hPolyLine, plInfo, &ptMousePos);
        HANPPLOTDS pLBDownVAR = HANPolyLineMouseInVarText(hPolyLine, plInfo, &(plInfo->ptMouseActPos));
        if ((NULL != pCurrentVar) && (pCurrentVar == pLBDownVAR))
        {
            pCurrentVar->bDraw = (!(pCurrentVar->bDraw));
        }
    }

    plInfo->eLastMouseAct = HPL_MA_LEFTUP;
    plInfo->ptMouseActPos = ptMousePos;

    return bRedraw;
}
static HANPPLOTDS HANPolyLineMouseInVarText(HWND hPolyLine, HANPCPOLYLINE plInfo, const POINT* pptMousePos)
{
    HANPPLOTDS ret = plInfo->pVarData;
    HANINT nTextY = HPL_VAR_TEXT_Y_BEGIN;
    HDC hdc = GetDC(hPolyLine);
    HFONT hOldFont = SelectObject(hdc, plInfo->hTextFont);

    while (NULL != ret)
    {
        if ((HPL_VAR_POS_LEFT == (EHPLVARPOS)(ret->ePos)) || (HPL_VAR_POS_RIGHT == (EHPLVARPOS)(ret->ePos)))
        {
            SIZE sTextSize;
            RECT rcWin;
            GetWindowRect(hPolyLine, &rcWin);
            LONG nWinW = GetRectW(&rcWin);
            GetTextExtentExPoint(hdc, ret->pName, (HANINT)(HAN_strlen(ret->pName)), 0, NULL, NULL, &sTextSize);
            RECT rcTextRect = {
                .left = nWinW - 10 - sTextSize.cx,
                .top = nTextY,
                .right = nWinW - 10,
                .bottom = nTextY + plInfo->nTextH,
            };
            if (TRUE == PosInRect(pptMousePos, &rcTextRect))
            {
                break;
            }
            nTextY += plInfo->nTextH;
        }
        ret = ret->next;
    }
    SelectObject(hdc, hOldFont);

    return ret;
}
static BOOL HANPolyLineDrawVar(HANPCPLOTDS pVar)
{
    BOOL bRet;
    if (pVar->bDraw && ((HPL_VAR_POS_LEFT == (EHPLVARPOS)(pVar->ePos)) || (HPL_VAR_POS_RIGHT == (EHPLVARPOS)(pVar->ePos)))) { bRet = TRUE; }
    else { bRet = FALSE; }
    return bRet;
}
// WndProc & 实现 定义
static LRESULT CALLBACK HANPolyLineWndProc(HWND hPolyLine, UINT message, WPARAM wParam, LPARAM lParam) // 过程函数
{
    LRESULT lWndProcRet = 0;
    // 读取属性
    HANPPOLYLINE plInfo = (HANPPOLYLINE)GetWindowLongPtr(hPolyLine, 0);

    switch (message) {
        case WM_CREATE: { // 创建窗口
            HANDLE hHeap = GetProcessHeap();
            if (NULL == hHeap) { lWndProcRet = -1; break; }
            plInfo = (HANPPOLYLINE)HANWinHeapAlloc(hHeap, NULL, sizeof(HANPOLYLINE));
            if (NULL == plInfo) { lWndProcRet = -1; break; }
            SetWindowLongPtr(hPolyLine, 0, (LONG_PTR)plInfo);
            plInfo->hHeap = hHeap;
            // 获取窗口风格
            LONG style = ((LPCREATESTRUCTW)lParam)->style;
            // 获取窗口大小
            RECT rcWin;
            GetWindowRect(hPolyLine, &rcWin);
            LONG nWinW = GetRectW(&rcWin);
            LONG nWinH = GetRectH(&rcWin);
            // 创建子窗口
            HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
            HANINT xPos = nWinW - (HPL_CWDX_BUTTON + HPL_CWSIZE_BUTTON);
            DWORD nButtonStyle = HPLOT_CWS_BUTTON;
            if (0 != (style & HPLS_ENABLEMOUSE)) { nButtonStyle |= WS_VISIBLE; }
            plInfo->hbRefresh = CreateWindow(TEXT("button"), TEXT("R"), nButtonStyle, xPos, HPL_CWY_BUTTON, HPL_CWSIZE_BUTTON, HPL_CWSIZE_BUTTON, hPolyLine, (HMENU)HPL_WID_REFRESH, hInst, NULL); xPos -= (HANINT)HPL_CWDX_BUTTON + (HANINT)HPL_CWSIZE_BUTTON;
            plInfo->hbReduce = CreateWindow(TEXT("button"), TEXT("-"), nButtonStyle, xPos, HPL_CWY_BUTTON, HPL_CWSIZE_BUTTON, HPL_CWSIZE_BUTTON, hPolyLine, (HMENU)HPL_WID_REDUCE, hInst, NULL); xPos -= (HANINT)HPL_CWDX_BUTTON + (HANINT)HPL_CWSIZE_BUTTON;
            plInfo->hbEnlarge = CreateWindow(TEXT("button"), TEXT("+"), nButtonStyle, xPos, HPL_CWY_BUTTON, HPL_CWSIZE_BUTTON, HPL_CWSIZE_BUTTON, hPolyLine, (HMENU)HPL_WID_ENLARGE, hInst, NULL); xPos -= (HANINT)HPL_CWDX_BUTTON + (HANINT)HPL_CWSIZE_BUTTON;
            plInfo->hbMove = CreateWindow(TEXT("button"), TEXT("M"), nButtonStyle, xPos, HPL_CWY_BUTTON, HPL_CWSIZE_BUTTON, HPL_CWSIZE_BUTTON, hPolyLine, (HMENU)HPL_WID_MOVE, hInst, NULL); xPos -= (HANINT)HPL_CWDX_BUTTON + (HANINT)HPL_CWSIZE_BUTTON;
            plInfo->hButtonFont = CreateFont(18, 9, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, HAN_PLOT_DEF_TEXT_FACE);
            SendMessage(plInfo->hbMove, WM_SETFONT, (WPARAM)plInfo->hButtonFont, (LPARAM)TRUE);
            SendMessage(plInfo->hbEnlarge, WM_SETFONT, (WPARAM)plInfo->hButtonFont, (LPARAM)TRUE);
            SendMessage(plInfo->hbReduce, WM_SETFONT, (WPARAM)plInfo->hButtonFont, (LPARAM)TRUE);
            SendMessage(plInfo->hbRefresh, WM_SETFONT, (WPARAM)plInfo->hButtonFont, (LPARAM)TRUE);
            (void)HANPolyLineSetBackgroundColorCallback(plInfo, RGB(0xFF, 0xFF, 0xFF));
            (void)HANPolyLineSetBackgroundBorderCallback(plInfo, RGB(0x00, 0x00, 0x00));
            (void)HANPolyLineSetRectangleColorCallback(plInfo, RGB(0xFF, 0xFF, 0xFF));
            (void)HANPolyLineSetRectangleBorderCallback(plInfo, RGB(0x00, 0x00, 0x00));
            (void)HANPolyLineSetTitleFontCallback(plInfo, HPL_DEF_TITLE_W, HPL_DEF_TITLE_H, HAN_PLOT_DEF_TEXT_FACE);
            (void)HANPolyLineSetTextFontCallback(plInfo, HPL_DEF_TEXT_W, HPL_DEF_TEXT_H, HAN_PLOT_DEF_TEXT_FACE);
            RECT rcRectangle = {
                .left = HPL_DEF_RECT_LEFT,
                .top = HPL_DEF_RECT_TOP,
                .right = HPL_DEF_RECT_RIGHT,
                .bottom = HPL_DEF_RECT_BOTTOM,
            };
            if (((LONG)HPL_DEF_RECT_LEFT + (LONG)HPL_DEF_RECT_RIGHT) > nWinW) { rcRectangle.left = nWinW / (LONG)3; rcRectangle.right = nWinW / (LONG)3; }
            if (((LONG)HPL_DEF_RECT_TOP + (LONG)HPL_DEF_RECT_BOTTOM) > nWinH) { rcRectangle.top = nWinH / (LONG)3; rcRectangle.bottom = nWinH / (LONG)3; }
            HANPolyLineSetRcRectangleCallback(hPolyLine, plInfo, &rcRectangle, TRUE);
            (void)HANPolyLineSetRcInvHCallback(plInfo, (LONG)((nWinH - rcRectangle.bottom - rcRectangle.top) * HPL_DEF_RC_INV_HK));
            HANPolyLineSetHValueCountCallback(plInfo, (HANSIZE)HPL_DEF_HVALUE_COUNT);
            HANPolyLineSetVValueCountCallback(plInfo, (HANSIZE)HPL_DEF_VVALUE_COUNT);
            HANPolyLineSetDataModeCallback(plInfo, HPL_DM_HIGH_ACCURATE);
            (void)HANPolyLineSetLineSizeCallback(plInfo, 1);
            HANPolyLineSetPointSizeCallback(plInfo, 3);
            HANPolyLineSetTipWCallback(plInfo, 100);
            (void)HANPolyLineSetTipFontCallback(plInfo, 8, 16, HAN_PLOT_DEF_TEXT_FACE);
            (void)HANPolyLineSetTipBkColorCallback(plInfo, RGB(0xFF, 0xFF, 0xFF));
            (void)HANPolyLineSetTipBorderCallback(plInfo, RGB(0, 0, 0));
            SetTimer(hPolyLine, 1, HPL_DEF_TIMER_T, NULL);
        } break;
        case WM_SIZE: {
            if (TRUE == plInfo->bAutoRect) { HANPolyLineSizeCallback(hPolyLine, plInfo); }
            else { lWndProcRet = DefWindowProc(hPolyLine, message, wParam, lParam); }
        } break;
        case WM_SETCURSOR: { // 拦截系统光标
            LONG nStyle = GetWindowStyle(hPolyLine);
            if (!(nStyle & HPLS_ENABLEMOUSE)) { lWndProcRet = DefWindowProc(hPolyLine, message, wParam, lParam); }
        } break;
        case WM_TIMER: {
            HANPolyLineTimerCallback(hPolyLine, plInfo);
        } break;
        case WM_COMMAND: { // 控件指令
            HANINT wmId = LOWORD(wParam);
            switch (wmId) {
                case HPL_WID_MOVE: {
                    HANPolyLineMoveButtonCallback(plInfo);
                } break;
                case HPL_WID_ENLARGE: {
                    HANPolyLineEnlargeButtonCallback(plInfo);
                } break;
                case HPL_WID_REDUCE: {
                    plInfo->bRedraw = HANPolyLineReduceButtonCallback(plInfo);
                } break;
                case HPL_WID_REFRESH: {
                    plInfo->bRedraw = HANPolyLineRefreshButtonCallback(plInfo, TRUE);
                } break;

                default: {
                    /* idle */
                } break;
            }
        } break;
        case WM_LBUTTONDOWN: { // 鼠标左键按下
            LONG nStyle = GetWindowStyle(hPolyLine);
            if (0 != (nStyle & HPLS_ENABLEMOUSE)) { HANPolyLineLButtonDownCallback(plInfo, lParam); }
        } break;
        case WM_LBUTTONUP: { // 鼠标左键弹起
            LONG nStyle = GetWindowStyle(hPolyLine);
            if (0 != (nStyle & HPLS_ENABLEMOUSE)) { plInfo->bRedraw = HANPolyLineLButtonUpCallback(hPolyLine, plInfo, lParam); }
        } break;
        case WM_LBUTTONDBLCLK: { // 鼠标左键双击
            LONG nStyle = GetWindowStyle(hPolyLine);
            if (0 != (nStyle & HPLS_ENABLEMOUSE)) { plInfo->bRedraw = HANPolyLineRefreshButtonCallback(plInfo, FALSE); }
        } break;
        case WM_MOUSEMOVE: { // 鼠标移动
            LONG nStyle = GetWindowStyle(hPolyLine);
            if (0 != (nStyle & HPLS_ENABLEMOUSE)) { plInfo->bRedraw = HANPolyLineMouseMoveCallback(hPolyLine, plInfo, wParam, lParam); }
        } break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            // 开始绘图
            HDC hdc = BeginPaint(hPolyLine, &ps);
            RECT rcWin;
            GetClientRect(hPolyLine, &rcWin);
            // 双缓冲绘图，分配资源
            HDC hMemDC = CreateCompatibleDC(hdc);   // 获取内存设备上下文，图形统一在该 HDC 下绘制
            HBITMAP hBitMap = CreateCompatibleBitmap(hdc, GetRectW(&rcWin), GetRectH(&rcWin));  // 创建位图
            SelectObject(hMemDC, hBitMap);

            // 绘制内容
            HANPolyLinePaintCallback(hMemDC, hPolyLine, plInfo, &rcWin);
            
            // 将位图中绘制好的图像贴到屏幕上
            BitBlt(hdc, 0, 0, GetRectW(&rcWin), GetRectH(&rcWin), hMemDC, 0, 0, SRCCOPY);
            
            // 释放内存和位图
            DeleteDC(hMemDC);
            DeleteObject(hBitMap);
            // 结束绘图
            EndPaint(hPolyLine, &ps);
        } break;
        case WM_DESTROY: {
            if (NULL != plInfo)
            {
                KillTimer(hPolyLine, 1);
                HANPPLOTDS pNode = plInfo->pVarData;
                HANPPLOTDS pNext;
                HANWinHeapFree(plInfo->hHeap, 0, plInfo->pBufPoint);
                HANWinHeapFree(plInfo->hHeap, 0, plInfo->pBufId);
                while (NULL != pNode)
                {
                    pNext = pNode->next;
                    HANWinHeapFree(plInfo->hHeap, 0, pNode->pData);
                    if (NULL != pNode->hPen) { DeleteObject(pNode->hPen); }
                    HANWinHeapFree(plInfo->hHeap, 0, pNode);
                    pNode = pNext;
                }
                HANWinHeapFree(plInfo->hHeap, 0, plInfo);
            }
        } break;

        case HPLM_SETDATALEN: {
            lWndProcRet = HANPolyLineSetDataLenCallback(plInfo, (HANSIZE)wParam);
        } break;
        case HPLM_ADDVAR: {
            lWndProcRet = HANPolyLineAddVarCallback(plInfo, (HANPCSTR)lParam, (EHPLVARPOS)wParam);
        } break;
        case HPLM_SETSTARTDATABYNAME: {
            lWndProcRet = HANPolyLineUpdateDataByNameCallback(plInfo, (HANPCSTR)wParam, (HANPCPLOTDATA)lParam, HAN_PLOT_UPDATE_DATA_SET_START);
        } break;
        case HPLM_PUSHSTARTDATABYNAME: {
            lWndProcRet = HANPolyLineUpdateDataByNameCallback(plInfo, (HANPCSTR)wParam, (HANPCPLOTDATA)lParam, HAN_PLOT_UPDATE_DATA_PUSH_START);
        } break;
        case HPLM_SETENDDATABYNAME: {
            lWndProcRet = HANPolyLineUpdateDataByNameCallback(plInfo, (HANPCSTR)wParam, (HANPCPLOTDATA)lParam, HAN_PLOT_UPDATE_DATA_SET_END);
        } break;
        case HPLM_PUSHENDDATABYNAME: {
            lWndProcRet = HANPolyLineUpdateDataByNameCallback(plInfo, (HANPCSTR)wParam, (HANPCPLOTDATA)lParam, HAN_PLOT_UPDATE_DATA_PUSH_END);
        } break;
        case HPLM_SETSTARTDATABYID: {
            lWndProcRet = HANPolyLineUpdateDataByIdCallback(plInfo, (HANSIZE)wParam, (HANPCPLOTDATA)lParam, HAN_PLOT_UPDATE_DATA_SET_START);
        } break;
        case HPLM_PUSHSTARTDATABYID: {
            lWndProcRet = HANPolyLineUpdateDataByIdCallback(plInfo, (HANSIZE)wParam, (HANPCPLOTDATA)lParam, HAN_PLOT_UPDATE_DATA_PUSH_START);
        } break;
        case HPLM_SETENDDATABYID: {
            lWndProcRet = HANPolyLineUpdateDataByIdCallback(plInfo, (HANSIZE)wParam, (HANPCPLOTDATA)lParam, HAN_PLOT_UPDATE_DATA_SET_END);
        } break;
        case HPLM_PUSHENDDATABYID: {
            lWndProcRet = HANPolyLineUpdateDataByIdCallback(plInfo, (HANSIZE)wParam, (HANPCPLOTDATA)lParam, HAN_PLOT_UPDATE_DATA_PUSH_END);
        } break;
        case HPLM_SETVARCOLORBYNAME: {
            lWndProcRet = HANPolyLineSetVarColorCallback(plInfo, (HANPCSTR)wParam, (HANSIZE)(-1), (COLORREF)lParam);
        } break;
        case HPLM_SETVARCOLORBYID: {
            lWndProcRet = HANPolyLineSetVarColorCallback(plInfo, (HANPCSTR)NULL, (HANSIZE)wParam, (COLORREF)lParam);
        } break;
        case HPLM_SETLEFTMAXMIN: {
            HANPolyLineSetLeftMaxMinCallback(plInfo, (HANPCPLOTMAXMIN)lParam, (BOOL)wParam);
        } break;
        case HPLM_SETRIGHTMAXMIN: {
            HANPolyLineSetRightMaxMinCallback(plInfo, (HANPCPLOTMAXMIN)lParam, (BOOL)wParam);
        } break;
        case HPLM_SETBKCOLOR: {
            lWndProcRet = HANPolyLineSetBackgroundColorCallback(plInfo, (COLORREF)wParam);
        } break;
        case HPLM_SETBKBORDER: {
            lWndProcRet = HANPolyLineSetBackgroundBorderCallback(plInfo, (COLORREF)wParam);
        } break;
        case HPLM_SETRECTCOLOR: {
            lWndProcRet = HANPolyLineSetRectangleColorCallback(plInfo, (COLORREF)wParam);
        } break;
        case HPLM_SETRECTBORDER: {
            lWndProcRet = HANPolyLineSetRectangleBorderCallback(plInfo, (COLORREF)wParam);
        } break;
        case HPLM_SETTITLEFONT: {
            HANINT* pTextSize = (HANINT*)wParam;
            lWndProcRet = HANPolyLineSetTitleFontCallback(plInfo, pTextSize[0], pTextSize[1], (HANPCSTR)lParam);
        } break;
        case HPLM_SETTEXTFONT: {
            HANINT* pTextSize = (HANINT*)wParam;
            lWndProcRet = HANPolyLineSetTextFontCallback(plInfo, pTextSize[0], pTextSize[1], (HANPCSTR)lParam);
        } break;
        case HPLM_SETRECT: {
            HANPolyLineSetRcRectangleCallback(hPolyLine, plInfo, (RECT*)lParam, (BOOL)wParam);
        } break;
        case HPLM_GETRECT: {
            lWndProcRet = HANPolyLineGetRcRectangleCallback(plInfo, (RECT*)lParam);
        } break;
        case HPLM_SETRCINVH: {
            lWndProcRet = HANPolyLineSetRcInvHCallback(plInfo, (LONG)wParam);
        } break;
        case HPLM_SETHVALCOUNT: {
            HANPolyLineSetHValueCountCallback(plInfo, (HANSIZE)wParam);
        } break;
        case HPLM_SETVVALCOUNT: {
            HANPolyLineSetVValueCountCallback(plInfo, (HANSIZE)wParam);
        } break;
        case HPLM_ENABLEMOUSE: {
            HANPolyLineEnableMouseCallback(hPolyLine, plInfo, (BOOL)wParam);
        } break;
        case HPLM_SETDATAMODE: {
            HANPolyLineSetDataModeCallback(plInfo, (EHPLDATAMODE)wParam);
        } break;
        case HPLM_SETPOINTLINESIZE: {
            switch (wParam) {
                case HPL_PLS_LINE: {
                    lWndProcRet = HANPolyLineSetLineSizeCallback(plInfo, (HANINT)lParam);
                } break;
                case HPL_PLS_POINT: {
                    HANPolyLineSetPointSizeCallback(plInfo, (HANINT)lParam);
                } break;

                default: {
                    /* idle */
                } break;
            }
        } break;
        case HPLM_SETPOINTSTYLEBYNAME: {
            lWndProcRet = HANPolyLineSetPointStyleCallback(plInfo, (HANPCSTR)wParam, (HANSIZE)(-1), (HANHPOINT)lParam);
        } break;
        case HPLM_SETPOINTSTYLEBYID: {
            lWndProcRet = HANPolyLineSetPointStyleCallback(plInfo, (HANPCSTR)NULL, (HANSIZE)wParam, (HANHPOINT)lParam);
        } break;
        case HPLM_DRAWPOINTLINE: {
            HANPolyLineDrawPointLineCallback(hPolyLine, (BOOL)wParam, (BOOL)lParam);
        } break;
        case HPLM_CUTDATA: {
            HANPolyLineCutDataCallback(plInfo, (HANPCSIZE)lParam, (BOOL)wParam);
        } break;
        case HPLM_SETTIPW: {
            HANPolyLineSetTipWCallback(plInfo, (HANSIZE)wParam);
        } break;
        case HPLM_SETTIPFONT: {
            HANINT* pTextSize = (HANINT*)wParam;
            lWndProcRet = HANPolyLineSetTipFontCallback(plInfo, pTextSize[0], pTextSize[1], (HANPCSTR)lParam);
        } break;
        case HPLM_SETTIPBKCOLOR: {
            lWndProcRet = HANPolyLineSetTipBkColorCallback(plInfo, (COLORREF)wParam);
        } break;
        case HPLM_SETTIPBORDER: {
            lWndProcRet = HANPolyLineSetTipBorderCallback(plInfo, (COLORREF)wParam);
        } break;

        default: {
            lWndProcRet =  DefWindowProc(hPolyLine, message, wParam, lParam);
        } break;
    }

    // 返回
    return lWndProcRet;
}
static void HANPolyLineTimerCallback(HWND hPolyLine, HANPPOLYLINE plInfo)
{
    if (TRUE == plInfo->bRedraw)
    {
        RedrawWindow(hPolyLine, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
        plInfo->bRedraw = FALSE;
    }
}
static HANERROR HANPolyLineSetDataLenCallback(HANPPOLYLINE plInfo, HANSIZE nDataLen)
{
    HANERROR nRet = RET_OK;
    HANPPLOTDS pNode = plInfo->pVarData;
    POINT* pTempPoint;
    HANSIZE* pTempId;
    HANPPLOTNUM pTempData;

    if (nDataLen > plInfo->nDataLen)
    {
        pTempPoint = (POINT*)HANWinHeapAlloc(plInfo->hHeap, plInfo->pBufPoint, nDataLen * sizeof(POINT));
        pTempId = (HANSIZE*)HANWinHeapAlloc(plInfo->hHeap, plInfo->pBufId, nDataLen * sizeof(HANSIZE));
        if ((NULL == pTempPoint) || (NULL == pTempId))
        {
            if (NULL != pTempPoint) { plInfo->pBufPoint = pTempPoint; }
            if (NULL != pTempId) { plInfo->pBufId = pTempId; }
            nRet = NOT_ENOUGH_MEMORY;
        }
        else
        {
            plInfo->pBufPoint = pTempPoint;
            plInfo->pBufId = pTempId;
            while (NULL != pNode)
            {
                pTempData = (HANPPLOTNUM)HANWinHeapAlloc(plInfo->hHeap, pNode->pData, nDataLen * sizeof(HANPLOTNUM));
                if (NULL == pTempData) { nRet = NOT_ENOUGH_MEMORY; break; }
                else { pNode->pData = pTempData; }
                pNode = pNode->next;
            }
        }
    }
    if (RET_OK == nRet)
    {
        plInfo->lVRectangleW = GetRectW(&(plInfo->rcRectangle));
        plInfo->lRectangleX = 0;
        plInfo->nDataLen = nDataLen;
    }

    return nRet;
}
static HANERROR HANPolyLineAddVarCallback(HANPPOLYLINE plInfo, HANPCSTR pVarName, EHPLVARPOS vpPos)
{
    HANERROR nRet = RET_OK;
    COLORREF crColor = 0;
    HANPPLOTNUM pData = NULL;
    HANPPLOTDS pNode = NULL;
    HPEN hPen = NULL;
    HBRUSH hBrush = NULL;

    if ((NULL != plInfo->pTopVar) && (HPL_VAR_POS_TOP == vpPos)) { nRet = HPL_VAR_HPOS_EXIST; }
    if ((NULL != plInfo->pBottomVar) && (HPL_VAR_POS_BOTTOM == vpPos)) { nRet = HPL_VAR_HPOS_EXIST; }

    if (RET_OK == nRet)
    {
        for (HANSIZE i = 0; i < plInfo->nVarCount; i++)
        {
            crColor = HANPlotAddColor(crColor);
        }
        crColor = HANPlotAddColor(crColor);
        if (plInfo->nDataLen > (HANSIZE)0)
        {
            pData = (HANPPLOTNUM)HANWinHeapAlloc(plInfo->hHeap, NULL, plInfo->nDataLen * sizeof(HANPLOTNUM));
            if (NULL == pData) { nRet = NOT_ENOUGH_MEMORY; }
        }
    }

    if (RET_OK == nRet)
    {
        pNode = (HANPPLOTDS)HANWinHeapAlloc(plInfo->hHeap, NULL, sizeof(HANPLOTDS));
        hPen = CreatePen(PS_SOLID, 0, crColor);
        hBrush = CreateSolidBrush(crColor);
        if (NULL == pNode) { nRet = NOT_ENOUGH_MEMORY; }
        if (NULL == hPen) { nRet = CREATE_PEN_ERR; }
        if (NULL == hBrush) { nRet = CREATE_BRUSH_ERR; }
    }

    if (RET_OK == nRet)
    {
        // 将节点加入链表
        HANPPLOTDS pLast = plInfo->pVarData;
        if (NULL == pLast) { plInfo->pVarData = pNode; }
        else { while (NULL != pLast->next) { pLast = pLast->next; } pLast->next = pNode; }
        pNode->pData = pData;
        pNode->nMax = 0;
        pNode->nMin = 0;
        HAN_snprintf(pNode->pName, HAN_PLOT_STR_LEN_MAX, TEXT("%s"), pVarName);
        pNode->crColor = crColor;
        pNode->ePos = vpPos;
        pNode->hPen = hPen;
        pNode->hBrush = hBrush;
        pNode->bDraw = TRUE;
        pNode->DrawPoint = HANPlotGetStockPoint(HAN_PLOT_DEF_DRAW_POINT);
        pNode->pChild = NULL;
        pNode->next = NULL;
        plInfo->nVarCount++;
        if (HPL_VAR_POS_TOP == vpPos) { plInfo->pTopVar = pNode; }
        else if (HPL_VAR_POS_BOTTOM == vpPos) { plInfo->pBottomVar = pNode; }
        else { }
    }
    else
    {
        if (NULL != pData) { HANWinHeapFree(plInfo->hHeap, 0, pData); }
        if (NULL != pNode) { HANWinHeapFree(plInfo->hHeap, 0, pNode); }
        if (NULL != hPen) { DeleteObject(hPen); }
        if (NULL != hBrush) { DeleteObject(hBrush); }
    }
    
    return RET_OK;
}
static HANERROR HANPolyLineUpdateDataByNameCallback(HANPPOLYLINE plInfo, HANPCSTR pVarName, HANPCPLOTDATA pData, EPLOTUPDATEDATA modeUpdate)
{
    HANERROR nRet = PLOT_VAR_NOT_EXIST;
    HANPPLOTDS pNode = plInfo->pVarData;

    if (NULL != pVarName)
    {
        while (NULL != pNode)
        {
            if (!HAN_strcmp(pNode->pName, pVarName))
            {

                HANPLOTMAXMIN mmMaxMin;
                HANPLOTDATA pdData = { .pData = pNode->pData, .nLen = plInfo->nDataLen, };

                nRet = HANPlotUpdateData(&pdData, pData, modeUpdate);
                if (nRet == RET_OK)
                {
                    HANPlotGetDataMaxMin(&pdData, &mmMaxMin);

                    pNode->nMax = mmMaxMin.max;
                    pNode->nMin = mmMaxMin.min;
                }
                break;
            }
            pNode = pNode->next;
        }
    }

    return nRet;
}
static HANERROR HANPolyLineUpdateDataByIdCallback(HANPPOLYLINE plInfo, HANSIZE nId, HANPCPLOTDATA pData, EPLOTUPDATEDATA modeUpdate)
{
    HANERROR nRet = PLOT_VAR_NOT_EXIST;
    HANPPLOTDS pNode = plInfo->pVarData;
    HANSIZE nCount = 0;
    
    while (NULL != pNode)
    {
        if (nId == nCount)
        {
            HANPLOTMAXMIN mmMaxMin;
            HANPLOTDATA pdData = { .pData = pNode->pData, .nLen = plInfo->nDataLen, };

            nRet = HANPlotUpdateData(&pdData, pData, modeUpdate);
            if (nRet == RET_OK)
            {
                HANPlotGetDataMaxMin(&pdData, &mmMaxMin);

                pNode->nMax = mmMaxMin.max;
                pNode->nMin = mmMaxMin.min;
            }
            break;
        }
        pNode = pNode->next;
        nCount++;
    }

    return nRet;
}
static HANERROR HANPolyLineSetVarColorCallback(HANPPOLYLINE plInfo, HANPCSTR pVarName, HANSIZE nId, COLORREF rgb)
{
    HANERROR nRet = PLOT_VAR_NOT_EXIST;
    HANPPLOTDS pNode = plInfo->pVarData;
    HANSIZE nCount = 0;
    while (NULL != pNode)
    {
        if (((NULL != pVarName) && (!HAN_strcmp(pNode->pName, pVarName))) || (nId == nCount))
        {
            nRet = RET_OK;
            HPEN hPen = CreatePen(PS_SOLID, plInfo->nLineWidth, rgb);
            HBRUSH hBrush = CreateSolidBrush(rgb);
            if (NULL == hPen) { nRet = CREATE_PEN_ERR; }
            if (NULL == hBrush) { nRet = CREATE_BRUSH_ERR; }
            if (RET_OK != nRet)
            {
                if (NULL != hPen) { DeleteObject(hPen); }
                if (NULL != hBrush) { DeleteObject(hBrush); }
            }
            else
            {
                pNode->crColor = rgb;
                pNode->hPen = hPen;
                pNode->hBrush = hBrush;
            }
            break;
        }
        pNode = pNode->next;
        nCount++;
    }

    return nRet;
}
static void HANPolyLineSetLeftMaxMinCallback(HANPPOLYLINE plInfo, HANPCPLOTMAXMIN lMaxMin, BOOL bUserMaxMin)
{
    if (TRUE == bUserMaxMin)
    {
        if (lMaxMin->max > lMaxMin->min)
        {
            plInfo->nLeftMax = lMaxMin->max;
            plInfo->nLeftMin = lMaxMin->min;
        }
        else
        {
            plInfo->nLeftMax = lMaxMin->min;
            plInfo->nLeftMin = lMaxMin->max;
        }
    }
    plInfo->bLeftMaxMin = bUserMaxMin;
}
static void HANPolyLineSetRightMaxMinCallback(HANPPOLYLINE plInfo, HANPCPLOTMAXMIN rMaxMin, BOOL bUserMaxMin)
{
    if (TRUE == bUserMaxMin)
    {
        if (rMaxMin->max > rMaxMin->min)
        {
            plInfo->nRightMax = rMaxMin->max;
            plInfo->nRightMin = rMaxMin->min;
        }
        else
        {
            plInfo->nRightMax = rMaxMin->min;
            plInfo->nRightMin = rMaxMin->max;
        }
    }
    plInfo->bRightMaxMin = bUserMaxMin;
}
static HANERROR HANPolyLineSetBackgroundColorCallback(HANPPOLYLINE plInfo, COLORREF rgb)
{
    HANERROR nRet = RET_OK;
    HBRUSH hBrush = CreateSolidBrush(rgb);

    if (NULL == hBrush) { nRet = CREATE_BRUSH_ERR; }
    else
    {
        if (NULL != plInfo->hBkBrush) { DeleteObject(plInfo->hBkBrush); }
        plInfo->crBkColor = rgb;
        plInfo->hBkBrush = hBrush;
    }

    return nRet;
}
static HANERROR HANPolyLineSetBackgroundBorderCallback(HANPPOLYLINE plInfo, COLORREF rgb)
{
    HANERROR nRet = RET_OK;
    HPEN hPen = CreatePen(PS_SOLID, 0, rgb);

    if (NULL == hPen) { nRet = CREATE_PEN_ERR; }
    else
    {
        if (NULL != plInfo->hBkBorder) { DeleteObject(plInfo->hBkBorder); }
        plInfo->crBkBorder = rgb;
        plInfo->hBkBorder = hPen;
    }

    return nRet;
}
static HANERROR HANPolyLineSetRectangleColorCallback(HANPPOLYLINE plInfo, COLORREF rgb)
{
    HANERROR nRet = RET_OK;
    HBRUSH hBrush = CreateSolidBrush(rgb); if (NULL == hBrush) { nRet = CREATE_BRUSH_ERR; }
    COLORREF crDark = RGB(GetRValue(rgb) * HPL_DEF_RECT_DARK, GetGValue(rgb) * HPL_DEF_RECT_DARK, GetBValue(rgb) * HPL_DEF_RECT_DARK);
    HBRUSH hDarkBrush = CreateSolidBrush(crDark); if (NULL == hDarkBrush) { nRet = CREATE_BRUSH_ERR; }
    COLORREF crMarkLine = RGB(GetRValue(rgb) * HPL_DEF_MARK_LINE_DARK, GetGValue(rgb) * HPL_DEF_MARK_LINE_DARK, GetBValue(rgb) * HPL_DEF_MARK_LINE_DARK);
    HPEN hMarkLine = CreatePen(PS_SOLID, 0, crMarkLine); if (NULL == hMarkLine) { nRet = CREATE_PEN_ERR; }
    
    if (RET_OK == nRet)
    {
        if (NULL != plInfo->hRcBrush) { DeleteObject(plInfo->hRcBrush); }
        if (NULL != plInfo->hRcDarBrush) { DeleteObject(plInfo->hRcDarBrush); }
        if (NULL != plInfo->hMarkLine) { DeleteObject(plInfo->hMarkLine); }
        plInfo->crRcColor = rgb;
        plInfo->hRcBrush = hBrush;
        plInfo->hRcDarBrush = hDarkBrush;
        plInfo->hMarkLine = hMarkLine;
    }
    else
    {
        if(NULL != hBrush) { DeleteObject(hBrush); }
        if(NULL != hDarkBrush) { DeleteObject(hDarkBrush); }
        if(NULL != hMarkLine) { DeleteObject(hMarkLine); }
    }
    
    return nRet;
}
static HANERROR HANPolyLineSetRectangleBorderCallback(HANPPOLYLINE plInfo, COLORREF rgb)
{
    HANERROR nRet = RET_OK;
    HPEN hPen = CreatePen(PS_SOLID, 0, rgb);

    if (NULL == hPen) { nRet = CREATE_PEN_ERR; }
    else
    {
        if (NULL != plInfo->hRcBorder) { DeleteObject(plInfo->hRcBorder); }
        plInfo->crRcBorder = rgb;
        plInfo->hRcBorder = hPen;
    }

    return nRet;
}
static HANERROR HANPolyLineSetTitleFontCallback(HANPPOLYLINE plInfo, HANINT w, HANINT h, HANPCSTR Face)
{
    HANERROR nRet = RET_OK;
    HFONT hFont = CreateFont(h, w, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, Face);
    
    if (NULL == hFont) { nRet = CREATE_FONT_ERR; }
    else
    {
        if (NULL != plInfo->hTitleFont) { DeleteObject(plInfo->hTitleFont); }
        plInfo->nTitleW = abs(w);
        plInfo->nTitleH = abs(h);
        plInfo->hTitleFont = hFont;
    }

    return nRet;
}
static HANERROR HANPolyLineSetTextFontCallback(HANPPOLYLINE plInfo, HANINT w, HANINT h, HANPCSTR Face)
{
    HANERROR nRet = RET_OK;
    HFONT hFont = CreateFont(h, w, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, Face);
    
    if (NULL == hFont) { nRet = CREATE_FONT_ERR; }
    else
    {
        if (NULL != plInfo->hTextFont) { DeleteObject(plInfo->hTextFont); }
        plInfo->nTextW = abs(w);
        plInfo->nTextH = abs(h);
        plInfo->hTextFont = hFont;
    }
    
    return nRet;
}
static void HANPolyLineSetRcRectangleCallback(HWND hPolyLine, HANPPOLYLINE plInfo, const RECT* rcRectangle, BOOL bAutoRect)
{
    plInfo->rcRectangle = *rcRectangle;
    plInfo->lVRectangleW = GetRectW(&(plInfo->rcRectangle));
    plInfo->lRectangleX = 0;
    if (TRUE == bAutoRect)
    {
        RECT rcWin;
        GetClientRect(hPolyLine, &rcWin);
        plInfo->rcRectangle.right = rcWin.right - rcRectangle->right;
        plInfo->rcRectangle.bottom = rcWin.bottom - rcRectangle->bottom;
        plInfo->rcAutoRectDis = *rcRectangle;
    }
    plInfo->lVRectangleW = GetRectW(&(plInfo->rcRectangle));
    plInfo->lRectangleX = 0;
    plInfo->bAutoRect = bAutoRect;
}
static BOOL HANPolyLineGetRcRectangleCallback(HANPPOLYLINE plInfo, RECT* rcRectangle)
{
    *rcRectangle = plInfo->rcRectangle;
    return plInfo->bAutoRect;
}
static HANERROR HANPolyLineSetRcInvHCallback(HANPPOLYLINE plInfo, LONG nHeight)
{
    HANERROR nRet = RET_OK;
    LONG nRcH = GetRectH(&(plInfo->rcRectangle));
    if ((nHeight * 2) >= nRcH) { nRet = HPL_RCINVH_ERR; }
    if (RET_OK == nRet) { plInfo->lRcInvH = nHeight; }
    return nRet;
}
static void HANPolyLineSetHValueCountCallback(HANPPOLYLINE plInfo, HANSIZE n)
{
    if (n < (HANSIZE)2) { plInfo->nHValueCount = (HANSIZE)2; }
    else { plInfo->nHValueCount = n; }
}
static void HANPolyLineSetVValueCountCallback(HANPPOLYLINE plInfo, HANSIZE n)
{
    if (n < (HANSIZE)2) { plInfo->nVValueCount = (HANSIZE)2; }
    else { plInfo->nVValueCount = n; }
}
static void HANPolyLineEnableMouseCallback(HWND hPolyLine, HANPPOLYLINE plInfo, BOOL bEnable)
{
    LONG nStyle = GetWindowStyle(hPolyLine);
    if (TRUE == bEnable) { nStyle |= HPLS_ENABLEMOUSE; }
    else { nStyle &= ~HPLS_ENABLEMOUSE; }
    SetWindowLong(hPolyLine, GWL_STYLE, nStyle);
    HANINT iShow;
    if (TRUE == bEnable) { iShow = SW_SHOW; }
    else { iShow = SW_HIDE; plInfo->eCursorMode = HPLOT_CM_NORMAL; }
    ShowWindow(plInfo->hbMove, iShow);
    ShowWindow(plInfo->hbEnlarge, iShow);
    ShowWindow(plInfo->hbReduce, iShow);
    ShowWindow(plInfo->hbRefresh, iShow);
}
static void HANPolyLineSetDataModeCallback(HANPPOLYLINE plInfo, EHPLDATAMODE dmDataMode)
{
    switch (dmDataMode) {
        case HPL_DM_FAST: {
            plInfo->DrawLine = HANPolyLineLineOutFast;
        } break;
        case HPL_DM_HIGH_ACCURATE: {
            plInfo->DrawLine = HANPolyLineLineOutHighAccurate;
        } break;

        default: {
            /* idle */
        } break;
    }
}
static HANERROR HANPolyLineSetLineSizeCallback(HANPPOLYLINE plInfo, HANINT nWidth)
{
    plInfo->nLineWidth = nWidth;
    return HANPolyLineUpdateResources(plInfo);
}
static void HANPolyLineSetPointSizeCallback(HANPPOLYLINE plInfo, HANINT nSize)
{
    plInfo->nPointSize = nSize;
}
static HANERROR HANPolyLineSetPointStyleCallback(HANPPOLYLINE plInfo, HANPCSTR pVarName, HANSIZE nId, HANHPOINT hPoint)
{
    HANERROR nRet = PLOT_VAR_NOT_EXIST;
    HANPPLOTDS pNode = plInfo->pVarData;
    HANSIZE nCount = 0;

    while (NULL != pNode)
    {
        if (((NULL != pVarName) && (!HAN_strcmp(pNode->pName, pVarName))) || (nId == nCount))
        {
            pNode->DrawPoint = hPoint;
            nRet = RET_OK;
            break;
        }
        pNode = pNode->next;
        nCount++;
    }
    return nRet;
}
static void HANPolyLineDrawPointLineCallback(HWND hPolyLine, BOOL bPoint, BOOL bLine)
{
    LONG lStyle = GetWindowLong(hPolyLine, GWL_STYLE);
    if (TRUE == bPoint) { lStyle |= HPLS_POINT; }
    if (TRUE == bLine) { lStyle |= HPLS_LINE; }
    SetWindowLong(hPolyLine, GWL_STYLE, lStyle);
}
static void HANPolyLineCutDataCallback(HANPPOLYLINE plInfo, HANPCSIZE pPos, BOOL bCut)
{
    if (TRUE == bCut)
    {
        if (pPos[0] < pPos[1])
        {
            plInfo->nCutBegin = pPos[0];
            plInfo->nCutEnd = pPos[1];
        }
        else
        {
            plInfo->nCutBegin = pPos[1];
            plInfo->nCutEnd = pPos[0];
        }
    }
    plInfo->bCut = bCut;
}
static void HANPolyLineSetTipWCallback(HANPPOLYLINE plInfo, HANSIZE w)
{
    plInfo->nTipW = w;
}
static HANERROR HANPolyLineSetTipFontCallback(HANPPOLYLINE plInfo, HANINT w, HANINT h, HANPCSTR Face)
{
    HANERROR nRet = RET_OK;
    HFONT hFont = CreateFont(h, w, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, Face);
    if (NULL == hFont) { nRet = CREATE_FONT_ERR; }
    else
    {
        if (NULL != plInfo->hTipFont) { DeleteObject(plInfo->hTipFont); }
        plInfo->hTipFont = hFont;
        plInfo->nTipTextH = abs(h);
    }
    
    return nRet;
}
static HANERROR HANPolyLineSetTipBkColorCallback(HANPPOLYLINE plInfo, COLORREF rgb)
{
    HANERROR nRet = RET_OK;
    HBRUSH hBrush = CreateSolidBrush(rgb); if (NULL == hBrush) { nRet = CREATE_BRUSH_ERR; }
    HPEN hPen = CreatePen(PS_SOLID, 0, rgb); if (NULL == hPen) { nRet = CREATE_PEN_ERR; }

    if (RET_OK == nRet)
    {
        if (NULL != plInfo->hTipBkBrush) { DeleteObject(plInfo->hTipBkBrush); }
        if (NULL != plInfo->hTipBkPen) { DeleteObject(plInfo->hTipBkPen); }
        plInfo->hTipBkBrush = hBrush;
        plInfo->hTipBkPen = hPen;
    }
    else
    {
        if (NULL != hBrush) { DeleteObject(hBrush); }
        if (NULL != hPen) { DeleteObject(hPen); }
    }
    
    return RET_OK;
}
static HANERROR HANPolyLineSetTipBorderCallback(HANPPOLYLINE plInfo, COLORREF rgb)
{
    HANERROR nRet = RET_OK;
    HPEN hPen = CreatePen(PS_SOLID, 0, rgb);

    if (NULL == hPen) { nRet = CREATE_PEN_ERR; }
    else
    {
        if (NULL != plInfo->hTipBorder) { DeleteObject(plInfo->hTipBorder); }
        plInfo->hTipBorder = hPen;
    }

    return nRet;
}
#endif

#if 1 /******************** HANScatter ********************/
// 子窗口 HMENU
#define HST_WID_MOVE            0
#define HST_WID_ENLARGE         1
#define HST_WID_REDUCE          2
#define HST_WID_REFRESH         3
// 子窗口 button 参数
#define HST_CWY_BUTTON          10  // Y 坐标
#define HST_CWDX_BUTTON         10  // 横向间隔
#define HST_CWSIZE_BUTTON       20  // 边长
#define HST_VAR_TEXT_Y_BEGIN    (HST_CWY_BUTTON + HST_CWSIZE_BUTTON)
typedef enum { // 散点图位置参数，该参数不对外开放，用户需通过不同指令来区分
    HST_VAR_POS_POINT,
    HST_VAR_POS_TIP,
} EHSTVARPOS, * EHSTPVARPOS;
typedef struct tagHANSCATTERDP { // 绘图参数
    BOOL            bPointExist;    // 是否有点变量
    HANPLOTNUM      nHMax;          // 水平最大值
    HANPLOTNUM      nVMax;          // 垂直最大值
    HANPLOTNUM      nHMin;          // 水平最小值
    HANPLOTNUM      nVMin;          // 垂直最小值
    HANPLOTNUM      nHRange;        // 右侧取值范围
    HANPLOTNUM      nVRange;        // 左侧取值范围
    LONG            w;              // 绘图区域宽
    LONG            h;              // 绘图区域高
    BOOL            bPoint;         // 绘制点
    BOOL            bLine;          // 绘制线
} HANSCATTERDP, * HANPSCATTERDP;
typedef const HANSCATTERDP* HANPCSCATTERDP;
typedef struct tagHANSCATTER HANSCATTER, * HANPSCATTER;
typedef const HANSCATTER* HANPCSCATTER;
struct tagHANSCATTER { // 散点图结构体，也是类附加空间存放的指针指向的窗口信息
    HANDLE          hHeap;          // 堆句柄
    BOOL            bRedraw;        // 重绘 flag
    HANSIZE         nVarCount;      // 变量个数
    HANSIZE         nDataLen;       // 每个变量容纳的数据个数
    HANPPLOTDS      pVarData;       // 变量的具体信息
    HANSIZE         nHValueCount;   // 绘图界面上水平方向显示的值的个数
    HANSIZE         nVValueCount;   // 绘图界面上垂直方向显示的值的个数
    HPEN            hMarkLine;      // 标线画笔
    BOOL            bAutoRect;      // 自动大小模式
    RECT            rcAutoRectDis;  // 自动大小模式下绘图区到窗口的距离
    RECT            rcRectangle;    // 绘图区域的矩形范围
    HANLSIZE        lVRectangleW;   // 虚拟矩形（画下全部点所需要的矩形）的宽度，绘图时根据实际矩形的宽度和该宽度计算需要绘制的点
    HANLSIZE        lVRectangleH;   // 虚拟矩形（画下全部点所需要的矩形）的高度，绘图时根据实际矩形的高度和该高度计算需要绘制的点
    HANLSIZE        lRectangleX;    // 绘图矩形区域在虚拟区的 X 坐标
    HANLSIZE        lRectangleY;    // 绘图矩形区域在虚拟区的 Y 坐标
    HANLSIZE        lVRectangleWt;  // 最后一次放大操作后的虚拟矩形宽
    HANLSIZE        lVRectangleHt;  // 最后一次放大操作后的虚拟矩形高
    COLORREF        crTitleColor;   // 标题文字颜色
    LOGFONT         lfTitleFont;    // 标题文字字体
    HFONT           hTitleFont;     // 标题文字字体句柄
    COLORREF        crTextColor;    // 其它文字颜色
    LOGFONT         lfTextFont;     // 其它文本字体
    HFONT           hTextFont;      // 其它文字字体句柄
    COLORREF        crBkColor;      // 窗口背景色
    HBRUSH          hBkBrush;       // 窗口背景画刷句柄
    COLORREF        crBkBorder;     // 窗口背景边框颜色
    HPEN            hBkBorder;      // 窗口背景边框画笔句柄
    COLORREF        crRcColor;      // 绘图区域背景色
    COLORREF        crRcBorder;     // 绘图区域边框颜色
    HBRUSH          hRcBrush;       // 绘图区域背景画刷句柄
    HBRUSH          hRcDarBrush;    // 绘图区域背景暗色画刷句柄
    HPEN            hRcBorder;      // 绘图区域边框画笔句柄
    PPOINT          pBufPoint;      // 绘制点的缓存，在设置数据长度时同时分配好该缓存，避免绘图时反复申请内存
    HANSIZE         nTipW;          // 鼠标悬停数值提示区域宽
    LOGFONT         lfTipFont;      // 鼠标悬停数值提示字体
    HFONT           hTipFont;       // 鼠标悬停数值提示字体句柄
    COLORREF        crTipBk;        // 鼠标悬停数值提示背景颜色
    COLORREF        crTipBorder;    // 鼠标悬停数值提示边框颜色
    HPEN            hTipBorder;     // 鼠标悬停数值提示边框画笔句柄
    HPEN            hTipBkPen;      // 鼠标悬停数值提示背景画笔句柄
    HBRUSH          hTipBkBrush;    // 鼠标悬停数值提示背景画刷句柄
    HFONT           hButtonFont;    // 按钮字体句柄
    HWND            hbMove;         // 移动按钮句柄
    HWND            hbEnlarge;      // 放大按钮句柄
    HWND            hbReduce;       // 缩小按钮句柄
    HWND            hbRefresh;      // 刷新按钮句柄
    EPLOTCURSORMODE eCursorMode;    // 鼠标指针模式
    EPLOTMOUSEACT   eLastMouseAct;  // 最后一次鼠标动作
    POINT           ptMouseActPos;  // 鼠标按下 / 弹起的位置
    POINT           ptEnlargeBegin; // 放大模式下鼠标按下的位置
    POINT           ptEnlargeCur;   // 放大模式下鼠标的当前位置
    POINT           ptMoveBegin;    // 移动模式下上一刻鼠标的位置
    BOOL            bEnlage;        // 放大 flag
    BOOL            bMove;          // 移动 flag
    HANINT             nPointSize;     // 点的直径
    HANINT             nLineWidth;     // 线宽
    BOOL            bMarkDis;       // 由用户设置了标线间隔
    HANPLOTNUM      nMarkDis;       // 用户设置的标线间隔
    BOOL            bMaxMin;        // 由用户设置了左侧最值
    HANPLOTNUM      nHMax;          // 用户设置的左侧最大值
    HANPLOTNUM      nHMin;          // 用户设置的左侧最小值
    HANPLOTNUM      nVMax;          // 用户设置的右侧最大值
    HANPLOTNUM      nVMin;          // 用户设置的右侧最大值
    BOOL            bCut;           // 裁剪数据
    HANSIZE         nCutBegin;      // 裁剪的起始下标
    HANSIZE         nCutEnd;        // 裁剪的截止下标
    HANSCATTERDP    dpDrawParam;    // 绘图参数，绘图时所有参数在这里计算
};
// 内部函数声明
static HANERROR HANScatterUpdateResources(HANPSCATTER stInfo);
static void HANScatterPaintCallback(HDC hdc, HWND hScatter, HANPSCATTER stInfo, const RECT* rcWin);
static void HANScatterTextOut(HDC hdc, HWND hScatter, HANPSCATTER stInfo, const RECT* rcWin);
static void HANScatterLineOut(HDC hdc, HWND hScatter, HANPSCATTER stInfo);
static void HANScatterMarkLine(HDC hdc, HANPSCATTER stInfo);
static void HANScatterDrawTip(HDC hdc, HWND hScatter, HANPSCATTER stInfo);
static void HANScatterMoveButtonCallback(HANPSCATTER stInfo);
static void HANScatterEnlargeButtonCallback(HANPSCATTER stInfo);
static BOOL HANScatterReduceButtonCallback(HANPSCATTER stInfo);
static BOOL HANScatterRefreshButtonCallback(HANPSCATTER stInfo, BOOL bResetCursor);
static void HANScatterSizeCallback(HWND hScatter, HANPSCATTER stInfo);
static BOOL HANScatterMouseMoveCallback(HWND hScatter, HANPSCATTER stInfo, WPARAM wParam, LPARAM lParam);
static void HANScatterLButtonDownCallback(HANPSCATTER stInfo, LPARAM lParam);
static BOOL HANScatterLButtonUpCallback(HWND hScatter, HANPSCATTER stInfo, LPARAM lParam);
static HANPPLOTDS HANScatterMouseInVarText(HWND hScatter, HANPCSCATTER stInfo, const POINT* pptMousePos);
static BOOL HANScatterDrawVar(HANPCPLOTDS pVar);
static BOOL HANScatterEnlargeRect(HWND hScatter, HANPCSCATTER stInfo, RECT* rcRect);
// WndProc & 实现 声明
static LRESULT CALLBACK HANScatterWndProc(HWND hScatter, UINT message, WPARAM wParam, LPARAM lParam);
static void HANScatterTimerCallback(HWND hScatter, HANPSCATTER stInfo);
static HANERROR HANScatterSetDataLenCallback(HANPSCATTER stInfo, HANSIZE nDataLen);
static HANERROR HANScatterAddPointVarCallback(HANPSCATTER stInfo, HANPCSTR pVarName);
static HANERROR HANScatterAddTipVarCallback(HANPSCATTER stInfo, HANPCSTR pVarName);
static HANERROR HANScatterUpdateDataByNameCallback(HANPSCATTER stInfo, HANPCSTR pVarName, HANPCSTSETVALUE svData, EPLOTUPDATEDATA modeUpdate);
static HANERROR HANScatterUpdateDataByIdCallback(HANPSCATTER stInfo, HANSIZE nId, HANPCSTSETVALUE svData, EPLOTUPDATEDATA modeUpdate);
static HANERROR HANScatterSetVarColorCallback(HANPSCATTER stInfo, HANPCSTR pVarName, HANSIZE nId, COLORREF rgb);
static void HANScatterSetMaxMinCallback(HANPSCATTER stInfo, HANPCSTSETMAXMIN hvMaxMin, BOOL bUserMaxMin);
static HANERROR HANScatterSetBackgroundColorCallback(HANPSCATTER stInfo, COLORREF rgb);
static HANERROR HANScatterSetBackgroundBorderCallback(HANPSCATTER stInfo, COLORREF rgb);
static HANERROR HANScatterSetRectangleColorCallback(HANPSCATTER stInfo, COLORREF rgb);
static HANERROR HANScatterSetRectangleBorderCallback(HANPSCATTER stInfo, COLORREF rgb);
static HANERROR HANScatterSetTitleFontCallback(HANPSCATTER stInfo, const LOGFONT* pTitleFont);
static HANERROR HANScatterSetTextFontCallback(HANPSCATTER stInfo, const LOGFONT* pTextFont);
static void HANScatterSetRcRectangleCallback(HWND hScatter, HANPSCATTER stInfo, const RECT* rcRectangle, BOOL bAutoRect);
static BOOL HANScatterGetRcRectangleCallback(HANPSCATTER stInfo, RECT* rcRectangle);
static void HANScatterSetHValueCountCallback(HANPSCATTER stInfo, HANSIZE n);
static void HANScatterSetVValueCountCallback(HANPSCATTER stInfo, HANSIZE n);
static HANERROR HANScatterSetLineSizeCallback(HANPSCATTER stInfo, HANINT nWidth);
static void HANScatterSetPointSizeCallback(HANPSCATTER stInfo, HANINT nSize);
static HANERROR HANScatterSetPointStyleCallback(HANPSCATTER stInfo, HANPCSTR pVarName, HANSIZE nId, HANHPOINT hPoint);
static void HANScatterCutDataCallback(HANPSCATTER stInfo, HANPCSIZE pPos, BOOL bCut);
static void HANScatterSetTipWCallback(HANPSCATTER stInfo, HANSIZE w);
static HANERROR HANScatterSetTipFontCallback(HANPSCATTER stInfo, const LOGFONT* pTipFont);
static HANERROR HANScatterSetTipBkColorCallback(HANPSCATTER stInfo, COLORREF rgb);
static HANERROR HANScatterSetTipBorderCallback(HANPSCATTER stInfo, COLORREF rgb);
static void HANScatterSetMarkDisCallback(HANPSCATTER stInfo, HANPCPLOTNUM nMarkDis, BOOL bMarkDis);
// 接口定义
void RegisterHANScatter(HINSTANCE hInstance)
{
    WNDCLASSEX wcexHANScatter = {
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_PARENTDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = HANScatterWndProc,
        .cbClsExtra = 0,
        .cbWndExtra = sizeof(HANPSCATTER),
        .hInstance = hInstance,
        .hIcon = 0,
        .hCursor = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName = NULL,
        .lpszClassName = HST_CLASS,
        .hIconSm = NULL
    };
    RegisterClassEx(&wcexHANScatter);
}
HANERROR HANScatterSetDataLen(HWND hScatter, HANSIZE nDataLen)
{
    return SendMessage(hScatter, HSTM_SETDATALEN, (WPARAM)nDataLen, 0);
}
HANERROR HANScatterAddPointVar(HWND hScatter, HANPCSTR pVarName)
{
    return SendMessage(hScatter, HSTM_ADDPOINTVAR, 0, (LPARAM)pVarName);
}
HANERROR HANScatterAddTipVar(HWND hScatter, HANPCSTR pVarName)
{
    return SendMessage(hScatter, HSTM_ADDTIPVAR, 0, (LPARAM)pVarName);
}
HANERROR HANScatterSetStartPointByName(HWND hScatter, HANPCSTR pVarName, HANPCPLOTDATA pHData, HANPCPLOTDATA pVData)
{
    HANSTSETVALUE svData = {
        .nSetMode = HSTSV_SET_TWO_ARR,
        .pHData = *pHData,
        .pVData = *pVData,
    };
    HANERROR ret = SendMessage(hScatter, HSTM_SETSTARTDATABYNAME, (WPARAM)pVarName, (LPARAM)(&svData));
    return ret;
}
HANERROR HANScatterPushStartPointByName(HWND hScatter, HANPCSTR pVarName, HANPCPLOTDATA pHData, HANPCPLOTDATA pVData)
{
    HANSTSETVALUE svData = {
        .nSetMode = HSTSV_SET_TWO_ARR,
        .pHData = *pHData,
        .pVData = *pVData,
    };
    HANERROR ret = SendMessage(hScatter, HSTM_PUSHSTARTDATABYNAME, (WPARAM)pVarName, (LPARAM)(&svData));
    return ret;
}
HANERROR HANScatterSetEndPointByName(HWND hScatter, HANPCSTR pVarName, HANPCPLOTDATA pHData, HANPCPLOTDATA pVData)
{
    HANSTSETVALUE svData = {
        .nSetMode = HSTSV_SET_TWO_ARR,
        .pHData = *pHData,
        .pVData = *pVData,
    };
    HANERROR ret = SendMessage(hScatter, HSTM_SETENDDATABYNAME, (WPARAM)pVarName, (LPARAM)(&svData));
    return ret;
}
HANERROR HANScatterPushEndPointByName(HWND hScatter, HANPCSTR pVarName, HANPCPLOTDATA pHData, HANPCPLOTDATA pVData)
{
    HANSTSETVALUE svData = {
        .nSetMode = HSTSV_SET_TWO_ARR,
        .pHData = *pHData,
        .pVData = *pVData,
    };
    HANERROR ret = SendMessage(hScatter, HSTM_PUSHENDDATABYNAME, (WPARAM)pVarName, (LPARAM)(&svData));
    return ret;
}
HANERROR HANScatterSetStartPointById(HWND hScatter, HANSIZE nId, HANPCPLOTDATA pHData, HANPCPLOTDATA pVData)
{
    HANSTSETVALUE svData = {
        .nSetMode = HSTSV_SET_TWO_ARR,
        .pHData = *pHData,
        .pVData = *pVData,
    };
    HANERROR ret = SendMessage(hScatter, HSTM_SETSTARTDATABYID, (WPARAM)nId, (LPARAM)(&svData));
    return ret;
}
HANERROR HANScatterPushStartPointById(HWND hScatter, HANSIZE nId, HANPCPLOTDATA pHData, HANPCPLOTDATA pVData)
{
    HANSTSETVALUE svData = {
        .nSetMode = HSTSV_SET_TWO_ARR,
        .pHData = *pHData,
        .pVData = *pVData,
    };
    HANERROR ret = SendMessage(hScatter, HSTM_PUSHSTARTDATABYID, (WPARAM)nId, (LPARAM)(&svData));
    return ret;
}
HANERROR HANScatterSetEndPointById(HWND hScatter, HANSIZE nId, HANPCPLOTDATA pHData, HANPCPLOTDATA pVData)
{
    HANSTSETVALUE svData = {
        .nSetMode = HSTSV_SET_TWO_ARR,
        .pHData = *pHData,
        .pVData = *pVData,
    };
    HANERROR ret = SendMessage(hScatter, HSTM_SETENDDATABYID, (WPARAM)nId, (LPARAM)(&svData));
    return ret;
}
HANERROR HANScatterPushEndPointById(HWND hScatter, HANSIZE nId, HANPCPLOTDATA pHData, HANPCPLOTDATA pVData)
{
    HANSTSETVALUE svData = {
        .nSetMode = HSTSV_SET_TWO_ARR,
        .pHData = *pHData,
        .pVData = *pVData,
    };
    HANERROR ret = SendMessage(hScatter, HSTM_PUSHENDDATABYID, (WPARAM)nId, (LPARAM)(&svData));
    return ret;
}
HANERROR HANScatterSetTipValueByName(HWND hScatter, HANPCSTR pVarName, HANPCPLOTDATA pData)
{
    HANSTSETVALUE svData = {
        .nSetMode = HSTSV_SET_TIP,
        .pTipData = *pData,
    };
    HANERROR ret = SendMessage(hScatter, HSTM_SETSTARTDATABYNAME, (WPARAM)pVarName, (LPARAM)(&svData));
    return ret;
}
HANERROR HANScatterSetTipValueById(HWND hScatter, HANSIZE nId, HANPCPLOTDATA pData)
{
    HANSTSETVALUE svData = {
        .nSetMode = HSTSV_SET_TIP,
        .pTipData = *pData,
    };
    HANERROR ret = SendMessage(hScatter, HSTM_SETSTARTDATABYID, (WPARAM)nId, (LPARAM)(&svData));
    return ret;
}
HANERROR HANScatterSetVarColorByName(HWND hScatter, HANPCSTR pVarName, COLORREF rgb, BOOL bRedraw)
{
    HANERROR ret = SendMessage(hScatter, HSTM_SETVARCOLORBYNAME, (WPARAM)rgb, (LPARAM)pVarName);
    if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hScatter, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    return ret;
}
HANERROR HANScatterSetVarColorById(HWND hScatter, HANSIZE nId, COLORREF rgb, BOOL bRedraw)
{
    HANERROR ret = SendMessage(hScatter, HSTM_SETVARCOLORBYID, (WPARAM)rgb, (LPARAM)nId);
    if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hScatter, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    return ret;
}
void HANScatterSetMaxMin(HWND hScatter, HANPCPLOTMAXMIN hMaxMin, HANPCPLOTMAXMIN vMaxMin, BOOL bUserMaxMin, BOOL bRedraw)
{
    HANSTSETMAXMIN hvMaxMin = {
        .hMaxMin = *hMaxMin,
        .vMaxMin = *vMaxMin,
    };
    SendMessage(hScatter, HSTM_SETMAXMIN, (WPARAM)bUserMaxMin, (LPARAM)(&hvMaxMin));
    if (TRUE == bRedraw) { RedrawWindow(hScatter, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
}
HANERROR HANScatterSetBackgroundColor(HWND hScatter, COLORREF rgb, BOOL bRedraw)
{
    HANERROR ret = SendMessage(hScatter, HSTM_SETBKCOLOR, (WPARAM)rgb, 0);
    if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hScatter, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    return ret;
}
HANERROR HANScatterSetBkBorder(HWND hScatter, COLORREF rgb, BOOL bRedraw)
{
    HANERROR ret = SendMessage(hScatter, HSTM_SETBKBORDER, (WPARAM)rgb, 0);
    if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hScatter, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    return ret;
}
HANERROR HANScatterSetRectangleColor(HWND hScatter, COLORREF rgb, BOOL bRedraw)
{
    HANERROR ret = SendMessage(hScatter, HSTM_SETRECTCOLOR, (WPARAM)rgb, 0);
    if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hScatter, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    return ret;
}
HANERROR HANScatterSetRectBorder(HWND hScatter, COLORREF rgb, BOOL bRedraw)
{
    HANERROR ret = SendMessage(hScatter, HSTM_SETRECTBORDER, (WPARAM)rgb, 0);
    if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hScatter, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    return ret;
}
HANERROR HANScatterSetTitleFont(HWND hScatter, const LOGFONT* pTitleFont, BOOL bRedraw)
{
    HANERROR ret = SendMessage(hScatter, HSTM_SETTITLEFONT, 0, (LPARAM)pTitleFont);
    if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hScatter, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    return ret;
}
HANERROR HANScatterSetTextFont(HWND hScatter, const LOGFONT* pTextFont, BOOL bRedraw)
{
    HANERROR ret = SendMessage(hScatter, HSTM_SETTEXTFONT, 0, (LPARAM)pTextFont);
    if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hScatter, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    return ret;
}
void HANScatterSetRcRectangle(HWND hScatter, const RECT* rcRectangle, BOOL bAutoRect, BOOL bRedraw)
{
    SendMessage(hScatter, HSTM_SETRECT, (WPARAM)bAutoRect, (LPARAM)rcRectangle);
    if (TRUE == bRedraw) { RedrawWindow(hScatter, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
}
BOOL HANScatterGetRcRectangle(HWND hScatter, RECT* rcRectangle)
{
    return (BOOL)SendMessage(hScatter, HSTM_GETRECT, 0, (LPARAM)rcRectangle);
}
void HANScatterSetHValueCount(HWND hScatter, unsigned int n, BOOL bRedraw)
{
    SendMessage(hScatter, HSTM_SETHVALCOUNT, (WPARAM)n, 0);
    if (TRUE == bRedraw) { RedrawWindow(hScatter, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
}
void HANScatterSetVValueCount(HWND hScatter, unsigned int n, BOOL bRedraw)
{
    SendMessage(hScatter, HSTM_SETVVALCOUNT, (WPARAM)n, 0);
    if (TRUE == bRedraw) { RedrawWindow(hScatter, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
}
HANERROR HANScatterSetPointLineSize(HWND hScatter, HANSTPLSIZE eObject, HANINT nObjSize, BOOL bRedraw)
{
    HANERROR ret = SendMessage(hScatter, HSTM_SETPOINTLINESIZE, (WPARAM)eObject, (LPARAM)nObjSize);
    if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hScatter, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    return ret;
}
HANERROR HANScatterSetPointStyleByName(HWND hScatter, HANPCSTR pVarName, HANHPOINT hPoint, BOOL bRedraw)
{
    HANERROR ret;
    if (NULL != hPoint)
    {
        ret = SendMessage(hScatter, HSTM_SETPOINTSTYLEBYNAME, (WPARAM)pVarName, (LPARAM)hPoint);
        if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hScatter, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    }
    else
    {
        ret = PLOT_INVALID_HPOINT;
    }
    return ret;
}
HANERROR HANScatterSetPointStyleById(HWND hScatter, HANSIZE nId, HANHPOINT hPoint, BOOL bRedraw)
{
    HANERROR ret;
    if (NULL != hPoint)
    {
        ret = SendMessage(hScatter, HSTM_SETPOINTSTYLEBYID, (WPARAM)nId, (LPARAM)hPoint);
        if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hScatter, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    }
    else
    {
        ret = PLOT_INVALID_HPOINT;
    }
    return ret;
}
void HANScatterCutData(HWND hScatter, HANPCSIZE pPos, BOOL bCut, BOOL bRedraw)
{
    SendMessage(hScatter, HSTM_CUTDATA, (WPARAM)bCut, (LPARAM)pPos);
    if (TRUE == bRedraw) { RedrawWindow(hScatter, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
}
void HANScatterSetTipW(HWND hScatter, HANSIZE w, BOOL bRedraw)
{
    SendMessage(hScatter, HSTM_SETTIPW, (WPARAM)w, 0);
    if (TRUE == bRedraw) { RedrawWindow(hScatter, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
}
HANERROR HANScatterSetTipFont(HWND hScatter, const LOGFONT* pTipFont, BOOL bRedraw)
{
    HANERROR ret = SendMessage(hScatter, HSTM_SETTIPFONT, 0, (LPARAM)pTipFont);
    if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hScatter, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    return ret;
}
HANERROR HANScatterSetTipBkColor(HWND hScatter, COLORREF rgb, BOOL bRedraw)
{
    HANERROR ret = SendMessage(hScatter, HSTM_SETTIPBKCOLOR, (WPARAM)rgb, 0);
    if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hScatter, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    return ret;
}
HANERROR HANScatterSetTipBorder(HWND hScatter, COLORREF rgb, BOOL bRedraw)
{
    HANERROR ret = SendMessage(hScatter, HSTM_SETTIPBORDER, (WPARAM)rgb, 0);
    if ((TRUE == bRedraw) && (RET_OK == ret)) { RedrawWindow(hScatter, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    return ret;
}
HANERROR HANScatterSetMarkDis(HWND hScatter, HANPLOTNUM nMarkDis, BOOL bMarkDis, BOOL bRedraw)
{
    HANERROR nRet = RET_OK;
    if (nMarkDis <= (HANPLOTNUM)0) { nRet = HST_MARK_DIS_ERR; }
    else
    {
        SendMessage(hScatter, HSTM_SETMARKDIS, (WPARAM)bMarkDis, (LPARAM)(&nMarkDis));
        if (TRUE == bRedraw) { RedrawWindow(hScatter, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); }
    }
    return nRet;
}
// 内部函数定义
static HANERROR HANScatterUpdateResources(HANPSCATTER stInfo)
{
    HANERROR nRet = RET_OK;
    HANPPLOTDS pNode = stInfo->pVarData;
    HPEN* hPen = (HPEN*)HANWinHeapAlloc(stInfo->hHeap, NULL, (stInfo->nVarCount) * sizeof(HPEN));
    HBRUSH* hBrush = (HBRUSH*)HANWinHeapAlloc(stInfo->hHeap, NULL, (stInfo->nVarCount) * sizeof(HBRUSH));

    if ((NULL == hPen) || (NULL == hBrush)) { nRet = NOT_ENOUGH_MEMORY; }
    if (RET_OK == nRet)
    {
        for (HANSIZE i = 0; i < stInfo->nVarCount; i++)
        {
            hPen[i] = CreatePen(PS_SOLID, stInfo->nLineWidth, pNode->crColor);
            hBrush[i] = CreateSolidBrush(pNode->crColor);
            if ((NULL == hPen[i]) || (NULL == hBrush[i]))
            {
                for (HANSIZE j = 0; j <= i; j++)
                {
                    if (NULL != *(hPen + i)) { DeleteObject(*(hPen + i)); }
                    if (NULL != *(hBrush + i)) { DeleteObject(*(hBrush + i)); }
                }
                nRet = CREATE_OBJECT_ERR;
                break;
            }
            pNode = pNode->next;
        }
    }
    if (RET_OK == nRet)
    {
        pNode = stInfo->pVarData;
        for (HANSIZE i = 0; i < stInfo->nVarCount; i++)
        {
            if (NULL != pNode->hPen) { DeleteObject(pNode->hPen); }
            if (NULL != pNode->hBrush) { DeleteObject(pNode->hBrush); }
            pNode->hPen = *(hPen + i);
            pNode->hBrush = *(hBrush + i);
            pNode = pNode->next;
        }
    }

    if (NULL != hPen) { HANWinHeapFree(stInfo->hHeap, 0, hPen); }
    if (NULL != hBrush) { HANWinHeapFree(stInfo->hHeap, 0, hBrush); }
    return nRet;
}
static void HANScatterPaintCallback(HDC hdc, HWND hScatter, HANPSCATTER stInfo, const RECT* rcWin)
{
    stInfo->dpDrawParam.bPointExist = FALSE;
    HANPCPLOTDS pNode = stInfo->pVarData;
    // 获取最值
    if (TRUE == stInfo->bMaxMin)
    {
        stInfo->dpDrawParam.nHMax = stInfo->nHMax;
        stInfo->dpDrawParam.nHMin = stInfo->nHMin;
        stInfo->dpDrawParam.nVMax = stInfo->nVMax;
        stInfo->dpDrawParam.nVMin = stInfo->nVMin;
        stInfo->dpDrawParam.bPointExist = TRUE;
        stInfo->dpDrawParam.nHRange = stInfo->dpDrawParam.nHMax - stInfo->dpDrawParam.nHMin;
        stInfo->dpDrawParam.nVRange = stInfo->dpDrawParam.nVMax - stInfo->dpDrawParam.nVMin;
    }
    else
    {
        while (NULL != pNode)
        {
            if (HST_VAR_POS_POINT == (EHSTVARPOS)(pNode->ePos))
            {
                if (TRUE == stInfo->dpDrawParam.bPointExist)
                {
                    if (pNode->nMax > stInfo->dpDrawParam.nHMax) { stInfo->dpDrawParam.nHMax = pNode->nMax; }
                    if (pNode->nMin < stInfo->dpDrawParam.nHMin) { stInfo->dpDrawParam.nHMin = pNode->nMin; }
                    if (pNode->pChild->nMax > stInfo->dpDrawParam.nVMax) { stInfo->dpDrawParam.nVMax = pNode->pChild->nMax; }
                    if (pNode->pChild->nMin < stInfo->dpDrawParam.nVMin) { stInfo->dpDrawParam.nVMin = pNode->pChild->nMin; }
                }
                else
                {
                    stInfo->dpDrawParam.nHMax = pNode->nMax;
                    stInfo->dpDrawParam.nHMin = pNode->nMin;
                    stInfo->dpDrawParam.nVMax = pNode->pChild->nMax;
                    stInfo->dpDrawParam.nVMin = pNode->pChild->nMin;
                    stInfo->dpDrawParam.bPointExist = TRUE;
                }
            }
            pNode = pNode->next;
        }
        stInfo->dpDrawParam.nHRange = stInfo->dpDrawParam.nHMax - stInfo->dpDrawParam.nHMin;
        stInfo->dpDrawParam.nVRange = stInfo->dpDrawParam.nVMax - stInfo->dpDrawParam.nVMin;
        // 跟据参数修正要画的最值
        if ((0 != (GetWindowLong(hScatter, GWL_STYLE) & HSTS_MAP)) && (0 != stInfo->dpDrawParam.nHRange) && (0 != stInfo->dpDrawParam.nVRange))
        {
            HANDOUBLE kRange = stInfo->dpDrawParam.nHRange / stInfo->dpDrawParam.nVRange;
            HANDOUBLE kRect = (HANDOUBLE)GetRectW(&(stInfo->rcRectangle)) / GetRectH(&(stInfo->rcRectangle));
            if (kRect > kRange)
            {
                HANPLOTNUM nMean = stInfo->dpDrawParam.nHMin + (stInfo->dpDrawParam.nHRange / 2);
                stInfo->dpDrawParam.nHMin = nMean - (stInfo->dpDrawParam.nHRange / (HANPLOTNUM)2 * kRect / kRange);
                stInfo->dpDrawParam.nHMax = nMean + (stInfo->dpDrawParam.nHRange / (HANPLOTNUM)2 * kRect / kRange);
                stInfo->dpDrawParam.nHRange = stInfo->dpDrawParam.nHMax - stInfo->dpDrawParam.nHMin;
            }
            else
            {
                HANPLOTNUM nMean = stInfo->dpDrawParam.nVMin + (stInfo->dpDrawParam.nVRange / 2);
                stInfo->dpDrawParam.nVMin = nMean - (stInfo->dpDrawParam.nVRange / (HANPLOTNUM)2 * kRange / kRect);
                stInfo->dpDrawParam.nVMax = nMean + (stInfo->dpDrawParam.nVRange / (HANPLOTNUM)2 * kRange / kRect);
                stInfo->dpDrawParam.nVRange = stInfo->dpDrawParam.nVMax - stInfo->dpDrawParam.nVMin;
            }
        }
    }
    // 跟据放大参数修正最值
    HANDOUBLE kHMin = (HANDOUBLE)(stInfo->lRectangleX) / stInfo->lVRectangleW;
    HANDOUBLE kHMax = (HANDOUBLE)(stInfo->lRectangleX + GetRectW(&(stInfo->rcRectangle))) / stInfo->lVRectangleW;
    HANDOUBLE kVMax = (HANDOUBLE)(stInfo->lRectangleY) / stInfo->lVRectangleH;
    HANDOUBLE kVMin = (HANDOUBLE)(stInfo->lRectangleY + GetRectH(&(stInfo->rcRectangle))) / stInfo->lVRectangleH;
    stInfo->dpDrawParam.nHMax = stInfo->dpDrawParam.nHMin +  (stInfo->dpDrawParam.nHRange * kHMax);
    stInfo->dpDrawParam.nHMin += stInfo->dpDrawParam.nHRange * kHMin;
    stInfo->dpDrawParam.nVMin = stInfo->dpDrawParam.nVMax -  (stInfo->dpDrawParam.nVRange * kVMin);
    stInfo->dpDrawParam.nVMax -= stInfo->dpDrawParam.nVRange * kVMax;
    stInfo->dpDrawParam.nHRange = stInfo->dpDrawParam.nHMax - stInfo->dpDrawParam.nHMin;
    stInfo->dpDrawParam.nVRange = stInfo->dpDrawParam.nVMax - stInfo->dpDrawParam.nVMin;
    // 计算参数
    LONG lStyle = GetWindowLong(hScatter, GWL_STYLE);
    if (0 != (lStyle & HSTS_POINT)) { stInfo->dpDrawParam.bPoint = TRUE; } 
    else { stInfo->dpDrawParam.bPoint = FALSE; }
    if (0 != (lStyle & HSTS_LINE)) { stInfo->dpDrawParam.bLine = TRUE; } 
    else { stInfo->dpDrawParam.bLine = FALSE; }
    stInfo->dpDrawParam.w = GetRectW(&(stInfo->rcRectangle)) - 3;
    stInfo->dpDrawParam.h = GetRectH(&(stInfo->rcRectangle)) - 3;
    // 画线
    HANScatterLineOut(hdc, hScatter, stInfo);
    // 画背景，保存现场 hdc
    HPEN hOldPen = SelectObject(hdc, GetStockObject(NULL_PEN));
    HBRUSH hOldBrush = SelectObject(hdc, stInfo->hBkBrush);
    LONG nWinW = GetRectW(rcWin);
    LONG nWinH = GetRectH(rcWin);
    Rectangle(hdc, 1, 1, stInfo->rcRectangle.left + 1, nWinH);
    Rectangle(hdc, stInfo->rcRectangle.left, 1, stInfo->rcRectangle.right + 1, stInfo->rcRectangle.top + 1);
    Rectangle(hdc, stInfo->rcRectangle.right, 1, nWinW, nWinH);
    Rectangle(hdc, stInfo->rcRectangle.left, stInfo->rcRectangle.bottom, stInfo->rcRectangle.right + 1, nWinH);
    // 画边框
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    SelectObject(hdc, stInfo->hBkBorder);
    Rectangle(hdc, 0, 0, nWinW, nWinH);
    SelectObject(hdc, stInfo->hRcBorder);
    Rectangle(hdc, stInfo->rcRectangle.left, stInfo->rcRectangle.top, stInfo->rcRectangle.right, stInfo->rcRectangle.bottom);
    // 还原 hdc
    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
    // 输出文字
    HANScatterTextOut(hdc, hScatter, stInfo, rcWin);
    // 输出鼠标悬停数值提示
    HANScatterDrawTip(hdc, hScatter, stInfo);
}
static void HANScatterTextOut(HDC hdc, HWND hScatter, HANPSCATTER stInfo, const RECT* rcWin)
{
    HANCHAR pText[HAN_PLOT_STR_LEN_MAX];
    LONG nWinW = GetRectW(rcWin);
    LONG nWinH = GetRectH(rcWin);
    (void)nWinH;
    // 设置 hdc 参数：标题文字颜色、字体、对齐方式、透明背景
    COLORREF crOldTextColor = SetTextColor(hdc, stInfo->crTitleColor);
    HFONT hOldFont = SelectObject(hdc, stInfo->hTitleFont);
    UINT nOldTextAlign = SetTextAlign(hdc, TA_CENTER | TA_TOP);
    HANINT nOldBkMode = SetBkMode(hdc, TRANSPARENT);
    // 绘制标题
    GetWindowText(hScatter, pText, HAN_PLOT_STR_LEN_MAX);
    TextOut(hdc, nWinW / 2, 1, pText, (HANINT)(HAN_strlen(pText)));
    // 绘制变量
    HANINT nTextY = HST_VAR_TEXT_Y_BEGIN;
    HANPCPLOTDS pNode = stInfo->pVarData;
    HANPCSTR pName;
    SelectObject(hdc, stInfo->hTextFont);
    while (NULL != pNode)
    {
        if (HST_VAR_POS_POINT == (EHSTVARPOS)(pNode->ePos))
        {
            SetTextAlign(hdc, TA_RIGHT | TA_TOP);
            SetTextColor(hdc, pNode->crColor);
            pName = pNode->pName;
            TextOut(hdc, nWinW - 10, nTextY, pName, (HANINT)(HAN_strlen(pName)));
            nTextY += stInfo->lfTextFont.lfHeight;
        }
        pNode = pNode->next;
    }
    // 绘制数值
    if (stInfo->nDataLen > (HANSIZE)0)
    {
        HANINT nTextCharCount;
        HANDOUBLE kNum;
        // 绘制垂直数值
        SetTextColor(hdc, stInfo->crTextColor);
        HANINT nVValueBeginY = stInfo->rcRectangle.top;
        kNum = (HANDOUBLE)GetRectH(&(stInfo->rcRectangle)) / (HANDOUBLE)(stInfo->nVValueCount - (HANSIZE)1);
        for (HANSIZE i = 0; i < stInfo->nVValueCount; i++)
        {
            if (TRUE == stInfo->dpDrawParam.bPointExist)
            {
                SetTextAlign(hdc, TA_RIGHT);
                nTextCharCount = HAN_snprintf(
                    pText, HAN_PLOT_STR_LEN_MAX, TEXT("%g"),
                    stInfo->dpDrawParam.nVMax - (i * (stInfo->dpDrawParam.nVRange) / (stInfo->nVValueCount - (HANSIZE)1))
                );
                TextOut(
                    hdc,
                    stInfo->rcRectangle.left - (LONG)3,
                    nVValueBeginY - (stInfo->lfTextFont.lfHeight / (LONG)2) + (HANINT)((HANDOUBLE)i * kNum),
                    pText,
                    nTextCharCount
                );
            }
        }
        // 绘制水平数值
        HANINT nHValueBeginX = stInfo->rcRectangle.left;
        kNum = (HANDOUBLE)GetRectW(&(stInfo->rcRectangle)) / (HANDOUBLE)(stInfo->nHValueCount - (HANSIZE)1);
        for (HANSIZE i = 0; i < stInfo->nHValueCount; i++)
        {
            SetTextAlign(hdc, TA_CENTER | TA_TOP);
            if (TRUE == stInfo->dpDrawParam.bPointExist)
            {
                nTextCharCount = HAN_snprintf(
                    pText, HAN_PLOT_STR_LEN_MAX, TEXT("%g"),
                    stInfo->dpDrawParam.nHMin + (i * (stInfo->dpDrawParam.nHRange) / (stInfo->nHValueCount - (HANSIZE)1))
                );
                TextOut(
                    hdc,
                    nHValueBeginX + (HANINT)((HANDOUBLE)i * kNum),
                    stInfo->rcRectangle.bottom + (stInfo->lfTextFont.lfHeight / 2),
                    pText,
                    nTextCharCount
                );
            }
        }
    }
    // 还原 hdc 参数
    SetTextColor(hdc, crOldTextColor);
    SetTextAlign(hdc, nOldTextAlign);
    SetBkMode(hdc, nOldBkMode);
    SelectObject(hdc, hOldFont);
}
static void HANScatterLineOut(HDC hdc, HWND hScatter, HANPSCATTER stInfo)
{
    // 绘制区域矩形，保护 hdc
    HPEN hOldPen = SelectObject(hdc, GetStockObject(NULL_PEN));
    HBRUSH hOldBrush = SelectObject(hdc, stInfo->hRcBrush);
    Rectangle(hdc, stInfo->rcRectangle.left, stInfo->rcRectangle.top, stInfo->rcRectangle.right, stInfo->rcRectangle.bottom);
    SelectObject(hdc, stInfo->hRcDarBrush);
    HANPPLOTDS pNode = stInfo->pVarData;
    // 绘制放大按钮按下时的阴影区域
    if (TRUE == stInfo->bEnlage)
    {
        RECT rcLight;
        HANScatterEnlargeRect(hScatter, stInfo, &rcLight);
        Rectangle(hdc, stInfo->rcRectangle.left, stInfo->rcRectangle.top, stInfo->rcRectangle.right, rcLight.top);
        Rectangle(hdc, stInfo->rcRectangle.left, rcLight.bottom, stInfo->rcRectangle.right, stInfo->rcRectangle.bottom);
        Rectangle(hdc, stInfo->rcRectangle.left, rcLight.top - 1, rcLight.left, rcLight.bottom + 1);
        Rectangle(hdc, rcLight.right, rcLight.top - 1, stInfo->rcRectangle.right, rcLight.bottom + 1);
    }
    // 标线
    if (0 != (GetWindowLong(hScatter, GWL_STYLE) & HSTS_MARKLINE)) { HANScatterMarkLine(hdc, stInfo); }
    // 画线
    HANSIZE iBegin = 0;
    HANSIZE iLen = stInfo->nDataLen;
    if (TRUE == stInfo->bCut)
    {
        iBegin = stInfo->nCutBegin;
        if (stInfo->nCutEnd < iLen) { iLen = stInfo->nCutEnd; }
    }
    HANPLOTDRAWPOINTPARAM dpDrawPointParam = { .hdc = hdc, .nSize = stInfo->nPointSize, .ptPoint = stInfo->pBufPoint, .nPointLen = iLen - iBegin };
    while (NULL != pNode)
    {
        if (TRUE == HANScatterDrawVar(pNode))
        {
            pNode->nIdBegin = 0;
            pNode->nIdEnd = 0;
            SelectObject(hdc, pNode->hBrush);
            SelectObject(hdc, pNode->hPen);
            pNode->bInRect = FALSE;
            for (HANSIZE i = iBegin; i < iLen; i++)
            {
                stInfo->pBufPoint[i].x = stInfo->rcRectangle.left + (LONG)(GetRectW(&(stInfo->rcRectangle)) * (pNode->pData[i] - stInfo->dpDrawParam.nHMin) / stInfo->dpDrawParam.nHRange);
                stInfo->pBufPoint[i].y = stInfo->rcRectangle.top + (LONG)(GetRectH(&(stInfo->rcRectangle)) * (stInfo->dpDrawParam.nVMax - pNode->pChild->pData[i]) / stInfo->dpDrawParam.nVRange);
                if (TRUE == PosInRect(&(stInfo->pBufPoint)[i], &(stInfo->rcRectangle))) // 如果点在绘图区内，记下 id ，供鼠标悬停使用
                {
                    if (!pNode->bInRect) { pNode->nIdBegin = i; pNode->bInRect = TRUE; }
                    pNode->nIdEnd = i + (HANSIZE)1;
                }
            }
            if (TRUE == stInfo->dpDrawParam.bLine) { Polyline(hdc, stInfo->pBufPoint + iBegin, (HANINT)(iLen - iBegin)); }
            if (TRUE == stInfo->dpDrawParam.bPoint) { pNode->DrawPoint(&dpDrawPointParam); }
        }
        pNode = pNode->next;
    }
    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
}
static void HANScatterMarkLine(HDC hdc, HANPSCATTER stInfo)
{
    SelectObject(hdc, stInfo->hMarkLine);
    HANDOUBLE nHLineDis;
    HANDOUBLE nVLineDis;
    LONG nHForLen;
    LONG nVForLen;
    if (TRUE == stInfo->bMarkDis)
    {
        nHLineDis = ((HANDOUBLE)GetRectW(&(stInfo->rcRectangle))) / (stInfo->dpDrawParam.nHRange / stInfo->nMarkDis);
        nVLineDis = ((HANDOUBLE)GetRectH(&(stInfo->rcRectangle))) / (stInfo->dpDrawParam.nVRange / stInfo->nMarkDis);
        if (nHLineDis < (HANDOUBLE)2) { nHLineDis = (HANDOUBLE)2; }
        if (nVLineDis < (HANDOUBLE)2) { nVLineDis = (HANDOUBLE)2; }
        nHForLen = (LONG)(GetRectW(&(stInfo->rcRectangle)) / nHLineDis) + 1;
        nVForLen = (LONG)(GetRectW(&(stInfo->rcRectangle)) / nVLineDis) + 1;
    }
    else
    {
        nHLineDis = ((HANDOUBLE)GetRectW(&(stInfo->rcRectangle))) / (stInfo->nHValueCount - (HANSIZE)1);
        nVLineDis = ((HANDOUBLE)GetRectH(&(stInfo->rcRectangle))) / (stInfo->nVValueCount - (HANSIZE)1);
        nHForLen = (LONG)(stInfo->nHValueCount - (HANSIZE)1);
        nVForLen = (LONG)(stInfo->nVValueCount - (HANSIZE)1);
    }
    for (LONG i = 1; i < nHForLen; i++)
    {
        LONG nMarkLineX = stInfo->rcRectangle.left + (LONG)(i * nHLineDis);
        MoveToEx(hdc, nMarkLineX, stInfo->rcRectangle.top + 1, NULL);
        LineTo(hdc, nMarkLineX, stInfo->rcRectangle.bottom - 1);
    }
    for (LONG i = 1; i < nVForLen; i++)
    {
        LONG nMarkLineY = stInfo->rcRectangle.top + (LONG)(i * nVLineDis);
        MoveToEx(hdc, stInfo->rcRectangle.left + 1, nMarkLineY, NULL);
        LineTo(hdc, stInfo->rcRectangle.right - 1, nMarkLineY);
    }
}
static void HANScatterDrawTip(HDC hdc, HWND hScatter, HANPSCATTER stInfo)
{
    BOOL bRet = FALSE;
    if (0 != (GetWindowLong(hScatter, GWL_STYLE) & HSTS_ENABLETIP))
    {
        if (!(stInfo->dpDrawParam.bPointExist)) { bRet = TRUE; }
        // 检测数据，生成文本
        if (!bRet)
        {
            POINT ptMousePos;
            GetCursorPos(&ptMousePos);
            ScreenToClient(hScatter, &ptMousePos);
            if (TRUE == PosInRect(&ptMousePos, &(stInfo->rcRectangle)))
            {
                HANSIZE nLineCount = 1; // 准备绘制的文本行数
                HANPPLOTDS pNode = stInfo->pVarData;
                HANPPLOTDS pTar = NULL; // 目标节点
                HANDOUBLE nDisMin = (HANDOUBLE)HST_DEF_TIP_DIS + (HANDOUBLE)1;
                HANDOUBLE nDisTemp;
                HANSIZE iTar;   // 目标 id
                while (NULL != pNode)
                {
                    if (HST_VAR_POS_TIP == (EHSTVARPOS)(pNode->ePos)) { nLineCount++; }
                    pNode->pText[0] = TEXT('\0');
                    if (TRUE == pNode->bDraw)
                    {
                        for (HANSIZE i = pNode->nIdBegin; i < pNode->nIdEnd; i++)
                        {
                            LONG lY;
                            LONG lX;
                            HANPLOTNUM nHRange = stInfo->dpDrawParam.nHRange;
                            HANPLOTNUM nHMin = stInfo->dpDrawParam.nHMin;
                            HANPLOTNUM nVRange = stInfo->dpDrawParam.nVRange;
                            HANPLOTNUM nVMax = stInfo->dpDrawParam.nVMax;
                            if ((HANPLOTNUM)0 != nHRange)
                            {
                                lX = stInfo->rcRectangle.left + (LONG)round((pNode->pData[i] - nHMin) * (HANPLOTNUM)(stInfo->dpDrawParam.w) / nHRange);
                            }
                            else
                            {
                                lX = stInfo->rcRectangle.left + (stInfo->dpDrawParam.w / (LONG)2);
                            }
                            if ((HANPLOTNUM)0 != nVRange)
                            {
                                lY = stInfo->rcRectangle.top + (LONG)round((nVMax - pNode->pChild->pData[i]) * (HANPLOTNUM)(stInfo->dpDrawParam.h) / nVRange);
                            }
                            else
                            {
                                lY = stInfo->rcRectangle.top + (stInfo->dpDrawParam.h / (LONG)2);
                            }
                            nDisTemp = sqrt(pow(lX - ptMousePos.x, 2) + pow(lY - ptMousePos.y, 2));
                            if (nDisTemp < nDisMin)
                            {
                                iTar = i;
                                pTar = pNode;
                            }
                        }
                    }
                    pNode = pNode->next;
                }

                if (NULL != pTar)
                {
                    // 保存 hdc 现场
                    COLORREF crOldColor = SetTextColor(hdc, 0);
                    HPEN hOldPen = SelectObject(hdc, stInfo->hTipBorder);
                    HBRUSH hOldBrush = SelectObject(hdc, stInfo->hTipBkBrush);
                    HFONT hOldFont = SelectObject(hdc, stInfo->hTipFont);
                    HANINT nOldBkMode = SetBkMode(hdc, TRANSPARENT);

                    // 绘制背景
                    LONG lTriangleW = 8;
                    LONG lTriangleH = 10;
                    LONG lRdRectR = 5;
                    LONG lTextH = (LONG)(stInfo->lfTipFont.lfHeight * nLineCount);
                    LONG lTipRectTop = ptMousePos.y - (lTriangleH / 2) - lRdRectR;
                    RECT rcTipRect = {
                        .left = ptMousePos.x + lTriangleW,
                        .top = lTipRectTop,
                        .right = ptMousePos.x + lTriangleW + (LONG)(stInfo->nTipW),
                        .bottom = lTipRectTop + lTextH + 2,
                    };
                    if (GetRectH(&rcTipRect) < (lTriangleH + (lRdRectR * 2))) { rcTipRect.bottom = rcTipRect.top + lTriangleH + (lRdRectR * 2); }
                    RoundRect(hdc, rcTipRect.left, rcTipRect.top, rcTipRect.right, rcTipRect.bottom, lRdRectR, lRdRectR);
                    POINT ptTriangle[] = {
                        { .x = ptMousePos.x, .y = ptMousePos.y, },
                        { .x = ptMousePos.x + lTriangleW, .y = ptMousePos.y - (lTriangleH / 2), },
                        { .x = ptMousePos.x + lTriangleW, .y = ptMousePos.y + (lTriangleH / 2), },
                    };
                    Polygon(hdc, ptTriangle, ArrLen(ptTriangle));
                    SelectObject(hdc, stInfo->hTipBkPen);
                    POINT ptBkPen[] = {
                        { .x = ptTriangle[1].x, .y = ptTriangle[1].y + 1 },
                        { .x = ptTriangle[2].x, .y = ptTriangle[2].y },
                    };
                    Polyline(hdc, ptBkPen, ArrLen(ptBkPen));
                    // 绘制选中的点数值
                    rcTipRect.left += 2;
                    rcTipRect.right -= 2;
                    pNode = stInfo->pVarData;
                    HANCHAR pVarText[HAN_PLOT_STR_LEN_MAX * 2];
                    HAN_snprintf(
                        pVarText, HAN_PLOT_STR_LEN_MAX * 2,
                        TEXT("%s=%g,%g"), pTar->pName, pTar->pData[iTar], pTar->pChild->pData[iTar]
                    );
                    SetTextColor(hdc, pTar->crColor);
                    DrawText(hdc, pVarText, -1, &rcTipRect, DT_SINGLELINE);
                    rcTipRect.top += stInfo->lfTipFont.lfHeight;
                    // 绘制 HST_VAR_POS_TIP 位置的数值
                    while (NULL != pNode)
                    {
                        if (HST_VAR_POS_TIP == (EHSTVARPOS)(pNode->ePos))
                        {
                            HAN_snprintf(
                                pNode->pText, HAN_PLOT_STR_LEN_MAX,
                                TEXT("%s=%g"), pNode->pName, pNode->pData[iTar]
                            );
                            SetTextColor(hdc, pNode->crColor);
                            DrawText(hdc, pNode->pText, -1, &rcTipRect, DT_SINGLELINE);
                            rcTipRect.top += stInfo->lfTipFont.lfHeight;
                        }
                        pNode = pNode->next;
                    }
                    
                    // 恢复 hdc 现场
                    SetTextColor(hdc, crOldColor);
                    SelectObject(hdc, hOldPen);
                    SelectObject(hdc, hOldBrush);
                    SelectObject(hdc, hOldFont);
                    SetBkMode(hdc, nOldBkMode);
                }
            }
        }
    }
}
static void HANScatterMoveButtonCallback(HANPSCATTER stInfo)
{
    stInfo->eCursorMode = HPLOT_CM_MOVE;
}
static void HANScatterEnlargeButtonCallback(HANPSCATTER stInfo)
{
    stInfo->eCursorMode = HPLOT_CM_ENLARGE;
}
static BOOL HANScatterReduceButtonCallback(HANPSCATTER stInfo)
{
    BOOL bRedraw = FALSE;
    HANLSIZE nDrawW = (HANLSIZE)GetRectW(&(stInfo->rcRectangle));
    HANLSIZE nDrawH = (HANLSIZE)GetRectH(&(stInfo->rcRectangle));
    HANLSIZE nOldCenterX = stInfo->lRectangleX + (nDrawW / (HANLSIZE)2);
    HANLSIZE nOldCenterY = stInfo->lRectangleY + (nDrawH / (HANLSIZE)2);
    HANLSIZE nOldPaperW = stInfo->lVRectangleW;
    HANLSIZE nOldPaperH = stInfo->lVRectangleH;
    HANLSIZE nReduceW = (HANLSIZE)(stInfo->lVRectangleWt * HST_DEF_REDUCE_SIZE);
    HANLSIZE nReduceH = (HANLSIZE)(stInfo->lVRectangleHt * HST_DEF_REDUCE_SIZE);
    HANLSIZE nNewPaperW;
    HANLSIZE nNewPaperH;
    if (nOldPaperW < (nReduceW + nDrawW)) { nNewPaperW = nDrawW; } else { nNewPaperW = nOldPaperW - nReduceW; }
    if (nOldPaperH < (nReduceH + nDrawH)) { nNewPaperH = nDrawH; } else { nNewPaperH = nOldPaperH - nReduceH; }
    HANLSIZE nNewCenterX = nOldCenterX * nNewPaperW / nOldPaperW;
    HANLSIZE nNewCenterY = nOldCenterY * nNewPaperH / nOldPaperH;
    HANLSIZE nNewX;
    HANLSIZE nNewY;
    if (nNewPaperW < (nNewCenterX + (nDrawW / 2))) { nNewX = nNewPaperW - nDrawW; }
    else if (nNewCenterX < (nDrawW / 2)) { nNewX = 0; }
    else { nNewX = nNewCenterX - (nDrawW / 2); }
    if (nNewPaperH < (nNewCenterY + (nDrawH / 2))) { nNewY = nNewPaperH - nDrawH; }
    else if (nNewCenterY < (nDrawH / 2)) { nNewY = 0; }
    else { nNewY = nNewCenterY - (nDrawH / 2); }
    
    if ((nNewPaperW != stInfo->lVRectangleW) || (nNewX != stInfo->lRectangleX) ||
        (nNewPaperH != stInfo->lVRectangleH) || (nNewY != stInfo->lRectangleY))
    {
        bRedraw = TRUE;
    }
    stInfo->lVRectangleW = nNewPaperW;
    stInfo->lVRectangleH = nNewPaperH;
    stInfo->lRectangleX = nNewX;
    stInfo->lRectangleY = nNewY;

    return bRedraw;
}
static BOOL HANScatterRefreshButtonCallback(HANPSCATTER stInfo, BOOL bResetCursor)
{
    BOOL bRedraw = FALSE;
    if (TRUE == bResetCursor) { stInfo->eCursorMode = HPLOT_CM_NORMAL; }
    if ((stInfo->lVRectangleW != (HANLSIZE)GetRectW(&(stInfo->rcRectangle))) || (stInfo->lVRectangleH != (HANLSIZE)GetRectH(&(stInfo->rcRectangle)))) { bRedraw = TRUE; }
    stInfo->lVRectangleW = (HANLSIZE)GetRectW(&(stInfo->rcRectangle));
    stInfo->lVRectangleH = (HANLSIZE)GetRectH(&(stInfo->rcRectangle));
    stInfo->lRectangleX = 0;
    stInfo->lRectangleY = 0;

    return bRedraw;
}
static void HANScatterSizeCallback(HWND hScatter, HANPSCATTER stInfo)
{
    RECT rcClient;
    GetClientRect(hScatter, &rcClient);
    LONG nOldWinW = GetRectW(&(stInfo->rcRectangle));
    LONG nOldWinH = GetRectH(&(stInfo->rcRectangle));
    stInfo->rcRectangle.right = rcClient.right - stInfo->rcAutoRectDis.right;
    stInfo->rcRectangle.bottom = rcClient.bottom - stInfo->rcAutoRectDis.bottom;
    HANDOUBLE kW = (HANDOUBLE)GetRectW(&(stInfo->rcRectangle)) / nOldWinW;
    HANDOUBLE kH = (HANDOUBLE)GetRectH(&(stInfo->rcRectangle)) / nOldWinH;
    stInfo->lRectangleX = (HANLSIZE)((HANDOUBLE)(stInfo->lRectangleX) * kW);
    stInfo->lRectangleY = (HANLSIZE)((HANDOUBLE)(stInfo->lRectangleY) * kH);
    stInfo->lVRectangleW = (HANLSIZE)((HANDOUBLE)(stInfo->lVRectangleW) * kW);
    stInfo->lVRectangleH = (HANLSIZE)((HANDOUBLE)(stInfo->lVRectangleH) * kH);
}
static BOOL HANScatterMouseMoveCallback(HWND hScatter, HANPSCATTER stInfo, WPARAM wParam, LPARAM lParam)
{
    BOOL bRedraw = FALSE;
    POINT ptMousePos = {
        .x = LOWORD(lParam),
        .y = HIWORD(lParam),
    };
    if (!(wParam & MK_LBUTTON)) // 按下左键后要锁定指针（跳过该 if ）
    {
        if (TRUE == PosInRect(&ptMousePos, &(stInfo->rcRectangle)))
        {
            switch (stInfo->eCursorMode) {
                case HPLOT_CM_NORMAL: {
                    SetCursor(LoadCursor(NULL, IDC_ARROW));
                } break;
                case HPLOT_CM_MOVE: {
                    SetCursor(LoadCursor(NULL, IDC_SIZEALL));
                } break;
                case HPLOT_CM_ENLARGE: {
                    SetCursor(LoadCursor(NULL, IDC_CROSS));
                } break;

                default: {
                    /* idle */
                } break;
            }
        }
        else
        {
            SetCursor(LoadCursor(NULL, IDC_ARROW));
        }
    }
    else
    {
        switch (stInfo->eCursorMode) {
            case HPLOT_CM_MOVE: {
                LONG dX = LOWORD(lParam) - stInfo->ptMoveBegin.x;   // 鼠标 X坐标
                LONG dY = HIWORD(lParam) - stInfo->ptMoveBegin.y;   // 鼠标 X坐标
                HANLSIZE nNewX = stInfo->lRectangleX - dX;
                HANLSIZE nNewY = stInfo->lRectangleY - dY;
                HANLSIZE nMaxX = stInfo->lVRectangleW - GetRectW(&(stInfo->rcRectangle));
                HANLSIZE nMaxY = stInfo->lVRectangleH - GetRectH(&(stInfo->rcRectangle));
                if ((dX > 0) && (stInfo->lRectangleX < (HANLSIZE)dX)) { nNewX = 0; }
                if ((dY > 0) && (stInfo->lRectangleY < (HANLSIZE)dY)) { nNewY = 0; }
                if (nNewX > nMaxX) { nNewX = nMaxX; }
                if (nNewY > nMaxY) { nNewY = nMaxY; }
                if ((stInfo->lRectangleX != nNewX) || (stInfo->lRectangleY != nNewY))
                {
                    stInfo->lRectangleX = nNewX;
                    stInfo->lRectangleY = nNewY;
                    stInfo->ptMoveBegin = ptMousePos;
                    bRedraw = TRUE;
                }
            } break;
            case HPLOT_CM_ENLARGE: {
                stInfo->ptEnlargeCur = ptMousePos;
                bRedraw = TRUE;
            } break;

            default: {
                /* idle */
            } break;
        }
    }
    if (0 != (GetWindowLong(hScatter, GWL_STYLE) & HSTS_ENABLETIP)) { bRedraw = TRUE; }
    
    return bRedraw;
}
static void HANScatterLButtonDownCallback(HANPSCATTER stInfo, LPARAM lParam)
{
    POINT ptMousePos = {
        .x = LOWORD(lParam),
        .y = HIWORD(lParam),
    };
    if (TRUE == PosInRect(&ptMousePos, &(stInfo->rcRectangle)))
    {
        switch (stInfo->eCursorMode) {
            case HPLOT_CM_MOVE: {
                stInfo->bMove = TRUE;
                stInfo->ptMoveBegin = ptMousePos;
            } break;
            case HPLOT_CM_ENLARGE: {
                stInfo->bEnlage = TRUE;
                stInfo->ptEnlargeBegin = ptMousePos;
                stInfo->ptEnlargeCur = ptMousePos;
            } break;

            default: {
                /* idle */
            } break;
        }
    }

    stInfo->eLastMouseAct = HPL_MA_LEFTDOWN;
    stInfo->ptMouseActPos = ptMousePos;
}
static BOOL HANScatterLButtonUpCallback(HWND hScatter, HANPSCATTER stInfo, LPARAM lParam)
{
    BOOL bRedraw = FALSE;
    POINT ptMousePos = {
        .x = LOWORD(lParam),
        .y = HIWORD(lParam),
    };

    switch (stInfo->eCursorMode) { // 绘图区操作
        case HPLOT_CM_MOVE: {
            stInfo->bMove = FALSE;
        } break;
        case HPLOT_CM_ENLARGE: {
            if (HPL_MA_LEFTDOWN == stInfo->eLastMouseAct)
            {
                LONG rcRectW = GetRectW(&(stInfo->rcRectangle)); // 绘图区域宽
                LONG rcRectH = GetRectH(&(stInfo->rcRectangle)); // 绘图区域高
                RECT rcEnlarge;
                HANScatterEnlargeRect(hScatter, stInfo, &rcEnlarge);
                LONG nEnlargeW = GetRectW(&rcEnlarge); // 鼠标拖动前后位置的宽
                LONG nEnlargeH = GetRectH(&rcEnlarge); // 鼠标拖动前后位置的高
                if ((nEnlargeW > 0) && (nEnlargeH > 0))
                {
                    HANLSIZE sNewW = stInfo->lVRectangleW * rcRectW / nEnlargeW;
                    HANLSIZE sNewH = stInfo->lVRectangleH * rcRectH / nEnlargeH;
                    stInfo->lRectangleX = (stInfo->lRectangleX + rcEnlarge.left - stInfo->rcRectangle.left) * rcRectW / nEnlargeW;
                    stInfo->lRectangleY = (stInfo->lRectangleY + rcEnlarge.top - stInfo->rcRectangle.top) * rcRectH / nEnlargeH;
                    stInfo->lVRectangleW = sNewW;
                    stInfo->lVRectangleH = sNewH;
                    stInfo->lVRectangleWt = sNewW;
                    stInfo->lVRectangleHt = sNewH;
                    bRedraw = TRUE;
                }
                stInfo->bEnlage = FALSE;
            }
        } break;

        default: {
            /* idle */
        } break;
    }

    if (HPL_MA_LEFTDOWN == stInfo->eLastMouseAct) // 点击变量
    {
        HANPPLOTDS pCurrentVar = HANScatterMouseInVarText(hScatter, stInfo, &ptMousePos);
        HANPPLOTDS pLBDownVAR = HANScatterMouseInVarText(hScatter, stInfo, &(stInfo->ptMouseActPos));
        if ((NULL != pCurrentVar) && (pCurrentVar == pLBDownVAR))
        {
            pCurrentVar->bDraw = (!(pCurrentVar->bDraw));
        }
    }

    stInfo->eLastMouseAct = HPL_MA_LEFTUP;
    stInfo->ptMouseActPos = ptMousePos;

    return bRedraw;
}
static HANPPLOTDS HANScatterMouseInVarText(HWND hScatter, HANPCSCATTER stInfo, const POINT* pptMousePos)
{
    HANPPLOTDS pRet = stInfo->pVarData;
    HANINT nTextY = HST_VAR_TEXT_Y_BEGIN;
    HDC hdc = GetDC(hScatter);
    HFONT hOldFont = SelectObject(hdc, stInfo->hTextFont);
    while (NULL != pRet)
    {
        if (HST_VAR_POS_POINT == (EHSTVARPOS)(pRet->ePos))
        {
            SIZE sTextSize;
            RECT rcWin;
            GetWindowRect(hScatter, &rcWin);
            LONG nWinW = GetRectW(&rcWin);
            GetTextExtentExPoint(hdc, pRet->pName, (HANINT)(HAN_strlen(pRet->pName)), 0, NULL, NULL, &sTextSize);
            RECT rcTextRect = {
                .left = nWinW - 10 - sTextSize.cx,
                .top = nTextY,
                .right = nWinW - 10,
                .bottom = nTextY + stInfo->lfTextFont.lfHeight,
            };
            if (TRUE == PosInRect(pptMousePos, &rcTextRect))
            {
                break;
            }
            nTextY += stInfo->lfTextFont.lfHeight;
        }
        pRet = pRet->next;
    }
    SelectObject(hdc, hOldFont);

    return pRet;
}
static BOOL HANScatterDrawVar(HANPCPLOTDS pVar)
{
    BOOL bRet;
    if ((TRUE == pVar->bDraw) && (HST_VAR_POS_POINT == (EHSTVARPOS)(pVar->ePos))) { bRet = TRUE; }
    else { bRet = FALSE; }
    return bRet;
}
static BOOL HANScatterEnlargeRect(HWND hScatter, HANPCSCATTER stInfo, RECT* rcRect)
{
    BOOL bRet;
    if (0 != (GetWindowLong(hScatter, GWL_STYLE) & HSTS_MAP))
    {
        bRet = SimilarRect(rcRect, &(stInfo->rcRectangle), &(stInfo->ptEnlargeBegin), &(stInfo->ptEnlargeCur));
        if (TRUE == bRet)
        {
            BOOL bRe;
            POINT ptNew;
            HANINT xHeading;
            HANINT yHeading;
            for (size_t i = 0; i < (size_t)2; i++)
            {
                if (rcRect->right > stInfo->ptEnlargeBegin.x) { xHeading = 1; } else { xHeading = -1; }
                if (rcRect->bottom > stInfo->ptEnlargeBegin.y) { yHeading = 1; } else { yHeading = -1; }
                bRe = FALSE;
                if (rcRect->left < stInfo->rcRectangle.left) { ptNew.x = stInfo->rcRectangle.left; ptNew.y = stInfo->ptEnlargeBegin.y + yHeading; bRe = TRUE; }
                else if (rcRect->right > stInfo->rcRectangle.right) { ptNew.x = stInfo->rcRectangle.right; ptNew.y = stInfo->ptEnlargeBegin.y + yHeading; bRe = TRUE; }
                else if (rcRect->top < stInfo->rcRectangle.top) { ptNew.y = stInfo->rcRectangle.top; ptNew.x = stInfo->ptEnlargeBegin.x + xHeading; bRe = TRUE; }
                else if (rcRect->bottom > stInfo->rcRectangle.bottom) { ptNew.y = stInfo->rcRectangle.bottom; ptNew.x = stInfo->ptEnlargeBegin.x + xHeading; bRe = TRUE; }
                else { }
                if (TRUE == bRe) { bRet = SimilarRect(rcRect, &(stInfo->rcRectangle), &(stInfo->ptEnlargeBegin), &ptNew); }
                if (FALSE == bRet) { break; }
            }
        }
    }
    else
    {
        if ((stInfo->ptEnlargeBegin.x == stInfo->ptEnlargeCur.x) || (stInfo->ptEnlargeBegin.y == stInfo->ptEnlargeCur.y))
        {
            bRet = FALSE;
        }
        else
        {
            if (stInfo->ptEnlargeBegin.x < stInfo->ptEnlargeCur.x)
            {
                rcRect->left = stInfo->ptEnlargeBegin.x;
                rcRect->right = stInfo->ptEnlargeCur.x;
            }
            else
            {
                rcRect->left = stInfo->ptEnlargeCur.x;
                rcRect->right = stInfo->ptEnlargeBegin.x;
            }
            if (stInfo->ptEnlargeBegin.y < stInfo->ptEnlargeCur.y)
            {
                rcRect->top = stInfo->ptEnlargeBegin.y;
                rcRect->bottom = stInfo->ptEnlargeCur.y;
            }
            else
            {
                rcRect->top = stInfo->ptEnlargeCur.y;
                rcRect->bottom = stInfo->ptEnlargeBegin.y;
            }
            if (rcRect->left < stInfo->rcRectangle.left) { rcRect->left = stInfo->rcRectangle.left; }
            if (rcRect->top < stInfo->rcRectangle.top) { rcRect->top = stInfo->rcRectangle.top; }
            if (rcRect->right > stInfo->rcRectangle.right) { rcRect->right = stInfo->rcRectangle.right; }
            if (rcRect->bottom > stInfo->rcRectangle.bottom) { rcRect->bottom = stInfo->rcRectangle.bottom; }
            bRet = TRUE;
        }
    }
    return bRet;
}
// WndProc & 实现 定义
static LRESULT CALLBACK HANScatterWndProc(HWND hScatter, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lWndProcRet = 0;
    // 读取属性
    HANPSCATTER stInfo = (HANPSCATTER)GetWindowLongPtr(hScatter, 0);

    switch (message) {
        case WM_CREATE: { // 创建窗口
            HANDLE hHeap = GetProcessHeap();
            if (NULL == hHeap) { lWndProcRet = -1; break; }
            stInfo = (HANPSCATTER)HANWinHeapAlloc(hHeap, NULL, sizeof(HANSCATTER));
            if (NULL == stInfo) { lWndProcRet = -1; break; }
            SetWindowLongPtr(hScatter, 0, (LONG_PTR)stInfo);
            stInfo->hHeap = hHeap;
            // 获取窗口风格
            LONG style = ((LPCREATESTRUCTW)lParam)->style;
            // 获取窗口大小
            RECT rcWin;
            GetClientRect(hScatter, &rcWin);
            LONG nWinW = GetRectW(&rcWin);
            LONG nWinH = GetRectH(&rcWin);
            // 创建子窗口
            HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
            HANINT xPos = nWinW - (HST_CWDX_BUTTON + HST_CWSIZE_BUTTON);
            DWORD nButtonStyle = HPLOT_CWS_BUTTON;
            if (0 != (style & HSTS_ENABLEMOUSE)) { nButtonStyle |= WS_VISIBLE; }
            stInfo->hbRefresh = CreateWindow(TEXT("button"), TEXT("R"), nButtonStyle, xPos, HST_CWY_BUTTON, HST_CWSIZE_BUTTON, HST_CWSIZE_BUTTON, hScatter, (HMENU)HST_WID_REFRESH, hInst, NULL); xPos -= (HANINT)HST_CWDX_BUTTON + (HANINT)HST_CWSIZE_BUTTON;
            stInfo->hbReduce = CreateWindow(TEXT("button"), TEXT("-"), nButtonStyle, xPos, HST_CWY_BUTTON, HST_CWSIZE_BUTTON, HST_CWSIZE_BUTTON, hScatter, (HMENU)HST_WID_REDUCE, hInst, NULL); xPos -= (HANINT)HST_CWDX_BUTTON + (HANINT)HST_CWSIZE_BUTTON;
            stInfo->hbEnlarge = CreateWindow(TEXT("button"), TEXT("+"), nButtonStyle, xPos, HST_CWY_BUTTON, HST_CWSIZE_BUTTON, HST_CWSIZE_BUTTON, hScatter, (HMENU)HST_WID_ENLARGE, hInst, NULL); xPos -= (HANINT)HST_CWDX_BUTTON + (HANINT)HST_CWSIZE_BUTTON;
            stInfo->hbMove = CreateWindow(TEXT("button"), TEXT("M"), nButtonStyle, xPos, HST_CWY_BUTTON, HST_CWSIZE_BUTTON, HST_CWSIZE_BUTTON, hScatter, (HMENU)HST_WID_MOVE, hInst, NULL); xPos -= (HANINT)HST_CWDX_BUTTON + (HANINT)HST_CWSIZE_BUTTON;
            stInfo->hButtonFont = CreateFont(18, 9, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, HAN_PLOT_DEF_TEXT_FACE);
            SendMessage(stInfo->hbMove, WM_SETFONT, (WPARAM)stInfo->hButtonFont, (LPARAM)TRUE);
            SendMessage(stInfo->hbEnlarge, WM_SETFONT, (WPARAM)stInfo->hButtonFont, (LPARAM)TRUE);
            SendMessage(stInfo->hbReduce, WM_SETFONT, (WPARAM)stInfo->hButtonFont, (LPARAM)TRUE);
            SendMessage(stInfo->hbRefresh, WM_SETFONT, (WPARAM)stInfo->hButtonFont, (LPARAM)TRUE);
            (void)HANScatterSetBackgroundColorCallback(stInfo, RGB(0xFF, 0xFF, 0xFF));
            (void)HANScatterSetBackgroundBorderCallback(stInfo, RGB(0x00, 0x00, 0x00));
            (void)HANScatterSetRectangleColorCallback(stInfo, RGB(0xFF, 0xFF, 0xFF));
            (void)HANScatterSetRectangleBorderCallback(stInfo, RGB(0x00, 0x00, 0x00));
            LOGFONT lfTitleFont = GetDefaultLogFont(HST_DEF_TITLE_H, HST_DEF_TITLE_W, HAN_PLOT_DEF_TEXT_FACE);
            (void)HANScatterSetTitleFontCallback(stInfo, &lfTitleFont);
            LOGFONT lfTextFont = GetDefaultLogFont(HST_DEF_TEXT_H, HST_DEF_TEXT_W, HAN_PLOT_DEF_TEXT_FACE);
            (void)HANScatterSetTextFontCallback(stInfo, &lfTextFont);
            RECT rcRectangle = {
                .left = HST_DEF_RECT_LEFT,
                .top = HST_DEF_RECT_TOP,
                .right = HST_DEF_RECT_RIGHT,
                .bottom = HST_DEF_RECT_BOTTOM,
            };
            if ((HST_DEF_RECT_LEFT + HST_DEF_RECT_RIGHT) > nWinW) { rcRectangle.left = nWinW / 3; rcRectangle.right = nWinW / 3; }
            if ((HST_DEF_RECT_TOP + HST_DEF_RECT_BOTTOM) > nWinH) { rcRectangle.top = nWinH / 3; rcRectangle.bottom = nWinH / 3; }
            HANScatterSetRcRectangleCallback(hScatter, stInfo, &rcRectangle, TRUE);
            HANScatterSetHValueCountCallback(stInfo, HST_DEF_HVALUE_COUNT);
            HANScatterSetVValueCountCallback(stInfo, HST_DEF_VVALUE_COUNT);
            (void)HANScatterSetLineSizeCallback(stInfo, 1);
            HANScatterSetPointSizeCallback(stInfo, 10);
            HANScatterSetTipWCallback(stInfo, 100);
            LOGFONT lfTipFont = GetDefaultLogFont(HST_DEF_TIP_H, HST_DEF_TIP_W, HAN_PLOT_DEF_TEXT_FACE);
            (void)HANScatterSetTipFontCallback(stInfo, &lfTipFont);
            (void)HANScatterSetTipBkColorCallback(stInfo, RGB(0xFF, 0xFF, 0xFF));
            (void)HANScatterSetTipBorderCallback(stInfo, RGB(0, 0, 0));
            SetTimer(hScatter, 1, HST_DEF_TIMER_T, NULL);
        } break;
        case WM_SIZE: {
            if (TRUE == stInfo->bAutoRect) { HANScatterSizeCallback(hScatter, stInfo); }
            else { lWndProcRet = DefWindowProc(hScatter, message, wParam, lParam); }
        } break;
        case WM_SETCURSOR: { // 拦截系统光标
            LONG nStyle = GetWindowStyle(hScatter);
            if (!(nStyle & HSTS_ENABLEMOUSE)) { lWndProcRet = DefWindowProc(hScatter, message, wParam, lParam); }
        } break;
        case WM_TIMER: {
            HANScatterTimerCallback(hScatter, stInfo);
        } break;
        case WM_COMMAND: { // 控件指令
            HANINT wmId = LOWORD(wParam);
            switch (wmId) {
                case HST_WID_MOVE: {
                    HANScatterMoveButtonCallback(stInfo);
                } break;
                case HST_WID_ENLARGE: {
                    HANScatterEnlargeButtonCallback(stInfo);
                } break;
                case HST_WID_REDUCE: {
                    stInfo->bRedraw = HANScatterReduceButtonCallback(stInfo);
                } break;
                case HST_WID_REFRESH: {
                    stInfo->bRedraw = HANScatterRefreshButtonCallback(stInfo, TRUE);
                } break;

                default: {
                    /* idle */
                } break;
            }
        } break;
        case WM_LBUTTONDOWN: { // 鼠标左键按下
            LONG nStyle = GetWindowStyle(hScatter);
            if (0 != (nStyle & HSTS_ENABLEMOUSE)) { HANScatterLButtonDownCallback(stInfo, lParam); }
        } break;
        case WM_LBUTTONUP: { // 鼠标左键弹起
            LONG nStyle = GetWindowStyle(hScatter);
            if (0 != (nStyle & HSTS_ENABLEMOUSE)) { stInfo->bRedraw = HANScatterLButtonUpCallback(hScatter, stInfo, lParam); }
        } break;
        case WM_LBUTTONDBLCLK: { // 鼠标左键双击
            LONG nStyle = GetWindowStyle(hScatter);
            if (0 != (nStyle & HSTS_ENABLEMOUSE)) { stInfo->bRedraw = HANScatterRefreshButtonCallback(stInfo, FALSE); }
        } break;
        case WM_MOUSEMOVE: { // 鼠标移动
            LONG nStyle = GetWindowStyle(hScatter);
            if (0 != (nStyle & HSTS_ENABLEMOUSE)) { stInfo->bRedraw = HANScatterMouseMoveCallback(hScatter, stInfo, wParam, lParam); }
        } break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            // 开始绘图
            HDC hdc = BeginPaint(hScatter, &ps);
            // 双缓冲绘图，分配资源
            RECT rcWin;
            GetClientRect(hScatter, &rcWin);
            HDC hMemDC = CreateCompatibleDC(hdc);   // 获取内存设备上下文，图形统一在该 HDC 下绘制
            HBITMAP hBitMap = CreateCompatibleBitmap(hdc, GetRectW(&rcWin), GetRectH(&rcWin));  // 创建位图
            SelectObject(hMemDC, hBitMap);

            // 绘制内容
            HANScatterPaintCallback(hMemDC, hScatter, stInfo, &rcWin);

            // 将位图中绘制好的图像贴到屏幕上
            BitBlt(hdc, 0, 0, GetRectW(&rcWin), GetRectH(&rcWin), hMemDC, 0, 0, SRCCOPY);
            
            // 释放内存和位图
            DeleteDC(hMemDC);
            DeleteObject(hBitMap);
            // 结束绘图
            EndPaint(hScatter, &ps);
        } break;
        case WM_DESTROY: {
            if (NULL != stInfo)
            {
                KillTimer(hScatter, 1);
                HANPPLOTDS pNode = stInfo->pVarData;
                HANPPLOTDS pNext;
                HANWinHeapFree(stInfo->hHeap, 0, stInfo->pBufPoint);
                while (NULL != pNode)
                {
                    pNext = pNode->next;
                    if (HST_VAR_POS_POINT == (EHSTVARPOS)(pNode->ePos))
                    {
                        HANWinHeapFree(stInfo->hHeap, 0, pNode->pChild->pData);
                        HANWinHeapFree(stInfo->hHeap, 0, pNode->pChild);
                    }
                    HANWinHeapFree(stInfo->hHeap, 0, pNode->pData);
                    if (NULL != pNode->hPen) { DeleteObject(pNode->hPen); }
                    HANWinHeapFree(stInfo->hHeap, 0, pNode);
                    pNode = pNext;
                }
                HANWinHeapFree(stInfo->hHeap, 0, stInfo);
            }
        } break;

        case HSTM_SETDATALEN: {
            lWndProcRet = HANScatterSetDataLenCallback(stInfo, (HANSIZE)wParam);
        } break;
        case HSTM_ADDPOINTVAR: {
            lWndProcRet = HANScatterAddPointVarCallback(stInfo, (HANPCSTR)lParam);
        } break;
        case HSTM_ADDTIPVAR: {
            lWndProcRet = HANScatterAddTipVarCallback(stInfo, (HANPCSTR)lParam);
        } break;
        case HSTM_SETSTARTDATABYNAME: {
            lWndProcRet = HANScatterUpdateDataByNameCallback(stInfo, (HANPCSTR)wParam, (HANPCSTSETVALUE)lParam, HAN_PLOT_UPDATE_DATA_SET_START);
        } break;
        case HSTM_PUSHSTARTDATABYNAME: {
            lWndProcRet = HANScatterUpdateDataByNameCallback(stInfo, (HANPCSTR)wParam, (HANPCSTSETVALUE)lParam, HAN_PLOT_UPDATE_DATA_PUSH_START);
        } break;
        case HSTM_SETENDDATABYNAME: {
            lWndProcRet = HANScatterUpdateDataByNameCallback(stInfo, (HANPCSTR)wParam, (HANPCSTSETVALUE)lParam, HAN_PLOT_UPDATE_DATA_SET_END);
        } break;
        case HSTM_PUSHENDDATABYNAME: {
            lWndProcRet = HANScatterUpdateDataByNameCallback(stInfo, (HANPCSTR)wParam, (HANPCSTSETVALUE)lParam, HAN_PLOT_UPDATE_DATA_PUSH_END);
        } break;
        case HSTM_SETSTARTDATABYID: {
            lWndProcRet = HANScatterUpdateDataByIdCallback(stInfo, (HANSIZE)wParam, (HANPCSTSETVALUE)lParam, HAN_PLOT_UPDATE_DATA_SET_START);
        } break;
        case HSTM_PUSHSTARTDATABYID: {
            lWndProcRet = HANScatterUpdateDataByIdCallback(stInfo, (HANSIZE)wParam, (HANPCSTSETVALUE)lParam, HAN_PLOT_UPDATE_DATA_PUSH_START);
        } break;
        case HSTM_SETENDDATABYID: {
            lWndProcRet = HANScatterUpdateDataByIdCallback(stInfo, (HANSIZE)wParam, (HANPCSTSETVALUE)lParam, HAN_PLOT_UPDATE_DATA_SET_END);
        } break;
        case HSTM_PUSHENDDATABYID: {
            lWndProcRet = HANScatterUpdateDataByIdCallback(stInfo, (HANSIZE)wParam, (HANPCSTSETVALUE)lParam, HAN_PLOT_UPDATE_DATA_PUSH_END);
        } break;
        case HSTM_SETVARCOLORBYNAME: {
            lWndProcRet = HANScatterSetVarColorCallback(stInfo, (HANPCSTR)lParam, (HANSIZE)(-1), (COLORREF)wParam);
        } break;
        case HSTM_SETVARCOLORBYID: {
            lWndProcRet = HANScatterSetVarColorCallback(stInfo, (HANPCSTR)NULL, (HANSIZE)lParam, (COLORREF)wParam);
        } break;
        case HSTM_SETMAXMIN: {
            HANPCSTSETMAXMIN hvMaxMin = (HANPCSTSETMAXMIN)lParam;
            HANScatterSetMaxMinCallback(stInfo, hvMaxMin, (BOOL)wParam);
        } break;
        case HSTM_SETBKCOLOR: {
            (void)HANScatterSetBackgroundColorCallback(stInfo, (COLORREF)wParam);
        } break;
        case HSTM_SETBKBORDER: {
            lWndProcRet = HANScatterSetBackgroundBorderCallback(stInfo, (COLORREF)wParam);
        } break;
        case HSTM_SETRECTCOLOR: {
            lWndProcRet = HANScatterSetRectangleColorCallback(stInfo, (COLORREF)wParam);
        } break;
        case HSTM_SETRECTBORDER: {
            lWndProcRet = HANScatterSetRectangleBorderCallback(stInfo, (COLORREF)wParam);
        } break;
        case HSTM_SETTITLEFONT: {
            lWndProcRet = HANScatterSetTitleFontCallback(stInfo, (const LOGFONT*)lParam);
        } break;
        case HSTM_SETTEXTFONT: {
            lWndProcRet = HANScatterSetTextFontCallback(stInfo, (const LOGFONT*)lParam);
        } break;
        case HSTM_SETRECT: {
            HANScatterSetRcRectangleCallback(hScatter, stInfo, (RECT*)lParam, (BOOL)wParam);
        } break;
        case HSTM_GETRECT: {
            lWndProcRet = HANScatterGetRcRectangleCallback(stInfo, (RECT*)lParam);
        } break;
        case HSTM_SETHVALCOUNT: {
            HANScatterSetHValueCountCallback(stInfo, (HANINT)wParam);
        } break;
        case HSTM_SETVVALCOUNT: {
            HANScatterSetVValueCountCallback(stInfo, (HANINT)wParam);
        } break;
        case HSTM_SETPOINTLINESIZE: {
            switch (wParam) {
                case HST_PLS_LINE: {
                    lWndProcRet = HANScatterSetLineSizeCallback(stInfo, (HANINT)lParam);
                } break;
                case HST_PLS_POINT: {
                    HANScatterSetPointSizeCallback(stInfo, (HANINT)lParam);
                } break;

                default: {
                    /* idle */
                } break;
            }
        } break;
        case HSTM_SETPOINTSTYLEBYNAME: {
            lWndProcRet = HANScatterSetPointStyleCallback(stInfo, (HANPCSTR)wParam, (HANSIZE)(-1), (HANHPOINT)lParam);
        } break;
        case HSTM_SETPOINTSTYLEBYID: {
            lWndProcRet = HANScatterSetPointStyleCallback(stInfo, (HANPCSTR)NULL, (HANSIZE)wParam, (HANHPOINT)lParam);
        } break;
        case HSTM_CUTDATA: {
            HANScatterCutDataCallback(stInfo, (HANPCSIZE)lParam, (BOOL)wParam);
        } break;
        case HSTM_SETTIPW: {
            HANScatterSetTipWCallback(stInfo, (HANSIZE)wParam);
        } break;
        case HSTM_SETTIPFONT: {
            lWndProcRet = HANScatterSetTipFontCallback(stInfo, (const LOGFONT*)lParam);
        } break;
        case HSTM_SETTIPBKCOLOR: {
            lWndProcRet = HANScatterSetTipBkColorCallback(stInfo, (COLORREF)wParam);
        } break;
        case HSTM_SETTIPBORDER: {
            lWndProcRet = HANScatterSetTipBorderCallback(stInfo, (COLORREF)wParam);
        } break;
        case HSTM_SETMARKDIS: {
            HANScatterSetMarkDisCallback(stInfo, (HANPCPLOTNUM)lParam, (BOOL)wParam);
        } break;

        default: {
            lWndProcRet = DefWindowProc(hScatter, message, wParam, lParam);
        } break;
    }

    // 返回
    return lWndProcRet;
}
static void HANScatterTimerCallback(HWND hScatter, HANPSCATTER stInfo)
{
    if (TRUE == stInfo->bRedraw)
    {
        RedrawWindow(hScatter, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
        stInfo->bRedraw = FALSE;
    }
}
static HANERROR HANScatterSetDataLenCallback(HANPSCATTER stInfo, HANSIZE nDataLen)
{
    HANPPLOTDS pNode = stInfo->pVarData;
    POINT* pTmpPoint = NULL;
    HANPPLOTNUM pTmpData = NULL;
    HANERROR nRet = RET_OK;

    if (nDataLen > stInfo->nDataLen)
    {
        /* 点缓存重新分配内存 */
        pTmpPoint = (POINT*)HANWinHeapAlloc(stInfo->hHeap, stInfo->pBufPoint, nDataLen * sizeof(POINT));
        if (NULL == pTmpPoint) { nRet = NOT_ENOUGH_MEMORY; }
        else { stInfo->pBufPoint = pTmpPoint; }
        if (RET_OK == nRet)
        {
            while ((NULL != pNode) && (RET_OK == nRet))
            {
                /* 主节点重新分配内存 */
                pTmpData = (HANPPLOTNUM)HANWinHeapAlloc(stInfo->hHeap, pNode->pData, nDataLen * sizeof(HANPLOTNUM));
                if (NULL == pTmpData) { nRet = NOT_ENOUGH_MEMORY; }
                else { pNode->pData = pTmpData; }

                /* 子节点重新分配内存 */
                if ((NULL != pNode->pChild) && (RET_OK == nRet))
                {
                    pTmpData = HANWinHeapAlloc(stInfo->hHeap, pNode->pChild->pData, nDataLen * sizeof(HANPLOTNUM));
                    if (NULL == pTmpData) { nRet = NOT_ENOUGH_MEMORY; }
                    else { pNode->pChild->pData = pTmpData; }
                }
                pNode = pNode->next;
            }
        }
    }
    if (RET_OK == nRet) { stInfo->nDataLen = nDataLen; }

    return nRet;
}
static HANERROR HANScatterAddPointVarCallback(HANPSCATTER stInfo, HANPCSTR pVarName)
{
    HANERROR nRet = RET_OK;

    /* 获取新变量的颜色 */
    COLORREF crColor = 0;
    for (HANSIZE i = 0; i < stInfo->nVarCount; i++)
    {
        crColor = HANPlotAddColor(crColor);
    }
    crColor = HANPlotAddColor(crColor);
    
    /* 创建次源 */
    HANPPLOTNUM pHData = NULL;
    HANPPLOTNUM pVData = NULL;
    HANPPLOTDS pHNode = NULL;
    HANPPLOTDS pVNode = NULL;
    HPEN hPen = NULL;
    HBRUSH hBrush = NULL;

    if (stInfo->nDataLen > (HANSIZE)0)
    {
        pHData = (HANPPLOTNUM)HANWinHeapAlloc(stInfo->hHeap, NULL, stInfo->nDataLen * sizeof(HANPLOTNUM));
        pVData = (HANPPLOTNUM)HANWinHeapAlloc(stInfo->hHeap, NULL, stInfo->nDataLen * sizeof(HANPLOTNUM));
        if ((NULL == pHData) || (NULL == pVData)) { nRet = NOT_ENOUGH_MEMORY; }
    }
    pHNode = (HANPPLOTDS)HANWinHeapAlloc(stInfo->hHeap, NULL, sizeof(HANPLOTDS));
    pVNode = (HANPPLOTDS)HANWinHeapAlloc(stInfo->hHeap, NULL, sizeof(HANPLOTDS));
    if ((NULL == pHNode) || (NULL == pVNode)) { nRet = NOT_ENOUGH_MEMORY; }
    hPen = CreatePen(PS_SOLID, 0, crColor);
    if (NULL == hPen) { nRet = CREATE_PEN_ERR; }
    hBrush = CreateSolidBrush(crColor);
    if (NULL == hBrush) { nRet = CREATE_BRUSH_ERR; }
    if (RET_OK != nRet) /* 创建失败，清空资源，并返回失败值 */
    {
        if (NULL != pHData) { HANWinHeapFree(stInfo->hHeap, 0, pHData); }
        if (NULL != pVData) { HANWinHeapFree(stInfo->hHeap, 0, pVData); }
        if (NULL != pHNode) { HANWinHeapFree(stInfo->hHeap, 0, pHNode); }
        if (NULL != pVNode) { HANWinHeapFree(stInfo->hHeap, 0, pVNode); }
        if (NULL != hPen) { DeleteObject(hPen); }
        if (NULL != hBrush) { DeleteObject(hBrush); }
    }
    else    /* 将节点加入链表 */
    {
        HANPPLOTDS pLast = stInfo->pVarData;
        if (NULL == pLast) { stInfo->pVarData = pHNode; }
        else { while (NULL != pLast->next) { pLast = pLast->next; } pLast->next = pHNode; }
        pVNode->pData = pVData; /* 子节点只用作保存垂直数据，其它参数不用设置 */
        pVNode->nMax = 0;
        pVNode->nMin = 0;
        pHNode->pData = pHData;
        pHNode->nMax = 0;
        pHNode->nMin = 0;
        HAN_snprintf(pHNode->pName, HAN_PLOT_STR_LEN_MAX, TEXT("%s"), pVarName);
        pHNode->crColor = crColor;
        pHNode->ePos = HST_VAR_POS_POINT;
        pHNode->hPen = hPen;
        pHNode->hBrush = hBrush;
        pHNode->bDraw = TRUE;
        pHNode->DrawPoint = HANPlotGetStockPoint(HAN_PLOT_DEF_DRAW_POINT);
        pHNode->pChild = pVNode; /* 连接子节点 */
        pHNode->next = NULL;
        stInfo->nVarCount++;
    }

    return nRet;
}
static HANERROR HANScatterAddTipVarCallback(HANPSCATTER stInfo, HANPCSTR pVarName)
{
    HANERROR nRet = RET_OK;

    HANPPLOTNUM pData = NULL;
    HANPPLOTDS pNode = NULL;

    if (stInfo->nDataLen > (HANSIZE)0)
    {
        pData = (HANPPLOTNUM)HANWinHeapAlloc(stInfo->hHeap, NULL, stInfo->nDataLen * sizeof(HANPLOTNUM));
        if (NULL == pData) { nRet = NOT_ENOUGH_MEMORY; }
    }
    pNode = (HANPPLOTDS)HANWinHeapAlloc(stInfo->hHeap, NULL, sizeof(HANPLOTDS));
    if (NULL == pNode) { nRet = NOT_ENOUGH_MEMORY; }
    if (RET_OK != nRet)
    {
        if (NULL != pData) { HANWinHeapFree(stInfo->hHeap, 0, pData); }
        if (NULL != pNode) { HANWinHeapFree(stInfo->hHeap, 0, pNode); }
    }
    else
    {
        pNode->pData = pData;
        pNode->next = NULL;
        pNode->ePos = HST_VAR_POS_TIP;
        HAN_snprintf(pNode->pName, HAN_PLOT_STR_LEN_MAX, TEXT("%s"), pVarName);

        HANPPLOTDS pLast = stInfo->pVarData;
        if (NULL == pLast)
        {
            stInfo->pVarData = pNode;
        }
        else
        {
            while (NULL != pLast->next) { pLast = pLast->next; }
            pLast->next = pNode;
        }
    }

    return nRet;
}
static HANERROR HANScatterUpdateDataByNameCallback(HANPSCATTER stInfo, HANPCSTR pVarName, HANPCSTSETVALUE svData, EPLOTUPDATEDATA modeUpdate)
{
    HANERROR nRet = PLOT_VAR_NOT_EXIST;
    HANPPLOTDS pNode = stInfo->pVarData;
    HANSIZE nCount = 0;

    unsigned int ePos = (unsigned int)(-1);
    if ((HSTSV_SET_POINT == svData->nSetMode) || (HSTSV_SET_TWO_ARR == svData->nSetMode)) { ePos = HST_VAR_POS_POINT; }
    if (HSTSV_SET_TIP == svData->nSetMode) { ePos = HST_VAR_POS_TIP; }

    if ((stInfo->nDataLen < svData->pHData.nLen) || (stInfo->nDataLen < svData->pVData.nLen))
    {
        nRet = INDEX_OUT_OF_RANGE;
    }

    if (PLOT_VAR_NOT_EXIST == nRet)
    {
        while (NULL != pNode)
        {
            if ((NULL != pVarName) && (!HAN_strcmp(pNode->pName, pVarName)))
            {
                if (ePos != pNode->ePos) { nRet = HST_VAR_POS_ERR; }
                else
                {
                    switch (pNode->ePos) {
                        case HST_VAR_POS_POINT: {
                            switch (svData->nSetMode) {
                                case HSTSV_SET_TWO_ARR: {
                                    HANPLOTMAXMIN mmHMaxMin;
                                    HANPLOTMAXMIN mmVMaxMin;
                                    HANPPLOTDS pHNode = pNode;
                                    HANPPLOTDS pVNode = pNode->pChild;
                                    HANPLOTDATA pdHData = { .pData = pHNode->pData, .nLen = stInfo->nDataLen, };
                                    HANPLOTDATA pdVData = { .pData = pVNode->pData, .nLen = stInfo->nDataLen, };

                                    HANPlotUpdateData(&pdHData, &(svData->pHData), modeUpdate);
                                    HANPlotUpdateData(&pdVData, &(svData->pVData), modeUpdate);
                                    HANPlotGetDataMaxMin(&pdHData, &mmHMaxMin);
                                    HANPlotGetDataMaxMin(&pdVData, &mmVMaxMin);

                                    pHNode->nMax = mmHMaxMin.max;
                                    pHNode->nMin = mmHMaxMin.min;
                                    pVNode->nMax = mmVMaxMin.max;
                                    pVNode->nMin = mmVMaxMin.min;
                                } break;

                                default: {
                                    /* idle */
                                } break;
                            }
                            nRet = RET_OK;
                        } break;
                        case HST_VAR_POS_TIP: {
                            HANPLOTMAXMIN mmMaxMin;
                            HANPLOTDATA pdData = { .pData = pNode->pData, .nLen = stInfo->nDataLen, };

                            HANPlotUpdateData(&pdData, &(svData->pTipData), modeUpdate);
                            HANPlotGetDataMaxMin(&pdData, &mmMaxMin);

                            pNode->nMax = mmMaxMin.max;
                            pNode->nMin = mmMaxMin.min;
                            nRet = RET_OK;
                        } break;

                        default: {
                            /* idle */
                        } break;
                    }
                }
                break;
            }
            pNode = pNode->next;
            nCount++;
        }
    }

    return nRet;
}
static HANERROR HANScatterUpdateDataByIdCallback(HANPSCATTER stInfo, HANSIZE nId, HANPCSTSETVALUE svData, EPLOTUPDATEDATA modeUpdate)
{
    HANERROR nRet = PLOT_VAR_NOT_EXIST;
    HANPPLOTDS pNode = stInfo->pVarData;
    HANSIZE nCount = 0;
    unsigned int ePos = (unsigned int)(-1);

    if ((HSTSV_SET_POINT == svData->nSetMode) || (HSTSV_SET_TWO_ARR == svData->nSetMode)) { ePos = HST_VAR_POS_POINT; }
    if (HSTSV_SET_TIP == svData->nSetMode) { ePos = HST_VAR_POS_TIP; }
    
    if (stInfo->nDataLen < svData->pHData.nLen) { nRet = INDEX_OUT_OF_RANGE; }
    if (stInfo->nDataLen < svData->pVData.nLen) { nRet = INDEX_OUT_OF_RANGE; }

    while (NULL != pNode)
    {
        if (nId == nCount)
        {
            if (ePos != pNode->ePos) { nRet = HST_VAR_POS_ERR; }
            else
            {
                switch (pNode->ePos) {
                    case HST_VAR_POS_POINT: {
                        switch (svData->nSetMode) {
                            case HSTSV_SET_TWO_ARR: {
                                HANPLOTMAXMIN mmHMaxMin;
                                HANPLOTMAXMIN mmVMaxMin;
                                HANPPLOTDS pHNode = pNode;
                                HANPPLOTDS pVNode = pNode->pChild;
                                HANPLOTDATA pdHData = { .pData = pHNode->pData, .nLen = stInfo->nDataLen, };
                                HANPLOTDATA pdVData = { .pData = pVNode->pData, .nLen = stInfo->nDataLen, };

                                HANPlotUpdateData(&pdHData, &(svData->pHData), modeUpdate);
                                HANPlotUpdateData(&pdVData, &(svData->pVData), modeUpdate);
                                HANPlotGetDataMaxMin(&pdHData, &mmHMaxMin);
                                HANPlotGetDataMaxMin(&pdVData, &mmVMaxMin);

                                pHNode->nMax = mmHMaxMin.max;
                                pHNode->nMin = mmHMaxMin.min;
                                pVNode->nMax = mmVMaxMin.max;
                                pVNode->nMin = mmVMaxMin.min;
                            } break;

                            default: {
                                /* idle */
                            } break;
                        }
                        nRet = RET_OK;
                    } break;
                    case HST_VAR_POS_TIP: {
                        HANPLOTMAXMIN mmMaxMin;
                        HANPLOTDATA pdData = { .pData = pNode->pData, .nLen = stInfo->nDataLen, };

                        HANPlotUpdateData(&pdData, &(svData->pTipData), modeUpdate);
                        HANPlotGetDataMaxMin(&pdData, &mmMaxMin);

                        pNode->nMax = mmMaxMin.max;
                        pNode->nMin = mmMaxMin.min;
                        nRet = RET_OK;
                    } break;

                    default: {
                        /* idle */
                    } break;
                }
            }
            break;
        }
        pNode = pNode->next;
        nCount++;
    }

    return nRet;
}
static HANERROR HANScatterSetVarColorCallback(HANPSCATTER stInfo, HANPCSTR pVarName, HANSIZE nId, COLORREF rgb)
{
    HANERROR nRet = PLOT_VAR_NOT_EXIST;
    HANPPLOTDS pNode = stInfo->pVarData;
    HANSIZE nCount = 0;

    while (NULL != pNode)
    {
        if (((NULL != pVarName) && (!HAN_strcmp(pNode->pName, pVarName))) ||
            (nId == nCount))
        {
            nRet = RET_OK;
            HPEN hPen = CreatePen(PS_SOLID, stInfo->nLineWidth, rgb);
            HBRUSH hBrush = CreateSolidBrush(rgb);
            if (NULL == hPen) { nRet = CREATE_PEN_ERR; }
            if (NULL == hBrush) { nRet = CREATE_BRUSH_ERR; }
            if (RET_OK != nRet)
            {
                if (NULL != hPen) { DeleteObject(hPen); }
                if (NULL != hBrush) { DeleteObject(hBrush); }
            }
            else
            {
                if (NULL != pNode->hPen) { DeleteObject(pNode->hPen); }
                if (NULL != pNode->hBrush) { DeleteObject(pNode->hBrush); }
                pNode->crColor = rgb;
                pNode->hPen = hPen;
                pNode->hBrush = hBrush;
            }
            break;
        }
        pNode = pNode->next;
        nCount++;
    }

    return nRet;
}
static void HANScatterSetMaxMinCallback(HANPSCATTER stInfo, HANPCSTSETMAXMIN hvMaxMin, BOOL bUserMaxMin)
{
    if (TRUE == bUserMaxMin)
    {
        if (hvMaxMin->hMaxMin.max > hvMaxMin->hMaxMin.min)
        {
            stInfo->nHMax = hvMaxMin->hMaxMin.max;
            stInfo->nHMin = hvMaxMin->hMaxMin.min;
        }
        else
        {
            stInfo->nHMax = hvMaxMin->hMaxMin.min;
            stInfo->nHMin = hvMaxMin->hMaxMin.max;
        }

        if (hvMaxMin->vMaxMin.max > hvMaxMin->hMaxMin.min)
        {
            stInfo->nVMax = hvMaxMin->vMaxMin.max;
            stInfo->nVMin = hvMaxMin->vMaxMin.min;
        }
        else
        {
            stInfo->nVMax = hvMaxMin->vMaxMin.min;
            stInfo->nVMin = hvMaxMin->vMaxMin.max;
        }
    }
    stInfo->bMaxMin = bUserMaxMin;
}
static HANERROR HANScatterSetBackgroundColorCallback(HANPSCATTER stInfo, COLORREF rgb)
{
    HANERROR nRet = RET_OK;
    HBRUSH hBrush = CreateSolidBrush(rgb);

    if (NULL == hBrush) { nRet = CREATE_BRUSH_ERR; }
    else
    {
        if (NULL != stInfo->hBkBrush) { DeleteObject(stInfo->hBkBrush); }
        stInfo->crBkColor = rgb;
        stInfo->hBkBrush = hBrush;
    }

    return nRet;
}
static HANERROR HANScatterSetBackgroundBorderCallback(HANPSCATTER stInfo, COLORREF rgb)
{
    HANERROR nRet = RET_OK;
    HPEN hPen = CreatePen(PS_SOLID, 0, rgb);

    if (NULL == hPen) { nRet = CREATE_PEN_ERR; }
    else
    {
        if (NULL != stInfo->hBkBorder) { DeleteObject(stInfo->hBkBorder); }
        stInfo->crBkBorder = rgb;
        stInfo->hBkBorder = hPen;
    }

    return nRet;
}
static HANERROR HANScatterSetRectangleColorCallback(HANPSCATTER stInfo, COLORREF rgb)
{
    HANERROR nRet = RET_OK;
    COLORHSB crHSB;
    COLORREF crDark;
    COLORREF crMarkLine;
    HBRUSH hBrush = NULL;
    HBRUSH hDarkBrush = NULL;
    HPEN hMarkLine = NULL;
    
    PRGBToPHSB(&rgb, &crHSB);
    crHSB.kBrightness *= (float)(HST_DEF_RECT_DARK);
    PHSBToPRGB(&crHSB, &crDark);
    PRGBToPHSB(&rgb, &crHSB);
    crHSB.kBrightness *= (float)(HST_DEF_MARK_LINE_DARK);
    PHSBToPRGB(&crHSB, &crMarkLine);
    hBrush = CreateSolidBrush(rgb);
    hDarkBrush = CreateSolidBrush(crDark);
    hMarkLine = CreatePen(PS_SOLID, 0, crMarkLine);
    if (NULL == hBrush) { nRet = CREATE_BRUSH_ERR; }
    if (NULL == hDarkBrush) { nRet = CREATE_BRUSH_ERR; }
    if (NULL == hMarkLine) { nRet = CREATE_PEN_ERR; }
    if (RET_OK != nRet)
    {
        if (NULL != hBrush) { DeleteObject(hBrush); }
        if (NULL != hDarkBrush) { DeleteObject(hDarkBrush); }
        if (NULL != hMarkLine) { DeleteObject(hMarkLine); }
    }
    else
    {
        if (NULL != stInfo->hRcBrush) { DeleteObject(stInfo->hRcBrush); }
        if (NULL != stInfo->hRcDarBrush) { DeleteObject(stInfo->hRcDarBrush); }
        if (NULL != stInfo->hMarkLine) { DeleteObject(stInfo->hMarkLine); }
        stInfo->crRcColor = rgb;
        stInfo->hRcBrush = hBrush;
        stInfo->hRcDarBrush = hDarkBrush;
        stInfo->hMarkLine = hMarkLine;
    }
    
    return nRet;
}
static HANERROR HANScatterSetRectangleBorderCallback(HANPSCATTER stInfo, COLORREF rgb)
{
    HANERROR nRet = RET_OK;
    HPEN hPen = CreatePen(PS_SOLID, 0, rgb);

    if (NULL == hPen) { nRet = CREATE_PEN_ERR; }
    else
    {
        if (NULL != stInfo->hRcBorder) { DeleteObject(stInfo->hRcBorder); }
        stInfo->crRcBorder = rgb;
        stInfo->hRcBorder = hPen;
    }

    return nRet;
}
static HANERROR HANScatterSetTitleFontCallback(HANPSCATTER stInfo, const LOGFONT* pTitleFont)
{
    HANERROR nRet = RET_OK;
    HFONT hFont = CreateFontIndirect(pTitleFont);

    if (NULL == hFont) { nRet = CREATE_FONT_ERR; }
    else
    {
        if (NULL != stInfo->hTitleFont) { DeleteObject(stInfo->hTitleFont); }
        stInfo->lfTitleFont = *pTitleFont;
        stInfo->hTitleFont = hFont;
    }

    return nRet;
}
static HANERROR HANScatterSetTextFontCallback(HANPSCATTER stInfo, const LOGFONT* pTextFont)
{
    HANERROR nRet = RET_OK;
    HFONT hFont = CreateFontIndirect(pTextFont);

    if (NULL == hFont) { nRet = CREATE_FONT_ERR; }
    else
    {
        if (NULL != stInfo->hTextFont) { DeleteObject(stInfo->hTextFont); }
        stInfo->lfTextFont = *pTextFont;
        stInfo->hTextFont = hFont;
    }
    
    return nRet;
}
static void HANScatterSetRcRectangleCallback(HWND hScatter, HANPSCATTER stInfo, const RECT* rcRectangle, BOOL bAutoRect)
{
    stInfo->rcRectangle = *rcRectangle;
    if (TRUE == bAutoRect)
    {
        RECT rcWin;
        GetClientRect(hScatter, &rcWin);
        stInfo->rcRectangle.right = rcWin.right - rcRectangle->right;
        stInfo->rcRectangle.bottom = rcWin.bottom - rcRectangle->bottom;
        stInfo->rcAutoRectDis = *rcRectangle;
    }
    stInfo->lVRectangleW = GetRectW(&(stInfo->rcRectangle));
    stInfo->lVRectangleH = GetRectH(&(stInfo->rcRectangle));
    stInfo->lRectangleX = 0;
    stInfo->lRectangleY = 0;
    stInfo->bAutoRect = bAutoRect;
}
static BOOL HANScatterGetRcRectangleCallback(HANPSCATTER stInfo, RECT* rcRectangle)
{
    *rcRectangle = stInfo->rcRectangle;
    return stInfo->bAutoRect;
}
static void HANScatterSetHValueCountCallback(HANPSCATTER stInfo, HANSIZE n)
{
    if (n < (HANSIZE)2) { stInfo->nHValueCount = 2; }
    else { stInfo->nHValueCount = n; }
}
static void HANScatterSetVValueCountCallback(HANPSCATTER stInfo, HANSIZE n)
{
    if (n < (HANSIZE)2) { stInfo->nVValueCount = 2; }
    else { stInfo->nVValueCount = n; }
}
static HANERROR HANScatterSetLineSizeCallback(HANPSCATTER stInfo, HANINT nWidth)
{
    HANINT nOldWidth = stInfo->nLineWidth;
    stInfo->nLineWidth = nWidth;
    HANERROR nRet = HANScatterUpdateResources(stInfo);
    if (RET_OK != nRet) { stInfo->nLineWidth = nOldWidth; }
    return nRet;
}
static void HANScatterSetPointSizeCallback(HANPSCATTER stInfo, HANINT nSize)
{
    stInfo->nPointSize = nSize;
}
static HANERROR HANScatterSetPointStyleCallback(HANPSCATTER stInfo, HANPCSTR pVarName, HANSIZE nId, HANHPOINT hPoint)
{
    HANERROR nRet = PLOT_VAR_NOT_EXIST;
    HANPPLOTDS pNode = stInfo->pVarData;
    HANSIZE nCount = 0;

    while (NULL != pNode)
    {
        if (((NULL != pVarName) && (!HAN_strcmp(pNode->pName, pVarName))) || (nId == nCount))
        {
            pNode->DrawPoint = hPoint;
            nRet = RET_OK;
            break;
        }
        pNode = pNode->next;
        nCount++;
    }
    return nRet;
}
static void HANScatterCutDataCallback(HANPSCATTER stInfo, HANPCSIZE pPos, BOOL bCut)
{
    if (TRUE == bCut)
    {
        if (pPos[0] < pPos[1])
        {
            stInfo->nCutBegin = pPos[0];
            stInfo->nCutEnd = pPos[1];
        }
        else
        {
            stInfo->nCutBegin = pPos[1];
            stInfo->nCutEnd = pPos[0];
        }
    }
    stInfo->bCut = bCut;
}
static void HANScatterSetTipWCallback(HANPSCATTER stInfo, HANSIZE w)
{
    stInfo->nTipW = w;
}
static HANERROR HANScatterSetTipFontCallback(HANPSCATTER stInfo, const LOGFONT* pTipFont)
{
    HANERROR nRet = RET_OK;
    HFONT hFont = CreateFontIndirect(pTipFont);

    if (NULL == hFont) { nRet = CREATE_FONT_ERR; }
    else
    {
        if (NULL != stInfo->hTipFont) { DeleteObject(stInfo->hTipFont); }
        stInfo->lfTipFont = *pTipFont;
        stInfo->hTipFont = hFont;
    }
    
    return nRet;
}
static HANERROR HANScatterSetTipBkColorCallback(HANPSCATTER stInfo, COLORREF rgb)
{
    HANERROR nRet = RET_OK;
    HBRUSH hBrush = CreateSolidBrush(rgb);
    HPEN hPen = CreatePen(PS_SOLID, 0, rgb);

    if (NULL == hBrush) { nRet = CREATE_BRUSH_ERR; }
    if (NULL == hPen) { nRet = CREATE_PEN_ERR; }
    if (RET_OK != nRet)
    {
        if (NULL != hBrush) { DeleteObject(hBrush); }
        if (NULL != hPen) { DeleteObject(hPen); }
    }
    else
    {
        if (NULL != stInfo->hTipBkBrush) { DeleteObject(stInfo->hTipBkBrush); }
        if (NULL != stInfo->hTipBkPen) { DeleteObject(stInfo->hTipBkPen); }
        stInfo->hTipBkBrush = hBrush;
        stInfo->hTipBkPen = hPen;
    }
    
    return RET_OK;
}
static HANERROR HANScatterSetTipBorderCallback(HANPSCATTER stInfo, COLORREF rgb)
{
    HANERROR nRet = RET_OK;
    HPEN hPen = CreatePen(PS_SOLID, 0, rgb);

    if (NULL == hPen) { nRet = CREATE_PEN_ERR; }
    if (RET_OK == nRet)
    {
        if (NULL != stInfo->hTipBorder) { DeleteObject(stInfo->hTipBorder); }
        stInfo->hTipBorder = hPen;
    }

    return nRet;
}
static void HANScatterSetMarkDisCallback(HANPSCATTER stInfo, HANPCPLOTNUM nMarkDis, BOOL bMarkDis)
{
    if (TRUE == bMarkDis) { stInfo->nMarkDis = *nMarkDis; }
    stInfo->bMarkDis = bMarkDis;
}
#endif

#endif
