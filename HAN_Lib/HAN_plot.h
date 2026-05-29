#ifndef HAN_PLOT_H
#define HAN_PLOT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAN_def.h"
#include "HAN_windows.h"

#if 1 /******************** HANPlot ********************/
typedef double HANPLOTNUM, * HANPPLOTNUM;
typedef const double* HANPCPLOTNUM;

typedef struct tagHANPLOTDATA {
    HANPPLOTNUM pData;
    HANSIZE     nLen;
} HANPLOTDATA, * HANPPLOTDATA;
typedef const HANPLOTDATA* HANPCPLOTDATA;

/* 浮点型 POINT */
typedef struct tagHANFPOINT {
    HANPLOTNUM  x;
    HANPLOTNUM  y;
} HANFPOINT, * HANPFPOINT;
typedef const HANFPOINT* HANPCFPOINT;

/* 最值结构 */
typedef struct tagHANPLOTMAXMIN {
    HANPLOTNUM  max;
    HANPLOTNUM  min;
} HANPLOTMAXMIN, * HANPPLOTMAXMIN;
typedef const HANPLOTMAXMIN* HANPCPLOTMAXMIN;

/* 自定义绘制点的参数 */
typedef struct tagHANPLOTDRAWPOINTPARAM {
    HDC             hdc;        /* 设备环境 */
    const POINT*    ptPoint;    /* 需要绘制的点数组 */
    HANSIZE         nPointLen;  /* ptPoint 数组的长度 */
    int             nSize;      /* 用户定义的点的尺寸 */
} HANPLOTDRAWPOINTPARAM, * HANPPLOTDRAWPOINTPARAM;
typedef const HANPLOTDRAWPOINTPARAM* HANPCPLOTDRAWPOINTPARAM;

/* 绘制点的句柄 */
typedef void (*HANHPOINT)(HANPCPLOTDRAWPOINTPARAM pParam);

/* HAN_plot 自带的点风格 */
typedef enum {
    HAN_PLOT_DRAW_POINT_CIRCLE,
    HAN_PLOT_DRAW_POINT_SQUARE,
    HAN_PLOT_DRAW_POINT_TRIANGLE,
    HAN_PLOT_DRAW_POINT_STAR,
} HANPLOTDRAWPOINTSTYLE;
#define HAN_PLOT_DEF_DRAW_POINT HAN_PLOT_DRAW_POINT_SQUARE

typedef enum { // 更新数据方式
    HAN_PLOT_UPDATE_DATA_SET_START,
    HAN_PLOT_UPDATE_DATA_PUSH_START,
    HAN_PLOT_UPDATE_DATA_SET_END,
    HAN_PLOT_UPDATE_DATA_PUSH_END,
} EPLOTUPDATEDATA;

/* 点风格结构 */
typedef struct tagHANPLOTPOINTSTYLE {
    HANHPOINT   nPointStyle;
    int         nPointSize;
} HANPLOTPOINTSTYLE, * HANPPLOTPOINTSTYLE;
typedef const HANPLOTPOINTSTYLE* HANPCPLOTPOINTSTYLE;

#define HANPlotNumAbs(n)                fabs(n)

#if 1 // 默认参数，这些参数不对外开放，仅在创建窗口时会参考
#define HAN_PLOT_STR_LEN_MAX    32  // 所有字符串的长度上限，含 '\0'
#define HAN_PLOT_DEF_TEXT_FACE  TEXT("宋体")

#define HAN_PLOT_DRAW_POINT_BUF_LEN     (HANSIZE)(3)

/* 获取系统自带的点句柄 */
HANHPOINT HANPlotGetStockPoint(HANPLOTDRAWPOINTSTYLE nPointStyle);

#endif

#if 1 /* HANBarChart 条形图
 */
#define HBC_CLASS               TEXT("HANBarChart") // 窗口类名

#if 1 // 默认参数，这些参数不对外开放，仅在创建窗口时会参考
#define HBC_DEF_TITLE_W         16  // 标题字宽
#define HBC_DEF_TITLE_H         32  // 标题字高
#define HBC_DEF_TEXT_W          8   // 文本字宽
#define HBC_DEF_TEXT_H          16  // 文本字高
#define HBC_DEF_RECT_LEFT       80  // 绘制区域到窗口左边界的距离
#define HBC_DEF_RECT_TOP        100 // 绘制区域到窗口上边界的距离
#define HBC_DEF_RECT_RIGHT      100 // 绘制区域到窗口右边界的距离
#define HBC_DEF_RECT_BOTTOM     50  // 绘制区域到窗口下边界的距离
#define HBC_DEF_BAR_WIDTH       20  // 条宽
#define HBC_DEF_BAR_DX          5   // 同组中的条间距
#define HBC_DEF_VVALUE_COUNT    5   // 垂直方向数值个数
#endif

#if 1 // 窗口风格
#define HBCS_2D                 0x00000000  // 2D，默认此风格
#define HBCS_3D                 0x00000001  // 3D
#define HBCS_CLUSTERED          0x00000000  // 簇状条形图，默认此风格
#define HBCS_STACKED            0x00000002  // 堆叠条形图
#define HBCS_NOTPERCENTAGE      0x00000000  // 非百分比，默认此风格
#define HBCS_PERCENTAGE         0x00000004  // 百分比
#define HBCS_ENABLETIP          0x00000008  // 使能鼠标悬停数值提示
#define HBCS_MARKLINE           0x00000010  // 显示标线
#endif

#if 1 // 窗口消息
typedef enum {
    HBCM_NOTUSED = WM_USER,     // 不使用的消息值，新消息请在下方添加
    HBCM_SETDATALEN,            // 设置数组形状（WPARAM：行数，即数据组数，LPARAM：列数，即每组数据的元素个数）
    HBCM_ADDVAR,                // 添加变量（WPARAM：未使用，LPARAM：变量名字符串）
    HBCM_SETGROUPNAME,          // 设置组名（WPARAM：起始为 0 的组号，不接收负数，LPARAM：组名字符串）
    HBCM_SETVALUEBYNAME,        // 导入数据（WPARAM：变量名字符串，LPARAM：数据数组地址）
    HBCM_SETVALUEBYID,          // 导入数据（WPARAM：起始为 0 的下标值，不接收负数，LPARAM：数据数组地址）
    HBCM_SETBKCOLOR,            // 设置窗口的背景色（WPARAM：COLORREF 颜色，LPARAM：未使用）
    HBCM_SETBKBORDER,           // 设置窗口边框颜色（WPARAM：COLORREF 颜色，LPARAM：未使用）
    HBCM_SETTITLEFONT,          // 设置标题字体（WPARAM：int size[] = { w, h }，LPARAM：字体名字符串）
    HBCM_SETTEXTFONT,           // 设置文本字体（WPARAM：int size[] = { w, h }，LPARAM：字体名字符串）
    HBCM_SETRECT,               // 设置绘图区域大小（WPARAM：BOOL 自动大小模式，LPARAM：RECT*）
    HBCM_GETRECT,               // 获取绘图区域大小（WPARAM：未使用，LPARAM：RECT*，return：BOOL 自动大小模式）
    HBCM_SETBARWIDTH,           // 设置条形图宽度（WPARAM：宽度，LPARAM：未使用）
    HBCM_SETBARINTERVAL,        // 设置条形图间隔（WPARAM：间隔，LPARAM：未使用）
} HANBARCHARTMESSAGE;
#endif

/* 注册条形图类，应用程序在使用此控件前必须先调用该函数注册窗口类
 */
void RegisterHANBarChart(HINSTANCE hInstance);
/* 绘制条形图，该函数封装了一些必要的操作，使用前必须先调用一次 RegisterHANBarChart 函数数，失败返回 NULL
 */
HWND CreateHANBarChart(
    HANPCSTR pWindowName,
    DWORD dwStyle,
    int x,
    int y,
    int w,
    int h,
    HWND hParent,               // 同 CreateWindow 对应参数
    HMENU hMenu,                // 同 CreateWindow 对应参数
    HINSTANCE hInstance,        // 同 CreateWindow 对应参数
    HANSIZE nVarCnt,            // 变量个数
    HANSIZE nDataLen,           // 每组变量存放的数据个数
    const HANPSTR* pVarName,    // 变量名，仅接收指针数组，每个指针指向一个字符串
    const HANPSTR* pGroupName,  // 组名，仅接收指针数组，每个指针指向一个字符串
    const HANPPLOTNUM* pData    // 数据，仅接收指针数组，每个指针指向一个 HANPLOTNUM 型数组用来存放数据
);
/* 设置数据长度
 * nDataLen     每个变量的数据长度（新创建的窗口未执行此函数长度默认为 0 ，所以要导入数据必须先执行此函数）
 */
HANERROR HANBarChartSetDataLen(HWND hBarChart, HANSIZE nDataLen);
/* 添加变量
 * pVarName     新变量的名字
 */
HANERROR HANBarChartAddVar(HWND hBarChart, HANPCSTR pVarName);
/* 设置组名
 * nGroupCnt    目标组号，从 0 开始
 * pName        需要设置的名字
 */
void HANBarChartSetGroupName(HWND hBarChart, HANSIZE nGroupCnt, HANPCSTR pName, BOOL bRedraw);
/* 导入数据，需要提供变量 id
 * nId          变量 id
 * pData        数据（数组长度必须与 HANBarChartSetDataLen 设置的一致）
 */
HANERROR HANBarChartSetValueById(HWND hBarChart, HANSIZE nId, HANPCPLOTNUM pData);
/* 设置窗口的背景色
 */
HANERROR HANBarChartSetBackgroundColor(HWND hBarChart, COLORREF rgb, BOOL bRedraw);
/* 设置窗口边框颜色
 */
HANERROR HANBarChartSetBkBorder(HWND hBarChart, COLORREF rgb, BOOL bRedraw);
/* 设置标题字体（一般 h / w = 2 / 1 即是正常比例）
 * w            字宽（正数即可）
 * h            字高（正数即可）
 * pFace        字体名
 */
HANERROR HANBarChartSetTitleFont(HWND hBarChart, int w, int h, HANPCSTR pFace, BOOL bRedraw);
/* 设置文本字体（一般 h / w = 2 / 1 即是正常比例）
 * w            字宽（正数即可）
 * h            字高（正数即可）
 * pFace        字体名
 */
HANERROR HANBarChartSetTextFont(HWND hBarChart, int w, int h, HANPCSTR pFace, BOOL bRedraw);
/* 设置绘图区域大小
 * rcRectangle  绘图区的矩形范围
 * bAutoRect    自动大小（TRUE：窗口变化时绘图区会跟着变，此时 rcRectangle 参数代表绘图区到窗口的距离，FALSE：绘图区固定不变，大小就是 rcRectangle ）
 */
void HANBarChartSetRcRectangle(HWND hBarChart, const RECT* rcRectangle, BOOL bAutoRect, BOOL bRedraw);
/* 获取绘图区域大小（获取的始终是绝对尺寸）
 * rcRectangle  绘图区的矩形范围
 * return       返回的是绘图区的易变属性，TRUE：绘图会随窗口变化而变化，FALSE：绘图区大小固定不变
 */
BOOL HANBarChartGetRcRectangle(HWND hBarChart, RECT* rcRectangle);
/* 设置条形图宽度
 * w            宽度
 */
void HANBarChartSetBarWidth(HWND hBarChart, int w, BOOL bRedraw);
/* 设置条形图间隔
 * nInterval    间隔
 */
void HANBarChartSetBarInterval(HWND hBarChart, int nInterval, BOOL bRedraw);

#endif

#if 1 /* HANPolyLine 折线图
 * 必要操作：
 * 1. 调用 RegisterHANPolyLine() 注册窗口类
 * 2. 调用 CreateWindow(TEXT("HANPolyLine"), ...) 创建窗口，省略号内的参数参考 CreateWindow 的标准用法
 * 3. 调用 HANPolyLineSetDataLen 设置每个变量的数组长度，
 * 4. 调用 HANPolyLineAddVar 向容口添加变量，变量名可以是长度为 0 的字符串
 *    步骤 3 和 4 原则上不分先后，可以先添加变量再设置变量长度
 * 5. 调用 HANPolyLineSetValueByName 或 HANPolyLineSetValueById 给变量导入数据
 *    前者以变量名为依据，后者以添加变量时的先后顺序为依据
 *    导入数据所用的数组长度必须与 HANPolyLineSetDataLen 所设置的长度一致
 * 6. 绘制折线图的必要步骤就是这些了
 */
#define HPL_CLASS               TEXT("HANPolyLine") // 窗口类名

#if 1 // 默认参数，这些参数不对外开放，仅在创建窗口时会参考
#define HPL_DEF_TIMER_T         50  // 定时器周期，单位 ms（出于效率考虑，窗口不会对所有操作都立即做出重绘操作，而是定时器触发生重绘）
#define HPL_DEF_TITLE_W         16  // 标题字宽
#define HPL_DEF_TITLE_H         32  // 标题字高
#define HPL_DEF_TEXT_W          8   // 文本字宽
#define HPL_DEF_TEXT_H          16  // 文本字高
#define HPL_DEF_RECT_LEFT       80  // 绘制区域到窗口左边界的距离
#define HPL_DEF_RECT_TOP        100 // 绘制区域到窗口上边界的距离
#define HPL_DEF_RECT_RIGHT      100 // 绘制区域到窗口右边界的距离
#define HPL_DEF_RECT_BOTTOM     50  // 绘制区域到窗口下边界的距离
#define HPL_DEF_RECT_DARK       0.9 // 绘制区域暗色背景的系数
#define HPL_DEF_MARK_LINE_DARK  0.8 // 标线暗色系数
#define HPL_DEF_HVALUE_COUNT    6   // 水平方向数值个数
#define HPL_DEF_VVALUE_COUNT    5   // 垂直方向数值个数
#define HPL_DEF_REDUCE_SIZE     0.1 // 缩小按钮按下后画面缩小的尺寸（该值为新绘制的数据长度与数据总长的比值，若该值为 0.1 ，则最多缩小 10 次即可还原）
#define HPL_DEF_RC_INV_HK       0.1 // 绘图区域到上下留白的高度（单边高度，所以该值必须小于 0.5 ）占绘图区总高度的比值
#define HPL_DEF_TIP_DIS         5   // 鼠标悬停显示数值的最大像素距离
#endif

#if 1 // 窗口风格，可用或运算组合
#define HPLS_LINE               0x00000001  // 画线，可以和 HPLS_POINT 并用绘制点线
#define HPLS_POINT              0x00000002  // 画点，可以和 HPLS_LINE 并用绘制点线
#define HPLS_ENABLEMOUSE        0x00000004  // 允许鼠标对绘图区域进行放大、拖动等操作
#define HPLS_ENABLETIP          0x00000008  // 使能鼠标悬停数值提示
#define HPLS_MARKLINE           0x00000010  // 显示标线
#endif

#if 1 // 窗口消息
typedef enum {
    HPLM_NOTUSED = WM_USER,     // 不使用的消息值，新消息请在下方添加
    HPLM_SETDATALEN,            // 设置数据长度（WPARAM：数据长度，LPARAM：未使用）
    HPLM_ADDVAR,                // 添加变量（WPARAM：要添加到的位置，LPARAM：变量名字符串）
    HPLM_SETSTARTDATABYNAME,    // 根据变量名在头部设置数据（WPARAM：变量名字符串，LPARAM：HANPCPLOTDATA 数据）
    HPLM_PUSHSTARTDATABYNAME,   // 根据变量名在头部添加数据，移除最后的数据（WPARAM：变量名字符串，LPARAM：HANPCPLOTDATA 数据）
    HPLM_SETENDDATABYNAME,      // 根据变量名在尾部设置数据（WPARAM：变量名字符串，LPARAM：HANPCPLOTDATA 数据）
    HPLM_PUSHENDDATABYNAME,     // 根据变量名在尾部添加数据，移除最早的数据（WPARAM：变量名字符串，LPARAM：HANPCPLOTDATA 数据）
    HPLM_SETSTARTDATABYID,      // 根据变量ID在头部设置数据（WPARAM：长度，LPARAM：HANPCPLOTDATA 数据）
    HPLM_PUSHSTARTDATABYID,     // 根据变量ID在头部添加数据，移除最后的数据（WPARAM：长度，LPARAM：HANPCPLOTDATA 数据）
    HPLM_SETENDDATABYID,        // 根据变量ID在尾部设置数据（WPARAM：长度，LPARAM：HANPCPLOTDATA 数据）
    HPLM_PUSHENDDATABYID,       // 根据变量ID在尾部添加数据，移除最早的数据（WPARAM：长度，LPARAM：HANPCPLOTDATA 数据）
    HPLM_SETVARCOLORBYNAME,     // 设置变量颜色（WPARAM：变量名字符串，LPARAM：颜色）
    HPLM_SETVARCOLORBYID,       // 设置变量颜色（WPARAM：起始为 0 的下标值，不接收负数，LPARAM：颜色）
    HPLM_SETLEFTMAXMIN,         // 设置左侧最值（WPARAM：BOOL 使能 / 禁用设置用户最值，LPARAM：最值数组地址）
    HPLM_SETRIGHTMAXMIN,        // 设置右侧最值（WPARAM：BOOL 使能 / 禁用设置用户最值，LPARAM：最值数组地址）
    HPLM_SETBKCOLOR,            // 设置窗口的背景色（WPARAM：COLORREF 颜色，LPARAM：未使用）
    HPLM_SETBKBORDER,           // 设置窗口边框颜色（WPARAM：COLORREF 颜色，LPARAM：未使用）
    HPLM_SETRECTCOLOR,          // 设置绘图区域的背景色（WPARAM：COLORREF 颜色，LPARAM：未使用）
    HPLM_SETRECTBORDER,         // 设置绘图区域边框颜色（WPARAM：COLORREF 颜色，LPARAM：未使用）
    HPLM_SETTITLEFONT,          // 设置标题字体（WPARAM：int size[] = { w, h }，LPARAM：字体名字符串）
    HPLM_SETTEXTFONT,           // 设置文本字体（WPARAM：int size[] = { w, h }，LPARAM：字体名字符串）
    HPLM_SETRECT,               // 设置绘图区域大小（WPARAM：BOOL 自动大小模式，LPARAM：RECT*）
    HPLM_GETRECT,               // 获取绘图区域大小（WPARAM：未使用，LPARAM：RECT*，return：BOOL 自动大小模式）
    HPLM_SETRCINVH,             // 设置绘图区留白高度（WPARAM：高度，LPARAM：未使用）
    HPLM_SETHVALCOUNT,          // 设置水平方向数值个数（WPARAM：数值个数，LPARAM：未使用）
    HPLM_SETVVALCOUNT,          // 设置垂直方向数值个数（WPARAM：数值个数，LPARAM：未使用）
    HPLM_ENABLEMOUSE,           // 使能鼠标功能（WPARAM：BOOL bEnable，LPARAM：未使用）
    HPLM_SETDATAMODE,           // 设置数据绘制模式（WPARAM：EHPLDATAMODE 模式，LPARAM：未使用）
    HPLM_SETPOINTLINESIZE,      // 设置线条风格（WPARAM：HANSTPLSIZE 要设置的对象，LPARAM：对象尺寸）
    HPLM_SETPOINTSTYLEBYNAME,   // 设置点风格（WPARAM：变量名字符串，LPARAM：HANHPOINT 点句柄）
    HPLM_SETPOINTSTYLEBYID,     // 设置点风格（WPARAM：起始为 0 的下标值，不接收负数，LPARAM：HANHPOINT 点句柄）
    HPLM_DRAWPOINTLINE,         // 绘制点线（WPARAM：BOOL 绘制点，LPARAM：BOOL 绘制线）
    HPLM_CUTDATA,               // 裁剪数据（WPARAM：BOOL 是否裁剪，LPARAM：HANPCSIZE 裁剪范围）
    HPLM_SETTIPW,               // 设置鼠标悬停数值提示框宽度（WPARAM：HANSIZE w，LPARAM：未使用）
    HPLM_SETTIPFONT,            // 设置鼠标悬停数值提示字体（WPARAM：int size[] = { w, h }，LPARAM：字体名字符串）
    HPLM_SETTIPBKCOLOR,         // 设置鼠标悬停数值提示背景颜色（WPARAM：COLORREF 颜色，LPARAM：未使用）
    HPLM_SETTIPBORDER,          // 设置鼠标悬停数值提示边框颜色（WPARAM：COLORREF 颜色，LPARAM：未使用）
} HANPOLYLINEMESSAGE;
#endif

typedef enum { // 变量在窗口中的位置
    HPL_VAR_POS_LEFT,
    HPL_VAR_POS_TOP,
    HPL_VAR_POS_RIGHT,
    HPL_VAR_POS_BOTTOM,
} EHPLVARPOS, * EHPLPVARPOS;
typedef enum HAN_polyline_data_mode_enum { // 绘图模式
    HPL_DM_FAST,            // 快速绘图，该模式下会跳过一些数据点，只绘制画布上每列像素点的典型值
    HPL_DM_HIGH_ACCURATE,   // 高精度绘图，该模式下会将所有数据绘制到画布上
} EHPLDATAMODE, * EHPLPDATAMODE;
typedef enum HAN_polyline_line_style_enum { // 折线风格，可以同时使能两种风格
    HPL_PLS_LINE,   // 折线
    HPL_PLS_POINT,  // 点
} EHPLPLSTYLE, * EHPLPPLSTYLE;

/* 注册折线图类，应用程序在使用此控件前必须先调用该函数注册窗口类
 */
void RegisterHANPolyLine(HINSTANCE hInstance);
/* 设置数据长度
 * nDataLen     每个变量的数据长度（新创建的窗口未执行此函数长度默认为 0 ，所以要导入数据必须先执行此函数）
 */
HANERROR HANPolyLineSetDataLen(HWND hPolyLine, HANSIZE nDataLen);
/* 添加变量
 * pVarName     新变量的名字
 * ePos         变量对应的数值在绘图区的位置
 */
HANERROR HANPolyLineAddVar(HWND hPolyLine, HANPCSTR pVarName, EHPLVARPOS ePos);
/* 根据变量名在头部设置数据
 * pVarName     变量名
 * pData        数据（长度必须与 HANPolyLineSetDataLen 设置的一致）
 */
HANERROR HANPolyLineSetStartDataByName(HWND hPolyLine, HANPCSTR pVarName, HANPCPLOTDATA pData);
/* 根据变量名在头部添加数据，移除最后的数据
 * pVarName     变量名
 * pData        数据（长度必须与 HANPolyLineSetDataLen 设置的一致）
 */
HANERROR HANPolyLinePushStartDataByName(HWND hPolyLine, HANPCSTR pVarName, HANPCPLOTDATA pData);
/* 根据变量名在尾部设置数据
 * pVarName     变量名
 * pData        数据（长度必须与 HANPolyLineSetDataLen 设置的一致）
 */
HANERROR HANPolyLineSetEndDataByName(HWND hPolyLine, HANPCSTR pVarName, HANPCPLOTDATA pData);
/* 根据变量名在尾部添加数据，移除最早的数据
 * pVarName     变量名
 * pData        数据（长度必须小于等于 HANPolyLineSetDataLen 设置的值）
 */
HANERROR HANPolyLinePushEndDataByName(HWND hPolyLine, HANPCSTR pVarName, HANPCPLOTDATA pData);
/* 根据变量ID在头部设置数据
 * nId          变量 ID
 * pData        数据（长度必须与 HANPolyLineSetDataLen 设置的一致）
 */
HANERROR HANPolyLineSetStartDataById(HWND hPolyLine, HANSIZE nId, HANPCPLOTDATA pData);
/* 根据变量ID在头部添加数据，移除最后的数据
 * nId          变量 ID
 * pData        数据（长度必须与 HANPolyLineSetDataLen 设置的一致）
 */
HANERROR HANPolyLinePushStartDataById(HWND hPolyLine, HANSIZE nId, HANPCPLOTDATA pData);
/* 根据变量ID在尾部设置数据
 * nId          变量 ID
 * pData        数据（长度必须与 HANPolyLineSetDataLen 设置的一致）
 */
HANERROR HANPolyLineSetEndDataById(HWND hPolyLine, HANSIZE nId, HANPCPLOTDATA pData);
/* 根据变量ID在尾部添加数据，移除最早的数据
 * nId          变量 ID
 * pData        数据（长度必须小于等于 HANPolyLineSetDataLen 设置的值）
 */
HANERROR HANPolyLinePushEndDataById(HWND hPolyLine, HANSIZE nId, HANPCPLOTDATA pData);
/* 修改变量颜色，需要提供变量名
 * pVarName     变量名
 * crColor      颜色
 */
HANERROR HANPolyLineSetVarColorByName(HWND hPolyLine, HANPCSTR pVarName, COLORREF rgb, BOOL bRedraw);
/* 修改变量颜色，需要提供变 id
 * nId          变量 id
 * crColor      颜色
 */
HANERROR HANPolyLineSetVarColorById(HWND hPolyLine, HANSIZE nId, COLORREF rgb, BOOL bRedraw);
/* 手动设置左侧最大最小值，设置后会强制按此最值绘图，若不设置，则自动计算最值
 * nMaxMin      最值数组地址，大小先后顺序无所谓，程序会自动识别
 * bUserMaxMin  使能 / 禁用最值（TRUE：绘图时会强制使用用户设置的最值，设置的留白空间会失效，
 *                              FALSE：关闭设置的最值，nMaxMin 忽略，绘图时根据数据自动计算最值，会有留白）
 */
void HANPolyLineSetLeftMaxMin(HWND hPolyLine, HANPCPLOTMAXMIN lMaxMin, BOOL bUserMaxMin, BOOL bRedraw);
/* 手动设置右侧最大最小值，设置后会强制按此最值绘图，若不设置，则自动计算最值
 * nMaxMin      最值数组地址，大小先后顺序无所谓，程序会自动识别
 * bUserMaxMin  使能 / 禁用最值（TRUE：绘图时会强制使用用户设置的最值，设置的留白空间会失效，
 *                              FALSE：关闭设置的最值，nMaxMin 忽略，绘图时根据数据自动计算最值，会有留白）
 */
void HANPolyLineSetRightMaxMin(HWND hPolyLine, HANPCPLOTMAXMIN rMaxMin, BOOL bUserMaxMin, BOOL bRedraw);
/* 设置窗口的背景色
 */
HANERROR HANPolyLineSetBackgroundColor(HWND hPolyLine, COLORREF rgb, BOOL bRedraw);
/* 设置窗口边框颜色
 */
HANERROR HANPolyLineSetBkBorder(HWND hPolyLine, COLORREF rgb, BOOL bRedraw);
/* 设置绘图区域的背景色
 */
HANERROR HANPolyLineSetRectangleColor(HWND hPolyLine, COLORREF rgb, BOOL bRedraw);
/* 设置绘图区域边框颜色
 */
HANERROR HANPolyLineSetRectBorder(HWND hPolyLine, COLORREF rgb, BOOL bRedraw);
/* 设置标题字体（一般 h / w = 2 / 1 即是正常比例）
 * w            字宽（正数即可）
 * h            字高（正数即可）
 * pFace        字体名
 */
HANERROR HANPolyLineSetTitleFont(HWND hPolyLine, int w, int h, HANPCSTR pFace, BOOL bRedraw);
/* 设置文本字体（一般 h / w = 2 / 1 即是正常比例）
 * w            字宽（正数即可）
 * h            字高（正数即可）
 * pFace        字体名
 */
HANERROR HANPolyLineSetTextFont(HWND hPolyLine, int w, int h, HANPCSTR pFace, BOOL bRedraw);
/* 设置绘图区域大小
 * rcRectangle  绘图区的矩形范围
 * bAutoRect    自动大小（TRUE：窗口变化时绘图区会跟着变，此时 rcRectangle 参数代表绘图区到窗口的距离，FALSE：绘图区固定不变，大小就是 rcRectangle ）
 */
void HANPolyLineSetRcRectangle(HWND hPolyLine, const RECT* rcRectangle, BOOL bAutoRect, BOOL bRedraw);
/* 获取绘图区域大小（获取的始终是绝对尺寸）
 * rcRectangle  绘图区的矩形范围
 * return       返回的是绘图区的易变属性，TRUE：绘图会随窗口变化而变化，FALSE：绘图区大小固定不变
 */
BOOL HANPolyLineGetRcRectangle(HWND hPolyLine, RECT* rcRectangle);
/* 设置绘图区上下留白的高度
 * nHeight      留白高度，该值必须小于 HANPolyLineSetRcRectangle 所设置的区域高度的一半
 */
HANERROR HANPolyLineSetRcInvH(HWND hPolyLine, LONG nHeight, BOOL bRedraw);
/* 设置水平方向数值个数
 */
void HANPolyLineSetHValueCount(HWND hPolyLine, unsigned int n, BOOL bRedraw);
/* 设置垂直方向数值个数
 */
void HANPolyLineSetVValueCount(HWND hPolyLine, unsigned int n, BOOL bRedraw);
/* 使能鼠标功能
 * bEnable      启用鼠标功能（TRUE 为使能，FALSE 为禁用，若禁用，还会隐藏按钮）
 */
void HANPolyLineEnableMouse(HWND hPolyLine, BOOL bEnable);
/* 设置数据绘制模式
 * dmDataMode   数据绘制模式（HPL_DM_FAST：快速模式，该模式下若数据过长会跳过部分数据。HPL_DM_HIGH_ACCURATE：高精模式，绘制所有数据）
 */
void HANPolyLineSetDataMode(HWND hPolyLine, EHPLDATAMODE dmDataMode, BOOL bRedraw);
/* 设置点线风格
 * eLineStyle   线条风格（HPL_PLS_LINE：线。HPL_PLS_POINT：点）
 * nObjSize     对象尺寸（eLineStyle = HPL_PLS_LINE 时，该参数代表线宽。eLineStyle = HPL_PLS_POINT 时，该参数代表点的直径）
 */
HANERROR HANPolyLineSetPointLineSize(HWND hPolyLine, EHPLPLSTYLE eLineStyle, int nObjSize, BOOL bRedraw);
/* 根据变量名设置点风格
 * pVarName     变量名
 * hPoint       点句柄（可以通过 HANPlotGetStockPoint 函数获取系统自带的句柄）
 */
HANERROR HANPolyLineSetPointStyleByName(HWND hPolyLine, HANPCSTR pVarName, HANHPOINT hPoint, BOOL bRedraw);
/* 根据变量 id 设置点风格
 * nId          变量 id
 * hPoint       点句柄（可以通过 HANPlotGetStockPoint 函数获取系统自带的句柄）
 */
HANERROR HANPolyLineSetPointStyleById(HWND hPolyLine, HANSIZE nId, HANHPOINT hPoint, BOOL bRedraw);
/* 绘制点线，可以同时绘制点和线
 * bPoint       绘制点
 * bLine        绘制线
 */
void HANPolyLineDrawPointLine(HWND hPolyLine, BOOL bPoint, BOOL bLine, BOOL bRedraw);
/* 裁剪数据（生效后，即 bCut 参数为 TRUE ，在绘制时只会绘制 pPos 参数指定的范围内的数据，该函数不影响绘制区周围的数值，即不影响最值计算）
 * pPos         裁剪数据的位置，该参数应为长度为 2 的数组，值为最小值为 0 的下标，前后顺序不重要，程序会自动判断。
 *              包括下标小值但不包括大值，即该参数为 { 0, 100 } 时，实际绘制的是 [0, 100)
 *              超过数据长度则视作画到最后一个点，即该参数为 { 0, (HANSIZE)(-1) } 时，相当于全画
 *              该范围的对象是所有的数据而非绘图区所映射的数据，若使能了裁剪且对画布进行了放大操作，则画面上仍有可能呈现出填满的情况
 * bCut         是否裁剪，TRUE：裁剪，此时参数 pPos 必须有效，FALSE：取消裁剪，此时忽略参数 pPos
 */
void HANPolyLineCutData(HWND hPolyLine, HANPCSIZE pPos, BOOL bCut, BOOL bRedraw);
/* 设置鼠标悬停数值提示框宽度
 */
void HANPolyLineSetTipW(HWND hPolyLine, HANSIZE w, BOOL bRedraw);
/* 设置鼠标悬停数值提示字体
 * w            字宽（正数即可）
 * h            字高（正数即可）
 * pFace        字体名
 */
HANERROR HANPolyLineSetTipFont(HWND hPolyLine, int w, int h, HANPCSTR pFace, BOOL bRedraw);
/* 设置鼠标悬停数值提示背景颜色
 */
HANERROR HANPolyLineSetTipBkColor(HWND hPolyLine, COLORREF rgb, BOOL bRedraw);
/* 设置鼠标悬停数值提示边框颜色
 */
HANERROR HANPolyLineSetTipBorder(HWND hPolyLine, COLORREF rgb, BOOL bRedraw);

#endif

#if 1 /* HANScatter 散点图
 *
 * 必要操作：
 * 1. 调用 RegisterHANScatter() 注册窗口类
 * 2. 调用 CreateWindow(TEXT("HANScatter"), ...) 创建窗口，省略号内的参数参考 CreateWindow 的标准用法
 * 3. 调用 HANScatterSetDataLen 设置每个变量的数组长度，
 * 4. 调用 HANScatterAddVar 向容口添加变量，变量名可以是长度为 0 的字符串
 *    步骤 3 和 4 原则上不分先后，可以先添加变量再设置变量长度
 * 5. 调用 HANScatterSetPointValueByName 或 HANScatterSetPointValueById 给点变量导入数据
 *    调用 HANScatterSetTipValueByName 或 HANScatterSetTipValueById 给标注变量导入数据
 *    前者以变量名为依据，后者以添加变量时的先后顺序为依据
 *    导入数据所用的数组长度必须与 HANScatterSetDataLen 所设置的长度一致
 * 6. 绘制折线图的必要步骤就是这些了
 * 
 * 例程：
 *  #define x           10
 *  #define y           10
 *  #define w           1000
 *  #define h           500
 *  #define DATA_LEN    100
 *  #define VAR1_NAME   TEXT("var1")
 *  #define VAR2_NAME   TEXT("var2")
 *  #define TIP_NAME    TEXT("tip")
 * 
 *  // 定义全局变量窗口句柄
 *  HWND hScatter;
 * 
 *  // 在创建窗口前执行以下初始化操作
 *      // 注册窗口类
 *      RegisterHANScatter(hInstance);
 *      // 创建窗口实例
 *      hScatter = CreateWindow(
 *          HST_CLASS,              // 窗口类名
 *          TEXT("Scatter demo"),   // 创建的窗口标题
 *             WS_CHILD             // 作为子窗口创建
 *           | WS_VISIBLE           // 创建时可见
 *           | HSTS_POINT           // 绘制点（可与 HSTS_LINE 同时生效）
 *           | HSTS_LINE            // 绘制线（可与 HSTS_POINT 同时生效）
 *           | HSTS_ENABLEMOUSE     // 启用鼠标操作（放大、缩小、拖动）
 *           | HSTS_ENABLETIP       // 启用鼠标悬停提示值
 *           | HSTS_MARKLINE        // 显示标线
 *           | HSTS_MAP,            // 坐标模式（画面不再填满绘图区，而是 1 : 1 的坐标模式）
 *          x, y, w, h,             // 坐标及大小
 *          hWnd,                   // 父窗口句柄
 *          (HMENU)1,               // 窗口编号
 *          hInst,                  // 与窗口相关联的模块实例的句柄
 *          NULL                    // 该窗口不接受参数
 *      );
 *      // 窗口实例创建后，设置属性
 *      HANScatterSetDataLen(hScatter, DATA_LEN);   // 设置数据长度（点数）
 *      HANScatterAddPointVar(hScatter, VAR1_NAME); // 添加点 1
 *      HANScatterAddPointVar(hScatter, VAR2_NAME); // 添加点 2
 *      HANScatterAddTipVar(hScatter, TIP_NAME);    // 添加鼠标悬停提示 3 （Tip 和 Point 的编号是连续计算的，不会共用同一编号），鼠标悬停提示也可添加多个
 * 
 *      // 必要的窗口属性已设置完毕，导入数据即可画出图形
 *      HANPLOTNUM pHData1[DATA_LEN], pVData1[DATA_LEN], pHData2[DATA_LEN], pVData2[DATA_LEN], pTip3[DATA_LEN];
 *      for (size_t i = 0; i < DATA_LEN; i++)
 *      {
 *          pHData1[i] = i;
 *          pVData1[i] = i + 1;
 *          pHData2[i] = DATA_LEN - i;
 *          pVData2[i] = DATA_LEN - i - 1;
 *          pTip3[i] = i + i;
 *      }
 *      HANScatterSetPointValueById(hScatter, 0, pHData1, pVData1); // 导入点 1 的数据
 *      HANScatterSetPointValueById(hScatter, 1, pHData2, pVData2); // 导入点 2 的数据
 *      HANScatterSetTipValueById(hScatter, 2, pTip3);              // 导入鼠标悬停提示的数据
 *      RedrawWindow(hScatter, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW); // 重画窗口，生效新导入的数据
 * 
 * 例程结束
 */

#define HST_CLASS               TEXT("HANScatter") // 窗口类名

#if 1 // 默认参数，这些参数不对外开放，仅在创建窗口时会参考
#define HST_DEF_TIMER_T         50  // 定时器周期，单位 ms（出于效率考虑，窗口不会对所有操作都立即做出重绘操作，而是定时器触发生重绘）
#define HST_DEF_TITLE_W         16  // 标题字宽
#define HST_DEF_TITLE_H         32  // 标题字高
#define HST_DEF_TEXT_W          8   // 文本字宽
#define HST_DEF_TEXT_H          16  // 文本字高
#define HST_DEF_TIP_W           8   // 鼠标悬停提示字宽
#define HST_DEF_TIP_H           16  // 鼠标悬停提示字高
#define HST_DEF_RECT_LEFT       80  // 绘制区域到窗口左边界的距离
#define HST_DEF_RECT_TOP        100 // 绘制区域到窗口上边界的距离
#define HST_DEF_RECT_RIGHT      100 // 绘制区域到窗口右边界的距离
#define HST_DEF_RECT_BOTTOM     50  // 绘制区域到窗口下边界的距离
#define HST_DEF_RECT_DARK       0.9 // 绘制区域暗色背景的系数
#define HST_DEF_MARK_LINE_DARK  0.8 // 标线暗色系数
#define HST_DEF_HVALUE_COUNT    6   // 水平方向数值个数
#define HST_DEF_VVALUE_COUNT    5   // 垂直方向数值个数
#define HST_DEF_REDUCE_SIZE     0.1 // 缩小按钮按下后画面缩小的尺寸
#define HST_DEF_TIP_DIS         5   // 鼠标悬停显示数值的最大像素距离
#endif

#if 1 // 窗口风格，可用或运算组合
#define HSTS_LINE               0x00000001  // 画线，可以和 HSTS_POINT 并用绘制点线
#define HSTS_POINT              0x00000002  // 画点，可以和 HSTS_LINE 并用绘制点线
#define HSTS_ENABLEMOUSE        0x00000004  // 允许鼠标对绘图区域进行放大、拖动等操作
#define HSTS_ENABLETIP          0x00000008  // 使能鼠标悬停数值提示
#define HSTS_MARKLINE           0x00000010  // 显示标线
#define HSTS_MAP                0x00000020  // 以地图模式横纵等比例绘图，同时也会约束放大操作下的控制区
#endif

#if 1 // 窗口消息
typedef enum {
    HSTM_NOTUSED = WM_USER,     // 不使用的消息值，新消息请在下方添加
    HSTM_SETDATALEN,            // 设置数据长度（WPARAM：数据长度，LPARAM：未使用）
    HSTM_ADDPOINTVAR,           // 添加点变量（WPARAM：未使用，LPARAM：变量名字符串）
    HSTM_ADDTIPVAR,             // 添加鼠标县停数值提示变量（WPARAM：未使用，LPARAM：变量名字符串）
    HSTM_SETSTARTDATABYNAME,    // 根据变量名在头部设置数据（WPARAM：变量名字符串，LPARAM：HANPCPLOTDATA 数据）
    HSTM_PUSHSTARTDATABYNAME,   // 根据变量名在头部添加数据，移除最后的数据（WPARAM：变量名字符串，LPARAM：HANPCPLOTDATA 数据）
    HSTM_SETENDDATABYNAME,      // 根据变量名在尾部设置数据（WPARAM：变量名字符串，LPARAM：HANPCPLOTDATA 数据）
    HSTM_PUSHENDDATABYNAME,     // 根据变量名在尾部添加数据，移除最早的数据（WPARAM：变量名字符串，LPARAM：HANPCPLOTDATA 数据）
    HSTM_SETSTARTDATABYID,      // 根据变量ID在头部设置数据（WPARAM：长度，LPARAM：HANPCPLOTDATA 数据）
    HSTM_PUSHSTARTDATABYID,     // 根据变量ID在头部添加数据，移除最后的数据（WPARAM：长度，LPARAM：HANPCPLOTDATA 数据）
    HSTM_SETENDDATABYID,        // 根据变量ID在尾部设置数据（WPARAM：长度，LPARAM：HANPCPLOTDATA 数据）
    HSTM_PUSHENDDATABYID,       // 根据变量ID在尾部添加数据，移除最早的数据（WPARAM：长度，LPARAM：HANPCPLOTDATA 数据）
    HSTM_SETVARCOLORBYNAME,     // 设置变量颜色（WPARAM：颜色，LPARAM：变量名字符串）
    HSTM_SETVARCOLORBYID,       // 设置变量颜色（WPARAM：颜色，LPARAM：起始为 0 的下标值，不接收负数）
    HSTM_SETMAXMIN,             // 设置最值（WPARAM：BOOL 使能 / 禁用设置用户最值，LPARAM：指向 HANSTSETMAXMIN 结构体的指针）
    HSTM_SETBKCOLOR,            // 设置窗口的背景色（WPARAM：COLORREF 颜色，LPARAM：未使用）
    HSTM_SETBKBORDER,           // 设置窗口边框颜色（WPARAM：COLORREF 颜色，LPARAM：未使用）
    HSTM_SETRECTCOLOR,          // 设置绘图区域的背景色（WPARAM：COLORREF 颜色，LPARAM：未使用）
    HSTM_SETRECTBORDER,         // 设置绘图区域边框颜色（WPARAM：COLORREF 颜色，LPARAM：未使用）
    HSTM_SETTITLEFONT,          // 设置标题字体（WPARAM：未使用，LPARAM：指向 LOGFONT 结构体的指针）
    HSTM_SETTEXTFONT,           // 设置文本字体（WPARAM：未使用，LPARAM：指向 LOGFONT 结构体的指针）
    HSTM_SETRECT,               // 设置绘图区域大小（WPARAM：BOOL 使能 / 禁用自动大小模式，LPARAM：RECT*）
    HSTM_GETRECT,               // 获取绘图区域大小（WPARAM：未使用，LPARAM：RECT*，return：BOOL 自动大小模式）
    HSTM_SETHVALCOUNT,          // 设置水平方向数值个数（WPARAM：数值个数，LPARAM：未使用）
    HSTM_SETVVALCOUNT,          // 设置垂直方向数值个数（WPARAM：数值个数，LPARAM：未使用）
    HSTM_SETPOINTLINESIZE,      // 设置线条风格（WPARAM：HANSTPLSIZE 要设置的对象，LPARAM：对象尺寸）
    HSTM_SETPOINTSTYLEBYNAME,   // 设置点风格（WPARAM：变量名字符串，LPARAM：HANHPOINT 点句柄）
    HSTM_SETPOINTSTYLEBYID,     // 设置点风格（WPARAM：起始为 0 的下标值，不接收负数，LPARAM：HANHPOINT 点句柄）
    HSTM_CUTDATA,               // 裁剪数据（WPARAM：BOOL 使能 / 禁用裁剪，LPARAM：HANPCSIZE 裁剪范围）
    HSTM_SETTIPW,               // 设置鼠标悬停数值提示框宽度（WPARAM：HANSIZE w，LPARAM：未使用）
    HSTM_SETTIPFONT,            // 设置鼠标悬停数值提示字体（WPARAM：未使用，LPARAM：指向 LOGFONT 结构体的指针）
    HSTM_SETTIPBKCOLOR,         // 设置鼠标悬停数值提示背景颜色（WPARAM：COLORREF 颜色，LPARAM：未使用）
    HSTM_SETTIPBORDER,          // 设置鼠标悬停数值提示边框颜色（WPARAM：COLORREF 颜色，LPARAM：未使用）
    HSTM_SETMARKDIS,            // 设置标线间隔（WPARAM：BOOL 使能 / 禁用标线间隔，LPARAM：标线间隔）
} HANSCATTERMESSAGE;
#endif

#if 1 // 相关 enum 、struct
typedef enum { // 点线风格
    HST_PLS_LINE,   // 折线
    HST_PLS_POINT,  // 点
} HANSTPLSIZE, * HANPSTPLSIZE;

typedef enum {
    HSTSV_SET_TIP,
    HSTSV_SET_TWO_ARR,
    HSTSV_SET_POINT,
} HSTSVSETMODE;

typedef struct tagHANSTSETVALUE {
    HSTSVSETMODE    nSetMode;
    HANPLOTDATA     pTipData;
    HANPLOTDATA     pHData;
    HANPLOTDATA     pVData;
    HANPCFPOINT     pPoint;
} HANSTSETVALUE, HANPSTSETVALUE;
typedef const HANSTSETVALUE* HANPCSTSETVALUE;

typedef struct tagHANSTSETMAXMIN {
    HANPLOTMAXMIN   hMaxMin;
    HANPLOTMAXMIN   vMaxMin;
} HANSTSETMAXMIN, * HANPSTSETMAXMIN;
typedef const HANSTSETMAXMIN* HANPCSTSETMAXMIN;
#endif

/* 注册散点图类，应用程序在使用此控件前必须先调用该函数注册窗口类
 */
void RegisterHANScatter(HINSTANCE hInstance);
/* 设置数据长度
 * nDataLen     每个变量的数据长度（新创建的窗口未执行此函数长度默认为 0 ，所以要导入数据必须先执行此函数）
 */
HANERROR HANScatterSetDataLen(HWND hScatter, HANSIZE nDataLen);
/* 添加点变量
 * pVarName     新变量的名字
 */
HANERROR HANScatterAddPointVar(HWND hScatter, HANPCSTR pVarName);
/* 添加一个提示变量（该变量不参与绘图，只在鼠标县停数值提示里显示）
 * pVarName     新变量的名字
 */
HANERROR HANScatterAddTipVar(HWND hScatter, HANPCSTR pVarName);
/* 根据变量名在头部设置数据
 * pVarName     变量名
 * pData        数据（长度必须与 HANPolyLineSetDataLen 设置的一致）
 */
HANERROR HANScatterSetStartPointByName(HWND hScatter, HANPCSTR pVarName, HANPCPLOTDATA pHData, HANPCPLOTDATA pVData);
/* 根据变量名在头部添加数据，移除最后的数据
 * pVarName     变量名
 * pData        数据（长度必须与 HANPolyLineSetDataLen 设置的一致）
 */
HANERROR HANScatterPushStartPointByName(HWND hScatter, HANPCSTR pVarName, HANPCPLOTDATA pHData, HANPCPLOTDATA pVData);
/* 根据变量名在尾部设置数据
 * pVarName     变量名
 * pData        数据（长度必须与 HANPolyLineSetDataLen 设置的一致）
 */
HANERROR HANScatterSetEndPointByName(HWND hScatter, HANPCSTR pVarName, HANPCPLOTDATA pHData, HANPCPLOTDATA pVData);
/* 根据变量名在尾部添加数据，移除最早的数据
 * pVarName     变量名
 * pData        数据（长度必须小于等于 HANPolyLineSetDataLen 设置的值）
 */
HANERROR HANScatterPushEndPointByName(HWND hScatter, HANPCSTR pVarName, HANPCPLOTDATA pHData, HANPCPLOTDATA pVData);
/* 根据变量ID在头部设置数据
 * nId          变量 ID
 * pData        数据（长度必须与 HANPolyLineSetDataLen 设置的一致）
 */
HANERROR HANScatterSetStartPointById(HWND hScatter, HANSIZE nId, HANPCPLOTDATA pHData, HANPCPLOTDATA pVData);
/* 根据变量ID在头部添加数据，移除最后的数据
 * nId          变量 ID
 * pData        数据（长度必须与 HANPolyLineSetDataLen 设置的一致）
 */
HANERROR HANScatterPushStartPointById(HWND hScatter, HANSIZE nId, HANPCPLOTDATA pHData, HANPCPLOTDATA pVData);
/* 根据变量ID在尾部设置数据
 * nId          变量 ID
 * pData        数据（长度必须与 HANPolyLineSetDataLen 设置的一致）
 */
HANERROR HANScatterSetEndPointById(HWND hScatter, HANSIZE nId, HANPCPLOTDATA pHData, HANPCPLOTDATA pVData);
/* 根据变量ID在尾部添加数据，移除最早的数据
 * nId          变量 ID
 * pData        数据（长度必须小于等于 HANPolyLineSetDataLen 设置的值）
 */
HANERROR HANScatterPushEndPointById(HWND hScatter, HANSIZE nId, HANPCPLOTDATA pHData, HANPCPLOTDATA pVData);
/* 导入鼠标悬停数值提示数据，需要提供变量名
 * pVarName     变量名
 * pData        数据（数组长度必须与 HANScatterSetDataLen 设置的一致）
 */
HANERROR HANScatterSetTipValueByName(HWND hScatter, HANPCSTR pVarName, HANPCPLOTDATA pData);
/* 导入鼠标悬停数值提示数据，需要提供变量 id
 * nId          变量 id
 * pData        数据（数组长度必须与 HANScatterSetDataLen 设置的一致）
 */
HANERROR HANScatterSetTipValueById(HWND hScatter, HANSIZE nId, HANPCPLOTDATA pData);
/* 修改变量颜色，需要提供变量名
 * pVarName     变量名
 * crColor      颜色
 */
HANERROR HANScatterSetVarColorByName(HWND hScatter, HANPCSTR pVarName, COLORREF rgb, BOOL bRedraw);
/* 修改变量颜色，需要提供变 id
 * nId          变量 id
 * crColor      颜色
 */
HANERROR HANScatterSetVarColorById(HWND hScatter, HANSIZE nId, COLORREF rgb, BOOL bRedraw);
/* 手动设置最大最小值，设置后会强制按此最值绘图，若不设置，则自动计算最值（该功能优先级高于 HSTS_MAP 风格）
 * nHMaxMin     水平方向最值数组地址，大小先后顺序无所谓，程序会自动识别
 * nVMaxMin     垂直方向最值数组地址，大小先后顺序无所谓，程序会自动识别
 * bUserMaxMin  使能 / 禁用最值（TRUE：绘图时会强制使用用户设置的最值，设置的留白空间会失效，
 *                              FALSE：关闭设置的最值，nMaxMin 忽略，绘图时根据数据自动计算最值，会有留白）
 */
void HANScatterSetMaxMin(HWND hScatter, HANPCPLOTMAXMIN hMaxMin, HANPCPLOTMAXMIN vMaxMin, BOOL bUserMaxMin, BOOL bRedraw);
/* 设置窗口的背景色
 */
HANERROR HANScatterSetBackgroundColor(HWND hScatter, COLORREF rgb, BOOL bRedraw);
/* 设置窗口边框颜色
 */
HANERROR HANScatterSetBkBorder(HWND hScatter, COLORREF rgb, BOOL bRedraw);
/* 设置绘图区域的背景色
 */
HANERROR HANScatterSetRectangleColor(HWND hScatter, COLORREF rgb, BOOL bRedraw);
/* 设置绘图区域边框颜色
 */
HANERROR HANScatterSetRectBorder(HWND hScatter, COLORREF rgb, BOOL bRedraw);
/* 设置标题字体
 */
HANERROR HANScatterSetTitleFont(HWND hScatter, const LOGFONT* pTitleFont, BOOL bRedraw);
/* 设置文本字体（一般 h / w = 2 / 1 即是正常比例）
 * w            字宽（正数即可）
 * h            字高（正数即可）
 * pFace        字体名
 */
HANERROR HANScatterSetTextFont(HWND hScatter, const LOGFONT* pTextFont, BOOL bRedraw);
/* 设置绘图区域大小
 * rcRectangle  绘图区的矩形范围
 * bAutoRect    自动大小（TRUE：窗口变化时绘图区会跟着变，此时 rcRectangle 参数代表绘图区到窗口的距离，FALSE：绘图区固定不变，大小就是 rcRectangle ）
 */
void HANScatterSetRcRectangle(HWND hScatter, const RECT* rcRectangle, BOOL bAutoRect, BOOL bRedraw);
/* 获取绘图区域大小（获取的始终是绝对尺寸）
 * rcRectangle  绘图区的矩形范围
 * return       返回的是绘图区的易变属性，TRUE：绘图会随窗口变化而变化，FALSE：绘图区大小固定不变
 */
BOOL HANScatterGetRcRectangle(HWND hScatter, RECT* rcRectangle);
/* 设置水平方向数值个数
 */
void HANScatterSetHValueCount(HWND hScatter, unsigned int n, BOOL bRedraw);
/* 设置垂直方向数值个数
 */
void HANScatterSetVValueCount(HWND hScatter, unsigned int n, BOOL bRedraw);
/* 设置点线大小
 * eObject      要设置的对象（HST_PLS_LINE：设置线。HST_PLS_POINT：设置点）
 * nObjSize     对象尺寸（eLineStyle = HST_PLS_LINE 时，该参数代表线宽。eLineStyle = HST_PLS_POINT 时，该参数代表点的直径）
 */
HANERROR HANScatterSetPointLineSize(HWND hScatter, HANSTPLSIZE eObject, int nObjSize, BOOL bRedraw);
/* 根据变量名设置点风格
 * pVarName     变量名
 * hPoint       点句柄（可以通过 HANPlotGetStockPoint 函数获取系统自带的句柄）
 */
HANERROR HANScatterSetPointStyleByName(HWND hScatter, HANPCSTR pVarName, HANHPOINT hPoint, BOOL bRedraw);
/* 根据变量 id 设置点风格
 * nId          变量 id
 * hPoint       点句柄（可以通过 HANPlotGetStockPoint 函数获取系统自带的句柄）
 */
HANERROR HANScatterSetPointStyleById(HWND hScatter, HANSIZE nId, HANHPOINT hPoint, BOOL bRedraw);
/* 裁剪数据（生效后，即 bCut 参数为 TRUE ，在绘制时只会绘制 pPos 参数指定的范围内的数据，该函数不影响绘制区周围的数值，即不影响最值计算）
 * pPos         裁剪数据的位置，该参数应为长度为 2 的数组，值为最小值为 0 的下标，前后顺序不重要，程序会自动判断。
 *              包括下标小值但不包括大值，即该参数为 { 0, 100 } 时，实际绘制的是 [0, 100)
 *              超过数据长度则视作画到最后一个点，即该参数为 { 0, (HANSIZE)(-1) } 时，相当于全画
 *              该范围的作用对象是所有的数据而非绘图区所映射的数据，若使能了裁剪且对画布进行了放大操作，则画面上仍有可能呈现出填满的情况
 * bCut         是否裁剪，TRUE：裁剪，此时参数 pPos 必须有效，FALSE：取消裁剪，此时忽略参数 pPos
 */
void HANScatterCutData(HWND hScatter, HANPCSIZE pPos, BOOL bCut, BOOL bRedraw);
/* 设置鼠标悬停数值提示框宽度
 */
void HANScatterSetTipW(HWND hScatter, HANSIZE w, BOOL bRedraw);
/* 设置鼠标悬停数值提示字体
 * w            字宽（正数即可）
 * h            字高（正数即可）
 * pFace        字体名
 */
HANERROR HANScatterSetTipFont(HWND hScatter, const LOGFONT* pTipFont, BOOL bRedraw);
/* 设置鼠标悬停数值提示背景颜色
 */
HANERROR HANScatterSetTipBkColor(HWND hScatter, COLORREF rgb, BOOL bRedraw);
/* 设置鼠标悬停数值提示边框颜色
 */
HANERROR HANScatterSetTipBorder(HWND hScatter, COLORREF rgb, BOOL bRedraw);
/* 设置标线间隔值（标线对应的数值，而非像素数）
 * nMarkDis     标线间隔
 * bMarkDis     使能标线间隔
 */
HANERROR HANScatterSetMarkDis(HWND hScatter, HANPLOTNUM nMarkDis, BOOL bMarkDis, BOOL bRedraw);

#endif

#endif

#ifdef __cplusplus
}
#endif

#endif
